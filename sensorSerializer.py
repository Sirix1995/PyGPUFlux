import sys
import pyopencl as cl
import pyopencl.tools
import pyopencl.array
import numpy as np
from openalea.plantgl.all import *
import structfill
import aabbtree
import bvhBuilder

class SensorSerializer():
    def __init__(self) -> None:
        #Structure
        self.sensor = [("groupIndex", np.int32), ("WtOMatrix", np.float32, 12), ("twoSided", np.bool8), ("color", np.float32, 3), ("exponent", np.float32)]

        #Attributes
        self.tree = aabbtree.AABBTree()
        self.sensorList = []

    # Add a sensor object to the list (also add it in the BVH)
    def addSensor(self, groupIndex, matrix, twoSided, color, exponent, bbox):
            #First we add the sensor to the tree thanks to the bounding box
            aabb = bvhBuilder.plantGLBBtoAABB(bbox)
            self.tree.add(aabb, len(self.sensorList))

            #Then we add it to the list
            self.sensorList.append({"groupIndex": groupIndex, "WtOMatrix": matrix, "twoSided": twoSided, "color": color, "exponent": exponent, "bbox": bbox})

    # Remove sensor at index in the list
    def removeSensor(self, index):
        #We remove the sensor from the list 
        self.sensorList.pop(index)

        #Now, we need to recreate the tree
        self.tree = aabbtree.AABBTree()
        for i, sensor in enumerate(self.sensorList):
            self.tree.add(sensor["bbox"], i)

    # Serialize a sensor
    def serializeSensor(self, groupIndex, matrix, twoSided, color, exponent):
        buffer = np.array(1, dtype=self.sensor)

        buffer["groupIndex"].fill(groupIndex)
        structfill.fillMatrix34(buffer,"WtOMatrix", matrix)
        buffer["twoSided"].fill(twoSided)
        structfill.fillVec3(buffer, "color", color)
        buffer["exponent"].fill(exponent)
        
        return buffer.tobytes()

    # Serialize the sensor list and the BVH
    def serialize(self):

        #Serializing the sensors
        sensorsInByte = self.serializeSensor(self.sensorList[0]["groupIndex"], self.sensorList[0]["WtOMatrix"], self.sensorList[0]["twoSided"], self.sensorList[0]["color"], self.sensorList[0]["exponent"])
        for sensor in self.sensorList[1:]:
            sensorsInByte+= self.serializeSensor(sensor["groupIndex"], sensor["WtOMatrix"], sensor["twoSided"], sensor["color"], sensor["exponent"])


        #Serializing the BVH
        builder = bvhBuilder.BVHBuilder()
        builder.setBVH(self.tree)
        bvhInBytes = builder.serializeBVH()

        return sensorsInByte, bvhInBytes

    # Testing method
    def test(self):
        self.addSensor(0, Matrix4((1, 0, 0, 0 , 0, 1, 0, 0 , 0, 0, 1, 0 , 0, 0, 0, 1)), False, Vector3(1.0, 1.0, 1.0), 1.0, BoundingBox(Vector3(0.0,1.0,0.0),Vector3(1.0,1.0,0.0)))

        self.removeSensor(0)

        self.addSensor(0, Matrix4((1, 0, 0, 0 , 0, 1, 0, 0 , 0, 0, 1, 0 , 0, 0, 0, 1)), False, Vector3(1.0, 1.0, 1.0), 1.0, BoundingBox(Vector3(0.0,1.0,0.0),Vector3(1.0,1.0,0.0)))

        sensors, bvh = self.serialize()

        print("Sensors : ", sensors)
        print("BVH : ", bvh)

if __name__ == __main__:
    seri = sensorSerializer()
    seri.test()
