#pragma once

#include "engine/core/math/Math.h"
#include "engine/core/render/render/Color.h"
#include "engine/core/render/Material.h"
#include "engine/core/render/mesh/Mesh.h"
#include "engine/core/render/render/Renderable.h"
#include "engine/core/scene/node.h"

namespace Echo
{
	class Gizmos : public Node
	{
		ECHO_CLASS(Gizmos, Node)

	public:
		// vertex format
		struct VertexFormat
		{
			Vector3		m_position;

			VertexFormat(const Vector3& pos)
				: m_position(pos)
			{}
		};
		typedef vector<VertexFormat>::type	VertexArray;
		typedef vector<Word>::type			IndiceArray;

	public:
		Gizmos();

		// draw line
		void drawLine(const Vector3& from, const Vector3& to, const Color& color, bool transparent, float thickNess);

		// get global uniforms
		virtual void* getGlobalUniformValue(const String& name);

		// update
		virtual void update();

	private:
		VertexArray		m_vertexsOpaque;
		IndiceArray		m_indicesOpaque;
		AABB			m_aabb;
		MaterialPtr		m_materialOpaque;
		Mesh*			m_meshOpaque;
		Renderable*		m_renderableOpaque;
		MaterialPtr		m_materialTransparent;
	};
}