/**********************************************************************
 *<
	FILE: hierclas.h

	DESCRIPTION: Simple utility class for describing hierarchies

	CREATED BY: Tom Hudson

	HISTORY: Created 3 July 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "strclass.h"

#define INVALID_HIERARCHY -1

/*! \sa  Class GenericHierarchy.\n\n
\par Description:
This class represents an entry in a <b>GenericHierarchy</b>. All methods of the
class are implemented by the system.
\par Data Members:
<b>int data;</b>\n\n
This is the polygon number. For example a donut shape will have polygon 0 and
polygon 1. This is an index into the list of polygons for the shape.\n\n
<b>int children;</b>\n\n
Number of children of this entry.\n\n
<b>HierarchyEntry *parent;</b>\n\n
Points to the parent entry.\n\n
<b>HierarchyEntry *sibling;</b>\n\n
Points to the first sibling.\n\n
<b>HierarchyEntry *child;</b>\n\n
Points to the first child.\n\n
<b>MSTR sortKey;</b>\n\n
The sort key. This is used internally by the hierarchy so that it can determine
whether two hierarchies are compatible. Developers shouldn't alter this value.
 */
class HierarchyEntry: public MaxHeapOperators {
	public:
		int data;
		int children;
		HierarchyEntry *parent;
		HierarchyEntry *sibling;
		HierarchyEntry *child;
		MSTR sortKey;
		/*! \remarks Constructor. The entry is initialized as empty. */
		UtilExport HierarchyEntry();
		/*! \remarks Constructor. The entry is initialized to the data passed.
		\par Parameters:
		<b>int d</b>\n\n
		The value for <b>data</b>. This is the polygon number.\n\n
		<b>HierarchyEntry *p</b>\n\n
		The <b>parent</b> pointer is initialized to this value.\n\n
		<b>HierarchyEntry *s</b>\n\n
		The <b>sibling</b> pointer is initialized to this value. */
		UtilExport HierarchyEntry(int d, HierarchyEntry *p, HierarchyEntry *s);
		/*! \remarks Returns the level in the hierarchy this entry represents.
		This is the number of parents this item has. The root level is not
		counted.
		\return  The level in the hierarchy of this entry. */
		UtilExport int HierarchyLevel();
		/*! \remarks Adds a new child entry to this entry using the data
		specified.
		\par Parameters:
		<b>int d</b>\n\n
		The <b>data</b> value of the child. This is the polygon number. */
		UtilExport void AddChild(int d);
		/*! \remarks Returns the specified child of this entry.
		\par Parameters:
		<b>int index</b>\n\n
		The child to retrieve.
		\return  The specified child of this entry. If the specified index is
		greater than or equal to the number of children
		<b>INVALID_HIERARCHY</b> is returned. */
		UtilExport int GetChild(int index);
		/*! \remarks Returns the number of children of this entry. */
		int Children() { return children; }
		/*! \remarks Sorts the hierarchy using the <b>sortKey</b>s. */
		UtilExport void Sort();
	};

/*! \sa  Class HierarchyEntry.\n\n
\par Description:
This is a utility class for describing hierarchies of shapes. All methods of
this class are implemented by the system.\n\n
This is used in generating mesh objects from shapes. In order for a mesh object
to be generated correctly, nested shapes must be oriented clockwise or
counter-clockwise depending on their level of nesting. For example, a donut
shape with two circular curves will have the outer shape going
counter-clockwise and the inner shape going clockwise. If a third shape was
nested inside both of these, its points would be going counter-clockwise. <br>
*/
class GenericHierarchy: public MaxHeapOperators {
	private:
		HierarchyEntry root;
		void FreeTree(HierarchyEntry* start = NULL);
		BOOL isSorted;
		void CopyTree(int parent, HierarchyEntry* ptr);
	public:
		/*! \remarks Constructor. The hierarchy is set as initially empty. */
		GenericHierarchy() { root = HierarchyEntry(-1,NULL,NULL); isSorted = FALSE; }
		UtilExport ~GenericHierarchy();
		/*! \remarks This method adds one entry given its parent.
		\par Parameters:
		<b>int data</b>\n\n
		The polygon index of the entry to add.\n\n
		<b>int parent = -1</b>\n\n
		The index of the parent of the entry. */
		UtilExport void AddEntry(int data, int parent = -1);		// Add one entry, given its parent
		/*! \remarks Returns the total number of members in the hierarchy. */
		UtilExport int Entries();									// Total number of members in the hierarchy
		/*! \remarks Retrieves the first item under the root.
		\return  The first <b>HierarchyEntry</b> under the root. */
		UtilExport HierarchyEntry* GetStart() { return root.child; } // Get the first item under the root
		/*! \remarks Finds the specified entry in the hierarchy.
		\par Parameters:
		<b>int data</b>\n\n
		The polygon index of the entry to find.\n\n
		<b>HierarchyEntry* start = NULL</b>\n\n
		The entry at which to begin the search. If NULL is specified the search
		starts at the root.
		\return  A pointer to the <b>HierarchyEntry</b> of the found entry. If
		not found, NULL is returned. */
		UtilExport HierarchyEntry* FindEntry(int data, HierarchyEntry* start = NULL);
		/*! \remarks Returns the number of children for this item.
		\par Parameters:
		<b>int data</b>\n\n
		The index of the polygon to return the number of children of. */
		UtilExport int NumberOfChildren(int data);					// The number of children for this item
		/*! \remarks Returns the specified child of the specified entry.
		\par Parameters:
		<b>int data</b>\n\n
		The index of the polygon whose child is to be returned.\n\n
		<b>int index</b>\n\n
		Specifies which child to return.
		\return  The specified child of the entry. */
		UtilExport int GetChild(int data, int index);				// Get the nth child of this item
		/*! \remarks Clear out the hierarchy tree. */
		UtilExport void New();										// Clear out the hierarchy tree
		/*! \remarks Sorts the hierarchy tree by children / siblings. This is
		used internally as all the sorting is done automatically as the
		hierarchy is generated. */
		UtilExport void Sort();										// Sort tree by children/siblings
		/*! \remarks Determines if this hierarchy and the specified hierarchy
		are compatible.
		\par Parameters:
		<b>GenericHierarchy\& hier</b>\n\n
		The hierarchy to check for compatibility.
		\return  TRUE if the hierarchies are compatible; otherwise FALSE. */
		UtilExport BOOL IsCompatible(GenericHierarchy& hier);		// Are they compatible?
		/*! \remarks This method is used internally to <b>DebugPrint()</b> the
		tree. See
		<a href="ms-its:3dsmaxsdk.chm::/debug_debugging.html">Debugging</a>. */
		UtilExport void Dump(HierarchyEntry* start = NULL);			// DebugPrint the tree
		/*! \remarks Copy operator.
		\par Parameters:
		<b>GenericHierarchy\& from</b>\n\n
		The hierarchy to copy from. */
		UtilExport GenericHierarchy& operator=(GenericHierarchy& from);	// Copy operator
		/*! \remarks Returns the sort key for the hierarchy. This is used
		internally.
		\par Operators:
		*/
		UtilExport MSTR& SortKey();									// Get the sort key for the hierarchy
	};

