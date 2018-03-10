#pragma once

#include "iFnPub.h"
#include "matrix3.h"

//***************************************************************
//Function Publishing System stuff   
//****************************************************************
#define MORPHBYBONE_INTERFACE Interface_ID(0xDE17A34f, 0x8B52E4E3)


enum {  morphbybone_addbone,
		morphbybone_removebone,
		morphbybone_selectbone,
		morphbybone_getselectedbone,
		morphbybone_getselectedmorph,
		morphbybone_selectvertices,
		morphbybone_isselectedvertex,
		morphbybone_resetgraph,
		morphbybone_grow,morphbybone_shrink,
		morphbybone_ring,morphbybone_loop,

		morphbybone_createmorph,
		morphbybone_removemorph,
		morphbybone_editmorph,

		morphbybone_clearvertices,
		morphbybone_deletevertices,
		morphbybone_resetorientation,

		morphbybone_reloadtarget,
		morphbybone_mirrorpaste,
		morphbybone_editfalloffgraph,
		
		morphbybone_setexternalnode,

		morphbybone_moveverts,
		morphbybone_transformverts,

		morphbybone_numberofbones,

		morphbybone_bonegetinitialnodetm,
		morphbybone_bonesetinitialnodetm,

		morphbybone_bonegetinitialobjecttm,
		morphbybone_bonesetinitialobjecttm,
		
		morphbybone_bonegetinitialparenttm,
		morphbybone_bonesetinitialparenttm,
		
		morphbybone_bonegetnumberofmorphs,

		morphbybone_bonegetmorphname,
		morphbybone_bonesetmorphname,

		morphbybone_bonegetmorphangle,
		morphbybone_bonesetmorphangle,

		morphbybone_bonegetmorphtm,
		morphbybone_bonesetmorphtm,

		morphbybone_bonegetmorphparenttm,
		morphbybone_bonesetmorphparenttm,

		morphbybone_bonegetmorphisdead,
		morphbybone_bonegetmorphsetdead,

		morphbybone_bonegetmorphnumpoints,
		morphbybone_bonesetmorphnumpoints,

		morphbybone_bonegetmorphvertid,
		morphbybone_bonesetmorphvertid,

		morphbybone_bonegetmorphvec,
		morphbybone_bonesetmorphvec,

		morphbybone_bonegetmorphpvec,
		morphbybone_bonesetmorphpvec,

		morphbybone_bonegetmorphop,
		morphbybone_bonesetmorphop,

		morphbybone_bonegetmorphowner,
		morphbybone_bonesetmorphowner,

		morphbybone_bonegetmorphfalloff,
		morphbybone_bonesetmorphfalloff,

		morphbybone_bonegetjointtype,
		morphbybone_bonesetjointtype,

		morphbybone_update,

		morphbybone_getweight,

		morphbybone_bonegetmorphenabled,
		morphbybone_bonesetmorphenabled,

		};
//****************************************************************

/*! \par Description:
	This is the interface into the Skin Morph modifier. 
	It contains methods to get access to the Skin Morph UI actions and the data in the Skin Morph modifier.
*/
class IMorphByBone : public FPMixinInterface 
{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP

			VFN_1(morphbybone_addbone, fnAddBone, TYPE_INODE);
			VFN_1(morphbybone_removebone, fnRemoveBone, TYPE_INODE);
			VFN_2(morphbybone_selectbone, fnSelectBone, TYPE_INODE,TYPE_STRING);
			FN_0(morphbybone_getselectedbone, TYPE_INODE, fnGetSelectedBone);
			FN_0(morphbybone_getselectedmorph, TYPE_STRING, fnGetSelectedMorph);
			VFN_2(morphbybone_selectvertices, fnSelectVertices, TYPE_INODE,TYPE_BITARRAY);
			FN_2(morphbybone_isselectedvertex,TYPE_BOOL, fnIsSelectedVertex, TYPE_INODE,TYPE_INT);

			VFN_0(morphbybone_resetgraph, fnResetGraph);
			VFN_0(morphbybone_grow, fnGrow);
			VFN_0(morphbybone_shrink, fnShrink);
			VFN_0(morphbybone_ring, fnRing);
			VFN_0(morphbybone_loop, fnLoop);

			VFN_1(morphbybone_createmorph, fnCreateMorph, TYPE_INODE);
			VFN_2(morphbybone_removemorph, fnRemoveMorph, TYPE_INODE,TYPE_STRING);

			VFN_1(morphbybone_editmorph, fnEdit,TYPE_BOOL);

			VFN_0(morphbybone_clearvertices, fnClearSelectedVertices);
			VFN_0(morphbybone_deletevertices, fnDeleteSelectedVertices);

			VFN_2(morphbybone_resetorientation, fnResetOrientation, TYPE_INODE,TYPE_STRING);
			VFN_2(morphbybone_reloadtarget, fnReloadTarget, TYPE_INODE,TYPE_STRING);
			VFN_1(morphbybone_mirrorpaste, fnMirrorPaste, TYPE_INODE);
			
			VFN_2(morphbybone_editfalloffgraph, fnEditFalloffGraph, TYPE_INODE,TYPE_STRING);

			VFN_3(morphbybone_setexternalnode, fnSetExternalNode, TYPE_INODE,TYPE_STRING,TYPE_INODE);

			VFN_1(morphbybone_moveverts, fnMoveVerts, TYPE_POINT3);

			VFN_2(morphbybone_transformverts, fnTransFormVerts, TYPE_MATRIX3, TYPE_MATRIX3);

			FN_0(morphbybone_numberofbones, TYPE_INT, fnNumberOfBones);

			FN_1(morphbybone_bonegetinitialnodetm, TYPE_MATRIX3, fnBoneGetInitialNodeTM, TYPE_INODE);
			VFN_2(morphbybone_bonesetinitialnodetm, fnBoneSetInitialNodeTM, TYPE_INODE, TYPE_MATRIX3);

			FN_1(morphbybone_bonegetinitialobjecttm, TYPE_MATRIX3, fnBoneGetInitialObjectTM, TYPE_INODE);
			VFN_2(morphbybone_bonesetinitialobjecttm, fnBoneSetInitialObjectTM, TYPE_INODE, TYPE_MATRIX3);

			FN_1(morphbybone_bonegetinitialparenttm, TYPE_MATRIX3, fnBoneGetInitialParentTM, TYPE_INODE);
			VFN_2(morphbybone_bonesetinitialparenttm, fnBoneSetInitialParentTM, TYPE_INODE, TYPE_MATRIX3);

			FN_1(morphbybone_bonegetnumberofmorphs, TYPE_INT, fnBoneGetNumberOfMorphs,TYPE_INODE);

			FN_2(morphbybone_bonegetmorphname, TYPE_STRING, fnBoneGetMorphName,TYPE_INODE,TYPE_INT);
			VFN_3(morphbybone_bonesetmorphname, fnBoneSetMorphName,TYPE_INODE,TYPE_INT,TYPE_STRING);

			FN_2(morphbybone_bonegetmorphangle, TYPE_FLOAT, fnBoneGetMorphAngle,TYPE_INODE,TYPE_INT);
			VFN_3(morphbybone_bonesetmorphangle, fnBoneSetMorphAngle,TYPE_INODE,TYPE_INT,TYPE_FLOAT);

			FN_2(morphbybone_bonegetmorphtm, TYPE_MATRIX3, fnBoneGetMorphTM,TYPE_INODE,TYPE_INT);
			VFN_3(morphbybone_bonesetmorphtm, fnBoneSetMorphTM,TYPE_INODE,TYPE_INT,TYPE_MATRIX3);

			FN_2(morphbybone_bonegetmorphparenttm, TYPE_MATRIX3, fnBoneGetMorphParentTM,TYPE_INODE,TYPE_INT);
			VFN_3(morphbybone_bonesetmorphparenttm, fnBoneSetMorphParentTM,TYPE_INODE,TYPE_INT,TYPE_MATRIX3);

			FN_2(morphbybone_bonegetmorphisdead, TYPE_BOOL, fnBoneGetMorphIsDead,TYPE_INODE,TYPE_INT);
			VFN_3(morphbybone_bonegetmorphsetdead, fnBoneSetMorphSetDead,TYPE_INODE,TYPE_INT,TYPE_BOOL);

			FN_2 (morphbybone_bonegetmorphnumpoints, TYPE_INT, fnBoneGetMorphNumPoints,TYPE_INODE,TYPE_INT);
			VFN_3(morphbybone_bonesetmorphnumpoints, fnBoneSetMorphNumPoints,TYPE_INODE,TYPE_INT,TYPE_INT);


			FN_3 (morphbybone_bonegetmorphvertid, TYPE_INT, fnBoneGetMorphVertID,TYPE_INODE,TYPE_INT,TYPE_INT);
			VFN_4(morphbybone_bonesetmorphvertid, fnBoneSetMorphVertID,TYPE_INODE,TYPE_INT,TYPE_INT,TYPE_INT);


			FN_3 (morphbybone_bonegetmorphvec, TYPE_POINT3, fnBoneGetMorphVec,TYPE_INODE,TYPE_INT,TYPE_INT);
			VFN_4(morphbybone_bonesetmorphvec, fnBoneSetMorphVec,TYPE_INODE,TYPE_INT,TYPE_INT,TYPE_POINT3);

			FN_3 (morphbybone_bonegetmorphpvec, TYPE_POINT3, fnBoneGetMorphPVec,TYPE_INODE,TYPE_INT,TYPE_INT);
			VFN_4(morphbybone_bonesetmorphpvec, fnBoneSetMorphPVec,TYPE_INODE,TYPE_INT,TYPE_INT,TYPE_POINT3);


			FN_3 (morphbybone_bonegetmorphop, TYPE_POINT3, fnBoneGetMorphOP,TYPE_INODE,TYPE_INT,TYPE_INT);
			VFN_4(morphbybone_bonesetmorphop, fnBoneSetMorphOP,TYPE_INODE,TYPE_INT,TYPE_INT,TYPE_POINT3);


			FN_3 (morphbybone_bonegetmorphowner, TYPE_INODE, fnBoneGetMorphOwner,TYPE_INODE,TYPE_INT,TYPE_INT);
			VFN_4(morphbybone_bonesetmorphowner, fnBoneSetMorphOwner,TYPE_INODE,TYPE_INT,TYPE_INT,TYPE_INODE);

			FN_2 (morphbybone_bonegetmorphfalloff, TYPE_INT, fnBoneGetMorphFalloff,TYPE_INODE,TYPE_INT);
			VFN_3(morphbybone_bonesetmorphfalloff, fnBoneSetMorphFalloff,TYPE_INODE,TYPE_INT,TYPE_INT);

			FN_1 (morphbybone_bonegetjointtype, TYPE_INT, fnBoneGetJointType,TYPE_INODE);
			VFN_2(morphbybone_bonesetjointtype, fnBoneSetJointType,TYPE_INODE,TYPE_INT);

			VFN_0(morphbybone_update, fnUpdate);

			FN_2(morphbybone_getweight, TYPE_FLOAT, fnGetWeight,TYPE_INODE,TYPE_STRING);
			

			FN_2 (morphbybone_bonegetmorphenabled, TYPE_BOOL, fnBoneGetMorphEnabled,TYPE_INODE,TYPE_INT);
			VFN_3(morphbybone_bonesetmorphenabled, fnBoneSetMorphEnabled,TYPE_INODE,TYPE_INT,TYPE_BOOL);


		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();
		
		//note functions that start with fn are to be used with maxscript since these expect 1 based indices
		
		/// void fnAddBone(INode *node)=0;
		/// This adds a bone to the system
		/// INode *node the bone to add
		virtual void	fnAddBone(INode *node)=0;

		/// void fnRemoveBone(INode *node)
		/// This removes a bone from the system
		/// INode *node the bone to remove
		virtual void	fnRemoveBone(INode *node)=0;

		/// void fnSelectBone(INode *node,MCHAR* morphName)
		/// This selects a bone and/or morph in the selection list.  If the morphName is null
		/// only the bone will be selected.
		/// INode *node this is the node to be selected
		/// MCHAR* morphName this is the morph to be selected
		virtual void	fnSelectBone(INode *node,MCHAR* morphName)=0;

		/// INode* fnGetSelectedBone()
		/// This returns the current active bone
		virtual INode*	fnGetSelectedBone()=0;

		/// MCHAR* fnGetSelectedMorph()
		/// This returns the name of the current selected morph
		virtual MCHAR*	fnGetSelectedMorph()=0;
		
		/// void fnSelectVertices(INode *node,BitArray *sel)
		/// This lets you select vertices.
		/// INode *node this is the node that owns the modifier, this is used to find a specific
		///				instance of the modifier and apply it to the right local data
		/// BitArray *sel the selection array
		virtual void	fnSelectVertices(INode *node,BitArray *sel)=0;
		/// BOOL fnIsSelectedVertex(INode *node,int index)
		/// INode *node this is the node that owns the modifier, this is used to find a specific
		///				instance of the modifier and apply it to the right local data
		/// int index this the index of the vertex that you want to check
		virtual BOOL	fnIsSelectedVertex(INode *node,int index)=0;
		
		/// void	fnResetGraph()
		/// this is equivalent to pressing the reset graph button in the UI which 
		/// resets the the soft selection graph to a sinual graph
		virtual void	fnResetGraph()=0;
		/// void	fnShrink()
		/// This contracts your vertex selection
		virtual void	fnShrink()=0;
		/// void	fnGrow()
		/// This expands your vertex selection
		virtual void	fnGrow()=0;
		/// void	fnRing()
		/// This turns your current selection into a ring selection
		virtual void	fnRing()=0;
		/// void	fnLoop()
		/// This turns your current selection into a loop selection		
		virtual void	fnLoop()=0;

		/// void	fnCreateMorph(INode *node)
		/// This creates a new morph for a bone
		/// INode *node this is the bone that you want to create the morph on
		virtual void	fnCreateMorph(INode *node)=0;

		/// void	fnRemoveMorph(INode *node, MCHAR *name)
		/// This removes a morph target
		/// INode *node this is the bone that owns the morph that you want to remove
		/// MCHAR *name this is the name of the morph that you want to remove
		virtual void	fnRemoveMorph(INode *node, MCHAR *name)=0;

		/// fnEdit(BOOL edit)
		/// This toggles on/off the edit morph mode.  The current active morph is the one
		/// that will be edited
		/// BOOL edit the edit mode state
		virtual void	fnEdit(BOOL edit)=0;

		/// void	fnClearSelectedVertices()
		/// This ie equivalent to press the clear verts button in the UI which takes the
		/// selected vertices and sets their delta to zero
		virtual void	fnClearSelectedVertices()=0;
		/// void	fnDeleteSelectedVertices()
		/// This ie equivalent to press the delet verts button in the UI which takes the
		/// selected vertices and removes them from the current morph
		virtual void	fnDeleteSelectedVertices()=0;

		/// void	fnResetOrientation(INode *node, MCHAR *name)
		/// This resets the orientation of morph to the current orientation of the bone that owns it
		/// INode *node this is the bone that owns the morph to be reset
		/// MCHAR *name this is the name of the morph to be reset
		virtual void	fnResetOrientation(INode *node, MCHAR *name)=0;
		/// void	fnReloadTarget(INode *node, MCHAR *name)
		/// This reloads the external mesh for a morph if there is one
		/// INode *node this is the bone that owns the morph to be reloaded
		/// MCHAR *name this is the name of the morph to be reloaded
		virtual void	fnReloadTarget(INode *node, MCHAR *name)=0;

		/// fnMirrorPaste(INode *node)
		/// This mirror pastes the node to its mirror target based on the mirror parameters
		/// INode *node this is the node that is to be mirrored
		virtual void	fnMirrorPaste(INode *node)=0;

		/// void	fnEditFalloffGraph(INode *node, MCHAR *name)
		/// This brings up the falloff graph of a morph if there is one
		/// INode *node this is the bone that owns the morph 
		/// MCHAR *name this is the name of the morph to to have its graph brought up
		virtual void	fnEditFalloffGraph(INode *node, MCHAR *name)=0;
		/// void	fnSetExternalNode(INode *node, MCHAR *name,INode *exnode)
		/// This lets you set the external mesh node for a morph
		/// INode *node this is the bone that owns the morph 
		/// MCHAR *name this is the name of the morph to to have its external node set
		virtual void	fnSetExternalNode(INode *node, MCHAR *name,INode *exnode)=0;
		
		/// void	fnMoveVerts(Point3 vec)
		/// This moves the selected vertices along a vec which is applied to an active morph if there is one
		/// Point3 vec this is the vec which the vertices are moved.
		virtual void	fnMoveVerts(Point3 vec)=0;
		/// void	fnTransFormVerts(Matrix3 a, Matrix3 b)
		/// This transforms the selected vertices which is applied to an active morph if there is one
		/// Matrix a this is matrix transformation
		/// Matrix b this is the space?
		virtual void	fnTransFormVerts(Matrix3 a, Matrix3 b)=0;

		/// int		fnNumberOfBones()
		/// This is the number of bones in the system
		virtual int		fnNumberOfBones() = 0;

		/// Matrix3*	fnBoneGetInitialNodeTM(INode *node)
		/// This returns the initial node tm of bone.   This is the tm of the bone
		/// when it is added to the system
		/// the initial node tm is used to determine the actual deformation
		/// INode *node this is the bone that you want get the tm from
		virtual Matrix3*	fnBoneGetInitialNodeTM(INode *node) = 0;
		/// void	fnBoneSetInitialNodeTM(INode *node, Matrix3 tm)
		/// This lets you set the initial node tm of a bone
		/// INode *node this is the bone that you want set the tm on
		/// Matrix3 tm this is the initial tm
		virtual void	fnBoneSetInitialNodeTM(INode *node, Matrix3 tm) = 0;


		/// Matrix3*	fnBoneGetInitialObjectTM(INode *node)
		/// This returns the initial object tm of bone.   This is the tm of the bone
		/// when it is added to the system
		/// the initial object tm is used only for display
		/// INode *node this is the bone that you want get the tm from
		virtual Matrix3*	fnBoneGetInitialObjectTM(INode *node) = 0;
		/// void	fnBoneSetInitialObjectTM(INode *node, Matrix3 tm)
		/// This lets you set the initial object tm of a bone
		/// INode *node this is the bone that you want set the tm on
		/// Matrix3 tm this is the initial tm
		virtual void	fnBoneSetInitialObjectTM(INode *node, Matrix3 tm) = 0;


		/// Matrix3*	fnBoneGetInitialParentTM(INode *node)
		/// This returns the initial node tm parent that owns bone.   
		/// INode *node this is the bone that you want get the parent tm from
		virtual Matrix3*	fnBoneGetInitialParentTM(INode *node) = 0;
		/// void	fnBoneSetInitialParentTM(INode *node, Matrix3 tm)
		/// This lets you set the parent tm of a bone.  
		/// INode *node this is the bone that you want set the tm on
		/// Matrix3 tm this is the parent tm
		virtual void	fnBoneSetInitialParentTM(INode *node, Matrix3 tm) = 0;

		/// int		fnBoneGetNumberOfMorphs(INode *node)
		/// This returns the number of morphs that a bone owns
		/// INode *node this the bone to get the number of morphs from
		virtual int		fnBoneGetNumberOfMorphs(INode *node) = 0;

		/// the follow le get/set properties of individual morphs they all follow the same format
		/// INode *node is the bone that owns the morph
		/// int morphIndex is the index of the morph 
		virtual MCHAR*	fnBoneGetMorphName(INode *node, int morphIndex) = 0;
		virtual void	fnBoneSetMorphName(INode *node, int morphIndex,MCHAR* name) = 0;

		virtual float	fnBoneGetMorphAngle(INode *node, int morphIndex) = 0;
		virtual void	fnBoneSetMorphAngle(INode *node, int morphIndex,float angle) = 0;


		virtual Matrix3* fnBoneGetMorphTM(INode *node, int morphIndex) = 0;
		virtual void	 fnBoneSetMorphTM(INode *node, int morphIndex,Matrix3 tm) = 0;

		virtual Matrix3* fnBoneGetMorphParentTM(INode *node, int morphIndex) = 0;
		virtual void	 fnBoneSetMorphParentTM(INode *node, int morphIndex,Matrix3 tm) = 0;

		virtual BOOL	 fnBoneGetMorphIsDead(INode *node, int morphIndex) = 0;
		virtual void	 fnBoneSetMorphSetDead(INode *node, int morphIndex,BOOL dead) = 0;

		virtual int		 fnBoneGetMorphNumPoints(INode *node, int morphIndex) = 0;
		virtual void	 fnBoneSetMorphNumPoints(INode *node, int morphIndex, int numberPoints) = 0;

		///These let you get access to the morph vertex properties
		///every morph contains a list of vertices and properties
		/// INode *node is the bone that owns the morph
		/// int morphIndex is the index of the morph 
		/// int ithIndex is which vertex you want to get info about
		/// the index back into the mesh vertex list
		virtual int		 fnBoneGetMorphVertID(INode *node, int morphIndex, int ithIndex) = 0;
		virtual void	 fnBoneSetMorphVertID(INode *node, int morphIndex, int ithIndex, int vertIndex) = 0;

		/// the vector in local bone space
		virtual Point3*	 fnBoneGetMorphVec(INode *node, int morphIndex, int ithIndex) = 0;
		virtual void	 fnBoneSetMorphVec(INode *node, int morphIndex, int ithIndex, Point3 vec) = 0;

		/// the vector in parent space
		virtual Point3*	 fnBoneGetMorphPVec(INode *node, int morphIndex, int ithIndex) = 0;
		virtual void	 fnBoneSetMorphPVec(INode *node, int morphIndex, int ithIndex, Point3 vec) = 0;

		/// the original point before any deformation
		virtual Point3*	 fnBoneGetMorphOP(INode *node, int morphIndex, int ithIndex) = 0;
		virtual void	 fnBoneSetMorphOP(INode *node, int morphIndex, int ithIndex, Point3 vec) = 0;

		/// this returns which node owns the vertex if ther modifier is instanced
		virtual INode*	 fnBoneGetMorphOwner(INode *node, int morphIndex, int ithIndex) = 0;
		virtual void	 fnBoneSetMorphOwner(INode *node, int morphIndex, int ithIndex, INode *onode) = 0;

		virtual int		 fnBoneGetMorphFalloff(INode *node, int morphIndex) = 0;
		virtual void	 fnBoneSetMorphFalloff(INode *node, int morphIndex, int falloff) = 0;


		virtual int		 fnBoneGetJointType(INode *node) = 0;
		virtual void	 fnBoneSetJointType(INode *node, int jointType) = 0;

		/// void	 fnUpdate()
		/// This forces a system to update
		virtual void	 fnUpdate() = 0;

		/// float	 fnGetWeight(INode *node, MCHAR*name)
		/// This return the weight of particular morph is contributing
		/// INode *node is the bone that owns the morph
		/// MCHAR*name this is the name of the morph
		virtual float	 fnGetWeight(INode *node, MCHAR*name) = 0;

		virtual BOOL	 fnBoneGetMorphEnabled(INode *node, int morphIndex) = 0;
		virtual void	 fnBoneSetMorphEnabled(INode *node, int morphIndex, BOOL enabled) = 0;


	};





