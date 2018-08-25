#include "gltf_module.h"
#include "gltf_mesh.h"

namespace Echo
{
	GltfModule::GltfModule()
	{
	}

	void GltfModule::registerTypes()
	{
		Class::registerType<GltfMesh>();
	}
}