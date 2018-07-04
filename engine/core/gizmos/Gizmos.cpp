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
	// constructor
	Gizmos::Gizmos()
		: m_meshOpaque(nullptr)
		, m_renderableOpaque(nullptr)
	{
		m_materialOpaque = ECHO_CREATE_RES(Material);
		m_materialOpaque->setShaderContent(g_gizmoOpaqueMaterial);
		m_materialOpaque->setRenderStage("Transparent");
		m_materialOpaque->onLoaded();

		m_meshOpaque = Mesh::create(true, true);
	}

	void Gizmos::bindMethods()
	{
	}

	// draw line
	void Gizmos::drawLine(const Vector3& from, const Vector3& to, const Color& color, bool transparent, float thickNess)
	{
		m_indicesOpaque.push_back(m_vertexsOpaque.size());
		m_indicesOpaque.push_back(m_vertexsOpaque.size() + 1);
		m_indicesOpaque.push_back(m_vertexsOpaque.size() + 2);
		m_indicesOpaque.push_back(m_vertexsOpaque.size() + 0);
		m_indicesOpaque.push_back(m_vertexsOpaque.size() + 2);
		m_indicesOpaque.push_back(m_vertexsOpaque.size() + 3);

		m_vertexsOpaque.push_back(VertexFormat(from + Vector3::NEG_UNIT_Z * thickNess));
		m_vertexsOpaque.push_back(VertexFormat(from - Vector3::NEG_UNIT_Z * thickNess));
		m_vertexsOpaque.push_back(VertexFormat(to - Vector3::NEG_UNIT_Z * thickNess));
		m_vertexsOpaque.push_back(VertexFormat(to + Vector3::NEG_UNIT_Z * thickNess));
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
		if (m_vertexsOpaque.size())
		{
			// safe release renderable
			if (m_renderableOpaque)
			{
				m_renderableOpaque->release();
				m_renderableOpaque = nullptr;
			}

			MeshVertexFormat define;

			m_meshOpaque->updateIndices(m_indicesOpaque.size(), m_indicesOpaque.data());
			m_meshOpaque->updateVertexs(define, m_vertexsOpaque.size(), (const Byte*)m_vertexsOpaque.data(), m_aabb);

			m_renderableOpaque = Renderable::create(m_meshOpaque, m_materialOpaque, this);

			m_renderableOpaque->submitToRenderQueue();

			m_vertexsOpaque.clear();
			m_indicesOpaque.clear();
		}
	}
}