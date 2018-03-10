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
// DESCRIPTION: Defines a class for monitoring a ReferenceTarget by holding a weak reference to it
// (IsRealDependency returns false) and passing NotifyRefChanged messages to the owner of the
// class instance. 
// AUTHOR: Larry.Minton - created May.14.2004
//***************************************************************************/

#pragma once
#include "maxheap.h"
#include "ref.h"

//! Id for IRefTargMonitor interface
#define IID_REFTARG_MONITOR 0x3070552c

//! \brief This notification is sent to a IRefTargetMonitor's owner when it sets it's target
/*! Without this message, it's possible for the IRefTargetMontitor target to change with the owner not knowing about it.
\see IRefTargMonitor
*/
#define REFMSG_REFTARGMONITOR_TARGET_SET    REFMSG_USER + 0x3070552c  

//! \brief The interface for an owner of a RefTargMonitorRefMaker class.
/*! The owner of a RefTargMonitorRefMaker instance must implement this interface. Messages from
the RefTargMonitorRefMaker instance are passed to the owner through this interface.
\see RefTargMonitorRefMaker, IIndirectReferenceMaker
*/
class IRefTargMonitor: public MaxHeapOperators {
public:
	//! \brief Calls to the RefTargMonitorRefMaker instance's NotifyRefChanged method are passed to its owner through this method. 
	/*! The RefTargMonitorRefMaker owner is responsible for ensuring that an infinite recursion of message passing does not occur. 
	Typically the owner would set a flag while propogating a message, and not propogate a new message if that flag
	is set. 

	Since the owner doesn't hold a reference to the RefTargMonitorRefMaker's watched object, the watched object may not be 
	saved or loaded when the owner is saved or loaded. The owner must handle this case. Typically, the owner would also derive from
	IIndirectReferenceMaker, which will result in the watched object being saved and loaded. For an example, see the NodeTransformMonitor 
	class in maxsdk\\samples\\controllers\\nodetransformmonitor.cpp.

	The arguments to this method, other than fromMonitoredTarget, correspond to the arguments of ReferenceMaker::NotifyRefChanged.
	\param changeInt - This is the interval of time over which the message is active.
	\param hTarget - This is the handle of the reference target the message was sent by.
	\param partID - This contains information specific to the message passed in.
	\param message - The message which needs to be handled. 
	\param fromMonitoredTarget - true if message orginated from RefTargMonitorRefMaker's monitored target.
	\returns Typically REF_SUCCEED indicating the message was processed. 
	*/
	virtual RefResult ProcessRefTargMonitorMsg(
						Interval changeInt, 
						RefTargetHandle hTarget, 
						PartID& partID,  
						RefMessage message,
						bool fromMonitoredTarget) = 0;

	//! \brief Calls to the RefTargMonitorRefMaker instance's DoEnumDependents method are passed to its owner through this method. 
	/*! The RefTargMonitorRefMaker owner is responsible for ensuring that an infinite recursion  does not occur. 
	Typically the owner would set a flag while enumerating dependents, and not start a new enumeration if that flag
	is set. 
	\param dep - the DependentEnumProc.
	\returns 1 to stop the enumeration and 0 to continue. 
	*/
	virtual int ProcessEnumDependents(DependentEnumProc* dep) = 0;
};

//! \brief Defines a class for monitoring a ReferenceTarget
/*! Defines a class for monitoring a ReferenceTarget by holding a weak reference to it
(IsRealDependency returns false) and passing NotifyRefChanged messages to the owner of the
class instance. The owner would create an instance of this class passing itself as the 
owner and the ReferenceTarget to watch as the target. The owner must derive from IRefTargMonitor, 
and would typically derive from IIndirectReferenceMaker. If the owner derives from IIndirectReferenceMaker,
during scene load IIndirectReferenceMaker::SetIndirectReference is called to set the indirect reference. Thus, 
you wouldn't normally call this class's Save or Load methods from the owner if it derives from 
IRefTargMonitor. 
\see IRefTargMonitor, IIndirectReferenceMaker
*/
class RefTargMonitorRefMaker: public ReferenceMaker, public PostPatchProc {
private:
	IRefTargMonitor &mOwner;	// the owner of this instance
	RefTargetHandle mpTarget;	// the object being monitored

	//! \brief Private destructor for class instances. Instances should be deleted via DeleteThis()
	~RefTargMonitorRefMaker();

public:
	CoreExport Class_ID ClassID();

	//! \brief Constructor for class instances.
	/*! \param myOwner - The owner of the instance. The owner is responsible for deleting this instance. The owner must 
	implement the IRefTargMonitor interface.
	\param theTarget - The object instance to be watched.
	*/
	CoreExport RefTargMonitorRefMaker(IRefTargMonitor &myOwner, RefTargetHandle theTarget = NULL);

	//! \brief Set the object being watched.
	/*! \param theTarget - The object instance to be watched.
	*/
	CoreExport void SetRef(RefTargetHandle theTarget);

	//! \brief Get the object being watched.
	/*! \returns The object instance being watched.
	*/
	CoreExport RefTargetHandle GetRef();

	//! \brief Save a pointer to the object being watched. 
	/*! The owner can call this method from its Save method it it wants to persistently watch the object 
	across of load/save. As noted above, if the owner derives from IIndirectReferenceMaker, the owner typically
	would not call this method.
	\param isave - The ISave pointer passed to the owner's Save method.
	\returns One of the following values:
	IO_OK - The result was acceptable - no errors.
	IO_ERROR - This is returned if an error occurred.
	*/
	IOResult Save(ISave *isave);

	//! \brief Load a pointer to the object being watched. 
	/*! The owner can call this method from its Load method it it wants to persistently watch the object 
	across of load/save. The owner would create a new RefTargMonitorRefMaker setting itself as an owner, and 
	then call this method using that instance. As noted above, if the owner derives from IIndirectReferenceMaker, 
	the owner typically would not call this method.
	\param iload - The ILoad pointer passed to the owner's Load method.
	\returns One of the following values:
	IO_OK - The result was acceptable - no errors.
	IO_ERROR - This is returned if an error occurred.
	*/
	IOResult Load(ILoad *iload);

	//! \brief The PostPatchProc used when cloning. 
	/*! The PostPatchProc is used when cloning since the watched object may or may not have also been cloned.
	When the owner is cloned, it would typically create a new RefTargMonitorRefMaker pointing at the cloned
	watched object. At the time the owner is cloned, the watched object may not have been cloned yet, or it 
	may not be cloned at all. The owner should check to see if the watched object has been cloned already, and
	set the new RefTargMonitorRefMaker to watch it if so. If not, the new RefTargMonitorRefMaker should be set 
	to watch the original watch object, and register this proc via remap.AddPostPatchProc. This proc will check
	after the cloning is complete whether the watch object was cloned, and if so set the clone as the watched 
	object. If it was not cloned, the original object will be watched by the new RefTargMonitorRefMaker.
	\param remap - The RemapDir passed to the owner's Clone method.
	\returns TRUE in all cases
	*/
	int Proc(RemapDir& remap);

	// ReferenceMaker Methods
	//! \brief This method will notify the RefTargMonitorRefMaker of changes to the watched object. 
	/*! Implemented by the system.
	Changes to the watched object cause messages to be passed to this method, and the implementation of this 
	method passes those messages to the owner via the owner's ProcessRefTargMonitorMsg callback
	\returns The value returned from the owner's ProcessRefTargMonitorMsg callback
	*/
	RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message );

	//! \brief Get the number of references instance makes.
	/*! Implemented by the system.
	RefTargMonitorRefMaker makes 1 reference - the watched object.
	\returns The number of references instance makes.
	*/
	int NumRefs();

	//! \brief Get the indexed reference.
	/*! Implemented by the system.
	The watched object should be accessed via GetRef and SetRef. This method is used by the system.
	\returns The watched object.
	*/
	RefTargetHandle GetReference(int i);

	//! \brief Set the indexed reference.
	/*! Implemented by the system.
	Sets the watched object.
	The watched object should be accessed via GetRef and SetRef. This method is used by the system.
	*/
protected:
	virtual void SetReference(int i, RefTargetHandle rtarg);
public:

	//! \brief Specifies that this reference to the watched object should not prevent the watched object from being deleted
	/*! Implemented by the system.
	\returns FALSE in all cases
	*/
	BOOL IsRealDependency(ReferenceTarget *rtarg);

	//! \brief Allows a reference target to enumerate all references to it.
	/*! Implemented by the system.
	This method allows a reference target to enumerate all references to it. The implementation of this method passes the 
	DependentEnumProc to the owner via the owner's ProcessEnumDependents callback.
	\returns 1 to stop the enumeration and 0 to continue
	*/
	int DoEnumDependentsImpl(DependentEnumProc* dep);

	//! \brief Delete this instance.
	void DeleteThis();
};

