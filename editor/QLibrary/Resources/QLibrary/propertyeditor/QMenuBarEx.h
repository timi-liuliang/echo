#pragma once

#include <QMenuBar>
#include <QTOOLButton>
#include <QMainWindow>

namespace QT_UI
{
	class QMenuBarEx : public QMenuBar
	{
		Q_OBJECT

	public:
		// Button type
		enum ButtonType
		{
			Minus,
			Minimize,
			FullScreen,
			Close,
		};

	public:
		QMenuBarEx(QWidget* parent);
		~QMenuBarEx(){}

		// set top left corner icon
		void setTopLeftCornerIcon(const char* icon);

		// set button visible
		void setCornderButtonVisible(ButtonType type, bool visible);

	protected:
		virtual void mouseMoveEvent(QMouseEvent *e) override;
		virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
		virtual void mousePressEvent(QMouseEvent *event) override;
		virtual void mouseReleaseEvent(QMouseEvent *event) override;

	protected slots:
		void onMinus();
		void onMaxmized();
		void onFullScreen();

	protected:
		// redefine paintEvent
		virtual void paintEvent(QPaintEvent* event) override;

	private:
		QPointF			m_lastPos;
		QWidget*		m_parent;
		QToolButton*	m_minusButton = nullptr;
		QToolButton*	m_minimizeButton = nullptr;
		QToolButton*	m_fullScreenButton = nullptr;
		QToolButton*	m_closeButton = nullptr;
	};
}
