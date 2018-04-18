#pragma once

#include <QWidget>
#include <QMenu>
#include "App.h"

namespace Game
{
	//-----------------------------------
	// 显示窗口
	//-----------------------------------
	class Window : public QWidget
	{
		Q_OBJECT

	public:
		Window( QWidget* parent = NULL);
		virtual ~Window();

		// 开始渲染
		void start(const Echo::String& echoProject);

		// 窗口大小改变
		void ResizeWindow();

		// aspect ratio
		void SetAspectRatio(const QSize& size);
		QSize GetAspectRatio() const { return m_ratio; }

		// 鼠标滚轮事件
		virtual void wheelEvent( QWheelEvent * e);

		// 鼠标移动事件
		virtual void mouseMoveEvent( QMouseEvent* e);

		// 鼠标按下事件
		virtual void mousePressEvent( QMouseEvent* e);

		// 鼠标双击事件
		virtual void mouseDoubleClickEvent(QMouseEvent* e);

		// 鼠标释放事件
		virtual void mouseReleaseEvent( QMouseEvent* e);

		// 鼠标按下事件
		virtual void keyPressEvent( QKeyEvent* e);

		// 鼠标抬起事件
		virtual void keyReleaseEvent( QKeyEvent* e);

	private slots:
		// 渲染
		void  Render();

		// 重置设备
		void  ResetDevice();

	private:
		QSize					m_dx9Size;					// 调整后大小
		QTimer*					m_timer;					// 计时器
		QSize					m_ratio;
		App*					m_app;
	};
}