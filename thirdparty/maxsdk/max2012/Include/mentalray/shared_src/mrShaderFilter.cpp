/*==============================================================================

  file:     mrShaderFilter.cpp

  author:   Daniel Levesque

  created:  21feb2003

  description:

    The filter for the material/map browser, which also doubles as a PBValidator.

  modified:	


© 2003 Autodesk
==============================================================================*/
#include "mrShaderFilter.h"
#include "..\..\maxapi.h"
#include "..\..\imtl.h"
#include "..\imrShaderClassDesc.h"
#include "..\imrPreferences.h"
#include "..\imrShaderTranslation.h"

//==============================================================================
// class mrShaderFilter
//==============================================================================

mrShaderFilter::mrShaderFilter(
	unsigned int applyTypes,	// Accepted apply types, a combination of imrShaderClassDesc::ApplyFlags
	ParamType2 paramType,		// Accepted return types. Pass TYPE_MAX_TYPE to accept all types.
	bool acceptStructs			// Should shaders returning structures be accepted? (the structure has to contain a member with a valid type)
)
: m_enabled(true),
  m_applyTypes(applyTypes),
  m_paramType(paramType),
  m_acceptStructs(acceptStructs)
{
}

mrShaderFilter::~mrShaderFilter() {

}

const MCHAR* mrShaderFilter::FilterName() {

	return _M("mental ray: Shader Filter");
}

bool mrShaderFilter::Enabled() {

	return m_enabled;
}

void mrShaderFilter::Enable(bool enable) {

	m_enabled = enable;
}

void mrShaderFilter::Registered() {

	// do nothing
}

void mrShaderFilter::Unregistered() {

	// do nothing
}

bool mrShaderFilter::Include(MtlBase& mtlBase, DWORD flags)
{
	Interface* ip = GetCOREInterface();
	if (!ip) {
		return false;
	}
	// Validate using the class ID
	ClassDesc* classDesc = ip->GetDllDir().ClassDir().FindClass( mtlBase.SuperClassID(), mtlBase.ClassID() );
	DbgAssert(classDesc != NULL);
	if(classDesc != NULL) {
		return Include(*classDesc, flags);
	}
	else {
		return true;
	}
}
#pragma warning(push)
#pragma warning(disable:4100)
bool mrShaderFilter::Include(ClassDesc& classDesc, DWORD flags)
{
	return (ValidateApplyType(classDesc) && ValidateReturnType(classDesc));
}

BOOL mrShaderFilter::Validate(PB2Value& v) {

	ReferenceTarget* refTarg = v.r;
	if((refTarg != NULL) && IsMtlBase(refTarg)) {
		MtlBase* mtlBase = static_cast<MtlBase*>(refTarg);
		return Include(*mtlBase, 0);
	}
	else {
		// Null values are acceptable
		return TRUE;
	}
}

BOOL mrShaderFilter::Validate(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex)
{
	return Validate(v);
}
#pragma warning(pop)

BaseInterface* mrShaderFilter::GetInterface(Interface_ID id) {

	if(id == MRSHADERFILTER_INTERFACEID) {
		return this;
	}
	else if(id == IMTLBROWSERFILTER_INTERFACEID) {
		return static_cast<IMtlBrowserFilter*>(this);
	}
	else {
		return PBValidator::GetInterface(id);
	}
}

bool mrShaderFilter::ValidateApplyType(ClassDesc& classDesc) {

	SClass_ID superClassID = classDesc.SuperClassID();

	if(superClassID == MATERIAL_CLASS_ID) {

		// Validate if materials are accepted
		return ((m_applyTypes & imrShaderClassDesc::kApplyFlag_MtlPhen) != 0);
	}
	else if(superClassID == TEXMAP_CLASS_ID) {

		// First, query for a possible custom apply type
		imrShaderTranslation_ClassInfo* customClassInfo = Get_imrShaderTranslation_ClassInfo(classDesc);
		if(customClassInfo != NULL) {

			unsigned int customApplyType = customClassInfo->GetApplyTypes();
			return ((customApplyType & m_applyTypes) != 0);
		}
		else {
			imrShaderClassDesc* shaderClassDesc = Get_mrShaderClassDesc(&classDesc);

			if(shaderClassDesc == NULL) {
				// Not a mr shader. Validate if texture shaders are accepted.
				return ((m_applyTypes & imrShaderClassDesc::kApplyFlag_Texture) != 0);
			}
			else {

				// Depends on the apply type of the shader
				unsigned int shaderApplyTypes = shaderClassDesc->GetApplyTypes();
				return ((shaderApplyTypes & m_applyTypes) != 0);
			}
		}
	}
	else {
		// Shouldn't occur
		DbgAssert(false);
		return false;
	}
}

bool mrShaderFilter::ValidateReturnType(ClassDesc& classDesc) {


	SClass_ID superClassID = classDesc.SuperClassID();

	if(superClassID == MATERIAL_CLASS_ID) {

		// Validate if we want a TYPE_MTL return type
		//return ValidType(TYPE_MTL);
		
		// Do not validate types for materials. Only use the apply type for validatoin.
		return true;
	}
	else if(superClassID == TEXMAP_CLASS_ID) {

		imrShaderClassDesc* shaderClassDesc = Get_mrShaderClassDesc(&classDesc);

		if(shaderClassDesc == NULL) {
			// Assume that the texture returns a color
			return ValidType(static_cast<ParamType2>(TYPE_RGBA));
		}
		else {

			// Go through the results structure
			ParamBlockDesc2* pbDesc = shaderClassDesc->GetResultPBDesc();
			if((pbDesc != NULL) && ValidateReturnType(*pbDesc)) {
				return true;
			}

			// Go through the sub-structures, is allowed
			if(m_acceptStructs) {
				Tab<ParamBlockDesc2*>& resultDescs = shaderClassDesc->GetResultPBDescs();
				int count = resultDescs.Count();
				for(int i = 0; i < count; ++i) {
					ParamBlockDesc2* pbDesc = resultDescs[i];
					if((pbDesc != NULL) && ValidateReturnType(*pbDesc)) {
						return true;
					}
				}
			}			

			return false;
		}
	}
	else {
		// Shouldn't occur
		DbgAssert(false);
		return false;
	}
}

bool mrShaderFilter::ValidType(ParamType2 type)
{
	int choice = (int)m_paramType;
	switch(choice)
	{
	case TYPE_MAX_TYPE:
		return true;

	// Casts to more complex types are acceptable
	case TYPE_RGBA:
	case TYPE_FRGBA:
		return ((type == TYPE_RGBA) || (type == TYPE_FRGBA));

	case TYPE_POINT3:
	case TYPE_POINT4:
		return ((type == TYPE_RGBA)
			|| (type == TYPE_FRGBA)
			|| (type == TYPE_POINT3)
			|| (type == TYPE_POINT4));

	case TYPE_FLOAT:
		return ((type == TYPE_RGBA)
			|| (type == TYPE_FRGBA)
			|| (type == TYPE_POINT3)
			|| (type == TYPE_FLOAT));

	default:
		return (type == m_paramType);
	}
}

bool mrShaderFilter::ValidateReturnType(ParamBlockDesc2& pbDesc) {

	// Go through all the parameters of the block
	int count = pbDesc.Count();
	for(int i = 0; i < count; ++i) {
		const ParamDef& paramDef = pbDesc.GetParamDef(pbDesc.IndextoID(i));
		
		if(ValidType(paramDef.type))
			return true;
	}

	return false;
}

void mrShaderFilter::DeleteThis() {

	delete this;
}