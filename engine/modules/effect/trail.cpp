#include "trail.h"
#include "engine/core/log/log.h"
#include "engine/core/scene/node_tree.h"
#include "base/renderer.h"
#include "base/shader/shader_program.h"
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
		CLASS_BIND_METHOD(Trail, getFadeTime);
		CLASS_BIND_METHOD(Trail, setFadeTime);
		CLASS_BIND_METHOD(Trail, getStepLength);
		CLASS_BIND_METHOD(Trail, setStepLength);
		CLASS_BIND_METHOD(Trail, getMaterial);
		CLASS_BIND_METHOD(Trail, setMaterial);

		CLASS_REGISTER_PROPERTY(Trail, "FadeTime", Variant::Type::Real, getFadeTime, setFadeTime);
		CLASS_REGISTER_PROPERTY(Trail, "StepLength", Variant::Type::Real, getStepLength, setStepLength);
		CLASS_REGISTER_PROPERTY(Trail, "Material", Variant::Type::Object, getMaterial, setMaterial);
		CLASS_REGISTER_PROPERTY_HINT(Trail, "Material", PropertyHintType::ObjectType, "Material");
	}

	void Trail::reset()
	{
		m_controlPoints.clear();
		m_renderable.reset();
		m_mesh.reset();
	}

	void Trail::add(const Vector3& position, const Vector3& forward, const Vector3& up, float width, const Color& color, bool separator)
	{
		ControlPoint controlPoint;
		controlPoint.m_position = position;
		controlPoint.m_forward = forward;
		controlPoint.m_up = up;
		controlPoint.m_width = width;
		controlPoint.m_length = m_controlPoints.empty() ? 0.f : m_controlPoints.back().m_length + (m_controlPoints.back().m_position - position).len();
		controlPoint.m_color = color;
		controlPoint.m_separator = separator;

		m_controlPoints.emplace_back(controlPoint);

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

	void Trail::setFadeTime(float fadeTime)
	{
		if (m_fadeTime != fadeTime)
		{
			m_fadeTime = fadeTime;
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
			if (!m_material)
			{
				StringArray macros = { "ALPHA_ADJUST" };
				ShaderProgramPtr shader = ShaderProgram::getDefault2D(macros);

				m_material = ECHO_CREATE_RES(Material);
				m_material->setShaderPath(shader->getPath());
				m_material->setUniformValue("u_Alpha", 1.f);
			}

			// mesh
			updateMeshBuffer();

			// create render able
			m_renderable = RenderProxy::create(m_mesh, m_material, this, false);

			m_isRenderableDirty = false;
		}
	}

	void Trail::updateInternal(float elapsedTime)
	{
		updateControlPoints();

		if (isNeedRender())
			buildRenderable();

		if (m_renderable)
			m_renderable->setEnable(isNeedRender());
	}

	void Trail::updateControlPoints()
	{
		if (m_controlPoints.empty())
		{
			Vector3 up = getWorldOrientation() * Vector3::UNIT_Z;
			add(getWorldPosition(), Vector3::UNIT_X, up, 30.f, Color::RED, false);
		}
		else
		{
			Vector3 dir = getWorldPosition() - m_controlPoints.back().m_position;
			if (dir.len() > m_stepLength)
			{
				Vector3 up = getWorldOrientation() * Vector3::UNIT_Z;
				add(getWorldPosition(), dir.normalize(), up, 30.f, Color::RED);
			}
		}

		// update life
		for (ControlPoint& point : m_controlPoints)
		{
			point.m_life += Engine::instance()->getFrameTime();
			point.m_color.a = 1.f - Math::Clamp(point.m_life, 0.f, m_fadeTime) / m_fadeTime;
		}

		// remove control points
		for (vector<ControlPoint>::type::iterator it = m_controlPoints.begin(); it != m_controlPoints.end();)
		{
			if (it->m_life > m_fadeTime)
				it = m_controlPoints.erase(it);
			else
				it++;
		}

		if (m_mesh)
		{
			updateMeshBuffer();
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

			for (size_t i = 0; i < m_controlPoints.size(); i++)
			{
				const ControlPoint& controlPoint = m_controlPoints[i];

				Vector3 halfRightDir = controlPoint.m_forward.cross(controlPoint.m_up) * controlPoint.m_width * 0.5f;

				// Update Vertices
				VertexFormat v0;
				v0.m_position = controlPoint.m_position + halfRightDir;
				//v0.m_color = controlPoint.Color;
				v0.m_uv = Vector2(0.f, controlPoint.m_length / controlPoint.m_width) * m_uvScale;

				VertexFormat v1;
				v1.m_position = controlPoint.m_position - halfRightDir;
				//v1.Color = controlPoint.Color;
				v1.m_uv = Vector2(1.f, controlPoint.m_length / controlPoint.m_width) * m_uvScale;

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
