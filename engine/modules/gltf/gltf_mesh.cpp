#include "gltf_mesh.h"
#include "gltf_ibl.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/scene/NodeTree.h"
#include "render/renderer.h"
#include "render/ShaderProgramRes.h"
#include "engine/core/script/lua/luaex.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	GltfMesh::GltfMesh()
		: m_assetPath("", ".gltf")
		, m_renderable(nullptr)
		, m_meshIdx(-1)
		, m_primitiveIdx(-1)
		, m_material(nullptr)
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
		CLASS_BIND_METHOD(GltfMesh, getMaterial, DEF_METHOD("getMaterial"));
		CLASS_BIND_METHOD(GltfMesh, setMaterial, DEF_METHOD("setMaterial"));

		CLASS_REGISTER_PROPERTY(GltfMesh, "Gltf", Variant::Type::ResourcePath, "getGltfRes", "setGltfRes");
		CLASS_REGISTER_PROPERTY(GltfMesh, "Mesh", Variant::Type::Int, "getMeshIdx", "setMeshIdx");
		CLASS_REGISTER_PROPERTY(GltfMesh, "Primitive", Variant::Type::Int, "getPrimitiveIdx", "setPrimitiveIdx");
		CLASS_REGISTER_PROPERTY(GltfMesh, "Material", Variant::Type::Object, "getMaterial", "setMaterial");
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
			Material* origMaterial = m_asset->m_meshes[m_meshIdx].m_primitives[m_primitiveIdx].m_materialInst;
			if (origMaterial)
			{
				clearRenderable();

				Mesh* mesh = m_asset->m_meshes[m_meshIdx].m_primitives[m_primitiveIdx].m_mesh;
				m_materialDefault = ECHO_CREATE_RES(Material);
				m_materialDefault->clone(origMaterial);
				m_renderable = Renderable::create(mesh, m_materialDefault, this);
			}
		}
	}

	// update per frame
	void GltfMesh::update()
	{
		if (m_renderable)
		{
			m_matWVP = getWorldMatrix() * NodeTree::instance()->get3dCamera()->getViewProjMatrix();
			m_renderable->submitToRenderQueue();
		}
	}

	// 获取全局变量值
	void* GltfMesh::getGlobalUniformValue(const String& name)
	{
		void* value = Node::getGlobalUniformValue(name);
		if (value)
			return value;	

		if (name == "u_WVPMatrix")
			return (void*)(&m_matWVP);
		else if (name == "u_CameraPosition")
			return (void*)&(NodeTree::instance()->get3dCamera()->getPosition());
		else if (name == "u_LightDirection")
		{
			static Vector3 lightDirectionFromSurfaceToLight(1.f, 1.f, 0.5f);
			lightDirectionFromSurfaceToLight.normalize();
			return &lightDirectionFromSurfaceToLight;
		}
		else if (name == "u_LightColor")
		{
			static Vector3 lightColor(2.f, 2.f, 2.f);
			return &lightColor;
		}
		else if (name == "u_DiffuseEnvSampler")
		{
			static i32 idx = i32(GltfImageBasedLight::TextureIndex::DiffuseCube);
			return &idx;
		}
		else if (name == "u_SpecularEnvSampler")
		{
			static i32 idx = i32(GltfImageBasedLight::TextureIndex::SpecularCube);
			return &idx;
		}
		else if (name == "u_brdfLUT")
		{
			static i32 idx = i32(GltfImageBasedLight::TextureIndex::BrdfLUT);
			return &idx;
		}

		return nullptr;
	}

	void GltfMesh::clear()
	{
		clearRenderable();
	}

	void GltfMesh::clearRenderable()
	{
		EchoSafeRelease(m_renderable);
	}
}