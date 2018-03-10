 /**********************************************************************
 *<
	FILE: iColorMan.h

	DESCRIPTION: Color Manager

	CREATED BY:	Scott Morrison

	HISTORY: Created 19 April, 2000,

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#pragma once

#ifdef CUSTDLG_IMP
#define CustDlgExport __declspec(dllexport)
#else
#define CustDlgExport __declspec(dllimport)
#endif

#include "iFnPub.h"

typedef DWORD ColorId;

/*! \defgroup standardColorIDs Standard Color IDs
This is the list of the standard color IDs that 3ds Max registers.
\sa Class IColorManager
*/
//@{
const ColorId kBackground    = 0;  //!< used for all windows backgrounds
const ColorId kText          = 1;  //!< Used for static and button text
const ColorId kActiveCommand = 4;  //!< Used for active command mode buttons
const ColorId kHilight       = 5;  //!< COLOR_BTNHILIGHT
const ColorId kShadow        = 6;  //!< COLOR_BTNSHADOW
const ColorId kWindow        = 7;  //!< COLOR_WINDOW
const ColorId kActiveCaption = 8;  //!< COLOR_ACTIVECAPTION
const ColorId kToolTipBackground = 9; //!< COLOR_INFOBK
const ColorId kToolTipText   = 10; //!< COLOR_INFOTEXT
const ColorId kHilightText   = 11; //!< COLOR_HILIGHTTEXT
const ColorId kWindowText    = 12; //!< COLOR_WINDOWTEXT
const ColorId kItemHilight   = 13; //!< COLOR_HILIGHT
const ColorId kSubObjectColor= 14; //!< Blue sub-object color
const ColorId k3dDarkShadow  = 15; //!< COLOR_3DDKSHADOW
const ColorId k3dLight       = 16; //!< COLOR_3DLIGHT
const ColorId kAppWorkspace  = 17; //!< COLOR_APPWORKSPACE
const ColorId kTrackbarBg    = 18; //!< 
const ColorId kTrackbarBgSel = 19; //!< 
const ColorId kTrackbarText  = 20; //!< 
const ColorId kTrackbarTicks = 21; //!< 
const ColorId kTrackbarKeys  = 22; //!< 
const ColorId kTrackbarSelKeys = 23; //!< 
const ColorId kTrackbarCursor  = 24; //!< 
const ColorId kPressedButton   = 25; //!< 
const ColorId kTimeSliderBg    = 26; //!< 
const ColorId kViewportBorder  = 27; //!< 
const ColorId kActiveViewportBorder = 28; //!< 
const ColorId kRollupTitleFace      = 29; //!< 
const ColorId kRollupTitleText      = 30; //!< 
const ColorId kRollupTitleHilight   = 31; //!< 
const ColorId kRollupTitleShadow    = 32; //!< 
const ColorId kSelectionRubberBand  = 33; //!< 
const ColorId kStackViewSelection = 34; //!< yellow sub-object color
const ColorId kImageViewerBackground = 35; //!< defaults to grey
const ColorId kIRenderProgressHoriz = 36; //!< interactive rendering - horizontal progress bar
const ColorId kIRenderProgressVert  = 37; //!< interactive rendering - vertical progress bar
const ColorId kPressedHierarchyButton  = 38; //!< 
const ColorId kTrackViewBackground  = 39; //!< 
const ColorId kTrackViewInactiveBackground  = 40; //!< 
const ColorId kManipulatorsActive  = 41; //!< 
const ColorId kManipulatorsSelected  = 42; //!< 
const ColorId kManipulatorsInactive  = 43; //!< 
const ColorId kFunctionCurveX = 44; //!< 
const ColorId kFunctionCurveY = 45; //!< 
const ColorId kFunctionCurveZ = 46; //!<  
const ColorId kFunctionCurveW = 47;  //!< 
const ColorId kFunctionCurveFloat = 48;  //!< 
const ColorId kFunctionCurveSelected = 49;  //!< 
const ColorId kTrackViewTrackText = 50; //!< 
const ColorId kTrackViewSelInstantTime = 51; //!< 
const ColorId kTrackViewKeyCursor = 52; //!< 
const ColorId kTrackViewSelectedBackground = 53; //!< 
const ColorId kTrackViewDisabledTrack1 = 54; //!< 
const ColorId kTrackViewDisabledTrack2 = 55; //!< 
const ColorId kTrackViewEnabledTrack1 = 56; //!< 
const ColorId kTrackViewEnabledTrack2 = 57; //!< 
const ColorId kTrackViewTimeCursor = 58; //!< 
const ColorId kTrackViewGrid = 59; //!< 
const ColorId kTrackViewUnSelectedKeys = 60; //!< 
const ColorId kTrackViewSelectedKeys = 61; //!< 
const ColorId kTrackViewKeyOutline = 62; //!< 
const ColorId kTrackViewKeyOutlineOnTop = 63; //!< 
const ColorId kViewportShowDependencies = 64; //!< 
const ColorId kTrackViewSoundTrackRight = 65; //!< 
const ColorId kTrackViewSoundTrackLeft = 66; //!< 
const ColorId kAssemblyOutline	= 67; //!< 
const ColorId kTrackViewKeyPosition	= 68; //!< 
const ColorId kTrackViewKeyRotation	= 69; //!< 
const ColorId kTrackViewKeyScale = 70; //!< 
const ColorId kTrackViewKeyTransform = 71; //!< 
const ColorId kTrackViewKeyObject = 72; //!< 
const ColorId kTrackViewKeyMaterial = 73; //!< 
const ColorId kTrackViewTangentHandle = 74; //!< 
const ColorId kTrackViewAutoTangentHandle = 75; //!< 
const ColorId kTrackViewKeyMixed = 76; //!< 
const ColorId kTrackViewKeyFake = 77; //!< 

const ColorId kTrackbarKeyPosition	= 79; //!< 
const ColorId kTrackbarKeyRotation	= 80; //!< 
const ColorId kTrackbarKeyScale = 81; //!< 
const ColorId kTrackbarKeyTransform = 82; //!< 
const ColorId kTrackbarKeyObject = 83; //!< 
const ColorId kTrackbarKeyMaterial = 84; //!< 
const ColorId kTrackbarKeyMixed = 85; //!< 
const ColorId kTrackViewScaleOriginLine = 86; //!< 
const ColorId kTrackViewKeyFCurveSelected = 87; //!< 
const ColorId kTrackViewKeyFCurveUnSelected = 88; //!< 

const ColorId kOutOfRangeLow = 89; //!< 
const ColorId kOutOfRangeHigh = 90; //!< 

const ColorId kTrackViewRangeIgnored = 91; //!< 
const ColorId kTrackViewRangeIgnoredSel = 92; //!< 

const ColorId kAdaptiveDegActive = 93; //!<

const ColorId kTrackViewControllerWindowToggle  = 94; //!<  track view region selection tool - color of the controller window toggle
const ColorId kTrackViewRegionSelectBorder  = 95; //!< track view region selection tool - color of the border
const ColorId kTrackViewRegionSelectGrip  = 96; //!< track view region selection tool - color of the selectable grip area
const ColorId kTrackViewRegionSelectRegion = 97; //!< track view region selection tool - color of the region surrounding the selected keys
const ColorId kTrackViewRegionSelectOuterRegion = 98; //!< track view region selection tool - color of the outer region 

//@}

#define COLOR_MGR_INTERFACE  Interface_ID(0x1bf46c90, 0x18bf6199)

/*! \sa  Class FPStaticInterface
Class Point3, <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF</a>,
\ref standardColorIDs,
<a href="javascript:UniqueId0.Click()">Generate a Class_ID</a>,
Class GUP, <a href="ms-its:3dsmaxsdk.chm::/ui_customization_user_preferences.html">Getting and Setting User Preferences</a>.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class is an interface to the Color Manager. Within 3ds Max using the
Customize pull down menu / Customize User Interface choice / Colors tab a user
is able to alter the colors used for various UI elements. They can change the
saturation, value and transparency of elements, and load and save color
schemes.\n\n
Using the methods of this class developers can do the same (the 3ds Max color
manager uses this class internally). Developers can add their own named custom
colors. Developers wanting to do this need to pick a random 32-bit
<b>ColorId</b> to identify it. (Note: <b>typedef DWORD ColorId;</b>). The
Class_ID generator may be used for this where only one of the two DWORDS is
used. See Class Class_ID for more details.
The low integer ColorIds are reserved for 3ds Max internal use. These colors
should be registered on startup, so a Global Utility Plug-In (GUP) is the best
way to handle this. See Class GUP for details.
Methods that are marked as internal should not be used. */
class IColorManager: public FPStaticInterface {

public:

	enum ColorSchemeType
	{
		//! Customized color scheme
		CST_CUSTOM = 0,
		//! Standard windows color scheme
		CST_STANDARDWINDOWS,
		//! Windows theme color scheme
		CST_WINDOWSTHEME
	};

	enum AppFrameColorTheme {
		//! Dark theme
		kDarkTheme	= 0,
		//! Light theme
		kLightTheme	= 1
	};

    /*! \remarks Returns CST_CUSTOM if use the customized color scheme, or
    CST_STANDARDWINDOWS if use the Windows standard color scheme, or
    CST_WINDOWSTHEME if let Windows theme to control the color scheme.
    */
	virtual ColorSchemeType GetColorSchemeType() = 0;

    /*! \remarks Sets which color scheme are used. This allows the developer to 
    tell the system to use custom colors, standard Windows colors, and Windows theme.
    \par Parameters:
    <b>ColorSchemeType type</b>\n\n
    Pass CST_CUSTOM to use the customized color scheme.
    Pass CST_STANDARDWINDOWS to use the Windows standard color scheme.
    Pass CST_WINDOWSTHEME to let Windows theme to control the color scheme */
	virtual void SetColorSchemeType(ColorSchemeType type) = 0;

    // Register a new color with the system.
    // Return false if the color is already registered.
    /*! \remarks This method registers a new color with the system. For
    plug-in developers this should be done at startup using a Global Utility
    Plug-in which calls this method. See the Description section above for
    details. If developers want to add a color of their own, they need to pick
    a random 32-bit integer ColorId to identify it.
    \par Parameters:
    <b>ColorId id</b>\n\n
    The ID of the color to register. This should be generated by the developer
    using a single DWORD from the output of the Class_ID program. See
    Class Class_ID for more details.\n\n
    <b>MCHAR* pName</b>\n\n
    The name for the color.\n\n
    <b>MCHAR* pCategory</b>\n\n
    The category for the color. If the name passed matches one of the existing
    3ds Max categories the color will be place in there, otherwise a new one
    will be created.\n\n
    <b>COLORREF defaultValue</b>\n\n
    The default value for the color. This is the value that the color will be
    reset to when a 3ds Max user presses "Reset" in the color customization
    dialog. See <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF</a>.
    \return  Returns false if the color is already registered; otherwise true.
    */
    virtual bool RegisterColor(ColorId id, const MCHAR* pName, const MCHAR* pCategory, COLORREF defaultValue) = 0;

    /*! \remarks This method will load the specified color file from the
    current UI directory.
    \par Parameters:
    <b>MCHAR* pFileName</b>\n\n
    The filename of the color file to load.
    \return  TRUE if the load was successful, otherwise FALSE. */
    virtual BOOL LoadColorFile(const MCHAR* pFileName) = 0;
    /*! \remarks This method will save the specified color file from the
    current UI directory.
    \par Parameters:
    <b>MCHAR* pFileName</b>\n\n
    The filename of the color file to save.
    \return  TRUE if the save process was successful, otherwise FALSE. */
    virtual BOOL SaveColorFile(const MCHAR* pFileName) = 0;
    /*! \remarks This method returns the file name of the current color file.
    */
    virtual const MCHAR* GetColorFile() = 0;

    // Access to the customized colors
    /*! \remarks Sets the color value of the previously registered color whose
    ID is passed.
    \par Parameters:
    <b>ColorId id</b>\n\n
    Specifies which color to set.\n\n
    <b>COLORREF color</b>\n\n
    The color value to set. See <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF</a>.
    \return  Returns true if the color was set and false if the id passed could
    not be found. */
    virtual bool     SetColor(ColorId id, COLORREF color) = 0;
    /*! \remarks Returns the color value of the color whose ID is passed.
    \par Parameters:
    <b>ColorId id</b>\n\n
    Specifies which color to get.
    \return  The color is returned or black (RGB(0,0,0)) if the ColorId passed
    was not found. */
    virtual COLORREF GetColor(ColorId id) = 0;
    /*! \remarks This method returns the color associated with a specified
    color ID as a Point3.
    \par Parameters:
    <b>ColorId id</b>\n\n
    The ID of the color you wish to get. */
    virtual Point3   GetColorAsPoint3(ColorId id) = 0;
    /*! \remarks Returns a handle to the brush for the color whose id is
    specified. NULL is returned if the id passed is not found. Note that the
    color manager does resource management for brushes, so the value returned
    should not be deleted.
    \par Parameters:
    <b>ColorId id</b>\n\n
    The color whose brush handle is returned. */
    virtual HBRUSH   GetBrush(ColorId id) = 0;
    /*! \remarks Returns the name of the color whose ID is passed.
    \par Parameters:
    <b>ColorId id</b>\n\n
    The ID of the color. */
    virtual const MCHAR*   GetName(ColorId id) = 0;
    /*! \remarks Returns the category string of the color whose ID is passed.
    \par Parameters:
    <b>ColorId id</b>\n\n
    The ID of the color. */
    virtual const MCHAR*   GetCategory(ColorId id) = 0;


    /*! \remarks This method takes the specified windows color definition (for
    example <b>COLOR_BTNHILIGHT</b>) and returns the 3ds Max customized version
    of that color. Anyone writing a plug-in with custom windows code should use
    this call (and <b>CustSysColorBrush()</b> below) instead of the Win32
    <b>GetSysColor()</b> and <b>GetSysColorBrush()</b> if they want to
    participate in the 3ds Max custom color scheme.\n\n
    Note the following #define which may be used to simplify coding:\n\n
    <b>#define GetCustSysColor(which) (ColorMan()-\>CustSysColor(which))</b>
    \par Parameters:
    <b>int which</b>\n\n
    Specifies the windows color definition. See \ref standardColorIDs. For a full list of windows color definitions, please refer to the
    Win32 API, in particular the methods <b>GetSysColor()</b> and
    <b>SetSysColor()</b>. */
    virtual COLORREF CustSysColor(int which) = 0;
    /*! \remarks This method returns a handle identifying a logical brush that
    corresponds to the specified color index. Note that the color manager does
    resource management for brushes, so the value returned should not be
    deleted.
    \par Parameters:
    <b>int which</b>\n\n
    Specifies the windows color definition. See \ref standardColorIDs. For a full list of windows color definitions, please refer to the
    Win32 API, in particular the methods <b>GetSysColor()</b> and
    <b>SetSysColor()</b>.\n\n
    Note the following #define which may be used to simplify coding a bit:\n\n
    <b>#define GetCustSysColorBrush(which)
    (ColorMan()-\>CustSysColorBrush(which))</b> */
    virtual HBRUSH   CustSysColorBrush(int which) = 0;

    // Interface for old "GetUIColor" call from gfx.
    // The colors are defined in gfx.h
    /*! \remarks Returns the specified color value for drawing various items
    in the viewports. This is the same as the previous GetUIColor() function.
    \par Parameters:
    <b>int which</b>\n\n
    Specifies which color to retrieve. See \ref viewportDrawingColors.
    \return  The color as a Point3. */
    virtual Point3 GetOldUIColor(int which) = 0;
    /*! \remarks Sets the specified color value for drawing various items in
    the viewports. This is the same as the previous SetUIColor() function.
    \par Parameters:
    <b>int which</b>\n\n
    Specifies which color to retrieve. See \ref viewportDrawingColors.\n\n
    <b>Point3 *clr</b>\n\n
    Points to the color value to set. */
    virtual void   SetOldUIColor(int which, Point3 *clr) = 0;
    /*! \remarks Returns the default color used for drawing various items in
    the 3ds Max user interface. The values returned are not affected by the
    user's color selections or those set by SetUIColor(). This is the same as
    the previous GetDefaultUIColor() function.
    \par Parameters:
    <b>int which</b>\n\n
    Specifies which color to retrieve. See \ref viewportDrawingColors. */
    virtual Point3 GetOldDefaultUIColor(int which) = 0;

    // Icon Image processing
    // The following values are used to do image processing on the icons
    // at start-up time.
    enum IconType {
        kDisabledIcon,
        kEnabledIcon
    };

    enum IconColorScale {
        kSaturationScale,
        kValueScale,
        kAlphaScale,
		kContrastScale,
		kGradientMapScale
    };

    /*! \remarks Returns a floating point value (in the range 0.0f to 1.0f)
    that is one of the scale factors applied to the specified icon type. These
    scale values used to do image processing on the icons at start-up time.
    \par Parameters:
    <b>IconType type</b>\n\n
    The icon type. One of the following values:\n\n
    <b>kDisabledIcon</b>\n\n
    The disabled icons.\n\n
    <b>kEnabledIcon</b>\n\n
    The enabled icons.\n\n
    <b>IconColorScale which</b>\n\n
    The icon color scale. One of the following values:\n\n
    <b>kSaturationScale</b>\n\n
    The saturation scale.\n\n
    <b>kValueScale</b>\n\n
    The value scale.\n\n
    <b>kAlphaScale</b>\n\n
    The alpha scale.
	<b>kContrastScale</b>\n\n
	The contrast scale.
	<b>kGradientMapScale</b>\n\n
	The gradient map scale. */
    virtual float GetIconColorScale(IconType type, IconColorScale which) = 0;
    /*! \remarks Sets the specified scale factor for the icon type passed. The
    color manager maintains the values for the 3ds Max icon image processing
    system. Developers can set values to scale the saturation, value and
    transparency for enabled and disabled icon images using this method.
    \par Parameters:
    <b>IconType type</b>\n\n
    The icon type. One of the following values:\n\n
    <b>kDisabledIcon</b>\n\n
    The disabled icons.\n\n
    <b>kEnabledIcon</b>\n\n
    The enabled icons.\n\n
    <b>IconColorScale which</b>\n\n
    The icon color scale. One of the following values:\n\n
    <b>kSaturationScale</b>\n\n
    The saturation scale.\n\n
    <b>kValueScale</b>\n\n
    The value scale.\n\n
    <b>kAlphaScale</b>\n\n
    The alpha scale.\n\n
    <b>float value</b>\n\n
    The value to set (in the range 0.0f to 1.0f). */
    virtual void  SetIconColorScale(IconType type, IconColorScale which, float value) = 0;

    /*! \remarks Returns true if the invert flag is set for the specified icon
    type and false if not set.
    \par Parameters:
    <b>IconType type</b>\n\n
    The icon type. One of the following values:\n\n
    <b>kDisabledIcon</b>\n\n
    The disabled icons.\n\n
    <b>kEnabledIcon</b>\n\n
    The enabled icons. */
    virtual bool GetIconColorInvert(IconType type) = 0;
    /*! \remarks Sets the invert flag for the specified icon type to on or
    off.
    \par Parameters:
    <b>IconType type</b>\n\n
    The icon type. One of the following values:\n\n
    <b>kDisabledIcon</b>\n\n
    The disabled icons.\n\n
    <b>kEnabledIcon</b>\n\n
    The enabled icons.\n\n
    <b>bool value</b>\n\n
    Pass true for inverted; false for not inverted. */
    virtual void SetIconColorInvert(IconType type, bool value) = 0;

	/*! \remarks Returns the current color theme.\n\n
	*/
	virtual AppFrameColorTheme GetAppFrameColorTheme() const = 0;
	/*! \remarks Sets the current color theme.
    \par Parameters:
    <b>AppFrameColorTheme value</b>\n\n
    The value of color theme to be set.\n\n
    */
	virtual void SetAppFrameColorTheme(AppFrameColorTheme value) = 0;

    /*! \remarks Returns the file name of the currently loaded color file. */
    virtual const MCHAR* GetFileName() = 0;

    /*! \remarks Returns the default color for the specified ID. The default
    color is the value passed as <b>defaultValue</b> in <b>RegisterColor()</b>,
    regardless if a <b>SetColor()</b> has been done subsequently. This is used
    by the UI when the user presses "Reset" to reset a color to its default
    value.
    \par Parameters:
    <b>ColorId id</b>\n\n
    The ID of the color. */
    virtual COLORREF GetDefaultColor(ColorId id) = 0;

    // Get an old UI color as a COLORREF
    /*! \remarks This method returns the color associated with a user
    interface color as a COLORREF.
    \par Parameters:
    <b>int which</b>\n\n
    The UI color index (see gfx.h). */
    /*! \remarks Returns the specified color value for drawing various items
    in the viewports as a COLORREF.
    \par Parameters:
    <b>int which</b>\n\n
    Specifies which color to retrieve. See \ref viewportDrawingColors. */
    virtual COLORREF GetOldUIColorCOLORREF(int which) = 0;

    enum RepaintType {
        kRepaintAll,
        kRepaintTrackBar,
        kRepaintTimeBar,
    };

    /*! \remarks This method allows you to issue a repaint of the user
    interface.
    \par Parameters:
    <b>RepaintType type</b>\n\n
    The type of repaint you wish to issue; <b>kRepaintAll</b>,
    <b>kRepaintTrackBar</b>, <b>kRepaintTimeBar</b>. */
    virtual void RepaintUI(RepaintType type) = 0;

    // Set the folder that is searched for UI icon files.
    // This must be a folder that lives under the "UI" folder.
    /*! \remarks This method takes the name of a folder that must be in 3ds
    Max "UI" folder. If the folder exists, then it sets 3ds Max icon folder to
    point to it, and redraws the UI with those new icons.Warning: All of the
    3ds Max standard icon BMP files must exist in that folder. If any of the
    standard files are missing, icons will appear blank in the UI. All the
    icons files needed live in the <b>UI/icons</b> folder, which is the default
    icon folder.
    \par Parameters:
    <b>MCHAR* pFolder</b>\n\n
    The icon folder to set. */
    virtual BOOL SetIconFolder(const MCHAR* pFolder) = 0;

    // returns the full path to the icon folder.
    virtual const MCHAR* GetIconFolder() = 0;

	// reinitializes the display icons using the UI icon files.
	virtual void ReInitIcons() = 0;

	// given an icon file name (no path), locate the icon file in the various
	// icon folders and return the fully resolved file name with path. If file
	// is found return TRUE and fill in path, otherwise return FALSE;
	virtual BOOL ResolveIconFolder(const MCHAR* pFilename, MSTR &path) = 0;

#ifdef USE_NEW_CUI_IO_METHODS // russom - 02/16/02
	// save and load icon folder file
	virtual BOOL SaveIconFile( const MCHAR *szFilename, const MCHAR *szIconPath ) = 0;
	virtual BOOL LoadIconFile( const MCHAR *szFilename ) = 0;
	virtual const MCHAR* ReadIconFile() = 0;
#endif

	//Function-published interface
	enum {
#ifndef NO_CUI	// russom - 02/12/02
        registerColor,
		loadColorFile,
        saveColorFile,
        getColorFile,
        setColor,
#endif // NO_CUI
        getColor,
		getName,
        getCategory,
        getIconColorScale,
#ifndef NO_CUI	// russom - 02/20/02
        setIconColorScale,
#endif
		getIconColorInvert,
#ifndef NO_CUI	// russom - 02/20/02
        setIconColorInvert,
#endif
        getFileName,
        getDefaultColor,
        getOldUIColorCOLORREF,
        repaintUI,
#ifndef NO_CUI	// russom - 02/20/02
        setIconFolder,
		reInitIcons,
#endif
        getIconFolder,
#ifdef USE_NEW_CUI_IO_METHODS
		saveIconFile,
 #ifndef NO_CUI	// russom - 02/25/02
		loadIconFile,
		readIconFile,
 #endif // NO_CUI
#endif // USE_NEW_CUI_IO_METHODS
		resolveIconFolder,
		getColorSchemeType,
		setColorSchemeType
	};

    // Function-published enums
    enum {
        iconType,
        iconColorScale,
        repaintType,
		colorSchemeType,
    };
};

// defined in CustomizationDialgos/CustColorDlg.cpp
CustDlgExport void SaveColors();

#ifdef CoreExport
CoreExport void DeleteColorManager(IColorManager* pColorMan);

/*! \remarks This global function is available in release 4.0 and later
only.\n\n
Returns a pointer to the color manager with which you can call the methods of
this class.\n\n
Note the following #define which may be used to simplify coding a bit:\n\n
<b>#define ColorMan() (GetColorManager())</b>  */
CoreExport IColorManager* GetColorManager();
#endif

// Easy access macros for the system's color manager
#define ColorMan() (GetColorManager())
#define GetCustSysColor(which) (ColorMan()->CustSysColor(which))
#define GetCustSysColorBrush(which) (ColorMan()->CustSysColorBrush(which))


	
