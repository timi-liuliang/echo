//
// Copyright 2009 Autodesk, Inc.  All rights reserved. 
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may
// not be disclosed to, copied or used by any third party without the prior
// written consent of Autodesk, Inc.
#pragma once
#include "MaxHeap.h"
#include "maxtypes.h"
#include <WTypes.h>
#include "TabTypes.h"

//! \brief Graphics Strip Class
class Strip : public MaxHeapOperators
{
public:
	MtlID	mID;
	DWORD	smGrp;
	DWTab	v;
	DWTab	n;
	DWTab	tv;
	void AddVert(DWORD vtx)	{ v.Append(1, &vtx); }
	void AddVert(DWORD vtx, DWORD tvtx) { v.Append(1, &vtx); tv.Append(1, &tvtx); }
	void AddVertN(DWORD vtx, DWORD nor) { v.Append(1, &vtx); n.Append(1, &nor); }
	void AddVertN(DWORD vtx, DWORD tvtx, DWORD nor) { v.Append(1, &vtx); tv.Append(1, &tvtx); n.Append(1, &nor); }
};

typedef Strip* StripPtr;
typedef Tab<StripPtr> StripTab;