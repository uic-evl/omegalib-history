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

	WindowImpl* window = static_cast<WindowImpl*>(getWindow());
    Renderer* client = window->getRenderer();
    myDC.gpuContext = client->getGpuContext();
	myDC.renderer = client;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
void ChannelImpl::frameDraw( const co::base::uint128_t& frameID )
{
    eq::Channel::frameDraw( frameID );

	// If local tiles are hidden, we are done.
	if(!sLocalTilesVisible) return;

	// (spin is 128 bits, gets truncated to 64... 
	// do we really need 128 bits anyways!?)
	myDC.drawFrame(frameID.low());
}

///////////////////////////////////////////////////////////////////////////////
void ChannelImpl::frameViewFinish( const co::base::uint128_t& frameID )
{
    eq::Channel::frameViewFinish( frameID );

	// If local tiles are hidden, we are done.
	if(!sLocalTilesVisible) return;

    EQ_GL_CALL( applyBuffer( ));
    EQ_GL_CALL( applyViewport( ));
    EQ_GL_CALL( setupAssemblyState( ));

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

