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

#ifndef GU_MESH_FACTORY_H
#define GU_MESH_FACTORY_H

#include "PxTriangleMesh.h"
#include "PxConvexMesh.h"
#include "PxHeightField.h"

#include "CmPhysXCommon.h"
#include "PsMutex.h"
#include "PsArray.h"

#include "PsUserAllocated.h"
#include "PsHashSet.h"

#include "PxIO.h"
namespace physx
{

class PxHeightFieldDesc;

namespace Gu
{
	class ConvexMesh;
	class HeightField;
	class TriangleMesh;
}

class GuMeshFactoryListener
{
protected:
	virtual ~GuMeshFactoryListener(){}
public:
	virtual void onGuMeshFactoryBufferRelease(const PxBase* object, PxType type, bool memRelease) = 0;
};

#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4251 ) // class needs to have dll-interface to be used by clients of class
#endif
class PX_PHYSX_COMMON_API GuMeshFactory : public Ps::UserAllocated
{
	PX_NOCOPY(GuMeshFactory)
public:
									GuMeshFactory()	{}
protected:
	virtual							~GuMeshFactory();

public:
	void							release();

	// Triangle mehes
	void							addTriangleMesh(Gu::TriangleMesh* np, bool lock=true);
	PxTriangleMesh*					createTriangleMesh(PxInputStream&);
	bool							removeTriangleMesh(PxTriangleMesh&);
	PxU32							getNbTriangleMeshes()	const;
	PxU32							getTriangleMeshes(PxTriangleMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex)	const;

	// Convexes
	void							addConvexMesh(Gu::ConvexMesh* np, bool lock=true);
	PxConvexMesh*					createConvexMesh(PxInputStream&);
	bool							removeConvexMesh(PxConvexMesh&);
	PxU32							getNbConvexMeshes() const;
	PxU32							getConvexMeshes(PxConvexMesh** userBuffer, PxU32 bufferSize, PxU32 startIndex)	const;

	// Heightfields
	void							addHeightField(Gu::HeightField* np, bool lock=true);
	PxHeightField*					createHeightField(const PxHeightFieldDesc&);
	PxHeightField*					createHeightField(PxInputStream&);
	bool							removeHeightField(PxHeightField&);
	PxU32							getNbHeightFields()	const;
	PxU32							getHeightFields(PxHeightField** userBuffer, PxU32 bufferSize, PxU32 startIndex)	const;

	void							addFactoryListener( GuMeshFactoryListener& listener );
	void							removeFactoryListener( GuMeshFactoryListener& listener );
	void							notifyFactoryListener(const PxBase*, PxType typeID, bool memRelease);

protected:
#if PX_SUPPORT_GPU_PHYSX
	virtual void					notifyReleaseTriangleMesh(const PxTriangleMesh&) {}
	virtual void					notifyReleaseHeightField(const PxHeightField&) {}
	virtual void					notifyReleaseConvexMesh(const PxConvexMesh&) {}
#endif

	mutable Ps::Mutex				mTrackingMutex;
private:
	Ps::CoalescedHashSet<Gu::TriangleMesh*>	mTriangleMeshes;
	Ps::CoalescedHashSet<Gu::ConvexMesh*>	mConvexMeshes;
	Ps::CoalescedHashSet<Gu::HeightField*>	mHeightFields;

	Ps::Array<GuMeshFactoryListener*>		mFactoryListeners;
};
#if defined(PX_VC) 
     #pragma warning(pop) 
#endif
}

#endif
