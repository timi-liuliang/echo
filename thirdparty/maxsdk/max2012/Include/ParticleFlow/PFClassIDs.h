/*! \file PFClassIDs.h
    \brief Class and SubClass IDs for PF objects and actions.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-23-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "..\plugapi.h"

//==================================================================================//
//						PF SubClass IDs											//
//==================================================================================//
#define PFSubClassID_PartB PF_SUBCLASS_ID_PARTB
// for classes that should not be directly creatable by maxscript
#define PFSubClassID_PartB_MXS_NonCreatable PF_SUBCLASS_ID_PARTB+1

//  SubClassID to identify PF Engine plug-ins
#define PFEngineSubClassID Class_ID(0x74f93a00, PFSubClassID_PartB_MXS_NonCreatable)

//	SubClassID to identify particle channel plug-ins
#define ParticleChannelSubClassID Class_ID(0x74f93a01, PFSubClassID_PartB_MXS_NonCreatable)

//  SubClassID to identify PF Operator plug-ins that don't have 3D representation (icon)
#define PFOperatorSubClassID Class_ID(PF_OPERATOR_SUBCLASS_ID, PFSubClassID_PartB)
//  SubClassID to identify PF Operator plug-ins that have 3D representation (icon)
#define PFOperator3DSubClassID Class_ID(PF_OPERATOR3D_SUBCLASS_ID, PFSubClassID_PartB)
//  SubClassID to identify PF Test plug-ins that don't have 3D representation (icon)
#define PFTestSubClassID Class_ID(PF_TEST_SUBCLASS_ID, PFSubClassID_PartB)
//  SubClassID to identify PF Test plug-ins that have 3D representation (icon)
#define PFTest3DSubClassID Class_ID(PF_TEST3D_SUBCLASS_ID, PFSubClassID_PartB)

//  SubClassID to identify misc PF classes that don't have 3D representation (icon)
#define PFItemSubClassID Class_ID(PF_ITEM_SUBCLASS_ID, PFSubClassID_PartB)
#define PFItemSubClassID_MXS_NonCreatable Class_ID(PF_ITEM_SUBCLASS_ID, PFSubClassID_PartB_MXS_NonCreatable)

//  SubClassID to identify misc PF classes that have 3D representation (icon or else)
#define PFObjectSubClassID Class_ID(PF_OBJECT_SUBCLASS_ID, PFSubClassID_PartB)
#define PFObjectSubClassID_MXS_NonCreatable Class_ID(PF_OBJECT_SUBCLASS_ID, PFSubClassID_PartB_MXS_NonCreatable)

//  SubClassID to identify classes that represent particle system icons (like ParticleFlow shell)
#define PFSystemSubClassID Class_ID(0x74f93a08, PFSubClassID_PartB)

// SubClassID to identify classes to store action state
#define PFActionStateSubClassID Class_ID(0x74f93a09, PFSubClassID_PartB_MXS_NonCreatable)

// SubClassID to identify classes to store particle container
#define ParticleContainerSubClassID Class_ID(0x74f93a0a, PFSubClassID_PartB_MXS_NonCreatable)

//  SubClassID to identify misc PF classes that have their own initialization in PView
#define PViewItemSubClassID Class_ID(0x74f93a0b, PFSubClassID_PartB_MXS_NonCreatable)


//==================================================================================//
//						Standard Particle Channels									//
//==================================================================================//
#define ParticleChannelClassID_PartB 0x1eb34100

#define ParticleChannelNew_Class_ID			Class_ID(0x74f93b01, ParticleChannelClassID_PartB)
#define ParticleChannelID_Class_ID			Class_ID(0x74f93b02, ParticleChannelClassID_PartB)
#define ParticleChannelBool_Class_ID		Class_ID(0x74f93b03, ParticleChannelClassID_PartB) 
#define ParticleChannelInt_Class_ID			Class_ID(0x74f93b04, ParticleChannelClassID_PartB)
#define ParticleChannelFloat_Class_ID		Class_ID(0x74f93b05, ParticleChannelClassID_PartB) 
#define ParticleChannelPoint2_Class_ID		Class_ID(0x74f93b06, ParticleChannelClassID_PartB)// NIY 
#define ParticleChannelPoint3_Class_ID		Class_ID(0x74f93b07, ParticleChannelClassID_PartB)
#define ParticleChannelPTV_Class_ID			Class_ID(0x74f93b08, ParticleChannelClassID_PartB)
#define ParticleChannelInterval_Class_ID	Class_ID(0x74f93b09, ParticleChannelClassID_PartB)// NIY 
#define ParticleChannelAngAxis_Class_ID		Class_ID(0x74f93b0a, ParticleChannelClassID_PartB) 
#define ParticleChannelQuat_Class_ID		Class_ID(0x74f93b0b, ParticleChannelClassID_PartB) 
#define ParticleChannelMatrix3_Class_ID		Class_ID(0x74f93b0c, ParticleChannelClassID_PartB) 
#define ParticleChannelMesh_Class_ID		Class_ID(0x74f93b0d, ParticleChannelClassID_PartB)
#define ParticleChannelMeshMap_Class_ID		Class_ID(0x74f93b0e, ParticleChannelClassID_PartB)
#define ParticleChannelINode_Class_ID		Class_ID(0x74f93b0f, ParticleChannelClassID_PartB) 
#define ParticleChannelTabPoint3_Class_ID	Class_ID(0x74f93b10, ParticleChannelClassID_PartB)// NIY 
#define ParticleChannelTabFace_Class_ID		Class_ID(0x74f93b11, ParticleChannelClassID_PartB)// NIY 
#define ParticleChannelTabUVVert_Class_ID	Class_ID(0x74f93b12, ParticleChannelClassID_PartB)
#define ParticleChannelTabTVFace_Class_ID	Class_ID(0x74f93b13, ParticleChannelClassID_PartB)
#define ParticleChannelMap_Class_ID			Class_ID(0x74f93b14, ParticleChannelClassID_PartB)
#define ParticleChannelVoid_Class_ID		Class_ID(0x74f93b15, ParticleChannelClassID_PartB)

//==================================================================================//
//						Standard PF Actions										//
//==================================================================================//
#define PFActionClassID_PartB 0x1eb34200

//----------------------------------------------------------------------------------//
//						Standard PF Operators										//

#define PFOperatorViewportRender_Class_ID			Class_ID(0x74f93b01, PFActionClassID_PartB)// retired - Oleg
#define PFOperatorDisplay_Class_ID					Class_ID(0x74f93b02, PFActionClassID_PartB)// Oleg
#define PFOperatorRender_Class_ID					Class_ID(0x74f93b03, PFActionClassID_PartB)// Oleg
#define PFOperatorViewportMetaball_Class_ID			Class_ID(0x74f93b04, PFActionClassID_PartB)// NIY
#define PFOperatorRenderMetaball_Class_ID			Class_ID(0x74f93b05, PFActionClassID_PartB)// NIY
#define PFOperatorSimpleBirth_Class_ID				Class_ID(0x74f93b06, PFActionClassID_PartB)// Andy
#define PFOperatorSimplePosition_Class_ID			Class_ID(0x74f93b07, PFActionClassID_PartB)// Andy 
#define PFOperatorSimpleSpeed_Class_ID				Class_ID(0x74f93b08, PFActionClassID_PartB)// DavidT
#define PFOperatorSimpleOrientation_Class_ID		Class_ID(0x74f93b09, PFActionClassID_PartB)// DavidT
#define PFOperatorSimpleSpin_Class_ID				Class_ID(0x74f93b0a, PFActionClassID_PartB)// DavidT
#define PFOperatorSimpleShape_Class_ID				Class_ID(0x74f93b0b, PFActionClassID_PartB)// Andy
#define PFOperatorSimpleScale_Class_ID				Class_ID(0x74f93b0c, PFActionClassID_PartB)// DavidT
#define PFOperatorSimpleMapping_Class_ID			Class_ID(0x74f93b0d, PFActionClassID_PartB)// Oleg 
#define PFOperatorMaterial_Class_ID					Class_ID(0x74f93b0e, PFActionClassID_PartB)// retired - Oleg 
#define PFOperatorInstanceShape_Class_ID			Class_ID(0x74f93b0f, PFActionClassID_PartB)// Andy
#define PFOperatorMarkShape_Class_ID				Class_ID(0x74f93b10, PFActionClassID_PartB)// Andy
#define PFOperatorFacingShape_Class_ID				Class_ID(0x74f93b11, PFActionClassID_PartB)// Andy
#define PFOperatorMetaballShape_Class_ID			Class_ID(0x74f93b12, PFActionClassID_PartB)// NIY
#define PFOperatorFragmentShape_Class_ID			Class_ID(0x74f93b13, PFActionClassID_PartB)// NIY 
#define PFOperatorLongShape_Class_ID				Class_ID(0x74f93b14, PFActionClassID_PartB)// NIY 
#define PFOperatorExit_Class_ID						Class_ID(0x74f93b15, PFActionClassID_PartB)// Andy 
#define PFOperatorForceSpaceWarp_Class_ID			Class_ID(0x74f93b16, PFActionClassID_PartB)// Watje
#define PFOperatorPositionOnObject_Class_ID			Class_ID(0x74f93b17, PFActionClassID_PartB)// Oleg
#define PFOperatorPositionAgglomeration_Class_ID	Class_ID(0x74f93b18, PFActionClassID_PartB)// NIY 
#define PFOperatorSpeedAvoidCollisions_Class_ID		Class_ID(0x74f93b19, PFActionClassID_PartB)// NIY 
#define PFOperatorSpeedCopy_Class_ID				Class_ID(0x74f93b1a, PFActionClassID_PartB)// Oleg 
#define PFOperatorSpeedFollowLeader_Class_ID		Class_ID(0x74f93b1b, PFActionClassID_PartB)// NIY 
#define PFOperatorSpeedKeepApart_Class_ID			Class_ID(0x74f93b1c, PFActionClassID_PartB)// Oleg 
#define PFOperatorSpeedSurfaceNormals_Class_ID		Class_ID(0x74f93b1d, PFActionClassID_PartB)// Oleg 
#define PFOperatorOrientationFollowPath_Class_ID	Class_ID(0x74f93b1e, PFActionClassID_PartB)// NIY 
#define PFOperatorOrientationFacing_Class_ID		Class_ID(0x74f93b1f, PFActionClassID_PartB)// NIY 
#define PFOperatorSpinBySpeed_Class_ID				Class_ID(0x74f93b20, PFActionClassID_PartB)// NIY 
#define PFOperatorBirthByObjectGroup_Class_ID		Class_ID(0x74f93b21, PFActionClassID_PartB)// NIY 
#define PFOperatorScriptBirth_Class_ID				Class_ID(0x74f93b22, PFActionClassID_PartB)// Oleg
#define PFOperatorScript_Class_ID					Class_ID(0x74f93b23, PFActionClassID_PartB)// Oleg
#define PFOperatorComments_Class_ID					Class_ID(0x74f93b24, PFActionClassID_PartB)// Oleg
#define PFOperatorCache_Class_ID					Class_ID(0x74f93b25, PFActionClassID_PartB)// Oleg
#define PFOperatorMaterialStatic_Class_ID			Class_ID(0x74f93b26, PFActionClassID_PartB)// Oleg 
#define PFOperatorMaterialDynamic_Class_ID			Class_ID(0x74f93b27, PFActionClassID_PartB)// Oleg 
#define PFOperatorMaterialFrequency_Class_ID		Class_ID(0x74f93b28, PFActionClassID_PartB)// Oleg 
#define PFOperatorFileBirth_Class_ID                Class_ID(0x74f93b29, PFActionClassID_PartB)
#define PFOperatorShapeLib_Class_ID					Class_ID(0x53346701, PFActionClassID_PartB)// Box#1
#define PFOperatorExpressSave_Class_ID				Class_ID(0x5334670A, PFActionClassID_PartB)// Box#1

//----------------------------------------------------------------------------------//
//						Standard PF Tests and Test Operators						//

#define PFTestDuration_Class_ID						Class_ID(0x74f93c01, PFActionClassID_PartB)// Oleg
#define PFTestSpawn_Class_ID						Class_ID(0x74f93c02, PFActionClassID_PartB)// Oleg 
#define PFTestCollisionSpaceWarp_Class_ID			Class_ID(0x74f93c03, PFActionClassID_PartB)// Watje 
#define PFTestSpawnCollisionSW_Class_ID				Class_ID(0x74f93c04, PFActionClassID_PartB)// Oleg 
#define PFTestSpeed_Class_ID						Class_ID(0x74f93c05, PFActionClassID_PartB)// Oleg 
#define PFTestSpeedGoToTarget_Class_ID				Class_ID(0x74f93c06, PFActionClassID_PartB)// Oleg 
#define PFTestScale_Class_ID						Class_ID(0x74f93c07, PFActionClassID_PartB)// Oleg 
#define PFTestProximity_Class_ID					Class_ID(0x74f93c08, PFActionClassID_PartB)// NIY 
#define PFTestScript_Class_ID						Class_ID(0x74f93c09, PFActionClassID_PartB)// Oleg 
#define PFTestGoToNextEvent_Class_ID				Class_ID(0x74f93c0a, PFActionClassID_PartB)// Oleg
#define PFTestSplitByAmount_Class_ID				Class_ID(0x74f93c0b, PFActionClassID_PartB)// Oleg
#define PFTestSplitBySource_Class_ID				Class_ID(0x74f93c0c, PFActionClassID_PartB)// Oleg
#define PFTestSplitSelected_Class_ID				Class_ID(0x74f93c0d, PFActionClassID_PartB)// Oleg
#define PFTestGoToRotation_Class_ID					Class_ID(0x74f93c0e, PFActionClassID_PartB)// Oleg
 


//==================================================================================//
//								PF Actors											//
//==================================================================================//
#define PFActorClassID_PartB 0x1eb34300

#define  PFEngine_Class_ID				Class_ID(0x74f93b01, PFActorClassID_PartB)
#define  ParticleGroup_Class_ID			Class_ID(0x74f93b02, PFActorClassID_PartB)
#define  PFActionList_Class_ID			Class_ID(0x74f93b03, PFActorClassID_PartB)
#define  PFArrow_Class_ID				Class_ID(0x74f93b04, PFActorClassID_PartB)
#define  PFIntegrator_Class_ID			Class_ID(0x74f93b05, PFActorClassID_PartB)
#define  PViewManager_Class_ID			Class_ID(0x74f93b06, PFActorClassID_PartB)
#define  ParticleView_Class_ID			Class_ID(0x74f93b07, PFActorClassID_PartB)
#define  PFActionListPool_Class_ID		Class_ID(0x74f93b08, PFActorClassID_PartB)
#define  PFSystemPool_Class_ID			Class_ID(0x74f93b09, PFActorClassID_PartB)
#define  PFSimpleActionState_Class_ID	Class_ID(0x74f93b0a, PFActorClassID_PartB)
#define  ParticleContainer_Class_ID		Class_ID(0x74f93b0b, PFActorClassID_PartB)
#define	 PFNotifyDepCatcher_Class_ID	Class_ID(0x74f93b0c, PFActorClassID_PartB)

//==================================================================================//
//								PF Materials										//
//==================================================================================//
#define PFMaterialClassID_PartB 0x1eb34400

#define  ParticleBitmap_Class_ID		Class_ID(0x74f93d01, PFMaterialClassID_PartB)

//==================================================================================//
//								PF Color Scheme										//
//==================================================================================//
#define PFSourceGizmoColorId 0x13597ec7
#define pfSourceGizmoColor RGB(243,180,97)
#define PFOperatorGizmoColorId 0x204449b5
#define pfOperatorGizmoColor RGB(68,115,149)
#define PFTestGizmoColorId 0x0be30545
#define pfTestGizmoColor RGB(253,229,55)
#define PFSubselectionColorId 0x35350e4
#define pfSubselectionColor RGB(255, 0, 0)


