#pragma once

#include "engine/core/io/IO.h"
#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/Mesh.h"
#include "engine/core/render/base/Material.h"
#include "engine/core/render/base/Renderable.h"
#include "live2d_cubism_motion.h"

namespace Echo
{
	class Live2dCubism : public Render
	{
		ECHO_CLASS(Live2dCubism, Render)

		struct CanvasInfo
		{
			float		m_width;
			float		m_height;
			Vector2		m_originInPixels;
			float	    m_pixelsPerUnit;
		};

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
		typedef vector<Word>::type	IndiceArray;

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
			AABB				m_box;
			vector<Word>::type	m_indices;

			void reset()
			{
				m_vertices.clear();
				m_indices.clear();
			}
		};

		typedef map<String, Live2dCubismMotion*>::type MotionMap;

	public:
		Live2dCubism();
		virtual ~Live2dCubism();

		// moc
		void setMoc(const ResourcePath& res);
		const ResourcePath& getMoc() { return m_mocRes; }

		// texture res path
		void setTextureRes(const ResourcePath& path);
		const ResourcePath& getTextureRes() { return m_textureRes; }

		// material
		Material* getMaterial() const { return m_material; }
		void setMaterial( Object* material);

		// anim res path
		void setMotionRes(const ResourcePath& path);
		const ResourcePath& getMotionRes() const { return m_curMotionRes; }

		// set parameter value
		void setParameter(const String& name, float value);

	protected:
		// build drawable
		void buildRenderable();

		// update
		virtual void update_self() override;

		// update vertex buffer
		void updateMeshBuffer();

		// parse paramters
		void parseParams();

		// parse canvas info
		void parseCanvasInfo();

		// parse parts
		void parseParts();

		// parse drawables
		void parseDrawables();

		// build mesh data by drawables data
		void buildMeshDataByDrawables(VertexArray& oVertices, IndiceArray& oIndices);

		// clear
		void clear();
		void clearRenderable();

	private:
		ResourcePath			m_mocRes;
		ResourcePath			m_textureRes;
		ResourcePath			m_curMotionRes;
		Live2dCubismMotion*		m_curMotion;
		MemoryReaderAlign*		m_mocMemory;
		csmMoc*					m_moc;
		ui32					m_modelSize;
		void*					m_modelMemory;
		csmModel*				m_model = nullptr;
		ui32					m_tableSize = 0;
		void*					m_tableMemory = nullptr;
		csmModelHashTable*		m_table = nullptr;
		CanvasInfo				m_canvas;
		vector<Paramter>::type	m_params;
		vector<Part>::type		m_parts;
		vector<Drawable>::type	m_drawables;
		MotionMap				m_motions;

		Mesh*					m_mesh;				// Geometry Data for render
		MaterialPtr				m_material;			// Custom Material
		MaterialPtr				m_materialDefault;
		Renderable*				m_renderable;
	};
}
