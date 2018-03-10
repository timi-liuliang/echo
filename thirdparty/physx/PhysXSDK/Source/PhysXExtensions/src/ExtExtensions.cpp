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

#include "PxExtensionsAPI.h"
#include "PsFoundation.h"
#include "PxMetaData.h"
#include "PxGaussMapLimit.h"
#include "ExtDistanceJoint.h"
#include "ExtD6Joint.h"
#include "ExtFixedJoint.h"
#include "ExtPrismaticJoint.h"
#include "ExtRevoluteJoint.h"
#include "ExtSphericalJoint.h"
#include "PxIO.h"
#include "PxRepXSerializer.h"
#include "SnRepXCoreSerializer.h"
#include "SnRepXSerializerImpl.h"
#include "PxExtensionMetaDataObjects.h"
#include "PxJointRepXSerializer.h"
#include "PxSerializer.h"
#include "ExtSerialization.h"

#if PX_SUPPORT_VISUAL_DEBUGGER
#include "PvdConnectionManager.h"
#include "ExtVisualDebugger.h"
#include "PvdConnection.h"
#include "PvdDataStream.h"
#endif

using namespace physx;
using namespace physx::debugger;
using namespace physx::debugger::comm;

#if PX_SUPPORT_VISUAL_DEBUGGER
struct JointConnectionHandler : public physx::debugger::comm::PvdConnectionHandler
{
	virtual void onPvdSendClassDescriptions( physx::debugger::comm::PvdConnection& inFactory )
	{
		using namespace physx::debugger;
		//register the joint classes.
		PvdDataStream* connection = &inFactory.createDataStream();
		connection->addRef();
		Ext::VisualDebugger::sendClassDescriptions( *connection );
		connection->flush();
		connection->release();
	}
	virtual void onPvdConnected( physx::debugger::comm::PvdConnection& )
	{
	}
	virtual void onPvdDisconnected( physx::debugger::comm::PvdConnection&)
	{
	}
};

static JointConnectionHandler gPvdHandler;
#endif

bool PxInitExtensions(PxPhysics& physics)
{
	PX_ASSERT(static_cast<Ps::Foundation*>(&physics.getFoundation()) == &Ps::Foundation::getInstance());
	PX_UNUSED(physics);
	Ps::Foundation::incRefCount();

#if PX_SUPPORT_VISUAL_DEBUGGER
	if ( physics.getPvdConnectionManager() != NULL )
		physics.getPvdConnectionManager()->addHandler( gPvdHandler );
#endif

	return true;
}

void PxCloseExtensions(void)
{	
	Ps::Foundation::decRefCount();
}

void Ext::RegisterExtensionsSerializers(PxSerializationRegistry& sr)
{
	//for repx serialization
	sr.registerRepXSerializer(PxConcreteType::eMATERIAL,		PX_NEW_REPX_SERIALIZER( PxMaterialRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eSHAPE,			PX_NEW_REPX_SERIALIZER( PxShapeRepXSerializer ));	
	sr.registerRepXSerializer(PxConcreteType::eTRIANGLE_MESH,	PX_NEW_REPX_SERIALIZER( PxTriangleMeshRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eHEIGHTFIELD,		PX_NEW_REPX_SERIALIZER( PxHeightFieldRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eCONVEX_MESH,		PX_NEW_REPX_SERIALIZER( PxConvexMeshRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eRIGID_STATIC,	PX_NEW_REPX_SERIALIZER( PxRigidStaticRepXSerializer ));	
	sr.registerRepXSerializer(PxConcreteType::eRIGID_DYNAMIC,	PX_NEW_REPX_SERIALIZER( PxRigidDynamicRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eARTICULATION,	PX_NEW_REPX_SERIALIZER( PxArticulationRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eAGGREGATE,		PX_NEW_REPX_SERIALIZER( PxAggregateRepXSerializer ));
	
#if PX_USE_CLOTH_API
	sr.registerRepXSerializer(PxConcreteType::eCLOTH_FABRIC,	PX_NEW_REPX_SERIALIZER( PxClothFabricRepXSerializer ));
	sr.registerRepXSerializer(PxConcreteType::eCLOTH,			PX_NEW_REPX_SERIALIZER( PxClothRepXSerializer ));
#endif
#if PX_USE_PARTICLE_SYSTEM_API
	sr.registerRepXSerializer(PxConcreteType::ePARTICLE_SYSTEM,	PX_NEW_REPX_SERIALIZER( PxParticleRepXSerializer<PxParticleSystem> ));
	sr.registerRepXSerializer(PxConcreteType::ePARTICLE_FLUID,	PX_NEW_REPX_SERIALIZER( PxParticleRepXSerializer<PxParticleFluid> ));
#endif
	
	sr.registerRepXSerializer(PxJointConcreteType::eFIXED,		PX_NEW_REPX_SERIALIZER( PxJointRepXSerializer<PxFixedJoint> ));
	sr.registerRepXSerializer(PxJointConcreteType::eDISTANCE,	PX_NEW_REPX_SERIALIZER( PxJointRepXSerializer<PxDistanceJoint> ));
	sr.registerRepXSerializer(PxJointConcreteType::eD6,			PX_NEW_REPX_SERIALIZER( PxJointRepXSerializer<PxD6Joint> ));
	sr.registerRepXSerializer(PxJointConcreteType::ePRISMATIC,	PX_NEW_REPX_SERIALIZER( PxJointRepXSerializer<PxPrismaticJoint> ));
	sr.registerRepXSerializer(PxJointConcreteType::eREVOLUTE,	PX_NEW_REPX_SERIALIZER( PxJointRepXSerializer<PxRevoluteJoint> ));
	sr.registerRepXSerializer(PxJointConcreteType::eSPHERICAL,	PX_NEW_REPX_SERIALIZER( PxJointRepXSerializer<PxSphericalJoint> ));

	//for binary serialization
	sr.registerSerializer(PxJointConcreteType::eFIXED,		  PX_NEW_SERIALIZER_ADAPTER( FixedJoint ));
	sr.registerSerializer(PxJointConcreteType::eDISTANCE,     PX_NEW_SERIALIZER_ADAPTER( DistanceJoint ));
	sr.registerSerializer(PxJointConcreteType::eD6,           PX_NEW_SERIALIZER_ADAPTER( D6Joint) );
	sr.registerSerializer(PxJointConcreteType::ePRISMATIC,    PX_NEW_SERIALIZER_ADAPTER( PrismaticJoint ));
	sr.registerSerializer(PxJointConcreteType::eREVOLUTE,     PX_NEW_SERIALIZER_ADAPTER( RevoluteJoint ));
	sr.registerSerializer(PxJointConcreteType::eSPHERICAL,    PX_NEW_SERIALIZER_ADAPTER( SphericalJoint ));
}

void Ext::UnregisterExtensionsSerializers(PxSerializationRegistry& sr)
{
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxJointConcreteType::eFIXED));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxJointConcreteType::eDISTANCE));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxJointConcreteType::eD6 ));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxJointConcreteType::ePRISMATIC));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxJointConcreteType::eREVOLUTE));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxJointConcreteType::eSPHERICAL));

	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eMATERIAL));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eSHAPE));	
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eTRIANGLE_MESH));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eHEIGHTFIELD));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eCONVEX_MESH));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eRIGID_STATIC));	
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eRIGID_DYNAMIC));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eARTICULATION));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eAGGREGATE));
	
#if PX_USE_CLOTH_API
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eCLOTH_FABRIC));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::eCLOTH));
#endif
#if PX_USE_PARTICLE_SYSTEM_API
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::ePARTICLE_SYSTEM));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxConcreteType::ePARTICLE_FLUID));
#endif
	
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxJointConcreteType::eFIXED));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxJointConcreteType::eDISTANCE));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxJointConcreteType::eD6));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxJointConcreteType::ePRISMATIC));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxJointConcreteType::eREVOLUTE));
	PX_DELETE_REPX_SERIALIZER(sr.unregisterRepXSerializer(PxJointConcreteType::eSPHERICAL));
}
