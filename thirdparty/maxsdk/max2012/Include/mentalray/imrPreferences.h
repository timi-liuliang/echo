/*==============================================================================

  file:     imrPreferences.h

  author:   Daniel Levesque

  created:  26aug2002

  description:

    Interface for accessing the mental ray preferences.

    PERFORMANCE NOTICE: 
        Most the the methods of the preferences interface directly access a .ini
        file to get/set the preference values. It is therefore important to
        minimize calls to this interface in performance critical code.

  modified:	


(c) 2002 Autodesk
==============================================================================*/
#pragma once
#include "..\GetCOREInterface.h"
#include "..\ifnpub.h"

#define IMRPREFERENCES_INTERFACEID Interface_ID(0x594511cc, 0x15505bac)

//==============================================================================
// class imrPreferences
//
// This interface provides access to the mental ray preferences.
//==============================================================================
class imrPreferences : public FPStaticInterface {

public:

    // Mode used for clearing the VFB before rendering
    enum VFBClearMode {
        kVFBClear_None,             // Do not clear the VFB
        kVFBClear_OneLineOnTwo,     // Clear every other line (1 line on 2)
        kVFBClear_All,              // Clear the entire VFB
    };

    // Register/unregister preference change callbacks
    typedef void(*mrPreferencesCallback)(void* param);
    virtual void RegisterChangeCallback(mrPreferencesCallback callback, void* param) = 0;
    virtual void UnRegisterChangeCallback(mrPreferencesCallback callback, void* param) = 0;

    // Are the mental ray extensions ON/OFF?
    virtual bool GetMRExtensionsActive() const = 0;
    virtual void SetMRExtensionsActive(bool active) = 0;

    // Is mental ray the default production renderer?
    virtual bool GetMRDefaultProductionRenderer() const = 0;
    virtual void SetMRDefaultProductionRenderer(bool val) = 0;

    ////////////////////////////////////////////////////////////////////////////
    // Message dialog verbosity & options, log file
    //

    virtual bool GetMsgDlgOpenOnError() const = 0;
    virtual void SetMsgDlgOpenOnError(bool val) = 0;

    virtual bool GetMsgVerbosity_Info() const = 0;
    virtual void SetMsgVerbosity_Info(bool val) = 0;
    virtual bool GetMsgVerbosity_Progress() const = 0;
    virtual void SetMsgVerbosity_Progress(bool val) = 0;
    virtual bool GetMsgVerbosity_Debug() const = 0;
    virtual void SetMsgVerbosity_Debug(bool val) = 0;

    // Opens the message dialog
    virtual void OpenMessageDialog() const = 0;

    // log file name
    virtual const MCHAR* GetLogFileName() const = 0;
    virtual void SetLogFileName(const MCHAR* filename) = 0;
    // log file ON
    virtual bool GetLogFileON() const = 0;
    virtual void SetLogFileON(bool on) = 0;
    // log file Append
    virtual bool GetLogFileAppend() const = 0;
    virtual void SetLogFileAppend(bool on) = 0;

    //
    // Message dialog verbosity & options
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Rendering Options
    //

    // Show brackets around the bucket(s) currently being rendered
    virtual bool GetShowRenderBrackets() const = 0;
    virtual void SetShowRenderBrackets(bool on) = 0;

	// Show visual Final Gather progress, in the VFB
	virtual bool GetShowVisualFGProgress() const = 0;
	virtual void SetShowVisualFGProgress(bool on) = 0;

    //
    // Rendering Options
    ////////////////////////////////////////////////////////////////////////////

};

// Retrieve a pointer to the preferences interface
inline imrPreferences* GetMRPreferences() {

    return static_cast<imrPreferences*>(GetCOREInterface(IMRPREFERENCES_INTERFACEID));
}

