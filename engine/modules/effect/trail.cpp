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
		EchoSafeRelease(m_renderable);
		m_mesh.reset();
	}

	void Trail::bindMethods()
	{
		CLASS_BIND_METHOD(Trail, getWidth, DEF_METHOD("getWidth"));
		CLASS_BIND_METHOD(Trail, setWidth, DEF_METHOD("setWidth"));
		CLASS_BIND_METHOD(Trail, getHeight, DEF_METHOD("getHeight"));
		CLASS_BIND_METHOD(Trail, setHeight, DEF_METHOD("setHeight"));
		CLASS_BIND_METHOD(Trail, getMaterial, DEF_METHOD("getMaterial"));
		CLASS_BIND_METHOD(Trail, setMaterial, DEF_METHOD("setMaterial"));

		CLASS_REGISTER_PROPERTY(Trail, "Width", Variant::Type::Int, "getWidth", "setWidth");
		CLASS_REGISTER_PROPERTY(Trail, "Height", Variant::Type::Int, "getHeight", "setHeight");
		CLASS_REGISTER_PROPERTY(Trail, "Material", Variant::Type::Object, "getMaterial", "setMaterial");
		CLASS_REGISTER_PROPERTY_HINT(Trail, "Material", PropertyHintType::ResourceType, "Material");
	}

	void Trail::setWidth(i32 width)
	{
		if (m_width != width)
		{
			m_width = width;
			m_isRenderableDirty = true;
		}
	}

	void Trail::setHeight(i32 height)
	{
		if (m_height != height)
		{
			m_height = height;
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
			if (m_width && m_height)
				m_mesh = Mesh::create(true, true);
		}

		if (m_mesh)
		{
			// indices
			IndiceArray indices = { 0, 1, 2, 0, 2, 3 };

			float hw = m_width * 0.5f;
			float hh = m_height * 0.5f;

			// vertices
			VertexArray vertices;
			vertices.emplace_back(Vector3(-hw, -hh, 0.f), Vector2(0.f, 1.f));
			vertices.emplace_back(Vector3(-hw, hh, 0.f), Vector2(0.f, 0.f));
			vertices.emplace_back(Vector3(hw, hh, 0.f), Vector2(1.f, 0.f));
			vertices.emplace_back(Vector3(hw, -hh, 0.f), Vector2(1.f, 1.f));

			// format
			MeshVertexFormat define;
			define.m_isUseUV = true;

			m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
			m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());

			m_localAABB = m_mesh->getLocalBox();
		}
	}
}
