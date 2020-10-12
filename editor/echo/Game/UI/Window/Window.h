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
		virtual void start(const Echo::String& echoProject);

		// aspect ratio
		void setAspectRatio(const QSize& size);
		QSize getAspectRatio() const { return m_ratio; }

		// mouse events
		virtual void mouseMoveEvent( QMouseEvent* e) override;
		virtual void mousePressEvent( QMouseEvent* e) override;
		virtual void mouseDoubleClickEvent(QMouseEvent* e) override;
		virtual void mouseReleaseEvent( QMouseEvent* e) override;
        virtual void wheelEvent( QWheelEvent* e) override;

		// key events
		virtual void keyPressEvent(QKeyEvent* e) override;
		virtual void keyReleaseEvent( QKeyEvent* e) override;

    protected slots:
		// render every frame
		void  render();

		// on reset device
		void  resetDevice();

		// check window size
		void checkWindowSize();

	protected:
		QSize					m_size;
		QTimer*					m_timer = nullptr;
		QSize					m_ratio;
		App*					m_app = nullptr;
	};
}
