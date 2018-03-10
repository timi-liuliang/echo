/**********************************************************************
 *<
	FILE: IMtlEdit.h

	DESCRIPTION: Material Editor Interface

	CREATED BY: Nikolai Sander

	HISTORY: Created 6/22/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "ifnpub.h"
#include "GetCOREInterface.h"

// forward declarations
class MtlBase;

/*! \sa  Class MtlBase, Class FPStaticInterface\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface to the Material Editor. You can obtain a
pointer to the Material Editor interface using; <b>IMtlEditInterface*
GetMtlEditInterface ()</b>. This macro will return\n\n
<b>(IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE).</b>\n\n
All methods of this class are Implemented by the System.  */
class IMtlEditInterface : public FPStaticInterface 
{
public:
// function IDs 
	enum { 
		   fnIdGetCurMtl,
		   fnIdSetActiveMtlSlot,
		   fnIdGetActiveMtlSlot,
		   fnIdPutMtlToMtlEditor,
		   fnIdGetTopMtlSlot,
		   fnIdOkMtlForScene,
		   fnIdUpdateMtlEditorBrackets,
	};

	/*! \remarks This method returns a material base pointer for the current
	material. */
	virtual MtlBase *GetCurMtl() = 0;
	/*! \remarks This method allows you to set the active material slot.
	\par Parameters:
	<b>int i</b>\n\n
	The material slot index.\n\n
	<b>BOOL forceUpdate = FALSE</b>\n\n
	Set this to TRUE to update the slot contents. */
	virtual void SetActiveMtlSlot(int i, BOOL forceUpdate = FALSE)=0;
	/*! \remarks This method returns the index of the active material slot. */
	virtual int GetActiveMtlSlot()=0;
	/*! \remarks This method allows you to put the specified material to the
	specified material editor slot.
	\par Parameters:
	<b>MtlBase *mtlBase</b>\n\n
	The material you want to put.\n\n
	<b>int slot</b>\n\n
	The index of the material slot you wish to put the material into. */
	virtual void PutMtlToMtlEditor(MtlBase *mtlBase, int slot)=0;
	/*! \remarks This method returns a pointer to the material base from the
	specified slot.
	\par Parameters:
	<b>int slot</b>\n\n
	The index of the material slot for which you wish to obtain the material.
	*/
	virtual MtlBase* GetTopMtlSlot(int slot)=0;
	/*! \remarks	Before assigning material to scene, call this to avoid duplicate names.
	\par Parameters:
	<b>MtlBase *m</b>\n\n
	The pointer to the material. */
	virtual BOOL OkMtlForScene(MtlBase *m)=0;
	/*! \remarks	This method makes sure the Materials Editor slots correctly reflect which
	materials are used in the scene, which are used by selected objects, etc.
	This is used internally for the drag-and-drop of materials to nodes --
	there is no reason why a plug-in developer should need to call it. */
	virtual void UpdateMtlEditorBrackets()=0;

};


#define MTLEDIT_INTERFACE Interface_ID(0x2c7b3f6e, 0x16fb35d4)
inline IMtlEditInterface* GetMtlEditInterface () { return (IMtlEditInterface *)GetCOREInterface(MTLEDIT_INTERFACE); }

