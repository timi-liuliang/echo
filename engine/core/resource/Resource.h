#pragma once

#include <engine/core/Memory/MemManager.h>
#include <engine/core/Thread/Threading.h>
#include <engine/core/io/DataStream.h>
#include "engine/core/Base/EchoDef.h"

// 版本号
#define VERSION_ORIGIN_BASE 1
#define VERSION_HALF_FLOAT_BASE 100
#define VERSION_EXTAND_MATERIAL 200

#define VERSION_SKELETON_BASE 1
#define VERSION_SKELETON_SUPPORT_SCALE 2

#define VERSION_ANIM_BASE 1
#define VERSION_ANIM_SEPERATE 2
#define VERSION_ANIM_SUPPORT_SCALE 3
#define VERSION_ANIM_ADD_BONEWEIGHTS 4

#define RESOURCE_USE_HALF_FLOAT 1

namespace Echo
{
	// 资源类型
	enum ResourceType
	{
		RT_Unknown = 0,		// 未知
		RT_Texture,			// 纹理
	};

	/**
	 * 资源接口
	 */
	class ResourceManager;
	class Resource
	{
	public:
		EE_AUTO_MUTEX

		// 加载状态
		enum LoadingState
		{
			LOADSTATE_CREATED,		// 资源已创建
			LOADSTATE_DOWNLOADING,	// 资源下载中(http archive etc)
			LOADSTATE_PREPARED,		// 准备工作完成(已加载到内存)
			LOADSTATE_LOADED,		// 资源加载完成(可使用)
			LOADSTATE_DESTORY		// 已销毁
		};

		friend class ResourceManager;

	protected:
		Resource(ResourceType type);

		virtual bool prepareImpl(DataStream* stream) = 0;
		virtual void unprepareImpl(void) {}
		virtual bool loadImpl(void) = 0;
		virtual void unloadImpl(void) = 0;
		virtual size_t calculateSize(void) const = 0;

	public:
		Resource(const String& name, ResourceType type, bool isManual = false);
		virtual ~Resource();

		// 准备资源
		bool prepare();

		// 准备资源(不保证同步)
		bool prepare(DataStream* stream);

		// 加载(同步)
		virtual bool load();

		// 资源加载(本地资源同步,网络资源异步)
		bool prepareLoad();

		// 重新加载
		virtual bool reload();

		/** Returns true if the Resource is reloadable, false otherwise.
		*/
		virtual bool isReloadable(void) const { return !mIsManual;}

		/** Is this resource manually loaded?
		*/
		virtual bool isManuallyLoaded(void) const { return mIsManual;}

		void delayedRelease();
		void reserveDelayed();
		ui32 addDelayedTime(ui32 deltaTime);
		void resetDelayedTime() { mDelayedTime = 0; };

		/** Unloads the resource; this is not permanent, the resource can be reloaded later if required.*/
		virtual void unload(void);

		/** Retrieves info about the size of the resource.
		*/
		virtual size_t getSize(void) const { return mSize; }

		/** Gets resource name.*/
		virtual const String& getName(void) const { return mName; }

		/** Returns true if the Resource has been loaded, false otherwise.  No lock required to read this state since no modify*/
		virtual bool isLoaded(void) const { return (mLoadingState == LOADSTATE_LOADED); }

		// 返回资源状态
		LoadingState getLoadingState() const { return mLoadingState; }

		// 获取类型
		ResourceType getType() const { return m_resourceType; }

		// 添加引用计数
		void addRefrenceCount();

	private:
		// 减少引用计数
		bool subRefrenceCount();

	protected:		
		String			mName;				// Unique name of the resource	
		LoadingState	mLoadingState;		// Is the resource currently loaded?
		size_t			mSize;				// The size of the resource in bytes
		bool			mIsManual;			// Is this file manually loaded?
		volatile int	mRefrenceCount;		// 引用计数
		EE_MUTEX(m_refrenceCountMutex);
		ui32			mDelayedTime;		// 延迟释放时间
		LoadingState	mOldLoadingState;	// 旧加载状态
		ResourceType	m_resourceType;		// 资源类型
	};
}
