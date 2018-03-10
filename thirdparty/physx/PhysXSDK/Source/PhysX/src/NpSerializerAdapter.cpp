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

#include "GuHeightField.h"
#include "GuConvexMesh.h"
#include "GuTriangleMesh.h"
#include "NpClothFabric.h"
#include "NpCloth.h"
#include "NpParticleSystem.h"
#include "NpParticleFluid.h"

#include "NpRigidStatic.h"
#include "NpRigidDynamic.h"
#include "NpArticulation.h"
#include "NpArticulationLink.h"
#include "NpArticulationJoint.h"
#include "NpMaterial.h"
#include "NpAggregate.h"
#include "GuHeightFieldData.h"

#include "PxBase.h"
#include "PxSerialFramework.h"
#include "PxSerializer.h"

namespace physx
{
	using namespace physx::Gu;

	template<>
	void PxSerializerDefaultAdapter<NpRigidDynamic>::exportData(PxBase& obj, PxSerializationContext& s)   const 
	{
		PxU32 classSize = sizeof(NpRigidDynamic);
		NpRigidDynamic& dynamic = static_cast<NpRigidDynamic&>(obj);

		PxsBodyCore serialCore;
		size_t address = dynamic.getScbBodyFast().getScBody().getSerialCore(serialCore);
		PxU32 offset =  (PxU32)(address - reinterpret_cast<size_t>(&dynamic));
		PX_ASSERT(offset + sizeof(serialCore) <= classSize);
		s.writeData(&dynamic, offset); 
		s.writeData(&serialCore, sizeof(serialCore));
		void* tail = reinterpret_cast<PxU8*>(&dynamic) + offset + sizeof(serialCore);
		s.writeData(tail, classSize - offset - sizeof(serialCore));
	}

	template<>
	void PxSerializerDefaultAdapter<NpRigidDynamic>::registerReferences(PxBase& obj, PxSerializationContext& s)   const 
	{
		NpRigidDynamic& dynamic = static_cast<NpRigidDynamic&>(obj);

		s.registerReference(obj, PX_SERIAL_REF_KIND_PXBASE, size_t(&obj));

		struct RequiresCallback : public PxProcessPxBaseCallback
		{
			RequiresCallback(physx::PxSerializationContext& c) : context(c) {}
			RequiresCallback& operator=(const RequiresCallback&) { PX_ASSERT(0); return *this; } //PX_NOCOPY doesn't work for local classes
			void process(PxBase& base)
			{
				context.registerReference(base, PX_SERIAL_REF_KIND_PXBASE, size_t(&base));
			}
			PxSerializationContext& context;
		};

		RequiresCallback callback(s);
		dynamic.requires(callback);
	}

	template<>
	void PxSerializerDefaultAdapter<NpShape>::registerReferences(PxBase& obj, PxSerializationContext& s)   const 
	{	
		NpShape& shape = static_cast<NpShape&>(obj);

		s.registerReference(obj, PX_SERIAL_REF_KIND_PXBASE, size_t(&obj));

		struct RequiresCallback : public PxProcessPxBaseCallback
		{
			RequiresCallback(physx::PxSerializationContext& c) : context(c) {}
			RequiresCallback &operator=(const RequiresCallback&) { PX_ASSERT(0); return *this; } //PX_NOCOPY doesn't work for local classes
			void process(PxBase& base)
			{
				PxMaterial* pxMaterial = base.is<PxMaterial>();
				if (!pxMaterial)
				{
					context.registerReference(base, PX_SERIAL_REF_KIND_PXBASE, size_t(&base));
				}
				else
				{
					//ideally we would move this part to ScShapeCore but we don't yet have a MaterialManager available there.
					PxU32 index = static_cast<NpMaterial*>(pxMaterial)->getHandle();
					context.registerReference(base, PX_SERIAL_REF_KIND_MATERIAL_IDX, size_t(index));
				}
			}
			PxSerializationContext& context;
		};

		RequiresCallback callback(s);
		shape.requires(callback);
	}

	template<>
	bool PxSerializerDefaultAdapter<NpConstraint>::isSubordinate() const
	{
		return true;
	}

	template<>
	bool PxSerializerDefaultAdapter<NpArticulationJoint>::isSubordinate() const
	{
		return true;
	}

	template<>
	bool PxSerializerDefaultAdapter<NpArticulationLink>::isSubordinate() const
	{
		return true;
	}
}



void PxRegisterPhysicsSerializers(PxSerializationRegistry& sr)
{
	sr.registerSerializer(PxConcreteType::eCONVEX_MESH,         PX_NEW_SERIALIZER_ADAPTER(ConvexMesh));
	sr.registerSerializer(PxConcreteType::eTRIANGLE_MESH,       PX_NEW_SERIALIZER_ADAPTER(TriangleMesh));
	sr.registerSerializer(PxConcreteType::eHEIGHTFIELD,         PX_NEW_SERIALIZER_ADAPTER(HeightField));
	sr.registerSerializer(PxConcreteType::eRIGID_DYNAMIC,       PX_NEW_SERIALIZER_ADAPTER(NpRigidDynamic));
	sr.registerSerializer(PxConcreteType::eRIGID_STATIC,        PX_NEW_SERIALIZER_ADAPTER(NpRigidStatic));
	sr.registerSerializer(PxConcreteType::eSHAPE,               PX_NEW_SERIALIZER_ADAPTER(NpShape));
	sr.registerSerializer(PxConcreteType::eMATERIAL,            PX_NEW_SERIALIZER_ADAPTER(NpMaterial));
	sr.registerSerializer(PxConcreteType::eCONSTRAINT,          PX_NEW_SERIALIZER_ADAPTER(NpConstraint));
	sr.registerSerializer(PxConcreteType::eAGGREGATE,           PX_NEW_SERIALIZER_ADAPTER(NpAggregate));
	sr.registerSerializer(PxConcreteType::eARTICULATION,        PX_NEW_SERIALIZER_ADAPTER(NpArticulation));
	sr.registerSerializer(PxConcreteType::eARTICULATION_LINK,   PX_NEW_SERIALIZER_ADAPTER(NpArticulationLink));
	sr.registerSerializer(PxConcreteType::eARTICULATION_JOINT,  PX_NEW_SERIALIZER_ADAPTER(NpArticulationJoint));
	
#if PX_USE_CLOTH_API
	sr.registerSerializer(PxConcreteType::eCLOTH,               PX_NEW_SERIALIZER_ADAPTER(NpCloth));
	sr.registerSerializer(PxConcreteType::eCLOTH_FABRIC,        PX_NEW_SERIALIZER_ADAPTER(NpClothFabric));
#endif

#if PX_USE_PARTICLE_SYSTEM_API
	sr.registerSerializer(PxConcreteType::ePARTICLE_SYSTEM,     PX_NEW_SERIALIZER_ADAPTER(NpParticleSystem));
	sr.registerSerializer(PxConcreteType::ePARTICLE_FLUID,      PX_NEW_SERIALIZER_ADAPTER(NpParticleFluid));
#endif

}


void PxUnregisterPhysicsSerializers(PxSerializationRegistry& sr)
{
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eCONVEX_MESH));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eTRIANGLE_MESH));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eHEIGHTFIELD));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eRIGID_DYNAMIC));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eRIGID_STATIC));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eSHAPE));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eMATERIAL));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eCONSTRAINT));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eAGGREGATE));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eARTICULATION));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eARTICULATION_LINK));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eARTICULATION_JOINT));
	
#if PX_USE_CLOTH_API
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eCLOTH));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::eCLOTH_FABRIC));
#endif

#if PX_USE_PARTICLE_SYSTEM_API
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::ePARTICLE_SYSTEM));
	PX_DELETE_SERIALIZER_ADAPTER(sr.unregisterSerializer(PxConcreteType::ePARTICLE_FLUID));
#endif
}
