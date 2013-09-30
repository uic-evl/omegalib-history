#include <omega.h>
#include <omegaGl.h>

using namespace omega;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Contains data for a single animation frame.
struct Frame: public ReferenceType
{
	Ref<ImageUtils::LoadImageAsyncTask> imageLoader;
	int frameNumber;
	//float deadline;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! omegalib module implementing a flipbook-style animation player.
class FlipbookPlayer: public EngineModule
{
public:
	//! Convenience method to create the module, register and initialize it.
	static FlipbookPlayer* createAndInitialize();

	FlipbookPlayer();

	//! Called for each frame.
	virtual void update(const UpdateContext& context);
	//! Called when a new renderer is being initialized (i.e. for a new gl window)
	virtual void initializeRenderer(Renderer* r);
	virtual void dispose();

	//! Start loading an animation.
	void load(const String& framePath, int totalFrames, int startFrame);
	void loadMultidir(const String& framePath, int totalFrames, int startFrame, int filesPerDir, int startDirIndex);

	//! Begin playing an animation.
	void play();
	//! Pause animation
	void pause();
	// Stop and rewind animation
	void stop();
	void loop(bool value) { myLoop = value; }
	void setCurrentFrameNumber(int frameNumber);

	//! Adds the next frame to the buffering queue.
	void queueNextFrame();
	//! Returns the number of consecutive frames that finished buffering and are ready to display.
	int getNumReadyFrames();
	int getNumBufferedFrames() { return myBufferedFrames.size(); }
	int getTotalFrames() { return myTotalFrames; }

	// Return the current frame.
	Frame* getCurrentFrame() { return myCurrentFrame; }

	//! Sets / gets the number of frames to keep in the buffer
	int getFramesToBuffer() { return myFramesToBuffer; }
	void setFramesToBuffer(int value) { myFramesToBuffer = value; }

	void setFrameTime(float value) { myFrameTime = value; }
	float getFrameTime() { return myFrameTime; }

	void setPlaybackBarVisible(bool value) { myPlaybackBarVisible = value; }
	bool isPlaybackBarVisible() { return myPlaybackBarVisible; }
	
	void setNumGpuBuffers(int num) { myNumGpuBuffers = num; }
	int getNumGpuBuffers() { return myNumGpuBuffers; }

private:
	//! Frame file name. Should contain a printf-style placeholder 
	//! that will be replaced with the frame number.
	String myFramePath;

	//! Total frames for this animation
	int myTotalFrames;
	//! Start frame index for this animation
	int myStartFrame;
	//! When true use multiple subdirectories for movie segments.
	bool myIsMultidir;
	//! Number of files per directory
	int myFramesPerDir;
	int myStartDirIndex;

	//! Number of frames that we should keep in the loading queue.
	int myFramesToBuffer;

	//! Number of frames that should be fully loaded at any time during playback.
	//! If loaded frames go under this threshold, play goes into 'buffering mode' and will
	//! resume when enough frames have loaded.
	int myFrameBufferThreshold;

	//! Time each frame should be displayed, in seconds.
	float myFrameTime;

	//! Total playback time.
	float myPlaybackTime;
	
	//! When set to true playback will start again once over.
	bool myLoop;
	//! True when the animation is playing (even when buffering)
	bool myPlaying;
	//! True when the animation play is suspended because not enough frames are in the queue.
	bool myBuffering;
	//! True when we are loading new frames
	bool myLoading;
	
	//! Current frame.
	Ref<Frame> myCurrentFrame;
	float myCurrentFrameTime;
	int myCurrentFrameIndex;

	//! Buffer of frames that are loading or ar ready to view.
	List< Ref<Frame> > myBufferedFrames;

	bool myPlaybackBarVisible;
	
	int myNumGpuBuffers;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Render pass for the flipbook player
class FlipbookRenderPass: public RenderPass
{
public:
	FlipbookRenderPass(FlipbookPlayer* player, Renderer* client): 
		myPlayer(player), RenderPass(client, "FlipbookRenderPass"), myActiveTextureIndex(0) {}
	virtual ~FlipbookRenderPass() { omsg("~FlipbookRenderPass"); }
	//! Drawing happens here.
	virtual void render(Renderer* client, const DrawContext& context);

private:
	//! Draw a youtube-style playback bar. Useful for debugging buffering performance.
	void drawPlaybackBar(DrawInterface* di, int x, int y, int width, int height);

private:
	Ref<PixelData> myPixelBuffer[16];
	Ref<Texture> myFrameTexture;
	int myActiveTextureIndex;
	FlipbookPlayer* myPlayer;
	Ref<Frame> myLastFrame;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Python wrapper code.
#ifdef OMEGA_USE_PYTHON
#include "omega/PythonInterpreterWrapper.h"
BOOST_PYTHON_MODULE(flipbookPlayer)
{
	// FlipbookPlayer
	PYAPI_REF_BASE_CLASS(FlipbookPlayer)
		PYAPI_STATIC_REF_GETTER(FlipbookPlayer, createAndInitialize)
		PYAPI_METHOD(FlipbookPlayer, load)
		PYAPI_METHOD(FlipbookPlayer, loadMultidir)
		PYAPI_METHOD(FlipbookPlayer, play)
		PYAPI_METHOD(FlipbookPlayer, pause)
		PYAPI_METHOD(FlipbookPlayer, stop)
		PYAPI_METHOD(FlipbookPlayer, loop)
		PYAPI_METHOD(FlipbookPlayer, getFramesToBuffer)
		PYAPI_METHOD(FlipbookPlayer, setFramesToBuffer)
		PYAPI_METHOD(FlipbookPlayer, getFrameTime)
		PYAPI_METHOD(FlipbookPlayer, setFrameTime)
		PYAPI_METHOD(FlipbookPlayer, setCurrentFrameNumber)
		PYAPI_METHOD(FlipbookPlayer, setPlaybackBarVisible)
		PYAPI_METHOD(FlipbookPlayer, isPlaybackBarVisible)
		PYAPI_METHOD(FlipbookPlayer, getNumGpuBuffers)
		PYAPI_METHOD(FlipbookPlayer, setNumGpuBuffers)
		;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
FlipbookPlayer* FlipbookPlayer::createAndInitialize()
{
	// Initialize and register the flipbook player module.
	FlipbookPlayer* instance = new FlipbookPlayer();
	ModuleServices::addModule(instance);
	instance->doInitialize(Engine::instance());
	return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
FlipbookPlayer::FlipbookPlayer():
	EngineModule("FlipbookPlayer"),
	myPlaying(false),
	myPlaybackBarVisible(true),
	myLoop(false)
{
	myFramesToBuffer = 100;
	myFrameTime = 0.2f;
	myNumGpuBuffers = 2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FlipbookPlayer::load(const String& filePath, int totalFrames, int startFrame)
{
	myFramePath = filePath;
	myTotalFrames = totalFrames;
	myStartFrame = startFrame;
	myLoading = true;
	myIsMultidir = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FlipbookPlayer::loadMultidir(const String& filePath, int totalFrames, int startFrame, int framesPerDir, int startDirIndex)
{
	myFramePath = filePath;
	myTotalFrames = totalFrames;
	myStartFrame = startFrame;
	myLoading = true;
	myIsMultidir = true;
	myFramesPerDir = framesPerDir;
	myStartDirIndex = startDirIndex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FlipbookPlayer::play()
{
	myPlaying = true;
	myBuffering = true;
	myCurrentFrame = NULL;
	myPlaybackTime = 0;

	// The initial buffering threshold is half the maximum queued frames.
	myFrameBufferThreshold = myFramesToBuffer / 2;
	myCurrentFrameTime = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FlipbookPlayer::pause()
{
	myPlaying = false;
	myCurrentFrame = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FlipbookPlayer::stop()
{
	myPlaying = false;
	myLoading = true;
	myBufferedFrames.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FlipbookPlayer::setCurrentFrameNumber(int frameNumber)
{
	// not implemented.... would be fairly complex given way playback is currently implemented.
	// Need to rethink playback frame timing & buffering first.
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FlipbookPlayer::update(const UpdateContext& context)
{
	if(myLoading)
	{
		// Do we need to queue more frames?
		while(myBufferedFrames.size() < myFramesToBuffer && myLoading) queueNextFrame();
	}
	if(myBuffering)
	{
		if(!myLoading || getNumReadyFrames() >= myFrameBufferThreshold)
		{
			// If we are out here it means that we buffered enough frames, or there are no 
			// more frames we can possibly buffer.
			myBuffering = false;
			// Reduce frame buffer threshold to 75% so we don't keep switching back and forth 
			// in buffering
			myFrameBufferThreshold *= 0.75;
		}
		else
		{
			// If we are in buffering mode, wait until we have enough frames buffered
			while(myLoading && getNumReadyFrames() < myFrameBufferThreshold)
			{
				osleep(100);
				// Do we need to queue more frames?
				while(myBufferedFrames.size() < myFramesToBuffer && myLoading) queueNextFrame();
			}
		}
	}
	else if(myPlaying)
	{
		// If the current frame is still valid, keep displaying it...
		if(myCurrentFrame.isNull() || myCurrentFrameTime >= myFrameTime)
		{
			if(myBufferedFrames.size() == 0)
			{
					myCurrentFrame = NULL;
			}
			else
			{
				myCurrentFrame = myBufferedFrames.front();
				myBufferedFrames.pop_front();
				myCurrentFrameTime = 0;
				myCurrentFrameIndex = myCurrentFrame->frameNumber;
			}
		}

		//myPlaybackTime += context.dt;
		//ofmsg("dt %1%", %context.dt);

		if(!myCurrentFrame.isNull())
		{
			// Increment the playback time
			myPlaybackTime += context.dt;
			myCurrentFrameTime += context.dt;
		}
		else
		{
			omsg("Finished playback");
			myPlaying = false;
			if(myLoop)
			{
				stop();
				play();
			}
		}

		// If we dont have enough frames left to continue playback, switch to buffering mode and increase
		// buffering thresholds.
		int readyFrames = getNumReadyFrames();
		if(myLoading && readyFrames < myFrameBufferThreshold)
		{
			myBuffering = true;
			myFrameBufferThreshold *= 2;
			myFramesToBuffer *= 2;
			ofmsg("BUFFERING: ready frames: %1%    frames to buffer:%2%    frames to queue:%3%", %readyFrames %myFrameBufferThreshold %myFramesToBuffer);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FlipbookPlayer::queueNextFrame()
{
	// Get the last buffered frame number
	int lastBufferedFrame = -1;
	if(myBufferedFrames.size() > 0) lastBufferedFrame = myBufferedFrames.back()->frameNumber;

	// Increment it, to get the frame number for the next frame.
	lastBufferedFrame++;

	// Do we still have frames to load?
	if(lastBufferedFrame < myTotalFrames)
	{
		// Create a new frame object to hold the frame.
		Frame* nextFrame = new Frame();
		nextFrame->frameNumber = lastBufferedFrame;
		
		// Compute the time at which this frame should be displayed (useful for dropping frames)
		//if(nextFrame->frameNumber == 0) nextFrame->deadline = 0;
		//else nextFrame->deadline = myBufferedFrames.back()->deadline + myFrameTime;

		int nextFrameIndex = nextFrame->frameNumber + myStartFrame;
		
		// Get the path to the frame image file.
		String framePath = "";
		if(myIsMultidir)
		{
			int dirIndex = myStartDirIndex + nextFrameIndex / myFramesPerDir;
			framePath = ostr(myFramePath, %dirIndex %nextFrameIndex);
		}
		else
		{
			framePath = ostr(myFramePath, %nextFrameIndex);
		}

		// Load the frame image asynchronously. This call queues the image loading and 
		// returns a handle to check on the image load status.
		nextFrame->imageLoader = ImageUtils::loadImageAsync(framePath);

		// Add the frame to the list of currently buffering frames.
		myBufferedFrames.push_back(nextFrame);
	}
	else
	{
		omsg("Finished loading movie frames");
		myLoading = false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int FlipbookPlayer::getNumReadyFrames()
{
	// Count the number of consecutive frames whose image data has been loaded.
	int readyFrames = 0;
	foreach(Frame* frame, myBufferedFrames)
	{
		if(frame->imageLoader->isComplete()) readyFrames++;
		else break;
	}
	return readyFrames;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FlipbookPlayer::initializeRenderer(Renderer* r) 
{
	// Add our custom render pass to the renderer.
	r->addRenderPass(new FlipbookRenderPass(this, r));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FlipbookPlayer::dispose()
{
	getEngine()->removeRenderPass("FlipbookRenderPass");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FlipbookRenderPass::render(Renderer* renderer, const DrawContext& context)
{
	if(context.task == DrawContext::SceneDrawTask)
	{
		// Get the current frame data from the player
		Frame* frame = myPlayer->getCurrentFrame();
		if(frame != NULL)
		{
			PixelData* pixels = frame->imageLoader->getData().image;

			// If we did not initialize a texture for rendering, let's do it now.
			if(myFrameTexture == NULL)
			{
				myFrameTexture = renderer->createTexture();
				myFrameTexture->initialize(pixels->getWidth(), pixels->getHeight());
			}

			if(myPixelBuffer[myActiveTextureIndex] == NULL)
			{
				myPixelBuffer[myActiveTextureIndex] = new PixelData(pixels->getFormat(), pixels->getWidth(), pixels->getHeight(), NULL, PixelData::PixelBufferObject);
			}

			if(frame != myLastFrame)
			{
				int nextIndex = myActiveTextureIndex + 1;
				if(nextIndex == myPlayer->getNumGpuBuffers()) nextIndex = 0;

				myFrameTexture->writePixels(myPixelBuffer[nextIndex]);

				// Copy the frame pixels to the texture
				myPixelBuffer[myActiveTextureIndex]->copyFrom(pixels);

				myLastFrame = frame;
			
				// Swap textures
				myActiveTextureIndex = nextIndex;
			}
		}
	
		if(myFrameTexture != NULL)
		{
			// Draw the current frame.
			//if(context.task == DrawContext::OverlayDrawTask && context.eye == DrawContext::EyeCyclop)
			{
				DrawInterface* di = renderer->getRenderer();
				di->beginDraw2D(context);
				glColor4f(1, 1, 1, 1);

				const DisplayTileConfig* tile = context.tile;
				float cx = tile->offset.x();
				float cy = tile->offset.y();
				float sx = tile->pixelSize.x();
				float sy = tile->pixelSize.y();

				di->drawRectTexture(myFrameTexture, Vector2f(cx, cy), Vector2f(sx, sy));

				if(myPlayer->isPlaybackBarVisible()) drawPlaybackBar(di, cx + 5, cy + sy - 25, sx - 10, 20); 
			
				di->endDraw();
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FlipbookRenderPass::drawPlaybackBar(DrawInterface* di, int x, int y, int width, int height)
{
	Frame* frame = myPlayer->getCurrentFrame();

	int totalFrames = myPlayer->getTotalFrames();
	int currentFrame = (frame != NULL ? frame->frameNumber : 0);
	int bufferedFrames = myPlayer->getNumBufferedFrames() + currentFrame;
	int readyFrames = myPlayer->getNumReadyFrames() + currentFrame;

	int bufferedFramesLength = width * bufferedFrames / totalFrames;
	int readyFramesLength = width * readyFrames / totalFrames;
	int currentFrameLength = width * currentFrame / totalFrames;

	Color borderColor = Color(1, 1, 1);
	Color backColor = Color(0, 0, 0, 0.8f);
	Color currentFrameColor = Color(1, 0.2f, 0.2f);
	Color bufferedFramesColor = Color(0.3f, 0.3f, 0.3f);
	Color readyFramesColor = Color(0.5f, 0.5f, 0.5f);

	di->drawRect(Vector2f(x, y), Vector2f(width, height), backColor);
	di->drawRect(Vector2f(x, y), Vector2f(bufferedFramesLength, height), bufferedFramesColor);
	di->drawRect(Vector2f(x, y), Vector2f(readyFramesLength, height), readyFramesColor);
	di->drawRect(Vector2f(x, y), Vector2f(currentFrameLength, height), currentFrameColor);
	di->drawRectOutline(Vector2f(x, y), Vector2f(width, height), borderColor);
}