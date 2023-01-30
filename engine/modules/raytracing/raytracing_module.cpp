#include "raytracing_module.h"
#include "raytracing_mesh.h"
#include "raytracing_camera.h"
#include "raytracing_world.h"

namespace Echo
{
	DECLARE_MODULE(RaytracingModule, __FILE__)

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
	}

	void RaytracingModule::update(float elapsedTime)
	{
		RaytracingWorld::instance()->step(elapsedTime);
	}
}
