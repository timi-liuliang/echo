/*! \file PFActionStateDesc.h
    \brief	Class Descriptor for PF ActionState objects (declaration)
*/
/**********************************************************************
 *<
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 28-10-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\iparamb2.h"
#include "..\ifnpub.h"

//	ActionState-generic Descriptor declarations
class PFActionStateDesc: public ClassDesc2 {
public:
	PFExport virtual int IsPublic();
	virtual void*	Create(BOOL loading = FALSE) = 0;
	PFExport virtual const MCHAR *	ClassName();
	PFExport virtual SClass_ID SuperClassID();
	PFExport virtual Class_ID SubClassID();
	virtual Class_ID	ClassID() = 0;
	PFExport const MCHAR* Category();
	virtual const MCHAR* InternalName() = 0;
};

