#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/material.h"
#include "engine/core/render/base/renderable.h"

namespace Echo
{
	class MeshRender : public Render
	{
		ECHO_CLASS(MeshRender, Render);

	public:
		MeshRender();
		virtual ~MeshRender();

		// material
		Mesh* getMesh() const { return m_mesh; }
		void setMesh(Object* mesh);

		// material
		Material* getMaterial() const { return m_material; }
		void setMaterial(Object* material);

	protected:
		// update self
		virtual void update_self() override;

		// build drawable
		void buildRenderable();

		// clear
		void clearRenderable();

	private:
		bool			m_isRenderableDirty = true;
		MeshPtr		m_mesh;
		MaterialPtr		m_material;
		Renderable*		m_renderable = nullptr;
	};
}
