#include "EffectSystem.h"
#include "EffectLayer.h"
#include "EffectSystemManager.h"
#include "engine/core/Util/Exception.h"
#include "Render/Image.h"
#include "Engine/core/Scene/Node.h"
#include "engine/core/resource/DataStream.h"

namespace Echo
{
	const i32		EffectSystem::DEFAULT_TIME_STEP = 100;
	const float		EffectSystem::DEFAULT_SCALE_VELOCITY = 1.0f;
	const float		EffectSystem::DEFAULT_SCALE_TIME = 1.0f;

	// 构造函数
	EffectSystem::EffectSystem(const String& name, bool isUIeffect)
		: mName(name)
		, mTemplateName(StringUtil::BLANK)
		, mState(PSS_CREATED)
		, mTimeElapsedSinceStart(0)
		, mDelayTimeTick(0)
		, mIsPreparing(false)
		, mIsDestroying(false)
		, mEffectSystemScaleTime(DEFAULT_SCALE_TIME)
		, mEffectSystemColor(Color::WHITE)
		, mAABB()
		, mCurrentCamera(NULL)
		, mParentNode(NULL)
		, mbEffectSystemShowVisualHelp(false)
		, mbIsVisible(true)
		, mbIsUIEffect(isUIeffect)
		, mbIsCallStartUnormal(false)
		, mbIsRealPlaying(false)
		, mCurDelta(0.f)
		, mSchedule(0)
	{
		// 以1开始计数
		static int identifier = 1;
		m_identifier = identifier++;
	}

	// 析构函数
	EffectSystem::~EffectSystem(void)
	{
		unprepare_sys();
		destroyAllLayers();
	}
	EffectSystem& EffectSystem::operator=(const EffectSystem& ps)
	{
		destroyAllLayers();

		// copy all layers
		size_t i = 0;
		EffectLayer* psLayer = 0;
		EffectLayer* clonedLayer = 0;
		for (i = 0; i < ps.getNumLayers(); ++i)
		{
			psLayer = ps.getLayer(i);
			clonedLayer = EffectSystemManager::instance()->cloneLayer(psLayer);
			addLayer(clonedLayer);
		}

		mTemplateName = ps.mTemplateName;
		mAABB = ps.mAABB;

		// copy fallowpair
		mFallowPair.clear();
		mFallowPair.reserve(ps.mFallowPair.size());
		mFallowPair.resize(ps.mFallowPair.size());
		mFallowPair = ps.mFallowPair;

		mbEffectSystemShowVisualHelp = ps.mbEffectSystemShowVisualHelp;

		return *this;
	}
	//-----------------------------------------------------------------------
	EffectLayer* EffectSystem::createLayer(EFFECT_LAYER_TYPE layerType)
	{
		EffectLayer* Layer = EffectSystemManager::instance()->createLayer(layerType);
		addLayer(Layer);
		return Layer;
	}
	//-----------------------------------------------------------------------
	void EffectSystem::addLayer(EffectLayer* layer)
	{
		mLayers.push_back(layer);
		layer->setParentSystem(this);
	}
	//-----------------------------------------------------------------------
	EffectLayer* EffectSystem::getLayer(size_t index) const
	{
		assert(index < mLayers.size() && "Layer index out of bounds!");
		return mLayers[index];
	}
	//-----------------------------------------------------------------------
	EffectLayer* EffectSystem::getLayer(const String &LayerName) const
	{
		if (StringUtil::BLANK == LayerName)
		{
			return 0;
		}

		EffectLayerList::const_iterator it;
		for (it = mLayers.begin(); it != mLayers.end(); ++it)
		{
			if ((*it)->getName() == LayerName)
			{
				return *it;
			}
		}

		return 0;
	}
	//-----------------------------------------------------------------------
	size_t EffectSystem::getNumLayers(void) const
	{
		return mLayers.size();
	}
	//-----------------------------------------------------------------------
	EffectSystem::EffectLayerList EffectSystem::getLayers() const
	{
		return mLayers;
	}
	//-----------------------------------------------------------------------
	void EffectSystem::destroyLayer(EffectLayer *Layer)
	{
		assert(Layer && "Layer is null!");
		EffectLayerIterator it;
		for (it = mLayers.begin(); it != mLayers.end(); ++it)
		{
			if ((*it) == Layer)
			{
				EffectSystemManager::instance()->destroyLayer(*it);
				mLayers.erase(it);
				break;
			}
		}
	}
	//-----------------------------------------------------------------------
	void EffectSystem::destroyLayer(size_t index)
	{
		destroyLayer(getLayer(index));
	}
	//-----------------------------------------------------------------------
	void EffectSystem::destroyAllLayers(void)
	{
		EffectLayerIterator it;
		for (it = mLayers.begin(); it != mLayers.end(); ++it)
		{
			EffectSystemManager::instance()->destroyLayer(*it);
		}
		mLayers.clear();
	}

	float EffectSystem::getEffectTimeLength(void) const
	{
		float maxLength = 0.0f;
		float currentLength;

		EffectLayerConstIterator it;
		for (it = mLayers.begin(); it != mLayers.end(); ++it)
		{
			const EffectLayer* pLayer = *it;
			currentLength = pLayer->getLayerTimeLength();
			if (0.0f > currentLength)
			{
				return -1.0f;
			}
			if (currentLength > maxLength)
			{
				maxLength = currentLength;
			}
		}

		return maxLength;
	}
	//-----------------------------------------------------------------------
	const Echo::vector<ui32>::type& EffectSystem::getFallowPair() const
	{
		return mFallowPair;
	}
	//-----------------------------------------------------------------------
	String EffectSystem::getFallowPairString() const
	{
		String result;

		for (size_t i = 0; i < mFallowPair.size(); ++i)
		{
			String strIndex = StringUtil::ToString(mFallowPair[i]);

			if (i != mFallowPair.size() - 1)
			{
				strIndex += "_";
			}
			result += strIndex;
		}

		return result;
	}
	//-----------------------------------------------------------------------
	void EffectSystem::setFallowPair(const Echo::vector<ui32>::type &pairVector)
	{
		mFallowPair.clear();

		for (size_t i = 0; i < pairVector.size(); i += 2)
		{
			ui32 fallow = pairVector[i];
			ui32 fallowed = pairVector[i + 1];

			mFallowPair.push_back(fallow);
			mFallowPair.push_back(fallowed);
		}
	}
	//-----------------------------------------------------------------------
	void EffectSystem::setFallowPair(const String &pairString)
	{
		StringArray pairsStringVector = StringUtil::Split(pairString, "_");

		UbiLayerFallowPair mTempPair;
		mTempPair.reserve(pairsStringVector.size());
		mTempPair.resize(pairsStringVector.size());
		for (size_t i = 0; i < pairsStringVector.size(); ++i)
		{
			mTempPair[i] = StringUtil::ParseInt(pairsStringVector[i]);
		}
		setFallowPair(mTempPair);
	}
	//-----------------------------------------------------------------------
	void EffectSystem::setExtenalParam(const Vector3& p1, const Vector3& p2)
	{
		EffectLayerIterator it;
		for (it = mLayers.begin(); it != mLayers.end(); ++it)
		{
			(*it)->_notifyExtenalParam(p1, p2);
		}
	}

	void EffectSystem::setExtenalParamWithXYZ(Real x1, Real y1, Real z1, Real x2, Real y2, Real z2)
	{
		Vector3 p1(x1, y1, z1);
		Vector3 p2(x2, y2, z2);
		setExtenalParam(p1, p2);
	}

	//-----------------------------------------------------------------------
	void EffectSystem::handleDelayQueue()
	{
		EffectDelayQueue::reverse_iterator it;
		for (it = mDelayQueue.rbegin(); it != mDelayQueue.rend(); ++it)
		{
			DelayQueueItem& item = *it;
			if (item.mType == DQT_UNPREPARE)
			{
				unprepare_sys();
				break;
			}
			else if (item.mType == DQT_STOP)
			{
				start();
				stop();
				break;
			}
			else if (item.mType == DQT_START)
			{
				i32 delayUpdateTime = mDelayTimeTick - item.mTime;

				if (mSchedule > 0)
				{
					schedule(mSchedule - delayUpdateTime);
					mDelayTimeTick = 0;
					break;
				}
				start();

				if (delayUpdateTime < DEFAULT_TIME_STEP / 10)
				{
					_update_sys(delayUpdateTime);
					mDelayTimeTick = 0;
					break;
				}

				delayUpdateTime -= DEFAULT_TIME_STEP / 10;
				_update_sys(DEFAULT_TIME_STEP / 10);
				while (delayUpdateTime > DEFAULT_TIME_STEP)
				{
					delayUpdateTime -= DEFAULT_TIME_STEP;
					_update_sys(DEFAULT_TIME_STEP);
				}
				if (delayUpdateTime>0.0f)
					_update_sys(delayUpdateTime);

				mDelayTimeTick = 0;
				break;
			}
			else if (item.mType == DQT_PAUSE)
			{
				pause();
				break;
			}
		}

		mDelayQueue.clear();
	}
	//-----------------------------------------------------------------------
	void EffectSystem::prepare_io_sys(void)
	{
		EffectLayerIterator it;
		for (it = mLayers.begin(); it != mLayers.end(); ++it)
		{
			(*it)->prepare_io();
		}
	}
	//-----------------------------------------------------------------------
	void EffectSystem::prepare_sys(bool bSingleThread)
	{
		if (mIsPreparing)
			return;

		mState = PSS_PREPARING;
		prepare_res();

		if (bSingleThread)
		{
			prepare_io_sys();
			prepareLayer();
			mState = PSS_PREPARED;
		}
		else
		{
			StreamThread* streamThread = StreamThread::Instance();
			if (streamThread && streamThread->IsRunning())
			{
				mIsPreparing = true;
				mDelayTimeTick = 0;

				EffectPrepareEvent* pEffectPrepareEvent = EchoNew(EffectPrepareEvent(this));
				streamThread->addTask(pEffectPrepareEvent);
			}
			else
			{
				prepare_io_sys();
				prepareLayer();
				mState = PSS_PREPARED;
			}
		}
	}
	//-----------------------------------------------------------------------
	void EffectSystem::prepare_res()
	{
		EffectLayerIterator it;
		for (it = mLayers.begin(); it != mLayers.end(); ++it)
		{
			(*it)->prepare_res();
		}
	}
	//-----------------------------------------------------------------------
	void EffectSystem::prepareLayer(void)
	{
		EffectLayerIterator it;
		for (it = mLayers.begin(); it != mLayers.end(); ++it)
		{
			(*it)->prepare();
		}

		if (mbEffectSystemShowVisualHelp)
		{
			for (it = mLayers.begin(); it != mLayers.end(); ++it)
			{
				(*it)->_notifyShowVisualHelp(mbEffectSystemShowVisualHelp);
			}
		}
		mState = PSS_PREPARED;
	}
	//-----------------------------------------------------------------------
	void EffectSystem::unprepareLayer(void)
	{
		for (EffectLayerIterator it = mLayers.begin(); it != mLayers.end(); ++it)
		{
			(*it)->unprepare();
		}
	}
	//-----------------------------------------------------------------------
	void EffectSystem::unprepare_sys(void)
	{
		if (mIsPreparing)
		{
			DelayQueueItem delayItem;
			delayItem.mType = DQT_UNPREPARE;
			delayItem.mTime = mDelayTimeTick;
			mDelayQueue.push_back(delayItem);
		}
		else if (mState == PSS_STOPPED || mState == PSS_PREPARED || mState == PSS_SCHEDULE)
		{
			unprepareLayer();
			mState = PSS_CREATED;
		}
		else if (mState == PSS_STARTED || mState == PSS_PAUSED)
		{
			stop();
			unprepareLayer();
			mState = PSS_CREATED;
		}
	}

	/**
	* 特效更新任务
	*/
	struct EffectSystemUpdateTask : public CpuThreadPool::Job
	{
		int			 m_effectSystemID;		// 特效系统
		ui32		 m_deltaTime;

		// 执行任务
		virtual bool process()
		{
			EffectSystem* effectSystem = EffectSystemManager::instance()->getEffectSystem(m_effectSystemID);
			if (effectSystem)
				effectSystem->updateInternal(m_deltaTime);

			return true;
		}

		// 任务完成
		virtual bool onFinished()
		{
			EffectSystem* effectSystem = EffectSystemManager::instance()->getEffectSystem(m_effectSystemID);
			if (effectSystem)
				effectSystem->updateRenderDataToMainThread();

			return true;
		}

		// 获取任务类型
		virtual int getType()
		{
			return static_cast<int>(OpenMPTaskMgr::TT_EffectSystem);
		}
	};

	// 更新
	void EffectSystem::_update_sys(i32 timeElapsed, bool isSync)
	{
	//#ifdef ECHO_EDITOR_MODE
		isSync = true;
	//#endif

		if(isSync)
		{
			updateInternal(timeElapsed);
			updateRenderDataToMainThread();
		}
		else
		{
			EffectSystemUpdateTask* task = EchoNew(EffectSystemUpdateTask);
			task->m_effectSystemID = getIdentifier();
			task->m_deltaTime = timeElapsed;
			OpenMPTaskMgr::instance()->addTask(OpenMPTaskMgr::TT_EffectSystem, task);
		}
	}

	// 内部更新
	void EffectSystem::updateInternal(i32 timeElapsed)
	{
		if (timeElapsed == 0)
			return;

		if (mSchedule > 0)
		{
			schedule(mSchedule - timeElapsed);
			return;
		}
		if (mIsPreparing)
		{
			mDelayTimeTick += timeElapsed;
			return;
		}
		if (mState == PSS_STARTED)
		{
			if (!mbIsCallStartUnormal)
			{
				mbIsRealPlaying = true;
			}
			mCurDelta  += timeElapsed * mEffectSystemScaleTime;
			if (mCurDelta > 10.0f)
			{
				if (!_updateLayers(i32(mCurDelta)))
					stop();

				mCurDelta = 0.f;
			}
			else
			{
				return;
			}			
		}

		// 更新渲染数据
		updateRenderData();
	}

	// 更新渲染数据
	void EffectSystem::updateRenderData()
	{
		if (!mbIsVisible || mIsPreparing)
			return;

		/// need to check if camera's frustom is contaion the mAABB.
		if ((mState == PSS_PAUSED || mState == PSS_STARTED) && mCurrentCamera && mParentNode && mParentNode->isVisible())
		{
			mCameraPosInEffect = mCurrentCamera->getPosition();
			mCameraOriInEffect = mCurrentCamera->getOritation();

			Quaternion effectOri = mParentNode->getWorldOrientation();
			effectOri.conjugate();
			mCameraOriInEffect = effectOri * mCameraOriInEffect;
			mCameraPosInEffect = effectOri*(mCameraPosInEffect - mParentNode->getWorldPosition()) / mParentNode->getWorldScaling();

			for (size_t i = 0; i < mLayers.size(); ++i)
			{
				mLayers[i]->updateRenderData();
			}
		}
	}

	// 更新渲染数据到主线程
	void EffectSystem::updateRenderDataToMainThread()
	{
		for (size_t i = 0; i < mLayers.size(); ++i)
		{
			mLayers[i]->updateRenderDataToMainThread();
		}
	}

	// 提交到渲染队列
	void EffectSystem::submitToRenderQueue()
	{
		if (!mbIsVisible || mIsPreparing)
			return;

		mRenderables.clear();

		for (size_t i = 0; i < mLayers.size(); ++i)
		{
			mLayers[i]->submitToRenderQueue(mRenderables);
		}
	}

	//-----------------------------------------------------------------------
	void EffectSystem::_preUpdate(i32 timeElapsed)
	{
		if (mState == PSS_STARTED)
		{
			i32 tickTime = 100;
			int tickCount = int(timeElapsed / tickTime);
			for (int i = 0; i<tickCount; ++i)
			{
				if (!_updateLayers(i32(tickTime * mEffectSystemScaleTime)))
				{
					stop();
					break;
				}
			}
		}
	}
	//-----------------------------------------------------------------------
	int EffectSystem::_updateLayers(i32 timeElapsed)
	{
		size_t sumParticle = 0;
		size_t layerParticle = 0;

		i32 time = timeElapsed;
#if _DEBUG
		if (time > 100) // for debug
			time = 100;
		if (time == 0)
			time = 10;
#endif
		mTimeElapsedSinceStart += time;

		for (size_t i = 0; i<mLayers.size(); ++i)
		{
			layerParticle = mLayers[i]->_update(time);
			sumParticle += layerParticle;
		}
		return sumParticle;
	}

	// 设置特效当前使用的摄像机
	void EffectSystem::setCurrentCamera(Camera* camera, bool recursive)
	{/*
		mCurrentCamera = camera;
		if (recursive)
		{
			for (EffectLayerList::iterator it = mLayers.begin(); it != mLayers.end(); it++)
			{
				if ((*it)->getLayerType() == ELT_Child)
				{
					EffectLayerChild* layerChild = ECHO_DOWN_CAST<EffectLayerChild*>(*it); EchoAssert(layerChild);
					EffectSystem* childEffectSystem = EffectSystemManager::instance()->getEffectSystem(layerChild->getEffectSystemID());
					if (childEffectSystem)
						childEffectSystem->setCurrentCamera(camera);
				}
			}
		}*/
	}

	// 获取当前特效所使用摄像机
	Camera* EffectSystem::getCurrentCamera()
	{
		return mCurrentCamera;
	}

	void EffectSystem::setParentSceneNode(Node* pNode)
	{
		mParentNode = pNode;
	}

	// 开始播放
	void EffectSystem::start(void)
	{
		if (mIsPreparing)
		{
			mState = PSS_STARTED;
			DelayQueueItem delayItem;
			delayItem.mType = DQT_START;
			delayItem.mTime = mDelayTimeTick;
			mDelayQueue.push_back(delayItem);
		}
		else if (mState == PSS_PREPARED || mState == PSS_STOPPED || mState == PSS_SCHEDULE)
		{
			mState = PSS_STARTED;

			if (!mbIsCallStartUnormal)
			{
				mbIsRealPlaying = true;
			}

			mTimeElapsedSinceStart = 0;
			mCurDelta = 0.f;
			EffectLayerIterator it;

			for (it = mLayers.begin(); it != mLayers.end(); ++it)
			{
				(*it)->setFallowLayer(NULL);
				(*it)->_notifyStart();
			}

			size_t fallow, fallowed;
			for (size_t i = 0; i<mFallowPair.size(); i += 2)
			{
				fallow = mFallowPair[i];
				fallowed = mFallowPair[i + 1];

				mLayers[fallow]->setFallowLayer(mLayers[fallowed]);
			}
		}
	}

	void EffectSystem::schedule(i32 time)
	{
		mState = PSS_SCHEDULE;
		mSchedule = time;
		if (mSchedule <= 0)
		{
			start();
		}
	}

	//-----------------------------------------------------------------------
	void EffectSystem::stop(void)
	{
		mbIsRealPlaying = false;
		mCurDelta = 0.f;
		if (mIsPreparing)
		{
			mState = PSS_STOPPED;
			DelayQueueItem delayItem;
			delayItem.mType = DQT_STOP;
			delayItem.mTime = mDelayTimeTick;
			mDelayQueue.push_back(delayItem);
		}
		else if (mState == PSS_STARTED || mState == PSS_PAUSED)
		{
			EffectLayerIterator it;
			for (it = mLayers.begin(); it != mLayers.end(); ++it)
			{
				(*it)->_notifyStop();
			}

			mState = PSS_STOPPED;
		}
	}
	//-----------------------------------------------------------------------
	void EffectSystem::pause(void)
	{
		if (!mbIsCallStartUnormal)
		{
			mbIsRealPlaying = false;
		}

		if (mIsPreparing)
		{
			mState = PSS_STOPPED;
			DelayQueueItem delayItem;
			delayItem.mType = DQT_PAUSE;
			delayItem.mTime = mDelayTimeTick;
			mDelayQueue.push_back(delayItem);
		}
		if (mState == PSS_STARTED)
		{
			mState = PSS_PAUSED;

			EffectLayerIterator it;
			for (it = mLayers.begin(); it != mLayers.end(); ++it)
			{
				(*it)->_notifyPause();
			}
		}
	}
	//-----------------------------------------------------------------------
	void EffectSystem::resume(void)
	{
		if (mState == PSS_PAUSED)
		{
			mState = PSS_STARTED;

			if (mbIsCallStartUnormal)
			{
				mbIsRealPlaying = true;
			}

			EffectLayerIterator it;
			for (it = mLayers.begin(); it != mLayers.end(); ++it)
			{
				(*it)->_notifyResume();
			}
		}
	}
	//-----------------------------------------------------------------------
	void EffectSystem::startAndPause(i32 time)
	{
		mbIsCallStartUnormal = true;
		if (mState == PSS_STARTED || mState == PSS_PAUSED)
			stop();

		start();
		if (time >= 0 && time < DEFAULT_TIME_STEP / 10)
		{
			_update_sys(time, true);
			pause();
			mbIsCallStartUnormal = false;
			return;
		}

		time -= DEFAULT_TIME_STEP / 10;
		_update_sys(DEFAULT_TIME_STEP / 10, true);
		while (time > DEFAULT_TIME_STEP)
		{
			time -= DEFAULT_TIME_STEP;
			_update_sys(DEFAULT_TIME_STEP, true);
		}
		if (time>0.0f)
			_update_sys(time, true);

		pause();

		mbIsCallStartUnormal = false;
	}
	//-----------------------------------------------------------------------
	void EffectSystem::stepIn(i32 time)
	{
		if (mState == PSS_PREPARED)
		{
			start();
		}

		if (mState == PSS_STARTED || mState == PSS_PAUSED)
		{
			if (!_updateLayers(time))
				stop();
		}
		pause();
	}
	//-----------------------------------------------------------------------
	bool EffectSystem::getShowVisualHelp(void) const
	{
		return mbEffectSystemShowVisualHelp;
	}
	//-----------------------------------------------------------------------
	void EffectSystem::setShowVisualHelp(bool show)
	{
		mbEffectSystemShowVisualHelp = show;

		EffectLayerIterator it;
		for (it = mLayers.begin(); it != mLayers.end(); ++it)
		{
			(*it)->_notifyShowVisualHelp(mbEffectSystemShowVisualHelp);
		}

		if (0 == mTimeElapsedSinceStart)
		{
			mTimeElapsedSinceStart = 12;
		}
		startAndPause(mTimeElapsedSinceStart);
	}

	void EffectSystem::changeLayerPosition(size_t p1, size_t p2)
	{
		if (p1 < mLayers.size() && p2 < mLayers.size())
		{
			EffectLayer* temp = mLayers[p1];
			mLayers[p1] = mLayers[p2];
			mLayers[p2] = temp;
		}
	}

	void EffectSystem::caculateAligenBox(bool fromChild)
	{
		mAABB.reset();
		if (!fromChild)
		{
			unprepare_sys();
			prepare_sys(true);
			start();

			ui32 maxTime = 10000; // 10 sec.
			ui32 innerTime = 0;
			ui32 stepTime = 25;
			while (isPlaying() && innerTime < maxTime)
			{
				_update_sys(stepTime);
				innerTime += stepTime;
				for (size_t i = 0; i < mLayers.size(); ++i)
				{
					mLayers[i]->mergeAligenBox(mAABB);
				}
			}
		}
		else
		{
			for (size_t i = 0; i < mLayers.size(); ++i)
			{
				mLayers[i]->mergeAligenBox(mAABB);
			}
		}
		
	}

	void EffectSystem::exportData(DataStream* datastream)
	{
		// version
		ui32 version = 0x00010017;
		datastream->write(&version, sizeof(ui32));
		// template name
		ui32 strlength = static_cast<ui32>(mTemplateName.length());
		datastream->write(&strlength, sizeof(ui32));
		datastream->write(mTemplateName.c_str(), strlength);

		// aabb
		datastream->write(&mAABB, sizeof(Box));

		// layers size.
		ui32 layersSize = static_cast<ui32>(mLayers.size());
		datastream->write(&layersSize, sizeof(ui32));
		EffectLayer* pLayer = NULL;
		for (ui32 i = 0; i<layersSize; ++i)
		{
			pLayer = mLayers[i];
			EFFECT_LAYER_TYPE type = pLayer->getLayerType();
			datastream->write(&type, sizeof(EFFECT_LAYER_TYPE));
			pLayer->exportData(datastream, version);
		}

		// fallow pair
		ui32 fallowPairCount = static_cast<ui32>(mFallowPair.size());
		ui32 fallowIdx;
		datastream->write(&fallowPairCount, sizeof(ui32));
		for (size_t i = 0; i<fallowPairCount; ++i)
		{
			fallowIdx = mFallowPair[i];
			datastream->write(&fallowIdx, sizeof(int));
		}
	}

	void EffectSystem::importData(DataStream* datastream)
	{
		EchoAssertX(mLayers.empty(), "Error: cannot importData because the effect system is initialized");
		// version

		ui32 version;
		datastream->read(&version, sizeof(ui32));

		// template name
		ui32 strLength;
		datastream->read(&strLength, sizeof(ui32));
		char str[128];
		datastream->read(str, sizeof(char)*strLength);
		str[strLength] = 0;
		mTemplateName = str;

		//
		datastream->read(&mAABB, sizeof(Box));

		// layers
		ui32 layerSize;
		EFFECT_LAYER_TYPE type;
		EffectLayer* pLayer = NULL;
		datastream->read(&layerSize, sizeof(ui32));
		for (ui32 i = 0; i<layerSize; ++i)
		{
			datastream->read(&type, sizeof(EFFECT_LAYER_TYPE));
			pLayer = createLayer(type);
			pLayer->importData(datastream, version);
		}

		ui32 fallowParamSize;
		datastream->read(&fallowParamSize, sizeof(ui32));
		if (fallowParamSize > 0 && fallowParamSize % 2 == 0)
		{
			mFallowPair.clear();
			mFallowPair.reserve(fallowParamSize);
			mFallowPair.resize(fallowParamSize);
			datastream->read(&(mFallowPair[0]), fallowParamSize*sizeof(ui32));
		}

	}

	void EffectSystem::getPropertyList(PropertyList & list)
	{
	}

	bool EffectSystem::getPropertyType(const String & name, PropertyType &type)
	{
		return false;
	}

	bool EffectSystem::getPropertyValue(const String & name, String &value)
	{
		return false;
	}

	bool EffectSystem::setPropertyValue(const String & name, const String & value)
	{
		return false;
	}

	bool EffectSystem::getVisible(void) const
	{
		return mbIsVisible;
	}

	void EffectSystem::setVisible(bool visible)
	{
		mbIsVisible = visible;
	}

	void EffectSystem::addRenderable(EffectRenderable* renderable)
	{
		mRenderables.push_back(renderable);
	}


	// 构造函数 
	EffectPrepareEvent::EffectPrepareEvent(EffectSystem* pEffect)
		: /*m_effectSystemID(pEffect->getIdentifier()),*/
		mEffectSystem(pEffect)
	{
	}

	EffectPrepareEvent::~EffectPrepareEvent()
	{
		EffectSystem* pEffectSystem = mEffectSystem;//EffectSystemManager::instance()->getEffectSystem(m_effectSystemID);
		if (pEffectSystem && pEffectSystem->mIsPreparing)
		{
			pEffectSystem->mIsPreparing = false;
			pEffectSystem->mState = EffectSystem::PSS_CREATED;
		}
	}

	bool EffectPrepareEvent::process()
	{
		EffectSystem* pEffectSystem = mEffectSystem;//EffectSystemManager::instance()->getEffectSystem(m_effectSystemID);
		if (pEffectSystem)
			pEffectSystem->prepare_io_sys();

		return true;
	}

	// 特效加载完成后执行
	bool EffectPrepareEvent::finished()
	{
		EffectSystem* pEffectSystem = mEffectSystem;//EffectSystemManager::instance()->getEffectSystem(m_effectSystemID);
		if (pEffectSystem)
		{
			pEffectSystem->mIsPreparing = false;

			if (!pEffectSystem->mIsDestroying)
			{
				pEffectSystem->prepareLayer();

				pEffectSystem->mState = EffectSystem::PSS_PREPARED;
				pEffectSystem->handleDelayQueue();
			}
			return true;
		}

		EchoLogError("Error: Effect: Null Point in EffectPrepareEvent::RespondEvent");
		return false;
	}

	EffectDestroyEvent::EffectDestroyEvent(EffectSystem* effectSystem)
		: mbRespond(false)
		, mEffectSystem(effectSystem)
	{

	}

	bool EffectDestroyEvent::finished()
	{
		EffectSystem* pEffectSystem = mEffectSystem;//EffectSystemManager::instance()->getEffectSystem(mpEffect);
		if (pEffectSystem)
		{
			if (pEffectSystem->mIsPreparing)
			{
				EchoLogError("Unable state: mIsPreparing cann't be true");
				return false;
			}

			//if (pEffectSystem->mIsDestroying)
			//{
			//	pEffectSystem->setParentSceneNode(NULL);
			//}

			if (pEffectSystem->mState == EffectSystem::PSS_CREATED)
			{
				//EffectSystemManager::instance()->destroyEffectSystem(mpEffect);
				//mpEffect = NULL;
				mbRespond = true;
				EchoSafeDelete(pEffectSystem, EffectSystem);
				return true;
			}
			else
			{
				pEffectSystem->stop();
				pEffectSystem->unprepare_sys();

				EchoSafeDelete(pEffectSystem, EffectSystem);

				//EffectSystemManager::instance()->destroyEffectSystem(mpEffect);
				//mpEffect = NULL;
				mbRespond = true;
				return true;
			}
			EchoLogError("Error: Effect: Unknow Effect State in EffectDestroyEvent::RespondEvent");
		}

		EchoLogError("Error: Effect: Null Point in EffectPrepareEvent::RespondEvent");
		return false;
	}

	EffectDestroyEvent::~EffectDestroyEvent()
	{
		EffectSystem* pEffectSystem = mEffectSystem;//EffectSystemManager::instance()->getEffectSystem(mpEffect);
		if (pEffectSystem)
		{
			if (!pEffectSystem->mIsPreparing && !mbRespond)
			{
				finished();
				EchoLogDebug("Effect has not destroy in multiThread, so destroy it in main thread. ");
			}
		}
	}
}
