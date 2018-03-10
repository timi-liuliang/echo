/**********************************************************************
*<
    FILE: RadiostiyMesh.h

    DESCRIPTION: Exposes the radiosity meshes for use by other clients.

	CREATED BY: Cleve Ard, Discreet Development

	HISTORY: 8/6/02

*>	Copyright (c) 2002, All Rights Reserved. 
 **********************************************************************/

#pragma once

#include "radiosity.h"
#include "iFnPub.h"

#define RADIOSITY_MESH_INTERFACE_ID Interface_ID(0x3b004730, 0x7957272c)

class RenderGlobalContext;
class Matrix3;

#pragma warning(push)
#pragma warning(disable:4239)

class RadiosityMesh : public FPMixinInterface {
public:

    // Does the solution exist. The solution doesn't exist until we
    // have actually tried to solve it. CreateMeshes below will
    // create the meshes and the can be retrieved, but they won't
    // have any useful lighting data in them.
    virtual bool DoesSolutionExist() = 0;

	// Determine whether the radiosity plugin has a mesh for the INode node.
	virtual bool DoesMeshExist(INode* node) = 0;

	// Determine whether the radiosity plugin thinks its mesh for the
	// INode node is valid at time t.
	//
	// If rendering is true, a mesh will be considered invalid, if the
	// number of faces in it is too large compared to the number of faces
	// in the original object. Too large is defined by the ratio of the number
	// of faces in the radiosity mesh and the number of faces in the original
	// object. If this ratio is bigger than 1.5 then the radiosity mesh is
	// too large. This ratio is can be controlled by a setting in
	// plugcfg/DiscreetRadiosity.ini:
	//
	//  [Radiosity]
	//  ShareMeshes=1.5
	virtual bool IsMeshValid(TimeValue t, INode* node, bool rendering) = 0;

	// Get the Validity for the mesh. If the radiosity plugin cannot
	// determine the validity at time t, it returns the NEVER.
    virtual ::Interval GetValidity(TimeValue t, INode* node) = 0;

	// Create the meshes. This can be called to force the radiosity
	// plugin to update its meshes at time t. Once the meshes are
	// updated, they will not be deleted when radiosity is solved;
	// the light is added to the existing mesh. This method is used
	// by the renderer to create the meshes before the radiosity
	// solution is calculated.
	virtual bool CreateMeshes(TimeValue t, RenderGlobalContext* rgc) = 0;

	// Get the mesh for a node. Returns true if the mesh exists.
	// Also restricts validity to the validity interval for the mesh.
	// If the mesh doesn't exist, the validity is not changed.
	// The address of the mesh is returned in mesh. You shouldn't
	// keep this address for an extended period. The mesh could
	// be deleted when the radiosity solution calculates a new
	// solution. There is no instancing for meshes in the radiosity
	// solution. Each instance in MAX is assigned a unique mesh
	// in the solution.
	virtual bool GetMesh(INode* node, Mesh*& mesh,
		::Interval& validity = ::Interval(0,0)) = 0;

	// Get the TM for the node. Returns true if the mesh exists.
	// Also restricts validity to the validity interval for the TM.
	// If the mesh doesn't exist, the validity is not changed.
	// TM is the mesh to world space transform. The radiosity
	// solution does not keep track of object to world transforms.
	// The transform returned is usually just a scale to translate
	// the radiosity plugins units, meters, to the MAX units.
	virtual bool GetMeshTM(INode* node, Matrix3& TM,
		::Interval& validity = ::Interval(0,0)) = 0;

	// Get the mesh and TM for a node. Returns NULL if the mesh doesn't
	// exist. TM is the mesh to world space transform (See GetMeshTM).
	// Also restricts meshValid to the validity interval for the mesh,
	// and tmValid to the validity interval for the TM. If the mesh
	// doesn't exist, neither validity interval is changed.
	virtual Mesh* GetMeshAndTM(INode* node, Matrix3& TM,
		::Interval& meshValid = ::Interval(0,0),
		::Interval& tmValid = ::Interval(0,0)) = 0;
};

#pragma warning(pop)

inline RadiosityMesh* GetRadiosityMesh(RadiosityEffect* rad)
{
	return static_cast<RadiosityMesh*>(rad->GetInterface(
		RADIOSITY_MESH_INTERFACE_ID));
}

