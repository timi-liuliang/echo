#include "Engine/Core.h"
#include "Engine/modules/Anim/SkeletonManager.h"
#include "Engine/modules/Anim/Skeleton.h"

namespace Echo
{
	__ImplementSingleton(SkeletonManager);

	SkeletonManager::SkeletonManager()
	{
		__ConstructSingleton;
	}

	SkeletonManager::~SkeletonManager()
	{
		__DestructSingleton;
		// subclasses should unregister with resource group manager
	}

	Skeleton* SkeletonManager::createSkeleton(const String& name, bool isManual)
	{
		Skeleton* pSkeleton =(Skeleton*)ResourceManager::createResource(name, isManual);
		return pSkeleton;
	}

	Resource* SkeletonManager::createImpl(const String& name, bool isManual)
	{
		return EchoNew(Skeleton(name, isManual));
	}
}
