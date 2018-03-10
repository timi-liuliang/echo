/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __SAMPLE_TRI_MESH_ACTOR_H__
#define __SAMPLE_TRI_MESH_ACTOR_H__

#include "SampleShapeActor.h"
#include "RendererMeshShape.h"
#include "PxMemoryBuffer.h"
#include "PsArray.h"
#include "NxFromPx.h"
#include "NxApexSDK.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxActor.h"
#include "NxActorDesc.h"
#include <NxTriangleMeshDesc.h>
#include <NxTriangleMeshShapeDesc.h>
#include <NxCooking.h>
#include "NxScene.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "PxActor.h"
#include "PxScene.h"
#include "cooking/PxCooking.h"
#include "cooking/PxConvexMeshDesc.h"
#include "geometry/PxConvexMeshGeometry.h"

#include "geometry/PxTriangleMesh.h"
#include "cooking/PxTriangleMeshDesc.h"
#include "geometry/PxTriangleMeshGeometry.h"

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
//typedef NxConvexMesh ConvexMesh;
typedef NxTriangleMesh TriMesh;
#elif NX_SDK_VERSION_MAJOR == 3
//typedef physx::PxConvexMesh ConvexMesh;
typedef physx::PxTriangleMesh TriMesh;
#endif

class SampleTriMeshActor : public SampleShapeActor
{
public:

#if NX_SDK_VERSION_MAJOR == 2

	SampleTriMeshActor(SampleRenderer::Renderer* renderer,
	               SampleFramework::SampleMaterialAsset& material,
	               NxScene& physxScene,
				   const physx::PxVec3* verts, 
				   const physx::PxU32 nbVerts,
				   const physx::PxU32* indices,
				   const physx::PxU32 nbIndices,
				   physx::PxReal* uvs,
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
		, mTriMesh(NULL)
		, mRendererMeshShape(NULL)
	{
		PX_UNUSED(nxmaterial);
		mRenderer = renderer;

		mTransform = physx::PxMat44::createIdentity();
		mTransform.setPosition(pos);

		createActor(physxScene, verts, nbVerts, indices, nbIndices, pos, vel, density, nxmaterial, useGroupsMask);

		NxTriangleMeshDesc triMeshDesc;
		mTriMesh->saveToDesc(triMeshDesc);

		const physx::PxVec3* vertices = reinterpret_cast<const physx::PxVec3*>(triMeshDesc.points);
		const physx::PxU32* indexBuffer = reinterpret_cast<const physx::PxU32*>(triMeshDesc.triangles);

		physx::PxU32 nbTriangles = triMeshDesc.numTriangles;

		mNbVerts	= 3 * nbTriangles;
		mNbFaces	= mNbVerts;

		mVerts		= new physx::PxVec3[mNbVerts];
		mNormals	= new physx::PxVec3[mNbVerts];
		mFaces		= new physx::PxU16[mNbFaces];
		if (uvs != NULL)
		{
			mUvs		= new physx::PxReal[mNbVerts * 2];
		}

		// no one implemented the PhysX 2 case for the new uvs parameter
		PX_ASSERT(uvs==NULL);

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
			rdebug->debugText(physx::PxVec3(0.f, 2.f, 0.f), "Sample Triangle Mesh:%d", bcount++);
			rdebug->endDrawGroup();
		}
	}

#elif NX_SDK_VERSION_MAJOR == 3

	SampleTriMeshActor(SampleRenderer::Renderer* renderer,
	               SampleFramework::SampleMaterialAsset& material,
	               physx::PxScene& physxScene,
				   const physx::PxVec3* verts, 
				   const physx::PxU32 nbVerts,
				   const physx::PxU32* indices,
				   const physx::PxU32 nbIndices,
				   physx::PxReal* uvs,
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
		, mTriMesh(NULL)
		, mRendererMeshShape(NULL)
	{
		mRenderer = renderer;

		createActor(physxScene, verts, nbVerts, indices, nbIndices, pos, vel, density, pxmaterial, useGroupsMask);

		const bool has16BitIndices = (mTriMesh->getTriangleMeshFlags() & physx::PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES);

		const physx::PxU32   nbTris			= mTriMesh->getNbTriangles();
		const physx::PxU32*  indexBuffer	= (physx::PxU32*) (has16BitIndices ? NULL : mTriMesh->getTriangles());
		const physx::PxU16*  indexBuffer16	= (physx::PxU16*) (has16BitIndices ? mTriMesh->getTriangles() : NULL);
		const physx::PxVec3* vertices		= mTriMesh->getVertices();

		mNbVerts = 3*nbTris;
		mNbFaces = 3*nbTris;

		mVerts		= new physx::PxVec3[mNbVerts];
		mNormals	= new physx::PxVec3[mNbVerts];
		mFaces		= new physx::PxU16[mNbFaces];
		if (uvs != NULL)
		{
			mUvs		= new physx::PxReal[mNbVerts * 2];
		}

		if(indexBuffer)
		{
			for (physx::PxU32 i = 0; i < nbTris; i++)
			{
				const physx::PxVec3& A( vertices[indexBuffer[3*i+0]] );
				const physx::PxVec3& B( vertices[indexBuffer[3*i+1]] );
				const physx::PxVec3& C( vertices[indexBuffer[3*i+2]] );

				physx::PxVec3 a(B-A),b(C-A);
				physx::PxVec3 normal = a.cross(b);
				normal.normalize();

				mVerts[3*i+0] = A;
				mVerts[3*i+1] = C;
				mVerts[3*i+2] = B;
				mNormals[3*i+0] = normal;
				mNormals[3*i+1] = normal;
				mNormals[3*i+2] = normal;
				mFaces[3*i+0] = physx::PxU16(3*i+0);
				mFaces[3*i+1] = physx::PxU16(3*i+1);
				mFaces[3*i+2] = physx::PxU16(3*i+2);
			}
		}
		else if(indexBuffer16)
		{
			for (physx::PxU32 i = 0; i < nbTris; i++)
			{
				const physx::PxVec3& A( verts[indices[3*i+0]] );
				const physx::PxVec3& B( verts[indices[3*i+1]] );
				const physx::PxVec3& C( verts[indices[3*i+2]] );

				physx::PxVec3 a(B-A),b(C-A);
				physx::PxVec3 normal = a.cross(b);
				normal.normalize();

				mVerts[3*i+0] = A;
				mVerts[3*i+1] = C;
				mVerts[3*i+2] = B;
				if (uvs != NULL)
				{
					mUvs[6*i+0] = uvs[2*indices[3*i+0]];
					mUvs[6*i+1] = uvs[2*indices[3*i+0] + 1];
					mUvs[6*i+2] = uvs[2*indices[3*i+2]];
					mUvs[6*i+3] = uvs[2*indices[3*i+2] + 1];
					mUvs[6*i+4] = uvs[2*indices[3*i+1]];
					mUvs[6*i+5] = uvs[2*indices[3*i+1] + 1];
				}
				mNormals[3*i+0] = normal;
				mNormals[3*i+1] = normal;
				mNormals[3*i+2] = normal;
				mFaces[3*i+0] = physx::PxU16(3*i+0);
				mFaces[3*i+1] = physx::PxU16(3*i+1);
				mFaces[3*i+2] = physx::PxU16(3*i+2);
			}
		}
		else
		{
			PX_ASSERT(0 && "Invalid Index Data");
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
			rdebug->debugText(physx::PxVec3(0, 1.0f + 0.01f, 0), "Sample Triangle Mesh:%d", bcount++);
			rdebug->endDrawGroup();
		}
	}

#endif

	virtual ~SampleTriMeshActor()
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
				   const physx::PxU32* indices,
				   const physx::PxU32 nbIndices,
	               const physx::PxVec3& pos,
	               const physx::PxVec3& vel,
	               physx::PxF32 density,
	               void* nxmaterial,
	               bool useGroupsMask) 
	{
		PX_UNUSED(nxmaterial);

		NxTriangleMeshShapeDesc	shapeDesc;
		NxActorDesc			actorDesc;
		NxTriangleMeshDesc	triMeshDesc;

		triMeshDesc.numVertices		= nbVerts;
		triMeshDesc.points			= verts;
		triMeshDesc.pointStrideBytes = sizeof(physx::PxVec3);
		triMeshDesc.numTriangles	= nbIndices/3;
		triMeshDesc.triangles		= indices;
		triMeshDesc.triangleStrideBytes = 3*sizeof(physx::PxU32);

		physx::PxMemoryBuffer stream;
		stream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
		physx::NxFromPxStream nxs(stream);

		physx::NxApexSDK* sdk = physx::apex::NxGetApexSDK();
		if (sdk->getCookingInterface()->NxCookTriangleMesh(triMeshDesc, nxs))
		{
			mTriMesh = sdk->getPhysXSDK()->createTriangleMesh(nxs);
			PX_ASSERT(mTriMesh);
		}

		shapeDesc.meshData = mTriMesh;
		shapeDesc.shapeFlags = NX_SF_VISUALIZATION;

		if (useGroupsMask)
		{
			shapeDesc.groupsMask.bits0 = 1;
			shapeDesc.groupsMask.bits2 = ~0;
		}

		physx::apex::NxFromPxMat34(actorDesc.globalPose, mTransform);
		actorDesc.flags = 0;
		actorDesc.shapes.push_back(&shapeDesc);

		mPhysxActor = physxScene.createActor(actorDesc);
		PX_ASSERT(mPhysxActor);
	}

#elif NX_SDK_VERSION_MAJOR == 3

	void createActor(physx::PxScene& physxScene,
					 const physx::PxVec3* verts,
					 const physx::PxU32 nbVerts,
					 const physx::PxU32* indices,
					 const physx::PxU32 nbIndices,
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
		actor = physxScene.getPhysics().createRigidStatic(physx::PxTransform(mTransform));

		physx::PxTriangleMeshDesc triMeshDesc;
		triMeshDesc.points.count = nbVerts;
		triMeshDesc.points.data = verts;
		triMeshDesc.points.stride = sizeof(physx::PxVec3);
		triMeshDesc.triangles.count = nbIndices/3;
		triMeshDesc.triangles.data = indices;
		triMeshDesc.triangles.stride = 3*sizeof(physx::PxU32);

		physx::PxMemoryBuffer stream;
		stream.setEndianMode(physx::PxFileBuf::ENDIAN_NONE);
		physx::apex::NxFromPxStream nxs(stream);

		physx::NxApexSDK* sdk = physx::apex::NxGetApexSDK();
		if (sdk->getCookingInterface()->cookTriangleMesh(triMeshDesc, nxs))
		{
			mTriMesh = sdk->getPhysXSDK()->createTriangleMesh(nxs);
			PX_ASSERT(mTriMesh);
		}

		physx::PxTriangleMeshGeometry triMeshGeom(mTriMesh);
		physx::PxShape* shape = actor->createShape(triMeshGeom, *pxmaterial);
		PX_ASSERT(shape);
		if (shape && useGroupsMask)
		{
			shape->setSimulationFilterData(physx::PxFilterData(1, 0, ~0u, 0));
			shape->setQueryFilterData(physx::PxFilterData(1, 0, ~0u, 0));
		}
		{
			physxScene.lockWrite(__FILE__, __LINE__);
			physxScene.addActor(*actor);
			physxScene.unlockWrite();
		}		
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

	TriMesh* mTriMesh;

	SampleRenderer::RendererMeshShape* mRendererMeshShape;
};

#endif
