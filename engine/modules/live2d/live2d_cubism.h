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

		// bind class methods to script
		static void bindMethods();

		// set moc
		void setMoc(const String& res);

		// set model
		void setModel(const String& model);

	protected:
		// update
		virtual void update();

	private:
		csmMoc*			m_moc;
		csmModel*		m_model;
		Mesh*			m_mesh;			// Geometry Data for render
		MaterialInst*	m_materialInst;	// Material Instance
	};
}