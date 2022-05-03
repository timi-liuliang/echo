#include "opendrive_dynamic_mesh.h"
#include "engine/core/log/log.h"
#include "engine/core/scene/node_tree.h"
#include "base/renderer.h"
#include "base/shader/shader_program.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	OpenDriveDynamicMesh::OpenDriveDynamicMesh()
		: Render()
	{
		setRenderType("3d");
	}

	OpenDriveDynamicMesh::~OpenDriveDynamicMesh()
	{
		reset();
	}

	void OpenDriveDynamicMesh::bindMethods()
	{
		CLASS_BIND_METHOD(OpenDriveDynamicMesh, getStepLength);
		CLASS_BIND_METHOD(OpenDriveDynamicMesh, setStepLength);
		CLASS_BIND_METHOD(OpenDriveDynamicMesh, getMaterial);
		CLASS_BIND_METHOD(OpenDriveDynamicMesh, setMaterial);

		CLASS_REGISTER_PROPERTY(OpenDriveDynamicMesh, "StepLength", Variant::Type::Real, getStepLength, setStepLength);
		CLASS_REGISTER_PROPERTY(OpenDriveDynamicMesh, "Material", Variant::Type::Object, getMaterial, setMaterial);
		CLASS_REGISTER_PROPERTY_HINT(OpenDriveDynamicMesh, "Material", PropertyHintType::ObjectType, "Material");
	}

	void OpenDriveDynamicMesh::reset()
	{
		m_controlPoints.clear();
		m_renderable.reset();
		m_mesh.reset();
	}

	void OpenDriveDynamicMesh::add(const Vector3& position, const Vector3& forward, const Vector3& up, float width, const Color& color, bool separator)
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

	void OpenDriveDynamicMesh::setStepLength(float stepLength)
	{
		if (m_stepLength != stepLength)
		{
			m_stepLength = stepLength;
			m_isRenderableDirty = true;
		}
	}

	void OpenDriveDynamicMesh::setMaterial(Object* material)
	{
		m_material = (Material*)material;

		m_isRenderableDirty = true;
	}

	void OpenDriveDynamicMesh::buildRenderable()
	{
		if (m_isRenderableDirty)
		{
			if (!m_material)
			{
				StringArray macros = { "HAS_NORMALS" };
				ShaderProgramPtr shader = ShaderProgram::getDefault3D(macros);

				m_material = ECHO_CREATE_RES(Material);
				m_material->setShaderPath(shader->getPath());
			}

			// mesh
			updateMeshBuffer();

			// create render able
			m_renderable = RenderProxy::create(m_mesh, m_material, this, true);
			m_renderable->setCastShadow(true);

			m_isRenderableDirty = false;
		}
	}

	void OpenDriveDynamicMesh::updateInternal(float elapsedTime)
	{
		if (isNeedRender())
			buildRenderable();

		if (m_renderable)
			m_renderable->setSubmitToRenderQueue(isNeedRender());
	}

	void OpenDriveDynamicMesh::updateMeshBuffer()
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
				v0.m_normal = controlPoint.m_up;
				//v0.m_color = controlPoint.Color;
				v0.m_uv = Vector2(0.f, controlPoint.m_length / controlPoint.m_width) * m_uvScale;

				VertexFormat v1;
				v1.m_position = controlPoint.m_position - halfRightDir;
				v1.m_normal = controlPoint.m_up;
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
			define.m_isUseNormal = true;
			define.m_isUseUV = true;

			m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(ui32), indices.data());
			m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());

			m_localAABB = m_mesh->getLocalBox();
		}
	}

	void* OpenDriveDynamicMesh::getGlobalUniformValue(const String& name)
	{
		if (name == "u_WorldMatrix")
			return (void*)(&Matrix4::IDENTITY);

		return Render::getGlobalUniformValue(name);
	}
}
