#pragma once

#include "engine/core/scene/Node.h"
#include "engine/core/render/mesh/Mesh.h"
#include "engine/core/render/MaterialInst.h"

extern "C"
{
#include "thirdparty\live2d\Cubism31SdkNative-EAP5\Core\include\Live2DCubismCore.h"
}


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

	protected:
		// update
		virtual void update();

	private:
		csmMoc*			m_moc;
		ui32			m_modelSize;
		void*			m_modelMemory;
		csmModel*		m_model;
		Mesh*			m_mesh;			// Geometry Data for render
		MaterialInst*	m_materialInst;	// Material Instance
	};
}