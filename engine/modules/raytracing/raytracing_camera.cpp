#include "raytracing_world.h"
#include "raytracing_camera.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/geom/Ray.h"

namespace Echo
{
	RaytracingCamera::RaytracingCamera()
	{

	}

	RaytracingCamera::~RaytracingCamera()
	{
		clearBuffer();
	}

	void RaytracingCamera::update()
	{
		//RadeonRays::IntersectionApi* api = RaytracingWorld::instance()->getIntersectionApi();
		//if (api)
		//{
		//	if (!m_rayBuffer)
		//	{
		//		RadeonRays::ray		rays[3];
		//		for (i32 i = 0; i < 3; i++)
		//		{
		//			rays[i] = RadeonRays::ray(RadeonRays::float3(0, 0, 2), RadeonRays::float3(0, 0, -1.0));
		//		}

		//		m_rayBuffer = api->CreateBuffer(3 * sizeof(RadeonRays::ray), rays);
		//	}

		//	if (!m_intersectionBuffer)
		//	{
		//		RadeonRays::Intersection intersections[3];
		//		m_intersectionBuffer = api->CreateBuffer(3 * sizeof(RadeonRays::Intersection), intersections);
		//	}

		//	// query intersection
		//	api->QueryIntersection(m_rayBuffer, 3, m_intersectionBuffer, nullptr, nullptr);

		//	RadeonRays::Intersection* intersections = nullptr;
		//	api->MapBuffer(m_intersectionBuffer, RadeonRays::kMapRead, 0, 3 * sizeof(RadeonRays::Intersection), (void**)&intersections, nullptr);
		//	api->UnmapBuffer(m_intersectionBuffer, intersections, nullptr);
		//}


		RadeonRays::IntersectionApi* api = RaytracingWorld::instance()->getIntersectionApi();
		if (api)
		{
			if (m_rayBuffer && m_intersectionBuffer)
			{
				// query intersection
				api->QueryIntersection(m_rayBuffer, m_rays.size(), m_intersectionBuffer, nullptr, nullptr);

				// process result
				RadeonRays::Intersection* intersections = nullptr;
				api->MapBuffer(m_intersectionBuffer, RadeonRays::kMapRead, 0, m_rays.size() * sizeof(RadeonRays::Intersection), (void**)&intersections, nullptr);
				for (i32 h = 0; h < m_height; h++)
				{
					for (i32 w = 0; w < m_width; w++)
					{
						i32 offset = h * m_width + w;
						if (intersections[offset].shapeid!=-1)
						{
							m_imageColors[offset] = Color::BLUE;
						}
						else
						{
							m_imageColors[offset] = Color(0.f, 0.f, 0.f, 0.f);
						}
					}
				}
				api->UnmapBuffer(m_intersectionBuffer, intersections, nullptr);
			}
			else
			{
				// prepare rays
				prepareRays();
			}
		}
	}

	void RaytracingCamera::clearBuffer()
	{
		RadeonRays::IntersectionApi* api = RaytracingWorld::instance()->getIntersectionApi();
		if (api)
		{
			if (m_rayBuffer)
			{
				api->DeleteBuffer(m_rayBuffer);
				m_rayBuffer = nullptr;
			}

			if (m_intersectionBuffer)
			{
				api->DeleteBuffer(m_intersectionBuffer);
				m_intersectionBuffer = nullptr;
			}
		}
	}

	void RaytracingCamera::prepareRays()
	{
		RadeonRays::IntersectionApi* api = RaytracingWorld::instance()->getIntersectionApi();
		if (api)
		{
			clearBuffer();

			Camera* camera = NodeTree::instance()->get3dCamera();
			if (camera)
			{
				m_width = camera->getWidth();
				m_height = camera->getHeight();

				// 1. prepare rays buffer
				m_rays.resize(m_width * m_height);

				Echo::Ray ray;
				for (i32 h = 0; h < m_height; h++)
				{
					for (i32 w = 0; w < m_width; w++)
					{
						i32 offset = h * m_width + w;

						camera->getCameraRay(ray, Vector2(w, h));
						m_rays[offset] = RadeonRays::ray(RadeonRays::float3(ray.m_origin.x, ray.m_origin.y, ray.m_origin.z), RadeonRays::float3(ray.m_dir.x, ray.m_dir.y, ray.m_dir.z));
						//m_rays[offset] = RadeonRays::ray(RadeonRays::float3(0, 0, 2), RadeonRays::float3(0, 0, -1.0));
					}
				}

				m_rayBuffer = api->CreateBuffer(m_rays.size() * sizeof(RadeonRays::ray), m_rays.data());

				// 2. prepare intersection buffer
				m_intersections.resize(m_width * m_height);
				m_intersectionBuffer = api->CreateBuffer(m_intersections.size() * sizeof(RadeonRays::Intersection), m_intersections.data());

				// 3. prepare colors
				if (m_imageColors.size() != m_width * m_height)
				{
					m_imageColors.resize(m_width * m_height);
				}
			}
		}
	}
}