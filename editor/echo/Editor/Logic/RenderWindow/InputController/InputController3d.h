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
	class InputController3d : public IRWInputController
	{
	public:
		InputController3d();
		virtual ~InputController3d();

		// tick
		virtual void tick(const InputContext& ctx) override;

		// events
		virtual void wheelEvent(QWheelEvent* e) override;
		virtual void mouseMoveEvent(QMouseEvent* e) override;
		virtual void mousePressEvent(QMouseEvent* e) override;
		virtual void mouseDoubleClickEvent(QMouseEvent* e) override {}
		virtual void mouseReleaseEvent(QMouseEvent* e) override;

		// key events
		virtual void keyPressEvent(QKeyEvent* e) override;
		virtual void keyReleaseEvent(QKeyEvent* e) override;

		// press button
		virtual Qt::MouseButton pressedMouseButton() override;

		// mouse position
		virtual QPointF mousePosition() override;

		// on size
		virtual void onSizeCamera(unsigned int width, unsigned int height) override;

		// zoom
		virtual void CameraZoom(const Echo::AABB& box, float scale);

		// camera radius
		float GetCameraRadius() { return m_cameraRadius; }
		void SetCameraRadius(float radius) { m_cameraRadius = Echo::Math::Clamp(radius, 0.1f, 1000.f); }

		bool isCameraMoving() const;

		// need update camera
		void setNeedUpdateCamera(bool need) { m_bNeedUpdateCamera = need; UpdateCameraPosition(0.01f); }

		// on focus node
		virtual void onFocusNode(Echo::Node* node) override;

	protected:
		// update camera
		void UpdateCameraPosition(float elapsedTime);

		// set move dir
		void UpdateCameraMoveDir();

		// zoom
		void CameraZoom(float zValue);

		// rotation
		void rotationCamera(float xValue, float yValue);

		// get camera
		Echo::Camera* GetCamera() { return m_camera; }

		// rotation
		void UpdateCameraRotation(float elapsedTime);

		// Apply orientation
		void ApplyOrientation(float hAngle, float vAngle);

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
		Echo::Camera*		m_camera;
		float				m_cameraRadius;
		Echo::Vector3		m_cameraLookAt;
		Echo::Vector3		m_cameraMoveDir;
		Echo::Vector3		m_cameraPositon;
		float				m_horizonAngle;
		float				m_verticleAngle;
		float				m_horizonAngleGoal;
		float				m_verticleAngleGoal;
		bool				m_bNeedUpdateCamera;
	};
}
