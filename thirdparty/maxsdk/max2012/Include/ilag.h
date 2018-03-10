//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
//
// \TODO Consider these classes for a move to maxsdk/samples/modifiers/flex/...
//
#pragma once

#include "maxheap.h"
#include "iFnPub.h"
#include "object.h"

#define LAZYID 0xDE17A34f, 0x8A41E2B0


class SpringClass: public MaxHeapOperators
{
public:
	Point3 vel, pos, init_pos;
	float InheritVel;
	BOOL modified;
	Point3 LocalPt;

	Point3 tempVel[6];
	Point3 tempPos[6];
	float dist;
	float mass;
};

class EdgeBondage: public MaxHeapOperators
{
public:
	float dist;
	int v1,v2;
	int flags;
};


class CacheClass: public MaxHeapOperators
{
public:
Point3 vel, pos;

};


class LagModData : public LocalModData {
	public:

		Tab<EdgeBondage> edgeSprings;

		int id;
		INode *SelfNode;
		Tab<SpringClass> SpringList;
		Tab<CacheClass> WholeFrameCache;

		Tab<BYTE> esel;  //selection for edges vertices
		Tab<BYTE> wsel;		//vertex weight selection
		Tab<BYTE> psel;		
		Matrix3 InverseTM;


		BOOL isMesh;
		BOOL isPatch;

		BOOL addSprings;
		BOOL removeSprings;
		BOOL computeEdges;
		BOOL ignoreInteriorHandles;
		BOOL simpleSoft;

		Point3 hitPoint;
		BOOL isHit;
		TimeValue lastFrame;
		BOOL nukeRenderCache;
		Tab<Point3> pointCache;

		LagModData()
			{
			SelfNode = NULL;
			id = -1;
			computeEdges = TRUE;
			isMesh = FALSE;
			isPatch = FALSE;
			lastFrame = 999999999;
			nukeRenderCache = TRUE;
			addSprings = FALSE;
			removeSprings = FALSE;
			simpleSoft = FALSE;

			}
		LagModData(int i, INode *n)
			{
			id = i;
			SelfNode = n;
			isMesh = FALSE;
			isPatch = FALSE;
			lastFrame = 999999999;
			nukeRenderCache = TRUE;
			computeEdges = TRUE;
			addSprings = FALSE;
			removeSprings = FALSE;
			simpleSoft = FALSE;

			}
		~LagModData()
			{
			SelfNode = NULL;
			}	
 		LocalModData*	Clone();                     	


	};


class ILagMod;


// block IDs
enum { lag_params };
// lag_param param IDs
enum { lag_flex, lag_strength, lag_sway, lag_referenceframe, lag_paint_strength,
		lag_paint_radius,lag_paint_feather,lag_paint_backface, lag_force_node, lag_absolute,
		lag_solver, lag_samples,
		lag_chase, lag_center,
		lag_endframeon, lag_endframe,
		lag_collider_node,
		lag_stretch_str, lag_stretch_sway,
		lag_torque_str, lag_torque_sway,
		lag_extra_str, lag_extra_sway,
		lag_hold_radius,
		lag_add_mode,
		lag_displaysprings,
		lag_holdlength,
		lag_holdlengthpercent,
		lag_lazyeval,
		lag_stretch,
		lag_stiffness,
		lag_enable_advance_springs,
		lag_springcolors,
		lag_customspringdisplay,
		lag_affectall,

		lag_createspringdepth,
		lag_createspringmult,
		


 };



//***************************************************************
//Function Publishing System stuff   
//****************************************************************
#define LAG_INTERFACE Interface_ID(0xDE17A34f, 0x8A41E3C1)

#define GetILagInterface(cd) \
			(ILagMod *)(cd)->GetInterface(LAG_INTERFACE)

enum {  lag_paint, lag_setreference, lag_reset,
		lag_addforce, lag_removeforce, 
		lag_numbervertices,
		lag_selectvertices,lag_getselectedvertices,
		lag_getvertexweight,lag_setvertexweight,
		lag_setedgelist,lag_getedgelist,
		lag_addspringselection,
		lag_addspring,
		lag_removeallsprings,
		lag_addspring_button,
		lag_removespring_button,
		lag_option_button,
		lag_simplesoft_button,

		lag_removespring_by_end,
		lag_removespring_by_both_ends,
		lag_removespringbyindex,
		lag_numbersprings,
		lag_getspringgroup,
		lag_setspringgroup,

		lag_getspringlength,
		lag_setspringlength,
		lag_getindex


		};
//****************************************************************


class ILagMod : public FPMixinInterface 
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP

			VFN_0(lag_paint, fnPaint);
			VFN_0(lag_setreference, fnSetReference);
			VFN_0(lag_reset, fnReset);
			VFN_1(lag_addforce, fnAddForce,TYPE_INODE);
			VFN_1(lag_removeforce, fnRemoveForce,TYPE_INT);
			FN_0(lag_numbervertices,TYPE_INT, fnNumberVertices);

			VFN_2(lag_selectvertices, fnSelectVertices, TYPE_BITARRAY, TYPE_BOOL);
			FN_0(lag_getselectedvertices, TYPE_BITARRAY, fnGetSelectedVertices);

			FN_1(lag_getvertexweight, TYPE_FLOAT, fnGetVertexWeight, TYPE_INT);
			VFN_2(lag_setvertexweight, fnSetVertexWeight, TYPE_INT_TAB,TYPE_FLOAT_TAB);

			VFN_2(lag_setedgelist, fnSetEdgeList, TYPE_BITARRAY, TYPE_BOOL);
			FN_0(lag_getedgelist, TYPE_BITARRAY, fnGetEdgeList);

			VFN_2(lag_addspringselection, fnAddSingleSpringFromSelection,TYPE_INT,TYPE_BOOL);
			VFN_4(lag_addspring, fnAddSpring,TYPE_INT,TYPE_INT,TYPE_INT,TYPE_BOOL);
			VFN_0(lag_removeallsprings, fnRemoveAllSprings);

			VFN_0(lag_addspring_button, fnAddSpringButton);
			VFN_0(lag_removespring_button, fnRemoveSpringButton);
			VFN_0(lag_option_button, fnOptionButton);
			VFN_0(lag_simplesoft_button, fnSimpleSoftButton);

			VFN_1(lag_removespring_by_end,fnRemoveSpring,TYPE_INT);
			VFN_2(lag_removespring_by_both_ends,fnRemoveSpring,TYPE_INT,TYPE_INT);
			VFN_1(lag_removespringbyindex,fnRemoveSpringByIndex,TYPE_INT);

			FN_0(lag_numbersprings,TYPE_INT,fnNumberSprings);
			FN_1(lag_getspringgroup,TYPE_FLOAT,fnGetSpringGroup,TYPE_INT);
			VFN_2(lag_setspringgroup,fnSetSpringGroup,TYPE_INT,TYPE_INT);

			FN_1(lag_getspringlength,TYPE_FLOAT,fnGetSpringLength,TYPE_INT);
			VFN_2(lag_setspringlength,fnSetSpringLength,TYPE_INT,TYPE_FLOAT);

			FN_2(lag_getindex,TYPE_INT,fnGetIndex,TYPE_INT,TYPE_INT);


		END_FUNCTION_MAP


		FPInterfaceDesc* GetDesc();    // <-- must implement 
//note functions that start with fn are to be used with maxscript since these expect 1 based indices
		BitArray		tempBitArray;
		virtual void	fnPaint()=0;
		virtual void	fnSetReference()=0;
		virtual void	fnReset()=0;
		virtual void	fnAddForce(INode *node)=0;
		virtual void	fnRemoveForce(int whichNode)=0;
		virtual int		fnNumberVertices()=0;
		virtual void	fnSelectVertices(BitArray *selList, BOOL updateViews)=0;
		virtual BitArray *fnGetSelectedVertices()=0;

		virtual float	fnGetVertexWeight(int index)=0;
		virtual void	fnSetVertexWeight(Tab<int> *indexList, Tab<float> *values)=0;

		virtual void	fnSetEdgeList(BitArray *selList, BOOL updateViews)=0;
		virtual BitArray *fnGetEdgeList()=0;

		virtual void	fnAddSingleSpringFromSelection(int flag,BOOL addDupes)=0;
		virtual void	AddSingleSpringFromSelection(LagModData *lmd, int flag,BOOL addDupes)=0;

		virtual void	fnAddSpring(int a, int b, int flag,BOOL addDupes)=0;
		virtual void	AddSpring(LagModData *lmd, int a, int b, int flag,BOOL addDupes)=0;


		virtual void	fnRemoveAllSprings()=0;
		virtual void	RemoveAllSprings(LagModData *lmd)=0;
		
/*		virtual void	fnDeleteSpring(int a, int b)=0;
		virtual void	fnDeleteSpring(int index)=0;
		virtual void	DeleteSpring(LagModData *lmd, int a, int b)=0;
		virtual void	DeleteSpring(LagModData *lmd, int index)=0;

		virtual void	fnSetSpringFlag(int index, int flag)=0;
		virtual void	SetSpringFlag(LagModData *lmd,int index, int flag)=0;
		virtual int		fnGetSpringFlag(int index)=0;
		virtual int		GetSpringFlag(LagModData *lmd,int index)=0;
*/

		virtual void	fnAddSpringButton()=0;
		virtual void	fnRemoveSpringButton()=0;
		virtual void	fnOptionButton()=0;
		virtual void	fnSimpleSoftButton()=0;

		virtual void	fnRemoveSpring(int a) = 0;
		virtual void	RemoveSpring(LagModData *lmd,int a) = 0;
		virtual void	fnRemoveSpring(int a,int b) = 0;
		virtual void	RemoveSpring(LagModData *lmd,int a,int b)=0;
		virtual void	fnRemoveSpringByIndex(int index) = 0;
		virtual void	RemoveSpringByIndex(LagModData *lmd,int index) = 0;

		virtual int		fnNumberSprings()=0;
		virtual int		NumberSprings(LagModData *lmd)=0;

		virtual int		fnGetSpringGroup(int index) = 0;
		virtual int		GetSpringGroup(LagModData *lmd,int index)=0;
		virtual void	fnSetSpringGroup(int index, int group)=0;
		virtual void	SetSpringGroup(LagModData *lmd,int index, int group)=0;

		virtual float	fnGetSpringLength(int index)=0;
		virtual float	GetSpringLength(LagModData *lmd,int index) = 0;
		virtual void	fnSetSpringLength(int index,float dist)=0;
		virtual void	SetSpringLength(LagModData *lmd,int index,float dist) = 0;

		virtual int		fnGetIndex(int a, int b)=0;
		virtual int		GetIndex(LagModData *lmd,int a, int b)=0;
	};





