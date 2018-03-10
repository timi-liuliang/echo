//*****************************************************************************/
// Copyright (c) 1998-2005 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//*****************************************************************************/

/**********************************************************************
 *<
	FILE: BrushPresets.h

	DESCRIPTION:	Brush Preset functionality for the PainterInterface

	CREATED BY:		Michaelson Britt

	HISTORY:		Created December 2004
 *>
 **********************************************************************/
#pragma once
#include "maxheap.h"
#include "maxtypes.h"
#include "ifnpub.h"
#include "GetCOREInterface.h"

// forward declarations
class IBrushPreset;
class IBrushPresetParams;
class IBrushPresetContext;
class IBrushPresetMgr;

//-----------------------------------------------------------------------------
// class IBrushPreset

//! \brief The interface to a brush preset object, holding parameters for the PainterInterface and other brush tools.
/*! The interface to Brush Preset objects.  A brush preset corresponds to a button on the
    brush preset toolbar, or a row listed in the brush preset manager dialog.
    A preset holds one set of parameters for each brush preset context registered with the manager.
    Applying or fetching a preset will apply or fetch parameters for each currently active context.
    The context and parameter objects are implemented by plug-ins, but the presets holding the
    parameter objects are implemented by the system. */
class IBrushPreset: public MaxHeapOperators {
	public:
		//! \brief Destructor
		virtual ~IBrushPreset() {;}

		//! \brief Returns an ID which uniquely identifies this preset.
		/*! Presets have an ID number unique from other presets at any given time; however
		   if a preset is deleted, its ID number may be reassigned to other presets created later. */
		virtual int			PresetID() = 0;

		//! \brief Applies parameters from the preset into the UI, as per activating its toolbar button.
		/*! For each active brush preset context, the corresponding set of parameters
		    held by the preset is applied via ApplyParams(). */
		virtual void		Apply() = 0;	// Calls ApplyParams() on every param set
		//! \brief Fetches parameters from the UI into the preset, as per deactivating its toolbar button.
		/*! For each active brush preset context, the corresponding set of parameters
		    held by the preset is fetched via FetchParams(). */
		virtual void		Fetch() = 0;	// Calls FetchParams() on every param set

		//! \brief Returns the parameter set held by the brush preset for a given context.
		/*! \param[in] contextID The ID of the context */
		virtual IBrushPresetParams* GetParams( Class_ID contextID ) = 0;
		//! \brief Returns the number of contexts for which this preset is holding parameters.
		virtual int			GetNumContexts() = 0;
		//! \brief Translates the index number of a parameter set into the corresponding context ID
		/*! \param[in] contextIndex The index of the parameter set
		    \return The context ID for the parameter set, or Class_ID(0,0) if the index is out of range */
		virtual Class_ID	GetContextID( int contextIndex ) = 0;
		//! \brief Translates the context ID for a parameter set into the corresponding index number
		/*! \param[in] contextID The ID of the context
		    \return The index of the parameter set, or -1 if the preset has no parameters for the given context */
		virtual int			GetContextIndex( Class_ID contextID ) = 0;
};


//-----------------------------------------------------------------------------
// class IBrushPresetParams

//! \brief Interface to a set of brush preset parameter values, associated with a particular Brush Preset context.
/*! Implemented by the plug-in. */
class IBrushPresetParams: public MaxHeapOperators  {
	public:
		//! \brief Destructor
		virtual ~IBrushPresetParams() {;}

		//! \brief Returns the ID of the brush preset context associated with this parameter set.
		virtual Class_ID	ContextID()=0; // The ID of the param set is equal to the ID of its context

		// The presets param object is expected to have access to its associated context.
		// These methods apply the params into the context, or fetch the param values from the context

		//! \brief Applies the values from the parameter set into the UI of the corresponding context.
		/*! The parameter object is assumed to be part of the same plug-in as its context object, and to have
		    knowledge of its state.  The parameter object should apply its values into the UI as appropriate. */
		virtual void		ApplyParams() = 0;
		//! \brief Fetches the values from the UI of the corresponding context into the parameter set.
		/*! The parameter object is assumed to be part of the same plug-in as its context object, and to have
		    knowledge of its state.  The parameter object should fetch its values from the UI as appropriate. */
		virtual void		FetchParams() = 0;

		// Use methods in the BrushPresetContext for metadata about the parameters.
		// Parameter values are stored here.

		//! \brief Returns the number of individual parameters in the parameter set.
		virtual int			GetNumParams() = 0;
		//! \brief Translates the index number of a parameter value into the corresponding parameter ID
		/*! \param[in] paramIndex The index of the parameter value
		    \return The parameter ID for the value, or zero if the index is out of range */
		virtual int			GetParamID( int paramIndex ) = 0;
		//! \brief Translates the ID of a parameter value into the corresponding parameter index number
		/*! \param[in] paramID The ID of the parameter value
		    \return The index of the parameter value, or -1 if there is no parameter with the given ID */
		virtual int			GetParamIndex( int paramID ) = 0;
		//! \brief Returns a parameter value by ID.
		/*! \param[in] paramID The ID of the parameter value
		   \return The value of the parameter */
		virtual FPValue		GetParamValue( int paramID ) = 0;
		//! \brief Sets a parameter value by ID.
		/*! \param[in] paramID The ID of the parameter value
		    \param[in] val The value to set */
		virtual void		SetParamValue( int paramID, FPValue val ) = 0;
		//! \brief Returns the value of a parameter as it should be displayed in the UI.
		/*! This is used to display values with a user-friendly format in the brush preset manager dialog.
		    For example, a minimum and maximum value of 0 and 100 might be displayed together as one entry
		    with the string value "0-100". In this case, the parameter set should return TYPE_VOID for the
		    minimum and maximum display values, but provide a third, display-only parameter, with the string value.
		    \param[in] paramID The ID of the parameter value
		    \return The value of the parameter to display in the UI */
		virtual FPValue		GetDisplayParamValue( int paramID )		{return GetParamValue(paramID);}

		//! \brief Informs the parameter set of the brush preset holding it.
		/*! This is called by the system when parameter sets are created, to make them aware of the
		    preset they are held by. No specific action is required by the parameter set.
		    \param[in] parent The brush preset parent which holds this parameter set. */
		virtual void		SetParent( IBrushPreset* parent ) = 0;
};


//-----------------------------------------------------------------------------
// class IBrushPresetContext

//! \brief The ID of the standard brush preset context, implemented by the system
/*! The standard context is responsible for the common values in the Painter Interface dialog,
    such as the brush radius and falloff.  This context is considered to be an active context
    anytime any other brush preset context is active.
    The standard parameters are listed in the first columns of the brush preset manager dialog,
    and the context-specific parameters are listed in the remaining columns */
#define STDBRUSHPRESETCONTEXT_ID	Class_ID(0,0x5fb1707c)

//! \brief Interface to a brush preset context.
/*! A context represents a specific brush-based feature, defining a set of parameters used by the features and a UI.
    Context objects are responsible for creating and deleting the parameter set objects held by a brush preset,
    and for providing general information about the parameters.
    Contexts are required to communicate with the manager in the following ways:
    1) Contexts should register themselves with the manager as soon as they loaded into memory,
    using IBrushPresetMgr::RegisterContext().  If the context plug-in is loaded before the manager plug-in,
    then the context can use a notification handler to register with the manager after system startup.
    2) When a context is active, meaning its UI is displayed and ready to begin painting, the context
    should call IBrushPresetMgr::BeginContext(), and EndContext() when the context is deactivated.
    3) When the user changes any parameter in the context's UI, the context should call IBrushPresetMgr::OnContextUpdated().
	\n
    Implemented by the plug-in. */
class IBrushPresetContext: public MaxHeapOperators {
	public:
		//! \brief Destructor
		virtual ~IBrushPresetContext() {;}

		//! \brief Returns the unique ID for the context. These ID numbers should be random, as per standard class IDs
		virtual Class_ID	ContextID() = 0;
		//! \brief Returns a name for the context.  This is for UI display and may be localized
		virtual MCHAR*		GetContextName() = 0;

		//! \brief Creates and returns a parameter set object of appropriate type for this context.
		virtual IBrushPresetParams* CreateParams() = 0;
		//! \brief Deletes a parameter set object created by this context.
		/*! This is called by the system when the user deletes a preset or at shutdown;
		    the object passed is always a parameter set created previously by the context. */
		virtual void		DeleteParams( IBrushPresetParams* params ) = 0;

		//! \brief Returns the number of individual parameters in a parameter set of this context.
		virtual int			GetNumParams() = 0;
		//! \brief Translates the index number of a parameter value into the corresponding parameter ID
		/*! \param[in] paramIndex The index of the parameter value
		    \return The parameter ID for the value, or zero if the index is out of range */
		virtual int			GetParamID( int paramIndex ) = 0;
		//! \brief Translates the ID of a parameter value into the corresponding parameter index number
		/*! \param[in] paramID The ID of the parameter value
		    \return The index of the parameter value, or -1 if there is no parameter with the given ID */
		virtual int			GetParamIndex( int paramID ) = 0;
		//! \brief Returns the name of a parameter value
		/*! \param[in] paramID The ID of the parameter value */
		virtual MCHAR*		GetParamName( int paramID ) = 0;
		//! \brief Returns the type of a parameter value
		/*! \param[in] paramID The ID of the parameter value */
		virtual ParamType2	GetParamType( int paramID ) = 0;

		//! \brief Returns the number of parameters that should be displayed in the brush preset manager dialog.
		virtual int			GetNumDisplayParams()					{return GetNumParams();}
		//! \brief Translates the index number of a display parameter into the corresponding parameter ID
		/*! \param[in] displayIndex The index of the display parameter
		    \return The parameter ID for the value, or zero if the index is out of range */
		virtual int			GetDisplayParamID( int displayIndex )	{return GetParamID(displayIndex);}
		//! \brief Translates the ID of a parameter value into the corresponding display parameter index number
		/*! \param[in] paramID The ID of the parameter value
		    \return The index of the parameter value, or -1 if there is no display parameter with the given ID */
		virtual int			GetDisplayParamIndex( int paramID )		{return GetParamIndex(paramID);}
		//! \brief Returns the name of a display parameter
		/*! \param[in] paramID The ID of the display parameter */
		virtual MCHAR*		GetDisplayParamName( int paramID )		{return GetParamName(paramID);}

		//! \brief Returns whether the given parameter should be displayed in the brush preset manager dialog
		/*! \param[in] paramID The ID of the parameter
		    \return TRUE if the parameter should be displayed, FALSE otherwise */
		virtual BOOL		IsDisplayParam( int paramID )			{ UNUSED_PARAM(paramID); return TRUE;}
		//! \brief Returns whether the given parameter is transient
		/*! Transient parameters are not stored to disk.  This can be useful, for example, with
		    display-only parameters, whose value is a user-friendly string built from other non-display parameters.
		    \param[in] paramID The ID of the parameter
		    \return TRUE if the parameter is transient, FALSE otherwise */
		virtual BOOL		IsTransientParam( int paramID )			{ UNUSED_PARAM(paramID); return FALSE; }

		// Support for deferred plug-in loading

		//! \brief Returns the class ID of a plug-in implemented in the same module as this context.
		/*! The class ID is used to support deferred plug-in loading.  When the brush preset manager needs to
		    ensure that a context implementation is loaded into memory, it will load the module using this ID.
		    The manager calls this method when a context is first registered, and then stores the value
		    in the brush preset file for later use. */
		virtual Class_ID	PluginClassID()=0; // Class ID of the plug-in containing the context
		//! \brief Returns the superclass ID of a plug-in implemented in the same module as this context.
		/*! The superclass ID is used to support deferred plug-in loading.  When the brush preset manager needs to
		    ensure that a context implementation is loaded into memory, it will load the module using this ID.
		    The manager calls this method when a context is first registered, and then stores the value
		    in the brush preset file for later use. */
		virtual SClass_ID	PluginSuperClassID()=0; // Superclass ID of the plug-in containing the context
};


//-----------------------------------------------------------------------------
// class BrushPresetMgr

//! \brief Interface to the brush preset manager.
/*! The manager is a singleton object implemented by the system.
    It maintains the list of contexts and presets.
    It communicates with the contexts to track which is active, and when a context is updated.
    It tracks which preset is active in order to store parameters into the preset when contexts are updated.
    It has functionality to read and write brush preset files.
    Finally, the brush preset manager provides a maxscript interface to activate and deactivate presets. */
class IBrushPresetMgr : public FPStaticInterface {
	public:
		DECLARE_DESCRIPTOR( IBrushPresetMgr );

		//! \brief Registers a context with the manager.
		/*! Contexts should call this method at system startup,
		    or immediately when loaded into memory to support deferred loading.
		    \param[in] context The context to register */
		virtual void		RegisterContext( IBrushPresetContext* context ) = 0;
		//! \brief Unregisters a context with the manager.
		/*! Contexts may call this method if they are unloaded from memory, but it is not required during normal shutdown.
		    \param[in] context The context to unregister */
		virtual void		UnRegisterContext( IBrushPresetContext* context ) = 0;

		//! \brief Returns a context object from its ID.
		/*! This returns the context immediately if it is currently registered, otherwise it
		    loads the context plug-in if it is not in memory but supports deferred loading.
		    \param[in] contextID The ID of the context */
		virtual IBrushPresetContext* GetContext( Class_ID contextID ) = 0;
		//! \brief Returns the number of contexts.
		/*! This includes all currently registered contexts, plus contexts in deferred plug-ins which were
		    registered previously but are not yet loaded. */
		virtual int			GetNumContexts() = 0;
		//! \brief Translates the index number of a context into the corresponding context ID
		/*! \param[in] contextIndex The index of the context
		    \return The context ID, or Class_ID(0,0) if the index is out of range */
		virtual Class_ID	GetContextID( int contextIndex ) = 0; // ID numbers are always > 0
		//! \brief Translates the ID of a context into the corresponding context index number
		/*! \param[in] contextID The ID of the context
		    \return The index of the context, or -1 if no context exists with the given ID */
		virtual int			GetContextIndex( Class_ID contextID ) = 0;
		//! \brief Notifies the manager that a context is activated
		/*! This should be called by a context when its UI is opened and ready for painting
		    \param[in] contextID The ID of the context */
		virtual int			BeginContext( Class_ID contextID ) = 0;
		//! \brief Notifies the manager that a context is deactived
		/*! This should be called by a context when its UI is closed
		    \param[in] contextID The ID of the context */
		virtual int			EndContext( Class_ID contextID ) = 0;
		//! \brief Returns TRUE if the given context is active, FALSE otherwise
		/*! \param[in] contextID The ID of the context */
		virtual BOOL		IsContextActive( Class_ID contextID ) = 0;

		//! \brief Returns a brush preset object from its ID, or NULL if no preset exists with the given ID.
		/*! \param[in] presetID The ID of the preset */
		virtual IBrushPreset* GetPreset( int presetID ) = 0;
		//! \brief Creates a new preset, and adds a corresponding button to the brush preset toolbar.
		virtual IBrushPreset* CreatePreset() = 0;
		//! \brief Deletes a preset, removing the corresponding button from the brush preset toolbar.
		virtual void		DeletePreset( IBrushPreset* preset ) = 0;
		//! \brief Returns the current number of brush presets.
		virtual int			GetNumPresets() = 0;
		//! \brief Translates the index number of a brush preset into the corresponding preset ID
		/*! \param[in] presetIndex The index of the preset
		    \return The preset ID, or zero if the index is out of range */
		virtual int			GetPresetID( int presetIndex ) = 0;
		//! \brief Translates the ID of a brush preset into the corresponding preset index number
		/*! \param[in] presetID The ID of the preset
		    \return The index of the preset, or -1 if no preset exists with the given ID */
		virtual int			GetPresetIndex( int presetID ) = 0;

		//! \brief Returns the ID of the active preset
		/*! This corresponds to the currently checked button on the brush preset toolbar, or zero if no button is checked. */
		virtual int			GetActivePreset() = 0;
		//! \brief Sets the active preset
		/*! The button on the brush preset toolbar corresponding to this preset will become checked.
		    \param[in] presetID The ID of the preset */
		virtual void		SetActivePreset( int presetID ) = 0; // Pass zero to deactivate

		//! \brief Notifies the manager that a parameter has changed within a brush preset context
		/*! Contexts should call this method whenever they are active, and any parameter changes which needs
		    to be stored into the active preset
		    \param[in] contextID The ID of the updated context */
		virtual void		OnContextUpdated( Class_ID contextID ) = 0;

		//! \brief Reads a brush preset file, including the preset values and the layout of the toolbar
		/*! \param[in] cfg The path of the file, or NULL to use the standard startup file
		    \return Non-zero if successful, otherwise zero on error */
		virtual int			ReadConfig(MCHAR *cfg=NULL) = 0;
		//! \brief Writes a brush preset file, including the preset values and the layout of the toolbar
		/*! \param[in] cfg The path of the file, or NULL to use the standard startup file
		    \return Non-zero if successful, otherwise zero on error */
		virtual int			WriteConfig(MCHAR *cfg=NULL) = 0;
};

//! \brief The interface ID of the brush preset manager function published interface
#define IBRUSHPRESETMGR_INTERFACE_ID Interface_ID(0x586b1d59, 0x454d0ae9)

//! \brief Helper macro returning the brush preset manager singleton object
inline IBrushPresetMgr* GetIBrushPresetMgr()
{ return (IBrushPresetMgr*)GetCOREInterface(IBRUSHPRESETMGR_INTERFACE_ID); }


