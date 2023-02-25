#pragma once

#include <engine/core/util/Array.hpp>
#include "engine/core/render/base/shader/shader_program.h"
#include "engine/core/render/base/texture/texture.h"
#include "engine/core/render/base/state/render_state.h"
#include "engine/core/render/base/shader/material.h"
#include "engine/core/render/base/mesh/mesh.h"

namespace Echo
{
	typedef ui32 RenderableID;

	class Render;
	class RenderCamera;
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

		// Main camera
		void setCamera(RenderCamera* camera) { m_camera = camera; }
		RenderCamera* getCamera() { return m_camera; }

		// Shadow camera
		void setCameraShadow(RenderCamera* camera) { m_cameraShadow = camera; }
		RenderCamera* getCameraShadow() { return m_cameraShadow; }

		// Is a part of raytracing
		bool isRaytracing() const { return m_raytracing; }
		void setRaytracing(bool raytracing) { m_raytracing = raytracing; }

		// Cast shadow
		bool isCastShadow() const { return m_castShadow; }
		void setCastShadow(bool castShadow) { m_castShadow = castShadow; }

		// Custom depth stencil
		bool isCustomDepth() const { return m_customDepth; }
		void setCustomDepth( bool customDepth) { m_customDepth = customDepth; }

		// Is enable submit to render queues
		bool isSubmitToRenderQueue() const;
		void setSubmitToRenderQueue(bool enable);

		// submit to renderqueue
		void submitToRenderQueue(class RenderPipeline* pipeline);

	protected:
		RenderProxy();
		virtual ~RenderProxy();

	protected:
		Render*			m_node = nullptr;
		RenderCamera*	m_camera = nullptr;
		RenderCamera*	m_cameraShadow = nullptr;
		i32				m_bvhNodeId = -1;
		class Bvh*		m_bvh = nullptr;
		MeshPtr			m_mesh;
		MaterialPtr		m_material;
		bool			m_raytracing = false;
		bool			m_castShadow = false;
		bool			m_customDepth = false;
		bool			m_isSubmitToRenderQueue = false;
	};
	typedef ResRef<RenderProxy> RenderProxyPtr;
}
