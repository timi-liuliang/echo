#include "gltf_module.h"
#include "gltf_mesh.h"
#include "gltf_skeleton.h"

namespace Echo
{
	GltfModule::GltfModule()
	{
	}

	void GltfModule::registerTypes()
	{
		Class::registerType<GltfRes>();
		Class::registerType<GltfMesh>();
		Class::registerType<GltfSkeleton>();
	}
}