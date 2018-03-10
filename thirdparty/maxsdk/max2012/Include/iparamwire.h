/*	
 *		iParamWire.h - Public interface to Parameter Wiring Manager & Wire Controllers
 *
 *			Copyright (c) Autodesk, Inc, 2000.  John Wainwright.
 *
 */

#pragma once

#ifndef NO_PARAMETER_WIRING

#include "iFnPub.h"
#include "GetCOREInterface.h"
#include "control.h"

// forward declarations
class IParamWireMgr;
class IBaseWireControl;

// ---------  parameter wiring manager, provides gerenal access to param wiring functions ---

#define PARAMWIRE_MGR_INTERFACE   Interface_ID(0x490d0e99, 0xbe87c96)
inline IParamWireMgr* GetParamWireMgr() { return (IParamWireMgr*)GetCOREInterface(PARAMWIRE_MGR_INTERFACE); }

/*! \defgroup ParamWireFlags Parameter Wire Menu Flags
These flags determine the action taken when calling ParamWireMenu.  
Use these flags to specify the target type and the options.
\sa IParamWireMgr::ParamWireMenu
*/
//!@{
/*! \remarks Specifies that the Target will be displayed on the left side of the parameter wiring dialog. This is typically the first target picked. */
#define PWMF_LEFT_TARGET	0x001
/*! \remarks Specifies that the Target will be displayed on the right side of the parameter wiring dialog. This is typically the second target picked. */
#define PWMF_RIGHT_TARGET	0x002
/*! \remarks Can be used to test if a context menu will be displayed or not with the specified target. If a menu can be displayed, the target will be returned by the method. If not, NULL is returned */
#define PWMF_HAS_MENU		0x004
/*! \remarks Specifies whether the parameter wiring dialog should open once the user has selected an item from the context menu. This is typically used in conjunction with PWMF_RIGHT_TARGET. */
#define PWMF_OPEN_EDITOR	0x008
//!@}

//==============================================================================
// class IParamWireMgr
//    parameter wiring manager interface 
/*! \sa  Class FPStaticInterface,  Class Control, Class ReferenceTarget\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface that provides general access to the
parameter wiring functions. You can obtain a pointer to the Parameter Wire
Manager interface using; 
\code
	IParamWireMgr* GetParamWireMgr();
\endcode. 
This macro will return
\code
	static_cast<IParamWireMgr*>(GetCOREInterface(PARAMWIRE_MGR_INTERFACE));
\endcode
All methods of this class are Implemented by the System.  
*/
//==============================================================================
class IParamWireMgr : public FPStaticInterface 
{
public:
	// function IDs 
	enum { startParamWire,
		   openEditor,
		   editParams, editParam,
		   editControllers, editController,
		   connect, connect2Way,
		   disconnect, disconnect2Way
		}; 

	/*! \remarks This method will launch the parameter wiring UI mode. */
	virtual void StartParamWire()=0;					// launch param wiring UI mode
	/*! \remarks This method will open up the parameter wiring dialog on the
	selected objects. */
	virtual void OpenEditor()=0;						// open param wiring dialog on selected objects
	/*! \remarks This method allows you to edit the left- and right-hand
	parameters and opens the parameter wiring dialog using the provided
	parameters.
	\param leftParent - A pointer to the left-hand reference target.
	\param leftSubNum - The sub-animatable of the left-hand reference target.
	\param rightParent - A pointer to the right-hand reference target.
	\param rightSubNum - The sub-animatable of the right-hand reference target. */
	virtual void EditParams(ReferenceTarget* leftParent, int leftSubNum,
							ReferenceTarget* rightParent, int rightSubNum)=0;  // edit params
	virtual void EditParam(ReferenceTarget* parent, int subNum)=0;
	/*! \remarks This method allows you to setup the two controllers for the
	left- and right-hand to edit.
	\param leftWire - A pointer to the controller for the left-hand wire.
	\param rightWire - A pointer to the controller for the right-hand wire. */
	virtual void EditControllers(Control* leftWire, Control* rightWire)=0;
	/*! \remarks This method is identical to the EditControllers() but
	accepts a single wire controller for the left-hand. This method effectively
	calls EditControllers(wire, NULL).
	\param wire - A pointer to the controller being edited. */
	virtual void EditController(Control* wire)=0;
	/*! \remarks This method allows you to set up a one-way wire.
	\param fromParent - A pointer to the reference target to wire from.
	\param fromSubNum - The sub-animatable to wire from.
	\param toParent - A pointer to the reference target to wire to.
	\param toSubNum - The sub-animatable to wire to.
	\param toExpr - A string containing the expression on the "to wire".
	\return  TRUE if the connection can be made, otherwise FALSE. */
	virtual bool Connect(ReferenceTarget* fromParent, int fromSubNum,
							ReferenceTarget* toParent, int toSubNum,
							MCHAR* toExpr)=0;		 // set up a one-way wire from -> to
	/*! \remarks This method allows you to set up a two-way wire.
	\param leftParent - A pointer to the left-hand reference target.
	\param leftSubNum - The sub-animatable of the left-hand reference target.
	\param rightParent - A pointer to the right-hand reference target.
	\param rightSubNum - The sub-animatable of the right-hand reference target.
	\param leftExpr - A string containing the expression for the left-hand target.
	\param rightExpr = NULL - A string containing the expression for the right-hand target.
	\return  TRUE if the connection can be made, otherwise FALSE. */
	virtual bool Connect2Way(ReferenceTarget* leftParent, int leftSubNum,
							ReferenceTarget* rightParent, int rightSubNum,
							MCHAR* leftExpr, MCHAR* rightExpr=NULL)=0;     // set up a two-way wire
	/*! \remarks This method allows you to disconnect a one-way wire.
	\param wireController - A pointer to the wire controller you wish to disconnect.
	\return  TRUE if the disconnect was successful, otherwise FALSE. */
	virtual bool Disconnect(Control* wireController)=0;  // disconnect one-way
	/*! \remarks This method allows you to disconnect a two-way wire.
	\param wireController1 - A pointer to the first wire controller you wish to disconnect.
	\param wireController2 - A pointer to the second wire controller you wish to disconnect.
	\return  TRUE if the disconnect was successful, otherwise FALSE. */
	virtual bool Disconnect2Way(Control* wireController1, Control* wireController2)=0;  // disconnect two-way

	/*! \remarks The ParamWireMenu method allows a plugin to host the parameter 
	wiring context menus, typically found in the viewport, into any window.
	\param pTarget - The parent of the target animatable that the context menu will be derived from.  pTarget should support the I_WIRECONTROL interface.
	\param iSubNum - The SubAnim number in pTarget of the target animatable that the context menu will be derived from.  If iSubNum is negative, pTarget will be used.
	\param iFlags - one of the <a href="ms-its:3dsMaxSDKRef.chm::/group___param_wire_flags.html">parameter wire flags</a>.  This sets which which action to take.
	\param hWnd - used as the parent window for the context menus and parameter wiring dialog.
	\param pPt - If iFlags is not PWMF_HAS_MENU, pPt should specify the position of the context menu in screen coordinates
	\return The animatable that derived the context menu, or NULL if no menu was displayed */
	virtual Animatable* ParamWireMenu( ReferenceTarget* pTarget, int iSubNum, int iFlags = PWMF_LEFT_TARGET, HWND hWnd = NULL, IPoint2 *pPt = NULL )=0;
}; 

// ------ individual wire controller interface -------------

// wire controller classes & names

#define FLOAT_WIRE_CONTROL_CLASS_ID		Class_ID(0x498702e7, 0x71f11549)
#define POSITION_WIRE_CONTROL_CLASS_ID	Class_ID(0x5065767c, 0x683a42a6)
#define POINT3_WIRE_CONTROL_CLASS_ID	Class_ID(0x4697286a, 0x2f7f05cf)
#define POINT4_WIRE_CONTROL_CLASS_ID	Class_ID(0x4697286b, 0x2f7f05ff)
#define ROTATION_WIRE_CONTROL_CLASS_ID	Class_ID(0x31381913, 0x3a904167)
#define SCALE_WIRE_CONTROL_CLASS_ID		Class_ID(0x7c8f3a2b, 0x1e954d92)

#define WIRE_CONTROLLER_INTERFACE   Interface_ID(0x25ce0f5c, 0x6c303d2f)
inline IBaseWireControl* GetWireControlInterface(Animatable* a) { return (IBaseWireControl*)a->GetInterface(WIRE_CONTROLLER_INTERFACE); }

/*! \sa  Class Control, Class StdControl, Class FPMixinInterface\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface for individual wire controllers. You can
obtain a pointer to the Base Wire control interface using; 
\code 
	IBaseWireControl* GetWireControlInterface(Animatable* a);
\endcode
This macro will return
\code
	(IBaseWireControl*)a-\>GetInterface(WIRE_CONTROLLER_INTERFACE);
\endcode
The Class ID's of individual wire controllers are:
 FLOAT_WIRE_CONTROL_CLASS_ID \n
 POSITION_WIRE_CONTROL_CLASS_ID \n
 POINT3_WIRE_CONTROL_CLASS_ID \n
 ROTATION_WIRE_CONTROL_CLASS_ID \n
 SCALE_WIRE_CONTROL_CLASS_ID\n\n
All methods of this class are Implemented by the System.  */
class IBaseWireControl : public StdControl, public FPMixinInterface {
	public:
		// local 
		// number of wires, wire param access
		/*! \remarks This method returns the number of wires out of this
		controller (i.e. the number of dependent params). */
		virtual int		get_num_wires()=0;			 // how many wires out of this controller (number of dependent params)
		/*! \remarks This method returns a pointer to the i-th dependent
		parameter parent.
		\param i - The index you wish to retrieve. */
		virtual Animatable* get_wire_parent(int i)=0;  // get ith dependent parameter parent animatable
		/*! \remarks This method returns the i-th dependent parameter subanim
		num in the animatable.
		param i - The index of the subanim. */
		virtual int		get_wire_subnum(int i)=0;    // get ith dependent parameter subanim num in the animatable
		/*! \remarks This method returns a pointer to the i-th CoController.
		\param i - The index of the controller. */
		virtual Control* get_co_controller(int i)=0; // get ith co_controller

		// transfer expression script
		/*! \remarks This method returns the expression string of the i-th
		wire parameter.
		param  i - 	The index of the parameter. */
		virtual MCHAR*	get_expr_text(int i)=0;
		/*! \remarks This method allows you to set the expression string of
		the i-th wire parameter.
		\param  i - The index of the parameter
		\param  text - The expression you wish to set. */
		virtual void	set_expr_text(int i, MCHAR* text)=0;

		// animation sub-controller
		/*! \remarks This method allows you to set the slave animation
		controller.
		\param c - A pointer to the controller you wish to set. */
		virtual void	set_slave_animation(Control* c)=0;
		/*! \remarks This method returns a pointer to the slave animation
		controller. */
		virtual Control* get_slave_animation()=0;

		// type predicate
		/*! \remarks This method will return TRUE if the wire is a master
		predicate, otherwise it will return FALSE. */
		virtual bool    is_master()=0;
		/*! \remarks This method will return TRUE if the wire is a slave
		predicate, otherwise it will return FALSE. */
		virtual bool	is_slave()=0;
		/*! \remarks This method will return TRUE if the wire is a two-way
		predicate, otherwise it will return FALSE. */
		virtual bool	is_two_way()=0;

		// parent/subnum transfers
		virtual void transfer_parent(ReferenceTarget* oldp, ReferenceTarget* newp)=0;
		virtual void transfer_subnum(short oldn, short newn)=0;

		// FnPub stuff
		enum { getNumWires, getWireParent, getWireSubnum, getCoController, 
			   getExprText, setExprText, 
			   getSlaveAnimation, setSlaveAnimation, isMaster, isSlave, isTwoWay, };

		// from FPInterface
		FPInterfaceDesc* GetDesc() { return GetDescByID(WIRE_CONTROLLER_INTERFACE); }

		BEGIN_FUNCTION_MAP
			FN_1(getWireParent,		 TYPE_REFTARG,	 get_wire_parent,	TYPE_INDEX);
			FN_1(getWireSubnum,		 TYPE_INDEX,	 get_wire_subnum,	TYPE_INDEX);
			FN_1(getCoController,	 TYPE_CONTROL,	 get_co_controller,	TYPE_INDEX);
			FN_1(getExprText,		 TYPE_STRING,	 get_expr_text,		TYPE_INDEX);
			VFN_2(setExprText,						 set_expr_text,		TYPE_INDEX, TYPE_STRING);
			RO_PROP_FN(getNumWires,	 get_num_wires,	 TYPE_INT);
			RO_PROP_FN(isMaster,	 is_master,		 TYPE_bool);
			RO_PROP_FN(isSlave,		 is_slave,		 TYPE_bool);
			RO_PROP_FN(isTwoWay,	 is_two_way,	 TYPE_bool);
			PROP_FNS(getSlaveAnimation, get_slave_animation, setSlaveAnimation, set_slave_animation, TYPE_CONTROL);
		END_FUNCTION_MAP

};


#endif // NO_PARAMETER_WIRING

