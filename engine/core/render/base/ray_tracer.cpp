#include "ray_tracer.h"

namespace Echo
{
	static RayTracer* g_inst = nullptr;

	RayTracer::RayTracer()
	{
		g_inst = this;
	}

	RayTracer::~RayTracer()
	{

	}

	void RayTracer::bindMethods()
	{

	}

	RayTracer* RayTracer::instance()
	{
		return g_inst;
	}
}