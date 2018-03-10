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


#include "PxMemory.h"
#include "EdgeList.h"
#include "PxTriangle.h"
#include "PsMathUtils.h"
#include "GuRevisitedRadixBuffered.h"
#include "GuSerialize.h"

using namespace physx;
using namespace Gu;

Gu::EdgeList::EdgeList()
{
	mData.mNbEdges = 0;
	mData.mEdgeFaces = NULL;
	mData.mEdges = NULL;
	mData.mEdgeToTriangles = NULL;
	mData.mFacesByEdges = NULL;
}

Gu::EdgeList::~EdgeList()
{
	PX_FREE_AND_RESET(mData.mFacesByEdges);
	PX_FREE_AND_RESET(mData.mEdgeToTriangles);
	PX_FREE_AND_RESET(mData.mEdges);
	PX_DELETE_POD(mData.mEdgeFaces);
}

bool Gu::EdgeList::Load(PxInputStream& stream)
{
	// Import header
	PxU32 Version;
	bool Mismatch;
	if(!ReadHeader('E', 'D', 'G', 'E', Version, Mismatch, stream))
		return false;

	// Import edges
	mData.mNbEdges = readDword(Mismatch, stream);
	//mEdges = ICE_NEW_MEM(Edge[mNbEdges],Edge);
	mData.mEdges = (Gu::EdgeData*)PX_ALLOC(sizeof(Gu::EdgeData)*mData.mNbEdges, PX_DEBUG_EXP("Gu::EdgeData"));
	stream.read(mData.mEdges, sizeof(Gu::EdgeData)*mData.mNbEdges);

	mData.mNbFaces = readDword(Mismatch, stream);
	//mEdgeFaces	= ICE_NEW_MEM(EdgeTriangle[mNbFaces],EdgeTriangle);
	mData.mEdgeFaces = (Gu::EdgeTriangleData*)PX_ALLOC(sizeof(Gu::EdgeTriangleData)*mData.mNbFaces, PX_DEBUG_EXP("Gu::EdgeTriangleData"));
	stream.read(mData.mEdgeFaces, sizeof(Gu::EdgeTriangleData)*mData.mNbFaces);

	//mEdgeToTriangles = ICE_NEW_MEM(EdgeDesc[mNbEdges],EdgeDesc);
	mData.mEdgeToTriangles = (Gu::EdgeDescData*)PX_ALLOC(sizeof(Gu::EdgeDescData)*mData.mNbEdges, PX_DEBUG_EXP("Gu::EdgeDescData"));
	stream.read(mData.mEdgeToTriangles, sizeof(Gu::EdgeDescData)*mData.mNbEdges);
	

	PxU32 LastOffset = mData.mEdgeToTriangles[mData.mNbEdges-1].Offset + mData.mEdgeToTriangles[mData.mNbEdges-1].Count;
	mData.mFacesByEdges = (PxU32*)PX_ALLOC(sizeof(PxU32)*LastOffset, PX_DEBUG_EXP("Gu::EdgeList FacesByEdges"));
	stream.read(mData.mFacesByEdges, sizeof(PxU32)*LastOffset);

	return true;
}

//#ifdef PX_COOKING

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Initializes the edge-list.
 *	\param		create	[in] edge-list creation structure
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Gu::EdgeListBuilder::Init(const EDGELISTCREATE& create)
{
	bool FacesToEdges = create.Verts ? true : create.FacesToEdges;
	bool EdgesToFaces = create.Verts ? true : create.EdgesToFaces;

	// "FacesToEdges" maps each face to three edges.
	if(FacesToEdges && !CreateFacesToEdges(create.NbFaces, create.DFaces, create.WFaces))	return false;

	// "EdgesToFaces" maps each edge to the set of faces sharing this edge
	if(EdgesToFaces && !CreateEdgesToFaces(create.NbFaces, create.DFaces, create.WFaces))	return false;

	// Create active edges
	if(create.Verts && !ComputeActiveEdges(create.NbFaces, create.DFaces, create.WFaces, create.Verts, create.Epsilon))	return false;

	// Get rid of useless data
	if(!create.FacesToEdges)	
	{ 
		PX_FREE_AND_RESET(mData.mEdgeFaces); 
	}
	if(!create.EdgesToFaces)	
	{ 
		PX_FREE_AND_RESET(mData.mEdgeToTriangles); 
		PX_FREE_AND_RESET(mData.mFacesByEdges);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes FacesToEdges.
 *	After the call:
 *	- mNbEdges		is updated with the number of non-redundant edges
 *	- mEdges		is a list of mNbEdges edges (one edge is 2 vertex-references)
 *	- mEdgesRef		is a list of nbfaces structures with 3 indexes in mEdges for each face
 *
 *	\param		nb_faces	[in] a number of triangles
 *	\param		dfaces		[in] list of triangles with PxU32 vertex references (or NULL)
 *	\param		wfaces		[in] list of triangles with PxU16 vertex references (or NULL)
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Gu::EdgeListBuilder::CreateFacesToEdges(PxU32 nb_faces, const PxU32* dfaces, const PxU16* wfaces)
{
	// Checkings
	if(!nb_faces || (!dfaces && !wfaces))
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "EdgeList::CreateFacesToEdges: NULL parameter!");
		return false;
	}

	if(mData.mEdgeFaces)	return true;	// Already computed!

	// 1) Get some bytes: I need one EdgesRefs for each face, and some temp buffers
	mData.mEdgeFaces			= PX_NEW(Gu::EdgeTriangleData)[nb_faces];	// Link faces to edges
	PxU32*			VRefs0		= PX_NEW_TEMP(PxU32)[nb_faces*3];			// Temp storage
	PxU32*			VRefs1		= PX_NEW_TEMP(PxU32)[nb_faces*3];			// Temp storage
	Gu::EdgeData*	Buffer		= PX_NEW_TEMP(Gu::EdgeData)[nb_faces*3];	// Temp storage

	// 2) Create a full redundant list of 3 edges / face.
	for(PxU32 i=0;i<nb_faces;i++)
	{
		// Get right vertex-references
		const PxU32 Ref0 = dfaces ? dfaces[i*3+0] : wfaces ? wfaces[i*3+0] : 0;
		const PxU32 Ref1 = dfaces ? dfaces[i*3+1] : wfaces ? wfaces[i*3+1] : 1;
		const PxU32 Ref2 = dfaces ? dfaces[i*3+2] : wfaces ? wfaces[i*3+2] : 2;

		// Pre-Sort vertex-references and put them in the lists
		if(Ref0<Ref1)	{ VRefs0[i*3+0] = Ref0; VRefs1[i*3+0] = Ref1; }		// Edge 0-1 maps (i%3)
		else			{ VRefs0[i*3+0] = Ref1; VRefs1[i*3+0] = Ref0; }		// Edge 0-1 maps (i%3)

		if(Ref1<Ref2)	{ VRefs0[i*3+1] = Ref1; VRefs1[i*3+1] = Ref2; }		// Edge 1-2 maps (i%3)+1
		else			{ VRefs0[i*3+1] = Ref2; VRefs1[i*3+1] = Ref1; }		// Edge 1-2 maps (i%3)+1

		if(Ref2<Ref0)	{ VRefs0[i*3+2] = Ref2; VRefs1[i*3+2] = Ref0; }		// Edge 2-0 maps (i%3)+2
		else			{ VRefs0[i*3+2] = Ref0; VRefs1[i*3+2] = Ref2; }		// Edge 2-0 maps (i%3)+2
	}

	// 3) Sort the list according to both keys (VRefs0 and VRefs1)
	RadixSortBuffered Sorter;
	const PxU32* Sorted = Sorter.Sort(VRefs1, nb_faces*3).Sort(VRefs0, nb_faces*3).GetRanks();

	// 4) Loop through all possible edges
	// - clean edges list by removing redundant edges
	// - create EdgesRef list
	mData.mNbEdges = 0;												// #non-redundant edges
	mData.mNbFaces = nb_faces;
	PxU32 PreviousRef0 = PX_INVALID_U32;
	PxU32 PreviousRef1 = PX_INVALID_U32;
	for(PxU32 i=0;i<nb_faces*3;i++)
	{
		PxU32 Face = Sorted[i];								// Between 0 and nbfaces*3
		PxU32 ID = Face % 3;									// Get edge ID back.
		PxU32 SortedRef0 = VRefs0[Face];						// (SortedRef0, SortedRef1) is the sorted edge
		PxU32 SortedRef1 = VRefs1[Face];

		if(SortedRef0!=PreviousRef0 || SortedRef1!=PreviousRef1)
		{
			// New edge found! => stored in temp buffer
			Buffer[mData.mNbEdges].Ref0	= SortedRef0;
			Buffer[mData.mNbEdges].Ref1	= SortedRef1;
			mData.mNbEdges++;
		}
		PreviousRef0 = SortedRef0;
		PreviousRef1 = SortedRef1;

		// Create mEdgesRef on the fly
		mData.mEdgeFaces[Face/3].mLink[ID] = mData.mNbEdges-1;
	}

	// 5) Here, mNbEdges==#non redundant edges
	mData.mEdges = (Gu::EdgeData*)PX_ALLOC(sizeof(Gu::EdgeData)*mData.mNbEdges, PX_DEBUG_EXP("Gu::EdgeData"));

	// Create real edges-list.
	PxMemCopy(mData.mEdges, Buffer, mData.mNbEdges*sizeof(Gu::EdgeData));

	// 6) Free ram and exit
	PX_DELETE_POD(Buffer);
	PX_DELETE_POD(VRefs1);
	PX_DELETE_POD(VRefs0);

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes EdgesToFaces.
 *	After the call:
 *	- mEdgeToTriangles		is created
 *	- mFacesByEdges			is created
 *
 *	\param		nb_faces	[in] a number of triangles
 *	\param		dfaces		[in] list of triangles with PxU32 vertex references (or NULL)
 *	\param		wfaces		[in] list of triangles with PxU16 vertex references (or NULL)
 *	\return		true if success.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool Gu::EdgeListBuilder::CreateEdgesToFaces(PxU32 nb_faces, const PxU32* dfaces, const PxU16* wfaces)
{
	// 1) I need FacesToEdges !
	if(!CreateFacesToEdges(nb_faces, dfaces, wfaces)) return false;

	// 2) Get some bytes: one Pair structure / edge
	mData.mEdgeToTriangles = (Gu::EdgeDescData*)PX_ALLOC(sizeof(Gu::EdgeDescData)*mData.mNbEdges, PX_DEBUG_EXP("Gu::EdgeDescData"));
	PxMemZero(mData.mEdgeToTriangles, sizeof(Gu::EdgeDescData)*mData.mNbEdges);

	// 3) Create Counters, ie compute the #faces sharing each edge
	for(PxU32 i=0;i<nb_faces;i++)
	{
		mData.mEdgeToTriangles[mData.mEdgeFaces[i].mLink[0]].Count++;
		mData.mEdgeToTriangles[mData.mEdgeFaces[i].mLink[1]].Count++;
		mData.mEdgeToTriangles[mData.mEdgeFaces[i].mLink[2]].Count++;
	}

	// 3) Create Radix-like Offsets
	mData.mEdgeToTriangles[0].Offset=0;
	for(PxU32 i=1;i<mData.mNbEdges;i++)	
		mData.mEdgeToTriangles[i].Offset = mData.mEdgeToTriangles[i-1].Offset + mData.mEdgeToTriangles[i-1].Count;

	PxU32 LastOffset = mData.mEdgeToTriangles[mData.mNbEdges-1].Offset + mData.mEdgeToTriangles[mData.mNbEdges-1].Count;

	// 4) Get some bytes for mFacesByEdges. LastOffset is the number of indices needed.
	mData.mFacesByEdges = (PxU32*)PX_ALLOC(sizeof(PxU32)*LastOffset, PX_DEBUG_EXP("Gu::EdgeListBuilder FacesByEdges"));

	// 5) Create mFacesByEdges
	for(PxU32 i=0;i<nb_faces;i++)
	{
		mData.mFacesByEdges[mData.mEdgeToTriangles[mData.mEdgeFaces[i].mLink[0]].Offset++] = i;
		mData.mFacesByEdges[mData.mEdgeToTriangles[mData.mEdgeFaces[i].mLink[1]].Offset++] = i;
		mData.mFacesByEdges[mData.mEdgeToTriangles[mData.mEdgeFaces[i].mLink[2]].Offset++] = i;
	}

	// 6) Recompute offsets wasted by 5)
	mData.mEdgeToTriangles[0].Offset=0;
	for(PxU32 i=1;i<mData.mNbEdges;i++)
	{
		mData.mEdgeToTriangles[i].Offset = mData.mEdgeToTriangles[i-1].Offset + mData.mEdgeToTriangles[i-1].Count;
	}

	return true;
}

PX_INLINE PxU32 OppositeVertex(PxU32 r0, PxU32 r1, PxU32 r2, PxU32 vref0, PxU32 vref1)
{
	/*fix TTP 8059,	the followed codes will make a wrong result when compile with \O2  on XBOX360 
	for example,  input 1,0,3,1,3  return 0 not 1.   maybe xbox complier bug.
			if(r0==vref0 && r1==vref1)	return r2;
	else	if(r0==vref1 && r1==vref0)	return r2;
	else	if(r0==vref0 && r2==vref1)	return r1;
	else	if(r0==vref1 && r2==vref0)	return r1;
	else	if(r1==vref0 && r2==vref1)	return r0;
	else	if(r1==vref1 && r2==vref0)	return r0;
	return PX_INVALID_U32;*/

	if(vref0==r0)
	{
	  if (vref1==r1) return r2;
	  else if(vref1==r2) return r1;
	}
	else if(vref0==r1)
	{
	  if (vref1==r0) return r2;
	  else if(vref1==r2) return r0;
	}
	else if(vref0==r2)
	{
	  if (vref1==r1) return r0;
	  else if(vref1==r0) return r1;
	}
	return PX_INVALID_U32;
}

bool Gu::EdgeListBuilder::ComputeActiveEdges(PxU32 nb_faces, const PxU32* dfaces, const PxU16* wfaces, const PxVec3* verts, float epsilon)
{
	// Checkings
	if(!verts || (!dfaces && !wfaces))
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "EdgeList::ComputeActiveEdges: NULL parameter!");
		return false;
	}

	PxU32 NbEdges = GetNbEdges();
	if(!NbEdges)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "ActiveEdges::ComputeConvexEdges: no edges in edge list!");
		return false;
	}

	const Gu::EdgeData* Edges = GetEdges();
	if(!Edges)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "ActiveEdges::ComputeConvexEdges: no edge data in edge list!");
		return false;
	}

	const Gu::EdgeDescData* ED = GetEdgeToTriangles();
	if(!ED)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "ActiveEdges::ComputeConvexEdges: no edge-to-triangle in edge list!");
		return false;
	}

	const PxU32* FBE = GetFacesByEdges();
	if(!FBE)
	{
		Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "ActiveEdges::ComputeConvexEdges: no faces-by-edges in edge list!");
		return false;
	}

	// We first create active edges in a temporaray buffer. We have one bool / edge.
	bool* ActiveEdges = (bool*)PX_ALLOC_TEMP(sizeof(bool)*NbEdges, PX_DEBUG_EXP("bool"));

	// Loop through edges and look for convex ones
	bool* CurrentMark = ActiveEdges;

	while(NbEdges--)
	{
		// Get number of triangles sharing current edge
		PxU32 Count = ED->Count;
		// Boundary edges are active => keep them (actually they're silhouette edges directly)
		// Internal edges can be active => test them
		// Singular edges ? => discard them
		bool Active = false;
		if(Count==1)
		{
			Active = true;
		}
		else if(Count==2)
		{
			PxU32 FaceIndex0 = FBE[ED->Offset+0]*3;
			PxU32 FaceIndex1 = FBE[ED->Offset+1]*3;

			PxU32 VRef00, VRef01, VRef02;
			PxU32 VRef10, VRef11, VRef12;

			if(dfaces)
			{
				VRef00 = dfaces[FaceIndex0+0];
				VRef01 = dfaces[FaceIndex0+1];
				VRef02 = dfaces[FaceIndex0+2];
				VRef10 = dfaces[FaceIndex1+0];
				VRef11 = dfaces[FaceIndex1+1];
				VRef12 = dfaces[FaceIndex1+2];
			}
			else //if(wfaces)
			{
				PX_ASSERT(wfaces);
				VRef00 = wfaces[FaceIndex0+0];
				VRef01 = wfaces[FaceIndex0+1];
				VRef02 = wfaces[FaceIndex0+2];
				VRef10 = wfaces[FaceIndex1+0];
				VRef11 = wfaces[FaceIndex1+1];
				VRef12 = wfaces[FaceIndex1+2];
			}

			if(1)
			{
				// We first check the opposite vertex against the plane

				PxU32 Op = OppositeVertex(VRef00, VRef01, VRef02, Edges->Ref0, Edges->Ref1);

				PxPlane PL1(verts[VRef10], verts[VRef11], verts[VRef12]);

				if(PL1.distance(verts[Op])<0.0f)	// If opposite vertex is below the plane, i.e. we discard concave edges
				{
					PxTriangle T0(verts[VRef00], verts[VRef01], verts[VRef02]);
					PxTriangle T1(verts[VRef10], verts[VRef11], verts[VRef12]);

					PxVec3 N0, N1;
					T0.normal(N0);
					T1.normal(N1);
					const float a = Ps::angle(N0, N1);

					if(fabsf(a)>epsilon)	Active = true;
				}
				else
				{
					PxTriangle T0(verts[VRef00], verts[VRef01], verts[VRef02]);
					PxTriangle T1(verts[VRef10], verts[VRef11], verts[VRef12]);
					PxVec3 N0, N1;
					T0.normal(N0);
					T1.normal(N1);

					if(N0.dot(N1) < -0.999f)
					{
						Active = true;
					}

				}
			}

		}
		else
		{
			//Connected to more than 2 
			//We need to loop through the triangles and count the number of unique triangles (considering back-face triangles as non-unique). If we end up with more than 2 unique triangles,
			//then by definition this is an inactive edge. However, if we end up with 2 unique triangles (say like a double-sided tesselated surface), then it depends on the same rules as above

			PxU32 FaceInd0 = FBE[ED->Offset]*3;
			PxU32 VRef00, VRef01, VRef02;
			PxU32 VRef10=0, VRef11=0, VRef12=0;
			if(dfaces)
			{
				VRef00 = dfaces[FaceInd0+0];
				VRef01 = dfaces[FaceInd0+1];
				VRef02 = dfaces[FaceInd0+2];
			}
			else //if(wfaces)
			{
				PX_ASSERT(wfaces);
				VRef00 = wfaces[FaceInd0+0];
				VRef01 = wfaces[FaceInd0+1];
				VRef02 = wfaces[FaceInd0+2];
			}


			PxU32 numUniqueTriangles = 1;
			bool doubleSided0 = false;
			bool doubleSided1 = 0;

			for(PxU32 a = 1; a < Count; ++a)
			{
				PxU32 FaceInd = FBE[ED->Offset+a]*3;

				PxU32 VRef0, VRef1, VRef2;
				if(dfaces)
				{
					VRef0 = dfaces[FaceInd+0];
					VRef1 = dfaces[FaceInd+1];
					VRef2 = dfaces[FaceInd+2];
				}
				else //if(wfaces)
				{
					PX_ASSERT(wfaces);
					VRef0 = wfaces[FaceInd+0];
					VRef1 = wfaces[FaceInd+1];
					VRef2 = wfaces[FaceInd+2];
				}

				if(((VRef0 != VRef00) && (VRef0 != VRef01) && (VRef0 != VRef02)) || 
					((VRef1 != VRef00) && (VRef1 != VRef01) && (VRef1 != VRef02)) || 
					((VRef2 != VRef00) && (VRef2 != VRef01) && (VRef2 != VRef02)))
				{
					//Not the same as trig 0
					if(numUniqueTriangles == 2)
					{
						if(((VRef0 != VRef10) && (VRef0 != VRef11) && (VRef0 != VRef12)) || 
							((VRef1 != VRef10) && (VRef1 != VRef11) && (VRef1 != VRef12)) || 
							((VRef2 != VRef10) && (VRef2 != VRef11) && (VRef2 != VRef12)))
						{
							//Too many unique triangles - terminate and mark as inactive
							numUniqueTriangles++;
							break;
						}
						else
						{
							PxTriangle T0(verts[VRef10], verts[VRef11], verts[VRef12]);
							PxTriangle T1(verts[VRef0], verts[VRef1], verts[VRef2]);
							PxVec3 N0, N1;
							T0.normal(N0);
							T1.normal(N1);

							if(N0.dot(N1) < -0.999f)
								doubleSided1 = true;
						}
					}
					else
					{
						VRef10 = VRef0;
						VRef11 = VRef1;
						VRef12 = VRef2;
						numUniqueTriangles++;
					}
				}
				else
				{
					//Check for double sided...
					PxTriangle T0(verts[VRef00], verts[VRef01], verts[VRef02]);
					PxTriangle T1(verts[VRef0], verts[VRef1], verts[VRef2]);
					PxVec3 N0, N1;
					T0.normal(N0);
					T1.normal(N1);

					if(N0.dot(N1) < -0.999f)
						doubleSided0 = true;
				}
			}

			if(numUniqueTriangles == 1)
				Active = true;
			if(numUniqueTriangles == 2)
			{
				//Potentially active. Let's check the angles between the surfaces...

				if(doubleSided0 || doubleSided1)
				{
				
		//			Plane PL1 = faces[FBE[ED->Offset+1]].PlaneEquation(verts);
					PxPlane PL1(verts[VRef10], verts[VRef11], verts[VRef12]);

	//				if(PL1.Distance(verts[Op])<-epsilon)	Active = true;
					//if(PL1.distance(verts[Op])<0.0f)	// If opposite vertex is below the plane, i.e. we discard concave edges
					//KS - can't test signed distance for concave edges. This is a double-sided poly
					{
						PxTriangle T0(verts[VRef00], verts[VRef01], verts[VRef02]);
						PxTriangle T1(verts[VRef10], verts[VRef11], verts[VRef12]);

						PxVec3 N0, N1;
						T0.normal(N0);
						T1.normal(N1);
						const float a = Ps::angle(N0, N1);

						if(fabsf(a)>epsilon)	
							Active = true;
					}
				}
				else
				{
					
					//Not double sided...must have had a bunch of duplicate triangles!!!!
					//Treat as normal
					PxU32 Op = OppositeVertex(VRef00, VRef01, VRef02, Edges->Ref0, Edges->Ref1);

		//			Plane PL1 = faces[FBE[ED->Offset+1]].PlaneEquation(verts);
					PxPlane PL1(verts[VRef10], verts[VRef11], verts[VRef12]);

	//				if(PL1.Distance(verts[Op])<-epsilon)	Active = true;
					if(PL1.distance(verts[Op])<0.0f)	// If opposite vertex is below the plane, i.e. we discard concave edges
					{
						PxTriangle T0(verts[VRef00], verts[VRef01], verts[VRef02]);
						PxTriangle T1(verts[VRef10], verts[VRef11], verts[VRef12]);

						PxVec3 N0, N1;
						T0.normal(N0);
						T1.normal(N1);
						const float a = Ps::angle(N0, N1);

						if(fabsf(a)>epsilon)	
							Active = true;
					}
				}
			}
			else
			{
				//Lots of triangles all  smooshed together. Just activate the edge in this case
				Active = true;
			}

		}

		*CurrentMark++ = Active;
		ED++;
		Edges++;
	}


	// Now copy bits back into already existing edge structures
	// - first in edge triangles
	for(PxU32 i=0;i<mData.mNbFaces;i++)
	{
		Gu::EdgeTriangleData& ET = mData.mEdgeFaces[i];
		for(PxU32 j=0;j<3;j++)
		{
			PxU32 Link = ET.mLink[j];
			if(!(Link & MSH_ACTIVE_EDGE_MASK))	// else already active
			{
				if(ActiveEdges[Link & MSH_EDGE_LINK_MASK])	ET.mLink[j] |= MSH_ACTIVE_EDGE_MASK;	// Mark as active
			}
		}
	}

	// - then in edge-to-faces
	for(PxU32 i=0;i<mData.mNbEdges;i++)
	{
		if(ActiveEdges[i])	mData.mEdgeToTriangles[i].Flags |= PX_EDGE_ACTIVE;
	}

	// Free & exit
	PX_FREE_AND_RESET(ActiveEdges);

	if(1)
	{
		//initially all vertices are flagged to ignore them. (we assume them to be flat)
		//for all NONFLAT edges, incl boundary
		//unflag 2 vertices in up to 2 trigs as perhaps interesting
		//for all CONCAVE edges
		//flag 2 vertices in up to 2 trigs to ignore them.

		// Handle active vertices
		PxU32 MaxIndex = 0;
		for(PxU32 i=0;i<nb_faces;i++)
		{
			PxU32 VRef0, VRef1, VRef2;
			if(dfaces)
			{
				VRef0 = dfaces[i*3+0];
				VRef1 = dfaces[i*3+1];
				VRef2 = dfaces[i*3+2];
			}
			else //if(wfaces)
			{
				PX_ASSERT(wfaces);
				VRef0 = wfaces[i*3+0];
				VRef1 = wfaces[i*3+1];
				VRef2 = wfaces[i*3+2];
			}
			if(VRef0>MaxIndex)	MaxIndex = VRef0;
			if(VRef1>MaxIndex)	MaxIndex = VRef1;
			if(VRef2>MaxIndex)	MaxIndex = VRef2;
		}

		MaxIndex++;
		bool* ActiveVerts = (bool*)PX_ALLOC_TEMP(sizeof(bool)*MaxIndex, PX_DEBUG_EXP("bool"));
		PxMemZero(ActiveVerts, MaxIndex*sizeof(bool));

		PX_ASSERT(dfaces || wfaces);
		for(PxU32 i=0;i<mData.mNbFaces;i++)
		{
			PxU32 VRef[3];
			if(dfaces)
			{
				VRef[0] = dfaces[i*3+0];
				VRef[1] = dfaces[i*3+1];
				VRef[2] = dfaces[i*3+2];
			}
			else if(wfaces)
			{
				VRef[0] = wfaces[i*3+0];
				VRef[1] = wfaces[i*3+1];
				VRef[2] = wfaces[i*3+2];
			}

			const Gu::EdgeTriangleData& ET = mData.mEdgeFaces[i];
			for(PxU32 j=0;j<3;j++)
			{
				PxU32 Link = ET.mLink[j];
				if(Link & MSH_ACTIVE_EDGE_MASK)
				{
					// Active edge => mark edge vertices as active
					PxU32 r0, r1;
							if(j==0)		{ r0=0;	r1=1; }
					else	if(j==1)		{ r0=1;	r1=2; }
					else	/*if(j==2)*/	{ PX_ASSERT(j==2);	r0=0;	r1=2; }
					ActiveVerts[VRef[r0]] = ActiveVerts[VRef[r1]] = true;
				}
			}
		}

/*		for(PxU32 i=0;i<mNbFaces;i++)
		{
			PxU32 VRef[3];
			if(dfaces)
			{
				VRef[0] = dfaces[i*3+0];
				VRef[1] = dfaces[i*3+1];
				VRef[2] = dfaces[i*3+2];
			}
			else if(wfaces)
			{
				VRef[0] = wfaces[i*3+0];
				VRef[1] = wfaces[i*3+1];
				VRef[2] = wfaces[i*3+2];
			}

			const EdgeTriangle& ET = mEdgeFaces[i];
			for(PxU32 j=0;j<3;j++)
			{
				PxU32 Link = ET.mLink[j];
				if(!(Link & MSH_ACTIVE_EDGE_MASK))
				{
					// Inactive edge => mark edge vertices as inactive
					PxU32 r0, r1;
					if(j==0)	{ r0=0;	r1=1; }
					if(j==1)	{ r0=1;	r1=2; }
					if(j==2)	{ r0=0;	r1=2; }
					ActiveVerts[VRef[r0]] = ActiveVerts[VRef[r1]] = false;
				}
			}
		}*/

		// Now stuff this into the structure
		for(PxU32 i=0;i<mData.mNbFaces;i++)
		{
			PxU32 VRef[3];
			if(dfaces)
			{
				VRef[0] = dfaces[i*3+0];
				VRef[1] = dfaces[i*3+1];
				VRef[2] = dfaces[i*3+2];
			}
			else if(wfaces)
			{
				VRef[0] = wfaces[i*3+0];
				VRef[1] = wfaces[i*3+1];
				VRef[2] = wfaces[i*3+2];
			}

			Gu::EdgeTriangleData& ET = mData.mEdgeFaces[i];
			for(PxU32 j=0;j<3;j++)
			{
				PxU32 Link = ET.mLink[j];
				if(!(Link & MSH_ACTIVE_VERTEX_MASK))	// else already active
				{
					if(ActiveVerts[VRef[j]])	ET.mLink[j] |= MSH_ACTIVE_VERTEX_MASK;	// Mark as active
				}
			}
		}

		PX_FREE_AND_RESET(ActiveVerts);
	}

	return true;
}

Gu::EdgeListBuilder::EdgeListBuilder()
{
}

Gu::EdgeListBuilder::~EdgeListBuilder()
{
}
/*
bool EdgeListBuilder::Save(Stream& stream) const
{
	bool PlatformMismatch = PxPlatformMismatch();

	// Export header
	if(!WriteHeader('E', 'D', 'G', 'E', gVersion, PlatformMismatch, stream))
		return false;

	// Export edges
//	stream.StoreDword(mNbEdges);
	WriteDword(mNbEdges, PlatformMismatch, stream);

//	stream.StoreBuffer(mEdges, sizeof(Edge)*mNbEdges);
	for(PxU32 i=0;i<mNbEdges;i++)
	{
		Edge TmpCopy = mEdges[i];
		if(PlatformMismatch)
		{
			Flip(TmpCopy.mRef0);
			Flip(TmpCopy.mRef1);
		}
		stream.StoreBuffer(&TmpCopy, sizeof(Edge));
	}

//	stream.StoreDword(mNbFaces);
	WriteDword(mNbFaces, PlatformMismatch, stream);

//	stream.StoreBuffer(mEdgeFaces, sizeof(EdgeTriangle)*mNbFaces);
	WriteDwordBuffer((const PxU32*)mEdgeFaces, mNbFaces*3, PlatformMismatch, stream);

//	stream.StoreBuffer(mEdgeToTriangles, sizeof(EdgeDesc)*mNbEdges);
	for(PxU32 i=0;i<mNbEdges;i++)
	{
		EdgeDesc TmpCopy = mEdgeToTriangles[i];
		if(PlatformMismatch)
		{
			Flip(TmpCopy.Flags);
			Flip(TmpCopy.Count);
			Flip(TmpCopy.Offset);
		}
		stream.StoreBuffer(&TmpCopy, sizeof(EdgeDesc));
	}

	PxU32 LastOffset = mEdgeToTriangles[mNbEdges-1].Offset + mEdgeToTriangles[mNbEdges-1].Count;
//	stream.StoreBuffer(mFacesByEdges, sizeof(PxU32)*LastOffset);
	WriteDwordBuffer(mFacesByEdges, LastOffset, PlatformMismatch, stream);

	return true;
}*/
//#endif




#ifndef ICE_EDGELIST_V22

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ActiveEdges::ActiveEdges() : mActiveEdges(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ActiveEdges::~ActiveEdges()
{
	PX_FREE_AND_RESET(mActiveEdges);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Computes active (a.k.a. convex) edges.
 *	\param		edges		[in] edge-list built from source data
 *	\param		faces		[in] source topology
 *	\param		verts		[in] source geometry
 *	\param		epsilon		[in] threshold value used for distance computation
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ActiveEdges::Compute(const EdgeList& edges, const IndexedTriangle* faces, const PxVec3* verts, float epsilon)
{
	// Checkings
	if(!faces || !verts)	return SetIceError("ActiveEdges::ComputeConvexEdges: NULL parameter!");

	PxU32 NbEdges = edges.GetNbEdges();
	if(!NbEdges)			return SetIceError("ActiveEdges::ComputeConvexEdges: no edges in edge list!");

	const Edge* Edges = edges.GetEdges();
	if(!Edges)				return SetIceError("ActiveEdges::ComputeConvexEdges: no edge data in edge list!");

	const EdgeDesc* ED = edges.GetEdgeToTriangles();
	if(!ED)					return SetIceError("ActiveEdges::ComputeConvexEdges: no edge-to-triangle in edge list!");

	const PxU32* FBE = edges.GetFacesByEdges();
	if(!FBE)				return SetIceError("ActiveEdges::ComputeConvexEdges: no faces-by-edges in edge list!");

	PX_FREE_AND_RESET(mActiveEdges);
	mActiveEdges = (bool*)ICE_ALLOC_MEM(sizeof(bool)*NbEdges,EdgeList_ActiveEdges);

	// Loop through edges and look for convex ones
	bool* CurrentMark = mActiveEdges;
	while(NbEdges--)
	{
		// Get number of triangles sharing current edge
		PxU32 Count = ED->Count;
		// Boundary edges are active => keep them (actually they're silhouette edges directly)
		// Internal edges can be active => test them
		// Singular edges ? => discard them
		bool Active = false;
		if(Count==1)
		{
			Active = true;
		}
		else if(Count==2)
		{
			PxU32 Op = faces[FBE[ED->Offset+0]].OppositeVertex(Edges->mRef0, Edges->mRef1);
			Plane PL1 = faces[FBE[ED->Offset+1]].PlaneEquation(verts);
			if(PL1.Distance(verts[Op])<-epsilon)	Active = true;
		}

		*CurrentMark++ = Active;
		ED++;
		Edges++;
	}
	return true;
}
#endif



