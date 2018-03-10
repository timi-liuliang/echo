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


#ifndef PXS_PARTICLE_SHAPE_H
#define PXS_PARTICLE_SHAPE_H

#include "PxsFluidConfig.h"
#include "PxTransform.h"
#include "PxBounds3.h"
#include "PxcGridCellVector.h"
#include "PxsFluidSpatialHash.h"
#include "PxvParticleShape.h"

namespace physx
{

class PxsContext;

class PxsParticleShape : public PxvParticleShape
{
public:
											PxsParticleShape(PxsContext* context, PxU32 index);
	virtual									~PxsParticleShape();

			void							init(class PxsParticleSystemSim* particleSystem, const PxsParticleCell* packet);

	// Implements PxsParticleShape
	virtual	PxBounds3						getBoundsV()						const	{ return mBounds; }
	virtual void							setUserDataV(void* data)					{ mUserData = data; }
	virtual void*							getUserDataV()						const	{ return mUserData; }
	virtual void							destroyV();
	//~Implements PxsParticleShape

	PX_FORCE_INLINE void								setFluidPacket(const PxsParticleCell* packet){ PX_ASSERT(packet); mPacket = packet; }
	PX_FORCE_INLINE	const PxsParticleCell*				getFluidPacket()					const	{ return mPacket;			}

	PX_FORCE_INLINE	PxU32								getIndex()							const	{ return mIndex;			}
	PX_FORCE_INLINE	class PxsParticleSystemSim*			getParticleSystem()							{ return mParticleSystem;	}
	PX_FORCE_INLINE	const class PxsParticleSystemSim*	getParticleSystem()					const	{ return mParticleSystem;	}
	PX_FORCE_INLINE PxcGridCellVector					getPacketCoordinates()				const	{ return mPacketCoordinates; }

private:
	PxU32						mIndex;
	class PxsParticleSystemSim*	mParticleSystem;
	PxBounds3					mBounds;
	PxcGridCellVector			mPacketCoordinates;	//This is needed for the remapping process.
	const PxsParticleCell*		mPacket;
	void*						mUserData;
};

}

#endif // PXS_PARTICLE_SHAPE_H
