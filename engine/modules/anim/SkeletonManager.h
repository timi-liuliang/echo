#ifndef __ECHO_SKELETONMANAGER_H__
#define __ECHO_SKELETONMANAGER_H__

#include "engine/core/Resource/ResourceManager.h"

namespace Echo
{
	class Skeleton;
	class SkeletonManager : public ResourceManager
	{
		__DeclareSingleton(SkeletonManager);

	public:
		SkeletonManager();
		virtual ~SkeletonManager();

		Skeleton* createSkeleton(const String& name, bool isManual=false);

	protected:
		/// @copydoc ResourceManager::createImpl
		Resource* createImpl(const String& name, bool isManual);
	};

}

#endif
