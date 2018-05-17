#include "gltf_module.h"
#include "gltf_mesh.h"
#include "gltf_ibl.h"

namespace Echo
{
	GltfModule::GltfModule()
	{
	}

	void GltfModule::registerTypes()
	{
		Class::registerType<GltfMesh>();
		Class::registerType<GltfImageBasedLight>();
	}
}