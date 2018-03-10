/**********************************************************************
*<
FILE: patchcap.h

DESCRIPTION:  Patch Capping header file

CREATED BY: Tom Hudson

HISTORY: created 21 December, 1995

*>	Copyright (c) 1995, All Rights Reserved.
**********************************************************************/

#pragma once

#include "maxheap.h"
#include "linklist.h"
#include "patch.h"

class PatchSeg: public MaxHeapOperators
{
public:
	int vert;
	int vec1;
	int vec2;
	PatchSeg() {}
	PatchSeg(int v, int v1, int v2) { vert=v; vec1=v1; vec2=v2; }
};

typedef LinkedEntryT<PatchSeg> PatchSegEntry;
typedef LinkedListT<PatchSeg,PatchSegEntry> PatchSegList;

typedef LinkedEntryT<PatchSegList> PatchSegListEntry;
typedef LinkedListT<PatchSegList,PatchSegListEntry> PatchSegListList;

typedef LinkedEntryT<Patch> PatchEntry;
typedef LinkedListT<Patch,PatchEntry> PatchList;

typedef LinkedEntryT<PatchVert> PatchVertEntry;
typedef LinkedListT<PatchVert,PatchVertEntry> PatchVertList;

typedef LinkedEntryT<PatchVec> PatchVecEntry;
typedef LinkedListT<PatchVec,PatchVecEntry> PatchVecList;

class PatchCapWork: public MaxHeapOperators
{
public:
	PatchSegListList plist;
	PatchVertList newVerts;
	PatchVecList newVecs;
	PatchList newPatches;
	CoreExport void Dump(char *title = NULL);
};

