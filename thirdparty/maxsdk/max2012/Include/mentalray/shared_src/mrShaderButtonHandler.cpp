/*==============================================================================

  file:     mrShaderButtonHandler.h

  author:   Daniel Levesque

  created:  23 April 2003

  description:

    Implementation of a DADMgr which handles shader buttons outside of the
	material editor.

  modified:	


© 2003 Autodesk
==============================================================================*/

#include "mrShaderButtonHandler.h"
#include "..\..\render.h"
#include "..\imrShader.h"

//==============================================================================
// class ShaderButtonDADMgr
//==============================================================================

mrShaderButtonHandler::mrShaderButtonHandler(unsigned int applyTypes, int ctrlID) 
: m_shaderFilter(applyTypes, TYPE_MAX_TYPE, true),
  m_ctrlID(ctrlID),
  m_dialogHWnd(NULL)
{

}

mrShaderButtonHandler::~mrShaderButtonHandler() {

}

void mrShaderButtonHandler::Enable(bool enable) {

	DbgAssert(m_dialogHWnd != NULL);

	HWND ctrlHWnd = GetDlgItem(m_dialogHWnd, m_ctrlID);
	ICustButton* custButton = GetICustButton(ctrlHWnd);
	if(custButton != NULL) {
		custButton->Enable(enable);
		ReleaseICustButton(custButton);
	}
	else {
		DbgAssert(false);
	}
}

void mrShaderButtonHandler::OnInitDialog(HWND hDialog) {

	m_dialogHWnd = hDialog;

	DbgAssert(m_dialogHWnd != NULL);

	HWND ctrlHWnd = GetDlgItem(m_dialogHWnd, m_ctrlID);
	ICustButton* custButton = GetICustButton(ctrlHWnd);
	if(custButton != NULL) {
		custButton->SetDADMgr(this);
		ReleaseICustButton(custButton);
	}
	else {
		DbgAssert(false);
	}

	Update();
}

void mrShaderButtonHandler::OnCommand() {

	DbgAssert(m_dialogHWnd != NULL);

	// Add the filter
	IMtlBrowserFilter_Manager* filterManager = Get_IMtlBrowserFilter_Manager();
	if(filterManager != NULL) {
		filterManager->AddFilter(m_shaderFilter);
	}

	// Browse for a texmap
	BOOL newMat;
	BOOL cancel;
	MtlBase* mtlBase = GetCOREInterface()->DoMaterialBrowseDlg(m_dialogHWnd, (BROWSE_MAPSONLY | BROWSE_INCNONE), newMat, cancel);
	if(!cancel) {
		DbgAssert((mtlBase == NULL) || ((mtlBase->SuperClassID() == TEXMAP_CLASS_ID)));

		Texmap* texmap = static_cast<Texmap*>(mtlBase);
		SetShader(texmap);
	
		Update();
	}

	if(filterManager != NULL) {
		filterManager->RemoveFilter(m_shaderFilter);
	}
}

void mrShaderButtonHandler::OnClose() {

	DbgAssert(m_dialogHWnd != NULL);
	m_dialogHWnd = NULL;
}

void mrShaderButtonHandler::Update() {

	DbgAssert(m_dialogHWnd != NULL);

	HWND ctrlHWnd = GetDlgItem(m_dialogHWnd, m_ctrlID);
	ICustButton* custButton = GetICustButton(ctrlHWnd);
	if(custButton != NULL) {

		MSTR text;
		Texmap* shader = GetShader();
		if(shader != NULL)
			text = shader->GetFullName();
		else
			text = GetNoneString();

		custButton->SetText(text.data());
		
		ReleaseICustButton(custButton);
	}
	else {
		DbgAssert(false);
	}
}
#pragma warning(push)
#pragma warning(disable:4100)
SClass_ID mrShaderButtonHandler::GetDragType(HWND hwnd, POINT p) {

	return TEXMAP_CLASS_ID;
}

ReferenceTarget* mrShaderButtonHandler::GetInstance(HWND hwnd, POINT p, SClass_ID type) {

	if(type == TEXMAP_CLASS_ID) {
		Texmap* shader = GetShader();
		return shader;
	}
	else {
		return NULL;
	}
}

BOOL mrShaderButtonHandler::OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew) {

	if(type == TEXMAP_CLASS_ID) {

		Texmap* texmap = static_cast<Texmap*>(dropThis);
		if(texmap == NULL) {
			return TRUE;
		}
		else {
			return m_shaderFilter.Include(*texmap, 0);
		}
	}

	return FALSE;
}
#pragma warning(pop)
int mrShaderButtonHandler::SlotOwner() {

	return OWNER_SCENE;
}

void mrShaderButtonHandler::Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type, DADMgr*, BOOL) {

	DbgAssert(OkToDrop(dropThis, NULL, hwnd, p, type, FALSE));
	if((dropThis == NULL) || (dropThis->SuperClassID() == TEXMAP_CLASS_ID)) {
		Texmap* texmap = static_cast<Texmap*>(dropThis);
		SetShader(texmap);

		Update();
	}
}

BOOL mrShaderButtonHandler::AutoTooltip() {

	return TRUE;
}