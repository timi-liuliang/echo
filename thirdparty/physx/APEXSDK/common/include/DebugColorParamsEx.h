/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef HEADER_DebugColorParamsListener_h
#define HEADER_DebugColorParamsListener_h

#include "NxParameterized.h"
#include "NxParameters.h"
#include "NxParameterizedTraits.h"

#include "DebugColorParams.h"
#include "NiApexSDK.h"

namespace physx
{
namespace apex
{

#define MAX_COLOR_NAME_LENGTH 32

class DebugColorParamsEx : public DebugColorParams
{
public:
	DebugColorParamsEx(NxParameterized::Traits* traits, NiApexSDK* mSdk) :
		DebugColorParams(traits),
		mApexSdk(mSdk) {}

	~DebugColorParamsEx()
	{
	}

	void destroy()
	{
		this->~DebugColorParamsEx();
		this->DebugColorParams::destroy();
	}

	NxParameterized::ErrorType setParamU32(const NxParameterized::Handle& handle, physx::PxU32 val)
	{
		NxParameterized::ErrorType err = NxParameterized::NxParameters::setParamU32(handle, val);

		NxParameterized::Handle& h = const_cast<NxParameterized::Handle&>(handle);
		char color[MAX_COLOR_NAME_LENGTH];
		h.getLongName(color, MAX_COLOR_NAME_LENGTH);
		mApexSdk->updateDebugColorParams(color, val);

		return err;
	}

private:
	NiApexSDK* mApexSdk;
};

}
} // namespace physx::apex::

#endif