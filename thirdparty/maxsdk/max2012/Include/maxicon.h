 /**********************************************************************
 *<
   FILE: MaxIcon.h

   DESCRIPTION: Max Icon and Icon Table definitions

   CREATED BY: Scott Morrison

   HISTORY: Created 15 March, 2000,

 *>   Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "object.h"
#include "iColorMan.h"
#include "strbasic.h"

class ICustButton;

/*! \sa  Class InterfaceServer, Class ICustButton, Class MaxBmpFileIcon.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This abstract class represents an icon image for toolbar buttons, icons in list
boxes, etc. The class is based on Win32 ImageLists. MaxIcons must provide an
image list and index into the list for both large (24x24) and small (16x15 or 16x16) icons.
*/
class MaxIcon : public InterfaceServer {
public:
	//! Destructor
	CoreExport virtual ~MaxIcon();
    // Get the image list for the size of icons that the user has chose
    /*! \remarks Returns the handle to the image list for the size of icons
    that the user has chosen. */
    virtual HIMAGELIST GetDefaultImageList()  = 0;
    // Get the image list for small icons
    /*! \remarks Returns the image list for small icons. */
    virtual HIMAGELIST GetSmallImageList()  = 0;
    // Get the image list for large icons
    /*! \remarks Returns the image list for large icons. */
    virtual HIMAGELIST GetLargeImageList()  = 0;

    // Get the index into the image list for the small version of this
    // particular icon.
	 /*! \remarks Returns the zero based index into the image list for the small
	 version of this particular icon.
	 \par Parameters:
	 <b>bool enabledVersion = true</b>\n\n
	 Pass true to get the index of the enabled version of the icon; false to get the
	 disabled version.\n\n
	 <b>COLORREF backgroundColor = GetCustSysColor(COLOR_BTNFACE)</b>\n\n
	 The background color to use for the icon.\n\n
	 Specifies the windows color definition. See \ref standardColorIDs.
	 For a full list of windows color definitions, please refer to the Win32 API, in
	 particular the methods <b>GetSysColor()</b> and <b>SetSysColor()</b>. */
    virtual int GetSmallImageIndex(bool enabledVersion = true,
                                   COLORREF backgroundColor =
                                   GetCustSysColor( COLOR_BTNFACE) ) = 0;
    // Get the index into the image list for the large version of this
    // particular icon.
	 /*! \remarks Returns the zero based index into the image list for the large
	 version of this particular icon.
	 \par Parameters:
	 <b>bool enabledVersion = true</b>\n\n
	 Pass true to get the enabled version of the icon or false to get the disabled
	 version.\n\n
	 <b>COLORREF backgroundColor = GetCustSysColor(COLOR_BTNFACE)</b>\n\n
	 The background color to use for the icon.\n\n
	 Specifies the windows color definition. See \ref standardColorIDs.
	 For a full list of windows color definitions, please refer to the Win32 API, in
	 particular the methods <b>GetSysColor()</b> and <b>SetSysColor()</b>. */
    virtual int GetLargeImageIndex(bool enabledVersion = true,
                                   COLORREF backgroundColor =
                                   GetCustSysColor( COLOR_BTNFACE) ) = 0;
    // Get the index into the image list for the default version of this
    // particular icon.
	 /*! \remarks Returns the zero based index into the image list for the default
	 version of this particular icon.
	 \par Parameters:
	 <b>bool enabledVersion = true</b>\n\n
	 Pass true to get the enabled version of the icon or false to get the disabled
	 version.\n\n
	 <b>COLORREF backgroundColor = GetCustSysColor(COLOR_BTNFACE)</b>\n\n
	 The background color to use for the icon.\n\n
	 Specifies the windows color definition. See \ref standardColorIDs.
	 For a full list of windows color definitions, please refer to the Win32 API, in
	 particular the methods <b>GetSysColor()</b> and <b>SetSysColor()</b>. */
            int GetDefaultImageIndex(bool enabledVersion = true,
                                     COLORREF backgroundColor =
                                     GetCustSysColor( COLOR_BTNFACE) );

    // returns true if the icons has an alpha mask that needs to be blended
    // with the background color.
    /*! \remarks This method returns true if the icons has an alpha mask that
    needs to be blended with the background color and false if it doesn't use
    an alpha mask. */
    virtual bool UsesAlphaMask() = 0;
};

// This implementation of MaxIcon is for the icon images that are stored
// as ".bmp" files in MAX's UI directory.  This is used by the macroScript
// facility in MAXSrcipt to specify icons.  See the documentation for
// "macroScript" for the exact meaning of the filename and index.

/*! \sa  Class MaxIcon, Class ICustButton.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This implementation of MaxIcon is for the icon images that are stored as ".bmp"
files in 3ds Max's UI directory. This is used by the macroScript facility in
MAXSrcipt to specify icons. See the MAXScript documentation for "macroScript"
for the exact meaning of the filename and index and details.  */
class MaxBmpFileIcon: public MaxIcon {
public:
	//! Default Destructor
	CoreExport ~MaxBmpFileIcon();
    /*! \remarks Constructor.
    \par Parameters:
    <b>MCHAR* pFilePrefix</b>\n\n
    The file prefix to initialize with.\n\n
    <b>int index</b>\n\n
    The index of the icon. */
    CoreExport MaxBmpFileIcon(const MCHAR* pFilePrefix, int index);
	/*! \remarks Constructor.
	\par Parameters:
	<b>SClass_ID sid</b>\n\n
	The superclass ID\n\n
	<b>Class_ID cid</b>\n\n
	The class ID/ */
	CoreExport MaxBmpFileIcon(SClass_ID sid, Class_ID cid);

    /*! \remarks Returns the handle to the image list for the size of icons
    that the user has chosen. */
    CoreExport HIMAGELIST GetDefaultImageList();
    /*! \remarks Returns the image list for small icons. */
    CoreExport HIMAGELIST GetSmallImageList();
    /*! \remarks Returns the image list for large icons. */
    CoreExport HIMAGELIST GetLargeImageList();

	 /*! \remarks Returns the zero based index into the image list for the small
	 version of this particular icon.
	 \par Parameters:
	 <b>bool enabledVersion = true</b>\n\n
	 Pass true for the enalbed version of the icon; false for the disabled
	 version.\n\n
	 <b>COLORREF backgroundColor = GetCustSysColor(COLOR_BTNFACE)</b>\n\n
	 The background color for use in alpha blending. The files that define these
	 icons always have an alpha mask, and so a background color is needed to blend
	 it with. */
    CoreExport int GetSmallImageIndex(bool enabledVersion = true,
                                      COLORREF backgroundColor =
                                      GetCustSysColor( COLOR_BTNFACE) );
	 /*! \remarks Returns the zero based index into the image list for the large
	 version of this particular icon.
	 \par Parameters:
	 <b>bool enabledVersion = true</b>\n\n
	 Pass true for the enalbed version of the icon; false for the disabled
	 version.\n\n
	 <b>COLORREF backgroundColor = GetCustSysColor(COLOR_BTNFACE)</b>\n\n
	 The background color for use in alpha blending. The files that define these
	 icons always have an alpha mask, and so a background color is needed to blend
	 it with. */
    CoreExport int GetLargeImageIndex(bool enabledVersion = true,
                                      COLORREF backgroundColor =
                                      GetCustSysColor( COLOR_BTNFACE) );

    /*! \remarks Returns true if the icon uses an alpha mask; otherwise false.
    */
    CoreExport bool UsesAlphaMask();
    /*! \remarks Returns the directory of the icon. */
    CoreExport MSTR& GetFilePrefix() { return mFilePrefix; }
    /*! \remarks Returns the index of the icon in the image file list. */
    CoreExport int GetIndex() { return mIndex; }

    /*! \remarks Tells the icon to limit color correction. */
    CoreExport void SetKeepColor() { mKeepColor = true; }

    /*! \return Tells the icon to limit color correction. */
    CoreExport bool GetKeepColor() { return mKeepColor; }

private:
    int     mIndex;
    MSTR    mFilePrefix;
    bool    mKeepColor;
};

CoreExport HIMAGELIST GetIconManDefaultImageList();
CoreExport HIMAGELIST GetIconManSmallImageList();
CoreExport HIMAGELIST GetIconManLargeImageList();

CoreExport BOOL LoadMAXFileIcon(MCHAR* pFile, HIMAGELIST hImageList, ColorId color, BOOL disabled);

