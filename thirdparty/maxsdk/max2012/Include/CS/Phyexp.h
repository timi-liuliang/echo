/******************************************************************************
 *<
	FILE: PhyExp.h
				  
	DESCRIPTION:  Export Interface Functionality for Physique

	CREATED BY: John Chadwick with a lot of help from Jurie Horneman

	HISTORY: created July 10, 1997, modified for Version 2.0 March 5, 1998

 *>     Copyright (c) Unreal Pictures, Inc. 1997, 1998 All Rights Reserved.
 *******************************************************************************/
#pragma once

#include "..\maxheap.h"
#include "..\modstack.h"

#ifdef BLD_PHYSIQUE
#define PHYExport __declspec( dllexport )
#else
#define PHYExport __declspec( dllimport )
#endif

// this is the interface ID for a Physique Modifier	Interface
#define I_PHYINTERFACE 0x00100100
#define I_PHYEXPORT	0x00100100 
#define I_PHYIMPORT 0x00100101

//return values for GetInitNodeTM
#define MATRIX_RETURNED		0
#define NODE_NOT_FOUND		1
#define NO_MATRIX_SAVED		2
#define INVALID_MOD_POINTER	3

// version control 
#define CURRENT_VERSION	313


// these are the Class ID defines for Physique Modifier: 
// Physique Class_ID = ClassID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B);
#define PHYSIQUE_CLASS_ID_A 0x00100
#define PHYSIQUE_CLASS_ID_B	0x00000


#define RIGID_TYPE		1
#define	DEFORMABLE_TYPE	2
#define	BLENDED_TYPE	4  	
#define	FLOATING_TYPE	8	

#define RIGID_NON_BLENDED_TYPE		(RIGID_TYPE)
#define	DEFORMABLE_NON_BLENDED_TYPE (DEFORMABLE_TYPE)
#define RIGID_BLENDED_TYPE			(RIGID_TYPE | BLENDED_TYPE)
#define DEFORMABLE_BLENDED_TYPE		(DEFORMABLE_TYPE | BLENDED_TYPE)

// NOTE: for Character Studio 2.1 Deformable Offset Vertices and Vertex Assignment 
// Import has been added.  These classes and descriptions follow the standard Rigid
// Export functionality at the end of this file.


// Using the Physique Export Interface:
// 
//  1.	Find the Physique Modifier you which to export rigid vertex assignments from.
//		(there is a comment at the bottom of this file giving an example of this)
//
//	2.	Given Physique Modifier Modifier *phyMod get the Physique Export Interface:
//		IPhysiqueExport *phyExport = (IPhysqiueExport *)phyMod->GetInterface(I_PHYINTERFACE);		
//
//  3.	For a given Object's INode get this ModContext Interface from the Physique Export Interface:
//		IPhyContextExport *contextExport = (IPhyContextExport *)phyExport->GetContextInterface(INode* nodePtr);
//
//  4.  For each vertex of the Object get the Vertex Interface from the ModContext Export Interface:
//		IPhyVertexExport *vtxExport = (IPhyVertexExport *)contextExport->GetVertexInterface(int i);	
//		NOTE: only Rigid Vertices are currently supported: (see ConvertToRigid(TRUE) to make all vertices rigid)
//		IPhyRigidVertex *rigidExport = (IPhyRigidVertex *)contextExport->GetVertexInterface(int i);
//
//  5.	Get the INode for each Rigid Vertex Interface:
//		INode *nodePtr = rigidExport->GetNode();
//
//  6.	Get the Point3 Offset Vector in INode coordinates for each Rigid Vertex Interface:
//		Point3 offsetVector = rigidExport->GetOffsetVector();





// IPhyVertexExport: this is the base class for Vertex Export Interface
//					 NOTE: currently only RIGID_TYPE vertices are supported (IPhyRigidVertex)
//					 When a vertex is not assigned Rigid in Physique, the VertexInterface will be NULL
//					 Unless you call IPhyContextExport->ConvertToRigid(TRUE) (see IPhyContextExport below)
//					 With ConvertToRigid(TRUE) you will always get a IPhyRigidVertex 
//					 from IPhyContextExport->GetVertexInterface(i) 

class IPhyFloatingVertex;

class IPhyVertexExport: public MaxHeapOperators
{
	public:

		PHYExport virtual ~IPhyVertexExport() {}

		// NOTE: currently only type RIGID_TYPE | RIGID_BLENDED_TYPE are supported
		PHYExport virtual int GetVertexType() = 0;

		PHYExport virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) = 0;
		
};



class IPhyRigidVertex : public IPhyVertexExport
{
	public:

		PHYExport virtual ~IPhyRigidVertex() {}

		// GetNode() will return the INode pointer of the link of the given VertexInterface
		PHYExport virtual INode *GetNode() = 0;

		// GetOffsetVector() will return the coordinates of the vertex 
		// in the local coordinates of associated INode pointer from GetNode
		// this is NOT THE SAME as the .vph file coordinates. (It is simpler)
		// the world coordinates of the vertex have been transformed by the Inverse	of the INode pointer.
		PHYExport virtual Point3 GetOffsetVector() = 0;

		PHYExport virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) = 0;
		
};

class IPhyBlendedRigidVertex : public IPhyVertexExport
{
	public:

		PHYExport virtual ~IPhyBlendedRigidVertex() {}

		// GetNumberNodes() returns the number of nodes assigned to the given VertexInterface
		PHYExport virtual int GetNumberNodes() = 0;

		// GetNode(i) will return the i'th INode pointer of the link of the given VertexInterface
		PHYExport virtual INode *GetNode(int i) = 0;

		// GetOffsetVector(i) will return the coordinates of the vertex 
		// in the local coordinates of associated i'th INode pointer from GetNode(i)
		// this is NOT THE SAME as the .vph file coordinates. (It is simpler)
		// the world coordinates of the vertex have been transformed by the Inverse	of the INode pointer.
		PHYExport virtual Point3 GetOffsetVector(int i) = 0;

		// GetWeight(i) will return the weight of the vertex associated with the i'th Node
		// pointer from GetNode(i)
		PHYExport virtual float GetWeight(int i) = 0;

		PHYExport virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) = 0;

};





// IPhyContextExport: This class can be returned by :
// passing the INode pointer of an Object with the given Physique Modifier's IPhysiqueExport::GetContextInterface(node)
// If this Node does contain the Physique Modifier associated with this interface an interface to this object's
// interface is returned (several object's may share the same physique modifier), else returns NULL.

class IPhyContextExport: public MaxHeapOperators
{
	public:

		PHYExport virtual ~IPhyContextExport() {}
		
		// this returns the number of vertices for the given modContext's Object
		PHYExport virtual int GetNumberVertices() = 0;

		// If ConvertToRigid is set to TRUE, all GetVertexInterface calls will be IPhyRigidVertex OR IPhyBlendedRigidVertex.
		// Vertices which are not Rigid in Physique will be converted to Rigid for the VertexInterface.
		// When ConvertToRigid is set to FALSE, GetVertexInterface for non-Rigid vertices currently returns NULL.
		// By default, if you do not call ConvertToRigid, it is set to FALSE.
		PHYExport virtual void ConvertToRigid(BOOL flag = TRUE) = 0;

		// If AllowBlending is set to FALSE then GetVertexInterface will return a non-blended subclass.
		// Currently the only valid VertexInterface subclasses are IPhyRigidVertex, and IPhyBlendedRigidVertex.
		// When AllowBlending is FALSE, either IPhyRigidVertex or NULL will be returned.
		// If ConvertToRigid is TRUE and AllowBlending is FALSE, all vertices will return
		// IPhyRigidVertex. By default AllowBlending is set to TRUE.
		PHYExport virtual void AllowBlending(BOOL flag = TRUE) = 0;

		// GetVertexInterface return's a VertexInterface (IPhyVertexExport *) for the i'th vertex.
		// If ConvertToRigid has been set to TRUE, then all VertexInterfaces will return IPhyRigidVertex OR IPhyBlendedRigidVertex.
		// When ConvertToRigid has been set to FALSE, non-Rigid vertices will return NULL (CURRENTLY).
		PHYExport virtual IPhyVertexExport *GetVertexInterface(int i) = 0;

		// You must call ReleaseVertexInterface to delete the VertexInterface when finished with it.
		PHYExport virtual void ReleaseVertexInterface(IPhyVertexExport *vertexExport) = 0;

		PHYExport virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) = 0;

		PHYExport virtual IPhyFloatingVertex *GetFloatingVertexInterface(int i) = 0;
};

// IPhysiqueExport: This class can be returned by calling the method GetInterface() for a Physique Modifier
// given Modifier *mod points to a Physique Modifier, then:
// IPhysiqueExport *phyInterface = (IPhysiqueExport *)( mod->GetInterface(I_PHYINTERFACE) );
// will return the interface for this Physique Modifier, else returns NULL.

class IPhysiqueExport: public MaxHeapOperators
{

	public:

		PHYExport virtual ~IPhysiqueExport() {}

		//GetContextInterface will return a pointer to IPhyContextExport Interface for a given INode.
		// Ff the given INode does not contain the Physique Modifier of this IPhysique Export then NULL is returned.
		PHYExport virtual IPhyContextExport *GetContextInterface(INode* nodePtr) = 0;

		// You must call ReleaseContextInterface to delete the COntextInterface when finished with it.
		PHYExport virtual void ReleaseContextInterface(IPhyContextExport *contextExport) = 0;

		PHYExport virtual int GetInitNodeTM(INode *node, Matrix3 &initNodeTM) = 0;

		PHYExport virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) = 0;

		PHYExport virtual int Version(void) = 0;

		PHYExport virtual void SetInitialPose(bool set) = 0;
};

// example code to find if a given node contains a Physique Modifier
// DerivedObjectPtr requires you include "modstack.h" from the MAX SDK
/*
Modifier* FindPhysiqueModifier (INode* nodePtr)
{
	Object* ObjectPtr = nodePtr->GetObjectRef();

	if (!ObjectPtr ) return NULL;
	
	while (ObjectPtr->SuperClassID() == GEN_DERIVOB_CLASS_ID && ObjectPtr)
	{
			// Yes -> Cast.
			IDerivedObject* DerivedObjectPtr = (IDerivedObject *)(ObjectPtr);				
			
		// Iterate over all entries of the modifier stack.
		int ModStackIndex = 0;

		while (ModStackIndex < DerivedObjectPtr->NumModifiers())
		{
			// Get current modifier.
			Modifier* ModifierPtr = DerivedObjectPtr->GetModifier(ModStackIndex);

			// Is this Physique ?
			if (ModifierPtr->ClassID() == Class_ID(PHYSIQUE_CLASS_ID_A, PHYSIQUE_CLASS_ID_B))
			{
				// is this the correct Physique Modifier based on index?
				SkinMod *phyMod = (SkinMod *)ModifierPtr;
				
				if (phyMod == mod)
				{
					ModContext *mc = DerivedObjectPtr->GetModContext(ModStackIndex);
				}
			}

				ModStackIndex++;
		}

		ObjectPtr = DerivedObjectPtr->GetObjRef();
	}

	// Not found.
	return NULL;
}
*/


// Deformable Vertex Export via Offset Vectors
//
// Character Studio 2.1 allows users to export Deformable Vertices
// as time frame offsets from a rigid node offset
// The Rigid Offset Vector is identical to the Rigid Vertex Offset Vector.
// This represents the vertex in local coordinates from the attached node.
// Note that since the deformable offset is relative to a single node,
// Blending between links is handled internal to Physique in determining
// this offset, so that the export is a single offset for a given time
// for a single Node.
// Basically, you export a deformable offset from a single node, while
// Physique uses blending between several deformable links to determine
// this offset.
// NOTE: while the Rigid Vertex and Rigid Blended Vertex Export require
// a one time export for replicating the deformabion internal to the game engine,
// the Deformable Offset Vertex Export requires the export of DeformVertexOffset(t)
// at several (or every) frame.

class IPhyDeformableOffsetVertex : public IPhyVertexExport
{
	public:

		PHYExport virtual ~IPhyDeformableOffsetVertex() {}

		// GetNode() will return the INode pointer of the link of the given VertexInterface
		PHYExport virtual INode *GetNode() = 0;

		// GetOffsetVector() will return the coordinates of the undeformed vertex 
		// in the local coordinates of associated INode pointer from GetNode().
		// This is IDENTICAL to the Rigid Non Blended OffsetVector.
		// It represents the Vertex in the local coordinates of the node via GetNode().
		PHYExport virtual Point3 GetOffsetVector() = 0;

		// GetDeformOffsetVector(t) will return the coorinates of the deformed vertex
		// in the local coordinates of the associated INode pointer from GetNode().
		// Game Developers wishing to store relative offsets can subtract the
		// Deformable Offsets for given frames from the Base Offset Vector.
		PHYExport virtual Point3 GetDeformOffsetVector(TimeValue t) = 0;

		PHYExport virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) = 0;
		
};





// Import Interface Functionality for Physique
//
// Character Studio 2.1 allows users to programatically set the Rigid
// Vertex Assignments (with or without Blending) via Import Interface


// IPhyVertexImport: this is the base class for Vertex Export Interface
//					 NOTE: currently only RIGID_TYPE vertices are supported (IPhyRigidVertex)
//					 When a vertex is not assigned Rigid in Physique, the VertexInterface will be NULL
//					 Unless you call IPhyContextExport->ConvertToRigid(TRUE) (see IPhyContextExport below)
//					 With ConvertToRigid(TRUE) you will always get a IPhyRigidVertex 
//					 from IPhyContextExport->GetVertexInterface(i) 

class IPhyVertexImport: public MaxHeapOperators
{
	public:

		PHYExport virtual ~IPhyVertexImport() {}

		// NOTE: currently only type RIGID_TYPE | RIGID_BLENDED_TYPE are supported
		PHYExport virtual int GetVertexType() = 0;

		// You can lock (TRUE) or unlock (FALSE) vertex assignments
		// To avoid changes to manual vertex assignments locking prevents
		// these vertices from being reinitialized by Physique.
		PHYExport virtual void LockVertex(BOOL lock) = 0;

		PHYExport virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) = 0;
};



class IPhyRigidVertexImport : public IPhyVertexImport
{
	public:

		PHYExport virtual ~IPhyRigidVertexImport() {}

		// SetNode() will define the INode pointer of the link of the given Rigid Vertex Interface
		// If this INode is a valid Physique Link SetNode returns TRUE, and this vertex is
		// now assigned Rigid (NonBlended) to this INode.
		// If this INode is not a valid Physique Link, then SetNode returns FALSE,
		// and no change to the vertex assignment occurs.
		PHYExport virtual BOOL SetNode(INode *nodePtr) = 0;

		PHYExport virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) = 0;

};

class IPhyBlendedRigidVertexImport : public IPhyVertexImport
{
	public:

		PHYExport virtual ~IPhyBlendedRigidVertexImport() {}

		// SetWeightedNode will assign the given vertex to the nodePtr with the weight specified.
		// When init = TRUE, this will remove all existing asignments and define this nodePtr
		// as the first and only nodePtr assigned to the given vertex.  When init = FALSE this
		// nodePtr and weight are appeneded to the current assignments.
		// Always set init = TRUE for the first nodePtr and weight for a given vertex
		// and init = FALSE for all additional 2nd thru Nth nodeptr and weights.
		// SetWeightedNode returns the number of Nodes assigned to the given Vertex
		// when set successfully, or when the given nodePtr is not a valid Physique Link
		// SetWeightedNode returns 0.
		PHYExport virtual int SetWeightedNode(INode *nodePtr, float weight, BOOL init = FALSE) = 0;

		PHYExport virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) = 0;

};



class IPhyContextImport: public MaxHeapOperators
{
	public:

		PHYExport virtual ~IPhyContextImport() {}
		
		// this returns the number of vertices for the given modContext's Object
		PHYExport virtual int GetNumberVertices() = 0;

		// SetVertexInterface return's a VertexInterface (IPhyVertexImport *) for the i'th vertex.
		// type = RIGID_NON_BLENDED_TYPE | RIGID_BLENDED_TYPE are currently supported.
		// Any other value for type will return NULL.
		
		PHYExport virtual IPhyVertexImport *SetVertexInterface(int i, int type) = 0;

		// You must call ReleaseVertexInterface to delete the VertexInterface when finished with it.
		PHYExport virtual void ReleaseVertexInterface(IPhyVertexImport *vertexImport) = 0;

		PHYExport virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) = 0;

};


class IPhysiqueImport: public MaxHeapOperators
{
	public:
		
		PHYExport virtual ~IPhysiqueImport() {}

		//GetContextInterface will return a pointer to IPhyContextImport Interface for a given INode.
		// if the given INode does not contain the Physique Modifier of this IPhysique Import then NULL is returned.
		PHYExport virtual IPhyContextImport *GetContextInterface(INode* nodePtr) = 0;

		// You must call ReleaseContextInterface to delete the ContextInterface when finished with it.
		PHYExport virtual void ReleaseContextInterface(IPhyContextImport *contextImport) = 0;

		// You call AttachRootNode to define the root node of the Physique Modifier
		// This will create default vertex assignments.
		// You MUST call AttachRootNode prior to any Imported Vertex Assignments
		// This method is designed specifically for developers wanting to 
		// define Physique fully via import.  Predictable results should only be
		// expected when this call is followed by a series of Vertex Import calls
		// for every ModContext and Vertex of the Modifier.
		// If the attach is successful it returns TRUE, otherwise it returns FALSE.
		// TimeValue t specifies the initial skeleton pose, the position of the
		// skeleton relative to the Object Geometry.
		PHYExport virtual BOOL AttachRootNode(INode *nodePtr, TimeValue t) = 0;

		PHYExport virtual BOOL InitializePhysique(INode *nodePtr, TimeValue t) = 0;

		PHYExport virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) = 0;
		
};

class IPhyFloatingVertex : public IPhyVertexExport
{
	public:

		PHYExport virtual ~IPhyFloatingVertex() {}

		PHYExport virtual int GetNumberNodes() = 0;
		PHYExport virtual INode *GetNode(int i) = 0;
		PHYExport virtual float GetWeight(int i, float &totalweight) = 0;
		PHYExport virtual Point3 GetOffsetVector(int i) = 0;
		PHYExport virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) = 0;

		// GetNumberNodes() returns the number of nodes (bones) assigned to the given Vertex
		
		// GetNode(i) will return the INode pointer of the ith node assigned to
		//this Vertex

		// GetOffsetVector(i) will return the coordinates of the vertex 
		// in the local coordinates of associated i'th INode pointer from GetNode(i)
		// this is NOT THE SAME as the .vph file coordinates. (It is simpler)
		// the world coordinates of the vertex have been transformed by the Inverse	of the INode pointer.
		
		// GetWeight(i) will return the normalized weight of the vertex associated with the i'th Node
		// pointer from GetNode(i)
	
};




