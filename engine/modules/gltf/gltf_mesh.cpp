#include "gltf_mesh.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "render/renderer.h"
#include "render/ShaderProgramRes.h"
#include "engine/core/main/Engine.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
	GltfMesh::GltfMesh()
		: m_assetPath("", ".gltf")
		, m_renderableDirty(true)
		, m_renderable(nullptr)
		, m_nodeIdx(-1)
		, m_meshIdx(-1)
		, m_skinIdx(-1)
		, m_primitiveIdx(-1)
		, m_material(nullptr)
		, m_skeletonDirty(false)
		, m_skeleton(nullptr)
	{
		set2d(false);
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
		CLASS_BIND_METHOD(GltfMesh, getSkeletonPath, DEF_METHOD("getSkeletonPath"));
		CLASS_BIND_METHOD(GltfMesh, setSkeletonPath, DEF_METHOD("setSkeletonPath"));

		CLASS_REGISTER_PROPERTY(GltfMesh, "Gltf", Variant::Type::ResourcePath, "getGltfRes", "setGltfRes");
		CLASS_REGISTER_PROPERTY(GltfMesh, "Mesh", Variant::Type::Int, "getMeshIdx", "setMeshIdx");
		CLASS_REGISTER_PROPERTY(GltfMesh, "Primitive", Variant::Type::Int, "getPrimitiveIdx", "setPrimitiveIdx");
		CLASS_REGISTER_PROPERTY_WITH_HINT(GltfMesh, "Material", Variant::Type::Object, PropertyHint::ResourceType, "Material", "getMaterial", "setMaterial");
		CLASS_REGISTER_PROPERTY(GltfMesh, "Skeleton", Variant::Type::NodePath, "getSkeletonPath", "setSkeletonPath");
	}

	// set gltf resource
	void GltfMesh::setGltfRes(const ResourcePath& path)
	{
		if (m_assetPath.setPath(path.getPath()))
		{
			m_asset = GltfRes::create(m_assetPath);
			m_renderableDirty = true;
		}
	}

	void GltfMesh::setSkeletonPath(const NodePath& skeletonPath)
	{
		if (m_skeletonPath.setPath(skeletonPath.getPath()))
		{
			m_skeletonDirty = true;
		}
	}

	// set mesh index
	void GltfMesh::setMeshIdx(int meshIdx) 
	{ 
		m_meshIdx = meshIdx;
		m_nodeIdx = m_asset->getNodeIdxByMeshIdx(m_meshIdx);
		m_skinIdx = m_asset->m_nodes[m_nodeIdx].m_skin;

		// prepare joints matrix
		const GltfSkinInfo& skinInfo = m_asset->m_skins[m_skinIdx];
		if (skinInfo.m_joints.size())
		{
			m_jointMatrixs.resize(skinInfo.m_joints.size());
		}

		m_renderableDirty = true;
	}

	// set primitive index
	void GltfMesh::setPrimitiveIdx(int primitiveIdx) 
	{
		m_primitiveIdx = primitiveIdx;
		m_renderableDirty = true;
	}

	void GltfMesh::setMaterial(Object* material) 
	{
		m_material = (Material*)material;
		m_renderableDirty = true;
	}

	// build drawable
	void GltfMesh::buildRenderable()
	{
		if ( m_renderableDirty && m_asset && m_meshIdx!=-1 && m_primitiveIdx!=-1)
		{
			Material* material = m_material ? m_material : m_asset->m_meshes[m_meshIdx].m_primitives[m_primitiveIdx].m_materialInst;
			if (material)
			{
				clearRenderable();

				Mesh* mesh = m_asset->m_meshes[m_meshIdx].m_primitives[m_primitiveIdx].m_mesh;
				m_renderable = Renderable::create(mesh, material, this);

				m_renderableDirty = false;
			}
		}
	}

	// update per frame
	void GltfMesh::update_self()
	{
		if (isNeedRender())
		{
			// update animation
			if (m_skeletonDirty)
			{
				m_skeleton = ECHO_DOWN_CAST<GltfSkeleton*>(getNode(m_skeletonPath.getPath().c_str()));
				m_skeletonDirty = false;
			}

			if (m_skeleton)
			{
				syncGltfNodeAnim();
				syncGltfSkinAnim();
			}

			buildRenderable();
			if (m_renderable)
				m_renderable->submitToRenderQueue();
		}
	}

	void GltfMesh::syncGltfNodeAnim()
	{
		if (m_skeleton)
		{
			if (m_skeleton->getGltfNodeTransform(m_localTransform, m_nodeIdx))
				needUpdate();
		}
	}

	void GltfMesh::syncGltfSkinAnim()
	{
		if (m_skeleton && m_skinIdx!=-1)
		{
			Transform tranform;

			const GltfSkinInfo& skinInfo = m_asset->m_skins[m_skinIdx];
			for (size_t i = 0; i < skinInfo.m_joints.size(); i++)
			{
				if (m_skeleton->getGltfNodeTransform(tranform, skinInfo.m_joints[i]))
				{
					tranform.buildMatrix(m_jointMatrixs[i]);

					// inverse matrix
					m_jointMatrixs[i] = skinInfo.m_inverseMatrixs[i] * m_jointMatrixs[i];
				}
			}
		}
	}

	void* GltfMesh::getGlobalUniformValue(const String& name)
	{
		void* value = Render::getGlobalUniformValue(name);
		if (value)
			return value;	

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
		else if (name == "u_JointMatrixs")
		{
			return m_jointMatrixs.data();
		}
		//else if (name == "u_DiffuseEnvSampler")
		//{
		//	static i32 idx = i32(GltfImageBasedLight::TextureIndex::DiffuseCube);
		//	return &idx;
		//}
		//else if (name == "u_SpecularEnvSampler")
		//{
		//	static i32 idx = i32(GltfImageBasedLight::TextureIndex::SpecularCube);
		//	return &idx;
		//}
		//else if (name == "u_brdfLUT")
		//{
		//	static i32 idx = i32(GltfImageBasedLight::TextureIndex::BrdfLUT);
		//	return &idx;
		//}

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