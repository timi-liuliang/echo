#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/material.h"
#include "engine/core/render/base/proxy/render_proxy.h"

namespace Echo
{
	class Sprite : public Render
	{
		ECHO_CLASS(Sprite, Render)

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
		enum BillboardType
		{
			None,
			LookAt,
			ViewDir,
		};

	public:
		Sprite();
		virtual ~Sprite();

		// type
		StringOption getBillboardType();
		void setBillobardType(const StringOption& type);

		// width
		float getWidth() const { return m_width; }
		void setWidth(float width);

		// width
		float getHeight() const { return m_height; }
		void setHeight(float height);

		// center offset
		const Vector2& getOffset() const { return m_offset; }
		void setOffset(const Vector2& offset);

		// material
		Material* getMaterial() const { return m_material; }
		void setMaterial(Object* material);

	protected:
		// build drawable
		void buildRenderable();

		// update
		virtual void updateInternal() override;

		// update billboard
		void updateBillboard();

		// update vertex buffer
		void updateMeshBuffer();

	private:
		bool                    m_isRenderableDirty = true;
		BillboardType			m_billboardType = BillboardType::None;
		float					m_width = 64.f;
		float					m_height = 64.f;
		Vector2					m_offset = Vector2::ZERO;
		MeshPtr				    m_mesh;						// Geometry Data for render
		ShaderProgramPtr		m_shaderDefault;
		MaterialPtr				m_material;		            // Material Instance
		RenderProxyPtr			m_renderable = nullptr;
	};
}
