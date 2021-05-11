#include "trail.h"
#include "engine/core/log/log.h"
#include "engine/core/scene/node_tree.h"
#include "base/renderer.h"
#include "base/shader_program.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	Trail::Trail()
		: Render()
	{
	}

	Trail::~Trail()
	{
		reset();
	}

	void Trail::bindMethods()
	{
		CLASS_BIND_METHOD(Trail, getStepLength, DEF_METHOD("getStepLength"));
		CLASS_BIND_METHOD(Trail, setStepLength, DEF_METHOD("setStepLength"));
		CLASS_BIND_METHOD(Trail, getMaterial, DEF_METHOD("getMaterial"));
		CLASS_BIND_METHOD(Trail, setMaterial, DEF_METHOD("setMaterial"));

		CLASS_REGISTER_PROPERTY(Trail, "StepLength", Variant::Type::Int, "getStepLength", "setStepLength");
		CLASS_REGISTER_PROPERTY(Trail, "Material", Variant::Type::Object, "getMaterial", "setMaterial");
		CLASS_REGISTER_PROPERTY_HINT(Trail, "Material", PropertyHintType::ResourceType, "Material");
	}

	void Trail::reset()
	{
		m_controlPoints.clear();
		EchoSafeRelease(m_renderable);
		m_mesh.reset();
	}

	void Trail::add(const Vector3& position, const Vector3& forward, const Vector3& up, float width, const Color& color, bool separator)
	{
		ControlPoint controlPoint;
		controlPoint.m_position = position;
		controlPoint.m_forward = forward;
		controlPoint.m_up = up;
		controlPoint.m_width = width;
		controlPoint.m_color = color;
		controlPoint.m_separator = separator;

		m_controlPoints.push_back(controlPoint);

		m_isRenderableDirty = true;
	}

	void Trail::setStepLength(float stepLength)
	{
		if (m_stepLength != stepLength)
		{
			m_stepLength = stepLength;
			m_isRenderableDirty = true;
		}
	}

	void Trail::setMaterial(Object* material)
	{
		m_material = (Material*)material;

		m_isRenderableDirty = true;
	}

	void Trail::buildRenderable()
	{
		if (m_isRenderableDirty)
		{
			EchoSafeRelease(m_renderable);

			if (!m_material)
			{
				StringArray macros = { "ALPHA_ADJUST" };
				ShaderProgramPtr shader = ShaderProgram::getDefault2D(macros);

				m_material = ECHO_CREATE_RES(Material);
				m_material->setShaderPath(shader->getPath());
			}

			// mesh
			updateMeshBuffer();

			// create render able
			m_renderable = RenderProxy::create(m_mesh, m_material, this);

			m_isRenderableDirty = false;
		}
	}

	void Trail::update_self()
	{
		// update control points
		if (m_controlPoints.empty())
		{
			add(getWorldPosition(), Vector3::UNIT_X, Vector3::UNIT_Z, 30.f, Color::RED, false);
		}
		else
		{
			Vector3 dir = getWorldPosition() - m_controlPoints.back().m_position;
			if (dir.len() > m_stepLength)
			{
				add(getWorldPosition(), dir.normalize(), Vector3::UNIT_Z, 30.f, Color::RED);
			}
		}

		// update render
		if (isNeedRender())
		{
			buildRenderable();
			if (m_renderable)
			{
				m_renderable->submitToRenderQueue();
			}
		}
	}

	void Trail::updateMeshBuffer()
	{
		if (!m_mesh)
		{
			if (m_controlPoints.size()>1)
				m_mesh = Mesh::create(true, true);
		}

		if (m_mesh)
		{
			IndiceArray indices;
			VertexArray vertices;

			float length = 0.f;
			for (size_t i = 0; i < m_controlPoints.size(); i++)
			{
				const ControlPoint& controlPoint = m_controlPoints[i];
				Vector3 halfRightDir = controlPoint.m_forward.cross(controlPoint.m_up) * controlPoint.m_width * 0.5f;

				// Calculate Length
				length += i == 0 ? 0.f : (controlPoint.m_position - m_controlPoints[i - 1].m_position).len();

				// Update Vertices
				VertexFormat v0;
				v0.m_position = controlPoint.m_position + halfRightDir;
				//v0.m_color = controlPoint.Color;
				v0.m_uv = Vector2(0.f, length / controlPoint.m_width) * m_uvScale;

				VertexFormat v1;
				v1.m_position = controlPoint.m_position - halfRightDir;
				//v1.Color = controlPoint.Color;
				v1.m_uv = Vector2(1.f, length / controlPoint.m_width) * m_uvScale;

				vertices.push_back(v0);
				vertices.push_back(v1);

				// Update Indices
				if (i != 0 && !controlPoint.m_separator)
				{
					i32 base = vertices.size() - 4;

					indices.push_back(base);
					indices.push_back(base + 3);
					indices.push_back(base + 2);

					indices.push_back(base);
					indices.push_back(base + 1);
					indices.push_back(base + 3);
				}
			}

			// format
			MeshVertexFormat define;
			define.m_isUseUV = true;

			m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
			m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());

			m_localAABB = m_mesh->getLocalBox();
		}
	}

	void* Trail::getGlobalUniformValue(const String& name)
	{
		if (name == "u_WorldMatrix")
			return (void*)(&Matrix4::IDENTITY);

		return Render::getGlobalUniformValue(name);
	}
}
