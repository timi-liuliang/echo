/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef NX_APEX_PARAMETERIZED_TRAITS_H
#define NX_APEX_PARAMETERIZED_TRAITS_H

#include "NxParameterizedTraits.h"
#include "NxTraitsInternal.h"

#include "PsUserAllocated.h"
#include "PsAtomic.h"
#include "PsHashMap.h"
#include "PsHashSet.h"

#define NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE "NxParameterizedFactories"

#define DEBUG_ASSERT(x) //PX_ASSERT(x)
#define DEBUG_ALWAYS_ASSERT() DEBUG_ASSERT(0)

#define DEBUG_FW 0

#define PATH_INF physx::PxU32(-1)
#define PATH_NULL physx::PxU32(-2)

#define ALIGNED_ALLOC(n, align) physx::shdfnd::AlignedAllocator<align>().allocate(n, __FILE__, __LINE__)

// Does not depend on alignment in AlignedAllocator
#define ALIGNED_FREE(p) physx::shdfnd::AlignedAllocator<16>().deallocate(p)

namespace physx
{
namespace apex
{

class ApexParameterizedTraits: public NxParameterized::Traits, public physx::UserAllocated
{
	typedef physx::Pair<PxU32, PxU32> PxU32Pair;

	struct ClassInfo
	{
		static const PxU32 initialHashTableSize = 0;

		PxU32 currentVersion;

		typedef physx::HashMap<PxU32, NxParameterized::Factory*> MapVer2Factory;
		MapVer2Factory factors;

		//Conversions
		typedef HashMap<PxU32Pair, NxParameterized::Conversion*> MapVers2Conversion;
		MapVers2Conversion convs;

		//Shortest paths
		bool arePathsValid;
		typedef HashMap<PxU32Pair, PxU32> MapVers2Next;
		MapVers2Next next;

		ClassInfo() :
			currentVersion(PX_MAX_U32),
			factors(initialHashTableSize),
			convs(initialHashTableSize),
			arePathsValid(true),
			next(initialHashTableSize)
		{}

		NxParameterized::Factory* currentFactory()
		{
			return PX_MAX_U32 == currentVersion ? 0 : factors[currentVersion];
		}

		//FIXME: we need this because of broken copy constructor of PsHashMap
		ClassInfo(const ClassInfo& inf): currentVersion(inf.currentVersion), factors(inf.factors.size()), arePathsValid(false)
		{
			ClassInfo& inf_ = const_cast<ClassInfo&>(inf);
			for (MapVer2Factory::Iterator i = inf_.factors.getIterator(); !i.done(); ++i)
			{
				factors[i->first] = i->second;
			}
		}

		//Update shortest paths using Floyd-Warshall algorithm
		void updatePaths(NxParameterized::Traits* traits)
		{
			arePathsValid = true;

			this->next.clear();

			//Enumerate versions

			HashSet<PxU32> vers;
			for (MapVers2Conversion::Iterator i = convs.getIterator(); !i.done(); ++i)
			{
				vers.insert(i->first.first);
				vers.insert(i->first.second);
			}

			HashMap<PxU32, PxU32> mapVer2Idx;
			Array <PxU32> mapIdx2Ver;

#if DEBUG_FW
			printf("Encoding:\n");
#endif
			for (HashSet<PxU32>::Iterator i = vers.getIterator(); !i.done(); ++i)
			{
				mapVer2Idx[*i] = mapIdx2Ver.size();
				mapIdx2Ver.pushBack(*i);
#if DEBUG_FW
				printf("%d.%d - %d\n", (*i & 0xffff0000U) >> 16, *i & 0xffffU, mapIdx2Ver.size() - 1);
#endif
			}

			if (!mapIdx2Ver.size())
			{
				return;
			}

			//Calculate shortest paths (see http://en.wikipedia.org/wiki/Floyd-Warshall_algorithm)

			PxU32 n = mapIdx2Ver.size();

			PxU32* cost = (PxU32*)traits->alloc(n * n * sizeof(PxU32)),
			       *next = (PxU32*)traits->alloc(n * n * sizeof(PxU32));

#			define ELEM(a, i, j) (*((a) + n * (i) + (j)))

			for (PxU32 i = 0; i < n * n; ++i)
			{
				cost[i] = next[i] = PATH_INF;
			}

			for (PxU32 i = 0; i < n; ++i)
			{
				ELEM(cost, i, i) = 0;
				ELEM(next, i, i) = PATH_INF;
#if DEBUG_FW
				printf("next[%2d, %2d] = %2d (cost %d)\n", i, i, i, 0);
#endif
			}

#if DEBUG_FW
			printf("Existing conversions:\n");
#endif
			for (MapVers2Conversion::Iterator i = convs.getIterator(); !i.done(); ++i)
			{
				PxU32 from = mapVer2Idx[i->first.first],
				      to = mapVer2Idx[i->first.second];

				ELEM(cost, from, to) = 1;
				ELEM(next, from, to) = PATH_NULL;
#if DEBUG_FW
				printf("%d.%d -> %d.%d\n",
				       (i->first.first & 0xffff0000U) >> 16, i->first.first & 0xffffU,
				       (i->first.second & 0xffff0000U) >> 16, i->first.second & 0xffffU
				      );
				printf("next[%2d, %2d] = %2d (cost %d)\n", from, to, to, 1);
#endif
			}

			for (PxU32 k = 0; k < n; ++k)
				for (PxU32 i = 0; i < n; ++i)
				{
					PxU32 ikCost = ELEM(cost, i, k);

					for (PxU32 j = 0; j < n; ++j)
					{
						PxU32 kjCost = ELEM(cost, k, j);

						PxU32 ijCost = PATH_INF == ikCost || PATH_INF == kjCost ? PATH_INF : ikCost + kjCost;
						if (ijCost < ELEM(cost, i, j))
						{
#if DEBUG_FW
							printf("next[%2d, %2d] = %2d (cost %d)\n", i, j, k, ijCost);
#endif
							ELEM(cost, i, j) = ijCost;
							ELEM(next, i, j) = k;
						}
					}
				}

			//Back to versions

#if DEBUG_FW
			printf("Next versions:\n");
#endif
			for (PxU32 i = 0; i < n; ++i)
			{
				PxU32 from = mapIdx2Ver[i];

				for (PxU32 j = 0; j < n; ++j)
				{
					PxU32 to = mapIdx2Ver[j];

					PxU32 k = ELEM(next, i, j);
					this->next[PxU32Pair(from, to)] = PATH_INF != k && PATH_NULL != k ? mapIdx2Ver[k] : k;

#if DEBUG_FW
					if (PATH_INF != k && PATH_NULL != k)
					{
						PxU32 thru = this->next[PxU32Pair(from, to)];
						printf("%d.%d, %d.%d -> %d.%d\n",
						       (from & 0xffff0000U) >> 16, from & 0xffffU,
						       (to & 0xffff0000U) >> 16, to & 0xffffU,
						       (thru & 0xffff0000U) >> 16, thru & 0xffffU
						      );
					}
					else if (PATH_NULL == k)
					{
						printf("%d.%d, %d.%d -> edge\n",
						       (from & 0xffff0000U) >> 16, from & 0xffffU,
						       (to & 0xffff0000U) >> 16, to & 0xffffU
						      );
					}
#endif
				}
			}

#			undef ELEM

			traits->free(cost);
			traits->free(next);
		}
	};

	ClassInfo* getClassInfo(const char* name, bool emitDebugWarning = true) const
	{
		NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();
		NxResourceProvider* xnrp = NiGetApexSDK()->getNamedResourceProvider();

		if (!nrp->checkResource(mFactoryNsResID, name))
		{
			if (emitDebugWarning)
			{
				APEX_DEBUG_WARNING("No NxParameterized::Factory is registered for %s", name);
			}
			return NULL;
		}

		return static_cast<ClassInfo*>(
		           xnrp->getResource(NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, name));
	}

	NxParameterized::Factory* getFactory(const char* name, PxU32 ver) const
	{
		ClassInfo* info = getClassInfo(name);
		if (!info)
		{
			return 0;
		}

		const physx::HashMap<PxU32, NxParameterized::Factory*>::Entry* e = info->factors.find(ver);
		return e ? const_cast<NxParameterized::Factory*>(e->second) : 0;
	}

	void reportNoClassError(const char* name) const
	{
		PX_UNUSED(name);
		NX_PARAM_TRAITS_WARNING(this, "Class %s is not registered, did you forget to load its module?", name);
		DEBUG_ALWAYS_ASSERT();
	}

	void reportNoFactoryError(const char* name, PxU32 ver) const
	{
		ClassInfo* info = getClassInfo(name, false);			
		if (info && ver < info->currentVersion)
		{
			NX_PARAM_TRAITS_WARNING(
				this,
				"No factory for version %u of class %s, did you forget to load some *_Legacy module?",
				(unsigned)ver, name );
		}
		else if(info && ver > info->currentVersion)
		{
			NX_PARAM_TRAITS_WARNING(
				this,
				"No factory for version %u of class %s, was this asset serialized for future version of APEX?",
				(unsigned)ver, name );
		}
		else
		{
			NX_PARAM_TRAITS_WARNING(this,
				"No factory for version %u of class %s",
				(unsigned)ver, name );
		}

		DEBUG_ALWAYS_ASSERT();
	}

	typedef physx::HashSet<const char*> StringSet;
	StringSet classNames;

public:
	ApexParameterizedTraits() :
		mFactoryNsResID(INVALID_RESOURCE_ID)
	{
		if (NiGetApexSDK()->getInternalResourceProvider())
		{
			mFactoryNsResID = NiGetApexSDK()->getInternalResourceProvider()->createNameSpace(NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, false);
		}
	}

	~ApexParameterizedTraits()
	{
		NxResourceProvider* xnrp = NiGetApexSDK()->getNamedResourceProvider();

		for (StringSet::Iterator i = classNames.getIterator(); !i.done(); ++i)
		{
			const char* className = *i;

			ClassInfo* info = getClassInfo(className, false);
			if (!info)
			{
				PX_ALWAYS_ASSERT();
				continue;
			}

			if (info->convs.size())
			{
				APEX_DEBUG_WARNING("Some conversions for class %s were not released", className);
			}

			if (info->factors.size())
			{
				APEX_DEBUG_WARNING("Some factories for class %s were not released", className);
			}

			info->~ClassInfo();
			free(info);

			// remove ref in NRP
			xnrp->setResource(NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, className, NULL, false);
		}
	}

	void registerFactory(NxParameterized::Factory& factory)
	{
		const char* factoryName = factory.getClassName();
		if (!factoryName || !factoryName[0])
		{
			NX_PARAM_TRAITS_WARNING(this, "NxParameterized::Factory does not have a valid name");
			return;
		}

		// check if we didn't have the NRP when the constructor ran
		if (INVALID_RESOURCE_ID == mFactoryNsResID)
		{
			if (NiGetApexSDK()->getInternalResourceProvider())
			{
				mFactoryNsResID = NiGetApexSDK()->getInternalResourceProvider()->createNameSpace(NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, false);
			}

			if (INVALID_RESOURCE_ID == mFactoryNsResID)
			{
				NX_PARAM_TRAITS_WARNING(this, "Failed to create namespace for NxParameterized factories");
				DEBUG_ALWAYS_ASSERT();
				return;
			}
		}

		NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();
		NxResourceProvider* xnrp = NiGetApexSDK()->getNamedResourceProvider();

		PxU32 factoryVer = factory.getVersion();

		if (nrp->checkResource(mFactoryNsResID, factoryName))
		{
			ClassInfo* info = static_cast<ClassInfo*>(xnrp->getResource(NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, factoryName));

			info->currentVersion = PxMax(factoryVer, info->currentVersion); //Assume that the biggest version is the current one

			const ClassInfo::MapVer2Factory::Entry* i = info->factors.find(factoryVer);
			if (i)
			{
				NX_PARAM_TRAITS_WARNING(this, "Factory for version %d of class %s registered more than once", factoryVer, factoryName);
				DEBUG_ALWAYS_ASSERT();
				return;
			}
			else
			{
				info->factors[factoryVer] = &factory;
			}
		}
		else
		{
			ClassInfo* info = PX_PLACEMENT_NEW(alloc(sizeof(ClassInfo)), ClassInfo)();

			nrp->createResource(mFactoryNsResID, factoryName, false);
			xnrp->setResource(NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, factoryName, info, false);

			info->currentVersion = factoryVer;
			info->factors[factoryVer] = &factory;
		}

		classNames.insert(factoryName);
	}

	NxParameterized::Factory* removeFactory(const char* className)
	{
		ClassInfo* info = getClassInfo(className, false);
		if (!info)
		{
			return 0;
		}

		return removeFactory(className, info->currentVersion);
	}

	NxParameterized::Factory* removeFactory(const char* className, physx::PxU32 version)
	{
		ClassInfo* info = getClassInfo(className, false);
		if (!info)
		{
			return 0;
		}

		NxParameterized::Factory* factor = info->factors[version];
		if (!factor)
		{
			NX_PARAM_TRAITS_WARNING(
				this,
				"Trying to remove unregistered factory for version %u of class %s",
				(unsigned)version, className);
			DEBUG_ALWAYS_ASSERT();
			return 0;
		}

		info->factors.erase(version);

		return factor;
	}

	bool doesFactoryExist(const char* className)
	{
		ClassInfo* info = getClassInfo(className, false);
		return info != 0;
	}

	bool doesFactoryExist(const char* className, physx::PxU32 version)
	{
		ClassInfo* info = getClassInfo(className, false);
		if (!info)
		{
			return false;
		}

		return info->factors.find(version) != 0;
	}

	NxParameterized::Interface* createNxParameterized(const char* name)
	{
		ClassInfo* info = getClassInfo(name);
		if (!info)
		{
			reportNoClassError(name);
			return 0;
		}

		return info->currentFactory()->create(this);
	}

	NxParameterized::Interface* createNxParameterized(const char* name, PxU32 ver)
	{
		NxParameterized::Factory* factory = getFactory(name, ver);
		if (!factory)
		{
			reportNoFactoryError(name, ver);
			return 0;
		}

		return factory->create(this);
	}

	NxParameterized::Interface* finishNxParameterized(const char* name, void* obj, void* buf, physx::PxI32* refCount)
	{
		ClassInfo* info = getClassInfo(name);
		if (!info)
		{
			reportNoClassError(name);
			return 0;
		}

		return info->currentFactory()->finish(this, obj, buf, refCount);
	}

	NxParameterized::Interface* finishNxParameterized(const char* name, PxU32 ver, void* obj, void* buf, physx::PxI32* refCount)
	{
		NxParameterized::Factory* factory = getFactory(name, ver);
		if (!factory)
		{
			reportNoFactoryError(name, ver);
			return 0;
		}

		return factory->finish(this, obj, buf, refCount);
	}

	PxU32 getCurrentVersion(const char* className) const
	{
		ClassInfo* info = getClassInfo(className);
		if (!info)
		{
			reportNoClassError(className);
			return (physx::PxU32)-1;
		}

		return info->currentVersion;
	}

	PxU32 getAlignment(const char* className, PxU32 version) const
	{
		ClassInfo* info = getClassInfo(className);
		if (!info)
		{
			reportNoClassError(className);
			return (physx::PxU32) - 1;
		}

		NxParameterized::Factory* factory = info->factors[version];
		if (!factory)
		{
			NX_PARAM_TRAITS_WARNING(
				this,
				"Quering information about unregistered factory for version %u of class %s",
				(unsigned)version, className);
			DEBUG_ALWAYS_ASSERT();
			return PX_MAX_U32;
		}

		return factory->getAlignment();
	}

	bool getNxParameterizedNames(const char** names, physx::PxU32& outCount, physx::PxU32 inCount) const
	{
		NxResID ids[512];
		NiResourceProvider* nrp = NiGetApexSDK()->getInternalResourceProvider();

		if (!nrp->getResourceIDs(NX_APEX_PARAMETERIZED_FACTORY_NAME_SPACE, ids, outCount, PX_ARRAY_SIZE(ids)) || outCount > inCount)
		{
			return false;
		}

		for (physx::PxU32 i = 0, j = 0; i < outCount; i++)
		{
			if (nrp->checkResource(ids[i]))
			{
				names[j] = nrp->getResourceName(ids[i]);
				j++;
			}
		}

		return true;
	}

	bool getNxParameterizedVersions(const char* className, physx::PxU32* versions, physx::PxU32& outCount, physx::PxU32 inCount) const
	{
		ClassInfo* info = getClassInfo(className);
		if (!info)
		{
			reportNoClassError(className);
			return false;
		}

		HashSet<PxU32> vers;
		for (HashMap<PxU32, NxParameterized::Factory*>::Iterator it = info->factors.getIterator(); !it.done(); ++it)
		{
			vers.insert(it->first);
		}

		outCount = vers.size();
		if (outCount > inCount)
		{
			return false;
		}

		physx::PxU32 i = 0;
		for (HashSet<physx::PxU32>::Iterator it = vers.getIterator(); !it.done(); ++it)
		{
			versions[i] = *it;
			i++;
		}

		return true;
	}

	void registerConversion(const char* className, PxU32 from, PxU32 to, NxParameterized::Conversion& conv)
	{
		if (from == to)
		{
			return;
		}

		if (from > to)
		{
			APEX_DEBUG_WARNING(
				"Trying to register conversion from higher (%u) to lower (%u) version for class %s",
				(unsigned)from, (unsigned)to, className);
			return;
		}

		ClassInfo* info = getClassInfo(className, false);
		if (!info)
		{
			reportNoClassError(className);
			return;
		}

		info->arePathsValid = false; //Recalculate paths

		PxU32Pair key(from, to);

		const ClassInfo::MapVers2Conversion::Entry* i = info->convs.find(key);
		if (i)
		{
			NX_PARAM_TRAITS_WARNING(
				this,
				"Conversion from version %u to %u for class %s registered more than once",
				(unsigned)from, (unsigned)to, className);
			DEBUG_ALWAYS_ASSERT();
			return;
		}
		else
		{
			info->convs[key] = &conv;
		}
	}

	NxParameterized::Conversion* removeConversion(const char* className, PxU32 from, PxU32 to)
	{
		ClassInfo* info = getClassInfo(className, false);
		if (!info)
		{
			return 0;
		}

		info->arePathsValid = false; //Recalculate paths

		PxU32Pair key(from, to);

		NxParameterized::Conversion* conv = info->convs[key];
		if (!conv)
		{
			NX_PARAM_TRAITS_WARNING(
				this,
				"Trying to remove unregistered conversion from version %u to %u of class %s",
				(unsigned)from, (unsigned)to, className);
			DEBUG_ALWAYS_ASSERT();
			return 0;
		}

		info->convs.erase(key);

		return conv;
	}

	bool updateLegacyNxParameterized(NxParameterized::Interface& legacyObj, NxParameterized::Interface& targetObj)
	{
		const char* className = targetObj.className();

		ClassInfo* info = getClassInfo(className);
		if (!info)
		{
			reportNoClassError(className);
			return false;
		}

		if (!info->arePathsValid)
		{
#if DEBUG_FW
			printf("Updating paths for %s:\n", className);
#endif
			info->updatePaths(this);
		}

		NxParameterized::Interface* curObj = &legacyObj;

		PxU32 curVer = curObj->version();
		PxU32 targetVer = targetObj.version();

		if (curVer > targetVer)
		{
			APEX_DEBUG_WARNING("Trying to downgrade object of class %s from version %u to %u",
				className, (unsigned)curVer, (unsigned)targetVer);
			DEBUG_ALWAYS_ASSERT();
			return false;
		}

		if (curVer == targetVer)
		{
			APEX_DEBUG_WARNING("Trying to upgrade an up-to-date object of class %s", className);
			DEBUG_ALWAYS_ASSERT();
			return false;
		}

		while (true)
		{
			//Next intermediate version
			const ClassInfo::MapVers2Next::Entry* e = info->next.find(PxU32Pair(curVer, targetVer));
			if (!e)
			{
				APEX_DEBUG_WARNING("Internal error while upgrading object of class %s", className);
				DEBUG_ALWAYS_ASSERT();
				return false;
			}

			PxU32 nextVer = e->second;
			if (nextVer == curVer || nextVer == targetVer)
			{
				APEX_DEBUG_WARNING("Internal error while upgrading object of class %s", className);
				DEBUG_ALWAYS_ASSERT();
				return false;
			}

			if (PATH_INF == nextVer)  // There is no path between curVer and targetVer?
			{
				APEX_DEBUG_WARNING(
				    "No appropriate conversion for upgrading object of class %s from version %u to version %u",
				    className,
				    (unsigned)curVer,
				    (unsigned)targetVer
				);
				DEBUG_ALWAYS_ASSERT();
				return false;
			}

			if (PATH_NULL == nextVer)  // Edge (i.e. conversion) between curVer and targetVer?
			{
				break;
			}

			// No direct edge (i.e. conversion) => convert to nextVer and continue

			NxParameterized::Interface* nextObj = createNxParameterized(className, nextVer);
			if (!nextObj)
			{
				APEX_DEBUG_WARNING("Failed to create object of class %s and version %u", className, (unsigned)nextVer);
				DEBUG_ALWAYS_ASSERT();
				return false;
			}

			if (!updateLegacyNxParameterized(*curObj, *nextObj))
			{
				// Error message was already reported
				DEBUG_ALWAYS_ASSERT();
				return false;
			}

			if (curObj != &legacyObj)  //legacyObj is owned by app
			{
				curObj->destroy();
			}

			curObj = nextObj;
			curVer = nextVer;
		} // while

		// Direct conversion exists

		NxParameterized::Conversion* conv = info->convs[PxU32Pair(curVer, targetVer)];
		if (!conv)
		{
			APEX_DEBUG_WARNING("Internal error while upgrading object of class %s", className);
			DEBUG_ALWAYS_ASSERT();
			return false;
		}

		if (!(*conv)(*curObj, targetObj))
		{
			APEX_DEBUG_WARNING(
				"User-supplied conversion failed to upgrade object of class %s from version %u to %u",
			    className,
			    (unsigned)curVer,
			    (unsigned)targetVer );
			DEBUG_ALWAYS_ASSERT();
			return false;
		}

		if (curObj != &legacyObj)  //legacyObj is owned by app
		{
			curObj->destroy();
		}

		return true;
	}

	void* alloc(physx::PxU32 nbytes)
	{
		return alloc(nbytes, 16);
	}

	void* alloc(physx::PxU32 nbytes, physx::PxU32 align)
	{
		if (align <= 16)
		{
			return ALIGNED_ALLOC(nbytes, 16);
		}
		else switch (align)
			{
			case 32:
				return ALIGNED_ALLOC(nbytes, 32);
			case 64:
				return ALIGNED_ALLOC(nbytes, 64);
			case 128:
				return ALIGNED_ALLOC(nbytes, 128);
			}

		// Do not support larger alignments

		return 0;
	}

	void free(void* buf)
	{
		ALIGNED_FREE(buf);
	}

	physx::PxI32 incRefCount(physx::PxI32* refCount)
	{
		return physx::atomicIncrement(refCount);
	}

	virtual physx::PxI32 decRefCount(physx::PxI32* refCount)
	{
		return physx::atomicDecrement(refCount);
	}

	virtual void traitsWarn(const char* msg) const
	{
		APEX_DEBUG_WARNING("%s", msg);
	}

private:
	NxResID								mFactoryNsResID;
};

}
} // end namespace physx::apex

#undef DEBUG_FW
#undef PATH_INF
#undef PATH_NULL

#undef DEBUG_ASSERT
#undef DEBUG_ALWAYS_ASSERT

#endif

