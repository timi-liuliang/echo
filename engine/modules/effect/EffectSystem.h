#ifndef __ECHO_EFFECT_SYSTEM_BASE_H__
#define __ECHO_EFFECT_SYSTEM_BASE_H__

#include "Engine/Core.h"
#include "EffectRender.h"
#include "EffectIElement.h"
#include "Engine/core/Resource/EchoThread.h"
#include "engine/core/Scene/Node.h"

namespace Echo
{
	/**
	 * 特效系统
	 */
	class EffectSystem : public IElement
	{
		friend class EffectPrepareEvent;
		friend class EffectDestroyEvent;
		friend class EffectSystemManager;
	public:
		//interface
		virtual void getPropertyList(PropertyList& list);
		virtual bool getPropertyType(const String& name, PropertyType& type);
		virtual bool getPropertyValue(const String& name, String& value);
		virtual bool setPropertyValue(const String& name,const String& value);

	public:
		//default property
		static const i32	DEFAULT_TIME_STEP;
		static const float	DEFAULT_SCALE_VELOCITY;
		static const float	DEFAULT_SCALE_TIME;

		//effect state
		enum EffectSystemState
		{
			PSS_CREATED,	
			PSS_PREPARING,	
			PSS_PREPARED,
			PSS_SCHEDULE,
			PSS_STARTED,		
			PSS_STOPPED,	
			PSS_PAUSED		
		};

		//delay queue
		enum DelayQueueType
		{
			DQT_START,
			DQT_PAUSE,
			DQT_STOP,
			DQT_UNPREPARE
		};

		struct DelayQueueItem
		{
			DelayQueueType mType;
			i32 mTime;
		};

		// typedef
		typedef vector<EffectLayer*>::type			EffectLayerList;
		typedef EffectLayerList::iterator			EffectLayerIterator;
		typedef EffectLayerList::const_iterator     EffectLayerConstIterator;

		typedef vector<DelayQueueItem>::type		EffectDelayQueue;
		typedef EffectDelayQueue::iterator			EffectDelayIterator;

		typedef vector<ui32>::type                  UbiLayerFallowPair;

		// 获取唯一标识符
		ui32 getIdentifier() const { return m_identifier; }

		// name time.
		inline const String& getName() const { return mName; }
		void setTemplateName(const String& templateName) { mTemplateName = templateName; }
		inline const String& getTemplateName() const { return mTemplateName; }
		inline i32 getTimeElapsedSinceStart(void) const { return mTimeElapsedSinceStart; };

		// 获取特效时长
		float getEffectTimeLength(void) const;

		// layer
		EffectLayer* createLayer(EFFECT_LAYER_TYPE layerType);
		virtual void addLayer(EffectLayer* layer);
		EffectLayer* getLayer(size_t index) const;
		EffectLayer* getLayer(const String& name) const;
		size_t getNumLayers(void) const;
		void destroyLayer(EffectLayer* layer);
		void destroyLayer(size_t index);
		void destroyAllLayers(void);

		// inilised and update
		virtual void prepare_sys(bool bSingleThread = false );
		virtual void prepare_res();
		virtual void prepare_io_sys(void);  // stream threading
		void prepareLayer(void);			// main threading
		void unprepare_sys(void);
		void unprepareLayer(void);

		void importData(DataStream* datastream);
		void exportData(DataStream* datastream);

		// 特效系统更新
		virtual void _update_sys(i32 timeElapsed, bool isSync=false);

		// 内部更新
		void updateInternal( i32 timeElapsed);

		virtual void _preUpdate(i32 timeElapsed);

		// 提交到渲染队列
		virtual void submitToRenderQueue();

		// 更新渲染数据(数据保存到工作线程)
		virtual void updateRenderData();

		// 更新渲染数据到主线程
		void updateRenderDataToMainThread();

		// EffectLayer add renderable to EffectSystem
		void addRenderable(EffectRenderable* renderable);
		const std::vector<EffectRenderable*>* getRenderables() const { return &mRenderables; }

	private:
		// 特效层更新
		virtual int _updateLayers(i32 timeElapsed);

	public:
		// operate
		void start(void);
		void schedule(i32 time);
		void stop(void);
		void pause(void);
		void resume(void);
		void startAndPause(i32 time);
		virtual void stepIn(i32 time);
		EffectSystemState getState(void) const { return mState; }
		bool isScheduled() const { return PSS_SCHEDULE == mState; }
		bool isPlaying(void) { return (PSS_STARTED == mState); }
		bool isPaused(void) { return PSS_PAUSED == mState; }
		bool isStoped(void) { return PSS_STOPPED == mState; }

		// 设置当前特效所使用的摄像机
		void	setCurrentCamera(Camera* camera, bool recursive=true);
		Camera* getCurrentCamera();
		inline const Vector3& getCameraPosInEffect() const { return mCameraPosInEffect; }
		inline const Quaternion& getCameraOriInEffect() const { return mCameraOriInEffect; }
		inline Node* getEffectSceneNode() { return mParentNode; }
		void	setParentSceneNode(Node* pNode);
		inline const Box& getBoundingBox() const { return mAABB; };

		// only the timescale control, no position, no oritation, no scale, no color. 
		// pos, ori and scale use the parent bone to control
		// color and alpha use the layer to set.
		inline float getScaleTime(void) const { return mEffectSystemScaleTime; }
		
		// 设置缩放时间
		inline void setScaleTime(float scaleTime){ mEffectSystemScaleTime = scaleTime; }

		// 获取系统色
		inline const Color& getSystemColor(void) const{ return mEffectSystemColor; }

		// 设置系统色
		inline void setSystemColor(const Color& color){ mEffectSystemColor = color; }

		// this is to reduce the keyframe cal cost. use more.
		const Echo::vector<ui32>::type& getFallowPair(void) const;
		String getFallowPairString(void) const;
		void setFallowPair(const String& pairString);
		void setFallowPair(const Echo::vector<ui32>::type& pairVector);

		/** external params */
		void setExtenalParam(const Vector3& p1, const Vector3& p2);
		void setExtenalParamWithXYZ(Real x1, Real y1, Real z1, Real x2, Real y2, Real z2);

		// editor about.
		EffectLayerList getLayers(void) const;
		bool getShowVisualHelp(void) const;
		void setShowVisualHelp(bool show);
		void changeLayerPosition(size_t p1, size_t p2);
		void caculateAligenBox(bool fromChild = false);
		bool getVisible(void) const;
		void setVisible(bool visible);
		void setIsUIEffect(bool isUIeffect){mbIsUIEffect = isUIeffect;}
		bool getIsUIEffect(){return mbIsUIEffect;}
		EffectSystem& operator=(const EffectSystem& ps);
		bool getIsRealPlaying(){ return mbIsRealPlaying; }
	protected:
		// constructor
		EffectSystem(const String& name,bool isUIeffect = false);
		virtual ~EffectSystem(void);

		bool effectVisible();
		void handleDelayQueue();

	protected:
		ui32			m_identifier;			// 唯一标识
		String			mName;					// 名称
		String			mTemplateName;			// 资源名称
		EffectSystemState mState;				// 特效状态
		i32				mTimeElapsedSinceStart;
		i32				mDelayTimeTick;		
		bool			mIsPreparing;
		bool			mIsDestroying;
		float			mEffectSystemScaleTime;
		Color			mEffectSystemColor;
		Box				mAABB;					// 包围盒
		Camera*			mCurrentCamera;			// 当前摄像机
		Vector3			mCameraPosInEffect;
		Quaternion		mCameraOriInEffect;
		Node*		mParentNode;
		EffectLayerList mLayers;				// 特效层

		EffectDelayQueue	mDelayQueue;	
		UbiLayerFallowPair	mFallowPair;
		std::vector<EffectRenderable*> mRenderables;

		bool		mbEffectSystemShowVisualHelp;
		bool		mbIsVisible;
		bool        mbIsUIEffect;
		bool		mbIsCallStartUnormal;		 //是否是非正常调用start，如startandpuase等
		bool		mbIsRealPlaying;			 //是否是真正的在播放中,因为经常会有一些如StartAndPause之类的东西捣乱
		float		mCurDelta;
		i32			mSchedule;
	};

	/**
	 * 异步线程加载特效
	 */
	class EffectPrepareEvent : public StreamThread::Task
	{
	public:
		EffectPrepareEvent(EffectSystem* pEffect);
		virtual ~EffectPrepareEvent();

		// 处理
		virtual bool process();

		// 加载后执行
		virtual bool finished();

		// 失败后是否重复通知
		virtual bool isRepeatNotifyIfFail()	{ return true; }

		// 任务本身是否自动销毁
		virtual bool isAutoDestroy() { return true; }

		// 返回该事件的优先级别
		virtual StreamThread::TaskLevel GetLevel() { return StreamThread::TL_Normal; }

	protected:		
		//EffectSystemID		m_effectSystemID;
		EffectSystem*	mEffectSystem;
	};

	/**
	 * 销毁事件
	 */
	class EffectDestroyEvent : public StreamThread::Task
	{
	public:
		EffectDestroyEvent(EffectSystem* effectSystem);
		virtual ~EffectDestroyEvent();

		virtual bool finished() ;

		virtual bool process()	{ return true; /* nothing to do...*/ }

		virtual bool isRepeatNotifyIfFail()	{ return true; }
		virtual bool isAutoDestroy()			{ return true; }

		// 返回该事件的优先级别
		virtual StreamThread::TaskLevel GetLevel() { return StreamThread::TL_Normal; }

	protected:
		EffectSystemID	mpEffect;
		bool			mbRespond;
		EffectSystem*	mEffectSystem;
	};
}
#endif
