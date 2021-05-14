#pragma once

#include <engine/core/util/Array.hpp>
#include "base/shader_program.h"
#include "base/texture.h"
#include "base/render_state.h"
#include "base/material.h"
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
		// identifier
		ui32 getIdentifier() const { return m_identifier; }

		// create method
		static RenderProxy* create(MeshPtr mesh, Material* matInst, Render* node);

		// release
		virtual void subRefCount() override;

		// set mesh
		MeshPtr getMesh() { return m_mesh; }
		virtual void setMesh(MeshPtr mesh) = 0;

		// set material
		virtual void setMaterial(Material* material) { m_material = material; }
		Material* getMaterial() { return m_material; }

		// node(owner)
		void setNode( Render* node) { m_node = node; }
		Render* getNode() { return m_node; }

		// submit to renderqueue
		void submitToRenderQueue();

	protected:
		RenderProxy(int identifier);
		virtual ~RenderProxy();

	public:
		ui32			m_identifier;
		Render*			m_node = nullptr;
		MeshPtr			m_mesh;
		MaterialPtr		m_material;
	};
	typedef ResRef<RenderProxy> RenderProxyPtr;
}
