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
 *	The Equalizer channel implementation: this class is the entry point for
 *	every rendering operation. It sets up the draw context and calls the
 *  omegalib Renderer.draw method to perform the actual rendering.
 ******************************************************************************/
#include "eqinternal.h"
#include "omega/DisplaySystem.h"
#include "omega/SageManager.h"

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

using namespace omega;
using namespace co::base;
using namespace std;

using namespace eq;

// This static variable is used to signal local tiles visibility
// This external variable is declared in EqualizerDisplaySystem.cpp
extern bool sLocalTilesVisible;

///////////////////////////////////////////////////////////////////////////////
ChannelImpl::ChannelImpl( eq::Window* parent ) 
    :eq::Channel( parent ), myWindow(parent), myDrawBuffer(NULL), myStencilInitialized(false)
{
}

///////////////////////////////////////////////////////////////////////////////
ChannelImpl::~ChannelImpl() 
{}

///////////////////////////////////////////////////////////////////////////////
bool ChannelImpl::configInit(const eq::uint128_t& initID)
{
    eq::Channel::configInit(initID);

    EqualizerDisplaySystem* ds = (EqualizerDisplaySystem*)SystemManager::instance()->getDisplaySystem();
    String name = getName();

	if(name == "stats")
	{
		myDC.tile = &ds->getDisplayConfig().statsTile;
	}
	else
	{
		if(ds->getDisplayConfig().tiles.find(name) == ds->getDisplayConfig().tiles.end())
		{
			oferror("ChannelImpl::configInit: could not find tile %1%", %name);
		}
		else
		{
			myDC.tile = ds->getDisplayConfig().tiles[name];
		}
	}

    return true;
}

///////////////////////////////////////////////////////////////////////////////
void ChannelImpl::setupDrawContext(DrawContext* context, const co::base::uint128_t& spin, eq::fabric::Eye eye)
{
    WindowImpl* window = static_cast<WindowImpl*>(getWindow());
    Renderer* client = window->getRenderer();

	EqualizerDisplaySystem* ds = (EqualizerDisplaySystem*)SystemManager::instance()->getDisplaySystem();
	const DisplayConfig& dcfg = ds->getDisplayConfig();

	float nearz = ds->getNearZ();
	float farz = ds->getFarZ();
	if(nearz != 0 && farz != 0) setNearFar(nearz, farz);

    eq::PixelViewport pvp = getPixelViewport();

    context->gpuContext = client->getGpuContext();
	context->renderer = (Renderer*)client;

    // setup the context viewport.
    // (spin is 128 bits, gets truncated to 64... do we really need 128 bits anyways!?)
    context->frameNum = spin.low();

	switch( eye )
    {
        case eq::fabric::EYE_LEFT:
            context->eye = DrawContext::EyeLeft;
            break;
        case eq::fabric::EYE_RIGHT:
            context->eye = DrawContext::EyeRight;
            break;
        case eq::fabric::EYE_CYCLOP:
            context->eye = DrawContext::EyeCyclop;
            break;
    }

	// Setup side-by-side stereo if needed.
	if(myDC.tile->stereoMode == DisplayTileConfig::SideBySide ||
		(myDC.tile->stereoMode == DisplayTileConfig::Default && dcfg.stereoMode == DisplayTileConfig::SideBySide))
	{
		if(ds->getDisplayConfig().forceMono)
		{
			// Runtime stereo disable switch
			context->viewport = Rect(pvp.x, pvp.y, pvp.w, pvp.h);
		}
		else
		{
			// Do we want to invert stereo?
			bool invertStereo = ds->getDisplayConfig().invertStereo || myDC.tile->invertStereo; 

			if(context->eye == DrawContext::EyeLeft)
			{
				if(invertStereo)
				{
					context->viewport = Rect(pvp.x + pvp.w / 2, pvp.y, pvp.w / 2, pvp.h);
				}
				else
				{
					context->viewport = Rect(pvp.x, pvp.y, pvp.w / 2, pvp.h);
				}
			}
			else if(context->eye == DrawContext::EyeRight)
			{
				if(invertStereo)
				{
					context->viewport = Rect(pvp.x, pvp.y, pvp.w / 2, pvp.h);
				}
				else
				{
					context->viewport = Rect(pvp.x + pvp.w / 2, pvp.y, pvp.w / 2, pvp.h);
				}
			}
			else
			{
				context->viewport = Rect(pvp.x, pvp.y, pvp.w, pvp.h);
			}
		}
	}
	else
	{
		context->viewport = Rect(pvp.x, pvp.y, pvp.w, pvp.h);
	}
        
    AffineTransform3 mw;
	
	vmml::matrix<4, 4, float> proj = getPerspective().compute_matrix();
	const eq::fabric::Matrix4f& eqmw = getPerspectiveTransform();

	for(int i = 0; i < 16; i++)
	{
		mw.data()[i] = eqmw[i];
		context->projection.data()[i] = proj.array[i];
	}

	if(myDC.tile->camera != NULL)
	{
		//ofmsg("CUSTOM CAM %1%", %myDC.tile->camera->getName());
		Camera* cam = myDC.tile->camera; 
		context->modelview = mw * cam->getViewTransform();
	}
	else
	{
		//omsg("DEFAULT CAM");
		Camera* cam = client->getEngine()->getDefaultCamera();
		context->modelview = mw * cam->getViewTransform();
	}

	// Setup the stencil buffer if needed.
	// The stencil buffer is set up if th tile is using an interleaved mode (line or pixel)
	// or if the tile is left in default mode and the global stereo mode is an interleaved mode
	if(myDC.tile->stereoMode == DisplayTileConfig::LineInterleaved ||
		myDC.tile->stereoMode == DisplayTileConfig::PixelInterleaved ||
		(myDC.tile->stereoMode == DisplayTileConfig::Default && (
				dcfg.stereoMode == DisplayTileConfig::LineInterleaved ||
				dcfg.stereoMode == DisplayTileConfig::PixelInterleaved)))
	{
		if(!myStencilInitialized)
		{
			setupStencil(myDC.tile->pixelSize[0], myDC.tile->pixelSize[1]);
			myStencilInitialized = true;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void ChannelImpl::frameDraw( const co::base::uint128_t& frameID )
{
    eq::Channel::frameDraw( frameID );

	// If local tiles are hidden, we are done.
	if(!sLocalTilesVisible) return;

	EqualizerDisplaySystem* ds = dynamic_cast<EqualizerDisplaySystem*>(SystemManager::instance()->getDisplaySystem());
	if(getEye() == eq::fabric::EYE_LEFT || getEye() == eq::fabric::EYE_CYCLOP) 
	{
		// This is the first eye being drawn: clear the depth and color buffers.
		const Color& b = ds->getBackgroundColor();
		glClearColor(b[0], b[1], b[2], b[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	if(ds->getDisplayConfig().forceMono && getEye() != eq::fabric::EYE_LAST) return;

    //ofmsg("frameDraw: channel %1% frame %2%", %this %frameID);
    //PipeImpl* pipe = static_cast<PipeImpl*>(getPipe());
    //Renderer* client = pipe->getClient();

    setupDrawContext(&myDC, frameID, getEye());

	// Configure stencil test when rendering interleaved with stencil is enabled.
	if(myStencilInitialized)
	{
		if(ds->getDisplayConfig().forceMono)
		{
			// Disable stencil
			glStencilFunc(GL_ALWAYS,1,1); // to avoid interaction with stencil content
		}
		else
		{
			if(myDC.eye == DrawContext::EyeLeft)
			{
				glStencilFunc(GL_NOTEQUAL,1,1); // draws if stencil <> 1
			}
			else if(myDC.eye == DrawContext::EyeRight)
			{
				glStencilFunc(GL_EQUAL,1,1); // draws if stencil <> 0
 			}
		}
	}

	// Draw scene
	myDC.task = DrawContext::SceneDrawTask;
	myDC.renderer->draw(myDC);

	// Draw overlay when drawing stereo, otherwise we will do a single overlay drawing pass in
	// frameViewFinish
    if(getEye() != eq::fabric::EYE_CYCLOP)
    {
        myDC.task = DrawContext::OverlayDrawTask;
        myDC.renderer->draw(myDC);
    }
}

///////////////////////////////////////////////////////////////////////////////
void ChannelImpl::frameViewFinish( const co::base::uint128_t& frameID )
{
    eq::Channel::frameViewFinish( frameID );

	// If local tiles are hidden, we are done.
	if(!sLocalTilesVisible) return;

	// Overlay always renders in a single pass eve when stereo is enabled.
	// So, if we are in stereo mode skip one eye.
	if(getEye() != eq::fabric::EYE_LAST && getEye() != eq::fabric::EYE_CYCLOP) return;

	setupDrawContext(&myDC, frameID, eq::fabric::EYE_CYCLOP);
    myDC.task = DrawContext::OverlayDrawTask;

    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

    myDC.renderer->draw(myDC);

	EqualizerDisplaySystem* ds = (EqualizerDisplaySystem*)myDC.renderer->getDisplaySystem();

	if(myDC.tile->drawStats)
    {
		if(frameID.low() % 10 == 0)
		{
			if(myStatsBuffer == NULL)
			{
				myStatsTexture = myDC.renderer->createTexture();
				myStatsTexture->initialize(myDC.tile->pixelSize[0], myDC.tile->pixelSize[1]);
				myStatsBuffer = myDC.renderer->createRenderTarget(RenderTarget::RenderToTexture);
				myStatsBuffer->setTextureTarget(myStatsTexture);
			}
			myStatsBuffer->bind();
			glClearColor(0,0,0,0);
			myStatsBuffer->clear();
			drawStats();
			myStatsBuffer->unbind();
		}
		//else
		{
			if(myStatsTexture != NULL)
			{
				Renderer* r = myDC.renderer;
				DrawInterface* di = r->getRenderer();
				di->beginDraw2D(myDC);
				di->drawRectTexture(myStatsTexture, omicron::Vector2f::Zero(), omicron::Vector2f(myDC.tile->pixelSize[0], myDC.tile->pixelSize[1]));
				di->endDraw();
			}
		}
    }
	else if(ds->isDrawFpsEnabled())
    {
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        applyScreenFrustum();

        glMatrixMode( GL_MODELVIEW );
        glDisable( GL_LIGHTING );

        getWindow()->drawFPS();
    }

	// If SAGE support is enabled, notify frame finish
#ifdef OMEGA_USE_SAGE
	SageManager* sage = getRenderer()->getSystemManager()->getSageManager();
	if(sage != NULL)
	{
		sage->finishFrame(myDC);
	}
#endif

    EQ_GL_CALL( resetAssemblyState( ));
}

///////////////////////////////////////////////////////////////////////////////
omega::Renderer* ChannelImpl::getRenderer()
{
    WindowImpl* window = static_cast<WindowImpl*>(getWindow());
    return window->getRenderer();
}

///////////////////////////////////////////////////////////////////////////////
void ChannelImpl::setupStencil(int gliWindowWidth, int gliWindowHeight)
{
	GLint gliStencilBits;
	glGetIntegerv(GL_STENCIL_BITS,&gliStencilBits);

	EqualizerDisplaySystem* ds = dynamic_cast<EqualizerDisplaySystem*>(SystemManager::instance()->getDisplaySystem());
	DisplayTileConfig::StereoMode stereoMode = myDC.tile->stereoMode;
	if(stereoMode == DisplayTileConfig::Default) stereoMode = ds->getDisplayConfig().stereoMode;

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
		bool invertStereo = ds->getDisplayConfig().invertStereo || myDC.tile->invertStereo; 
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
