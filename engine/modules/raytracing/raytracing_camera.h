#pragma once

#include <radeon_rays.h>

namespace Echo
{
	// https://github.com/GPUOpen-LibrariesAndSDKs/RadeonRays_SDK/blob/master/Doc/RadeonRays.md
	class RaytracingCamera
	{
	public:
		RaytracingCamera();
		~RaytracingCamera();

		// update
		void update();

	protected:
		RadeonRays::Buffer*	m_rayBuffer = nullptr;
		RadeonRays::Buffer*	m_intersectionBuffer = nullptr;
	};
}