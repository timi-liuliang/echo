#include "Engine/core/Resource/Resource.h"
#include "Engine/core/Resource/ResourceManager.h"
#include "Engine/core/Resource/EchoThread.h"
#include "engine/core/resource/ResourceGroupManager.h"
#include "engine/core/Util/PathUtil.h"
#include "engine/core/Util/Exception.h"
#include "engine/core/Util/LogManager.h"

namespace Echo
{
	// 资源下载完成回调
	static bool onDownloadResourceComplete(const Echo::String& name, DataStream* stream)
	{
		Resource* resource = ResourceManager::getResource(name);
		if (resource)
		{
			resource->prepare(stream);
		}

		return true;
	}

	/**
	* 资源异步加载事件
	*/
	class ResourcePrepareEvent : public Echo::StreamThread::Task
	{
	public:
		ResourcePrepareEvent(Resource* resource) : m_resource(resource) {}
		virtual ~ResourcePrepareEvent() {}

		// 加载失败后是否重复通知
		virtual bool isRepeatNotifyIfFail()	{ return true; }

		// 是否自动销毁
		virtual bool isAutoDestroy(){ return true; }

		// 执行纹理加载
		virtual bool process()
		{
			m_resource->prepare();
			return true;
		}

		// 纹理加载完成后调用
		virtual bool finished()
		{
			if (m_resource->getLoadingState() == Resource::LOADSTATE_PREPARED)
			{
				m_resource->load();
				return true;
			}

			return false;
		}

		// 返回该事件的优先级别
		virtual Echo::StreamThread::TaskLevel GetLevel() { return Echo::StreamThread::TL_Normal; }

	protected:
		Resource*		m_resource;
	};

	// 构造函数
	Resource::Resource(ResourceType type)
		: mLoadingState(LOADSTATE_CREATED)
		, mSize(0)
		, mIsManual(0)
		, mRefrenceCount(0)
		, m_resourceType(type)
	{
	}

	// 构造函数
	/** Standard constructor.
	@param creator Pointer to the ResourceManager that is creating this resource
	@param name The unique name of the resource
	@param isManual Is this resource manually loaded? If so, you should really
	populate the loader parameter in order that the load process
	can call the loader back when loading is required.
	*/
	Resource::Resource(const String& name, ResourceType type, bool isManual)
		: mName(name)
		, mLoadingState(LOADSTATE_CREATED)
		, mSize(0)
		, mIsManual(isManual)
		, mRefrenceCount(0)
		, mDelayedTime(0)
		, mOldLoadingState(LOADSTATE_CREATED)
		, m_resourceType(type)
	{
	}

	// 析构函数
	/** Virtual destructor. Shouldn't need to be overloaded, as the resource
	deallocation code should reside in unload()
	@see
	Resource::unload()
	*/
	Resource::~Resource()
	{
	}

	// 准备资源
	bool Resource::prepare()
	{
		EE_LOCK_AUTO_MUTEX

		if (mLoadingState != LOADSTATE_CREATED)
			return false;

		bool isAsync = ResourceGroupManager::instance()->isAsync(mName.c_str());
		if (isAsync)
		{
			ResourceGroupManager::instance()->openResourceAsync(getName(), std::bind(&onDownloadResourceComplete, std::placeholders::_1, std::placeholders::_2));

			mLoadingState = LOADSTATE_DOWNLOADING;
		}
		else
		{
			DataStream* pStream = ResourceGroupManager::instance()->openResource(getName());
			if (!pStream)
			{
				// 若文件不存在，尝试全路径
				if (!Echo::PathUtil::IsFileExist(mName))
					return false;

				pStream = EchoNew(Echo::FileHandleDataStream(mName, DataStream::READ));
				if (static_cast<FileHandleDataStream*>(pStream)->fail())
					return false;
			}

			if (pStream)
			{
				bool result = prepare(pStream);

				EchoSafeDelete(pStream, DataStream);

				return result;
			}
		}

		return true;
	}

	// 准备资源
	bool Resource::prepare(DataStream* stream)
	{
		EE_LOCK_AUTO_MUTEX

		if (mLoadingState != LOADSTATE_CREATED && mLoadingState != LOADSTATE_DOWNLOADING)
			return false;

		if (mIsManual)
		{
			EchoLogWarning("Instance [%s] was defined as manually loaded, but no manual loader was provided. this Resource will be lost if it has to be reloaded.", mName.c_str());
		}
		else
		{
			if (!prepareImpl(stream))
			{
				mLoadingState = LOADSTATE_CREATED;
				EchoLogError("Resource instance [%s] prepared failed.", mName.c_str());
				return false;
			}
		}

		mLoadingState = LOADSTATE_PREPARED;

		return true;
	}

	// 加载
	bool Resource::load()
	{
		if (mLoadingState == LOADSTATE_LOADED)
			return true;

		// 资源状态不正确，加载资源失败
		if (mLoadingState != LOADSTATE_PREPARED)
		{
			EchoLogError("[%s] Resource::load() failed. Loading state [%d]", mName.c_str(), mLoadingState);
			return false;
		}

		// Scope lock for actual loading
		EE_LOCK_AUTO_MUTEX

		if(mIsManual)
		{
			EchoLogWarning("Instance [%s] was defined as manually loaded, but no manual loader was provided. this Resource will be lost if it has to be reloaded.", mName.c_str());
		}
		else
		{
			bool bRet = loadImpl();

			unprepareImpl();

			if(!bRet)
				return false;
		}

		// Calculate resource size
		mSize = calculateSize();

		mLoadingState = LOADSTATE_LOADED;

		return true;
	}

	// 资源加载(异步)
	bool Resource::prepareLoad()
	{
		if (mLoadingState == LOADSTATE_LOADED)
			return true;

		// Scope lock for actual loading
		EE_LOCK_AUTO_MUTEX

		if (mIsManual)
		{
			EchoLogWarning("Instance [%s] was defined as manually loaded, but no manual loader was provided. this Resource will be lost if it has to be reloaded.", mName.c_str());
		}
		else
		{
			// 刚创建纹理，调用准备函数
			if (mLoadingState == LOADSTATE_CREATED)
			{
				if (ResourceGroupManager::instance()->isAsync(mName.c_str()))
				{
					// 取流加载器
					Echo::StreamThread* streamThread = Echo::StreamThread::Instance();
					ResourcePrepareEvent* resourcePrepareEvent = EchoNew(ResourcePrepareEvent(this));
					streamThread->addTask(resourcePrepareEvent);

					return true;
				}
				else
				{
					if( prepare())
						return load();
				}
			}
		}

		EchoLogError("[%s] Resource State failed[%d]", mName.c_str(), mLoadingState);

		return false;
	}

	// 卸载
	void Resource::unload(void)
	{
		// Early-out without lock (mitigate perf cost of ensuring unloaded)
		if (mLoadingState == LOADSTATE_CREATED) return;

		// Scope lock for actual unload
		{
			EE_LOCK_AUTO_MUTEX
			if (mLoadingState==LOADSTATE_PREPARED) 
			{
				unprepareImpl();
			} 
			else
			{
				unloadImpl();
			}
		}

		mLoadingState = LOADSTATE_CREATED;
	}

	// 重新加载
	bool Resource::reload()
	{
		EE_LOCK_AUTO_MUTEX

		if( isReloadable())
		{
			if(mLoadingState == LOADSTATE_PREPARED) 
			{
				unprepareImpl();
			} 
			else if(mLoadingState == LOADSTATE_LOADED)
			{
				unloadImpl();
			}
			mLoadingState = LOADSTATE_CREATED;

			prepareLoad();

			return true;
		}

		return false;
	}

	// 延迟释放
	void Resource::delayedRelease()
	{
		mOldLoadingState = mLoadingState;
		mLoadingState = LOADSTATE_DESTORY;
		mDelayedTime = 0;
	}

	// 回退延迟释放时间
	void Resource::reserveDelayed()
	{
		mLoadingState = mOldLoadingState;
		mDelayedTime = 0;
	}

	// 附加延迟释放时间
	ui32 Resource::addDelayedTime(ui32 deltaTime)
	{
		mDelayedTime += deltaTime;
		return mDelayedTime;
	}

	// 添加引用计数
	void Resource::addRefrenceCount()
	{
		EE_LOCK_MUTEX(m_refrenceCountMutex);
		++mRefrenceCount;
	}

	// 减少引用计数
	bool Resource::subRefrenceCount()
	{
		EE_LOCK_MUTEX(m_refrenceCountMutex);
		--mRefrenceCount;
		return mRefrenceCount == 0;
	}
}