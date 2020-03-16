#include "sray_module.h"
#include "sray_mesh.h"
#include "sray_camera.h"
#include "sray_world.h"

namespace Echo
{
	DECLARE_MODULE(SRayModule)

	SRayModule::SRayModule()
	{
	}

    SRayModule::~SRayModule()
    {
        EchoSafeDeleteInstance(SRayWorld);
    }

	SRayModule* SRayModule::instance()
	{
		static SRayModule* inst = EchoNew(SRayModule);
		return inst;
	}

	void SRayModule::bindMethods()
	{

	}
    
	void SRayModule::registerTypes()
	{
		Class::registerType<SRayMesh>();
        Class::registerType<SRayWorld>();
	}
}
