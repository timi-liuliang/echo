#pragma once

#include <QWidget>
#include <QMenu>
#include "IRenderWindowInputController.h"

namespace Studio
{
	//-----------------------------------
	// Dx9MdiArea dx显示窗口
	//-----------------------------------
	class RenderWindow : public QWidget
	{
		Q_OBJECT

	public:
		RenderWindow( QWidget* parent = NULL);
		virtual ~RenderWindow();

		// 开始渲染
		void BeginRender();

		// 窗口大小改变
		void ResizeWindow();

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

		// 输入控制器
		void setInputController( IRWInputController* controller) { m_inputController = controller; }

		// 获取输入控制器
		IRWInputController* getInputController(){return m_inputController? m_inputController:m_defaultInputController; }

	private slots:
		// 渲染
		void  Render();

		// 重置设备
		void  ResetDevice();

	private:
		QSize					m_dx9Size;					// 调整后大小
		QTimer*					m_timer;					// 计时器
		QMenu*					m_mouseMenu;				// 鼠标菜单
		IRWInputController*		m_inputController;			// 输入控制器
		IRWInputController*		m_defaultInputController;	// 默认输入控制器
		bool					m_isLeftButtonDown;			// 鼠标左键是否按下
		QSize					m_ratio;
	};
}