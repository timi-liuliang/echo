#include "direct_lighting.h"
#include "core/render/base/renderer.h"
#include "modules/light/light/direction_light.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	DirectLighting::DirectLighting()
		: DeferredLighting()
	{
	}

	DirectLighting::~DirectLighting()
	{

	}

	void DirectLighting::bindMethods()
	{

	}

	void DirectLighting::render(FrameBufferPtr& frameBuffer)
	{
		onRenderBegin();

		Renderer* render = Renderer::instance();
		if (render)
		{
			if (buildRenderable())
				render->draw(m_renderable, frameBuffer);
		}

		onRenderEnd();
	}

	void DirectLighting::clearRenderable()
	{
		m_renderable.reset();
		m_mesh.reset();
	}

	bool DirectLighting::buildRenderable()
	{
		if (m_dirty && m_material)
		{
			clearRenderable();

			updateMeshBuffer();

			// create render able
			m_renderable = RenderProxy::create(m_mesh, m_material, nullptr, false);
			m_renderable->setSubmitToRenderQueue(false);

			m_dirty = false;
		}
		else
		{
			updateMeshBuffer();
		}

		return m_renderable ? true : false;
	}

	void DirectLighting::buildMeshData(VertexArray& oVertices, IndiceArray& oIndices)
	{
		vector<Light*>::type dirLights = Light::gatherLights(Light::Type::Direction);
		for(size_t i=0; i<dirLights.size(); i++)
		{
			DirectionLight* dirLight = ECHO_DOWN_CAST<DirectionLight*>(dirLights[i]);

			float hw = -1.f;
			float hh = 1.f;

			Vector3 lightDir = dirLight->getDirection();
			Dword lightColor = dirLight->getColor();

			// vertices
			oVertices.emplace_back(Vector3(-hw, -hh, 0.f), lightDir, lightColor);
			oVertices.emplace_back(Vector3(hw, -hh, 0.f), lightDir, lightColor);
			oVertices.emplace_back(Vector3(hw, hh, 0.f), lightDir, lightColor);
			oVertices.emplace_back(Vector3(-hw, hh, 0.f), lightDir, lightColor);

			// https://www.khronos.org/opengl/wiki/Face_Culling
			// On a freshly created OpenGL Context, the default front face is Counter-Clockwise(CL_CCW)
			i32 base = i*4;
			oIndices.emplace_back(base);
			oIndices.emplace_back(base+1);
			oIndices.emplace_back(base+2);
			oIndices.emplace_back(base);
			oIndices.emplace_back(base+2);
			oIndices.emplace_back(base+3);
		}
	}

	void DirectLighting::updateMeshBuffer()
	{
		// create mesh
		if (!m_mesh) m_mesh = Mesh::create(true, true);

		// update data
		VertexArray    vertices;
		IndiceArray    indices;
		buildMeshData(vertices, indices);

		MeshVertexFormat define;
		define.m_isUseNormal = true;
		define.m_isUseVertexColor = true;

		m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(ui16), indices.data());
		m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());
	}
}