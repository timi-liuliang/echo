#include "InputController2d.h"
#include "EchoEngine.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <engine/core/Math/EchoMathFunction.h>

namespace Studio
{
	static const float kCameraRotationYScalar = 0.08f;
	static const float kCameraRotationZScalar = 0.3f;
	static const float kCameraRadiusScalar = 0.003f;

	InputController2d::InputController2d()
		: m_mouseLButtonPressed(false)
		, m_mouseMButtonPressed(false)
		, m_mouseRButtonPressed(false)
		, m_keyADown(false)
		, m_keyWDown(false)
		, m_keySDown(false)
		, m_keyDDown(false)
		, m_keyQDown(false)
		, m_keyEDown(false)
		, m_keyAltDown(false)
		, m_keyCtrlDown(false)
		, m_keyShiftDown(false)
		, m_cameraOperateMode(1)
		, m_camera(NULL)
		, m_cameraMoveDir(Echo::Vector3::UNIT_X)
		, m_cameraForward(-Echo::Vector3::UNIT_Z)
		, m_cameraPositon(Echo::Vector3::ZERO)
		, m_bNeedUpdateCamera(true)
		, m_orthoTopCamRot(-Echo::Vector3::UNIT_Y)
		, m_orthoFrontCamRot(-Echo::Vector3::UNIT_Z)
		, m_orthoLeftCamRot(Echo::Vector3::UNIT_X)
		, m_orthoTopDis(0.f)
		, m_orthoFrontDis(0.f)
		, m_orthoLeftDis(0.f)
		, m_orthoTopCamPos(Echo::Vector3::ZERO)
		, m_orthoFrontCamPos(Echo::Vector3::ZERO)
		, m_orthoLeftCamPos(Echo::Vector3::ZERO)
		, m_cameraScale(1.f)
	{
		m_camera = Echo::NodeTree::instance()->get2dCamera();

		// 初始化摄像机参数
		InitializeCameraSettings();

		m_orthoTopCamRot.z -= 0.01f;
		m_orthoTopCamRot.normalize();

	}

	InputController2d::~InputController2d()
	{
	}

	// 每帧更新
	void InputController2d::tick(const InputContext& ctx)
	{
		// 移动摄像机
		Echo::Vector3 cameraMoveDir = Echo::Vector3::ZERO;
		if ( m_keyADown ) 
			cameraMoveDir.x += -1.f;
		if ( m_keyDDown ) 
			cameraMoveDir.x += 1.f;
		if ( m_keyWDown ) 
			cameraMoveDir.y += 1.f;
		if ( m_keySDown && !m_keyCtrlDown) 
			cameraMoveDir.y += -1.f;

		m_cameraMoveDir = cameraMoveDir;
		m_cameraMoveDir.normalize();

		// 更新摄像机
		UpdateCamera(ctx.elapsedTime);
	}

	// 鼠标滚轮事件
	void InputController2d::wheelEvent(QWheelEvent* e)
	{
		CameraZoom(e->delta()  * -0.015f);
	}

	// 鼠标移动事件
	void InputController2d::mouseMoveEvent(QMouseEvent* e)
	{
		QPointF posChanged = e->localPos() - m_pos;
		if (m_mouseMButtonPressed)
		{
			MoveCamera(posChanged.x() * kCameraRotationYScalar * kCameraRotationYScalar, m_cameraOperateMode* posChanged.y() * kCameraRadiusScalar);
		}

		m_pos = e->localPos();
	}

	// 鼠标按下事件
	void InputController2d::mousePressEvent(QMouseEvent* e)
	{
		updateMouseButtonPressedStatus(e, true);
	}

	// 鼠标释放事件
	void InputController2d::mouseReleaseEvent(QMouseEvent* e)
	{
		updateMouseButtonPressedStatus(e, false);
	}

	// 鼠标按下事件
	void InputController2d::keyPressEvent(QKeyEvent* e)
	{
		updateKeyPressedStatus(e, true);
	}

	// 鼠标抬起事件
	void InputController2d::keyReleaseEvent(QKeyEvent* e)
	{
		updateKeyPressedStatus(e, false);
	}

	void InputController2d::updateMouseButtonPressedStatus(QMouseEvent* e, bool pressed)
	{
		if ( e->button() == Qt::LeftButton )
		{
			m_mouseLButtonPressed = pressed;
		}
		else if ( e->button() == Qt::RightButton )
		{
			m_mouseRButtonPressed = pressed;
		}
		else if ( e->button() == Qt::MidButton )
		{
			m_mouseMButtonPressed = pressed;
		}
		m_pos = e->localPos();
	}

	void InputController2d::updateKeyPressedStatus(QKeyEvent* e, bool pressed)
	{
		switch ( e->key() )
		{
			case Qt::Key_A: m_keyADown = pressed; break;
			case Qt::Key_W: m_keyWDown = pressed; break;
			case Qt::Key_S: m_keySDown = pressed; break;
			case Qt::Key_D: m_keyDDown = pressed; break;
			case Qt::Key_Q: m_keyQDown = pressed; break;
			case Qt::Key_E: m_keyEDown = pressed; break;
			case Qt::Key_Control: m_keyCtrlDown = pressed; break;
			case Qt::Key_Alt: m_keyAltDown = pressed; break;
			case Qt::Key_Shift: m_keyShiftDown = pressed; break;
			default: break;
		}
	}

	// 鼠标按键
	Qt::MouseButton InputController2d::pressedMouseButton()
	{
		if ( m_mouseLButtonPressed )
		{
			return Qt::LeftButton;
		}
		else if ( m_mouseMButtonPressed )
		{
			return Qt::MiddleButton;
		}
		else if ( m_mouseRButtonPressed )
		{
			return Qt::RightButton;
		}
		else
		{
			return Qt::NoButton;
		}
	}

	// 鼠标位置
	QPointF InputController2d::mousePosition()
	{
		return m_pos;
	}

	//设置相机操作模式
	void InputController2d::SetCameraOperateMode(int mode)
	{
		m_cameraOperateMode = mode;
	}

	//返回当前相机操作模式
	int InputController2d::GetCameraOperateMode()
	{
		return m_cameraOperateMode;
	}

	//
	void InputController2d::onSizeCamera(unsigned int width, unsigned int height)
	{
	}

	//
	void InputController2d::onAdaptCamera()
	{
		AdaptCamera();
	}

	void InputController2d::onInitCameraSettings(float offsetdir /* = 0 */)
	{
		InitializeCameraSettings(offsetdir);
	}

	// 初始化摄像机参数
	void InputController2d::InitializeCameraSettings(float diroffset)
	{
	}

	// 摄像机更新
	void InputController2d::UpdateCamera(float elapsedTime)
	{
		if (m_bNeedUpdateCamera)
		{
			m_cameraPositon += m_cameraMoveDir * elapsedTime * 300;

			m_cameraForward.normalize();
			m_camera->setScale(m_cameraScale);
			m_camera->setPosition(m_cameraPositon);
			m_camera->setDirection(Echo::Vector3::NEG_UNIT_Z);
		}
	}

	// 适应模型
	void InputController2d::CameraZoom(const Echo::AABB& box, float scale)
	{
		//float         radius = (box.getSize().len() * 0.5f);
		//Echo::Vector3 center = ((box.vMin + box.vMax) * 0.5f);
		//m_cameraRadius = radius * scale;
		//m_cameraLookAt = center;
		//m_cameraPositon = m_cameraLookAt - m_cameraForward * m_cameraRadius;
	}

	// 操作摄像机
	void InputController2d::CameraZoom(float zValue)
	{
		m_cameraScale = Echo::Math::Clamp(m_cameraScale + zValue * 0.03f, 0.01f, 100.f);
	}

	// 旋转摄像机
	void InputController2d::MoveCamera(float xValue, float yValue)
	{
		if ( !xValue && !yValue )
			return;

		Echo::Vector3 cameraMoveDir(-xValue, yValue, 0.f);
		m_cameraPositon += cameraMoveDir * 300;
	}

	void InputController2d::AdaptCamera()
	{
	}

	void InputController2d::UpdateCameraInfo()
	{
		m_camera->setPosition(m_cameraPositon);
	}

	bool InputController2d::isCameraMoving() const
	{
		return m_keyADown ||
			m_keyWDown ||
			m_keySDown ||
			m_keyDDown ||
			m_keyQDown ||
			m_keyEDown;
	}
}
