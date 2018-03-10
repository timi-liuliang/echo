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


#ifndef PX_PHYSICS_SCP_CORE_INTERACTION
#define PX_PHYSICS_SCP_CORE_INTERACTION

#include "framework/ScInteraction.h"

namespace physx
{
namespace Sc
{

	class NPhaseCore;
	class ElementSimInteraction;
	class ActorInteraction;

	class CoreInteraction
	{
	public:
		enum DirtyFlag
		{
			CIF_DIRTY_FILTER_STATE		= (1 << 0), // All changes filtering related
			CIF_DIRTY_NOTIFICATION		= CIF_DIRTY_FILTER_STATE,
			CIF_DIRTY_MATERIAL			= (1 << 1),
			CIF_DIRTY_BODY_KINEMATIC	= (1 << 2) | CIF_DIRTY_FILTER_STATE,
			CIF_DIRTY_DOMINANCE			= (1 << 3),
			CIF_DIRTY_REST_OFFSET		= (1 << 4),
			CIF_DIRTY_VISUALIZATION		= (1 << 5),
			CIF_DIRTY_LAST				= CIF_DIRTY_VISUALIZATION,
			CIF_DIRTY_ALL				= 0xffff
		};

		enum Flag
		{
			IS_ELEMENT_INTERACTION				= (1 << 0),  // This interaction is a ElementSimInteraction and not a ActorInteraction
			IN_DIRTY_LIST						= (1 << 1),  // The interaction is in the dirty list
			IS_FILTER_PAIR						= (1 << 2)  // The interaction is tracked by the filter callback mechanism
		};

		PX_INLINE					CoreInteraction(bool bElementInteraction);
		virtual						~CoreInteraction() {}

		PX_INLINE	virtual void	updateState(bool removeFromDirtyList);
		PX_INLINE	void			setDirty(PxU32 dirtyFlags);
		PX_INLINE	bool			isDirty();
		PX_INLINE	void			setClean(bool removeFromList);
		PX_INLINE	bool			isElementInteraction()	const;
		PX_INLINE	bool			isActorInteraction()	const;
		virtual		NPhaseCore*		getNPhaseCore()			const = 0;

		PX_INLINE	bool			readCoreFlag(Flag flag)	const	{ return (mFlags & flag) == flag;						}
		PX_INLINE	void			raiseCoreFlag(Flag flag)		{ mFlags |= flag;										}
		PX_INLINE	void			clearCoreFlag(Flag flag)		{ mFlags &= ~flag;										}
		PX_INLINE	Ps::IntBool		needsRefiltering()		const	{ return (getDirtyFlags() & CIF_DIRTY_FILTER_STATE);	}

		static CoreInteraction*		isCoreInteraction(Interaction* interaction);
	protected:
		PX_INLINE	PxU16			getDirtyFlags()			const	{ return mDirtyFlags;									}
		PX_INLINE	static bool		readDirtyFlag(PxU16 dirtyFlags, DirtyFlag flag)	{ return (dirtyFlags & flag) == flag;	}

	private:
					void			addToDirtyList();
					void			removeFromDirtyList();

					PxU16			mDirtyFlags;
					PxU16			mFlags;
	};

} // namespace Sc

PX_INLINE Sc::CoreInteraction::CoreInteraction(bool bElementInteraction)
	: mDirtyFlags(CIF_DIRTY_ALL), mFlags(0)
{
	if (bElementInteraction)
		raiseCoreFlag(IS_ELEMENT_INTERACTION);
}


PX_INLINE void Sc::CoreInteraction::updateState(bool removeFromDirtyList)
{
	setClean(removeFromDirtyList);
	mDirtyFlags = 0;
}


PX_INLINE void Sc::CoreInteraction::setDirty(PxU32 dirtyFlags)
{
	mDirtyFlags |= (PxU16)dirtyFlags;
	if (!readCoreFlag(IN_DIRTY_LIST))
	{
		addToDirtyList();
		raiseCoreFlag(IN_DIRTY_LIST);
	}
}


PX_INLINE bool Sc::CoreInteraction::isDirty()
{
	return (mDirtyFlags != 0);
}


PX_INLINE void Sc::CoreInteraction::setClean(bool removeFromList)
{
	if (readCoreFlag(IN_DIRTY_LIST))
	{
		if (removeFromList)  // if we process all dirty interactions anyway, then we can just clear the list at the end and save the work here.
			removeFromDirtyList();
		clearCoreFlag(IN_DIRTY_LIST);
	}
}


PX_INLINE bool Sc::CoreInteraction::isElementInteraction() const
{
	return readCoreFlag(IS_ELEMENT_INTERACTION);
}


PX_INLINE bool Sc::CoreInteraction::isActorInteraction() const
{
	return !readCoreFlag(IS_ELEMENT_INTERACTION);
}

}

#endif
