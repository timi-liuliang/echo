/**********************************************************************
 *<
	FILE: ILinkTMCtrl.h

	DESCRIPTION: Interface for FileLink's LinkTMCtrl

	CREATED BY: Stew Sabadell

	HISTORY:

 *>	Copyright (c) 2005, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "maxheap.h"
#include "control.h"
#include "AnimatableInterfaceIDs.h"
// forward declarations
class Matrix3;
class Point3;
class Control;
class Object;
class INode;
class Interface;

// This Class_ID can be used to create an instance of the implementation class.
#define LINKTMCTRL_CLASSID Class_ID(0x615b56e4, 0x5f0e6032)

class ILinkTMCtrl : public MaxHeapOperators //: public BaseInterface
{
public:
	// BaseInterface methods
	//virtual Interface_ID GetID() { return ILINKTMCTRL_INTERFACE_ID; }

	virtual void SetLinkTM(Matrix3 tm) = 0;
	virtual Matrix3 GetLinkTM() const = 0;
	virtual Point3 GetScales() const = 0;

	// Creates the subcontroller, makes a reference to it, and returns a pointer
	// to the subcontroller.
	virtual Control* CreateSubController() = 0;
};

//! \brief ISubTargetCtrl gives access to sub-transform controller.
/*! This interface is used to detect when a transform controller is just modifying
    the transform of a sub-transform-controller. The target of the sub-controller
    must be returned as the target of the parent controller. Setting the value
    of the parent controller should modify the value and set the value of the
    sub-controller to the modified value. */
class ISubTargetCtrl : public MaxHeapOperators {
public:
	//! \brief Return whether this controller allows the sub-controller to be assigned.
	virtual bool CanAssignTMController() const = 0;

	//! \brief Assign the sub-controller.
	/*! \param[in] tm Sub-controller to assign; must be a Matrix3 controller. */
	virtual void AssignTMController(Control* tm) = 0;

	//! \brief Returns the sub-controller
	virtual Control* GetTMController() const = 0;
};

// This inline give you an easy way to retrieve the interface ptr from a control;
// I did it as an inline to provide type-checking. It returns NULL if either the
// Control ptr is NULL, or it does not support the I_LINKTMCTRL interface.
inline ILinkTMCtrl* GetLinkTMInterface(Control* c) { return c ? (ILinkTMCtrl*)c->GetInterface(I_LINKTMCTRL) : NULL; }

// This inline give you an easy way to retrieve the version 2 interface ptr from a control;
// I did it as an inline to provide type-checking. 
//! \brief Returns the ISubTargetCtrl interface to a sub-transform controller
/*! \param[in] c Sub-transform controller to fetch the interface from 
    \return The ISubTargetCtrl interface.
	Returns NULL if either the control ptr is NULL, or it does not support the I_LINKTMCTRL2 interface. */
inline ISubTargetCtrl* GetSubTargetInterface(Control* c) { return c ? (ISubTargetCtrl*)c->GetInterface(I_SUBTARGETCTRL) : NULL; }


