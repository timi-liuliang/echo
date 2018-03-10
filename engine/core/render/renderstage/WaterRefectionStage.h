#pragma once

#include "RenderStage.h"

namespace Echo
{
	/**
	 * 水面镜反图渲染阶段
	 */
	class WaterRefectRenderStage : public RenderStage
	{
		friend class RenderStageManager;
	public:
		WaterRefectRenderStage();
		virtual ~WaterRefectRenderStage();

		// 初始化
		virtual bool initialize();

		// 执行渲染
		virtual void render();

		// 销毁
		virtual void destroy();

		//创建rendertarget 只有在真正用的时候在创建,在创建水面实体的时候创建
		virtual void createRenderTarget();
	};
}