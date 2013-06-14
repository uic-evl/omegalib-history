# Basic example showing how to load a static model, and display it in a basic scene.
from math import *
from euclid import *
from omega import *
from omegaToolkit import *
from cyclops import *

actors = []

# create a custom actor class
class SpinningCube(Actor):
	cube = None
	def __init__(self, position):
		actors.append(self)
		super(SpinningCube, self).__init__("spinCube")
		self.cube = BoxShape.create(0.5, 0.5, 0.5)
		self.cube.setPosition(position)
		self.cube.setEffect('colored -d #ff8080')
		self.setUpdateEnabled(True)
		
	def dispose(self):
		actors.remove(self)
		self.cube.getParent().removeChildByRef(self.cube)
		print('deleting spinningcube')
	
	def __del__(self):
		print("dtor")
		
	def onUpdate(self, frame, time, dt):
		self.cube.yaw(dt)
		self.cube.pitch(sin(time) / 5.0)

scene = getSceneManager()

# Create first light
light1 = Light.create()
light1.setColor(Color(0.8, 0.8, 0.8, 1))
light1.setPosition(Vector3(0, 4, -4))
light1.setEnabled(True)

# Create second light
light2 = Light.create()
light2.setColor(Color(0.8, 0.8, 0.1, 1))
light2.setPosition(Vector3(0, 4, 4))
light2.setEnabled(True)

#------------------------------------------------------------------------------
def onEvent():
	e = getEvent()
	if(e.getServiceType() == ServiceType.Pointer or e.getServiceType() == ServiceType.Wand):
		# Button mappings are different when using wand or mouse
		confirmButton = EventFlags.Button2
		if(e.getServiceType() == ServiceType.Wand): confirmButton = EventFlags.Button5
		
		# When the confirm button is pressed:
		if(e.isButtonDown(confirmButton)):
			r = getRayFromEvent(e)
			# Place objects 5 meters away from pointer.
			pos = r[1] + r[2] * 5
			print(pos)
			SpinningCube(pos)
			#s = SpinningCube(pos)
			#s.setUpdateEnabled(True)

setEventFunction(onEvent)




