#include "Gizmos.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/scene/NodeTree.h"

// opaque material
static const char* g_gizmoOpaqueMaterial = R"(
<?xml version = "1.0" encoding = "utf-8"?>
<Shader>
	<VS>
		#version 300

		attribute vec3 a_Position;

		uniform mat4 u_VPMatrix;

		void main(void)
		{
			vec4 position = u_VPMatrix * vec4(a_Position, 1.0);
			gl_Position = position;
		}
	</VS>
	<PS>
		#version 100

		void main(void)
		{
			gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
		}
	</PS>
	<BlendState>
		<BlendEnable value = "false" />
	</BlendState>
	<RasterizerState>
		<CullMode value = "CULL_NONE" />
	</RasterizerState>
	<DepthStencilState>
		<DepthEnable value = "true" />
		<WriteDepth value = "true" />
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
			if (m_vertexs.size())
			{
				// safe release renderable
				if (m_renderable)
				{
					m_renderable->release();
					m_renderable = nullptr;
				}

				MeshVertexFormat define;

				m_mesh->updateIndices(m_indices.size(), m_indices.data());
				m_mesh->updateVertexs(define, m_vertexs.size(), (const Byte*)m_vertexs.data(), m_aabb);

				m_renderable = Renderable::create(m_mesh, m_material, m_gizmos);

				m_meshDirty = false;
			}
		}

		// render
		if(m_renderable)
			m_renderable->submitToRenderQueue();

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
		m_material->setShaderContent(g_gizmoOpaqueMaterial);
		m_material->setRenderStage("Transparent");
		m_material->onLoaded();

		m_lineBatch = EchoNew(Batch(m_material, this));
		m_lineBatch->m_mesh->setTopologyType(RenderInput::TT_LINELIST);
	}

	void Gizmos::bindMethods()
	{
	}

	// draw line
	void Gizmos::drawLine(const Vector3& from, const Vector3& to, const Color& color)
	{
		m_lineBatch->addIndex((Word)m_lineBatch->m_vertexs.size());
		m_lineBatch->addIndex((Word)m_lineBatch->m_vertexs.size() + 1);

		m_lineBatch->addVertex(VertexFormat(from));
		m_lineBatch->addVertex(VertexFormat(to));
	}

	// clear mesh data
	void Gizmos::clear()
	{
		m_lineBatch->clear();
	}

	// get global uniforms
	void* Gizmos::getGlobalUniformValue(const String& name)
	{
		if (name == "u_VPMatrix")
			return (void*)(&NodeTree::instance()->get3dCamera()->getViewProjMatrix());

		return nullptr;
	}

	// update
	void Gizmos::update()
	{
		m_lineBatch->update();
	}
}