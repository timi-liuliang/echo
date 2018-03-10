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
#ifndef SN_REPX_CORE_SERIALIZER_H
#define SN_REPX_CORE_SERIALIZER_H
/** \addtogroup RepXSerializers
  @{
*/
#include "foundation/PxSimpleTypes.h"
#include "SnRepXSerializerImpl.h"

#ifndef PX_DOXYGEN
namespace physx
{
#endif

	template<typename TLiveType>
	struct RepXSerializerImpl;
	
	class XmlReader;
	class XmlMemoryAllocator;
	class XmlWriter;
	class MemoryBuffer;
				
	struct PxMaterialRepXSerializer : RepXSerializerImpl<PxMaterial>
	{
		PxMaterialRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxMaterial>( inCallback ) {}
		virtual PxMaterial* allocateObject( PxRepXInstantiationArgs& );
	};

	struct PxShapeRepXSerializer : public RepXSerializerImpl<PxShape>
	{
		PxShapeRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxShape>( inCallback ) {}
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxShape* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }
	};
	
	struct PxTriangleMeshRepXSerializer  : public RepXSerializerImpl<PxTriangleMesh>
	{
		PxTriangleMeshRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxTriangleMesh>( inCallback ) {}
		virtual void objectToFileImpl( const PxTriangleMesh*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxTriangleMesh* allocateObject( PxRepXInstantiationArgs&  ) { return NULL; }
	};
	
	struct PxHeightFieldRepXSerializer : public RepXSerializerImpl<PxHeightField>
	{
		PxHeightFieldRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxHeightField>( inCallback ) {}
		virtual void objectToFileImpl( const PxHeightField*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxHeightField* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }
	};
	
	struct PxConvexMeshRepXSerializer  : public RepXSerializerImpl<PxConvexMesh>
	{
		PxConvexMeshRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxConvexMesh>( inCallback ) {}
		virtual void objectToFileImpl( const PxConvexMesh*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxConvexMesh* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }
	};

	struct PxRigidStaticRepXSerializer : public RepXSerializerImpl<PxRigidStatic>
	{
		PxRigidStaticRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxRigidStatic>( inCallback ) {}
		virtual PxRigidStatic* allocateObject( PxRepXInstantiationArgs& );
	};

	struct PxRigidDynamicRepXSerializer : public RepXSerializerImpl<PxRigidDynamic>
	{
		PxRigidDynamicRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxRigidDynamic>( inCallback ) {}
		virtual PxRigidDynamic* allocateObject( PxRepXInstantiationArgs& );
	};
	
	struct PxArticulationRepXSerializer  : public RepXSerializerImpl<PxArticulation>
	{
		PxArticulationRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxArticulation>( inCallback ) {}
		virtual void objectToFileImpl( const PxArticulation*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxArticulation* allocateObject( PxRepXInstantiationArgs& );
	};
	
	struct PxAggregateRepXSerializer :  public RepXSerializerImpl<PxAggregate>
	{
		PxAggregateRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxAggregate>( inCallback ) {}
		virtual void objectToFileImpl( const PxAggregate*, PxCollection*, XmlWriter& , MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxAggregate* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }	
	};

#if PX_USE_CLOTH_API
	struct PxClothFabricRepXSerializer : public RepXSerializerImpl<PxClothFabric>
	{
		PxClothFabricRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxClothFabric>( inCallback ) {}
		virtual void objectToFileImpl( const PxClothFabric*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxClothFabric* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }	
	};

	struct PxClothRepXSerializer : public RepXSerializerImpl<PxCloth>
	{
		PxClothRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<PxCloth>( inCallback ) {}
		virtual void objectToFileImpl( const PxCloth*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual PxCloth* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }	
	};
#endif
	
#if PX_USE_PARTICLE_SYSTEM_API
	template<typename TParticleType>
	struct PxParticleRepXSerializer : RepXSerializerImpl<TParticleType>
	{
		PxParticleRepXSerializer( PxAllocatorCallback& inCallback ) : RepXSerializerImpl<TParticleType>( inCallback ) {}
		virtual void objectToFileImpl( const TParticleType*, PxCollection*, XmlWriter&, MemoryBuffer&, PxRepXInstantiationArgs& );
		virtual PxRepXObject fileToObject( XmlReader&, XmlMemoryAllocator&, PxRepXInstantiationArgs&, PxCollection* );
		virtual TParticleType* allocateObject( PxRepXInstantiationArgs& ) { return NULL; }	
	};
#endif

#ifndef PX_DOXYGEN
} // namespace physx
#endif

#endif
/** @} */

