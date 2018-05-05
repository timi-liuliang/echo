#include "gltf_module.h"
#include "gltf_scene.h"

namespace Echo
{
	GltfModule::GltfModule()
	{
	}

	void GltfModule::registerTypes()
	{
		Class::registerType<GltfScene>();
	}
}