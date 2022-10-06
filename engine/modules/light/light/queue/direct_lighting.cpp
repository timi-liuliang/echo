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
		if (m_dirty)
		{
			clearRenderable();
			m_dirty = false;
		}

		updateMeshBuffer();

		if (!m_renderable && m_material && m_mesh && m_mesh->isValid())
		{
			m_renderable = RenderProxy::create(m_mesh, m_material, nullptr, false);
			m_renderable->setSubmitToRenderQueue(false);
		}

		return m_renderable && m_mesh && m_mesh->getPrimitiveCount() ? true : false;
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
			oVertices.emplace_back(Vector3(-hw, -hh, 0.f), lightDir, lightColor, Vector2(1.f, 0.f));
			oVertices.emplace_back(Vector3(hw, -hh, 0.f), lightDir, lightColor, Vector2(0.f, 0.f));
			oVertices.emplace_back(Vector3(hw, hh, 0.f), lightDir, lightColor, Vector2(0.f, 1.f));
			oVertices.emplace_back(Vector3(-hw, hh, 0.f), lightDir, lightColor, Vector2(1.f, 1.f));

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

	bool DirectLighting::updateMeshBuffer()
	{
		// update data
		VertexArray    vertices;
		IndiceArray    indices;
		buildMeshData(vertices, indices);
		
		if (vertices.size() && indices.size())
		{
			MeshVertexFormat define;
			define.m_isUseNormal = true;
			define.m_isUseVertexColor = true;
			define.m_isUseUV = true;

			// create mesh
			if (!m_mesh) m_mesh = Mesh::create(true, true);

			m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(ui16), indices.data());
			m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());

			return true;
		}

		return false;
	}
}