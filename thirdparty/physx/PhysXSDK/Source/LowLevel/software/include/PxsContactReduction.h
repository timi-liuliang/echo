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

#ifndef PXS_CONTACT_REDUCTION_H
#define PXS_CONTACT_REDUCTION_H

#include "GuContactPoint.h"
#include "PxsMaterialManager.h"

namespace physx
{

//KS - might be OK with 4 but 5 guarantees the deepest + 4 contacts that contribute to largest surface area
#define CONTACT_REDUCTION_MAX_CONTACTS 6
#define CONTACT_REDUCTION_MAX_PATCHES 32
#define PXS_NORMAL_TOLERANCE 0.995f
#define PXS_SEPARATION_TOLERANCE 0.001f


	//A patch contains a normal, pair of material indices and a list of indices. These indices are 
	//used to index into the PxContact array that's passed by the user
	struct PxsReducedContactPatch
	{
		PxU32 numContactPoints;
		PxU32 contactPoints[CONTACT_REDUCTION_MAX_CONTACTS];	
	};

	struct PxsContactPatch
	{	
		PxVec3 rootNormal;
		PxsContactPatch* mNextPatch;
		PxReal maxPenetration;
		PxU16 startIndex;
		PxU16 stride;
		PxU16 rootIndex;
		PxU16 index;
	};

	struct SortBoundsPredicateManifold
	{
		bool operator()(const PxsContactPatch* idx1, const PxsContactPatch* idx2) const
		{
			return idx1->maxPenetration < idx2->maxPenetration;
		}
	};



	template <PxU32 MaxPatches>
	class PxsContactReduction
	{
	public:
		PxsReducedContactPatch mPatches[MaxPatches];
		PxU32 mNumPatches;
		PxsContactPatch mIntermediatePatches[CONTACT_REDUCTION_MAX_PATCHES];
		PxsContactPatch* mIntermediatePatchesPtrs[CONTACT_REDUCTION_MAX_PATCHES];
		PxU32 mNumIntermediatePatches;
		Gu::ContactPoint* PX_RESTRICT mOriginalContacts;
		PxsMaterialInfo* PX_RESTRICT mMaterialInfo;
		PxU32 mNumOriginalContacts;

		PxsContactReduction(Gu::ContactPoint* PX_RESTRICT originalContacts, PxsMaterialInfo* PX_RESTRICT materialInfo, PxU32 numContacts) : 
		mNumPatches(0), mNumIntermediatePatches(0),	mOriginalContacts(originalContacts), mMaterialInfo(materialInfo), mNumOriginalContacts(numContacts)
		{
		}

		void reduceContacts()
		{
			//First pass, break up into contact patches, storing the start and stride of the patches
			//We will need to have contact patches and then coallesce them
			mIntermediatePatches[0].rootNormal = mOriginalContacts[0].normal;
			mIntermediatePatches[0].mNextPatch = NULL;
			mIntermediatePatches[0].startIndex = 0;
			mIntermediatePatches[0].rootIndex = 0;
			mIntermediatePatches[0].maxPenetration = mOriginalContacts[0].separation;
			mIntermediatePatches[0].index = 0;
			PxU16 numPatches = 1;
			//PxU32 startIndex = 0;
			PxU32 numUniquePatches = 1;
			PxU16 m = 1;
			for(; m < mNumOriginalContacts; ++m)
			{
				PxI32 index = -1;
				for(PxU32 b = numPatches; b > 0; --b)
				{
					PxsContactPatch& patch = mIntermediatePatches[b-1];
					if(mMaterialInfo[patch.startIndex].mMaterialIndex0 == mMaterialInfo[m].mMaterialIndex0 && mMaterialInfo[patch.startIndex].mMaterialIndex1 == mMaterialInfo[m].mMaterialIndex1 && 
						patch.rootNormal.dot(mOriginalContacts[m].normal) >= PXS_NORMAL_TOLERANCE)
					{
						index = PxI32(b-1);
						break;
					}
				}

				if(index != numPatches - 1)
				{
					mIntermediatePatches[numPatches-1].stride = PxU16(m - mIntermediatePatches[numPatches - 1].startIndex);
					//Create a new patch...
					if(numPatches == CONTACT_REDUCTION_MAX_PATCHES)
					{
						break;
					}
					mIntermediatePatches[numPatches].startIndex = m;
					mIntermediatePatches[numPatches].mNextPatch = NULL;
					if(index == -1)
					{
						mIntermediatePatches[numPatches].rootIndex = numPatches;
						mIntermediatePatches[numPatches].rootNormal = mOriginalContacts[m].normal;
						mIntermediatePatches[numPatches].maxPenetration = mOriginalContacts[m].separation;
						mIntermediatePatches[numPatches].index = numPatches;
						++numUniquePatches;
					}
					else
					{
						//Find last element in the link
						PxU16 rootIndex = mIntermediatePatches[index].rootIndex;
						mIntermediatePatches[index].mNextPatch = &mIntermediatePatches[numPatches];
						mIntermediatePatches[numPatches].rootNormal = mIntermediatePatches[index].rootNormal;
						mIntermediatePatches[rootIndex].maxPenetration = mIntermediatePatches[numPatches].maxPenetration = PxMin(mIntermediatePatches[rootIndex].maxPenetration, mOriginalContacts[m].separation);
						mIntermediatePatches[numPatches].rootIndex = rootIndex;
						mIntermediatePatches[numPatches].index = numPatches;
					}
					++numPatches;
				}
			}
			mIntermediatePatches[numPatches-1].stride = PxU16(m - mIntermediatePatches[numPatches-1].startIndex);

			//OK, we have a list of contact patches so that we can start contact reduction per-patch

			//OK, now we can go and reduce the contacts on a per-patch basis...

			for(PxU32 a = 0; a < numPatches; ++a)
			{
				mIntermediatePatchesPtrs[a] = &mIntermediatePatches[a];
			}


			SortBoundsPredicateManifold predicate;
			Ps::sort(mIntermediatePatchesPtrs, numPatches, predicate);

			PxU32 numReducedPatches = 0;
			for(PxU32 a = 0; a < numPatches; ++a)
			{
				if(mIntermediatePatchesPtrs[a]->rootIndex == mIntermediatePatchesPtrs[a]->index)
				{
					//Reduce this patch...
					if(numReducedPatches == MaxPatches)
						break;

					PxsReducedContactPatch& reducedPatch = mPatches[numReducedPatches++];
					//OK, now we need to work out if we have to reduce patches...
					PxU32 contactCount = 0;
					{
						PxsContactPatch* tmpPatch = mIntermediatePatchesPtrs[a];

						while(tmpPatch)
						{
							contactCount += tmpPatch->stride;
							tmpPatch = tmpPatch->mNextPatch;
						}
					}

					if(contactCount <= CONTACT_REDUCTION_MAX_CONTACTS)
					{
						//Just add the contacts...
						PxsContactPatch* tmpPatch = mIntermediatePatchesPtrs[a];

						PxU32 ind = 0;
						while(tmpPatch)
						{
							for(PxU32 b = 0; b < tmpPatch->stride; ++b)
							{
								reducedPatch.contactPoints[ind++] = tmpPatch->startIndex + b;
							}
							tmpPatch = tmpPatch->mNextPatch;
						}
						reducedPatch.numContactPoints = contactCount;
					}
					else
					{
						//Iterate through and find the most extreme point
						

						PxU32 ind = 0;

						{
							PxReal dist = 0.f;
							PxsContactPatch* tmpPatch = mIntermediatePatchesPtrs[a];
							while(tmpPatch)
							{
								for(PxU32 b = 0; b < tmpPatch->stride; ++b)
								{
									PxReal magSq = mOriginalContacts[tmpPatch->startIndex + b].point.magnitudeSquared();
									if(dist < magSq)
									{
										ind = tmpPatch->startIndex + b;
										dist = magSq;
									}
								}
								tmpPatch = tmpPatch->mNextPatch;
							}
						}	
						reducedPatch.contactPoints[0] = ind;
						const PxVec3 p0 = mOriginalContacts[ind].point;

						//Now find the point farthest from this point...						
						{
							PxReal maxDist = 0.f;
							PxsContactPatch* tmpPatch = mIntermediatePatchesPtrs[a];
							while(tmpPatch)
							{
								for(PxU32 b = 0; b < tmpPatch->stride; ++b)
								{
									PxReal magSq = (p0 - mOriginalContacts[tmpPatch->startIndex + b].point).magnitudeSquared();
									if(magSq > maxDist)
									{
										ind = tmpPatch->startIndex + b;
										maxDist = magSq;
									}
								}
								tmpPatch = tmpPatch->mNextPatch;
							}
						}
						reducedPatch.contactPoints[1] = ind;
						const PxVec3 p1 = mOriginalContacts[ind].point;

						//Now find the point farthest from the segment

						PxVec3 n = (p0 - p1).cross(mIntermediatePatchesPtrs[a]->rootNormal);

						//PxReal tVal = 0.f;
						{
							PxReal maxDist = 0.f;
							//PxReal tmpTVal;
							
							PxsContactPatch* tmpPatch = mIntermediatePatchesPtrs[a];
							while(tmpPatch)
							{
								for(PxU32 b = 0; b < tmpPatch->stride; ++b)
								{
									
									//PxReal magSq = tmpDistancePointSegmentSquared(p0, p1, mOriginalContacts[tmpPatch->startIndex + b].point, tmpTVal);
									PxReal magSq = (mOriginalContacts[tmpPatch->startIndex + b].point - p0).dot(n);
									if(magSq > maxDist)
									{
										ind = tmpPatch->startIndex + b;
										//tVal = tmpTVal;
										maxDist = magSq;
									}
								}
								tmpPatch = tmpPatch->mNextPatch;
							}
						}
						reducedPatch.contactPoints[2] = ind;

						//const PxVec3 closest = (p0 + (p1 - p0) * tVal);

						const PxVec3 dir = -n;//closest - p3;

						{
							PxReal maxDist = 0.f;
							//PxReal tVal = 0.f;
							PxsContactPatch* tmpPatch = mIntermediatePatchesPtrs[a];
							while(tmpPatch)
							{
								for(PxU32 b = 0; b < tmpPatch->stride; ++b)
								{
									PxReal magSq =  (mOriginalContacts[tmpPatch->startIndex + b].point - p0).dot(dir);
									if(magSq > maxDist)
									{
										ind = tmpPatch->startIndex + b;
										maxDist = magSq;
									}
								}
								tmpPatch = tmpPatch->mNextPatch;
							}
						}
						reducedPatch.contactPoints[3] = ind;

						//Now, we iterate through all the points, and cluster the points. From this, we establish the deepest point that's within a 
						//tolerance of this point and keep that point

						PxReal separation[CONTACT_REDUCTION_MAX_CONTACTS];
						PxU32 deepestInd[CONTACT_REDUCTION_MAX_CONTACTS];
						for(PxU32 i = 0; i < 4; ++i)
						{
							PxU32 index = reducedPatch.contactPoints[i];
							separation[i] = mOriginalContacts[index].separation - PXS_SEPARATION_TOLERANCE;
							deepestInd[i] = index;
						}

						PxsContactPatch* tmpPatch = mIntermediatePatchesPtrs[a];
						while(tmpPatch)
						{
							for(PxU32 b = 0; b < tmpPatch->stride; ++b)
							{
								Gu::ContactPoint& point = mOriginalContacts[tmpPatch->startIndex + b];
								
								PxReal distance = PX_MAX_REAL;
								PxU32 index = 0;
								for(PxU32 c = 0; c < 4; ++c)
								{
									PxVec3 dif = mOriginalContacts[reducedPatch.contactPoints[c]].point - point.point;
									PxReal d = dif.magnitudeSquared();
									if(distance > d)
									{
										distance = d;
										index = c;
									}
								}
								if(separation[index] > point.separation)
								{
									deepestInd[index] = tmpPatch->startIndex+b;
									separation[index] = point.separation;
								}

							}
							tmpPatch = tmpPatch->mNextPatch;
						}

						bool chosen[64];
						PxMemZero(chosen, sizeof(chosen));
						for(PxU32 i = 0; i < 4; ++i)
						{
							reducedPatch.contactPoints[i] = deepestInd[i];
							chosen[deepestInd[i]] = true;
						}						
						
						for(PxU32 i = 4; i < CONTACT_REDUCTION_MAX_CONTACTS; ++i)
						{
							separation[i] = PX_MAX_REAL;
							deepestInd[i] = 0;
						}
						tmpPatch = mIntermediatePatchesPtrs[a];
						while(tmpPatch)
						{
							for(PxU32 b = 0; b < tmpPatch->stride; ++b)
							{
								if(!chosen[tmpPatch->startIndex+b])
								{
									Gu::ContactPoint& point = mOriginalContacts[tmpPatch->startIndex + b];	
									for(PxU32 j = 4; j < CONTACT_REDUCTION_MAX_CONTACTS; ++j)
									{
										if(point.separation < separation[j])
										{
											for(PxU32 k = CONTACT_REDUCTION_MAX_CONTACTS-1; k > j; --k)
											{
												separation[k] = separation[k-1];
												deepestInd[k] = deepestInd[k-1];
											}
											separation[j] = point.separation;
											deepestInd[j] = tmpPatch->startIndex+b;
											break;
										}
									}
								}
							}
							tmpPatch = tmpPatch->mNextPatch;
						}

						for(PxU32 i = 4; i < CONTACT_REDUCTION_MAX_CONTACTS; ++i)
						{
							reducedPatch.contactPoints[i] = deepestInd[i];
						}

						reducedPatch.numContactPoints = CONTACT_REDUCTION_MAX_CONTACTS;
					}
				}
			}
			mNumPatches = numReducedPatches;
		}

	};
}


#endif
