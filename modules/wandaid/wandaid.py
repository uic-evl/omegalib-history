## @package wandaid
# This class creates a 3D model of a CAVE2 wand and labels it to give the user instructions on what
# the buttons do.
# It is enabled by raising one's wand above 1.5 meters.
#
# Contains only static members and member functions so you do not need to create any objects of this class to use it.
# To import it into your own program:
#	from wandaid import wandaid
#
# If you are running in simulator mode make sure to call:
#	wandaid.setSimulator(True)	# do this before you do the init()
#
# To initialize call:
#	wandaid.init(scene,getDefaultCamera())
#
# Then use it in your onUpdate function as follows:
#	wandaid.update(dt) -- where dt is the dt given to you by Omegalib on each frame update.
#
# You can also set the position of wandaid relative to the physical wand with:
# wandaid.setOffsetFromWand(x,y,z)
#
# To set the labels use for example:
# wandaid.setJoystickLabel("Drive")
#
# wandaid.setJoystickLabel("") 		# Set to null string to clear out the label
#
# wandaid.setShoulderLabel("Rotate")
#
# wandaid.setTriggerLabel("Fly")
#
# wandaid.setLeftButtonLabel("Menu Select")
#
# wandaid.setRightButtonLabel("Menu Open")
#
# wandaid.setDPADUpLabel("Scale Up")
#
# wandaid.setDPADDownLabel("Scale Down")
#
# wandaid.setDPADLeftLabel("Left")
#
# wandaid.setDPADRightLabel("Right")
#
# (C) 2013 - Jason Leigh, Electronic Visualization Laboratory, University of Illinois at Chicago
# Original Version 7/3/2013
# Current Version 7/7/2013

from math import *
from euclid import *
from omega import *
from cyclops import *
from omegaToolkit import *

## wandaid class
# This class creates a 3D model of a CAVE2 wand and labels it to give the user instructions on what the buttons do.
# It's kind of like a tooltip for the wand.
# In CAVE2 when you raise your wand more than 1.5 meters wandaid will appear. Then disappear when you lower your wand.
# Note:
# To use this class you do not instantiate a class object. Instead you directly use the static member functions.
# To set the labels for the wand use the appropriate member function.
#
class wandaid:
	__SIM = False
	
	__wandOffset = Vector3(-0.1,-0.3,0)
	__wandModel = None
	__wandObj = None
	__mainCamera = None
	__light = None
	__activationHeight = 1.5
	__isEnabled = True
	__shoulder = None
	__trigger = None
	__joystick = None
	__leftButton = None
	__rightButton = None
	__leftDPAD = None
	__rightDPAD = None
	__topDPAD = None
	__bottomDPAD = None
	__dummyNode = None
	
	__labelJoystick = "Drive"
	__textJoystick = None
	__labelLeftButton = "Select Menu"
	__textLeftButton = None
	__labelRightButton = "Open Menu"
	__textRightButton = None
	__labelTrigger = "Fly"
	__textTrigger = None
	__labelTopButton = "Rotate"
	__textTopButton = None
	__labelDPADUp = "Scale Up"
	__textDPADUp = None
	__labelDPADDown="Scale Down"
	__textDPADDown = None
	__labelDPADLeft="Left"
	__textDPADLeft=None
	__textDPADRight=None
	__labelDPADRight="Right"
	__scale = 0
	
	@staticmethod
	## Initialize wandaid.
	# @param scene the Omegalib scene. Usually obtained with getSceneManager()
	# @param camera the Omegalib camera. Usually you pass it getDefaultCamera()
	def init(scene, camera):

		wandaid.__wandModel=ModelInfo()
		wandaid.__wandModel.name = "wandaid model"
		wandaid.__wandModel.path="resources-wandaid/wandanim.fbx"
		scene.loadModel(wandaid.__wandModel)
		wandaid.__wandObj = AnimatedObject.create("wandaid model")
		wandaid.__wandObj.setEffect("colored")
		#wandaid.__wandObj.setEffect("textured")
		wandaid.__wandObj.loopAnimation(0)
	
		wandaid.__shoulderModel = ModelInfo()
		wandaid.__shoulderModel.name="shoulder"
		wandaid.__shoulderModel.path="resources-wandaid/shoulder.fbx"
		scene.loadModel(wandaid.__shoulderModel)
		wandaid.__shoulder = StaticObject.create("shoulder")
		wandaid.__shoulder.setEffect("textured")
		wandaid.__wandObj.addChild(wandaid.__shoulder)
		wandaid.__shoulder.setVisible(False)

		wandaid.__triggerModel = ModelInfo()
		wandaid.__triggerModel.name="trigger"
		wandaid.__triggerModel.path="resources-wandaid/trigger.fbx"
		scene.loadModel(wandaid.__triggerModel)
		wandaid.__trigger = StaticObject.create("trigger")
		wandaid.__trigger.setEffect("textured")
		wandaid.__wandObj.addChild(wandaid.__trigger)
		wandaid.__trigger.setVisible(False)

		wandaid.__rightButtonModel = ModelInfo()
		wandaid.__rightButtonModel.name="rightButton"
		wandaid.__rightButtonModel.path="resources-wandaid/rightbutton.fbx"
		scene.loadModel(wandaid.__rightButtonModel)
		wandaid.__rightButton = StaticObject.create("rightButton")
		wandaid.__rightButton.setEffect("textured")
		wandaid.__wandObj.addChild(wandaid.__rightButton)
		wandaid.__rightButton.setVisible(False)
		
		wandaid.__joystickModel = ModelInfo()
		wandaid.__joystickModel.name="joystick"
		wandaid.__joystickModel.path="resources-wandaid/joystick.fbx"
		scene.loadModel(wandaid.__joystickModel)
		wandaid.__joystick = StaticObject.create("joystick")
		wandaid.__joystick.setEffect("textured")
		wandaid.__wandObj.addChild(wandaid.__joystick)
		wandaid.__joystick.setVisible(False)

		wandaid.__leftButtonModel = ModelInfo()
		wandaid.__leftButtonModel.name="leftButton"
		wandaid.__leftButtonModel.path="resources-wandaid/leftbutton.fbx"
		scene.loadModel(wandaid.__leftButtonModel)
		wandaid.__leftButton = StaticObject.create("leftButton")
		wandaid.__leftButton.setEffect("textured")
		wandaid.__wandObj.addChild(wandaid.__leftButton)
		wandaid.__leftButton.setVisible(False)

		wandaid.__topDPADModel = ModelInfo()
		wandaid.__topDPADModel.name="topDPAD"
		wandaid.__topDPADModel.path="resources-wandaid/topdpad.fbx"
		scene.loadModel(wandaid.__topDPADModel)
		wandaid.__topDPAD = StaticObject.create("topDPAD")
		wandaid.__topDPAD.setEffect("textured")
		wandaid.__wandObj.addChild(wandaid.__topDPAD)
		wandaid.__topDPAD.setVisible(False)

		wandaid.__bottomDPADModel = ModelInfo()
		wandaid.__bottomDPADModel.name="bottomDPAD"
		wandaid.__bottomDPADModel.path="resources-wandaid/bottomdpad.fbx"
		scene.loadModel(wandaid.__bottomDPADModel)
		wandaid.__bottomDPAD = StaticObject.create("bottomDPAD")
		wandaid.__bottomDPAD.setEffect("textured")
		wandaid.__bottomDPAD.setPosition(Vector3(0,-0.004,0))
		wandaid.__wandObj.addChild(wandaid.__bottomDPAD)
		wandaid.__bottomDPAD.setVisible(False)

		wandaid.__leftDPADModel = ModelInfo()
		wandaid.__leftDPADModel.name="leftDPAD"
		wandaid.__leftDPADModel.path="resources-wandaid/leftdpad.fbx"
		scene.loadModel(wandaid.__leftDPADModel)
		wandaid.__leftDPAD = StaticObject.create("leftDPAD")
		wandaid.__leftDPAD.setEffect("textured")
		wandaid.__leftDPAD.setPosition(Vector3(0,-0.002,0))
		wandaid.__wandObj.addChild(wandaid.__leftDPAD)
		wandaid.__leftDPAD.setVisible(False)

		wandaid.__rightDPADModel = ModelInfo()
		wandaid.__rightDPADModel.name="rightDPAD"
		wandaid.__rightDPADModel.path="resources-wandaid/rightdpad.fbx"
		scene.loadModel(wandaid.__rightDPADModel)
		wandaid.__rightDPAD = StaticObject.create("rightDPAD")
		wandaid.__rightDPAD.setEffect("textured")
		wandaid.__rightDPAD.setPosition(Vector3(0,-0.002,0))
		wandaid.__wandObj.addChild(wandaid.__rightDPAD)
		wandaid.__rightDPAD.setVisible(False)

		# Apply wand tracking to a dummy node so that the actual wand object can be offset from it
		# if desired. Actual wand object is added as a child to the dummy node.
		wandaid.__dummyNode = BoxShape.create(1,1,1)
		wandaid.__dummyNode.followTrackable(1)

		if wandaid.__SIM:
			wandaid.__wandObj.setPosition(Vector3(0,2.1,-1) + wandaid.__wandOffset)
			camera.addChild(wandaid.__wandObj)
		else:
			wandaid.__wandObj.setPosition(wandaid.__wandOffset)
			wandaid.__wandObj.pitch(radians(-90))
			wandaid.__dummyNode.addChild(wandaid.__wandObj)
			
		camera.addChild(wandaid.__dummyNode)
		wandaid.__dummyNode.setVisible(False)
		wandaid.__wandObj.setVisible(True)
		wandaid.__mainCamera = camera
		
		wandaid.__light = Light.create()
		wandaid.__light.setColor(Color(1,0.8,0.8,1))
		wandaid.__light.setAmbient(Color("#303030"))
		wandaid.__light.setPosition(Vector3(2,2,2))
		wandaid.__light.setEnabled(False)
		camera.addChild(wandaid.__light)

	@staticmethod
	## Run in either simulation or real CAVE2 mode.
	# @param bool True for simulator, False for CAVE2.
	#
	def setSimulator(bool):
		wandaid.__SIM=bool
		
	@staticmethod
	## Offset wandaid from the actual position of the wand. Default is (-0.1,-0.3,0)
	# @param x X position
	# @param y Y position
	# @param z Z position
	def setOffsetFromWand(x,y,z):
		wandaid.__wandOffset = Vector3(x,y,z)
		wandaid.__wandObj.setPosition(wandaid.__wandOffset)
		
	@staticmethod
	## Enables and disables wandaid
	# @param bool Set to either True or False.
	def setEnabled(bool):
		wandaid.__isEnabled = bool

	@staticmethod
	def __textSize(str):
		return len(str)*0.01
		
	@staticmethod
	## Set label for wand's joystick.
	# @param str String to use for the label. Set str to "" to remove a label.
	def setJoystickLabel(str):

		if (wandaid.__textJoystick != None):
			wandaid.__wandObj.removeChildByRef(wandaid.__textJoystick)

		if len(str) != 0:
			wandaid.__joystick.setVisible(True)
		else:
			wandaid.__joystick.setVisible(False)
			
		wandaid.__labelJoystick = str
		text = Text3D.create('resources-wandaid/wandaid.ttf',1,wandaid.__labelJoystick)
		text.setScale(Vector3(0.01,0.01,0.01))
		min=text.getBoundMinimum()
		max=text.getBoundMaximum()
		size = max-min
		
		text.setPosition(Vector3(-size.x-0.025,0.087,0.04))
		text.setFontResolution(120)
		text.setColor(Color('white'))
		wandaid.__wandObj.addChild(text)
		wandaid.__textJoystick = text

	@staticmethod
	## Set label for wand's shoulder button.
	# @param str String to use for the label. Set str to "" to remove a label.
	def setShoulderLabel(str):
	
		if (wandaid.__textTopButton != None):
			wandaid.__wandObj.removeChildByRef(wandaid.__textTopButton)
		
		if len(str) != 0:
			wandaid.__shoulder.setVisible(True)
		else:
			wandaid.__shoulder.setVisible(False)
			
		wandaid.__labelTopButton = str
		text = Text3D.create('resources-wandaid/wandaid.ttf',1,wandaid.__labelTopButton)
		text.setPosition(Vector3(0.037,0.083,-0.02))
		text.setScale(Vector3(0.01,0.01,0.01))
		text.setFontResolution(120)
		text.setColor(Color('white'))
		#text.setColor(Color(1,0.3,0.3,1))
		wandaid.__wandObj.addChild(text)
		wandaid.__textTopButton = text
	
	@staticmethod
	## Set label for wand's trigger button.
	# @param str String to use for the label. Set str to "" to remove a label.
	def setTriggerLabel(str):

		if (wandaid.__textTrigger != None):
			wandaid.__wandObj.removeChildByRef(wandaid.__textTrigger)

		if len(str) != 0:
			wandaid.__trigger.setVisible(True)
		else:
			wandaid.__trigger.setVisible(False)
			
		wandaid.__labelTrigger = str
		text = Text3D.create('resources-wandaid/wandaid.ttf',1,wandaid.__labelTrigger)
		text.setPosition(Vector3(0.045,0.065,-0.02))
		text.setScale(Vector3(0.01,0.01,0.01))
		text.setFontResolution(120)
		text.setColor(Color('white'))
		#text.setColor(Color(1,0.3,0.3,1))
		wandaid.__wandObj.addChild(text)
		wandaid.__textTrigger = text
	
	@staticmethod
	## Set label for wand's right button.
	# @param str String to use for the label. Set str to "" to remove a label.
	def setRightButtonLabel(str):
	
		if (wandaid.__textRightButton != None):
			wandaid.__wandObj.removeChildByRef(wandaid.__textRightButton)
			
		if len(str) != 0:
			wandaid.__rightButton.setVisible(True)
		else:
			wandaid.__rightButton.setVisible(False)
			
		wandaid.__labelRightButton = str
		text = Text3D.create('resources-wandaid/wandaid.ttf',1,wandaid.__labelRightButton)
		text.setPosition(Vector3(0.035,0.042,0.02))
		text.setScale(Vector3(0.01,0.01,0.01))
		text.setFontResolution(120)
		text.setColor(Color('white'))
		#text.setColor(Color(0,0.5,1,1))
		wandaid.__wandObj.addChild(text)
		wandaid.__textRightButton = text

	@staticmethod
	## Set label for wand's left.
	# @param str String to use for the label. Set str to "" to remove a label.
	def setLeftButtonLabel(str):

		if (wandaid.__textLeftButton != None):
			wandaid.__wandObj.removeChildByRef(wandaid.__textLeftButton)

		if len(str) != 0:
			wandaid.__leftButton.setVisible(True)
		else:
			wandaid.__leftButton.setVisible(False)
			
		wandaid.__labelLeftButton = str
		text = Text3D.create('resources-wandaid/wandaid.ttf',1,wandaid.__labelLeftButton)

		text.setScale(Vector3(0.01,0.01,0.01))
		min=text.getBoundMinimum()
		max=text.getBoundMaximum()
		size = max-min
		
		text.setPosition(Vector3(-size.x-0.032,0.042,0.02))

		text.setFontResolution(120)
		text.setColor(Color('white'))
		#text.setColor(Color(0,0.5,1,1))
		wandaid.__wandObj.addChild(text)
		wandaid.__textLeftButton = text

	@staticmethod
	## Set label for wand's DPAD right button.
	# @param str String to use for the label. Set str to "" to remove a label.
	def setDPADRightLabel(str):

		if (wandaid.__textDPADRight != None):
			wandaid.__wandObj.removeChildByRef(wandaid.__textDPADRight)

		if len(str) != 0:
			wandaid.__rightDPAD.setVisible(True)
		else:
			wandaid.__rightDPAD.setVisible(False)
			
		wandaid.__labelDPADRight = str
		text = Text3D.create('resources-wandaid/wandaid.ttf',1,wandaid.__labelDPADRight)
		text.setPosition(Vector3(0.03,0.012,0.03))
		text.setScale(Vector3(0.01,0.01,0.01))
		text.setFontResolution(120)
		text.setColor(Color('white'))
		#text.setColor(Color(1,0.7,0,1))
		wandaid.__wandObj.addChild(text)
		wandaid.__textDPADRight = text

	@staticmethod
	## Set label for wand's DPAD left button.
	# @param str String to use for the label. Set str to "" to remove a label.
	def setDPADLeftLabel(str):

		if (wandaid.__textDPADLeft != None):
			wandaid.__wandObj.removeChildByRef(wandaid.__textDPADLeft)

		if len(str) != 0:
			wandaid.__leftDPAD.setVisible(True)
		else:
			wandaid.__leftDPAD.setVisible(False)			
		wandaid.__labelDPADLeft = str
		text = Text3D.create('resources-wandaid/wandaid.ttf',1,wandaid.__labelDPADLeft)

		text.setScale(Vector3(0.01,0.01,0.01))
		min=text.getBoundMinimum()
		max=text.getBoundMaximum()
		size = max-min
		#text.setPosition(Vector3(-wandaid.__textSize(str)-0.03,0.012,0.03))
		text.setPosition(Vector3(-size.x-0.03,0.012,0.03))
		text.setFontResolution(120)
		text.setColor(Color('white'))
		#text.setColor(Color(1,0.7,0,1))
		wandaid.__wandObj.addChild(text)
		wandaid.__textDPADLeft = text

	@staticmethod
	## Set label for wand's DPAD up button.
	# @param str String to use for the label. Set str to "" to remove a label.
	def setDPADUpLabel(str):

		if (wandaid.__textDPADUp != None):
			wandaid.__wandObj.removeChildByRef(wandaid.__textDPADUp)

		if len(str) != 0:
			wandaid.__topDPAD.setVisible(True)
		else:
			wandaid.__topDPAD.setVisible(False)
			
		wandaid.__labelDPADUp = str
		text = Text3D.create('resources-wandaid/wandaid.ttf',1,wandaid.__labelDPADUp)
		text.setPosition(Vector3(0.015,0.029,0.02))
		text.setScale(Vector3(0.01,0.01,0.01))
		text.setFontResolution(120)
		text.setColor(Color('white'))
		#text.setColor(Color(1,0.7,0,1))
		wandaid.__wandObj.addChild(text)
		wandaid.__textDPADUp = text

	@staticmethod
	## Set label for wand's DPAD down button.
	# @param str String to use for the label. Set str to "" to remove a label.
	def setDPADDownLabel(str):
		if (wandaid.__textDPADDown != None):
			wandaid.__wandObj.removeChildByRef(wandaid.__textDPADDown)

		if len(str) != 0:
			wandaid.__bottomDPAD.setVisible(True)
		else:
			wandaid.__bottomDPAD.setVisible(False)
			
		wandaid.__labelDPADDown = str
		text = Text3D.create('resources-wandaid/wandaid.ttf',1,wandaid.__labelDPADDown)
		text.setScale(Vector3(0.01,0.01,0.01))
		min=text.getBoundMinimum()
		max=text.getBoundMaximum()
		size = max-min

		#text.setPosition(Vector3(-wandaid.__textSize(str)-0.013,-0.007,0.022))
		text.setPosition(Vector3(-size.x-0.013,-0.007,0.022))

		text.setFontResolution(120)
		text.setColor(Color('white'))
		#text.setColor(Color(1,0.7,0,1))
		wandaid.__wandObj.addChild(text)
		wandaid.__textDPADDown = text	
	

	@staticmethod
	## Set height at which wand has to be raised to activate wandaid.
	# @param height Height is specified in meters.
	def setActivationHeight(height):
		wandaid.__activationHeight = height

	@staticmethod
	## Call this in your Omegalib update function so that wandaid updates regularly.
	# @param dt dt from Omegalib update function.
	def update(dt):

		if wandaid.__isEnabled:
			wandPos = wandaid.__mainCamera.localToWorldPosition(wandaid.__dummyNode.getPosition())
			if((wandPos.y - wandaid.__mainCamera.getPosition().y) > wandaid.__activationHeight):
				if not(wandaid.__SIM):
					wandaid.__wandObj.setVisible(True)
					wandaid.__light.setEnabled(True)
					wandaid.__scale += dt*2
					if (wandaid.__scale >1):
						wandaid.__scale = 1
					wandaid.__wandObj.setScale(Vector3(wandaid.__scale,wandaid.__scale,wandaid.__scale))
			else:
				if not(wandaid.__SIM):
					wandaid.__scale -= dt*2
					if (wandaid.__scale <=0):
						wandaid.__scale = 0
					wandaid.__wandObj.setScale(Vector3(wandaid.__scale, wandaid.__scale, wandaid.__scale))
					if (wandaid.__scale == 0):
						wandaid.__wandObj.setVisible(False)
						wandaid.__light.setEnabled(False)
		else:
			wandaid.__wandObj.setVisible(False)
			wandaid.__light.setEnabled(False)
			
			
		