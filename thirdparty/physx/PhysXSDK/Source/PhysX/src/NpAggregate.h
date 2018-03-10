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


#ifndef PX_PHYSICS_NP_AGGREGATE
#define PX_PHYSICS_NP_AGGREGATE

#include "CmPhysXCommon.h"
#include "PxAggregate.h"
#include "ScbAggregate.h"
#include "PsUserAllocated.h"

namespace physx
{

class NpScene;

class NpAggregate : public PxAggregate, public Ps::UserAllocated
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
// PX_SERIALIZATION
										NpAggregate(PxBaseFlags baseFlags) : PxAggregate(baseFlags), mAggregate(PxEmpty) {}
	    virtual	        void	     	exportExtraData(PxSerializationContext& stream);
						void			importExtraData(PxDeserializationContext& context);
						void			resolveReferences(PxDeserializationContext& context);
	    virtual	        void			requires(PxProcessPxBaseCallback& c);
		static			NpAggregate*	createObject(PxU8*& address, PxDeserializationContext& context);
		static			void			getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
										NpAggregate(PxU32 maxActors, bool selfCollision);
		virtual							~NpAggregate();

		virtual			void			release();
		virtual			bool			addActor(PxActor&);
		virtual			bool			removeActor(PxActor&);
		virtual			bool			addArticulation(PxArticulation&);
		virtual			bool			removeArticulation(PxArticulation&);

		virtual			PxU32			getNbActors() const;
		virtual			PxU32			getMaxNbActors() const;
		virtual			PxU32			getActors(PxActor** userBuffer, PxU32 bufferSize, PxU32 startIndex) const;

		virtual			PxScene*		getScene();
		virtual			bool			getSelfCollision()	const;

		PX_FORCE_INLINE	PxU32			getCurrentSizeFast()	const	{ return mNbActors; }
		PX_FORCE_INLINE	PxActor*		getActorFast(PxU32 i)	const	{ return mActors[i]; }
		PX_FORCE_INLINE	bool			getSelfCollideFast()	const	{ return mAggregate.getSelfCollide(); }

						NpScene*		getAPIScene() const;
						NpScene*		getOwnerScene() const; // the scene the user thinks the actor is in, or from which the actor is pending removal

						void			addActorInternal(PxActor& actor, NpScene& s);
						void			removeAndReinsert(PxActor& actor, bool reinsert);
						bool			removeActorAndReinsert(PxActor& actor, bool reinsert);
						bool			removeArticulationAndReinsert(PxArticulation& art, bool reinsert);

		PX_FORCE_INLINE	Scb::Aggregate&	getScbAggregate() { return mAggregate; }

private:
						Scb::Aggregate	mAggregate;
						PxU32			mNbActors;
						PxActor**		mActors;
};

}

#endif
