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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "EdgeList.h"
#include "Adjacencies.h"

// 7: added mHullDataFacesByVertices8
static const physx::PxU32 gVersion = 7;

#include "PxCooking.h"
#include "CookingUtils.h"
#include "ConvexHullBuilder.h"
#include "PsUserAllocated.h"
#include "GuRevisitedRadixBuffered.h"
#include "GuContainer.h"
#include "MeshCleaner.h"

using namespace physx;
using namespace Gu;


#define USE_PRECOMPUTED_HULL_PROJECTION

	//! A generic couple structure
	class Pair : public Ps::UserAllocated
	{
		public:
		PX_FORCE_INLINE	Pair()										{}
		PX_FORCE_INLINE	Pair(PxU32 i0, PxU32 i1) : id0(i0), id1(i1)	{}
		PX_FORCE_INLINE	~Pair()										{}

		//! Operator for "if(Pair==Pair)"
		PX_FORCE_INLINE	bool			operator==(const Pair& p)	const	{ return (id0==p.id0) && (id1==p.id1);	}
		//! Operator for "if(Pair!=Pair)"
		PX_FORCE_INLINE	bool			operator!=(const Pair& p)	const	{ return (id0!=p.id0) || (id1!=p.id1);	}

						PxU32	id0;	//!< First index of the pair
						PxU32	id1;	//!< Second index of the pair
	};
	PX_COMPILE_TIME_ASSERT(sizeof(Pair)==8);

template <class T>
PX_INLINE PxPlane PlaneEquation(const T& t, const PxVec3* verts)
{
	const PxVec3& p0 = verts[t.v[0]];
	const PxVec3& p1 = verts[t.v[1]];
	const PxVec3& p2 = verts[t.v[2]];
	return PxPlane(p0, p1, p2);
}

// Inverse a buffer in-place
static bool inverseBuffer(PxU32 nbEntries, PxU8* entries)
{
	if(!nbEntries || !entries)	return false;

	for(PxU32 i=0; i < (nbEntries>>1); i++)
		Ps::swap(entries[i], entries[nbEntries-1-i]);

	return true;
}

static PX_FORCE_INLINE void negatePlane(Gu::HullPolygonData& data)
{
	data.mPlane.n = -data.mPlane.n;
	data.mPlane.d = -data.mPlane.d;
}

// Extracts a line-strip from a list of non-sorted line-segments (slow)
static bool findLineStrip(Container& lineStrip, const Container& lineSegments)
{
	// Ex:
	//
	// 4-2
	// 0-1
	// 2-3
	// 4-0
	// 7-3
	// 7-1
	//
	// => 0-1-7-3-2-4-0

	// 0-0-1-1-2-2-3-3-4-4-7-7

	// 0-1
	// 0-4
	// 1-7
	// 2-3
	// 2-4
	// 3-7

	// Naive implementation below

	Container Copy(lineSegments);

	PxU32 NbDuplicates=0;
RunAgain:
	{
		Pair* Segments = (Pair*)Copy.GetEntries();
		PxU32 NbSegments = Copy.GetNbEntries()/2;
		for(PxU32 j=0;j<NbSegments;j++)
		{
			PxU32 ID0 = Segments[j].id0;
			PxU32 ID1 = Segments[j].id1;

			for(PxU32 i=j+1;i<NbSegments;i++)
			{
				if(
					(Segments[i].id0==ID0 && Segments[i].id1==ID1)
					||	(Segments[i].id1==ID0 && Segments[i].id0==ID1)
					)
				{
					// Duplicate segment found => remove both
					PX_ASSERT(Copy.GetNbEntries()>=4);
					Copy.DeleteIndex(i*2+1);
					Copy.DeleteIndex(i*2+0);
					Copy.DeleteIndex(j*2+1);
					Copy.DeleteIndex(j*2+0);
					NbDuplicates++;
					goto RunAgain;	
				}
			}
		}
		// Goes through when everything's fine
	}

	PxU32 Ref0 = 0xffffffff;
	PxU32 Ref1 = 0xffffffff;
	if(Copy.GetNbEntries()>=2)
	{
		Pair* Segments = (Pair*)Copy.GetEntries();
		if(Segments)
		{
			Ref0 = Segments->id0;
			Ref1 = Segments->id1;
			lineStrip.Add(Ref0).Add(Ref1);
			PX_ASSERT(Copy.GetNbEntries()>=2);
			Copy.DeleteIndex(1);
			Copy.DeleteIndex(0);
		}
	}

Wrap:
	// Look for same vertex ref in remaining segments
	PxU32 Nb = Copy.GetNbEntries()/2;
	if(!Nb)
	{
		// ### check the line is actually closed?
		return true;
	}

	const PxU32* S = Copy.GetEntries();
	for(PxU32 i=0;i<Nb;i++)
	{
		PxU32 NewRef0 = S[i*2+0];
		PxU32 NewRef1 = S[i*2+1];

		// We look for Ref1 only
		if(NewRef0==Ref1)
		{
			// r0 - r1
			// r1 - x
			lineStrip.Add(NewRef1);	// Output the other reference
			Ref0 = NewRef0;
			Ref1 = NewRef1;
			Copy.DeleteIndex(i*2+1);
			Copy.DeleteIndex(i*2+0);
			goto Wrap;
		}
		else if(NewRef1==Ref1)
		{
			// r0 - r1
			// x - r1	=> r1 - x
			lineStrip.Add(NewRef0);	// Output the other reference
			Ref0 = NewRef1;
			Ref1 = NewRef0;
			Copy.DeleteIndex(i*2+1);
			Copy.DeleteIndex(i*2+0);
			goto Wrap;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
*	Analyses a redundant vertices and splits the polygons if necessary.

	Go through the redundant vertices and check if the vertex does not belong to a polygon with only 3 vertices, this means we cannot remove the vertex, we need to split the polygons.
	We put all the polygons into a container 
	If we do need to split the polygons we iterate over the container and  split all the polygons containing the redundant vertices. 
	
	We clean the reported redundant vertices, since we deal with them by breaking the polygons back to the triangles.

*	\relates	ConvexHull
*	\fn			ExtractHullPolygons(Container& polygon_data, const ConvexHull& hull)
*	\param		nb_polygons		[out] number of extracted polygons
*	\param		polygon_data	[out] polygon data: (Nb indices, index 0, index 1... index N)(Nb indices, index 0, index 1... index N)(...)
*	\param		hull			[in] convex hull
*	\param      redundantVertices [out] redundant vertices found inside the polygons - we want to remove them because of PCM
*	\return		true if success
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void CheckRedundantVertices(PxU32& nb_polygons, Container& polygon_data, const ConvexHullBuilder& hull, Container& triangle_data, Container& redundantVertices)
{
	const PxU32* DFaces	= (const PxU32*)hull.GetFaces();
	bool needToSplitPolygons = false;

	bool* polygonMarkers = (bool*)PxAlloca(nb_polygons*sizeof(bool));
	PxMemZero(polygonMarkers, nb_polygons*sizeof(bool));

	bool* redundancyMarkers = (bool*)PxAlloca(redundantVertices.GetNbEntries()*sizeof(bool));
	PxMemZero(redundancyMarkers, redundantVertices.GetNbEntries()*sizeof(bool));

	// parse through the redundant vertices and if we cannot remove them split just the actual polygon if possible
	Container polygonsContainer;	
	PxU32 numEntriesTotal = 0;
	PxU32 numEntries = 0;
	for (PxU32 i = redundantVertices.GetNbEntries(); i--;)
	{
		numEntries = 0;	
		// go through polygons, if polygons does have only 3 verts we cannot remove any vertex from it, try to decompose the second one
		PxU32* Data = polygon_data.GetEntries();		
		for(PxU32 t=0;t<nb_polygons;t++)
		{			
			PxU32 NbVerts = *Data++;
			PX_ASSERT(NbVerts>=3);			// Else something very wrong happened...

			for(PxU32 j=0;j<NbVerts;j++)
			{
				if(redundantVertices[i] == Data[j])
				{
					polygonsContainer.Add(t);
					polygonsContainer.Add(NbVerts);
					numEntries++;
					break;
				}
			}
			Data += NbVerts;
		}

		for (PxU32 j = numEntriesTotal; j < numEntriesTotal + numEntries; j++)
		{
			PxU32 numInternalVertices = polygonsContainer[j*2 + 1];
			if(numInternalVertices == 3)
			{
				needToSplitPolygons = true;				
			}
		}

		numEntriesTotal += numEntries;
	}

	if(needToSplitPolygons)
	{
		redundantVertices.Reset();
		for (PxU32 j = 0; j < numEntriesTotal; j++)
		{
			polygonMarkers[polygonsContainer[j*2]] = true;
		}

		Container newPolygon_data;
		Container newTriangle_data;
		PxU32 newNb_polygons = 0;

		PxU32* Data = polygon_data.GetEntries();		
		PxU32* TriData = triangle_data.GetEntries();		
		for(PxU32 i=0;i<nb_polygons;i++)
		{			
			PxU32 NbVerts = *Data++;
			PxU32 NbTris = *TriData++;
			if(polygonMarkers[i])
			{
				// split the polygon into triangles
				for(PxU32 k=0;k< NbTris; k++)
				{
					newNb_polygons++;
					const PxU32 faceIndex = TriData[k];
					newPolygon_data.Add((PxU32)3);
					newPolygon_data.Add(DFaces[3*faceIndex]);
					newPolygon_data.Add(DFaces[3*faceIndex + 1]);
					newPolygon_data.Add(DFaces[3*faceIndex + 2]);
					newTriangle_data.Add((PxU32)1);
					newTriangle_data.Add(faceIndex);
				}
			}
			else
			{	
				newNb_polygons++;
				// copy the original polygon
				newPolygon_data.Add(NbVerts);
				for(PxU32 j=0;j<NbVerts;j++)				
					newPolygon_data.Add(Data[j]);

				// copy the original polygon triangles
				newTriangle_data.Add(NbTris);
				for(PxU32 k=0;k< NbTris; k++)
				{
					newTriangle_data.Add(TriData[k]);
				}
			}
			Data += NbVerts;
			TriData += NbTris;
		}

		// now put the data to output
		polygon_data.Empty();
		triangle_data.Empty();

		// the copy does copy even the data
		polygon_data = newPolygon_data;
		triangle_data = newTriangle_data;
		nb_polygons = newNb_polygons;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
*	Analyses a convex hull made of triangles and extracts polygon data out of it.
*	\relates	ConvexHull
*	\fn			ExtractHullPolygons(Container& polygon_data, const ConvexHull& hull)
*	\param		nb_polygons		[out] number of extracted polygons
*	\param		polygon_data	[out] polygon data: (Nb indices, index 0, index 1... index N)(Nb indices, index 0, index 1... index N)(...)
*	\param		hull			[in] convex hull
*	\param      rendundantVertices [out] redundant vertices found inside the polygons - we want to remove them because of PCM
*	\return		true if success
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool ExtractHullPolygons(PxU32& nb_polygons, Container& polygon_data, const ConvexHullBuilder& hull, Container* triangle_data, Container& rendundantVertices)
{
	PxU32 NbFaces	= hull.GetNbFaces();
	const PxVec3* V	= hull.mHullDataHullVertices;
	const PxU32 NbVertices = hull.mHull->mNbHullVertices;

	const PxU16* WFaces	= NULL;
	const PxU32* DFaces	= (const PxU32*)hull.GetFaces();
	PX_ASSERT(WFaces || DFaces);

	ADJACENCIESCREATE Create;
	Create.NbFaces	= NbFaces;
	Create.DFaces	= DFaces;
	Create.WFaces	= WFaces;
	Create.Verts	= V;
	//Create.Epsilon	= 0.01f;	// PT: trying to fix Rob Elam bug. Also fixes TTP 2467
	//	Create.Epsilon	= 0.001f;	// PT: for "Bruno's bug"
	Create.Epsilon	= 0.005f;	// PT: middle-ground seems to fix both. Expose this param?


	AdjacenciesBuilder Adj;
	if(!Adj.Init(Create))	return false;

	PxU32 NbBoundaryEdges = Adj.ComputeNbBoundaryEdges();
	if(NbBoundaryEdges)	return false;	// A valid hull shouldn't have open edges!!

	bool* Markers = (bool*)PxAlloca(NbFaces*sizeof(bool));
	PxMemZero(Markers, NbFaces*sizeof(bool));

	PxU8* VertexMarkers = (PxU8*)PxAlloca(NbVertices*sizeof(PxU8));
	PxMemZero(VertexMarkers, NbVertices*sizeof(PxU8));

	PxU32 CurrentFace = 0;	// Start with first triangle
	nb_polygons = 0;
	do
	{
		CurrentFace = 0;
		while(CurrentFace<NbFaces && Markers[CurrentFace])	CurrentFace++;

		// Start from "closest" face and floodfill through inactive edges
		struct Local
		{
			static void FloodFill(Container& indices, const AdjTriangle* faces, PxU32 current, bool* markers)
			{
				if(markers[current])	return;
				markers[current] = true;

				indices.Add(current);
				const AdjTriangle& AT = faces[current];

				// We can floodfill through inactive edges since the mesh is convex (inactive==planar)
				if(!AT.HasActiveEdge01())	FloodFill(indices, faces, AT.GetAdjTri(EDGE01), markers);
				if(!AT.HasActiveEdge20())	FloodFill(indices, faces, AT.GetAdjTri(EDGE02), markers);
				if(!AT.HasActiveEdge12())	FloodFill(indices, faces, AT.GetAdjTri(EDGE12), markers);
			}

			static bool GetNeighborFace(PxU32 index,PxU32 triangleIndex,const AdjTriangle* faces, const PxU32* dfaces, PxU32& neighbor, PxU32& current)
			{			
				PxU32 currentIndex = index;
				PxU32 previousIndex = index;
				bool firstFace = true;
				bool next = true;
				while (next)
				{
					const AdjTriangle& currentAT = faces[currentIndex];
					PxU32 refTr0 = dfaces[currentIndex*3 + 0];
					PxU32 refTr1 = dfaces[currentIndex*3 + 1];

					PxU32 edge[2];
					edge[0] = 1;
					edge[1] = 2;
					if(triangleIndex == refTr0)
					{
						edge[0] = 0;
						edge[1] = 1;
					}
					else
					{
						if(triangleIndex == refTr1)
						{
							edge[0] = 0;
							edge[1] = 2;
						}
					}

					if(currentAT.HasActiveEdge(edge[0]) && currentAT.HasActiveEdge(edge[1]))
					{
						return false;					
					}

					if(!currentAT.HasActiveEdge(edge[0]) && !currentAT.HasActiveEdge(edge[1]))
					{
						// not interested in testing transition vertices 
						if(currentIndex == index)
						{
							return false;
						}

						// transition one
						for (PxU32 i = 0; i < 2; i++)
						{
							PxU32 testIndex = currentAT.GetAdjTri((SharedEdgeIndex) (edge[i]));

							// exit if we circle around the vertex back to beginning
							if(testIndex == index && previousIndex != index)
							{
								return false;
							}

							if(testIndex != previousIndex)
							{
								// move to next 
								previousIndex = currentIndex;
								currentIndex = testIndex;
								break;
							}							
						}
					}
					else
					{
						if(!currentAT.HasActiveEdge(edge[0]))
						{
							PxU32 t = edge[0];
							edge[0] = edge[1];
							edge[1] = t;
						}

						if(currentAT.HasActiveEdge(edge[0]))
						{
							PxU32 testIndex = currentAT.GetAdjTri((SharedEdgeIndex) (edge[0]));
							if(firstFace)
							{
								firstFace = false;
							}
							else
							{
								neighbor = testIndex;
								current = currentIndex;
								return true;									
							}
						}

						if(!currentAT.HasActiveEdge(edge[1]))
						{
							PxU32 testIndex = currentAT.GetAdjTri((SharedEdgeIndex) (edge[1]));
							if(testIndex != index)
							{
								previousIndex = currentIndex;
								currentIndex = testIndex;
							}
						}
					}

				}

				return false;
			}

			static bool CheckFloodFillFace(PxU32 index,const AdjTriangle* faces, const PxU32* dfaces)
			{
				if(!dfaces)
					return true;

				const AdjTriangle& checkedAT = faces[index];

				PxU32 refTr0 = dfaces[index*3 + 0];
				PxU32 refTr1 = dfaces[index*3 + 1];
				PxU32 refTr2 = dfaces[index*3 + 2];

				for (PxU32 i = 0; i < 3; i++)
				{
					if(!checkedAT.HasActiveEdge(i))
					{
						PxU32 testTr0 = refTr1;
						PxU32 testTr1 = refTr2;
						PxU32 testIndex0 = 0;
						PxU32 testIndex1 = 1;
						if(i == 0)
						{
							testTr0 = refTr0;
							testTr1 = refTr1;
							testIndex0 = 1;
							testIndex1 = 2;
						}
						else
						{
							if(i == 1)
							{
								testTr0 = refTr0;
								testTr1 = refTr2;
								testIndex0 = 0;
								testIndex1 = 2;
							}
						}

						PxU32 adjFaceTested = checkedAT.GetAdjTri((SharedEdgeIndex)testIndex0);
						
						PxU32 neighborIndex00;
						PxU32 neighborIndex01;
						bool found0 = GetNeighborFace(index,testTr0,faces,dfaces, neighborIndex00, neighborIndex01);
						PxU32 neighborIndex10;
						PxU32 neighborIndex11;
						bool found1 = GetNeighborFace(adjFaceTested,testTr0,faces,dfaces, neighborIndex10, neighborIndex11);

						if(found0 && found1 && neighborIndex00 == neighborIndex11 && neighborIndex01 == neighborIndex10)
						{
							return false;
						}

						adjFaceTested = checkedAT.GetAdjTri((SharedEdgeIndex)testIndex1);
						found0 = GetNeighborFace(index,testTr1,faces,dfaces,neighborIndex00,neighborIndex01);
						found1 = GetNeighborFace(adjFaceTested,testTr1,faces,dfaces,neighborIndex10,neighborIndex11);

						if(found0 && found1 && neighborIndex00 == neighborIndex11 && neighborIndex01 == neighborIndex10)
						{
							return false;
						}

					}
				}

				return true;
			}

			static bool CheckFloodFill(Container& indices,AdjTriangle* faces,bool* markers, const PxU32* dfaces)
			{
				bool valid = true;

				for(PxU32 i=0;i<indices.GetNbEntries();i++)
				{
					//const AdjTriangle& AT = faces[indices.GetEntry(i)];

					for(PxU32 j= i + 1;j<indices.GetNbEntries();j++)
					{						
						const AdjTriangle& testAT = faces[indices.GetEntry(j)];

						if(testAT.GetAdjTri(EDGE01) == indices.GetEntry(i))
						{
							if(testAT.HasActiveEdge01())
							{								
								valid = false;
							}
						}
						if(testAT.GetAdjTri(EDGE02) == indices.GetEntry(i))
						{
							if(testAT.HasActiveEdge20())
							{							
								valid = false;
							}
						}
						if(testAT.GetAdjTri(EDGE12) == indices.GetEntry(i))
						{
							if(testAT.HasActiveEdge12())
							{							
								valid  = false;
							}
						}

						if(!valid)
							break;				
					}

					if(!CheckFloodFillFace(indices.GetEntry(i), faces, dfaces))
					{
						valid = false;
					}

					if(!valid)
						break;
				}

				if(!valid)
				{
					for(PxU32 i=0;i<indices.GetNbEntries();i++)
					{
						AdjTriangle& AT = faces[indices.GetEntry(i)];
						AT.mATri[0] |= 0x20000000;
						AT.mATri[1] |= 0x20000000;
						AT.mATri[2] |= 0x20000000;

						markers[indices.GetEntry(i)] = false;
					}					

					indices.Reset();

					return true;
				}

				return false;
			}
		};

		if(CurrentFace!=NbFaces)
		{
			Container Indices;	// Indices of triangles forming hull polygon

			bool doFill = true;
			while (doFill)
			{
				Local::FloodFill(Indices, Adj.mFaces, CurrentFace, Markers);

				doFill = Local::CheckFloodFill(Indices,Adj.mFaces,Markers, DFaces);
			}			

			// Now it would be nice to recreate a closed linestrip, similar to silhouette extraction. The line is composed of active edges, this time.

			Container ActiveSegments;
			// Loop through triangles composing the polygon
			for(PxU32 i=0;i<Indices.GetNbEntries();i++)
			{
				const PxU32 CurrentTriIndex = Indices.GetEntry(i);	// Catch current triangle
				const PxU32 VRef0 = DFaces ? DFaces[CurrentTriIndex*3+0] : WFaces[CurrentTriIndex*3+0];
				const PxU32 VRef1 = DFaces ? DFaces[CurrentTriIndex*3+1] : WFaces[CurrentTriIndex*3+1];
				const PxU32 VRef2 = DFaces ? DFaces[CurrentTriIndex*3+2] : WFaces[CurrentTriIndex*3+2];

				// Keep active edges
				if(Adj.mFaces[CurrentTriIndex].HasActiveEdge01())	{ ActiveSegments.Add(VRef0).Add(VRef1);	}
				if(Adj.mFaces[CurrentTriIndex].HasActiveEdge20())	{ ActiveSegments.Add(VRef0).Add(VRef2);	}
				if(Adj.mFaces[CurrentTriIndex].HasActiveEdge12())	{ ActiveSegments.Add(VRef1).Add(VRef2);	}
			}

			// We assume the polygon is convex. In that case it should always be possible to retriangulate it so that the triangles are
			// implicit (in particular, it should always be possible to remove interior triangles)

			Container LineStrip;
			if(findLineStrip(LineStrip, ActiveSegments))
			{
				PxU32 Nb = LineStrip.GetNbEntries();
				if(Nb)
				{
					const PxU32* Entries = LineStrip.GetEntries();
					PX_ASSERT(Entries[0] == Entries[Nb-1]);	// findLineStrip() is designed that way. Might not be what we want!

					for (PxU32 i = 0; i < Nb-1; i++)
					{
						VertexMarkers[Entries[i]]++;
					}

					// We get rid of the last (duplicated) index
					polygon_data.Add(Nb-1);
					polygon_data.Add(Entries, Nb-1);
					nb_polygons++;

					// Loop through vertices composing the line strip polygon end mark the redundant vertices inside the polygon 
					for(PxU32 i=0;i<Indices.GetNbEntries();i++)
					{
						const PxU32 CurrentTriIndex = Indices.GetEntry(i);	// Catch current triangle
						const PxU32 VRef0 = DFaces ? DFaces[CurrentTriIndex*3+0] : WFaces[CurrentTriIndex*3+0];
						const PxU32 VRef1 = DFaces ? DFaces[CurrentTriIndex*3+1] : WFaces[CurrentTriIndex*3+1];
						const PxU32 VRef2 = DFaces ? DFaces[CurrentTriIndex*3+2] : WFaces[CurrentTriIndex*3+2];

						bool found0 = false;
						bool found1 = false;
						bool found2 = false;

						for (PxU32 j=0;j < Nb - 1; j++)
						{
							if(VRef0 == Entries[j])
							{
								found0 = true;								
							}

							if(VRef1 == Entries[j])
							{
								found1 = true;								
							}

							if(VRef2 == Entries[j])
							{
								found2 = true;								
							}

							if(found0 && found1 && found2)
								break;
						}

						if(!found0)
						{
							rendundantVertices.AddUnique(VRef0);
						}

						if(!found1)
						{
							rendundantVertices.AddUnique(VRef1);
						}

						if(!found2)
						{
							rendundantVertices.AddUnique(VRef2);
						}
					}					

					// If needed, output triangle indices used to build this polygon
					if(triangle_data)
					{
						triangle_data->Add(Indices.GetNbEntries());
						triangle_data->Add(Indices);
					}
				}
			}
			else
			{
				Ps::getFoundation().error(PxErrorCode::eINVALID_OPERATION, __FILE__, __LINE__, "Meshmerizer::ExtractHullPolygons: line strip extraction failed");				
				return false;
			}
		}
	}
	while(CurrentFace!=NbFaces);

	for (PxU32 i = 0; i < NbVertices; i++)
	{
		if(VertexMarkers[i] < 3)
		{
			rendundantVertices.AddUnique(i);
		}
	}

	if(rendundantVertices.GetNbEntries() > 0 && triangle_data)
		CheckRedundantVertices(nb_polygons,polygon_data,hull,*triangle_data,rendundantVertices);

	return true;
}

// Test for duplicate triangles
PX_COMPILE_TIME_ASSERT(sizeof(Gu::TriangleT<PxU32>)==sizeof(PxVec3));	// ...
static bool TestDuplicateTriangles(PxU32& nbFaces, Gu::TriangleT<PxU32>* faces, bool repair)
{
	if(!nbFaces || !faces)
		return true;

	Gu::TriangleT<PxU32>* Indices32 = (Gu::TriangleT<PxU32>*)PxAlloca(nbFaces*sizeof(Gu::TriangleT<PxU32>));
	for(PxU32 i=0;i<nbFaces;i++)
	{
		Indices32[i].v[0] = faces[i].v[0];
		Indices32[i].v[1] = faces[i].v[1];
		Indices32[i].v[2] = faces[i].v[2];
	}

	// Radix-sort power...
	ReducedVertexCloud	Reducer(reinterpret_cast<PxVec3*>(Indices32), nbFaces);
	REDUCEDCLOUD rc;
	Reducer.Reduce(&rc);
	if(rc.NbRVerts<nbFaces)
	{
		if(repair)
		{
			nbFaces = rc.NbRVerts;
			for(PxU32 i=0;i<nbFaces;i++)
			{
				const Gu::TriangleT<PxU32>* CurTri = reinterpret_cast<const Gu::TriangleT<PxU32>*>(&rc.RVerts[i]);
				faces[i].v[0] = CurTri->v[0];
				faces[i].v[1] = CurTri->v[1];
				faces[i].v[2] = CurTri->v[2];
			}
		}
		return false;	// Test failed
	}
	return true;	// Test succeeded
}

static PX_FORCE_INLINE bool testCulling(const Gu::TriangleT<PxU32>& triangle, const PxVec3* verts, const PxVec3& center)
{
	const PxPlane plane(verts[triangle.v[0]], verts[triangle.v[1]], verts[triangle.v[2]]);
	return plane.distance(center)>0.0f;
}

static bool TestUnifiedNormals(PxU32 nbVerts, const PxVec3* verts, PxU32 nbFaces, Gu::TriangleT<PxU32>* faces, bool repair)
{
	if(!nbVerts || !verts || !nbFaces || !faces)
		return false;

	// Unify normals so that all hull faces are well oriented

	// Compute geometric center - we need a vertex inside the hull
	const float Coeff = 1.0f / float(nbVerts);
	PxVec3 GeomCenter(0.0f, 0.0f, 0.0f);
	for(PxU32 i=0;i<nbVerts;i++)
	{
		GeomCenter.x += verts[i].x * Coeff;
		GeomCenter.y += verts[i].y * Coeff;
		GeomCenter.z += verts[i].z * Coeff;
	}

	// We know the hull is (hopefully) convex so we can easily test whether a point is inside the hull or not.
	// The previous geometric center must be invisible from any hull face: that's our test to decide whether a normal
	// must be flipped or not.
	bool Status = true;
	for(PxU32 i=0;i<nbFaces;i++)
	{
		// Test face visibility from the geometric center (supposed to be inside the hull).
		// All faces must be invisible from this point to ensure a strict CCW order.
		if(testCulling(faces[i], verts, GeomCenter))
		{
			if(repair)	faces[i].flip();
			Status = false;
		}
	}

	return Status;
}

static bool cleanMesh(PxU32& nbVerts, PxU32& nbTris, PxVec3* verts, Gu::TriangleT<PxU32>* faces)
{
	MeshCleaner cleaner(nbVerts, verts, nbTris, faces->v, 0.0f);
	if(!cleaner.mNbTris)
		return false;

	nbVerts = cleaner.mNbVerts;
	nbTris = cleaner.mNbTris;

	PxMemCopy(verts, cleaner.mVerts, cleaner.mNbVerts*sizeof(PxVec3));

	for(PxU32 i=0;i<cleaner.mNbTris;i++)
	{
		faces[i].v[0] = cleaner.mIndices[i*3+0];
		faces[i].v[1] = cleaner.mIndices[i*3+1];
		faces[i].v[2] = cleaner.mIndices[i*3+2];
	}
	return true;
}

static bool CleanHull(PxU32& nbFaces, Gu::TriangleT<PxU32>* faces, PxU32& nbVerts, PxVec3* verts)
{
	// Brute force mesh cleaning.
	// PT: I added this back on Feb-18-05 because it fixes bugs with hulls from QHull.
	if(!cleanMesh(nbVerts, nbFaces, verts, faces))	return false;

	// Get rid of duplicates
	TestDuplicateTriangles(nbFaces, faces, true);

	// Unify normals
	TestUnifiedNormals(nbVerts, verts, nbFaces, faces, true);

	// Remove zero-area triangles
	//	TestZeroAreaTriangles(nbFaces, faces, verts, true);

	// Unify normals again
	TestUnifiedNormals(nbVerts, verts, nbFaces, faces, true);

	// Get rid of duplicates again
	TestDuplicateTriangles(nbFaces, faces, true);

	return true;
}

static bool CheckHull(PxU32 nbFaces, const Gu::TriangleT<PxU32>* faces, PxU32 nbVerts, const PxVec3* verts)
{
	// Remove const since we use functions that can do both testing & repairing. But we won't change the data.
	Gu::TriangleT<PxU32>* f = const_cast<Gu::TriangleT<PxU32>*>(faces);

	// Test duplicate faces
	if(!TestDuplicateTriangles(nbFaces, f, false))	return false;

	// Test unified normals
	if(!TestUnifiedNormals(nbVerts, verts, nbFaces, f, false))	return false;

	// Test zero-area triangles
	//	if(!TestZeroAreaTriangles(nbFaces, f, verts, false))	return false;

	return true;
}

ConvexHullBuilder::ConvexHullBuilder(Gu::ConvexHullData* hull) : 
	mHullDataHullVertices		(NULL),
	mHullDataPolygons			(NULL),
	mHullDataVertexData8		(NULL),
	mHullDataFacesByEdges8		(NULL),
	mHullDataFacesByVertices8	(NULL),
	mHull						(hull), 
	mEdgeToTriangles			(0),
	mEdgeData16					(NULL),
	mNbHullFaces				(0),
	mFaces						(NULL)
{
}

ConvexHullBuilder::~ConvexHullBuilder()
{
	PX_DELETE_POD(mEdgeData16);
	PX_DELETE_POD(mFaces);

	PX_DELETE_POD(mHullDataHullVertices);
	PX_DELETE_POD(mHullDataPolygons);
	PX_DELETE_POD(mHullDataVertexData8);
	PX_DELETE_POD(mHullDataFacesByEdges8);
	PX_DELETE_POD(mHullDataFacesByVertices8);
}

bool ConvexHullBuilder::Init(PxU32 nbVerts, const PxVec3* verts, PxU32 nbTris, const PxU32* indices, const PxU32 nbPolygons, const PxHullPolygon* hullPolygons)
{
	mHullDataHullVertices			= NULL;
	mHullDataPolygons				= NULL;
	mHullDataVertexData8			= NULL;
	mHullDataFacesByEdges8			= NULL;
	mHullDataFacesByVertices8		= NULL;

	mNbHullFaces					= nbTris;
	mHull->mNbHullVertices			= Ps::to8(nbVerts);
	mHullDataHullVertices			= (PxVec3*)PX_ALLOC(sizeof(PxVec3) * mHull->mNbHullVertices, PX_DEBUG_EXP("PxVec3"));
	PxMemCopy(mHullDataHullVertices, verts, mHull->mNbHullVertices*sizeof(PxVec3));

	if(indices && !hullPolygons)
	{
		// Convert to 16 bits
		mFaces = PX_NEW(HullTriangleData)[mNbHullFaces];
		for(PxU32 i=0;i<mNbHullFaces;i++)
		{
			PX_ASSERT(indices[i*3+0]<=0xffff);
			PX_ASSERT(indices[i*3+1]<=0xffff);
			PX_ASSERT(indices[i*3+2]<=0xffff);
			mFaces[i].mRef[0] = indices[i*3+0];
			mFaces[i].mRef[1] = indices[i*3+1];
			mFaces[i].mRef[2] = indices[i*3+2];
		}
	}

	Container polygons;
	PxU32* polygon_data = NULL;
	if(hullPolygons)
	{		
		for (PxU32 i = 0; i < nbPolygons; i++)
		{
			const PxHullPolygon& hullPolygon = hullPolygons[i];
			polygons.Add((PxU32)hullPolygon.mNbVerts);
			for (PxU32 j = 0; j < hullPolygon.mNbVerts; j++)
			{
				polygons.Add(indices[hullPolygon.mIndexBase + j]);
			}
		}
		polygon_data = polygons.GetEntries();
	}

	//return true;	ok if we return here
	if(nbPolygons && polygon_data)
//	if(0)
	{
		// Special codepath where we accept hull polygons directly:
		// - the hull vertices are not cleaned because any modification of source data could invalidate
		//   polygon data (indexing source vertices)
		// - the hull polygons are directly copied, but still checked afterwards (this part is safe)

		// This really shouldn't be there
//		TestUnifiedNormals(mNbHullVertices, mHullVertices, mNbHullFaces, mFaces, true);

		// CreatePolygonData
		{
			// Cleanup
			mHull->mNbPolygons = 0;
			PX_DELETE_POD(mHullDataVertexData8);
			PX_FREE_AND_RESET(mHullDataPolygons);

			const PxU32 NbPolygons = nbPolygons;

			if(NbPolygons>255)
			{
  				Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "ConvexHullBuilder::Init: convex hull has more than 255 polygons!");
				return false;
			}

			// Precompute hull polygon structures
			mHull->mNbPolygons = Ps::to8(NbPolygons);
			mHullDataPolygons = (Gu::HullPolygonData*)PX_ALLOC(sizeof(Gu::HullPolygonData)*mHull->mNbPolygons, PX_DEBUG_EXP("Gu::HullPolygonData"));//PX_NEW(Gu::HullPolygonData)[mHull->mNbPolygons];

			// Loop through polygons
			// We have N polygons => remove N entries for number of vertices

			const PxU32* Data = polygon_data;
			PxU32 Tmp=0;
			for(PxU32 i=0;i<NbPolygons;i++)
			{
				PxU32 NbVerts = *Data++;
				// Next one
				Data += NbVerts;			// Skip vertex indices
				Tmp += NbVerts;
			}

			mHullDataVertexData8 = PX_NEW(PxU8)[Tmp];
			Data = polygon_data;
			PxU8* Dest = mHullDataVertexData8;
			for(PxU32 i=0;i<NbPolygons;i++)
			{
				mHullDataPolygons[i].mVRef8 = PxU16(Dest - mHullDataVertexData8);	// Setup link for current polygon

				PxU32 NbVerts = *Data++;
				PX_ASSERT(NbVerts>=3);			// Else something very wrong happened...
				mHullDataPolygons[i].mNbVerts = Ps::to8(NbVerts);

				for (PxU32 j = 0; j < NbVerts; j++)
				{
					Dest[j] = Ps::to8(Data[j]);
				}

				mHullDataPolygons[i].mPlane = PxPlane(hullPolygons[i].mPlane[0],hullPolygons[i].mPlane[1],hullPolygons[i].mPlane[2],hullPolygons[i].mPlane[3]);				

				// Next one
				Data += NbVerts;			// Skip vertex indices
				Dest += NbVerts;
			}

			if(!CalculateVertexMapTable(NbPolygons, true))
				return false;

#ifdef USE_PRECOMPUTED_HULL_PROJECTION
			// Loop through polygons
			for(PxU32 j=0;j<NbPolygons;j++)
			{
				// Precompute hull projection along local polygon normal
				const PxU32 NbVerts = mHull->mNbHullVertices;
				const PxVec3* PX_RESTRICT Verts = mHullDataHullVertices;
				Gu::HullPolygonData& polygon = mHullDataPolygons[j];
				PxReal minimum = PX_MAX_F32;
				PxU8 minIndex = 0xff;
				for (PxU8 i = 0; i < NbVerts; i++)
				{
					const float dp = (*Verts++).dot(polygon.mPlane.n);
					if(dp < minimum)	
					{ 
						minimum = dp; 
						minIndex = i; 
					} 
				}
				polygon.mMinIndex = minIndex;
//				polygon.mMinObsolete = minimum;
			}
#endif
		}
		// ~CreatePolygonData

		if(!CreateTrianglesFromPolygons())
			return false;

		return CheckHullPolygons();
//		return true;
	}

	// PT:
	//	if(!TestZeroAreaTriangles(mHull->mNbHullFaces, mHull->mFaces, mHull->mHullVertices, true))	return false;

	Gu::TriangleT<PxU32>* hullAsIndexedTriangle = reinterpret_cast<Gu::TriangleT<PxU32>*>(mFaces);

	// We don't trust the user at all... So, clean the hull.
	PxU32 nbHullVerts = mHull->mNbHullVertices;
	CleanHull(mNbHullFaces, hullAsIndexedTriangle, nbHullVerts, mHullDataHullVertices);
	PX_ASSERT(nbHullVerts<256);
	mHull->mNbHullVertices = Ps::to8(nbHullVerts);

	// ...and then run the full tests again.
	if(!CheckHull(mNbHullFaces, hullAsIndexedTriangle, mHull->mNbHullVertices, mHullDataHullVertices))	return false;
	//return true;	ok here

	// Transform triangles-to-polygons
	if(!CreatePolygonData())	return false;
	//return true;	not ok there!!!

	return CheckHullPolygons();
}

namespace physx
{
	PX_INLINE void Flip(HullTriangleData& data)
	{
		PxU32 tmp = data.mRef[2];
		data.mRef[2] = data.mRef[1];
		data.mRef[1] = tmp;
	}
}

bool ConvexHullBuilder::CreateTrianglesFromPolygons()
{
	if(!mHull->mNbPolygons || !mHullDataPolygons)	return false;

	PxU32 MaxNbTriangles = 0;
	for(PxU32 i=0;i<mHull->mNbPolygons;i++)
	{
		if(mHullDataPolygons[i].mNbVerts < 3)
		{
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "ConvexHullBuilder::CreateTrianglesFromPolygons: convex hull has a polygon with less than 3 vertices!");
			return false;
		}
		MaxNbTriangles += mHullDataPolygons[i].mNbVerts - 2;
	}

	HullTriangleData* TmpFaces = PX_NEW(HullTriangleData)[MaxNbTriangles];

	HullTriangleData* CurrFace = TmpFaces;
	PxU32 NbTriangles = 0;
	const PxU8* vertexData = mHullDataVertexData8;
	const PxVec3* hullVerts = mHullDataHullVertices;
	for(PxU32 i=0;i<mHull->mNbPolygons;i++)
	{
		const PxU8* Data = vertexData + mHullDataPolygons[i].mVRef8;
		PxU32 NbVerts = mHullDataPolygons[i].mNbVerts;

		// Triangulate the polygon such that all all generated triangles have one and the same vertex
		// in common.
		//
		// Make sure to avoid creating zero area triangles. Imagine the following polygon:
		//
		// 4                  3
		// *------------------*
		// |                  |
		// *---*----*----*----*
		// 5   6    0    1    2
		//
		// Choosing vertex 0 as the shared vertex, the following zero area triangles will be created:
		// [0 1 2], [0 5 6]
		//
		// Check for these triangles and discard them
		// Note: Such polygons should only occur if the user defines the convex hull, i.e., the triangles
		//       of the convex shape, himself. If the convex hull is built from the vertices only, the
		//       hull algorithm removes the useless vertices.
		//
		for(PxU32 j=0;j<NbVerts-2;j++)
		{
			CurrFace->mRef[0] = Data[0];
			CurrFace->mRef[1] = Data[(j+1)%NbVerts];
			CurrFace->mRef[2] = Data[(j+2)%NbVerts];

			const PxVec3& p0 = hullVerts[CurrFace->mRef[0]];
			const PxVec3& p1 = hullVerts[CurrFace->mRef[1]];
			const PxVec3& p2 = hullVerts[CurrFace->mRef[2]];

			const float area = ((p1-p0).cross(p2-p0)).magnitudeSquared();

			if (area != 0.0f)	// Else discard the triangle
			{
				NbTriangles++;
				CurrFace++;
			}
		}
	}

	PX_DELETE_POD(mFaces);
	HullTriangleData* Faces;
	PX_ASSERT(NbTriangles <= MaxNbTriangles);
	if (MaxNbTriangles == NbTriangles)
	{
		// No zero area triangles, hence the face buffer has correct size and can be used directly.
		Faces = TmpFaces;
	}
	else
	{
		// Resize face buffer because some triangles were discarded.
		Faces = PX_NEW(HullTriangleData)[NbTriangles];
		if (!Faces)
		{
			PX_DELETE_POD(TmpFaces);
			return false;
		}
		PxMemCopy(Faces, TmpFaces, sizeof(HullTriangleData)*NbTriangles);
		PX_DELETE_POD(TmpFaces);
	}
	mFaces = Faces;
	mNbHullFaces = NbTriangles;
	// TODO: at this point useless vertices should be removed from the hull. The current fix is to initialize
	// support vertices to known valid vertices, but it's not really convincing.

	// Re-unify normals
	PxVec3 GeomCenter;
	ComputeGeomCenter(GeomCenter);

	for(PxU32 i=0;i<mNbHullFaces;i++)
	{
		const PxPlane P(	hullVerts[mFaces[i].mRef[0]],
							hullVerts[mFaces[i].mRef[1]],
							hullVerts[mFaces[i].mRef[2]]);
		if(P.distance(GeomCenter)>0.0f)
		{
			Flip(mFaces[i]);
		}
	}
	return true;
}

bool ConvexHullBuilder::CheckHullPolygons() const
{
	const PxVec3* hullVerts = mHullDataHullVertices;
	const PxU8* vertexData = mHullDataVertexData8;
	Gu::HullPolygonData* hullPolygons = mHullDataPolygons;

	// Check hull validity
	if(!hullVerts || !mFaces || !hullPolygons)	return false;

	// dilip: reject if there are fewer than 3 faces (fix TTP#7128)

	if(mHull->mNbPolygons<4)
		return false;

	PxVec3 max(-FLT_MAX,-FLT_MAX,-FLT_MAX);
	for(PxU32 j=0;j<mHull->mNbHullVertices;j++)
	{
		PxVec3 hullVert = hullVerts[j];
		if(fabsf(hullVert.x) > max.x)
			max.x = fabsf(hullVert.x);

		if(fabsf(hullVert.y) > max.y)
			max.y = fabsf(hullVert.y);

		if(fabsf(hullVert.z) > max.z)
			max.z = fabsf(hullVert.z);
	}

	max += PxVec3(0.02f,0.02f,0.02f);	

	PxVec3 testVectors[8];
	bool	foundPlane[8];
	for (PxU32 i = 0; i < 8; i++)
	{
		foundPlane[i] = false;
	}

	testVectors[0] = PxVec3(max.x,max.y,max.z);
	testVectors[1] = PxVec3(max.x,-max.y,-max.z);
	testVectors[2] = PxVec3(max.x,max.y,-max.z);
	testVectors[3] = PxVec3(max.x,-max.y,max.z);
	testVectors[4] = PxVec3(-max.x,max.y,max.z);
	testVectors[5] = PxVec3(-max.x,-max.y,max.z);
	testVectors[6] = PxVec3(-max.x,max.y,-max.z);
	testVectors[7] = PxVec3(-max.x,-max.y,-max.z);


	// Extra convex hull validity check. This is less aggressive than previous convex decomposer!
	if(1)
	{
		// Loop through polygons
		for(PxU32 i=0;i<mHull->mNbPolygons;i++)
		{
			const PxPlane& P = hullPolygons[i].mPlane;

			for (PxU32 k = 0; k < 8; k++)
			{
				if(!foundPlane[k])
				{
					const float d = P.distance(testVectors[k]);
					if(d >= 0)
					{
						foundPlane[k] = true;
					}
				}
			}

			// Test hull vertices against polygon plane
			for(PxU32 j=0;j<mHull->mNbHullVertices;j++)
			{
				// Don't test vertex if it belongs to plane (to prevent numerical issues)
				PxU32 Nb = hullPolygons[i].mNbVerts;
				bool Discard=false;
				for(PxU32 k=0;k<Nb;k++)
				{
					if(vertexData[hullPolygons[i].mVRef8+k]==(PxU8)j)
					{
						Discard = true;
						break;
					}
				}

				if(!Discard)
				{
					const float d = P.distance(hullVerts[j]);
//					if(d>0.0001f)
					if(d>0.02f)
					{
						PX_ALWAYS_ASSERT_MESSAGE("Hull check failed! Please report this repro case to Pierre.");
						return false;
					}
				}
			}
		}

		for (PxU32 i = 0; i < 8; i++)
		{
			if(!foundPlane[i])
			{
				PX_ALWAYS_ASSERT_MESSAGE("Hull check failed! Hull seems to have opened volume or do (some) faces have reversed winding?");
				return false;
			}
		}
	}

	if(0)
	{
		PxU32 NbVerts = mHull->mNbHullVertices;
		for(PxU32 i=0;i<NbVerts;i++)
		{
			for(PxU32 j=0;j<NbVerts;j++)
			{
				if(i!=j && (hullVerts[i]-hullVerts[j]).magnitude()<0.1f)
//				if(i!=j && mHull->getHullVertices()[i].distance(mHull->getHullVertices()[j])<0.1f)
//				if(i!=j && mHull->mHullVertices[i].distance(mHull->mHullVertices[j])<0.1f)
				{
					int k=0;
					PX_UNUSED(k);
				}
			}
		}
	}
	return true;
}

PX_COMPILE_TIME_ASSERT(sizeof(Gu::EdgeDescData)==8);
PX_COMPILE_TIME_ASSERT(sizeof(Gu::EdgeData)==8);
bool ConvexHullBuilder::Save(PxOutputStream& stream, bool platformMismatch) const
{
	// Export header
	if(!WriteHeader('C', 'L', 'H', 'L', gVersion, platformMismatch, stream))
		return false;

	// Export header
	if(!WriteHeader('C', 'V', 'H', 'L', gVersion, platformMismatch, stream))
		return false;

	if(!const_cast<ConvexHullBuilder*>(this)->CreateEdgeList())
		return false;

	// Export figures
	writeDword(mHull->mNbHullVertices, platformMismatch, stream);
	writeDword(mHull->mNbEdges, platformMismatch, stream);
	writeDword(ComputeNbPolygons(), platformMismatch, stream);	// Use accessor to lazy-build
	PxU32 Nb=0;
	for(PxU32 i=0;i<mHull->mNbPolygons;i++)
		Nb += mHullDataPolygons[i].mNbVerts;
	writeDword(Nb, platformMismatch, stream);

	// Export triangles

	writeFloatBuffer(&mHullDataHullVertices->x, PxU32(mHull->mNbHullVertices*3), platformMismatch, stream);

	// Export polygons
	// TODO: allow lazy-evaluation
	// We can't really store the buffer in one run anymore!
	for(PxU32 i=0;i<mHull->mNbPolygons;i++)
	{
		Gu::HullPolygonData TmpCopy = mHullDataPolygons[i];
		if(platformMismatch)
			flipData(TmpCopy);

		stream.write(&TmpCopy, sizeof(Gu::HullPolygonData));
	}

	// PT: why not storeBuffer here?
	for(PxU32 i=0;i<Nb;i++)
		stream.write(&mHullDataVertexData8[i], sizeof(PxU8));

	stream.write(mHullDataFacesByEdges8, PxU32(mHull->mNbEdges*2));
	stream.write(mHullDataFacesByVertices8, PxU32(mHull->mNbHullVertices*3));

	return true;
}

static bool computeNewellPlane(PxPlane& plane, PxU32 nbVerts, const PxU8* indices, const PxVec3* verts)
{
	if(!nbVerts || !indices || !verts)
		return false;

	PxVec3 Centroid(0,0,0), Normal(0,0,0);
	for(PxU32 i=nbVerts-1, j=0; j<nbVerts; i=j, j++)
	{
		Normal.x += (verts[indices[i]].y - verts[indices[j]].y) * (verts[indices[i]].z + verts[indices[j]].z);
		Normal.y += (verts[indices[i]].z - verts[indices[j]].z) * (verts[indices[i]].x + verts[indices[j]].x);
		Normal.z += (verts[indices[i]].x - verts[indices[j]].x) * (verts[indices[i]].y + verts[indices[j]].y);
		Centroid += verts[indices[j]];
	}
	plane.n = Normal;
	plane.n.normalize();
	plane.d = -(Centroid.dot(plane.n))/float(nbVerts);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
*	Computes polygon data.
*	\return		true if success
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ConvexHullBuilder::CreatePolygonData()
{
	// Cleanup
	mHull->mNbPolygons = 0;
	PX_DELETE_POD(mHullDataVertexData8);
	PX_DELETE_POD(mHullDataFacesByVertices8);
	PX_FREE_AND_RESET(mHullDataPolygons);

	// Extract polygon data from triangle data
	Container Temp;
	Container Temp2;
	// redundant vertices - degenerated polygons do have vertices which are redundant - inside or on a line between 2 vertices
	// we detect those vertices and remove them
	Container RendundantVertices;
	PxU32 NbPolygons;
	if(!ExtractHullPolygons(NbPolygons, Temp, *this, &Temp2,RendundantVertices))
		return false;

	PxVec3*	 reducedHullDataHullVertices = mHullDataHullVertices;
	PxU8 numReducedHullDataVertices = mHull->mNbHullVertices;

	// Remove the verts from the polygons that are redundant and reindex
	if(RendundantVertices.GetNbEntries() > 0)
	{
		numReducedHullDataVertices = Ps::to8(mHull->mNbHullVertices - RendundantVertices.GetNbEntries());
		reducedHullDataHullVertices = static_cast<PxVec3*> (PX_ALLOC_TEMP(sizeof(PxVec3)*numReducedHullDataVertices,"Reduced vertices hull data"));
		PxU8* remapTable = PX_NEW(PxU8)[mHull->mNbHullVertices];

		PxU8 currentIndex = 0;
		for (PxU8 i = 0; i < mHull->mNbHullVertices; i++)
		{
			if(!RendundantVertices.Contains(i))
			{
				PX_ASSERT(currentIndex < numReducedHullDataVertices);
				reducedHullDataHullVertices[currentIndex] = mHullDataHullVertices[i];
				remapTable[i] = currentIndex;
				currentIndex++;
			}
			else
			{
				remapTable[i] = 0xFF;
			}
		}

		PxU32* Data = Temp.GetEntries();
		for(PxU32 i=0;i<NbPolygons;i++)
		{			
			PxU32 NbVerts = *Data++;
			PX_ASSERT(NbVerts>=3);			// Else something very wrong happened...

			for(PxU32 j=0;j<NbVerts;j++)
			{
				PX_ASSERT(Data[j] < mHull->mNbHullVertices);
				Data[j] = remapTable[Data[j]];
			}

			Data += NbVerts;
		}

		PX_DELETE_POD(remapTable);
	}

	if(NbPolygons>255)
	{
  		Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "ConvexHullBuilder: convex hull has more than 255 polygons!");
		return false;
	}

	// Precompute hull polygon structures
	mHull->mNbPolygons = Ps::to8(NbPolygons);
	mHullDataPolygons = (Gu::HullPolygonData*)PX_ALLOC(sizeof(Gu::HullPolygonData)*mHull->mNbPolygons, PX_DEBUG_EXP("Gu::HullPolygonData"));//PX_NEW(Gu::HullPolygonData)[mHull->mNbPolygons];
	PxMemZero(mHullDataPolygons, sizeof(Gu::HullPolygonData)*mHull->mNbPolygons);

	// The winding hasn't been preserved so we need to handle this. Basically we need to "unify normals"
	// exactly as we did at hull creation time - except this time we work on polygons
	PxVec3 GeomCenter;
	ComputeGeomCenter(GeomCenter);

	// Loop through polygons
	// We have N polygons => remove N entries for number of vertices
	PxU32 Tmp = Temp.GetNbEntries() - NbPolygons;
	mHullDataVertexData8 = PX_NEW(PxU8)[Tmp];
	PxU8* Dest = mHullDataVertexData8;
	const PxU32* Data = Temp.GetEntries();
	const PxU32* TriData = Temp2.GetEntries();
	for(PxU32 i=0;i<NbPolygons;i++)
	{
		mHullDataPolygons[i].mVRef8 = PxU16(Dest - mHullDataVertexData8);	// Setup link for current polygon
		PxU32 NbVerts = *Data++;
		PX_ASSERT(NbVerts>=3);			// Else something very wrong happened...
		mHullDataPolygons[i].mNbVerts = Ps::to8(NbVerts);

		PxU32 index = 0;
		for(PxU32 j=0;j<NbVerts;j++)
		{
			if(Data[j] != 0xFF)
			{
				Dest[index] = Ps::to8(Data[j]);
				index++;
			}
			else
			{
				mHullDataPolygons[i].mNbVerts--;
			}
		}

		// Compute plane equation
		{
			computeNewellPlane(mHullDataPolygons[i].mPlane, mHullDataPolygons[i].mNbVerts, Dest, reducedHullDataHullVertices);

			if(0)
			{
				PxU32 NbTris = *TriData++;		// #tris in current poly
				for(PxU32 k=0;k< NbTris; k++)
				{
					PxU32 TriIndex = *TriData++;	// Index of one triangle composing polygon
					PX_ASSERT(TriIndex<mNbHullFaces);
					const Gu::TriangleT<PxU32>& T = (Gu::TriangleT<PxU32>&)mFaces[TriIndex];
					const PxPlane PL = PlaneEquation(T, mHullDataHullVertices);
					if(k==0 && PL.n.dot(mHullDataPolygons[i].mPlane.n) < 0.0f) 
					{
						negatePlane(mHullDataPolygons[i]);
					}
					//#pragma message("Commmented out by John Ratcliff.  Hitting this assert on the horse ragdoll")
					PX_ASSERT(PL.n.dot(mHullDataPolygons[i].mPlane.n) > 0.0f) ;
				}
			}
			else
			{
				PxU32 NbTris = *TriData++;		// #tris in current poly
				bool flip = false;
				for(PxU32 k=0;k< NbTris; k++)
				{
					PxU32 TriIndex = *TriData++;	// Index of one triangle composing polygon
					PX_ASSERT(TriIndex<mNbHullFaces);
					const Gu::TriangleT<PxU32>& T = (Gu::TriangleT<PxU32>&)mFaces[TriIndex];
					const PxPlane PL = PlaneEquation(T, mHullDataHullVertices);
					if(k==0 && PL.n.dot(mHullDataPolygons[i].mPlane.n) < 0.0f) 
					{
						flip = true;
					}
				}
				if(flip)
				{
					negatePlane(mHullDataPolygons[i]);
					inverseBuffer(mHullDataPolygons[i].mNbVerts, Dest);
				}
			}

			for(PxU32 j=0;j<mHull->mNbHullVertices;j++)
			{
				float d = - (mHullDataPolygons[i].mPlane.n).dot(mHullDataHullVertices[j]);
				if(d<mHullDataPolygons[i].mPlane.d)	mHullDataPolygons[i].mPlane.d=d;
			}
		}

		// "Unify normal"
		if(mHullDataPolygons[i].mPlane.distance(GeomCenter)>0.0f)
		{
			inverseBuffer(mHullDataPolygons[i].mNbVerts, Dest);

			negatePlane(mHullDataPolygons[i]);
			PX_ASSERT(mHullDataPolygons[i].mPlane.distance(GeomCenter)<=0.0f);
		}

		// Next one
		Data += NbVerts;			// Skip vertex indices
		Dest += mHullDataPolygons[i].mNbVerts;
	}

	if(reducedHullDataHullVertices != mHullDataHullVertices)
	{
		PxMemCopy(mHullDataHullVertices,reducedHullDataHullVertices,sizeof(PxVec3)*numReducedHullDataVertices);
		PX_FREE(reducedHullDataHullVertices);

		mHull->mNbHullVertices = numReducedHullDataVertices;
	}

	//calculate the core hull vertex for no scale hull



	//This code validate whether the vertice on the same plane in one of the polygon
	//for(PxU32 j=0; j<NbPolygons; ++j)
	//{    
	//	Gu::HullPolygonData& polygon = mHullDataPolygons[j];
	//	for(PxU32 k=0; k< polygon.mNbVerts; ++k)
	//	{
	//		PxU8 index = mHullDataVertexData8[polygon.mVRef8 + k];
	//		PxVec3 vert = this->mHullDataHullVertices[index];

	//		//Ensure that vert is on plane
	//		PxReal d = polygon.mPlane.n.dot(vert);
	//		PX_ASSERT(PxAbs(d + polygon.mPlane.d) < 0.00001f);
	//	}
	//}


	//calculate the vertex map table
	if(!CalculateVertexMapTable(NbPolygons))
		return false;

#ifdef USE_PRECOMPUTED_HULL_PROJECTION
			// Loop through polygons
			for(PxU32 j=0;j<NbPolygons;j++)
			{
				// Precompute hull projection along local polygon normal
				PxU32 NbVerts = mHull->mNbHullVertices;
				const PxVec3* Verts = mHullDataHullVertices;
				Gu::HullPolygonData& polygon = mHullDataPolygons[j];
				PxReal min = PX_MAX_F32;
				PxU8 minIndex = 0xff;
				for (PxU8 i = 0; i < NbVerts; i++)
				{
					float dp = (*Verts++).dot(polygon.mPlane.n);
					if(dp < min)	
					{ 
						min = dp; 
						minIndex = i; 
					} 
				}
				polygon.mMinIndex = minIndex;
			}
#endif

	// Triangulate newly created polygons to recreate a clean vertex cloud.
	return CreateTrianglesFromPolygons();
}

bool ConvexHullBuilder::ComputeHullPolygons(const PxU32& nbVerts,const PxVec3* verts, const PxU32& nbTriangles, const PxU32* triangles)
{
	PX_ASSERT(triangles);
	PX_ASSERT(verts);

	mHullDataHullVertices			= NULL;
	mHullDataPolygons				= NULL;
	mHullDataVertexData8			= NULL;
	mHullDataFacesByEdges8			= NULL;
	mHullDataFacesByVertices8		= NULL;

	mNbHullFaces					= nbTriangles;
	mHull->mNbHullVertices			= Ps::to8(nbVerts);
	mHullDataHullVertices			= (PxVec3*)PX_ALLOC(sizeof(PxVec3) * mHull->mNbHullVertices, PX_DEBUG_EXP("PxVec3"));
	PxMemCopy(mHullDataHullVertices, verts, mHull->mNbHullVertices*sizeof(PxVec3));
	
	mFaces = PX_NEW(HullTriangleData)[mNbHullFaces];
	for(PxU32 i=0;i<mNbHullFaces;i++)
	{
		PX_ASSERT(triangles[i*3+0]<=0xffff);
		PX_ASSERT(triangles[i*3+1]<=0xffff);
		PX_ASSERT(triangles[i*3+2]<=0xffff);
		mFaces[i].mRef[0] = triangles[i*3+0];
		mFaces[i].mRef[1] = triangles[i*3+1];
		mFaces[i].mRef[2] = triangles[i*3+2];
	}

	Gu::TriangleT<PxU32>* hullAsIndexedTriangle = reinterpret_cast<Gu::TriangleT<PxU32>*>(mFaces);

	// We don't trust the user at all... So, clean the hull.
	PxU32 nbHullVerts = mHull->mNbHullVertices;
	CleanHull(mNbHullFaces, hullAsIndexedTriangle, nbHullVerts, mHullDataHullVertices);
	PX_ASSERT(nbHullVerts<256);
	mHull->mNbHullVertices = Ps::to8(nbHullVerts);

	// ...and then run the full tests again.
	if(!CheckHull(mNbHullFaces, hullAsIndexedTriangle, mHull->mNbHullVertices, mHullDataHullVertices))	return false;
	//return true;	ok here

	// Transform triangles-to-polygons
	if(!CreatePolygonData())	return false;
	//return true;	not ok there!!!

	return CheckHullPolygons();	
}

bool ConvexHullBuilder::CalculateVertexMapTable(PxU32 nbPolygons, bool userPolygons)
{
	mHullDataFacesByVertices8 = PX_NEW(PxU8)[(unsigned int)(mHull->mNbHullVertices*3)];
	bool noPlaneShift = false;
	for(PxU32 i=0; i< mHull->mNbHullVertices; ++i)
	{
		PxU32 count = 0;
		PxU8 inds[3];
		for(PxU32 j=0; j<nbPolygons; ++j)
		{
			Gu::HullPolygonData& polygon = mHullDataPolygons[j];
			for(PxU32 k=0; k< polygon.mNbVerts; ++k)
			{
				PxU8 index = mHullDataVertexData8[polygon.mVRef8 + k];
				if(i == index)
				{
					//Found a polygon
					inds[count++] = Ps::to8(j);
					break;
				}
			}
			if(count == 3)
				break;
		}
		//We have 3 indices
		//PX_ASSERT(count == 3);
		//Do something here
		if(count == 3)
		{
			mHullDataFacesByVertices8[i*3+0] = inds[0];
			mHullDataFacesByVertices8[i*3+1] = inds[1];
			mHullDataFacesByVertices8[i*3+2] = inds[2];
		}
		else
		{
			noPlaneShift = true;
			break;
		}

	}

	if(noPlaneShift)
	{
		//PCM will use the original shape, which means it will have a huge performance drop
		if(!userPolygons)
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "ConvexHullBuilder: convex hull does not have vertex-to-face info! Try to use different convex mesh cooking settings.");
		else
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "ConvexHullBuilder: convex hull does not have vertex-to-face info! Some of the vertices have less than 3 neighbor polygons. The vertex is most likely inside a polygon or on an edge between 2 polygons, please remove those vertices.");
		for(PxU32 i=0; i< mHull->mNbHullVertices; ++i)
		{
			mHullDataFacesByVertices8[i*3+0] = 0xFF;
			mHullDataFacesByVertices8[i*3+1] = 0xFF;
			mHullDataFacesByVertices8[i*3+2] = 0xFF;
		}
		return false;
	}

	return true;
}

bool ConvexHullBuilder::CreateEdgeList()
{
	// Code below could be greatly simplified if we assume manifold meshes!

	// We need the adjacency graph for hull polygons, similar to what we have for triangles.
	// - sort the polygon edges and walk them in order
	// - each edge should appear exactly twice since a convex is a manifold mesh without boundary edges
	// - the polygon index is implicit when we walk the sorted list => get the 2 polygons back and update adjacency graph
	//
	// Two possible structures:
	// - polygon to edges: needed for local search (actually: polygon to polygons)
	// - edge to polygons: needed to compute edge normals on-the-fly

	// Below is largely copied from the edge-list code

	// Polygon to edges:
	//
	// We're dealing with convex polygons made of N vertices, defining N edges. For each edge we want the edge in
	// an edge array.
	//
	// Edges to polygon:
	//
	// For each edge in the array, we want two polygon indices - ie an edge.

	// 0) Compute the total size needed for "polygon to edges"
	PxU32 NbPolygons = ComputeNbPolygons();
	PxU32 NbEdgesUnshared = 0;
	for(PxU32 i=0;i<NbPolygons;i++)
		NbEdgesUnshared += mHullDataPolygons[i].mNbVerts;


	// 1) Get some bytes: I need one EdgesRefs for each face, and some temp buffers
	PxU32*	VRefs0		= PX_NEW_TEMP(PxU32)[NbEdgesUnshared];	// Temp storage
	PxU32*	VRefs1		= PX_NEW_TEMP(PxU32)[NbEdgesUnshared];	// Temp storage
	PxU32*	PolyIndex	= PX_NEW_TEMP(PxU32)[NbEdgesUnshared];	// Temp storage
	PxU32*	VertexIndex	= PX_NEW_TEMP(PxU32)[NbEdgesUnshared];	// Temp storage

	PxU32* Run0 = VRefs0;
	PxU32* Run1 = VRefs1;
	PxU32* Run2 = PolyIndex;
	PxU32* Run3 = VertexIndex;

	// 2) Create a full redundant list of edges
	for(PxU32 i=0;i<NbPolygons;i++)
	{
		PxU32 NbVerts = mHullDataPolygons[i].mNbVerts;
		const PxU8* Data = mHullDataVertexData8 + mHullDataPolygons[i].mVRef8;

		// Loop through polygon vertices
		for(PxU32 j=0;j<NbVerts;j++)
		{
			PxU32 VRef0 = Data[j];
			PxU32 VRef1 = Data[(j+1)%NbVerts];
			if(VRef0>VRef1)	physx::shdfnd::swap(VRef0, VRef1);

			*Run0++ = VRef0;
			*Run1++ = VRef1;
			*Run2++ = i;
			*Run3++ = j;
		}
	}
	PX_ASSERT((PxU32)(Run0-VRefs0)==NbEdgesUnshared);
	PX_ASSERT((PxU32)(Run1-VRefs1)==NbEdgesUnshared);

	// 3) Sort the list according to both keys (VRefs0 and VRefs1)
	RadixSortBuffered Sorter;
	const PxU32* Sorted = Sorter.Sort(VRefs1, NbEdgesUnshared).Sort(VRefs0, NbEdgesUnshared).GetRanks();

	// 4) Loop through all possible edges
	// - clean edges list by removing redundant edges
	// - create EdgesRef list
	mHull->mNbEdges = 0;												// #non-redundant edges
	PxU32*	PolyIndex2		= PX_NEW_TEMP(PxU32)[NbEdgesUnshared];	// Temp storage
	PxU32*	VertexIndex2	= PX_NEW_TEMP(PxU32)[NbEdgesUnshared];	// Temp storage
	PxU32*	EdgeIndex		= PX_NEW_TEMP(PxU32)[NbEdgesUnshared];	// Temp storage
	//	mNbFaces = nbFaces;
	PxU32 PreviousRef0 = PX_INVALID_U32;
	PxU32 PreviousRef1 = PX_INVALID_U32;
	for(PxU32 i=0;i<NbEdgesUnshared;i++)
	{
		PxU32 SortedIndex = Sorted[i];							// Between 0 and Nb
		PxU32 PolyID = PolyIndex[SortedIndex];					// Poly index
		PxU32 VertexID = VertexIndex[SortedIndex];				// Poly index
		PxU32 SortedRef0 = VRefs0[SortedIndex];				// (SortedRef0, SortedRef1) is the sorted edge
		PxU32 SortedRef1 = VRefs1[SortedIndex];

		if(SortedRef0!=PreviousRef0 || SortedRef1!=PreviousRef1)
		{
			// ### TODO: change this in edge list as well
			PreviousRef0 = SortedRef0;
			PreviousRef1 = SortedRef1;

			mHull->mNbEdges++;
		}

		// Create mEdgesRef on the fly
		PolyIndex2[i] = PolyID;
		VertexIndex2[i] = VertexID;
		EdgeIndex[i] = PxU32(mHull->mNbEdges-1);
	}


	//
	Sorted = Sorter.Sort(VertexIndex2, NbEdgesUnshared).Sort(PolyIndex2, NbEdgesUnshared).GetRanks();

	PX_DELETE_POD(mEdgeData16);
	mEdgeData16 = PX_NEW(PxU16)[NbEdgesUnshared];
	for(PxU32 i=0;i<NbEdgesUnshared;i++)	mEdgeData16[i] = Ps::to16(EdgeIndex[Sorted[i]]);

	PX_DELETE_POD(VertexIndex);
	PX_DELETE_POD(PolyIndex);
	PX_DELETE_POD(VRefs1);
	PX_DELETE_POD(VRefs0);


	//////////////////////

	// 2) Get some bytes: one Pair structure / edge
	PX_DELETE_POD(mEdgeToTriangles);
	mEdgeToTriangles = PX_NEW(Gu::EdgeDescData)[mHull->mNbEdges];
	PxMemZero(mEdgeToTriangles, sizeof(Gu::EdgeDescData)*mHull->mNbEdges);

	// 3) Create Counters, ie compute the #faces sharing each edge

	PxU16* Data = mEdgeData16;
	for(PxU32 i=0;i<NbEdgesUnshared;i++)	// <= maybe not the same Nb
	{
		mEdgeToTriangles[*Data++].Count++;
	}

	// if we don't have a manifold mesh, this can fail... but the runtime would assert in any case
	for(PxU32 i=0;i<mHull->mNbEdges;i++)
	{
		if(mEdgeToTriangles[i].Count!=2)
		{
			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Cooking::cookConvexMesh: non-manifold mesh cannot be used, invalid mesh!");
			return false;
		}
	}

	// 3) Create Radix-like Offsets
	mEdgeToTriangles[0].Offset=0;
	for(PxU32 i=1;i<mHull->mNbEdges;i++)	mEdgeToTriangles[i].Offset = mEdgeToTriangles[i-1].Offset + mEdgeToTriangles[i-1].Count;

	PxU32 LastOffset = mEdgeToTriangles[mHull->mNbEdges-1].Offset + mEdgeToTriangles[mHull->mNbEdges-1].Count;

	// 4) Get some bytes for mFacesByEdges. LastOffset is the number of indices needed.
	PX_DELETE_POD(mHullDataFacesByEdges8);
	mHullDataFacesByEdges8 = PX_NEW(PxU8)[LastOffset];

	// 5) Create mFacesByEdges
	Data = mEdgeData16;
	for(PxU32 i=0;i<NbEdgesUnshared;i++)	//<= maybe not the same Nb
	{
		mHullDataFacesByEdges8[mEdgeToTriangles[*Data++].Offset++] = Ps::to8(PolyIndex2[Sorted[i]]);//	... or something
	}

	// 6) Recompute offsets wasted by 5)
	mEdgeToTriangles[0].Offset=0;
	for(PxU32 i=1;i<mHull->mNbEdges;i++)
	{
		mEdgeToTriangles[i].Offset = mEdgeToTriangles[i-1].Offset + mEdgeToTriangles[i-1].Count;
	}

	// ### free temp ram

	PX_DELETE_POD(EdgeIndex);
	PX_DELETE_POD(VertexIndex2);
	PX_DELETE_POD(PolyIndex2);
	PX_DELETE_POD(mEdgeToTriangles);


	// New: edge normals...
	PX_ASSERT(mHullDataPolygons);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
*	Computes the center of the hull. It should be inside it !
*	\param		center	[out] hull center
*	\return		true if success
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ConvexHullBuilder::ComputeGeomCenter(PxVec3& center) const
{
	// Checkings
	const PxVec3* PX_RESTRICT hullVerts = mHullDataHullVertices;
	if(!mHull->mNbHullVertices || !hullVerts)	return false;

	// ### refactor with IndexedSurface

	// Use the topological method
	float TotalArea = 0.0f;
	center = PxVec3(0);
	for(PxU32 i=0;i<mNbHullFaces;i++)
	{
		//const HullTriangle& CurTri = mFaces[i];
		Gu::TriangleT<PxU32> CurTri(mFaces[i].mRef[0], mFaces[i].mRef[1], mFaces[i].mRef[2]);
		const float Area = CurTri.area(hullVerts);
		PxVec3 Center;	CurTri.center(hullVerts, Center);
		center += Area * Center;
		TotalArea += Area;
	}
	center /= TotalArea;

	return true;
}
