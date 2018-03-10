/*==============================================================================

  file:     imrShaderTranslation.h

  author:   Daniel Levesque

  created:  10feb2003

  description:

    Defition of the mental ray shader translation interface.

  modified:	

	[dl] 28 April 2004. Added advanced translation of parameters, incremented
	interface IDs to distinguish between the old (MAX 6) interface and the new (MAX 7)
	interface.

(c) 2004 Autodesk
==============================================================================*/
#pragma once

#include "..\maxheap.h"
#include "..\strbasic.h"
#include "..\baseinterface.h"
#include "..\ifnpub.h"
#include "..\plugapi.h" // for classdesc
#include "imrAdvancedTranslation.h"

// forward declarations
class imrShader;
class imrShaderCreation;
class Texmap;
class imrMaterialCustAttrib;
class ReferenceTarget;
class Point3;

#define IMRSHADERTRANSLATION_INTERFACE_ID Interface_ID(0x1c396abd, 0x5f964e0c)
#define IMRMTLPHENTRANSLATION_INTERFACE_ID Interface_ID(0x5f970a9c, 0x65e75de4)
#define IMRGEOMSHADERTRANSLATION_INTERFACE_ID Interface_ID(0x31882a12, 0x2102a73)
#define IMRSHADERTRANSLATION_CLASSINFO_INTERFACE_ID Interface_ID(0x17a76fd6, 0x5ab32bcd)

//==============================================================================
// class imrShaderTranslation
//
// Interface to be implemented by the MAX plugin which translates to a mental
// ray shader. Materials do not use this interface. Instead, they use the
// imrMaterialTranslation interface.
//
// For example:
//		A 3rd party implements a MAX texmap pluggin for a brick effect. By default,
//	the mental ray translator does not support this texture and will be unable to
//  render it. If the 3rd party has also implemented a mental ray shader for this
//  effect, then this interface may be used to tell the mental ray translator 
//  which shader is to be used.
//==============================================================================
class imrShaderTranslation : public BaseInterface {

public:

	// List of requirements that may be queried on this translation interface
	enum Requirement {
		// This shader requires tangent basis vectors.
		// The tangent basis vectors will be translated for the object on which
		// this shader is used. 
		// Return true if the vectors are needed, false otherwise. 
		// The 't', and 'valid' parameters are not used.
		// The 'arg' variable is a pointer to an 'int', to which to map channel
		// to be used should be assigned.
		kReq_TangentBasisVectors = 0,
	};

	struct AdditionalDependency : public MaxHeapOperators {

		AdditionalDependency(ReferenceTarget* rt, bool assign = true) : refTarg(rt), assignAsChild(assign) {}

		// The reference target that is to be translated as a dependency
		ReferenceTarget* refTarg;
		// This should be true in all but exceptional cases where the translator
		// could run into potential problems with reference looping.
		bool assignAsChild;
	};

	typedef Tab<AdditionalDependency> AdditionalDependencyTable;

	// Called by the mental ray translator to get the mental ray shader which
	// is to be used. The method may return a shader which was already created,
	// or create a new one via the imrShaderCreation interface.
	// This method will never be called more than once per render.
	virtual imrShader* GetMRShader(imrShaderCreation& shaderCreation) = 0;

	// This may be called when rendering is finished to discard the mental ray shaders.
	// If the derived class holds a reference to the shader created by the call to GetMRShader(),
	// then it may release that reference in this call. This is not an obligation,
	// but it can free memory and/or resolve issues such as the "Make Unique"
	// button always being enabled in the material editor.
	virtual void ReleaseMRShader() = 0;

	// Should the translator automatically copy the parameters based on the param
	// block 2 system?
	// NOTE: The metnal ray translator will correctly handle automatic translation
	// of the parameters IF the 3rd party plugin's parameters all use the ParamBlock2
	// system AND if the names & types of the parameters match the shader definition
	// (names are not case sensitive).
	// If this returns 'true', then the automatic translation will take place
	// after the call to TranslateParameters().
	virtual bool NeedsAutomaticParamBlock2Translation() = 0;

	// Translates the parameters of this plugin to the mental ray shader. Called
	// for the first frame, and for each subsequent frame that is not in the
	// validity interval. The method must store the validity interval of the
	// translated parameters in 'valid'.
	// **The parameters must be stored in the shader at time 0.
	virtual void TranslateParameters(imrTranslation& translationInterface, imrShader* shader, TimeValue t, Interval& valid) = 0;

	// If this class needs any ReferenceTarget to be translated, but which are not
	// sub-references or within a parameter block of the plugin class, then these
	// ReferenceTarget's may be given here. The mental ray translator will automatically
	// create a dependency between the plugin and the given ReferenceTarget's.
	virtual void GetAdditionalReferenceDependencies(AdditionalDependencyTable& refTargets) = 0;

	// This method is meant for advanced users for whom TranslateParameters() either is not
	// sufficient or causes performance issues.
	// Whereas TranslateParameters() translates parameters using the ParamBlock2 system,
	// this method is used to send parameters directly to mental ray, bypassing the ParamBlock2 system.
	// If TranslateParameters() is enough for you, or if you do not understand why you would need
	// this advanced translation method, then do not implement this method.
	// See the documentation of class imrAdvancedTranslation for more details.
	virtual void TranslateParameters_Advanced(imrAdvancedTranslation& advancedTranslation, TimeValue t, Interval& valid);

	// This method is meant to query specific requirements from the translation interface.
	// Look at the documentation for each requirement flag for more information.
	// Note that 'arg' may or may not be used, depending on the requirement.
	virtual bool HasRequirement(Requirement requirement, TimeValue t, Interval& valid, void* arg = NULL);

	// -- from BaseInterface
	virtual Interface_ID GetID();
};

inline imrShaderTranslation* GetIMRShaderTranslation(InterfaceServer* iserver) {

	return static_cast<imrShaderTranslation*>(iserver->GetInterface(IMRSHADERTRANSLATION_INTERFACE_ID));
}

//==============================================================================
// class imrMaterialPhenomenonTranslation
//
// A material plugin that wishes to have a custom translation to a material
// phenomenon must derive from this interface. The interface is identical to 
// imrShaderTranslation, except that:
// - the interface ID is different
// - only material phenomenon may be created via the imrShaderCreation interface.
//
// SPECIAL CASE: Using the mental ray material directly.
// To translate a material to a mental ray material directly, without actually
// using a phenomenon of your own, create an instance of the phenomenon called
// "max_default_mtl_phen". Even though this is a phenomenon, it is not translated
// as one, but instead it is translated as a material. NOTE, however, that this
// particular phenomenon does NOT support the automatic ParamBlock2 translation,
// so NeedsAutomaticParamBlock2Translation() should return false.
//==============================================================================
class imrMaterialPhenomenonTranslation : public imrShaderTranslation {

public:
	
	// Returns whether this material supports the 'mental ray connection' custom
	// attributes rollup. This rollup allows users to override any of the 10 shaders
	// on the material. Note that only materials that translate to the default
	// material phenomenon ("max_default_mtl_phen") may return true here, since
	// the translator needs that phenomenon to translate the shader overrides.
	// If the material does not return the default material phenomenon but still
	// wishes to support the overrides, then it will have to do so itself.
	virtual bool SupportsMtlOverrides() = 0;

	// Initializes the mtl overrides custom attribute, if necessary. Only called
	// if SupportsMtlOverrides() returned true.
	// All that this function should do is enable/disable the shader locks of the custom
	// attribute.
	// A material may simply do nothing here, or it may disable some of the shader locks
	// that don't apply.
	virtual void InitializeMtlOverrides(imrMaterialCustAttrib* mtlCustAttrib) = 0;

	// -- from BaseInterface
	virtual Interface_ID GetID();
};

inline imrMaterialPhenomenonTranslation* GetIMRMaterialPhenomenonTranslation(InterfaceServer* iserver) {

	return static_cast<imrMaterialPhenomenonTranslation*>(iserver->GetInterface(IMRMTLPHENTRANSLATION_INTERFACE_ID));
}

//==============================================================================
// class imrGeomShaderTranslation
//
// A geometry object plugin that wishes to have a custom translation to a mental
// ray shader must derive from this interface. The interface is used to retrieve
// an instance of the geometry shader to be used.
// Derive your GeomObject or ShapeObject from this class to have it translated
// to a geometry shader.
//==============================================================================
class imrGeomShaderTranslation : public BaseInterface {

public:

	// Returns the geometry shader to be used with this object. The ReferenceTarget
	// returned by this method should be a Texmap plugin that translates to a mental
	// ray geometry shader. You may need to implement your own Texmap plugin that
	// uses the imrShaderTranslation interface to translate itself to a mental ray
	// shader.
	virtual ReferenceTarget* GetGeomShader() = 0;

	// Returns a scale for the geometry shader, to be applied on the instance transform.
	// This is useful in the case that a geometry shader produces objects of a fixed
	// size (e.g. the mib_geo_* shaders), but you still want to be able to expose
	// a size parameter in the UI.
	// If you don't care about scaling, just return a scale of (1,1,1).
	virtual void GetScale(Point3& scale) = 0;
	
	// -- from BaseInterface
	virtual Interface_ID GetID();
};

inline imrGeomShaderTranslation* GetIMRGeomShaderTranslation(InterfaceServer* iserver) {

	return static_cast<imrGeomShaderTranslation*>(iserver->GetInterface(IMRGEOMSHADERTRANSLATION_INTERFACE_ID));
}

//==============================================================================
// class imrShaderCreation
//
// Interface used to create new shader instances.
//
// A reference to this interface is passed to the "GetMRShader()" method of the
// imrShaderTranslation interface. If one tries to create a shader which does
// nto exist, CreateShader() outputs an error in the mental ray message window
// and returns NULL.
//
// IMPORTANT: If the implementation of this interface holds a pointer to
// a shader, to avoid re-creating it every time, then it MUST make a reference
// to that shader. Otherwise, the shader will be deleted when the render ends.
//==============================================================================
class imrShaderCreation: public MaxHeapOperators {

public:

	// Creates an instance of a shader with the given name. The name must be as it
	// appears in the .mi declaration.
	// This method may return NULL if the shader is not found, and the caller
	// should check for NULL values.
	virtual imrShader* CreateShader(
		const MCHAR* declarationName,
		const MCHAR* mtlName		// The material's name is used when reporting errors. 
									// (Tip: Pass the result of MtlBase::GetFullName())
									// Passing NULL suppresses error messages.
	) = 0;
};

//==============================================================================
// class imrShaderTranslation_ClassInfo
//
// IMPLEMENTATION INSTRUCTIONS:
//
//   To be subclassed by a ClassDesc or ClassDesc2 of a Texmap plugin.
//   The subclass HAS TO call Init(*this) from its constructor.
//
// DESCRIPTION:
//
// This class is used to provide additional 'ApplyType' information for texmaps
// that translate to a custom shaders.
//
// All mental ray shaders have an 'apply' type. This type classifies shaders into
// different categories (e.g. texture shader, light shader, environment shader, etc.).
// The apply type restricts where and how a shader may be used. By default, if
// this interface is not implemented, a Texmap plugin will have the 
// imrShaderClassDesc::kApplyFlag_Default apply type. If this is incorrect, then
// this interface should be implemented - otherwise it may be impossible to use
// the shader and crashes could occur if the shader is not used correctly.
//==============================================================================
class imrShaderTranslation_ClassInfo : public FPStaticInterface {

public:

	// Initialization method. MUST be called from the constructor of the subclass. i.e. "Init(*this);".
	void Init(
		ClassDesc& classDesc
	);

	// Get the types to which this shader may be applied (combination of 
	// imrShaderClassDesc::ApplyFlags values).
	virtual unsigned int GetApplyTypes() = 0;
};

//==============================================================================
// class imrShaderTranslation_ClassInfo inlined methods
//==============================================================================

// Given the class descriptor of a Mtl/Texmap plugin, this returns its compatibility interface (if it exists).
inline imrShaderTranslation_ClassInfo* Get_imrShaderTranslation_ClassInfo(ClassDesc& mtlBaseClassDesc) {

	return static_cast<imrShaderTranslation_ClassInfo*>(mtlBaseClassDesc.GetInterface(IMRSHADERTRANSLATION_CLASSINFO_INTERFACE_ID));
}

//==============================================================================
// class imrShaderTranslation inlined methods
//==============================================================================

inline Interface_ID imrShaderTranslation::GetID() {

	return IMRSHADERTRANSLATION_INTERFACE_ID;
}
#pragma warning(push)
#pragma warning(disable:4100)
inline void imrShaderTranslation::TranslateParameters_Advanced(imrAdvancedTranslation& advancedTranslation, TimeValue t, Interval& valid) {

	// Default implementation does nothing
}

inline bool imrShaderTranslation::HasRequirement(Requirement, TimeValue t, Interval& valid, void* arg) {

	// Default implementation has no requirements
	return 0;
}
#pragma warning(pop)
//==============================================================================
// class imrMaterialPhenomenonTranslation inlined methods
//==============================================================================

inline Interface_ID imrMaterialPhenomenonTranslation::GetID() {

	return IMRMTLPHENTRANSLATION_INTERFACE_ID;
}

//==============================================================================
// class imrGeomShaderTranslation inlined methods
//==============================================================================

inline Interface_ID imrGeomShaderTranslation::GetID() {

	return IMRGEOMSHADERTRANSLATION_INTERFACE_ID;
}

//==============================================================================
// class imrShaderTranslation_ClassInfo inlined methods
//==============================================================================

inline void imrShaderTranslation_ClassInfo::Init(ClassDesc& classDesc) {

	LoadDescriptor(IMRSHADERTRANSLATION_CLASSINFO_INTERFACE_ID, _M("imrShaderTranslation_ClassInfo"), 0, &classDesc, 0, end);
}

