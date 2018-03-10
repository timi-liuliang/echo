/**********************************************************************
 *<
	FILE: namesel.h

	DESCRIPTION:  A named sel set class for sub-object named selections ets

	CREATED BY: Rolf Berteig

	HISTORY: 3/18/96

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "tab.h"
#include "strclass.h"
#include "maxtypes.h"

// forward declarations
class BitArray;
class ILoad;
class ISave;

#pragma warning(push)
#pragma warning(disable:4239)

/*! \sa  Class BitArray.
\par Description:
This class is available in release 3.0 and later only.\n\n
This class is a tool for manipulating lists of named selection sets. This class
is used by modifiers such as the edit mesh, mesh select, spline select and edit
patch.\n\n
All methods of this class are implemented by the system.
\par Data Members:
<b>Tab\<MSTR*\> names;</b>\n\n
A table of names, one for each selection set.\n\n
<b>Tab\<BitArray*\> sets;</b>\n\n
The bit array pointers for the selection sets.\n\n
<b>Tab\<DWORD\> ids;</b>\n\n
A table of Ids, one for each selection set.  */
class GenericNamedSelSetList: public MaxHeapOperators {
	public:
		Tab<MSTR*> names;
		Tab<BitArray*> sets;
		Tab<DWORD> ids;

		/*! \remarks Destructor. The names and sets are deleted. */
		CoreExport ~GenericNamedSelSetList();
		/*! \remarks Returns a pointer to the bit array corresponding to the specified
		name. If the set is not found NULL is returned.
		\par Parameters:
		<b>MSTR name</b>\n\n
		The name of the selection set to retrieve. */
		CoreExport BitArray *GetSet(MSTR &name);
		/*! \remarks Returns a poniter to the bit array corresponding to the
		specified ID. If the set is not found NULL is returned.
		\par Parameters:
		<b>DWORD id</b>\n\n
		The id of the selection set to retrieve. */
		CoreExport BitArray *GetSet(DWORD id);
		/*! \remarks Returns a pointer to the bit array corresponding to the
		specified index in the list. If the set is not found NULL is returned.
		\par Parameters:
		<b>int index</b>\n\n
		The zero based index of the selection set to retrieve (\>= 0 and \<
		sets.Count()). */
		CoreExport BitArray *GetSetByIndex(int index);
		/*! \remarks Returns the number of selection sets. */
		int Count() {return sets.Count();}
		/*! \remarks Appends the named selection set data to the list of sets
		maintained by this class.
		\par Parameters:
		<b>BitArray \&nset</b>\n\n
		The selection set data to append.\n\n
		<b>DWORD id=0</b>\n\n
		An ID for the selection set.\n\n
		<b>MSTR name=_M("")</b>\n\n
		The name for the selection set. */
		CoreExport void AppendSet(BitArray &nset,DWORD id=0,MSTR &name=MSTR(""));
		/*! \remarks Inserts the named selection set data into the list of sets
		maintained by this class.
		\par Parameters:
		<b>int pos</b>\n\n
		The position in the list where this named selection set should be inserted. If
		pos \>= Count(), AppendSet() is automatically used instead.\n\n
		<b>BitArray \&nset</b>\n\n
		The selection set data to insert.\n\n
		<b>DWORD id=0</b>\n\n
		An ID for the selection set.\n\n
		<b>MSTR \&name=MSTR("")</b>\n\n
		The name for the selection set. */
		CoreExport void InsertSet(int pos, BitArray &nset,DWORD id=0,MSTR &name=MSTR(""));
		/*! \remarks This method is similar to <b>InsertSet()</b> above, however
		instead of accepting an explicit location this method inserts the new set
		alphabetically in the list. (Of course, this requires an alphabetized list to
		work properly, although there's no problem if the list is not alphabetized.)
		\par Parameters:
		<b>BitArray \&nset</b>\n\n
		The selection set data to insert.\n\n
		<b>DWORD id=0</b>\n\n
		An ID for the selection set.\n\n
		<b>MSTR \&name=MSTR("")</b>\n\n
		The name for the selection set.
		\return  The position where the set was inserted. */
		CoreExport int InsertSet(BitArray &nset,DWORD id=0,MSTR &name=MSTR(""));
		/*! \remarks Removes the selection set whose name is passed.
		\par Parameters:
		<b>MSTR name</b>\n\n
		The name of the selection set to remove.
		\return  TRUE on success; otherwise FALSE. */
		CoreExport BOOL RemoveSet(MSTR &name);
		/*! \remarks Removes the selection set whose ID is passed.
		\par Parameters:
		<b>DWORD id</b>\n\n
		The id of the selection set to retrieve.
		\return  TRUE on success; otherwise FALSE. */
		CoreExport BOOL RemoveSet(DWORD id);
		/*! \remarks This method is used internally to load the selection sets
		from disk. */
		CoreExport IOResult Load(ILoad *iload);
		/*! \remarks This method is used internally to save the selection sets
		to disk. */
		CoreExport IOResult Save(ISave *isave);
		/*! \remarks Resizes the selectin set bit arrays to the specified
		number of bits. The old selection set data is preserved.
		\par Parameters:
		<b>int size</b>\n\n
		The new size for the bit arrays in bits. */
		CoreExport void SetSize(int size);
		/*! \remarks Assignment operator. This list of sets is emptied and
		then copied from the list passed.
		\par Parameters:
		<b>GenericNamedSelSetList\& from</b>\n\n
		The list of selection sets to copy. */
		CoreExport GenericNamedSelSetList& operator=(GenericNamedSelSetList& from);
		/*! \remarks This method is not currently used. What it does however,
		is go through all of the named selection sets and deletes array
		elements according to which bits are set in the given bit array. It
		could be used to keep the named selection set bit arrays in line with
		the vertex array (for example). */
		CoreExport void DeleteSetElements(BitArray &set,int m=1);
		/*! \remarks Deletes the named selection set whose index is passed.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the set to delete (\>=0 and \< sets.Count()).
		*/
		CoreExport void DeleteSet(int i);
		/*! \remarks This locates the named selection set <b>oldName</b> and
		renames it to <b>newName</b>.
		\par Parameters:
		<b>MSTR \&oldName</b>\n\n
		The old name of the set.\n\n
		<b>MSTR \&newName</b>\n\n
		The new name for the set.
		\return  TRUE if the operation succeeded; otherwise FALSE. It will only
		fail if the <b>oldName</b> set is not present. */
		CoreExport BOOL RenameSet(MSTR &oldName, MSTR &newName);
		/*! \remarks Alphabetizes the list of names. */
		CoreExport void Alphabetize ();	// Bubble Sort!
		/*! \remarks Returns a reference to the 'i-th' selection set.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the selection set to return. */
		BitArray &operator[](int i) {return *sets[i];}
	};

#pragma warning(pop)

