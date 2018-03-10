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


#ifndef PX_EDGELIST
#define PX_EDGELIST

// PT: this file should be moved to cooking lib

#include "Px.h"
#include "PsUserAllocated.h"

// Data/code shared with LL
#include "GuEdgeListData.h"

namespace physx
{
class PxInputStream;

#define ICE_EDGELIST_V22

namespace Gu
{
//	enum EdgeType
//	{
//		EDGE_UNDEFINED,
//
//		EDGE_BOUNDARY,		//!< Edge belongs to a single triangle
//		EDGE_INTERNAL,		//!< Edge belongs to exactly two triangles
//		EDGE_SINGULAR,		//!< Edge belongs to three or more triangles
//
//		EDGE_FORCE_DWORD	= 0x7fffffff
//	};
//
//	enum EdgeFlag
//	{
//		EDGE_ACTIVE	= (1<<0)
//	};
/*
	class Edge8 : public Ice::Allocateable
	{
		public:
		//! Constructor
		PX_INLINE				Edge8()		{}
		//! Destructor
		PX_INLINE				~Edge8()	{}

				PxU8		mRef0;		//!< First vertex reference
				PxU8		mRef1;		//!< Second vertex reference
	};
	PX_COMPILE_TIME_ASSERT(sizeof(Edge8)==2);
*/

	//! The edge-list creation structure.
	struct EDGELISTCREATE
	{
		//! Constructor
								EDGELISTCREATE()	: NbFaces(0), DFaces(NULL), WFaces(NULL), FacesToEdges(false), EdgesToFaces(false)
#ifdef ICE_EDGELIST_V22
								,Verts(NULL),
								Epsilon(0.1f)
#endif
								{}
				
				PxU32			NbFaces;		//!< Number of faces in source topo
				const PxU32*	DFaces;			//!< List of faces (dwords) or NULL
				const PxU16*	WFaces;			//!< List of faces (words) or NULL

				bool			FacesToEdges;
				bool			EdgesToFaces;
#ifdef ICE_EDGELIST_V22
				const PxVec3*	Verts;
				float			Epsilon;
#endif
	};

//	#define MSH_EDGE_LINK_MASK		0x3fffffff
//	#define MSH_ACTIVE_EDGE_MASK	0x80000000
//	#define MSH_ACTIVE_VERTEX_MASK	0x40000000
//	#define MSH_EDGE_LINK_MASK		0x0fffffff
//	#define MSH_ACTIVE_EDGE_MASK	0x80000000
//	#define MSH_ACTIVE_VERTEX_MASK	0x40000000
//	#define MSH_ACTIVE_USER_BIT0	0x20000000
//	#define MSH_ACTIVE_USER_BIT1	0x10000000


	class EdgeList : public Ps::UserAllocated
	{
		public:
										EdgeList();
										~EdgeList();

				bool					Load(PxInputStream& stream);
		// Data access
		PX_INLINE	PxU32					GetNbEdges()							const	{ return mData.mNbEdges;						}
		PX_INLINE	const Gu::EdgeData*		GetEdges()								const	{ return mData.mEdges;						}
		PX_INLINE	const Gu::EdgeData&		GetEdge(PxU32 edge_index)				const	{ return mData.mEdges[edge_index];			}
		//
		PX_INLINE	PxU32					GetNbFaces()							const	{ return mData.mNbFaces;						}
		PX_INLINE	const Gu::EdgeTriangleData* GetEdgeTriangles()					const	{ return mData.mEdgeFaces;					}
		PX_INLINE	const Gu::EdgeTriangleData& GetEdgeTriangle(PxU32 face_index)		const	{ return mData.mEdgeFaces[face_index];		}
		//
		PX_INLINE	const Gu::EdgeDescData*	GetEdgeToTriangles()					const	{ return mData.mEdgeToTriangles;				}
		PX_INLINE	const Gu::EdgeDescData&	GetEdgeToTriangles(PxU32 edge_index)	const	{ return mData.mEdgeToTriangles[edge_index];	}
		PX_INLINE	const PxU32*			GetFacesByEdges()						const	{ return mData.mFacesByEdges;					}
		PX_INLINE	PxU32					GetFacesByEdges(PxU32 face_index)		const	{ return mData.mFacesByEdges[face_index];		}

//		PX_INLINE Gu::EdgeListData*			GetEdgeListData()								{ return &mData; }

		protected:
				Gu::EdgeListData			mData;					//!< Pointer to edgelist data
//		// The edge list
//				PxU32					mNbEdges;				//!< Number of edges in the list
//				Gu::EdgeData*				mEdges;					//!< List of edges
//		// Faces to edges
//				PxU32					mNbFaces;				//!< Number of faces for which we have data
//				Gu::EdgeTriangleData*		mEdgeFaces;				//!< Array of edge-triangles referencing mEdges
//		// Edges to faces
//				Gu::EdgeDescData*			mEdgeToTriangles;		//!< An EdgeDesc structure for each edge
//				PxU32*					mFacesByEdges;			//!< A pool of face indices
		// Internal methods
/*				bool					CreateFacesToEdges(PxU32 nb_faces, const PxU32* dfaces, const PxU16* wfaces);
				bool					CreateEdgesToFaces(PxU32 nb_faces, const PxU32* dfaces, const PxU16* wfaces);
#ifdef ICE_EDGELIST_V22
				bool					ComputeActiveEdges(PxU32 nb_faces, const PxU32* dfaces, const PxU16* wfaces, const Point* verts, float epsilon);
#endif*/
	};


	class EdgeListBuilder : public EdgeList
	{
		public:
										EdgeListBuilder();
										~EdgeListBuilder();

				bool					Init(const EDGELISTCREATE& create);
//				bool					Save(Stream& stream)					const;
		private:
		// Internal methods
				bool					CreateFacesToEdges(PxU32 nb_faces, const PxU32* dfaces, const PxU16* wfaces);
				bool					CreateEdgesToFaces(PxU32 nb_faces, const PxU32* dfaces, const PxU16* wfaces);
#ifdef ICE_EDGELIST_V22
				bool					ComputeActiveEdges(PxU32 nb_faces, const PxU32* dfaces, const PxU16* wfaces, const PxVec3* verts, float epsilon);
#endif
	};

#ifndef ICE_EDGELIST_V22
	class ActiveEdges
	{
		public:
									ActiveEdges();
									~ActiveEdges();

				bool				Compute(const EdgeList& edges, const IndexedTriangle* faces, const PxVec3* verts, float epsilon=0.001f);

		// Data access
		PX_INLINE	const bool*			GetActiveEdges()						const	{ return mActiveEdges;				}
		PX_INLINE	bool				GetActiveEdge(PxU32 edge_index)		const	{ return mActiveEdges[edge_index];	}

		private:
				bool*				mActiveEdges;			//!< mNbEdges bools marking active edges
	};
#endif
	
}

}

#endif
