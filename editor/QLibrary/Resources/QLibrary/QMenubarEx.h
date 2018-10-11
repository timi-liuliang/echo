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

		// set top left corner icon
		void setTopLeftCornerIcon(const char* icon);

	protected:
		virtual void mouseMoveEvent(QMouseEvent *e) override;
		virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
		virtual void mousePressEvent(QMouseEvent *event) override;
		virtual void mouseReleaseEvent(QMouseEvent *event) override;

	protected slots:
		void onMinus();
		void onMaxmized();

	protected:
		// redefine paintEvent
		virtual void paintEvent(QPaintEvent* event) override;

	private:
		QPointF			m_lastPos;
		QMainWindow*	m_parent;
	};
}