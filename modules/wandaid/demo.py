########################################################################
# (C) 2013 - Jason Leigh, Electronic Visualization Laboratory, University of Illinois at Chicago
# Version 7/4/2013
#
# Demo application to show the use of Wandaid
# Look for comments below that say ADD THIS TO USE WANDAID
# to find out how to use it in your own programs.
#
from math import *
from euclid import *
from omega import *
from cyclops import *
from omegaToolkit import *

########################################################################
# ADD THIS TO USE WANDAID
from wandaid import wandaid

########################################################################
# Make a scene.

g_scene = getSceneManager()

# Create a light
g_light = Light.create()
g_light.setColor(Color("#FFFFFF"))
g_light.setAmbient(Color("#303030"))
g_light.setPosition(Vector3(-5,5,5))
g_light.setEnabled(True)

# Load a static model
g_sceneModel = ModelInfo()
g_sceneModel.name = "platform"
g_sceneModel.path = "walkplatform.fbx"
g_scene.loadModel(g_sceneModel)

# Create a scene object using the loaded model
g_platform = StaticObject.create("platform")
g_platform.setSelectable(True)
g_platform.setPosition(Vector3(0, 0,0))
g_platform.setEffect("colored")
g_platform.setEffect("textured")

########################################################################
# ADD THIS TO USE WANDAID
# Set to False to run in CAVE2 mode and True for simulator mode. By default it is set to CAVE2.
# Must do this before calling wandaid.init() if you plan to use simulator mode.
wandaid.setSimulator(True)

# Initialize wandaid
wandaid.init(g_scene,getDefaultCamera())



########################################################################
# ADD THIS TO USE WANDAID
# Set the labels
# To remove a label set label as ""
wandaid.setJoystickLabel("Drive")
wandaid.setShoulderLabel("Rotate")
#wandaid.setShoulderLabel("")
wandaid.setTriggerLabel("Fly")
wandaid.setLeftButtonLabel("Menu Select")
wandaid.setRightButtonLabel("Menu Open")
wandaid.setDPADUpLabel("Scale Up")
wandaid.setDPADDownLabel("Scale\nDown")
wandaid.setDPADLeftLabel("Left")
wandaid.setDPADRightLabel("Right")

# You're going to need to have an onUpdate function in which you add the wandaid.update call.
def onUpdate(frame, time, dt):

	########################################################################
	# ADD THIS TO USE WANDAID
	wandaid.update(dt)

# Tell Omegalib about the onUpdate function
setUpdateFunction(onUpdate)
