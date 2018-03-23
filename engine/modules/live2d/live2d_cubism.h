#pragma once

#include "engine/core/scene/Node.h"
#include "engine/core/render/mesh/Mesh.h"
#include "engine/core/render/MaterialInst.h"

namespace Echo
{
	class Live2dCubism : public Node
	{
		ECHO_CLASS(Live2dCubism, Node)

	public:
		Live2dCubism();
		virtual ~Live2dCubism();

	protected:
		// update
		virtual void update();

	private:
		void*			m_model;
		Mesh*			m_mesh;			// Geometry Data for render
		MaterialInst*	m_materialInst;	// Material Instance
	};
}