/*==============================================================================

  file:     mrShaderButtonHandler.h

  author:   Daniel Levesque

  created:  23 April 2003

  description:

    Implementation of a DADMgr which handles shader buttons outside of the
	material editor.

  modified:	


(c) 2003 Autodesk
==============================================================================*/
#pragma once

#include <WTypes.h>
#include "..\..\custcont.h"
#include "mrShaderFilter.h"
// forward declarations
class imrShader;

//==============================================================================
// class mrShaderButtonHandler
//
// Base class for shader DAD managers.
//
// Users need only derive from this class and implement the pure virtual
// methods. The DialogProc should call the OnInitDialog(), OnCommand(), and OnClose()
// methods.
//==============================================================================
class mrShaderButtonHandler : public DADMgr  {

public:

	// Initialize with accepted apply types
	mrShaderButtonHandler(
		unsigned int applyTypes,		// Accepted apply types, a combination of imrShaderClassDesc::ApplyFlags
		int ctrlID						// ID of the button control. Must be an ICustButton control.
	);
	virtual ~mrShaderButtonHandler();

	void OnInitDialog(HWND hDialog);		// To be called on WM_INITDIALOG
	void OnCommand();						// To be called on WM_COMMAND for this button.
	void OnClose();							// To be called on WM_CLOSE

	// Updates the text on the button
	void Update();

	void Enable(bool enable);

	// -- from DADMgr
	virtual SClass_ID GetDragType(HWND hwnd, POINT p);
	virtual ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type);
	virtual BOOL OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew = FALSE);
	virtual int SlotOwner();
	virtual void Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type, DADMgr* srcMgr = NULL, BOOL bSrcClone = FALSE);
	virtual BOOL AutoTooltip();

protected:

	virtual void SetShader(Texmap* shader) = 0;
	virtual Texmap* GetShader() = 0;

	// Loads a localized string for "None" (Shows up on the button when no shader is assigned)
	virtual const MCHAR* GetNoneString() = 0;
	
private:

	// The shader filter used to validate the shaders
	mrShaderFilter m_shaderFilter;	

	int m_ctrlID;

	HWND m_dialogHWnd;
};






