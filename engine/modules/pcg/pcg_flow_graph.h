#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/material.h"
#include "engine/core/render/base/renderable.h"
#include "pcg/node/pcg_node.h"
#include "pcg/data/pcg_data.h"

namespace Echo
{
	class PCGFlowGraph : public Render
	{
		ECHO_CLASS(PCGFlowGraph, Render)

	public:
		PCGFlowGraph();
		virtual ~PCGFlowGraph();

		// PGNode
		PCGNode* getPGNode() { return m_pgNode; }

		// set mesh
		void setMesh(MeshPtr mesh);

		// material
		Material* getMaterial() const { return m_material; }
		void setMaterial(Object* material);

		// content
		const Base64String& getPGContent();
		void setPGContent(const Base64String& content);

		// is dirty
		bool isDirty();

		// run
		void run();

	protected:
		// update self
		virtual void update_self() override;

		// build drawable
		void buildRenderable();

		// clear
		void clearRenderable();

	protected:
		PCGNode*				m_pgNode = nullptr;
		PCGData					m_pgData;
		Base64String			m_pgContent;
		bool					m_isRenderableDirty = true;
		MeshPtr					m_mesh;
		MaterialPtr             m_material;
		Renderable*				m_renderable = nullptr;
	};
}
