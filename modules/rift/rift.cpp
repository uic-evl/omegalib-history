#include <omega.h>

using namespace omega;

// This is the flag value we use to mark tiles that should use the oculus rift
// barrel correction shaders during postprocessing
uint RiftEnabledFlag = 1 << 16;

///////////////////////////////////////////////////////////////////////////////
// The oculus rift service implements the ICameraListener interface to perform
// postprocessing during rendering.
class OculusRiftService: public Service, ICameraListener
{
public:
	static OculusRiftService* New() { return new OculusRiftService(); }

	OculusRiftService():
		myCamera(NULL)
		{}

	// Service overrides
	virtual void initialize();
	virtual void start();
	virtual void poll();
	virtual void stop();
	virtual void dispose();

	// ICameraListener overrides
	virtual void beginDraw(Camera* cam, const DrawContext& context);
	virtual void endDraw(Camera* cam, const DrawContext& context);

private:
	Ref<Camera> myCamera;
	Ref<RenderTarget> myRenderTarget;
	Ref<Texture> myRenderTexture;
	Vector2f myViewportSize;
};

///////////////////////////////////////////////////////////////////////////////
// this fuction register the Oculus Rift service with the omegalib service
// manager
void registerService()
{
	ServiceManager* sm = SystemManager::instance()->getServiceManager();
	sm->registerService("OculusRiftService", (ServiceAllocator)OculusRiftService::New);
}

///////////////////////////////////////////////////////////////////////////////
// Python wrapper code.
#ifdef OMEGA_USE_PYTHON
#include "omega/PythonInterpreterWrapper.h"
BOOST_PYTHON_MODULE(rift)
{
	def("registerService", registerService);
}
#endif

///////////////////////////////////////////////////////////////////////////////
void OculusRiftService::initialize() 
{
	// Loop through display tiles and see which ones are marked to use the rift.
	// This is done for performance reasons: checking a flag at render time is
	// faster than going through the tile settings.
	DisplayConfig& cfg = SystemManager::instance()->getDisplaySystem()->getDisplayConfig();
	typedef KeyValue<String, DisplayTileConfig*> TileItem;
	foreach(TileItem tile, cfg.tiles)
	{
		bool riftEnabled = Config::getBoolValue("riftEnabled", tile->settingData, false);
		if(riftEnabled)
		{
			//tile->flags |= RiftEnabledFlag;
			// Force the stereo mode for this tile to be side-by-side.
			tile->stereoMode = DisplayTileConfig::SideBySide;
			ofmsg("OculusRiftService::initialize: rift postprocessing enabled for tile %1%", %tile->name);
		}
	}

	// The camera does not exist yet here, so we deal with it in the poll function.
}

///////////////////////////////////////////////////////////////////////////////
void OculusRiftService::start() 
{}

///////////////////////////////////////////////////////////////////////////////
void OculusRiftService::poll() 
{
	if(myCamera == NULL)
	{
		// Register myself as a camera listener.
		myCamera = Engine::instance()->getDefaultCamera();
		myCamera->addListener(this);
	}
}

///////////////////////////////////////////////////////////////////////////////
void OculusRiftService::stop() 
{}

///////////////////////////////////////////////////////////////////////////////
void OculusRiftService::dispose() 
{}

///////////////////////////////////////////////////////////////////////////////
void OculusRiftService::beginDraw(Camera* cam, const DrawContext& context)
{
	// Do we need to do rift postprocessing on this tile?
	if(context.tile->flags & RiftEnabledFlag)
	{
		// Create a render target if we have not done it yet.
		if(myRenderTarget == NULL)
		{
			myViewportSize = Vector2f(
				context.tile->pixelSize[0], context.tile->pixelSize[1]);

			myRenderTarget = new RenderTarget(context.gpuContext, RenderTarget::RenderToTexture);
			myRenderTexture = new Texture(context.gpuContext);
			myRenderTexture->initialize(myViewportSize[0], myViewportSize[1]);
			myRenderTarget->setTextureTarget(myRenderTexture);
		}

		//if(context.task == DrawContext::SceneDrawTask)
		{
			myRenderTarget->bind();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void OculusRiftService::endDraw(Camera* cam, const DrawContext& context)
{
	// Do we need to do rift postprocessing on this tile?
	if(context.tile->flags & RiftEnabledFlag)
	{
		//if(context.task == DrawContext::SceneDrawTask)
		{
			myRenderTarget->unbind();
		}
		if(context.task == DrawContext::OverlayDrawTask)
		{
			DrawInterface* di = context.renderer->getRenderer();
			di->beginDraw2D(context);
			di->drawRectTexture(myRenderTexture, Vector2f::Zero(), myViewportSize);
			di->endDraw();
			//myRenderTarget->clear();
		}
	}
}
