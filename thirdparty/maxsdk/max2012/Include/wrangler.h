/**********************************************************************
 *<
	FILE: wrangler.h
				  
	DESCRIPTION:  Wrangler information
	CREATED BY: Audrey Peterson

	HISTORY: created 1 February 2000

 *>     Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "tvnode.h"
#include "inode.h"
#include "maxapi.h"
#include "plugapi.h"
#include "pod.h"

#define OPENPV_NEW		0
#define OPENPV_LAST		-1
#define WRANGLER_CLASS_ID 		0x141a5cd6

#define BASER		1
class Wrangler: public ITrackViewNode {
public:
	Tab<INode*> podlist;
	int podmax,groupmax;
	Tab<TVNodeNotify*> notifyCBs;
	IParamBlock2 *pblock;
	virtual void DeleteThis(){};
	// Reference
	int NumRefs() { return BASER + podlist.Count();}
	#pragma warning(push)
	#pragma warning(disable:4100)
	virtual void AddPod(INode *node,int pos = TVNODE_APPEND){};
	virtual int FindPod(INode *pod){return -1;};
	virtual void RemovePod(INode *pod){};
	virtual void RemovePod(int i){};

	// From ITrackViewNode
	void AddNode(ITrackViewNode *node, MCHAR *name, Class_ID cid, int pos=TVNODE_APPEND){};
	void AddController(Control *c, MCHAR *name, Class_ID cid, int pos=TVNODE_APPEND){};
	int FindItem(Class_ID cid) {return -1;}
	void RemoveItem(int i){};
	void RemoveItem(Class_ID cid){};
	Control *GetController(int i) {return NULL;}
	Control *GetController(Class_ID cid) {return GetController(FindItem(cid));}
	ITrackViewNode *GetNode(int i) {return (ITrackViewNode*)podlist[i];}
	ITrackViewNode *GetNode(Class_ID cid){ return NULL;}
	int NumItems() {return podlist.Count();}
	virtual void SwapPositions(int i1, int i2){};
	MCHAR *GetName(int i) {return podlist[i]->GetName();}
	virtual void SetName(int i,MCHAR *name) {};
	virtual void RegisterTVNodeNotify(TVNodeNotify *notify){};
	virtual void UnRegisterTVNodeNotify(TVNodeNotify *notify){};
	virtual PodObj* GetPodByNumber(int num){return NULL;}
	virtual void MergeGroups(int oldg,int newg){};
	virtual IOResult Load(ILoad *iload)=0;
	virtual IOResult Save(ISave *isave)=0;

	// This is a temporary build fix from Scott Morrison 6/14/00
	virtual void HideChildren(BOOL chide) {}
	#pragma warning(pop)

};

