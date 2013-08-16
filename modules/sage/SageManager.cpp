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
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer. Redistributions in binary 
 * form must reproduce the above copyright notice, this list of conditions and 
 * the following disclaimer in the documentation and/or other materials provided 
 * with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * What's in this file
 *	The interface between the Scalable Adaptive Graphics Environment (SAGE)
 *	and omegalib.
 ******************************************************************************/
#include "SageManager.h"
#include "libsage.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
SageManager::SageManager(): EngineModule("SageManager"),
	myWindowedConfig(NULL),
	mySail(NULL),
	myWindowed(false)
{
	myWindowedConfig = new DisplayTileConfig();

}

///////////////////////////////////////////////////////////////////////////////
SageManager::~SageManager()
{
	// Make sure al resources have been deallocated.
	dispose();
}


///////////////////////////////////////////////////////////////////////////////
void SageManager::initialize()
{
	SystemManager* sys = SystemManager::instance();
	Config* cfg = sys->getSystemConfig();
	DisplayConfig& dcfg = sys->getDisplaySystem()->getDisplayConfig();
	if(cfg->exists("config/sage"))
	{
		Setting& ss = cfg->lookup("config/sage");
		bool enabled = Config::getBoolValue("enabled", ss, false);
		if(enabled)
		{
			myWindowed = Config::getBoolValue("windowed", ss, false);
			myFsManagerAddress = Config::getStringValue("fsManagerAddress", ss, "localhost");
			ofmsg("\tfsManagerAddress = %1%", %myFsManagerAddress);
			myWindowedRenderNode = Config::getStringValue("windowedRenderMode", ss, "localhost");

			if(ss.exists("windowedConfig"))
			{
				myWindowedConfig = new DisplayTileConfig();
				myWindowedConfig->parseConfig(ss["windowedConfig"], dcfg);
			}
			else
			{
				owarn("SageManager::initialize: missing windowedConfig section");
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void SageManager::dispose()
{
	if(mySail != NULL)
	{
		omsg("SageManager::dispose: deleting SAIL buffer");
		deleteSAIL(mySail);
		mySail = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
void SageManager::switchToWindowedMode()
{
	sagePixFmt pixFmt;
	if(myWindowedConfig->stereoMode == DisplayTileConfig::Mono || 
		myWindowedConfig->stereoMode == DisplayTileConfig::Default)
	{
		pixFmt = PIXFMT_888;
	}
	else if(myWindowedConfig->stereoMode == DisplayTileConfig::PixelInterleaved)
	{
		pixFmt = PIXFMT_888;
	}
	else
	{
		owarn("SageManager::initialize: unsupported tile stereo format, defaulting to mono");
		pixFmt = PIXFMT_888;
	}

	// Connect to free space manager and create a sail frame buffer
	mySail = createSAIL(
		SystemManager::instance()->getApplication()->getName(),
		myWindowedConfig->pixelSize[0], myWindowedConfig->pixelSize[1],
		pixFmt,
		myFsManagerAddress.c_str());
}

///////////////////////////////////////////////////////////////////////////////
void SageManager::switchToFullscreenMode()
{
}

///////////////////////////////////////////////////////////////////////////////
void SageManager::update(const UpdateContext& context)
{
	if(myCamera == NULL)
	{
		// Register myself as a camera listener.
		myCamera = Engine::instance()->getDefaultCamera();
	}
	if(myImage == NULL && myWindowedConfig != NULL)
	{
		myImage = new PixelData(
			PixelData::FormatRgb, 
			myWindowedConfig->pixelSize[0],
			myWindowedConfig->pixelSize[1]);
		// Setup camera output to the newly created image buffer: the camera 
		// will automatically output pixels to this buffer at the end of each 
		// frame.
		myCamera->getOutput(0)->setReadbackTarget(myImage);
		myCamera->getOutput(0)->setEnabled(true);
	}
	else
	{
		byte* source = myImage->map();

		// Grab data from the frame buffer and copy it to the SAGE frame buffer
		GLubyte *rgbBuffer = nextBuffer(mySail);
		memcpy(rgbBuffer, source, myImage->getSize());
		swapBuffer(mySail);

		myImage->unmap();
	}
}


