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


#ifndef PX_PHYSX_PROFILE_FOUNDATION_WRAPPER_H
#define PX_PHYSX_PROFILE_FOUNDATION_WRAPPER_H
#include "PxProfileBase.h"
#include "PxFoundation.h"
#include "PxBroadcastingAllocator.h"
#include "PxAllocatorCallback.h"
#include "PxErrorCallback.h"
#include "PsArray.h"
#include "PsHashMap.h"

namespace physx { namespace profile {

	struct NullErrorCallback : public PxErrorCallback
	{
		void reportError(PxErrorCode::Enum, const char* , const char* , int )
		{

//			fprintf( stderr, "%s:%d: %s\n", file, line, message );
		}
	};

	struct FoundationWrapper
	{
		PxAllocatorCallback*			mUserAllocator;

		FoundationWrapper( PxAllocatorCallback& inUserAllocator )
			: mUserAllocator( &inUserAllocator )
		{
		}

		FoundationWrapper( PxAllocatorCallback* inUserAllocator )
			: mUserAllocator( inUserAllocator )
		{
		}
		
		FoundationWrapper( PxFoundation* inUserFoundation, bool useBaseAllocator = false )
			: mUserAllocator( NULL )
		{
			if ( inUserFoundation != NULL )
			{
				if ( useBaseAllocator )
					mUserAllocator = &inUserFoundation->getAllocatorCallback();
				else
					mUserAllocator = &inUserFoundation->getAllocator();
			}
		}

		PxAllocatorCallback&		getAllocator() const
		{
			PX_ASSERT( NULL != mUserAllocator );
			return *mUserAllocator;
		}
	};

	template <typename T>
	class WrapperReflectionAllocator
	{
		static const char* getName()
		{
#if defined PX_GNUC || defined PX_GHS
			return __PRETTY_FUNCTION__;
#else
			return typeid(T).name();
#endif
		}
		FoundationWrapper* mWrapper;

	public:
		WrapperReflectionAllocator(FoundationWrapper& inWrapper) : mWrapper( &inWrapper )	{}
		WrapperReflectionAllocator( const WrapperReflectionAllocator& inOther )
			: mWrapper( inOther.mWrapper )
		{
		}
		WrapperReflectionAllocator& operator=( const WrapperReflectionAllocator& inOther )
		{
			mWrapper = inOther.mWrapper;
			return *this;
		}
		PxAllocatorCallback& getAllocator() { return mWrapper->getAllocator(); }
		void* allocate(size_t size, const char* filename, int line)
		{
#if defined(PX_CHECKED) // checked and debug builds
			if(!size)
				return 0;
			return getAllocator().allocate(size, getName(), filename, line);
#else
			return getAllocator().allocate(size, "<no allocation names in this config>", filename, line);
#endif
		}
		void deallocate(void* ptr)
		{
			if(ptr)
				getAllocator().deallocate(ptr);
		}
	};
	
	struct WrapperNamedAllocator
	{
		FoundationWrapper*	mWrapper;
		const char*			mAllocationName;
		WrapperNamedAllocator(FoundationWrapper& inWrapper, const char* inAllocationName) 
			: mWrapper( &inWrapper )
			, mAllocationName( inAllocationName ) 
		{}
		WrapperNamedAllocator( const WrapperNamedAllocator& inOther )
			: mWrapper( inOther.mWrapper )
			, mAllocationName( inOther.mAllocationName )
		{
		}
		WrapperNamedAllocator& operator=( const WrapperNamedAllocator& inOther )
		{
			mWrapper = inOther.mWrapper;
			mAllocationName = inOther.mAllocationName;
			return *this;
		}
		PxAllocatorCallback& getAllocator() { return mWrapper->getAllocator(); }
		void* allocate(size_t size, const char* filename, int line)
		{
			if(!size)
				return 0;
			return getAllocator().allocate(size, mAllocationName, filename, line);
		}
		void deallocate(void* ptr)
		{
			if(ptr)
				getAllocator().deallocate(ptr);
		}
	};

	template<class T>
	struct ProfileArray : public shdfnd::Array<T, WrapperReflectionAllocator<T> >
	{
		typedef WrapperReflectionAllocator<T> TAllocatorType;

		ProfileArray( FoundationWrapper& inWrapper )
			: shdfnd::Array<T, TAllocatorType >( TAllocatorType( inWrapper ) )
		{
		}
		
		ProfileArray( const ProfileArray< T >& inOther )
			: shdfnd::Array<T, TAllocatorType >( inOther, inOther )
		{
		}
	};

	template<typename TKeyType, typename TValueType, typename THashType=shdfnd::Hash<TKeyType> >
	struct ProfileHashMap : public shdfnd::HashMap<TKeyType, TValueType, THashType, WrapperReflectionAllocator< TValueType > >
	{
		typedef shdfnd::HashMap<TKeyType, TValueType, THashType, WrapperReflectionAllocator< TValueType > > THashMapType;
		typedef WrapperReflectionAllocator<TValueType> TAllocatorType;
		ProfileHashMap( FoundationWrapper& inWrapper )
			: THashMapType( TAllocatorType( inWrapper ) )
		{
		}
	};

	template<typename TDataType>
	inline TDataType* PxProfileAllocate( PxAllocatorCallback* inAllocator, const char* file, int inLine )
	{
		FoundationWrapper wrapper( inAllocator );
		typedef WrapperReflectionAllocator< TDataType > TAllocator;
		TAllocator theAllocator( wrapper );
		return reinterpret_cast<TDataType*>( theAllocator.allocate( sizeof( TDataType ), file, inLine ) );
	}

	template<typename TDataType>
	inline TDataType* PxProfileAllocate( PxAllocatorCallback& inAllocator, const char* file, int inLine )
	{
		return PxProfileAllocate<TDataType>( &inAllocator, file, inLine );
	}

	template<typename TDataType>
	inline TDataType* PxProfileAllocate( PxFoundation* inFoundation, const char* file, int inLine )
	{
		FoundationWrapper wrapper( inFoundation );
		typedef WrapperReflectionAllocator< TDataType > TAllocator;
		TAllocator theAllocator( wrapper );
		return reinterpret_cast<TDataType*>( theAllocator.allocate( sizeof( TDataType ), file, inLine ) );
	}

	template<typename TDataType>
	inline void PxProfileDeleteAndDeallocate( FoundationWrapper& inAllocator, TDataType* inDType )
	{
		PX_ASSERT(inDType);
		PxAllocatorCallback& allocator( inAllocator.getAllocator() );
		inDType->~TDataType();
		allocator.deallocate( inDType );
	}

	template<typename TDataType>
	inline void PxProfileDeleteAndDeallocate( PxAllocatorCallback& inAllocator, TDataType* inDType )
	{
		FoundationWrapper wrapper( &inAllocator );	
		PxProfileDeleteAndDeallocate( wrapper, inDType );
	}
	
	template<typename TDataType>
	inline void PxProfileDeleteAndDeallocate( PxFoundation* inAllocator, TDataType* inDType )
	{
		FoundationWrapper wrapper( inAllocator );		
		PxProfileDeleteAndDeallocate( wrapper, inDType );
	}
} }

#define PX_PROFILE_NEW( allocator, dtype ) new (PxProfileAllocate<dtype>( allocator, __FILE__, __LINE__ )) dtype
#define PX_PROFILE_DELETE( allocator, obj ) PxProfileDeleteAndDeallocate( allocator, obj );

#endif
