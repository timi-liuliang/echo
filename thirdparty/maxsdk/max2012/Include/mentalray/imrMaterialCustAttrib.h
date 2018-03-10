/*==============================================================================

  file:     imrMaterialCustAttrib.h

  author:   Daniel Levesque

  created:  26sept2002

  description:
     
       Interface definition for the mental ray custom attributes. This custom
    attributes is used to override the default translation of a MAX materal
    by assigning custom shaders.

  modified:	


(c) 2002 Autodesk
==============================================================================*/
#pragma once

#include "..\CustAttrib.h"

// forward declarations
class Texmap;
#define MRMATERIALCUSTATTRIB_CLASS_ID Class_ID(0x218ab459, 0x25dc8980)

//==============================================================================
// class imrMaterialCustAttrib
//
// The mental ray material custom attributes. Used to assign custom shaders
// to MAX materals, overriding the default translation.
//
//==============================================================================
class imrMaterialCustAttrib : public CustAttrib {

public:

    enum ShaderSlot {
        kShaderSlot_Surface,
        kShaderSlot_Displacement,
        kShaderSlot_Shadow,
        kShaderSlot_Volume,
        kShaderSlot_Environment,
        kShaderSlot_Contour,
        kShaderSlot_Photon,
        kShaderSlot_PhotonVolume,
        kShaderSlot_LightMap,

		kShaderSlot_Count
    };

    // Gets/Sets the "Opaque" flag at the given time
    virtual bool GetOpaque(TimeValue t = 0) const = 0;
    virtual void SetOpaque(bool val, TimeValue t = 0) = 0;

    // Gets/Sets the different shaders. 
    virtual Texmap* GetShader(ShaderSlot slot) const = 0;
    virtual void SetShader(ShaderSlot slot, Texmap* shader) = 0;

	// Enables/Disables the locks for each slot
	virtual bool GetLockEnabled(ShaderSlot slot) const = 0;
	virtual void SetLockEnabled(ShaderSlot slot, bool enable) = 0;

	// Turns the locks for each slot ON/OFF.
	virtual bool GetShaderLocked(ShaderSlot slot) const = 0;
	virtual void SetShaderLocked(ShaderSlot slot, bool lock) = 0;
	
	// Returns whether the given shader slot is locked. Shader locks that are
	// disabled are never ON, regardless of the value returned by GetShaderLock()
	virtual bool IsShaderLocked(ShaderSlot slot) const = 0;
};

// Returns the mental ray custom attribute associate with the given material,
// if it one exists.
// Cut and paste in your source file to use it.
/*
imrMaterialCustAttrib* Get_mrMaterialCustAttrib(MtlBase* rt) {

    ICustAttribContainer* cc = rt->GetCustAttribContainer();
    if(cc != NULL) {
        int nbCustAttribs = cc->GetNumCustAttribs();
        for(int i = 0; i < nbCustAttribs; ++i) {

            CustAttrib* ca = cc->GetCustAttrib(i);
            if((ca != NULL) && (ca->ClassID() == MRMATERIALCUSTATTRIB_CLASS_ID)) {
                return static_cast<imrMaterialCustAttrib*>(ca);
            }
        }
    }

    return NULL;
}
*/

