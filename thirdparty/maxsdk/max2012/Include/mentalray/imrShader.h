/*==============================================================================

  file:     imrShader.h

  author:   Daniel Levesque

  created:  10feb2003

  description:

    Interface definition for mental ray shaders.

  modified:	


(c) 2003 Autodesk
==============================================================================*/
#pragma once

#include "..\tab.h"
#include "..\maxtypes.h"
#include "..\BaseInterface.h"
#include "..\strclass.h"

// forward declarations
class imrShaderClassDesc;
class IParamBlock2;
class ReferenceTarget;

// Interface ID for imrShader interface
#define IMRSHADER_INTERFACE_ID Interface_ID(0x3b2f7b97, 0x5766e45)

//==============================================================================
// class imrShader
//
// Abstract class from which the mental ray shaders are derived.
//
// 3rd parties must not derive from this interface.
//
// This interface is implemented by both material plugins (class Mtl) and texture
// map plugins (class Texmap). To query this interface from a material or texture
// map plugin, use the GetIMRShader() and IsIMRShader() functions below.
//
//==============================================================================
class imrShader : public BaseInterface {

public:

	// Naming methods
	virtual void SetName(const MCHAR* name) = 0;
	virtual const MCHAR* GetName() = 0;
	virtual MSTR GetFullName() = 0;

    // Returns the class descriptor of this shader.
    virtual imrShaderClassDesc& GetClassDesc() = 0;

    // Access to the four main parameter blocks
    virtual IParamBlock2* GetResultsParamBlock() = 0;
    virtual IParamBlock2* GetParametersParamBlock() = 0;
    virtual IParamBlock2* GetConnectionsParamBlock() = 0;

    // Called when WM_HELP is received by the shader editor. Show shader help
    // and return true, or return false for default handling.
    virtual bool HandleHelp() = 0;

	// Returns the ReferenceTarget associated with the given shader. Class imrShader
	// does not derive from ReferenceTarget, but its implementation does.
	// This method is necessary if the client wishes to create a reference to the
	// shader.
	virtual ReferenceTarget& GetReferenceTarget() = 0;

	// Controls whether this shader was created for custom translation.
	// USED INTERNALLY.
	virtual bool IsCustomTranslationShader() = 0;
	virtual void SetCustomTranslationShader(bool val) = 0;

    // -- from BaseInterface
    virtual Interface_ID GetID();
};

inline Interface_ID imrShader::GetID() {

    return IMRSHADER_INTERFACE_ID;
}


//==============================================================================
// GetIMRShader()
//
// Queries and returns the imrShader interface on an object. Returns NULL if
// the given object is _not_ a mental ray shader.
//==============================================================================
inline imrShader* GetIMRShader(InterfaceServer* iserver) {

	if(iserver == NULL)
		return NULL;
	else
		return static_cast<imrShader*>(iserver->GetInterface(IMRSHADER_INTERFACE_ID));
}

//==============================================================================
// IsIMRShader()
//
// Returns whether an object implements the imrShader interface (i.e. returns
// whether an object _is_ a mental ray shader).
//==============================================================================
inline bool IsIMRShader(InterfaceServer* iserver) {

	return (GetIMRShader(iserver) != NULL);
}

//==============================================================================
// GetReferenceTarget()
//
// Converts a mental ray shader back to a reference target.
//==============================================================================
inline ReferenceTarget* GetReferenceTarget(imrShader* shader) {

	return ((shader != NULL) ? &shader->GetReferenceTarget() : NULL);
}


