#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/resource/ResRef.h"
#include "engine/core/math/Math.h"
#include "engine/core/geom/Frustum.h"

namespace Echo
{
	class RenderScene : public Refable
	{
	public:
		RenderScene();
		~RenderScene();

		// Update
		void update(const Frustum& frustum);

		// render
		static void renderAll();

	protected:
		// Render
		void render();

	protected:
		Vector3				m_location;
		Matrix4				m_viewMatrix;
		Matrix4				m_projMatrix;
		Frustum				m_3dFrustum;
		//RenderPipelinePtr	m_pipeline;
	};
	typedef ResRef<RenderScene> RenderScenePtr;
}