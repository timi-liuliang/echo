 /**********************************************************************
 
	FILE: ISkin.h

	DESCRIPTION:  Skin Bone Deformer API

	CREATED BY: Nikolai Sander, Discreet

	HISTORY: 7/12/99


 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#pragma once
#include <WTypes.h>
#include "maxheap.h"
#include "ISkinCodes.h"
#include "point3.h"
#include "strbasic.h"
#include "tab.h"
#include "maxtypes.h"
#include "ref.h"
#include "iparamb2.h"
#include "matrix3.h"

// forward declarations
class INode;
class Matrix3;
class BitArray;
class ViewExp;
class Box3;
class ModContext;
class GraphicsWindow;
class HitRecord;
class SubObjAxisCallback;

#define I_SKIN 0x00010000
#define I_SKINIMPORTDATA 0x00020000

//New interface for max 6
#define I_SKIN2 0x00030000

#define I_GIZMO 9815854
//Gizmo interface for r5 additions
#define I_GIZMO2 9815855
//Gizmo interface for r5.1 additions
#define I_GIZMO3 9815856

#define SKIN_INVALID_NODE_PTR 0
#define SKIN_OK				  1

//#define SKIN_CLASSID Class_ID(0x68477bb4, 0x28cf6b86)
#define SKIN_CLASSID Class_ID(9815843,87654)

#pragma warning(push)
#pragma warning(disable:4100)


/*! \sa  Class ISkin\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface that allows you to get at the local
modifier data for skin. The data is stored in the form of vertex weight arrays.
Each vertex has an array of boneID's and weights which affect that particular
vertex.  */
class ISkinContextData: public MaxHeapOperators
{
public:
	/*! \remarks Destructor. */
	virtual ~ISkinContextData() {}
	/*! \remarks This method returns the number of points that skin
	affects.  */
	virtual int GetNumPoints()=0;
	/*! \remarks This method returns the number of bones that control this
	vertex.
	\par Parameters:
	<b>int vertexIdx</b>\n\n
	The index into the vertex array. */
	virtual int GetNumAssignedBones(int vertexIdx)=0;
	/*! \remarks This method returns the index of the bone that affecting the
	vertex
	\par Parameters:
	<b>int vertexIdx</b>\n\n
	The index into the vertex array.\n\n
	<b>int boneIdx</b>\n\n
	The index into the affected bone array for that vertex. */
	virtual int GetAssignedBone(int vertexIdx, int boneIdx)=0;
	/*! \remarks This method returns the weight that is affecting the vertex
	\par Parameters:
	<b>int vertexIdx</b>\n\n
	The index into the vertex array.\n\n
	<b>int boneIdx</b>\n\n
	The index into the affected bone array for that vertex. */
	virtual float GetBoneWeight(int vertexIdx, int boneIdx)=0;
	
	// These are only used for Spline animation
	/*! \remarks This method currently returns 0 since the only the first
	curve is used in for spline animation.
	\par Parameters:
	<b>int vertexIdx</b>\n\n
	The index into the vertex array.\n\n
	<b>int boneIdx</b>\n\n
	The index into the affected bone array for that vertex. */
	virtual int GetSubCurveIndex(int vertexIdx, int boneIdx)=0;
	/*! \remarks This method returns the segment that of the spline that is
	affecting the vertex.
	\par Parameters:
	<b>int vertexIdx</b>\n\n
	The index into the vertex array.\n\n
	<b>int boneIdx</b>\n\n
	The index into the affected bone array for that vertex. */
	virtual int GetSubSegmentIndex(int vertexIdx, int boneIdx)=0;
	/*! \remarks This method returns how far away the point was initially from
	the curve.
	\par Parameters:
	<b>int vertexIdx</b>\n\n
	The index into the vertex array.\n\n
	<b>int boneIdx</b>\n\n
	The index into the affected bone array for that vertex. */
	virtual float GetSubSegmentDistance(int vertexIdx, int boneIdx)=0;
	/*! \remarks This method returns closest initial tangent to the vertex.
	\par Parameters:
	<b>int vertexIdx</b>\n\n
	The index into the vertex array.\n\n
	<b>int boneIdx</b>\n\n
	The index into the affected bone array for that vertex. */
	virtual Point3 GetTangent(int vertexIdx, int boneIdx)=0;
	/*! \remarks This method returns closest initial point on the curve to the
	vertex.
	\par Parameters:
	<b>int vertexIdx</b>\n\n
	The index into the vertex array.\n\n
	<b>int boneIdx</b>\n\n
	The index into the affected bone array for that vertex. */
	virtual Point3 GetOPoint(int vertexIdx, int boneIdx)=0;

};

/*! \sa  Class ISkinContextData,  Class INode, Class Matrix3\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface for the Skin modifier.  */
class ISkin: public MaxHeapOperators 
{
public:
	/*! \remarks Destructor. */
	virtual ~ISkin() {}
	/*! \remarks This method retrieves the intial matrix of the bone when it
	was applied to skin.
	\par Parameters:
	<b>INode *pNode</b>\n\n
	The INode of the bone you want to get the TM from.\n\n
	<b>Matrix3 \&InitTM</b>\n\n
	This returns either the initial node TM or object TM of the bone when it
	was applied to skin.\n\n
	<b>bool bObjOffset = false</b>\n\n
	When TRUE the InitTM returns the object TM else the node TM.
	\return  <b>SKIN_OK</b> if the <b>pNode</b> was in the system else it
	returns <b>SKIN_INVALID_NODE_PTR.</b> */
	virtual int GetBoneInitTM(INode *pNode, Matrix3 &InitTM, bool bObjOffset = false)=0;
	/*! \remarks This method gets the initial matrix of the skinned object
	when it was applied to skin.
	\par Parameters:
	<b>INode *pNode</b>\n\n
	The INode of the bone you want to get the TM from.\n\n
	<b>Matrix3 \&InitTM</b>\n\n
	This returns either the initial skinned node tm or object tm of the bone
	when it was applied to skin.\n\n
	<b>bool bObjOffset = false</b>\n\n
	When TRUE the InitTM returns the object TM else the node TM.
	\return  <b>SKIN_OK</b> if the <b>pNode</b> was in the system else it
	returns <b>SKIN_INVALID_NODE_PTR.</b> */
	virtual int GetSkinInitTM(INode *pNode, Matrix3 &InitTM, bool bObjOffset = false)=0;
	/*! \remarks This method returns the actual number of non NULL bones in
	the system. */
	virtual int GetNumBones()=0;
	/*! \remarks This method returns the INode of a particular bone.
	\par Parameters:
	<b>int idx</b>\n\n
	The index of the bone that you want to get the INode from. Note there are 2
	tables that contain bone date. One has NULL entries and the other has all
	the invalid NULL bones removed. The index here points to the table that has
	the NULL entries removed. */
	virtual INode *GetBone(int idx)=0;
	/*! \remarks This method returns the flags of a particular bone.
	\par Parameters:
	<b>int idx</b>\n\n
	The index of the bone that you want to get the INode from. Note there are 2
	tables that contain bone data. One has NULL entries and the other has all
	the invalid NULL bones removed. The index here points to the table that has
	the NULL entries removed.
	\return  One of the following;\n\n
	<b>BONE_LOCK_FLAG</b>\n\n
	Not used.\n\n
	<b>BONE_ABSOLUTE_FLAG</b>\n\n
	The bone is in absolute mode (i.e. all the vertices weight must add up to
	1.0f).\n\n
	<b>BONE_SPLINE_FLAG</b>\n\n
	This bone is a spline.\n\n
	<b>BONE_SPLINECLOSED_FLAG</b>\n\n
	This bone is a closed spline.\n\n
	<b>BONE_DRAW_ENVELOPE_FLAG</b>\n\n
	Always draw this bones envelope.\n\n
	<b>BONE_BONE_FLAG</b>\n\n
	This bone is one of 3ds Max' bone objects..\n\n
	<b>BONE_DEAD_FLAG</b>\n\n
	This bone is no longer used. */
	virtual DWORD GetBoneProperty(int idx)=0;
	/*! \remarks This method returns the interface pointer to the local skin
	data for the node.
	\par Parameters:
	<b>INode *pNode</b>\n\n
	The node that you want to retrieve the skin interface to. */
	virtual ISkinContextData *GetContextInterface(INode *pNode)=0;
//new stuff
	/*! \remarks This method returns the name of the bone pointed to by index.
	\par Parameters:
	<b>int index</b>\n\n
	The index of the bone that you want to get the name of. Note there are 2
	tables that contain bone data. One has NULL entries and the other has all
	the invalid NULL bones removed. The index here points to the table that has
	the NULL entries in them. */
	virtual MCHAR *GetBoneName(int index) = 0;
	/*! \remarks This method returns the current selected bone. */
	virtual int GetSelectedBone() = 0;
	/*! \remarks This method forces the gizmo list box to update. */
	virtual void UpdateGizmoList() = 0;
	/*! \remarks This method retrieves the start and end points of the
	envelope in bone space.
	\par Parameters:
	<b>int id</b>\n\n
	The index of the bone that you want to get the INode from. Note there are 2
	tables that contain bone data. One has NULL entries and the other has all
	the invalid NULL bones removed. The index here points to the table that has
	the NULL entries\n\n
	<b>Point3 \&l1</b>\n\n
	The start point of the envelope in bone space.\n\n
	<b>Point3 \&l2</b>\n\n
	The end point of the envelope in bone space. */
	virtual void GetEndPoints(int id, Point3 &l1, Point3 &l2) = 0;
	/*! \remarks This method returns the bone TM of the specified bone.
	\par Parameters:
	<b>int id</b>\n\n
	The index of the bone. */
	virtual Matrix3 GetBoneTm(int id) = 0;
	/*! \remarks This method returns the INode of a particular bone.
	\par Parameters:
	<b>int idx</b>\n\n
	The index of the bone that you want to get the INode from. Note there are 2
	tables that contain bone data. One has NULL entries and the other has all
	the invalid NULL bones removed. The index here points to the table that has
	the NULL entries */
	virtual INode *GetBoneFlat(int idx)=0;
	/*! \remarks This method returns the actual number of NULL bones in the
	system including NULL entries. */
	virtual int GetNumBonesFlat()=0;
	/*! \remarks This returns the reference frame of the skin modifier. */
	virtual int GetRefFrame()=0;

};


// The following {class, member, macro, flag} has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.


/*!  \n\n
class ISkinImportData\n\n

\par Description:
This class is available in release 4.2 and later only.\n\n
The interface that allows to import data: bones, initial skin and bone
transforms, and vertex weighting into skin.\n\n
   */
class ISkinImportData: public MaxHeapOperators
{
public:

/*! \remarks Destructor. */
	virtual ~ISkinImportData() {}

/*** BOOL AddBoneEx(INode *node, BOOL update) ***/
/*
INode *node the bone to be added to skin
BOOL update is used to update the UI

 Adds a bone to the skin system.  Return TRUE if the operation succeeded.
*/
	/*! \remarks Adds a bone to the skin system\n\n

	\par Parameters:
	<b>INode *node</b>\n\n
	The bone to be added to skin.\n\n
	<b>BOOL update</b>\n\n
	The flag for UI update\n\n

	\return  TRUE if the operation succeeded.\n\n
	  */
	virtual BOOL AddBoneEx(INode *boneNode, BOOL update)=0;



/**** virtual BOOL SetSkinBaseTm(INode *skinNode, Matrix3 tm) ***/
/*
INode *boneNode is that node that skin is applied to, need this so I can extract the local mod data
Matrix3 objectTm is the object matrix to assign as the new skin object tm
Matrix3 nodeTm is the node matrix to assign as the new skin node tm

When skin is applied to a node, that nodes initial objecttm is stored off so we can recompute the initial position
of the skin object.  This function allows you to change that tm.  It will store tm and the inverse tm of the 
matrix passed to it.  Return TRUE if the operation succeeded.

Below is the actual code that computes this when skin is added (node is the node that skin is applied to)
				d->BaseTM = node->GetObjectTM(RefFrame);
				d->BaseNodeTM = node->GetNodeTM(RefFrame); //ns
				d->InverseBaseTM = Inverse(d->BaseTM);
*/
	/*! \remarks When skin is applied to a node, that nodes initial ObjectTM
	is stored off, so the initial position of the skin object can be
	recomputed. The function allows to set that TM. Stores TM and the inverse
	TM of the matrix passed to it<b>.</b>\n\n

	\par Parameters:
	<b>INode *skinNode</b>\n\n
	The node that Skin is applied to\n\n
	<b>Matrix3 objectTm</b>\n\n
	The object matrix to assign as the new Skin object TM.\n\n
	<b>Matrix3 nodeTm</b>\n\n
	The node matrix to assign as the new Skin node TM\n\n

	\return  TRUE if the operation succeeded.\n\n
	  */
	virtual BOOL SetSkinTm(INode *skinNode, Matrix3 objectTm, Matrix3 nodeTm)=0;


/**** virtual BOOL SetBoneTm(INode *boneNode, Matrix3 objectTm, Matrix3 nodeTm) ***/
/*
INode *boneNode is the pointer to the bone that is having its initial matrix change
Matrix3 objectTm is the object matrix to assign as the new skin object tm
Matrix3 nodeTm is the node matrix to assign as the new skin node tm

When a bone is added to skin it stores off the initial object and node tm of that bone.  This function allows 
you to change the intial matrix of a particular bone. Return TRUE if the operation succeeded.

Below is the actual code from skin that is called when a bone is added (t.node is the node of the bone being added)

			Matrix3 otm = t.Node->GetObjectTM(RefFrame);  //ns	
			Matrix3 ntm = t.Node->GetStretchTM(RefFrame) * t.Node->GetNodeTM(RefFrame);	
	
			BoneData[current].InitObjectTM = otm;		//ns
			BoneData[current].InitNodeTM = ntm;
			BoneData[current].tm    = Inverse(otm);
*/
	/*! \remarks When a bone is added to skin it stores off the initial object
	and node TM of that bone. The function sets the initial matrix of a
	particular bone.\n\n

	\par Parameters:
	<b>INode *boneNode</b>\n\n
	The pointer to the bone that is having its initial matrix change\n\n
	<b>Matrix3 objectTm</b>\n\n
	The object matrix to assign as the new Skin object TM.\n\n
	<b>Matrix3 nodeTm</b>\n\n
	The node matrix to assign as the new Skin node TM\n\n

	\return  TRUE if the operation succeeded.\n\n
	  */
	virtual BOOL SetBoneTm(INode *boneNode, Matrix3 objectTm, Matrix3 nodeTm)=0;



/*** BOOL AddWeights(INode *skinNode, int vertexID, Tab<INode*> &boneNodeList, Tab<float> &weights) ***/
/*
INode *skinNode  is the node that has skin applied to it, need this so I can get the local mod data for that node and skin since the same modifier can be applied to mulitple nodes thhrough instancing
int vertexID is the index of the vertex that you want to apply weights to
Tab<INode*> &boneNodeList is the list of bones that will control the vertex, these must already be added to the skin modifier
Tab<float> &weights is the weight of each bone
*/
	/*! \remarks Adds weight to a specific vertex. Replaces the current vertex
	weight data supplied. The weights should sum to 1.0f. Both boneNodelist and
	weights must be the same size .\n\n

	\par Parameters:
	<b>INode *skinNode</b>\n\n
	The node that has skin applied to it, used to obtain the local mod data for
	that node and skin since the same modifier can be applied to multiple nodes
	through instancing.\n\n
	<b>int vertexID</b>\n\n
	The index of the vertex to apply weights to.\n\n
	<b>Tab\<INode*\> \&boneNodeList</b>\n\n
	The list of bones that will control the vertex, these must already be added to
	the skin modifier.\n\n
	<b>Tab\<float\> \&weights</b>\n\n
	The weight of each bone.\n\n

	\return  TRUE if the operation succeeded. */
	virtual BOOL AddWeights(INode* skinNode, int vertexID, Tab<INode*>& boneNodeList, Tab<float>& weights)=0;
};


// End of 3ds max 4.2 Extension

//New For Max6
//this exposes the initial stretch matrix that a bone as since we seperated this out from the base
//matrix so we could turn it off.

/*!  \n\n
class ISkin2\n\n
\sa Class ISkin, Class INode\n\n
\par Description:
This class is available in release 6.0 and later only.\n\n
This interface provide access to the stretch matrix for bones. This update is provide from
a result of previous work on bones where stretch data was extracted from the base matrix.\n\n
Methods are available for both setting and getting. Also an additional method has been added 
that provides the ability to select vertices in the modifier. */
class ISkin2: public MaxHeapOperators
{
public:
	/*! \remarks Destructor. */
	virtual ~ISkin2() {}
/*! \remarks This will set the initial stretch tm for a bone in skin.
	The stretch matrix is the bone stretch matrix applied to bone objects. You can get a node stretch tm by calling
	INode::GetStretchTM()\n	
	NOTE SetBoneTm will clear the stretchTM and set it to identity so make sure you call this after SetBoneTm
	\param boneNode - the node of the bone to set
	\param stretchTm - the stretch matrix
	\return true if the function succeeds */
	virtual BOOL SetBoneStretchTm(INode *boneNode, Matrix3 stretchTm)=0;

/*!	\remarks This will return  the initial stretch tm for a bone in skin
	\param boneNode - The node of the bone to get the stretch TM off.
	\return The stretch matrix off boneNode */
	virtual Matrix3 GetBoneStretchTm(INode *boneNode)=0;

/*! \remarks This will let you get the current vertex selection set for skin
	\param skinNode - the node of the skinned object 
	\param[out] sel - The vertex selection set */
	virtual void GetVertexSelection(INode *skinNode, BitArray &sel) = 0;

/*! \remarks This will let you set the current vertex selection set for skin
	\param skinNode - the node of the skinned object 
	\param sel - The vertex selection set */
	virtual void SetVertexSelection(INode *skinNode, BitArray &sel) = 0;
};

/*! \sa  Class GizmoClass\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class is used to copy and paste gizmo buffers. It is passed to the Gizmo
Class which news and instance of its buffer and fills out it ClassID.
\par Data Members:
<b>Class_ID cid;</b>\n\n
The Class ID of the gizmo.  */
class IGizmoBuffer: public MaxHeapOperators
{
public:
	Class_ID cid;
	/*! \remarks Destructor. */
	virtual ~IGizmoBuffer() {}
	/*! \remarks Delete the instance.
	\par Default Implementation:
	<b>{ delete this; }</b> */
	virtual void DeleteThis()=0;

};


/*! \sa  Class ReferenceTarget,  Class ClassDesc2,  Class IgizmoBuffer,  Hit Test Types, Hit Test Flags, 
Class HitRecord,  Class SubObjAxisCallback,  Class Sub-Object Coordinate Systems\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is a simple reference class for plugin gizmos for the skin modifier. This
allows developers to create plugin deformers for skin. This class is of super
object type REFTARGET and must use the category <b>BonesDefGizmoDeformer</b>
for skin to detect it.\n\n
If the client of <b>GizmoClass</b> maintains several parameter blocks then the
client must implement the methods <b>NumSubs()</b>, <b>SubAnim(i)</b>,
<b>SubAnimName(i)</b>, <b>NumRefs()</b>, <b>GetReference(i)</b> and
<b>SetReference(i)</b> and call the <b>GizmoClass</b> methods when 'i' refers
to the parameters maintained by <b>GizmoClass</b>.
\par Data Members:
<b>Class_ID cid;</b>\n\n
The Class ID of the gizmo.\n\n
<b>ISkin *bonesMod;</b>\n\n
The pointer back to the skin modifier so the gizmo can ask for basic
information about the modifier.\n\n
<b>IParamBlock2 *pblock_gizmo_data;</b>\n\n
The param block provided for the gizmo to store its data.  */
class GizmoClass : public ReferenceTarget
{
// all the refernce stuff and paramblock stuff here
public:

	ISkin *bonesMod;
    IParamBlock2 *pblock_gizmo_data;

	GizmoClass() { pblock_gizmo_data = NULL; bonesMod = NULL; }
	/*! \remarks This method returns the number of parameter blocks.
	\par Default Implementation:
	<b>{ return 1; }</b> */
	int	NumParamBlocks() { return 1; }
	/*! \remarks This method return 'i-th' ParamBlock2 in this instance (or
	NULL if not available).
	\par Parameters:
	<b>int i</b>\n\n
	The zero based index of the ParamBlock2 to return. */
	IParamBlock2* GetParamBlock(int i)
				{
				if (i == 0) return pblock_gizmo_data;
				else return NULL;
				}
	/*! \remarks This method returns a pointer to the ParamBlock2 as specified
	by the ID passed (or NULL if not available).
	\par Parameters:
	<b>BlockID id</b>\n\n
	The BlockID of the ParamBlock2 instance. */
	IParamBlock2* GetParamBlockByID(BlockID id)
				{
				if (pblock_gizmo_data->ID() == id) return pblock_gizmo_data ;
				 else return  NULL; 
				 }

	int NumRefs() {return 1;}
	RefTargetHandle GetReference(int i)
		{
		if (i==0)
			{
			return (RefTargetHandle)pblock_gizmo_data;
			}
		return NULL;
		}

protected:
	virtual void SetReference(int i, RefTargetHandle rtarg)
		{
		if (i==0)
			{
			pblock_gizmo_data = (IParamBlock2*)rtarg;
			}
		}
public:
    /*! \remarks Self deletion. */
    void DeleteThis()=0;


	int NumSubs() {return 1;}
    Animatable* SubAnim(int i) { return GetReference(i);}
 

	MSTR SubAnimName(int i)	{return _M("");	}

	int SubNumToRefNum(int subNum) {return -1;}

	RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
		   PartID& partID, RefMessage message)	
		{
		return REF_SUCCEED;
		}

    virtual void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev) {}
    virtual void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next) {}         
    virtual IOResult Load(ILoad *iload) {return IO_OK;}
    virtual IOResult Save(ISave *isave) {return IO_OK;}

//	void* GetInterface(ULONG id);  

//this retrieves the boudng bx of the gizmo in world space
	virtual void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc){}               
// this called in the bonesdef display code to show the gizmo
	virtual int Display(TimeValue t, GraphicsWindow *gw, Matrix3 tm ) { return 1;}
	virtual Interval LocalValidity(TimeValue t) {return FOREVER;}
	/*! \remarks This method returns whether or not this gizmo is active or not.
	\par Default Implementation:
	<b>{ return TRUE; }</b> */
	virtual BOOL IsEnabled() { return TRUE; }
	/*! \remarks This method returns whether or not this gizmo is volume based or not. If it
	is volume based IsInVolume is used to check to see if a point is deformed
	otherwise a tab list of points is used to determine if a point is affected.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL IsVolumeBased() {return FALSE;}
	/*! \remarks If IsVolumBased returns TRUE his function is called to determine if the
	point is deformed.
	\par Parameters:
	<b>Point3 p</b>\n\n
	The point to be checked in the skinned objects local space.\n\n
	<b>Matrix3 tm</b>\n\n
	This matrix will transform the point into world space.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL IsInVolume(Point3 p, Matrix3 tm) { return FALSE;}

//this is what deforms the point
// this is passed in from the Map call in bones def
	/*! \remarks This method does the actual deformation of the points. Both initial and
	current point positions are passed in so the deformer has a chance to start
	from scratch minus the weighted transformation deformation. For instance a
	joint lattice deformer would want to work with the initial position while a
	bulge deformer would use the current point position.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time of the modification.\n\n
	<b>int index</b>\n\n
	Te index of the point into the object.\n\n
	<b>Point3 initialP</b>\n\n
	The initial position of the point.\n\n
	<b>Point3 p</b>\n\n
	The current deformed position of the point.\n\n
	<b>Matrix3 tm</b>\n\n
	The matrix to convert <b>p</b> and <b>initialP</b> into world space.
	\par Default Implementation:
	<b>{ return p; }</b> */
	virtual  Point3 DeformPoint(TimeValue t, int index, Point3 initialP, Point3 p, Matrix3 tm)
		{return p;}
//this is the suggested name that the gizmo should be called in the list
	/*! \remarks This method is called to let the gizmo name it self after creation.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void SetInitialName() {}
//this is the final name of the gizmo in th list
	/*! \remarks This method returns the name of he gizmo.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual const MCHAR *GetName(){return NULL;}
//this sets the final name of the gizmo in the list
	/*! \remarks This method sets the name of the gizmo.
	\par Parameters:
	<b>THCAR *name</b>\n\n
	The name to set the gizmo to.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void SetName(const MCHAR *name) {}

	//! \deprecated Implement SetName(const MCHAR *name) 
	virtual void SetName(MCHAR *name) { SetName(const_cast<const MCHAR*>(name)); }

// this is called when the gizmo is initially created
// it is passed to the current selected verts in the world space
	//count is the number of vertice in *p
	//*p is the list of point being affected in world space
	//numberOfInstances is the number of times this modifier has been instanced
	//mapTable is an index list into the original vertex table for *p
	/*! \remarks This method is called when the gizmo is created. A list of points that
	where selected are passed in world space coordinates. This allows the gizmo
	to setup whatever initial parameters are needed when it is created. This
	should return TRUE if it was successful, else FALSE.
	\par Parameters:
	<b>int count</b>\n\n
	The number of points that are to be affected.\n\n
	<b>Point3 *p</b>\n\n
	The list of points that where selected when the gizmo was created in world
	space.\n\n
	<b>int numbeOfInstances</b>\n\n
	The number of times that the modifier has been instanced.\n\n
	<b>int *mapTable</b>\n\n
	An index list of back into the original point list for the object for
	<b>*p</b>. This is used so the gizmo can map the points back into the
	original point list from the object so you can figure out later on where
	the points are.
	\par Default Implementation:
	<b>{ return TRUE; }</b> */
	virtual BOOL InitialCreation(int count, Point3 *p, int numbeOfInstances, int *mapTable) { return TRUE;}
//this is called before the deformation on a frame to allow the gizmo to do some
//initial setupo
	/*! \remarks This method is called before the actual deformation loop to allow the
	plug-in to create any temporary data it may need.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time of the modification.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void PreDeformSetup(TimeValue t) {}
	/*! \remarks This method is called after the actual deformation loop to allow the
	plug-in to create any temporary data it may need.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time of the modification.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void PostDeformSetup(TimeValue t) {}

	/*! \remarks This method will ask the plugin to create a copy buffer of its current
	parameters. The plugin needs to allocate the memory for this buffer. The
	skin modifier will take care of the destruction of this memory.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual IGizmoBuffer *CopyToBuffer() { return NULL;}
	/*! \remarks This method is called when the user paste data to the gizmo. The gizmo
	needs to cast his pointer into its copy class and the use the data to paste
	into it.
	\par Parameters:
	<b>IGizmoBuffer *buffer</b>\n\n
	The buffer to paste from.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void PasteFromBuffer(IGizmoBuffer *buffer) {}

	/*! \remarks This sets the current to gizmo to be enabled or disabled.
	\par Parameters:
	<b>BOOL enable</b>\n\n
	TRUE to enable; FALSE to disable.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void Enable(BOOL enable) {}
	/*! \remarks This method returns whether the user is in the edit mode for the gizmo.
	When this is TRUE the gizmo will get its HitTest, SelectSubComponents,
	Move, SetSubObjectCenters, SetObjectTMs, ClearSelection, SelectAll, and
	InvertSelections will be passed from skin to the gizmo.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL IsEditing() { return FALSE;}
	/*! \remarks This is called when the system wants the gizmo to stop editing.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EndEditing() {}
	/*! \remarks When the skin modifier wants the gizmo to disable any controls that are not
	used outside the sub object mode this function is called. This gives the
	gizmo a chance to disable/hide any UI elements that it does not want the
	user to manipulate when out of the subobject mode.
	\par Parameters:
	<b>BOOL enable</b>\n\n
	TRUE to enable; FALSE to disable.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EnableEditing(BOOL enable) {}

// From BaseObject
    /*! \remarks This method is called to determine if the specified screen point intersects
    the item. The method returns nonzero if the item was hit; otherwise 0. This
    method differs from its BaseObject version in that this is based a TM which
    is the matrix that transfroms the skin modifier into world space.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time to perform the hit test.\n\n
    <b>INode* inode</b>\n\n
    A pointer to the node to test.\n\n
    <b>int type</b>\n\n
    The type of hit testing to perform. See \ref SceneAndNodeHitTestTypes for details.\n\n
    <b>int crossing</b>\n\n
    The state of the crossing setting. If TRUE crossing selection is on.\n\n
    <b>int flags</b>\n\n
    The hit test flags. See \ref SceneAndNodeHitTestFlags for details.\n\n
    <b>IPoint2 *p</b>\n\n
    The screen point to test.\n\n
    <b>ViewExp *vpt</b>\n\n
    An interface pointer that may be used to call methods associated with the
    viewports.\n\n
    <b>ModContext* mc</b>\n\n
    A pointer to the modifier context.\n\n
    <b>Matrix3 tm</b>\n\n
    The transform matrix to transform the skin into world space.
    \return  Nonzero if the item was hit; otherwise 0.
    \par Default Implementation:
    <b>{ return 0; }</b> */
    virtual int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc, Matrix3 tm) {return 0;}
    /*! \remarks This method is called to change the selection state of the component
    identified by <b>hitRec</b>.
    \par Parameters:
    <b>HitRecord *hitRec</b>\n\n
    Identifies the component whose selected state should be set. See
    Class HitRecord .\n\n
    <b>BOOL selected</b>\n\n
    TRUE if the item should be selected; FALSE if the item should be
    de-selected.\n\n
    <b>BOOL all</b>\n\n
    TRUE if all components in the HitRecord chain should be selected; FALSE if
    only the top-level HitRecord should be selected. (A HitRecord contains a
    Next() pointer; typically you want to do whatever you're doing to all the
    Next()'s until Next() returns NULL).\n\n
    <b>BOOL invert=FALSE</b>\n\n
    This is set to TRUE when <b>all</b> is also set to TRUE and the user is
    holding down the Shift key while region selecting in select mode. This
    indicates the items hit in the region should have their selection state
    inverted.
    \par Default Implementation:
    <b>{ }</b> */
    virtual void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE) {}
    /*! \remarks When this method is called the plug-in should respond by moving its
    selected sub-object components.
    \par Parameters:
    <b>TimeValue t</b>\n\n
    The time of the transformation.\n\n
    <b>Matrix3\& partm</b>\n\n
    The 'parent' transformation matrix. This matrix represents a transformation
    that would take points in the modifier's space and convert them into world
    space points. This is constructed as the node's transformation matrix times
    the inverse of the ModContext's transformation matrix. The node whose
    transformation is used is the node the user clicked on in the scene -
    modifiers can be instanced so there could be more than one node.\n\n
    <b>Matrix3\& tmAxis</b>\n\n
    The matrix that represents the axis system. This is the space in which the
    transformation is taking place.\n\n
    <b>Point3\& val</b>\n\n
    This value is a vector with X, Y, and Z representing the movement along
    each axis.\n\n
    <b>Matrix3 tm</b>\n\n
    The transform matrix.\n\n
    <b>BOOL localOrigin=FALSE</b>\n\n
    When TRUE the transformation is occurring about the sub-object's local
    origin.
    \par Default Implementation:
    <b>{ }</b> */
    virtual void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, Matrix3 tm, BOOL localOrigin=FALSE ) {}
    /*! \remarks When the user is in a sub-object selection level, the system needs to get
    the reference coordinate system definition from the current modifier being
    edited so that it can display the axis. This method specifies the position
    of the center. The plug-in enumerates its centers and calls the callback
    <b>cb</b> once for each. This method differs from its BaseObject version in
    that this is based a TM which is the matrix that transfroms the skin
    modifier into world space.
    \par Parameters:
    <b>SubObjAxisCallback *cb</b>\n\n
    The callback object whose methods may be called. See
    Class SubObjAxisCallback.\n\n
    <b>TimeValue t</b>\n\n
    The time to enumerate the centers.\n\n
    <b>INode *node</b>\n\n
    A pointer to the node.\n\n
    <b>Matrix3 tm</b>\n\n
    The transform matrix to transform the skin into world space.
    \par Default Implementation:
    <b>{ }</b> */
    virtual void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm) {}
	 /*! \remarks When the user is in a sub-object selection level, the system needs to get the
	 reference coordinate system definition from the current modifier being edited
	 so that it can display the axis. This method returns the axis system of the
	 reference coordinate system. The plug-in enumerates its TMs and calls the
	 callback <b>cb</b> once for each. See
	 <a href="ms-its:3dsmaxsdk.chm::/selns_sub_object_coordinate_systems.html">Sub-Object
	 Coordinate Systems</a>. This method differs from its BaseObject version in that
	 this is based a TM which is the matrix that transfroms the skin modifier into
	 world space.
	 \par Parameters:
	 <b>SubObjAxisCallback *cb</b>\n\n
	 The callback object whose methods may be called.\n\n
	 <b>TimeValue t</b>\n\n
	 The time to enumerate the TMs.\n\n
	 <b>INode *node</b>\n\n
	 A pointer to the node.\n\n
	 <b>Matrix3 tm</b>\n\n
	 The transform matrix to transform the skin into world space.
	 \par Default Implementation:
	 <b>{ }</b> */
    virtual void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm) {}
    /*! \remarks This method is called to clear the selection for the given sub-object
    level. All sub-object elements of this type should be deselected. This will
    be called when the user chooses Select None from the 3ds Max Edit menu.
    \par Parameters:
    <b>int selLevel</b>\n\n
    Specifies the selection level to clear.
    \par Default Implementation:
    <b>{}</b> */
    virtual void ClearSelection(int selLevel) {}
	/*! \remarks This method is called to select every element of the given sub-object
	level. This will be called when the user chooses Select All from the Edit
	menu.
	\par Parameters:
	<b>int selLevel</b>\n\n
	Specifies the selection level to select.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SelectAll(int selLevel) {}
    /*! \remarks This method is called to invert the specified sub-object
    level. If the element is selected it should be deselected. If it's
    deselected it should be selected. This will be called when the user chooses
    Select Invert from the Edit menu.
    \par Parameters:
    <b>int selLevel</b>\n\n
    Specifies the selection level to invert.
    \par Default Implementation:
    <b>{}</b> */
    virtual void InvertSelection(int selLevel) {}
};

#pragma warning(pop)

	
//Gizmo extensions for R5
class IGizmoClass2: public MaxHeapOperators 
	{
public:
	virtual ~IGizmoClass2() {}

	//this lets skin pass some tms to the gizmo so they can deal with the double transform
	//when the skin object and the skeleton are linked to the same node and that node is moved there will be a double transformation
	//in R5 we have an option to remove that transformation,

	//points come into the gizmo with the double trsansform on, this lets the gizmo remove and put back the transfrom
	//skin expects the points to have the double transform when it gets the points back from the gizmo since it willl
	//remove it later on
	//removeDoubleTransform - is a matrix3 that will remove the double transform
	//putbackDoubleTransform - is a matrix3 that will putback the double transform
	virtual void SetBackTransFormMatrices(Matrix3 removeDoubleTransform, Matrix3 putbackDoubleTransform) = 0;
	};


//Gizmo extensions for R5.1
class IGizmoClass3: public MaxHeapOperators 
	{
public:
	virtual ~IGizmoClass3() {}
	//This exposes a tool that lets the gizmo know that the user wants to reset the plane of rotation using
	//the current bone orientation to define the plabe
	virtual void ResetRotationPlane()=0;
	};


