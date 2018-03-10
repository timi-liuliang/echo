#pragma once

namespace Echo
{
	// 相机振动结构定义
	struct CameraShakeInfo
	{
		enum ShakeType
		{
			ST_OLD = 0,			// 默认方式
			ST_SQUARE  = 1,		// 方波
			ST_RHOMBUS = 2,		// 菱形波
			ST_LINEAR  = 3,		// 线性波
		};

		float beginTime;		// 开始时间
		float scale;			// 振动幅度
		float timelast;			// 剩余振动时间
		float duration;			// 总振动时间
		float shakeSpeed;		//
		int shakeTimes;			// 震动次数
		ShakeType shakeType;	// 类型

		CameraShakeInfo()
		{
			beginTime  = 0.f;
			scale      = 0.f;
			timelast   = 0.f;
			duration   = -1.0f;
			shakeSpeed = 0.f;
			shakeTimes = 0;
			shakeType = ST_OLD;
		}
	};

	// 摄像机拉近拉远
	struct CameraPush
	{	
		float beginTime;		// 开始时间
		float closeTime;		// 拉近时间	
		float durationTime;		// 持续时间	
		float farawayTime;		// 拉远时间		
		float scale;			// 振幅		
		bool  isInversion;		// 是否翻转	
		bool  isShow;			//是否显示		
		bool  isAddToAllActors;	//是否广播

		CameraPush()
		{
			beginTime    = 0.0f;
			scale        = 0.0f;
			durationTime = 0.0f;
			closeTime	 = 0.0f;
			farawayTime  = 0.0f;
			isInversion  = false; 
			isShow		 = true;
			isAddToAllActors = false;
		}
	};

	//摄像机拉近拉远定义
	struct CameraPushEvent
	{
		float beginTime;
		float closeTime;
		float durationTime;
		float farawayTime;
		float scale;
		float stopTime;
		float closeSpeed;
		float farawaySpeed;
		float curentScale;

		CameraPushEvent()
		{
			beginTime    = 0.0f;
			scale        = 0.0f;
			durationTime = 0.0f;
			closeTime	 = 0.0f;
			farawayTime  = 0.0f;
			closeSpeed   = 0.0f;
			farawaySpeed = 0.0f;
			curentScale  = 0.0f;
			stopTime     = 0.0f;
		}
	};

	/**
	 * 摄像机效果管理器 2014-6-13
	 * 处理摄像机震动,拉近拉远效果等
	 */
	class CameraMain;
	class CameraShakeModule
	{
	public:
		CameraShakeModule( CameraMain& camera);

		/** 更新(单位秒) */
		void frameMove( float delta); 

		/** 开始 */
		void start();

		/** 重置 */
		void stop();

		/** 添加拉近拉远 */
		void addCameraPush( const CameraPush* pData );

		/** 添加震动 */
		void addCameraShake(float beginTime, float scale, float durationTime, int shakeTimes, CameraShakeInfo::ShakeType type = CameraShakeInfo::ST_OLD);

		/** 摄像机震动判断 */
		bool isCameraShake() { return m_cameraShake.duration > 0.f || m_cameraPush.durationTime>0.f; }

	private:
		// 更新震动
		void updateShake( float delta);

		/** 更新摄像机移动 */
		void updatePush( float delta);

		void stopShake();

		void stopPush();

		//
		void updateDefaultShake(float delta);
		void updateSquareShake(float delta);

	private:
		//摄像机相关
		float					m_cameraShakeDelta;
		int						m_cameraShakeStep1;		// 摄像机震动步骤1
		int						m_cameraShakeStep2;		// 摄像机震动步骤2
		CameraShakeInfo			m_cameraShake;			// 摄像机震动
		float					m_cameraPushDelta;
		CameraPushEvent			m_cameraPush;			// 摄像机拉近拉远
		CameraMain*				m_camera;				// 所影响摄像机
		float					m_minShakeExtent;		// 震动范围
		float					m_maxShakeExtent;
		int						m_cameraShakedTimes; 
	private: 
		vector<Vector2>::type	m_shakeArray;			// 震动单元;	
	};
}