#pragma once

#include "engine/core/scene/Node.h"
#include "engine/core/render/mesh/Mesh.h"
#include "engine/core/render/MaterialInst.h"
#include "engine/core/render/render/Renderable.h"

namespace Echo
{
	class Sprite : public Node
	{
		ECHO_CLASS(Sprite, Node)

		struct VertexFormat
		{
			Vector3		m_position;
			Vector2		m_uv;

			VertexFormat(const Vector3& pos, const Vector2& uv)
				: m_position(pos), m_uv(uv)
			{}
		};
		typedef vector<VertexFormat>::type	VertexArray;
		typedef vector<Word>::type	IndiceArray;

	public:
		Sprite();
		virtual ~Sprite();

		// bind class methods to script
		static void bindMethods();

		// set texture res path
		void setTextureRes(const ResourcePath& path);

		// get texture res
		const ResourcePath& getTextureRes() { return m_textureRes; }

	protected:
		// build drawable
		void buildRenderable();

		// update
		virtual void update();

		// update vertex buffer
		void updateMeshBuffer();

		// build mesh data by drawables data
		void buildMeshData(VertexArray& oVertices, IndiceArray& oIndices);

		// get global uniforms
		virtual void* getGlobalUniformValue(const String& name);

		// clear
		void clear();
		void clearRenderable();

	private:
		ResourcePath			m_textureRes;
		Mesh*					m_mesh;			// Geometry Data for render
		MaterialInst*			m_materialInst;	// Material Instance
		Renderable*				m_renderable;
		Matrix4					m_matWVP;
	};
}