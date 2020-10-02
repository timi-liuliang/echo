#pragma once

#include <QWidget>
#include <QMenu>
#include "IInputController.h"
#include "TransformWidget.h"

namespace Studio
{
	class RenderWindow : public QWidget
	{
		Q_OBJECT

	public:
		RenderWindow( QWidget* parent = NULL);
		virtual ~RenderWindow();
        
        // create
        static RenderWindow* create();

		// begin render
		void BeginRender();

		// resize window
		void ResizeWindow();

		void SetAspectRatio(const QSize& size);

		QSize GetAspectRatio() const { return m_ratio; }

		// mouse event
		virtual void wheelEvent( QWheelEvent * e) override;
		virtual void mouseMoveEvent( QMouseEvent* e) override;
		virtual void mousePressEvent( QMouseEvent* e) override;
		virtual void mouseDoubleClickEvent(QMouseEvent* e) override;
		virtual void mouseReleaseEvent( QMouseEvent* e) override;

		// key event
		virtual void keyPressEvent( QKeyEvent* e) override;
		virtual void keyReleaseEvent( QKeyEvent* e) override;

		// set controller
		void setInputController( IRWInputController* controller) { m_inputController = controller; }

		// input controller
		IRWInputController* getInputController(){return m_inputController; }
		IRWInputController* getInputController2D() { return m_inputController2d; }
		IRWInputController* getInputController3D() { return m_inputController3d; }

		// switch controller
		void switchToController2d() { m_inputController = m_inputController2d; }
		void switchToController3d() { m_inputController = m_inputController3d; }

		// transform widget
		TransformWidget* getTransformWidget() { return m_transformWidget; }

	protected:
		// size hint
		virtual QSize sizeHint() const override;

	private slots:
		// render
        void  Render();

		// reset device
		void  ResetDevice();

	private:
		QSize					m_dx9Size;
		QTimer*					m_timer = nullptr;
		QMenu*					m_mouseMenu = nullptr;
		IRWInputController*		m_inputController = nullptr;
		IRWInputController*		m_inputController2d = nullptr;
		IRWInputController*		m_inputController3d = nullptr;
		TransformWidget*		m_transformWidget = nullptr;
		bool					m_isLeftButtonDown = false;
		QSize					m_ratio;
	};
}
