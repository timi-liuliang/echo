#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/renderer.h"
#include "image_filter.h"

namespace Echo
{
	ImageFilter::~ImageFilter()
	{
	}

	void ImageFilter::bindMethods()
	{
		CLASS_BIND_METHOD(ImageFilter, getMaterial, DEF_METHOD("getMaterial"));
		CLASS_BIND_METHOD(ImageFilter, setMaterial, DEF_METHOD("setMaterial"));

		CLASS_REGISTER_PROPERTY(ImageFilter, "Material", Variant::Type::Object, "getMaterial", "setMaterial");
		CLASS_REGISTER_PROPERTY_HINT(ImageFilter, "Material", PropertyHintType::ResourceType, "Material");
	}

	void ImageFilter::setMaterial(Object* material)
	{
		m_material = (Material*)material;
		if (m_material)
		{
			setGlobalUniforms();
			m_material->onShaderChanged.connectClassMethod(this, createMethodBind(&ImageFilter::setGlobalUniforms));
		}

		m_isRenderableDirty = true;
	}

	void ImageFilter::setGlobalUniforms()
	{
		if (m_material)
		{
			m_material->setUniformValue("u_WorldMatrix", &Matrix4::IDENTITY);
			m_material->setUniformValue("u_ViewProjMatrix", &Matrix4::IDENTITY);
		}
	}

	void ImageFilter::render(FrameBufferPtr& frameBuffer)
	{
		onRenderBegin();

		Renderer* render = Renderer::instance();
		if (render)
		{
			if(buildRenderable())
				render->draw(m_renderable, frameBuffer);
		}

		onRenderEnd();
	}

	void ImageFilter::clearRenderable()
	{
		EchoSafeRelease(m_renderable);
		m_mesh.reset();
	}

	bool ImageFilter::buildRenderable()
	{
		if (m_isRenderableDirty && m_material)
		{
			clearRenderable();

			updateMeshBuffer();

			// create render able
			m_renderable = RenderProxy::create(m_mesh, m_material, nullptr);

			m_isRenderableDirty = false;
		}

		return m_renderable ? true : false;
	}

	void ImageFilter::buildMeshData(VertexArray& oVertices, IndiceArray& oIndices)
	{
		// https://www.khronos.org/opengl/wiki/Face_Culling
		// On a freshly created OpenGL Context, the default front face is Counter-Clockwise(CL_CCW)
		oIndices = { 0, 1, 2, 0, 2, 3 };

		float hw = -1.f;
		float hh =  1.f;

		// vertices
		oVertices.emplace_back(Vector3(-hw, -hh, 0.f), Vector2(1.f, 0.f));
		oVertices.emplace_back(Vector3(hw, -hh, 0.f), Vector2(0.f, 0.f));
		oVertices.emplace_back(Vector3(hw, hh, 0.f), Vector2(0.f, 1.f));
		oVertices.emplace_back(Vector3(-hw, hh, 0.f), Vector2(1.f, 1.f));
	}

	void ImageFilter::updateMeshBuffer()
	{
		// create mesh
		if (!m_mesh) m_mesh = Mesh::create(true, true);

		// update data
		VertexArray    vertices;
		IndiceArray    indices;
		buildMeshData(vertices, indices);

		MeshVertexFormat define;
		define.m_isUseUV = true;

		m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(ui16), indices.data());
		m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());
	}
}