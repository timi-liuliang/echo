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

#ifndef PX_PHYSICS_SN_SERIALIZATION_REGISTRY
#define PX_PHYSICS_SN_SERIALIZATION_REGISTRY

#include "PxSerialization.h"
#include "PxRepXSerializer.h"
#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"

namespace physx
{

namespace Cm { class Collection; }

namespace Sn {
	
	class SerializationRegistry : public PxSerializationRegistry, public Ps::UserAllocated
	{
	public:
		SerializationRegistry(PxPhysics& physics);					
		virtual						~SerializationRegistry();

		virtual void				release(){ PX_DELETE(this);  }
		
		PxPhysics&			        getPhysics() const			{ return mPhysics; }
		
		//binary
		void						registerSerializer(PxType type, PxSerializer& serializer);
		PxSerializer*               unregisterSerializer(PxType type);
		void						registerBinaryMetaDataCallback(PxBinaryMetaDataCallback callback);	
		void						getBinaryMetaData(PxOutputStream& stream) const;
		const PxSerializer*			getSerializer(PxType type) const;
		const char*			        getSerializerName(PxU32 index) const;
		PxType                      getSerializerType(PxU32 index) const;
		PxU32                       getNbSerializers() const { return mSerializers.size(); } 
		//repx
		void						registerRepXSerializer(PxType type, PxRepXSerializer& serializer);
		PxRepXSerializer*			getRepXSerializer(const char* typeName) const;
		PxRepXSerializer*           unregisterRepXSerializer(PxType type);
	
	protected:
		SerializationRegistry &operator=(const SerializationRegistry &);
	private:
		typedef Ps::CoalescedHashMap<PxType, PxSerializer*>		SerializerMap;
		typedef Ps::HashMap<PxType, PxRepXSerializer*>	        RepXSerializerMap;

		PxPhysics&										mPhysics;
		SerializerMap									mSerializers;
		RepXSerializerMap								mRepXSerializers;
		Ps::Array<PxBinaryMetaDataCallback>				mMetaDataCallbacks;	
	};

	void  sortCollection(Cm::Collection& collection,  SerializationRegistry& sr, bool isRepx);
} // Sn

} // physx



#endif

