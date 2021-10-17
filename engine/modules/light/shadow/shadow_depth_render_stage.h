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
		virtual void setName(const String& name) override { m_name = "ShadowDepth"; }
	};
}