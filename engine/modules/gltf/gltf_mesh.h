#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/scene/node_path.h"
#include "engine/core/render/mesh/Mesh.h"
#include "engine/core/render/Material.h"
#include "engine/core/render/render/Renderable.h"
#include "gltf_res.h"
#include "gltf_skeleton.h"

namespace Echo
{
	class GltfMesh : public Render
	{
		ECHO_CLASS(GltfMesh, Render)

	public:
		GltfMesh();
		virtual ~GltfMesh();

		// set gltf resource
		const ResourcePath& getGltfRes() { return m_assetPath; }
		void setGltfRes(const ResourcePath& path);

		// set mesh index
		int getMeshIdx() const { return m_meshIdx; }
		void setMeshIdx(int meshIdx);

		// set primitive index
		int getPrimitiveIdx() { return m_primitiveIdx; }
		void setPrimitiveIdx(int primitiveIdx);

		// material
		Material* getMaterial() const { return m_material; }
		void setMaterial( Object* material);

		// skeleton
		const NodePath& getSkeletonPath() { return m_skeletonPath; }
		void setSkeletonPath(const NodePath& skeletonPath);

	protected:
		// build drawable
		void buildRenderable();

		// update
		virtual void update_self();

		// get global uniforms
		virtual void* getGlobalUniformValue(const String& name);

		// clear
		void clear();
		void clearRenderable();

		// gltf anim
		void syncGltfNodeAnim();
		void syncGltfSkinAnim();

	private:
		bool					m_renderableDirty;
		Renderable*				m_renderable;
		Matrix4					m_matWVP;
		ResourcePath			m_assetPath;
		GltfResPtr				m_asset;			// gltf asset ptr
		int						m_nodeIdx;			// node index in the asset, used by skeleton
		int						m_meshIdx;			// mesh index in the asset
		int						m_primitiveIdx;		// sub mesh index
		MaterialPtr				m_material;			// custom material
		NodePath				m_skeletonPath;
		GltfSkeleton*			m_skeleton;
	};
}