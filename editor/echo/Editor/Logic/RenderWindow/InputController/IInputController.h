#pragma once

#include <QPoint>
#include "engine/core/geom/AABB.h"
#include <engine/core/scene/node.h>

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

		// 相机更新size
		virtual void onSizeCamera(unsigned int width, unsigned int height) = 0;

		// 相机自适应
		virtual void onAdaptCamera() {}

		// on focuse node
		virtual void onFocusNode(Echo::Node* node)=0;

	public:
		// on open node tree
		virtual void onOpenNodeTree(const Echo::String& resPath) {}

		// on save node tree
		virtual void onSaveConfig() {}
	};
	typedef IRenderWindowInputController IRWInputController;
}