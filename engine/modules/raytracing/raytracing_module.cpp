#include "raytracing_module.h"
#include "raytracing_mesh.h"
#include "raytracing_camera.h"
#include "raytracing_world.h"
#include "raytracing_queue.h"

namespace Echo
{
	DECLARE_MODULE(RaytracingModule)

	RaytracingModule::RaytracingModule()
	{
	}

    RaytracingModule::~RaytracingModule()
    {
        EchoSafeDeleteInstance(RaytracingWorld);
    }

	RaytracingModule* RaytracingModule::instance()
	{
		static RaytracingModule* inst = EchoNew(RaytracingModule);
		return inst;
	}

	void RaytracingModule::bindMethods()
	{

	}
    
	void RaytracingModule::registerTypes()
	{
		Class::registerType<RaytracingMesh>();
        Class::registerType<RaytracingWorld>();
		Class::registerType<RaytracingQueue>();
	}

	void RaytracingModule::update(float elapsedTime)
	{
		RaytracingWorld::instance()->step(elapsedTime);
	}
}
