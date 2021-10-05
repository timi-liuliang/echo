#pragma once

#include "base/ray_tracer.h"

namespace Echo
{
	// https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/
	class VKRayTracer : public RayTracer
	{
	public:
		// 
		struct AccelerationStructureBuilder
		{
			// Initialize
			void init()
			{

			}
		};

	public:
		VKRayTracer();
		virtual ~VKRayTracer();

		// Initialize
		void init();
	};
}