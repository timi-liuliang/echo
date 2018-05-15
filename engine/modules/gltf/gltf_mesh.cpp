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
	GltfMesh::GltfMesh()
		: m_assetPath("", ".gltf")
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
		CLASS_BIND_METHOD(GltfMesh, getGltfRes, DEF_METHOD("getGltfRes"));
		CLASS_BIND_METHOD(GltfMesh, setGltfRes, DEF_METHOD("setGltfRes"));
		CLASS_BIND_METHOD(GltfMesh, getMeshIdx, DEF_METHOD("getMeshIdx"));
		CLASS_BIND_METHOD(GltfMesh, setMeshIdx, DEF_METHOD("setMeshIdx"));
		CLASS_BIND_METHOD(GltfMesh, getPrimitiveIdx, DEF_METHOD("getPrimitiveIdx"));
		CLASS_BIND_METHOD(GltfMesh, setPrimitiveIdx, DEF_METHOD("setPrimitiveIdx"));

		CLASS_REGISTER_PROPERTY(GltfMesh, "Gltf", Variant::Type::ResourcePath, "getGltfRes", "setGltfRes");
		CLASS_REGISTER_PROPERTY(GltfMesh, "Mesh", Variant::Type::Int, "getMeshIdx", "setMeshIdx");
		CLASS_REGISTER_PROPERTY(GltfMesh, "Primitive", Variant::Type::Int, "getPrimitiveIdx", "setPrimitiveIdx");
	}

	// set gltf resource
	void GltfMesh::setGltfRes(const ResourcePath& path)
	{
		if (m_assetPath.setPath(path.getPath()))
		{
			m_asset = GltfRes::create(m_assetPath);

			buildRenderable();
		}
	}

	// set mesh index
	void GltfMesh::setMeshIdx(int meshIdx) 
	{ 
		m_meshIdx = meshIdx;

		buildRenderable();
	}

	// set primitive index
	void GltfMesh::setPrimitiveIdx(int primitiveIdx) 
	{
		m_primitiveIdx = primitiveIdx;

		buildRenderable();
	}

	// build drawable
	void GltfMesh::buildRenderable()
	{
		if ( m_asset && m_meshIdx!=-1 && m_primitiveIdx!=-1)
		{
			clearRenderable();

			// material
			m_materialInst = MaterialInst::create();
			m_materialInst->setOfficialMaterialContent( GeneralMaterial::getPbrMetalicRoughnessContent());
			m_materialInst->setRenderStage("Transparent");
			m_materialInst->applyLoadedData();

			Mesh* mesh = m_asset->m_meshes[m_meshIdx].m_primitives[m_primitiveIdx].m_mesh;
			m_renderable = Renderable::create( mesh, m_materialInst, this);
		}
	}

	// update per frame
	void GltfMesh::update()
	{
		if (m_renderable)
		{
			m_matWVP = getWorldMatrix() * NodeTree::instance()->get3dCamera()->getViewProjMatrix();;
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