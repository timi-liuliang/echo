
#pragma once

#include "inode.h"

//! \brief Class representing a dynamic array of INodes.
class INodeTab : public Tab<INode*> 
{
public:     
	//! \brief Deletes all temporary nodes, such as those of type INodeTransformed, from the array.
	/*! \see INodeTransformed 
	*/
	void DisposeTemporary() {
		for (int i=0; i<Count(); i++) {
			if ((*this)[i] != NULL) {
				(*this)[i]->DisposeTemporary();
			}
		}
	}

	//! \brief Finds the specified node in the array and returns its index.
	/*! \param node - The node to find
	\return int - The index of the node in this array, or -1 if the node is not found
	*/
	int IndexOf(INode* node)
	{
		for (int i=0; i<Count(); i++) {
			if ((*this)[i] == node) {
				return i;
			}
		}
		return -1;
	}
	//! \brief Checks whether the specified node is in the array.
	/*! \param node - The node to find
	\return bool - true if the node is in the array, otherwise false
	*/
	bool Contains(INode *node)
	{
		return (IndexOf(node) >= 0);
	}

	//! \brief Adds a node to the end of the array.
	/*! \param node - The node to add to the array
	\param allowDups - If true, the specified node is added to the array 
	without checking whether it's contained already in it. If false, the node
	is added to the array only if it doesn't exist in it yet.
	\param allocExtra - The number of extra items by which the array should 
	be enlarged if all its items have been filled with nodes.
	\return - The number of nodes in the array prior to adding the specified node
	*/
	int AppendNode(INode* node, bool allowDups = false, int allocExtra = 0) {
		if (!allowDups && Contains(node)) {
			return Count();
		}
		return Append(1, &node, allocExtra); 
	}

	//! \brief Inserts a node into the array at the specified position.
	/*! \param node - The node to add to the array
	\param at - Array index where to insert the specified node. If a negative 
	value is specified, the node will be appended to the array.
	\param allowDups - If true, the specified node is added to the array 
	without checking whether it's contained already in it. If false, the node
	is added to the array only if it doesn't exist in it yet.
	\return - The array index at which the node was inserted, or -1 if the node 
	was not inserted into the array
	*/
	int InsertNode(INode* node, int at, bool allowDups = false) {
		if (at < 0) {
			AppendNode(node, allowDups);
			return Count();
		}
		else if (allowDups || !Contains(node)) {
			return Insert(at, 1, &node);
		}
		return -1;
	} 
};
