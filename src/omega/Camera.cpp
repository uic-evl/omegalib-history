/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory,  
 * University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer. Redistributions in binary 
 * form must reproduce the above copyright notice, this list of conditions and 
 * the following disclaimer in the documentation and/or other materials 
 * provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE  GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * What's in this file
 *	The Camera class: handles information about a view transformation, head 
 *	tracking and optional target buffers for off screen rendering
 *	A camera can have a controller that is used to implement a navigation 
 *	technique.
 ******************************************************************************/
#include "omega/RenderTarget.h"
#include "omega/Camera.h"
#include "omega/CameraOutput.h"
#include "omega/DisplaySystem.h"
#include "omega/ModuleServices.h"
#include "omega/WandCameraController.h"
#include "omega/GamepadCameraController.h"
#include "omega/MouseCameraController.h"
#include "omega/KeyboardMouseCameraController.h"

using namespace omega;


///////////////////////////////////////////////////////////////////////////////
Camera::Camera(Engine* e, uint flags):
	SceneNode(e),
	myAutoAspect(false),
	myFlags(flags),
	myController(NULL),
	myControllerEnabled(false),
	myTrackingEnabled(false),
	myTrackerSourceId(-1),
	myHeadOrientation(Quaternion::Identity()),
	myHeadOffset(Vector3f::Zero()),
	myMask(0),
	myEyeSeparation(0.06f),
	myListener(NULL),
	myNearZ(0.1f),
	myFarZ(1000.0f),
	myViewPosition(0, 0),
	myViewSize(1, 1)
{
	myCustomTileConfig = new DisplayTileConfig();
	//myProjectionOffset = -Vector3f::UnitZ();

	// set camera Id and increment the counter
	this->myCameraId = omega::CamerasCounter++;
}

///////////////////////////////////////////////////////////////////////////////
void Camera::setup(Setting& s)
{
	//set position of camera
    Vector3f camPos = Config::getVector3fValue("position", s, getPosition()); 
    setPosition(camPos);

	//set orientation of camera
	// NOTE: we want to either read orientation from the config or keep the default one.
	// Since orientation is expressed in yaw, pitch roll in the config file but there is no
	// way to get that from the camera (rotation is only as a quaternion) we cannot use the default
	// value in the Config::getVector3fValue.
	if(s.exists("orientation"))
	{
		Vector3f camOri = Config::getVector3fValue("orientation", s); 
		setPitchYawRoll(camOri * Math::DegToRad);
	}
	
	myTrackerSourceId = Config::getIntValue("trackerSourceId", s, -1);
	if(myTrackerSourceId != -1) myTrackingEnabled = true;

	//setup camera controller.  The camera needs to be setup before this otherwise its values will be rewritten

	String controllerName;
	controllerName = Config::getStringValue("controller", s);
	StringUtils::toLowerCase(controllerName);

	if(controllerName != "")
	{
		CameraController* controller = NULL;
		ofmsg("Camera controller: %1%", %controllerName);
		if(controllerName == "keyboardmouse") controller = new KeyboardMouseCameraController();
		if(controllerName == "mouse") controller = new MouseCameraController();
		if(controllerName == "wand") controller = new WandCameraController();
		if(controllerName == "gamepad") controller = new GamepadCameraController();

		setController(controller);
		if(myController != NULL) 
		{
			myController->setup(s);
			setControllerEnabled(true);
		}
	}

	Vector3f position = Vector3f::Zero();
	if(s.exists("headOffset"))
	{
		Setting& st = s["headOffset"];
		myHeadOffset.x() = (float)st[0];
		myHeadOffset.y() = (float)st[1];
		myHeadOffset.z() = (float)st[2];
	}
}

///////////////////////////////////////////////////////////////////////////////
void Camera::handleEvent(const Event& evt)
{
	if(myTrackingEnabled)
	{
		if(evt.getServiceType() == Event::ServiceTypeMocap && evt.getSourceId() == myTrackerSourceId)
		{
			myHeadOffset = evt.getPosition();
			myHeadOrientation = evt.getOrientation();
			
			Vector3f dir = myHeadOrientation * -Vector3f::UnitZ();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void Camera::updateTraversal(const UpdateContext& context)
{
	// Update the view transform
	myHeadTransform = AffineTransform3::Identity();
	myHeadTransform.translate(myHeadOffset);
	myHeadTransform.rotate(myHeadOrientation);

	// BUG: if we attach a child node to the camera, isUpdateNeeded gets reset at the wrong
	// time and the camera view transform does not get updated.
	// Needs fixing, but for now best solution is to disable the check and always update
	// the view transform
	//if(isUpdateNeeded())
	{
		// Update view transform.
		myViewTransform = Math::makeViewMatrix(
			getDerivedPosition(), // + myHeadOffset, 
			getDerivedOrientation());
	}
	
	SceneNode::updateTraversal(context);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::lookAt(const Vector3f& position, const Vector3f& upVector)
{
	Node::lookAt(myHeadOffset - position, upVector);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::focusOn(SceneNode* node)
{
	// Compute direction vector
	Vector3f dir = mPosition - node->getPosition();
	dir.normalize();

	const Sphere& bs = node->getBoundingSphere();
	ofmsg("Camera:focuson %1% %2%", %bs.getCenter() %bs.getRadius());
	mPosition = bs.getCenter() + Vector3f(0, 0, bs.getRadius() * 2) - myHeadOffset;
	lookAt(node->getPosition(), Vector3f::UnitY());
	//mOrientation = Math::buildRotation(Vector3f::UnitZ(), dir, Vector3f::UnitY());
	//mPosition = bs.getCenter() + Vector3f(0, 0, bs.getRadius() * 2);
    //needUpdate();
}

///////////////////////////////////////////////////////////////////////////////
CameraOutput* Camera::getOutput(uint contextId)
{
	oassert(contextId < GpuContext::MaxContexts);
	// Camera outputs are created on-demand here.
	if(myOutput[contextId] == NULL)
	{
		ofmsg("Camera::getOutput: creating camera output for context %1%", %contextId);
		myOutput[contextId] = new CameraOutput();
	}

	return myOutput[contextId].get();
}

///////////////////////////////////////////////////////////////////////////////
bool Camera::isEnabled(const DrawContext& context)
{
	//CameraOutput* output = getOutput(context.gpuContext->getId());
	//if(!output->isEnabled()) return false;

	const DisplayTileConfig* tile = context.tile;
	const DisplayConfig& dcfg = getEngine()->getDisplaySystem()->getDisplayConfig();
	const Vector2i& canvasSize = dcfg.canvasPixelSize;

	// Convert the normalized view coordinates into pixel coordinates
	Vector2i vmin(
		myViewPosition[0] * canvasSize[0],
		myViewPosition[1] * canvasSize[1]);
	Vector2i vmax(
		myViewSize[0] * canvasSize[0],
		myViewSize[1] * canvasSize[1]);
	vmax += vmin;

	// If the view minimum or maximum position are contained by the tile, the camera is enabled.
	if(vmin[0] >= tile->offset[0] &&
		vmin[1] >= tile->offset[1] &&
		vmin[0] <= tile->offset[0] + tile->pixelSize[0] &&
		vmin[1] <= tile->offset[1] + tile->pixelSize[1]) return true;
	if(vmax[0] >= tile->offset[0] &&
		vmax[1] >= tile->offset[1] &&
		vmax[0] <= tile->offset[0] + tile->pixelSize[0] &&
		vmax[1] <= tile->offset[1] + tile->pixelSize[1]) return true;

	return false;
}

///////////////////////////////////////////////////////////////////////////////
void Camera::beginDraw(DrawContext& context)
{
	CameraOutput* output = myOutput[context.gpuContext->getId()];
	if(output != NULL && output->isEnabled())
	{
		output->beginDraw(context);
	}

	if(myCustomTileConfig->enabled)
	{
		context.pushTileConfig(myCustomTileConfig);
		updateViewBounds(context, myCustomTileConfig->pixelSize);
	}
	else
	{
		const DisplayConfig& dcfg = getEngine()->getDisplaySystem()->getDisplayConfig();
		updateViewBounds(context, dcfg.canvasPixelSize);
	}
	updateOffAxisProjection(context);
	context.modelview = myViewTransform;

	if(myListener != NULL) myListener->beginDraw(this, context);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::endDraw(DrawContext& context)
{
	CameraOutput* output = myOutput[context.gpuContext->getId()];
	if(output != NULL && output->isEnabled())
	{
		output->endDraw(context);
	}
	if(myCustomTileConfig->enabled)
	{
		context.popTileConfig();
	}
	if(myListener != NULL) myListener->endDraw(this, context);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::startFrame(const FrameInfo& frame)
{
	CameraOutput* output = myOutput[frame.gpuContext->getId()];
	if(output != NULL && output->isEnabled())
	{
		output->startFrame(frame);
	}

	if(myListener != NULL) myListener->startFrame(this, frame);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::finishFrame(const FrameInfo& frame)
{
	CameraOutput* output = myOutput[frame.gpuContext->getId()];
	if(output != NULL && output->isEnabled())
	{
		output->finishFrame(frame);
	}
	if(myListener != NULL) myListener->finishFrame(this, frame);
}

///////////////////////////////////////////////////////////////////////////////
Vector3f Camera::localToWorldPosition(const Vector3f& position)
{
	Vector3f res = mPosition + mOrientation * position;
    return res;
}

///////////////////////////////////////////////////////////////////////////////
Quaternion Camera::localToWorldOrientation(const Quaternion& orientation)
{
	return mOrientation * orientation;
}

///////////////////////////////////////////////////////////////////////////////
Vector3f Camera::worldToLocalPosition(const Vector3f& position)
{
	Vector3f res = mOrientation.inverse() * (position - mPosition);
	return res;
}

///////////////////////////////////////////////////////////////////////////////
void Camera::setController(CameraController* value) 
{ 
	if(myController != NULL)
	{
		ModuleServices::removeModule(myController);
	}

	myController = value; 
	if(myController != NULL)
	{
		myController->setCamera(this);
		ModuleServices::addModule(myController);
	}
}

///////////////////////////////////////////////////////////////////////////////
void Camera::updateOffAxisProjection(DrawContext& context)
{
	DisplaySystem* ds = context.renderer->getDisplaySystem();
	DisplayConfig& dcfg = ds->getDisplayConfig();
	
	Vector3f pa = context.tile->bottomLeft;
	Vector3f pb = context.tile->bottomRight;
	Vector3f pc = context.tile->topLeft;
	
	// half eye separation
	float hes = myEyeSeparation / 2;
	Vector3f pe = Vector3f::Zero();
	switch(context.eye)
	{
	case DrawContext::EyeLeft:
		pe[0] = -hes;
		break;
	case DrawContext::EyeRight:
		pe[0] = hes;
		break;
	}

	// Transform eye with head position / orientation. After this, eye position
	// and tile coordinates are all in the same reference frame.
	if(dcfg.panopticStereoEnabled)
	{
		// CAVE2 SIMPLIFICATION: We are just interested in adjusting the observer yaw
		//om.rotate_y(-otd.yaw * Math::DegToRad);
		pe = myHeadTransform.rotate(
			AngleAxis(-context.tile->yaw * Math::DegToRad, Vector3f::UnitY())) * pe;
	}
	else
	{
		pe = myHeadTransform * pe;
	}

	Vector3f vr = pb - pa;
	Vector3f vu = pc - pa;
	Vector3f vn = vr.cross(vu);

	Vector2f viewSize = context.viewMax - context.viewMin;

	// Update tile corners based on local view position and size
	pa = pa + vr * context.viewMin[0] + vu * context.viewMin[1];
	pb = pa + vr * viewSize[0];
	pc = pa + vu * viewSize[1];

	vr.normalize();
	vu.normalize();
	vn.normalize();

	// Compute the screen corner vectors.
	Vector3f va = pa - pe;
	Vector3f vb = pb - pe;
	Vector3f vc = pc - pe;

	// Find distance from eye to screen plane.
	//Vector3f tm = pe - pa;
	float d = -(vn.dot(va));

	// Find the extent of the perpendicular projection.
	float l = vr.dot(va) * myNearZ / d;
	float r = vr.dot(vb) * myNearZ / d;
	float b = vu.dot(va) * myNearZ / d;
	float t = vu.dot(vc) * myNearZ / d;

	Transform3 oax;
	oax.setIdentity();
	oax(0,0) = 2 * myNearZ / (r - l);
	oax(0,2) = (r + l) / (r - l);
	oax(1,1) = 2 * myNearZ / (t - b);
	oax(1,2) = (t + b) / (t - b);
	oax(2,2) = - (myFarZ + myNearZ) / (myFarZ - myNearZ);
	oax(2,3) = - (2 * myFarZ * myNearZ) / (myFarZ - myNearZ);
	oax(3,2) = - 1;
	oax(3,3) = 0;

	Transform3 newBasis;
	newBasis.setIdentity();
	newBasis.data()[0] = vr[0];
	newBasis.data()[1] = vu[0];
	newBasis.data()[2] = vn[0];

	newBasis.data()[4] = vr[1];
	newBasis.data()[5] = vu[1];
	newBasis.data()[6] = vn[1];

	newBasis.data()[8] = vr[2];
	newBasis.data()[9] = vu[2];
	newBasis.data()[10] = vn[2];

	oax = oax * newBasis;
	
	// Translate to apex of the frustum to the origin
	context.projection = oax.translate(-pe);
}

///////////////////////////////////////////////////////////////////////////////
void Camera::updateViewBounds(DrawContext& context, const Vector2i& canvasSize)
{
	const DisplayTileConfig* tile = context.tile;

	float aw = (float)canvasSize[0] / tile->pixelSize[0];
	float ah = (float)canvasSize[1] / tile->pixelSize[1];
	Vector2f a(aw, ah);

	// Convert the tile pixel offset in normalized coordinates
	Vector2f offset(
		(float)tile->offset[0] / canvasSize[0],
		(float)tile->offset[1] / canvasSize[1]);

	context.viewMin = (myViewPosition - offset).cwiseProduct(a);
	context.viewMax = (myViewPosition + myViewSize - offset).cwiseProduct(a);
	
	context.viewMin = context.viewMin.cwiseMax(Vector2f::Zero());
	context.viewMax = context.viewMax.cwiseMin(Vector2f::Ones());

	// Adjust the local pixel viewport.
	context.viewport.min[0] = context.viewMin[0] * tile->pixelSize[0];
	context.viewport.min[1] = context.viewMin[1] * tile->pixelSize[1];
	context.viewport.max[0] = context.viewMax[0] * tile->pixelSize[0];
	context.viewport.max[1] = context.viewMax[1] * tile->pixelSize[1];
}
