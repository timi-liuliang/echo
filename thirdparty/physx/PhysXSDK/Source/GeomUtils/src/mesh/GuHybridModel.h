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

#ifndef OPC_HYBRIDMODEL_H
#define OPC_HYBRIDMODEL_H

#include "PsUserAllocated.h"
#include "GuRTree.h"
#include "PxMetaData.h"

namespace physx
{
class PxInputStream;
namespace Gu
{

#if defined(PX_VC) 
    #pragma warning(push)   
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif
	// AP: It looks like this class doesn't need to exist and should be cleaned up
	class PX_PHYSX_COMMON_API RTreeMidphase : public physx::shdfnd::UserAllocated
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
		public:
// PX_SERIALIZATION
												RTreeMidphase(const PxEMPTY&) : mRTree(PxEmpty)
												{
												}
		static			void					getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
												RTreeMidphase();
												~RTreeMidphase();

		PX_FORCE_INLINE	void					getRTreeMidphaseData(RTreeMidphaseData& data)	const
												{
													data.mIMesh			= mIMesh;
													data.mRTree			= &mRTree;
													data.mGeomEpsilon	= mGeomEpsilon;
												}

		PX_FORCE_INLINE	const MeshInterface*	GetMeshInterface()						const	{ return mIMesh;		}
		PX_FORCE_INLINE	void					SetMeshInterface(const MeshInterface* imesh)	{ mIMesh = imesh;		}

// PX_SERIALIZATION
						void					exportExtraData(PxSerializationContext&);
						void					importExtraData(PxDeserializationContext& context);
//~PX_SERIALIZATION

		protected:
						const MeshInterface*	mIMesh;			//!< User-defined mesh interface
		public:
						PxReal					mGeomEpsilon;	//!< see comments in cooking code referencing this variable
						Gu::RTree				mRTree;
	};

#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

} // namespace Gu

}

#endif // OPC_HYBRIDMODEL_H
