#include "raytracing_mesh.h"
#include "raytracing_world.h"

namespace Echo
{
	RaytracingMesh::RaytracingMesh()
	{
		buildMesh();
	}

	RaytracingMesh::~RaytracingMesh()
	{
		RadeonRays::IntersectionApi* api = RaytracingWorld::instance()->getIntersectionApi();
		if (api && m_rayShape)
		{
			api->DetachShape(m_rayShapeInstance);
			api->DeleteShape(m_rayShapeInstance);
			api->DeleteShape(m_rayShape);
		}
	}

	void RaytracingMesh::bindMethods()
	{

	}

	void RaytracingMesh::buildMesh()
	{
		RadeonRays::IntersectionApi* api = RaytracingWorld::instance()->getIntersectionApi();
		if (api)
		{
			// Mesh vertices
			float const vertices[] =
			{
				-1.f,-1.f,0.f,
				1.f,-1.f,0.f,
				0.f,1.f,0.f,
			};
			int const g_indices[] = { 0, 1, 2 };
			// Number of vertices for the face
			const int g_numfaceverts[] = { 3 };

			m_rayShape = api->CreateMesh(vertices, 3, 3 * sizeof(float), g_indices, 0, g_numfaceverts, 1);
			m_rayShapeInstance = api->CreateInstance(m_rayShape);

			api->AttachShape(m_rayShapeInstance);
			api->Commit();
		}
	}
}