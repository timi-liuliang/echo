#pragma once

#include <radeon_rays.h>
#include "engine/core/render/base/texture/texture_render.h"
#include "engine/core/render/base/material.h"

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

		// render to framebuffer
		void renderToFrameBuffer();

	protected:
		ui32				m_width = 0;
		ui32				m_height = 0;
		RadeonRaysArray		m_rays;
		RadeonRays::Buffer*	m_rayBuffer = nullptr;
		RadeonHitInfoArray	m_intersections;
		RadeonRays::Buffer*	m_intersectionBuffer = nullptr;
		vector<Dword>::type	m_imageColors;
		TextureRenderPtr	m_texture;
		ShaderProgramPtr    m_shader;
		MaterialPtr         m_material;
		class ImageFilter*	m_imageFilter = nullptr;
	};
}