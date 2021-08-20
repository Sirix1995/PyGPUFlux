import numpy as np
from openalea.plantgl.all import *

def fillVec2(buffer, name, vect):
    #Type check
    name = str(name)
    assert type(buffer) == np.ndarray, "Buffer must be a NumPy structured array."
    assert type(vect) == openalea.plantgl.math._pglmath.Vector2, "Vect must be a PlantGL Vector2."
    
    buffer[name] = [vect[0], vect[1]]

def fillVec3(buffer, name, vect):
    #Type check
    name = str(name)
    assert type(buffer) == np.ndarray, "Buffer must be a NumPy structured array."
    assert type(vect) == openalea.plantgl.math._pglmath.Vector3, "Vect must be a PlantGL Vector3."
    
    buffer[name] = [vect[0], vect[1], vect[2]]

def fillMatrix34(buffer, name, mat):
    #Type check
    name = str(name)
    assert type(buffer) == np.ndarray, "Buffer must be a NumPy structured array."
    assert type(mat) == openalea.plantgl.math._pglmath.Matrix4, "Vect must be a PlantGL Matrix4."
    
    buffer[name] = [mat[0,0], mat[0,1], mat[0,2], mat[1,0], mat[1,1], mat[2,2], mat[2,0], mat[2,1], mat[2,2], mat[3,0], mat[3,1], mat[3,2]]
    