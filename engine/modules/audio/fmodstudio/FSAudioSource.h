#pragma once

#include "FSAudioBase.h"

namespace Echo
{
	class AudioSource;

	// 事件回调类型
	enum AudioSouceEventCBType
	{
		AS_CreateFinished,			// 创建完成
		AS_Unknown,					// 未知
	};
	typedef void(*AudioSourceEventCB)(AudioSouceEventCBType type, AudioSource* audioSource);

	// 音频3D属性
	struct Audio3DAttributes
	{
		Vector3		m_position;		// 位置
		Vector3		m_velocity;		// 位移速度
		Vector3		m_forward;		// 朝向(必须为单位向量,必须垂直于上方向)
		Vector3		m_up;			// 上方向

		Audio3DAttributes()
			: m_position( Vector3::ZERO)
			, m_velocity( Vector3::ZERO)
			, m_forward( Vector3::UNIT_Z)
			, m_up( Vector3::UNIT_Y)
		{}
	};

	/**
	 * 音源
	 */
	class AudioSource
	{
		friend class FSAudioManager;
	public:
		// 加载类型
		enum LoadType
		{
			LT_Normal,		// 阻塞加载
			LT_NoBlocking,	// 流式加载,边加载边播放
		};

		// 声音属性
		enum SoundProperty
		{
			SP_MINI_DISTANCE = 1,
			SP_MAX_DISTANCE,
			SP_FINAL_PROPERTY,  //结尾属性，无任何意义
		};
		
		// 声音参数
		struct ParamValue
		{
			String	m_paramName;		// 参数名
			float	m_value;			// 参数值
		};
		typedef vector<ParamValue>::type ParamValues;

		// 构建信息
		struct Cinfo
		{
			String		m_name;				// 事件名称
			float		m_volume;			// 初始声音大小
			LoadType	m_loadType;			// 资源加载方式(阻塞非阻塞)
			bool		m_is3DMode;			// 是否强制以2D模式播放
			float		m_minDistane;		// 声音大小衰减起始距离
			float		m_maxDistance;		// 声音大小衰减结束距离
			Vector3		m_pos;				// 位置
			ui32		m_group;			// 层级(分批控制声音)
			bool		m_isOneShoot;		// 是否单次播放(相对于one shoot)
			ParamValues m_initParamValues;	// 声音初始参数值
			bool		m_isUseCB;			// 是否执行注册使用主回调函数
			bool		m_pausable;			// 是否可暂停
			Cinfo() 
				: m_volume( 1.f)
				, m_is3DMode( true)
				, m_pos( Vector3::ZERO)
				, m_minDistane( 0.f)
				, m_maxDistance( 20.f)
				, m_loadType( LT_Normal)
				, m_group( 0)
				, m_isOneShoot( true)
				, m_isUseCB(false)
				, m_pausable(false)
			{}
		};

	public:
		// 获取唯一标识符
		ui32 getIdentifier() const { return m_identifier; }

		// 暂停
		void pause();

		// 继续播放
		void resume();

		// 停止
		void stop();

		// 是否是3d音效
		bool is3D() const { return m_is3D; }

		// 是否以2D模式播放
		bool is3DMode() const { return m_info.m_is3DMode; }

		// 是否已停止
		bool isStopped();

		bool isPausable() const { return m_info.m_pausable; }

		// 设置音量大小
		void setVolume( float volume);

		// 设置3D属性			
		void set3dAttributes( const Audio3DAttributes& attributes);

		// 获取3D属性
		const Audio3DAttributes& get3dAttributes();

		// 获取初始构建信息
		const Cinfo& getCinfo() const { return m_info; }

		// 获取参数值
		void getParameterValue( const char* name, float& oValue);

		// 获取参数范围
		void getParameterRange( const char* name, float& oRangeMin, float& oRangeMax);

		// 设置参数值
		void setParameterValue( const char* name, float value);

		// 设置属性值
		void setProperty(SoundProperty sp_type, float value);

		// 获得属性值
		void getProperty(SoundProperty sp_type, float& oValue);

		// 获得长度
		int getLenth();

	private:
		AudioSource( const Cinfo& cinfo);
		~AudioSource();

	private:
		AudioSourceID					m_identifier;	// 唯一标识符
		FMOD::Studio::EventInstance*	m_eventInstance;// 声源实例
		Cinfo							m_info;			// 构建信息
		bool							m_is3D;			// 是否是3D声源
		Audio3DAttributes				m_3dAttributes;	// 3D属性
	};
}