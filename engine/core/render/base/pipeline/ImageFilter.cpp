#include "engine/core/render/base/mesh/Mesh.h"
#include "engine/core/render/base/Renderer.h"
#include "ImageFilter.h"

namespace Echo
{
	ImageFilter::ImageFilter(RenderPipeline* pipeline, RenderStage* stage)
		: IRenderQueue(pipeline, stage)
	{
	}

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
	}

	void ImageFilter::render()
	{
		Renderer* render = Renderer::instance();
		if (render && m_material)
		{

		}
	}

	void ImageFilter::buildRenderable()
	{
		if (m_material)
		{

		}
	}

	void ImageFilter::buildMeshData(VertexArray& oVertices, IndiceArray& oIndices)
	{
		oIndices = { 0, 1, 2, 0, 2, 3 };

		float hw = -1.f;
		float hh =  1.f;

		// vertices
		VertexArray vertices;
		vertices.push_back(VertexFormat(Vector3(-hw, -hh, 0.f), Vector2(0.f, 1.f)));
		vertices.push_back(VertexFormat(Vector3(-hw, hh, 0.f), Vector2(0.f, 0.f)));
		vertices.push_back(VertexFormat(Vector3(hw, hh, 0.f), Vector2(1.f, 0.f)));
		vertices.push_back(VertexFormat(Vector3(hw, -hh, 0.f), Vector2(1.f, 1.f)));
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

		m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(ui32), indices.data());
		m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());
	}
}