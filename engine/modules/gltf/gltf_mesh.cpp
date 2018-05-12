#include "gltf_mesh.h"
#include "engine/core/util/LogManager.h"
#include "engine/core/scene/NodeTree.h"
#include "render/renderer.h"
#include "render/Material.h"
#include "engine/core/script/lua/luaex.h"
#include "engine/core/main/Root.h"
#include "engine/core/render/material/generalmaterial.h"

namespace Echo
{
	static void csmLogFunc(const char* message)
	{
		EchoLogError(message);
	}

	GltfMesh::GltfMesh()
		: m_textureRes("", ".png")
		, m_materialInst(nullptr)
		, m_renderable(nullptr)
		, m_meshIdx(-1)
		, m_primitiveIdx(-1)
	{
	}

	GltfMesh::~GltfMesh()
	{
		clear();
	}

	void GltfMesh::bindMethods()
	{
		CLASS_BIND_METHOD(GltfMesh, getTextureRes, DEF_METHOD("getTextureRes"));
		CLASS_BIND_METHOD(GltfMesh, setTextureRes, DEF_METHOD("setTextureRes"));

		CLASS_REGISTER_PROPERTY(GltfMesh, "Texture", Variant::Type_ResourcePath, "getTextureRes", "setTextureRes");
	}

	// set texture res path
	void GltfMesh::setTextureRes(const ResourcePath& path)
	{
		if (m_textureRes.setPath(path.getPath()))
		{
			clearRenderable();
			buildRenderable();
		}
	}

	// set geometry data
	void GltfMesh::setGeometryData(GltfRes* asset, int meshIdx, int primitiveIdx)
	{
		m_asset = asset;
		m_meshIdx = meshIdx;
		m_primitiveIdx = primitiveIdx;
		if (m_meshIdx != -1)
		{
			clearRenderable();
			buildRenderable();
		}
	}

	// build drawable
	void GltfMesh::buildRenderable()
	{
		if (!m_textureRes.getPath().empty() && m_asset)
		{
			// material
			m_materialInst = MaterialInst::create();
			m_materialInst->setOfficialMaterialContent( GeneralMaterial::getContent());
			m_materialInst->setRenderStage("Transparent");
			m_materialInst->applyLoadedData();

			m_materialInst->setTexture(0, m_textureRes.getPath());

			Mesh* mesh = m_asset->m_meshes[m_meshIdx].m_primitives[m_primitiveIdx].m_mesh;
			m_renderable = Renderable::create( mesh, m_materialInst, this);
		}
	}

	// update per frame
	void GltfMesh::update()
	{
		if (m_renderable)
		{
			m_matWVP = getWorldMatrix() * NodeTree::instance()->get2DCamera()->getViewProjMatrix();;
			m_renderable->submitToRenderQueue();
		}
	}

	// 获取全局变量值
	void* GltfMesh::getGlobalUniformValue(const String& name)
	{
		if (name == "matWVP")
			return (void*)(&m_matWVP);

		return nullptr;
	}

	void GltfMesh::clear()
	{
		clearRenderable();
	}

	void GltfMesh::clearRenderable()
	{
		EchoSafeRelease(m_renderable);
		EchoSafeRelease(m_materialInst);
	}
}