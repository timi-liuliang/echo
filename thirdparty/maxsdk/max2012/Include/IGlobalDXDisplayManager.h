/**********************************************************************

FILE:           IGlobalDXDisplayManager.h

DESCRIPTION:    Public interface controlling the display of the Dx Effect
in the Viewport

CREATED BY:     Neil Hazzard, Discreet

HISTORY:        Created 21 May 2003

*>	Copyright (c) 2003, All Rights Reserved.
**********************************************************************/
#pragma once

#include "iFnPub.h"
#include "GetCOREInterface.h"

#define GLOBAL_DX_DISPLAY_MANAGER_INTERFACE Interface_ID(0x7ebe15d6, 0x2b7b422b)

//!This interface provides the user control over how the viewport shaders are displayed in the viewport.
/*!
 This interface provides the user control over how the viewport shaders are displayed in the viewport. 
 Control is provided to turn of the display totally and use the original material in the viewport, or 
 to display the shader only on the selected node. The new material options for viewport shaders 
 use this interface to control the system.

 Developers can control this themselves using this interface, you simply have to use GetGlobalDXDisplayManager()
 to get to the actual interface.
*/
class IGlobalDXDisplayManager : public FPStaticInterface
{
public:

	//!Sets a global overide to turn off display of Dx Shaders in the viewport
	/*!
	\param set Sets the Force Software flag to true/false.  If true default software rendering will be performed
	*/
	virtual void	SetForceSoftware(BOOL set=TRUE)=0;

	//!Sets the force display of Dx Effects when an object is selected
	/*!Sets the force display of Dx Effects when an object is selected.  This only works if SetForceSoftware is set to TRUE
	\param set Turns Force selected to on/off
	*/
	virtual void    SetForceSelected(BOOL set =TRUE )=0;

	//!Gets the state of the Force Software flag
	/*!
	\return Whether the viewports are using software rendering or not
	*/
	virtual BOOL	IsForceSoftware()=0;

	//!Gets the state of the Force Selected falg
	/*!
	\return Whether hardware rendering of the selected object is active or not
	*/
    virtual BOOL	IsForceSelected()=0;

	//!Queries whether DX is available - useful for UI handlers
	/*!
	\return Is DX active - if false it could either be OpenGL or Heidi
	*/
	virtual BOOL	IsDirectXActive()=0;

};

inline IGlobalDXDisplayManager* GetGlobalDXDisplayManager() { return (IGlobalDXDisplayManager*)GetCOREInterface(GLOBAL_DX_DISPLAY_MANAGER_INTERFACE); }


//! Provides methods to turn on DirectX based Scene effects
/*! 
\sa IDxSceneManager
*/
class IGlobalDXDisplayManager2 : public IGlobalDXDisplayManager
{
public:
	//! Are Dx scene %effects active.
	/*!
	\return True if scene effects are active
	*/
	virtual BOOL IsSceneActive()=0;

	//! Enables of disables Scene %effects
	/*!
	\param set Turn on/off scene effects
	*/
	virtual void SetSceneActive(BOOL set)=0;
};

inline IGlobalDXDisplayManager2* GetGlobalDXDisplayManager2() { return (IGlobalDXDisplayManager2*)GetCOREInterface(GLOBAL_DX_DISPLAY_MANAGER_INTERFACE); }


