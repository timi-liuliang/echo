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


#ifndef PX_PHYSICS_SCB_RIGID_STATIC
#define PX_PHYSICS_SCB_RIGID_STATIC

#include "ScStaticCore.h"
#include "ScbScene.h"
#include "ScbActor.h"
#include "ScbRigidObject.h"

namespace physx
{

namespace Scb
{

#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4324 ) // Padding was added at the end of a structure because of a __declspec(align) value.
#endif

struct RigidStaticBuffer : public RigidObjectBuffer
{
	template <PxU32 I, PxU32 Dummy> struct Fns {};		// TODO: make the base class traits visible
	typedef Sc::StaticCore Core;
	typedef RigidStaticBuffer Buf;

	// regular attributes
	enum { BF_Base = RigidObjectBuffer::AttrCount };
	SCB_REGULAR_ATTRIBUTE_ALIGNED(BF_Base,			PxTransform,		Actor2World, 16)
};

#if defined(PX_VC) 
     #pragma warning(pop) 
#endif


class RigidStatic : public Scb::RigidObject
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	typedef RigidStaticBuffer Buf;
	typedef Sc::StaticCore Core;

public:
// PX_SERIALIZATION
										RigidStatic(const PxEMPTY&) :	Scb::RigidObject(PxEmpty), mStatic(PxEmpty)	{}
	static		void					getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
	PX_INLINE							RigidStatic(const PxTransform& actor2World);
	PX_INLINE							~RigidStatic() {}

	PX_INLINE		const PxTransform&	getActor2World() const					{ return read<Buf::BF_Actor2World>(); }
	PX_INLINE		void				setActor2World(const PxTransform& m)	{ write<Buf::BF_Actor2World>(m); }

	PX_FORCE_INLINE void				onOriginShift(const PxVec3& shift)		{ mStatic.onOriginShift(shift); }

	//---------------------------------------------------------------------------------
	// Data synchronization
	//---------------------------------------------------------------------------------
	PX_INLINE void						syncState();

	static size_t getScOffset()													
	{ 
		return reinterpret_cast<size_t>(&reinterpret_cast<RigidStatic*>(0)->mStatic);
	}

	PX_FORCE_INLINE Sc::StaticCore&			getScStatic()				{	return mStatic; }

private:
			Sc::StaticCore					mStatic;

	PX_FORCE_INLINE	const Buf*		getRigidActorBuffer()	const	{ return (const Buf*)getStream();	}
	PX_FORCE_INLINE	Buf*			getRigidActorBuffer()			{ return (Buf*)getStream();			}

	//---------------------------------------------------------------------------------
	// Infrastructure for regular attributes
	//---------------------------------------------------------------------------------

	struct Access: public BufferedAccess<Buf, Core, RigidStatic> {};

	template<PxU32 f> PX_FORCE_INLINE typename Buf::Fns<f,0>::Arg read() const		{	return Access::read<Buf::Fns<f,0> >(*this, mStatic);	}
	template<PxU32 f> PX_FORCE_INLINE void write(typename Buf::Fns<f,0>::Arg v)		{	Access::write<Buf::Fns<f,0> >(*this, mStatic, v);		}
	template<PxU32 f> PX_FORCE_INLINE void flush(const Buf& buf)					{	Access::flush<Buf::Fns<f,0> >(*this, mStatic, buf);	}

};

RigidStatic::RigidStatic(const PxTransform& actor2World) : 
	mStatic(actor2World)
{
	setScbType(ScbType::RIGID_STATIC);
}


//--------------------------------------------------------------
//
// Data synchronization
//
//--------------------------------------------------------------

PX_INLINE void RigidStatic::syncState()
{
	PxU32 bufferFlags = getBufferFlags();

	if (bufferFlags & Buf::BF_ActorFlags)
		syncNoSimSwitch(*getRigidActorBuffer(), mStatic, false);

	RigidObject::syncState();

	if (bufferFlags & Buf::BF_Actor2World)
		flush<Buf::BF_Actor2World>(*getRigidActorBuffer());

	postSyncState();
}

}  // namespace Scb

}

#endif
