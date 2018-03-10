#ifndef __ECHO_RESOURCE_MANAGER_H__
#define __ECHO_RESOURCE_MANAGER_H__

#include "Resource.h"

namespace Echo
{
	/**
	 * 资源管理器(线程安全)
	 */
	class ResourceManager
	{
	public: 
		typedef map< String, Resource* >::type ResourceMap;
		typedef std::pair<Resource*, bool> ResourceCreateOrRetrieveResult;

	public:
		ResourceManager();
		virtual ~ResourceManager();

		// 获取内存使用状态
		virtual size_t getMemoryUsage(void) const { return mMemoryUsage; }

		// create resource
		virtual Resource* createResource(const String& name, bool isManual = false);

		// 释放资源 
		virtual bool releaseResource(Resource* r, bool isUnloadImmediately=false);

		// 根据名称获取资源
		virtual Resource* getByName(const String& name);

		void setReleaseDelayTime(ui32 t);

		ui32 getReleaseDelayTime() const;

		virtual void updateDelayedRelease(ui32 frameTime);

		virtual void releaseAllDelayed();

		void outputResourceLog(const String& managerType);

		// 重新加载指定类型的资源
		void reloadResources( ResourceType type);

		// 根据名称获取资源
		static Resource* getResource( const String& name);

		// 获取资源数量
		i32 getResourceNum() { return mResources.size(); }

	protected:
		// 创建新资源
		virtual Resource* createNewResource(const String& name, bool isManual = false);

		virtual ResourceCreateOrRetrieveResult createOrRetrieve(const String& name, bool isManual = false);

		// 资源创建实现
		virtual Resource* createImpl(const String& name, bool isManual)=0;

	protected:
		EE_MUTEX(	mResourcesMutex);		// 资源操作互斥锁
		ResourceMap mResources;
		EE_MUTEX(	mResourcesDelayMutex);
		ResourceMap mResources_delay;
		size_t		mMemoryUsage;		// In bytes
		ui32		mReleaseDelayTime;
	};
}

#endif
