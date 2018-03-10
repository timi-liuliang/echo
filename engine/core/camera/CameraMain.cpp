#include "Engine/core/Camera/CameraMain.h"
#include "Engine/core/Camera/CameraShakeModule.h"

namespace Echo
{
	CameraMain::CameraMain(ProjMode mode, bool isFixedYaw)
		: Camera(mode, isFixedYaw)
		, m_CameraState(NORMAL)
		, m_shakeOffset(Vector3::ZERO)
		, m_pushOffset(Vector3::ZERO)
	{
		m_shakeModule = EchoNew(CameraShakeModule(*this));
	}

	CameraMain::~CameraMain()
	{
		EchoSafeDelete(m_shakeModule, CameraShakeModule);
	}

	// 更新
	void CameraMain::frameMove(float elapsedTime)
	{
		m_shakeModule->frameMove(elapsedTime);
	}

	void CameraMain::setPosition(const Vector3& pos)
	{
		if (m_CameraState == NORMAL || m_CameraState == SHAKE)
		{
			m_position = pos + m_shakeOffset + m_pushOffset;
			m_bNeedUpdateView = true;
		}
	}

	// 设置摄像机振动偏移
	void CameraMain::setShakeOffset(const Vector3& oldPosition, const Vector3& offset)
	{
		m_shakeOffset = offset;
		m_position = oldPosition + offset;
		m_bNeedUpdateView = true;
	}

	void CameraMain::setPushOffset(const Vector3& oldPosition, const Vector3& offset)
	{
		m_pushOffset = offset;
		m_position = oldPosition + offset;
		m_bNeedUpdateView = true;
	}

	void CameraMain::setDirection(const Vector3& dir)
	{
		if (m_CameraState == NORMAL || m_CameraState == SHAKE)
		{
			m_dir = dir;
			m_dir.normalize();
			m_bNeedUpdateView = true;
		}
	}

	void CameraMain::setUp(const Vector3& vUp)
	{
		if (m_CameraState == NORMAL || m_CameraState == SHAKE)
		{
			m_up = vUp;
			m_bNeedUpdateView = true;
		}
	}

	void CameraMain::yaw(Real rad)
	{
		if (m_CameraState == NORMAL || m_CameraState == SHAKE)
		{
			Matrix4 matRot;
			matRot.rotateAxisReplace(m_fixedYawAxis, rad);

			m_dir = matRot.transform(m_dir);
			m_up = matRot.transform(m_up);
			m_bNeedUpdateView = true;
		}
	}

	void CameraMain::pitch(Real rad)
	{
		if (m_CameraState == NORMAL || m_CameraState == SHAKE)
		{
			Matrix4 matRot;
			matRot.rotateAxisReplace(m_right, rad);

			m_dir = matRot.transform(m_dir);
			m_up = matRot.transform(m_up);
			m_bNeedUpdateView = true;
		}
	}

	void CameraMain::roll(Real rad)
	{
		if (m_CameraState == NORMAL || m_CameraState == SHAKE)
		{
			Matrix4 matRot;
			matRot.rotateAxisReplace(m_dir, rad);

			m_dir = matRot.transform(m_dir);
			m_up = matRot.transform(m_up);
			m_bNeedUpdateView = true;
		}
	}

	void CameraMain::rotate(const Vector3& vAxis, Real rad)
	{
		if (m_CameraState == NORMAL || m_CameraState == SHAKE)
		{
			Matrix4 matRot;
			matRot.rotateAxisReplace(vAxis, rad);

			m_dir = matRot.transform(m_dir);
			m_up = matRot.transform(m_up);
			m_bNeedUpdateView = true;
		}
	}

	void CameraMain::move(const Vector3& offset)
	{
		if (m_CameraState == NORMAL || m_CameraState == SHAKE)
		{
			m_position += offset;
			m_bNeedUpdateView = true;
		}
	}

	// 克隆
	void CameraMain::clone(Camera* other)
	{
		Camera::clone(other);

		CameraMain* otheMain = ECHO_DOWN_CAST<CameraMain*>(other);
		m_CameraState = otheMain->m_CameraState;
		m_shakeModule = otheMain->m_shakeModule;
		m_shakeOffset = otheMain->m_shakeOffset;
	}

}
