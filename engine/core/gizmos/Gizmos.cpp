#include "Gizmos.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/scene/node_tree.h"

// opaque material
static const char* g_gizmoOpaqueMaterial = R"(
<?xml version = "1.0" encoding = "utf-8"?>
<Shader>
	<VS>#version 100

		attribute vec3 a_Position;
		attribute vec4 a_Color;

		uniform mat4 u_WorldMatrix;
		uniform mat4 u_ViewProjMatrix;

		varying vec3 v_Position;
		varying vec4 v_Color;

		void main(void)
		{
			vec4 position = /*u_WorldMatrix */ vec4(a_Position, 1.0);

			v_Position  = position.xyz;
			gl_Position = u_ViewProjMatrix * position;

			v_Color = a_Color;
		}
	</VS>
	<PS>#version 100

		precision mediump float;

		uniform vec3  u_CameraPosition;
		uniform float u_CameraFar;

		varying vec3  v_Position;
		varying vec4  v_Color;

		void main(void)
		{
			gl_FragColor    = v_Color;
		}
	</PS>
	<BlendState>
		<BlendEnable value = "true" />
		<SrcBlend value = "BF_SRC_ALPHA" />
		<DstBlend value = "BF_INV_SRC_ALPHA" />
	</BlendState>
	<RasterizerState>
		<CullMode value = "CULL_NONE" />
	</RasterizerState>
	<DepthStencilState>
		<DepthEnable value = "true" />
		<WriteDepth value = "false" />
	</DepthStencilState>
</Shader>
)";

namespace Echo
{
	Gizmos::Batch::Batch(Material* material, Gizmos* gizmos)
	{
		m_gizmos = gizmos;
		m_material = material;
		m_mesh = Mesh::create(true, true);
		m_renderable = nullptr;
		m_meshDirty = true;
	}

	Gizmos::Batch::~Batch()
	{
		// safe release renderable
		if (m_renderable)
		{
			m_renderable->release();
			m_renderable = nullptr;
		}
	}

	void Gizmos::Batch::addVertex(const Gizmos::VertexFormat& vert)
	{
		m_vertexs.push_back(vert);
		m_meshDirty = true;
	}

	void Gizmos::Batch::addIndex(Word idx)
	{
		m_indices.push_back(idx);
		m_meshDirty = true;
	}

	void Gizmos::Batch::clear()
	{
		m_vertexs.clear();
		m_indices.clear();

		m_meshDirty = true;
	}

	void Gizmos::Batch::update()
	{
		// update mesh and renderable
		if (m_meshDirty)
		{
			MeshVertexFormat define;
			define.m_isUseVertexColor = true;

			m_mesh->updateIndices((ui32)m_indices.size(), sizeof(Word), m_indices.data());
			m_mesh->updateVertexs(define, (ui32)m_vertexs.size(), (const Byte*)m_vertexs.data(), m_aabb);

			m_meshDirty = false;
		}

		// render
		if (m_renderable)
		{
			m_renderable->submitToRenderQueue();
		}
		else if(m_vertexs.size())
		{
			m_renderable = Renderable::create(m_mesh, m_material, m_gizmos);
			m_renderable->submitToRenderQueue();
		}

		// auto clear data
		if (m_gizmos->isAutoClear())
		{
			clear();
		}
	}

	// constructor
	Gizmos::Gizmos()
		: m_isAutoClear(false)
	{
		m_material = ECHO_CREATE_RES(Material);
		m_material->setShaderContent( "echo_gizmo_default_shader", g_gizmoOpaqueMaterial);
		m_material->setRenderStage("Transparent");

		m_lineBatch = EchoNew(Batch(m_material, this));
		m_lineBatch->m_mesh->setTopologyType(Mesh::TT_LINELIST);

		m_triangleBatch = EchoNew(Batch(m_material, this));
		m_triangleBatch->m_mesh->setTopologyType(Mesh::TT_TRIANGLELIST);
	}

	void Gizmos::bindMethods()
	{
	}

	// draw line
	void Gizmos::drawLine(const Vector3& from, const Vector3& to, const Color& color)
	{
		m_lineBatch->addIndex((Word)m_lineBatch->m_vertexs.size());
		m_lineBatch->addIndex((Word)m_lineBatch->m_vertexs.size() + 1);

		m_lineBatch->addVertex(VertexFormat(from, color));
		m_lineBatch->addVertex(VertexFormat(to, color));
	}

	// draw trangle
	void Gizmos::drawTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Color& color)
	{
		m_triangleBatch->addIndex((Word)m_triangleBatch->m_vertexs.size());
		m_triangleBatch->addIndex((Word)m_triangleBatch->m_vertexs.size() + 1);
		m_triangleBatch->addIndex((Word)m_triangleBatch->m_vertexs.size() + 2);

		m_triangleBatch->addVertex(VertexFormat(v0, color));
		m_triangleBatch->addVertex(VertexFormat(v1, color));
		m_triangleBatch->addVertex(VertexFormat(v2, color));
	}

	// clear mesh data
	void Gizmos::clear()
	{
		m_lineBatch->clear();
		m_triangleBatch->clear();
	}

	// update
	void Gizmos::update_self()
	{
		if (isNeedRender())
		{
			m_lineBatch->update();
			m_triangleBatch->update();
		}
	}
}