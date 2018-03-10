#pragma once

#include "Camera.h"
#include "CameraShakeModule.h"

namespace Echo
{
	/**
	 * 主摄像机
	 */
	class CameraMain: public Camera
	{
	public:
		enum CAMERA_STATE
		{
			NORMAL,
			SHAKE,
			ANIMA,
			LOCK,
			STATEMAX
		};

	public:
		CameraMain(ProjMode mode = PM_PERSPECTIVE, bool isFixedYaw = true);
		virtual ~CameraMain();

		// 更新
		virtual void frameMove(float elapsedTime);

		virtual void			setPosition(const Vector3& pos);
		//virtual void			setTarget(const Vector3& target);
		virtual void			setDirection(const Vector3& dir);
		virtual void			setUp(const Vector3& vUp);

		// 设置摄像机振动偏移
		void setShakeOffset(const Vector3& oldPosition, const Vector3& offset);

		void setPushOffset(const Vector3& oldPosition, const Vector3& offset);

		inline void setPosition_forEcho(const Vector3& pos)
		{
			m_position = pos;
			m_bNeedUpdateView = true;
		}

		inline void setTarget_forEcho(const Vector3& target)
		{
			m_dir = target - m_position;
			m_dir.normalize();

			m_bNeedUpdateView = true;
		}

		inline void setDirection_forEcho(const Vector3& dir)
		{
			EchoAssert(dir != m_up);

			m_dir = dir;
			m_bNeedUpdateView = true;
		}

		inline void setUp_forEcho(const Vector3& vUp)
		{
			m_up = vUp;
			m_bNeedUpdateView = true;
		}

		virtual void			yaw(Real rad);
		virtual void			pitch(Real rad);
		virtual void			roll(Real rad);
		virtual void			rotate(const Vector3& vAxis, Real rad);
		virtual void			move(const Vector3& offset);

		// 设置摄像机状态
		void setCameraState(ui32 state) { m_CameraState = state; }

		// 获取摄像机状态
		ui32 getCameraState() { return m_CameraState; }

		/** 获取震动模块 */
		CameraShakeModule* getShakeModule() { return m_shakeModule; }

	public:
		// 克隆
		virtual void clone(Camera* other);

	protected:
		ui32				m_CameraState;
		CameraShakeModule*	m_shakeModule;
		Vector3				m_shakeOffset;
		Vector3				m_pushOffset;
	};
}
