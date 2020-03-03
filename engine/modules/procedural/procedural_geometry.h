#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/Mesh.h"
#include "engine/core/render/base/Material.h"
#include "engine/core/render/base/Renderable.h"

namespace Echo
{
	class ProceduralGeometry : public Render
	{
		ECHO_CLASS(ProceduralGeometry, Render)

	public:
		ProceduralGeometry();
		virtual ~ProceduralGeometry();

		// set mesh
		void setMesh(Mesh* mesh);

		// material
		Material* getMaterial() const { return m_material; }
		void setMaterial(Object* material);

	protected:
		// update self
		virtual void update_self() override;

		// build mesh
		virtual void buildMesh();

		// build drawable
		void buildRenderable();

		// clear
		void clearRenderable();

	protected:
		bool					m_isRenderableDirty = true;
		Mesh*					m_mesh = nullptr;
		MaterialPtr             m_material;
		Renderable*				m_renderable = nullptr;
	};
}
