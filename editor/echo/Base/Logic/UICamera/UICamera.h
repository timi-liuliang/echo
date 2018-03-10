#pragma once

#include <engine/core/camera/Camera.h>

namespace Studio
{
	/**
	 * UI摄像机
	 */
	class UICamera : public Echo::Camera
	{
	public:
		UICamera();
		~UICamera();

		// 设置屏幕宽高
		void setWidthHeight( Echo::ui32 width, Echo::ui32 height);

		// 更新
		virtual void update();
	};

	// UI逻辑大小映射
	void MappingUILogicWH( Echo::ui32 screenWidth, Echo::ui32 screeHeight, Echo::ui32& oLogicWidth, Echo::ui32& oLogicHeight);
}