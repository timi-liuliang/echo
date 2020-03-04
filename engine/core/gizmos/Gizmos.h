#pragma once

#include "engine/core/math/Math.h"
#include "engine/core/render/base/Material.h"
#include "engine/core/render/base/mesh/Mesh.h"
#include "engine/core/render/base/Renderable.h"
#include "engine/core/scene/render_node.h"

namespace Echo
{
	class Gizmos : public Render
	{
		ECHO_CLASS(Gizmos, Render)

	public:
		// vertex format
		struct VertexFormat
		{
			Vector3		m_position;
			Dword		m_color;

			VertexFormat(const Vector3& pos, const Color& color)
				: m_position(pos), m_color(color.getABGR())
			{		
			}
		};
		typedef vector<VertexFormat>::type	VertexArray;
		typedef vector<Word>::type			IndiceArray;

		// batch
		struct Batch
		{
			Gizmos*			m_gizmos;
			VertexArray		m_vertexs;
			IndiceArray		m_indices;
			MaterialPtr		m_material;
			Mesh*			m_mesh;
			Renderable*		m_renderable;
			bool			m_meshDirty;

			Batch(Material* material, Gizmos* gizmos);
			~Batch();
			void update();
			void addVertex(const VertexFormat& vert);
			void addIndex(Word idx);
			void clear();
		};

	public:
		Gizmos();
		~Gizmos();

		// draw line
		void drawLine(const Vector3& from, const Vector3& to, const Color& color);

		// draw trangle
		void drawTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Color& color);

		// is auto clear
		bool isAutoClear() const { return m_isAutoClear; }

		// set auto clear
		void setAutoClear(bool autClear) { m_isAutoClear = autClear; }

		// clear mesh data
		void clear();

	protected:
		// update
		virtual void update_self() override;
        
        // get default shader
        void initDefaultShader();

	private:
		bool			    m_isAutoClear;
        ShaderProgramPtr    m_shader;
		MaterialPtr		    m_material;
		Batch*			    m_lineBatch = nullptr;
		Batch*			    m_triangleBatch = nullptr;
	};
}
