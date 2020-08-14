#pragma once

#include <engine/core/util/Array.hpp>
#include "ShaderProgram.h"
#include "Texture.h"
#include "RenderState.h"
#include "Material.h"
#include "mesh/mesh.h"

namespace Echo
{
	class Render;
	class Material;
	class Renderable : public Object
	{
		friend class Renderer;

	public:
		// identifier
		ui32 getIdentifier() const { return m_identifier; }

		// create method
		static Renderable* create(MeshResPtr mesh, Material* matInst, Render* node);

		// release
		void release();

		// set mesh
		MeshResPtr getMesh() { return m_mesh; }
		virtual void setMesh(MeshResPtr mesh) = 0;

		// set material
		virtual void setMaterial(Material* material) { m_material = material; }
		Material* getMaterial() { return m_material; }

		// node(owner)
		void setNode( Render* node) { m_node = node; }
		Render* getNode() { return m_node; }

		// submit to renderqueue
		void submitToRenderQueue();

	protected:
		Renderable(int identifier);
		virtual ~Renderable();

	public:
		// bind render state
		void bindRenderState();

	public:
		ui32									m_identifier;
		Render*									m_node = nullptr;
		MeshResPtr								m_mesh;
		MaterialPtr								m_material;
	};
	typedef ui32 RenderableID;
}
