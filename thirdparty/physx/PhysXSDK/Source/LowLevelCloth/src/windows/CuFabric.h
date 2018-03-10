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

#pragma once

#include "Fabric.h"
#include "Range.h"
#include "Types.h"
#include "Allocator.h"
#include "CuContextLock.h"
#include "CuDeviceVector.h"

namespace physx
{

namespace cloth
{

struct CuTether
{
	CuTether(uint16_t, uint16_t);
	uint16_t mAnchor;
	uint16_t mLength;
};

class CuFabric : public UserAllocated, private CuContextLock, public Fabric
{
	PX_NOCOPY(CuFabric)
public:
	CuFabric( CuFactory& factory, uint32_t numParticles, 
		Range<const uint32_t> phases, Range<const uint32_t> sets, 
		Range<const float> restvalues, Range<const uint32_t> indices, 
		Range<const uint32_t> anchors, Range<const float> tetherLengths,
		uint32_t id );

	virtual ~CuFabric();
	static void* operator new(size_t n) { return UserAllocated::operator new(n); }
	static void operator delete(void*) {}	

	virtual Factory& getFactory() const;

	virtual uint32_t getNumPhases() const;
	virtual uint32_t getNumRestvalues() const;

	virtual uint32_t getNumSets() const;
	virtual uint32_t getNumIndices() const;

	virtual uint32_t getNumParticles() const;

	virtual uint32_t getNumTethers() const;

	virtual void scaleRestvalues(float);
	virtual void scaleTetherLengths(float);

public:

	CuFactory& mFactory;

	uint32_t mNumParticles;

	CuDeviceVector<uint32_t> mPhases; // index of set to use
	CuDeviceVector<uint32_t> mSets; // offset of first restvalue, with 0 prefix

	CuDeviceVector<float> mRestvalues;
	CuDeviceVector<uint16_t> mIndices;

	CuDeviceVector<CuTether> mTethers;
	float mTetherLengthScale;

	Vector<uint32_t>::Type mNumConstraintsInPhase;
	Vector<CuDevicePointer<const float> >::Type mRestvaluesInPhase;
	Vector<CuDevicePointer<const uint16_t> >::Type mIndicesInPhase;

	uint32_t mId;
};

}

}
