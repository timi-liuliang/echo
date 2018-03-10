#pragma once

#include "FSAudioSource.h"
#include "engine/core/Util/Singleton.h"

namespace Echo
{
	/**
	* 基于FMOD Studio实现的音频管理器
	*/
	struct BankUserData
	{
		std::string filename;
	};

	struct AudioBank;
	class FSAudioManager
	{
		__DeclareSingleton(FSAudioManager);

	public:
		typedef EchoHashMap<String, AudioBank*> BankMap;
		typedef EchoHashMap<AudioSourceID, AudioSource*> AudioSourceMap;
		typedef EchoHashSet<FMOD::Studio::EventDescription*> AudioSampleSet;
		typedef vector<AudioSourceID>::type	AudioSourceIdArray;
	public:
		FSAudioManager();
		virtual ~FSAudioManager();

		// 初始化
		virtual void init(int, bool);

		// 逻辑轮循函数
		virtual void tick(ui32 elapsedTime);

		// 释放
		virtual void release();

		// 加载所有bank文件
		void loadAllBankFile();

		// 获取声源
		AudioSource* getAudioSource(AudioSourceID asID);

		// 获取所有声源
		const AudioSourceMap& getAllAduioSources() { return m_audioSources; }

		// 创建声源
		AudioSourceID createAudioSource(const AudioSource::Cinfo& cinfo);

		// 销毁声源
		void destroyAudioSources(AudioSourceID* audioSourceIDs, int num);

		// 销毁所有声源
		void destroyAllAudioSources();

		// 设置听者属性
		void setListenerAttributes(const Audio3DAttributes& attributes);

		// 获取听者属性
		const Audio3DAttributes& getListenerAttributes() { return m_listenerAttributes; }

		// 根据标签获取音源
		void listAudioSourcesByGroup(AudioSourceIdArray& oIds, int groupFilter);

		// 设置组是否可用
		void setGroupEnabled(int group, bool isEnabled) { m_isGroupEnabled[group] = isEnabled; }

		// 判断组是否可用
		bool isGroupEnabled(int group) const;

		// 设置声音组音量音子
		void setGroupVolumeFactor(int group, float volueFactor);

		// 获取声音组音量音子
		float getGroupVolumeFactor(int group) const;

		// 获取系统
		FMOD::Studio::System* getStudioSystem() { return m_studioSystem; }

		// 根据bank文件名获取事件
		void getAudioEvents(StringArray& oEvents, const char* bankName);

		// 获取所有事件
		void getAllAudioEvent(StringArray& oEvents);

		//挂起Fmod声音系统
		void suspendFmodSystem();

		//从挂起中恢复Fmod声音系统
		void resumeFmodSystem();

		//设置速度调节是否可用
		void setSpeedAdjustEnable(bool isEnable){ m_SpeedAdjustEnable = isEnable; }

		// 缓存事件
		void cacheEvent(FMOD::Studio::EventDescription* desc);

		// 释放全部事件缓存
		void clearEventCaches();

		// 加载时是否解压缩
		bool NeedLoadDecompress(){ return m_loadDecompress; }

		// 注册回调函数
		void setAudioEventCb(AudioSourceEventCB cb) { m_audioEventCallback = cb; }

		// 获取回调函数
		AudioSourceEventCB getAudioEventCb() { return m_audioEventCallback; }

	private:
		// 加载bank文件
		bool loadBankFile(const char* bankFile);

		// 卸载bank文件
		bool unloadBankFile(const char* bankFile);

	private:
		FMOD::Studio::System*	m_studioSystem;			// 高级FMOD Studio系统
		FMOD::System*			m_lowLevelSystem;		// 低级FMOD系统
		BankMap					m_loadedBanks;			// 已加载的bank资源
		AudioSourceMap			m_audioSources;			// 所有音源
		Audio3DAttributes		m_listenerAttributes;	// 听者属性
		map<int, bool>::type	m_isGroupEnabled;		// 判断组是否可用
		map<int, float>::type	m_groupVolumeFactor;	// 组音量大小音子
		bool					m_SpeedAdjustEnable;	// 是否开启时间调节
		AudioSampleSet			m_audioSampleSet;		// 事件缓存
		bool					m_loadDecompress;		// 是否在加载完后进行提前解码，避免实时解码对某些机型的CPU消耗
		AudioSourceEventCB		m_audioEventCallback;	// 声音事件回调
	};

	struct AudioExtraInfo
	{
		int lastTime;	//上一帧停到的时间
		bool firstTime; //是否第一次进入加速逻辑
	};
}