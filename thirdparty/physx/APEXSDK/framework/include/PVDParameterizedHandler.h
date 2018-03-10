/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PVD_PARAMETERIZED_HANDLER
#define PVD_PARAMETERIZED_HANDLER

#include "PsShare.h"
#ifndef WITHOUT_PVD

#include "PsUserAllocated.h"
#include "PVDBinding.h"

#include "PsHashSet.h"
#include "PsHashMap.h"

namespace physx
{
	namespace debugger
	{
		struct NamespacedName;

		namespace comm
		{
			class PvdDataStream;
		}
	}
}

namespace NxParameterized
{
class Definition;
class Handle;
}

namespace PVD
{
	class StructId
	{
	public:
		StructId(void* address, const char* name) :
		  mAddress(address),
			  mName(name)
		{}

		bool operator<(const StructId& other) const
		{
			if (mAddress < other.mAddress)
				return true;
			else
				return (mAddress == other.mAddress) && strcmp(mName, other.mName) < 0;
		}

		bool operator==(const StructId& other) const
		{
			return (mAddress == other.mAddress) && strcmp(mName, other.mName) == 0;
		}

		operator size_t() const
		{
			return (size_t)mAddress;
		}

	private:
		void* mAddress;
		const char* mName;
	};

	class PvdParameterizedHandler : public physx::shdfnd::UserAllocated
	{
	public:

		PvdParameterizedHandler(physx::debugger::comm::PvdDataStream& pvdStream) :
			 mPvdStream(&pvdStream)
			,mNextStructId(1)
		{
		}

		/**
		\brief Adds properties to the provided pvdClassName and creates classes for Structs that are inside the paramDefinition tree (not for references, though)
		*/
		void									initPvdClasses(const NxParameterized::Definition& paramDefinition, const char* pvdClassName);

		/**
		\brief Updates the provided pvd instance properties with the values in the provided handle, recursively.
		pvdAction specifies if only properties are updated, if pvd instances for structs should be created (for initialization) or if they should be destroyed.
		*/
		void									updatePvd(const void* pvdInstance, NxParameterized::Handle& paramsHandle, PvdAction::Enum pvdAction = PvdAction::UPDATE);

	protected:

		bool									createClass(const physx::debugger::NamespacedName& className);
		bool									getPvdType(const NxParameterized::Definition& def, physx::debugger::NamespacedName& pvdTypeName);
		size_t									getStructId(void* structAddress, const char* structName, bool deleteId);
		const void*								getPvdId(const NxParameterized::Handle& handle, bool deleteId);
		bool									setProperty(const void* pvdInstance, NxParameterized::Handle& propertyHandle, bool isArrayElement, PvdAction::Enum pvdAction);

	
		physx::debugger::comm::PvdDataStream*	mPvdStream;

		physx::shdfnd::HashSet<const char*>		mCreatedClasses;
		physx::shdfnd::HashSet<const void*>		mInstanceIds;

		size_t									mNextStructId;
		physx::shdfnd::HashMap<StructId, size_t>mStructIdMap;
	};

}; // namespacePvdNxParamSerializer

#endif //WITHOUT_PVD

#endif // #ifndef PVD_PARAMETERIZED_HANDLER