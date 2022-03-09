#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/math/Math.h"

namespace Echo
{
	class RenderScene
	{
	public:
		RenderScene();
		~RenderScene();

		// render
		static void renderAll();

	protected:
		// render
		void render();

	protected:
		Vector3				m_location;
		Matrix4				m_viewMatrix;
		Matrix4				m_projMatrix;
		//RenderPipelinePtr	m_pipeline;
	};
}