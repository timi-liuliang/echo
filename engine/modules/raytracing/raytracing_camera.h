#pragma once

#include <radeon_rays.h>

namespace Echo
{
	// https://github.com/GPUOpen-LibrariesAndSDKs/RadeonRays_SDK/blob/master/Doc/RadeonRays.md
	class RaytracingCamera
	{
		typedef vector<RadeonRays::ray>::type RadeonRaysArray;
		typedef vector<RadeonRays::Intersection>::type RadeonHitInfoArray;

	public:
		RaytracingCamera();
		~RaytracingCamera();

		// update
		void update();

	private:
		// update buffer
		void clearBuffer();
		void prepareRays();

	protected:
		ui32				m_width = 0;
		ui32				m_height = 0;
		RadeonRaysArray		m_rays;
		RadeonRays::Buffer*	m_rayBuffer = nullptr;
		RadeonHitInfoArray	m_intersections;
		RadeonRays::Buffer*	m_intersectionBuffer = nullptr;
		vector<Color>::type	m_imageColors;
	};
}