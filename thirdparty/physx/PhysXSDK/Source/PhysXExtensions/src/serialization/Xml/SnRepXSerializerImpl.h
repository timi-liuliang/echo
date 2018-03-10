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
#ifndef PX_REPX_SERIALIZER_IMPL_H
#define PX_REPX_SERIALIZER_IMPL_H

#include "PsUserAllocated.h"
#include "SnXmlVisitorWriter.h"
#include "SnXmlVisitorReader.h"

namespace physx { 
	using namespace Sn;

	/**
	 *	The repx serializer impl takes the raw, untyped repx extension interface
	 *	and implements the simpler functions plus does the reinterpret-casts required 
	 *	for any object to implement the serializer safely.
	 */
	template<typename TLiveType>
	struct RepXSerializerImpl : public PxRepXSerializer, UserAllocated
	{
	protected:
		RepXSerializerImpl( const RepXSerializerImpl& inOther );
		RepXSerializerImpl& operator=( const RepXSerializerImpl& inOther );

	public:
		PxAllocatorCallback& mAllocator;

		RepXSerializerImpl( PxAllocatorCallback& inAllocator )
			: mAllocator( inAllocator )
		{
		}
				
		virtual const char* getTypeName() { return PxTypeInfo<TLiveType>::name(); }
		
		virtual void objectToFile( const PxRepXObject& inLiveObject, PxCollection* inCollection, XmlWriter& inWriter, MemoryBuffer& inTempBuffer, PxRepXInstantiationArgs& inArgs )
		{
			const TLiveType* theObj = reinterpret_cast<const TLiveType*>( inLiveObject.serializable );
			objectToFileImpl( theObj, inCollection, inWriter, inTempBuffer, inArgs );
		}

		virtual PxRepXObject fileToObject( XmlReader& inReader, XmlMemoryAllocator& inAllocator, PxRepXInstantiationArgs& inArgs, PxCollection* inCollection )
		{
			TLiveType* theObj( allocateObject( inArgs ) );
			if ( theObj )
				if(fileToObjectImpl( theObj, inReader, inAllocator, inArgs, inCollection ))
				 	return createRepXObject( theObj );	
			return PxRepXObject();
		}
		
		virtual void objectToFileImpl( const TLiveType* inObj, PxCollection* inCollection, XmlWriter& inWriter, MemoryBuffer& inTempBuffer, PxRepXInstantiationArgs& /*inArgs*/)
		{
			writeAllProperties( inObj, inWriter, inTempBuffer, *inCollection );
		}

		virtual bool fileToObjectImpl( TLiveType* inObj, XmlReader& inReader, XmlMemoryAllocator& inAllocator, PxRepXInstantiationArgs& inArgs, PxCollection* inCollection )
		{
			return readAllProperties( inArgs, inReader, inObj, inAllocator, *inCollection );
		}

		virtual TLiveType* allocateObject( PxRepXInstantiationArgs& inArgs ) = 0;
	};
}

#endif
