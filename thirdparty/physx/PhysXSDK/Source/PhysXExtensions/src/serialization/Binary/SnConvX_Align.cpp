/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "SnConvX.h"
#include "PxSerialFramework.h"
#include "CmUtils.h"

using namespace physx;

void Sn::ConvX::alignTarget(int alignment)
{
	const int outputSize = getCurrentOutputSize();
	const PxU32 outPadding = Cm::getPadding(size_t(outputSize), (PxU32)alignment);
	if(outPadding)
	{
		assert(outPadding<CONVX_ZERO_BUFFER_SIZE);
		output(mZeros, (int)outPadding);
	}
}

const char* Sn::ConvX::alignStream(const char* buffer, int alignment)
{
	const PxU32 padding = Cm::getPadding(size_t(buffer), (PxU32)alignment);
	assert(!Cm::getPadding(size_t(buffer + padding), (PxU32)alignment));

	const int outputSize = getCurrentOutputSize();
	const PxU32 outPadding = Cm::getPadding(size_t(outputSize), (PxU32)alignment);
	if(outPadding==padding)
	{
		assert(outPadding<CONVX_ZERO_BUFFER_SIZE);
		output(mZeros, (int)outPadding);
	}
	else if(outPadding)
	{
		assert(outPadding<CONVX_ZERO_BUFFER_SIZE);
		output(mZeros, (int)outPadding);
	}

	return buffer + padding;
}
