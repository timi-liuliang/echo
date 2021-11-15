#pragma once

#include <engine/core/util/Array.hpp>
#include "base/shader/shader_program.h"
#include "base/state/render_state.h"
#include "base/shader/material.h"
#include "base/mesh/mesh.h"

namespace Echo
{
	class Render;
	class Material;
	class ComputeProxy : public Object
	{
		friend class Renderer;

	public:
		// identifier
		ui32 getIdentifier() const { return m_identifier; }

		// create method
		static ComputeProxy* create(MeshPtr mesh, Material* matInst, Render* node);

		// release
		void release();

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
		ComputeProxy(int identifier);
		virtual ~ComputeProxy();

	public:
		ui32			m_identifier;
		Render*			m_node = nullptr;
		MeshPtr			m_mesh;
		MaterialPtr		m_material;
	};
	typedef ui32 RenderableID;
}
