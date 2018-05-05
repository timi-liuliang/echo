#include "Engine/modules/Anim/SkeletonManager.h"
#include "Engine/modules/Anim/Skeleton.h"

namespace Echo
{
	SkeletonManager::SkeletonManager()
	{
	}

	SkeletonManager::~SkeletonManager()
	{
	}

	// instance
	SkeletonManager* SkeletonManager::instance()
	{
		static SkeletonManager* inst = EchoNew(SkeletonManager);
		return inst;
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
