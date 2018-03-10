#pragma once

#include <QMenuBar>
#include <QMainWindow>

namespace QT_UI
{
	class QMenuBarEx : public QMenuBar
	{
		Q_OBJECT

	public:
		QMenuBarEx(QWidget* parent);
		~QMenuBarEx(){}

		// 设置左上角图标
		void setTopLeftCornerIcon(const char* icon);

	protected:
		// 鼠标移动事件
		virtual void mouseMoveEvent(QMouseEvent *e);

		// 鼠标双击事件
		virtual void mouseDoubleClickEvent(QMouseEvent *event);

		// 鼠标按下事件
		virtual void mousePressEvent(QMouseEvent *event);

		// 鼠标释放事件
		virtual void mouseReleaseEvent(QMouseEvent *event);

	protected slots:
		// 最小化
		void onMinus();

		// 最大化
		void onMaxmized();

	private:
		QPointF			m_lastPos;
		QMainWindow*	m_parent;
	};
}