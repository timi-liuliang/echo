/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __SAMPLE_CONVEX_MESH_ACTOR_H__
#define __SAMPLE_CONVEX_MESH_ACTOR_H__

#include "SampleShapeActor.h"
#include "RendererMeshShape.h"
#include "PxMemoryBuffer.h"
#include "PsArray.h"
#include "NxFromPx.h"
#include "NxApexSDK.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxActor.h"
#include "NxActorDesc.h"
#include <NxConvexMeshDesc.h>
#include <NxConvexShapeDesc.h>
#include <NxCooking.h>
#include "NxScene.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxActor.h"
#include "PxScene.h"
#include "cooking/PxCooking.h"
#include "cooking/PxConvexMeshDesc.h"
#include "geometry/PxConvexMeshGeometry.h"

#include "PxRigidDynamic.h"
#include "PxRigidStatic.h"
#include "extensions/PxExtensionsAPI.h"



namespace physx
{
class PxMaterial;
}
#endif

#include "NxApexRenderDebug.h"
#include <Renderer.h>
#include <RendererMeshContext.h>

#if NX_SDK_VERSION_MAJOR == 2
typedef NxConvexMesh ConvexMesh; 
#elif NX_SDK_VERSION_MAJOR == 3
typedef physx::PxConvexMesh ConvexMesh;
#endif

class SampleConvexMeshActor : public SampleShapeActor
{
public:

#if NX_SDK_VERSION_MAJOR == 2

	SampleConvexMeshActor(SampleRenderer::Renderer* renderer,
	               SampleFramework::SampleMaterialAsset& material,
	               NxScene& physxScene,
				   const physx::PxVec3* verts, 
				   const physx::PxU32 nbVerts,
	               const physx::PxVec3& pos,
	               const physx::PxVec3& vel,
	               physx::PxF32 density,
	               void* nxmaterial,
	               bool useGroupsMask,
	               physx::apex::NxApexRenderDebug* rdebug = NULL)
		: SampleShapeActor(rdebug)
		, mVerts(NULL)
		, mNormals(NULL)
		, mUvs(NULL)
		, mFaces(NULL)
		, mNbVerts(0)
		, mNbFaces(0)
		, mConvexMesh(NULL)
		, mRendererMeshShape(NULL)
	{
		PX_UNUSED(nxmaterial);
		mRenderer = renderer;

		mTransform = physx::PxMat44::createIdentity();
		mTransform.setPosition(pos);

		createActor(physxScene, verts, nbVerts, pos, vel, density, nxmaterial, useGroupsMask);

		NxConvexMeshDesc convexMeshDesc;
		mConvexMesh->saveToDesc(convexMeshDesc);

		const physx::PxVec3* vertices = reinterpret_cast<const physx::PxVec3*>(convexMeshDesc.points);
		const physx::PxU32* indexBuffer = reinterpret_cast<const physx::PxU32*>(convexMeshDesc.triangles);

		physx::PxU32 nbTriangles = convexMeshDesc.numTriangles;

		mNbVerts	= 3 * nbTriangles;
		mNbFaces	= mNbVerts;

		mVerts		= new physx::PxVec3[mNbVerts];
		mNormals	= new physx::PxVec3[mNbVerts];
		mFaces		= new physx::PxU16[mNbFaces];

		for (physx::PxU32 i = 0; i < nbTriangles; i++)
		{
		
			mVerts[3*i]		= vertices[indexBuffer[3*i]];
			mVerts[3*i + 1] = vertices[indexBuffer[3*i + 1]];
			mVerts[3*i + 2] = vertices[indexBuffer[3*i + 2]];

			physx::PxVec3 norm = (mVerts[3*i + 1] - mVerts[3*i]).cross(mVerts[3*i + 2] - mVerts[3*i]);
			PX_ASSERT(norm.normalize());

			mNormals[3*i]		= norm;
			mNormals[3*i + 1]	= norm;
			mNormals[3*i + 2]	= norm;

			mFaces[3*i]			= 3*i;
			mFaces[3*i + 1]		= 3*i + 2;
			mFaces[3*i + 2]		= 3*i + 1;
		}

		mRendererMeshShape = new SampleRenderer::RendererMeshShape(*mRenderer, mVerts, mNbVerts, mNormals, mUvs, mFaces, mNbFaces / 3);

		mRendererMeshContext.material         = material.getMaterial();
		mRendererMeshContext.materialInstance = material.getMaterialInstance();
		mRendererMeshContext.mesh             = mRendererMeshShape->getMesh();
		mRendererMeshContext.transform        = &mTransform;

		if (rdebug)
		{
			mBlockId = rdebug->beginDrawGroup(mTransform);
			rdebug->addToCurrentState(physx::DebugRenderState::SolidShaded);
			static physx::PxU32 bcount /* = 0 */;
			rdebug->setCurrentColor(0xFFFFFF);
			rdebug->setCurrentTextScale(0.5f);
			rdebug->addToCurrentState(physx::DebugRenderState::CenterText);
			rdebug->addToCurrentState(physx::DebugRenderState::CameraFacing);
			rdebug->debugText(physx::PxVec3(0.f, 2.f, 0.f), "Sample Convex Mesh:%d", bcount++);
			rdebug->endDrawGroup();
		}
	}

#elif NX_SDK_VERSION_MAJOR == 3

	SampleConvexMeshActor(SampleRenderer::Renderer* renderer,
	               SampleFramework::SampleMaterialAsset& material,
	               physx::PxScene& physxScene,
				   const physx::PxVec3* verts, 
				   const physx::PxU32 nbVerts,
	               const physx::PxVec3& pos,
	               const physx::PxVec3& vel,
	               physx::PxF32 density,
	               physx::PxMaterial* pxmaterial,
	               bool useGroupsMask,
	               physx::apex::NxApexRenderDebug* rdebug = NULL)
		: SampleShapeActor(rdebug)
		, mVerts(NULL)
		, mNormals(NULL)
		, mUvs(NULL)
		, mFaces(NULL)
		, mNbVerts(0)
		, mNbFaces(0)
		, mConvexMesh(NULL)
		, mRendererMeshShape(NULL)
	{
		mRenderer = renderer;

		createActor(physxScene, verts, nbVerts, pos, vel, density, pxmaterial, useGroupsMask);

		const physx::PxU32   nbPolygons		= mConvexMesh->getNbPolygons();
		const physx::PxU8*   indexBuffer	= mConvexMesh->getIndexBuffer();
		const physx::PxVec3* vertices		= mConvexMesh->getVertices();

		for (physx::PxU32 i = 0; i < nbPolygons; i++)
		{
			physx::PxHullPolygon data;
			bool status = mConvexMesh->getPolygonData(i, data);
			PX_ASSERT(status);
			PX_UNUSED(status);

			physx::PxU32 nbPolyVerts = data.mNbVerts;
			mNbVerts += nbPolyVerts;
			mNbFaces += (nbPolyVerts - 2)*3;
		}

		mVerts		= new physx::PxVec3[mNbVerts];
		mNormals	= new physx::PxVec3[mNbVerts];
		mFaces		= new physx::PxU16[mNbFaces];

		physx::PxU32 vertCounter = 0;
		physx::PxU32 facesCounter = 0;
		for (physx::PxU32 i = 0; i < nbPolygons; i++)
		{
			physx::PxHullPolygon data;
			bool status = mConvexMesh->getPolygonData(i, data);
			PX_ASSERT(status);
			PX_UNUSED(status);

			physx::PxVec3 normal(data.mPlane[0], data.mPlane[1], data.mPlane[2]);

			physx::PxU32 vI0 = vertCounter;
			for (physx::PxU32 vI = 0; vI < data.mNbVerts; vI++)
			{
				mVerts[vertCounter] = vertices[indexBuffer[data.mIndexBase + vI]];
				mNormals[vertCounter] = normal;
				vertCounter++;
			}

			for (physx::PxU32 vI = 1; vI < physx::PxU32(data.mNbVerts) - 1; vI++)
			{
				mFaces[facesCounter++]	= physx::PxU16(vI0);
				mFaces[facesCounter++]	= physx::PxU16(vI0 + vI + 1);
				mFaces[facesCounter++]	= physx::PxU16(vI0 + vI);
			}
		}

		mRendererMeshShape = new SampleRenderer::RendererMeshShape(*mRenderer, mVerts, mNbVerts, mNormals, mUvs, mFaces, mNbFaces / 3);

		mRendererMeshContext.material         = material.getMaterial();
		mRendererMeshContext.materialInstance = material.getMaterialInstance();
		mRendererMeshContext.mesh             = mRendererMeshShape->getMesh();
		mRendererMeshContext.transform        = &mTransform;

		if (rdebug)
		{
			mBlockId = rdebug->beginDrawGroup(mTransform);
			rdebug->addToCurrentState(physx::DebugRenderState::SolidShaded);
			static physx::PxU32 bcount /* = 0 */;
			rdebug->setCurrentColor(0xFFFFFF);
			rdebug->setCurrentTextScale(0.5f);
			rdebug->addToCurrentState(physx::DebugRenderState::CenterText);
			rdebug->addToCurrentState(physx::DebugRenderState::CameraFacing);
			rdebug->debugText(physx::PxVec3(0, 1.0f + 0.01f, 0), "Sample Convex Mesh:%d", bcount++);
			rdebug->endDrawGroup();
		}
	}

#endif

	virtual ~SampleConvexMeshActor()
	{
		if (mRendererMeshShape)
		{
			delete[] mVerts;
			delete[] mNormals;

			delete mRendererMeshShape;
			mRendererMeshShape = NULL;
		}
	}

private:

#if NX_SDK_VERSION_MAJOR == 2

	void createActor(NxScene& physxScene,
				   const physx::PxVec3* verts, 
				   const physx::PxU32 nbVerts,
	               const physx::PxVec3& pos,
	               const physx::PxVec3& vel,
	               physx::PxF32 density,
	               void* nxmaterial,
	               bool useGroupsMask) 
	{
		PX_UNUSED(nxmaterial);

		NxBodyDesc			bodyDesc;
		NxConvexShapeDesc	shapeDesc;
		NxActorDesc			actorDesc;
		NxConvexMeshDesc	convexMeshDesc;

		convexMeshDesc.numVertices		= nbVerts;
		convexMeshDesc.points			= verts;
		convexMeshDesc.pointStrideBytes = sizeof(physx::PxVec3);
		convexMeshDesc.flags			= NX_CF_COMPUTE_CONVEX;

		physx::PxMemoryBuffer stream;
		stream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
		physx::NxFromPxStream nxs(stream);

		physx::NxApexSDK* sdk = physx::apex::NxGetApexSDK();
		if (sdk->getCookingInterface()->NxCookConvexMesh(convexMeshDesc, nxs))
		{
			mConvexMesh = sdk->getPhysXSDK()->createConvexMesh(nxs);
			PX_ASSERT(mConvexMesh);
		}
		bodyDesc.linearVelocity = NXFROMPXVEC3(vel);
		bodyDesc.flags = NX_BF_VISUALIZATION;

		shapeDesc.meshData = mConvexMesh;
		shapeDesc.shapeFlags = NX_SF_VISUALIZATION | NX_SF_CLOTH_TWOWAY;
		shapeDesc.materialIndex = 0;

		if (useGroupsMask)
		{
			shapeDesc.groupsMask.bits0 = 1;
			shapeDesc.groupsMask.bits2 = ~0;
		}

		physx::apex::NxFromPxMat34(actorDesc.globalPose, mTransform);
		if (density > 0)
		{
			actorDesc.body = &bodyDesc;
			actorDesc.density = density;
		}
		actorDesc.flags = 0;
		actorDesc.shapes.push_back(&shapeDesc);

		mPhysxActor = physxScene.createActor(actorDesc);
		PX_ASSERT(mPhysxActor);
	}

#elif NX_SDK_VERSION_MAJOR == 3

	void createActor(physx::PxScene& physxScene,
					 const physx::PxVec3* verts,
					 const physx::PxU32 nbVerts,
	                 const physx::PxVec3& pos,
	                 const physx::PxVec3& vel,
	                 physx::PxF32 density,
	                 physx::PxMaterial* pxmaterial,
					 bool useGroupsMask) 
	{
		if (!pxmaterial)
		{
			physxScene.getPhysics().getMaterials(&pxmaterial, 1);
		}

		mTransform = physx::PxMat44::createIdentity();
		mTransform.setPosition(pos);

		physx::PxRigidActor* actor = NULL;
		if (density > 0)
		{
			actor = physxScene.getPhysics().createRigidDynamic(physx::PxTransform(mTransform));
			(static_cast<physx::PxRigidDynamic*>(actor))->setAngularDamping(0.5f); // Is it correct?
			(static_cast<physx::PxRigidDynamic*>(actor))->setLinearVelocity(vel);
		}
		else
		{
			actor = physxScene.getPhysics().createRigidStatic(physx::PxTransform(mTransform));
		}		
		PX_ASSERT(actor);

		physx::PxConvexMeshDesc convexMeshDesc;
		convexMeshDesc.points.count = nbVerts;
		convexMeshDesc.points.data = verts;
		convexMeshDesc.points.stride = sizeof(physx::PxVec3);
		convexMeshDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

		physx::PxMemoryBuffer stream;
		stream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
		physx::apex::NxFromPxStream nxs(stream);

		physx::NxApexSDK* sdk = physx::apex::NxGetApexSDK();
		if (sdk->getCookingInterface()->cookConvexMesh(convexMeshDesc, nxs))
		{
			mConvexMesh = sdk->getPhysXSDK()->createConvexMesh(nxs);
			PX_ASSERT(mConvexMesh);
		}

		physx::PxConvexMeshGeometry convexMeshGeom(mConvexMesh);
		physx::PxShape* shape = actor->createShape(convexMeshGeom, *pxmaterial);
		PX_ASSERT(shape);
		if (shape && useGroupsMask)
		{
			shape->setSimulationFilterData(physx::PxFilterData(1, 0, ~0u, 0));
			shape->setQueryFilterData(physx::PxFilterData(1, 0, ~0u, 0));
		}

		if (density > 0)
		{
			physx::PxRigidBodyExt::updateMassAndInertia(*(static_cast<physx::PxRigidDynamic*>(actor)), density); // () -> static_cast
		}
		SCOPED_PHYSX3_LOCK_WRITE(&physxScene);
		physxScene.addActor(*actor);
		mPhysxActor = actor;
	}
#endif

private:
	physx::PxVec3*	mVerts;
	physx::PxVec3*	mNormals;
	physx::PxReal*	mUvs;
	physx::PxU16*	mFaces;

	physx::PxU32	mNbVerts;
	physx::PxU32	mNbFaces;

	ConvexMesh*		mConvexMesh;

	SampleRenderer::RendererMeshShape* mRendererMeshShape;
};

#endif
