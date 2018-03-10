//
// Copyright [2008] Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//
#pragma once

#include "tab.h"
#include "coreexp.h"
#include "strclass.h"

class MtlBase;
typedef MtlBase* MtlBaseHandle;

//! \brief Simple list of MtlBase
/*! A simple list of MtlBases. All methods of this class are implemented by the
system.\n\n
\sa  Template Class Tab, Class MtlBase, Class Interface.\n\n
Note the following typedefs:\n\n
<b>typedef MtlBase* MtlBaseHandle;</b>\n\n
<b>typedef Mtl* MtlHandle;</b>\n\n
<b>typedef Texmap* TexmapHandle;</b>  */
class MtlBaseList : public Tab<MtlBaseHandle>
{
public:
	//! \brief Adds the specified MtlBase to the list.
	/*! 
	\param[in] m - The MtlBase to add.
	\param[in] checkUnique - If TRUE this method checks to make sure the MtlBase 
	is unique, and will only add it if so.
	\return  Nonzero if the MtlBase was added; otherwise zero. */
	CoreExport int AddMtl(MtlBase *m, BOOL checkUnique=TRUE);
	//! \brief Finds the specified MtlBase in this material list and returns 
	//! its index. Returns -1 if not found. 
	/*! 
	\param[in] m - The MtlBase to find. */
	CoreExport int FindMtl(MtlBase *m);
	//! \brief Finds the specified material by name and returns its index.
	//! Returns -1 if not found.
	/*! 
	\param[in] name - The name to find. */
	CoreExport int FindMtlByName(MSTR& name);
	//! \brief Removes the specified MtlBase from the list.
	/*! 
	\param[in] n - The index of the MtlBase to remove. */
	CoreExport void RemoveEntry(int n);
	//! \brief Removes all MtlBases from the list. 
	CoreExport void Empty();
};

