//**************************************************************************/
// Copyright (c) 1998-2005 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Interface to MNChamfeData10 class
//***************************************************************************/
#pragma once
#include "export.h"
#include "maxheap.h"
#include "point3.h"
#include "tab.h"

// forward declarations
class InterpolationData;
class TriFaceData;
class MapInterpolationData;
class MNChamferData;
class MNMesh;
class Point3;


//! \brief MNChamferData for use with IMNMeshUtilities10::ChamferEdges.
/*! \sa Class MNChamferData, Class IMNTempData10, Class IMNMeshUtilities10  \n \n

This class enhances MNChamferData. IMNMeshUtilities10::ChamferEdges
requires an MNChamferData10 object. The following code sample demonstrates how to use this class.

Some well-defined pointers:

\code
MNTempData* tempData;
MNMesh* mesh;
\endcode

First, effect the topological change with IMNMeshUtilities10::ChamferEdges:

\code
IMNMeshUtilities10* mesh10 = static_cast<IMNMeshUtilities10*>(mesh->GetInterface(IMNMESHUTILITIES10_INTERFACE_ID));
IMNTempData10* tempData10 = static_cast<IMNTempData10*>(tempData->GetInterface( IMNTEMPDATA10_INTERFACE_ID ));
MNChamferData10& chamferData10 = tempData10->ChamferData();
mesh10->ChamferEdges(MN_SEL, chamferData10, false, 1);
\endcode

After making the topological change, modify the mesh geometry with the desired chamfer amount:

\code
Tab<int> delta;
IMNTempData10* tempData10 = static_cast<IMNTempData10*>(tempData->GetInterface( IMNTEMPDATA10_INTERFACE_ID ));
MNChamferData10& chamferData10 = tempData10->ChamferData();
chamferData10.SetDelta(chamferAmount, delta);
\endcode

Do not do this: 

\code
MNChamferData* chamferData = tempData->ChamferData(); 
chamferData->GetDelta(chamferAmount, delta);
\endcode

*/
class MNChamferData10 : public MaxHeapOperators {
public:
	//! \brief Initializes decorated object. Functions are forwarded to underlying MNChamferData object in_data. 
	//! Some functions append additional functionality. Thus in_data is updated with every function call. 
	//! \param[in] in_data underlying MNChamferData object.
	DllExport MNChamferData10(MNChamferData& in_data);
	DllExport virtual ~MNChamferData10();

	//! Decorator for MNChamferData::GetDelta. This function also interpolates between vertices created
	// on segmented edges.
	DllExport void GetDelta (float in_amount, Tab<Point3>&  out_delta);
	//! Decorator for MNChamferData::setNumVerts.
	DllExport void setNumVerts (int in_nv, bool in_keep=TRUE, int in_resizer=0);
	//! Decorator for MNChamferData::InitToMesh.
	DllExport void InitToMesh (const MNMesh&  in_m);
	//! Decorator for MNChamferData::ClearLimits.
	DllExport void ClearLimits();
	//! Decorator for MNChamferData::GetMapDelta. This function also interpolates between mapping vertices created
	//! on segmented edges. 
	DllExport bool GetMapDelta (MNMesh&  in_mm, int in_mapChannel, float in_amount, Tab<UVVert>&  out_delta);
	//! Decorator for MNChamferData::MDir.
	DllExport Tab<UVVert>& MDir(int in_mapChannel);
	//! Returns MNChamferData::vdir
	DllExport Tab<Point3>& GetVDIR();
	//! Returns MNChamferData::vmax
	DllExport Tab<float>& GetVMAX();
	//! Returns MNChamferData::mdir
	DllExport Tab<UVVert>*& GetMDIR();

private:
	//! Unimplemented Assignment operator
	MNChamferData10& operator=(const MNChamferData10&);
	friend class MNMesh;
	// New functions used by MNMeshUtilities10::ChamferEdges. They don't really make sense in
	// any other context, so they're private.

	// Call before adding vertices with AddNextEdgeInterpolationVertex
	void StartEdgeInterlopation();
	// Adds the next vertex in a sequence to be interpolated. The vertices being interpolated
	// between must be included.
	void AddNextEdgeInterpolationVertex(int in_vertex);
	// Call after the last vertex has been added.
	void FinishEdgeInterpolation();

	// see TriFaceData::TriFaceData
	void StartTriFace(int in_rootVertex, const Point3& in_position, int in_segments, bool in_smoothable);
	// See TriFaceData::AddNextOppositeVertex
	void AddNextOppositeVertex(int in_triIndex, int in_vertex, const Point3& in_position);
	// See TriFacedata::AddNextTriFaceVertex
	void AddNextTriFaceVertex(int in_segmentIndex, int in_vertex);

	// Call before add any mapping vertices. Set in_round to true if the mapping vertices 
	// lie on an original face (i.e., not a quad face)
	void StartMapInterpolation(bool in_round);
	// Adds the next map vertex in a sequence to be interpolated.
	void AddNextMapInterpolationVertex(int in_mapChannel, int in_vertex);
	// Call after adding the last map vertex.
	void FinishMapInterpolation();

	// decorated MNChamferData object
	MNChamferData& m_MNChamferData;

	// sequences of segmented vertices along quad-face edges
	Tab<InterpolationData*> m_interpolationSequences;
	// data pertaining to all triangular faces created during ChamferEdges
	Tab<TriFaceData*> m_triFaces;
	// sequences of segmented texture vertices 
	Tab<MapInterpolationData*> m_mapInterpolationSequences;
};

