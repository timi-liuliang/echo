#include "gltf_module.h"
#include "gltf_mesh.h"
#include "gltf_skeleton.h"

namespace Echo
{
	DECLARE_MODULE(GltfModule, __FILE__)

	GltfModule::GltfModule()
	{
	}

	GltfModule* GltfModule::instance()
	{
		static GltfModule* inst = EchoNew(GltfModule);
		return inst;
	}

	void GltfModule::bindMethods()
	{

	}

	void GltfModule::registerTypes()
	{
		Class::registerType<GltfRes>();
		Class::registerType<GltfMesh>();
		Class::registerType<GltfSkeleton>();
	}
}