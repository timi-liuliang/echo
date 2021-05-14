#include "gltf_mesh.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "base/renderer.h"
#include "base/shader_program.h"
#include "engine/core/main/Engine.h"
#include "engine/core/gizmos/Gizmos.h"
#include "engine/modules/light/light_module.h"

namespace Echo
{
	GltfMesh::GltfMesh()
		: m_meshIdx(-1)
		, m_skinIdx(-1)
		, m_primitiveIdx(-1)
		, m_material(nullptr)
		, m_skeletonDirty(false)
		, m_skeleton(nullptr)
		, m_iblDiffuseSlot(-1)
		, m_iblSpecularSlot(-1)
		, m_iblBrdfSlot(-1)
	{
		setRenderType("3d");
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
		CLASS_REGISTER_PROPERTY(GltfMesh, "Material", Variant::Type::Object, "getMaterial", "setMaterial");
        CLASS_REGISTER_PROPERTY_HINT(GltfMesh, "Material", PropertyHintType::ResourceType, "Material");
        CLASS_REGISTER_PROPERTY(GltfMesh, "Skeleton", Variant::Type::NodePath, "getSkeletonPath", "setSkeletonPath");
	}

	// set gltf resource
	void GltfMesh::setGltfRes(const ResourcePath& path)
	{
		if (m_assetPath.setPath(path.getPath()))
		{
			m_asset = (GltfRes*)Res::get(m_assetPath);
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
		if (m_skinIdx != -1)
		{
			const GltfSkinInfo& skinInfo = m_asset->m_skins[m_skinIdx];
			if (skinInfo.m_joints.size())
			{
				m_jointMatrixs.resize(/*skinInfo.m_joints.size()*/ 72);
			}
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

				MeshPtr mesh = m_asset->m_meshes[m_meshIdx].m_primitives[m_primitiveIdx].m_mesh;
				m_renderable = RenderProxy::create(mesh, material, this);

				m_localAABB = mesh->getLocalBox();

				// image based lighting
				if (material->isMacroUsed("USE_IBL"))
				{
					//m_iblDiffuseSlot = material->getUniformValue("u_DiffuseEnvSampler") ? *(i32*)material->getUniformValue("u_DiffuseEnvSampler") : -1;
					//m_iblSpecularSlot = material->getUniformValue("u_SpecularEnvSampler") ? *(i32*)material->getUniformValue("u_SpecularEnvSampler") : -1;
					//m_iblBrdfSlot = material->getUniformValue("u_brdfLUT") ? *(i32*)material->getUniformValue("u_brdfLUT") : -1;
				}

				m_renderableDirty = false;
			}
		}
	}

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

			if(/*m_isUseLight*/ true)
			{
				syncLightData();
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

	// light data
	void GltfMesh::syncLightData()
	{
		if (m_renderable)
		{
			Material* material = m_material ? m_material : m_asset->m_meshes[m_meshIdx].m_primitives[m_primitiveIdx].m_materialInst;
			if (material->isMacroUsed("USE_IBL"))
			{
				//m_renderable->setTexture(m_iblDiffuseSlot, LightModule::instance()->getIBLDiffuseTexture());
				//m_renderable->setTexture(m_iblSpecularSlot, LightModule::instance()->getIBLSpecularTexture());
				//m_renderable->setTexture(m_iblBrdfSlot, LightModule::instance()->getIBLBrdfTexture());
			}
		}
	}

	void* GltfMesh::getGlobalUniformValue(const String& name)
	{
		void* value = Render::getGlobalUniformValue(name);
		if (value)
			return value;	

		if (name == "u_LightDirection")
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
		else if (name == "u_DiffuseEnvSampler")
		{
			static i32 idx = 0;// i32(GltfImageBasedLight::TextureIndex::DiffuseCube);
			return &idx;
		}
		else if (name == "u_SpecularEnvSampler")
		{
			static i32 idx = 0;// i32(GltfImageBasedLight::TextureIndex::SpecularCube);
			return &idx;
		}
		else if (name == "u_brdfLUT")
		{
			static i32 idx = 0;// i32(GltfImageBasedLight::TextureIndex::BrdfLUT);
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
		m_renderable.reset();
	}
}
