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

#ifndef PX_META_DATA_PVD_BINDING_H
#define PX_META_DATA_PVD_BINDING_H

#if PX_SUPPORT_VISUAL_DEBUGGER

#include "PxPhysXConfig.h"

namespace physx
{

class PxScene;
class PxMaterial;
class PxShape;
class PxRigidStatic;
class PxRigidDynamic;
class PxArticulation;
class PxArticulationLink;
class PxArticulationJoint;
class PxActor;
class PxParticleSystem;
class PxParticleFluid;
class PxCloth;
class PxParticleReadData;
class PxParticleFluidReadData;
class PxPhysics;
class PxClothFabric;
class PxCloth;
class PxHeightField;
class PxTriangleMesh;
class PxConvexMesh;
class PxRigidActor;

namespace Sc
{
	class ContactIterator;
	class BodyIterator;
	class BodyCore;
	struct ClothBulkData;
}

namespace debugger { namespace comm {
	class PvdDataStream;
}}

namespace Pvd
{
	using namespace physx::debugger::comm;
	struct PvdMetaDataBindingData;

	class BufferRegistrar
	{
	protected:
		virtual ~BufferRegistrar(){}
	public:
		virtual void addRef( const PxMaterial* buffer ) = 0;
		virtual void addRef( const PxConvexMesh* buffer ) = 0;
		virtual void addRef( const PxTriangleMesh* buffer ) = 0;
		virtual void addRef( const PxHeightField* buffer ) = 0;
#if PX_USE_CLOTH_API
		virtual void addRef( const PxClothFabric* buffer ) = 0;
#endif
	};

	class PvdVisualizer
	{
	protected:
		virtual ~PvdVisualizer(){}
	public:
		virtual void visualize( PxArticulationLink& link ) = 0;
	};

	class PvdMetaDataBinding
	{
		PvdMetaDataBindingData*				mBindingData;

	public:
		PvdMetaDataBinding();
		~PvdMetaDataBinding();
		void registerSDKProperties( PvdDataStream& inStream );

		void sendAllProperties( PvdDataStream& inStream, const PxPhysics& inPhysics );

		void sendAllProperties( PvdDataStream& inStream, const PxScene& inScene );
		//per frame update
		void sendBeginFrame( PvdDataStream& inStream, const PxScene* inScene, PxReal simulateElapsedTime );
		void sendContacts( PvdDataStream& inStream, const PxScene& inScene, Sc::ContactIterator& inContacts );
		void sendContacts( PvdDataStream& inStream, const PxScene& inScene );
		void sendSceneQueries( PvdDataStream& inStream, const PxScene& inScene, bool hasValue );
		void sendStats( PvdDataStream& inStream, const PxScene* inScene );
		void sendEndFrame( PvdDataStream& inStream, const PxScene* inScene );


		void createInstance( PvdDataStream& inStream, const PxMaterial& inMaterial, const PxPhysics& ownerPhysics );
		void sendAllProperties( PvdDataStream& inStream, const PxMaterial& inMaterial );
		void destroyInstance( PvdDataStream& inStream, const PxMaterial& inMaterial, const PxPhysics& ownerPhysics );
		void createInstance( PvdDataStream& inStream, const PxHeightField& inData, const PxPhysics& ownerPhysics );
		void sendAllProperties( PvdDataStream& inStream, const PxHeightField& inData );
		void destroyInstance( PvdDataStream& inStream, const PxHeightField& inData, const PxPhysics& ownerPhysics );
		void createInstance( PvdDataStream& inStream, const PxConvexMesh& inData, const PxPhysics& ownerPhysics );
		void destroyInstance( PvdDataStream& inStream, const PxConvexMesh& inData, const PxPhysics& ownerPhysics );
		void createInstance( PvdDataStream& inStream, const PxTriangleMesh& inData, const PxPhysics& ownerPhysics );
		void destroyInstance( PvdDataStream& inStream, const PxTriangleMesh& inData, const PxPhysics& ownerPhysics );

		void createInstance( PvdDataStream& inStream, const PxShape& inObj, const PxRigidActor& owner, BufferRegistrar& registrar );
		void sendAllProperties( PvdDataStream& inStream, const PxShape& inObj );
		void releaseAndRecreateGeometry( PvdDataStream& inStream, const PxShape& inObj, PxPhysics& ownerPhysics, BufferRegistrar& registrar );
		void updateMaterials( PvdDataStream& inStream, const PxShape& inObj, BufferRegistrar& registrar );
		void destroyInstance( PvdDataStream& inStream, const PxShape& inObj, const PxRigidActor& owner );

		void createInstance( PvdDataStream& inStream, const PxRigidStatic& inObj, const PxScene& ownerScene, BufferRegistrar& registrar );
		void sendAllProperties( PvdDataStream& inStream, const PxRigidStatic& inObj );
		void destroyInstance( PvdDataStream& inStream, const PxRigidStatic& inObj, const PxScene& ownerScene );
		void createInstance( PvdDataStream& inStream, const PxRigidDynamic& inObj, const PxScene& ownerScene, BufferRegistrar& registrar );
		void sendAllProperties( PvdDataStream& inStream, const PxRigidDynamic& inObj );
		void destroyInstance( PvdDataStream& inStream, const PxRigidDynamic& inObj, const PxScene& ownerScene );

		void createInstance( PvdDataStream& inStream, const PxArticulation& inObj, const PxScene& ownerScene, BufferRegistrar& registrar );
		void sendAllProperties( PvdDataStream& inStream, const PxArticulation& inObj );
		void destroyInstance( PvdDataStream& inStream, const PxArticulation& inObj, const PxScene& ownerScene );

		void createInstance( PvdDataStream& inStream, const PxArticulationLink& inObj, BufferRegistrar& registrar );
		void sendAllProperties( PvdDataStream& inStream, const PxArticulationLink& inObj );
		void destroyInstance( PvdDataStream& inStream, const PxArticulationLink& inObj );

		//These are created as part of the articulation link's creation process, so outside entities don't need to
		//create them.
		void sendAllProperties( PvdDataStream& inStream, const PxArticulationJoint& inObj );
		
		//per frame update
		void updateDynamicActorsAndArticulations( PvdDataStream& inStream, const PxScene* inScene, PvdVisualizer* linkJointViz );
		
		//Origin Shift
		void originShift( PvdDataStream& inStream, const PxScene* inScene, PxVec3 shift );

#if PX_USE_PARTICLE_SYSTEM_API
		void createInstance( PvdDataStream& inStream, const PxParticleSystem& inObj, const PxScene& ownerScene );
		void sendAllProperties( PvdDataStream& inStream, const PxParticleSystem& inObj );
		//per frame update
		void sendArrays( PvdDataStream& inStream, const PxParticleSystem& inObj, PxParticleReadData& inData, PxU32 inFlags );
		void destroyInstance( PvdDataStream& inStream, const PxParticleSystem& inObj, const PxScene& ownerScene );

		void createInstance( PvdDataStream& inStream, const PxParticleFluid& inObj, const PxScene& ownerScene );
		void sendAllProperties( PvdDataStream& inStream, const PxParticleFluid& inObj );
		//per frame update
		void sendArrays( PvdDataStream& inStream, const PxParticleFluid& inObj, PxParticleFluidReadData& inData, PxU32 inFlags );
		void destroyInstance( PvdDataStream& inStream, const PxParticleFluid& inObj, const PxScene& ownerScene );
#endif

#if PX_USE_CLOTH_API
		void createInstance( PvdDataStream& inStream, const PxClothFabric& fabric, const PxPhysics& ownerPhysics );
		void sendAllProperties( PvdDataStream& inStream, const PxClothFabric& fabric );
		void destroyInstance( PvdDataStream& inStream, const PxClothFabric& fabric, const PxPhysics& ownerPhysics );

		void createInstance( PvdDataStream& inStream, const PxCloth& cloth, const PxScene& ownerScene, BufferRegistrar& registrar );
		void sendAllProperties( PvdDataStream& inStream, const PxCloth& cloth );
		void sendSimpleProperties( PvdDataStream& inStream, const PxCloth& cloth );
		void sendMotionConstraints( PvdDataStream& inStream, const PxCloth& cloth );
		void sendCollisionSpheres( PvdDataStream& inStream, const PxCloth& cloth, bool sendPairs = true );
		void sendCollisionTriangles( PvdDataStream& inStream, const PxCloth& cloth );
		void sendVirtualParticles( PvdDataStream& inStream, const PxCloth& cloth );
		void sendSeparationConstraints( PvdDataStream& inStream, const PxCloth& cloth );
		void sendRestPositions( PvdDataStream& inStream, const PxCloth& cloth );
		void sendSelfCollisionIndices( PvdDataStream& inStream, const PxCloth& cloth );
		void sendParticleAccelerations( PvdDataStream& inStream, const PxCloth& cloth );
		//per frame update
		void updateCloths( PvdDataStream& inStream, const PxScene& inScene );
		void destroyInstance( PvdDataStream& inStream, const PxCloth& cloth, const PxScene& ownerScene );
#endif

		void createInstance( PvdDataStream& inStream, const PxAggregate& inObj, const PxScene& ownerScene, BufferRegistrar& registrar  );
		void sendAllProperties( PvdDataStream& inStream, const PxAggregate& inObj );
		void destroyInstance( PvdDataStream& inStream, const PxAggregate& inObj, const PxScene& ownerScene );
		void detachAggregateActor( PvdDataStream& inStream, const PxAggregate& inObj, const PxActor& inActor );
		void attachAggregateActor( PvdDataStream& inStream, const PxAggregate& inObj, const PxActor& inActor );
	};
}

}

#endif
#endif
