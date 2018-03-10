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
#include "PxMetaDataObjects.h"
#include "PxExtensionMetaDataObjects.h"
#include "ExtJointMetaDataExtensions.h" 
#include "SnRepXSerializerImpl.h"
#include "PxJointRepXSerializer.h"

namespace physx { 

	template<typename TJointType>
	inline TJointType* createJoint( PxPhysics& physics, 
									   PxRigidActor* actor0, const PxTransform& localFrame0, 
									   PxRigidActor* actor1, const PxTransform& localFrame1 )
	{
		return NULL;
	}

	template<>
	inline PxD6Joint* createJoint<PxD6Joint>(PxPhysics& physics, 
										PxRigidActor* actor0, const PxTransform& localFrame0, 
										PxRigidActor* actor1, const PxTransform& localFrame1)
	{
		return PxD6JointCreate( physics, actor0, localFrame0, actor1, localFrame1 );
	}

	template<>
	inline PxDistanceJoint*	createJoint<PxDistanceJoint>(PxPhysics& physics, 
									 		  PxRigidActor* actor0, const PxTransform& localFrame0, 
											  PxRigidActor* actor1, const PxTransform& localFrame1)
	{
		return PxDistanceJointCreate( physics, actor0, localFrame0, actor1, localFrame1 );
	}

	template<>
	inline PxFixedJoint* createJoint<PxFixedJoint>(PxPhysics& physics, 
										   PxRigidActor* actor0, const PxTransform& localFrame0, 
										   PxRigidActor* actor1, const PxTransform& localFrame1)
	{
		return PxFixedJointCreate( physics, actor0, localFrame0, actor1, localFrame1 );
	}

	template<>
	inline PxPrismaticJoint* createJoint<PxPrismaticJoint>(PxPhysics& physics, 
											   PxRigidActor* actor0, const PxTransform& localFrame0, 
											   PxRigidActor* actor1, const PxTransform& localFrame1)
	{
		return PxPrismaticJointCreate( physics, actor0, localFrame0, actor1, localFrame1 );
	}

	template<>
	inline PxRevoluteJoint*	createJoint<PxRevoluteJoint>(PxPhysics& physics, 
											  PxRigidActor* actor0, const PxTransform& localFrame0, 
											  PxRigidActor* actor1, const PxTransform& localFrame1)
	{
		return PxRevoluteJointCreate( physics, actor0, localFrame0, actor1, localFrame1 );
	}

	template<>
	inline PxSphericalJoint* createJoint<PxSphericalJoint>(PxPhysics& physics, 
											   PxRigidActor* actor0, const PxTransform& localFrame0, 
											   PxRigidActor* actor1, const PxTransform& localFrame1)
	{
		return PxSphericalJointCreate( physics, actor0, localFrame0, actor1, localFrame1 );
	}
	
	template<typename TJointType>
	PxRepXObject PxJointRepXSerializer<TJointType>::fileToObject( XmlReader& inReader, XmlMemoryAllocator& inAllocator, PxRepXInstantiationArgs& inArgs, PxCollection* inCollection )
	{
		PxRigidActor* actor0 = NULL;
		PxRigidActor* actor1 = NULL;
		PxTransform localPose0 = PxTransform(PxIdentity);
		PxTransform localPose1 = PxTransform(PxIdentity);
		bool ok = true;
		if ( inReader.gotoChild( "Actors" ) )
		{
			ok = readReference<PxRigidActor>( inReader, *inCollection, "actor0", actor0 );
			ok &= readReference<PxRigidActor>( inReader, *inCollection, "actor1", actor1 );
			inReader.leaveChild();
		}
		TJointType* theJoint = !ok ? NULL : createJoint<TJointType>( inArgs.physics, actor0, localPose0, actor1, localPose1 );
		
		if ( theJoint )
        {
            PxConstraint* constraint = theJoint->getConstraint();
			PX_ASSERT( constraint );
			inCollection->add( *constraint ); 
			this->fileToObjectImpl( theJoint, inReader, inAllocator, inArgs, inCollection );
        }
		return createRepXObject( theJoint );
	}

	template<typename TJointType>
	void PxJointRepXSerializer<TJointType>::objectToFileImpl( const TJointType* inObj, PxCollection* inCollection, XmlWriter& inWriter, MemoryBuffer& inTempBuffer, PxRepXInstantiationArgs&   ) 
	{
		writeAllProperties( inObj, inWriter, inTempBuffer, *inCollection );
	}
	
	// explicit instantiations
	template struct PxJointRepXSerializer<PxFixedJoint>;
	template struct PxJointRepXSerializer<PxDistanceJoint>;
	template struct PxJointRepXSerializer<PxD6Joint>;
	template struct PxJointRepXSerializer<PxPrismaticJoint>;
	template struct PxJointRepXSerializer<PxRevoluteJoint>;
	template struct PxJointRepXSerializer<PxSphericalJoint>;
}
