#include "Gizmos.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/render/base/renderer.h"

// material for vulkan or metal or opengles
static const char* g_gizmoVsCode = R"(
#version 450
// uniforms
layout(binding = 0) uniform UBO
{
    mat4 u_WorldMatrix;
    mat4 u_ViewProjMatrix;
} vs_ubo;

// inputs
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

// outputs
layout(location = 0) out vec3 v_Position;
layout(location = 1) out vec4 v_Color;

void main(void)
{
    vec4 position = vs_ubo.u_WorldMatrix * vec4(a_Position, 1.0);
    
    v_Position  = position.xyz;
    gl_Position = vs_ubo.u_ViewProjMatrix * position;
    
    v_Color = a_Color;
}
)";

static const char* g_gizmoPsCode = R"(
#version 450

precision mediump float;

// uniforms
layout(binding = 0) uniform UBO
{
    vec3  u_CameraPosition;
    float u_CameraFar;
} fs_ubo;


// inputs
layout(location = 0) in vec3  v_Position;
layout(location = 1) in vec4  v_Color;

// outputs
layout(location = 0) out vec4 o_FragColor;

void main(void)
{
    o_FragColor = v_Color;
}
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

		m_mesh.reset();
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
			m_mesh->updateVertexs(define, (ui32)m_vertexs.size(), (const Byte*)m_vertexs.data());

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

	Gizmos::Gizmos()
	{
        initDefaultShader();
        
		m_material = ECHO_CREATE_RES(Material);
		m_material->setShaderPath( m_shader->getPath());

		m_lineBatch = EchoNew(Batch(m_material, this));
		m_lineBatch->m_mesh->setTopologyType(Mesh::TT_LINELIST);

		m_triangleBatch = EchoNew(Batch(m_material, this));
		m_triangleBatch->m_mesh->setTopologyType(Mesh::TT_TRIANGLELIST);
	}

	Gizmos::~Gizmos()
	{
		m_material.reset();

		EchoSafeDelete(m_lineBatch, Batch);
		EchoSafeDelete(m_triangleBatch, Batch);
	}

	void Gizmos::bindMethods()
	{
	}

    void Gizmos::initDefaultShader()
    {
        ResourcePath shaderVirtualPath = ResourcePath("echo_gizmo_default_shader");
        m_shader = ECHO_DOWN_CAST<ShaderProgram*>(ShaderProgram::get(shaderVirtualPath));
        if(!m_shader)
        {
            m_shader = ECHO_CREATE_RES(ShaderProgram);

			// render state
			m_shader->setBlendMode("Transparent");
			m_shader->setCullMode("CULL_NONE");

			// set code
            m_shader->setPath(shaderVirtualPath.getPath());
            m_shader->setType("glsl");
            m_shader->setVsCode(g_gizmoVsCode);
            m_shader->setPsCode(g_gizmoPsCode);
        }
    }

	void Gizmos::drawPoint(const Vector3& from, const Color& color, float radius)
	{
		Vector3 v0 = from + Vector3(0.f,   0.f, radius);
		Vector3 v1 = from + Vector3(radius,  0.f, -radius);
		Vector3 v2 = from + Vector3(-radius, 0.f, -radius);

		drawTriangle(v0, v1, v2, color);
	}

	void Gizmos::drawLine(const Vector3& from, const Vector3& to, const Color& color)
	{
		m_lineBatch->addIndex((Word)m_lineBatch->m_vertexs.size());
		m_lineBatch->addIndex((Word)m_lineBatch->m_vertexs.size() + 1);

		m_lineBatch->addVertex(VertexFormat(from, color));
		m_lineBatch->addVertex(VertexFormat(to, color));

		m_localAABB.addPoint(from);
		m_localAABB.addPoint(to);
	}

	void Gizmos::drawTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Color& color)
	{
		m_triangleBatch->addIndex((Word)m_triangleBatch->m_vertexs.size());
		m_triangleBatch->addIndex((Word)m_triangleBatch->m_vertexs.size() + 1);
		m_triangleBatch->addIndex((Word)m_triangleBatch->m_vertexs.size() + 2);

		m_triangleBatch->addVertex(VertexFormat(v0, color));
		m_triangleBatch->addVertex(VertexFormat(v1, color));
		m_triangleBatch->addVertex(VertexFormat(v2, color));

		m_localAABB.addPoint(v0);
		m_localAABB.addPoint(v1);
		m_localAABB.addPoint(v2);
	}

	void Gizmos::drawSprite(const Vector3& position, TexturePtr texture)
	{

	}

	void Gizmos::mergeVertices()
	{

	}

	void Gizmos::clear()
	{
		m_localAABB.reset();
		m_lineBatch->clear();
		m_triangleBatch->clear();
	}

	void Gizmos::update_self()
	{
		if (isNeedRender())
		{
			m_lineBatch->update();
			m_triangleBatch->update();
		}
	}
}
