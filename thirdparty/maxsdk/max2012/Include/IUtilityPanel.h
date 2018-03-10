/**********************************************************************
 *<
	FILE: IUtilityPanel.h

	DESCRIPTION: Function-published interface to access the utility panel

	CREATED BY: David Cunningham

	HISTORY: August 31, 2001 file created

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "iFnPub.h"
#define IUTIL_FO_INTERFACE Interface_ID(0x7476dc, 0x4364dc)


//==============================================================================
// class IUtilityPanel
//  
// The interface to the utility panel manager.
//
// This class defines an interface for opening and closing utility plugins, and
// provides maxscript access to the same.
//
// An instance of this interface can be retrieved using the following line of
// code:
//
//   static_cast<IUtilityPanel*>(GetCOREInterface(IUTIL_FO_INTERFACE));
//
// Or, from maxscript (examples):
//
//   UtilityPanel.OpenUtility Resource_Collector
//   UtilityPanel.OpenUtility Bitmap_Photometric_Path_Editor
//   UtilityPanel.CloseUtility
//
// The last example closes the currently open utility, if any.
//
//==============================================================================

// maxscript method enum
enum { util_open, util_close };

/*!  \n\n
class IUtilityPanel : public FPStaticInterface\n\n

\par Description:
The interface to the utility panel manager. This class defines an interface for
opening and closing utility plugins, and provides maxscript access to the
same.\n\n
An instance of this interface can be retrieved as\n\n
<b>static_cast\<IUtilityPanel*\>(GetCOREInterface(IUTIL_FO_INTERFACE));</b>\n\n
   */
class IUtilityPanel : public FPStaticInterface	{
public:

	/*!   \remarks Opens the Utility plugin in UI\n\n

	\par Parameters:
	<b>ClassDesc*</b>\n\n
	The Utility class name to open\n\n

	\par Return Type:
	TRUE if succeeded\n\n
	  */
	virtual BOOL OpenUtility(ClassDesc*) = 0;
	/*! \remarks Closes the Utility plugin in UI*/
	virtual void CloseUtility() = 0;

};


