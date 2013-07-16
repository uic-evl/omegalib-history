#include <omega.h>
#include <omegaGl.h>

#include "OVRShaders.h"

using namespace omega;

// This is the flag value we use to mark tiles that should use the oculus rift
// barrel correction shaders during postprocessing
uint RiftEnabledFlag = 1 << 16;

// The global service instance, used by the python API to control the service.
class OculusRiftService* sInstance = NULL;

///////////////////////////////////////////////////////////////////////////////
// The oculus rift service implements the ICameraListener interface to perform
// postprocessing during rendering.
class OculusRiftService: public Service, ICameraListener
{
public:
	static OculusRiftService* New() { return new OculusRiftService(); }

	OculusRiftService():
		myCamera(NULL),
		myInitialized(false)
		{
		}

	// Service overrides
	virtual void initialize();
	virtual void start();
	virtual void poll();
	virtual void stop();
	virtual void dispose();

	virtual void initializeGraphics(Camera* cam, const DrawContext& context);

	// ICameraListener overrides
	virtual void beginDraw(Camera* cam, const DrawContext& context);
	virtual void endDraw(Camera* cam, const DrawContext& context);

	// Barrel shader parameters
	float getLensOffset() { return myLensOffset; }
	void setLensOffset(float value) { myLensOffset = value; }
	void setDistortionParam(int index, float value) { myDistortion[index] = value; }
	float getDistortionParam(int index) { return myDistortion[index]; }
	void setScaleParam(int index, float value) { myScaleParams[index] = value; }
	float getScaleParam(int index) { return myScaleParams[index]; }

private:
	bool myInitialized;
	Ref<Camera> myCamera;
	Ref<RenderTarget> myRenderTarget;
	Ref<Texture> myRenderTexture;
	Vector2f myViewportSize;

	Vector4f myDistortion;
	Vector4f myScaleParams;
	float myLensOffset;

	// Shader stuff
	GLuint myPostprocessProgram;
	GLint  myLensCenterUniform;
	GLint  myScreenCenterUniform;
	GLint  myScaleUniform;
	GLint  myScaleInUniform;
	GLint  myHmdWarpParamUniform;
	GLint  myTexture0Uniform;
};

///////////////////////////////////////////////////////////////////////////////
// Python API follows

// this fuction register the Oculus Rift service with the omegalib service
// manager
void registerService()
{
	ServiceManager* sm = SystemManager::instance()->getServiceManager();
	sm->registerService("OculusRiftService", (ServiceAllocator)OculusRiftService::New);
}

// Returns true when the rift service is enabled
bool isEnabled()
{
	if(sInstance != NULL) return true;
	return false;
}

// Returns a service instance
OculusRiftService* getService()
{
	return sInstance;
}

///////////////////////////////////////////////////////////////////////////////
// Python wrapper code.
#ifdef OMEGA_USE_PYTHON
#include "omega/PythonInterpreterWrapper.h"
BOOST_PYTHON_MODULE(rift)
{
	PYAPI_REF_BASE_CLASS(OculusRiftService)
		PYAPI_METHOD(OculusRiftService, getLensOffset)
		PYAPI_METHOD(OculusRiftService, setLensOffset)
		PYAPI_METHOD(OculusRiftService, setScaleParam)
		PYAPI_METHOD(OculusRiftService, getScaleParam)
		PYAPI_METHOD(OculusRiftService, setDistortionParam)
		PYAPI_METHOD(OculusRiftService, getDistortionParam)
		;

	def("registerService", registerService);
	def("isEnabled", isEnabled);
	def("getService", getService, PYAPI_RETURN_REF);
}
#endif


///////////////////////////////////////////////////////////////////////////////
void OculusRiftService::initialize() 
{
	sInstance = this;

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
			tile->flags |= RiftEnabledFlag;
			// Force the stereo mode for this tile to be side-by-side.
			tile->stereoMode = DisplayTileConfig::SideBySide;
			ofmsg("OculusRiftService::initialize: rift postprocessing enabled for tile %1%", %tile->name);
		}
	}

	// The camera does not exist yet here, so we deal with it in the poll function.
}

///////////////////////////////////////////////////////////////////////////////
void OculusRiftService::dispose() 
{
	sInstance = NULL;
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
void OculusRiftService::initializeGraphics(Camera* cam, const DrawContext& context)
{
	myViewportSize = Vector2f(
		context.tile->pixelSize[0], context.tile->pixelSize[1]);

	myRenderTarget = new RenderTarget(context.gpuContext, RenderTarget::RenderToTexture);
	myRenderTexture = new Texture(context.gpuContext);
	myRenderTexture->initialize(myViewportSize[0], myViewportSize[1]);
	myRenderTarget->setTextureTarget(myRenderTexture);

	// Setup shaders. Use some functions from the omegalib draw interface class 
	// to simplify shader and program creation.
	DrawInterface* di = context.renderer->getRenderer();
	GLuint vs = di->makeShaderFromSource(PostProcessVertexShaderSrc, 
		DrawInterface::VertexShader);

	GLuint fs = di->makeShaderFromSource(PostProcessFragShaderSrc, 
		DrawInterface::FragmentShader);

	myPostprocessProgram = di->createProgram(vs, fs);

	myLensCenterUniform = glGetUniformLocation(myPostprocessProgram, "LensCenter");
	myScreenCenterUniform = glGetUniformLocation(myPostprocessProgram, "ScreenCenter");
	myScaleUniform = glGetUniformLocation(myPostprocessProgram, "Scale");
	myScaleInUniform = glGetUniformLocation(myPostprocessProgram, "ScaleIn");
	myHmdWarpParamUniform = glGetUniformLocation(myPostprocessProgram, "HmdWarpParam");
	myTexture0Uniform = glGetUniformLocation(myPostprocessProgram, "Texture0");

	// Set default distortion values
	myDistortion = Vector4f(1.0f, 0.22f, 0.24f, 0.0f);

	// Set default scale parameters
	myScaleParams = Vector4f(0.145806f,  0.233290f, 4.0f, 2.5f);

	// Set default lens offset parameter
	myLensOffset =  0.287994f - 0.25f;

	myInitialized = true;
}

///////////////////////////////////////////////////////////////////////////////
void OculusRiftService::beginDraw(Camera* cam, const DrawContext& context)
{
	// Do we need to do rift postprocessing on this tile?
	if(context.tile->flags & RiftEnabledFlag)
	{
		// Create a render target if we have not done it yet.
		if(!myInitialized) initializeGraphics(cam, context);

		myRenderTarget->bind();
	}
}

///////////////////////////////////////////////////////////////////////////////
void OculusRiftService::endDraw(Camera* cam, const DrawContext& context)
{
	// Do we need to do rift postprocessing on this tile?
	if(context.tile->flags & RiftEnabledFlag)
	{
		myRenderTarget->unbind();

		// After all overlay rendering is done we have our full side-by-side
		// picture in the target texture. Now render it to the output framebuffer
		// performing barrel distortion postprocessing.
		if(context.task == DrawContext::OverlayDrawTask && 
			context.eye == DrawContext::EyeCyclop)
		{
			DrawInterface* di = context.renderer->getRenderer();
			di->beginDraw2D(context);

			glUseProgram(myPostprocessProgram);

			// Set uniforms common to left and right eye
			glUniform2f(myScaleUniform, myScaleParams[0],  myScaleParams[1]);

			glUniform2f(myScaleInUniform, myScaleParams[2], myScaleParams[3]);

			glUniform4f(myHmdWarpParamUniform,
				myDistortion[0],
				myDistortion[1],
				myDistortion[2],
				myDistortion[3]);

			// Set texture binding to texture unit 0 (the default used by the
			// drawRectTexture function).
			glUniform1i(myTexture0Uniform, 0);
		
			// Draw left eye
			// The left screen is centered at (0.25, 0.5)
			glUniform2f(myLensCenterUniform, 0.25f + myLensOffset, 0.5f);
			glUniform2f(myScreenCenterUniform, 0.25f, 0.5f);
			di->drawRectTexture(myRenderTexture, 
				Vector2f::Zero(), 
				Vector2f(myViewportSize[0] / 2, myViewportSize[1]),
				0,
				Vector2f(0, 0),	Vector2f(0.5f, 1.0f));

			// Draw right eye
			// The right screen is centered at (0.75, 0.5)
			glUniform2f(myLensCenterUniform, 0.75f, 0.5f);
			glUniform2f(myScreenCenterUniform, 0.75f - myLensOffset, 0.5f);
			di->drawRectTexture(myRenderTexture, 
				Vector2f(myViewportSize[0] / 2, 0), 
				Vector2f(myViewportSize[0] / 2, myViewportSize[1]),
				0,
				Vector2f(0.5, 0),	Vector2f(1.0f, 1.0f));

			glUseProgram(0);

			di->endDraw();
			myRenderTarget->clear();
		}
	}
}
