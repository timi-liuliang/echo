#include "raytracing_world.h"
#include "raytracing_camera.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/geom/Ray.h"
#include "engine/core/render/base/renderer.h"
#include "engine/core/render/base/pipeline/image_filter.h"

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

				renderToFrameBuffer();
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

	void RaytracingCamera::renderToFrameBuffer()
	{
		size_t pixelsize = PixelUtil::GetPixelSize(PF_RGBA8_UNORM);
		Buffer buffer(ui32(m_width * m_height * pixelsize), m_imageColors.data(), false);
		if (!m_texture)
		{
			static i32 idx = 0; idx++;
			m_texture = Renderer::instance()->createTextureRender(StringUtil::Format("RAYTRACING_TXTURE_RENDER_%d", idx));
		}

		m_texture->updateTexture2D(PF_RGBA8_UNORM, Texture::TU_GPU_READ, m_width, m_height, buffer.getData(), buffer.getSize());

		if (!m_imageFilter)
		{
			StringArray macros = { /* "ALPHA_ADJUST"*/ };
			m_shader = ShaderProgram::getDefault2D(macros);

			// material
			m_material = EchoNew(Material(StringUtil::Format("RayTracingMaterial")));
			m_material->setShaderPath(m_shader->getPath());
			m_material->getUniform("BaseColor")->setTexture(m_texture.ptr());

			m_imageFilter = EchoNew(ImageFilter);
			m_imageFilter->setMaterial(m_material);
		}

		if (m_imageFilter)
			m_imageFilter->render();
	}
}