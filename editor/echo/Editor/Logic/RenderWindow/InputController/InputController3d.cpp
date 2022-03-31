#include "InputController3d.h"
#include "EchoEngine.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <engine/core/render/base/renderer.h>
#include <engine/core/math/Function.h>
#include "Modules/settings/editor_camera_settings.h"
#include "Studio.h"

namespace Studio
{
	static const float kCameraRotationYScalar = 0.08f;
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
		, m_camera(NULL)
		, m_cameraRadius(8.f)
		, m_cameraLookAt(Echo::Vector3::ZERO)
		, m_cameraMoveDir(Echo::Vector3::ZERO)
		, m_cameraPositon(Echo::Vector3::ZERO)
		, m_horizonAngle(-Echo::Math::PI_DIV2)
		, m_verticleAngle(Echo::Math::PI_DIV2)
		, m_horizonAngleGoal(-Echo::Math::PI_DIV2)
 		, m_verticleAngleGoal(Echo::Math::PI_DIV2)
		, m_bNeedUpdateCamera(true)
	{
		m_cameraForward = Echo::Vector3( 0.f, -1.f, -1.f);
		m_cameraForward.normalize();
		m_cameraForward.toHVAngle(m_horizonAngleGoal, m_verticleAngleGoal);
		m_cameraForward.toHVAngle(m_horizonAngle, m_verticleAngle);

		m_camera = Echo::NodeTree::instance()->get3dCamera();
		m_camera->setNear(0.1f);
		m_camera->setFar(2500.f);
	}

	InputController3d::~InputController3d()
	{
	}

	void InputController3d::tick(const InputContext& ctx)
	{
		// rotation camera
		SmoothRotation(ctx.elapsedTime);

		// move camera
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

		// update
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
			rotationCamera(posChanged.x() * kCameraRotationYScalar * kCameraRotationYScalar, posChanged.y() * kCameraRadiusScalar);
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

	QPointF InputController3d::mousePosition()
	{
		return m_pos;
	}

	void InputController3d::onSizeCamera(unsigned int width, unsigned int height)
	{
	}

	//
	//void InputController3d::onAdaptCamera()
	//{
	//	AdaptCamera();
	//}

	void InputController3d::UpdateCamera(float elapsedTime)
	{
		if (m_bNeedUpdateCamera)
		{
			m_camera->setNear(Echo::EditorCameraSettings::instance()->getNearClip());
			m_camera->setFar(Echo::EditorCameraSettings::instance()->getFarClip());

			float moveSpeed = Echo::EditorCameraSettings::instance()->getMoveSpeed();
			m_cameraLookAt += m_cameraMoveDir * elapsedTime * moveSpeed;

			m_cameraForward.normalize();
			m_cameraPositon = m_cameraLookAt - m_cameraForward * m_cameraRadius;
			m_camera->setPosition(m_cameraPositon);

			m_camera->setOrientation(Echo::Quaternion::fromYawPitchRoll(m_horizonAngle, m_verticleAngle /*- Echo::Math::PI_DIV2*/, 0.0)/*xTohForward * hToForward*/);

			// save config
			static float totalElapsed = 0.f;
			totalElapsed += elapsedTime;
			if (totalElapsed > 0.5f)
			{
				onSaveConfig();
				totalElapsed = 0.f;
			}
		}
	}

	void InputController3d::onFocusNode(Echo::Node* node)
	{
		if (node)
		{
			Echo::AABB worldAABB;
			node->buildWorldAABB( worldAABB);
			CameraZoom(worldAABB, 2.3f);
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

	void InputController3d::rotationCamera(float xValue, float yValue)
	{
		if ( !xValue && !yValue )
			return;

		m_horizonAngleGoal += xValue;
		m_verticleAngleGoal += yValue;

        m_verticleAngleGoal = std::min<float>(m_verticleAngleGoal, Echo::Math::PI - 0.01f);
        m_verticleAngleGoal = std::max<float>(m_verticleAngleGoal, 0.01f);		
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

	void InputController3d::onOpenNodeTree(const Echo::String& resPath)
	{
		// camera 3d
		Echo::Camera* camera3D = Echo::NodeTree::instance()->get2dCamera();
		if (camera3D && !resPath.empty())
		{
			Echo::String preStr = Echo::Engine::instance()->getResPath() + ":" + resPath;
			Echo::String cameraLookat = AStudio::instance()->getConfigMgr()->getValue((preStr + "camera3dlookat").c_str());
			if (!cameraLookat.empty())
				m_cameraLookAt = Echo::StringUtil::ParseVec3(cameraLookat);

			Echo::String hAngle = AStudio::instance()->getConfigMgr()->getValue((preStr + "camera3dHAngle").c_str());
			Echo::String vAngle = AStudio::instance()->getConfigMgr()->getValue((preStr + "camera3dVAngle").c_str());
			if (!hAngle.empty() && !vAngle.empty())
			{
				m_horizonAngle = m_horizonAngleGoal = Echo::StringUtil::ParseReal(hAngle);
				m_verticleAngle = m_verticleAngleGoal = Echo::StringUtil::ParseReal(vAngle);
				m_cameraForward.fromHVAngle(m_horizonAngle, m_verticleAngle);
			}

			Echo::String radius = AStudio::instance()->getConfigMgr()->getValue((preStr + "camera3dRadius").c_str());
			if (!radius.empty())
				m_cameraRadius = Echo::StringUtil::ParseReal(radius);
		}
	}

	void InputController3d::onSaveConfig()
	{
		const Echo::String& resPath = EchoEngine::instance()->getCurrentEditNodeSavePath();
		if (!resPath.empty())
		{
			// camera 3d
			Echo::Camera* camera3D = Echo::NodeTree::instance()->get3dCamera();
			if (camera3D && !resPath.empty())
			{
				Echo::String preStr = Echo::Engine::instance()->getResPath() + ":" + resPath;

				AStudio::instance()->getConfigMgr()->setValue((preStr + "camera3dlookat").c_str(), Echo::StringUtil::ToString(m_cameraLookAt).c_str());
				AStudio::instance()->getConfigMgr()->setValue((preStr + "camera3dHAngle").c_str(), Echo::StringUtil::ToString(m_horizonAngleGoal).c_str());
				AStudio::instance()->getConfigMgr()->setValue((preStr + "camera3dVAngle").c_str(), Echo::StringUtil::ToString(m_verticleAngleGoal).c_str());
				AStudio::instance()->getConfigMgr()->setValue((preStr + "camera3dRadius").c_str(), Echo::StringUtil::ToString(m_cameraRadius).c_str());
			}
		}
	}
}
