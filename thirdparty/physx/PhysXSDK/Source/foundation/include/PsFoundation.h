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


#ifndef PX_FOUNDATION_PSFOUNDATION_H
#define PX_FOUNDATION_PSFOUNDATION_H

#include "Ps.h"
#include "PsInlineArray.h"
#include "foundation/PxFoundation.h"
#include "foundation/PxErrors.h"
#include "PsMutex.h"
#include "foundation/PxBroadcastingAllocator.h"
#include "PsPAEventSrc.h"

#include <stdarg.h>

#include "PsHashMap.h"

#include "PsErrorHandler.h"

namespace physx
{
namespace shdfnd
{

#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4251 ) // class needs to have dll-interface to be used by clients of class
#endif

	union TempAllocatorChunk;

	class PxAllocatorListenerManager;

	class PX_FOUNDATION_API Foundation : public PxFoundation
	{
		PX_NOCOPY(Foundation)
	public:
#ifdef PX_CHECKED
		typedef MutexT<Allocator> Mutex;
#else
		typedef MutexT<> Mutex;
#endif

	private:
		typedef HashMap<const NamedAllocator*, const char*, 
			Hash<const NamedAllocator*>, NonTrackingAllocator> AllocNameMap;

		typedef Array<TempAllocatorChunk*, Allocator> AllocFreeTable;

										Foundation(PxErrorCallback& errc, PxAllocatorCallback& alloc);
										~Foundation();

	public:
		void							release();

		// factory
		static Foundation*				createInstance(PxU32 version, PxErrorCallback& errc, PxAllocatorCallback& alloc);
		// note, you MUST call destroyInstance iff createInstance returned true!
		static void						destroyInstance();

		static Foundation& 				getInstance();

		static void						incRefCount();  // this call requires a foundation object to exist already
		static void						decRefCount();  // this call requires a foundation object to exist already

		virtual PxErrorCallback&		getErrorCallback() const;
		virtual void					setErrorLevel(PxErrorCode::Enum mask);
		virtual PxErrorCode::Enum		getErrorLevel() const;

		virtual PxBroadcastingAllocator& getAllocator() const { return mAllocator; }
		virtual PxAllocatorCallback&	getAllocatorCallback() const;
		PxAllocatorCallback& 			getCheckedAllocator() { return mAllocator; }

		virtual bool					getReportAllocationNames() const { return mReportAllocationNames; }
		virtual void					setReportAllocationNames(bool value) { mReportAllocationNames = value; }

		//! error reporting function
		void 							error(PxErrorCode::Enum, const char* file, int line, const char* messageFmt, ...);
		void 							errorImpl(PxErrorCode::Enum, const char* file, int line, const char* messageFmt, va_list );
		static PxU32					getWarnOnceTimestamp(); 

		PX_INLINE	Mutex&				getErrorMutex()			{ return mErrorMutex;			}
		PX_INLINE	AllocNameMap&		getNamedAllocMap()		{ return mNamedAllocMap;		}
		PX_INLINE	Mutex&				getNamedAllocMutex()	{ return mNamedAllocMutex;		}

		PX_INLINE	AllocFreeTable&		getTempAllocFreeTable()	{ return mTempAllocFreeTable;	}
		PX_INLINE	Mutex&				getTempAllocMutex()		{ return mTempAllocMutex;		}

		PX_INLINE   PAUtils&            getPAUtils()			{ return mPAUtils;				}
		
		PX_INLINE	ErrorHandler&		getErrorHandler()		{ return mInteralErrorHandler; }		

	private:
		class AlignCheckAllocator: public PxBroadcastingAllocator
		{
			static const PxU32 MAX_LISTENER_COUNT = 5;
		public:
			AlignCheckAllocator(PxAllocatorCallback& originalAllocator)
					: mAllocator(originalAllocator)
					, mListenerCount( 0 ){}

			void					deallocate(void* ptr)		
			{ 
				//So here, for performance reasons I don't grab the mutex.
				//The listener array is very rarely changing; for most situations
				//only at startup.  So it is unlikely that using the mutex
				//will help a lot but it could have serious perf implications.
				PxU32 theCount = mListenerCount;
				for( PxU32 idx = 0; idx < theCount; ++idx )
					mListeners[idx]->onDeallocation( ptr );
				mAllocator.deallocate(ptr); 
			}
			void*					allocate(size_t size, const char* typeName, const char* filename, int line);
			PxAllocatorCallback&	getBaseAllocator() const	{ return mAllocator; }
			void registerAllocationListener( PxAllocationListener& inListener )
			{
				PX_ASSERT( mListenerCount < MAX_LISTENER_COUNT );
				if ( mListenerCount < MAX_LISTENER_COUNT )
				{
					mListeners[mListenerCount] = &inListener;
					++mListenerCount;
				}
			}
			void deregisterAllocationListener( PxAllocationListener& inListener )
			{
				for( PxU32 idx = 0; idx < mListenerCount; ++idx )
				{
					if ( mListeners[idx] == &inListener )
					{
						mListeners[idx] = mListeners[mListenerCount-1];
						--mListenerCount;
						break;
					}
				}
			}
		protected:
			AlignCheckAllocator& operator=(const AlignCheckAllocator&);

		private:
			PxAllocatorCallback& mAllocator;
			//I am not sure about using a PxArray here.
			//For now, this is fine.
			PxAllocationListener*	mListeners[MAX_LISTENER_COUNT];
			volatile PxU32			mListenerCount;
			
		};

		// init order is tricky here: the mutexes require the allocator, the allocator may require the error stream
		PxErrorCallback&				mErrorCallback;
		mutable AlignCheckAllocator		mAllocator;

		bool							mReportAllocationNames;

		PxErrorCode::Enum				mErrorMask;
		Mutex							mErrorMutex;

		AllocNameMap					mNamedAllocMap;
		Mutex							mNamedAllocMutex;

		AllocFreeTable					mTempAllocFreeTable;
		Mutex							mTempAllocMutex;

		PAUtils							mPAUtils;

		ErrorHandler					mInteralErrorHandler;

		static Foundation*				mInstance;
		static PxU32					mRefCount;
		static PxU32					mWarnOnceTimestap;
	};
#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

	PX_INLINE Foundation& getFoundation()
	{
		return Foundation::getInstance();
	}

} // namespace shdfnd
} // namespace physx

//shortcut macros:
//usage: Foundation::error(PX_WARN, "static friction %f is is lower than dynamic friction %d", sfr, dfr);
#define PX_WARN ::physx::PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__
#define PX_INFO	::physx::PxErrorCode::eDEBUG_INFO, __FILE__, __LINE__

#if defined(PX_DEBUG) || defined(PX_CHECKED)
#ifdef __SPU__ // SCS: used in CCD from SPU. how can we fix that correctly?
#define PX_WARN_ONCE(condition, string) ((void)0)
#else
#define PX_WARN_ONCE(condition, string)	\
if (condition) { \
	static PxU32 timestap = 0; \
	if (timestap != Ps::getFoundation().getWarnOnceTimestamp()) { \
		timestap = Ps::getFoundation().getWarnOnceTimestamp(); \
		Ps::getFoundation().error(PX_WARN, string); \
	} \
}
#endif
#else
#define PX_WARN_ONCE(condition, string) ((void)0)
#endif

#endif

