/*==============================================================================

  file:	        IRadiosityPreferences.h

  author:       Daniel Levesque
  
  created:	    7 September 2001
  
  description:
        
        Interface for accessing radiosity preferences.


  modified:		September 14, 2001 David Cunningham
						- added use/compute radiosity file setting access

                September 17, 2001 Daniel Levesque
                        - addition of Get/Set 'Save Geometry' methods

				May 20, 2003 Alexandre Cossette
						- addition of Get/Set 'Update Data When Required on Start'

				June 9, 2003 Alexandre Cossette
						- removed 'Display Warning on GI Props Change'


(c) 2001 Autodesk
==============================================================================*/

#pragma once

#include "ifnpub.h"

#define IRADIOSITYPREFERENCES_INTERFACE Interface_ID(0x54442e40, 0x401621a4)

//==============================================================================
// class IRadiosityPreferences
//
// This class defines the interface for accessing the Radiosity Preferences
// from the 'Radiosity' tab in the preferences dialog.
//
// It also provides access to the use/compute radiosity controls found in the
// Rendering dialog.
//
// To get the a pointer to the interface, call:
//     static_cast<IRadiosityPreferences*>(GetCOREInterface(IRADIOSITYPREFERENCES_INTERFACE))
//
//==============================================================================
/*! \sa  : Class RadiosityEffect\n\n
\par Description:
This class is only available in release 5 or later.\n\n
This class defines the interface for accessing the Advanced Lighting
Preferences from the 'Advanced Lighting' tab in the preferences dialog. It also
provides access to the use/compute advanced lighting controls found in the
Rendering dialog. This is a function-published static interface; you can use
GetCOREInterface() to obtain an instance of the class, as follows:\n\n
 IRadiosityPreferences* r =
static_cast\<IRadiosityPreferences*\>(GetCOREInterface(IRADIOSITYPREFERENCES_INTERFACE));\n\n
This interface is also accessible via MAXScript as
"<b>RadiosityPreferences</b>".\n\n
All methods of this class are implemented by the system.\n\n
   */
class IRadiosityPreferences : public FPStaticInterface {

public:

	// Global user settings that are stored in a .ini file

    // Automatically process refine steps stored in objects
	/*! \remarks Returns the state of the checkbox "Automatically Process Refine
	Iterations Stored in Geometric Objects", in the preferences dialog Advanced
	Lighting tab*/
    virtual BOOL GetAutoProcessObjectRefine() const = 0;
	 /*! \remarks Sets the state of the checkbox "Automatically Process Refine
	 Iterations Stored in Geometric Objects", in the preferences dialog Advanced
	 Lighting tab\n\n

	 \par Parameters:
	 <b>BOOL val</b>\n\n
	 TRUE for on; FALSE for off.*/
    virtual void SetAutoProcessObjectRefine(BOOL val) = 0;
    //Display reflectance/transmittance information in the material editor
    /*! \remarks Returns the state of the checkbox "Display Reflectance \&
    Transmittance Information", in the preferences dialog Advanced Lighting
    tab\n\n
      */
    virtual BOOL GetDisplayReflectanceInMEditor() const = 0;
    /*! \remarks Sets the state of the checkbox "Display Reflectance \&
    Transmittance Information", in the preferences dialog Advanced Lighting
    tab\n\n

    \par Parameters:
    <b>BOOL val</b>\n\n
    TRUE for on; FALSE for off.\n\n
      */
    virtual void SetDisplayReflectanceInMEditor(BOOL val) = 0;
    // Display radiosity in the viewport
    /*! \remarks Returns the state of the checkbox "Display Radiosity in
    Viewports", in the preferences dialog Advanced Lighting tab\n\n
      */
    virtual BOOL GetDisplayInViewport() const = 0;
    /*! \remarks Sets the state of the checkbox "Display Radiosity in
    Viewports", in the preferences dialog Advanced Lighting tab\n\n

    \par Parameters:
    <b>BOOL val</b>\n\n
    TRUE for on; FALSE for off.\n\n
      */
    virtual void SetDisplayInViewport(BOOL val) = 0;
    // Display warning on reset
    /*! \remarks Returns the state of the checkbox "Display Reset Warning", in
    the preferences dialog Advanced Lighting tab\n\n
      */
    virtual BOOL GetDisplayResetWarning() const = 0;
    /*! \remarks Sets the state of the checkbox "Display Reset Warning", in
    the preferences dialog Advanced Lighting tab\n\n

    \par Parameters:
    <b>BOOL val</b>\n\n
    TRUE for on; FALSE for off.\n\n
      */
    virtual void SetDisplayResetWarning(BOOL val) = 0;
    // Automatically update solution data when required
    virtual BOOL GetUpdateDataWhenRequiredOnStart() const = 0;
    virtual void SetUpdateDataWhenRequiredOnStart(BOOL val) = 0;
    // Save the geometry along with the solution in the .max file, for faster load times.
    /*! \remarks Returns the state of the checkbox "Save Scene Information in
    MAX File", in the preferences dialog Advanced Lighting tab\n\n
      */
    virtual BOOL GetSaveScene() const = 0;
    /*! \remarks Sets the state of the checkbox "Save Scene Information in MAX
    File", in the preferences dialog Advanced Lighting tab\n\n

    \par Parameters:
    <b>BOOL val</b>\n\n
    TRUE for on; FALSE for off.\n\n
      */
    virtual void SetSaveScene(BOOL val) = 0;

    /**
     * The functions below provide access to current radiosity settings.
     * These properties are saved on a per file basis, so they
     * are not written into the .ini file.
     */
    // Use radiosity when rendering
    /*! \remarks Returns the state of the "Use Advanced Lighting" checkbox in
    the render dialog\n\n
      */
    virtual BOOL GetUseRadiosity() const = 0;
    /*! \remarks Sets the state of the "Use Advanced Lighting" checkbox in the
    render dialog\n\n

    \par Parameters:
    <b>BOOL val</b>\n\n
    TRUE for on; FALSE for off.\n\n
      */
    virtual void SetUseRadiosity(BOOL val) = 0;
    // Automatically compute radiosity when rendering, if necessary.
    /*! \remarks Returns the state of the "Compute Advanced Lighting when
    Required" checkbox in the render dialog\n\n
      */
    virtual BOOL GetComputeRadiosity() const = 0;
    /*! \remarks Sets the state of the "Compute Advanced Lighting when
    Required" checkbox in the render dialog\n\n

    \par Parameters:
    <b>BOOL val</b>\n\n
    TRUE for on; FALSE for off. */
    virtual void SetComputeRadiosity(BOOL val) = 0;
};


