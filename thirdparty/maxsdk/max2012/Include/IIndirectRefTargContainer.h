//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Defines an interface for a container of indirect references to
// reference targets. All actions performed instances of this class are undoable.
// AUTHOR: Attila.Szabo - created March.14.2006
//***************************************************************************/
#pragma once

#include "plugapi.h"
#include "IIndirectRefMaker.h"
#include "IRefTargContainer.h"


//! The class id of the indirect reference container
#define INDIRECT_REFTARG_CONTAINER_CLASS_ID Class_ID(0x28de7aca, 0x78236a6f)

//! RefMessage sent by IIndirectRefTargContainer::ProcessRefTargMonitorMsg when 
//! a monitored ReferenceTarget is deleted. The hTarg argument is the 
//! ReferenceTarget being deleted.
#define REFMSG_INDIRECT_REFTARG_MONITOR_CONTAINER_TARGET_DELETED	REFMSG_USER + 0x78236a6f

//! \brief The interface for a container of indirect references
/* Instances of this class store a variable number of indirect references, and 
zero regular (direct) references. Reference notification messages sent by the
reference targets indirectly referenced by this container are ignored by the 
container and are not propagated to the container's dependents. Consequently, 
the container does not report itself nor its dependents as dependents of the 
indirect reference targets stored by it (see ReferenceTarget::DoEnumDependents).
The indirect references stored in this container to reference targets do not 
cause those targets to be persisted when this container is persisted. 

This type of container is implemented by 3ds Max. Instances of it can be created 
via Interface::CreateInstance(SClass_ID superID, Class_ID classID).
Plugins that need to store a variable number of indirect references to other
plugin objects should create an instance of this container type and make a 
reference to it. 

3ds Max's implementation of this container type persists itself and its operations 
support being undone\redone. 

\see IIndirectReferenceMaker, IRefTargContainer, ReferenceTarget
*/
class IIndirectRefTargContainer : public IRefTargContainer, public IIndirectReferenceMaker
{
public:
	virtual SClass_ID SuperClassID() { return REF_TARGET_CLASS_ID; }
	virtual Class_ID ClassID() { return INDIRECT_REFTARG_CONTAINER_CLASS_ID; }	

	//! \brief Finds the first occurance of a given reference target in the container
	/* \param [in] refTargToFind - the reference target to look for. Can be NULL.
	\return the zero based index of the found reference target within the container. 
	-1 if the item wasn't found.
	*/
	virtual int FindFirstItem(ReferenceTarget* refTargToFind) const = 0;
};

