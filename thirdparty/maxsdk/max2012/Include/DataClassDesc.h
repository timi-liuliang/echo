#pragma once

// Copyright 2010 Autodesk, Inc.  All rights reserved. 
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may
// not be disclosed to, copied or used by any third party without the prior
// written consent of Autodesk, Inc.

#include "plugapi.h"
#include "Noncopyable.h"
#include "strclass.h"
#include "autoptr.h"

/** A proxy for a class descriptor. Plug-ins do not need to work directly with this class.
	When a plug-in DLL is defer loaded, the plug-in classes it exposes are represented
	by instances of this class. When the DataClassDesc instance does not have enough
	information to query out a request from client code, it will load the plug-in DLL
	which exposes the actual ClassDesc the DataClassDesc is a proxy for. From that point
	on, the actual ClassDesc instance is used.
	When a DataClassDesc instance is replaced by a full class descriptor, inside of 
	ClassDirectory::AddClass then a notification is sent:
	\code
	BroadcastNotification(NOTIFY_CLASSDESC_REPLACED, &cdr);
	\endcode
	\sa  Class ClassDesc, Class ClassEntry, Class BitmapIO, Class SceneImport, Class SceneExport.
*/
#pragma warning(push)
#pragma warning(disable:4100)
class DataClassDesc : public ClassDesc, public MaxSDK::Util::Noncopyable 
{
public:
	MSTR category;		//! The category string
	DWORD classIDA;		//! The first ulong of the Class_ID
	DWORD classIDB;		//! The second ulong of the Class_ID 
	SClass_ID superClassID; //! The SuperClassID of the class
	MSTR className;		//! The class name
	DWORD isPublic;		//! If this plugin is public or not.
	DWORD okToCreate;	//! If it is OK to create
	DWORD extCount;		//! Returns the number of file name extensions supported if this class descriptor represents a import or export plug-in
	MSTR ext;			//! The file name extension.
	MSTR shortDesc;		//! The short ASCII description.
	MSTR longDesc;		//! The long ASCII description
	DWORD supportsOptions;	//! The export options
	DWORD capability;	//! The BitmapIO module capability flags
	DWORD inputTypeA;	// For Modifiers. The first ulong of theModifier::InputType() Class_ID
	DWORD inputTypeB;	// For Modifiers. The second ulong of the Modifier::InputType() Class_ID
	MSTR internalName;
	MCHAR* internalNamePtr;

	/** Overwrites ClassDesc::IsPublic() */
	CoreExport virtual int IsPublic();
	CoreExport virtual void* Create(BOOL loading=FALSE);
	CoreExport virtual int BeginCreate(Interface* i);
	CoreExport virtual int EndCreate(Interface* i);
	/** Returns the class name for the plug-in (DataClassDesc::className). */
	CoreExport virtual const MCHAR* ClassName();
	/** Returns the SuperClassID (<b>superClassID</b>). Note:
	<b>typedef ulong SClass_ID;</b> */
	CoreExport virtual SClass_ID SuperClassID();
	/** Returns the Class_ID. */
	CoreExport virtual Class_ID ClassID();
	/** Returns the <b>category</b> string. */
	CoreExport virtual const MCHAR *Category();
	/** Returns the <b>okToCreate</b> state. */
	CoreExport virtual BOOL OkToCreate(Interface *i);
	CoreExport virtual BOOL HasClassParams();
	CoreExport virtual void EditClassParams(HWND hParent);
	CoreExport virtual void ResetClassParams(BOOL fileReset=FALSE);

	/** These functions return keyboard action tables that plug-ins can use */
	CoreExport virtual int NumActionTables();
	CoreExport virtual ActionTable *GetActionTable(int i);
	CoreExport virtual BOOL IsManipulator();
	CoreExport virtual BOOL CanManipulate(ReferenceTarget* hTarget);
	CoreExport virtual BOOL CanManipulateNode(INode* pNode);
	CoreExport virtual Manipulator* CreateManipulator(ReferenceTarget* hTarget, INode* pNode);
	CoreExport virtual Manipulator* CreateManipulator(INode* pNode);

	// Class IO
	CoreExport virtual BOOL NeedsToSave();
	CoreExport virtual IOResult Save(ISave *isave);
	CoreExport virtual IOResult Load(ILoad *iload);

	// bits of dword set indicate corresponding rollup page is closed.
	// the value 0x7fffffff is returned by the default implementation so the
	// command panel can detect this method is not being overridden, and just leave 
	// the rollups as is.
	CoreExport virtual DWORD InitialRollupPageState();

	// ParamBlock2-related metadata interface, supplied & implemented in ClassDesc2 (see maxsdk\include\iparamb2.h)
	CoreExport virtual const MCHAR* InternalName();
	CoreExport virtual HINSTANCE HInstance();
	//! access parameter block descriptors for this class
	CoreExport virtual int NumParamBlockDescs();
	CoreExport virtual ParamBlockDesc2* GetParamBlockDesc(int i);
	CoreExport virtual ParamBlockDesc2* GetParamBlockDescByID(BlockID id);
	CoreExport virtual void AddParamBlockDesc(ParamBlockDesc2* pbd);
	//! automatic UI management
	CoreExport virtual void BeginEditParams(IObjParam *ip, ReferenceMaker* obj, ULONG flags, Animatable *prev);
	CoreExport virtual void EndEditParams(IObjParam *ip, ReferenceMaker* obj, ULONG flags, Animatable *prev);
	CoreExport virtual void InvalidateUI(ParamBlockDesc2* pbd);
	CoreExport virtual MCHAR* GetRsrcString(INT_PTR id);
	//! automatic ParamBlock construction
	CoreExport virtual void MakeAutoParamBlocks(ReferenceMaker* owner);
	//! access automatically-maintained ParamMaps, by simple index or by associated ParamBlockDesc
	CoreExport virtual int NumParamMaps();
	CoreExport virtual IParamMap2* GetParamMap(int i);
	CoreExport virtual IParamMap2* GetParamMap(ParamBlockDesc2* pbd);
	//! maintain user dialog procs on automatically-maintained ParamMaps
	CoreExport virtual void SetUserDlgProc(ParamBlockDesc2* pbd, ParamMap2UserDlgProc* proc=NULL);
	CoreExport virtual ParamMap2UserDlgProc* GetUserDlgProc(ParamBlockDesc2* pbd);

	//! Class can draw an image to represent itself graphically...
	CoreExport virtual bool DrawRepresentation(COLORREF bkColor, HDC hDC, Rect &rect);

	CoreExport virtual int NumInterfaces();
	CoreExport virtual FPInterface* GetInterfaceAt(int i);
	CoreExport virtual FPInterface* GetInterface(Interface_ID id);
	CoreExport virtual FPInterface* GetInterface(MCHAR* name);
	CoreExport virtual void AddInterface(FPInterface* fpi);
	CoreExport virtual void ClearInterfaces();
	CoreExport virtual Class_ID SubClassID();
	CoreExport virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0); 

	/** \name Internal methods	*/
	//@{ 
	// Constructor that takes the mandatory information needed to create a DataClassDesc instance
	DataClassDesc(
		SClass_ID superClassId, 
		const Class_ID& classId, 
		const Class_ID& subClassId,
		int isClassPublic, 
		BOOL isClassOkToCreate, 
		const MCHAR* classNameStr,
		const MCHAR* categoryStr,
		BOOL hasClassParams,
		BOOL isManipulator,
		BOOL needsToSave,
		DWORD initialRollupPageState);
	~DataClassDesc();
	// Methods that set information that does not apply to all DataClassDesc instances
	void SetInternalName(const MSTR& theInternalName);
	//@}

private:
	DataClassDesc(); // disallowed
	class DataClassDescImpl;
	MaxSDK::Util::AutoPtr<DataClassDescImpl> mImpl;
};
#pragma warning(pop)
