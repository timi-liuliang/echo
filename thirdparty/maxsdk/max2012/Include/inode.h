//
// Copyright 2008 Autodesk, Inc.  All rights reserved. 
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may
// not be disclosed to, copied or used by any third party without the prior
// written consent of Autodesk, Inc.
//
//
#pragma once

#include "maxheap.h"
#include "iFnPub.h"
#include "ref.h"
#include "coreexp.h"
#include "InodeTransformModes.h"
#include "GetCOREInterface.h"
#include "quat.h"

// forward declaration
class Material;
class ObjectState;
class Object;
class Control;
class ScaleValue;
class Mtl;
class RenderData;
class View;
class IDerivedObject;
namespace MaxSDK
{
	namespace AssetManagement
	{
		class AssetUser;
	}
}

#define INODE_INTERFACE Interface_ID(0x67b113ca, 0x34204b2b)

// Types of vertex colors to display:
// nvct_num_types is the total number of vertex color types and should always stay at the end of nodeVertexColorType
enum nodeVertexColorType
{ nvct_color, nvct_illumination, nvct_alpha, nvct_color_plus_illum, nvct_soft_select, nvct_map_channel, nvct_num_types };

// Node interface
/*! \sa  Class ReferenceTarget, Class FPMixinInterface, Class Interface, Class INodeTransformed, Class Material, Class Matrix3,  Class Quat,  Class AngAxis, Class Control, Class Object, Class INodeTab.\n\n
\par Description:
The INode class is the interface to nodes in the scene. It provides methods to
access various parts of a node such as its name, transformation matrices,
parents and children, display status, etc. All methods of this class are
implemented by the system.
\par Method Groups:
See <a href="class_i_node_groups.html">Method Groups for Class INode</a>.
*/
#pragma warning(push)
#pragma warning(disable:4100)

class INode: public ReferenceTarget, public FPMixinInterface {
	public:
		//! \brief This represents the value of an invalid node handle
		static const ULONG kNullHandle = 0;

		// Prevents methods not overriden by this class to be hidden by the overriden version
		// Warning: the names it declares take on the access rights of the
		// section where the using statement is placed
		using ReferenceTarget::GetInterface;

		// If this was a temporary INode (like an INodeTransformed) this will delete it.
		/*! \remarks If this was a temporary INode (like an
		<b>INodeTransformed</b>) then this method will delete it.   Also see
		the method <b>Interface::GetModContexts()</b>. */
		virtual void DisposeTemporary() {}

		// In the case of INodeTransformed, this gets a pointer to the real node.
		/*! \remarks In the case of <b>INodeTransformed</b>, this method
		retrieves a pointer to the real node.
		\return A pointer to the node.*/
		virtual INode *GetActualINode() {return this;}

		/*! \remarks Returns the name of the node. */
		virtual MCHAR* 	GetName()=0;
		
		/*! \remarks Sets the name of the node.
		\param s   The name of the node. */
		virtual	void	SetName(MCHAR *s)=0; 		
		
		// Get/Set node's transform ( without object-offset or WSM affect)
		/*! \remarks This method returns the world space     transformation
		matrix of the node at the specified time. This     matrix contains its
		parents transformation. This matrix does not     include the
		object-offset transformation, or any world space     modifier (Space
		Warp) affects.   If you select a single node and change the
		reference coordinate system to 'Local', you'll see the node's axes
		tripod displayed. This tripod graphically depicts the nodes
		transformation matrix.   The Node TM may be considered the world space
		   transformation as far as kinematics is concerned. This is almost
		the complete world space transformation as far as the geometry of
		the object is concerned, except it does not include the
		object-offset transformation. See the method
		<b>INode::GetObjectTM()</b> for the complete world space
		transformation of the geometry of the object.   The Node TM is
		inherited. When a node asks to     retrieve its parents TM, it gets its
		parents Node TM. It does not     get its parents Object TM. The
		object-offset transformation is not inherited.
		\param t Specifies the time to retrieve the TM.
		\param valid Points to storage for the validity interval of the transformation matrix. 
		The interval, if passed, is intersected with the validity interval of the NodeTM.
		\return    The node's world space transform matrix. */
		virtual Matrix3	GetNodeTM(TimeValue t, Interval* valid=NULL)=0;
		/*! \remarks This methods sets the node's world space
		transformation matrix (without the object-offset transformation or
		world space modifier affect). This method will perform the
		appropriate operation to the node's transform controller. For
		example, if the node has a parent, this method will take the
		parent's transformation into consideration when it calls
		<b>SetValue()</b> on the controller.   This method can be used to set
		the world space position of the node.
		\param t Specifies the time to set the transformation matrix.
		\param tm The node's world space transformation matrix. */
		virtual void 	SetNodeTM(TimeValue t, Matrix3& tm)=0;

		// Invalidate node's caches
		/*! \remarks This method will notify the node's subtree that     the
		transformation matrix has changed.     <b>NotifyDependents()</b> is
		called with the message \ref REFMSG_CHANGE. */
		virtual void InvalidateTreeTM()=0;
		/*! \remarks This method will invalidate the node's
		transformation matrix cache. */
		virtual void InvalidateTM()=0;
		/*! \remarks This method will invalidate the node's world     space
		cache. */
		virtual void InvalidateWS()=0;

		// Invalidates the node's rectangle in the viewports.
		/*! \remarks This method invalidates the rectangle in the viewports that the
		node occupies. Rectangles flagged as invalid will be updated on the
		next screen redraw. 
		Each node caches the rectangle in the viewports that the node occupies. These 
		caches are updated when the node is drawn in the viewports.
		\param t - The time to invalidate the node's rectangle
		\param oldRect - If true, the cached viewport rectangles of the node are invalidated.
		This would be the case when a node property that doesn't affect the size or position 
		of the node's viewport rectangles, such as wireframe color, is changed.
		Otherwise, the node's current rectangle in each viewport is invalidated. If the 
		node's world space cache is not valid, the node's world state at time t will be evaluated,
		from which its current viewport rectangles are calculated.
		*/
		virtual void InvalidateRect( TimeValue t, bool oldRect = false) = 0;

		// Get object's transform (including object-offset)
		// and also the WSM affect when appropriate )
		// This is used inside object Display and HitTest routines
		/*! \remarks This method returns the transformation matrix     the
		object needs to be multiplied by to transform it into world     space.
		 At times, this matrix may be the identity. For     example, a
		deformable object that has a Space Warp applied to it     would already
		have been translated into world space by the space     warp. Since the
		object is already in world space the matrix needed     to get it there
		is the identity.   This matrix would <b>not</b> be the identity for
		a deformable object with only object space modifiers applied. This
		object would indeed need to be transformed. In this case the TM
		returned would include the NodeTM plus the object-offset
		transformation. So, <b>GetObjectTM()</b> is dependent on the
		context when it is called -- it will either be equal to
		<b>GetObjectTMAfterWSM()</b> or     <b>GetObjectTMBeforeWSM()</b>.
		Developers should use     <b>GetObjectTMBeforeWSM()</b> if what is
		wanted is the object   TM and not the identity matrix.   For
		non-deformable objects this matrix may     include the NodeTM, the
		object-offset transformation and the world     space modifier affect.
		This matrix could be used, for example, if you     have a TriObject and
		wanted to get the world space coordinate of     one of its vertices.
		You could do this by taking the vertex     coordinate in object space
		and multiplying it by the matrix     returned from this method.   This
		matrix is also often used inside an     object's <b>Display()</b> and
		<b>HitTest()</b> methods.     When an object goes to draw itself (in
		its     <b>BaseObject::Display()</b> method) it is given a node
		pointer. What the object normally does is use the node pointer and
		calls <b>GetObjectTM()</b>. It then takes the matrix returned     and
		sets it into the graphics window (using
		<b>GraphicsWindow::setTransform()</b>). In this way, when the
		object starts drawing points in object space, they will be
		transformed with this matrix. This will transform the points into
		world space when they are drawn.   The Object TM is not inherited.
		\param time Specifies the time to retrieve the object's transform matrix.
		\param valid Points to storage for the validity interval of the transformation matrix.
		\return    The object's transformation matrix.
		*/
		virtual Matrix3 GetObjectTM(TimeValue time, Interval* valid=NULL)=0;

		// Get object's transform including object-offset but not WSM affect
		/*! \remarks This method explicitly retrieves the pipeline
		ObjectState TM before any world space modifiers have been applied.
		This includes the node's TM and the object-offset transformation
		(but not any world space modifier affect). See the section on the
		<a href="ms-its:3dsmaxsdk.chm::/pipe_geometry_root.html">Geometry
		Pipeline</a> for additional details on this method.
		\param time Specifies the time to retrieve the transform matrix.
		\param valid Points to storage for the validity interval of the transformation matrix.
		*/
		virtual Matrix3 GetObjTMBeforeWSM(TimeValue time, Interval* valid=NULL)=0;

		// Get object's transform including object-offset and WSM affect
		/*! \remarks This method explicitly retrieves the pipeline
		ObjectState TM after any world space modifiers have been applied.
		This includes the Node TM, the object-offset transformation, and
		any world space modifier affects. In some cases a world space
		modifier can actually deform the TM itself if it cannot deform the
		object. Examples of this are cameras and lights. These objects do
		not have any 'object' to deform, so the space warp deforms the TM
		instead. See the section on the
		<a href="ms-its:3dsmaxsdk.chm::/pipe_geometry_root.html">Geometry
		Pipeline</a> for     additional details on this method.   Note: Beware
		of calling this method from inside     a function that performs a mesh
		traversal as doing so can invalidate the mesh.
		\param time Specifies the time to retrieve the object's transform matrix.
		\param valid Points to storage for the validity interval of the transformation matrix. */
		virtual Matrix3 GetObjTMAfterWSM(TimeValue time, Interval* valid=NULL)=0;

		// evaluate the State the object after offset and WSM's applied		
		// if evalHidden is FALSE and the node is hidden the pipeline will not
		// actually be evaluated (however the TM will).
		/*! \remarks This method should be called when a developer needs to
		work with an object that is the result of the node's pipeline. This is
		the object that the appears in the scene.\n\n
		This may not be an object that anyone has a reference to - it may just
		be an object that has flowed down the pipeline. For example, if there
		is a Sphere in the scene that has a Bend and Taper applied,
		<b>EvalWorldState()</b> would return an <b>ObjectState</b> containing a
		TriObject. This is the result of the sphere turning into a TriObject
		and being bent and tapered (just as it appeared in the scene).\n\n
		If a developer needs to access the object that the node in the scene
		references, then the method <b>INode::GetObjectRef()</b> should be used
		instead.
		\param time Specifies the time to retrieve the object state.
		\param evalHidden If FALSE and the node is hidden, the pipeline will not actually be
		evaluated (however the TM will).
		\return  The ObjectState that is the result of the pipeline. See
		Class ObjectState.
		\code
		// Get the object from the node
		ObjectState os = node->EvalWorldState(ip->GetTime());

		if (os.obj->SuperClassID()==GEOMOBJECT_CLASS_ID)
		{
		   obj = (GeomObject*)os.obj;
		   //...
		}
		\endcode
		The following code shows how a TriObject can be retrieved from a node. Note on the code that if you call ConvertToType() on an object and it returns a pointer other than itself, you are responsible for deleting that object.
		\code
		// Retrieve the TriObject from the node
		int deleteIt;
		TriObject *triObject = GetTriObjectFromNode(ip->GetSelNode(0), deleteIt);

		// Use the TriObject if available
		if (!triObject) return;
		// ...
		// Delete it when done...
		if (deleteIt) triObject->DeleteMe();

		// Return a pointer to a TriObject given an INode or return NULL
		// if the node cannot be converted to a TriObject
		TriObject *Utility::GetTriObjectFromNode(INode *node, int &deleteIt)
		{
		   deleteIt = FALSE;
		   Object *obj = node->EvalWorldState(ip->GetTime()).obj;

		   if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
		   {
		     TriObject *tri = (TriObject *) obj->ConvertToType(ip->GetTime(),
		     Class_ID(TRIOBJ_CLASS_ID, 0));
		     // Note that the TriObject should only be deleted
		     // if the pointer to it is not equal to the object
		     // pointer that called ConvertToType()
		     if (obj != tri) deleteIt = TRUE;
		     return tri;
		   }
		   else
		   {
		     return NULL;
		   }
		}
		\endcode
                */
		virtual	const ObjectState& EvalWorldState(TimeValue time,BOOL evalHidden=TRUE)=0;	

		// Hierarchy manipulation
		/*! \remarks Retrieves the parent node of this node. If the     node
		is not linked, its parent is the root node. This may be     checked
		using <b>INode::IsRootNode()</b>. */
		virtual INode* 	GetParentNode()=0;
		
		/*! \remarks Makes the specified node a child of this node.
		\param node Specifies the node to attach.
		\param keepTM If nonzero, the world transform matrix of the specified (child) 
		node is unchanged after the attach operation, i.e. INode::GetNodeTM returns 
		the same matrix both before and after the attach operation. Otherwise, the world 
		transform of the specified (child) node is affected by the parent node's transform.*/
		virtual void 	AttachChild(INode* node, int keepTM=1)=0; // make node a child of this one
		
		/*! \remarks Detaches this node from its parent.
		\param t Specifies the time at which to detach the node.
		\param keepTM If nonzero, the world transform matrix of the detached node
		is unchanged, i.e. INode::GetNodeTM returns the same matrix both before and after 
		the detach operation. Otherwise, the world transform of the detached node may change
		as a result of the detach operation since the node stops inheriting its parent node's transform.
		\note The node is detached from its current parent and attached to the scene's root node.*/
		virtual	void 	Detach(TimeValue t, int keepTM=1)=0;  	  // detach node
		
		/*! \remarks Returns the number of children of this node. */
		virtual int 	NumberOfChildren()=0;
		
		/*! \remarks Retrieves the 'i-th' child node of this     node.
		\param i Specifies the child node to retrieve. */
		virtual INode* 	GetChildNode(int i)=0;
		
		// This will delete a node, handle removing from the hierarchy, and also handle Undo.		
		/*! \remarks This method will delete the node, handle removing it from the hierarchy, and handle undo.  
		\param t The time for the deletion.
		\param keepChildPosition If TRUE the position of any children of this node are kept the same; 
		otherwise linked children may move due to the deletion. */
		virtual void Delete(TimeValue t, int keepChildPosition) {} 

		// display attributes
		//! \brief Controls the hidden state of the node in the scene.
		/*! \param onOff Pass TRUE to hide the node in the scene; pass FALSE to make the node visible. */
		virtual void	Hide(BOOL onOff)=0;				// set node's hide bit
		
		//! \brief Makes the node, and optionally the layer, visible in the scene.
		/*! \param doLayer Pass TRUE to unhide the node in the scene; pass FALSE to make the node visible. */
		virtual void	UnhideObjectAndLayer(bool dolayer = true) {Hide(FALSE);}
		
		//! \brief Returns the hidden state of this node in the scene
		/*! \return non-zero if this node is hidden, else false */
		virtual int		IsObjectHidden() {return 0;}	
		
		//! \brief Determines if the node is hidden in the scene.
		/*! \param hflags If you pass 0, you will get the hidden state of the node. 
		If you pass one or more of the flags shown below, the method checks
		the Class_ID of the node to see if it's hidden by the specified category.
		You may specify one or more of the following values:\n\n
		<b>HIDE_OBJECTS</b>\n
		<b>HIDE_SHAPES</b>\n
		<b>HIDE_LIGHTS</b>\n
		<b>HIDE_CAMERAS</b>\n
		<b>HIDE_HELPERS</b>\n
		<b>HIDE_WSMS</b>\n
		<b>HIDE_SYSTEMS</b>\n
		<b>HIDE_PARTICLES</b>\n
		<b>HIDE_ALL</b>\n
		<b>HIDE_NONE</b>\n\n
		See the method <b>Interface::GetHideByCategoryFlags()</b> for how to retrieve the
		currently set values to use as the flags for this method.
		\param forRenderer If true, test to see if the node is hidden for the render, else if it is hidden in the viewport
		\return Nonzero if the node is hidden; otherwise 0. */
		virtual int		IsHidden(DWORD hflags=0,BOOL forRenderer=FALSE) {return 0;}
		
		//! \brief Returns nonzero if the node is hidden in any way
		/*! This method takes into account both the node hidden attribute and the 'Hide By Category' flags.
		\param forRenderer If true, test to see if the node is hidden for the render, else if it is hidden in the viewport
		returns zero. */
		virtual int		IsNodeHidden(BOOL forRenderer=FALSE) {return 0;}
		
		//! \brief Controls the frozen state of the node in the scene.
		/*! A frozen node is visible but cannot be picked.
		\param onOff TRUE if the node should be frozen; FALSE if the node should not be frozen. */
		virtual void	Freeze(BOOL onOff)=0;	
		
		/*! \brief Unfreeze the node, and optionally its layer
		\param doLayer if true, unfreeze the layer */
		virtual void	UnfreezeObjectAndLayer(bool dolayer = true) {Freeze(FALSE);}
		
		/*! \brief Test the frozen property of this node
		\return non-zero if this node is frozen, else zero */
		virtual int		IsObjectFrozen() {return 0;};
		
		//! \brief Determines if the node is frozen in any way in the scene.
		/*! A node can have the frozen property set, or be frozen by 'ghosting'
		\return    Nonzero if the node is frozen; otherwise 0. */
		virtual int		IsFrozen()=0;
		
		/*! \remarks This method allows you to set the
		<b>NODE_SHOW_FRZN_WITH_MTL</b> flag in the node so that the   node will
		be displayed in a frozen state with materials   applied.
		\param onOff TRUE to set the flag; FALSE to disable. */
		virtual void	SetShowFrozenWithMtl(BOOL onOff)=0;
		
		/*! \remarks This method returns the state of the <b>NODE_SHOW_FRZN_WITH_MTL</b> flag 
		in the node and whether it is enabled or disabled. */
		virtual int		ShowFrozenWithMtl()=0;
		
		//! \breif Set the X-Ray node property
		/*! The X-Ray Material display property allows you to quickly 
		make objects transparent. This method toggles it on or off for this node.
		\param onOff TRUE to use; FALSE to not use. */
		virtual void	XRayMtl(BOOL onOff)=0;
		
		//! \brief Get the X-Ray node property
		/*! The X-Ray Material display property allows you to quickly 
		make objects transparent.
		\return TRUE if X-Ray materials are enabled; else FALSE */
		virtual int		HasObjectXRayMtl() {return 0;};
		
		/*! \brief Returns nonzero if the X-Ray Material display property is
		on for the node; otherwise zero. */
		virtual int		HasXRayMtl()=0;
		
		/*! \brief Ignore this node during zoom extents
		\param onOff If true, Set this node to be ignored during zoom extents. */
		virtual void	IgnoreExtents(BOOL onOff)=0;// ignore this node during zoom extents
		
		/*! \brief Is this node ignored during zoom extents.
		\return true if this node is to be ignored during zoom extents. */
		virtual int		GetIgnoreExtents()=0;
		
		/*! \remarks Controls if the node is displayed with a bounding box representation in the scene.
		\param onOff TRUE to display the node as its bounding box; FALSE for normal display. */
		virtual void	BoxMode(BOOL onOff)=0;
		
		/*! \remarks Determines if the node is displayed in box mode in the scene.
		\return Nonzero if the node is displayed in box mode; otherwise 0. */
		virtual int		GetBoxMode()=0;
		
		/*! \remarks Controls the display of all the edges of the node (including "hidden" ones).
		\param onOff TRUE to display all the node's edges; FALSE to not display "hidden" edges. */
		virtual void	AllEdges(BOOL onOff)=0;
		
		/*! \remarks Determines if all the edges of the node are displayed.
		\return    Nonzero if all the edges (including "hidden" ones) are displayed; otherwise 0. */
		virtual int		GetAllEdges()=0;
		
		/*! \remarks This method allows you to enable or disable the display of vertex ticks on the node.
		\param onOff TRUE to enable; FALSE to disable. */
		virtual void	VertTicks(int onOff)=0;
		
		/*! \remarks This method returns the state of the vertex ticks display. 
		TRUE if enabled; FALSE if disabled. */
		virtual int		GetVertTicks()=0;
		
		/*! \remarks Controls if the node is displayed using back-face
		culling (faces whose surface normals are pointing away from the observer are not drawn).
		\param onOff TRUE if the node should be drawn using back-face culling; 
		FALSE if all faces should be drawn. */
		virtual void	BackCull(BOOL onOff)=0;
		
		/*! \remarks Determines if back-face culling is being used to draw the node.
		\return    Nonzero if back-face culling is used; otherwise     0. */
		virtual int		GetBackCull()=0;
		
		/*! \remarks Sets the shadow casting attribute of the node to on or off.
		\param onOff TRUE to turn shadow casting on; FALSE to turn it off. */
		virtual void 	SetCastShadows(BOOL onOff)=0; 
		
		/*! \remarks Retrieves the shadow casting attribute of the     node.
		\return    Nonzero indicates the node casts shadows; zero     if the
		node does not cast shadows. */
		virtual int		CastShadows()=0;
		
		/*! \remarks Sets the shadow receiving attribute of the node to on or off.
		\param onOff TRUE to turn shadow receiving on; FALSE to turn it off. */
		virtual void 	SetRcvShadows(BOOL onOff)=0;
		
		/*! \remarks Retrieves the shadow receiving attribute of the node.
		\return    Nonzero indicates the node receives shadows; zero if the node does not receive shadows. */
		virtual int		RcvShadows()=0;
		
		virtual void 	SetGenerateCaustics(BOOL onOff)	{}
		virtual int		GenerateCaustics()				{return 0;}
		virtual void 	SetRcvCaustics(BOOL onOff)		{}
		virtual int		RcvCaustics()					{return 0;}
		

		/*! \remarks This method allows you to set the "apply atmospherics" flag for the node.
		\param onOff TRUE to enable the flag, FALSE to disable. */
		virtual void	SetApplyAtmospherics(BOOL onOff)=0;
		
		/*! \remarks This method returns the on/off state of the "apply atmospherics" flag. */
		virtual int		ApplyAtmospherics()=0;

		virtual void 	SetGenerateGlobalIllum(BOOL onOff)	{}
		virtual int		GenerateGlobalIllum()				{return 0;}
		virtual void 	SetRcvGlobalIllum(BOOL onOff)		{}
		virtual int		RcvGlobalIllum()					{return 0;}
		
		/*! \remarks Sets the type of motion blur used by the node.
		\param kind The kind of motion blur. One of the following values:\n\n
		<b>0</b>: None.\n
		<b>1</b>: Object Motion Blur.\n
		<b>2</b>: Image Motion Blur. */
		virtual void 	SetMotBlur(int kind)=0;
		
		/*! \remarks Retrieves the type of motion blur used by the node.
		\return One of the following values:\n\n
		<b>0</b>: None\n
		<b>1</b>: Object Motion Blur.\n
		<b>2</b>: Image Motion Blur. */
		virtual int		MotBlur()=0;
		/*! \remarks Returns the image motion blur multiplier value at the specified time.
		\param t The time to retrieve the value. */
		virtual float   GetImageBlurMultiplier(TimeValue t) { return 1.0f;}
		
		/*! \remarks Sets the image blur multiplier value for the node. This is
		used to increase or decrease the length of the blur 'streak'.
		\param t The time to set the value.
		\param m The value to set. */
		virtual void    SetImageBlurMultiplier(TimeValue t, float m){};
		
		/*! \remarks Sets the controller used for the image blur multiplier value.
		\param cont Points for the controller to use.     */
		virtual	void  	SetImageBlurMultController(Control *cont){}
		
		/*! \remarks Returns a pointer to the controller for the image blur multiplier value. */
		virtual	Control *GetImageBlurMultController() {return NULL; }

		// Object motion blur enable controller. This affects both object and image motion blur
		/*! \remarks Returns TRUE if the object motion blur controller is 'on' at the specified time; 
		otherwise FALSE.
		\param t The time to check.
		\par Default Implementation:
		<b>{ return 1; }</b> */
		virtual BOOL GetMotBlurOnOff(TimeValue t) { return 1;  }
		
		/*! \remarks Sets the state of the object motion blur controller to on or off at the specified time.
		\param t The time to set the value.
		\param m TRUE for on; FALSE for off.
		\par Default Implementation:
		<b>{}</b> */
		virtual void  SetMotBlurOnOff(TimeValue t, BOOL m) { }
		
		/*! \remarks Returns a pointer to the controller handling the object motion blur on / off setting.
		\par Default Implementation:
		<b>{ return NULL;}</b> */
		virtual Control *GetMotBlurOnOffController() { return NULL;}
		
		/*! \remarks Sets the controller used for handling the object motion blur on / off setting.
		\param cont Points to the controller to set.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetMotBlurOnOffController(Control *cont) { }

		/*! \remarks Sets the state of the node's renderable flag. If this flag
		is on the node will appear in rendered images; if off it won't.
		\param onOff TRUE for on; FALSE for off. */
		virtual void 	SetRenderable(BOOL onOff)=0;
		
		/*! \remarks  Returns nonzero if the renderable flag is on; zero if off. */
		virtual int		Renderable()=0;
		
		/*! \remarks This method allows you to set the primary visibility flag
		and define whether or not the node is visible to the camera.
		\param onOff TRUE to enable the flag, FALSE to disable. */
		virtual void	SetPrimaryVisibility(BOOL onOff) = 0;
		
		/*! \remarks This method returns the on/off state of the primary
		visibility to determine whether or not the node is visible to the camera. */
		virtual int		GetPrimaryVisibility() = 0;
		
		/*! \remarks  This method allows you to set the secondary visibility flag
		and define whether or not the node is visible to reflections and refractions..
		\param onOff TRUE to enable the flag, FALSE to disable. */
		virtual void	SetSecondaryVisibility(BOOL onOff) = 0;
		
		/*! \remarks This method returns the on/off state of the secondary
		visibility to determine whether or not the node is visible to
		reflections and refractions. */
		virtual int		GetSecondaryVisibility() = 0;

		/*! \remarks Sets the vertex color flag to on or off. This controls the
		display of assigned vertex colors. Vertex colors are assigned in
		the editable mesh in vertex or face sub-object level. Vertex colors
		only appear in viewports using Smooth or Smooth + Highlight display
		modes, regardless of the state of this flag.
		\param onOff Nonzero for on; zero for off. */
		virtual void    SetCVertMode(int onOff)		{}
		
		/*! \remarks Returns nonzero if the vertex color flag if on; otherwise zero. */
		virtual int     GetCVertMode()				{return 0;}
		
		/*! \remarks  Sets the vertex color shaded flag. This determines whether
		the vertex colors appears shaded in the viewport. When this is off,
		the colors are unshaded, and appear in their pure RGB values,
		looking a little like self-illuminated materials. When on, the
		colors appear like any other assigned color in the viewports
		\param onOff Nonzero of on; zero of off. */
		virtual void    SetShadeCVerts(int onOff)	{}
		
		/*! \remarks Returns nonzero if the vertex color shaded flag is on; zero if off. */
		virtual int     GetShadeCVerts()			{return 0;}
		
		/*! \remarks Get the node's vertex color "type".  This is one of the 
		nodeVertexColorType enum's above. */
		virtual int GetVertexColorType () { return 0; }
		
		/*! \remarks Set the node's vertex color "type".  This is one of the 
		nodeVertexColorType enum's above. */
		virtual void SetVertexColorType (int nvct) { }

		/*! \brief get the map channel to be displayed as vertex color. */
		virtual int GetVertexColorMapChannel () { return 1; }
		/*! \brief set the map channel to be displayed as vertex color. */
		virtual void SetVertexColorMapChannel (int vcmc) { }

		/*! \remarks Returns nonzero if the trajectory display is on; zero if the trajectory display is off. */
		virtual int		GetTrajectoryON() {return 0;}
		
		/*! \remarks This method toggles the trajectory display for the node.
		\param onOff TRUE for on; FALSE for off. */
		virtual void    SetTrajectoryON(BOOL onOff) {}

		// bone display attributes.
		/*! \remarks Controls the display of Bones in the scene. A bone
		is just the link (or line) connecting the node to its parent. These
		are the same options as available in the 3ds Max user interface in
		the Display branch, under Link Display, i.e. Display Links and Link
		Replaces Object.
		\param boneVis Specifies the display state:\n\n
		<b>0</b>: Bones are not drawn.\n
		<b>1</b>: Bones are drawn.\n
		<b>2</b>: Only bones are shown. */
		virtual void 	ShowBone(int boneVis)=0;
		
		/*! \remarks Controls the display of Bones as simple lines in the scene.
		\param onOff   Nonzero if bones should be shown as lines only; 0 for normal display. */
		virtual void	BoneAsLine(int onOff)=0;
		
		/*! \remarks Returns TRUE if the node's bone is turned on; otherwise FALSE. */
		virtual BOOL	IsBoneShowing()=0;
		
		/*! \remarks Returns TRUE if the bone is showing but the object is
		hidden; FALSE if both the bone and the node is hidden. */
		virtual BOOL	IsBoneOnly() { return 0; }

		// used for hit-testing and selecting node and target as a single unit
		/*! \remarks This method is used for hit-testing and selecting node and
		target as a single unit. In 3ds Max 2.0 and later you can click on
		the line connecting, say, a camera to its target and drag that
		around in the viewports. Doing so moves both the camera and its
		target as a locked pair. To accomplish this, the camera, light,
		and tape measure objects (those with two nodes linked by a Look At
		controller) check for a hit on the object-target line. If they get
		there (but not at the object or target itself), then they call this
		method passing TRUE. Then, when a hit is registered, 3ds Max checks
		the value of this variable (by calling
		<b>GetTargetNodePair()</b>), and, if it is TRUE, selects both the
		target and the node. If it's FALSE, then either the target or the
		node, but not both, gets selected, as with 3ds Max 1.x. For sample
		code see <b>/MAXSDK/SAMPLES/OBJECTS/LIGHT.CPP</b> or <b>TAPEHELP.CPP</b>.
		\param onOff TRUE for on; FALSE for off.
		\par Default Implementation:
		<b>{}</b> */
		virtual void	SetTargetNodePair(int onOff) {}
		
		/*! \remarks Returns the target/node pair setting stored by 3ds Max. See
		<b>SetTargetNodePair()</b> above for details.
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual int		GetTargetNodePair() { return 0; }

		// Access node's wire-frame color
		/*! \remarks Retrieves the node's wire-frame color. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF-DWORD format</a>. */
		virtual DWORD 	GetWireColor()=0;
		
		/*! \remarks Sets the node's wire-frame color. This can be any
		of the 16 million possible colors in 24 bit. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF-DWORD format</a>.
		\param newcol Specifies the new wire-frame color for the node.
		It may be specified using the RGB macro, for example:
		<b>RGB(0,0,255);</b> */
		virtual void 	SetWireColor(DWORD newcol)=0;

		// Test various flags
		/*! \remarks Determines if this node is the root node (does not
		have a parent node).
		\return    Nonzero if the node is the root node; otherwise 0. */
		virtual int 	IsRootNode()=0;
		
		/*! \remarks Determines if the node is selected.
		\return Nonzero if the node is selected; otherwise 0. */
		virtual int 	Selected()=0;
		
		/*! \remarks Returns nonzero if the node has its dependent flag
		set; otherwise 0. This is dependent in the sense of 3ds Max's
		Views/Show Dependencies mode. When in the Modify branch, Show
		Dependencies will show all the nodes that are dependent on the
		current modifier or object being editing by highlighting them in
		green. It also set a flag in the node. This method allows a
		developer to check this flag. */
		virtual int  	Dependent()=0;
		
		/*! \remarks Determines if the node is a target node of a lookat controller.
		\return    Nonzero if the node is a target; otherwise 0. */
		virtual int 	IsTarget()=0;
		
		/*! \remarks This method controls the property of the node indicating if
		it's a target or not. Calling this is necessary when hooking up
		targets as the target node must have its <b>IsTarget()</b> property set.
		\param b TRUE for set; FALSE for off. */
		virtual	void  	SetIsTarget(BOOL b)=0;

		// Node transform locks
		/*! \remarks Retrieves the specified transform lock state of     the
		node. When the user is doing interactive Moving / Rotating /
		Scaling these locks simply remove one of the components.
		\param type See transformLockTypes.
		\param axis See \ref transformLockAxis.
		\return TRUE if the lock is set; otherwise FALSE. */
		virtual BOOL GetTransformLock(int type, int axis)=0;
		
		/*! \remarks Sets the specified transform lock state of the
		node. When the user is doing interactive Moving / Rotating /
		Scaling these locks simply remove one of the components.
		\param type See \ref transformLockTypes.
		\param axis See \ref transformLockAxis.
		\param onOff TRUE sets the lock on; FALSE sets the lock off. */
		virtual void SetTransformLock(int type, int axis, BOOL onOff)=0;

		// Get target node if any.
		/*! \remarks Retrieves this node's target node if any.
		\return NULL if this node has no target. */
		virtual	INode* 	GetTarget()=0;
		
		/*! \remarks If this node is a target of a lookat controller,
		this method finds the node that looks at it.
		\return The node that looks at this node or NULL if the node is not a target. */
		virtual INode* 	GetLookatNode()=0;

		// This is just GetParent+GetNodeTM
		/*! \remarks Retrieves the parent node's transformation matrix.
		This is simply for convenience. It is the equivalent to the
		following code: <b>node-\>GetParentNode()-\>GetNodeTM();</b>
		\param t Specifies the time to retrieve the transformation matrix.
		\return    The parent node's transformation matrix. */
		virtual Matrix3 GetParentTM(TimeValue t)=0;

		// This is just GetTarget+GetNodeTM
		/*! \remarks Retrieves the target node's transformation matrix.
		This is simply for convenience. It is the equivalent to the
		following code:\n\n
		<b>node->GetTarget()->GetNodeTM();</b>
		\param t Specifies the time to retrieve the transformation matrix.
		\param m The result is stored here.
		\return Nonzero if the target matrix was retrieved (the node had a target); otherwise 0. */
		virtual int 	GetTargetTM(TimeValue t, Matrix3& m)=0;

		/*! \remarks Returns the object that this node references     unless
		the node has been bound to a Space Warp. In that case this     method
		will not return the WSM derived object even though the     node's
		object reference points at it. Instead it will return the     item that
		the WSM derived object references. Thus, this method will     always
		return essentially the object space portion of the pipeline.     In
		contrast, see <b>GetObjOrWSMRef()</b> below.   See the
		<a href="ms-its:3dsmaxsdk.chm::/pipe_geometry_root.html">Geometry
		Pipeline</a>     section for additional details. */
		virtual Object* GetObjectRef()=0;
		
		/*! \remarks Sets the object that this node references. See     the
		<a href="ms-its:3dsmaxsdk.chm::/pipe_geometry_root.html">Geometry
		Pipeline</a> section for additional details.
		\param o The object this node will reference. */
		virtual void 	SetObjectRef(Object *o)=0;
		
		/*! \remarks This method returns the actual object reference of the node
		directly. So if this node is bound to a Space Warp this method will
		return a WSM derived object. If you want the object space portion
		of the pipeline see <b>GetObjectRef()</b> above. */
		virtual Object* GetObjOrWSMRef()=0;
		
		/*! \remarks Retrieves the node's transform controller. The
		standard 3ds Max transform controllers each have sub -controllers
		for position, rotation and scale. To access the data of the node's
		transform controller you may use Class IKeyControl. The
		following code fragment shows an example of how this may be done
		for a PRS controller.
		\code
		Control *c;
		c = node->GetTMController()->GetPositionController();
		IKeyControl *ikeys = GetKeyControlInterface(c);
		\endcode
		With this controller interface you can use its methods to get
		information about the keys. For example:
		\code
		int num = ikeys->GetNumKeys();
		\endcode */
		virtual Control* GetTMController()=0;
		
		/*! \remarks Sets the node's transform controller.
		\param m3cont The Matrix3 transform controller to use.
				This controller must return a SuperClassID of CTRL_MATRIX3_CLASS_ID 
		\return TRUE if the controller was a valid transform controller for this node,
				and the reference was successfully created, else FALSE. */
		virtual BOOL 	SetTMController(Control *m3cont)=0;

		// Visibility controller
		/*! \remarks Returns the visibility controller for this node. */
		virtual Control *GetVisController()=0;
		
		/*! \remarks Sets the visibility controller for this     node.
		\param cont The controller to use for visibility control. */
		virtual void    SetVisController(Control *cont)=0;
		
		/*! \remarks Retrieves the visibility of the node at the time
		passed and updates the validity interval passed. Values \< 0
		indicate off while values \> 0 indicate on. The node is fully
		visible (opaque) when 1.0 and fully invisible (transparent) when 0.0.
		\note this value may be inherited by the children of this node
		\param t The time to get the visibility value.
		\param valid The validity interval to update based on the validity of the visibility. */
		virtual float   GetVisibility(TimeValue t,Interval *valid=NULL)=0;
		
		/*! \remarks Gets the visibility for the node within the specified view at the time passed,
		and updates the validity.  The visibility calculated includes the view dependent
		Level Of Detail visibility calculated by the Nodes visibility controller.
		See also Control::EvalVisibility for more info.
		\param t The time to get the visibility value.
		\param view This is the view to calculate the view-dependent LOD visibility from.
		\param valid The validity interval to update based on the validity of the visibility. 
		\return The visibilty of the node as rendered. Values <= 0 are invisible, and
			values >= 1 are fully opaque. */
		virtual float   GetVisibility(TimeValue t,View &view,Interval *valid=NULL) {return GetVisibility(t,valid);}
		
		/*! \remarks Sets the visibility of the node to the value passed at the time passed.
		\param t The time to set the visibility value.
		\param vis The visibility of the node to set. This is treated like a
		boolean value where < 0 means off and > 0 means on. */
		virtual void	SetVisibility(TimeValue t,float vis)=0;
		
		/*! \remarks Returns the local visibility of the node. The value
		returned from this method is treated like a boolean value where <
		0 means off and > 0 means on. If a node returns TRUE from
		<b>GetInheritVisibility()</b> then its visibility is determined by
		this method. If <b>GetInheritVisibility()</b> method returns FALSE
		then the visibility of the node is determined by <b>GetVisibility()</b>.
		\note This value does not take into account the parent nodes visibility, and is not inherited by the children of this node
		\param t The time to get the local visibility value.
		\param valid The validity interval to update based on the validity of the local visibility controller. */
		virtual float   GetLocalVisibility(TimeValue t,Interval *valid=NULL)=0;
		
		/*! \remarks Returns TRUE if the node's visibility is determined by the
		visibility of the parent of the node; otherwise returns FALSE. */
		virtual BOOL 	GetInheritVisibility()=0;
		
		/*! \remarks This method is called to set the state of the node's inherit visibility flag.
		\param onOff Pass TRUE to have the node inherit its
		visibility from its parent; otherwise pass FALSE and the node's
		visibility will be determine by the node itself (not its parent). */
		virtual void 	SetInheritVisibility(BOOL onOff)=0;

		// Set/Get REnderOccluded property
		/*! \remarks Sets the state of the node's 'Render Occluded Object' flag.
		\param onOff TRUE for on; FALSE for off. */
		virtual void  SetRenderOccluded(BOOL onOff)=0;
		
		/*! \remarks Returns TRUE if the node's 'Render Occluded Object' flag is set; otherwise FALSE. */
		virtual BOOL  GetRenderOccluded()=0;

		// Renderer Materials
		/*! \remarks Returns a pointer to the renderer material for the
		node. If the value returned is NULL the user has not assigned a
		material yet. See Class Mtl, 
		<a href="ms-its:3dsmaxsdk.chm::/mtls_materials.html">Working with Materials and Textures</a>. */
		virtual Mtl *GetMtl()=0;
		
		/*! \remarks Sets the renderer material used by the node. If the
		value set is NULL it indicates a material has not been assigned. In
		this case, the renderer uses the wireframe color of the node for
		rendering. See Class Mtl, 
		<a href="ms-its:3dsmaxsdk.chm::/mtls_materials.html">Working with Materials and Textures</a>.
		\param matl The materials used to render the node. */
		virtual void SetMtl(Mtl* matl)=0;

		// GraphicsWindow Materials
		/*! \remarks Returns a pointer to the <b>GraphicsWindow</b>
		materials. See <b>NumMtls()</b> below for the number of entries in this array. */
		virtual Material* Mtls()=0;   // Array  of GraphicsWindow Materials 
		
		/*! \remarks Returns the number of entries in the array of
		Materials returned by <b>Mtls()</b> above. */
		virtual int 	NumMtls()=0;  // number of entries in Mtls

		// Object offset from node:
		/*! \remarks Sets the position portion of the object offset from
		the node. See the Advanced Topics section on
		<a href="ms-its:3dsmaxsdk.chm::/trans_the_node_and_object_offset_transformations.html">Node
		and %Object Offset Transformations</a> for an overview of the %object offset transformation.
		\param p Specifies the position portion of the object-offset. */
		virtual void 	SetObjOffsetPos(Point3 p)=0;
		
		/*! \remarks Returns the position portion of the object-offset
		from the node as a Point3. See the Advanced Topics section on
		<a href="ms-its:3dsmaxsdk.chm::/trans_the_node_and_object_offset_transformations.html">
		Node and Object Offset Transformations</a> for an overview of the object offset transformation. */
		virtual	Point3 	GetObjOffsetPos()=0;
		
		/*! \remarks Sets the rotation portion of the object-offset from
		the node. See the Advanced Topics section on
		<a href="ms-its:3dsmaxsdk.chm::/trans_the_node_and_object_offset_transformations.html">
		Node and Object Offset Transformations</a> for an overview of the object offset transformation.
		\param q The rotation offset. */
		virtual	void 	SetObjOffsetRot(Quat q)=0;
		
		/*! \remarks Returns the rotation portion of the object-offset
		from the node. See the Advanced Topics section on
		<a href="ms-its:3dsmaxsdk.chm::/trans_the_node_and_object_offset_transformations.html">
		Node and Object Offset Transformations</a> for an overview of the object offset transformation. */
		virtual	Quat 	GetObjOffsetRot()=0;
		
		/*! \remarks Sets the scale portion of the object-offset matrix.
		See the Advanced Topics section on
		<a href="ms-its:3dsmaxsdk.chm::/trans_the_node_and_object_offset_transformations.html">
		Node and Object Offset Transformations</a> for an overview of the object offset transformation.
		\param sv The scale portion of the offset. See Class ScaleValue. */
		virtual	void 	SetObjOffsetScale(ScaleValue sv)=0;
		
		/*! \remarks Returns the scale portion of the object-offset from
		the node. See Class ScaleValue. See the Advanced Topics section on
		<a href="ms-its:3dsmaxsdk.chm::/trans_the_node_and_object_offset_transformations.html">
		Node and Object Offset Transformations</a> for an overview of the object offset transformation. */
		virtual	ScaleValue GetObjOffsetScale()=0;
		
		// Resetting of object offset

		/*! \remarks Centers the pivot to the object.
			The effect is the same as Hierarchy Panel/Pivot - [Center to Object or Center to Pivot]
			\param [in] t - not used
			\param [in] moveObject - If true, the object is moved to be centered on the pivot.  
									 If false, the pivot is centered on the object*/
		virtual void 	CenterPivot(TimeValue t, BOOL moveObject)=0;

		/*! \remarks Resets the pivot alignment.
			The effect is the same as Hierarchy Panel/Pivot - [Align to Object or Align to Pivot]
			\param [in] t - not used
			\param [in] moveObject - If true, the object is rotated to be aligned with the pivot.  
									 If false, the pivot rotation is reset to be realigned with the object*/
		virtual void 	AlignPivot(TimeValue t, BOOL moveObject)=0;

		/*! \remarks Aligns the object to the world .
			The effect is the same as Hierarchy Panel/Pivot/[Affect Pivot Only or Affect Object Only]/ - Align to World
			\param [in] t - not used
			\param [in] moveObject - If true, the object is rotated to be aligned with the world.  
									 If false, the pivot rotation is set to be aligned with the world*/
		virtual void 	WorldAlignPivot(TimeValue t, BOOL moveObject)=0;

		/*! \remarks Aligns the pivot to the node's parent's pivot, or to world if no parent.
			\param [in] t - not used */
		virtual void 	AlignToParent(TimeValue t)=0;

		/*! \remarks Aligns the pivot to the to world
			\param [in] t - not used */
		virtual void 	AlignToWorld(TimeValue t)=0;

		/*! \remarks Resets the pivot offset transform.
			The effect is the same as Hierarchy Panel/Pivot/[Reset Transform or Reset Scale]
			\param [in] t - not used
			\param [in] scaleOnly - If true, reset only the offset scale.  Else reset the rotation and scale parts */
		virtual void	ResetTransform(TimeValue t,BOOL scaleOnly)=0;

		/*! \remarks Resets the pivot transform.
			The effect is the same as Hierarchy Panel/Pivot/Reset Pivot
			\param [in] t - not used */
		virtual void	ResetPivot(TimeValue t)=0;

		/*! \remarks Indicates whether a call to ResetTransform() will succeed. The ResetTransform() method 
			also checks this internally, so it's not necessary to check beforehand.
			\return true if ResetTransform will succeed, false if no change would occur */
		virtual bool    MayResetTransform ()=0;

		// Misc.
		/*! \remarks Flags the node to put it in the foreground. For
		additional information see
		<a href="ms-its:3dsmaxsdk.chm::/vports_foreground_background_planes.html">
		Foreground / Background Planes</a>.
		\param t The time to put the node in the foreground.
		\param notify If TRUE, the reference message \ref REFMSG_FLAGDEPENDENTS with <b>PART_PUT_IN_FG</b>
		 is sent. */
		virtual void 	FlagForeground(TimeValue t,BOOL notify=TRUE)=0;
		
		/*! \remarks Determines if this node is the active grid object.
		\return    Nonzero indicates the node is the active grid object; zero indicates it is not.     */
		virtual int 	IsActiveGrid()=0;

		// A place to hang temp data. Don't expect the data to stay around after you return control
		/*! \remarks This method provides temporary storage of data with the
		node. Data stored with the node is only valid before you return control.
		\param l The data to store with the node. */
		virtual void SetNodeLong(LONG_PTR l)=0;
		
		/*! \remarks Returns the value set by SetNodeLong */
		virtual LONG_PTR GetNodeLong()=0;

		// Access render data
		/*! \remarks Returns the render data for the node. See Class RenderData. */
		virtual RenderData *GetRenderData()=0;
		
		/*! \remarks Sets the render data for the node.
		\param rd The render data to set. For additional overview information on these methods, see
		<a href="ms-its:3dsmaxsdk.chm::/nodes_custom_node_properties.html">
		Custom node properties and application data </a>. */
		virtual void SetRenderData(RenderData *rd)=0;

		//
		// Access user defined property text
		//
		// The first two functions access the entire buffer
		/*! \remarks This method allows access to the entire user defined property text buffer.
		\param buf The buffer to hold the user defined property text. */
		virtual void GetUserPropBuffer(MSTR &buf)=0;
		
		/*! \remarks This method allows a developer to set to the entire user defined property text buffer.
		\param buf The buffer containing the user defined property text. */
		virtual void SetUserPropBuffer(const MSTR &buf)=0;

		// These get individual properties - return FALSE if the key is not found
		/*! \remarks This method retrieves a string based on the key passed.
		\param key The key (or name) of the user defined property text.
		\param string Storage for the string to retrieve.
		\return    TRUE if the key was found; otherwise FALSE. */
		virtual BOOL GetUserPropString(const MSTR &key,MSTR &string)=0;
		
		/*! \remarks Retrieves an integer value from the node based on the key passed.
		\param key The key (or name) of the data to retrieve.
		\param val Storage for the integer value.
		\return TRUE if the key was found; otherwise FALSE. */
		virtual BOOL GetUserPropInt(const MSTR &key,int &val)=0;
		
		/*! \remarks Retrieves a floating point value from the node based on the key passed.
		\param key The key (or name) of the data to retrieve.
		\param val Storage for the float value.
		\return TRUE if the key was found; otherwise FALSE. */
		virtual BOOL GetUserPropFloat(const MSTR &key,float &val)=0;
		
		/*! \remarks Retrieves a boolean value from the node based on the key passed.
		\param key The key (or name) of the data to retrieve.
		\param b Storage for the boolean value.
		\return TRUE if the key was found; otherwise FALSE. */
		virtual BOOL GetUserPropBool(const MSTR &key,BOOL &b)=0;
		
		// These set individual properties - create the key if it doesn't exist
		/*! \remarks Stores a string in the node using the key passed.
		If the key name already exists it is overwritten; otherwise it is created.
		\param key The key (or name) of the data to store.
		\param string The string to store. */
		virtual void SetUserPropString(const MSTR &key,const MSTR &string)=0;
		
		/*! \remarks Stores an integer value in the node using the key
		passed. If the key name already exists it is overwritten; otherwise it is created.
		\param key The key (or name) of the data to store.
		\param val The value to store. */
		virtual void SetUserPropInt(const MSTR &key,int val)=0;
		
		/*! \remarks Stores a floating point value in the node using the
		key passed. If the key name already exists it is overwritten; otherwise it is created.
		\param key The key (or name) of the data to store.
		\param val The value to store. */
		virtual void SetUserPropFloat(const MSTR &key,float val)=0;
		
		/*! \remarks Stores a boolean value in the node using the key
		passed. If the key name already exists it is overwritten; otherwise it is created.
		\param key The key (or name) of the data to store.
		\param b The value to store. */
		virtual void SetUserPropBool(const MSTR &key,BOOL b)=0;
		
		// Just checks to see if a key exists
		/*! \remarks This method simply checks to see if a key exists.
		\param key The key string to search for.
		\return TRUE if the key was found; otherwise FALSE. */
		virtual BOOL UserPropExists(const MSTR &key)=0;

		// G-Buffer ID's  (user settable)
		/*! \remarks Returns the G-Buffer ID of this node. This is the
		ID available in the <b>BMM_CHAN_NODE_ID</b> channel. See
		<a href="ms-its:3dsmaxsdk.chm::/bitmaps_root.html">Working with
		Bitmaps</a>(G-Buffer) for additional details. */
		virtual ULONG GetGBufID()=0;
		
		/*! \remarks Sets the G-Buffer ID of this node. This is the ID
		available in the <b>BMM_CHAN_NODE_ID</b> channel. See
		<a href="ms-its:3dsmaxsdk.chm::/bitmaps_root.html">Working with
		Bitmaps</a>(G-Buffer) for additional details.
		\param id The G-Buffer ID. */
		virtual void SetGBufID(ULONG id)=0;

		// G-Buffer Render ID's (set by renderer)
		/*! \remarks Returns the G-Buffer render ID of the node. This is set by
		the renderer during a video post render when the <b>BMM_CHAN_NODE_RENDER_ID</b> is requested. */
		virtual UWORD GetRenderID() { return 0xffff; }
		
		/*! \remarks Sets the G-Buffer render ID of the node. This is set by the
		renderer during a video post render when the <b>BMM_CHAN_NODE_RENDER_ID</b> is requested.
		The following methods deal with IK parameters associated with a node. */
		virtual void SetRenderID(UWORD id) {}

		// Node ID - Unique Handle
		/*! \remarks This method returns the unique node handle. Each node is assigned a unique node handle.
		\return Zero.
		\note For additional information regarding transformation matrices, see the Advanced Topics section
		on <a href="ms-its:3dsmaxsdk.chm::/trans_the_node_and_object_offset_transformations.html">
		Node and Object Offset Transformations</a>. */
		virtual ULONG GetHandle() { return kNullHandle; }

		// Transform the node about a specified axis system.
		// Either the pivot point or the object or both can be transformed.
		// Also, the children can be counter transformed so they don't move.
		/*! \remarks This method may be called to move the node about
		the specified axis system. Either the pivot point, or the geometry
		of the object, or both the pivot and the object may be transformed.
		Optionally, any children of the node can be counter transformed so
		they don't move.
		\param t The time to transform the node.
		\param tmAxis The axis system about which the node is transformed.
		\param val The amount of the transformation relative to the axis system.
		\param localOrigin If TRUE the transformation takes place about
		the nodes local origin; otherwise about the world origin.
		\param affectKids TRUE If TRUE any child nodes are transformed along
		with the parent node. If FALSE any children of the node are counter
		transformed so they don't move.
		\param pivMode One of the following values:\n\n
		<b>PIV_NONE</b>\n
		Move both the pivot point and the geometry of the object.\n\n
		<b>PIV_PIVOT_ONLY</b>\n
		Move the pivot point only.\n\n
		<b>PIV_OBJECT_ONLY</b>\n
		Move the geometry of the object only.\n
		\param ignoreLocks If TRUE any transform locks associated with the node are ignored; 
		otherwise the locks govern the application of the transformation. */
		virtual void Move(TimeValue t, const Matrix3& tmAxis, const Point3& val, BOOL localOrigin=FALSE, BOOL affectKids=TRUE, int pivMode=PIV_NONE, BOOL ignoreLocks=FALSE)=0;
		
		/*! \remarks This method may be called to rotate the node about
		the specified axis system. Either the pivot point, or the geometry
		of the object, or both the pivot and the object may be transformed.
		Optionally, any children of the node can be counter transformed so
		they don't rotate.
		\param t The time to transform the node.
		\param tmAxis The axis system about which the node is transformed.
		\param val The amount of the transformation.
		\param localOrigin If TRUE the transformation takes place about the nodes local origin; 
		otherwise about the world origin.
		\param affectKids If TRUE any child nodes are transformed along with the parent node. 
		If FALSE any children of the node are counter transformed so they don't rotate.
		\param pivMode One of the following values:\n\n
		<b>PIV_NONE</b>\n
		Move both the pivot point and the geometry of the object.\n\n
		<b>PIV_PIVOT_ONLY</b>\n
		Move the pivot point only.\n\n
		<b>PIV_OBJECT_ONLY</b>\n
		Move the geometry of the object only.\n
		\param ignoreLocks If TRUE any transform locks associated with the node are ignored; 
		otherwise the locks govern the application of the transformation. */
		virtual void Rotate(TimeValue t, const Matrix3& tmAxis, const AngAxis& val, BOOL localOrigin=FALSE, BOOL affectKids=TRUE, int pivMode=PIV_NONE, BOOL ignoreLocks=FALSE)=0;
		
		/*! \remarks This method may be called to rotate the node about
		the specified axis system. Either the pivot point, or the geometry
		of the object, or both the pivot and the object may be transformed.
		Optionally, any children of the node can be counter transformed so
		they don't rotate.
		\param t The time to transform the node.
		\param tmAxis The axis system about which the node is transformed.
		\param val The amount of the transformation.
		\param localOrigin If TRUE the transformation takes place about the nodes local origin; 
		otherwise about the world origin.
		\param affectKids If TRUE any child nodes are transformed along with the parent node. 
		If FALSE any children of the node are counter transformed so they don't rotate.
		\param pivMode One of the following values:\n\n
		<b>PIV_NONE</b>\n
		Move both the pivot point and the geometry of the object.\n\n
		<b>PIV_PIVOT_ONLY</b>\n
		Move the pivot point only.\n\n
		<b>PIV_OBJECT_ONLY</b>\n
		Move the geometry of the object only.\n
		\param ignoreLocks If TRUE any transform locks associated with the node are ignored; 
		otherwise the locks govern the application of the transformation. */
		virtual void Rotate(TimeValue t, const Matrix3& tmAxis, const Quat& val, BOOL localOrigin=FALSE, BOOL affectKids=TRUE, int pivMode=PIV_NONE, BOOL ignoreLocks=FALSE)=0;
		
		/*! \remarks This method may be called to scale the node about
		the specified axis system. Either the pivot point, or the geometry
		of the object, or both the pivot and the object may be transformed.
		Optionally, any children of the node can be counter transformed so
		they don't scale.
		\param t The time to transform the node.
		\param tmAxis The axis system about which the node is transformed.
		\param val The amount of the transformation.
		\param localOrigin If TRUE the transformation takes place about the nodes local origin; 
		otherwise about the world origin.
		\param affectKids If TRUE any child nodes are transformed along with the parent node. 
		If FALSE any children of the node are counter transformed so they don't scale.
		\param pivMode One of the following values:\n\n
		<b>PIV_NONE</b>\n
		Move both the pivot point and the geometry of the object.\n\n
		<b>PIV_PIVOT_ONLY</b>\n
		Move the pivot point only.\n\n
		<b>PIV_OBJECT_ONLY</b>\n
		Move the geometry of the object only.\n
		\param ignoreLocks If TRUE any transform locks associated with the node are ignored; 
		otherwise the locks govern the application of the transformation. */
		virtual void Scale(TimeValue t, const Matrix3& tmAxis, const Point3& val, BOOL localOrigin=FALSE, BOOL affectKids=TRUE, int pivMode=PIV_NONE, BOOL ignoreLocks=FALSE)=0;

		/*! \remarks Returns TRUE if this node is a member of a group; otherwise FALSE. */
		virtual BOOL IsGroupMember()=0;
		
		/*! \remarks Returns TRUE if this node is the head of a group; otherwise FALSE. */
		virtual BOOL IsGroupHead()=0;
		
		/*! \remarks Returns TRUE if this node is a member of an open group; otherwise FALSE.
		\par Default Implementation:
		<b>{return 0; }</b> */
		virtual BOOL IsOpenGroupMember() {return 0; }
		
		/*! \remarks Returns TRUE if this node is the head of a group and that group is open; otherwise FALSE.
		\par Default Implementation:
		<b>{return 0; }</b> */
		virtual BOOL IsOpenGroupHead() {return 0; }

		/*! \remarks A node is marked as a group member or not. This method sets this state.
		\param b TRUE to mark the node as a group member; FALSE to indicate it's not in a group. */
		virtual void SetGroupMember(BOOL b) {}
		
		/*! \remarks A node is marked as the group head or not. This method sets this state.
		\param b TRUE to mark the node as a group head; FALSE to indicate it's not a group head. */
		virtual void SetGroupHead(BOOL b) {}
		
		/*! \remarks A node is marked as an open group member or not. This method sets this state.
		\param b TRUE to mark the node as a open; FALSE to indicate it's not open. */
		virtual void SetGroupMemberOpen(BOOL b) {}
		
		/*! \remarks A node is marked as being the head of a group and being open or not. 
		This method sets this state.
		\param b TRUE to mark the node as an open group head; FALSE to indicate it's not an open group head. */
		virtual void SetGroupHeadOpen(BOOL b) {}

		// Some node IK params
		/*! \remarks This method returns the position weight for the node. */
		virtual float GetPosTaskWeight() {return 1.0f;}
		
		/*! \remarks This method returns the rotation weight for the node. */
		virtual float GetRotTaskWeight() {return 1.0f;}
		
		/*! \remarks This method sets the position weight for the node.
		\param w The position weight for the node. This value is \>= 0.0. */
		virtual void SetPosTaskWeight(float w) {}
		
		/*! \remarks This method sets the rotation weight for the node.
		\param w The rotation weight for the node. This value is \>= 0.0. */
		virtual void SetRotTaskWeight(float w) {}
		
		/*! \remarks Returns TRUE of FALSE to indicate if the specified axis is set for position or rotation.
		\param which Indicates if the method returns the position state or the rotation state:\n\n
		<b>0</b>: specifies position;\n
		<b>1</b>: specifies rotation.\n
		\param axis The axis to check. Values are as follows:\n\n
		<b>0</b>: specifies X\n
		<b>1</b>: specifies Y\n
		<b>2</b>: specifies Z. */
		virtual BOOL GetTaskAxisState(int which,int axis) {return TRUE;}
		
		/*! \remarks Sets the specified axis state for position or rotation.
		\param which Indicates if the method returns the position state or the rotation state:\n\n
		<b>0</b>: specifies position\n
		<b>1</b>: specifies rotation.\n
		\param axis The axis to check. Values are as follows:\n\n
		<b>0</b>: specifies X\n
		<b>1</b> specifies Y\n
		<b>2</b> specifies Z\n
		\param onOff TRUE for on; FALSE for off. */
		virtual void SetTaskAxisState(int which,int axis,BOOL onOff) {}
		
		/*! \remarks This method returns the same information as GetTaskAxisState() above.
		\return    The first three bits indicate position X, Y and Z. Then
		the next three bits indicate rotation X, Y, Z. */
		virtual DWORD GetTaskAxisStateBits() {return 127;}

		// Access to WSM Derived object. Note that there is at most one
		// WSM derived object per node. Calling CreateWSMDerivedObject()
		// will create a WSM derived object for the node if one doesn't 
		// already exist.
		/*! \remarks Calling this method will create a WSM derived object for
		this node if one doesn't already exist.
		\par Default Implementation:
		<b>{}</b> */
		virtual void CreateWSMDerivedObject() {}
		
		/*! \remarks This method pointer to the WSM Derived object for this
		node. Note that there is at most one WSM derived object per node.
		\par Default Implementation:
		<b>{return NULL;}</b> */
		virtual IDerivedObject *GetWSMDerivedObject() {return NULL;}
		
		// Scene XRef related methods. These methods are only implemented by root nodes.
		// Scene XRefs are stored as complete scenes with root nodes where the XRef scene root
		// node is a child of the current scene's root node.

		/*! \remarks Returns the file asset of the scene xref whose index is passed.
		\param i The zero based index of the scene xref (<b>0</b> to <b>GetXRefFileCount()-1</b>).
		<b>{return MaxSDK::AssetManagement::AssetUser();}</b> */
		CoreExport virtual MaxSDK::AssetManagement::AssetUser GetXRefFile(int i);
		
		/*! \remarks Sets the file asset of the scene xref whose index is passed.
		\param i The zero based index of the scene xref (<b>0</b> to <b>GetXRefFileCount()-1</b>).
		\param file The file to set.
		\param reload TRUE to reload; FALSE to not reload.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetXRefFile(int i,const MaxSDK::AssetManagement::AssetUser& file,BOOL reload) {}
		
		/*! \remarks Returns the number of scene xrefs.
		\par Default Implementation:
		<b>{return 0;}</b> */
		virtual int GetXRefFileCount() {return 0;}

		/*! \remarks  Adds the specified file to the scene and optionally updates the scene now.
		\param file The file asset to load.
		\param loadNow If TRUE the file is loaded immediately and the scene updated; 
		if FALSE the scene is now updated until the user requests it.
		\param hideInManagerUI If TRUE (default value is FALSE) the scene xref flag XREF_SCENE_HIDEINMANAGERUI will be set,
		so that the scene xref will not be listed in the scene XRef manager UI.
		\return    TRUE if the XRef was loaded; otherwise FALSE.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL AddNewXRefFile(const MaxSDK::AssetManagement::AssetUser &file, BOOL loadNow=TRUE, BOOL hideInManagerUI=FALSE) {return FALSE;}
		
		/*! \remarks This method removes the 'i-th' Scene XRef.
		\param i The zero based index of the Scene XRef to load.
		\return    TRUE if the file was deleted from the scene; otherwise FALSE.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL DeleteXRefFile(int i) {return FALSE;}
		/*! \remarks This methods binds the specified XRef. This deletes the
		XRef after merging it into the scene.
		\param i The zero based index of the XRef to bind.
		\return TRUE if the file was deleted; otherwise FALSE.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL BindXRefFile(int i) {return FALSE;}
		
		/*! \remarks This method deletes all the XRefs from the scene. This is
		called when loading a new file, reseting or clearing the scene.
		\par Default Implementation:
		<b>{}</b> */
		virtual void DeleteAllXRefs() {}
		
		/*! \remarks This method will reload (updates from disk) the specified XRef.
		\param i The zero based index of the XRef to reload.
		\return TRUE if the XRef was reloaded; otherwise FALSE.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL ReloadXRef(int i) {return FALSE;}
		
		/*! \remarks This method indicates that the specified XRef has been changed and should be updated.
		\param i The zero based index of the XRef to flag.
		\par Default Implementation:
		<b>{}</b> */
		virtual void FlagXrefChanged(int i) {}
		
		/*! \remarks This method updates all XRefs which have their changed flag set.
		\param redraw TRUE to redraw the scene; otherwise FALSE.
		\return TRUE if the XRefs were loaded okay; otherwise FALSE.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL UpdateChangedXRefs(BOOL redraw=TRUE) {return FALSE;}
		
		/*! \remarks Returns the root node of the tree for the specified XRef.
		This method, when called on a root node, will access the various
		XRef scenes. Note that these can be nested so calling this on the
		root node of the scene will return the root node of one of the
		scene XRefs. Calling it on the root node of the scene XRef will get
		the root node of a nested XRef and so on. Note that this is not the
		parent of the XRef (see <b>GetXRefParent()</b> below).
		\param i The zero based index of the XRef.
		\par Default Implementation:
		<b>{return NULL;}</b> */
		virtual INode *GetXRefTree(int i) {return NULL;}
		
		/*! \remarks Returns the parent node of the specified XRef. This is the
		node in the scene (if any) which the scene XRef is linked to
		through the special bind function in the scene XRef dialog.
		\param i The zero based index of the XRef.
		\par Default Implementation:
		<b>{return NULL;}</b> */
		virtual INode *GetXRefParent(int i) {return NULL;}
		
		/*! \remarks Sets the parent of the specified XRef to the node passed.
		\param i The zero based index of the XRef.
		\param par The parent node to set.
		\param autoOffset Automatically offset XRef so that it keeps its world TM.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetXRefParent(int i, INode *par, BOOL autoOffset = TRUE) {}
		
		/*! \remarks Generates a table of names for the unresolved XRefs in the scene.
		\param fnames The table of names. See Template Class Tab.
		\return Returns TRUE if there are still unresolved XRefs; otherwise FALSE.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL FindUnresolvedXRefs(Tab<MSTR*> &fnames) {return FALSE;}
		
		/*! \remarks This method tries to load any XRefs that are currently unresolved.
		\par Default Implementation:
		<b>{}</b> */
		virtual void AttemptToResolveUnresolvedXRefs() {}
		
		/*! \remarks Returns the state of flags for the specified XRef.
		\param i The zero based index of the XRef whose flags are returned.
		\return See \ref xrefFlagBits.
		\par Default Implementation:
		<b>{return 0;}</b> */
		virtual DWORD GetXRefFlags(int i) {return 0;}
		
		/*! \remarks Sets the state of the specified flags in the specified XRef to on or off.
		\param i The zero based index of the XRef whose flags are set.
		\param flag See \ref xrefFlagBits.
		\param onOff TRUE for on; FALSE for off.
		\par Default Implementation:
		<b>{}</b>        */
		virtual void SetXRefFlags(int i,DWORD flag,BOOL onOff) {}

		// New bones
		/*! \remarks Sets the bone on/off property of the node.
		\param onOff Pass TRUE for on; FALSE for off.
		\param t The time at which to set the property.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetBoneNodeOnOff(BOOL onOff, TimeValue t) {}
		
		/*! \remarks Sets the bone auto-align property of the node.
		\param onOff Pass TRUE for on; FALSE for off.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetBoneAutoAlign(BOOL onOff) {}
		
		/*! \remarks Sets the bone freeze length property of the node.
		\par onOff Pass TRUE for on; FALSE for off.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetBoneFreezeLen(BOOL onOff) {}
		
		/*! \remarks Sets the bone scale (stretch) type to the specified value.
		\param which One of the following values:\n\n
		<b>BONE_SCALETYPE_SCALE</b>\n
		<b>BONE_SCALETYPE_SQUASH</b>\n
		<b>BONE_SCALETYPE_NONE</b>
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetBoneScaleType(int which) {}
		
		/*! \remarks Sets the bone axis of the node to the specified value.
		\param which One of the following values:\n\n
		<b>BONE_AXIS_X</b>\n
		<b>BONE_AXIS_Y</b>\n
		<b>BONE_AXIS_Z</b>
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetBoneAxis(int which) {}
		
		/*! \remarks Sets the state of the bone axis flip toggle.
		\param onOff Pass TRUE for on; FALSE for off.
		\par Default Implementation:
		<b>{}</b> */
		virtual void SetBoneAxisFlip(BOOL onOff) {}
		
		/*! \remarks Returns TRUE if the bone property if on; FALSE if off.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL GetBoneNodeOnOff() {return FALSE;}
		
		virtual BOOL GetBoneNodeOnOff_T (TimeValue t) { return GetBoneNodeOnOff(); } // for write property access via MXS
		
		/*! \remarks Returns TRUE if the bone auto-align property is on; FALSE if off.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL GetBoneAutoAlign() {return FALSE;}
		
		/*! \remarks Returns TRUE if the freeze length property is on; FALSE if off.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL GetBoneFreezeLen() {return FALSE;}
		
		/*! \remarks Returns a value which indicates the bone scale type.
		\return One of the following values:\n\n
		<b>BONE_SCALETYPE_SCALE</b>\n
		<b>BONE_SCALETYPE_SQUASH</b>\n
		<b>BONE_SCALETYPE_NONE</b>
		\par Default Implementation:
		<b>{return 0;}</b> */
		virtual int GetBoneScaleType() {return 0;}
		
		/*! \remarks Returns a value which indicates the bone axis.
		\return One of the following values:\n\n
		<b>BONE_AXIS_X</b>\n
		<b>BONE_AXIS_Y</b>\n
		<b>BONE_AXIS_Z</b>
		\par Default Implementation:
		<b>{return 0;}</b> */
		virtual int GetBoneAxis() {return 0;}
		
		/*! \remarks Returns TRUE if the axis flip toggle is on; FALSE if off.
		\par Default Implementation:
		<b>{return FALSE;}</b> */
		virtual BOOL GetBoneAxisFlip() {return FALSE;}
		
		/*! \remarks Calling this method is the equivalent of pressing the Realign button in the UI.
		\param t The time at which to reset the initial child position.
		\par Default Implementation:
		<b>{}</b> */
		virtual void RealignBoneToChild(TimeValue t) {}
		
		/*! \remarks Calling this method is the equivalent of pressing the Reset
		Stretch button in the UI. This will cause the X-axis of the bone to
		realign to point at the child bone (or average pivot of multiple children).
		\param t The time at which to reset the bone stretch.
		\par Default Implementation:
		<b>{}</b> */
		virtual void ResetBoneStretch(TimeValue t) {}
		
		/*! \remarks This method returns the strechTM without the object offset
		included.   Normally matrix concatenation occurs in the following
		manner:   
		<b>objectTM = objectOffsetTM * stretchTM * nodeTM</b>   
		A plug-in that uses the objectTM will transparently inherit the
		effects of the stretchTM. However if a plug-in (such as skin) wants
		the stretchTM included but not the object offset, this method will
		return the stretchTM alone. If the node is not a bone or has no
		stretching, this method will return the identity.
		\param t The time at which to obtain the strechtTM.
		\param valid The interval.
		\par Default Implementation:
		<b>{return Matrix3(1);}</b> */
		virtual Matrix3 GetStretchTM(TimeValue t, Interval *valid=NULL) {return Matrix3(1);}

		// FunPub stuff
		BaseInterface* GetInterface(Interface_ID id) { return (id == INODE_INTERFACE) ? this : FPMixinInterface::GetInterface(id); }
		FPInterfaceDesc* GetDesc() { return (FPInterfaceDesc*)GetCOREInterface(INODE_INTERFACE); }

		// FP-published function IDs
		enum {  getPosTaskWeight, getRotTaskWeight, setPosTaskWeight, setRotTaskWeight, 
				// new bones
				setBoneNodeOnOff, setBoneNodeOnOffM, setBoneAutoAlign, setBoneFreezeLen, setBoneScaleType, getBoneNodeOnOff, 
				getBoneAutoAlign, getBoneFreezeLen, getBoneScaleType, realignBoneToChild, resetBoneStretch, getStretchTM,
				getBoneAxis,      getBoneAxisFlip,  setBoneAxis,      setBoneAxisFlip,
				// rendering flag access
				setPrimaryVisibility, getPrimaryVisibility, setSecondaryVisibility, getSecondaryVisibility, setApplyAtmospherics, getApplyAtmospherics,
				// vertex color access
				getVertexColorType, setVertexColorType, getCVertMode, setCVertMode, getShadeCVerts, setShadeCVerts,
				getNodeHandle,  
				// get/set map channel to be displayed as vertex color.
				getVertexColorMapChannel, setVertexColorMapChannel,

				// new Func IDs   >>>> >>>  MUST << <<<<  be inserted before kLastFPFuncID
				// !!!!!   that means that this MUST be the last enum value  !!!!!
				kLastFPFuncID
		};
		// FP-published symbolic enumerations
		enum {  boneScaleTypeEnum, boneAxisEnum, 
				vertexColorTypeEnum,
		};

		#pragma warning(push)
		#pragma warning(disable:4238)
		// dispatch map for FP-published functions
		BEGIN_FUNCTION_MAP
			PROP_FNS(getPosTaskWeight, GetPosTaskWeight, setPosTaskWeight, SetPosTaskWeight, TYPE_FLOAT); 
			PROP_FNS(getRotTaskWeight, GetRotTaskWeight, setRotTaskWeight, SetRotTaskWeight, TYPE_FLOAT); 
			// new bones props & functions
			PROP_FNS(getBoneAutoAlign, GetBoneAutoAlign, setBoneAutoAlign, SetBoneAutoAlign, TYPE_BOOL); 
			PROP_FNS(getBoneFreezeLen, GetBoneFreezeLen, setBoneFreezeLen, SetBoneFreezeLen, TYPE_BOOL); 
			PROP_FNS(getBoneScaleType, GetBoneScaleType, setBoneScaleType, SetBoneScaleType, TYPE_ENUM); 
			PROP_FNS(getBoneAxis,      GetBoneAxis,      setBoneAxis,      SetBoneAxis,      TYPE_ENUM); 
			PROP_FNS(getBoneAxisFlip,  GetBoneAxisFlip,  setBoneAxisFlip,  SetBoneAxisFlip,  TYPE_BOOL); 
			RO_PROP_TFN(getStretchTM, GetStretchTM, TYPE_MATRIX3_BV); 
			PROP_TFNS(getBoneNodeOnOff, GetBoneNodeOnOff_T, setBoneNodeOnOff, SetBoneNodeOnOff, TYPE_BOOL); 
			VFNT_1(setBoneNodeOnOffM, SetBoneNodeOnOff, TYPE_BOOL);
			VFNT_0(realignBoneToChild, RealignBoneToChild);
			VFNT_0(resetBoneStretch, ResetBoneStretch);
			// rendering flag access
			PROP_FNS(getPrimaryVisibility, GetPrimaryVisibility, setPrimaryVisibility, SetPrimaryVisibility, TYPE_BOOL); 
			PROP_FNS(getSecondaryVisibility, GetSecondaryVisibility, setSecondaryVisibility, SetSecondaryVisibility, TYPE_BOOL); 
			PROP_FNS(getApplyAtmospherics, ApplyAtmospherics, setApplyAtmospherics, SetApplyAtmospherics, TYPE_BOOL); 
			PROP_FNS(getCVertMode, GetCVertMode, setCVertMode, SetCVertMode, TYPE_INT);
			PROP_FNS(getShadeCVerts, GetShadeCVerts, setShadeCVerts, SetShadeCVerts, TYPE_INT);
			PROP_FNS(getVertexColorType, GetVertexColorType, setVertexColorType, SetVertexColorType, TYPE_ENUM);
			// get/set map channel to be displayed as vertex color.
			PROP_FNS(getVertexColorMapChannel, GetVertexColorMapChannel, setVertexColorMapChannel, SetVertexColorMapChannel, TYPE_INT);
			RO_PROP_FN(getNodeHandle, GetHandle, TYPE_DWORD);
		END_FUNCTION_MAP
		#pragma warning(pop)
		/*! \remarks Calling this method copies the various display, rendering,
		bone and other general parameters from the "from" object. This is
		used in Edit(able) Spline and Edit(able) Patch to copy node
		settings into new nodes created by the "Detach" operations.
		\par Parameters:
		<b>INode *frame</b>   A pointer to the INode to copy the properties
		from. */
		CoreExport void CopyProperties(INode *from);			
};		

/*! \defgroup xrefFlagBits XRef Flag Bits
These flags are used by INode::SetXRefFlags and INode::GetXRefFlags
\sa Class INode */
//@{
#define XREF_UPDATE_AUTO		(1<<0)	//!< Automatic XRef file updating is ON.
#define XREF_BOX_DISP		(1<<1)	//!< The Box display option is set.
#define XREF_HIDDEN			(1<<2)	//!< The XRef is hidden.
#define XREF_DISABLED		(1<<3)	//!< The XRef is disabled.
#define XREF_IGNORE_LIGHTS	(1<<4)	//!< The XRef ignores lights in the file.
#define XREF_IGNORE_CAMERAS	(1<<5)	//!< The XRef ignores cameras in the file.
#define XREF_IGNORE_SHAPES	(1<<6)	//!< The XRef ignores shapes in the file.
#define XREF_IGNORE_HELPERS	(1<<7)	//!< The XRef ignores helpers in the file.
#define XREF_IGNORE_ANIM		(1<<8)	//!< The XRef ignores the animation in the file.
/*! It is not certain that the file has actually changed but the XRef should be reloaded. */
#define XREF_FILE_CHANGE		(1<<10)	//!< Is set when a change notification is sent indicating that the file may have changed. 
#define XREF_LOAD_ERROR		(1<<11)	//!< Is set when an XRef can not be resolved.
/*! An overlay XRef will be loaded only if it is a direct descendant of the master file.  This lets the user bring in files to compare against but other users do not see it if they XRef this file. */
#define XREF_SCENE_OVERLAY	(1<<12)	//!< Is set when a scene XRef is set to overlay.
#define XREF_SCENE_HIDEINMANAGERUI	(1<<13)	//!< Is set when a scene XRef is not displayed in scene XREF manager UI. Is set at creation.
//@}						

// Return values from GetBoneScaleType()
#define BONE_SCALETYPE_SCALE	1
#define BONE_SCALETYPE_SQUASH	2
#define BONE_SCALETYPE_NONE	0

// Bone axis
#define BONE_AXIS_X		0
#define BONE_AXIS_Y		1
#define BONE_AXIS_Z		2


/*! \defgroup transformLockTypes Transform Lock Types
\sa class INode */
//@{
#define INODE_LOCKPOS		0	//!< Position locked
#define INODE_LOCKROT		1	//!< Rotate locked
#define INODE_LOCKSCL		2	//!< Scale locked.
//@}

/*! \defgroup transformLockAxis Transform Lock Axis
\sa Class INode */
//@{
#define INODE_LOCK_X		0	//!< The X-axis is locked.
#define INODE_LOCK_Y		1	//!< The Y-axis is locked.
#define INODE_LOCK_Z		2	//!< The Z-axis is locked.
//@}

// Derive a class from this class, implementing the callback.
/*! \sa  Class IScene, Class INode.\n\n
\par Description:
This is the callback object used by <b>IScene::EnumTree()</b>. To use it,
derive a class from this class, and implement the callback method.  */
class ITreeEnumProc: public MaxHeapOperators {
	public:
		virtual ~ITreeEnumProc() { }
		/*! \remarks This method may flag the node passed based on some property of the
		node.
		\param node The node. The <b>INode</b> class has a method <b>FlagForeground()</b>
		that may be used to flag this node to go into the foreground.
		\return  One of the following values may be returned to control how
		enumeration continues:\n\n
		<b>TREE_CONTINUE</b>\n
		Continue enumerating.\n\n
		<b>TREE_IGNORECHILDREN</b>\n
		Don't enumerate the children of this node, but continue
		enumerating.\n\n
		<b>TREE_ABORT</b>\n
		Stop enumerating. */
		virtual int callback( INode *node )=0;
	};

// Return values for the TreeEnum callback:
#define TREE_CONTINUE			0	// Continue enumerating
#define TREE_IGNORECHILDREN		1	// Don't enumerate children, but continue
#define TREE_ABORT				2	// Stop enumerating

// Node properties:
#define PROPID_PINNODE		PROPID_USER+1  	// Returns a pointer to the node this node is pinned to
#define PROPID_PRECEDENCE	PROPID_USER+2	// Returns an integer representing this node's precedence
#define PROPID_RELPOS		PROPID_USER+3	// Returns a pointer to the relative vector between the node and its pin
#define PROPID_RELROT		PROPID_USER+4	// Returns a pointer to the relative quaternion between the node and its pin



class INodeTransformed;

// INodeTransformed can be allocated on the stack, but if you need
// to create one dynamically, use these methods.
/*! \remarks Deletes the INodeTransformed passed.
\par Parameters:
<b>INodeTransformed *n</b>\n\n
The INodeTransformed to delete. */
CoreExport void DeleteINodeTransformed(INodeTransformed* n);
/*! \remarks Creates an INodeTransformed on the heap.
\par Parameters:
<b>INode *n</b>\n\n
The original INode pointer.\n\n
<b>Matrix3 tm</b>\n\n
The additional transformation matrix.\n\n
<b>BOOL dm=TRUE</b>\n\n
If TRUE this item will be deleted; otherwise it is left alone.
\return  A pointer to the INodeTransformed created. */
CoreExport INodeTransformed* CreateINodeTransformed(INode* n, Matrix3 tm, BOOL dm = TRUE);

// This class provides a layer that will add in a transformation to the
// node's objectTM.
//
// Most methods pass through to the inode, except for the objectTM methods
// which pre-multiply in the given matrix.
//
/*! \sa  Class INode, <a href="ms-its:3dsmaxsdk.chm::/mods_modifier_stack_branching.html">Modifier Stack Branching</a>
\par Description:
This class provides a layer that will add in a transformation to the node's
objectTM. Most methods pass through to the INode, except for the objectTM
methods which pre-multiply in the given matrix. The methods of this class are
the same as INode. See Class INode for details.
Specifically see the methods related to INodeTransformed in
<a href="class_i_node.html#A_GM_inode_trf">INode - INodeTransformed methods</a>. 
All methods of this class are implemented by the system.
\par Data Members:
<b>INode *node;</b>\n\n
The original INode pointer.\n\n
<b>Matrix3 tm;</b>\n\n
The additional transformation.\n\n
<b>BOOL deleteMe;</b>\n\n
If set to FALSE this <b>INodeTransformed</b> will not get deleted. This may be
used if the object is not allocated dynamically. If you create an
<b>INodeTransformed</b> on the stack you'll want to set <b>deleteMe</b> to
FALSE.  */
class INodeTransformed : public INode {
	public:
		INode *node;
		Matrix3 tm;
		BOOL deleteMe;

		INodeTransformed(INode *n,Matrix3 tm,BOOL dm=TRUE) {node = n;this->tm = tm;deleteMe = dm;}
		
		/*! \remarks Deletes this <b>INodeTransformed</b>. */
		void DisposeTemporary() {node->DisposeTemporary(); if (deleteMe) DeleteINodeTransformed(this);}
		/*! \remarks Returns the actual <b>INode</b> pointer of this
		<b>INodeTransformed</b>.\n\n
		These functions are not part of this class but are available for
		use:\n\n
		INodeTransformed can be allocated on the stack, but if you need to
		create one dynamically, use these methods to create and delete them. */
		INode *GetActualINode() {return node->GetActualINode();}
		
		MCHAR* 	GetName() {return node->GetName();}
		void	SetName(MCHAR *s) {node->SetName(s);}
		Matrix3	GetNodeTM(TimeValue t, Interval* valid=NULL) {return node->GetNodeTM(t,valid);}
		void 	SetNodeTM(TimeValue t, Matrix3& tm) {node->SetNodeTM(t,tm);}
		void InvalidateTreeTM() {node->InvalidateTreeTM();}
		void InvalidateTM() {node->InvalidateTM();}
		void InvalidateWS() {node->InvalidateWS();}
		void InvalidateRect( TimeValue t, bool oldRect) { node->InvalidateRect(t,oldRect); }
		Matrix3 GetObjectTM(TimeValue time, Interval* valid=NULL) {return tm*node->GetObjectTM(time,valid);}
		Matrix3 GetObjTMBeforeWSM(TimeValue time, Interval* valid=NULL) {return tm*node->GetObjTMBeforeWSM(time,valid);}
		Matrix3 GetObjTMAfterWSM(TimeValue time, Interval* valid=NULL) {return tm*node->GetObjTMAfterWSM(time,valid);}
		const ObjectState& EvalWorldState(TimeValue time,BOOL evalHidden=TRUE) {return node->EvalWorldState(time,evalHidden);}
		INode* 	GetParentNode() {return node->GetParentNode();}
		void 	AttachChild(INode* node, int keepTM=1) {node->AttachChild(node,keepTM);}
		void 	Detach(TimeValue t, int keepTM=1) {node->Detach(t,keepTM);}
		int 	NumberOfChildren() {return node->NumberOfChildren();}
		INode* 	GetChildNode(int i) {return node->GetChildNode(i);}
		void    Delete(TimeValue t, int keepChildPosition) { node->Delete(t,keepChildPosition); } 
		void	Hide(BOOL onOff) {node->Hide(onOff);}
		int		IsHidden(DWORD hflags=0,BOOL forRenderer=FALSE) {return node->IsHidden(hflags,forRenderer);}
		int		IsNodeHidden(BOOL forRenderer=FALSE) { return node->IsNodeHidden(forRenderer); }
		void	Freeze(BOOL onOff) {node->Freeze(onOff);}
		int		IsFrozen() {return node->IsFrozen();}
		void	SetShowFrozenWithMtl(BOOL onOff) {node->SetShowFrozenWithMtl(onOff);}
		int		ShowFrozenWithMtl() {return node->ShowFrozenWithMtl();}
		void	XRayMtl(BOOL onOff) {node->XRayMtl(onOff);}
		int		HasXRayMtl() {return node->HasXRayMtl();}
		void	IgnoreExtents(BOOL onOff) {node->IgnoreExtents(onOff);}
		int		GetIgnoreExtents() {return node->GetIgnoreExtents();}
		void	BoxMode(BOOL onOff) {node->BoxMode(onOff);}
		int		GetBoxMode() {return node->GetBoxMode();}
		void	AllEdges(BOOL onOff) {node->AllEdges(onOff);}
		int		GetAllEdges() {return node->GetAllEdges();}
		void	VertTicks(int onOff) {node->VertTicks(onOff);}
		int		GetVertTicks() {return node->GetVertTicks();}
		void	BackCull(BOOL onOff) {node->BackCull(onOff);}
		int		GetBackCull() {return node->GetBackCull();}
		void 	SetCastShadows(BOOL onOff) { node->SetCastShadows(onOff); } 
		int		CastShadows() { return node->CastShadows(); }
		void 	SetRcvShadows(BOOL onOff) { node->SetRcvShadows(onOff); }
		int		RcvShadows() { return node->RcvShadows(); }
		void 	SetGenerateCaustics(BOOL onOff) { node->SetGenerateCaustics(onOff); } 
		int		GenerateCaustics() { return node->GenerateCaustics(); }
		void 	SetRcvCaustics(BOOL onOff) { node->SetRcvCaustics(onOff); }
		int		RcvCaustics() { return node->RcvCaustics(); }
		void	SetApplyAtmospherics(BOOL onOff) { node->SetApplyAtmospherics(onOff); }
		int		ApplyAtmospherics() { return node->ApplyAtmospherics(); }
		void 	SetGenerateGlobalIllum(BOOL onOff) { node->SetGenerateGlobalIllum(onOff); } 
		int		GenerateGlobalIllum() { return node->GenerateGlobalIllum(); }
		void 	SetRcvGlobalIllum(BOOL onOff) { node->SetRcvGlobalIllum(onOff); }
		int		RcvGlobalIllum() { return node->RcvGlobalIllum(); }

		void 	SetMotBlur(BOOL onOff) { node->SetMotBlur(onOff); }
		int		MotBlur() { return node->MotBlur(); }

		float   GetImageBlurMultiplier(TimeValue t) { return node->GetImageBlurMultiplier(t);}
		void    SetImageBlurMultiplier(TimeValue t, float m) {node->SetImageBlurMultiplier(t,m); };
		void  	SetImageBlurMultController(Control *cont){ node->SetImageBlurMultController(cont); }
		Control *GetImageBlurMultController() {return node->GetImageBlurMultController(); }

		// Object motion blur enable controller. This affects only object motion blur
		BOOL GetMotBlurOnOff(TimeValue t) { return node->GetMotBlurOnOff(t); }
		void  SetMotBlurOnOff(TimeValue t, BOOL m) { node->SetMotBlurOnOff(t,m); }
		Control *GetMotBlurOnOffController() { return node->GetMotBlurOnOffController();}
		void SetMotBlurOnOffController(Control *cont) { node->SetMotBlurOnOffController(cont);}

		void 	SetRenderable(BOOL onOff) { node->SetRenderable(onOff); }
		int		Renderable() { return node->Renderable(); }

		void	SetPrimaryVisibility(BOOL onOff) { node->SetPrimaryVisibility(onOff); }
		int		GetPrimaryVisibility() { return node->GetPrimaryVisibility(); }
		void	SetSecondaryVisibility(BOOL onOff) { node->SetSecondaryVisibility(onOff); }
		int		GetSecondaryVisibility() { return node->GetSecondaryVisibility(); }

		void 	ShowBone(int boneVis) {node->ShowBone(boneVis);}
		void	BoneAsLine(int onOff) {node->BoneAsLine(onOff);}
		BOOL	IsBoneShowing() {return node->IsBoneShowing();}
		BOOL	IsBoneOnly() { return node->IsBoneOnly(); }
		DWORD 	GetWireColor() {return node->GetWireColor();}
		void 	SetWireColor(DWORD newcol) {node->SetWireColor(newcol);}
		int 	IsRootNode() {return node->IsRootNode();}
		int 	Selected() {return node->Selected();}
		int  	Dependent() {return node->Dependent();}
		int 	IsTarget() {return node->IsTarget();}
		void  	SetIsTarget(BOOL b) { node->SetIsTarget(b);}
		BOOL 	GetTransformLock(int type, int axis) {return node->GetTransformLock(type,axis);}
		void 	SetTransformLock(int type, int axis, BOOL onOff) {node->SetTransformLock(type,axis,onOff);}
		INode* 	GetTarget() {return node->GetTarget();}
		INode* 	GetLookatNode() {return node->GetLookatNode();}
		Matrix3 GetParentTM(TimeValue t) {return node->GetParentTM(t);}
		int 	GetTargetTM(TimeValue t, Matrix3& m) {return node->GetTargetTM(t,m);}
		Object* GetObjectRef() {return node->GetObjectRef();}
		void 	SetObjectRef(Object *o) {node->SetObjectRef(o);}
		Object* GetObjOrWSMRef() { return node->GetObjOrWSMRef();}  
		Control* GetTMController() {return node->GetTMController();}
		BOOL 	SetTMController(Control *m3cont) { return node->SetTMController(m3cont);}		
		Control *GetVisController() {return node->GetVisController();}
		void    SetVisController(Control *cont) {node->SetVisController(cont);}
		float   GetVisibility(TimeValue t,Interval *valid=NULL) {return node->GetVisibility(t,valid);}
		void	SetVisibility(TimeValue t,float vis) { node->SetVisibility(t,vis); }
		float   GetLocalVisibility(TimeValue t,Interval *valid) { return node->GetLocalVisibility(t,valid); }
		BOOL 	GetInheritVisibility() { return node->GetInheritVisibility(); }
		void 	SetInheritVisibility(BOOL onOff) { node->SetInheritVisibility(onOff); }

		virtual void  SetRenderOccluded(BOOL onOff) { node->SetRenderOccluded(onOff); }
		virtual BOOL  GetRenderOccluded(){ return node->GetRenderOccluded(); }
		
		Mtl *GetMtl() { return node->GetMtl(); }
		void SetMtl(Mtl* matl) { node->SetMtl(matl); }

		Material* Mtls() { return node->Mtls(); }    
		int 	NumMtls() { return node->NumMtls(); }

		RenderData *GetRenderData() {return node->GetRenderData();}
		void SetRenderData(RenderData *rd) {node->SetRenderData(rd);}

		void 	SetObjOffsetPos(Point3 p) {node->SetObjOffsetPos(p);}
		Point3 	GetObjOffsetPos() {return node->GetObjOffsetPos();}
		void 	SetObjOffsetRot(Quat q) {node->SetObjOffsetRot(q);}
		Quat 	GetObjOffsetRot() {return node->GetObjOffsetRot();}		
		void 	FlagForeground(TimeValue t,BOOL notify=TRUE) {node->FlagForeground(t,notify);}
		int 	IsActiveGrid() {return node->IsActiveGrid();}
		void SetNodeLong(LONG_PTR l) {node->SetNodeLong(l);}
		LONG_PTR GetNodeLong() {return node->GetNodeLong();}

		void GetUserPropBuffer(MSTR &buf) {node->GetUserPropBuffer(buf);}
		void SetUserPropBuffer(const MSTR &buf) {node->SetUserPropBuffer(buf);}
		BOOL GetUserPropString(const MSTR &key,MSTR &string) {return node->GetUserPropString(key,string);}
		BOOL GetUserPropInt(const MSTR &key,int &val) {return node->GetUserPropInt(key,val);}
		BOOL GetUserPropFloat(const MSTR &key,float &val) {return node->GetUserPropFloat(key,val);}
		BOOL GetUserPropBool(const MSTR &key,BOOL &b) {return node->GetUserPropBool(key,b);}
		void SetUserPropString(const MSTR &key,const MSTR &string) {node->SetUserPropString(key,string);}
		void SetUserPropInt(const MSTR &key,int val) {node->SetUserPropInt(key,val);}
		void SetUserPropFloat(const MSTR &key,float val) {node->SetUserPropFloat(key,val);}
		void SetUserPropBool(const MSTR &key,BOOL b) {node->SetUserPropBool(key,b);}
		BOOL UserPropExists(const MSTR &key) {return node->UserPropExists(key);}
		ULONG GetGBufID() { return node->GetGBufID(); }
		void SetGBufID(ULONG id) { node->SetGBufID(id); }

		UWORD GetRenderID() { return node->GetRenderID(); }
		void SetRenderID(UWORD id) { node->SetRenderID(id); }

		CoreExport void 	SetObjOffsetScale(ScaleValue sv);
		CoreExport ScaleValue GetObjOffsetScale();

		void CenterPivot(TimeValue t, BOOL moveObject) { node->CenterPivot(t,moveObject); }
		void AlignPivot(TimeValue t, BOOL moveObject) { node->AlignPivot(t,moveObject); }
		void WorldAlignPivot(TimeValue t, BOOL moveObject) { node->WorldAlignPivot(t,moveObject); }
		void AlignToParent(TimeValue t) { node->AlignToParent(t); }
		void AlignToWorld(TimeValue t) { node->AlignToWorld(t); }
		void ResetTransform(TimeValue t,BOOL scaleOnly) { node->ResetTransform(t,scaleOnly); }
		void ResetPivot(TimeValue t) { node->ResetPivot(t); }
		bool MayResetTransform () { return node->MayResetTransform(); }

		void Move(TimeValue t, const Matrix3& tmAxis, const Point3& val, BOOL localOrigin=FALSE, BOOL affectKids=TRUE, int pivMode=PIV_NONE, BOOL ignoreLocks=FALSE) {node->Move(t,tmAxis,val,localOrigin,pivMode,ignoreLocks);}
		void Rotate(TimeValue t, const Matrix3& tmAxis, const AngAxis& val, BOOL localOrigin=FALSE, BOOL affectKids=TRUE, int pivMode=PIV_NONE, BOOL ignoreLocks=FALSE) {node->Rotate(t,tmAxis,val,localOrigin,pivMode,ignoreLocks);}
		void Rotate(TimeValue t, const Matrix3& tmAxis, const Quat& val, BOOL localOrigin=FALSE, BOOL affectKids=TRUE, int pivMode=PIV_NONE, BOOL ignoreLocks=FALSE) {node->Rotate(t,tmAxis,val,localOrigin,pivMode,ignoreLocks);}
		void Scale(TimeValue t, const Matrix3& tmAxis, const Point3& val, BOOL localOrigin=FALSE, BOOL affectKids=TRUE, int pivMode=PIV_NONE, BOOL ignoreLocks=FALSE) {node->Scale(t,tmAxis,val,localOrigin,pivMode,ignoreLocks);}

		BOOL IsGroupMember() {return node->IsGroupMember();}
		BOOL IsGroupHead() { return node->IsGroupHead();}
		BOOL IsOpenGroupMember(){return node->IsOpenGroupMember();}
		BOOL IsOpenGroupHead(){return node->IsOpenGroupHead();}

		void SetGroupMember(BOOL b) { node->SetGroupMember(b); }
		void SetGroupHead(BOOL b) { node->SetGroupHead(b); }
		void SetGroupMemberOpen(BOOL b) { node->SetGroupMemberOpen(b); }
		void SetGroupHeadOpen(BOOL b) { node->SetGroupHeadOpen(b); }

		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
		PartID& partID,  RefMessage message) {return REF_SUCCEED;}
		void CopyProperties(INode *from) {node->CopyProperties(from);}
	};

#pragma warning(pop)

