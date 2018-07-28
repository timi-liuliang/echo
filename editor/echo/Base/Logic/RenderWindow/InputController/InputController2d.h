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
	class InputController2d : public IRWInputController
	{
	public:
		InputController2d();
		virtual ~InputController2d();

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

		// 适应模型
		virtual void CameraZoom(const Echo::AABB& box, float scale);

		void SetCameraPosition(const Echo::Vector3& pos) { m_cameraPositon = pos; }
		const Echo::Vector3& GetCameraPosition() const { return m_cameraPositon; }

		void SetCameraForward(const Echo::Vector3& forward) { m_cameraForward = forward; }
		const Echo::Vector3& GetCameraForward() const { return m_cameraForward; }

		void SetCameraMoveDirVal(const Echo::Vector3& dir) { m_cameraMoveDir = dir; }
		const Echo::Vector3& GetCameraMoveDirVal() const { return m_cameraMoveDir; }

		void UpdateCameraInfo();

		bool isCameraMoving() const;

		// 控制是否更新摄像机
		void setNeedUpdateCamera(bool need) { m_bNeedUpdateCamera = need; UpdateCamera(0.01f); }

		void resetPerspectiveCamera();

	protected:
		// 初始化摄像机参数
		void InitializeCameraSettings(float offsetdir = 5);

		// 摄像机更新
		void UpdateCamera(float elapsedTime);

		// 操作摄像机
		void CameraZoom(float zValue);

		// 位移摄像机
		void MoveCamera(float xValue, float yValue);

		// 摄像机自适应
		void AdaptCamera();

		// 获取摄像机
		Echo::Camera* GetCamera() { return m_camera; }

	protected:
		void updateMouseButtonPressedStatus(QMouseEvent* e, bool pressed);
		void updateKeyPressedStatus(QKeyEvent* e, bool pressed);

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
		float				m_cameraScale;		// 缩放值
		Echo::Vector3		m_cameraMoveDir;	// 摄像机移动
		Echo::Vector3		m_cameraForward;	// 朝向
		Echo::Vector3		m_cameraPositon;	// 摄像机位置

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
	};
}
