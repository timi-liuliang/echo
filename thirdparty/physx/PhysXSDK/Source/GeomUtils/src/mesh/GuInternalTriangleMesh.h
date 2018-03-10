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

#ifndef GU_INTERNAL_TRIANGLE_MESH_H
#define GU_INTERNAL_TRIANGLE_MESH_H

#include "PxSimpleTriangleMesh.h"
#include "PxTriangleMesh.h"
#include "GuTriangleMeshData.h"
#include "GuMeshInterface.h"

namespace physx
{
	class PxInputStream;

#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4251 ) // class needs to have dll-interface to be used by clients of class
#endif
	// AP: It looks like this class doesn't need to exist should be merged with GuTriangleMesh
	class PX_PHYSX_COMMON_API InternalTriangleMesh
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
// PX_SERIALIZATION
												InternalTriangleMesh(const PxEMPTY&)	: mData(PxEmpty), mOwnsMemory(0)	{}

					void						exportExtraData(PxSerializationContext&);
					void						importExtraData(PxDeserializationContext&);
		static		void						getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
												InternalTriangleMesh();
												~InternalTriangleMesh();

					void						release();

					PxVec3*						allocateVertices(PxU32 nbVertices);
					void*						allocateTriangles(PxU32 nbTriangles, bool force32Bit = false);
					PxU16*						allocateMaterials();
					PxU32*						allocateFaceRemap();
					PxU32*						allocateAdjacencies();
	PX_FORCE_INLINE	bool						has16BitIndices()			const	{ return mData.mFlags & PxTriangleMeshFlag::eHAS_16BIT_TRIANGLE_INDICES ? true:false;	}	//this does not apply for mesh processing during cooking, there its always 32 bits.
	PX_FORCE_INLINE PxTriangleMeshFlags			getTriangleMeshFlags()		const	{ return PxTriangleMeshFlags(mData.mFlags); }
	PX_FORCE_INLINE	PxU32						getNumVertices()			const	{ return mData.mNumVertices;	}
	PX_FORCE_INLINE	PxU32						getNumTriangles()			const	{ return mData.mNumTriangles;	}
	PX_FORCE_INLINE	const PxVec3*				getVertices()				const	{ return mData.mVertices;		}
	PX_FORCE_INLINE	const void*					getTriangles()				const	{ return mData.mTriangles;		}
	PX_FORCE_INLINE	const PxU16*				getMaterials()				const	{ return mMaterialIndices;		}
	PX_FORCE_INLINE	const PxU32*				getFaceRemap()				const	{ return mFaceRemap;			}
	PX_FORCE_INLINE	const PxU32*				getAdjacencies()			const	{ return mAdjacencies;			}

	PX_FORCE_INLINE	const Gu::RTreeMidphase&	getCollisionModel()			const	{ return mData.mCollisionModel;	}

	// Data for convex-vs-arbitrary-mesh
	PX_FORCE_INLINE	PxReal						getConvexEdgeThreshold()	const	{ return mConvexEdgeThreshold;	}
	PX_FORCE_INLINE	PxU32						getTrigSharedEdgeFlagsFromData(PxU32 trigIndex)const;
	PX_FORCE_INLINE	void						setTrigSharedEdgeFlag(PxU32 trigIndex, PxU32 edgeIndex);

	// Data for adjacencies
	PX_FORCE_INLINE	void						setTriangleAdjacency(PxU32 triangleIndex, PxU32 adjacency, PxU32 offset);

					bool						loadRTree(PxInputStream& modelData, const PxU32 meshVersion);
					void						setupMeshInterface();
					const Gu::MeshInterface&	getMeshInterface() const { return mMeshInterface; }

				Gu::InternalTriangleMeshData	mData;
	protected:
					//@@NOT SEPARATED
					PxU16*						mMaterialIndices;		//!< the size of the array is numTriangles.
					//@@NOT SEPARATED
					PxU32*						mFaceRemap;				//!< new faces to old faces mapping (after cleaning, etc). Usage: old = faceRemap[new]

					PxU32*						mAdjacencies;			//!< Adjacency information for each face - 3 adjacent faces
																		//!< Set to 0xFFFFffff if no adjacent face

					PxU32						mNumAdjacencies;        //!< only needed because of serialization

public:	// only public for serial

					PxReal						mConvexEdgeThreshold;
protected:
					Gu::MeshInterface			mMeshInterface;
					PxU32						mOwnsMemory;	// PT: this should be packed as a bit with mData.m16BitIndices...

			friend class InternalTriangleMeshBuilder;
	};
#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

PX_FORCE_INLINE PxU32 InternalTriangleMesh::getTrigSharedEdgeFlagsFromData(PxU32 trigIndex) const
{
	PX_ASSERT(mData.mExtraTrigData);
	return mData.mExtraTrigData[trigIndex];
}

PX_FORCE_INLINE void InternalTriangleMesh::setTrigSharedEdgeFlag(PxU32 trigIndex, PxU32 edgeIndex)
{
		mData.mExtraTrigData[trigIndex] |= (1<<edgeIndex);
}

PX_FORCE_INLINE void InternalTriangleMesh::setTriangleAdjacency(PxU32 triangleIndex, PxU32 adjacency, PxU32 offset)
{
	PX_ASSERT(mAdjacencies);
	mAdjacencies[triangleIndex*3 + offset] = adjacency;
}
}

#endif
