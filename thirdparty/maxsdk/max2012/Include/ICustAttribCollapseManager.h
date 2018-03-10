/**********************************************************************

FILE:           ICustAttribCollapseManager.h

DESCRIPTION:    Public interface controlling whether Custom Attributes should survive a stack collapse

CREATED BY:     Neil Hazzard, Discreet

HISTORY:        Created 8th April 2004

*>	Copyright (c) 2004, All Rights Reserved.
**********************************************************************/
#pragma once

#include "iFnPub.h"
#include "GetCOREInterface.h"

#define CUSTATTRIB_COLLAPSE_MANAGER Interface_ID(0x7abc18a6, 0x2a7e522b)

//! This interface provides access for setting and querying the state of flag that defines whether a Custom Attribute should survive a stack collapse
/*!
	When the calling code needs to collapse the stack, it should bracket the call with an Enumeration using a class derived from GeomPipelineEnumProc.
	Now the maxsdk contains two types of enumeration callbacks.  One which collapses the stack with out any regard to Custom Attributes, and another that
	will copy Custom Attributes to the base object.  The calling code can find out which to use by using this interface, the user can set the state of flag
	explicitly defining the path to take.

	The calling code should only really need to query the flag, and not set it.  The user will most likely control the flag
*/

class ICustAttribCollapseManager : public FPStaticInterface
{
public:

	//! \brief Gets the current state of the survive stack collapse flag
	/* \return True if the Custom Attributes should be maintained during a stack collapse */
	virtual bool GetCustAttribSurviveCollapseState() = 0;

	//! \brief Sets the current state of the survive stack collapse flag
	/*	\param state True if the Custom Attributes should be maintained during a stack collapse */
	virtual void SetCustAttribSurviveCollapseState(bool state) = 0;

	//! \brief Gets the current state of the retain subAnim Custom Attributes flag
	/*	\return True if the Custom Attributes on subAnims of modifiers should be maintained during a stack collapse */
	virtual bool GetRetainSubAnimCustAttribs() = 0;

	//! \brief Sets the current state of the retain subAnim Custom Attributes flag
	/*	\param state True if the Custom Attributes on subAnims of modifiers should be maintained during a stack collapse */
	virtual void SetRetainSubAnimCustAttribs(bool state) = 0;

	//! \brief Returns an instance of the ICustAttribCollapseManager interface
	static ICustAttribCollapseManager* GetICustAttribCollapseManager() { 
		return static_cast<ICustAttribCollapseManager*>(GetCOREInterface(CUSTATTRIB_COLLAPSE_MANAGER));
	}

};


