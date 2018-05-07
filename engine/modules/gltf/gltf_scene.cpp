#include "gltf_scene.h"
#include "engine/core/util/LogManager.h"
#include "engine/core/scene/NodeTree.h"
#include "render/renderer.h"
#include "render/Material.h"
#include "engine/core/script/lua/luaex.h"
#include "engine/core/main/Root.h"

namespace Echo
{
	GltfScene::GltfScene()
		: m_gltfRes("", ".gltf")
	{
	}

	GltfScene::~GltfScene()
	{
	}

	void GltfScene::bindMethods()
	{
		CLASS_BIND_METHOD(GltfScene, getGltfRes, DEF_METHOD("getGltfRes"));
		CLASS_BIND_METHOD(GltfScene, setGltfRes, DEF_METHOD("setGltfRes"));

		CLASS_REGISTER_PROPERTY(GltfScene, "GltfRes", Variant::Type_ResourcePath, "getGltfRes", "setGltfRes");
	}

	// set texture res path
	void GltfScene::setGltfRes(const ResourcePath& path)
	{
		if (m_gltfRes.setPath(path.getPath()))
		{
			MemoryReader memReader(m_gltfRes.getPath());
			if (memReader.getSize())
			{
				// parse meshes
				int a = 10;
			}
		}
	}

	// update per frame
	void GltfScene::update()
	{
	}
}