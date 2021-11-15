#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
	class RayTracer : public Object
	{
		ECHO_SINGLETON_CLASS(RayTracer, Object);

	public:
		RayTracer();
		virtual ~RayTracer();

		// instance
		static RayTracer* instance();
	};
}