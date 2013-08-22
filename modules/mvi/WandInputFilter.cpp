#include "WandInputFilter.h"

///////////////////////////////////////////////////////////////////////////////
WandInputFilter* WandInputFilter::createAndInitialize()
{
	WandInputFilter* instance = new WandInputFilter();
	ModuleServices::addModule(instance);
	instance->doInitialize(Engine::instance());
	return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
WandInputFilter::WandInputFilter():
	EngineModule("WandInputFilter")
{
	// This module runs before every other, to be able to filter input events.
	setPriority(EngineModule::PriorityHighest);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WandInputFilter::initialize()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WandInputFilter::update(const UpdateContext& context)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WandInputFilter::handleEvent(const Event& evt)
{
	DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
	DisplayConfig& dcfg = ds->getDisplayConfig();

	// If the event is a wand event, get the ray and see if it intersects any
	// active tile.
	if(evt.getServiceType() == Event::ServiceTypeWand)
	{
		// Get the wand ray
		Ray ray;
		ds->getViewRayFromEvent(evt, ray);

		// Add enabled planes to the plane list
		List<Plane> tilePlanes;
		typedef KeyValue<String, DisplayTileConfig*> TileItem;
		foreach(TileItem dtc, dcfg.tiles)
		{
			if(dtc->enabled)
			{
				tilePlanes.push_back(Plane(
					dtc->topLeft,
					dtc->bottomRight,
					dtc->bottomRight));
			}
		}

		// See if there is an intersection between the wand ray and any of 
		// the enabled tiles.
		pair<bool, float> intersect = Math::intersects(ray, tilePlanes, false);

		// No intersection: mark the wand event as processed so it will not
		// be dispatched to other modules.
		if(!intersect.first)
		{
			evt.setProcessed();
		}
	}
}
