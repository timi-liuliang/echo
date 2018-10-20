#pragma once

#include "IInputController.h"
#include <QPoint>
#include "engine/core/Math/Vector3.h"
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

		// 每帧更新
		virtual void tick(const InputContext& ctx) override;

		// events
		virtual void wheelEvent(QWheelEvent* e) override;
		virtual void mouseMoveEvent(QMouseEvent* e) override;
		virtual void mousePressEvent(QMouseEvent* e) override;
		virtual void mouseDoubleClickEvent(QMouseEvent* e) override {}
		virtual void mouseReleaseEvent(QMouseEvent* e) override;
		virtual void keyPressEvent(QKeyEvent* e) override;
		virtual void keyReleaseEvent(QKeyEvent* e) override;

		// 鼠标按键
		virtual Qt::MouseButton pressedMouseButton() override;

		// 鼠标位置
		virtual QPointF mousePosition() override;

		// 相机更新size
		virtual void onSizeCamera(unsigned int width, unsigned int height) override;

		// 修正摄像机
		virtual void onAdaptCamera() override;

		// 适应模型
		virtual void CameraZoom(const Echo::AABB& box, float scale);

		// 获取摄像机半径（即摄像机与主角距离）
		float GetCameraRadius() { return m_cameraRadius; }

		// 设置摄像机半径
		void SetCameraRadius(float radius) { m_cameraRadius = Echo::Math::Clamp(radius, 0.1f, 1000.f); }

		bool isCameraMoving() const;

		// 控制是否更新摄像机
		void setNeedUpdateCamera(bool need) { m_bNeedUpdateCamera = need; UpdateCamera(0.01f); }

		void resetPerspectiveCamera();

	protected:
		// 摄像机更新
		void UpdateCamera(float elapsedTime);

		// 平移摄像机
		void SetCameraMoveDir(const Echo::Vector3& dir);

		// 操作摄像机
		void CameraZoom(float zValue);

		// 旋转摄像机
		void RotationCamera(float xValue, float yValue);

		// 摄像机自适应
		void AdaptCamera();

		// 获取摄像机
		Echo::Camera* GetCamera() { return m_camera; }

		// 旋转摄像机(平滑处理)
		void SmoothRotation(float elapsedTime);

	protected:
		void updateMouseButtonPressedStatus(QMouseEvent* e, bool pressed);
		void updateKeyPressedStatus(QKeyEvent* e, bool pressed);

		void rotateCameraAtPos(float xValue, float yValue, const Echo::Vector3& rotCenter);

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
		Echo::Camera*		m_camera;			// 主摄像机
		float				m_cameraRadius;		// 摄像机观察半径
		Echo::Vector3		m_cameraLookAt;		// 摄像机观察点
		Echo::Vector3		m_cameraMoveDir;	// 摄像机移动
		Echo::Vector3		m_cameraForward;	// 朝向
		Echo::Vector3		m_cameraPositon;	// 摄像机位置
		float				m_horizonAngle;		// 水平旋转角度(顺时针)
		float				m_verticleAngle;	// 垂直旋转角度(顺时针)
		float				m_horizonAngleGoal;	// 目标水平旋转角度
		float				m_verticleAngleGoal;// 目标垂直旋转角度
		bool				m_bNeedUpdateCamera;// 是否需要更新相机位置
	};
}
