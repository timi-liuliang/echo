/*! \file PFMessages.h
    \brief list of REFMSG_USER type messages for PF communications
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 12-19-01

 *>	Copyright (c) 2001, All Rights Reserved.
**********************************************************************/

#pragma once
#include "..\maxheap.h"
#include "..\ref.h"
#include "..\hitdata.h"
#include "PreciseTimeValue.h"
// forward declarations
class ModContext;
class ViewExp;
class Box3;
class Mesh;
class INode;
class FPMixinInterface;
class SubObjAxisCallback;
class Point3;
class Quat;
class AngAxis;

// PF notification messages
enum {	kPFMSG_UpdateToTime = REFMSG_USER + 4878,		// x1130E
		kPFMSG_HasSurplus,								// x1130F
		kPFMSG_PFShellWiringChanged,					// x11310
		kPFMSG_ActionListWiringChanged,					// x11311
		kPFMSG_GetNextActionList,						// x11312
		kPFMSG_GetNumParticles,							// x11313
		kPFMSG_GetNumParticlesGenerated,				// x11314
		kPFMSG_GetUpdateTime,
		kPFMSG_GetUpdateInterval,
		kPFMSG_GetParticleBornIndex,
		kPFMSG_GetParticleIndex,
		kPFMSG_GetParticleGroup,
		kPFMSG_GetParticleTime,
		kPFMSG_SetParticleTime,
		kPFMSG_GetParticleAge,
		kPFMSG_SetParticleAge,
		kPFMSG_GetParticleLifeSpan,
		kPFMSG_SetParticleLifeSpan,
		kPFMSG_GetParticleEventTime,
		kPFMSG_SetParticleEventTime,
		kPFMSG_GetParticlePosition,
		kPFMSG_SetParticlePosition,
		kPFMSG_GetParticleSpeed,
		kPFMSG_SetParticleSpeed,
		kPFMSG_GetParticleOrientation,
		kPFMSG_SetParticleOrientation,
		kPFMSG_GetParticleSpin,
		kPFMSG_SetParticleSpin,
		kPFMSG_GetParticleScale,
		kPFMSG_SetParticleScale,
		kPFMSG_GetParticleSelected,
		kPFMSG_SetParticleSelected,
		kPFMSG_GetParticleShape,
		kPFMSG_SetParticleShape,
		kPFMSG_SetParticleGlobalShape,
		kPFMSG_UpdateMaterial,
		kPFMSG_ActionListContentChanged,
		kPFMSG_OpenParticleView,
		kPFMSG_IsDownStream,
		kPFMSG_UpdateTypeChanged,
		kPFMSG_InvalidateParticles,
		kPFMSG_InvalidateViewportParticles,
		kPFMSG_InvalidateRenderParticles,
		kPFMSG_GetSubObjectCentersRequest,
		kPFMSG_GetSubObjectTMsRequest,
		kPFMSG_HitTestRequest,
		kPFMSG_GetBoundBoxRequest,
		kPFMSG_GetNumParticlesActionList,
		kPFMSG_GetParticleBornIndexActionList,
		kPFMSG_UpdateWireColor,
		kPFMSG_GetNodeValue,
		kPFMSG_HasUpStreamActionList,
		kPFMSG_IsActionActive,
		kPFMSG_GetParentActionList,
		kPFMSG_PreDeleteNode,
		kPFMSG_GetNumParticlesInParticleGroup,
		kPFMSG_InvalidateDownStream,
		kPFMSG_ActionListActivityChanged,
		kPFMSG_SyncRenderState,
		kPFMSG_CheckEscape,
		kPFMSG_DynamicNameChange,
		kPFMSG_ConfirmNotifyCatcherPresence,
		kPFMSG_cacheUpdateStart,
		kPFMSG_cacheUpdateFinish
	};

struct GetSubObjectRequestData: public MaxHeapOperators {
	SubObjAxisCallback* cb;
	TimeValue t;
	INode* node;
	ModContext* mc;
};

struct HitTestRequestData: public MaxHeapOperators {
	TimeValue t;
	INode* inode;
	int type;
	int crossing;
	int flags;
	IPoint2* p;
	ViewExp* vpt;
	ModContext* mc;
	int hitResult;
};

struct GetBoundBoxRequestData: public MaxHeapOperators {
	TimeValue t;
	INode* inode;
	ViewExp* vp;
	Box3* box;
};

struct GetParticleDataRequest: public MaxHeapOperators {
	int index;
	union {
		int count;
		int bornIndex;
		INode* pGroup;
		PreciseTimeValue* time;
		PreciseTimeValue* age;
		PreciseTimeValue* lifeSpan;
		PreciseTimeValue* eventTime;
		Point3* position;
		Point3* speed;
		Point3* acceleration;
		Quat*   orientation;
		AngAxis* spin;
		Point3* scale;
		bool selected;
		Mesh* shape;
	};
};

struct GetTimeDataRequest: public MaxHeapOperators {
	TimeValue time;
	TimeValue start;
	TimeValue finish;
};

// the class is used to identify particle group
// and action list the hit particle belongs to
class PFHitData : public HitData {
public:
	INode* particleGroupNode;
	INode* actionListNode;
	PFHitData() { particleGroupNode = NULL; actionListNode = NULL; }
	PFHitData(INode* pgroup, INode* alist) { particleGroupNode = pgroup; actionListNode = alist; }
};

// the class is used to retrieve activity status
// of an action item and its parent action list
struct PFActiveActionDataRequest: public MaxHeapOperators {
	INode* actionNode;
	int active;
	FPMixinInterface* iActionList;
};

