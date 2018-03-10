//**************************************************************************/
// Copyright (c) 1998-2005 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: MAXScript scriptable controllers for MAX
// AUTHOR: Larry.Minton - created May.14.2005
//***************************************************************************/

#pragma once

#include "..\..\ifnpub.h"
#include "..\..\control.h"

#define FLOAT_SCRIPT_CONTROL_CLASS_ID		Class_ID(0x498702e6, 0x71f11548)
#define POSITION_SCRIPT_CONTROL_CLASS_ID	Class_ID(0x5065767b, 0x683a42a5)
#define POINT3_SCRIPT_CONTROL_CLASS_ID		Class_ID(0x46972869, 0x2f7f05ce)
#define POINT4_SCRIPT_CONTROL_CLASS_ID		Class_ID(0x46972870, 0x2f7f05cf)
#define ROTATION_SCRIPT_CONTROL_CLASS_ID	Class_ID(0x31381912, 0x3a904166)
#define SCALE_SCRIPT_CONTROL_CLASS_ID		Class_ID(0x7c8f3a2a, 0x1e954d91)
#define PRS_SCRIPT_CONTROL_CLASS_ID			Class_ID(0x7f56455c, 0x1be66c68)

class IBaseScriptControl : public StdControl {
public:
	virtual MCHAR*	get_script_text() = 0;
	virtual void	set_script_text(MCHAR* text) = 0;
	virtual bool	update_refs() = 0;
	virtual void	depends_on(ReferenceTarget* ref) = 0;
};

class IBaseScriptControl8 : public IBaseScriptControl {
public:
	virtual int		getVarCount() = 0;
	virtual MSTR&	getVarName(int i) = 0;
	virtual FPValue&	getVarValue(int i, TimeValue t) = 0;
};


/***************************************************************
Function Publishing System   
****************************************************************/

#define IID_SCRIPT_CONTROL Interface_ID(0x15b3e322, 0x6a176aa5)

///////////////////////////////////////////////////////////////////////////////

// Base Script Controller Interface
class IScriptCtrl: public FPMixinInterface
{
public:

	// Function Publishing System
	enum {
		fnIdSetExpression, fnIdGetExpression,
		fnIdNumVars, 
		fnIdAddConstant, fnIdAddTarget, fnIdAddObject, fnIdAddNode,
		fnIdSetConstant, fnIdSetTarget, fnIdSetObject, fnIdSetNode,
		fnIdDeleteVariable,
		fnIdSetDescription, fnIdGetDescription,
		fnIdVariableExists,
		fnIdGetOffset, fnIdSetOffset,
		fnIdUpdate,
		fnIdGetConstant, fnIdGetTarget, fnIdGetObject, fnIdGetNode, fnIdGetValue, 
		fnIdGetVarValue,
		fnIdGetType,
		fnIdGetName, fnIdGetIndex,
		fnIdPrintDetails,
		fnIdGetThrowOnError, fnIdSetThrowOnError,
		fnIdRenameVariable,
		enumValueType,
	};

	// Function Map For Mixin Interface
	///////////////////////////////////////////////////////////////////////////
#pragma warning(push)
#pragma warning(disable:4238)
	BEGIN_FUNCTION_MAP

		FN_1( fnIdSetExpression, TYPE_BOOL, SetExpression, TYPE_TSTR_BR);
		FN_0( fnIdGetExpression, TYPE_TSTR_BV, GetExpression);

		FN_0 ( fnIdGetDescription, TYPE_TSTR_BV, GetDescription);
		FN_1 ( fnIdSetDescription, TYPE_BOOL, SetDescription, TYPE_TSTR_BR);

		FN_0 ( fnIdNumVars, TYPE_INT, NumVariables);

		FN_2 ( fnIdAddConstant, TYPE_BOOL, AddConstant, TYPE_TSTR_BR, TYPE_FPVALUE_BR); 
		FN_4 ( fnIdAddTarget, TYPE_BOOL, AddTarget, TYPE_TSTR_BR, TYPE_VALUE, TYPE_TIMEVALUE, TYPE_VALUE);
		FN_2 ( fnIdAddObject, TYPE_BOOL, AddObject, TYPE_TSTR_BR, TYPE_VALUE);
		FN_2 ( fnIdAddNode, TYPE_BOOL, AddNode, TYPE_TSTR_BR, TYPE_INODE);

		FN_2 ( fnIdSetConstant, TYPE_BOOL, SetConstant, TYPE_VALUE, TYPE_FPVALUE_BR); 
		FN_3 ( fnIdSetTarget, TYPE_BOOL, SetTarget, TYPE_VALUE, TYPE_VALUE, TYPE_VALUE);
		FN_2 ( fnIdSetObject, TYPE_BOOL, SetObject, TYPE_VALUE, TYPE_VALUE);
		FN_2 ( fnIdSetNode, TYPE_BOOL, SetNode, TYPE_VALUE, TYPE_INODE);

		FN_1 ( fnIdDeleteVariable, TYPE_BOOL, DeleteVariable, TYPE_VALUE);

		FN_2 ( fnIdRenameVariable, TYPE_BOOL, RenameVariable, TYPE_VALUE, TYPE_TSTR_BR);

		FN_1 ( fnIdVariableExists, TYPE_BOOL, VariableExists, TYPE_TSTR_BR);

		FN_1 ( fnIdGetOffset, TYPE_TIMEVALUE, GetOffset, TYPE_VALUE);
		FN_2 ( fnIdSetOffset, TYPE_BOOL, SetOffset, TYPE_VALUE, TYPE_TIMEVALUE);

		FNT_1 ( fnIdGetConstant, TYPE_FPVALUE_BV, GetConstant, TYPE_VALUE);
		FN_2 ( fnIdGetTarget, TYPE_VALUE, GetTarget, TYPE_VALUE, TYPE_BOOL);
		FN_1 ( fnIdGetObject, TYPE_REFTARG, GetObject, TYPE_VALUE);
		FN_1 ( fnIdGetNode, TYPE_INODE, GetNode, TYPE_VALUE);

		FNT_2 ( fnIdGetValue, TYPE_VALUE, GetVariable, TYPE_VALUE, TYPE_BOOL);
		FNT_1 ( fnIdGetVarValue, TYPE_VALUE, GetVarValue, TYPE_VALUE);

		FN_1 (fnIdGetType, TYPE_ENUM, GetVarType, TYPE_VALUE);
		FN_1 (fnIdGetName, TYPE_TSTR_BV, GetVarName, TYPE_INDEX);
		FN_1 (fnIdGetIndex, TYPE_INDEX, GetVarIndex, TYPE_TSTR_BR);

		VFN_0 ( fnIdUpdate, Update);
		FN_0 ( fnIdPrintDetails, TYPE_TSTR_BV, PrintDetails);

		VFN_1 (fnIdSetThrowOnError, SetThrowOnError, TYPE_bool);
		FN_0 (fnIdGetThrowOnError, TYPE_bool, GetThrowOnError);
	END_FUNCTION_MAP
#pragma warning(pop)
	FPInterfaceDesc* GetDesc(); 

	// Published Functions
	virtual bool		GetThrowOnError() = 0;
	virtual void		SetThrowOnError(bool bOn) = 0;

	virtual MSTR		PrintDetails() = 0;
	virtual void		Update() = 0;

	virtual BOOL		SetExpression(MSTR &expression) = 0;
	virtual MSTR		GetExpression() = 0;
	
	virtual MSTR		GetDescription() = 0;
	virtual BOOL		SetDescription(MSTR &expression) = 0;

	virtual BOOL		AddConstant(MSTR &name, FPValue &val) = 0; 
	virtual BOOL		AddTarget(MSTR &name, Value* target, int ticks, Value *owner = NULL) = 0;
	virtual BOOL		AddObject(MSTR &name, Value* obj) = 0;
	virtual BOOL		AddNode(MSTR &name, INode* theNode) = 0;

	virtual BOOL		SetConstant(Value* which, FPValue &val) = 0; 
	virtual BOOL		SetTarget(Value* which, Value* target, Value *owner = NULL) = 0;
	virtual BOOL		SetObject(Value* which, Value* obj) = 0;
	virtual BOOL		SetNode(Value* which, INode* theNode) = 0;

	virtual FPValue		GetConstant(Value* which, TimeValue t ) = 0;
	virtual Value*		GetTarget(Value* which, BOOL asObject = FALSE) = 0;
	virtual ReferenceTarget*	GetObject(Value* which) = 0;
	virtual INode*		GetNode(Value* which) = 0;
	virtual Value*		GetVariable(Value* which, BOOL asObject = FALSE, TimeValue t = 0) = 0;

	virtual int			NumVariables() = 0;
	virtual BOOL		DeleteVariable(Value* which) = 0;
	virtual BOOL		VariableExists(MSTR &name) = 0;
	virtual BOOL		RenameVariable(Value* which, MSTR &name) = 0;

	virtual TimeValue	GetOffset(Value* which) = 0;
	virtual BOOL		SetOffset(Value* which, TimeValue tick) = 0;

	virtual Value*		GetVarValue(Value* which, TimeValue t) = 0;

	virtual int			GetVarType(Value* which) = 0;
	virtual MSTR		GetVarName(int index) = 0;
	virtual int			GetVarIndex(MSTR &name) = 0;
};

