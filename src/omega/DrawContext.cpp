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
 *	Contains utility functions used to draw and manage graphic resources
 ******************************************************************************/
#include "omega/DrawContext.h"
#include "omega/Renderer.h"
#include "omega/DisplaySystem.h"
#include "omega/glheaders.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
DrawContext::DrawContext():
	stencilInitialized(false)
{
}

///////////////////////////////////////////////////////////////////////////////
void DrawContext::updateViewport()
{
	DisplaySystem* ds = renderer->getDisplaySystem();
	DisplayConfig& dcfg = ds->getDisplayConfig();

	int pvpx = 0;
	int pvpy = 0;
	int pvpw = tile->pixelSize[0];
	int pvph = tile->pixelSize[1];

	// Setup side-by-side stereo if needed.
	if(tile->stereoMode == DisplayTileConfig::SideBySide ||
		(tile->stereoMode == DisplayTileConfig::Default && 
		dcfg.stereoMode == DisplayTileConfig::SideBySide))
	{
		if(dcfg.forceMono)
		{
			// Runtime stereo disable switch
			viewport = Rect(pvpx, pvpy, pvpw, pvph);
		}
		else
		{
			// Do we want to invert stereo?
			bool invertStereo = ds->getDisplayConfig().invertStereo || tile->invertStereo; 

			if(eye == DrawContext::EyeLeft)
			{
				if(invertStereo)
				{
					viewport = Rect(pvpx + pvpw / 2, pvpy, pvpw / 2, pvph);
				}
				else
				{
					viewport = Rect(pvpx, pvpy, pvpw / 2, pvph);
				}
			}
			else if(eye == DrawContext::EyeRight)
			{
				if(invertStereo)
				{
					viewport = Rect(pvpx, pvpy, pvpw / 2, pvph);
				}
				else
				{
					viewport = Rect(pvpx + pvpw / 2, pvpy, pvpw / 2, pvph);
				}
			}
			else
			{
				viewport = Rect(pvpx, pvpy, pvpw, pvph);
			}
		}
	}
	else
	{
		viewport = Rect(pvpx, pvpy, pvpw, pvph);
	}
}

///////////////////////////////////////////////////////////////////////////////
void DrawContext::setupInterleaver()
{
	DisplaySystem* ds = renderer->getDisplaySystem();
	DisplayConfig& dcfg = ds->getDisplayConfig();

	// Setup the stencil buffer if needed.
	// The stencil buffer is set up if th tile is using an interleaved mode (line or pixel)
	// or if the tile is left in default mode and the global stereo mode is an interleaved mode
	if(tile->stereoMode == DisplayTileConfig::LineInterleaved ||
		tile->stereoMode == DisplayTileConfig::PixelInterleaved ||
		(tile->stereoMode == DisplayTileConfig::Default && (
				dcfg.stereoMode == DisplayTileConfig::LineInterleaved ||
				dcfg.stereoMode == DisplayTileConfig::PixelInterleaved)))
	{
		if(!stencilInitialized)
		{
			initializeStencilInterleaver(tile->pixelSize[0], tile->pixelSize[1]);
			stencilInitialized = true;
		}
	}
	// Configure stencil test when rendering interleaved with stencil is enabled.
	if(stencilInitialized)
	{
		if(dcfg.forceMono)
		{
			// Disable stencil
			glStencilFunc(GL_ALWAYS,1,1); // to avoid interaction with stencil content
		}
		else
		{
			if(eye == DrawContext::EyeLeft)
			{
				glStencilFunc(GL_NOTEQUAL,1,1); // draws if stencil <> 1
			}
			else if(eye == DrawContext::EyeRight)
			{
				glStencilFunc(GL_EQUAL,1,1); // draws if stencil <> 0
 			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void DrawContext::initializeStencilInterleaver(int gliWindowWidth, int gliWindowHeight)
{
	DisplaySystem* ds = renderer->getDisplaySystem();
	DisplayConfig& dcfg = ds->getDisplayConfig();

	GLint gliStencilBits;
	glGetIntegerv(GL_STENCIL_BITS,&gliStencilBits);

	//EqualizerDisplaySystem* ds = dynamic_cast<EqualizerDisplaySystem*>(SystemManager::instance()->getDisplaySystem());
	DisplayTileConfig::StereoMode stereoMode = tile->stereoMode;
	if(stereoMode == DisplayTileConfig::Default) stereoMode = dcfg.stereoMode;

	// seting screen-corresponding geometry
	glViewport(0,0,gliWindowWidth,gliWindowHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.5,gliWindowWidth + 0.5,0.5,gliWindowHeight + 0.5);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
		
		
	// clearing and configuring stencil drawing
	glDrawBuffer(GL_BACK);
	glEnable(GL_STENCIL_TEST);
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glStencilOp (GL_REPLACE, GL_REPLACE, GL_REPLACE); // colorbuffer is copied to stencil
	glDisable(GL_DEPTH_TEST);
	glStencilFunc(GL_ALWAYS,1,1); // to avoid interaction with stencil content
	
	// drawing stencil pattern
	glColor4f(1,1,1,0);	// alpha is 0 not to interfere with alpha tests
	
	if(stereoMode == DisplayTileConfig::LineInterleaved)
	{
		// Do we want to invert stereo?
		bool invertStereo = ds->getDisplayConfig().invertStereo || tile->invertStereo; 
		int startOffset = invertStereo ? -1 : -2;

		for(float gliY = startOffset; gliY <= gliWindowHeight; gliY += 2)
		{
			glLineWidth(1);
			glBegin(GL_LINES);
				glVertex2f(0, gliY);
				glVertex2f(gliWindowWidth, gliY);
			glEnd();	
		}
	}
	else if(stereoMode == DisplayTileConfig::PixelInterleaved)
	{
		for(float gliX=-2; gliX<=gliWindowWidth; gliX+=2)
		{
			glLineWidth(1);
			glBegin(GL_LINES);
				glVertex2f(gliX, 0);
				glVertex2f(gliX, gliWindowHeight);
			glEnd();	
		}
	}
	glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP); // disabling changes in stencil buffer
	glFlush();
}
