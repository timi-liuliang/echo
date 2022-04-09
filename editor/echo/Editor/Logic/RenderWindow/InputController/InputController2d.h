#pragma once

#include "IInputController.h"
#include <QPoint>
#include "engine/core/math/Vector3.h"
#include "engine/core/camera/Camera.h"

class QWheelEvent;
class QMouseEvent;
class QKeyEvent;

namespace Studio
{
	class InputController2d : public IRWInputController
	{
	public:
		InputController2d();
		virtual ~InputController2d();

		// tick
		virtual void tick(const InputContext& ctx) override;

		// event
		virtual void wheelEvent(QWheelEvent* e) override;
		virtual void mouseMoveEvent(QMouseEvent* e) override;
		virtual void mousePressEvent(QMouseEvent* e) override;
		virtual void mouseDoubleClickEvent(QMouseEvent* e) override {}
		virtual void mouseReleaseEvent(QMouseEvent* e) override;
		virtual void keyPressEvent(QKeyEvent* e) override;
		virtual void keyReleaseEvent(QKeyEvent* e) override;

		// press mouse button
		virtual Qt::MouseButton pressedMouseButton() override;

		// mouse position
		virtual QPointF mousePosition() override;

		// camera
		virtual void onSizeCamera(unsigned int width, unsigned int height) override;
		virtual void onAdaptCamera() override;
		virtual void CameraZoom(const Echo::AABB& box, float scale);

		void UpdateCameraInfo();

		bool isCameraMoving() const;

		// set camera update
		void setNeedUpdateCamera(bool need) { m_bNeedUpdateCamera = need; UpdateCamera(0.01f); }

		// on focus node
		virtual void onFocusNode(Echo::Node* node) override;

	protected:
		// init camera settings
		void InitializeCameraSettings(float offsetdir = 5);

		// update camera
		void UpdateCamera(float elapsedTime);

		// zoom
		void CameraZoom(float zValue);

		// move
		void MoveCamera(float xValue, float yValue);

		// adapte
		void AdaptCamera();

	protected:
		void updateMouseButtonPressedStatus(QMouseEvent* e, bool pressed);
		void updateKeyPressedStatus(QKeyEvent* e, bool pressed);

	protected:
		// on open node tree
		virtual void onOpenNodeTree(const Echo::String& resPath) override;

		// on save node tree
		virtual void onSaveConfig() override;

	protected:
		bool m_mouseLButtonPressed;
		bool m_mouseMButtonPressed;
		bool m_mouseRButtonPressed;
		bool m_keyADown;
		bool m_keyWDown;
		bool m_keySDown;
		bool m_keyDDown;
		bool m_keyQDown;
		bool m_keyEDown;
		bool m_keyAltDown;
		bool m_keyCtrlDown;
		bool m_keyShiftDown;
		QPointF m_pos;

	protected:
		float				m_cameraScale;
		Echo::Vector3		m_cameraMoveDir;
		Echo::Vector3		m_cameraPositon;
		bool				m_bNeedUpdateCamera = true;
	};
}
