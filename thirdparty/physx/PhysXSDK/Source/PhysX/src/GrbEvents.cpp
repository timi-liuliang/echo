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

#include "GrbEvents.h"
#include "GrbSceneEventDescs.h"

#include "NpScene.h"
#include "PsString.h" // for sprintf


#if USE_GRB_INTEROP


namespace physx
{

namespace GrbInterop3
{

class ClonedTriangleMesh : public Gu::TriangleMesh
{
public:

	virtual	const PxU32*			getTrianglesRemap()										const
	{
		return mRemap;
	}

	virtual	PxMaterialTableIndex	getTriangleMaterialIndex(PxTriangleID triangleIndex)	const
	{
		return mMaterials ? mMaterials[triangleIndex] : PxMaterialTableIndex(0xffff);	
	}

	PxU16*						mMaterials;		
	PxU32*						mRemap;				
};

//-----------------------------------------------------------------------------

class ClonedHeightField : public PxHeightField
{
public:

	void copyDataAndSamples(Gu::HeightField * other, GrbInterop3::StackAllocator &stackAlloc)
	{
		memcpy(&mData, &other->getData(), sizeof(Gu::HeightFieldData));

		PxU32 numBytes = other->getData().rows * other->getData().columns * sizeof(PxHeightFieldSample);
		mData.samples = (PxHeightFieldSample *)stackAlloc.allocate(numBytes);

		memcpy(mData.samples, other->getData().samples, numBytes);
	}

	Gu::HeightFieldData	mData;

	// Interface stuff
	ClonedHeightField() : PxHeightField(PxConcreteType::eHEIGHTFIELD, PxBaseFlag::eOWNS_MEMORY | PxBaseFlag::eIS_RELEASABLE) {}
	virtual		PxU32										getObjectSize() const { return sizeof(*this); }

	virtual	void						release() {}
	virtual	PxU32						saveCells(void* /*destBuffer*/, PxU32 /*destBufferSize*/) const { return 0; }
	virtual	bool						modifySamples(PxI32 /*startCol*/, PxI32 /*startRow*/, const PxHeightFieldDesc& /*subfieldDesc*/, bool /*shrinkBounds*/) { return false; }

	virtual	PxU32						getNbRows()						const { return mData.rows;								}
	virtual	PxU32						getNbColumns()					const { return mData.columns;							}
	virtual	PxHeightFieldFormat::Enum	getFormat()						const { return mData.format;							}
	virtual	PxU32						getSampleStride()				const { return sizeof(PxHeightFieldSample);				}
	virtual	PxReal						getThickness()					const { return mData.thickness;							}
	virtual	PxReal						getConvexEdgeThreshold()		const { return mData.convexEdgeThreshold;				}
	virtual	PxHeightFieldFlags			getFlags()						const { return mData.flags;								}
	virtual	PxU32						getReferenceCount()				const { return 0;										}
	
	PX_FORCE_INLINE	PxU16	getMaterialIndex0(PxU32 vertexIndex) const		{ return mData.samples[vertexIndex].materialIndex0;	}
	PX_FORCE_INLINE	PxU16	getMaterialIndex1(PxU32 vertexIndex) const		{ return mData.samples[vertexIndex].materialIndex1;	}
	PX_FORCE_INLINE PxReal	getHeight(PxU32 vertexIndex) const				{ return PxReal(mData.samples[vertexIndex].height); }
	PX_FORCE_INLINE	bool	isZerothVertexShared(PxU32 vertexIndex) const	{ return mData.samples[vertexIndex].tessFlag() != 0; }
	PX_FORCE_INLINE	bool	isFirstTriangle(PxU32 triangleIndex) const		{ return ((triangleIndex & 0x1) == 0);	}

	PxU32 computeCellCoordinates(PxReal x, PxReal z, PxReal& fracX, PxReal& fracZ) const
	{
		namespace i = physx::intrinsics;

		x = i::selectMax(x, 0.0f);
		z = i::selectMax(z, 0.0f);
		PxF32 epsx = 1.0f - PxAbs(x+1.0f) * 1e-6f; // epsilon needs to scale with values of x,z...
		PxF32 epsz = 1.0f - PxAbs(z+1.0f) * 1e-6f;
		PxF32 x1 = i::selectMin(x, mData.rowLimit+epsx);
		PxF32 z1 = i::selectMin(z, mData.colLimit+epsz);
		x = PxFloor(x1);
		fracX = x1 - x;
		z = PxFloor(z1);
		fracZ = z1 - z;
		PX_ASSERT(x >= 0.0f && x < PxF32(mData.rows));
		PX_ASSERT(z >= 0.0f && z < PxF32(mData.columns));

		const PxU32 vertexIndex = PxU32(x * (mData.nbColumns) + z);
		PX_ASSERT(vertexIndex < (mData.rows)*(mData.columns));

		return vertexIndex;
	}

	virtual	PxMaterialTableIndex		getTriangleMaterialIndex(PxTriangleID triangleIndex) const { return isFirstTriangle(triangleIndex) ? getMaterialIndex0(triangleIndex >> 1) : getMaterialIndex1(triangleIndex >> 1); }
	virtual	PxVec3						getTriangleNormal(PxTriangleID triangleIndex) const
	{
		PxU32 v0, v1, v2;
		getTriangleVertexIndices(triangleIndex, v0, v1, v2); 

		const PxI32 h0 = mData.samples[v0].height;
		const PxI32 h1 = mData.samples[v1].height;
		const PxI32 h2 = mData.samples[v2].height;

		// Fix for NvBug 685420
		//if(mThickness>0.0f)
		//	n = -n;
		const PxReal coeff = physx::intrinsics::fsel(mData.thickness, -1.0f, 1.0f);

		const PxU32 cell = triangleIndex >> 1;
		if (isZerothVertexShared(cell))
		{
			//      <---- COL  
			//      0----2  1 R
			//      | 1 /  /| O
			//      |  /  / | W
			//      | /  /  | |
			//      |/  / 0 | |
			//      1  2----0 V
			//      
			if (isFirstTriangle(triangleIndex))
			{
				return PxVec3(coeff*PxReal(h1-h0), coeff, coeff*PxReal(h0-h2));
			}
			else
			{
				return PxVec3(coeff*PxReal(h0-h1), coeff, coeff*PxReal(h2-h0));
			}
		}
		else
		{
			//      <---- COL  
			//      2  1----0 R
			//      |\  \ 0 | O
			//      | \  \  | W
			//      |  \  \ | |
			//      | 1 \  \| |
			//      0----1  2 V
			//                   
			if (isFirstTriangle(triangleIndex))
			{
				return PxVec3(coeff*PxReal(h0-h2), coeff, coeff*PxReal(h0-h1));
			}
			else
			{
				return PxVec3(coeff*PxReal(h2-h0), coeff, coeff*PxReal(h1-h0));
			}
		}
	}
	PX_INLINE		void getTriangleVertexIndices(PxU32 triangleIndex, PxU32& vertexIndex0, PxU32& vertexIndex1, PxU32& vertexIndex2) const
	{
		const PxU32 cell = triangleIndex >> 1;
		if (isZerothVertexShared(cell))
		{
			//      <---- COL  
			//      0----2  1 R
			//      | 1 /  /| O
			//      |  /  / | W
			//      | /  /  | |
			//      |/  / 0 | |
			//      1  2----0 V
			//      
			if (isFirstTriangle(triangleIndex))
			{
				vertexIndex0 = cell + mData.columns;
				vertexIndex1 = cell;
				vertexIndex2 = cell + mData.columns + 1;
			}
			else
			{
				vertexIndex0 = cell + 1;
				vertexIndex1 = cell + mData.columns + 1;
				vertexIndex2 = cell;
			}
		}
		else
		{
			//      <---- COL  
			//      2  1----0 R
			//      |\  \ 0 | O
			//      | \  \  | W
			//      |  \  \ | |
			//      | 1 \  \| |
			//      0----1  2 V
			//                   
			if (isFirstTriangle(triangleIndex))
			{
				vertexIndex0 = cell;
				vertexIndex1 = cell + 1;
				vertexIndex2 = cell + mData.columns;
			}
			else
			{
				vertexIndex0 = cell + mData.columns + 1;
				vertexIndex1 = cell + mData.columns;
				vertexIndex2 = cell + 1;
			}
		}
	}

	virtual	PxReal						getHeight(PxReal x, PxReal z)	const
	{
		PxReal fracX, fracZ;
		const PxU32 vertexIndex = computeCellCoordinates(x, z, fracX, fracZ);

		if (isZerothVertexShared(vertexIndex))
		{
			//    <----Z---+
			//      +----+ | 
			//      |   /| |
			//      |  / | X
			//      | /  | |
			//      |/   | |
			//      +----+ |
			//             V
			const PxReal h0 = getHeight(vertexIndex);
			const PxReal h2 = getHeight(vertexIndex + mData.columns + 1);
			if (fracZ > fracX)
			{
				//    <----Z---+
				//      1----0 | 
				//      |   /  |
				//      |  /   X
				//      | /    |
				//      |/     |
				//      2      |
				//             V
				const PxReal h1 = getHeight(vertexIndex + 1);
				return h0 + fracZ*(h1-h0) + fracX*(h2-h1);
			}
			else
			{
				//    <----Z---+
				//           0 | 
				//          /| |
				//         / | X
				//        /  | |
				//       /   | |
				//      2----1 |
				//             V
				const PxReal h1 = getHeight(vertexIndex + mData.columns);
				return h0 + fracX*(h1-h0) + fracZ*(h2-h1);
			}
		}
		else
		{
			//    <----Z---+
			//      +----+ | 
			//      |\   | |
			//      | \  | X
			//      |  \ | |
			//      |   \| |
			//      +----+ |
			//             V
			const PxReal h2 = getHeight(vertexIndex + mData.columns);
			const PxReal h1 = getHeight(vertexIndex + 1);
			if (fracX + fracZ < 1.0f)
			{
				//    <----Z---+
				//      1----0 | 
				//       \   | |
				//        \  | X
				//         \ | |
				//          \| |
				//           2 |
				//             V
				const PxReal h0 = getHeight(vertexIndex);
				return h0 + fracZ*(h1-h0) + fracX*(h2-h0);
			}
			else
			{
				//    <----Z---+
				//      1      | 
				//      |\     |
				//      | \    X
				//      |  \   |
				//      |   \  |
				//      0----2 |
				//             V
				//
				// Note that we need to flip fracX and fracZ since we are moving the origin
				const PxReal h0 = getHeight(vertexIndex + mData.columns + 1);
				return h0 + (1.0f - fracZ)*(h2-h0) + (1.0f - fracX)*(h1-h0);
			}
		}
	}
};

//-----------------------------------------------------------------------------
void clonePhysXGeometry(PxGeometryHolder & clone, const PxGeometryHolder & orig, StackAllocator &stackAlloc)
{
	clone = orig;

	//Some geometries have pointers that need to be cloned
	switch (orig.getType())
	{
		//Basic geometries don't have any pointers
	case PxGeometryType::eSPHERE:
	case PxGeometryType::ePLANE:
	case PxGeometryType::eBOX:
	case PxGeometryType::eCAPSULE:
			break;
	case PxGeometryType::eCONVEXMESH:
		{
			const PxConvexMeshGeometry & origGeom = orig.convexMesh();
			PxConvexMeshGeometry & clonedGeom = clone.convexMesh();

			Gu::ConvexMesh * origConvexMesh = static_cast<Gu::ConvexMesh *>(origGeom.convexMesh);
			Gu::ConvexMesh * clonedConvexMesh = new (stackAlloc.allocate(sizeof(Gu::ConvexMesh))) Gu::ConvexMesh();

			size_t numBytes = origConvexMesh->getBufferSize();
			clonedConvexMesh->getHull().mPolygons = reinterpret_cast<Gu::HullPolygonData *>((stackAlloc.allocate(numBytes)));
			memcpy(clonedConvexMesh->getHull().mPolygons, (void *)origConvexMesh->getHull().mPolygons, numBytes);

			clonedConvexMesh->getHull().mNbEdges = origConvexMesh->getHull().mNbEdges;
			clonedConvexMesh->getHull().mNbHullVertices = origConvexMesh->getHull().mNbHullVertices;
			clonedConvexMesh->getHull().mNbPolygons = origConvexMesh->getHull().mNbPolygons;

			clonedConvexMesh->getHull().mCenterOfMass = origConvexMesh->getHull().mCenterOfMass;
			clonedConvexMesh->getHull().mAABB = origConvexMesh->getHull().mAABB;
			clonedConvexMesh->getHull().mInternal = origConvexMesh->getHull().mInternal;

			clonedGeom.convexMesh = static_cast<PxConvexMesh *>(clonedConvexMesh);

			break;
		}
	case PxGeometryType::eTRIANGLEMESH:
		{
			//TODO: clone triangleMesh pointer
			const PxTriangleMeshGeometry & origGeom = orig.triangleMesh();
			PxTriangleMeshGeometry & clonedGeom = clone.triangleMesh();

			Gu::TriangleMesh * origTriMesh = static_cast<Gu::TriangleMesh *>(origGeom.triangleMesh);

			const PxU32 alignment = 16;
			void * allocatedMem = stackAlloc.allocate(sizeof(ClonedTriangleMesh) + (alignment - 1));
			void * allignedMem = reinterpret_cast<void *>( reinterpret_cast<ptrdiff_t>(reinterpret_cast<PxU8 *>(allocatedMem) + alignment - 1) & ~(alignment - 1) );

			ClonedTriangleMesh * clonedTriMesh = new (allignedMem) ClonedTriangleMesh();

			//clone vertices
			clonedTriMesh->mMesh.mData.mNumVertices = origTriMesh->getNbVertices();
			
			PxU32 numBytes = origTriMesh->getNbVertices() * sizeof(PxVec3);
			clonedTriMesh->mMesh.mData.mVertices = (PxVec3*)stackAlloc.allocate(numBytes+4);	// PT: +4 to make sure we can safely V4Load the last vertex
			memcpy(clonedTriMesh->mMesh.mData.mVertices, (void *)origTriMesh->getVertices(), numBytes);
			
			//clone indices
			clonedTriMesh->mMesh.mData.mNumTriangles = origTriMesh->getNbTriangles();
			clonedTriMesh->mMesh.mData.mFlags = origTriMesh->getTriangleMeshFlags();
			bool bHas16bitIndices = clonedTriMesh->mMesh.mData.mFlags & PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES ? true : false;			

			if(bHas16bitIndices)
			{
				numBytes = origTriMesh->getNbTriangles() * 3 * sizeof(PxU16);
				clonedTriMesh->mMesh.mData.mTriangles = (PxU16 *)stackAlloc.allocate(numBytes);
			}
			else
			{
				numBytes = origTriMesh->getNbTriangles() * 3 * sizeof(PxU32);
				clonedTriMesh->mMesh.mData.mTriangles = (PxU32 *)stackAlloc.allocate(numBytes);
			}

			memcpy(clonedTriMesh->mMesh.mData.mTriangles, (void *) origTriMesh->getTriangles(), numBytes);
		
			//clone materials
			if(origTriMesh->mMesh.getMaterials())
			{
				numBytes = origTriMesh->getNbTriangles() * sizeof(PxU16);
				clonedTriMesh->mMaterials = (PxU16 *)stackAlloc.allocate(numBytes);
				memcpy(clonedTriMesh->mMaterials, (void *) origTriMesh->mMesh.getMaterials(), numBytes);		
			}
			else
			{
				clonedTriMesh->mMaterials = 0;
			}

			//clone remap
			if(origTriMesh->mMesh.getFaceRemap())
			{
				numBytes = origTriMesh->getNbTriangles() * sizeof(PxU32);
				clonedTriMesh->mRemap = (PxU32 *)stackAlloc.allocate(numBytes);
				memcpy(clonedTriMesh->mRemap, (void *) origTriMesh->mMesh.getFaceRemap(), numBytes);		
			}
			else
			{
				clonedTriMesh->mRemap = 0;
			}

			//clone extraTrigData
			if(origTriMesh->mMesh.mData.mExtraTrigData)
			{
				numBytes = origTriMesh->getNbTriangles() * sizeof(PxU8);
				clonedTriMesh->mMesh.mData.mExtraTrigData = (PxU8 *)stackAlloc.allocate(numBytes);
				memcpy(clonedTriMesh->mMesh.mData.mExtraTrigData, (void *) origTriMesh->mMesh.mData.mExtraTrigData, numBytes);		
			}
			else
			{
				clonedTriMesh->mMesh.mData.mExtraTrigData = 0;
			}

			clonedTriMesh->mMesh.mData.mAABB = origTriMesh->mMesh.mData.mAABB;

			clonedGeom.triangleMesh = static_cast<PxTriangleMesh *>(clonedTriMesh);

			break;
		}
	case PxGeometryType::eHEIGHTFIELD:
		{
			//TODO: clone heightField pointer
			const PxHeightFieldGeometry & origGeom = orig.heightField();
			PxHeightFieldGeometry & clonedGeom = clone.heightField();

			const PxU32 alignment = 16;
			void * allocatedMem = stackAlloc.allocate(sizeof(ClonedHeightField) + (alignment - 1));
			void * allignedMem = reinterpret_cast<void *>( reinterpret_cast<ptrdiff_t>(reinterpret_cast<PxU8 *>(allocatedMem) + alignment - 1) & ~(alignment - 1) );

			Gu::HeightField * origHeightField = static_cast<Gu::HeightField *>(origGeom.heightField);
			ClonedHeightField * clonedHeightField = new (allignedMem) ClonedHeightField();

			clonedHeightField->copyDataAndSamples(origHeightField, stackAlloc);

			clonedGeom.columnScale = origGeom.columnScale;
			clonedGeom.rowScale = origGeom.rowScale;
			clonedGeom.heightScale = origGeom.heightScale;
			
			break;
		}
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::eINVALID:
	default:
		PX_ASSERT(0);
	};	
}
//-----------------------------------------------------------------------------
void clonePhysXName(char * & cloned, const char * orig, StackAllocator & stackAlloc)
{
	if (!orig)
	{
		cloned = NULL;
		return;
	}

	char addition[] = "_GRBinterop";
	PxU32 origLen = (PxU32)strlen(orig);
	PxU32 addLen = (PxU32)strlen(addition);

	cloned = (char *)stackAlloc.allocate((origLen + addLen + 1) * sizeof(char));
	sprintf(cloned, "%s%s", orig, addition);
}
//-----------------------------------------------------------------------------

void clonePhysXMaterial(GrbInterop3::MaterialDesc & materialDesc, const PxMaterial * orig)
{
	materialDesc.ptr					= TO_GRB_INTEROP_PTR(orig);
	materialDesc.dynamicFriction		= orig->getDynamicFriction();
	materialDesc.frictionCombineMode	= orig->getFrictionCombineMode();
	materialDesc.restitution			= orig->getRestitution();
	materialDesc.restitutionCombineMode	= orig->getRestitutionCombineMode();
	materialDesc.staticFriction			= orig->getStaticFriction();
	materialDesc.flags					= orig->getFlags();
}

GrbInterop3::MaterialDesc * clonePhysXMaterials(PxU32 materialsCount, PxMaterial * const * origMaterials, StackAllocator & stackAlloc)
{
	GrbInterop3::MaterialDesc * materialDescs = (GrbInterop3::MaterialDesc *)stackAlloc.allocate(materialsCount * sizeof(GrbInterop3::MaterialDesc));

	for (PxU32 i = 0; i < materialsCount; ++i)
	{
		clonePhysXMaterial(materialDescs[i], origMaterials[i]);
	}

	return materialDescs;
}

void clonePhysxShape(GrbInterop3::ShapeDesc & shapeDesc, const PxShape * shape, StackAllocator & stackAlloc)
{
	shapeDesc.ptr				= TO_GRB_INTEROP_PTR(shape);
	shapeDesc.contactOffset		= shape->getContactOffset();
	shapeDesc.restOffset		= shape->getRestOffset();
	shapeDesc.localPose			= shape->getLocalPose();
	shapeDesc.flags				= shape->getFlags();
	shapeDesc.simFilterData		= shape->getSimulationFilterData();
	shapeDesc.queryFilterData	= shape->getQueryFilterData();
	shapeDesc.isExclusive		= shape->isExclusive();

	PxU32 numMaterials = shape->getNbMaterials();
	PxMaterial ** materials = (PxMaterial **)stackAlloc.allocate(numMaterials * sizeof(PxMaterial *));
	shape->getMaterials(materials, numMaterials);

	shapeDesc.materialsCount = numMaterials;
	shapeDesc.materialDescs = clonePhysXMaterials(numMaterials, materials, stackAlloc);
	clonePhysXGeometry(shapeDesc.geometry,shape->getGeometry(), stackAlloc);
}

GrbInterop3::ShapeDesc * clonePhysXRigidActorShapes(PxU32 shapesCount, const PxRigidActor * rigidActor, StackAllocator & stackAlloc)
{
	GrbInterop3::ShapeDesc * shapeDescs = (GrbInterop3::ShapeDesc *)stackAlloc.allocate(shapesCount * sizeof(GrbInterop3::ShapeDesc));

	PxShape * userBuf[5];
	for (PxU32 shapeCnt = 0; shapeCnt < shapesCount; shapeCnt += 5)
	{
		PxU32 numShapesRemaining = PxMin<PxU32>(5, shapesCount - shapeCnt);
		rigidActor->getShapes(userBuf, 5, shapeCnt);
		
		for (PxU32 procShapeCnt = 0; procShapeCnt < numShapesRemaining; ++procShapeCnt)
		{
			clonePhysxShape(shapeDescs[shapeCnt + procShapeCnt], userBuf[procShapeCnt], stackAlloc);
		}
	}

	return shapeDescs;
}

void clonePhysXRigidStatic(GrbInterop3::RigidStaticDesc & rigidStaticDesc, const PxRigidStatic * rigidStatic, StackAllocator & stackAlloc)
{
	rigidStaticDesc.type				= GRB_INTEROP_RIGID_STATIC_DESC;

	rigidStaticDesc.ptr					= TO_GRB_INTEROP_PTR(rigidStatic);
	rigidStaticDesc.globalPose			= rigidStatic->getGlobalPose();
	rigidStaticDesc.dominanceGroup		= rigidStatic->getDominanceGroup();
	rigidStaticDesc.actorFlags			= rigidStatic->getActorFlags();
	rigidStaticDesc.clientID			= rigidStatic->getOwnerClient();

	clonePhysXName(rigidStaticDesc.name, rigidStatic->getName(), stackAlloc);

	rigidStaticDesc.shapesCount = rigidStatic->getNbShapes();
	rigidStaticDesc.shapeDescs = clonePhysXRigidActorShapes(rigidStaticDesc.shapesCount, static_cast<const PxRigidActor *>(rigidStatic), stackAlloc);
}

void clonePhysXRigidDynamic(GrbInterop3::RigidDynamicDesc & rigidDynamicDesc, const PxRigidDynamic * rigidDynamic, StackAllocator & stackAlloc)
{
	rigidDynamicDesc.type					= GRB_INTEROP_RIGID_DYNAMIC_DESC;

	rigidDynamicDesc.ptr					= TO_GRB_INTEROP_PTR(rigidDynamic);
	rigidDynamicDesc.globalPose				= rigidDynamic->getGlobalPose();
	rigidDynamicDesc.dominanceGroup			= rigidDynamic->getDominanceGroup();
	rigidDynamicDesc.actorFlags				= rigidDynamic->getActorFlags();
	rigidDynamicDesc.clientID				= rigidDynamic->getOwnerClient();

	rigidDynamicDesc.linearVelocity			= rigidDynamic->getLinearVelocity();
	rigidDynamicDesc.angularVelocity		= rigidDynamic->getAngularVelocity();
	rigidDynamicDesc.linearDamping			= rigidDynamic->getLinearDamping();
	rigidDynamicDesc.angularDamping			= rigidDynamic->getAngularDamping();
	rigidDynamicDesc.rigidBodyFlags			= rigidDynamic->getRigidBodyFlags();
	rigidDynamicDesc.contactReportThreshold	= rigidDynamic->getContactReportThreshold();

	clonePhysXName(rigidDynamicDesc.name, rigidDynamic->getName(), stackAlloc);

	rigidDynamicDesc.shapesCount = rigidDynamic->getNbShapes();
	rigidDynamicDesc.shapeDescs = clonePhysXRigidActorShapes(rigidDynamicDesc.shapesCount, static_cast<const PxRigidActor *>(rigidDynamic), stackAlloc);
}
//-----------------------------------------------------------------------------

void releaseShapeDesc(GrbInterop3::ShapeDesc & shapeDesc)
{
	switch (shapeDesc.geometry.getType())
	{
	case PxGeometryType::eSPHERE:
		{
			break;
		}
	case PxGeometryType::ePLANE:
		{
			break;
		}
	case PxGeometryType::eBOX:
		{
			break;
		}
	case PxGeometryType::eCAPSULE:
		{
			break;
		}

		/*
			avoroshilov: add special releasers for these three (internal arrays freeing)
		*/

	case PxGeometryType::eCONVEXMESH:
		{
			break;
		}
	case PxGeometryType::eTRIANGLEMESH:
		{
			break;
		}
	case PxGeometryType::eHEIGHTFIELD:
		{
			break;
		}
	case PxGeometryType::eGEOMETRY_COUNT:
	case PxGeometryType::eINVALID:
	default:
		break;
	}
}



void SceneFetchResultsEvent::set(PxScene & scene)
{
	// avoroshilov:
	// there is a bug in physx (reported and in the process of fixing already) which duplicates entries
	// when getActors(eDYNAMIC) performed, hence for now workaround should be applied
	// TODO avoroshilov: fix this after merging PhysX bugfix

	const PxU32 numRigidActors = scene.getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);	// Since this is called at the end of fetchResults(), this should not change
	PxActor * userBuf[5];
	for (PxU32 actorCnt = 0; actorCnt < numRigidActors; actorCnt += 5)
	{
		PxU32 numActorsRemaining = PxMin<PxU32>(5, numRigidActors - actorCnt);
		scene.getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, userBuf, 5, actorCnt);
		
		for (PxU32 procActorCnt = 0; procActorCnt < numActorsRemaining; ++procActorCnt)
		{
			PxRigidDynamic * curActor = userBuf[procActorCnt]->isRigidDynamic();
			if (curActor)
			{
				ActorUpdateData & updateData = updates.insert();
				updateData.pose =	curActor->getGlobalPose();
				updateData.linVel =	curActor->getLinearVelocity();
				updateData.angVel =	curActor->getAngularVelocity();
				updateData.actor =	TO_GRB_INTEROP_PTR(curActor);
			}
		}
	}
}
}

using namespace GrbInterop3;

#define SCENE_EVENT_IMPL(...) \
	GrbInteropEvent3::GrbInteropEvent3(StackAllocator &sa, Type t, __VA_ARGS__) : mStackAlloc(sa), type(t)
// GCC wants the initializer list to be in order. Added this macro to support initializing "object".
#define SCENE_EVENT_IMPL_OBJ(Obj, ...) \
	GrbInteropEvent3::GrbInteropEvent3(StackAllocator &sa, Type t, __VA_ARGS__) : mStackAlloc(sa), object(Obj), type(t)


SCENE_EVENT_IMPL(const PxVec3 & vec)											{ new (&vec3()) PxVec3(vec); }
SCENE_EVENT_IMPL(const PxRigidStatic * rigidStatic)								{ arg.sceneAddRigidStaticEventPtr = new(sa.allocate(sizeof(SceneAddRigidStaticEvent))) SceneAddRigidStaticEvent(rigidStatic, sa); }
SCENE_EVENT_IMPL(const PxRigidDynamic * rigidDynamic)							{ arg.sceneAddRigidDynamicEventPtr = new(sa.allocate(sizeof(SceneAddRigidDynamicEvent))) SceneAddRigidDynamicEvent(rigidDynamic, sa); }
SCENE_EVENT_IMPL(PxActor *const* actors, PxU32 actorsCount)						{ arg.sceneAddRigidActorsEventPtr = new(sa.allocate(sizeof(SceneAddRigidActorsEvent))) SceneAddRigidActorsEvent(actors, actorsCount, sa); }
SCENE_EVENT_IMPL(const PxAggregate * aggregate)									{ arg.sceneAddAggregateEventPtr = new(sa.allocate(sizeof(SceneAddAggregateEvent))) SceneAddAggregateEvent(aggregate, sa); }
SCENE_EVENT_IMPL(const PxAggregate * aggregate, PxU32)							{ arg.aggregatePtr = TO_GRB_INTEROP_PTR(aggregate); }
SCENE_EVENT_IMPL(const PxBroadPhaseRegion * bpRegion, bool populateRegion)		{ arg.sceneAddBroadphaseRegionEventPtr = new(sa.allocate(sizeof(SceneAddBroadphaseRegionEvent))) SceneAddBroadphaseRegionEvent(bpRegion, populateRegion, sa); }
SCENE_EVENT_IMPL(const PxMaterial * material)									{ arg.sceneUpdateMaterialEventPtr = new(sa.allocate(sizeof(SceneUpdateMaterialEvent))) SceneUpdateMaterialEvent(material, sa); }
SCENE_EVENT_IMPL(const PxMaterial * material, PxU32)							{ arg.materialPtr = TO_GRB_INTEROP_PTR(material); }
SCENE_EVENT_IMPL(const PxActor * actor)											{ arg.actorPtr = TO_GRB_INTEROP_PTR(actor); }
SCENE_EVENT_IMPL(PoolSet & pools, PxScene & scene)								{ (arg.sceneFetchResultsEventPtr = pools.sceneFetchResultsEventPool.getEvent())->set( scene ); }
SCENE_EVENT_IMPL_OBJ(scene, PxScene * scene)									{ }
SCENE_EVENT_IMPL(PxU32 pxu32)													{ arg.pxu32 = pxu32; }

	// PxActor
SCENE_EVENT_IMPL_OBJ(actor, const PxActor* actor, const PxShape* shape, PxU32 creationFlag)
{
	switch (creationFlag)
	{
	case 1:
		{
			arg.actorCreateShapeEventPtr = new(sa.allocate(sizeof(ActorCreateShapeEvent))) ActorCreateShapeEvent(actor, shape, sa);
			break;
		}
	case 2:
		{
			arg.actorAttachShapeEventPtr = new(sa.allocate(sizeof(ActorAttachShapeEvent))) ActorAttachShapeEvent(actor, shape, sa);
			break;
		}
	}
}

SCENE_EVENT_IMPL_OBJ(actor, const PxActor* actor, const PxShape * shape) 							{ arg.shapePtr = TO_GRB_INTEROP_PTR(shape); }
SCENE_EVENT_IMPL_OBJ(actor, const PxActor* actor, PxU8 pxu8)										{ arg.pxu8 = pxu8; }
SCENE_EVENT_IMPL_OBJ(actor, const PxActor* actor, PxU16 pxu16)										{ arg.pxu16 = pxu16; }
SCENE_EVENT_IMPL_OBJ(actor, const PxActor* actor, PxU32 pxu32)										{ arg.pxu32 = pxu32; }
SCENE_EVENT_IMPL_OBJ(actor, const PxActor* actor, PxReal real)										{ arg.real = real; }
SCENE_EVENT_IMPL_OBJ(actor, const PxActor* actor, const char * name)								{ clonePhysXName(arg.name, name, sa); }

	// PxShape
SCENE_EVENT_IMPL_OBJ(shape, const PxShape* shape, const PxGeometry& /*geom*/)						{ arg.shapeSetGeometryEventPtr = new(sa.allocate(sizeof(ShapeSetGeometryEvent))) ShapeSetGeometryEvent(shape, sa); }
SCENE_EVENT_IMPL_OBJ(shape, const PxShape* shape, PxMaterial*const* materials, PxU32 materialCount)	{ arg.setMaterialsPair.materialsCount = materialCount; arg.setMaterialsPair.materials = clonePhysXMaterials(materialCount, materials, sa); }
SCENE_EVENT_IMPL_OBJ(shape, const PxShape* shape, PxU8 pxu8)										{ arg.pxu8 = pxu8; }
SCENE_EVENT_IMPL_OBJ(shape, const PxShape* shape, PxU16 pxu16)										{ arg.pxu16 = pxu16; }
SCENE_EVENT_IMPL_OBJ(shape, const PxShape* shape, PxU32 pxu32)										{ arg.pxu32 = pxu32; }
SCENE_EVENT_IMPL_OBJ(shape, const PxShape* shape, PxReal real)										{ arg.real = real; }
SCENE_EVENT_IMPL_OBJ(shape, const PxShape* shape, const PxTransform & trans)						{ transform() = trans; }
SCENE_EVENT_IMPL_OBJ(shape, const PxShape* shape, const PxFilterData & simFilterData)				{ filterData() = simFilterData; }
SCENE_EVENT_IMPL_OBJ(shape, const PxShape* shape, const char * name)								{ clonePhysXName(arg.name, name, sa); }

	// PxAggregate
SCENE_EVENT_IMPL_OBJ(aggregate, const PxAggregate * aggregate, PxActor * actor)						{ arg.actorPtr = TO_GRB_INTEROP_PTR(actor); }


PxU32 grbGetNbEventStreams(PxScene& scene)
{
	return static_cast<NpScene&>(scene).getNumEventStreams();
}

StackAllocator&	grbGetEventStreamAlloc(PxScene& scene, PxU32 streamIndex)
{
	return static_cast<NpScene&>(scene).getEventStreamStackAlloc(streamIndex);
}

void grbSendEvent(PxScene& scene, const GrbInteropEvent3& e, PxU32 streamIndex)
{
	static_cast<NpScene&>(scene).eventStreamSend(e, streamIndex);
}



}
#endif
