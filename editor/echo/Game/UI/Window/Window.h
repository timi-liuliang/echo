#pragma once

#include <QWidget>
#include <QMenu>
#include "App.h"

namespace Game
{
	class Window : public QWidget
	{
		Q_OBJECT

	public:
		Window( QWidget* parent = NULL);
		virtual ~Window();

		// start
		void start(const Echo::String& echoProject);

		// aspect ratio
		void setAspectRatio(const QSize& size);
		QSize getAspectRatio() const { return m_ratio; }

		// mouse events
		virtual void mouseMoveEvent( QMouseEvent* e);
		virtual void mousePressEvent( QMouseEvent* e);
		virtual void mouseDoubleClickEvent(QMouseEvent* e);
		virtual void mouseReleaseEvent( QMouseEvent* e);
        virtual void wheelEvent( QWheelEvent * e);

		// key events
		virtual void keyPressEvent( QKeyEvent* e);
		virtual void keyReleaseEvent( QKeyEvent* e);

	private slots:
		// render every frame
		void  render();

		// on reset device
		void  resetDevice();

	private:
		// check window size
		void checkWindowSize();

	private:
		QSize					m_dx9Size;
		QTimer*					m_timer = nullptr;
		QSize					m_ratio;
		App*					m_app = nullptr;
	};
}
