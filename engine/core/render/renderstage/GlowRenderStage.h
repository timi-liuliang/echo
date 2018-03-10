#pragma once
#include "RenderStage.h"

namespace Echo
{
	class GlowRenderStage : public RenderStage
	{
		friend class RenderStageManager;
		typedef vector<int>::type RenderableList;
	public:
		GlowRenderStage();
		virtual ~GlowRenderStage();

		virtual bool initialize();
		virtual void render();
		virtual void destroy();
		virtual void setEnable(bool setting);
	};
}