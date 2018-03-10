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


#include "particles/PxParticleCreationData.h"
#include "PxsParticleData.h"
#include "PxsFluidParticle.h"
#include "PsAlignedMalloc.h"
#include "PxMetaData.h"
#include "PxIO.h"
#include "CmUtils.h"

using namespace physx;
using namespace Cm;
//----------------------------------------------------------------------------//

PxsParticleData::PxsParticleData(PxU32 maxParticles, bool perParticleRestOffset)
{	
	mOwnMemory = true;
	mMaxParticles = maxParticles;
	mHasRestOffsets = perParticleRestOffset;
	mValidParticleCount = 0;
	mValidParticleRange = 0;
	mWorldBounds = PxBounds3::empty();
	
	fixupPointers();
	mParticleMap.clear(mMaxParticles);

#if defined(PX_CHECKED)
	{
		PxU32 numWords = mMaxParticles*sizeof(PxsFluidParticle) >> 2;
		for (PxU32 i = 0; i < numWords; ++i)
			reinterpret_cast<PxU32*>(mParticleBuffer)[i] = 0xDEADBEEF;
	}
#endif
}

//----------------------------------------------------------------------------//

PxsParticleData::PxsParticleData(PxvParticleSystemStateDataDesc& particles, const PxBounds3& bounds)
{	
	mOwnMemory = true;
	mMaxParticles = particles.maxParticles;
	mHasRestOffsets = (particles.restOffsets.ptr() != NULL);
	mValidParticleCount = particles.numParticles;
	mValidParticleRange = particles.validParticleRange;
	mWorldBounds = bounds;

	fixupPointers();
	if (particles.bitMap)
		mParticleMap.copy(*particles.bitMap);
	else
		mParticleMap.clear(mMaxParticles);

	if (mValidParticleRange > 0)
	{
		for (PxU32 i = 0; i < mValidParticleRange; ++i)
			mParticleBuffer[i].flags.api = PxParticleFlags(0);

		for (PxU32 w = 0; w <= (mValidParticleRange-1) >> 5; w++)
			for (PxU32 b = mParticleMap.getWords()[w]; b; b &= b-1)
			{
				PxU32 index = (w<<5|Ps::lowestSetBit(b));
				PxsFluidParticle& dstParticle = mParticleBuffer[index];
				dstParticle.position = particles.positions[index];
				dstParticle.velocity = particles.velocities[index];
				dstParticle.density = 0.0f;	
				dstParticle.flags.low = 0;
				dstParticle.flags.api = PxParticleFlag::eVALID;
			}

		if (mHasRestOffsets)
		{
			PX_ASSERT(mRestOffsetBuffer);
			for (PxU32 w = 0; w <= (mValidParticleRange-1) >> 5; w++)
				for (PxU32 b = mParticleMap.getWords()[w]; b; b &= b-1)
				{
					PxU32 index = (w<<5|Ps::lowestSetBit(b));
					mRestOffsetBuffer[index] = particles.restOffsets[index];
				}
		}
	}
}

//----------------------------------------------------------------------------//

PxsParticleData::PxsParticleData(PxU8* address)
{
	PX_ASSERT(address == (PxU8*)this);
	PX_UNUSED(address);
	mOwnMemory = false;
	fixupPointers();
}

//----------------------------------------------------------------------------//

PxsParticleData::~PxsParticleData()
{
	Ps::AlignedAllocator<16> align16;

	if (mParticleBuffer)
		align16.deallocate(mParticleBuffer);
}

//----------------------------------------------------------------------------//

void PxsParticleData::fixupPointers()
{
	PX_ASSERT(size_t(this) % 16 == 0);
	PxU8* address = (PxU8*)this;

	address += getHeaderSize();
	PxU32 bitmapSize = getBitmapSize(mMaxParticles);
	mParticleMap.importData( bitmapSize/4, (PxU32*)address);
	address += (bitmapSize + 15) & ~15;
	mParticleBuffer = (PxsFluidParticle*)address;
	address += getParticleBufferSize(mMaxParticles);
	mRestOffsetBuffer = mHasRestOffsets ? (PxF32*)address : NULL;
	address += getRestOffsetBufferSize(mMaxParticles, mHasRestOffsets);	
}

//----------------------------------------------------------------------------//

void PxsParticleData::exportData(PxSerializationContext& stream)
{
	clearSimState();
	stream.alignData(16);
	stream.writeData(this, PxsParticleData::getTotalSize(mMaxParticles, mHasRestOffsets));	
}

void PxsParticleData::getBinaryMetaData(PxOutputStream& stream)
{
	//define PxvParticleFlags
	PX_DEF_BIN_METADATA_CLASS(stream,	PxvParticleFlags)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxvParticleFlags, PxU16, api, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxvParticleFlags, PxU16, low, 0)

	//define PxsFluidParticle
	PX_DEF_BIN_METADATA_CLASS(stream,	PxsFluidParticle)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsFluidParticle, PxVec3, position, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsFluidParticle, PxReal, density, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsFluidParticle, PxVec3, velocity, 0)
	PX_DEF_BIN_METADATA_ITEM(stream,	PxsFluidParticle, PxvParticleFlags, flags, 0)

	//define PxsParticleData
	PX_DEF_BIN_METADATA_VCLASS(stream,		PxsParticleData)

	PX_DEF_BIN_METADATA_ITEM(stream,		PxsParticleData, bool,				mOwnMemory,		    0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsParticleData, PxU32,				mMaxParticles,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsParticleData, bool,				mHasRestOffsets,	0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsParticleData, PxU32,				mValidParticleRange,0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsParticleData, PxU32,				mValidParticleCount,0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsParticleData, PxBounds3,			mWorldBounds,		0)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsParticleData, PxsFluidParticle,	mParticleBuffer,	PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsParticleData, PxReal,			mRestOffsetBuffer,	PxMetaDataFlag::ePTR)
	PX_DEF_BIN_METADATA_ITEM(stream,		PxsParticleData, BitMap,			mParticleMap,		0)

	//extra data
	PX_DEF_BIN_METADATA_EXTRA_ARRAY(stream,	PxsParticleData, PxsFluidParticle, mMaxParticles, 16, 0)
	PX_DEF_BIN_METADATA_EXTRA_ITEMS(stream,	PxsParticleData, PxReal, mHasRestOffsets, mMaxParticles, 0, 16)	
}

//----------------------------------------------------------------------------//

void PxsParticleData::clearSimState()
{
	if (mValidParticleRange > 0)
	{
		for (PxU32 w = 0; w <= (mValidParticleRange-1) >> 5; w++)
			for (PxU32 b = mParticleMap.getWords()[w]; b; b &= b-1)
			{
				PxU32 index = (w<<5|Ps::lowestSetBit(b));
				PxsFluidParticle& dstParticle = mParticleBuffer[index];
				dstParticle.flags.low = 0;
				dstParticle.density = 0.0f;			
			}
	}
}

//----------------------------------------------------------------------------//

void PxsParticleData::onOriginShift(const PxVec3& shift)
{
	if (mValidParticleRange > 0)
	{
		for (PxU32 w = 0; w <= (mValidParticleRange-1) >> 5; w++)
			for (PxU32 b = mParticleMap.getWords()[w]; b; b &= b-1)
			{
				PxU32 index = (w<<5|Ps::lowestSetBit(b));
				PxsFluidParticle& particle = mParticleBuffer[index];
				particle.position -= shift;
			}
	}

	mWorldBounds.minimum -= shift;
	mWorldBounds.maximum -= shift;
}

//----------------------------------------------------------------------------//

PxsParticleData* PxsParticleData::create(PxvParticleSystemStateDataDesc& particles, const PxBounds3& bounds)
{
	Ps::AlignedAllocator<16, Ps::ReflectionAllocator<PxsParticleData> > align16;
	PxU32 totalSize = getTotalSize(particles.maxParticles, particles.restOffsets.ptr() != NULL);
	PxsParticleData* mem = (PxsParticleData*)align16.allocate(totalSize, __FILE__, __LINE__);
	markSerializedMem(mem, totalSize);
	PX_PLACEMENT_NEW(mem, PxsParticleData)(particles, bounds);
	return mem;
}

//----------------------------------------------------------------------------//

PxsParticleData* PxsParticleData::create(PxU32 maxParticles, bool perParticleRestOffsets)
{
	Ps::AlignedAllocator<16, Ps::ReflectionAllocator<PxsParticleData> > align16;
	PxU32 totalSize = getTotalSize(maxParticles, perParticleRestOffsets);
	PxsParticleData* mem = (PxsParticleData*)align16.allocate(totalSize, __FILE__, __LINE__);
	markSerializedMem(mem, totalSize);
	PX_PLACEMENT_NEW(mem, PxsParticleData)(maxParticles, perParticleRestOffsets);
	return mem;
}

//----------------------------------------------------------------------------//

PxsParticleData* PxsParticleData::create(PxDeserializationContext& context)
{
	PxsParticleData* mem = context.readExtraData<PxsParticleData, PX_SERIAL_ALIGN>();
	new(mem)PxsParticleData(reinterpret_cast<PxU8*>(mem));
	context.readExtraData<PxU8>(getDataSize(mem->getMaxParticles(), mem->getRestOffsetBuffer() != NULL));
	return mem;
}

//----------------------------------------------------------------------------//

void PxsParticleData::release()
{
	if (!mOwnMemory)
		return;

	Ps::AlignedAllocator<16> align16;
	align16.deallocate(this);
}

//----------------------------------------------------------------------------//

bool PxsParticleData::addParticlesV(const PxParticleCreationData& creationData)
{
	PX_ASSERT(creationData.numParticles <= mMaxParticles);
	PX_ASSERT(creationData.indexBuffer.ptr() && creationData.positionBuffer.ptr());
	PX_ASSERT((mRestOffsetBuffer != NULL) == (creationData.restOffsetBuffer.ptr() != NULL));

	const PxVec3 zeroVector(0.0f);

	PxStrideIterator<const PxU32> indexIt = creationData.indexBuffer;
	PxStrideIterator<const PxVec3> positionIt = creationData.positionBuffer;
	PxStrideIterator<const PxVec3> velocityIt = creationData.velocityBuffer.ptr() ? creationData.velocityBuffer : PxStrideIterator<const PxVec3>(&zeroVector, 0);

	for(PxU32 i=0; i < creationData.numParticles; i++)
	{
		const PxU32 particleIndex = *indexIt;
		PX_ASSERT(particleIndex <= mMaxParticles);

		PxsFluidParticle& particle = mParticleBuffer[particleIndex];
		PX_ASSERT(!mParticleMap.test(particleIndex));
		mParticleMap.set(particleIndex);

		if (particleIndex+1 > mValidParticleRange)
		{
			mValidParticleRange = particleIndex+1;
		}
		else
		{
			PX_ASSERT(!(particle.flags.api & PxParticleFlag::eVALID));
		}

		particle.position = *positionIt;
		particle.velocity = *velocityIt;
		particle.flags.low = 0;
		particle.flags.api = PxParticleFlag::eVALID;
		particle.density = 0.0f;
	
		mWorldBounds.include(particle.position);

		positionIt++;
		velocityIt++;
		indexIt++;
	}

	if (mRestOffsetBuffer)
	{
		PxStrideIterator<const PxF32> restOffsetIt = creationData.restOffsetBuffer;
		indexIt = creationData.indexBuffer;

		for(PxU32 i=0; i < creationData.numParticles; i++)
		{
			const PxU32 particleIndex = *indexIt;
			mRestOffsetBuffer[particleIndex] = *restOffsetIt;
			restOffsetIt++;
			indexIt++;
		}
	}
		
	mValidParticleCount += creationData.numParticles;
	return true;
}

//----------------------------------------------------------------------------//

void PxsParticleData::removeParticlesV(PxU32 count, const PxStrideIterator<const PxU32>& indices)
{
	for (PxU32 i = 0; i < count; ++i)
		removeParticle(indices[i]);

	mValidParticleCount -= count;
	mValidParticleRange = (mValidParticleCount > 0) ? mParticleMap.findLast() + 1 : 0;
}

//----------------------------------------------------------------------------//

void PxsParticleData::removeParticlesV()
{
	Cm::BitMap::Iterator it(mParticleMap);
	for (PxU32 particleIndex = it.getNext(); particleIndex != Cm::BitMap::Iterator::DONE; particleIndex = it.getNext())
		removeParticle(particleIndex);

	mValidParticleCount = 0;
	mValidParticleRange = 0;
	PX_ASSERT(mValidParticleCount == 0);
}

//----------------------------------------------------------------------------//

PxU32 PxsParticleData::getParticleCountV() const
{
	return mValidParticleCount;
}

//----------------------------------------------------------------------------//

/**
In the non-gpu implementation the full state is always available.
*/
void PxsParticleData::getParticlesV(PxvParticleSystemStateDataDesc& particles, bool /*fullState*/, bool) const
{
	PX_ASSERT(mValidParticleCount <= mMaxParticles);

	particles.bitMap = &mParticleMap;
	particles.numParticles = mValidParticleCount;
	particles.maxParticles = mMaxParticles;
	particles.validParticleRange = mValidParticleRange;

	if (mValidParticleCount == 0)
	{	
		particles.positions			= PxStrideIterator<const PxVec3>();
		particles.velocities		= PxStrideIterator<const PxVec3>();
		particles.flags				= PxStrideIterator<const PxvParticleFlags>();
		particles.restOffsets		= PxStrideIterator<const PxF32>();
	}
	else
	{
		PX_ASSERT(mParticleBuffer);
		particles.positions			= PxStrideIterator<const PxVec3>(&mParticleBuffer->position, sizeof(PxsFluidParticle));
		particles.velocities		= PxStrideIterator<const PxVec3>(&mParticleBuffer->velocity, sizeof(PxsFluidParticle));
		particles.flags				= PxStrideIterator<const PxvParticleFlags>(&mParticleBuffer->flags, sizeof(PxsFluidParticle));
		particles.restOffsets		= mRestOffsetBuffer ? PxStrideIterator<const PxF32>(mRestOffsetBuffer) : PxStrideIterator<const PxF32>();
	}
}

//----------------------------------------------------------------------------//

void PxsParticleData::setPositionsV(PxU32 numParticles, const PxStrideIterator<const PxU32>& indices, const PxStrideIterator<const PxVec3>& positions)
{
	PX_ASSERT(indices.ptr() && positions.ptr());

	PxStrideIterator<const PxU32> indexIt(indices);
	PxStrideIterator<const PxVec3> positionIt(positions);

	for(PxU32 i = 0; i != numParticles; ++i)
	{
		PxU32 particleIndex = *indexIt++;
		PX_ASSERT(particleIndex <= mMaxParticles);
		PX_ASSERT(mParticleMap.test(particleIndex));
		PxsFluidParticle& particle = mParticleBuffer[particleIndex];
		particle.position = *positionIt++;
		mWorldBounds.include(particle.position);
	}
}

//----------------------------------------------------------------------------//

void PxsParticleData::setVelocitiesV(PxU32 numParticles, const PxStrideIterator<const PxU32>& indices, const PxStrideIterator<const PxVec3>& velocities)
{
	PX_ASSERT(indices.ptr() && velocities.ptr());

	PxStrideIterator<const PxU32> indexIt(indices);
	PxStrideIterator<const PxVec3> velocityIt(velocities);

	for(PxU32 i = 0; i != numParticles; ++i)
	{
		PxU32 particleIndex = *indexIt++;
		PX_ASSERT(particleIndex <= mMaxParticles);
		PX_ASSERT(mParticleMap.test(particleIndex));
		PxsFluidParticle& particle = mParticleBuffer[particleIndex];
		particle.velocity = *velocityIt++;
	}
}

//----------------------------------------------------------------------------//

void PxsParticleData::setRestOffsetsV(PxU32 numParticles, const PxStrideIterator<const PxU32>& indices, const PxStrideIterator<const PxF32>& restOffsets)
{
	PX_ASSERT(indices.ptr() && restOffsets.ptr());

	PxStrideIterator<const PxU32> indexIt(indices);
	PxStrideIterator<const PxF32> restOffsetIt(restOffsets);

	for(PxU32 i = 0; i != numParticles; ++i)
	{
		PxU32 particleIndex = *indexIt++;
		PX_ASSERT(particleIndex <= mMaxParticles);
		PX_ASSERT(mParticleMap.test(particleIndex));
		mRestOffsetBuffer[particleIndex] = *restOffsetIt++;
	}
}

//----------------------------------------------------------------------------//

void PxsParticleData::addDeltaVelocitiesV(const Cm::BitMap& bufferMap, const PxVec3* buffer, PxReal multiplier)
{
	Cm::BitMap::Iterator it(bufferMap);
	for (PxU32 particleIndex = it.getNext(); particleIndex != Cm::BitMap::Iterator::DONE; particleIndex = it.getNext())
	{
		PX_ASSERT(mParticleMap.boundedTest(particleIndex));
		mParticleBuffer[particleIndex].velocity += buffer[particleIndex]*multiplier;
	}
}

//----------------------------------------------------------------------------//

PxBounds3 PxsParticleData::getWorldBoundsV() const
{
	return mWorldBounds;
}

//----------------------------------------------------------------------------//

PxU32 PxsParticleData::getMaxParticlesV() const
{
	return mMaxParticles;
}

//----------------------------------------------------------------------------//

PX_FORCE_INLINE void PxsParticleData::removeParticle(PxU32 particleIndex)
{
	PX_ASSERT(particleIndex <= mMaxParticles);

	PxsFluidParticle& particle = mParticleBuffer[particleIndex];
	PX_ASSERT(particle.flags.api & PxParticleFlag::eVALID);
	PX_ASSERT(mParticleMap.test(particleIndex));

#if defined(PX_CHECKED)
	for (PxU32 i = 0; i < sizeof(PxsFluidParticle) >> 2; ++i)
		reinterpret_cast<PxU32*>(&particle)[i] = 0xDEADBEEF;
#endif
	particle.flags.api = PxParticleFlags(0);
	mParticleMap.reset(particleIndex);
}

//----------------------------------------------------------------------------//


