#include "FSAudioManager.h"
#include "FSAudioBase.h"
#include "Engine/Core.h"

namespace Echo
{
	// 为loadBankCustom提供的(数据流打开)回调函数
	FMOD_RESULT F_CALLBACK customFileOpen(const char* name, unsigned int *filesize, void **handle, void *userdata)
	{
		Echo::BankUserData* data = (Echo::BankUserData*)userdata;
		Echo::DataStream* stream = IO::instance()->open(data->filename.c_str());
		if (stream)
		{
			stream->seek(0, SEEK_END);
			size_t length = stream->tell();
			stream->seek(0, SEEK_SET);
			*filesize = (unsigned int)length;
			*handle = stream;
			return FMOD_OK;
		}
		EchoSafeDelete(stream, DataStream);
		return FMOD_ERR_FILE_BAD;
	}

	// 为loadBankCustom提供的(数据流关闭)回调函数
	FMOD_RESULT F_CALLBACK customFileClose(void *handle, void *userdata)
	{
		Echo::DataStream* stream = (Echo::DataStream*)handle;
		if (stream)
		{
			stream->close();
			EchoSafeDelete(stream, DataStream);
		}
		return FMOD_OK;
	}

	// 为loadBankCustom提供的(数据流读取)回调函数
	FMOD_RESULT F_CALLBACK customFileRead(void *handle, void* buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata)
	{
		*bytesread = 0;
		Echo::DataStream* stream = (Echo::DataStream*)handle;
		size_t read = stream->read(buffer, sizebytes);
		*bytesread = (unsigned int)read;

		if (read < sizebytes)
		{
			return FMOD_ERR_FILE_EOF;
		}

		return FMOD_OK;
	}

	// 为loadBankCustom提供的(数据流定位)回调函数
	FMOD_RESULT F_CALLBACK customFileSeek(void *handle, unsigned int pos, void *userdata)
	{
		Echo::DataStream* stream = (Echo::DataStream*)handle;
		stream->seek(pos, SEEK_SET);
		return FMOD_OK;
	}

	/**
	*针对Android平台下无法直接从APK通过文件路径来读取bank文件，并且若使用loadbankMemory会导致内存占用的上升,所以决定使用loadbankCustom
	**/
	struct AudioBank
	{
		FMOD::Studio::Bank* m_bank;		// FMODBank文件
		BankUserData userdata;
		bool decompress = FSAudioManager::instance()->NeedLoadDecompress();
		AudioBank(const String& bankFile)
		{
			bool isStream = true;
#ifdef ECHO_PLATFORM_ANDROID
			isStream = false;
#endif // ECHO_PLATFORM_ANDROID

			if (isStream)
			{
				IO::instance()->getFullPath(bankFile);
				userdata.filename = bankFile;

				FMOD_STUDIO_BANK_INFO bankinfo;
				memset(&bankinfo, 0, sizeof(bankinfo));
				bankinfo.size = sizeof(bankinfo);
				bankinfo.openCallback = customFileOpen;
				bankinfo.closeCallback = customFileClose;
				bankinfo.readCallback = customFileRead;
				bankinfo.seekCallback = customFileSeek;
				bankinfo.userData = (void*)&userdata;

				FS_ERRORCHECK(FSAudioManager::instance()->getStudioSystem()->loadBankCustom(&bankinfo, decompress ? FMOD_STUDIO_LOAD_BANK_DECOMPRESS_SAMPLES : FMOD_STUDIO_LOAD_BANK_NORMAL, &m_bank), nullptr);
			}
			else
			{
				MemoryReader memory = MemoryReader(bankFile);
				FS_ERRORCHECK(FSAudioManager::instance()->getStudioSystem()->loadBankMemory(memory.getData<const char*>(), memory.getSize(), FMOD_STUDIO_LOAD_MEMORY, decompress ? FMOD_STUDIO_LOAD_BANK_DECOMPRESS_SAMPLES : FMOD_STUDIO_LOAD_BANK_NORMAL, &m_bank), nullptr);
			}
		}

		~AudioBank()
		{
		}
	};

	__ImplementSingleton(FSAudioManager);

	// 构造函数
	FSAudioManager::FSAudioManager()
		: m_studioSystem(NULL)
		, m_lowLevelSystem(NULL)
		, m_SpeedAdjustEnable(false)
		, m_loadDecompress(false)
		, m_audioEventCallback(NULL)
	{
		__ConstructSingleton;
	}

	// 析构函数
	FSAudioManager::~FSAudioManager()
	{
		__DestructSingleton;
	}

	// 初始化
	void FSAudioManager::init(int maxVoribsCodecs, bool LoadDecompressed)
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		m_loadDecompress = LoadDecompressed;

		// 1.新建FMODE Studio System
		FS_ERRORCHECK(FMOD::Studio::System::create(&m_studioSystem), nullptr);

		// 2.lowlevelSystem
		FS_ERRORCHECK(m_studioSystem->getLowLevelSystem(&m_lowLevelSystem), nullptr);
		FS_ERRORCHECK(m_lowLevelSystem->setSoftwareFormat(0, FMOD_SPEAKERMODE_STEREO, NULL), nullptr);

		FMOD_ADVANCEDSETTINGS fmodSet = { sizeof(FMOD_ADVANCEDSETTINGS) };
		fmodSet.maxVorbisCodecs = maxVoribsCodecs;
		FS_ERRORCHECK(m_lowLevelSystem->setAdvancedSettings(&fmodSet), nullptr);
		// 3.channel
		FS_ERRORCHECK(m_studioSystem->initialize(maxVoribsCodecs, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_3D_RIGHTHANDED, NULL), nullptr);

		// 4.对流加载相关的设置
		// 设置读取bank的metadata的buffersize为128K,默认为2k会导致读取过去频繁
		FS_ERRORCHECK(m_lowLevelSystem->setFileSystem(NULL, NULL, NULL, NULL, NULL, NULL, 1024 * 128), nullptr);
		// 设置读取bank的Streamingdata的buffersize为64K,默认为16K，减少读取次数
		FS_ERRORCHECK(m_lowLevelSystem->setStreamBufferSize(64 * 1024, FMOD_TIMEUNIT_RAWBYTES), nullptr);
#endif
	}

	// 逻辑轮循函数
	void FSAudioManager::tick(ui32 elapsedTime)
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		// 系统更新
		FS_ERRORCHECK(m_studioSystem->update(), nullptr);

		// 移除非循环播放音源
		for (AudioSourceMap::iterator it = m_audioSources.begin(); it != m_audioSources.end();)
		{
			AudioSource* audioSource = it->second;
			if (!elapsedTime && audioSource->isPausable())
			{
				audioSource->pause();
			}
			if (elapsedTime && audioSource->isPausable())
			{
				audioSource->resume();
			}
			if (audioSource->isStopped() && audioSource->getCinfo().m_isOneShoot)
			{
				EchoSafeDelete(it->second, AudioSource);
				m_audioSources.erase(it++);
			}
			else
			{
				it++;
			}
		}
#endif
	}

	// 释放
	void FSAudioManager::release()
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		for (BankMap::iterator it = m_loadedBanks.begin(); it != m_loadedBanks.end(); it++)
		{
			EchoSafeDelete(it->second, AudioBank);
		}
		destroyAllAudioSources();

		m_loadedBanks.clear();

		FS_ERRORCHECK(m_studioSystem->release(), nullptr);
#endif
	}

	// 加载所有bank文件
	void FSAudioManager::loadAllBankFile()
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else

		// 获取所有bank文件
		StringArray bankFiles;
		IO::instance()->listFilesWithExt(bankFiles, ".bank");
		for (size_t i = 0; i<bankFiles.size(); i++)
		{
			loadBankFile(bankFiles[i].c_str());
		}
#endif
	}

	// 加载bank文件
	bool FSAudioManager::loadBankFile(const char* bankFile)
	{
#ifdef ECHO_PLATFORM_HTML5
		return false;
#else
		String key = bankFile;
		StringUtil::LowerCase(key);
		if (m_loadedBanks.find(key) == m_loadedBanks.end())
		{
			m_loadedBanks[key] = EchoNew(AudioBank(key));

			return true;
		}
		return false;
#endif
	}

	// 卸载bank文件
	bool FSAudioManager::unloadBankFile(const char* bankFile)
	{
#ifdef ECHO_PLATFORM_HTML5
		// return false
#else
		BankMap::iterator it = m_loadedBanks.find(bankFile);
		if (it != m_loadedBanks.end())
		{
			EchoSafeDelete(it->second, AudioBank);
			m_loadedBanks.erase(it);

			return true;
		}

		return false;
#endif
	}

	// 获取声源
	AudioSource* FSAudioManager::getAudioSource(AudioSourceID asID)
	{
#ifdef ECHO_PLATFORM_HTML5
		return NULL;
#else
		AudioSourceMap::iterator it = m_audioSources.find(asID);
		if (it != m_audioSources.end())
			return it->second;

		return NULL;
#endif
	}

	// 创建声源
	AudioSourceID FSAudioManager::createAudioSource(const AudioSource::Cinfo& cinfo)
	{
#ifdef ECHO_PLATFORM_HTML5
		return 0;
#else
		if (!isGroupEnabled(cinfo.m_group))
		{
			return 0;
		}

		AudioSource* audioSource = EchoNew(AudioSource(cinfo));
		m_audioSources[audioSource->getIdentifier()] = audioSource;

		return audioSource->getIdentifier();
#endif
	}

	// 销毁声源
	void FSAudioManager::destroyAudioSources(AudioSourceID* audioSourceIDs, int num)
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		for (int i = 0; i<num; i++)
		{
			AudioSourceMap::iterator it = m_audioSources.find(audioSourceIDs[i]);
			if (it != m_audioSources.end())
			{
				it->second->stop();
				EchoSafeDelete(it->second, AudioSource);
				m_audioSources.erase(it);
			}
		}
#endif
	}

	// 销毁所有声源
	void FSAudioManager::destroyAllAudioSources()
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		// 移除非循环播放音源
		for (AudioSourceMap::iterator it = m_audioSources.begin(); it != m_audioSources.end(); it++)
		{
			it->second->stop();
			EchoSafeDelete(it->second, AudioSource);
		}

		m_audioSources.clear();
#endif
	}

	// 设置听者属性
	void FSAudioManager::setListenerAttributes(const Audio3DAttributes& attributes)
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		// 设置听者属性
		m_listenerAttributes = attributes;
		m_listenerAttributes.m_forward.y = 0.f;
		m_listenerAttributes.m_forward.normalize();

		EchoAssert(sizeof(m_listenerAttributes) == sizeof(FMOD_3D_ATTRIBUTES));
		FS_ERRORCHECK(m_studioSystem->setListenerAttributes(0, (FMOD_3D_ATTRIBUTES*)&m_listenerAttributes), nullptr);

		// 设置所有2D音源(2D模式播放的3D音源)属性位置与听者一致
		for (AudioSourceMap::iterator it = m_audioSources.begin(); it != m_audioSources.end(); it++)
		{
			AudioSource* audioSource = it->second;
			if (!audioSource->is3DMode() && audioSource->is3D())
			{
				audioSource->set3dAttributes(m_listenerAttributes);
			}
		}
#endif
	}

	// 根据标签获取音源
	void FSAudioManager::listAudioSourcesByGroup(AudioSourceIdArray& oIds, int groupFilter)
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		oIds.clear();

		// 移除非循环播放音源
		for (AudioSourceMap::iterator it = m_audioSources.begin(); it != m_audioSources.end(); it++)
		{
			if (it->second->getCinfo().m_group & groupFilter)
			{
				oIds.push_back(it->first);
			}
		}
#endif
	}

	// 判断组是否可用
	bool FSAudioManager::isGroupEnabled(int group) const
	{
#ifdef ECHO_PLATFORM_HTML5
		return false;
#else
		map<int, bool>::type::const_iterator it = m_isGroupEnabled.find(group);
		if (it != m_isGroupEnabled.end())
		{
			return it->second;
		}

		return true;
#endif
	}

	// 设置声音组音量音子
	void FSAudioManager::setGroupVolumeFactor(int group, float volumeFactor)
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		volumeFactor = Math::Clamp(volumeFactor, 0.f, 1.f);
		m_groupVolumeFactor[group] = volumeFactor;

		AudioSourceIdArray ids;
		listAudioSourcesByGroup(ids, group);
		for (size_t i = 0; i<ids.size(); i++)
		{
			AudioSource* source = getAudioSource(ids[i]);
			if (source)
			{
				source->setVolume(source->getCinfo().m_volume);
			}
		}
#endif
	}

	// 获取声音组音量音子
	float FSAudioManager::getGroupVolumeFactor(int group) const
	{
#ifdef ECHO_PLATFORM_HTML5
		return 1.f;
#else
		map<int, float>::type::const_iterator it = m_groupVolumeFactor.find(group);
		if (it != m_groupVolumeFactor.end())
		{
			return it->second;
		}

		return 1.f;
#endif
	}

	// 根据bank文件名获取事件
	void FSAudioManager::getAudioEvents(StringArray& oEvents, const char* bankName)
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		BankMap::iterator it = m_loadedBanks.find(bankName);
		if (it != m_loadedBanks.end())
		{
			FMOD::Studio::Bank* bank = it->second->m_bank; EchoAssert(bank);

			// 获取事件数量
			int eventCount = 0;
			FS_ERRORCHECK(bank->getEventCount(&eventCount), nullptr);
			if (eventCount)
			{
				// 获取事件描述
				int getCount = 0;
				vector<FMOD::Studio::EventDescription*>::type eventDescriptions;
				eventDescriptions.resize(eventCount);
				FS_ERRORCHECK(bank->getEventList(eventDescriptions.data(), eventCount, &getCount), nullptr);
				EchoAssert(getCount == eventCount);

				// 获取事件名称
				char eventName[512];
				for (int i = 0; i<getCount; i++)
				{
					FS_ERRORCHECK(eventDescriptions[i]->getPath(eventName, 512, NULL), nullptr);
					oEvents.push_back(eventName);
				}
			}
		}
#endif
	}

	// 获取所有事件
	void FSAudioManager::getAllAudioEvent(StringArray& oEvents)
	{
#ifdef ECHO_PLATFORM_HTML5
		oEvents.clear();
#else
		oEvents.clear();
		for (BankMap::iterator it = m_loadedBanks.begin(); it != m_loadedBanks.end(); it++)
		{
			getAudioEvents(oEvents, it->first.c_str());
		}
#endif
	}

	//挂起Fmod声音系统
	void FSAudioManager::suspendFmodSystem()
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		if (m_lowLevelSystem)
		{
			m_lowLevelSystem->mixerSuspend();
		}
#endif
	}

	//从挂起中恢复Fmod声音系统
	void FSAudioManager::resumeFmodSystem()
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		if (m_lowLevelSystem)
		{
			m_lowLevelSystem->mixerResume();
		}
#endif
	}

	// 加载事件缓存

	void FSAudioManager::cacheEvent(FMOD::Studio::EventDescription* desc)
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		if (!desc)
		{
			EchoLogError("in function FSAudioManager::cacheEvent, FMOD::Studio::EventDescription* is nullptr.");
			return;
		}

		if (m_audioSampleSet.find(desc) == m_audioSampleSet.end())
		{
			bool isStream;
			FS_ERRORCHECK(desc->isStream(&isStream), nullptr);
			if (!isStream)
			{
				desc->loadSampleData();
				m_audioSampleSet.insert(desc);
			}
		}
#endif
	}

	// 释放全部事件缓存
	void FSAudioManager::clearEventCaches()
	{
#ifdef ECHO_PLATFORM_HTML5
		// do nothing
#else
		if (!m_audioSampleSet.empty())
		{
			AudioSampleSet::iterator it;
			for (it = m_audioSampleSet.begin(); it != m_audioSampleSet.end(); it++)
			{
				(*it)->unloadSampleData();
			}
			m_audioSampleSet.clear();
		}
#endif
	}
}