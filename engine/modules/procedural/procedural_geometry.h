#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/Material.h"
#include "engine/core/render/base/Renderable.h"
#include "node/pg/pg_node.h"
#include "data/pcg_data.h"

namespace Echo
{
	class ProceduralGeometry : public Render
	{
		ECHO_CLASS(ProceduralGeometry, Render)

	public:
		ProceduralGeometry();
		virtual ~ProceduralGeometry();

		// PGNode
		PGNode* getPGNode() { return m_pgNode; }

		// set mesh
		void setMesh(MeshPtr mesh);

		// material
		Material* getMaterial() const { return m_material; }
		void setMaterial(Object* material);

		// run
		void run();

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
		PGNode*					m_pgNode = nullptr;
		PCGData					m_data;
		bool					m_isRenderableDirty = true;
		MeshPtr					m_mesh;
		MaterialPtr             m_material;
		Renderable*				m_renderable = nullptr;
	};
}
