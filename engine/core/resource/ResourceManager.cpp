#include "Engine/Core.h"
#include "Engine/core/Resource/Resource.h"
#include "Engine/core/Resource/ResourceManager.h"
#include "Engine/core/Util/Exception.h"
#include "Engine/core/Util/LogManager.h"
#include "Engine/core/Util/AssertX.h"
#include "Engine/core/io/archive/Archive.h"
#include "Engine/core/io/IO.h"
#include "Engine/core/Memory/MemAllocDef.h"
#include "engine/core/Util/PathUtil.h"

namespace Echo
{
	vector<ResourceManager*>::type g_resourceMgrs;

	ResourceManager::ResourceManager()
		: mMemoryUsage(0)
		, mReleaseDelayTime(5000)
	{
		g_resourceMgrs.push_back(this);
	}
	//-----------------------------------------------------------------------
	ResourceManager::~ResourceManager()
	{
		{
			EE_LOCK_MUTEX(mResourcesMutex);

			ResourceMap::iterator it = mResources.begin();
			for (; it != mResources.end(); ++it)
			{
				EchoSafeDelete(it->second, Resource);
			}
			mResources.clear();
		}

		{
			EE_LOCK_MUTEX(mResourcesDelayMutex);

			ResourceMap::iterator it = mResources_delay.begin();
			for (; it != mResources_delay.end(); ++it)
			{
				EchoSafeDelete(it->second, Resource);
			}
			mResources_delay.clear();
		}
	}

	// 根据名称获取资源
	Resource* ResourceManager::getResource(const String& name)
	{
		for (size_t i = 0; i < g_resourceMgrs.size(); i++)
		{
			Resource* res = g_resourceMgrs[i]->getByName(name);
			if (res)
				return res;
		}

		return NULL;
	}

	Resource* ResourceManager::createResource(const String &name, bool isManual)
	{
		String lowerCaseName = name;
        if (!PathUtil::IsAbsolutePath(name)) {
            StringUtil::LowerCase(lowerCaseName);
        }
		ResourceCreateOrRetrieveResult res = ResourceManager::createOrRetrieve(lowerCaseName, isManual);
		return res.first;
	}
	
	Resource* ResourceManager::createNewResource(const String& name, bool isManual)
	{
		String lowerCaseName = name;
        if (!PathUtil::IsAbsolutePath(name)) {
            StringUtil::LowerCase(lowerCaseName);
        }

		EE_LOCK_MUTEX(mResourcesMutex);
		Resource* ret = createImpl(name, isManual);
		ret->addRefrenceCount();
		mResources.insert(ResourceMap::value_type(lowerCaseName, ret)); 

		return ret;
	}
	//-----------------------------------------------------------------------
	ResourceManager::ResourceCreateOrRetrieveResult 
		ResourceManager::createOrRetrieve(const String& name, bool isManual)
	{
		Resource* res = nullptr;

		EE_LOCK_MUTEX(mResourcesMutex); // mResourcesMutex 拿到前面锁是为了处理多个线程同时create的情况
		ResourceMap::iterator it = mResources.find(name);
		if (it != mResources.end())
		{
			res = it->second;
			res->addRefrenceCount();
			return ResourceCreateOrRetrieveResult(res, false);
		}

		{
			EE_LOCK_MUTEX(mResourcesDelayMutex);
			ResourceMap::iterator it = mResources_delay.find(name);
			if (it != mResources_delay.end())
			{
				res = it->second;
				mResources_delay.erase(it);

				res->addRefrenceCount();
				res->reserveDelayed();
				mResources.insert(ResourceMap::value_type(name, res));

				return ResourceCreateOrRetrieveResult(res, false);
			}
		}

		String lowerCaseName = name;
		if (!PathUtil::IsAbsolutePath(name)) {
			StringUtil::LowerCase(lowerCaseName);
		}

		res = createImpl(name, isManual);
		res->addRefrenceCount();
		mResources.insert(ResourceMap::value_type(lowerCaseName, res));

		return ResourceCreateOrRetrieveResult(res, true);
	}

	// 释放资源
	// 参数 isUnloadImmediately : 是否立即释放资源
	bool ResourceManager::releaseResource(Resource* r, bool isUnloadImmediately/*=false*/)
	{
		if (!r)
		{
			return false;
		}

		bool released = false;

		Echo::MutexLock resourceLock(mResourcesMutex);

		if (r->subRefrenceCount())
		{
			// 从列表中移除
			released = true;
			ResourceMap::iterator nameIt = mResources.find(r->getName());
			if (nameIt != mResources.end())
			{
				mResources.erase(nameIt);
			}
			else
			{
				EchoLogError("Error: release resource [%s] failed!", r->getName().c_str());
				return false;
			}

			// 执行资源移除
			if (!isUnloadImmediately)
			{
				r->delayedRelease();
				Echo::MutexLock resourceDelayLock(mResourcesDelayMutex);
				mResources_delay.insert(ResourceMap::value_type(r->getName(), r));
			}
		}

		// 这里正常不需要加锁，但TextureRes中引用了Texture，Texture类没有引用计数，再TextureRes::unload()时会产生竞争条件
		if (released)
		{
			if (isUnloadImmediately)
			{
				r->unload();
				EchoSafeDelete(r, Resource);
			}
			return true;
		}

		return false;
	}

	// 根据资源名称获取资源指针
	Resource* ResourceManager::getByName(const String& name)
	{	
		EE_LOCK_MUTEX(mResourcesMutex);

		Resource* res = NULL;
		ResourceMap::iterator it = mResources.find(name);
		if( it != mResources.end())
		{
			res = it->second;
		}

		return res;
	}

	// 设置资源释放延迟时间
	void ResourceManager::setReleaseDelayTime(ui32 t)
	{
		mReleaseDelayTime = t;
	}
	//-----------------------------------------------------------------------
	ui32 ResourceManager::getReleaseDelayTime() const
	{
		return mReleaseDelayTime;
	}
	//-----------------------------------------------------------------------
	void ResourceManager::updateDelayedRelease(ui32 frameTime)
	{
		vector<Resource*>::type waitForDelete;
		
		// 这里正常不需要锁住mResourcesMutex，但TextureRes中引用了Texture，Texture类没有引用计数，再TextureRes::unload()时会产生竞争条件
		EE_LOCK_MUTEX(mResourcesMutex);
		{
			EE_LOCK_MUTEX(mResourcesDelayMutex);
			ResourceMap::iterator it = mResources_delay.begin();
			Resource* res;
			for (; it != mResources_delay.end();)
			{
				res = it->second;
				if (res->addDelayedTime(frameTime) >= mReleaseDelayTime)
				{
					waitForDelete.push_back(res);
					mResources_delay.erase(it++);
				}
				else
				{
					++it;
				}
			}
		}
		
		for (Resource* res : waitForDelete)
		{
			res->unload();
			EchoSafeDelete(res, Resource);
		}
	}
	//-----------------------------------------------------------------------
	void ResourceManager::releaseAllDelayed()
	{
		// 这里正常不需要锁住mResourcesMutex，但TextureRes中引用了Texture，Texture类没有引用计数，再TextureRes::unload()时会产生竞争条件
		Echo::MutexLock resourceMutexLock(mResourcesMutex);
		EE_LOCK_MUTEX(mResourcesDelayMutex);

		ResourceMap::iterator it = mResources_delay.begin();
		Resource* res;
		for(; it!=mResources_delay.end(); ++it)
		{
			res = it->second;
			res->unload();
			EchoSafeDelete(res, Resource);
		}
		mResources_delay.clear();
	}
	//-----------------------------------------------------------------------
	void ResourceManager::outputResourceLog(const String& managerType)
	{
		EE_LOCK_MUTEX(mResourcesMutex);

		ResourceMap::iterator it = mResources.begin();
		EchoLogInfo("%s Resource Leak Begin.......", managerType.c_str());
		for(; it!=mResources.end(); ++it)
		{
			if(it->second->getLoadingState() != Resource::LOADSTATE_CREATED)
				EchoLogInfo("%s, refCount = %d", it->first.c_str(), (it->second)->mRefrenceCount);
		}
		EchoLogInfo("%s Resource Leak End.......", managerType.c_str());
	}

	// 重新加载指定类型的资源
	void ResourceManager::reloadResources( ResourceType type)
	{
		// 释放所有延迟释放资源
		{
			EE_LOCK_MUTEX(mResourcesDelayMutex);
			for (ResourceMap::iterator it = mResources_delay.begin(); it != mResources_delay.end(); ++it)
			{
				EchoSafeDelete(it->second, Resource);
			}
			mResources_delay.clear();
		}

		EE_LOCK_MUTEX(mResourcesMutex);

		// 重新加载所有匹配类型的资源
		for( ResourceMap::iterator it=mResources.begin(); it!=mResources.end(); )
		{
			Resource* pResource = it->second;
			if( pResource && pResource->getType()==type)
			{
				pResource->reload();
			}
		}
	}
}


