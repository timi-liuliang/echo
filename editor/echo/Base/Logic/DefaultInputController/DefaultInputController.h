#pragma once

#include "IRenderWindowInputController.h"
#include <QPoint>
#include "engine/core/Math/Vector3.h"
#include "engine/core/camera/Camera.h"

class QWheelEvent;
class QMouseEvent;
class QKeyEvent;

namespace Studio
{
	/**
	* 渲染窗口默认控制器
	*/
	class DefaultInputController : public IRWInputController
	{
	public:
		enum OrthoCamMode
		{
			OCM_TOP,
			OCM_FRONT,
			OCM_LEFT,
			OCM_NONE,
		};

		DefaultInputController();
		virtual ~DefaultInputController();

		// 每帧更新
		virtual void tick(const InputContext& ctx) override;

		// 鼠标滚轮事件
		virtual void wheelEvent(QWheelEvent* e) override;

		// 鼠标移动事件
		virtual void mouseMoveEvent(QMouseEvent* e) override;

		// 鼠标按下事件
		virtual void mousePressEvent(QMouseEvent* e) override;

		// 鼠标双击事件
		virtual void mouseDoubleClickEvent(QMouseEvent* e) override {}

		// 鼠标释放事件
		virtual void mouseReleaseEvent(QMouseEvent* e) override;

		// 鼠标按下事件
		virtual void keyPressEvent(QKeyEvent* e) override;

		// 鼠标抬起事件
		virtual void keyReleaseEvent(QKeyEvent* e) override;

		// 鼠标按键
		virtual Qt::MouseButton pressedMouseButton() override;

		// 鼠标位置
		virtual QPointF mousePosition() override;

		// 设置相机操作模式
		virtual void SetCameraOperateMode(int mode) override;

		// 返回当前相机操作模式
		virtual int GetCameraOperateMode() override;

		// 相机更新size
		virtual void onSizeCamera(unsigned int width, unsigned int height) override;

		// 修正摄像机
		virtual void onAdaptCamera() override;

		// 手动初始化摄像机参数
		virtual void onInitCameraSettings(float offsetdir) override;

		// 适应模型
		virtual void CameraZoom(const Echo::AABB& box, float scale);

		// 设置摄像机观察点
		void SetCameraLookAt(const Echo::Vector3& lookAt) { m_cameraLookAt = lookAt; }
		const Echo::Vector3& GetCameraLookAt() const { return m_cameraLookAt; }

		void SetCameraPosition(const Echo::Vector3& pos) { m_cameraPositon = pos; }
		const Echo::Vector3& GetCameraPosition() const { return m_cameraPositon; }

		void SetCameraForward(const Echo::Vector3& forward) { m_cameraForward = forward; }
		const Echo::Vector3& GetCameraForward() const { return m_cameraForward; }

		void SetCameraMoveDirVal(const Echo::Vector3& dir) { m_cameraMoveDir = dir; }
		const Echo::Vector3& GetCameraMoveDirVal() const { return m_cameraMoveDir; }

		void SetVerticleAngle(const float angle) { m_verticleAngle = angle; }
		const float GetVerticleAngle() const { return m_verticleAngle; }

		void SetHorizonAngle(const float angle) { m_horizonAngle = angle; }
		const float GetHorizonAngle() const { return m_horizonAngle; }

		void SetVerticleAngleGoal(const float angle) { m_verticleAngleGoal = angle; }
		const float GetVerticleAngleGoal() const { return m_verticleAngleGoal; }

		void SetHorizonAngleGoal(const float angle) { m_horizonAngleGoal = angle; }
		const float GetHorizonAngleGoal() const { return m_horizonAngleGoal; }

		void SetXOffset(const float offset) { m_xOffset = offset; }
		const float GetXOffset() const { return m_xOffset; }

		void SetYOffset(const float offset) { m_yOffset = offset; }
		const float GetYOffset() const { return m_yOffset; }

		// 获取摄像机半径（即摄像机与主角距离）
		float GetCameraRadius() { return m_cameraRadius; }

		// 设置摄像机半径
		void SetCameraRadius(float radius) { m_cameraRadius = Echo::Math::Clamp(radius, 0.1f, 1000.f); }

		void UpdateCameraInfo();

		bool isCameraMoving() const;

		// 控制是否更新摄像机
		void setNeedUpdateCamera(bool need) { m_bNeedUpdateCamera = need; UpdateCamera(0.01f); }

		void resetPerspectiveCamera();
		void switchToOrthoCam(OrthoCamMode destMode, Echo::Vector3 pos);
		void setOrthoCamDis(OrthoCamMode mode, float dis);
		float getOrthoCamDis(OrthoCamMode mode);
		void updateOrthoCamPos(OrthoCamMode mode);

	protected:
		// 初始化摄像机参数
		void InitializeCameraSettings(float offsetdir = 5);

		// 摄像机更新
		void UpdateCamera(float elapsedTime);

		// 平移摄像机
		void SetCameraMoveDir(const Echo::Vector3& dir);

		void SetCameraMoveDir(const Echo::Vector3& dir, Echo::Vector3 forward);

		// 操作摄像机
		void CameraZoom(float zValue);

		// 旋转摄像机
		void RotationCamera(float xValue, float yValue);

		// 添加屏幕偏移
		void AddScreenOffset(float xOffset, float yOffset);

		// 摄像机自适应
		void AdaptCamera();

		// 获取摄像机
		Echo::Camera* GetCamera() { return m_camera; }

		// 旋转摄像机(平滑处理)
		void SmoothRotation(float elapsedTime);
		void UpdateOrthoCamModeWH(OrthoCamMode mode);

	protected:
		void updateMouseButtonPressedStatus(QMouseEvent* e, bool pressed);
		void updateKeyPressedStatus(QKeyEvent* e, bool pressed);

		void rotateCameraAtPos(float xValue, float yValue, const Echo::Vector3& rotCenter);

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
		int m_cameraOperateMode; //1表示正常,-1表示上下旋转颠倒

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
		float				m_xOffset;			// X方向偏移
		float				m_yOffset;			// Y方向偏移

		bool				m_bNeedUpdateCamera;// 是否需要更新相机位置

		Echo::Vector3		m_backCameraPos;
		Echo::Vector3		m_backCameraRot;

		float				m_orthoTopDis;
		Echo::Vector3		m_orthoTopCamPos;
		Echo::Vector3		m_orthoTopCamRot;

		float				m_orthoFrontDis;
		Echo::Vector3		m_orthoFrontCamPos;
		Echo::Vector3		m_orthoFrontCamRot;

		float				m_orthoLeftDis;
		Echo::Vector3		m_orthoLeftCamPos;
		Echo::Vector3		m_orthoLeftCamRot;

		OrthoCamMode		m_preMode;
		OrthoCamMode		m_curMode;
	};
}
