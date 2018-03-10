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


#ifndef PX_COOKINGUTILS
#define PX_COOKINGUTILS

#include "PxPhysXConfig.h"
#include "PxVec3.h"

namespace physx
{
	//! Vertex cloud reduction result structure
	struct REDUCEDCLOUD
	{
				// Out
				PxVec3*				RVerts;		//!< Reduced list
				PxU32				NbRVerts;	//!< Reduced number of vertices
				PxU32*				CrossRef;	//!< nb_verts remapped indices
	};

	class ReducedVertexCloud
	{
		public:
		// Constructors/destructor
									ReducedVertexCloud(const PxVec3* verts, PxU32 nb_verts);
									~ReducedVertexCloud();
		// Free used bytes
				ReducedVertexCloud&	Clean();
		// Cloud reduction
				bool				Reduce(REDUCEDCLOUD* rc=NULL);
		// Data access
		PX_INLINE	PxU32				GetNbVerts()				const	{ return mNbVerts;		}
		PX_INLINE	PxU32				GetNbReducedVerts()			const	{ return mNbRVerts;		}
		PX_INLINE	const PxVec3*		GetReducedVerts()			const	{ return mRVerts;		}
		PX_INLINE	const PxVec3&		GetReducedVertex(PxU32 i)	const	{ return mRVerts[i];	}
		PX_INLINE	const PxU32*		GetCrossRefTable()			const	{ return mXRef;			}

		private:
		// Original vertex cloud
				PxU32				mNbVerts;	//!< Number of vertices
				const PxVec3*		mVerts;		//!< List of vertices (pointer copy)

		// Reduced vertex cloud
				PxU32				mNbRVerts;	//!< Reduced number of vertices
				PxVec3*				mRVerts;	//!< Reduced list of vertices
				PxU32*				mXRef;		//!< Cross-reference table (used to remap topologies)
	};

}

#endif
	
