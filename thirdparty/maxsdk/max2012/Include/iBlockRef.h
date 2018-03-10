	 /**********************************************************************
 
	FILE: IBlockRef.h

	DESCRIPTION:  Public intefaces that defines the concept of block references

	CREATED BY: Attila Szabo, Discreet

	HISTORY: - created Aug 06, 2002

 *>	Copyright (c) 1998-2002, All Rights Reserved.
 **********************************************************************/
#pragma once
#include "inode.h"
#include "iFnPub.h"
#include "maxtypes.h"


// "Block" is a generic term for one or more objects that are combined 
// to create a single object. A set of combined objects stored in a block 
// table are called block definitions while a block reference is the object 
// that references a block definition and is inserted in the scene.

// This interface models a component of a block reference
class IBlockRefComponent : public FPMixinInterface 
{
	public:
		// --- Operations on block components
		virtual const MCHAR* GetName() const = 0;

		// --- Block management
		virtual bool Add(IBlockRefComponent* component) = 0;
		virtual bool Remove(IBlockRefComponent* component) = 0;
		virtual unsigned long NumComponents() const = 0;
		virtual unsigned long GetComponents(Tab<IBlockRefComponent*>& components) const = 0;
		virtual void ExpandSelection(Tab<INode*>& nodesToSelect) const = 0;
		// Returns true if an instance of this block component needs a selection 
    // box displayed around it when selected. Otherwise returns false
		virtual bool IsSelectionBoxNeeded() const = 0;
		
		// --- Block data I/O 
		virtual IOResult Save(ISave* isave) const = 0;
		virtual IOResult Load(ILoad* iload) = 0;
}; 


// Interface ID
#define BLOCKREF_COMPONENT_INTERFACE Interface_ID(0x7f1f2104, 0x7ab7322b)
#define ADD_BLOCKREF_COMPONENT_INTERFACE Interface_ID(0x4c794c3a, 0x3a693dbb)
#define REMOVE_BLOCKREF_COMPONENT_INTERFACE Interface_ID(0x379c7c13, 0x47a32e07)

// Utilities:
// Client code has to make nodes either block components.
// This is accomplished by calling IBlockMgr::MakeBlockRefComponent on a node instance.

inline IBlockRefComponent* GetBlockRefComponent(INode& n) 
{
	return static_cast<IBlockRefComponent*>(n.GetInterface(BLOCKREF_COMPONENT_INTERFACE)); 
}


