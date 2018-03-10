/**********************************************************************
 *<
	FILE: impapi.h

	DESCRIPTION: Geometry import/export API header

	CREATED BY:	Tom Hudson

	HISTORY: Created 26 December 1994

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxheap.h"
// These includes get us the general camera and light interfaces
#include "gencam.h"
#include "genlight.h"

#include "render.h"

// Import Node class

/*! \sa  Class ImpInterface, Class INode,  Class Matrix3,  Class Point3.\n\n
\par Description:
Import Node class. Methods of this class may be used to set various properties
of the node. All methods of this class are implemented by the system.
\par Sample Code:
The following sample code fragment (from
<b>/MAXSDK/SAMPLES/IMPEXP/DXFIMP.CPP</b>) demonstrates the use of many of the
methods of this class.\n\n
\code
ImpNode *node = iface->CreateNode();
if (node)
{
	TriObject *tri = CreateNewTriObject();
// Now find the center of the vertices and use that as the 	pivot
	int verts = m->getNumVerts();
	Point3 accum(0,0,0);
	for(int i = 0; i < verts; ++i)
		accum += m->verts[i];
	Point3 delta = accum / (float)verts;
	for(i = 0; i < verts; ++i)
		m->verts[i] -= delta;
	tri->mesh = *m;
	node->Reference(tri);
	Matrix3 tm;
	tm.IdentityMatrix();						  // Reset initial matrix to identity
	tm.SetTrans(delta);							  // Add in the center point
	node->SetTransform(0,tm);
	iface->AddNodeToScene(node);
	node->SetName(n->name);
}
\endcode */
class ImpNode: public MaxHeapOperators {
public:
	/*! \remarks Sets the object that this node references.
	\par Parameters:
	<b>ObjectHandle obj</b>\n\n
	The object to reference.
	\return  One of the following values:\n\n
	<b> REF_FAIL</b>\n\n
	The operation failed.\n\n
	<b> REF_SUCCEED</b>\n\n
	The operation succeeded. */
	virtual RefResult	Reference(ObjectHandle obj)	= 0;
	/*! \remarks Sets the transformation matrix of the node.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the matrix.\n\n
	<b>Matrix3 tm</b>\n\n
	The new transformation matrix of the node. */
	virtual void		SetTransform( TimeValue t, Matrix3 tm ) = 0;
	/*! \remarks Sets the name of the node.
	\par Parameters:
	<b>const MCHAR *newname</b>\n\n
	The new name for the node. */
	virtual void 		SetName(const MCHAR *newname) = 0;
	/*! \remarks Sets the pivot point of the node.
	\par Parameters:
	<b>Point3 p</b>\n\n
	The pivot point of the node. */
	virtual void		SetPivot(Point3 p) = 0;
	/*! \remarks Returns the INode pointer for the node. */
	virtual INode *		GetINode()=0;			// Use with care -- Always use above methods instead
	// I'm stuffing these in here so that I can perhaps add API functions without recompiling
	virtual int			TempFunc1(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int			TempFunc2(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int			TempFunc3(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int			TempFunc4(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int			TempFunc5(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int			TempFunc6(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int			TempFunc7(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int			TempFunc8(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int			TempFunc9(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int			TempFunc10(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	};

// Import Interface class

/*! \sa  Class ImpNode, Class Interval, Class_ID, Class GenLight, Class GenCamera.\n\n
\par Description:
Import Interface class. Methods of this class allow the plug-in to create nodes
in the scene, create camera and light objects, and create objects by specifying
a super class ID and Class ID. Methods are also available to bind target nodes
to their Look At nodes. All methods of this class are implemented by the
system.  */
class ImpInterface: public MaxHeapOperators {
public:
	virtual ~ImpInterface() {}
	/*! \remarks Creates a new node. Methods of <b>ImpNode</b> may be used to
	assign properties to the node. See <b>AddNodeToScene()</b> to add a node to
	the scene given its <b>ImpNode</b> pointer.
	\return  An <b>ImpNode</b> pointer that may be used to set properties of
	the node. */
	virtual ImpNode *		CreateNode() = 0;
	/*! \remarks Redraws the 3ds Max viewports. */
	virtual void 			RedrawViews() = 0;
	/*! \remarks Creates a camera object and returns a pointer to it. The
	GenCamera pointer may be used to set the properties of the camera object.
	\par Parameters:
	<b>int type</b>\n\n
	One of the following values:\n\n
	<b>FREE_CAMERA</b>\n\n
	<b>TARGETED_CAMERA</b> */
	virtual GenCamera*	 	CreateCameraObject(int type) = 0;
	/*! \remarks Creates a target object and returns a pointer to it. */
	virtual Object *   		CreateTargetObject() = 0;
	/*! \remarks Creates a light object and returns a pointer to it. The
	GenLight pointer may be used to set the properties of the light object.
	\par Parameters:
	<b>int type</b>\n\n
	One of the following values:\n\n
	<b>OMNI_LIGHT</b> - Omnidirectional\n\n
	<b>TSPOT_LIGHT</b> - Targeted\n\n
	<b>DIR_LIGHT</b> - Directional\n\n
	<b>FSPOT_LIGHT</b> - Free */
	virtual GenLight*	 	CreateLightObject(int type) = 0;
	/*! \remarks Creates an object given its Super Class ID and its Class ID.
	\par Parameters:
	<b>SClass_ID sclass</b>\n\n
	The super class ID of the node to create.\n\n
	<b>Class_ID classid</b>\n\n
	The unique class ID of the node to create.
	\return  A pointer to the item. */
	virtual void *			Create(SClass_ID sclass, Class_ID classid)=0;
	/*! \remarks This method binds a node to a target using a Look At
	controller. This is typically used with target spotlights and cameras to
	bind them to their target node.
	\par Parameters:
	<b>ImpNode *laNode</b>\n\n
	The node that will have the Look At controller assigned.\n\n
	<b>ImpNode *targNode</b>\n\n
	The target node.
	\return  Nonzero if successful; otherwise 0. */
	virtual int 			BindToTarget(ImpNode *laNode, ImpNode *targNode)=0;
	/*! \remarks Adds a node to the scene given its ImpNode pointer.
	\par Parameters:
	<b>ImpNode *node</b>\n\n
	The node to add to the scene. */
	virtual void			AddNodeToScene(ImpNode *node)=0;
	/*! \remarks Sets the animation range for the node.
	\par Parameters:
	<b>Interval\& range</b>\n\n
	Specifies the animation range. */
	virtual void			SetAnimRange(Interval& range)=0;
	/*! \remarks Retrieves the animation range for the node (as an Interval).
	*/
	virtual Interval		GetAnimRange()=0;
	// Environment settings
	/*! \remarks Sets the current environment map to the specified map.
	\par Parameters:
	<b>Texmap *txm</b>\n\n
	The map to set. */
	virtual void 			SetEnvironmentMap(Texmap *txm)=0;
	/*! \remarks Sets the ambient light color at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the color.\n\n
	<b>Point3 col</b>\n\n
	The light color to set. */
	virtual void 			SetAmbient(TimeValue t, Point3 col)=0;
	/*! \remarks Sets the background color at the specified time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time to set the color.\n\n
	<b>Point3 col</b>\n\n
	The light color to set. */
	virtual void 			SetBackGround(TimeValue t,Point3 col)=0;
	/*! \remarks Sets the state of the environment 'Use Map' toggle.
	\par Parameters:
	<b>BOOL onoff</b>\n\n
	TRUE to turn on; FALSE to turn off. */
	virtual void 			SetUseMap(BOOL onoff)=0;
	/*! \remarks Adds the specified atmospheric effect to the environment.
	\par Parameters:
	<b>Atmospheric *atmos</b>\n\n
	The atmospheric effect. See Class Atmospheric. */
	virtual void 			AddAtmosphere(Atmospheric *atmos)=0;

	/*! \remarks This method deletes all existing geometry in the scene. */
	virtual int				NewScene()=0;  // delete all existing geometry
	// I'm stuffing these in here so that I can perhaps add API functions without recompiling
	virtual int				TempFunc1(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int				TempFunc2(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int				TempFunc3(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int				TempFunc4(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int				TempFunc5(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int				TempFunc6(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int				TempFunc7(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int				TempFunc8(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int				TempFunc9(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual int				TempFunc10(void *p1=NULL, void *p2=NULL, void *p3=NULL, void *p4=NULL, void *p5=NULL, void *p6=NULL, void *p7=NULL, void *p8=NULL)=0;
	virtual FILE *			DumpFile() = 0;			// For debugging -- Stream for dumping debug messages
	};

// Export Interface class

/*! \sa  Class IScene.\n\n
\par Description:
Export Interface class. A data member of this class allows the plug-in to
enumerate all nodes in the scene.
\par Data Members:
<b>IScene *theScene;</b>\n\n
A pointer to the scene. See Class IScene. */
class ExpInterface: public MaxHeapOperators {
public:
	IScene *		theScene;		// Pointer to the scene
	};

