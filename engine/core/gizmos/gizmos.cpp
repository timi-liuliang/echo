#include "Gizmos.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/render/base/renderer.h"
#include "engine/core/geom/Box3.h"

// material for vulkan or metal or opengles
static const char* g_gizmoVsCode = R"(#version 450

// uniforms
layout(binding = 0) uniform UBO
{
    mat4 u_WorldMatrix;
    mat4 u_ViewProjMatrix;
} vs_ubo;

// inputs
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_UV;

// outputs
layout(location = 0) out vec3 v_Position;
layout(location = 1) out vec4 v_Color;
layout(location = 2) out vec2 v_UV;

void main(void)
{
    vec4 position = vs_ubo.u_WorldMatrix * vec4(a_Position, 1.0);
    
    v_Position  = position.xyz;
    gl_Position = vs_ubo.u_ViewProjMatrix * position;
    
    v_Color = a_Color;
	v_UV = a_UV;
}
)";

static const char* g_gizmoPsCode = R"(#version 450

// uniforms
layout(binding = 1) uniform UBO
{
    vec3  u_CameraPosition;
    float u_CameraFar;
} fs_ubo;

// uniforms
#ifdef ENABLE_ALBEDO_TEXTURE
layout(set=1, binding = 3) uniform sampler2D BaseColor;
#endif

// inputs
layout(location = 0) in vec3  v_Position;
layout(location = 1) in vec4  v_Color;
layout(location = 2) in vec2  v_UV;

// outputs
layout(location = 0) out vec4 o_FragColor;

void main(void)
{
	vec4 finalColor = v_Color;

#ifdef ENABLE_ALBEDO_TEXTURE
    finalColor = texture(BaseColor, v_UV) * finalColor;
#endif

    o_FragColor = finalColor;
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
		m_renderable.reset();
		m_mesh.reset();
	}

	void Gizmos::Batch::addVertex(const Gizmos::VertexFormat& vert)
	{
		m_vertexs.push_back(vert);
		m_meshDirty = true;
	}

	void Gizmos::Batch::addIndex(ui32 idx)
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
		if (m_meshDirty)
		{
			MeshVertexFormat define;
			define.m_isUseVertexColor = true;
			define.m_isUseUV = true;

			m_mesh->updateIndices((ui32)m_indices.size(), sizeof(ui32), m_indices.data());
			m_mesh->updateVertexs(define, (ui32)m_vertexs.size(), (const Byte*)m_vertexs.data());

			m_meshDirty = false;
		}

		if (m_albedo && m_material->isMacroUsed("ENABLE_ALBEDO_TEXTURE"))
		{
			m_material->setUniformTexture("BaseColor", m_albedo);
		}

		// render
		if(!m_renderable && m_vertexs.size())
		{
			m_renderable = RenderProxy::create(m_mesh, m_material, m_gizmos, false);
		}

		if (m_renderable)
		{
			m_renderable->setSubmitToRenderQueue(m_gizmos->isNeedRender());
		}

		// auto clear data
		if (m_gizmos->isAutoClear())
		{
			clear();
		}
	}

	Gizmos::Gizmos()
	{
        m_shader = initDefaultShader(false);
		m_shaderSprite = initDefaultShader(true);
        
		m_material = ECHO_CREATE_RES(Material);
		m_material->setShaderPath( m_shader->getPath());

		m_rasterizerState = Renderer::instance()->createRasterizerState();
		m_rasterizerState->setCullMode(RasterizerState::CULL_NONE);
		m_material->setRasterizerState(m_rasterizerState);

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
		EchoSafeDeleteContainer(m_spriteBatchs, Batch);
	}

	void Gizmos::bindMethods()
	{
	}

    ShaderProgramPtr Gizmos::initDefaultShader(bool enableAlbedoTexture)
    {
        ResourcePath shaderVirtualPath = ResourcePath(enableAlbedoTexture ? "echo_gizmo_default_shader_sprite" : "echo_gizmo_default_shader");
        ShaderProgramPtr shader = ECHO_DOWN_CAST<ShaderProgram*>(ShaderProgram::get(shaderVirtualPath));
        if(!shader)
        {
			shader = ECHO_CREATE_RES(ShaderProgram);

			// macros
			if (enableAlbedoTexture)
				shader->setMacros({ "ENABLE_ALBEDO_TEXTURE" });

			// render state
			shader->setBlendMode("Transparent");
			shader->setCullMode("CULL_NONE");

			// set code
			shader->setPath(shaderVirtualPath.getPath());
			shader->setType("glsl");
			shader->setVsCode(g_gizmoVsCode);
			shader->setPsCode(g_gizmoPsCode);
        }

		return shader;
    }

	void Gizmos::adjustPointSize(float& radius, const Vector3& position, int flags)
	{
		Camera* camera = getCamera();
		if (camera)
		{
			if (camera->getProjectionMode() == Camera::ProjMode::PM_PERSPECTIVE && (flags & RenderFlags::FixedPixel))
			{
				float halfHeight = camera->getHeight() * 0.5f;
				float ratio = radius / halfHeight;
				float halfNearPlaneWidth = camera->getNear() * tan(camera->getFov() * 0.5f);
				float pointDistance = (camera->getPosition() - position).len();

				radius = (ratio * halfNearPlaneWidth) * (pointDistance / camera->getNear());
			}
		}
	}

	void Gizmos::drawPoint(const Vector3& position, const Color& color, float pixels, int segments, int flags)
	{
		Camera* camera = getCamera();
		if (camera)
		{
			float radius = pixels / 2.f;
			adjustPointSize(radius, position, flags);

			float deltaDegree = 2.f * Math::PI / segments;
			for (int i = 0; i < segments; i++)
			{
				Vector3 v0 = position - camera->getForward() * radius;
				Vector3 v1 = v0 + Quaternion::fromAxisAngle(camera->getForward(), i * deltaDegree + Math::PI_DIV4).rotateVec3(camera->getRight()) * radius;
				Vector3 v2 = v0 + Quaternion::fromAxisAngle(camera->getForward(), (i + 1) * deltaDegree + Math::PI_DIV4).rotateVec3(camera->getRight()) * radius;

				drawTriangle(v0, v1, v2, color);
			}
		}
	}

	void Gizmos::drawLine(const Vector3& from, const Vector3& to, const Color& color)
	{
		m_lineBatch->addIndex((ui32)m_lineBatch->m_vertexs.size());
		m_lineBatch->addIndex((ui32)m_lineBatch->m_vertexs.size() + 1);

		m_lineBatch->addVertex(VertexFormat(from, color));
		m_lineBatch->addVertex(VertexFormat(to, color));

		m_localAABB.addPoint(from);
		m_localAABB.addPoint(to);
	}

	void Gizmos::setLineWidth(float lineWidth)
	{
		if (m_rasterizerState)
		{
			m_rasterizerState->setLineWidth(lineWidth);
		}
	}

	float Gizmos::getLineWidth() const
	{
		return m_rasterizerState ? m_rasterizerState->getLineWidth() : 1.f;
	}

	void Gizmos::drawTriangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, const Color& color)
	{
		m_triangleBatch->addIndex((ui32)m_triangleBatch->m_vertexs.size());
		m_triangleBatch->addIndex((ui32)m_triangleBatch->m_vertexs.size() + 1);
		m_triangleBatch->addIndex((ui32)m_triangleBatch->m_vertexs.size() + 2);

		m_triangleBatch->addVertex(VertexFormat(v0, color));
		m_triangleBatch->addVertex(VertexFormat(v1, color));
		m_triangleBatch->addVertex(VertexFormat(v2, color));

		m_localAABB.addPoint(v0);
		m_localAABB.addPoint(v1);
		m_localAABB.addPoint(v2);
	}

	void Gizmos::drawCircle(const Vector3& center, const Vector3& up, float radius, const Color& color)
	{
		float segments = 36.f;

		float deltaDegree = 2.f * Math::PI / segments;
		for (int i = 0; i < segments; i++)
		{
			Vector3 v1 = Quaternion::fromAxisAngle(up, i * deltaDegree + Math::PI_DIV4).rotateVec3(Vector3::UNIT_X) * radius;
			Vector3 v2 = Quaternion::fromAxisAngle(up, (i + 1) * deltaDegree + Math::PI_DIV4).rotateVec3(Vector3::UNIT_X) * radius;

			drawLine(v1 + center, v2 + center, color);
		}
	}

	void Gizmos::drawCylinder(const Vector3& center, const Vector3& up, float height, float radius, const Color& color, i32 segments, bool isLinkCenter)
	{
		Vector3 bottomCenter = center - up * height * 0.5;
		Vector3 topCenter = center + up * height * 0.5;
		float   deltaDegree = 2.f * Math::PI / segments;

		for (int i = 0; i < segments; i++)
		{
			Vector3 dir1 = Quaternion::fromAxisAngle(up, i * deltaDegree + Math::PI_DIV4).rotateVec3(Vector3::UNIT_X) * radius;
			Vector3 dir2 = Quaternion::fromAxisAngle(up, (i + 1) * deltaDegree + Math::PI_DIV4).rotateVec3(Vector3::UNIT_X) * radius;

			Vector3 v1 = bottomCenter + dir1;
			Vector3 v2 = bottomCenter + dir2;
			Vector3 v3 = topCenter + dir1;
			Vector3 v4 = topCenter + dir2;

			drawLine(v1, v2, color);
			drawLine(v3, v4, color);
			drawLine(v1, v3, color);
			drawLine(v2, v4, color);

			if (isLinkCenter)
			{
				drawLine(v1, bottomCenter, color);
				drawLine(v3, topCenter, color);
			}
		}
	}

	void Gizmos::drawOBB(const Vector3& dims, const Matrix4& transform, const Color& color)
	{
		array<Vector3, 8> eightPoints;

		Box3 box(Vector3::ZERO, Vector3::UNIT_X, Vector3::UNIT_Y, Vector3::UNIT_Z, dims.x * 0.5f, dims.y * 0.5f, dims.z * 0.5f);
		box.buildEightPoints(eightPoints.data());

		for (Vector3& point : eightPoints)
		{
			point = transform.transform(point);
		}

		drawLine(eightPoints[0], eightPoints[1], color);
		drawLine(eightPoints[1], eightPoints[2], color);
		drawLine(eightPoints[2], eightPoints[3], color);
		drawLine(eightPoints[3], eightPoints[0], color);

		drawLine(eightPoints[4], eightPoints[5], color);
		drawLine(eightPoints[5], eightPoints[6], color);
		drawLine(eightPoints[6], eightPoints[7], color);
		drawLine(eightPoints[7], eightPoints[4], color);

		drawLine(eightPoints[0], eightPoints[4], color);
		drawLine(eightPoints[1], eightPoints[5], color);
		drawLine(eightPoints[2], eightPoints[6], color);
		drawLine(eightPoints[3], eightPoints[7], color);
	}

	void Gizmos::drawSprite(const Vector3& position, const Color& color, float pixels, TexturePtr texture, int flags)
	{
		Camera* camera = getCamera();
		if (camera)
		{
			float radius = pixels / 2.f;
			adjustPointSize(radius, position, flags);

			Vector3 center = position;
			Vector3 v0 = center + Quaternion::fromAxisAngle(camera->getForward(), Math::PI_DIV2 * 0.f + Math::PI_DIV4).rotateVec3(camera->getRight()) * radius;
			Vector3 v1 = center + Quaternion::fromAxisAngle(camera->getForward(), Math::PI_DIV2 * 1.f + Math::PI_DIV4).rotateVec3(camera->getRight()) * radius;
			Vector3 v2 = center + Quaternion::fromAxisAngle(camera->getForward(), Math::PI_DIV2 * 2.f + Math::PI_DIV4).rotateVec3(camera->getRight()) * radius;
			Vector3 v3 = center + Quaternion::fromAxisAngle(camera->getForward(), Math::PI_DIV2 * 3.f + Math::PI_DIV4).rotateVec3(camera->getRight()) * radius;

			Batch* spriteBatch = getSpriteBatch(texture);
			if (spriteBatch)
			{
				spriteBatch->addIndex((ui32)spriteBatch->m_vertexs.size() + 0);
				spriteBatch->addIndex((ui32)spriteBatch->m_vertexs.size() + 1);
				spriteBatch->addIndex((ui32)spriteBatch->m_vertexs.size() + 2);
				spriteBatch->addIndex((ui32)spriteBatch->m_vertexs.size() + 0);
				spriteBatch->addIndex((ui32)spriteBatch->m_vertexs.size() + 2);
				spriteBatch->addIndex((ui32)spriteBatch->m_vertexs.size() + 3);

				spriteBatch->addVertex(VertexFormat(v0, color, Vector2(1.f, 1.f)));
				spriteBatch->addVertex(VertexFormat(v1, color, Vector2(0.f, 1.f)));
				spriteBatch->addVertex(VertexFormat(v2, color, Vector2(0.f, 0.f)));
				spriteBatch->addVertex(VertexFormat(v3, color, Vector2(1.f, 0.f)));
			}

			m_localAABB.addPoint(v0);
			m_localAABB.addPoint(v1);
			m_localAABB.addPoint(v2);
		}
	}

	Gizmos::Batch* Gizmos::getSpriteBatch(TexturePtr texture)
	{
		if (texture)
		{
			for (Batch* batch : m_spriteBatchs)
			{
				if (batch->m_albedo == texture)
					return batch;
			}

			Material* material = ECHO_CREATE_RES(Material);
			material->setMacro("ENABLE_ALBEDO_TEXTURE", true);
			material->setShaderPath(m_shaderSprite->getPath());

			Batch* batch = EchoNew(Batch(material, this));
			batch->m_mesh->setTopologyType(Mesh::TT_TRIANGLELIST);
			batch->m_albedo = texture;
			m_spriteBatchs.insert(batch);

			return batch;
		}

		return nullptr;
	}

	void Gizmos::clear()
	{
		m_localAABB.reset();
		m_lineBatch->clear();
		m_triangleBatch->clear();

		for (Batch* batch : m_spriteBatchs)
			batch->clear();
	}

	void Gizmos::updateInternal(float elapsedTime)
	{
		m_lineBatch->update();
		m_triangleBatch->update();
			
		for (Batch* batch : m_spriteBatchs)
			batch->update();
	}
}
