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
// DESCRIPTION: Defines an interface for getting/setting indirect references.
// AUTHOR: Larry.Minton - created Jan.11.2006
//***************************************************************************/
#pragma once

#include "maxheap.h"
#include "ref.h"

//! Id for IIndirectReferenceMaker interface
#define IID_IINDIRECTREFERENCEMAKER					0x18f81902

//! \brief The interface for an object to specify the ReferenceTargets it is dependent on, but doesn't hold a reference to (i.e., indirect references).
/*! The owner of a RefTargMonitorRefMaker-like instance would implement this interface to cause the ReferenceTargets it is dependent on, 
but doesn't hold a reference to, to be saved when the owner is saved, and loaded when the owner is loaded. When the scene is loaded,
SetIndirectReference is called to set the indirect reference. 

Indirect references should typically implemented as weak references. Otherwise, if ref A holds a normal reference to B, and B holds an indirect 
reference to A, A would never be deleted because there is always at least one strong reference to it. 

For an example, see the NodeTransformMonitor class in maxsdk\\samples\\controllers\\nodetransformmonitor.cpp.
\see RefTargMonitorRefMaker.
*/
class IIndirectReferenceMaker: public MaxHeapOperators {
public:
	//! \brief The number of indirect references. 
	/*! \returns The number of indirect references.
	*/
	virtual int NumIndirectRefs() = 0;

	//! \brief Retrieve the indexed indirect reference. 
	/*! \param i - The virtual array index of the indirect reference to get.
	\returns The reference handle of the 'i-th' indirect reference.
	*/
	virtual RefTargetHandle GetIndirectReference(int i) = 0;

	//! \brief Set the indexed indirect reference. 
	/*! \param i - The virtual array index of the indirect reference to set.
	\param rtarg - The indirect reference.
	*/
	virtual void SetIndirectReference(int i, RefTargetHandle rtarg) = 0;

	//! \brief Specifies remapping of indirect references on load. 
	/*! This method is used when you have modified a IIndirectReferenceMaker to add or delete indirect references, and are loading old files. 
	This method is called during the reference mapping process, after the Load() method is called. You determine what version is loading 
	in the Load(), and store the version in a variable which you can look at in RemapIndirectRefOnLoad() to determine how to remap 
	indirect references. The default implementation of this method just returns the same value it is passed, so you don't need to 
	implement it unless you have added or deleted indirect references from your class.  
	\param iref - The input index of the indirect reference. 
	\returns The output index of the indirect reference.
	*/
	virtual int RemapIndirectRefOnLoad(int iref) { return iref; }

	//! \brief Specifies whether an indirect reference is persisted on a partial load or save. 
	/*! This method specifies the partial load/save behavior of an indirect reference. If this method returns true, 
	and the ref maker implementing this interface is loaded/saved, the indirect reference will be forced to be loaded/saved. 
	If false, the indirect reference will not be forced to be loaded/saved, but will be hooked back up if it is loaded.

	Since indirect references are typically implemented as weak references, the default implementation of this method is to return false. 
	Typical cases where an implementation of this method would return true is if the indirect reference is to a node (such as in 
	NodeTransformMonitor) or postload callbacks are used to check and process the indirect reference.

	\param rtarg - The indirect reference. 
	\returns Whether to force the load/save of the indirect reference if the implementing reference maker is saved.
	*/
	virtual BOOL ShouldPersistIndirectRef(RefTargetHandle rtarg) { UNUSED_PARAM(rtarg); return FALSE; }
};
