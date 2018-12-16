#pragma once

#include <QWidget>
#include <QMenu>
#include "IInputController.h"

namespace Studio
{
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

		// input controller
		IRWInputController* getInputController(){return m_inputController; }
		IRWInputController* getInputController2D() { return m_inputController2d; }
		IRWInputController* getInputController3D() { return m_inputController3d; }

		// switch controller
		void switchToController2d() { m_inputController = m_inputController2d; }
		void switchToController3d() { m_inputController = m_inputController3d; }

	protected:
		// size hint
		virtual QSize sizeHint() const override;

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
		IRWInputController*		m_inputController2d;		// 默认输入控制器
		IRWInputController*		m_inputController3d;
		bool					m_isLeftButtonDown;			// 鼠标左键是否按下
		QSize					m_ratio;
	};
}