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

#ifndef GU_BIG_CONVEX_DATA2_H
#define GU_BIG_CONVEX_DATA2_H

#include "GuBigConvexData.h"
#include "PxMetaData.h"

namespace physx
{
	class PxInputStream;
	class PxSerializationContext;
	class PxDeserializationContext;

	class PX_PHYSX_COMMON_API BigConvexData : public Ps::UserAllocated
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
		public:
// PX_SERIALIZATION
											BigConvexData(const PxEMPTY&)	{}
		static		void					getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
											BigConvexData();
											~BigConvexData();
		// Support vertex map
					bool					Load(PxInputStream& stream);

					PxU32					ComputeOffset(const PxVec3& dir)		const;
					PxU32					ComputeNearestOffset(const PxVec3& dir)	const;

		// Data access
		PX_INLINE	PxU32					GetSubdiv()								const	{ return mData.mSubdiv;											}
		PX_INLINE	PxU32					GetNbSamples()							const	{ return mData.mNbSamples;										}
		//~Support vertex map

		// Valencies
		// Data access
		PX_INLINE	PxU32					GetNbVerts()							const	{ return mData.mNbVerts;										}
		PX_INLINE	const Gu::Valency*		GetValencies()							const	{ return mData.mValencies;										}
		PX_INLINE	PxU16					GetValency(PxU32 i)						const	{ return mData.mValencies[i].mCount;							}
		PX_INLINE	PxU16					GetOffset(PxU32 i)						const	{ return mData.mValencies[i].mOffset;							}
		PX_INLINE	const PxU8*				GetAdjacentVerts()						const	{ return mData.mAdjacentVerts;									}

		PX_INLINE	PxU16					GetNbNeighbors(PxU32 i)					const	{ return mData.mValencies[i].mCount;							}
		PX_INLINE	const PxU8*				GetNeighbors(PxU32 i)					const	{ return &mData.mAdjacentVerts[mData.mValencies[i].mOffset];	}

// PX_SERIALIZATION
					void					exportExtraData(PxSerializationContext& stream);
					void					importExtraData(PxDeserializationContext& context);
//~PX_SERIALIZATION
					Gu::BigConvexRawData	mData;
		protected:
					void*					mVBuffer;
		// Internal methods
					void					CreateOffsets();
					bool					VLoad(PxInputStream& stream);
		//~Valencies
		friend class BigConvexDataBuilder;
	};

}

#endif // BIG_CONVEX_DATA_H

