#pragma once

#include "Frame/Example.h"
#include "Engine/Core.h"
/************************************************************************/
/*           空间扭曲                                                   */
/************************************************************************/
namespace Examples
{
	class SpaceDistortion : public Example
	{
	public:
		SpaceDistortion();
		virtual ~SpaceDistortion();

		// 帮助信息
		virtual const LORD::String getHelp();

		// 初始化
		virtual bool initialize();

		// 更新
		virtual void tick(LORD::ui32 elapsedTime);

		// 销毁
		virtual void destroy();

	private:
		LORD::Scene*	m_scene;	// 场景
		LORD::Camera*	m_camera;   // 主摄像机
		LORD::String	sceneName;  // 场景名称
	};
}