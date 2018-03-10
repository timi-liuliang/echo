#pragma once

#include "RenderStage.h"

namespace Echo
{
	/**
	* LDRRenderStage
	*/
	class QueryObject;
	class LDRSceneColorRenderStage : public RenderStage
	{
		friend class RenderStageManager;
		typedef vector<QueryObject*>::type QueryObjectVector;
	public:
		LDRSceneColorRenderStage();
		virtual ~LDRSceneColorRenderStage();

		// 初始化
		virtual bool initialize();

		// 执行渲染
		virtual void render();

		// 销毁
		virtual void destroy();
	};
}