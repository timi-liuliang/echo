#include "gltf_module.h"
#include "gltf_scene.h"
#include "gltf_mesh.h"

namespace Echo
{
	GltfModule::GltfModule()
	{
	}

	void GltfModule::registerTypes()
	{
		Class::registerType<GltfScene>();
		Class::registerType<GltfMesh>();
	}
}