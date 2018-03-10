#pragma once

#include "RenderStage.h"

namespace Echo
{
	/**
	* 后处理完成阶段
	*/
	class AfterPostProcessStage : public RenderStage
	{
		friend class RenderStageManager;
	public:
		AfterPostProcessStage();
		virtual ~AfterPostProcessStage();

		// 初始化
		virtual bool initialize();

		// 渲染
		virtual void render();

		// 销毁
		virtual void destroy();
	};
}