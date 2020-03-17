#include "raytracing_world.h"
#include "raytracing_camera.h"

namespace Echo
{
	RaytracingCamera::RaytracingCamera()
	{

	}

	RaytracingCamera::~RaytracingCamera()
	{

	}

	void RaytracingCamera::update()
	{
		RadeonRays::IntersectionApi* api = RaytracingWorld::instance()->getIntersectionApi();
		if (api)
		{
			if (!m_rayBuffer)
			{
				RadeonRays::ray		rays[3];
				for (i32 i = 0; i < 3; i++)
				{
					rays[i] = RadeonRays::ray(RadeonRays::float3(0, 0, 0), RadeonRays::float3(0, 0, -1.0));
				}

				m_rayBuffer = api->CreateBuffer(3 * sizeof(RadeonRays::ray), rays);
			}

			if (!m_intersectionBuffer)
			{
				RadeonRays::Intersection intersections[3];
				m_intersectionBuffer = api->CreateBuffer(3 * sizeof(RadeonRays::Intersection), intersections);
			}

			// query intersection
			api->QueryIntersection(m_rayBuffer, 3, m_intersectionBuffer, nullptr, nullptr);

			RadeonRays::Intersection* intersections = nullptr;
			api->MapBuffer(m_intersectionBuffer, RadeonRays::kMapRead, 0, 3 * sizeof(RadeonRays::Intersection),(void**)&intersections, nullptr);
			api->UnmapBuffer(m_intersectionBuffer, intersections, nullptr);
		}
	}
}