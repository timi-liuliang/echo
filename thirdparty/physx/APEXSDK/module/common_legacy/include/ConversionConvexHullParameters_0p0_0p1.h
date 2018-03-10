/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONCONVEXHULLPARAMETERS_0P0_0P1H_H
#define CONVERSIONCONVEXHULLPARAMETERS_0P0_0P1H_H

#include "ParamConversionTemplate.h"
#include "ConvexHullParameters_0p0.h"
#include "ConvexHullParameters_0p1.h"

#include "ApexSharedUtils.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<ConvexHullParameters_0p0, ConvexHullParameters_0p1, 0, 1> ConversionConvexHullParameters_0p0_0p1Parent;

class ConversionConvexHullParameters_0p0_0p1: ConversionConvexHullParameters_0p0_0p1Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionConvexHullParameters_0p0_0p1));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionConvexHullParameters_0p0_0p1)(t) : 0;
	}

protected:
	ConversionConvexHullParameters_0p0_0p1(NxParameterized::Traits* t) : ConversionConvexHullParameters_0p0_0p1Parent(t) {}

	const NxParameterized::PrefVer* getPreferredVersions() const
	{
		static NxParameterized::PrefVer prefVers[] =
		{
			//TODO:
			//	Add your preferred versions for included references here.
			//	Entry format is
			//		{ (const char*)longName, (PxU32)preferredVersion }

			{ 0, 0 } // Terminator (do not remove!)
		};

		return prefVers;
	}

	bool convert()
	{
		// The 0.0 format does not have the adjacentFaces array.  We will simply rebuild the convex hull from the
		// old version's vertices.

		physx::ConvexHull hull;
		hull.init();

		hull.buildFromPoints(mLegacyData->vertices.buf, (physx::PxU32)mLegacyData->vertices.arraySizes[0], (physx::PxU32)mLegacyData->vertices.elementSize);
		mNewData->copy(*hull.mParams);

		return true;
	}
};

}
}
} // namespace physx::apex

#endif
