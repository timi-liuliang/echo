#include "InputController3d.h"
#include "EchoEngine.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <engine/core/render/render/Renderer.h>
#include <engine/core/Math/EchoMathFunction.h>

namespace Studio
{
	static const float kCameraRotationYScalar = 0.08f;
	static const float kCameraRotationZScalar = 0.3f;
	static const float kCameraRadiusScalar = 0.003f;

	InputController3d::InputController3d()
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
		, m_cameraRadius(8.f)
		, m_cameraLookAt(Echo::Vector3::ZERO)
		, m_cameraMoveDir(Echo::Vector3::UNIT_X)
		, m_cameraPositon(Echo::Vector3::ZERO)
		, m_horizonAngle(-Echo::Math::PI_DIV2)
		, m_verticleAngle(Echo::Math::PI_DIV2)
		, m_horizonAngleGoal(-Echo::Math::PI_DIV2)
 		, m_verticleAngleGoal(Echo::Math::PI_DIV2)
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
		, m_preMode(OrthoCamMode::OCM_NONE)
		, m_curMode(OrthoCamMode::OCM_NONE)
	{
		m_cameraForward = Echo::Vector3( 0.f, -1.f, -1.f);
		m_cameraForward.normalize();
		m_cameraForward.toHVAngle(m_horizonAngleGoal, m_verticleAngleGoal);
		m_cameraForward.toHVAngle(m_horizonAngle, m_verticleAngle);

		m_camera = Echo::NodeTree::instance()->get3dCamera();
		m_camera->setNearClip(0.1f);
		m_camera->setFarClip(250.f);

		m_orthoTopCamRot.z -= 0.01f;
		m_orthoTopCamRot.normalize();
	}

	InputController3d::~InputController3d()
	{
	}

	void InputController3d::tick(const InputContext& ctx)
	{
		// 摄像机旋转更新
		SmoothRotation(ctx.elapsedTime);

		// 移动摄像机
		Echo::Vector3 cameraMoveDir = Echo::Vector3::ZERO;
		if ( m_keyADown ) 
			cameraMoveDir.x += 1.f;
		if ( m_keyDDown ) 
			cameraMoveDir.x += -1.f;
		if ( m_keyWDown ) 
			cameraMoveDir.z += 1.f;
		if ( m_keySDown && !m_keyCtrlDown) 
			cameraMoveDir.z += -1.f;

		SetCameraMoveDir(cameraMoveDir);

		// 更新摄像机
		UpdateCamera(ctx.elapsedTime);
	}

	void InputController3d::wheelEvent(QWheelEvent* e)
	{
		CameraZoom(e->delta()  * -0.015f);
	}

	void InputController3d::mouseMoveEvent(QMouseEvent* e)
	{
		QPointF posChanged = e->localPos() - m_pos;
		if ( m_mouseRButtonPressed )
		{
			RotationCamera(posChanged.x() * kCameraRotationYScalar * kCameraRotationYScalar, m_cameraOperateMode* posChanged.y() * kCameraRadiusScalar);
		}

		m_pos = e->localPos();
	}

	void InputController3d::mousePressEvent(QMouseEvent* e)
	{
		updateMouseButtonPressedStatus(e, true);
	}

	void InputController3d::mouseReleaseEvent(QMouseEvent* e)
	{
		updateMouseButtonPressedStatus(e, false);
	}

	void InputController3d::keyPressEvent(QKeyEvent* e)
	{
		updateKeyPressedStatus(e, true);
	}

	void InputController3d::keyReleaseEvent(QKeyEvent* e)
	{
		updateKeyPressedStatus(e, false);
	}

	void InputController3d::updateMouseButtonPressedStatus(QMouseEvent* e, bool pressed)
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

	void InputController3d::updateKeyPressedStatus(QKeyEvent* e, bool pressed)
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
	Qt::MouseButton InputController3d::pressedMouseButton()
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
	QPointF InputController3d::mousePosition()
	{
		return m_pos;
	}

	void InputController3d::switchToOrthoCam(OrthoCamMode destMode, Echo::Vector3 pos)
	{
		m_preMode = m_curMode;
		m_curMode = destMode;
		if (m_curMode == OrthoCamMode::OCM_TOP && m_orthoTopCamPos == Echo::Vector3::ZERO)
		{
			m_orthoTopCamPos = pos;
		}
		else if (m_curMode == OrthoCamMode::OCM_FRONT && m_orthoFrontCamPos == Echo::Vector3::ZERO)
		{
			m_orthoFrontCamPos = pos;
		}
		else if (m_curMode == OrthoCamMode::OCM_LEFT && m_orthoLeftCamPos == Echo::Vector3::ZERO)
		{
			m_orthoLeftCamPos = pos;
		}

		Echo::Vector3 dir = Echo::Vector3::ZERO;
		if (m_curMode == OrthoCamMode::OCM_TOP)
		{
			pos = m_orthoTopCamPos;
			dir = m_orthoTopCamRot;
		}
		else if (m_curMode == OrthoCamMode::OCM_FRONT)
		{
			pos = m_orthoFrontCamPos;
			dir = m_orthoFrontCamRot;
		}
		else if (m_curMode == OrthoCamMode::OCM_LEFT)
		{
			pos = m_orthoLeftCamPos;
			dir = m_orthoLeftCamRot;
		}

		updateOrthoCamPos(m_preMode);

		Echo::Camera* mainCamera = Echo::NodeTree::instance()->get3dCamera();
		mainCamera->setPosition(pos);
		mainCamera->setDirection(dir);
		mainCamera->setProjectionMode(Echo::Camera::PM_ORTHO);

		UpdateOrthoCamModeWH(m_curMode);
	}

	void InputController3d::updateOrthoCamPos(OrthoCamMode mode)
	{
		Echo::Camera* mainCamera = Echo::NodeTree::instance()->get3dCamera();
		if (mode == OrthoCamMode::OCM_TOP)
		{
			m_orthoTopCamPos = mainCamera->getPosition();
			m_orthoTopCamRot = mainCamera->getDirection();
		}
		else if (mode == OrthoCamMode::OCM_FRONT)
		{
			m_orthoFrontCamPos = mainCamera->getPosition();
			m_orthoFrontCamRot = mainCamera->getDirection();
		}
		else if (mode == OrthoCamMode::OCM_LEFT)
		{
			m_orthoLeftCamPos = mainCamera->getPosition();
			m_orthoLeftCamRot = mainCamera->getDirection();
		}
		else
		{
			m_backCameraPos = mainCamera->getPosition();
			m_backCameraRot = mainCamera->getDirection();
		}
	}

	void InputController3d::resetPerspectiveCamera()
	{
		m_preMode = m_curMode;
		m_curMode = OrthoCamMode::OCM_NONE;
		Echo::Camera* mainCamera = Echo::NodeTree::instance()->get3dCamera();
		mainCamera->setPosition(m_backCameraPos);
		mainCamera->setDirection(m_backCameraRot);
		mainCamera->setProjectionMode(Echo::Camera::PM_PERSPECTIVE);
	}

	float InputController3d::getOrthoCamDis(OrthoCamMode mode)
	{
		if (mode == OrthoCamMode::OCM_TOP)
		{
			return m_orthoTopDis;
		}
		else if (mode == OrthoCamMode::OCM_FRONT)
		{
			return m_orthoFrontDis;
		}
		else if (mode == OrthoCamMode::OCM_LEFT)
		{
			return m_orthoLeftDis;
		}

		return 0.f;
	}

	void InputController3d::setOrthoCamDis(OrthoCamMode mode, float dis)
	{
		if (mode == OrthoCamMode::OCM_TOP)
		{
			m_orthoTopDis = dis;
		}
		else if (mode == OrthoCamMode::OCM_FRONT)
		{
			m_orthoFrontDis = dis;
		}
		else if (mode == OrthoCamMode::OCM_LEFT)
		{
			m_orthoLeftDis = dis;
		}
	}

	void InputController3d::UpdateOrthoCamModeWH(OrthoCamMode mode)
	{
		float dis = 0.0f;
		if (mode == OrthoCamMode::OCM_TOP)
		{
			dis = m_orthoTopDis;
		}
		else if (mode == OrthoCamMode::OCM_FRONT)
		{
			dis = m_orthoFrontDis;
		}
		else if (mode == OrthoCamMode::OCM_LEFT)
		{
			dis = m_orthoLeftDis;
		}

		Echo::Camera* mainCamera = Echo::NodeTree::instance()->get3dCamera();
		if (Echo::Renderer::instance()->getScreenHeight() > Echo::Renderer::instance()->getScreenWidth())
		{
			float aspect = (float)Echo::Renderer::instance()->getScreenHeight() / Echo::Renderer::instance()->getScreenWidth();
			mainCamera->setWidth(Echo::Math::Max(dis, 1.0f));
			mainCamera->setHeight(Echo::Math::Max(dis * aspect, 1.0f));
		}
		else
		{
			float aspect = (float)Echo::Renderer::instance()->getScreenWidth() / Echo::Renderer::instance()->getScreenHeight();
			mainCamera->setWidth(Echo::Math::Max(dis * aspect, 1.0f));
			mainCamera->setHeight(Echo::Math::Max(dis, 1.0f));
		}
	}

	void InputController3d::SetCameraOperateMode(int mode)
	{
		m_cameraOperateMode = mode;
	}

	int InputController3d::GetCameraOperateMode()
	{
		return m_cameraOperateMode;
	}

	//
	void InputController3d::onSizeCamera(unsigned int width, unsigned int height)
	{
	}

	//
	void InputController3d::onAdaptCamera()
	{
		AdaptCamera();
	}

	void InputController3d::UpdateCamera(float elapsedTime)
	{
		if (m_bNeedUpdateCamera)
		{
			m_cameraLookAt += m_cameraMoveDir * elapsedTime;

			m_cameraForward.normalize();
			m_cameraPositon = m_cameraLookAt - m_cameraForward * m_cameraRadius;

			m_camera->setPosition(m_cameraPositon);
			m_camera->setDirection(m_cameraLookAt-m_cameraPositon);
		}
	}

	void InputController3d::CameraZoom(const Echo::AABB& box, float scale)
	{
		float         radius = (box.getSize().len() * 0.5f);
		Echo::Vector3 center = ((box.vMin + box.vMax) * 0.5f);
		m_cameraRadius = radius * scale;
		m_cameraLookAt = center;
		m_cameraPositon = m_cameraLookAt - m_cameraForward * m_cameraRadius;
	}

	void InputController3d::SetCameraMoveDir(const Echo::Vector3& dir)
	{
		Echo::Vector3 forward = m_cameraForward; forward.y = 0.f;
		forward.normalize();

		Echo::Vector3 right = forward.cross(Echo::Vector3::UNIT_Y);
		right.normalize();

		m_cameraMoveDir = m_cameraForward * dir.z - right * dir.x;
		m_cameraMoveDir.normalize();
		m_cameraMoveDir *= 5.f;
	}

	void InputController3d::SetCameraMoveDir(const Echo::Vector3& dir, Echo::Vector3 forward)
	{
		Echo::Vector3 tempForward = forward;
		tempForward.y = 0.f;
		tempForward.normalize();

		Echo::Vector3 right = tempForward.cross(Echo::Vector3::UNIT_Y);
		right.normalize();

		m_cameraMoveDir = forward * dir.z - right * dir.x + Echo::Vector3::UNIT_Y * dir.y;
	}

	void InputController3d::CameraZoom(float zValue)
	{
		float newRadius = m_cameraRadius + zValue;
		if (newRadius * m_cameraRadius > 0.0f)
		{
			m_cameraRadius = newRadius;
		}
		else
		{
			m_cameraLookAt -= m_cameraForward * zValue;
		}
	}

	void InputController3d::SmoothRotation(float elapsedTime)
	{
		float diffHorizonAngle = m_horizonAngleGoal - m_horizonAngle;
		float diffVerticleAngle = m_verticleAngleGoal - m_verticleAngle;

		if (diffHorizonAngle > Echo::Math::PI_DIV6)
		{
			m_horizonAngle = m_horizonAngleGoal;
		}
		else
		{
			m_horizonAngle += diffHorizonAngle * elapsedTime * 25.f;
		}

		if ( diffVerticleAngle > Echo::Math::PI_DIV6 )
		{
			m_verticleAngle = m_verticleAngleGoal;
		}
		else
		{
			m_verticleAngle += diffVerticleAngle* elapsedTime * 25.f;
		}

		if (!Echo::Math::IsEqual(diffHorizonAngle, 0.0f) && !Echo::Math::IsEqual(diffVerticleAngle, 0.0f))
		{
			m_cameraForward.fromHVAngle(m_horizonAngle, m_verticleAngle);
			m_cameraLookAt = m_cameraPositon + m_cameraForward * m_cameraRadius;
		}
	}

	void InputController3d::RotationCamera(float xValue, float yValue)
	{
		if ( !xValue && !yValue )
			return;

		m_horizonAngleGoal += xValue;
		m_verticleAngleGoal += yValue;

		m_verticleAngleGoal = min(m_verticleAngleGoal, Echo::Math::PI - 0.01f);
		m_verticleAngleGoal = max(m_verticleAngleGoal, 0.01f);
		
	}

	void InputController3d::AdaptCamera()
	{
		Echo::Vector3 defaultPos = Echo::Vector3(0,10,10);
		Echo::Vector3 defaultDir = Echo::Vector3(0,-1,-1);

		m_cameraPositon = defaultPos;
		m_cameraLookAt = m_cameraPositon + defaultDir * m_cameraRadius;

		m_cameraForward = m_cameraLookAt - m_cameraPositon;
		m_cameraForward.normalize();

		m_cameraForward.toHVAngle(m_horizonAngle, m_verticleAngle);
		m_verticleAngleGoal = m_verticleAngle;
		m_horizonAngleGoal = m_horizonAngle;

		m_camera->setPosition(m_cameraPositon);
		m_camera->setDirection(m_cameraLookAt-m_cameraPositon);
	}

	void InputController3d::UpdateCameraInfo()
	{
		m_camera->setPosition(m_cameraPositon);
		m_camera->setPosition(m_cameraLookAt-m_cameraPositon);
	}

	bool InputController3d::isCameraMoving() const
	{
		return m_keyADown ||
			m_keyWDown ||
			m_keySDown ||
			m_keyDDown ||
			m_keyQDown ||
			m_keyEDown;
	}

	void InputController3d::rotateCameraAtPos(float xValue, float yValue, const Echo::Vector3& rotCenter)
	{
		Echo::Vector3 rotVec = m_cameraPositon - rotCenter;
		Echo::Vector3 forward = m_cameraForward;
		Echo::Vector3 camRight = m_camera->getRight();
		float verticleAngle = acos(forward.y);
		float vertRotAngle = verticleAngle + yValue;
		vertRotAngle = Echo::Math::Clamp(vertRotAngle, 0.01f, Echo::Math::PI - 0.01f) - verticleAngle;
		if (!Echo::Math::IsEqual(vertRotAngle, 0.0f))
		{
			Echo::Quaternion camRightQuat;
			camRightQuat.fromAxisAngle(camRight, -vertRotAngle);
			camRightQuat.rotateVec3(forward, forward);
			camRightQuat.rotateVec3(rotVec, rotVec);
			m_cameraForward = forward;
		}

		float sinv = Echo::Math::Sin(xValue);
		float cosv = Echo::Math::Cos(xValue);
		forward = m_cameraForward;
		m_cameraForward = Echo::Vector3(forward.x*cosv - forward.z*sinv, forward.y, forward.x*sinv + forward.z*cosv);
		rotVec = Echo::Vector3(rotVec.x*cosv - rotVec.z*sinv, rotVec.y, rotVec.x*sinv + rotVec.z*cosv);

		m_cameraPositon = rotCenter + rotVec;
		m_cameraLookAt = m_cameraPositon + m_cameraForward * m_cameraRadius;

		m_camera->setPosition(m_cameraPositon);
		m_camera->setDirection(m_cameraForward);
		m_camera->update();

		m_cameraForward.toHVAngle(m_horizonAngle, m_verticleAngle);
		m_verticleAngleGoal = m_verticleAngle;
		m_horizonAngleGoal = m_horizonAngle;
	}
}
