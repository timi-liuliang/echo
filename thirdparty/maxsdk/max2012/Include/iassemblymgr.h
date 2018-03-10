 /**********************************************************************
 
	FILE: IAssemblyMgr.h

	DESCRIPTION:  Public interface for defining and working with assemblies

	CREATED BY: Attila Szabo, Discreet

	HISTORY: - created April 03, 2001

 *>	Copyright (c) 1998-2000, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "iFnPub.h"
#include "iassembly.h"
#include "GetCOREInterface.h"

// forward declarations
class INode;
class INodeTab;
class HitByNameDlgCallback;
class Box3;

// class IAssemblyMgr
//
// This interface allows for working with assemblies and its methods are
// implemented by the system (Max). One can get this interface by calling 
// IAssemblyMgr* GetAssemblyMgr();
//
// Assemblies are very similar to groups. The main differences are:
// - As opposed to a group head object, an assembly head can be any object, 
//   not just a dummy object
// - The base object of an assembly head node can be edited in the modify panel
class IAssemblyMgr : public FPStaticInterface 
{
	public:
		// Creates an assembly from the supplied nodes. 
		// If called with the default parameters, the user is asked to provide 
		// a name and pick a head object for the assembly. After creation the 
		// assembly is selected.
		//
		// PARAMETERS:
		// const INodeTab* const nodes	
		//   List of nodes to assemble. Only those nodes which can be assembled are
		//   assembled, the others are not taken into account. If no nodes are supplied
		//   (NULL) the current selection set is used.
		//
		// const MCHAR* name			
		//   The name of the assembly. If not supplied the user
		//   is presented with a unique assembly name that he can change.
		//
		// const ClassDesc* cd		
		//   Describes the head object type. 
		//   The head object must be a helper object.
		//   If a ClassDesc is not supplied, the user is asked to choose a head object
    //   from a list of objects
		//
		// bool select 
		//   Controls whether the assembly is selected or not after creation
		//
		// RETURNS:
		//   Returns a pointer to the assembly node created or NULL if none of the
		//   nodes could be assembled or something went wrong.
		virtual INode*	Assemble(
			const INodeTab* const nodes = NULL, 
			const MCHAR* name = NULL, 
			const ClassDesc* const cd = NULL,
			bool select = true) = 0;

		// Destroys the assemblies found in the given set of nodes. 
		// - Deletes the assembly head node and base object
		// - The members of the assembly are not destroyed
		// - If no list is supplied (NULL) the current selection set is used
		//
		// RETURNS:
		//   True on success   
		virtual bool		Disassemble(const INodeTab* const nodes = NULL) = 0;
		
		// Explodes the assemblies found in the given set of nodes. 
		// As opposed to Disassemble, it acts recursively on assemblies
		// (destroys nested assemblies). If no nodes are supplied (NULL) the 
		// current selection set is used
		//
		// RETURNS:
		//   True on success   
		virtual bool		Explode(const INodeTab* const nodes = NULL) = 0;
		
		// Puts the assemblies found in the given set of nodes
		// in a state where the users can access their members 
		// - It doesn't act recursively on assemblies.
		//
		// PARAMETERS:
		// const INodeTab* const nodes	
		// - Assembly head nodes to open. If no nodes are supplied (NULL) 
		// the current selection set is used
		//
		// bool clearSelection
		// - If TRUE, the nodes will be unselected after the operation is completed.
		//
		// RETURNS:
		//   True on success   
		virtual bool		Open(const INodeTab* const nodes = NULL, bool clearSelection = true) = 0;
		
		// Opposite of Open
		virtual bool		Close(const INodeTab* const nodes = NULL, bool select = true) = 0;

		// Attaches the supplied nodes to the given assembly 
		// - Only nodes that are attach-able are attached
		// - If called with the default parameters, the user is asked to provide
		// the assembly to which the nodes are to be attached
		//
		// PARAMETRS:
		// const INodeTab* const nodes	
		//   List of nodes to attach to an assembly. Only those nodes 
		//   which can be attached are attached. If no nodes are supplied (NULL) 
		//   the current selection set is used
		//
		// const INode* const assembly
		//   Pointer to a member or head node of the assembly to which the nodes
		//   should be attached. If none is supplied the user is asked to pick an
		//   assembly to which the nodes will be attached.
		//
		// RETURNS:
		//   Returns false if none of the nodes could be attached or if something went wrong
		virtual bool		Attach(const INodeTab* const nodes = NULL, const INode* const assembly = NULL) = 0;

		// Detaches the given nodes from the assembly they belong to.
		// If no nodes are supplied (NULL) the current selection set is used.
		//
		// RETURNS:
		//   Returns false if none of the nodes could be detached or if something went wrong
		virtual bool		Detach(const INodeTab* const nodes = NULL) = 0;

		//
		// --- Helper methods		
		//

		// Checks whether the nodes to be assembled comply with these rules:
		// - Nodes must have the scene root as their common and direct parent OR
		// their parent must be in the list of nodes to be assembled
		// - If the common parent node is an assembly head node, and has some children 
		// that are members, and others that aren't, then the candidate nodes for 
		// assembling must be all members, or all non-members.
		// - Closed member nodes are ignored and their parent assembly node 
		// is added to the new assembly. 
		// - In case of nodes that are already members of an assembly, their new
		// assembly node will become a member of that assembly.
		// - If no nodes are supplied (NULL) the current selection set is used.
		//
		// DEFAULTS:
		//   Operates on active selection set
		//
		// RETURNS:
		//   Returns true only if all nodes can be assembled, otherwise returns false.
		virtual bool	CanAssemble(const INodeTab* const nodes = NULL) = 0;

		// Checks whether the nodes comply with these rules:
		// - Only assembly closed heads can be disassembled
		// - Assembly heads which are members of another assembly can be
		// disassembled only if the enclosing assembly is open.
		// - If no nodes are supplied (NULL) the current selection set is used.
		//
		// DEFAULTS:
		//   Operates on active selection set
		//
		// RETURNS:
		//   Returns true if at least one assembly is found that complies with the
		//   rules above.
		virtual bool	CanDisassemble(const INodeTab* const nodes = NULL) = 0;

		// Checks whether the nodes comply with these rules:
		// - There has to be at least one closed assembly head selected
		// - If no nodes are supplied (NULL) the current selection set is used.
		//
		// DEFAULTS:
		//   Operates on active selection set
		//
		// RETURNS:
		//   Returns false if none of the given nodes pass the test
		virtual bool	CanExplode(const INodeTab* const nodes = NULL) = 0;

		// Checks whether the nodes comply with these rules:
		// - There has to be at least one closed assembly head selected
		// - If no nodes are supplied (NULL) the current selection set is used.
		//
		// DEFAULTS:
		//   Operates on active selection set
		//
		// RETURNS:
		//   Returns false if none of the given nodes pass the test
		virtual bool	CanOpen(const INodeTab* const nodes = NULL) = 0;

		// Checks whether the nodes comply with these rules:
		// - There has to be at least one open assembly head or member selected
		// - If no nodes are supplied (NULL) the current selection set is used.
		//
		// DEFAULTS:
		//   Operates on active selection set
		//
		// RETURNS:
		//   Returns false if none of the given nodes pass the test
		virtual bool	CanClose(const INodeTab* const nodes = NULL) = 0;

		// Checks whether the nodes comply with these rules:
		// - Nodes must be good for assembling 
		// - Nodes should not contain open assembly members
		// - If an assembly is not specified, there has to be an assembly 
		// in the current scene that the given nodes are not part of 
		//
		// PARAMETRS:
		// const INodeTab* const nodes	
		//   List of nodes to attach to an assembly. If no nodes are supplied (NULL) 
		//   the current selection set is used.
		//
		// const INode* const assembly
		//   Pointer to a member or head node of an assembly to attach the nodes to.
		//   If none supplied (NULL) the user has to pick one.
		//
		// DEFAULTS:
		//   Operates on active selection set
		//
		// RETURNS:
		//   Returns false if none of the given nodes can be attached
		//   or there's no assembly to attach the nodes to
		virtual bool	CanAttach(const INodeTab* const nodes = NULL, const INode* const assembly = NULL) = 0;

		// Checks whether the nodes comply with these rules:
		// - Nodes must each have a parent that is an open assembly member
		// - If no nodes are supplied (NULL) the current selection set is used.
		//
		// DEFAULTS:
		//   Operates on active selection set
		//
		// RETURNS:
		//   Returns false if none of the given nodes pass the test
		virtual bool	CanDetach(const INodeTab* const nodes = NULL) = 0;

		// Checks if all nodes in the given array belong to the same level (outer) assembly. 
		//
		// PARAMETRS:
		// const INodeTab* const nodes	
		//	List of nodes check. If no nodes are provided (NULL) the method works on the current selection.
		//
		// RETURNS:
		//	Pointer to the assembly head node - If the array of nodes passed in as parameter belong 
		//	to the same closed assembly. Nodes that are part of an inner assembly are not 
		//	taken into account, are ignored, they are considered as just one node in the outer assembly.
		//	NULL - If the array of nodes passed in as parameter contains no assembly, 
		//	more than one high level assembly or not all nodes are members of the same 
		//	high level\outer assembly.
		virtual INode*	IsAssembly(const INodeTab* const nodes = NULL) = 0;

		//	Filters the members of a given assembly based on a given object type.
		//	This method does not iterate through the nodes of inner closed assemblies.
		//	If the geometry pipeline associated with the assembly member node contains a
		//	modifier or base object with the super class ID specified as filtering criteria
		//	that node is saved into the filteredNodes output parameter.
		//
		// PARAMETRS:
		//	const INode* const assemblyHead - An assembly head node. Can be open or closed assembly head. 
		//	const SClass_ID filterSClassID	- Superclass ID describing the type wanted to be fileterd
		//	INodeTab&	filteredNodes					- The filtered assembly members. This is an output parameter
		//
		// RETURNS:
		//	An array of nodes. The array contains the nodes that are members of the 
		//	assembly parameter and which pass	the filter criteria. Nodes of inner 
		//	assemblies will not taken into account by this method.
		virtual bool	FilterAssembly(
			const INode* const assemblyHead, 
			const SClass_ID filterSClassID, 
			INodeTab& filteredNodes) = 0;

		// Returns a pointer to a hit by name dialog class
		virtual const HitByNameDlgCallback* GetHitByNameDlgCB() const = 0;

		// Calculates the world bounding box of the whole assembly and 
    // transforms it into the assembly head node's space
		virtual void  GetWorldBBox(TimeValue t, INode* assemblyHead, Box3& worldBBox) const = 0;

		// Function IDs for function publishing
		enum 
		{ 
			E_ASSEMBLE, 
			E_DISASSEMBLE, 
			E_OPEN, 
			E_CLOSE, 
			E_EXPLODE, 
			E_ATTACH, 
			E_DETACH, 
			E_CAN_ASSEMBLE, 
			E_CAN_DISASSEMBLE, 
			E_CAN_EXPLODE, 
			E_CAN_OPEN, 
			E_CAN_CLOSE, 
			E_CAN_ATTACH, 
			E_CAN_DETACH, 
			E_IS_ASSEMBLY,
			E_FILTER_ASSEMBLY,
		}; 

}; 


// Assembly Manager interface ID
#define ASSEMBLY_MGR_INTERFACE Interface_ID(0x576c528f, 0x5cc23607)
inline IAssemblyMgr* GetAssemblyMgr() { return static_cast<IAssemblyMgr*>(GetCOREInterface(ASSEMBLY_MGR_INTERFACE)); }


