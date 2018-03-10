 /**********************************************************************
 
	FILE: IBlockMgr.h

	DESCRIPTION:  Public interface for working with blocks (the same 
								concept as Autocad Blocks)

	CREATED BY: Attila Szabo, Discreet

	HISTORY: - created Aug 08, 2002

 *>	Copyright (c) 1998-2002, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "iFnPub.h"
#include "maxtypes.h"
#include "GetCOREInterface.h"

// --- Forward declaration
class IBlockRefComponent;
class INode;
class INodeTab;

// --- Interface IDs
#define BLOCK_MGR_INTERFACE Interface_ID(0x327d3c71, 0x542b7dac)

// class IBlockMgr
//
// This interface allows different parts of the system (max) and plugins
// to work with blocks
//
// Right now, it provides very limitted functionality, the plan being to
// extend it in the future as we identify functionality that can be factored 
// out from FileLink into this object.
class IBlockMgr : public FPStaticInterface
{
	public:
    // Checks whether a node is a block instance. If yes, the method it returns 
    // a pointer to the block interface.
    // INode& n - is this node a block instance?
		virtual IBlockRefComponent* IsInstance(INode& n) const = 0;

		// Retrieves instances of a block. 
		// INode& source - The block whose instances are to be retrieved. 
		// INodeTab& instances - An array of nodes representing the instances the
		// given block. This array is cleared before collection of instances 
		// starts. The original object is also included in this array since every  
		// object can be thought of as having at least one instance. 
		// Return value - The number of instances found. There's always at least 1 instance found
		virtual unsigned long GetInstances(INode& source, INodeTab& instances) const = 0;

		// Extends a node with a IBlockRefComponent interface. 
		// If the node is a block component already, it does nothing
		virtual IBlockRefComponent* MakeBlockRefComponent(INode& n) const = 0;

		// Removes the IBlockRefComponent interface from the node
		virtual bool RemoveBlockRefComponent(INode& n) const = 0;

		// Returns the top block of the component passed in as param.
		virtual INode* GetTopBlock(INode& n) const = 0;

		// For each block in the node array that has components, 
		// adds these components to the array of nodes to be selected
		virtual void ExpandSelection(INodeTab& nodes) const = 0;
		
		// --- File I/O 
		virtual IOResult Save(ISave* isave) const = 0;
		virtual IOResult Load(ILoad* iload) = 0;

		static IBlockMgr* GetBlockMgr()
		{
			return static_cast<IBlockMgr*>(GetCOREInterface(BLOCK_MGR_INTERFACE));
		}
}; 




