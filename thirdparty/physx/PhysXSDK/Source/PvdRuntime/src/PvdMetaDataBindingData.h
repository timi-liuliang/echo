/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  
#ifndef PX_META_DATA_PVD_BINDING_DATA_H
#define PX_META_DATA_PVD_BINDING_DATA_H
#include "PxSimpleTypes.h"
#include "PsArray.h"
#include "PsHashSet.h"
#include "PsHashMap.h"

namespace physx
{
namespace Pvd
{
	using namespace physx::debugger;
	using namespace physx::shdfnd;

	typedef HashSet<const PxRigidActor*>					OwnerActorsValueType;
	typedef HashMap<const PxShape*, OwnerActorsValueType*>	OwnerActorsMap;

	struct PvdMetaDataBindingData : public UserAllocated
	{
		Array<PxU8>							mTempU8Array;
		Array<PxActor*>						mActors;
		Array<PxArticulation*>				mArticulations;
		Array<PxArticulationLink*>			mArticulationLinks;
		HashSet<PxActor*>					mSleepingActors;
		OwnerActorsMap						mOwnerActorsMap;

		PvdMetaDataBindingData() :
			mTempU8Array(PX_DEBUG_EXP("TempU8Array")),
			mActors(PX_DEBUG_EXP("PxActor")),
			mArticulations(PX_DEBUG_EXP("Articulations")),
			mArticulationLinks(PX_DEBUG_EXP("ArticulationLinks")),
			mSleepingActors(PX_DEBUG_EXP("SleepingActors"))	
		{
		}

		template<typename TDataType>
		TDataType* allocateTemp( PxU32 numItems )
		{
			mTempU8Array.resize( numItems * sizeof( TDataType ) );
			if ( numItems )
				return reinterpret_cast<TDataType*>( mTempU8Array.begin() );
			else
				return NULL;
		}

		DataRef<const PxU8> tempToRef() { return DataRef<const PxU8>( mTempU8Array.begin(), mTempU8Array.size() ); }
	};
}

}

#endif
