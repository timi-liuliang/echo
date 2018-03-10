/*****************************************************************************

	FILE: ILimitCtrl.h

	DESCRIPTION: Describes all interfaces and constants about the limit 
	controllers that the outside world should need to know.

	CREATED BY:	Nicolas Leonard

	HISTORY:	November 3rd, 2004	Creation

 	Copyright (c) 2004, All Rights Reserved.
 *****************************************************************************/

#pragma once

#include "iFnPub.h"
#include "Animatable.h"

/***************************************************************
Function Publishing System   
****************************************************************/

#define IID_LIMIT_CONTROL Interface_ID(0xa85724f, 0x75325d35)

///////////////////////////////////////////////////////////////////////////////
//! Base Limit Controller Interface
//!
//! \brief ILimitControl provides access to the limit controller methods. 

//! A limit controller is used to restrict the range of values
//! produced by other controllers.
//! 
//! The controller whose range of value is being restricted is called the
//! limited controller.  Its presence is optional.  
//! 
//! This interface can be used to gain access to the limit controller's enable 
//! state and limited controller, and to set the upper or lower limits of  
//! the controller.
//!
//! Limit controllers can be disabled, in which case they just let the limited 
//! controller pass through.
///////////////////////////////////////////////////////////////////////////////

class ILimitControl: public FPMixinInterface
{
public:

	// Function Publishing System
	enum {
		is_enabled,
		set_enabled,
		get_limited_control,
		set_limited_control,
		set_upper_limit,
		set_lower_limit
	};

	// Function Map For Mixin Interface
	///////////////////////////////////////////////////////////////////////////
	BEGIN_FUNCTION_MAP
		FN_0(is_enabled, TYPE_bool, IsEnabled);
		VFN_1(set_enabled, SetEnabled, TYPE_bool);
		FN_0(get_limited_control, TYPE_CONTROL, GetLimitedControl);
		VFN_1(set_limited_control, SetLimitedControl, TYPE_CONTROL);
	END_FUNCTION_MAP

	FPInterfaceDesc* GetDesc(); 

	//! \name Accessors for the Enable state of the limit controller
	//@{ 
	virtual bool IsEnabled() const = 0;
	virtual void SetEnabled(bool in_enabled) = 0;
	//@}

	//! \name Accessors for the limited controller of the limit controller
	//@{ 
	virtual Control* GetLimitedControl() const = 0;
	virtual void SetLimitedControl(Control *in_limitedCtrl) = 0;
	//@}

	//! Set the upper limit of the controller at a given time. 
	//! \param[in] in_t - time for which the upper limit is to be set
	//! \param[in] in_val - pointer to a variable of type according to the limit controller type
	virtual void SetUpperLimit(const TimeValue& in_t, void *in_val) = 0;

	//! Set the lower limit of the controller at a given time. 
	//! \param[in] in_t - time for which the lower limit is to be set
	//! \param[in] in_val - pointer to a variable of type according to the limit controller type
	virtual void SetLowerLimit(const TimeValue& in_t, void *in_val) = 0;

	//! If the Animatable supports the ILimitControl interface, returns a pointer to it.
	//! \param[in] anim - Animatable to be tested for support of the ILimitControl interface
	//! \return - a pointer to an ILimitControl interface, or NULL.
	static inline ILimitControl* GetBaseLimitInterface(Animatable* anim) { 
		return anim? static_cast<ILimitControl*>(anim->GetInterface(IID_LIMIT_CONTROL)): NULL;}
};

