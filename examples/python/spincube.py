# basic omegalib script: just display a textured spinning cube.
from omega import *
from cyclops import *

box = BoxShape.create(0.8, 0.8, 0.8)
box.setPosition(Vector3(0, 2, -3))

# Apply an emissive textured effect (no lighting)
box.setEffect("textured -v emissive -d cyclops/test/omega-transparent.png")

# Spin the box!
def onUpdate(frame, t, dt):
	global pitch, yaw
	if(pitch == True): box.pitch(dt)
	if(yaw == True): box.yaw(dt / 3)
setUpdateFunction(onUpdate)

