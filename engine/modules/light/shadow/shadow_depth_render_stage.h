#pragma once

#include "engine/core/render/base/pipeline/render_stage.h"

namespace Echo
{
	class ShadowDepthRenderStage : public RenderStage
	{
		ECHO_CLASS(ShadowDepthRenderStage, RenderStage)

	public:
		ShadowDepthRenderStage();
		virtual ~ShadowDepthRenderStage();

		// Name
		virtual void setName(const String& name) override { m_name = "Shadow Depth"; }

		// Shdow depth render stage don't need renderqueue
		virtual void addRenderQueue(IRenderQueue* queue, ui32 position = -1) override {}

		// Process
		virtual void render() override;

	protected:
		std::unordered_map<i32, RenderProxy*>	m_shadowDepthRenderProxiers;
	};
}