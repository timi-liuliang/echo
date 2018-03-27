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

		struct Paramter
		{
			String	m_name;
			float	m_value;
			float	m_defaultValue;
			float	m_minValue;
			float	m_maxValue;
		};

		struct Part
		{
			String  m_name;
			float	m_opacities;
		};

		struct VertexFormat
		{
			Vector3		m_position;
			Vector2		m_uv;
		};
		typedef vector<VertexFormat>::type	VertexArray;

		struct Drawable
		{
			String				m_name;
			csmFlags			m_constantFlag;
			csmFlags			m_dynamicFlag;
			ui32				m_textureIndex;
			ui32				m_drawOrder;
			ui32				m_renderOrder;
			float				m_opacitie;
			vector<ui32>::type	m_masks;
			VertexArray			m_vertices;
			Box					m_box;
			vector<Word>::type	m_indices;
			Mesh*				m_mesh;			// Geometry Data for render
			MaterialInst*		m_materialInst;	// Material Instance

			// build for render
			void build();
		};

	public:
		Live2dCubism();
		virtual ~Live2dCubism();

		// bind class methods to script
		static void bindMethods();

		// set moc
		void setMoc(const String& res);

		// build drawable
		void buildDrawables();

	protected:
		// update
		virtual void update();

		// parse paramters
		void parseParams();

		// parse canvas info
		void parseCanvasInfo();

		// parse parts
		void parseParts();

		// parse drawables
		void parseDrawables();

	private:
		csmMoc*					m_moc;
		ui32					m_modelSize;
		void*					m_modelMemory;
		csmModel*				m_model;
		vector<Paramter>::type	m_params;
		vector<Part>::type		m_parts;
		vector<Drawable>::type	m_drawables;
	};
}