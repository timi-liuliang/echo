/**********************************************************************
 *<
	FILE: ICommandPanel

	DESCRIPTION: Command Panel API

	CREATED BY: Nikolai Sander

	HISTORY: created 7/11/00

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "ifnpub.h"
#include "GetCOREInterface.h"

/*! \sa  Class FPStaticInterface,  Class Interface\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface to the Command Panel. You can obtain a
pointer to the Command Panel interface using; <b>ICommandPanel*
GetICommandPanel()</b>. This macro will return\n\n
<b>(ICommandPanel*)GetCOREInterface(COMMAND_PANEL_INTERFACE).</b>\n\n
All methods of this class are Implemented by the System.  */
class ICommandPanel : public FPStaticInterface 
{
	public:
	
	// function IDs 
	enum { 
		   fnIdGetRollupThreshhold,
		   fnIdSetRollupThreshhold,
	};
	
	/*! \remarks This method returns the command panel's rollup threshold.\n\n
	The threshold (which defaults to 50 pixels) affects the way rollups are
	placed when the command panel is resized (from one to multiple columns). A
	number of rollups remain in the left column while others move over to the
	second and/or additional columns based on this. A lower threshold will
	increase the likelihood that a panel gets shifted over to the next column.
	*/
	virtual int GetRollupThreshold()=0;
	/*! \remarks This method allows you to set the command panel's rollup
	threshold.
	\par Parameters:
	<b>int iThresh</b>\n\n
	The threshold you wish to set, in pixels */
	virtual void SetRollupThreshold(int iThresh)=0;
};

#define COMMAND_PANEL_INTERFACE Interface_ID(0x411753f6, 0x69a93710)
inline ICommandPanel* GetICommandPanel() { return (ICommandPanel*)GetCOREInterface(COMMAND_PANEL_INTERFACE); }

