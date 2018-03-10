/**********************************************************************
 *<
	FILE: radiosity.h

	DESCRIPTION: Definitions for radiosity plugin

	CREATED BY: Cleve Ard

	HISTORY:
        [d.levesque | 21August2001]
            Addition of IRadiosityEffectExtension interface.

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/


#pragma once

#include "render.h"

typedef SFXParamDlg RadiosityParamDlg;

#pragma warning(push)
#pragma warning(disable:4100)

/*=====================================================================
 * Radiosity Interface class
 *===================================================================*/

/*! \sa Class ObjLightDesc, Class RadiosityInterface, Class IRadiosityEffectExtension,  Class NodeDisplayCallback,  Class IRadiosityPreferences\n\n
\par Description:
This class is only available in release 5 or later.\n\n
The class provides the interfaces for creating Advanced Lighting plug-ins in
3ds Max. RadiosityEffect plug-ins are controlled through the Advanced Lighting
dialog from the render menu.\n\n
A RadiosityEffect behaves as a light in the scene. An implementation of
RadiosityEffect must also provide an implementation of ObjLightDesc, to be
returned from RadiosityEffect::CreateLightDesc(). This ObjLightDesc is then
used by the renderer to apply the radiosity light during a render.\n\n
The architecture is designed to support scene based lighting solvers (which use
a precalculated solution), as well as image based solvers (which calculate
their solution at render time). The method RunProcess() and its relatives,
StopProcess(), AbortProcess(), and WaitForCompletion() are called by the system
to request processing of a scene based solution. Image based solvers only need
stub implementations of these methods.\n\n
A RadiosityEffect can use a NodeDisplayCallback to
control the viewport display of scene geometry. In particular, it is possible
for a lighting solution to be displayed interactively in the viewport if the
NodeDisplayCallback provides geometry with appropriate vertex illumination.\n\n
A RadiosityEffect may want to reference objects in the scene to detect events
which invalidate its lighting solution. A problem arises that the
RadiosityEffect can become inundated with reference messages, so to address
this problem the RadiosityEffect should ignore any message where the
PART_EXCLUDE_RADIOSITY flag is set in the PartID of the message. Any message
where ((partID|PART_EXCLUDE_RADIOSITY)!=0) is an event that the system deems
should not invalidate the radiosity, for example, when texture display in
viewport is turned on or off.\n\n
See also IsInterestedInChannels() in
Class IRadiosityEffectExtension.\n\n
All methods of this class are virtual.\n\n */
class RadiosityEffect : public SpecialFX {
public:
	enum CompletionCode {
		PROCESS_COMPLETED,
		PROCESS_TIMED_OUT,
		PROCESS_CANCELED,
		PROCESS_ABORTED
	};

	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	      PartID& partID,  RefMessage message) {return REF_SUCCEED;}
	SClass_ID SuperClassID() {return RADIOSITY_CLASS_ID;}
	
	//! \brief Saves name. These should be called at the start of
	//! a plug-in's save and load methods.
	IOResult Save(ISave *isave) { return SpecialFX::Save(isave); }
	//! \brief Loads name. These should be called at the start of
	//! a plug-in's save and load methods.
	IOResult Load(ILoad *iload) { return SpecialFX::Load(iload); }

	//! \brief Enables or disables the radiosity effect.
	/*! Corresponds to the
	Active checkbox in the Advanced Lighting dialog. Although a TimeValue
	parameter is passed to this method, the active state of the plug-in is not
	animatable. Plug-ins should call the default implementation of SetActive(),
	optionally adding their own code.
	\param active - Specifies whether the plug-in is active or inactive
	\param t - Specifies the current scene time when the method is called.
	  */
	virtual void SetActive(
		bool		active,
		TimeValue	t) 
	{
		if (active ^ (TestAFlag(A_ATMOS_DISABLED) == 0)) {
			if (active) {
				ClearAFlag(A_ATMOS_DISABLED);
			}
			else {
				SetAFlag(A_ATMOS_DISABLED);
			}
			NotifyDependents(FOREVER, (PartID)PART_ALL, REFMSG_CHANGE);
		}
	}

	//! \brief Merges this radiosity solution into the current solution.
	/*! This method is called for the currently selected radiosity plugin, when merging 
	objects from a file that also has a radiosity plugin. The default behaviour does 
	not merge the solutions.
	\param [in] iload - The file loader used to merge this radiosity solution 
	\param [in] id - Identifies the class of the radiosity plugin being loaded.
	\return IO_OK if the merge succeeded.
	*/
	virtual IOResult Merge(ILoad* iload, const Class_ID& id) { return IO_OK; }

	//! \brief Put up a modal dialog that lets the user edit the plug-ins parameters.
	/*! This method creates and returns a new instance of
	RadiosityParamDlg to manager the user interface. This displays one or more
	rollouts in the Advanced Lighting dialog. A typical implementation would call
	ClassDesc2::CreateParamDialogs() to instantiate the RadiosityParamDlg.
	\param ip - This is the interface given to the plug-in so it may display its parameters.
	\return  The instance of RadiosityParamDlg to manager the plug-in's UI
	*/
	virtual RadiosityParamDlg *CreateParamDialog(IRendParams *ip)	{ return NULL; }

	// Implement this if you are using the ParamMap2 AUTO_UI system and the 
	// atmosphere has secondary dialogs that don't have the effect as their 'thing'.
	// Called once for each secondary dialog for you to install the correct thing.
	// Return TRUE if you process the dialog, false otherwise.
	//! \brief Called once for each secondary dialog for you to install the	correct thing
	/*! Implement this if you are using the ParamMap2 AUTO_UI system
	and the RadiosityEffect has secondary dialogs that don't have the effect as
	their 'thing'. 
	\note Developers needing more information on this method can see the
	remarks for <a href="class_mtl_base.html">MtlBase::CreateParamDlg()</a>
	which describes a similar example of this method in use (in that case it's
	for use in a texture map plug-in).
	\param dlg - Points to the ParamDlg.
	\return  Return TRUE if you process the dialog; otherwise FALSE
	*/
	virtual BOOL SetDlgThing(RadiosityParamDlg* dlg) { return FALSE; }

	// Render access

	//! \brief Returns whether the given light should render it's illumination	in the production render. 
	/*! If the return value is true, the light is disabled
	while rendering.\n\n
	This is used to allow the radiosity plug-in to override lights in the scene
	with light from it's own solution.
	\param node - The INode of the light.
	\return  Return true if the light should be enabled while rendering; or false
	if it should be disabled
	*/
	virtual bool UseLight(INode* node, bool recalcSolution = false) { return true; }

	//! \brief Create light objects that the renderer can use to get the
	//! radiosity contribution. 
	/*! NumLightDesc returns the number of
	ObjLightDesc objects the radiosity plugin needs for
	rendering. CreateLightDesc creates all of the ObjLightDesc
	objects, and stores their addresses in buffer. Buffer must be
	large enough to hold all of the addresses.
	\return  The number of ObjLightDesc objects the RadisoityEffect will return
	from CreateLightDesc() */
	virtual int NumLightDesc( ) const = 0;
	
	//! \brief Creates light objects that the renderer can use to get the
	//! RadiosityEffect's contribution. 
	/*! CreateLightDesc() creates a number of
	ObjLightDesc objects indicated by NumLightDesc(), and stores their
	addresses in the buffer. Caller is responsible for ensuring that the buffer
	is large enough.\n\n
	Note: the caller will delete the ObjLightDesc objects when the render is
	completed, so this method should dynamically allocate the ObjLightDesc
	instances, rather than providing static instances.

	\param buffer - The buffer into which the ObjLightDesc pointers should be stored. */
	virtual void CreateLightDesc(ObjLightDesc** buffer) = 0;

	//! \brief Called by the system to start the radiosity processing.
	/*! This should start the process from the beginning, or where it stopped
	previously, if applicable. The method should launch a separate thread and
	return immediately; the system will call WaitForCompletion() to wait for
	the thread to complete.\n\n
	This is specific to solvers which use a pre-calculated solution. Other
	solvers need only a stub implementation.
	\param t - Specifies the scene time when the method is called.
	\param rgc - This can be used to retrieve information about the global rendering environment.
	\param interactiveRender - Specifies whether the lighting solution is being calculated for interactive
	rendering. Note that the default renderer does not support interactive
	rendering with Advanced Lighting, but other plug-in renderers might
	potentially do so, in which case the RadiosityEffect should attempt to
	generate a fast, lower quality solution for interactive display.	  */
	virtual void RunProcess(
		TimeValue				t,
		RenderGlobalContext*	rgc,
		bool					interactiveRender
    ) = 0;

	//! \brief Stop the lighting calculation. 
	/*! If possible, the RadiosityEffect should attempt to reach an intermediate solution,
	so that	calculation can be continued later. However, this may take awhile, and If
	allowAbort is true, the RadiosityEffect is expected to prompt the user with
	a dialog, asking if they wish to abort.

	\param allowAbort - If true, the RadisoityEffect is expected to prompt the user with a dialog
	so they may abort the process of stopping the calculation and saving an
	intermediate solution */
	virtual void StopProcess(bool allowAbort = true) = 0;

	// Abort the radiosity process. This method aborts the global
	// illumination process quickly. If AbortProcess is called, RunProcess
	// may start at the beginning the next time it is called.
	//! \brief Abort the lighting calculation. 
	/*! The RadiosityEffect should to
	abort immediately, without saving an intermediate solution. */
	virtual void AbortProcess() = 0;

	// Wait for radiosity process to complete. The RendContext is
	// used to provide progress reporting to the user, and to detect when the
	// cancel button is pressed. If the process doesn't complete in timeout
	// milliseconds, it is stopped and WaitForCompletion returns.
	// Completion criteria is set by the user in the radiosity plugin
	// UI. If the user cancels the render, the radiosity plugin should attempt
	// to stop the process, but allow the user to abort, when stopping
	// takes an excessive amount of time.
	//! \brief Wait for radiosity process to complete. 
	/*! This is called by the system when waiting for the lighting
	calculation to finish. This method should not return to the caller until the
	calculation is complete, or when the timeout (in milliseconds) expires. The
	RendContext can be used to display the progress of the calculation to the user,
	and to check if the cancel button is pressed.
	\param rc - 	Use this to display a progress bar for the user, or to detect when the user
	hits the cancel button.
	\param timeout - 	An amount of time (measured in milliseconds) that the system is willing to
	wait. The method should measure its own running time and return to the caller
	when the timeout expires.
	*/
	virtual CompletionCode WaitForCompletion(
		RendContext*	rc = NULL,
		DWORD		timeout = INFINITE
	) = 0;


	//! \brief Called right before RunProcess. 
	/*! Indicates whether the RadiosityEffect plug-in wants the
	renderer to build camera-space vertices.

	\param t - Specifies the current scene time when the method is called.
	\param rgc - 	This can be used to retrieve information about the global rendering
	environment.
	\param interactiveRender - 	Specifies whether the lighting solution is being calculated for interactive
	rendering. Note that the default renderer does not support interactive
	rendering with Advanced Lighting, but other plug-in renderers might
	potentially do so, in which case the RadiosityEffect should attempt to
	generate a fast, lower quality solution for interactive display.
	\param saveMem - 	True if the user has selected "Conserve Memory" in the Render Dialog, under
	the "MAX Default Scanline A-Buffer" rollout, or if the render is occurring
	in the material editor.

	\return  True if the RadiosityEffect wants the renderer to calculate
	camera-space vertices, false otherwise. */
	virtual bool NeedsCamVerts(
		TimeValue				t,
		RenderGlobalContext*	rgc,
		bool					interactiveRender,
		bool					saveMem
	) { return false; }
};


/*=====================================================================
 * Radisity Core Interface class
 *===================================================================*/

// This class is used to get access to the radiosity plugin
// and UI.
#define RADIOSITY_INTERFACE	Interface_ID(0x6711e7a, 0x5b504baa)

/*! \sa  : Class RadiosityEffect\n\n
\par Description:
This class is only available in release 5 or later.\n\n
This class provides access to the Advanced Lighting dialog. It allows you to
open and close the dialog, and get or set the currently active Advanced
Lighting plug-in. This class is a function-published static interface; you can
use GetCOREInterface() to obtain an instance of the class, as follows:\n\n
 <b>RadiosityInterface r =
static_cast\<IRadiosityInterface*\>(GetCOREInterface(RADIOSITY_INTERFACE))</b>\n\n
This interface is also accessible via MAXScript as "<b>SceneRadiosity</b>".\n\n
All methods of this class are implemented by the system.\n\n
   */
class RadiosityInterface : public FPStaticInterface {
public:
	//! Displays the Advanced Lighting dialog, unless it is already displayed.
	virtual void OpenRadiosityPanel()=0;
	//! Hides the Advanced Lighting dialog, if it is currently displayed.
	virtual void CloseRadiosityPanel()=0;
	//! Minimizes the Advanced Lighting dialog if it is open.
	virtual void MinimizeRadiosityPanel()=0;

	//! \brief Get the radiosity in the scene
	/*!
	\return  Returns a pointer to the currently active Advanced Lighting
	plug-in (RadiosityEffect) if any.
	  */
	virtual RadiosityEffect* GetRadiosity() const = 0;
	
	//! \brief Set the radiosity in the scene
	/*! Sets the given Advanced Lighting plug-in (RadiosityEffect) as
	the active one in the UI. When switching Advanced Lighting types in the UI,
	a dialog sometimes appears, warning that the current lighting solution will
	be discarded; but this dialog is not displayed when using SetRadiosity().
	It is also valid to pass NULL as a parameter, in which case no active
	lighting plug-in will be active.

	\param op - 	The RadiosityEffect instance to be made active, or NULL if no lighting
	plug-in should be active. */
	virtual void SetRadiosity(RadiosityEffect* op) = 0;
};


/*=====================================================================
 * Class IRadiosityEffectExtension
 *
 * Provides extended functionality to class RadiosityEffect. To use
 * this functionality with a RadiosityEffect class, derive the class
 * from both RadiosityEffect and IRadiosityEffectExtension, and implement
 * RadiosityEffect::GetInterface() to return a pointer to this interface
 * on request.
 *===================================================================*/

#define IRADIOSITYEFFECT_EXTENSION_INTERFACE Interface_ID(0x703149db, 0x43ed63b8)

/*! \sa  : Class RadiosityEffect\n\n
\par Description:
This class is only available in release 5 or later.\n\n
This class provides additional functionality for class RadiosityEffect.
To use this functionality with a RadiosityEffect class, derive the class
from both RadiosityEffect and IRadiosityEffectExtension, and implement
RadiosityEffect::GetInterface() to return a pointer to this interface
on request.\n\n
Given an instance of RadiosityEffect, you may retrieve the extension interface as
follows:\n\n
 <b>IRadiosityEffectExtension* r =
static_cast\<IRadiosityEffectExtension*\>(radiosityInstance-\>GetInterface(IRADIOSITYEFFECT_EXTENSION_INTERFACE));</b>\n\n
If the result is NULL, the RadiosityEffect does not support this interface.\n\n
All methods of this class are virtual.\n\n   */
class IRadiosityEffectExtension : public BaseInterface {
public:
	//! \brief Returns whether the specified default light should be used by the scanline renderer. 
	/*! The scanline renderer normally creates default lights when
	there are no lights in the scene. A radiosity plug-in could override this if it
	uses objects other than lights as light sources (e.g. self-emitting	surfaces)
	\param defLight - A default light created by the scanline renderer when it begins rendering.
	\return  Returns whether or not the scanline renderer should use the light for
	the current rendering. */
    virtual bool UseDefaultLight(const DefaultLight& defLight, bool recalcSolution = false) const = 0;

    //! \brief Returns whether the the radiosity plugin is interested in any
	//! of the channels in the part id.
    /*! This is used to control reference messages sent to the
    RadiosityEffect plug-in. It allows the RadiosityEffect to tell the system
    which messages will not invalidate the lighting solution.\n\n
    If the RadiosityEffect decides that all messages of a given PartID are
    irrelevant to the lighting solution, it can return false when that PartID
    flag is present in the input parameter (and no other, more relevant, PartID
    flags are present). Otherwise it should return true, indicating it needs
    those messages.\n\n
    If the return value is false, the system will add the PartID flag
    PART_EXCLUDE_RADIOSITY to all appropriate reference messages; this flag
    generically indicates an event which should not invalidate a radiosity
    solution. The RadiosityEffect will still receive the message but can ignore
    it. Other modules which need to discriminate messages pertinent radiosity,
    can also check this flag.\n\n
    As an example of when this is important, an edit mesh modifier may send
    change messages which are meant to flush internal caches but not relevant
    to radiosity. As the message propagates, some PartID flags are added in
    order to force modifiers to re-evaluate downstream in the stack. This may
    confuse the radiosity engine into invalidating its lighting solution. But
    the edit mesh ensures the original message bears the PART_EXCLUDE_RADIOSITY
    flag, and as subsequent messages inherit the flag, the RadiosityEffect can
    correctly ignore the message.

    \param part -     One or more PartID flags defining the category of messages to be
    filtered.
    \return  Whether or not to filter the given messages, by setting their
    PART_EXCLUDE_RADIOSITY PartID flag. */
    virtual bool IsInterestedInChannels(PartID part) const { return true; }

    // -- from BaseInterface
    //! \brief This returns the ID of the interface.
    /*! this should not be overridden by an implementation class.
    \return  The IRadiosityEffectExtension interface ID,
    IRADIOSITYEFFECT_EXTENSION_INTERFACE. */
    virtual Interface_ID GetID() { return IRADIOSITYEFFECT_EXTENSION_INTERFACE; }
};

#pragma warning(pop)

//===========================================================================
//
// Class IRadiosityRenderParameters
/*! \par Description:
 This interface provides access to our radiosity plug-in rendering parameters.
 */
//===========================================================================
#define IRADIOSITY_RENDER_PARAMETERS_INTERFACE Interface_ID(0x39981beb, 0x38df21ad)

class IRadiosityRenderParameters : public BaseInterface {

public:

    virtual bool GetReuseDirectIllumination(TimeValue in_time = 0, Interval *valid = NULL) const = 0;
    virtual void SetReuseDirectIllumination(bool in_reuse, TimeValue in_time = 0) = 0;

    virtual bool GetRegather(TimeValue in_time = 0, Interval *valid = NULL) const = 0;
    virtual void SetRegather(bool in_gather, TimeValue in_time = 0) = 0;

    virtual int GetRaysPerSample(TimeValue in_time = 0, Interval *valid = NULL) const = 0;
    virtual void SetRaysPerSample(int in_raysPerSample, TimeValue in_time = 0) = 0;

    virtual float GetFilterRadius(TimeValue in_time = 0, Interval *valid = NULL) const = 0;
    virtual void SetFilterRadius(float in_radius, TimeValue in_time = 0) = 0;

    virtual bool GetClampEnabled(TimeValue in_time = 0, Interval *valid = NULL) const = 0;
    virtual void SetClampEnabled(bool in_clamp, TimeValue in_time = 0) = 0;

    virtual float GetClampValue(TimeValue in_time = 0, Interval *valid = NULL) const = 0;
    virtual void SetClampValue(float in_clampValue, TimeValue in_time = 0) = 0;

    virtual bool GetAdaptiveEnabled(TimeValue in_time = 0, Interval *valid = NULL) const = 0;
    virtual void SetAdaptiveEnabled(bool in_adaptive, TimeValue in_time = 0) = 0;

    virtual int GetSampleSpacing(TimeValue in_time = 0, Interval *valid = NULL) const = 0;
    virtual void SetSampleSpacing(int in_sampleSpacing, TimeValue in_time = 0) = 0;

    virtual float GetSubdivionContrast(TimeValue in_time = 0, Interval *valid = NULL) const = 0;
    virtual void SetSubdivionContrast(float in_contrast, TimeValue in_time = 0) = 0;

    virtual int GetMinSampleSpacing(TimeValue in_time = 0, Interval *valid = NULL) const = 0;
    virtual void SetMinSampleSpacing(int in_minSpacing, TimeValue in_time = 0) = 0;

    virtual bool GetShowSamples(TimeValue in_time = 0, Interval *valid = NULL) const = 0;
    virtual void SetShowSamples(bool in_show, TimeValue in_time = 0) = 0;

    // -- from BaseInterface
    virtual Interface_ID GetID() { return IRADIOSITY_RENDER_PARAMETERS_INTERFACE; }
};

