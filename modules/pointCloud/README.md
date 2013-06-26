# pointCloud
This module contains classes used to load and handle point cloud data.

Right now the only exported class is a point cloud loader for cyclops

## PointsLoader
This class can be used to load models containing point cloud data in ASCII or binary xyz + RGB format.

To use `PointsLoader`:
```python
from omega import *
from cyclops import *
from pointCloud import *

# Register the points loader
scene = getSceneManager()
scene.addLoader(PointsLoader())

# Load a points cloud using the standard loading command
pointCloudData = ModelInfo()
pointCloudData.name = "points"
pointCloudData.path = "points.xyz"
pointCloudData.optimize = True
scene.loadModel(pointCloudData)
```

### Point shaders
The point clous library comes with a set of shaders to render points as spheres using a geometry shader. The shaders can be loaded as follows:

```python
# The '..' at the beginning is to move up and out of the omega data dir. default shaders for
# pointCloud are inside the modules directory.
shaderPath = "../modules/pointCloud/shaders";
program = ProgramAsset()
program.name = "pointsDepth"
program.vertexShaderName = shaderPath + "/Sphere.vert"
program.fragmentShaderName = shaderPath + "/Sphere.frag"
program.geometryShaderName = shaderPath + "/Sphere.geom"
program.geometryOutVertices = 4
program.geometryInput = PrimitiveType.Points
program.geometryOutput = PrimitiveType.TriangleStrip
scene.addProgram(program)
```

Applying the shader to a point cloud object can be done with the standard `object.getMaterial()setProgram('programName')` command, where `programName` should match the name used by the program above.
