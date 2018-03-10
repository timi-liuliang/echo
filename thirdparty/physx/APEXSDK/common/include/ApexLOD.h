/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_LOD_H
#define APEX_LOD_H

#include "PsShare.h"
#include "PsAlloca.h"
#include "PsUserAllocated.h"
#include "PsSort.h"

namespace physx
{
namespace apex
{

class LODNode
{
public:
	LODNode() : cachedBenefit(0.0f) {}
	virtual PxF32 getBenefit() = 0;
	virtual PxF32 setResource(PxF32 suggested, PxF32 maxRemaining, PxF32 relativeBenefit) = 0;
	// This method 'can' be implemented by LOD Nodes. If not all resources have been distributed, it will allow each node
	// to increase it's resource by maxRemaining. It should return the size of the increase.
	virtual PxF32 increaseResource(PxF32 /*maxRemaining*/) { return 0.0f; }
	PX_INLINE PxF32 getCachedBenefit()
	{
		return cachedBenefit;
	}
	PX_INLINE PxF32 computeBenefit()
	{
		cachedBenefit = getBenefit();
		return cachedBenefit;
	}

private:
	PxF32 cachedBenefit;
};


template<class LODAble>	//LODAble must have computeBenefit(), getCachedBenefit(), setResource()
class LODCollection
{
protected:
	struct Child
	{
		LODAble* node;
		PxF32 benefitCache;
	};

	class LODCollectionChildSort
	{
	public:
		inline bool operator()(const Child& a, const Child& b) const
		{
			// PH: Sort smallest first, otherwise the order of distributing the resources is wrong
			return a.benefitCache < b.benefitCache;
		}
	};

public:
	static PxF32 computeSumBenefit(LODAble** begin, LODAble** end)
	{
		PxF32 sumBenefit = 0.0f;
		for (LODAble** i = begin; i != end; i++)
		{
			PxF32 b = (*i)->computeBenefit();	//should cache as a side effect.
			PX_ASSERT(PxIsFinite(b));
			sumBenefit += b;
		}

		return sumBenefit;
	}

	static PxF32 distributeResource(LODAble** begin, LODAble** end, PxF32 sumBenefit, PxF32 relativeSumBenefit, PxF32 resourceBudget)   //returns qty taken (may round up)
	{
		/*
		simple distribute:
		for (PxU32 i = 0; i < children.size(); i++)
		{
		PxF32 relBenefit = children[i].benefitCache/sumBenefit;
		PxF32 rSuggested = relBenefit * resourceBudget;
		PxF32 taken = children[i].node->setResource(rSuggested, rSuggested);
		}
		*/

		//with remainders:
		PxU32 numChildren = static_cast<PxU32>(end - begin);

		//early out
		if (0 == numChildren)
		{
			return 0.0f;
		}

		//sort according to benefit to make this order independent:
		//Child *children = (Child*)PxAlloca( sizeof(Child) * numChildren );
		PX_ALLOCA(children, Child, numChildren);

		//init children:
		Child* c = children;
		for (LODAble** i = begin; i != end; i++, c++)
		{
			c->node = (*i);
			c->benefitCache = (*i)->getCachedBenefit();

			PX_ASSERT(PxIsFinite(c->benefitCache));
			if (!PxIsFinite(c->benefitCache))
			{
				c->benefitCache = 0.0f;
			}
		}

		if (numChildren > 1)
		{
			shdfnd::sort((Child*)children, numChildren, LODCollectionChildSort());
		}


		PxF32 resourceRemaining = resourceBudget;
		PxF32 sumBenefitOfRemaining = sumBenefit;
		PxF32 resourceUsed = 0.0f;

		for (Child* child = children; child != children + numChildren; ++child)
		{
			PxF32 relBenefit = (sumBenefitOfRemaining > 0.0f) ?  child->benefitCache / sumBenefitOfRemaining : 0.0f;
			PxF32 exactRelBenefit = (sumBenefit > 0.0f) ? relativeSumBenefit * (child->benefitCache / sumBenefit) : 0.0f;

			sumBenefitOfRemaining -= child->benefitCache;

			PxF32 rSuggested = relBenefit * resourceRemaining;
			PxF32 taken = child->node->setResource(rSuggested, resourceRemaining, exactRelBenefit);//as cloth takes time to respond this can take more than the remaining figure passed.
			//PX_ASSERT(taken <= resourceRemaining);

			//its possible for taken to be > than resourceRemaining if its e.g. taking us too long to smoothly drop down lod levels!
			//it is therefore possible for us to go temporarily over budget if the budget pool is reduced on us.
			resourceRemaining -= taken;
			resourceUsed += taken;
			if (resourceRemaining <= 0.0f)	//clamp
			{
				resourceRemaining = 0.0f;
			}
		}

		if (resourceRemaining > 0.0f)
		{
			// PH: Second iteration.
			// As it turns out it's possible that not all resources are used up even if all the children could make use of them.
			// This is mainly due to the fact that the resources are split by benefit ratios.
			// To circumvent these problems, the remaining resources are given away in a first-come first-serve manner.

			Child* child = children + numChildren;
			while (child-- != children)
			{
				PxF32 taken = child->node->increaseResource(resourceRemaining);
				resourceRemaining -= taken;
				PX_ASSERT(resourceRemaining >= 0.0f);
			}
		}

		return resourceUsed;
	}
};


/*
	LOD helper functions
*/

PX_INLINE PxF32 solidAngleImportance(PxF32 distanceSquared, PxF32 radiusSquared)
{
	const PxF32 nearDistanceSquared = PxPi * radiusSquared;

	if (distanceSquared <= nearDistanceSquared)
	{
		return 1.0f;
	}
	return nearDistanceSquared / distanceSquared;
}

PX_INLINE PxF32 solidAngleFiniteRangeImportance(PxF32 distanceSquared, PxF32 radiusSquared, PxF32 farDistanceSquared)
{
	const PxF32 nearDistanceSquared = PxPi * radiusSquared;
	const PxF32 squareRange = farDistanceSquared - nearDistanceSquared;

	if (squareRange <= 0.0f)
	{
		return 1.0f;
	}
	if (distanceSquared <= nearDistanceSquared)
	{
		return 1.0f;
	}
	if (distanceSquared >= farDistanceSquared)
	{
		return 0.0f;
	}
	return nearDistanceSquared * (farDistanceSquared - distanceSquared) / (distanceSquared * squareRange);
}

PX_INLINE PxF32 angularImportance(PxF32 distance, PxF32 radius)
{
	if (distance <= radius)
	{
		return 1.0f;
	}
	return radius / distance;
}

PX_INLINE PxF32 angularFiniteRangeImportance(PxF32 distance, PxF32 radius, PxF32 farDistance)
{
	const PxF32 range = farDistance - radius;

	if (range <= 0.0f)
	{
		return 1.0f;
	}
	if (distance <= radius)
	{
		return 1.0f;
	}
	if (distance >= farDistance)
	{
		return 0.0f;
	}
	return radius * (farDistance - distance) / (distance * range);
}

PX_INLINE PxF32 ageImportance(PxF32 age, PxF32 maximumAge)
{
	if (age <= 0.0f)
	{
		return 1.0f;
	}
	if (age >= maximumAge)
	{
		return 0.0f;
	}
	return 1.0f - age / maximumAge;
}

template<int ScalarCount>
class LodVolumeAccumulator
{
public:
	LodVolumeAccumulator()
	{
		clear();
	}

	void clear()
	{
		mVolume = 0.0f;
		mVariance = 0.0f;
		mCentroid = PxVec3(0.0f);
		for (PxU32 scalarNum = 0; scalarNum < ScalarCount; ++scalarNum)
		{
			mScalarAverage[scalarNum] = 0.0f;
		}
	}

	void accumulate(const PxVec3& position, PxF32 radius, PxF32 scalars[ScalarCount])
	{
		const PxF32 radiusSquared = radius * radius;
		const PxF32 volume = 4.1887902f * PxAbs(radius) * radiusSquared;
		mVolume += volume;
		mCentroid += position * volume;
		mVariance += (0.6f * radiusSquared + position.magnitudeSquared()) * volume;
		for (PxU32 scalarNum = 0; scalarNum < ScalarCount; ++scalarNum)
		{
			mScalarAverage[scalarNum] += scalars[scalarNum] * volume;
		}
	}

	void process()
	{
		if (mVolume > 0.0f)
		{
			const PxF32 norm = 1.0f / mVolume;
			mCentroid *= norm;
			mVariance = mVariance * norm - mCentroid.magnitudeSquared();
			for (PxU32 scalarNum = 0; scalarNum < ScalarCount; ++scalarNum)
			{
				mScalarAverage[scalarNum] *= norm;
			}
		}
	}

	PxVec3	getCentroid() const
	{
		return mCentroid;
	}

	PxF32	getEffectiveRadiusSquared() const
	{
		return 1.66666667f * mVariance;
	}

	PxF32	getScalarAverage(PxU32 scalarNum) const
	{
		PX_ASSERT(scalarNum < ScalarCount);
		return scalarNum < ScalarCount ? mScalarAverage[scalarNum] : 0.0f;
	}

private:
	PxF32	mVolume;
	PxF32	mVariance;
	PxVec3	mCentroid;
	PxF32	mScalarAverage[ScalarCount];
};

template<int ScalarCount>
class LodPointAccumulator
{
public:
	LodPointAccumulator()
	{
		clear();
	}

	void clear()
	{
		mCount = 0;
		mVariance = 0.0f;
		mCentroid = PxVec3(0.0f);
		for (PxU32 scalarNum = 0; scalarNum < ScalarCount; ++scalarNum)
		{
			mScalarAverage[scalarNum] = 0.0f;
		}
	}

	void accumulate(const PxVec3& position, PxF32 scalars[ScalarCount])
	{
		++mCount;
		mCentroid += position;
		mVariance += position.magnitudeSquared();
		for (PxU32 scalarNum = 0; scalarNum < ScalarCount; ++scalarNum)
		{
			mScalarAverage[scalarNum] += scalars[scalarNum];
		}
	}

	void process()
	{
		if (mCount > 0)
		{
			const PxF32 norm = 1.0f / mCount;
			mCentroid *= norm;
			mVariance = mVariance * norm - mCentroid.magnitudeSquared();
			for (PxU32 scalarNum = 0; scalarNum < ScalarCount; ++scalarNum)
			{
				mScalarAverage[scalarNum] *= norm;
			}
		}
	}

	PxVec3	getCentroid() const
	{
		return mCentroid;
	}

	PxF32	getEffectiveRadiusSquared() const
	{
		return 1.66666667f * mVariance;
	}

	PxF32	getScalarAverage(PxU32 scalarNum) const
	{
		PX_ASSERT(scalarNum < ScalarCount);
		return scalarNum < ScalarCount ? mScalarAverage[scalarNum] : 0.0f;
	}

private:
	PxU32	mCount;
	PxF32	mVariance;
	PxVec3	mCentroid;
	PxF32	mScalarAverage[ScalarCount];
};

} // namespace physx
} // namespace apex


#endif
