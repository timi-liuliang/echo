#pragma once

#include "engine/core/Resource/ResourceManager.h"

namespace Echo
{
	class Skeleton;
	class SkeletonManager : public ResourceManager
	{
	public:
		virtual ~SkeletonManager();

		// instance
		static SkeletonManager* instance();

		Skeleton* createSkeleton(const String& name, bool isManual=false);

	protected:
		SkeletonManager();

		/// @copydoc ResourceManager::createImpl
		Resource* createImpl(const String& name, bool isManual);
	};

}
