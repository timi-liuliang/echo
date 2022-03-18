#pragma once

#include <engine/core/util/Array.hpp>
#include "base/shader/shader_program.h"
#include "base/texture/texture.h"
#include "base/state/render_state.h"
#include "base/shader/material.h"
#include "base/mesh/mesh.h"

namespace Echo
{
	typedef ui32 RenderableID;

	class Render;
	class Material;
	class RenderProxy : public Object, public Refable
	{
		friend class Renderer;

	public:
		// Type
		enum RenderType
		{
			RenderType2D = 1 << 0,
			RenderType3D = 1 << 1,
			RenderTypeUI = 1 << 2,
			All = RenderType2D | RenderType3D | RenderTypeUI,
		};

	public:
		// Identifier
		ui32 getIdentifier() const { return m_identifier; }

		// Create method
		static RenderProxy* create(MeshPtr mesh, Material* matInst, Render* node, bool raytracing);

		// Release
		virtual void subRefCount() override;

		// Set mesh
		MeshPtr getMesh() { return m_mesh; }
		virtual void setMesh(MeshPtr mesh) = 0;

		// Set material
		virtual void setMaterial(Material* material) { m_material = material; }
		Material* getMaterial() { return m_material; }

		// Node(owner)
		void setNode( Render* node) { m_node = node; }
		Render* getNode() { return m_node; }

		// Is a part of raytracing
		bool IsRaytracing() const { return m_raytracing; }
		void setRaytracing(bool raytracing) { m_raytracing = raytracing; }

		// Cast shadow
		bool IsCastShadow() const { return m_castShadow; }
		void SetCastShadow(bool castShadow) { m_castShadow = castShadow; }

		// Is enable submit to render queues
		bool isSubmitToRenderQueue() const { return m_isSubmitToRenderQueue; }
		void setSubmitToRenderQueue(bool enable);

		// submit to renderqueue
		void submitToRenderQueue(class RenderPipeline* pipeline);

	protected:
		RenderProxy(int identifier);
		virtual ~RenderProxy();

	protected:
		ui32			m_identifier;
		Render*			m_node = nullptr;
		i32				m_bvhNodeId = -1;
		class Bvh*		m_bvh = nullptr;
		MeshPtr			m_mesh;
		MaterialPtr		m_material;
		bool			m_raytracing = false;
		bool			m_castShadow = false;
		bool			m_isSubmitToRenderQueue = false;
	};
	typedef ResRef<RenderProxy> RenderProxyPtr;
}
