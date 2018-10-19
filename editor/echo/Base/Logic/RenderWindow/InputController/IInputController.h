#pragma once

#include <QPoint>
#include "engine/core/geom/AABB.h"

class QWheelEvent;
class QMouseEvent;
class QKeyEvent;

namespace Studio
{
	struct InputContext
	{
		InputContext()
			: pressedButton(Qt::NoButton)
			, elapsedTime(0.f)
		{}

		InputContext(const QPointF& p, Qt::MouseButton button, float elapsed)
			: pos(p)
			, pressedButton(button)
			, elapsedTime(elapsed)
		{}

		QPointF pos;
		Qt::MouseButton pressedButton;
		float elapsedTime;
	};

	/**
	 * 渲染窗口控制接口
	 */
	class IRenderWindowInputController
	{
	public:
		virtual ~IRenderWindowInputController() {}

		// 每帧更新
		virtual void tick(const InputContext& ctx) = 0;

		// 鼠标滚轮事件
		virtual void wheelEvent(QWheelEvent* e) = 0;

		// 鼠标移动事件
		virtual void mouseMoveEvent(QMouseEvent* e) = 0;

		// 鼠标按下事件
		virtual void mousePressEvent(QMouseEvent* e) = 0;

		// 鼠标双击事件
		virtual void mouseDoubleClickEvent(QMouseEvent* e) = 0; 

		// 鼠标释放事件
		virtual void mouseReleaseEvent(QMouseEvent* e) = 0;

		// 鼠标按下事件
		virtual void keyPressEvent(QKeyEvent* e) = 0;

		// 鼠标抬起事件
		virtual void keyReleaseEvent(QKeyEvent* e) = 0;

		// 鼠标按键
		virtual Qt::MouseButton pressedMouseButton() = 0;

		// 鼠标位置
		virtual QPointF mousePosition() = 0;

		// 设置相机操作模式
		virtual void SetCameraOperateMode(int mode) = 0;

		// 返回当前相机操作模式
		virtual int GetCameraOperateMode() = 0;

		// 相机更新size
		virtual void onSizeCamera(unsigned int width, unsigned int height) = 0;

		// 相机自适应
		virtual void onAdaptCamera() = 0; 

		// 适应模型
		virtual void CameraZoom(const Echo::AABB& box, float scale){}

	public:
		// on open node tree
		virtual void onOpenNodeTree(const Echo::String& resPath) {}

		// on save node tree
		virtual void onSaveConfig() {}
	};
	typedef IRenderWindowInputController IRWInputController;
}