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
      

#ifndef PX_PHYSICS_SCP_ITERATOR
#define PX_PHYSICS_SCP_ITERATOR

#include "PxVec3.h"
#include "PxContact.h"
#include "CmRange.h"

namespace physx
{

class PxShape;

namespace Sc
{
	class ShapeSim;
	class BodyCore;
	class Interaction;
	class Actor;

	class BodyIterator
	{
		public:
			BodyIterator() {}
			explicit BodyIterator(const Cm::Range<Actor*const>& range) : mRange(range) {}
			BodyCore*		getNext();
		private:
			Cm::Range<Actor* const> mRange;
	};

	class ContactIterator
	{
		public:
			struct Contact
			{
				Contact() 
					: normal(0.0f)
					, point(0.0f)
					, separation(0.0f)
					, normalForce(0.0f)
				{}

				PxVec3 normal;
				PxVec3 point;
				PxShape* shape0;
				PxShape* shape1;
				PxReal separation;
				PxReal normalForce;
				PxU32 faceIndex0;  // these are the external indices
				PxU32 faceIndex1;
				bool normalForceAvailable;
			};

			class Pair
			{
			public:
				Pair() : mIter(NULL, 0) {}
				Pair(const void*& contactData, const PxU32 contactDataSize, const PxReal*& forces, PxU32 numContacts, ShapeSim& shape0, ShapeSim& shape1);
				Contact* getNextContact();

			private:
				PxU32						mIndex;
				PxU32						mNumContacts;
				PxContactStreamIterator		mIter;
				const PxReal*				mForces;
				Contact						mCurrentContact;
				ShapeSim*					mShape0;
				ShapeSim*					mShape1;
			};

			ContactIterator() {}
			explicit ContactIterator(const Cm::Range<Interaction*const>& range): mRange(range), mOffset(0) {}
			Pair* getNextPair();

		private:
			Cm::Range<Interaction* const>	mRange;
			Pair							mCurrentPair;
			PxU32							mOffset;
	};

}  // namespace Sc

}

#endif
