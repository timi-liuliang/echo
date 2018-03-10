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

#ifndef GU_CONTAINER_H
#define GU_CONTAINER_H

#include "PxPhysXCommonConfig.h"  // needed for dll export
#include "PxMemory.h"
#include "PsUserAllocated.h"
#include "PxAssert.h"
#include "CmPhysXCommon.h"
#include "PsUtilities.h"

namespace physx
{
namespace Gu
{
	class PX_PHYSX_COMMON_API Container : public Ps::UserAllocated
	{
		public:
										Container();
										Container(const Container& object);
										Container(PxU32 size, float growth_factor);
										~Container();
		// Management
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Initializes the container so that it uses an external memory buffer. The container doesn't own the memory, resizing is disabled.
		 *	\param		max_entries		[in] max number of entries in the container
		 *	\param		entries			[in] external memory buffer
		 *	\param		allow_resize	[in] false to prevent resizing the array (forced fixed length), true to enable it. In that last case the
		 *									initial memory is NOT released, it's up to the caller. That way the initial memory can be on the stack.
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						void			InitSharedBuffers(PxU32 max_entries, PxU32* entries, bool allow_resize=false);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	A O(1) method to add a value in the container. The container is automatically resized if needed.
		 *	The method is inline, not the resize. The call overhead happens on resizes only, which is not a problem since the resizing operation
		 *	costs a lot more than the call overhead...
		 *
		 *	\param		entry		[in] a PxU32 to store in the container
		 *	\see		Add(float entry)
		 *	\see		Empty()
		 *	\see		Contains(PxU32 entry)
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		PX_FORCE_INLINE	Container&		Add(PxU32 entry)
										{
											// Resize if needed
											if(mCurNbEntries==mMaxNbEntries)
												Resize();

											// Add new entry
											mEntries[mCurNbEntries++]	= entry;
											return *this;
										}

		PX_FORCE_INLINE	Container&		Add(const PxU32* entries, PxU32 nb)
										{
											if(entries && nb)
											{
												// Resize if needed
												if(mCurNbEntries+nb>mMaxNbEntries)
													Resize(nb);

												// Add new entry
												PxMemCopy(&mEntries[mCurNbEntries], entries, nb*sizeof(PxU32));
												mCurNbEntries += nb;
											}
											return *this;
										}

		PX_FORCE_INLINE	Container&		Add(const Container& container)
										{
											return Add(container.GetEntries(), container.GetNbEntries());
										}

		PX_FORCE_INLINE	PxU32*			Reserve(PxU32 nb)
										{
											// Resize if needed
											if(mCurNbEntries+nb>mMaxNbEntries)
												Resize(nb);

											// We expect the user to fill reserved memory with 'nb' PxU32s
											PxU32* Reserved = &mEntries[mCurNbEntries];

											// PT: leap of faith...
											Ps::invalidateCache(Reserved, PxI32(nb*sizeof(PxU32)));

											// Meanwhile, we do as if it had been filled
											mCurNbEntries += nb;

											// This is mainly used to avoid the copy when possible
											return Reserved;
										}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	A O(1) method to add a value in the container. The container is automatically resized if needed.
		 *	The method is inline, not the resize. The call overhead happens on resizes only, which is not a problem since the resizing operation
		 *	costs a lot more than the call overhead...
		 *
		 *	\param		entry		[in] a float to store in the container
		 *	\see		Add(PxU32 entry)
		 *	\see		Empty()
		 *	\see		Contains(PxU32 entry)
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		PX_FORCE_INLINE	Container&		Add(float entry)
										{
											// Resize if needed
											if(mCurNbEntries==mMaxNbEntries)
												Resize();

											// Add new entry
											union { PxU32 u; PxF32 f; } uf;
											uf.f = entry;
											mEntries[mCurNbEntries++] = uf.u;
											return *this;
										}

		PX_FORCE_INLINE	Container&		Add(const float* entries, PxU32 nb)
										{
											// Resize if needed
											if(mCurNbEntries+nb>mMaxNbEntries)
												Resize(nb);

											// Add new entry
											PxMemCopy(&mEntries[mCurNbEntries], entries, nb*sizeof(float));
											mCurNbEntries+=nb;
											return *this;
										}

		//! Add unique [slow]
		PX_FORCE_INLINE	Container&		AddUnique(PxU32 entry)
										{
											if(!Contains(entry))
												Add(entry);
											return *this;
										}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Clears the container. All stored values are deleted, and it frees used ram.
		 *	\see		Reset()
		 *	\return		Self-Reference
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						Container&		Empty();

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Resets the container. Stored values are discarded but the buffer is kept so that further calls don't need resizing again.
		 *	That's a kind of temporal coherence.
		 *	\see		Empty()
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		PX_FORCE_INLINE	void			Reset()
										{
											// Avoid the write if possible
											// ### CMOV
											if(mCurNbEntries)
												mCurNbEntries = 0;
										}

		// HANDLE WITH CARE - I hope you know what you're doing
		PX_FORCE_INLINE	void			ForceSize(PxU32 size)
										{
											mCurNbEntries = size;
										}

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Sets the initial size of the container. If it already contains something, it's discarded.
		 *	\param		nb		[in] Number of entries
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool			SetSize(PxU32 nb);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		/**
		 *	Refits the container and get rid of unused bytes.
		 *	\return		true if success
		 */
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						bool			Refit();

		// Checks whether the container already contains a given value.
						bool			Contains(PxU32 entry, PxU32* location=NULL) const;
		// Deletes an entry - doesn't preserve insertion order.
						bool			Delete(PxU32 entry);
		// Deletes an entry - does preserve insertion order.
						bool			DeleteKeepingOrder(PxU32 entry);
		//! Deletes the very last entry.
		PX_FORCE_INLINE	void			DeleteLastEntry()						{ if(mCurNbEntries)	mCurNbEntries--;			}
		//! Deletes the entry whose index is given
		PX_FORCE_INLINE	void			DeleteIndex(PxU32 index)				{ mEntries[index] = mEntries[--mCurNbEntries];	}

		// Data access.
		PX_FORCE_INLINE	PxU32			GetNbEntries()					const	{ return mCurNbEntries;					}	//!< Returns the current number of entries.
		PX_FORCE_INLINE	PxU32			GetMaxNbEntries()				const	{ return mMaxNbEntries;					}	//!< Returns maximum number of entries before resizing.
		PX_FORCE_INLINE	PxU32			GetEntry(PxU32 i)				const	{ return mEntries[i];					}	//!< Returns ith entry.
		PX_FORCE_INLINE	PxU32*			GetEntries()					const	{ return mEntries;						}	//!< Returns the list of entries.

		PX_FORCE_INLINE	PxU32			GetFirst()						const	{ return mEntries[0];					}
		PX_FORCE_INLINE	PxU32			GetLast()						const	{ return mEntries[mCurNbEntries-1];		}

		// Growth control
		PX_FORCE_INLINE	float			GetGrowthFactor()				const	{ return mGrowthFactor;					}	//!< Returns the growth factor
		PX_FORCE_INLINE	void			SetGrowthFactor(float growth)														//!< Sets the growth factor
										{
											// Negative growths are reserved for internal usages
											if(growth<0.0f)	growth = 0.0f;
											mGrowthFactor = growth;
										}

		PX_FORCE_INLINE	bool			IsFull()						const	{ return mCurNbEntries==mMaxNbEntries;	}	//!< Checks the container is full
		PX_FORCE_INLINE	Ps::IntBool		IsNotEmpty()					const	{ return (Ps::IntBool)mCurNbEntries;					}	//!< Checks the container is empty

		//! Read-access as an array
		PX_FORCE_INLINE	PxU32			operator[](PxU32 i)				const	{ PX_ASSERT(i<mCurNbEntries); return mEntries[i];	}
		//! Write-access as an array
		PX_FORCE_INLINE	PxU32&			operator[](PxU32 i)						{ PX_ASSERT(i<mCurNbEntries); return mEntries[i];	}

		//! Operator for "Container A = Container B"
						void			operator = (const Container& object);
		private:
		// Resizing
						bool			Resize(PxU32 needed=1);
		// Data
						PxU32			mMaxNbEntries;		//!< Maximum possible number of entries
						PxU32			mCurNbEntries;		//!< Current number of entries
						PxU32*			mEntries;			//!< List of entries
						float			mGrowthFactor;		//!< Resize: new number of entries = old number * mGrowthFactor
	};

	#define LocalContainer(x, n)	Container	x;	x.InitSharedBuffers(n, (PxU32*)PxAlloca(sizeof(PxU32)*n), true);

}

}

#endif // GU_CONTAINER_H
