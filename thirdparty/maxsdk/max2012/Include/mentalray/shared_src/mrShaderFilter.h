/*==============================================================================

  file:     mrShaderFilter.h

  author:   Daniel Levesque

  created:  21feb2003

  description:

    The filter for the material/map browser, which also doubles as a PBValidator.

  modified:	


(c) 2003 Autodesk
==============================================================================*/
#pragma once

#include "..\..\iparamb2.h"
#include "..\..\IMtlBrowserFilter.h"

#define MRSHADERFILTER_INTERFACEID Interface_ID(0x36ad69a6, 0x17db66d1)

//==============================================================================
// class mrShaderFilter
//
// Used to filter out unwanted shaders from the mtl/map browser. Also used as
// a PBValidtor.
//==============================================================================
class mrShaderFilter : public IMtlBrowserFilter, public PBValidator {

public:

	mrShaderFilter(
		unsigned int applyTypes,	// Accepted apply types, a combination of imrShaderClassDesc::ApplyFlags
		ParamType2 paramType,		// Accepted return types. Pass TYPE_MAX_TYPE to accept all types.
		bool acceptStructs			// Should shaders returning structures be accepted? (the structure has to contain a member with a valid type)
	);

	virtual ~mrShaderFilter();

	// Returns whether the given parameter type is accepted
	bool ValidType(ParamType2 type);

	// -- from IMtlBrowserFilter
	virtual const MCHAR* FilterName();
	virtual bool Enabled();
	virtual void Enable(bool enable);
	virtual void Registered();
	virtual void Unregistered();
	virtual bool Include(MtlBase& mtlBase, DWORD flags);
	virtual bool Include(ClassDesc& classDesc, DWORD flags);

	// -- from PBValidator
	virtual BOOL Validate(PB2Value& v);
	virtual BOOL Validate(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex);
	virtual void DeleteThis();

	// -- from InterfaceServer
	virtual BaseInterface* GetInterface(Interface_ID id);

private:

	// Returns whether the apply type of the given class is valid
	bool ValidateApplyType(ClassDesc& classDesc);
	// Returns whether the return type of the given class is valid
	bool ValidateReturnType(ClassDesc& classDesc);
	// Validates the return type based on a parameter block
	bool ValidateReturnType(ParamBlockDesc2& pbDesc);

	// -- Data Members -- 

	// Is this filter enabled?
	bool m_enabled;
	// Accept shaders returning structs?
	bool m_acceptStructs;

	// The accepted apply types
	unsigned int m_applyTypes;

	// The accepted return param types
	ParamType2 m_paramType;

};

inline mrShaderFilter* Get_mrShaderFilter(InterfaceServer* iserver) {

	if(iserver == NULL)
		return NULL;
	else
		return static_cast<mrShaderFilter*>(iserver->GetInterface(MRSHADERFILTER_INTERFACEID));
}


