# PyGPUFlux
PyGPUFlux is an implementation of the OpenCL raytracing engine GPUFlux, using the PlantGL Librairy. Actually, it's still in progress.

# Technologies used
Python
OpenCL
PyOpenCL
PlantGL
AABBTree library

# How to use
If you want to use the class, you need to import the pyGPUFlux.py file, and create a FluxLightModel object. At the object creation, you will specify the PlantGL scene you want to use.

When the FluxLightModel is created, now you add some light sources and some sensor objects. When everything is ready, you can call GPUFlux with the "compute" method.

# Code map
This implementation is made with several python scrips :

pyGPUFlux.py : main script, used by user to call everything.
serializer.py : primitive and detectors serializer, actually it can parse a PlantGL TriangleSet scene to a bytechain needed for the OpenCL buffer.
lightSerializer.py : light sources serializer. When created, you can spécify some lights to it, and then serialize them to a bytechain.
sensorSerializer.py : same thing, but for sensor objects. Sensor are used to measure the irradiance of an area.
bvhBuilder.py : this class is made to build BVHs (bounding volume hierarchy structures) and serialize them. GPUFlux needs to BVHs : one for primitives and one for sensors.
structfill.py : a script used to fill NumPy arrays with some data.

# What's working now and what's not.

Actually, all of the serializers are working. You can test them by running their test method (light serializer does not have one). You can set the main class FluxLightModel and configure it, but the OpenCL call to GPUFlux is not finished, channels needs to be implemented. This class also needs to be tested.
