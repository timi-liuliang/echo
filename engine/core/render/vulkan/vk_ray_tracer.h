#pragma once

#include "base/ray_tracer.h"

namespace Echo
{
	class VKRayTracer : public RayTracer
	{
	public:
		VKRayTracer();
		virtual ~VKRayTracer();
	};
}