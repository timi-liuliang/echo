/*==============================================================================

  file:     imrShaderClassDesc.h

  author:   Daniel Levesque

  created:  10feb2003

  description:

      Class descriptor for mental ray shaders.

  modified:	


(c) 2003 Autodesk
==============================================================================*/
#pragma once

#include "..\iparamb2.h"
// forward declarations
class imrShader;
class mrShaderDesc;

//==============================================================================
// class imrShaderClassDesc
//
// Class descriptor for a mental ray shader. Dynamically created for each shader
// encountered in the .mi files during startup. 
//
// This class derives from InterfaceServer to allow for future extensions.
//==============================================================================
class imrShaderClassDesc : public ClassDesc2, public InterfaceServer {

private:

	// Apply types for this shader
	enum {
		// Types defined by the mental ray API
		kApply_Lens,
		kApply_Material,
		kApply_Light,
		kApply_Shadow,
		kApply_Environment,
		kApply_Volume,
		kApply_Texture,
		kApply_Photon,
		kApply_Geometry,
		kApply_Displace,
		kApply_PhotonEmitter,
		kApply_Output,
		kApply_LightMap,
		kApply_PhotonVol,
		kApply_Other,		

		// Additional types defined by the GUI attributes
		kApply_Bump,
		kApply_ContourShader,
		kApply_ContoutStore,
		kApply_ContourContrast,
		kApply_ContourOutput,
		kApply_Illum,
		kApply_PassMerge,

		kApply_MtlPhen,		// Shader is a material phenomenon

		kApply_Count
	};

public:
	
	// Apply type bits when compined in a single value
	enum ApplyFlags {
		// Types defined by the mental ray API
		kApplyFlag_Lens = (1 << kApply_Lens),
		kApplyFlag_Material = (1 << kApply_Material),
		kApplyFlag_Light = (1 << kApply_Light),
		kApplyFlag_Shadow = (1 << kApply_Shadow),
		kApplyFlag_Environment = (1 << kApply_Environment),
		kApplyFlag_Volume = (1 << kApply_Volume),
		kApplyFlag_Texture = (1 << kApply_Texture),
		kApplyFlag_Photon = (1 << kApply_Photon),
		kApplyFlag_Geometry = (1 << kApply_Geometry),
		kApplyFlag_Displace = (1 << kApply_Displace),
		kApplyFlag_PhotonEmitter = (1 << kApply_PhotonEmitter),
		kApplyFlag_Output = (1 << kApply_Output),
		kApplyFlag_LightMap = (1 << kApply_LightMap),
		kApplyFlag_PhotonVol = (1 << kApply_PhotonVol),
		kApplyFlag_Other = (1 << kApply_Other),		

		// Additional types defined by the GUI attributes
		//kApply_None,		// Probably better remove this...
		kApplyFlag_Bump = (1 << kApply_Bump),
		kApplyFlag_ContourShader = (1 << kApply_ContourShader),
		kApplyFlag_ContourStore = (1 << kApply_ContoutStore),
		kApplyFlag_ContourContrast = (1 << kApply_ContourContrast),
		kApplyFlag_ContourOutput = (1 << kApply_ContourOutput),
		kApplyFlag_Illum = (1 << kApply_Illum),
		kApplyFlag_PassMerge = (1 << kApply_PassMerge),
		
		kApplyFlag_MtlPhen = (1 << kApply_MtlPhen),

		// All except mtl phenomenon
		kApplyFlag_All = (~0 & ~kApplyFlag_MtlPhen),
		
		// Default type accepts texure, material, illum, bump
		kApplyFlag_Default = (kApplyFlag_Material | kApplyFlag_Texture | kApplyFlag_Illum | kApplyFlag_Bump),
	};

	// Create a shader for the given apply type. The shader's default name will
	// match the apply type. Returns NULL if a bad apply type is given
	// virtual imrShader* CreateShader(ApplyType applyType) = 0;

	// Same as ClassDesc::Create(), but returns a pointer to a shader instead of a void*
	virtual imrShader* CreateShader(bool loading = false) = 0;

	// Get the types to which this shader may be applied (combination of 
	// ApplyFlags values).
	virtual unsigned int GetApplyTypes() = 0;

	// Get the shader descriptor for this class
	virtual mrShaderDesc& GetShaderDesc() = 0;

	// Access to the Parameters PBDesc & sub-PBDescs
    virtual ParamBlockDesc2* GetParamPBDesc() = 0;
    virtual Tab<ParamBlockDesc2*>& GetParamSubPBDescs() = 0;

	// Access to the Results PBDesc & sub-PBDescs
    virtual ParamBlockDesc2* GetResultPBDesc() = 0;
    virtual Tab<ParamBlockDesc2*>& GetResultPBDescs() = 0;

	// Access to the Connestions PBDesc & sub-PBDescs
    virtual ParamBlockDesc2* GetConnectionPBDesc() = 0;
    virtual Tab<ParamBlockDesc2*>& GetConnectionSubPBDescs() = 0;

	// Creates the auto param dialog for this shader
	virtual IAutoMParamDlg* CreateShaderParamDlgs(HWND hwMtlEdit, IMtlParams *imp, ReferenceTarget* obj) = 0;
};

// Pass this command ID to ClassDesc::Execute(). The function will return 
// (GETIMRSHADERCLASSDESC_CMD+1) if the ClassDesc* can safely be cast to a imrShaderClassDesc*
#define GETIMRSHADERCLASSDESC_CMD 0x984a860f

// Converts, if possible, a ClassDesc* to a imrShaderClassDesc*
inline imrShaderClassDesc* Get_mrShaderClassDesc(ClassDesc* cd) {

    if((cd != NULL) && (cd->Execute(GETIMRSHADERCLASSDESC_CMD) == (GETIMRSHADERCLASSDESC_CMD + 1))) {
        return static_cast<imrShaderClassDesc*>(cd);
    }
    else {
        return NULL;
    }
}

