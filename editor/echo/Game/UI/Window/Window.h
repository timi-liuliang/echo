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
		virtual void mouseMoveEvent( QMouseEvent* e) override;
		virtual void mousePressEvent( QMouseEvent* e) override;
		virtual void mouseDoubleClickEvent(QMouseEvent* e) override;
		virtual void mouseReleaseEvent( QMouseEvent* e) override;
        virtual void wheelEvent( QWheelEvent* e) override;

		// key events
		virtual void keyPressEvent(QKeyEvent* e) override;
		virtual void keyReleaseEvent( QKeyEvent* e) override;

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
