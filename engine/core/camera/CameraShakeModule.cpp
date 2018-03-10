#include "CameraMain.h"
#include "CameraShakeModule.h"
#include "Engine/core/Scene/Scene_Manager.h"

namespace Echo
{
	/** 构造函数 */
	CameraShakeModule::CameraShakeModule( CameraMain& camera)
		: m_camera( &camera)
		, m_cameraShakeStep1(0)
		, m_cameraShakeStep2(0)
		, m_cameraShakeDelta(0.f)
		, m_cameraPushDelta(0.f)
		, m_minShakeExtent( -10.f)
		, m_maxShakeExtent( 10.f)
		, m_cameraShakedTimes(0)
	{}

	/** 更新(单位秒) */
	void CameraShakeModule::frameMove( float delta)
	{
		updateShake( delta);
		updatePush( delta);
	}

	// 更新震动
	void CameraShakeModule::updateShake( float delta)
	{
		if( m_cameraShake.duration < 0.f)
			return;

		m_cameraShake.beginTime -= delta;
		if(m_cameraShake.beginTime > 0.0f)
			return;

		m_cameraShakeDelta += delta;
		if(m_cameraShakeDelta < 0.033f)
			return;
		else
			m_cameraShakeDelta -= 0.033f;

		// 执行震动
		switch (m_cameraShake.shakeType)
		{
		case CameraShakeInfo::ST_OLD:
			updateDefaultShake(delta); break;

		case CameraShakeInfo::ST_SQUARE:
		case CameraShakeInfo::ST_RHOMBUS:
		case CameraShakeInfo::ST_LINEAR:
			updateSquareShake(delta); break;
		default:
			break;
		}
	}

	void CameraShakeModule::updateDefaultShake(float delta)
	{
		//获取实例中摄像机位子
		Vector3 oldcamerapos = m_camera->getPosition();

		// 超时清除
		if (m_cameraShake.timelast < 0 && m_cameraShakeStep1 == 0 && m_cameraShakeStep2 == 0)
		{
			m_cameraShake.timelast = m_cameraShake.duration;
			m_cameraShake.shakeTimes -= 1;
			if (m_cameraShake.shakeTimes == 0)
			{
				stopShake();
			}
		}
		else
		{
			// 计算偏移
			Vector3 deltapos;
			if (m_cameraShake.timelast > m_cameraShake.duration * 0.6f)
			{
				deltapos = m_camera->getDirection()* Math::Clamp<float>(m_cameraShake.shakeSpeed * 0.3f, m_minShakeExtent, m_maxShakeExtent);
				m_camera->setShakeOffset(oldcamerapos, deltapos);
				++m_cameraShakeStep1;
			}
			else if (m_cameraShake.timelast > m_cameraShake.duration * 0.4f)
			{
				deltapos = m_camera->getDirection()* Math::Clamp<float>(m_cameraShake.shakeSpeed * 0.4f, m_minShakeExtent, m_maxShakeExtent);
				m_camera->setShakeOffset(oldcamerapos, -deltapos);
				++m_cameraShakeStep2;
			}
			else if (m_cameraShakeStep2)
			{
				deltapos = m_camera->getDirection()* Math::Clamp<float>(m_cameraShake.shakeSpeed * 0.4f, m_minShakeExtent, m_maxShakeExtent);
				m_camera->setShakeOffset(oldcamerapos, deltapos);
				--m_cameraShakeStep2;
			}
			else if (m_cameraShakeStep1)
			{
				if (m_cameraShakeStep1 > 1)
				{
					deltapos = m_camera->getDirection()* Math::Clamp<float>(m_cameraShake.shakeSpeed * 0.6f, m_minShakeExtent, m_maxShakeExtent);
					m_camera->setShakeOffset(oldcamerapos, -deltapos);
					m_cameraShakeStep1 -= 2;
					if (!m_cameraShakeStep1)
					{
						m_cameraShake.timelast = 0.f;
					}

				}
				else if (m_cameraShakeStep1 == 1)
				{
					deltapos = m_camera->getDirection()* Math::Clamp<float>(m_cameraShake.shakeSpeed*0.3f, m_minShakeExtent, m_maxShakeExtent);
					m_camera->setShakeOffset(oldcamerapos, -deltapos);
					--m_cameraShakeStep1;
					m_cameraShake.timelast = 0.f;
				}
			}
		}

		m_cameraShake.timelast -= 0.033f;
	}

	void CameraShakeModule::updateSquareShake(float dt)
	{
		if (m_cameraShakedTimes >= m_cameraShake.shakeTimes)
		{
			stopShake(); 
			return; 
		}

		//获取实例中摄像机位子
		Vector3 pos = m_camera->getPosition();
		Vector3 dir = m_camera->getDirection(); dir.y = 0; dir.normalize();
		Vector3 right = dir.cross(Vector3::UNIT_Y); right.normalize();

		// 计算波段;
		auto moveIdx = m_cameraShakedTimes++ % m_shakeArray.size();
		auto moveDir = m_shakeArray[moveIdx]; 

		Vector3 deltaDir = dir * moveDir.y - right * moveDir.x; deltaDir.normalize(); 
		Vector3 deltaPos = deltaDir * moveDir.len(); 

		// 更新位置
		m_camera->setShakeOffset(pos, deltaPos);
	}

	/** 更新摄像机移动 */
	void CameraShakeModule::updatePush( float delta)
	{
		if(m_cameraPush.durationTime <= 0.0f)
			return;

		m_cameraPushDelta += delta;
		if(m_cameraPushDelta < 0.033f)
		{
			return;
		}
		else
		{
			m_cameraPushDelta -= 0.033f;
		}

		//获取实例中摄像机位子
		const Vector3& oldcamerapos = m_camera->getPosition();

		// 超时清除
		if(m_cameraPush.farawayTime <= 0.0f)
		{
			stopPush();
		}
		else
		{
			// 计算偏移
			Vector3 deltapos;
			if( m_cameraPush.closeTime > 0.0f)
			{
				deltapos = m_camera->getDirection()* m_cameraPush.closeSpeed;
				m_camera->setPushOffset(oldcamerapos, deltapos);
				m_cameraPush.curentScale += m_cameraPush.closeSpeed;
				m_cameraPush.closeTime -= 0.033f;
			}
			else if( m_cameraPush.stopTime > 0.0f)
			{
				m_cameraPush.stopTime -= 0.033f;
			}
			else if(m_cameraPush.farawayTime > 0.0f)
			{
				m_cameraPush.farawayTime -= 0.033f;
				if(m_cameraPush.farawayTime > 0.0f)
				{
					deltapos = m_camera->getDirection() * m_cameraPush.farawaySpeed;
					m_camera->setPushOffset(oldcamerapos, -deltapos);
					m_cameraPush.curentScale -= m_cameraPush.farawaySpeed;
				}
				else
				{
					deltapos = m_camera->getDirection() * m_cameraPush.curentScale;
					m_camera->setPushOffset(oldcamerapos, -deltapos);
				}
			}
		}
	}

	/** 开始 */
	void CameraShakeModule::start()
	{
		switch (m_cameraShake.shakeType)
		{
		case CameraShakeInfo::ST_OLD: break; 
		case CameraShakeInfo::ST_SQUARE:  m_shakeArray = { Vector2(m_cameraShake.scale, 0), Vector2(0, m_cameraShake.scale), Vector2(-m_cameraShake.scale, 0), Vector2(0, -m_cameraShake.scale) }; break;
		case CameraShakeInfo::ST_RHOMBUS: m_shakeArray = { Vector2(m_cameraShake.scale, m_cameraShake.scale), Vector2(-m_cameraShake.scale, m_cameraShake.scale), Vector2(-m_cameraShake.scale, -m_cameraShake.scale), Vector2(m_cameraShake.scale, -m_cameraShake.scale) }; break;
		case CameraShakeInfo::ST_LINEAR:  m_shakeArray = { Vector2(m_cameraShake.scale, 0), Vector2(-m_cameraShake.scale, 0) }; break;
		default:
			break;
		}
	}

	/** 重置 */
	void CameraShakeModule::stop()
	{
		stopShake();
		stopPush();
	}

	void CameraShakeModule::stopShake()
	{
		Vector3 oldcamerapos = m_camera->getPosition();
		m_camera->setShakeOffset(oldcamerapos, Vector3::ZERO);
		m_cameraShakeDelta = 0.f;
		m_cameraShake.duration = -1.f;
		m_cameraShakeStep1 = 0;
		m_cameraShakeStep2 = 0;
		m_cameraShakedTimes = 0;
		if (m_cameraPush.durationTime <= 0.f)
		{
			m_camera->setCameraState(CameraMain::NORMAL);
		}
	}

	void CameraShakeModule::stopPush()
	{
		Vector3 oldcamerapos = m_camera->getPosition();
		m_camera->setPushOffset(oldcamerapos, Vector3::ZERO);
		m_cameraPush.durationTime = 0.0f;
		m_cameraPush.curentScale = 0.0f;
		m_cameraPushDelta = 0.f;
		m_cameraPush.durationTime = -1.f;
		if (m_cameraShake.duration <= 0.f)
		{
			m_camera->setCameraState(CameraMain::NORMAL);
		}
	}

	void CameraShakeModule::addCameraPush( const CameraPush* pData )
	{
		if( pData->durationTime > m_cameraPush.durationTime)
		{
			stopPush();

			m_camera->setCameraState( CameraMain::SHAKE);
			if( pData->isInversion)
			{
				m_cameraPush.durationTime = pData->durationTime;
				m_cameraPush.closeTime    = pData->farawayTime;
				m_cameraPush.farawayTime  = pData->closeTime;
				m_cameraPush.scale        = pData->scale;
				m_cameraPush.stopTime     = pData->durationTime - pData->farawayTime - pData->closeTime;
				m_cameraPush.closeSpeed   = - pData->scale / pData->farawayTime * 0.033f / ( m_camera->getFov() * 10);
				m_cameraPush.farawaySpeed = - pData->scale / pData->closeTime * 0.033f / ( m_camera->getFov() * 10);
			}
			else
			{
				m_cameraPush.durationTime = pData->durationTime;
				m_cameraPush.closeTime    = pData->closeTime;
				m_cameraPush.farawayTime  = pData->farawayTime;
				m_cameraPush.scale        = pData->scale;
				m_cameraPush.stopTime     = pData->durationTime - pData->farawayTime - pData->closeTime;
				m_cameraPush.closeSpeed   = pData->scale / pData->closeTime * 0.033f / ( m_camera->getFov() * 10);
				m_cameraPush.farawaySpeed = pData->scale / pData->farawayTime * 0.033f / ( m_camera->getFov() * 10);
			}
		}	
	}

	/** 添加震动 */
	void CameraShakeModule::addCameraShake(float beginTime, float scale, float durationTime, 
				int shakeTimes, CameraShakeInfo::ShakeType type /* = CameraShakeInfo::ST_OLD */)
	{
		if (durationTime > m_cameraShake.duration)
		{
			stopShake();

			m_camera->setCameraState( CameraMain::SHAKE);
			m_cameraShake.beginTime  = beginTime;
			m_cameraShake.scale	     = scale;
			m_cameraShake.duration   = durationTime;
			m_cameraShake.timelast   = durationTime;
			m_cameraShake.shakeSpeed = scale / durationTime / ( m_camera->getFov() * 10);
			m_cameraShake.shakeTimes = shakeTimes;
			m_cameraShake.shakeType  = type; 

			start();
		}
	}


}