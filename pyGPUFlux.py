import sys
import pyopencl as cl
import pyopencl.tools
import pyopencl.array
import numpy as np
from openalea.plantgl.all import *
import serializer
import lightSerializer
import bvhBuilder
import sensorSerializer
import structfill

SPECTRAL_WAVELENGTH_BINS = 1

class FluxLightModel():
    def __init__(self, aScene) -> None:
        # Type check :
        assert type(aScene) == openalea.plantgl.scenegraph._pglsg.Scene, "Error : input scene is not a PlantGL scene."

        # Attributes :
        self.scene = aScene #PlantGL scene
        self.serializer = serializer.Serializer() #Primitives and detectors serializer
        self.lightSerializer = lightSerializer.LightSerializer(SPECTRAL_WAVELENGTH_BINS) #Light sources serializer
        self.bvhBuilder = bvhBuilder.BVHBuilder() # Primitive Bounding Volume Hierarchy Builder and serializer
        self.sensorSerializer = sensorSerializer.SensorSerializer() #Sensor objects serializer

    # Setters
    def setScene(aScene):
        # Type check :
        assert type(aScene) == openalea.plantgl.scenegraph._pglsg.Scene, "Error : input scene is not a PlantGL scene."
        self.scene = aScene

    # Light serializer shortcuts 
    def addPointLight(self, samples, color, power, spectralCdF):
        self.lightSerializer.addPointLight(samples, color, power, spectralCdF)
    
    def addSpectralLight(self, samples, color, power, spectralCdF, rgb, distribution):
        self.lightSerializer.addSpectralLight(samples, color, power, spectralCdF, rgb, distribution)

    def removeLight(self, index):
        self.lightSerializer.removeLight(index)

    #Sensor serializer shortcuts
    def addSensor(self, groupIndex, matrix, twoSided, color, exponent, bbox):
        self.sensorSerializer.addSensor(groupIndex, matrix, twoSided, color, exponent, bbox)

    def removeSensor(self, index):
        self.sensorSerializer.removeSensor(index)

    #GPUFlux launcher
    def compute(self, nbRays, nthreads, sampleOffset, nsample, measurementBits, skyOffset, depth, minPower, sceneCenter, radius, rgb, power, seed):
        
        #PARAMS BUILDING
        bounds = np.array(1, dtype= [("center", np.float32, 3), ("radius", np.float32)])
        structfill.fillVec3(bounds, "center", sceneCenter)
        bounds["radius"] = radius

        sensivityCurves = np.array(1, dtype= [("rgb", np.float32, 3), ("power", np.float32, SPECTRAL_WAVELENGTH_BINS)])
        structfill.fillVec3(sensivityCurves, "rgb", rgb)
        sensivityCurves["power"] = power

        #INPUT BUFFER CONTENT BUILDING
        prims, primOffsets = self.serializer.serializeTriangleScene(self.scene)
        self.bvhBuilder.buildBVHfromScene(self.scene)
        primBVH = self.bvhBuilder.serializeBVH()
        detectors = self.serializer.serializeDetectors(1)
        lights, lightOffsets, cumLightPower = self.lightSerializer.serialize()
        sensors, sensorBVH = self.sensorSerializer.serialize()
        
        # GPUFLUX CONFIGURATION

        # GPUFlux specific options
        options = " -D MEASURE_FULL_SPECTRUM"
        options += " -D MEASURE_MIN_LAMBDA=380"
        options += " -D MEASURE_MAX_LAMBDA=720"
        options += " -D MEASURE_SPECTRUM_BINS=340"
        options += " -D SPECTRAL_WAVELENGTH_MIN=360"
        options += " -D SPECTRAL_WAVELENGTH_MAX=830"
        options += " -D SPECTRAL_WAVELENGTH_BINS=" + SPECTRAL_WAVELENGTH_BINS
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

        # KERNEL COMPILATION
        context = cl.create_some_context()
        queue = cl.CommandQueue(context)
        
        kernelFile = open("kernel/lightmodel_kernel.cl", "r")
        kernelSource = kernelFile.read()
        kernelFile.close()

        program = cl.Program(context, kernelSource).build(options)
        compute = program.compute

        # INPUT BUFFERS BUILDING

        bufPrim = cl.Buffer(context, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=prims)
        bufPrimOffsets = cl.Buffer(context, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=primOffsets)
        bufPrimBVH = cl.Buffer(context, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=primBVH)
        bufDetectors = cl.Buffer(context, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=detectors)
        bufLights = cl.Buffer(context, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=lights)
        bufLightOffsets = cl.Buffer(context, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=lightOffsets)
        bufCumLightPower = cl.Buffer(context, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=cumLightPower)
        bufSensors = cl.Buffer(context, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=sensors)
        bufSensorBVH = cl.Buffer(context, cl.mem_flags.READ_ONLY | cl.mem_flags.COPY_HOST_PTR, hostbuf=sensorBVH)

        # OUTPUT BUFFERS BUILDING
        bufAbsorbedPower = cl.Buffer(context, cl.mem_flags.WRITE_ONLY, len(self.scene))
        bufIrradiance = cl.Buffer(context, cl.mem_flags.WRITE_ONLY, len(self.sensorSerializer.sensorList))

        compute(queue, (nbRays,), None, nthreads, sampleOffset, nsample, bufAbsorbedPower, bufIrradiance, bufDetectors, measurementBits, len(self.scene), 0, bufPrim, bufPrimOffsets, 0, bufPrimBVH, None, channels, len(self.lightSerializer.lightList), bufLights, bufLightOffsets, bufCumLightPower, skyOffset, len(self.sensorSerializer.sensorList), bufSensors, 0, bufSensorBVH, depth, minPower, bounds, sensivityCurves, seed)