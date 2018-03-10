//-----------------------------------------------------------------------------
// ---------------------
// File ....: ViewFile.h
// ---------------------
// Author...: Gus J Grubba
// Date ....: September 1995
// O.S. ....: Windows NT 3.51
//
// History .: Nov, 02 1995 - Created
//
// This is the "View File" option in MAX's File menu.
//
//-----------------------------------------------------------------------------

#pragma once
#include "maxheap.h"
#include <WTypes.h>

#ifndef  VWFEXPORT
#define  VWFEXPORT __declspec( dllimport )
#endif

//-----------------------------------------------------------------------------
//--  Base Class Definition ---------------------------------------------------
//-----------------------------------------------------------------------------
// #> ViewFile
//
     
/*! class ViewFile
\par Description:
This class allows a developer to replace the file viewer used by 3ds Max (This
is the "View File" option in 3ds Max's File menu). By creating a DLL from this
class, and replacing the standard 3ds Max ViewFile. DLL the system will always
use the developer defined version. Note: To execute this plug-in, put the DLL
in the same directory as the 3DSMAX.EXE executable.\n\n
The following two functions are called by the system to create and delete the
instance of this class that handles the file viewing.\n\n
<b>void *ViewFileCreate();</b>\n\n
This function is implemented by the plug-in to create a new instance of this
class. For example:
\code
void *ViewFileCreate()
{
	return new ViewFile;
}
void ViewFileDestroy(ViewFile *v);
\endcode 
This function is implemented by the plug-in to delete the instance of this
class created above. For example:
\code
void ViewFileDestroy(ViewFile *v)
{
	if (v)
		delete v;
}
\endcode */
class ViewFile: public MaxHeapOperators {

     private:   
        
        //-- Windows Specific -------------------------------------------------
        
        HWND              hWnd;

     public:

        //-- Constructors/Destructors -----------------------------------------

        VWFEXPORT         ViewFile           ( );
        VWFEXPORT        ~ViewFile           ( );
     
        //-- The Method -------------------------------------------------------
        //

        /*! \remarks This method is called by the system to bring up the file viewer.
        \par Parameters:
        <b>HWND hWnd</b>\n\n
        The parent window handle.\n\n
        \sa  <b>/MAXSDK/SAMPLES/VIEWFILE/VIEWFILE.CPP</b> for an example of
        the standard 3ds Max file viewer. */
        VWFEXPORT void    View              ( HWND hWnd );

};

//-----------------------------------------------------------------------------
//-- Interface

VWFEXPORT void *ViewFileCreate  ( );
VWFEXPORT void  ViewFileDestroy ( ViewFile *v);


//-- EOF: ViewFile.h ----------------------------------------------------------
