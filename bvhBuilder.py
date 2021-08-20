import sys
import pyopencl as cl
import pyopencl.tools
import pyopencl.array
import numpy as np
from openalea.plantgl.all import *
import structfill
import aabbtree

# Convert a PlantGL BoundingBox to an AABBTree BoundingBox
def plantGLBBtoAABB(bb):
    # Type check
    assert type(bb) == BoundingBox, "Param is not a PlantGL BoundingBox"

    return aabbtree.AABB([(bb.getXMin(), bb.getXMax()), (bb.getYMin(), bb.getYMax()), (bb.getZMin(), bb.getZMax())])

class BVHBuilder():
    def __init__(self):

        # Structures
        # left BBox XY min and Max, both BBOx Z min and max, and right BBox XY min and Max
        self.firstInfos = [("n0xy", np.float32, 4), ("nz", np.float32, 4), ("n1xy", np.float32, 4)]
        # left child and right child index
        self.branch = self.firstInfos + [("c0idx", np.int32), ("c1idx", np.int32)]
        # primitive index, and primitive count
        self.leaf = self.firstInfos + [("idx", np.int32), ("pcount", np.int32)]

        # Attributes
        self.tree = aabbtree.AABBTree()
        self.serializedNodes = 0

    # Build the BVH from a PlantGL Scene using AABBTree
    def buildBVHfromScene(self, scene):
        # Type check
        assert type(scene) == openalea.plantgl.scenegraph._pglsg.Scene, "Tree must be constructed with a PlantGL scene."

        for i, shape in enumerate(scene):
            bb = BoundingBox(shape)
            aabb = plantGLBBtoAABB(bb)
            self.tree.add(aabb, i)

    # Set the BVHTree from an external source
    def setBVH(self, aTree):
        #Type check
        assert type(aTree) == aabbtree.AABBTree, "External BVH tree must be made with AABBTree library."
        self.tree = aTree

    # Serialize the BVH
    def serializeBVH(self):
        bvhInBytes, tampon, tampon2 = self.serializeBVHRec(self.tree)
        return bvhInBytes

    # Serialize a node
    # If node is a leaf, value1 refers to the prim index, and value2 refers to pcount.
    # If node isn't a leaf, value1 refers to left child index, and value2 to right child index.
    def serializeNode(self, isLeaf, value1, value2, n0xy, nz, n1xy):
        buffer = np.array(1, dtype=self.leaf if isLeaf else self.branch)

        if isLeaf:
            buffer["idx"].fill(value1)
            buffer["pcount"].fill(value2)
        else:
            buffer["c0idx"].fill(value1)
            buffer["c1idx"].fill(value2)

        buffer["n0xy"] = n0xy
        buffer["nz"] = nz
        buffer["n1xy"] = n1xy

        print("Value 1 : ", value1)
        print("Value 2 : ", value2)

        return buffer.tobytes()

    # Recursive call to serialize the tree 
    def serializeBVHRec(self, node):
        bufferInBytes = None
        pcount = 0

        if node.is_leaf:
            bufferInBytes = self.serializeNode(True, node.value, 1, [-1.0, -1.0, -1.0, -1.0], [-1.0, -1.0, -1.0, -1.0], [-1.0, -1.0, -1.0, -1.0])
            pcount = 1
        else:
            tamponGauche, c0idx, pcountLeft = self.serializeBVHRec(node.left)
            tamponDroit, c1idx, pcountRight = self.serializeBVHRec(node.right)
            
            pcount = pcountLeft + pcountRight

            if node.left.is_leaf:
                c0idx = -c0idx - 1

            if node.right.is_leaf:
                c1idx = -c1idx - 1

            bufferInBytes = self.serializeNode(False, c0idx, c1idx, [node.left.aabb.limits[0][0], node.left.aabb.limits[0][1], node.left.aabb.limits[1][0], node.left.aabb.limits[1][1]], [node.left.aabb.limits[2][0], node.left.aabb.limits[2][1], node.right.aabb.limits[2][0], node.right.aabb.limits[2][1]], [node.right.aabb.limits[0][0], node.right.aabb.limits[0][1], node.right.aabb.limits[1][0], node.right.aabb.limits[1][1]])

            bufferInBytes = bufferInBytes + tamponGauche + tamponDroit

        self.serializedNodes+= 1

        return bufferInBytes, self.serializedNodes, pcount

    # Testing method
    def test(self):
        points = [(0.0, 0.0, 0.0),
                  (0.0, 1.0, 0.0),
                  (1.0, 0.0, 0.0),
                  (1.0, 1.0, 1.0)]

        indices = [(0, 1, 2),
                   (0, 1, 3),
                   (0, 2, 3),
                   (1, 2, 3)]

        tetra = TriangleSet(points, indices)
        boule = Sphere(2)
        tessel = Tesselator()
        boule.apply(tessel)
        triBoule = tessel.triangulation
        scene = Scene()
        scene.add(tetra)
        scene.add(triBoule)

        self.buildBVHfromScene(scene)
        print(builder.tree)

        bytechain = builder.serializeBVH()

        # GPUFlux specific options
        options = " -D MEASURE_FULL_SPECTRUM"
        options += " -D MEASURE_MIN_LAMBDA=380"
        options += " -D MEASURE_MAX_LAMBDA=720"
        options += " -D MEASURE_SPECTRUM_BINS=340"
        options += " -D SPECTRAL_WAVELENGTH_MIN=360"
        options += " -D SPECTRAL_WAVELENGTH_MAX=830"
        options += " -D SPECTRAL_WAVELENGTH_BINS=1"
        options += " -D BVH"
        options += " -D ENABLE_SENSORS"

        # OpenCL config options
        options += " -D CL_KHR_GLOBAL_INT32_BASE_ATOMICS"
        options += " -D CL_KHR_GLOBAL_INT32_EXTENDED_ATOMICS"
        #options += " -D CL_KHR_LOCAL_INT32_BASE_ATOMICS"
        #options += " -D CL_KHR_LOCAL_INT32_EXTENDED_ATOMICS"
        #options += " -D CL_KHR_FP64"
        #options += " -D CL_KHR_BYTE_ADDRESSABLE_STORE"
        #options += " -D CL_KHR_ICD"
        #options += " -D CL_KHR_GL_SHARING"
        #options += " -D CL_NV_COMPILER_OPTIONS"
        #options += " -D CL_NV_DEVICE_ATTRIBUTE_QUERY"
        #options += " -D CL_NV_PRAGMA_UNROLL"
        #options += " -D CL_NV_COPY_OPTS"
        #options += " -D CL_KHR_GL_EVENT"
        #options += " -D CL_NV_CREATE_BUFFER"
        options += " -D CL_KHR_INT64_BASE_ATOMICS"
        #options += " -D CL_KHR_INT64_EXTENDED_ATOMICS"

        # Directory option
        options += " -I kernel/"

        context = cl.create_some_context()
        queue = cl.CommandQueue(context)

        taille = len(bytechain) / 56
        taille = int(taille)

        kernelSource =  """
                        #include "trace/bvh/bvh.h"

                        __kernel void structTest(__global char* buffer, __global int* value1, __global int* value2) {
                            int i = get_global_id(0);
                            int offset = i * 56;
                            const __global BVHNode* node = (const __global BVHNode*)(buffer + offset);
                            if(node->nz.s0 == -1.0) {
                                value1[i] = node->idx;
                                value2[i] = node->pcount;
                            } else {
                                value1[i] = node->c0idx;
                                value2[i] = node->c1idx;
                            }

                            printf("Value 1 for Node %d : %d ", i, value1[i]);
                            printf("Value 2 for Node %d : %d ", i, value2[i]);
                        }
                        """

        #Input buffer
        bufTree = cl.Buffer(context, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=bytechain)

        #Output buffers
        bufValue1 = cl.Buffer(context, cl.mem_flags.WRITE_ONLY, taille)
        bufValue2 = cl.Buffer(context, cl.mem_flags.WRITE_ONLY, taille)

        program = cl.Program(context, kernelSource).build(options)

        program.structTest(queue, (taille,), None, bufTree, bufValue1, bufValue2)

        value1 = np.empty(taille, np.int32)
        value2 = np.empty(taille, np.int32)

        cl.enqueue_copy(queue, value1, bufValue1)
        cl.enqueue_copy(queue, value2, bufValue2)

        print(value1)
        print(value2)

if __name__ == __main__:
    builder = BVHBuilder()
    builder.test()