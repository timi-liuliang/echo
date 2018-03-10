//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//

#pragma once

#include "maxheap.h"
#include "inode.h"
#include "maxapi.h"
#include "plugapi.h"
#include "snap.h"
#include "genshape.h"
#include "hitdata.h"
#include "imtl.h"
#include "channels.h"
#include "LightTypeEnums.h"
#include "mesh.h"
#include "GraphicsTypes.h"
#include "GraphicsWindow.h"
#include "INodeTab.h"
#include "UVWMapTypes.h"
#include "AnimPropertyID.h"

#include "Graphics/IObjectDisplay.h"
#include "Graphics/RenderItemHandle.h"

// forward declarations
class SubObjAxisCallback;


typedef short MtlIndex; 
typedef short TextMapIndex;


CoreExport void setHitType(int t);
CoreExport int  getHitType(void);
CoreExport BOOL doingXORDraw(void);

/// \defgroup SceneAndNodeHitTestTypes Scene and Node Hit Test Types.
//@{
/** Hit test by a single pick point. */
#define HITTYPE_POINT   1

/** Hit test by a rectangular area.*/
#define HITTYPE_BOX     2

/** Hit test by circular selection area.*/
#define HITTYPE_CIRCLE  3

/** Hit test a face as if it was solid, even in wireframe mode.
 * Treating an item as solid means the face will be hit if the mouse is anywhere 
 * inside the face region and not just over a visible edge. For example in 3ds Max when 
 * an object is not selected and you put the mouse over it to select it, you need to 
 * put it over the wireframe. When an object is selected however you can put the mouse 
 * anywhere over the object and the system still considers this a valid area for hit 
 * testing. This later case is treating the faces of the selected object as solids.  */
#define HITTYPE_SOLID   4

/** Hit testing by an arbitrary polygon fence.*/
#define HITTYPE_FENCE   5

#define HITTYPE_LASSO   6

#define HITTYPE_PAINT   7
//@}


/** \defgroup SceneAndNodeHitTestFlags Scene and Node Hit Testing Flags
 * The following describes hit testing flags that can be sent to the node and 
 * scene hit testing methods */
//@{
/** Hit test selected items only. */
#define HIT_SELONLY				(1<<0)
/** Hit test unselected items only. */
#define HIT_UNSELONLY			(1<<2)
/** Abort the process of hit testing after finding any hit. */
#define HIT_ABORTONHIT			(1<<3)
/** This treats selected items as solid and unselected items as not solid. 
 * Treating an item as solid means the face will be hit if the mouse is anywhere 
 * inside the face region and not just over a visible edge. */
#define HIT_SELSOLID			(1<<4)
/** This treats any item as solid.
 * Treating an item as solid means the face will be hit if the mouse is anywhere 
 * inside the face region and not just over a visible edge. */
#define HIT_ANYSOLID			(1<<5)
/** This forces hit testing for the transform gizmos.*/
#define HIT_TRANSFORMGIZMO		(1<<6)
/** This forces hit testing for the Switch axis when hit.
 * The selection processor that does the hit-testing will include this flag 
 * when hit-testing on a MOUSE_POINT message, because when this flag is active 
 * and the transform gizmo's hit-testing hits the manipulator, it should switch 
 * the axis mode to the axis that is hit. Normally the transform gizmo hit-testing 
 * will only highlight the axis if it hits it - but when this flag is active it 
 * should also set the axis mode using PushAxisMode() or SetAxisMode() */
#define HIT_SWITCH_GIZMO		(1<<7)
/** This forces hit testing for sub-manipulators*/
#define HIT_MANIP_SUBHIT        (1<<8)


/** For hit testing everything which can be combined into the flags parameter.*/
#define HITFLTR_ALL             (1<<10)
/** For hit testing just objects which can be combined into the flags parameter.*/
#define HITFLTR_OBJECTS         (1<<11)
/** For hit testing just cameras which can be combined into the flags parameter.*/
#define HITFLTR_CAMERAS         (1<<12)
/** For hit testing just lights which can be combined into the flags parameter.*/
#define HITFLTR_LIGHTS          (1<<13)
/** For hit testing just helpers which can be combined into the flags parameter.*/
#define HITFLTR_HELPERS         (1<<14)
/** For hit testing just world space objects which can be combined into the flags parameter.*/
#define HITFLTR_WSMOBJECTS      (1<<15)
/** For hit testing just splines which can be combined into the flags parameter.*/
#define HITFLTR_SPLINES         (1<<16)
/** For hit testing just bones which can be combined into the flags parameter.*/
#define HITFLTR_BONES	        (1<<17)
/** For hit testing scene xrefs. You must not make references to scene xrefs. */
#define HIT_SCENEXREFS	        (1<<18)
/** For hit testing scene xrefs hidden in the manager (identified by the XREF_SCENE_HIDEINMANAGERUI XRef flag e.g. closed containers). You must not make references to scene xrefs.*/
#define HIT_MANAGER_HIDDEN_SCENEXREFS	(1<<19)
/** Starting at this bit through the 31st bit can be used by plug-ins for sub-object hit testing.*/
#define HITFLAG_STARTUSERBIT    24     
//@}

class Modifier;
class Object;
class NameTab; 
class ExclList; 
class Texmap;
class ISubObjType;
class MaxIcon;

typedef Object* ObjectHandle;
typedef Tab<TextMapIndex> TextMapIndexTab;
typedef TextMapIndexTab TextTab;

#define BASEOBJAPPDATACID Class_ID(0x48a057d2, 0x44f70d8a)
#define BASEOBJAPPDATALASTSELCID Class_ID(0x1cef158c, 0x1da8486f)
#define BASEOBJAPPDATACURSELCID Class_ID(0x5b3b25fc, 0x35af6260)
#define BASEOBJAPPDATASCID USERDATATYPE_CLASS_ID


/** This class is another version of Matrix3 where the matrix is initialized to the
 * identity by the default constructor.
 * \see Class Matrix3
 */
class IdentityTM: public Matrix3 {
public:
	/** Constructor. The matrix is initialized to the identity
	 * matrix.
	 * There is also a global instance of this class available for use:
	 * extern IdentityTM idTM; 
	 */
	IdentityTM() { IdentityMatrix(); }              
};

CoreExport extern IdentityTM idTM;


//-------------------------------------------------------------
// This is passed in to GetRenderMesh to allow objects to do
// view dependent rendering.
//


/// \defgroup Flags_for_the_View_class__View__flags_ Flags for the View class 
/// These flags can be the arguments for View::flags
//@{
/** Indicates that Displacement Mapping is enabled. Note that for testing the
 * Displacement Mapping property, this flag should be
 * used instead of Interface::GetRendDisplacement(), because the values may
 * not be the same (e.g. while rendering in the Materials Editor).  */
#define RENDER_MESH_DISPLACEMENT_MAP  1
//@}

/** This class is passed in to GeomObject::GetRenderMesh() to allow objects
 * to do view dependent rendering. It is also passed to
 * Control::EvalVisibility().
 * For example particle systems use this to have the particles exactly face the
 * camera (if this option is enabled). If GetRenderMesh() is called by the
 * renderer, the methods of this class are implemented by the system. If a plug-in
 * is calling this method, they must implement these methods. The sample code
 * below shown a null implementation that may be used if a viewport is not
 * involved:
\code
class NullView : public View
{
	Point2 ViewToScreen(Point3 p)
		{ return Point2(p.x,p.y); }
	NullView() {
		worldToView.IdentityMatrix();
		screenW=640.0f; screenH = 480.0f;
	}
};
\endcode 
 * \see GeomObject, Interface, Control, Matrix3, Point3
 */

#pragma warning(push)
#pragma warning(disable:4100 4239)

class View : public InterfaceServer{
public: 
	/** The screen width in pixels*/
	float screenW; 

	/** The screen height in pixels*/
	float screenH;  

	/** A transformation matrix from world into view space. This is into the camera's space.*/
	Matrix3 worldToView;

	/** This method is used to convert a point in view space to
	 * screen space. This includes any perspective projection.
	 * \param p The point in view space.
	 * \return  The point in screen space (in pixel coordinates). */
	virtual Point2 ViewToScreen(Point3 p)=0;

	/** The view projection type: 0 is perspective, 1 is parallel.*/
	int projType;

	/** The field of view in radians.*/
	float fov;

	/** The pixel size setting.*/
	float pixelSize;

	/** World to camera transformation matrix.*/
	Matrix3 affineTM;

	/** Defined in \ref Flags_for_the_View_class__View__flags_ */
	DWORD flags;

	/** This method should be used by GetRenderMesh() implementations that
	 * require a lot of processing time. This allows these processes to be
	 * interrupted by the user. An example of this in use is the extensive
	 * computations done for displacement mapping. These may be interrupted by the
	 * user during a render.
	 * So, any implementation of GetRenderMesh() which takes a long time
	 * should periodically call this method to see if the user has canceled the
	 * render.
	 * \return  true if user has canceled, false otherwise. */
	virtual BOOL CheckForRenderAbort() { return FALSE; }

	/** Generic expansion function*/
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 

	View() { projType = -1; flags = RENDER_MESH_DISPLACEMENT_MAP; } // projType not set, this is to deal with older renderers.
};

/** Class ID of general deformable object.*/
extern CoreExport Class_ID defObjectClassID;

/** Class ID of general texture-mappable object.*/
extern CoreExport Class_ID mapObjectClassID;

/** an array of channel masks for all the channels "within" the Object.*/
CoreExport extern ChannelMask chMask[];

class Object;


/** The ObjectState is the structure that flows up the geometry pipeline. It
 * contains a matrix, a material index, some flags for channels, and a pointer to
 * the object in the pipeline. This is what is passed down the pipeline, and ultimately used by the Node
 * to Display, Hittest, render.
 * \see Object, Matrix3
 */
class ObjectState: public MaxHeapOperators {
	ulong flags;
	Matrix3 *tm;
	Interval tmvi;   
	int mtl;
	Interval mtlvi;                         
	void AllocTM();
public: 
	/** Pointer to the object in the pipeline. The validity interval of the
	 * object can be retrieved using obj-\>ObjectValidity()
	 */
	Object *obj;  

	/** Constructor. The object pointer is initialized to NULL. */
	CoreExport ObjectState();

	/** Constructor. The object pointer is set to the object passed.
	The tm pointer is set to NULL and the tm and mtl validity intervals are set
	to FOREVER.
	\param ob The object to initialize the object pointer to. */
	CoreExport ObjectState(Object *ob);

	/** Constructor. The object state is initialized to the object state passed.
	\param os The object state to initialized to os. */
	CoreExport ObjectState(const ObjectState& os); 

	/** Destructor. If the tm exists, it is deleted. */
	CoreExport ~ObjectState();

	/** Call this method to update the object state flags.
	\param f The flags to set. The specified flags are ORed into the current state of the flags. */
	void OSSetFlag(ulong f) { flags |= f; }

	/** Call this method to clear the specified object state flags.
	\param f The flags to clear. */
	void OSClearFlag(ulong f) { flags &= ~f; }

	/** Call this method to test the specified flags.
	\param f The flags to test.
	\return  Nonzero if the specified flags are all set; otherwise 0. */
	ulong OSTestFlag(ulong f) const { return flags&f; }

	/** Copies the specified flag settings from the specified object state to this object state.
	\param f The flags to copy.
	\param fromos The source object state. */
	CoreExport void OSCopyFlag(ulong f, const ObjectState& fromos);

	/** Assignment operator. The object pointer, flags,
	transformation matrix (and its validity interval), and material (and its
	validity interval) are copied from the specified object state. */
	CoreExport ObjectState& operator=(const ObjectState& os);

	/** Returns the validity interval of the object state's
	transformation matrix. */
	Interval tmValid() const { return tmvi; }

	/** Returns the validity interval of the object state's material. */
	Interval mtlValid() const  { return mtlvi; }

	/** Returns the validity interval of the object state. If the
	object is not defined, this interval is NEVER. Otherwise it is the
	intersection of the tm validity interval, the mtl validity interval and the
	interval returned from obj-\>ObjectValidity(t).
	\param t Specifies the time to retrieve the validity interval.
	\return  The validity interval of the object state. */
	CoreExport Interval Validity(TimeValue t) const;

	/** Returns nonzero if the object state's transformation matrix
	is the identity matrix; otherwise zero. */
	CoreExport int TMIsIdentity() const;

	/** Sets the object state's transformation matrix to the
	specified Matrix3 and its validity interval to the interval passed. If the
	specified matrix is NULL, a new Matrix3 is allocated and is initialized to
	the identity matrix.
	\param mat Specifies the matrix to set.
	\param iv Specifies the validity interval to set. */
	CoreExport void SetTM(Matrix3* mat, Interval iv);

	/** Returns the object state's transformation matrix. */
	CoreExport Matrix3* GetTM() const;

	/** Sets the object state tm to the identity transformation matrix. */
	CoreExport void SetIdentityTM();

	/** Applies the specified matrix to the object state tm. The
	object state tm is multiplied by the specified matrix. The specified
	interval is intersected with the object state tm validity interval.
	\param mat The matrix to apply.
	\param iv The interval to intersect with the object state's tm validity interval. */
	CoreExport void ApplyTM(Matrix3* mat, Interval iv);

	/** Copies the object state tm (and its validity interval) from
	the specified object state's tm.
	\param fromos The object state whose tm is to be copied. */
	CoreExport void CopyTM(const ObjectState &fromos);

	/** Copies the object state material (and its validity interval)
	from the specified object state's material.
	\param fromos The object state whose material is to be copied. */
	CoreExport void CopyMtl(const ObjectState &fromos);

	/** Invalidates the specified channels of the object state's
	object.
	\param channels The channels of the object to invalidate.
	\param checkLock If checkLock is TRUE and OBJ_CHANNELS is one of the specified
	channels, the object the object state points to is not deleted if it is
	locked; otherwise it is deleted. */
	CoreExport void Invalidate(ChannelMask channels, BOOL checkLock=FALSE);

	/** Deletes the object state's object.
	\param checkLock If checkLock is TRUE, the object the object state points to is not
	deleted if it is locked; otherwise it is always deleted.
	\par Operators:
	*/
	CoreExport ChannelMask DeleteObj(BOOL checkLock=FALSE);
};

/**This class allows a modifier to store application specific data. A reference to
a pointer to an instance of this class is passed in to ModifyObject() as
part of the ModContext. The value of the pointer starts out as NULL, but
the modifier can set it to point at an actual instance of a derived class. When
the mod app is deleted, if the pointer is not NULL, the LocalModData
will be deleted - the virtual destructor allows this to work.  
\see  Class ModContext
*/
class LocalModData : public InterfaceServer {
public:
	/** A plug-in using local data should implement this method to free its local
	data. */
	virtual ~LocalModData() {}

	/** This method is called to allow a plug-in to copy its local data. It is
	called when the system is copying a ModContext.
	\return  The plug-in should return a pointer to a new instance of its
	LocalModData. */
	virtual LocalModData *Clone()=0;

	using InterfaceServer::GetInterface;
	virtual void* GetInterface(ULONG id) { return NULL; }  // to access sub-obj selection interfaces, JBW 2/5/99
}; 

/** The ModContext stores information about the space the modifier was applied in,
and allows the modifier to store data is needs for its operation. All methods
are implemented by the system.
\see class LocalModData
*/
class ModContext : public BaseInterfaceServer {
public:
	/** This matrix represents the space the modifier was applied in. The modifier
	plug-in uses this matrix when it deforms an object. The plug-in first
	transforms the points with this matrix. Next it applies its own deformation.
	Then it transforms the points back through the inverse of this transformation
	matrix.*/
	Matrix3                 *tm;

	/** The Bounding Box of the Deformation. This represents the scale of the modifier.
	For a single object it is the bounding box of the object. If the modifier is
	being applied to a sub-object selection it represents the bounding box of the
	sub-object selection. If the modifier is being applied to a selection set of
	objects, then this is the bounding box of the entire selection set. For a
	selection set of objects the bounding box is constant. In the case of a single
	object, the bounding box is not constant.*/
	Box3                    *box;

	/** A pointer to an instance of a class derived from the LocalModData class. This
	is the part of the ModContext that the plug-in developer controls. It is the
	place where a modifier may store application specific data.*/
	LocalModData    *localData;

	/** Destructor. The tm, bounding box and local data are freed. */
	CoreExport ~ModContext();

	/** Constructor. The transformation matrix, bounding box, and
	local data pointer are initialized to NULL. */
	CoreExport ModContext();

	/** Constructor. The tm, bounding box and local data are
	initialized to those of the specified ModContext.
	\param mc The ModContext to copy. */
	CoreExport ModContext(const ModContext& mc);

	/** Constructor. The tm, bounding box, and local data are
	initialized to those specified.
	\param tm The transform matrix to copy.
	\param box The bounding box to copy.
	\param localData The local data that will be cloned. */
	CoreExport ModContext(Matrix3 *tm, Box3 *box, LocalModData *localData);
};

/** A modifier may be applied to several objects in the scene. The
Interface::GetModContexts() method retrieves a list of all the
ModContexts for the current place in the history. This class is used as a table
to hold the list of ModContexts. 
\see Template Class Tab, Class Interface::GetModContexts()*/
class ModContextList : public Tab<ModContext*> {};


class HitRecord;


/// \defgroup BaseObject__NewSetByOperator___arguments Arguments for BaseObject::NewSetByOperator()
/// These are valid arguments for the \c op parameter of the function BaseObject::NewSetByOperator().
//@{
#define NEWSET_MERGE			1	//!< The sets should be merged.
#define NEWSET_INTERSECTION		2	//!< The sets should be intersected -- that is the items common to both sets should appear in the new set.
#define NEWSET_SUBTRACT			3	//!< The new set should be the result of subtracting the 1st thru nth set from the 0th set.
//@}

/// \defgroup Display_Flags Display Flags
//@{
/** If this flag is set, only the damaged area needs to be displayed. 
The damaged rectangle may be retrieved using INode::GetDamagedRect(). See Class INode.*/
#define USE_DAMAGE_RECT                 (1<<0)  
/** This indicates if an item should display its sub-object selection state. 
The system will set this flag is the item is selected, the user is in the modify 
branch, and the item is in sub-object selection mode.*/
#define DISP_SHOWSUBOBJECT              (1<<1)
//@}


/** The base class of Geometric objects, Lights, Cameras, Modifiers, 
 * Deformation objects -- anything with a 3D representation in the UI scene. */
class IParamArray;

/** This is the base class for objects and modifiers. Anything with a
representation in the 3D viewports is derived from BaseObject (including
modifiers and controllers whose gizmos appear in the viewports). The methods
here are things such as displaying the object in the viewports, checking for
intersection of the object and points the user clicks with the mouse, snapping
to the object, and computing various bounding boxes for the object. Also there
are methods for returning the name of the object to appear in the modifier
stack, a method to deal with creating the object in the viewports (if
appropriate), and named selection set related methods. There are also methods
that allow other plug-ins to access the changeable parameters of the object.
Finally there are several method that deal with sub-object selection,
sub-object display, sub-object hit testing, and moving/rotating/scaling
sub-object components of the object.
\see Class ReferenceTarget, Class INode, Class ViewExp, Class Box3,  Class IPoint2  
\see Class Matrix3, Structure SnapInfo, Class Point3, Class CreateMouseCallBack 
\see Template Class Tab, Class Interface.
*/
class BaseObject : public ReferenceTarget {

	friend class ModifyTaskImp;
	int subObjLevel;

private:
	MaxSDK::Graphics::IObjectDisplay* mpDisplayInterface;

protected:
	MaxSDK::Graphics::RenderItemHandleArray mRenderItemHandles;

public:
	CoreExport virtual bool RequiresSupportForLegacyDisplayMode() const;
	CoreExport virtual bool UpdateDisplay(
		unsigned long renderItemCategories, 
		const MaxSDK::Graphics::MaterialRequiredStreams& materialRequiredStreams, 
		TimeValue t);
	CoreExport virtual const MaxSDK::Graphics::RenderItemHandleArray& GetRenderItems() const;

	CoreExport virtual void* GetInterface(ULONG id);
	CoreExport virtual BaseInterface* GetInterface(Interface_ID id);

	CoreExport BaseObject();
	CoreExport virtual ~BaseObject();

	/** This method is called to determine if the specified screen point intersects
	the item. The method returns nonzero if the item was hit; otherwise 0.
	\param t The time to perform the hit test.
	\param inode A pointer to the node to test.
	\param type The type of hit testing to perform. See \ref SceneAndNodeHitTestTypes for details.
	\param crossing The state of the crossing setting. If TRUE crossing selection is on.
	\param flags The hit test flags. See \ref SceneAndNodeHitTestFlags for details.
	\param p The screen point to test.
	\param vpt An interface pointer that may be used to call methods associated with the viewports.
	\return  Nonzero if the item was hit; otherwise 0. */
	virtual int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt){return 0;};
	
	/** This method is used for storing mode-dependent display
	attributes.
	Before an object's Display() method is called, the appropriate bits of the
	extended display flag variable are set and this method is called. After
	that, the Display() method is called. If the object must display itself
	differently based on the settings of the extended display bit fields, then
	the object must save the flags passed into the this method. Otherwise,
	there is no need for the object to store the flags.
	\param flags The flags to store. */
	virtual void SetExtendedDisplay(int flags)      {}   

	/** This is called by the system to have the item display itself (perform a
	quick render in viewport, using the current TM). Note: For this method to
	be called the object's validity interval must be invalid at the specified
	time t. If the interval is valid, the system may not call this
	method since it thinks the display is already valid.
	\param t The time to display the object.
	\param inode The node to display.
	\param vpt An interface pointer that may be used to call methods associated with the viewports.
	\param flags See \ref Display_Flags.
	\return  The return value is not currently used. */
	virtual int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) { return 0; };   

	/** Checks the point passed for a snap and updates the SnapInfo structure.
	\note Developers wanting to find snap points on an
	Editable Mesh object should see the method XmeshSnap::Snap() in
	/MAXSDK/SAMPLES/SNAPS/XMESH/XMESH.CPP.
	\param t The time to check.
	\param inode The node to check.
	\param snap The snap info structure to update.
	\param p The screen point to check.
	\param vpt An interface pointer that may be used to call methods associated with the viewports. */
	virtual void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) {}

	/** This method returns the world space bounding box for Objects (see below for
	the Sub-object gizmo or Modifiers gizmo version). The bounding box returned
	by this method does not need to be precise. It should however be calculated
	rapidly. The object can handle this by transforming the 8 points of its
	local bounding box into world space and take the minimums and maximums of
	the result. Although this isn't necessarily the tightest bounding box of
	the objects points in world space, it is close enough.
	\param t The time to compute the bounding box.
	\param inode The node to calculate the bounding box for.
	\param vp An interface pointer that can be used to call methods associated with the viewports.
	\param box Contains the returned bounding box. */
	virtual void GetWorldBoundBox(TimeValue t, INode * inode, ViewExp* vp, Box3& box ){}; 

	/** This is the object space bounding box, the box in the object's local
	coordinates. The system expects that requesting the object space bounding
	box will be fast.
	\param t The time to retrieve the bounding box.
	\param inode The node to calculate the bounding box for.
	\param vp An interface pointer that may be used to call methods associated with the viewports.
	\param box Contains the returned bounding box. */
	virtual void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vp,  Box3& box ){}; 

	/** This method allows the system to retrieve a callback object used in
	creating an object in the 3D viewports. This method returns a pointer to an
	instance of a class derived from CreateMouseCallBack. This class has
	a method proc() which is where the programmer defines the user/mouse
	interaction during the object creation phase.
	\return  A pointer to an instance of a class derived from
	CreateMouseCallBack. */
	virtual CreateMouseCallBack* GetCreateMouseCallBack()=0;

	/** \return the name that will appear in the history browser (modifier stack). */
	virtual MCHAR *GetObjectName() { return _M("Object"); }

	/** Implemented by the System. 
	This method is called to see if any modifiers down in the pipeline depend
	on topology. It sends the message \ref REFMSG_IS_OK_TO_CHANGE_TOPOLOGY to
	see if any modifiers or objects down the pipeline depend on topology.
	\param modName This parameter is set to the dependent modifier's name if there is an item
	that depends on topology.
	\return  TRUE if it is okay to change the topology; FALSE if it is
	not okay to change the topology. */
	CoreExport virtual BOOL OKToChangeTopology(MSTR &modName);

	// Return true if this object(or modifier) is cabable of changing 
	//topology when it's parameters are being edited.
	/** This method asks the question of an object or modifier "Do you change
	topology"? An object or modifier returns TRUE if it is capable of changing
	topology when its parameters are being edited; otherwise FALSE. 
	When an item is selected for editing, and there is a modifier in the
	pipeline that depends on topology, the system calls this method to see if
	it may potentially change the topology. If this method returns TRUE the
	system will put up a warning message indicating that a modifier exists in
	the stack that depends on topology. */
	virtual BOOL ChangeTopology() {return TRUE;}

	/** \deprecated This method is no longer used. */
	virtual void ForceNotify(Interval& i)
	{
		NotifyDependents( i, (PartID)PART_ALL, REFMSG_CHANGE );
	}

	/** An object or modifier should implement this method if it wishes to make its
	parameter block available for other plug-ins to access it. The system
	itself doesn't actually call this method. This method is optional.
	\return  A pointer to the item's parameter block. See
	Class IParamArray. */
	virtual IParamArray *GetParamBlock() {return NULL;}

	/** If a plug-in makes its parameter block available (using
	GetParamBlock()) then it will need to provide #defines for indices
	into the parameter block. These defines should not be directly used with
	the parameter block but instead converted by this function that the plug-in
	implements. This way if a parameter moves around in a future version of the
	plug-in the #define can be remapped. A return value of -1 indicates an
	invalid parameter id.
	\param id The parameter block id. See \ref paramBlockIDs.
	\return  The parameter block index or -1 if it is invalid. */
	virtual int GetParamBlockIndex(int id) {return -1;}
	
	/** When this method is called the plug-in should respond by moving its
	selected sub-object components.
	\param t The time of the transformation.
	\param partm The 'parent' transformation matrix. This matrix represents a transformation
	that would take points in the modifier's space and convert them into world
	space points. This is constructed as the node's transformation matrix times
	the inverse of the ModContext's transformation matrix. The node whose
	transformation is used is the node the user clicked on in the scene -
	modifiers can be instanced so there could be more than one node.
	\param tmAxis The matrix that represents the axis system. This is the space in which the
	transformation is taking place.
	\param val This value is a vector with X, Y, and Z representing the movement along each axis.
	\param localOrigin When TRUE the transformation is occurring about the sub-object's local origin. */
	virtual void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE ){}

	/** When this method is called the plug-in should respond by rotating its
	selected sub-object components.
	\param t The time of the transformation.
	\param partm The 'parent' transformation matrix. This matrix represents a transformation
	that would take points in the modifier's space and convert them into world
	space points. This is constructed as the node's transformation matrix times
	the inverse of the ModContext's transformation matrix. The node whose
	transformation is used is the node the user clicked on in the scene -
	modifiers can be instanced so there could be more than one node.
	\param tmAxis The matrix that represents the axis system. This is the space in which the
	transformation is taking place.
	\param val The amount to rotate the selected components.
	\param localOrigin When TRUE the transformation is occurring about the sub-object's local
	origin. Note: This information may be passed onto a transform controller
	(if there is one) so they may avoid generating 0 valued position keys for
	rotation and scales. For example if the user is rotating an item about
	anything other than its local origin then it will have to translate in
	addition to rotating to achieve the result. If a user creates an object,
	turns on the animate button, and rotates the object about the world origin,
	and then plays back the animation, the object does not do what the was done
	interactively. The object ends up in the same position, but it does so by
	both moving and rotating. Therefore both a position and a rotation key are
	created. If the user performs a rotation about the local origin however
	there is no need to create a position key since the object didn't move (it
	only rotated). So a transform controller can use this information to avoid
	generating 0 valued position keys for rotation and scales. */
	virtual void Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE ){}

	/** When this method is called the plug-in should respond by scaling its
	selected sub-object components.
	\param t The time of the transformation.
	\param partm The 'parent' transformation matrix. This matrix represents a transformation
	that would take points in the modifier's space and convert them into world
	space points. This is constructed as the node's transformation matrix times
	the inverse of the ModContext's transformation matrix. The node whose
	transformation is used is the node the user clicked on in the scene -
	modifiers can be instanced so there could be more than one node.
	\param tmAxis The matrix that represents the axis system. This is the space in which the
	transformation is taking place.
	\param val This value is a vector with X, Y, and Z representing the scale along X, Y,
	and Z respectively.
	\param localOrigin When TRUE the transformation is occurring about the sub-object's local
	origin. See the note above in the Rotate method. 
	The following methods may be used to receive notification about the
	starting and ending phases of transforming the item when in sub-object
	selection. */
	virtual void Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE ){}

	/** This method is called before the first Move(), Rotate() or
	Scale() call and before a hold is in effect.
	\param t The current time when this method is called. */
	virtual void TransformStart(TimeValue t) {}

	/** This method is called before the first Move(), Rotate() or
	Scale() call and after a hold is in effect.
	\param t The current time when this method is called. */
	virtual void TransformHoldingStart(TimeValue t) {}

	/** This method is called after the user has completed the Move(), Rotate() or
	Scale() operation and before the undo object has
	been accepted.
	\param t The current time when this method is called. */
	virtual void TransformHoldingFinish(TimeValue t) {}             

	/** This method is called after the user has completed the Move(), Rotate() or
	Scale() operation and the undo object has been
	accepted.
	\param t The current time when this method is called. */
	virtual void TransformFinish(TimeValue t) {}            

	/** This method is called when the transform operation is
	canceled by a right-click and the undo has been canceled.
	\param t The current time when this method is called. */
	virtual void TransformCancel(TimeValue t) {}            

	/** \name Sub-Object Display, Hit Test, and Bounding Box Methods
	* These methods are for sub-object selection. If the derived class 
	* is NOT a modifier, the modContext pointer passed to some of these 
	* methods will be NULL. */
	//@{
	/** This method is used in modifier gizmo hit testing. It is called to
	determine if the specified screen point intersects the gizmo. The method
	returns nonzero if the item was hit; otherwise 0.
	\param t The time to perform the hit test.
	\param inode A pointer to the node to test.
	\param type The type of hit testing to perform. See \ref SceneAndNodeHitTestTypes for details.
	\param crossing The state of the crossing setting. If TRUE crossing selection is on.
	\param flags The hit test flags. See \ref SceneAndNodeHitTestFlags for details.
	\param p The screen point to test.
	\param vpt An interface pointer that may be used to call methods associated with the viewports.
	\param mc A pointer to the modifiers ModContext.
	\return  Nonzero if the item was hit; otherwise 0. */
	
	virtual int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) { return 0; }
	/** When this method is called the plug-in should respond by performing a quick
	render of the modifier gizmo in viewport using the current TM. 
	Note for Modifiers: For this method to be called properly, one must send
	two reference messages using NotifyDependents. 
	In BeginEditParams send: 
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_ON);\n\n
	In EndEditParams send: 
	NotifyDependents(Interval(t,t), PART_ALL, REFMSG_MOD_DISPLAY_OFF);
	\param t The time to display the item.
	\param inode The node to render.
	\param vpt An interface pointer that may be used to call methods associated with the viewports.
	\param flags See \ref Display_Flags.
	\param mc A pointer to the modifiers ModContext.
	\return  Nonzero if the item was displayed; otherwise 0. */
	virtual int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext* mc) { return 0; };   // quick render in viewport, using current TM.         
	
	/** This method computes the world space bounding box of the modifier gizmo (or
	any object that when in sub-object mode has a gizmo).
	\param t The time to compute the bounding box.
	\param inode The node to calculate the bounding box for.
	\param vpt An interface pointer that may be used to call methods associated with the viewports.
	\param box The returned bounding box.
	\param mc A pointer to the modifiers ModContext. */
	virtual void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) {}

	/** This method is called to make a copy of the selected sub-object components
	of the item. If this is called on an object, the selection level of the
	object is used to determine which type of sub-objects are cloned. For
	instance in a Mesh, the selection level determines if selected verticies,
	edges or faces are cloned. If this is called on a Modifier then the
	selection level of the modifier is used. Modifiers call
	Interface::GetModContexts() to get a list of ModContexts, one for
	each object the modifier is applied to. Then the selected sub-objects are
	cloned for each object in the list.
	\param t The time at which to clone the selected sub-object components. */
	virtual void CloneSelSubComponents(TimeValue t) {}

	/**	This method is called when the user mouses up after shift-cloning a
	sub-object selection.
	\param t The time at which the clone of the selected components is being done. */
	virtual void AcceptCloneSelSubComponents(TimeValue t) {}

	/** This method is called to change the selection state of the component
	identified by hitRec.
	\param hitRec Identifies the component whose selected state should be set. See Class HitRecord .
	\param selected TRUE if the item should be selected; FALSE if the item should be de-selected.
	\param all TRUE if all components in the HitRecord chain should be selected; FALSE if
	only the top-level HitRecord should be selected. (A HitRecord contains a
	Next() pointer; typically you want to do whatever you're doing to all the
	Next()'s until Next() returns NULL).
	\param invert This is set to TRUE when all is also set to TRUE and the user is
	holding down the Shift key while region selecting in select mode. This
	indicates the items hit in the region should have their selection state
	inverted */
	virtual void SelectSubComponent(
		HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE) {}

	/** This method is called to clear the selection for the given sub-object
	level. All sub-object elements of this type should be deselected. This will
	be called when the user chooses Select None from the 3ds Max Edit menu.
	\param selLevel Specifies the selection level to clear. */
	virtual void ClearSelection(int selLevel) {}
	
	/** This method is called to select every element of the given sub-object
	level. This will be called when the user chooses Select All from the 3ds
	Max Edit menu.
	\param selLevel Specifies the selection level to select. */
	virtual void SelectAll(int selLevel) {}
	
	/** This method is called to invert the specified sub-object
	level. If the element is selected it should be deselected. If it's
	deselected it should be selected. This will be called when the user chooses
	Select Invert from the 3ds Max Edit menu.
	\param selLevel Specifies the selection level to invert. */
	virtual void InvertSelection(int selLevel) {}

	/** Returns the index of the sub-object element identified by the HitRecord
	hitRec. See Class HitRecord. The
	sub-object index identifies a sub-object component. The relationship
	between the index and the component is established by the modifier. For
	example an edit modifier may allow the user to select a group of faces and
	these groups of faces may be identified as group 0, group 1, group 2, etc.
	Given a hit record that identifies a face, the edit modifier's
	implementation of this method would return the group index that the face
	belonged to. */
	virtual int SubObjectIndex(HitRecord *hitRec) {return 0;}               

	/** When the user changes the selection of the sub-object drop down, this
	method is called to notify the plug-in. This method should provide
	instances of a class derived from
	CommandMode to support move, rotate,
	non-uniform scale, uniform scale, and squash modes. These modes replace
	their object mode counterparts however the user still uses the
	move/rotate/scale tool buttons in the toolbar to activate them. If a
	certain level of sub-object selection does not support one or more of the
	modes NULL may be passed. If NULL is specified the corresponding toolbar
	button will be grayed out.
	\param level The sub-object selection level the command modes should be set to support.
	A level of 0 indicates object level selection. If level is
	greater than or equal to 1 the index refers to the types registered by the
	object in the order they appeared in the list when registered by
	Interface::RegisterSubObjectTypes(). See Class Interface.
	\param modes The command modes to support
	\par Sample Code:
	\code
	void SimpleMod::ActivateSubobjSel(int level, XFormModes& modes)
	{
		switch ( level ) {
			case 1:									  // Modifier box
				modes = XFormModes(moveMode,rotMode,nuscaleMode,uscaleMode,squashMode,NULL);
				break;
			case 2:									  // Modifier Center
				modes =	XFormModes(moveMode,NULL,NULL,NULL,NULL,NULL);
				break;
		}
		NotifyDependents(FOREVER,PART_DISPLAY,REFMSG_CHANGE);
	}
	\endcode
	\see  Class XFormModes. */
	virtual void ActivateSubobjSel(int level, XFormModes& modes ) {}

	/** An object that supports sub-object selection can choose to
	 * support named sub object selection sets. Methods in the the
	 * interface passed to objects allow them to add items to the
	 * sub-object selection set drop down.
	 * The following methods are called when the user picks items
	 * from the list.
	 * \return true if the plug-in supports named sub-object selection sets,
	 * false otherwise. */
	virtual BOOL SupportsNamedSubSels() {return FALSE;}

	/** When the user chooses a name from the drop down list this method is called.
	The plug-in should respond by selecting the set identified by the name
	passed.
	\param setName The name of the set to select. */
	virtual void ActivateSubSelSet(MSTR &setName) {}

	/** If the user types a new name into the named selection set drop down then
	this method is called. The plug-in should respond by creating a new set and
	give it the specified name.
	\param setName The name for the selection set. */
	virtual void NewSetFromCurSel(MSTR &setName) {}

	/** If the user selects a set from the drop down and then chooses Remove Named
	Selections from the Edit menu this method is called. The plug-in should
	respond by removing the specified selection set.
	\param setName The selection set to remove. */
	virtual void RemoveSubSelSet(MSTR &setName) {}

	/**	To support the Edit Named Selections dialog, plug-ins must implement
	this method.
	It is called to rebuild the named selection set drop down list.
	This is usually done by calling
	Interface::ClearSubObjectNamedSelSets() followed by calls to
	Interface:: AppendSubObjectNamedSelSet(). */
	virtual void SetupNamedSelDropDown() {}

	/**	To support the Edit Named Selections dialog, plug-ins must implement
	this method.
	\return the number of named selection sets. */
	virtual int NumNamedSelSets() {return 0;}

	/**	To support the Edit Named Selections dialog, plug-ins must implement
	this method.
	\param i The index of the selection set whose name is returned. 
	\return the name of the 'i-th' named selection set.*/
	virtual MSTR GetNamedSelSetName(int i) {return _M("");}

	/**	To support the Edit Named Selections dialog, plug-ins must implement
	this method. It sets the name of the selection set whose index is passed to
	the name passed.
	Note: Developers need to implement Undo / Redo for modifications to their
	named selection sets.
	\param i The index of the selection set whose name is to be set.
	\param newName The new name for the selection set the plug-in should store. */
	virtual void SetNamedSelSetName(int i,MSTR &newName) {}

	/**	To support the Edit Named Selections dialog, plug-ins must implement
	this method.
	The user may bring up the Edit Named Selections dialog via the Edit / Edit
	Named Selection ... command. This dialog allows the user to create new
	selection sets using 'boolean' operations to the sets including 'Combine',
	'Subtract (A-B)', 'Subtract (B-A)' and 'Intersection'. This method is
	called on the plug-in to generate a new selection set via one of these
	operations. 
	This method assumes the developer will append a new seleciton set
	with the name passed. This will result in two sets with identical names.
	Then the system will call RemoveSubSelSet() afterwards, so that the
	first one that is found (the old one, since the new one was appended) will
	be deleted. 
	Note: Developers need to implement Undo / Redo for modifications to their
	named selection sets. See /MAXSDK/SAMPLES/MODIFIERS/MESHSEL.CPP for
	an example.
	\param newName The new name for the selection set is passed here.
	\param sets A table of the selection sets to operate on. There are sets.Count()
	sets in the table.
	\param op One of the following values defined in \ref BaseObject__NewSetByOperator___arguments*/
	virtual void NewSetByOperator(MSTR &newName,Tab<int> &sets,int op) {}
	//@}


	/** \defgroup  Sub_Object_Centers_and_TMs Sub-Object Centers and TMs
	 * New way of dealing with sub object coordinate systems.
	 * Plug-in enumerates its centers or TMs and calls the callback once for each.
	 * NOTE:cb->Center() should be called the same number of times and in the
	 * same order as cb->TM()
	 * NOTE: The SubObjAxisCallback class is defined in animatable and used in both the
	 * controller version and this version of GetSubObjectCenters() and GetSubObjectTMs()
	 */
	//@{
	/** When the user is in a sub-object selection level, the system needs to get
	the reference coordinate system definition from the current modifier being
	edited so that it can display the axis. This method specifies the position
	of the center. The plug-in enumerates its centers and calls the callback
	cb once for each.
	\param cb The callback object whose methods may be called. See Class SubObjAxisCallback.
	\param t The time to enumerate the centers.
	\param node A pointer to the node.
	\param mc A pointer to the ModContext. */
	virtual void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {}

	/** When the user is in a sub-object selection level, the system needs to get
	the reference coordinate system definition from the current modifier being
	edited so that it can display the axis. This method returns the axis system
	of the reference coordinate system. The plug-in enumerates its TMs and
	calls the callback cb once for each. See
	<a href="ms-its:3dsmaxsdk.chm::/selns_sub_object_coordinate_systems.html">Sub-Object
	Coordinate Systems</a>.
	\param cb The callback object whose methods may be called.
	\param t The time to enumerate the TMs.
	\param node A pointer to the node.
	\param mc A pointer to the ModContext. */
	virtual void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) {}                          
	//@}

	/**	It is called to find out if the object is has UVW coordinates. This method
	returns TRUE if the object has UVW coordinates; otherwise FALSE. In 3ds Max
	2.0 and later there is code in the renderer that will automatically turn on
	the UVW coordinates of the base object if UV's are missing (and needed).
	The base object has to implement two simple methods to make this work:
	HasUVW() and SetGenUVW(). 
	Developers are encouraged to put these methods in their objects: it makes
	using the program easier for the user. If they are not implemented, it
	doesn't cause any real harm: it will just operate as before and put up the
	missing UVW's message. 
	Here is how the procedural sphere implements these methods:
	\code
	BOOL SphereObject::GetGenUVW()
	{
		BOOL genUVs;
		Interval v;
		pblock->GetValue(PB_GENUVS, 0, genUVs, v);
		return genUVs;
	}
	
	void SphereObject::SetGenUVW(BOOL sw)
	{
		if (sw==GetGenUVW()) return;
		pblock->SetValue(PB_GENUVS,0, sw);
	}
	\endcode
	Important Note: The pblock-\>SetValue() will cause a call to
	NotifyDependents(FOREVER, PART_TEXMAP, REFMSG_CHANGE), which will
	invalidate the UVW cache. It is essential that this call be made, so if the
	'generate UVW' boolean is not handled by a parameter block, then
	NotifyDependents() needs to be called explicitly.
	Also Note: For "modifiable objects" that pass up the pipeline getting
	modified, such as TriObject, EditTriObject, etc., which cannot generate
	their own UVWs, but can carry them in their data structures, only this
	HasUVW() method needs to be implemented. For example, here is the
	implementation for TriObject:
	BOOL TriObject::HasUVW() { return mesh.tvFace?1:0; } */
	virtual BOOL HasUVW () { return 0; }
	/**	It is called to find out if the object is has UVW coordinates for the
	specified mapping channel. This method returns TRUE if the object has UVW
	coordinates; otherwise FALSE. See the method HasUVW() above for more
	details.
	\param mapChannel See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_mapping_channel_index_values.html">List of
	Mapping Channels Values</a>. */
	virtual BOOL HasUVW (int mapChannel) { return (mapChannel==1) ? HasUVW() : FALSE; }
	/**	This method is called to change the state of its Generate UVW boolean. If
	the state changes, the object must send a \ref REFMSG_CHANGE up the
	pipeline by calling NotifyDependents(). This applies to map channel 1.
	\param sw The new state for the generate UVW flag. */
	virtual void SetGenUVW(BOOL sw) {  }

	/**	This method is called to change the state of its Generate UVW boolean for
	the specified mapping channel. If the state changes, the object must send a
	\ref REFMSG_CHANGE up the pipeline by calling NotifyDependents().
	\param mapChannel The mapping channel index. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_mapping_channel_index_values.html">List of Mapping
	Channel Index Values</a>.
	\param sw The new state for the generate UVW flag. */
	virtual void SetGenUVW (int mapChannel, BOOL sw) { if (mapChannel==1) SetGenUVW (sw); }

	/**	This method notifies the BaseObject that the end result display has been
	switched (the "Show End Result" button has been toggled). Sometimes this is
	needed for display changes.
	This method is implemented in Edit Mesh, which uses it as shown below: 
	void EditMeshMod::ShowEndResultChanged(BOOL showEndResult) {\n\n
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);\n\n
	}\n\n
	This allows the Edit Mesh modifier to update itself in repsonse to a user
	click of the "Show End Result" button in the modifier panel.
	\param showEndResult TRUE if Show End Result is on; FALSE if off. */
	virtual void ShowEndResultChanged (BOOL showEndResult) { }

	/// \defgroup  Sub_Object_Centers_and_TMs
	//@{
	/**	This method is called before a modifier or object is collapsed. 
	\sa Class NotifyCollapseEnumProc.
	\param node Points to the node for the object being collapsed.
	\param derObj If the object associated with node above is a Modifier this points
	to the derived object.If it's an object then this is NULL.
	\param index If the object associated with node above is a Modifier this is the
	index of this modifier in the DerivedObject.. If it's an object then this is 0. */
	virtual void NotifyPreCollapse(INode *node, IDerivedObject *derObj, int index){};

	/**	This method is called after a modifier or object is collapsed. 
	\sa Class NotifyCollapseEnumProc.
	\param node Points to the node for the object being collapsed.
	\param obj
	\param derObj If the object associated with node above is a Modifier this points
	to the derived object. If it's an object then this is NULL.
	\param index If the object associated with node above is a Modifier this is the
	index of this modifier in the DerivedObject. If it's an object then this is 0. */
	virtual void NotifyPostCollapse(INode *node, Object *obj, IDerivedObject *derObj, int index){};

	/**	Objects and modifiers that support subobjects have to overwrite 
	 * this method - as well as GetSubObjType() - and return a class derived from 
	 * ISubObjType in GetSubObjType(). 
	 * Developers can use the GenSubObjType for convenience. If the parameter passed 
	 * into GetSubObjType is -1, the system requests a ISubObjType, for the current
	 * SubObjectLevel that flows up the modifier stack. If the subobject selection 
	 * of the modifier or base object does not affect the subobj selection that 
	 * flows up the stack, the method must return NULL. See meshsel.cpp for a 
	 * sample implementation.
	 * \return The number of sub-object types supported by the object or modifier.
	 */
	virtual int NumSubObjTypes(){ return 0;}

	/**	Returns a pointer to the sub-object type for the sub-object whose index is
	passed. 
	If the parameter i passed is -1 the system requests an
	ISubObjType for the current SubObjectLevel that flows up the
	modifier stack. If the subobject selection of the modifier or base object
	does not affect the sub-object selection that flows up the stack NULL must
	be returned. See the sample code in /MAXSDK/SAMPLES/MODIFIERS/MESHSEL.CPP.
	\param i The zero based index of the sub-object type to get. See the remarks above.
	\return  The sub-object type. See Class ISubObjType. */
	virtual ISubObjType *GetSubObjType(int i) { return NULL; }

	/**	This method returns an integer which indicates the current sub-object level
	of the modifier or base object.
	\return A value of 0 indicates object level. A value of 1 through
	NumSubObjTypes() are the sub-object levels in the same order as they
	are returned by GetSubObjType(int i) (with an offset of 1 of
	course). */
	CoreExport virtual int GetSubObjectLevel();
	//@}
	
	

	/** This method return true if GetWorldBoundBox returns different boxes for different viewports. 
	 * It is used to inhibit a caching of the bounding box for all viewports.
	 * This was added to allow developers to disable the bounding box cache. It may be overridden to return true
	 * if an object has a dynamic drawing style, or as the method suggests is view dependent.
	 * \return true if the bounding box should not be cached for use on all viewports, false otherwise. */
	virtual BOOL HasViewDependentBoundingBox() { return false; }

	// Prevent accident copying / assignment.
	// It also prevents a compiler quirk/bug which will complain about
	// Animatable not having a copy constructor, in some derived classes
	// from this one.
private:
	BaseObject(const BaseObject&);
	BaseObject& operator=(const BaseObject&);
};


/** The callback object used by modifiers to deform "Deformable" objects.
 * \see  Class Object, Class Point3.
 */
class Deformer: public MaxHeapOperators {
public:
	/** Destructor. */
	virtual ~Deformer() { }
	/** This is the method that is called to deform or alter a single point. Note
	that this method needs to be thread safe. A problem may occur when a
	non-local variable is modified inside of Map(). Since two versions of Map()
	could be executing at the same time, they could both end up modifying the
	same variable simultaneously which usually causes problems. See the
	Advanced Topics section
	<a href="ms-its:3dsmaxsdk.chm::/render_thread_safe_plugins.html">Thread
	Safe Plug-Ins</a> for more details.
	\param i The index of the point to be altered. Note: An index of -1 may be passed.
	This indicates that the deformer is not being applied to a regular object
	but instead points that are generated on the fly for display purposes.
	\param p The point to be altered.
	\return  The altered point. */
	virtual Point3 Map(int i, Point3 p) = 0; 
	void ApplyToTM(Matrix3* tm);
};

/*------------------------------------------------------------------- 
Object is the class of all objects that can be pointed to by a node:
It INcludes Lights,Cameras, Geometric objects, derived objects,
and deformation Objects (e.g. FFD lattices)
It EXcludes Modifiers
---------------------------------------------------------------------*/
enum { OBJECT_LOCKED = 0x08000000 };

class ShapeObject;
class XTCObject;

/** This class represents a container class for XTCObjects.
\see  Class XTCObject
*/
class XTCContainer: public MaxHeapOperators {

public:	
	XTCObject *obj; //!< A pointer to the XTCObject.
	int prio;		//!< The priority.
	int branchID;	//!< The branch identifier.  

	/** Constructor. */
	XTCContainer(){obj = NULL; prio = 0; branchID = -1;}

};

#define IXTCACCESS_INTERFACE_ID Interface_ID(0x60b033d7, 0x3e1d4d0d)

/** Provides an interface to access Extension Channels.  
 * \see Class Object, Class XTCObject
 */
class IXTCAccess : public BaseInterface
{
public:
	/** This method returns the IXTCAccess interface ID. */
	virtual Interface_ID	GetID() { return IXTCACCESS_INTERFACE_ID; }

	/** This method allows inquiries into the actual lifetime policy
	of a client and provide a server-controlled delete notify callback.
	\return  One of the following LifetimeTypes:
	\li noRelease
	Do not call release, use interface as long as you like.
	\li immediateRelease
	The interface is only good for one calls. The release is implied so a call
	to release is not required.
	\li wantsRelease
	The clients are controlling the lifetime, so the interface needs a
	Release() when the client has finished. This is the default. 
	\li serverControlled
	The server controls the lifetime and will use the InterfaceNotifyCallback
	to inform the code when it is gone. */
	virtual LifetimeType	LifetimeControl() { return noRelease; }

	/** This method adds an extension object into the pipeline.
	\param pObj The extension object you wish to add.
	\param priority The priority to set.
	\param branchID The branch identifier to set. */
	virtual void AddXTCObject(XTCObject *pObj, int priority = 0, int branchID = -1)=0;

	/** This method returns the number of extension objects. */
	virtual int NumXTCObjects()=0;

	/** This method returns the I-th extension object.
	\param index The index of the extension object to return. */
	virtual XTCObject *GetXTCObject(int index)=0;

	/** This method allows you to remove the I-th extension object.
	\param index The index of the extension object you wish to remove. */
	virtual void RemoveXTCObject(int index)=0;

	/** This method allows you to set the priority for the I-th extension object.
	\param index The index of the extension object for which to set the priority.
	\param priority The priority to set. */
	virtual void SetXTCObjectPriority(int index,int priority)=0;

	/** This method returns the priority for the I-th extension
	object.
	\param index The index of the extension object. */
	virtual int GetXTCObjectPriority(int index)=0;

	/** This method allows you to set the branch identifier for the
	I-th extension object.
	\param index The index of the extension object.
	\param branchID The branch identifier to set. */
	virtual void SetXTCObjectBranchID(int index,int branchID)=0;

	/** This method returns the branch identifier for the I-th extension object.
	\param index The index of the extension object. */
	virtual int GetXTCObjectBranchID(int index)=0;

	/** This method has to be called whenever the CompoundObject
	updates a branch (calling Eval on it). Object *from is the object returned
	from Eval (os.obj); branchID is an int, that specifies that branch.
	The extension channel will get a callback to
	RemoveXTCObjectOnMergeBranches() and MergeXTCObject(). By
	default it returns true to RemoveXTCObjectOnMergeBranches(), which
	means, that the existing XTCObjects with that branchID will be deleted. The
	method MergeXTCObject() simply copies the XTCObjects from the
	incoming branch into the compound object.
	\param from The object from which to merge additional channels
	\param branchID The branch identifier. */
	virtual void MergeAdditionalChannels(Object *from, int branchID)=0;

	/** This method has to be called on the CompoundObject, so it can
	delete the XTCObjects for the specific branch. The XTCObject will have
	again the final decision if the XTCObject gets really deleted or not in a
	callback to RemoveXTCObjectOnBranchDeleted(), which will return
	true, if the XTCObject should be removed.
	\param branchID The branch identifier.
	\param reorderChannels TRUE to reorder the channels, otherwise FALSE. */
	virtual void BranchDeleted(int branchID, bool reorderChannels)=0;	

	/** This method copies all extension objects from the "from"
	objects into the current object. In case deleteOld is false, the objects
	will be appended. In case it is true, the old XTCObjects will be deleted.
	\param from The object to copy from.
	\param deleteOld TRUE to delete the old channel, FALSE to append the channels.
	\param bShallowCopy TRUE to create a shallow copy, FALSE to create a deep copy. */
	virtual void CopyAdditionalChannels(Object *from, bool deleteOld = true, bool bShallowCopy = false)=0;

	/** This method allows you to delete all additional channels. */
	virtual void DeleteAllAdditionalChannels()=0;
};

class XTCAccessImp;

namespace MaxGraphics {

	class IDisplayInternal;

}

/** The object class is the base class for all objects. An object is one of two
things: A procedural object or a derived object. Derived objects are part of
the system and may not be created by plug-ins. They are containers for
modifiers. Procedural objects can be many different things such as cameras,
lights, helper objects, geometric objects, etc. Methods of this class are
responsible for things such as allowing the object to be deformed (changing its
points), retrieving a deformed bounding box, converting the object between
different types (to a mesh or patch for example), texture mapping the object
(if appropriate) and interacting with the system regarding mapping. There are
other methods involved in validity intervals for the object and its channels,
and a method used to return the sub-object selection state of the object.
\par Method Groups:
See <a href="class_object_groups.html">Method Groups for Class Object</a>.
\see  Class BaseObject, Class Deformer, Class Interval, Class GraphicsWindow, 
\see Template Class Tab, <a href="ms-its:3dsmaxsdk.chm::/pipe_geometry_root.html">Geometry Pipeline System</a>
*/
class Object : public BaseObject {
	ChannelMask locked;			//!< lock flags for each channel + object locked flag.
	Interval noEvalInterval;	//!< used in ReducingCaches.
	Interval xtcValid;

	Tab<XTCContainer *> xObjs;
	XTCAccessImp *pXTCAccess;

	MaxGraphics::IDisplayInternal* mpDisplayInternal;

public:
	CoreExport Object();
	CoreExport ~Object();

	/** Indicates whether the object may be rendered. Some objects such as
	construction grids and helpers should not be rendered and can return zero.
	\return  Nonzero if the object may be rendered; otherwise 0. */
	virtual int IsRenderable()=0;  // is this a renderable object?


	/** This is the default name of the node when it is created.
	\param s The default name of the node is stored here. */
	virtual void InitNodeName(MSTR& s)=0;

	/** This method determines if the object color is used for display.
	\return  TRUE if the object color is used for display; otherwise FALSE. */
	virtual int UsesWireColor() { return TRUE; }    // TRUE if the object color is used for display

	/** If an object wants to draw itself in the 3D viewports in its selected state
	in some custom manner this method should return nonzero. If this item
	returns nonzero, the BaseObject::Display() method should respect the
	selected state of the object when it draws itself. If this method returns
	zero the system will use its standard method of showing the object as
	selected.
	\return  Nonzero if the object will draw itself in the selected state;
	otherwise 0. If nonzero, the plug-in developer is responsible for
	displaying the object in the selected state as part of its Display()
	method. */
	virtual int DoOwnSelectHilite() { return 0; }

	// This used to be in GeomObject but I realized that other types of objects may
	// want this (mainly to participate in normal align) such as grid helper objects.
	/** This method is called to compute the intersection point and surface normal
	at this intersection point of the ray passed and the object.
	\param t The time to compute the intersection.
	\param r Ray to intersect. See Class Ray.
	\param at The point of intersection.
	\param norm Surface normal at the point of intersection.
	\return  Nonzero if a point of intersection was found; otherwise 0. 
	\sa  The Mesh class implementation of this method. */
	virtual int IntersectRay(TimeValue t, Ray& r, float& at, Point3& norm) {return FALSE;}

	// Objects that don't support IntersectRay() (like helpers) can implement this
	// method to provide a default vector for normal align.
	/**	Objects that don't support the IntersectRay() method (such as helper
	objects) can implement this method to provide a default vector for use with
	the normal align command in 3ds Max.
	\param t The time to compute the normal align vector.
	\param pt The point of intersection.
	\param norm The normal at the point of intersection.
	\return  TRUE if this method is implemented to return the normal align
	vector; otherwise FALSE. */
	virtual BOOL NormalAlignVector(TimeValue t,Point3 &pt, Point3 &norm) {return FALSE;}
	//@}

	/** \name Data Flow Evaluation Methods
	* Most plug-in procedural objects do not need to be concerned 
	* with the following methods associated with locks, channels 
	* and shallow copies. The only type of plug-ins that needs to 
	* be concerned with these methods are objects that actually flow 
	* down the pipeline. Most procedural plug-ins don't go down the 
	* pipeline, instead they convert themselves to a TriObject or 
	* PatchObject, and these goes down the pipeline. It is these 
	* TriObjects or PatchObject that deal with these methods. However 
	* plug-in objects that actually flow down the pipeline will use these 
	* methods. For more information see the Advanced Topics section on 
	* the Geometry Pipeline System. 
	*/
	//@{
	// locking of object as whole. defaults to NOT modifiable.
	/** Implemented by the System. 
	This method locks the object as a whole. The object defaults to not
	modifiable. */
	void LockObject() { locked |= OBJECT_LOCKED; }

	/** Implemented by the System. 
	This method unlocks the object as a whole. */
	void UnlockObject() { locked &= ~OBJECT_LOCKED; }

	/** Implemented by the System. 
	Returns nonzero if the object is locked; otherwise 0. */
	int  IsObjectLocked() { return (locked&OBJECT_LOCKED ? 1 : 0); }

	// the validity intervals are now in the object.
	/** This method is called to evaluate the object and return the result as an
	ObjectState. When the system has a pointer to an object it doesn't know if
	it's a procedural object or a derived object. So it calls Eval() on
	it and gets back an ObjectState. A derived object managed by the system may
	have to call Eval() on its input for example. A plug-in (like a
	procedural object) typically just returns itself. 
	A plug-in that does not just return itself is the Morph Object
	(/MAXSDK/SAMPLES/OBJECTS/MORPHOBJ.CPP). This object uses a morph
	controller to compute a new object and fill in an ObjectState which it returns.
	\param t Specifies the time to evaluate the object.
	\return  The result of evaluating the object as an ObjectState.
	\par Sample Code:
	Typically this method is implemented as follows: 
	\code
	{ return ObjectState(this); }
	\endcode */
	virtual ObjectState Eval(TimeValue t)=0;

	// Access the lock flags for th specified channels
	/** Implemented by the System. 
	Locks the specified channels of the object.
	\param channels The channels to lock. */
	void LockChannels(ChannelMask channels) { locked |= channels; } 

	/** Implemented by the System. 
	Unlocks the specified channel(s) of the object.
	\param channels Specifies the channels to unlock. */
	void UnlockChannels(ChannelMask channels) { locked &= ~channels; }

	/** Implemented by the System. 
	Returns the locked status of the channels.
	\return  The channels of the object that are locked. */
	ChannelMask     GetChannelLocks() { return locked; }    

	/** Implemented by the System. 
	Sets the locked status of the object's channels.
	\param channels The channel to set to locked. */
	void SetChannelLocks(ChannelMask channels) { locked = channels; }    

	/** Implemented by the System. 
	Returns the locked status of the channels.
	\param m Not used.
	\return  The channels of the object that are locked. */
	ChannelMask GetChannelLocks(ChannelMask m) { return locked; }

                               


	// Can this object have channels cached?
	// Particle objects flow up the pipline without making shallow copies of themselves and therefore cannot be cached
	/** This method determines if this object can have channels cached. Particle
	objects flow up the pipeline without making shallow copies of themselves
	and therefore cannot be cached. Objects other than particle system can just
	use the default implementation.
	\return  TRUE if the object can be cached; otherwise FALSE. */
	virtual BOOL CanCacheObject() {return TRUE;}

	// This is called by a node when the node's world space state has
	// become invalid. Normally an object does not (and should not) be
	// concerned with this, but in certain cases (particle systems) an
	// object is effectively a world space object an needs to be notified.
	/** This is called by a node when the node's world space state has become
	invalid. Normally an object does not (and should not) be concerned with
	this, but in certain cases like particle systems an object is effectively a
	world space object an needs to be notified. */
	virtual void WSStateInvalidate() {}

	// Identifies the object as a world space object. World space
	// objects (particles for example) can not be instanced because
	// they exist in world space not object space.
	/** Returns TRUE if the object as a world space object; otherwise FALSE. World
	space objects (particles for example) can not be instanced because they
	exist in world space not object space. Objects other than particle system
	can just use the default implementation. */
	virtual BOOL IsWorldSpaceObject() {return FALSE;}

	

	// This is only valid for world-space objects (they must return TRUE for
	// the IsWorldSpaceObject method).  It locates the node which contains the
	// object.  Non-world-space objects will return NULL for this!
	CoreExport INode *GetWorldSpaceObjectNode();

	// Is the derived class derived from ParticleObject?
	virtual BOOL IsParticleSystem() {return FALSE;}

	// copy specified flags from obj
	/** Implemented by the System. 
	Copies the specified channels from the object passed.
	\param obj The source object.
	\param needChannels Indicates the channels to copy. */
	CoreExport void CopyChannelLocks(Object *obj, ChannelMask needChannels);
	// topology has been changed by a modifier -- update mesh strip/edge lists
	virtual void TopologyChanged() { }


	//
	// does this object implement the generic Deformable Object procs?
	//
	/** Indicates whether this object is deformable. A deformable object is simply
	an object with points that can be modified. Deformable objects must
	implement the generic deformable object methods (NumPoints(),
	GetPoint(i), SetPoint(i), Deform()). 
	A deformable object is simply an object with points that can be modified.
	These points can be stored in any form the object wants. They are accessed
	through a virtual array interface with methods to get and set the 'i-th'
	point. If an object has tangents for instance, it would convert them to and
	from points as necessary. For example, a simple Bezier spline object that
	stored its control handles relative to the knot would convert them to be
	absolute when GetPoint() was called with 'i' specifying one of the
	control points. When the control point is later set, the object can convert
	it back to be relative to its knot. At this point it could also apply any
	constraints that it may have, such as maintaining a degree of continuity.
	The idea is that the entity calling GetPoint(i) and
	SetPoint(i) doesn't care what the point represents. It will simply
	apply some function to the point.
	\note The Deformable object methods only need to be implemented if the object
	returns TRUE from this method. 
	\return  Return nonzero if the object is deformable and implements the
	generic deformable object methods; otherwise 0.	*/
	virtual int IsDeformable() { return 0; } 

	// DeformableObject procs: only need be implemented  
	// IsDeformable() returns TRUE.
	/** The points of a deformable object are accessed through a virtual array
	interface. This method specifies the number of points in the object. The
	meaning of 'points' is defined by the object. A TriObject uses the vertices
	as the points for example. b>
	\return  The number of points in the object. */
	virtual int NumPoints(){ return 0;}

	/** The points of a deformable object are accessed through a virtual array
	interface. This method returns the 'i-th' point of the object.
	\note If your plug-in is a modifier and you want to operate on the selected
	points of the object you are modifying, you can't tell which points are
	selected unless you know the type of object. If it is a generic deformable
	object there is no way of knowing since the way the object handles
	selection is up to it. Therefore, if you want to operate on selected points
	of a generic deformable object, use a Deformer.
	\param i Specifies which point should be returned.
	\return  The 'i-th' point of the object. */
	virtual Point3 GetPoint(int i) { return Point3(0,0,0); }

	/** The points of a deformable object are accessed through a virtual array
	interface. This method stores the 'i-th' point of the object.
	\param i The index of the point to store.
	\param p The point to store. */
	virtual void SetPoint(int i, const Point3& p) {}             

	// Completes the deformable object access with two methods to
	// query point selection. 
	// IsPointSelected returns a TRUE/FALSE value
	// PointSelection returns the weighted point selection, if supported.
	// Harry D, 11/98
	/**	Returns TRUE if the 'i-th' point is selected; otherwise FALSE.
	\param i The zero based index of the point to check. */
	virtual BOOL IsPointSelected (int i) { return FALSE; }

	/**	Returns a floating point weighted point selection if the object supports
	it. The default implementation just returns 1.0f if selected and 0.0f if
	not.
	\param i The zero based index of the point to check. */
	virtual float PointSelection (int i) {
		return IsPointSelected(i) ? 1.0f : 0.0f;
	}


	// These allow the NURBS Relational weights to be modified
	/**	Returns TRUE if the object has weights for its points that can be set;
	otherwise FALSE. */
	virtual BOOL HasWeights() { return FALSE; }

	/**	Returns the weight of the specified point of the object.
	\param i The point to return the weight of. */
	virtual double GetWeight(int i) { return 1.0; }

	/**	Sets the weight of the specified point.
	\param i The point whose weight to set.
	\param w The value to set. */
	virtual void SetWeight(int i, const double w) {}


	// Get the count of faces and vertices for the polyginal mesh
	// of an object.  If it return FALSE, then this function
	// isn't supported.  Plug-ins should use GetPolygonCount(Object*, int&, int&)
	// to count the polys in an arbitrary object
	/**	Retreives the number of faces and vertices of the polyginal mesh
	representation of this object. If this method returns FALSE then this
	functionality is not supported. 
	Note: Plug-In developers should use the global function
	GetPolygonCount(Object*, int\&, int\&) to retrieve the number f
	vertices and faces in an arbitrary object.
	\param t The time at which to compute the number of faces and vertices.
	\param numFaces The number of faces is returned here.
	\param numVerts The number of vertices is returned here.
	\return  TRUE if the method is fully implemented; otherwise FALSE. */
	virtual BOOL PolygonCount(TimeValue t, int& numFaces, int& numVerts) { return FALSE; }
	// informs the object that its points have been deformed,
	// so it can invalidate its cache.
	/** Informs the object that its points have been deformed, so it can invalidate
	its cache. A developer who uses the GetPoint() / SetPoint()
	approach to modifying an object will call PointsWereChanged() to
	invalidate the object's cache. For example, if a modifier calls
	SetPoint(), when it is finished it should call this method so the
	object can invalidate and/or update its bounding box and any other data it
	might cache. */
	virtual void PointsWereChanged(){}

	// deform the object with a deformer.
	/** This is the method used to deform the object with a deformer. The developer
	should loop through the object's points calling the defProc for each
	point (or each selected point if useSel is nonzero). 
	The Deform() method is mostly a convenience. Modifiers can implement
	a 'Deformer' callback object which is passed to the Deform() method.
	The object then iterates through its points calling their deformer's
	callback for each point. The only difference between using the
	Deform() method as opposed to iterating through the points is that
	the Deform() method should respect sub-object selection. For
	example, the TriObject's implementation of Deform() iterates through
	its vertices, if the TriObject's selection level is set to vertex then it
	only calls the Deformer's callback for vertices that are selected. This way
	modifiers can be written that can be applied only to selection sets without
	any specific code to check selected points. The default implementation of
	this method just iterates through all points using GetPoint(i) and
	SetPoint(i). If an object supports sub-object selection sets then it
	should override this method.
	\param defProc A pointer to an instance of the Deformer class. This is the callback object
	that actually performs the deformation.
	\param useSel A flag to indicate if the object should use the selected points only. If
	nonzero the selected points are used; otherwise all the points of the object are used.
	\par Default Implementation:
	\code
	void Object::Deform(Deformer *defProc,int useSel)
	{
		int nv = NumPoints();
		for (int i=0; i<nv; i++)
			SetPoint(i,defProc->Map(i,GetPoint(i)));
		PointsWereChanged();
	}
	\endcode
	\par Sample Code:
	This code shows the TriObject implementation of this method.
	Note how it looks at the useSel parameter to only call the
	selected points if required. 
	\code
	void TriObject::Deform(Deformer *defProc,int useSel)
	{
		int nv = NumPoints();
		int i;
		if ( useSel ) {
			BitArray sel = mesh.VertexTempSel();
			float *vssel = mesh.getVSelectionWeights ();
			if (vssel) {
				for (i=0; i<nv; i++) {
					if(sel[i]) {
						SetPoint(i,defProc->Map(i,GetPoint(i)));
						continue;
					}
					if (vssel[i]==0) continue;
					Point3 & A = GetPoint(i);
					Point3 dir = defProc->Map(i,A) - A;
					SetPoint(i,A+vssel[i]*dir);
				}
			}
			else {
				for (i=0; i<nv; i++) if (sel[i])
					SetPoint(i,defProc->Map(i,GetPoint(i)));
			}
		}
		else {
			for (i=0; i<nv; i++)
				SetPoint(i,defProc->Map(i,GetPoint(i)));
		}
		PointsWereChanged();
	}
	\endcode  */
	CoreExport virtual void Deform(Deformer *defProc, int useSel=0);

	// box in objects local coords or optional space defined by tm
	// If useSel is true, the bounding box of selected sub-elements will be taken.
	/** This method computes the bounding box in the objects local coordinates or
	the optional space defined by tm. 
	Note: If you are looking for a precise bounding box, use this method and
	pass in the node's object TM (INode::GetObjectTM()) as the matrix.
	\param t The time to compute the box.
	\param box A reference to a box the result is stored in.
	\param tm This is an alternate coordinate system used to compute the box. If the
	tm is not NULL this matrix should be used in the computation of the result.
	\param useSel If TRUE, the bounding box of selected sub-elements should be computed;
	otherwise the entire object should be used. */
	CoreExport virtual void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm=NULL, BOOL useSel=FALSE );

	//
	// does this object implement the generic Mappable Object procs?
	//
	/** This method lets you know if the ApplyUVWMap() method
	is available for this object. This is used by things like the UVW mapping
	modifier, so that it can determine which objects can have their mapping
	modified. Returns nonzero if the object is mappable; otherwise zero. */
	virtual int IsMappable() { return 0; }

	/**	Returns the maximum number of channels supported by this type of object.
	TriObjects for instance return MAX_MESHMAPS which is currently set
	to 100. */
	virtual int NumMapChannels () { return IsMappable(); }	// returns number possible.

	/**	Returns the number of maps currently used by this object. This is at least
	1+(highest channel in use). This is used so a plug-in that does something
	to all map channels doesn't always have to do it to every channel up to
	MAX_MESHMAPS but rather only to this value. */
	virtual int NumMapsUsed () { return NumMapChannels(); }	// at least 1+(highest channel in use).

	// This does the texture map application -- Only need to implement if
	// IsMappable returns TRUE
	/** This method may be called to map the object with UVW mapping
	coordinates. If the object returns nonzero from IsMappable() then
	this method should be implemented.
	\param type The mapping type. One of the following values: 
	MAP_PLANAR\n
	MAP_CYLINDRICAL\n
	MAP_SPHERICAL\n
	MAP_BALL\n
	MAP_BOX\n
	\param utile Number of tiles in the U direction.
	\param vtile Number of tiles in the V direction.
	\param wtile Number of tiles in the W direction.
	\param uflip If nonzero the U values are mirrored.
	\param vflip If nonzero the V values are mirrored.
	\param wflip If nonzero the W values are mirrored.
	\param cap This is used with MAP_CYLINDRICAL. If nonzero, then any face normal
	that is pointing more vertically than horizontally will be mapped using planar coordinates.
	\param tm This defines the mapping space. As each point is mapped, it is multiplied
	by this matrix, and then it is mapped.
	\param channel This indicates which channel the mapping is applied to. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_mapping_channel_index_values.html">List of Mapping
	Channel Index Values</a>. */
	virtual void ApplyUVWMap(int type,
		float utile, float vtile, float wtile,
		int uflip, int vflip, int wflip, int cap,
		const Matrix3 &tm,int channel=1) {}

	// Objects need to be able convert themselves 
	// to TriObjects. Most modifiers will ask for
	// Deformable Objects, and triobjects will suffice.

	/** Indicates whether the object can be converted to the specified type. If the
	object returns nonzero to indicate it can be converted to the specified
	type, it must handle converting to and returning an object of that type
	from ConvertToType().
	\sa Class ObjectConverter for additional details on converting objects between types.
	\param obtype The Class_ID of the type of object to convert to. See
	Class Class_ID, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_class_ids.html">List of Class_IDs</a>.
	\return  Nonzero if the object can be converted to the specified type;
	otherwise 0.
	\par Default Implementation:
	{ return 0; } */
	CoreExport virtual int CanConvertToType(Class_ID obtype);

	// Developers have to make sure, that the channels, that the BaseObject implements 
	// (e.g. ExtensionChannels) are copied over to the new object as well. They can do this by simply
	// calling CopyXTCObjects(this,false); The validity will be automatically copied with it..
	/** This method converts this object to the type specified and returns a
	pointer it. Note that if ConvertToType() returns a new object it should be
	a completely different object with no ties (pointers or references) to the
	original.
	\see class ObjectConverter
	for additional details on converting objects between types.
	\par The following is an issue that developers of world space modifiers need to
	be aware of if the world space modifier specifies anything but generic
	deformable objects as its input type. In other words, if a world space
	modifier, in its implementation of Modifier::InputType(), doesn't
	specifically return defObjectClassID then the following issue
	regarding the 3ds Max pipeline needs to be considered. Developers of other
	plug-ins that don't meet this condition don't need to be concerned with
	this issue.
	\par World space modifiers that work on anything other than generic deformable
	objects are responsible for transforming the points of the object they
	modify into world space using the ObjectState TM. To understand why this is
	necessary, consider how 3ds Max applies the node transformation to the
	object flowing down the pipeline.
	\par In the geometry pipeline architecture, the node in the scene has its
	transformation applied to the object in the pipeline at the transition
	between the last object space modifier and the first world space modifier.
	The node transformation is what places the object in the scene -- thus this
	is what puts the object in world space. The system does this by
	transforming the points of the object in the pipeline by the node
	transformation. This is only possible however for deformable objects.
	Deformable objects are those that support the
	Object::IsDeformable(), NumPoints(), GetPoint() and
	SetPoint() methods. These deformable objects can be deformed by the
	system using these methods, and thus the system can modify the points to
	put them in world space itself.
	\par If a world space modifier does not specify that it works on deformable
	objects, the system is unable to transform the points of the object into
	world space. What it does instead is apply the transformation to the
	ObjectState TM. In this case, a world space modifier is responsible for
	transforming the points of the object into world space itself, and then
	setting the ObjectState TM to the identity. There is an example of this in
	the sample code for the Bomb space warp. The Bomb operates on
	TriObjects and implements InputType() as { return
	Class_ID(TRIOBJ_CLASS_ID,0); }. Since it doesn't specifically return
	defObjectClassID, it is thus responsible for transforming the points
	of the object into world space itself. It does this in its implementation
	of ModifyObject() as follows: 
	\code
	if (os->GetTM())
	{
		Matrix3 tm = *(os->GetTM());
		for (int i=0; i<triOb->mesh.getNumVerts(); i++) {
			triOb->mesh.verts[i] = triOb->mesh.verts[i] *tm;
		}
		os->obj->UpdateValidity(GEOM_CHAN_NUM,os->tmValid());
		os->SetTM(NULL,FOREVER);
	}
	\endcode
	As the code above shows, the Bomb checks if the ObjectState TM is non-NULL.
	If it is, the points of the object are still not in world space and thus
	must be transformed. It does this by looping through the points of the
	TriObject and multiplying each point by the ObjectState TM. When it
	is done, it sets the ObjectState TM to NULL to indicate the points are now
	in world space. This ensure that any later WSMs will not transform the
	points with this matrix again.
	\par For the Bomb world space modifier this is not a problem since it specifies
	in its implementation of ChannelsChanged() that it will operate on
	the geometry channel (PART_GEOM). Certain world space modifiers may
	not normally specify PART_GEOM in their implementation of
	ChannelsChanged(). Consider the camera mapping world space modifier.
	Its function is to apply mapping coordinates to the object it is applied
	to. Thus it would normally only specify PART_TEXMAP for
	ChannelsChanged(). However, since it operates directly on
	TriObjects, just like the Bomb, the system cannot transform the
	points into world space, and therefore the camera mapping modifier must do
	so in its implementation of ModifyObject(). But since it is actually
	altering the points of the object by putting them into world space it
	is altering the geometry channel. Therefore, it should really
	specify PART_GEOM | PART_TEXMAP in its implementation of
	ChannelsChanged(). If it didn't do this, but went ahead and modified
	the points of the object anyway, it would be transforming not copies of the
	points, but the original points stored back in an earlier cache or even the
	base object.
	\par This is the issue developers need to be aware of. To state this in simple
	terms then: Any world space modifier that needs to put the points of the
	object into world space (since it doesn't implement InputType() as
	defObjectClassID) needs to specify PART_GEOM in its
	implementation of ChannelsChanged().
	\param t The time at which to convert.
	\param obtype The Class_ID of the type of object to convert to. See
	Class Class_ID, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_class_ids.html">List of Class_IDs</a>.
	\return  A pointer to an object of type obtype.
	\par Default Implementation:
	{ return NULL; }
	\par Sample Code:
	The following code shows how a TriObject can be retrieved from a
	node. Note on the code that if you call ConvertToType() on an object
	and it returns a pointer other than itself, you are responsible for
	deleting that object.
	\code
	// Retrieve the TriObject from the node
	int deleteIt;
	TriObject *triObject = GetTriObjectFromNode(ip->GetSelNode(0),deleteIt);
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
		Object *obj = node->EvalWorldState(0).obj;
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) {
			TriObject *tri = (TriObject *) obj->ConvertToType(0,Class_ID(TRIOBJ_CLASS_ID, 0));
	// Note that the TriObject should only be deleted
	// if the pointer to it is not equal to the object
	// pointer that called ConvertToType()
			if (obj != tri) 
				deleteIt = TRUE;
			return tri;
		}
		else {
			return NULL;
		}
	}
	\endcode */
	CoreExport virtual Object* ConvertToType(TimeValue t, Class_ID obtype);
	// Indicate the types this object can collapse to
	/**	This method allows objects to specify the class that is the best class to
	convert to when the user collapses the stack. The main base classes have
	default implementations. For example, GeomObject specifies TriObjects as
	its preferred collapse type and shapes specify splines as their preferred
	collapse type
	\return  The Class_ID of the preferred object type. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_class_ids.html">List of Class_IDs</a>. */
	virtual Class_ID PreferredCollapseType() {return Class_ID(0,0);}

	/**	When the user clicks on the Edit Stack button in the modify branch a list of
	'Convert To:' types is presented. The use may click on one of these choices to
	collapse the object into one of these types (for instance, an Editable Mesh or
	an Editable NURBS object). This method returns a list of Class_IDs and
	descriptive strings that specify the allowable types of objects that this
	object may be collapsed into. 
	Note: Most plug-ins call the base class method in Object in their
	implementation of this method. The base class implementation provided by
	Object checks if the object can convert to both an editable mesh and an
	editable spline. If it can, these are added to the allowable types.
	\param clist The table of allowable Class_IDs.
	\param nlist The table of pointers to strings that correspond to the table of Class_IDs
	above.
	\par Sample Code:
	\code
	void SphereObject::GetCollapseTypes(Tab<Class_ID> &clist,Tab<MSTR*>&nlist)
	{
		Object::GetCollapseTypes(clist, nlist);
		Class_ID id = EDITABLE_SURF_CLASS_ID;
		MSTR *name = new MSTR(GetString(IDS_SM_NURBS_SURFACE));
		clist.Append(1,&id);
		nlist.Append(1,&name);
	}
	\endcode */
	CoreExport virtual void GetCollapseTypes(Tab<Class_ID> &clist,Tab<MSTR*> &nlist);

	/**	This method is called on the world space cache object when the stack gets
	collapsed, that lets the pipeline object decide, if it wants to return a
	different object than itself. The default implementation simply returns
	this. A PolyObject e.g. can create and return an EditablePolyObject in this
	method, so that the collapsed object has a UI. I only implemented this
	method for PolyObject, but this can potentially implemented that way for
	all pipeline objects, that currently pass up the editable version. 
	It is important, that all places, that collapse the stack are calling this
	method after evaluating the stack.
	\par It also is important, that the editable version implements this method to
	simply return this, otherwise you'll get a non-editable object when you
	collapse an editable polyobject.
	\return  A pointer to the resulting object. */
	virtual Object *CollapseObject() { return this;}

	// return the current sub-selection state
	/** For objects that have sub selection levels, this method returns the current
	selection level of the object. For example, a TriObject has the following
	selection levels: object, vertex, face, edge. Other object types may have
	different selection levels. The only standard is that a value of 0
	indicates object level. b>
	\return  The current selection level of the object. */
	virtual DWORD GetSubselState() {return 0;} 
	virtual void SetSubSelState(DWORD s) {}



	// If the requested channels are locked, replace their data
	// with a copy/ and unlock them, otherwise leave them alone
	/** Implemented by the System.
	\par If the requested channels are locked, this method will replace their data
	with a copy and unlock them, otherwise it leaves them alone.
	\param channels The channels to ready for modification. */
	CoreExport void ReadyChannelsForMod(ChannelMask channels);




	// Virtual methods to be implemented by plug-in object:-----

	// NS: 12-14-99 Classes that derive from Object *have* to implement certain
	// things to support the new Extension Channel (XTCObject)
	// See examples in Triobj.cpp [START]

	// access the current validity interval for the nth channel
	// For this method, the derived class has to check if the channel is the 
	// Extension channel and return the base classes ChannelValidity :
	// case EXTENSION_CHAN_NUM: return Object::ChannelValidity(t,nchan); break;
	/** Retrieve the current validity interval for the nchan channel of the
	object.
	\note Most procedural objects won't implement this method since they
	don't have individual channels. Developers wanting to get the validity
	interval for a procedural object should use Object::ObjectValidity()
	instead.
	\param t The time to retrieve the validity interval of the channel.
	\param nchan Specifies the channel to return the validity interval of. See
	<a href="#Object_Channel_Indices">Object Channel Indices</a>.
	\return  The validity interval of the specified channel. */
	CoreExport virtual Interval ChannelValidity(TimeValue t, int nchan);

	// The derived class has to simply call the implementation of the baseclass.
	/** Sets the validity interval of the specified channel.
	\param nchan Specifies the channel. See <a href="#Object_Channel_Indices">Object Channel Indices</a>
	\param v The validity interval for the channel. */
	CoreExport virtual void SetChannelValidity(int nchan, Interval v);

	// invalidate the specified channels
	// The derived class has to simply call the implementation of the baseclass.
	/** This method invalidates the intervals for the given channel mask. This just
	sets the validity intervals to empty (calling SetEmpty() on the interval).
	\param channels Specifies the channels to invalidate. */
	CoreExport virtual void InvalidateChannels(ChannelMask channels);

	// validity interval of Object as a whole at current time
	// The derived class has incorporate the BaseClasses validity into the returned validity
	/** This method returns the validity interval of the object as a whole at the
	specified time.
	\param t The time to compute the validity interval.
	\par Default Implementation:
	{ return FOREVER; }
	\return  The validity interval of the object. */
	CoreExport virtual Interval ObjectValidity(TimeValue t);



	// Makes a copy of its "shell" and shallow copies only the
	// specified channels.  Also copies the validity intervals of
	// the copied channels, and sets Invalidates the other intervals.
	// The derived class has to call ShallowCopy on the BaseClass, so it can copy all its channels
	/** This method must make a copy of its "shell" and then shallow copy (see
	below) only the specified channels. It must also copy the validity
	intervals of the copied channels, and invalidate the other intervals.
	\param channels The channels to copy.
	\return  A pointer to the shallow copy of the object. */
	virtual Object *MakeShallowCopy(ChannelMask channels) { return NULL; }

	// Shallow-copies the specified channels from the fromOb to this.
	// Also copies the validity intervals. 
	// The derived class has to simply call the implementation of the baseclass.
	/** This method copies the specified channels from the fromOb to
	this and copies the validity intervals. 
	A plug-in needs to copy the specified channels from the specified object
	fromOb to itself by just copying pointers (not actually copying the
	data). No new memory is typically allocated, this method is just copying
	the pointers.
	\param fromOb Object to copy the channels from.
	\param channels Channels to copy. */
	CoreExport virtual void ShallowCopy(Object* fromOb, ChannelMask channels);



	// Free the specified channels
	// The derived class has to simply call the implementation of the baseclass.
	/** This method deletes the memory associated with the specified channels and
	set the intervals associated with the channels to invalid (empty).
	\param channels Specifies the channels to free. */
	CoreExport virtual void FreeChannels(ChannelMask channels);   
	
		// This replaces locked channels with newly allocated copies.
	// It will only be called if the channel is locked.
	// The derived class has to simply call the implementation of the baseclass.
	/** This method replaces the locked channels with newly allocated copies. It
	will only be called if the channel is locked.
	\param channels The channels to be allocate and copy. */
	CoreExport virtual void NewAndCopyChannels(ChannelMask channels); 
	
	

	// Allow the object to enlarge its viewport rectangle, if it wants to.
	// The derived class has to simply call the implementation of the baseclass.
	/**	This method allows the object to enlarge its viewport rectangle, if it
	wants to. The system will call this method for all objects when calculating
	the viewport rectangle; the object can enlarge the rectangle if desired.
	This is used by the Editable Spline code to allow extra room for vertex
	serial numbers, which can extend outside the normal bounding rectangle.
	\param gw Points to the GraphicsWindow associated with the viewport.
	\param rect The enlarged rectangle is returned here.
	\par Default Implementation:
	{}
	\par Sample Code:
	\code
	void SplineShape::MaybeEnlargeViewportRect(GraphicsWindow *gw, Rect	&rect)
	{
		if(!showVertNumbers)
			return;
		MCHAR dummy[256];
		SIZE size;
		int maxverts = -1;
		for(int i = 0; i < shape.splineCount; ++i) {
			int verts = shape.splines[i]->KnotCount();
			if(verts > maxverts)
				maxverts = verts;
		}
		sprintf(dummy,"%d",maxverts);
		gw->getTextExtents(dummy, &size);
		rect.SetW(rect.w() + size.cx);
		rect.SetY(rect.y() - size.cy);
		rect.SetH(rect.h() + size.cy);
	}
	\endcode */
	CoreExport virtual void MaybeEnlargeViewportRect(GraphicsWindow *gw, Rect &rect);


	// quick render in viewport, using current TM.
	//CoreExport virtual Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) { return 0; };   

	// NS: 12-14-99 Classes that derive from Object *have* o call the following methods
	// See examples in Triobj.cpp [END]

	CoreExport bool IsBaseClassOwnedChannel(int nchan) { return (nchan == EXTENSION_CHAN_NUM) ? true : false;}
  
   	/** When a modifier is applied to an object, it needs to include its own
	validity interval in the interval of the object. To do this, a modifier
	calls the UpdateValidity() method of an object. This method
	intersects interval v to the nchan channel validity of the
	object.
	\param nchan The validity interval of the modifier is intersected with this channel of
	the object. See <a href="#Object_Channel_Indices">Object Channel Indices</a>.
	\param v The interval to intersect. */
	CoreExport void UpdateValidity(int nchan, Interval v);  // AND in interval v to channel validity

	/** \internal This method is used internally. */
	Interval GetNoEvalInterval() { return noEvalInterval; }
	
	/** \internal This method is used internally. */
	void SetNoEvalInterval(Interval iv) {noEvalInterval = iv; }

	// Give the object chance to reduce its caches, 
	// depending on the noEvalInterval.
	/** This method give the object the chance to reduce its caches.
	\param t The time to discard any caches the object has. */
	CoreExport virtual void ReduceCaches(TimeValue t);


             	// Is this object a construction object:
	/** This is called to determine if this is a construction object or not.
	\return  Nonzero if the object is a construction object; otherwise 0. */
	virtual int IsConstObject() { return 0; }  


	// Retreives sub-object branches from an object that supports branching.
	// Certain objects combine a series of input objects (pipelines) into
	// a single object. These objects act as a multiplexor allowing the
	// user to decide which branch(s) they want to see the history for.
	//
	// It is up to the object how they want to let the user choose. The object
	// may use sub object selection to allow the user to pick a set of
	// objects for which the common history will be displayed.
	// 
	// When the history changes for any reason, the object should send
	// a notification (REFMSG_BRANCHED_HISTORY_CHANGED) via NotifyDependents.

	// The selected parameter is new in Rel. 4 and must be supported by all
	// compound objects.
	// In case the selected parameter is true the obejct should only return
	// the number of pipebranches, that are currently selected in the UI (this
	// is the way it worked in R3 and before.
	// In case this parameter is false, the object has to return the number of 
	// *all* branches, no matter if they are selected or not


	/** This method returns the number of pipeline branches combined by the object.
	This is not the total number of branches, but rather the number that are
	active. For example in the boolean object, if the user does not have any
	operands selected, this methods would return zero. If they have one
	selected it would return one.
	\param selected This parameter must be supported by all compound objects. In case the
	selected parameter is true the object should only return the number of
	pipebranches, that are currently selected in the UI (this is the way it
	worked in R3 and before. In case this parameter is false, the object has to
	return the number of all branches, no matter if they are selected or not */
	virtual int NumPipeBranches(bool selected = true) {return 0;}

	// The selected parameter is new in Rel. 4 and must be supported by all
	// compound objects.
	// In case the selected parameter is true the obejct should only consider
	// the branches, that are currently selected in the UI (this
	// is the way it worked in R3 and before.
	// In case this parameter is false, the object has to consider 
	// *all* branches, no matter if they are selected or not

	/** Retrieves sub-object branches from an object that supports branching.
	Certain objects combine a series of input objects (pipelines) into a single
	object. These objects act as a multiplexer allowing the user to decide
	which branch(s) they want to see the history for. 
	It is up to the object how they want to let the user choose. For example
	the object may use sub-object selection to allow the user to pick a set of
	objects for which the common history will be displayed. 
	When the history changes for any reason, the object should send a
	notification (\ref REFMSG_BRANCHED_HISTORY_CHANGED) using \ref NotifyDependents().
	\param i The branch index.
	\param selected  This parameter must be supported by all compound objects. In case the
	selected parameter is true the object should only return the number of
	pipebranches, that are currently selected in the UI (this is the way it
	worked in R3 and before. In case this parameter is false, the object has to
	return the number of all branches, no matter if they are selected or not
	\return  The 'i-th' sub-object branch. */
	virtual Object *GetPipeBranch(int i, bool selected = true) {return NULL;}

	// When an object has sub-object branches, it is likely that the
	// sub-objects are transformed relative to the object. This method
	// gives the object a chance to modify the node's transformation so
	// that operations (like edit modifiers) will work correctly when 
	// editing the history of the sub object branch.

	// The selected parameter is new in Rel. 4 and must be supported by all
	// compound objects.
	// In case the selected parameter is true the obejct should only consider
	// the branches, that are currently selected in the UI (this
	// is the way it worked in R3 and before.
	// In case this parameter is false, the object has to consider 
	// *all* branches, no matter if they are selected or not

	/** When an object has sub-object branches, it is likely that the sub-objects
	are transformed relative to the object. This method gives the object a
	chance to modify the node's transformation so that operations (like edit
	modifiers) will work correctly when editing the history of the sub object
	branch. An object can implement this method by returning a pointer to a new
	INodeTransformed that is based on the node passed into this method.
	See Class INodeTransformed.
	\param t The time to get the INode.
	\param node The original INode pointer.
	\param i The branch index.
	\param selected This parameter must be supported by all compound objects. In case the
	selected parameter is true the object should only return the number of
	pipebranches, that are currently selected in the UI (this is the way it
	worked in R3 and before. In case this parameter is false, the object has to
	return the number of all branches, no matter if they are selected or not
	\return  A pointer to an INode. This can be the original passed in (the
	default implementation does this) or a new INodeTransformed. */
	virtual INode *GetBranchINode(TimeValue t,INode *node,int i, bool selected = true) {return node;}
	//@}

	/** \name Shapes Within Objects
	* Shape viewports can reference shapes contained within objects, 
	* so the system needs to be able to access the shapes within an object. 
	* The following four methods provide this access. These methods are used 
	* by the loft object. Since loft objects are made up of shapes, this 
	* gives the system the ability to query the object to find out if it is 
	* a shape container. Most objects don't contain shapes so they can just 
	* use the default implementations. 
	*/
	//@{
	// Shape viewports can reference shapes contained within objects, so we
	// need to be able to access shapes within an object.  The following methods
	// provide this access
	/** Returns the number of shapes contained inside this object. A shape
	container may return zero if it doesn't currently have any shapes.
	\return  The number of shapes. A return value of -1 indicates this is not a
	container. */
	virtual int NumberOfContainedShapes() { return -1; }    // NOT a container!

	/** This method returns the ShapeObject specified by the index passed at
	the time specified. See Class ShapeObject.
	\param t The time to return the shape.
	\param index The index of the shape. */
	virtual ShapeObject *GetContainedShape(TimeValue t, int index) { return NULL; }

	/** Returns the matrix associated with the shape whose index is passed. This
	matrix contains the offset within the object used to align the shape
	viewport to the shape.
	\param t The time to return the matrix.
	\param index The index of the shape whose matrix to return.
	\param mat The matrix is returned here. */
	virtual void GetContainedShapeMatrix(TimeValue t, int index, Matrix3 &mat) {}

	/** This is used by the lofter. The lofter can have several shapes selected,
	and the bit array returned here will have a bit set for each selected
	shape. See Class BitArray.
	\return */
	virtual BitArray ContainedShapeSelectionArray() { return BitArray(); }

	// Return TRUE for ShapeObject class or GeomObjects that are Shapes too
	virtual BOOL IsShapeObject() { return FALSE; }

	// For debugging only. TriObject inplements this method by making sure
	// its face's vert indices are all valid.
	/** This method is used for debugging only. The TriObject implements this
	method by making sure its face's vert indices are all valid.
	\return  TRUE if valid; otherwise FALSE. */
	virtual BOOL CheckObjectIntegrity() {return TRUE;}              

	// Find out if the Object is generating UVW's
	virtual BOOL HasUVW() { return 0; }
	// or on any map channel:
	virtual BOOL HasUVW (int mapChannel) { return (mapChannel==1) ? HasUVW() : FALSE; }

	
	// This is overridden by DerivedObjects to search up the pipe for the base object
	/** It is called to return a pointer to the base object (an object that is not
	a derived object). This method is overridden by DerivedObjects to search
	down the pipeline for the base object. The default implementation just
	returns this. This function is still implemented by derived objects
	and WSM's to search down the pipeline. This allows you to just call it on a
	nodes ObjectRef without checking for type. */
	virtual Object *FindBaseObject() { return this;	}

	// Access a parametric position on the surface of the object
	/**	There are several methods used to access a parametric position on the
	surface of the object. If this method returns TRUE then
	Object::GetSurfacePoint() will be called to return a point on the
	surface that corresponds to the u and v parameters passed to
	it. If this method returns FALSE then it is assumed the object does not
	support returning a point on the surface based on parameteric values. For
	sample code see /MAXSDK/SAMPLES/OBJECTS/SPHERE.CPP. If the object
	has several parametric surfaces then a second version of
	GetSurfacePoint() with an integer which specifies which surface will
	be called. */
	virtual BOOL IsParamSurface() {return FALSE;}

	/**	Returns the number of parametric surfaces within the object.
	\param t The time at which to check. */
	virtual int NumSurfaces(TimeValue t) {return 1;}

	// Single-surface version (surface 0)
	/**	This method needs to be implemented if Object::IsParamSurface()
	returns TRUE. This method is used to retrieve a point on the surface of the
	object based on two parameters of the surface, u and v. 
	Note: This method assumes there is a single parametric surface. If there is
	more than 1 (NumSurfaces() returns \> 1, use the GetSurface()
	method below which allows for multiple surfaces.
	\param t The time to retrieve the point.
	\param u The parameter along the horizontal axis of the surface.
	\param v The parameter along the vertical axis of the surface.
	\param iv This interval is updated based on the interval of the surface parameter. */
	virtual Point3 GetSurfacePoint(TimeValue t, float u, float v,Interval &iv) {return Point3(0,0,0);}

	// Multiple-surface version (Implement if you override NumSurfaces)
	/**	This method is used to retrieve a point on the specified surface of the
	object based on two parameters of the surface, u and v.
	\param t The time to retrieve the point.
	\param surface The zero based index of the surface. This number is \>=0 and
	\<NumSurfaces().
	\param u The parameter along the horizontal axis of the surface.
	\param v The parameter along the vertical axis of the surface.
	\param iv This interval is updated based on the interval of the surface parameter. */
	virtual Point3 GetSurfacePoint(TimeValue t, int surface, float u, float v,Interval &iv) {return Point3(0,0,0);}

	// Get information on whether a surface is closed (default is closed both ways)
	/**	This method allows the object to return flags that indicate whether the
	parametric surface is closed in the U and V dimensions. Set the appropriate
	closure variables to TRUE if the surface is closed in that direction, FALSE
	if it is not. A torus, for example, is closed in both directions.
	\param t The time to check the surface.
	\param surface The zero based index of the surface. This number is \>=0 and
	\<NumSurfaces().
	\param uClosed Set to TRUE if the surface is closed in U; otherwise to FALSE.
	\param vClosed Set to TRUE if the surface is closed in V; otherwise to FALSE. */
	virtual void SurfaceClosed(TimeValue t, int surface, BOOL &uClosed, BOOL &vClosed) {uClosed = vClosed = TRUE;}

	
	// Allow an object to return extended Properties fields
	// Return TRUE if you take advantage of these, and fill in all strings
	/**	This method allows an object to return extended Properties fields. It is
	called when the Object Properties dialog is being prepared. If you don't
	want to display any extended properties, simply return FALSE. 
	To display extended property fields, place the field label in the
	appropriate label string and the display value in a formatted string. Two
	fields are supplied, each with a label and a data string; if only using
	one, make the second label field and data field blank (""). Return TRUE to
	indicate you have filled in the fields. The properties dialog will display
	your returned values.
	\param t The time at which the strings are requested.
	\param prop1Label The string for the property 1 label.
	\param prop1Data The formatted data string to appear as property 1.
	\param prop2Label The string for the property 2 label.
	\param prop2Data The formatted data string to appear as property 2.
	\return  TRUE if this method is implemented and the fields are filled in;
	otherwise FALSE. */
	virtual BOOL GetExtendedProperties(TimeValue t, MSTR &prop1Label, MSTR &prop1Data, MSTR &prop2Label, MSTR &prop2Data) {return FALSE;}

	// Animatable Overides...

	CoreExport SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
	CoreExport bool SvHandleDoubleClick(IGraphObjectManager *gom, IGraphNode *gNode);
	CoreExport MSTR SvGetName(IGraphObjectManager *gom, IGraphNode *gNode, bool isBeingEdited);
	CoreExport COLORREF SvHighlightColor(IGraphObjectManager *gom, IGraphNode *gNode);
	CoreExport bool SvIsSelected(IGraphObjectManager *gom, IGraphNode *gNode);
	CoreExport MultiSelectCallback* SvGetMultiSelectCallback(IGraphObjectManager *gom, IGraphNode *gNode);
	CoreExport bool SvCanSelect(IGraphObjectManager *gom, IGraphNode *gNode);

	// Adds an extension object into the pipeline. The methods (Display,
	// PreChanChangedNotify etc) of higher priority XTCObjects will becalled 
	// before those of lower priority XTCObjects
	/**	This method adds an extension object into the pipeline. 
	Implemented by the System.
	\param pObj Points to the extension object to add.
	\param priority The priority of the object. The methods (XTCObject::Display(),
	PreChanChangedNotify() etc) of higher priority XTCObjects will be called before
	those of lower priority XTCObjects.
	\param branchID The branch identifier of the object. */
	CoreExport void AddXTCObject(XTCObject *pObj, int priority = 0, int branchID = -1);

	/**	Returns the number of extension objects maintained by this Object. 
	Implemented by the System. */
	CoreExport int NumXTCObjects();

	/**	Returns a pointer to the specified extension object. 
	Implemented by the System.
	\param index The zero based index of the extension object to return. */
	CoreExport XTCObject *GetXTCObject(int index);

	/**	Removes the extension object as indicated by the index. 
	Implemented by the System.
	\param index The zero based index of the extension object to remove. */
	CoreExport void RemoveXTCObject(int index);

	/**	Sets the priority for the extension object whose index is passed. 
	Implemented by the System.
	\param index The zero based index of the extension object to remove.
	\param priority The new priority to assign. */
	CoreExport void SetXTCObjectPriority(int index,int priority);

	/**	Returns the integer priority number of the extension object whose index is
	passed. 
	Implemented by the System.
	\param index The zero based index of the extension object to check. */
	CoreExport int GetXTCObjectPriority(int index);

	/**	Sets the branch ID of the extension object whose index is passed. 
	Implemented by the System.
	\param index The zero based index of the extension object whose branch ID is set.
	\param branchID The branch identifier to set. */
	CoreExport void SetXTCObjectBranchID(int index,int branchID);

	/**	Returns the integer branch ID of the extension object whose index is
	passed. 
	Implemented by the System.
	\param index The zero based index of the extension object whose branch ID is to be returned. */
	CoreExport int GetXTCObjectBranchID(int index);

	// This method has to be called whenever the CompoundObject updates a branch 
	// (calling Eval on it). Object *from is the object returned from Eval 
	// (os.obj);branchID is an int, that specifies that branch. The extension 
	// channel will get a callback to RemoveXTCObjectOnMergeBranches and MergeXTCObject. 
	// By default it returns true to RemoveXTCObjectOnMergeBranches, which means,
	// that the existing XTCObjects with that branchID will be deleted. The method 
	// MergeXTCObject simply copies the XTCObjects from the incoming branch into the 
	// compound object.

	/**	This method has to be called whenever the CompoundObject updates a branch
	(calling Eval() on it). Object *from is the object returned
	from Eval(os.obj). The branchID is an integer that specifies that
	branch. The extension channel will get a callback to
	XTCObject::RemoveXTCObjectOnMergeBranches() and
	XTCObject::MergeXTCObject(). By default it returns true to
	RemoveXTCObjectOnMergeBranches which means that the existing XTCObjects
	with that branchID will be deleted. The method MergeXTCObject simply copies
	the XTCObjects from the incoming branch into the compound object. 
	Implemented by the System.
	\param from The object to merge additional channels from.
	\param branchID The branch identifier to set. */
	CoreExport void MergeAdditionalChannels(Object *from, int branchID);

	// This method has to be called on the CompoundObject, so it can delete the 
	// XTCObjects for the specific branch. The XTCObject will have again the final 
	// decision if the XTCObject gets really deleted or not in a callback to 
	// RemoveXTCObjectOnBranchDeleted(), which will return true, if the XTCOject 
	// should be removed.

	/**	This method has to be called on the CompoundObject so it can delete the
	XTCObjects for the specified branch. The XTCObject will again have the
	final decision if the XTCObject gets really deleted or not in a callback to
	XTCObject::RemoveXTCObjectOnBranchDeleted() which will return true
	if the XTCOject should be removed. 
	Implemented by the System.
	\param branchID Specifies which brach of the compound object the extension objects are
	deleted from.
	\param reorderChannels TRUE to reorder the channels, otherwise FALSE. */
	CoreExport void BranchDeleted(int branchID, bool reorderChannels);

	// This method copies all extension objects from the "from" objects into the 
	// current object. In case deleteOld is false, the objects will be appended. 
	// In case it is true, the old XTCObjects will be deleted.
	/**	This method copies all extension objects from the "from" object into the
	current object. In case deleteOld is false the objects will be appended. If
	it is true the old XTCObjects will be deleted. 
	Implemented by the System.
	\param from The source object which contains extension objects.
	\param deleteOld If true the original objects are deleted after the copy; if false they
	remain after the copy.
	\param bShallowCopy If true only a ShallowCopy() is performed; if false a complete copy of the
	objects is done. */
	CoreExport void CopyAdditionalChannels(Object *from, bool deleteOld = true, bool bShallowCopy = false);

	/**	Implemented by the System. 
	This method will delete all additional channels. */
	CoreExport void DeleteAllAdditionalChannels();


	/**	This method allows an object to choose whether or not it will display selection
	brackets in shaded viewports. The method will return FALSE if no selection
	brackets are displayed or TRUE if it does display selection brackets. */
	virtual BOOL UseSelectionBrackets() { return TRUE; }

	// returns TRUE for manipulator objcts and FALSE for all others
	virtual BOOL IsManipulator() { return FALSE; }

	CoreExport void* GetInterface(ULONG id);
	CoreExport BaseInterface* GetInterface(Interface_ID id);

	//! \brief Should reduce any derived display data to save memory, since the node wont be drawn until the user undhides it
	/*! This function should delete any derived data used to display the object such as gfx normals, direct mesh caches etc.  
	Tnis is called when the user hides the node	or sets it as bounding box
	*/		
	virtual void ReduceDisplayCaches() {  }

	/*! \brief This returns whether the Graphics Cache for this object needs to be rebuilt
	\par Parameters:
	GraphicsWindow *gw the active graphics window \n
	Material *ma the material aray assigned to the mesh \n
	int numMat the number of materials in the material array \n
	*/
	virtual bool NeedGWCacheRebuilt(GraphicsWindow *gw, Material *ma, int numMat) { return false; }

	/*! \brief This builds the graphics window cached mesh 
	\par Parameters:
	GraphicsWindow *gw the active graphics window \n
	Material *ma the material aray assigned to the mesh \n
	int numMat the number of materials in the material array \n
	BOOL threaded whether when building the cache it can use additional threads.  This is needed since the system may be creating many meshes at the same time\n
	*/
	virtual void BuildGWCache(GraphicsWindow *gw, Material *ma, int numMat, BOOL threaded) {  }



};


// This function should be used to count polygons in an object.
// It uses Object::PolygonCount() if it is supported, and converts to
// a TriObject and counts faces and vertices otherwise.
/** This global function (not part of class Object) may be used to count the number
of faces and vertices in an object. It uses Object::PolygonCount() if it
is supported, and converts to TriObject and counts faces and vertices
otherwise.
\param t time at which to compute the number of faces and vertices.
\param pObj to the object to check.
\param numFaces number of faces is returned here.
\param numVerts number of vertices is returned here. */
CoreExport void GetPolygonCount(TimeValue t, Object* pObj, int& numFaces, int& numVerts);

/** This function is to count trifaces of a mesh
 * It works similar to GetPolygonCount(), where GetPolygonCount will count EditablePoly object's Polys as 1 Poly,
 * instead of several trifaces. It also will not count splines unless their renderable property is true
 */
CoreExport void GetTriMeshFaceCount(TimeValue t, Object* pObj, int& numFaces, int& numVerts);

// mjm - begin - 07.17.00
class  CameraObject;

/** The multipass camera effect allows modification of viewpoints and view
directions or time for each pass of a multipass rendering. Algorithms such as
Depth of Field, Scene motion blur can be implemented using multipass
techniques.
A multipass camera effect is a plug-in to camera objects. It allows
the renderer to query the camera for the view params for each pass of the
rendering, and provides a dithered combining function to combine the bitmaps
produced by each pass into the final bitmap. It also allows time to be
manipulated for each rendering pass, providing effects such as motion blur.
\see Class ReferenceTarget, Class CameraObject
 */
class IMultiPassCameraEffect : public ReferenceTarget
{
public:
	// allows effect to declare it's compatibility with the current camera object
	/** Some cameras are not compatible with some render effects,
	this method allows cameras to list compatible effects in the UI and as such
	allows the effect to declare its compatibility with the current camera
	object.
	\param pCameraObject A pointer to a camera object.
	\return  TRUE if compatible, otherwise FALSE. */
	virtual bool IsCompatible(CameraObject *pCameraObject) = 0;

	// indicates that the renderer should display each pass as it is rendered (not used by viewport renderer)
	/** There is a UI option on multipass effects that indicates
	whether the renderer should display each pass as it is rendered. Note this
	is not used by viewport renderer, because of the hardware involvement. This
	method returns whether to display individual passes as they are computed.
	\param renderTime The rendertime at which to check the display passes.
	\return  TRUE if display is on, otherwise FALSE. */
	virtual bool DisplayPasses(TimeValue renderTime) = 0;

	// indicates the total number of passes to be rendered
	/** The multipass effect also has a variable number of passes.
	This method tells the renderer how many passes to render per final output
	frame and as such returns the total number of passes to be rendered
	\param renderTime The rendertime at which to check the display passes. */
	virtual int TotalPasses(TimeValue renderTime) = 0;

	// called for each render pass. the effect can alter the camera node, camera object, or override the render time
	/** This method will modify the camera, camera node, or time
	value to affect each pass. 
	This is the modify function called for each pass of the frame. The effect
	can alter the camera node, camera object, or override the render time in
	the course of this call. 3ds Max renderers take an optional parameter
	viewParams* that when not NULL overrides the normal rendering camera. When
	this is called the override render time will be set to the current frame
	time. If the value is changed, this will be the time value used for the
	pass. Note that at the time that apply is called, the renderer has not yet
	been called, hence it is possible, with care, to alter the scene in a
	general way, not just the camera \& time parameters. Apply should return
	NULL if the normal unmodified camera is to be used.
	\param pCameraNode A pointer to the node of the camera.
	\param pCameraObject A pointer to the camera object.
	\param passNum The number of the pass.
	\param overrideRenderTime The time if you wish to override the render time.
	\return  The viewparams returned by apply which are supplied to the
	renderer. */
	virtual ViewParams *Apply(INode *pCameraNode, CameraObject *pCameraObject, int passNum, TimeValue &overrideRenderTime) = 0;
	
	// allows the effect to blend its own passes (not used by the viewport renderer)
	/** This method will blend each pass (src) into the final
	accumulator (dest).
	After each pass is rendered, it needs to be combined into the final output
	bitmap. The current multipass effects use a dithered combiner, so that hard
	edges from the passes are more smoothly blended. There are many ways to do
	this, with varying quality, so this method allows different future
	implementations. Note that this is not used by the viewport renderer, as
	there's no way to tell the hardware to do this. Hardware is for fast \&
	edgy, software is for slow and smooth.
	\param pDest The destination bitmap.
	\param pSrc The source bitmap.
	\param passNum The number of the pass.
	\param renderTime The render time. */
	virtual void AccumulateBitmap(Bitmap *pDest, Bitmap *pSrc, int passNum, TimeValue renderTime) = 0;
	// convenience function, called after all passes have been rendered. can be ignored.
	/** This method is called after all passes have been
	rendered.
	After all passes have been rendered \& accumulated, this method will be
	called so that the effect can do any final cleanup. Currently unused, it
	can be ignored by multipass effects if they wish. */
	virtual void PostRenderFrame() = 0;

	// from class ReferenceMaker
	/** This method is implemented to receive and respond to messages
	broadcast by all the dependants in the entire system.
	\param changeInt This is the interval of time over which the message is active.
	\param hTarget This is the handle of the reference target the message was sent by. The
	reference maker uses this handle to know specifically which reference
	target sent the message.
	\param partID This contains information specific to the message passed in. Some messages
	don't use the partID at all. See \ref Reference_Messages
	for more information about the meaning of the partID for some common messages.
	\param message The msg parameters passed into this method is the specific message which
	needs to be handled. See \ref Reference_Messages.
	\return  The return value from this method is of type RefResult. This is
	usually REF_SUCCEED indicating the message was processed. Sometimes,
	the return value may be REF_STOP. This return value is used to stop
	the message from being propagated to the dependents of the item. */
	virtual RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) { return REF_SUCCEED; }

	// from class Animatable
	/** This method returns the super class ID of the creator of the
	clip object. */
	virtual SClass_ID SuperClassID() { return MPASS_CAM_EFFECT_CLASS_ID; }
	virtual void BeginEditParams(IObjParam *ip, ULONG flags, Animatable *prev=NULL) {}
	virtual void EndEditParams(IObjParam *ip, ULONG flags, Animatable *next=NULL) {}
};
// mjm - end


// ------------
// CameraObject  
// ------------
/// \defgroup Clipping_Distances Clipping Distances
//@{
#define CAM_HITHER_CLIP         1	//!< The hither distance
#define CAM_YON_CLIP            2	//!< The yon distance.
//@}

/// \defgroup Environment_Range_Distances Environment Range Distances
//@{
#define ENV_NEAR_RANGE          0	//!< The near distance
#define ENV_FAR_RANGE           1	//!< The far distance
//@}

/** Note: The camera looks down the negative Z axis, with X to the right and Y up.
 * \see Class GenCamera
*/
struct CameraState: public MaxHeapOperators {
	inline CameraState() {cbStruct = sizeof(CameraState);}
	DWORD cbStruct;
	/** Flag to indicate if the camera uses orthographic projection (TRUE) or perspective (FALSE). */
	BOOL isOrtho;	// true if cam is ortho, false for persp
	/** The camera field-of-view in radians. */
	float fov;      // field-of-view for persp cams, width for ortho cams
	/** Target distance for free cameras. */
	float tdist;    // target distance for free cameras
	/** Horizon line display state. */
	BOOL horzLine;  // horizon line display state
	/** Flag to indicate if camera has manual clipping enabled. */
	int manualClip;
	/** Hither clipping plane distance. */
	float hither;
	/** Yon clipping plane distance. */
	float yon;
	/** Near camera range radius. */
	float nearRange;
	/** Far camera range radius. */
	float farRange;
};

/** This is a base class from which camera plug-ins may be derived. Methods of this
class are used to get and set properties of the camera. All methods of this
class are virtual.  To ensure that the camera has a valid targDist during
network rendering, be sure to call UpdateTargDistance( TimeValue t, INode* inode );
This call should be made PRIOR to cameraObj->EvalWorldState().
\see Class Object
*/
class CameraObject : public Object
{
public:
	SClass_ID SuperClassID() { return CAMERA_CLASS_ID; }
	int IsRenderable() { return(0);}
	virtual void InitNodeName(MSTR& s) { s = _M("Camera"); }
	virtual int UsesWireColor() { return FALSE; } // TRUE if the object color is used for display

	/** This method is called to update the CameraState and validity interval at
	the specified time.
	\param time Specifies the time to evaluate the camera.
	\param valid The plug-in computes the validity interval of the camera at the specified
	time and stores the result here.
	\param cs The camera state to update. See Structure CameraState.
	\note The view vector and 'up' vector for the camera are stored with the matrix
	transform for the node. Cameras can be multiple-instanced so it must work
	this way. To get at this matrix use the following method from
	Class INode:
	\code
	virtual Matrix3 GetObjTMAfterWSM(TimeValue time, Interval* valid=NULL)=0;
	\endcode
	The scaling of this matrix may be removed by normalizing each of the rows.
	\return  REF_SUCCEED if the camera state was updated successfully;
	otherwise REF_FAIL. */
	virtual RefResult EvalCameraState(TimeValue time, Interval& valid, CameraState* cs)=0;

	/** Sets whether the camera is on ortho mode or not.
	\param b Pass TRUE for ortho and FALSE for not ortho. */
	virtual void SetOrtho(BOOL b)=0;
	/** Returns TRUE if the camera is in ortho mode and FALSE if it is not. */
	virtual BOOL IsOrtho()=0;
	/** Sets the field-of-view of the camera at the specified time.
	\param t The time at which to set the field-of-view.
	\param f The value to set in radians. */
	virtual void SetFOV(TimeValue t, float f)=0; 
	/** Returns the field-of-view setting of the camera at the specified time and
	adjusts the validity interval of the camera at this time to reflect the
	field-of-view parameter.
	\param t The time to retrieve the field-of-view setting.
	\param valid  The validity interval to set.
	\return  The field-of-view of the camera in radians. */
	virtual float GetFOV(TimeValue t, Interval& valid = Interval(0,0))=0;
	/** Sets the target distance setting (for free cameras) at the specified time.
	\param t The time at which to set the target distance.
	\param f The value to set. */
	virtual void SetTDist(TimeValue t, float f)=0; 
	/** Returns the target distance setting of the camera at the specified time and
	adjusts the validity interval of the camera to reflect the target distance
	parameter.
	\param t The time to retrieve the target distance setting.
	\param valid  This validity interval is intersected with the validity interval of the
	target distance parameter.
	\return  The target distance of the camera. */
	virtual float GetTDist(TimeValue t, Interval& valid = Interval(0,0))=0;
	/** Returns the manual clip flag. This indicates the camera will perform
	clipping at its hither and yon distances.
	\return  Nonzero if manual clipping is enabled; otherwise 0. */
	virtual int GetManualClip()=0;
	/** Sets the manual clip flag. This indicates the camera will perform clipping
	at its hither and yon distances.
	\param onOff The state of the manual clipping flag to set. Nonzero indicates clipping
	will be performed. */
	virtual void SetManualClip(int onOff)=0;
	/** Retrieves the clipping distance of the specified plane at the specified
	time and modifies the validity interval to reflect the setting of the
	clipping distance parameter.
	\param t The time to retrieve the clipping distance.
	\param which Indicates which distance to return. One of the values in \ref Clipping_Distances
	\param valid The validity interval that this method will update to reflect the clipping
	distance interval.
	\return  The clipping distance. */
	virtual float GetClipDist(TimeValue t, int which, Interval &valid=Interval(0,0))=0;
	/** Sets the clipping distance of the specified plane at the specified time.
	\param t The time to set the clipping distance.
	\param which Indicates which distance to set. One of the values in \ref Clipping_Distances
	\param val The distance to set. */
	virtual void SetClipDist(TimeValue t, int which, float val)=0;
	/** Sets the environment range distance at the specified time.
	\param time The time to set the environment range.
	\param which Indicates which distance to set. One of the values in \ref Environment_Range_Distances
	\param f The distance to set. */
	virtual void SetEnvRange(TimeValue time, int which, float f)=0; 
	/** Retrieves the environment range distance at the specified time and
	intersects the specified validity interval with the interval of the
	environment range parameter.
	\param t The time to retrieve the environment range.
	\param which Indicate which distance to set. One of the values in \ref Environment_Range_Distances
	\param valid  The validity interval that this method will update to reflect the
	environment range setting.
	\return  The environment range distance at the specified time. */
	virtual float GetEnvRange(TimeValue t, int which, Interval& valid = Interval(0,0))=0;
	/** Sets the environment range display flag. This indicates if the camera will
	display its range settings.
	\param b The flag state to set.
	\param notify If notify is TRUE, dependents of this message are sent the \ref REFMSG_CHANGE message using 
	NotifyDependents(FOREVER, PART_OBJ, REFMSG_CHANGE);
	Otherwise no notification is sent. */
	virtual void SetEnvDisplay(BOOL b, int notify=TRUE)=0;
	/** Retrieves the environment range display setting.
	\return  TRUE if ranges are displayed; otherwise FALSE. */
	virtual BOOL GetEnvDisplay(void)=0;
	/**	This method is called on all cameras when the render aperture width has
	changed.
	\param t The time of the change. */
	virtual void RenderApertureChanged(TimeValue t)=0;
	/**	This method is called on all target cameras when the target distance has
	changed. For instance, a distance shown in the user interface may be
	updated in this method.
	\param t The time of the change.
	\param inode The camera node. */
	virtual void UpdateTargDistance(TimeValue t, INode* inode) { }
	// mjm - begin - 07.17.00
	/**	Enables or disables the multi-pass effect.
	\param t The time at which to enable the effect.
	\param enabled TRUE for enabled; FALSE for disabled. */
	virtual void SetMultiPassEffectEnabled(TimeValue t, BOOL enabled) { }
	/**	Returns the enabled or disabled state of the multi-pass effect setting for
	the camera.
	\param t The time at which to get the setting.
	\param valid The validity interfal for the setting.
	\return  TRUE for enabled; FALSE for disabled. */
	virtual BOOL GetMultiPassEffectEnabled(TimeValue t, Interval& valid) { return FALSE; }
	virtual void SetMPEffect_REffectPerPass(BOOL enabled) { }
	virtual BOOL GetMPEffect_REffectPerPass() { return FALSE; }
	/**		The IMultiPassCameraEffect should be checked to see if compatible
	with the camera	before being assigned.
	\param pIMultiPassCameraEffect The IMultiPassCameraEffect to assign. */
	virtual void SetIMultiPassCameraEffect(IMultiPassCameraEffect *pIMultiPassCameraEffect) { }
	/**	Returns a pointer to the current multi-pass camera effect. See
	Class IMultiPassCameraEffect. */
	virtual IMultiPassCameraEffect *GetIMultiPassCameraEffect() { return NULL; }
};

/// \defgroup Light_object_s_attenuation_range_distance_ Light object's attenuation range distance 
//@{
#define LIGHT_ATTEN_START       0	//!< The start range radius
#define LIGHT_ATTEN_END         1	//!< The end range radius
//@}

/** This structure describes the properties of a light.
 * \see Class GenLight, Class LightObject, Class Color, Class Matrix3
 */
struct LightState: public MaxHeapOperators {
	/** One of the following values from enum LightType: \n
	OMNI_LGT - Omnidirectional \n
	SPOT_LGT - Spot (cone) \n
	DIRECT_LGT - Directional (parallel) \n
	AMBIENT_LGT - Global
	*/
	LightType type;
	/** The transformation matrix of the light.  */
	Matrix3 tm;
	/** The color of the light (its intensity).  */
	Color color;
	/** The multiplier applied to the color.  */
	float 	intens;  // multiplier value
	/** The hotspot size in degrees.  */
	float 	hotsize; 
	/** The hotspot falloff size in degrees.  */
	float 	fallsize;
	/** Nonzero if near attenuation is used; otherwise zero. */
	int		useNearAtten;
	/** The near attenuation start value. */
	float	nearAttenStart;
	/** The near attenuation end value. */
	float	nearAttenEnd;
	/** Nonzero if (far) attenuation is used; otherwise zero.  */
	int   	useAtten;
	/** The (far) start attenuation value.  */
	float 	attenStart;
	/** The (far) end attenuation value.  */
	float 	attenEnd;
	/** One of the following values:  \n
	RECT_LIGHT, CIRCLE_LIGHT */
	int   	shape;
	/** The aspect ratio of the light.  */
	float 	aspect;
	/** TRUE if the light supports overshoot; otherwise FALSE.  */
	BOOL   	overshoot;
	/** TRUE if shadows are on; otherwise FALSE.  */
	BOOL   	shadow;
	/** TRUE if the light is on; otherwise FALSE.  */
	BOOL 	on;      // light is on
	/** TRUE if affect diffuse is on; otherwise FALSE.  */
	BOOL	affectDiffuse;
	/** TRUE if affect specular is on; otherwise FALSE.  */
	BOOL	affectSpecular;
	BOOL 	ambientOnly;  // affect only ambient component
	DWORD   extra;
};

class LightDesc;
class RendContext;


// This is a callback class that can be given to a ObjLightDesc
// to have a ray traced through the light volume.
/** This is a callback class that can be given to a ObjLightDesc to have a
ray traced through the light volume. A plug-in derives a class from this one
and passes it as a callback in the ObjLightDesc method
TraverseVolume(). This allows a developer to integrate the illumination
of a segment due to a light. t0 and t1 define the segment in
terms of the given ray. 
This is what the 3ds Max spotlights do: First they break the segment up into
three main pieces. The first piece is from the camera to where the ray
intersects the lights cone volume. The callback Step() is called once
over this segment (t0 and t1 will have this first piece). The
illumination is constant over this entire segment from t0 to t1.
It is a constant black since the light is not illuminating it at all. 
The next segment is inside the cone. This segment will be broken up into small
pieces. First as it's stepping along it will be between the falloff and the
hotspot. The illumination over this segment goes from black to brighter and
brighter as it moves towards to hotspot. Across the entire hotspot region the
illumination may be constant. Then as it steps from the hotspot to the falloff
the illumination will go back down to black. 
Inside the hotspot region, if shadows are turned on, the light may be brighter
or darker depending on if it's inside a shadow or on the edge of a shadow. The
light handles all of this. It takes care of the shadows, attenuation, etc. 
Now consider how the 3ds Max atmospheric effects such as the volume lights use
this information. For each light that they are bound to, they call the method
TraverseVolume() on the light. The volume light atmospheric effect
passes this callback to the TraverseVolume() method. The light then
calls this Step() method of the callback for each partial segment of the
ray. Given the illumination on the segment (illum) it computes the fog
density over that piece. The density may be constant if noise is not turned on,
or it may vary if noise is turned on. Using the fog density and the
illumination it computes the light reflected off the atmosphere for the
segment.  
\see  Class ObjLightDesc
*/
class LightRayTraversal: public MaxHeapOperators {
public:
	/** Destructor */
	virtual ~LightRayTraversal() {;}
	// This is called for every step (return FALSE to halt the integration).
	// t0 and t1 define the segment in terms of the given ray.
	// illum is the light intensity over the entire segment. It can be
	// assumed that the light intensty is constant for the segment.
	/** This method is called for every step defined by t0 and t1.
	The illumination over this segment is passed in illum.
	\param t0 The start of the segment. This is a distance along the ray. The ray is made
	up of a point p and a unit length direction vector dir. The
	point defined by t0 is thus ray.p+t0*ray.dir.
	\param t1 The end of the segment. This is a distance along the ray. The ray is made
	up of a point p and a unit length direction vector dir. The
	point defined by t1 is thus ray.p+t1*ray.dir.
	\param illum The light intensity over the entire segment. It can be assumed that the
	light intensity is constant for the segment.
	\param distAtten This parameter may be used so that volume effects can use the distance
	attenuation value as an input variable to their effects. For instance, the
	volume light uses this to change the fog color based on the distance from
	the light.
	\return  TRUE to continue; FALSE to halt the integration (stop the
	traversal). */
	virtual BOOL Step(float t0, float t1, Color illum, float distAtten)=0;
};

// Flags passed to TraverseVolume
/// \defgroup TraverseVolume_flags TraverseVolume flags
//@{
/** This is a simple filtering where the system samples a point in the shadow map and then some 
* of the neighboring points. This corresponds to 'Medium' in the Volume Light user interface 
*(a value of 0 for flags is 'Low' -- just sampling the shadow map with no filtering at all).
*/
#define TRAVERSE_LOWFILTSHADOWS (1<<0)
/** This is a higher resolution sampling. This corresponds to 'High' in the Volume Light user interface.*/
#define TRAVERSE_HIFILTSHADOWS  (1<<1)
/** This produces the highest quality. This corresponds to 'Use Light Sample Range' in the Volume Light 
user interface. This is like a box filter, but also takes into consideration the position of the point 
within the pixel to do additional weighting.*/
#define TRAVERSE_USESAMPLESIZE	(1<<2)
//@}

////////////////////////////////////////////////////////////////////////
// The following classes, IlluminateComponents & IlluminationComponents,
// have been added in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.


/** Provides the components of the Illuminate function to Shaders, Materials and 
Render Elements. The interface to lights to return the illumination as a set of
components. This interface separates the illumination into various components
as well as the final result.
*/
class IlluminateComponents : public BaseInterfaceServer {
public:
	/** Light Vector*/
	Point3  L;				
	/** N dot L, N and L are normalized*/
	float	NL;				

	// these are the attenuations due to the light
	/** The contrast applied to N.L... this is equivalent to the diffCoef returned by
	standard illuminate*/
	float	geometricAtten; // final constrast applied to N.L
	/** attenuation fraction due to cone(s) or rect falloff*/
	float	shapeAtten;		// due to cone(s) falloff
	/** Attenuation due to distance falloff*/
	float	distanceAtten;	// attenuation due to distance, falloff

	// this is the composite attenuation due to all shadowing objects
	// transparent bojects may supply some shadowAtten as well as filterAtten
	/** The composite attenuation due to all shadowing objects transparent objects may
	supply some shadowAtten as well as filterAtten.
	0 for all shadow, 1 for all light.
	light control over basic shading components*/
	float	shadowAtten;	 // 0 == all shadow, 1 == all light

	// these allow smooth control over how the light 
	// affects these basic shading components
	/** light control over ambitient shading components.
	0 = no ambient, 1 = all ambient*/
	float   ambientAtten;	// 0 == no ambient, 1 = all ambient
	/** light control over diffuse shading components.
	0 = no diffuse, 1 = all diffuse*/
	float   diffuseAtten;	// 0 == no diffuse, 1 = all diffuse
	/** light control over specular shading components.
	0 = no specular, 1 = all specular*/
	float   specularAtten;	// 0 == no specular, 1 = all specular

	// The complete amount of light falling on the point sc.P() orientied
	// in the direction sc.N() is the filteredColor. if( rawColor!=filteredColor)
	// then it's filtered, else unfiltered

	// light color modulated by map value, 
	// unattenuated, w/ raw light if no map
	/** Raw light color or mixed with light map value, unattenuated by shape, distance,
	or	shadow.*/
	Color	rawColor;		

	// light color modulated by map value, 
	// then filtered by a transparent object,
	// raw color * filterAtten, otherwise unattenuated, 
	/** Light color modulated by map value, then filtered by a transparent object, raw
	color * filterAtten, otherwise unattenuated.*/
	Color	filteredColor;

	// color due to user shadow color, modulated by a possible map,
	// attenuated by 1-shadowAtten
	/** Shadow color, attenuated by shape, distance and shadow fraction*/
	Color	shadowColor;	

	// these are the combined light colors modulated by the ambientAtten'uators
	// they can be used by shaders to compute diffuse & specular shading
	// complete component color is e.g. lightDiffuseColor+shadowColor

	// NB: the geometric atten is to be applied by the shader, not the light
	// e.g. lightAmbientColor = 
	//			ambientAtten * (shapeAtten*distAtten*shadowAtten) 
	//				* filteredColor;
	/** Ambient color due to light, attenuated, without shadow color*/
	Color	lightAmbientColor;	// ambient color due to light, attenuated, w/o shadow color
	/** Diffuse color due to light, attenuated, without shadow color*/
	Color	lightDiffuseColor;	// diffuse color due to light, attenuated, w/o shadow color
	/** Specular color due to light, attenuated, without shadow color */
	Color	lightSpecularColor;	// specular color due to light, attenuated, w/o shadow color

	// these are equivalent to 4.0 final illumination color, with & without shadows
	// finalColor = shadowColor + (shapeAtten * distAtten * shadowAtten) 
	//									*  filteredColor;
	/** Equivalent to 4.0 final illumination color, with and without shadows */
	Color	finalColor;		

	// Like final color but no shadow attenuation applied and no shadowColor
	// finalColorNS = (shapeAtten * distAtten) * filteredColor;
	/** Equivalent to finalColor, but with no shadow attenuation applied and no
	shadow */
	Color	finalColorNS;	

	// user extensible component outputs, name matched
	/** The number of user illumination channels, returned by nUserIllumChannels()*/
	int	nUserIllumOut;		// one set of names for all illum params instances
	/** Pointer to shared name array, never destroyed */
	MCHAR** userIllumNames;  // we just keep ptr to shared name array, never destroyed
	/** The user illumination color array, created and deleted with the class */
	Color* userIllumOut;	// the user illum color array, new'd and deleted with the class

	IlluminateComponents(): nUserIllumOut(0),userIllumOut(NULL),userIllumNames(NULL) {}

	CoreExport ~IlluminateComponents(); 

	// Returns number of user illum channels for this material
	/**   \return Number of user illumination channels for
	material
	  */
	int nUserIllumChannels(){ return nUserIllumOut; }

	// returns null if no name array specified
	/** Returns the name of the index in channels name array
	\param n The element in the userIllumNames array
	\return  The name of the user channel, NULL if no name specified */
	MCHAR* GetUserIllumName( int n ) { 
		DbgAssert( n < nUserIllumOut );
		if( userIllumNames )
			return userIllumNames[n];
		return NULL;
	}

	// render elements, mtls & shaders can use this to find the index associated with a name
	// returns -1 if it can't find the name
	/** Returns the index associated with a name
	\param name The channel name in the userIllumNames array
	\return  The index of the user channel, -1 if name no found
	  */
	CoreExport int FindUserIllumName( MCHAR* name );

	// knowing the index, these set/get the user illum output color
	/** Set the output illumination color to indexed channel
	\param n The channel index in the userIllumOut array
	\param out The illumination color to set */
	void SetUserIllumOutput( int n, Color& out ){
		DbgAssert( n < nUserIllumOut );
		userIllumOut[n] = out;
	}

	/** Get the output illumination color of indexed channel
	\param n The channel index in the userIllumOut array
	\return  The illumination color to get 
	  */
	Color GetUserIllumOutput( int n ){
		DbgAssert( n < nUserIllumOut );
		return userIllumOut[n];
	}

	/** It sets to black all the output colors */
	void Reset(){
		NL = geometricAtten = shapeAtten = distanceAtten = shadowAtten
			= ambientAtten = diffuseAtten = specularAtten = 0.0f;
		rawColor.Black();
		filteredColor.Black();
		lightAmbientColor.Black();
		lightDiffuseColor.Black();
		lightSpecularColor.Black();
		shadowColor.Black();
		finalColor.Black();
		finalColorNS.Black();
		L = Point3( 0,0,0 );
		if(nUserIllumOut>0 && userIllumOut){
			for(int i=0; i<nUserIllumOut; ++i )
				userIllumOut[i].Black();
		}
	}

}; // end, IlluminateComponents

// must be greater than I_USERINTERFACE in AnimatableInterfaceIDs.h
#define IID_IIlluminationComponents	Interface_ID(0xdae00001, 0x0)

/** this is the interface to use illumination by components
 this may be supported by a light object
 returned by lightObjDesc::GetInterface( IID_IIlluminationComponents );*/
class IIlluminationComponents : public BaseInterface {
public:
	virtual BOOL Illuminate(ShadeContext& sc, Point3& normal, IlluminateComponents& illumComp )=0;

};

// End of IlluminateComponents & IlluminationComponent 3ds max 4.2 Extension



// A light must be able to create one of these to give to the renderer.
// The Illuminate() method (inherited from LightDesc) is called by the renderer
// to illuminate a surface point.
/** A light must be able to create one of these objects to give to the renderer. As
	the renderer is getting ready to render, it will ask for one of these from each
	of the lights. The Illuminate() method (inherited from LightDesc)
	is called by the renderer to illuminate a surface point. 
	There is an ObjLightDesc for every instance of the light. The renderer
	will ask each light object to produce one of these ObjLightDescs. It
	will then set this data up in the node's render data (See
	Class RenderData). For example in 3ds
	Max's volume light implementation of Atmospheric::Update() it goes
	through its node references to lights and calls GetRenderData(). It then
	casts this as an ObjLightDesc. This is how a atmosphere effect can get
	access to these descriptors at render time.
	\see  Class LightDesc, Class LightRayTraversal, Class INode,  Class Point3
	\see Class Matrix3, Class Ray,  Class RendContext,  Class ShadeContext, Class RenderInstance
	*/
class ObjLightDesc : public LightDesc {
public:         
	// This data will be set up by the default implementation of Update()
	LightState ls;			//!< The light state structure. See Structure LightState.
	INode *inode;			//!< The INode pointer of the instance of the light that created the current object.
	BOOL uniformScale;		//!< Indicates if the light's scale is uniform or not. TRUE if uniform, FALSE if not. This saves some steps in the renderer if the scale is uniform.
	Point3 lightPos;		//!< The position of the light in camera space.
	Matrix3 lightToWorld;	//!< This is effectively the light node's object TM. This matrix will transform	points from light space to world space.
	Matrix3 worldToLight;	//!< This matrix will transform points from world space to light space. This is the	inverse of lightToWorld.
	Matrix3 lightToCam;		//!< This matrix will transform points from light space to camera space. This is updated in UpdateViewDepParams()
	/** This matrix will transform points from camera space to light space. This is
		updated in UpdateViewDepParams(). For example, the renderer would have
		points in camera space. To figure out if a point was in shadow it would
		transform the point from camera space to light space using this matrix. It
		could then look in the shadow buffer to see if the point was in shadow.
		*/
	Matrix3 camToLight;   
	/** This is set by the renderer. It is used in
		RenderInstance::CastsShadowsFrom(). This is a number used by the
		renderer to identify the lights so it can quickly determine if a given light
		casts shadows from a given object. It is for use by the renderer.  
		*/
	int renderNumber;

	/** Constructor. The inode data member is initialized to
	n. */
	CoreExport ObjLightDesc(INode *n);
	/** Destructor. */
	CoreExport virtual ~ObjLightDesc();

	/** Retrieves the light's exclusion list.
	\return  See Class NameTab. */
	virtual ExclList* GetExclList() { return NULL; }  

	// update light state that depends on position of objects&lights in world.
	/** This method is called once per render to update the light state for things
	that depend on the position of objects and lights in world space. A plug-in
	light could update any data it would need to here. The default
	implementation is shown below.
	\param t The time of the render.
	\param rc See Class RendContext.
	\param rgc This pointer may be used to retireve information about the global rendering enviornment.
	\param shadows TRUE if shadows are turned on (in the render parameters, not the light
	parameters); otherwise FALSE.
	\param shadowGeomChanged This tells the Update procedure that the geometry of the objects that are
	shadowed by the light has changed (TRUE) or not (FALSE). If it is a shadow
	buffer, shadowGeomChanged == TRUE means it has to re-render the
	shadow buffer, shadowGeomChanged == FALSE means it can use the
	shadow buffer from the previous frame.
	\return  The value return should normally be 1. A returned value of 0 means
	an error has occured (such as out of memory) and the render will be halted.
	\par Default Implementation:
	\code
	int ObjLightDesc::Update(TimeValue t, const RendContext& rc, RenderGlobalContext *rgc, BOOL shadows, BOOL shadowGeomChanged)
	{
		if (inode) {
			Interval valid;
			ObjectState os = inode->EvalWorldState(t);
			assert(os.obj->SuperClassID()==LIGHT_CLASS_ID);
			LightObject* lob = (LightObject	*)os.obj;
			lob->EvalLightState(t, valid, &ls);
			lightToWorld = inode->GetObjTMAfterWSM(t);
			worldToLight = Inverse(lightToWorld);
			uniformScale = IsUniformScale(lightToWorld);
			affectDiffuse = ls.affectDiffuse;
			affectSpecular = ls.affectSpecular;
			ambientOnly = ls.ambientOnly;
		}
		else {
			uniformScale = TRUE;
			lightToWorld.IdentityMatrix();
			worldToLight.IdentityMatrix();
		}
		return 1;
	}
	\endcode */
	CoreExport virtual int Update(TimeValue t, const RendContext &rc, RenderGlobalContext *rgc, BOOL shadows, BOOL shadowGeomChanged);

	// update light state that depends on global light level.
	/** This method is called to update the light state that depends on the global	light level.
	\param globLightLevel The global light level. */
	CoreExport virtual void UpdateGlobalLightLevel(Color globLightLevel) {}

	// update light state that depends on view matrix.
	/** This method is called to update the light state that depends on the view
	matrix. This is used to cache certain computed quantities that are
	dependent on where you are looking from. In a given scene at a given time,
	the system may render from several viewpoints. This is because of things
	like reflection maps and mirrors that need to get rendered. This method is
	called for each of these different viewpoints.
	\param worldToCam The world space to camera space transformation matrix. */
	CoreExport virtual int UpdateViewDepParams(const Matrix3& worldToCam);

	// default implementation 
	/** This method is from LightDesc. Here it provides a
	default implementation returning the lightPos data member. */
	CoreExport virtual Point3 LightPosition() { return lightPos; }

	// This function traverses a ray through the light volume.
	// 'ray' defines the parameter line that will be traversed.
	// 'minStep' is the smallest step size that caller requires, Note that
	// the callback may be called in smaller steps if they light needs to
	// take smaller steps to avoid under sampling the volume.
	// 'tStop' is the point at which the traversal will stop (ray.p+tStop*ray.dir).
	// Note that the traversal can terminate earlier if the callback returns FALSE.
	// 'proc' is the callback object.
	//
	// attenStart/End specify a percent of the light attenuation distances
	// that should be used for lighting durring the traversal.
	//
	// The shade context passed in should only be used for state (like are
	// shadows globaly disabled). The position, normal, etc. serve no purpose.
	/** This function traverses a ray through the light volume. This method is
	implemented by plug-in lights.
	\par Consider how the 3ds Max atmospheric effects like the volume lights use
	this information. For each light the atmospheric effect is bound to, it
	calls the this method (TraverseVolume()) on the light. The volume
	light atmospheric effect passes a callback to this method (proc).
	The light then calls the Step() method of the callback for each
	partial segment of the ray. Given the illumination on the segment it
	computes the fog density over that segment. The density may be constant if
	noise is not turned on, or it may change if noise is turned on. Using the
	fog density and the illumination it computes the light reflected off the
	atmosphere for the segment.
	\param sc This is the ShadeContext passed into the Shade() method of
	the Atmospheric effect. The shade context passed in should only be used for
	state (like are shadows globally disabled). The position, normal, etc.
	serve no purpose.
	\param ray Defines the world space ray that will be traversed.
	\param samples The number of samples to sample along the ray. A reasonable range is from
	25-100. This is more or less the suggested number of times the
	proc-\>Step() callback will be called. It is not precisely however
	because the system may take greater or fewer steps than specified as it
	needs to.
	\param tStop This is the end of the ray. This is the point at which the traversal
	will stop (ray.p+tStop*ray.dir). Note that the traversal can
	terminate earlier if the callback returns FALSE.
	\param attenStart Specifies a percent of the light attenuation distances that should be used
	for lighting during the traversal. This is used so a light can have an
	attenuation set to a certain percent, and then have the volume light be
	attenuated at a different point.
	\param attenEnd This specifies the ending percent of the light attenuation distances that
	should be used for lighting during the traversal.
	\param flags There are three ways the shadow maps can be sampled. If none of these flags
	are set, the shadow map is sampled directly (this is the fastest). One of
	the following values: 
	TRAVERSE_LOWFILTSHADOWS\n
	This is a simple filtering where the system samples a point in the shadow
	map and then some of the neighboring points. This corresponds to 'Medium'
	in the Volume Light user interface (a value of 0 for flags is 'Low' -- just
	sampling the shadow map with no filtering at all). 
	TRAVERSE_HIFILTSHADOWS\n
	This is a higher resolution sampling. This corresponds to 'High' in the
	Volume Light user interface. 
	TRAVERSE_USESAMPLESIZE\n
	This produces the highest quality. This corresponds to 'Use Light Sample
	Range' in the Volume Light user interface. This is like a box filter, but
	also takes into consideration the position of the point within the pixel to
	do additional weighting.\n
	\param proc A developer derives a class from LightRayTraversal and implements
	the Step() method. A pointer to it is passed here as the callback
	object. */
	virtual void TraverseVolume(
		ShadeContext& sc,       
		const Ray &ray, int samples, float tStop,
		float attenStart, float attenEnd,
		DWORD flags,
		LightRayTraversal *proc) {}
};


/** \defgroup Shadow_types Shadow types used by the light. 
 * Values returned from GetShadowMethod()
 */
//@{
#define LIGHTSHADOW_NONE                0
#define LIGHTSHADOW_MAPPED              1
#define LIGHTSHADOW_RAYTRACED   2
//@}


/** This is the base class from which plug-in lights may be derived. 
	\see  Class Object, Class ObjLightDesc, Class Interval, Class Texmap.*/
class LightObject : public Object {
public:
	SClass_ID SuperClassID() { return LIGHT_CLASS_ID; }
	int IsRenderable() { return(0);}
	virtual void InitNodeName(MSTR& s) { s = _M("Light"); }

	// Used to query a LightObject interface from Animatable.
	CoreExport virtual void* GetInterface(ULONG id);
	CoreExport virtual BaseInterface* GetInterface(Interface_ID id);

	// Methods specific to Lights:
	/** This method is called to update the passed LightState and validity
	interval of the light.
	\param time The time to evaluate the light state.
	\param valid The validity interval of the light about the specified time. This interval
	should be updated to reflect the validity interval of the light.
	\param ls A pointer to the LightState structure which describes the properties
	of the light. This function updates the data in the structure to reflect
	the properties of the light at the specified time. See
	Structure LightState.
	\return  REF_SUCCEED if the LightState was updated; otherwise
	REF_FAIL. */
	virtual RefResult EvalLightState(TimeValue time, Interval& valid, LightState *ls)=0;
	/** When the renderer goes to render the scene it asks all of the lights to
	create an ObjectLighDesc object. This is the method that is called
	to return this object.
	\param n The node pointer of the light.
	\param forceShadowBuffer Forces the creation of a shadow buffer.
	\return  An instance of ObjectLightDesc. See
	Class ObjectLightDesc. */
	virtual ObjLightDesc *CreateLightDesc(INode *n, BOOL forceShadowBuffer=FALSE) {return NULL;}
	//JH 06/03/03 new api to pass globalRenderContext
	virtual ObjLightDesc *CreateLightDesc(RenderGlobalContext *rgc, INode *inode, BOOL forceShadowBuf=FALSE ){return NULL;}
	/**	Sets if the light is on or off.
	\param onOff TRUE for on; FALSE for off. */
	virtual void SetUseLight(int onOff)=0;
	/**	Returns TRUE if the light is on; otherwise FALSE. */
	virtual BOOL GetUseLight(void)=0;
	/** Sets the hotspot to the specified angle at the specified time.
	\param time The time to set the hotspot angle.
	\param f The angle to set in degrees. */
	virtual void SetHotspot(TimeValue time, float f)=0; 
	/** Retrieves the hotspot angle.
	\param t The time to retrieve the angle.
	\param valid  The validity interval that this method will update to reflect the hotspot setting.
	\return  The hotspot angle (in degrees). */
	virtual float GetHotspot(TimeValue t, Interval& valid = Interval(0,0))=0;
	/** Sets the falloff setting of the light.
	\param time The time to set the falloff.
	\param f The falloff angle in degrees. */
	virtual void SetFallsize(TimeValue time, float f)=0; 
	/** Returns the falloff angle of the light in radians.
	\param t The time to retrieve the falloff angle.
	\param valid The validity interval that this method will update to reflect the falloff
	setting.
	\return  The falloff angle of the light in degrees. */
	virtual float GetFallsize(TimeValue t, Interval& valid = Interval(0,0))=0;
	/** Sets the specified attenuation range distance at the time passed.
	\param time The time to set the attenuation distance.
	\param which Indicates which distance to set. One of the following values: 
	\ref LIGHT_ATTEN_START - The start range radius.\n
	\ref LIGHT_ATTEN_END - The end range radius.\n
	\param f The distance to set. */
	virtual void SetAtten(TimeValue time, int which, float f)=0; 
	/** Returns the specified attenuation range distance at the time passed.
	\param t The time to retrieve the attenuation distance.
	\param which Indicates which distance to retrieve. One of the following values: 
	\ref LIGHT_ATTEN_START - The start range radius.\n
	\ref LIGHT_ATTEN_END - The end range radius.\n
	\param valid  The validity interval that this method will update to reflect the attenuation setting.
	\return  The specified attenuation range distance. */
	virtual float GetAtten(TimeValue t, int which, Interval& valid = Interval(0,0))=0;
	/** Sets the light's target distance.
	\param time The time to set the distance.
	\param f The distance to set. */
	virtual void SetTDist(TimeValue time, float f)=0; 
	/** Retrieves the light's target distance.
	\param t The time to retrieve the distance.
	\param valid  The validity interval that this method will update to reflect the target distance setting.
	\return  The light's target distance. */
	virtual float GetTDist(TimeValue t, Interval& valid = Interval(0,0))=0;
	/** Sets the light's cone display flag. This controls if the cone is depicted
	graphically in the viewports.
	\param s Indicates if the cone display should be on or off. If nonzero, the cone
	should be displayed; otherwise it should be turned off.
	\param notify If notify is TRUE the plug-in should call NotifyDependents() to
	notify its dependents. */
	virtual void SetConeDisplay(int s, int notify=TRUE)=0;
	/** Retrieves the light's cone display setting. This indicates if the cone is
	depicted graphically in the viewports.
	\return  TRUE to indicate the cone is displayed; FALSE to indicate it is
	turned off. */
	virtual BOOL GetConeDisplay(void)=0;
	/** Returns the type of shadows used by the light.
	\return  One of the following values: 
	\ref LIGHTSHADOW_NONE 
	\ref LIGHTSHADOW_MAPPED 
	\ref LIGHTSHADOW_RAYTRACED */
	virtual int GetShadowMethod() {return LIGHTSHADOW_NONE;}
	/** Sets the color of the light at the specified time.
	\param t The time to set the color.
	\param rgb The color to set. */
	virtual void SetRGBColor(TimeValue t, Point3& rgb) {}
	/** Returns the color of the light at the specified time and updates the
	validity interval to reflect this parameters validity interval.
	\param t The time to retrieve the value.
	\param valid  The validity interval to intersect with this parameters interval.
	\return  The color of the light at the specified time. */
	virtual Point3 GetRGBColor(TimeValue t, Interval &valid = Interval(0,0)) {return Point3(0,0,0);}        
	/** Sets the intensity of the light to the value passed.
	\param time The time to set the value.
	\param f The value to set. */
	virtual void SetIntensity(TimeValue time, float f) {}
	/** Retrieves the intensity of the light at the specified time and updates the
	validity interval passed to reflect the validity interval of this
	parameter.
	\param t The time to retrieve the value.
	\param valid  The validity interval to intersect with this parameters interval.
	\return  The intensity of the light at the specified time */
	virtual float GetIntensity(TimeValue t, Interval& valid = Interval(0,0)) {return 0.0f;}
	/** Sets the aspect ratio of the light at the specified time.
	\param t The time to set the value.
	\param f The value to set. */
	virtual void SetAspect(TimeValue t, float f) {}
	/** Retrieves the aspect ratio of the light at the specified time and updates
	the validity interval passed to reflect the validity interval of this
	parameter.
	\param t The time to retrieve the value.
	\param valid  The validity interval to intersect with this parameters interval.
	\return  The aspect ratio of the light at the specified time */
	virtual float GetAspect(TimeValue t, Interval& valid = Interval(0,0)) {return 0.0f;}    
	/** Sets the flag to indicate if the light is attenuated.
	\param s Nonzero to indicate the light is attenuated; otherwise 0. */
	virtual void SetUseAtten(int s) {}
	/** Returns TRUE to indicate the light is attenuated; otherwise FALSE. */
	virtual BOOL GetUseAtten(void) {return FALSE;}
	/** Sets the flag to indicate if the light attenuation ranges are displayed.
	\param s Nonzero to indicate the light attenuation ranges are displayed; otherwise 0. */
	virtual void SetAttenDisplay(int s) {}
	/** Returns TRUE if the light attenuation ranges are displayed; otherwise
	FALSE. */
	virtual BOOL GetAttenDisplay(void) {return FALSE;}      
	/** Sets the light to enabled or disables (on or off).
	\param enab Nonzero to set the light to on; zero to turn the light off. */
	virtual void Enable(int enab) {}
	/** Sets the map bias setting at the time passed.
	\param t The time to set the value.
	\param f The map bias value to set. The 3ds Max lights use a range of 0.0 to 100.0. */
	virtual void SetMapBias(TimeValue t, float f) {}
	/** Returns the map bias setting at the time passed and updates the validity
	interval to reflect the validity interval of this parameter.
	\param t The time to retrieve the value.
	\param valid  The validity interval to update to reflect this parameters validity interval.
	\return  The map bias setting at the time passed. */
	virtual float GetMapBias(TimeValue t, Interval& valid = Interval(0,0)) {return 0.0f;}
	/** Sets the map sample range setting to the value passed at the time passed.
	\param t The time to set the value.\
	\param f The value to set. The 3ds Max lights use a range of 0.0 to 20.0. */
	virtual void SetMapRange(TimeValue t, float f) {}
	/** Retrieves the lights map sample range setting at the specified time and
	updates the validity interval to reflect the validity interval of this
	parameter.
	\param t The time to retrieve the value.
	\param valid  The validity interval to update to reflect this parameters validity interval.
	\return  The lights map sample range setting. */
	virtual float GetMapRange(TimeValue t, Interval& valid = Interval(0,0)) {return 0.0f;}
	/** Sets the lights map size parameter to the value passed at the time passed.
	\param t The time to set the value.
	\param f The value to set. */
	virtual void SetMapSize(TimeValue t, int f) {}
	/** Returns the lights map size parameter at the specified time and updates the
	validity interval passed to reflect the validity interval of this
	parameter.
	\param t The time to retrieve the value.
	\param valid  The validity interval to update to reflect this parameters validity interval.
	\return  The lights map size parameter. */
	virtual int GetMapSize(TimeValue t, Interval& valid = Interval(0,0)) {return 0;}
	/** Sets the raytrace bias setting to the value passed at the specified time.
	\param t The time to set the value.
	\param f The value to set. */
	virtual void SetRayBias(TimeValue t, float f) {}
	/** Returns the lights raytrace bias setting at the specified time and updates
	the validity interval passed to reflect the validity interval of this
	parameter.
	\param t The time to retrieve the value.
	\param valid  The validity interval to update to reflect this parameters validity interval.
	\return  The lights raytrace bias setting at the specified time. */
	virtual float GetRayBias(TimeValue t, Interval& valid = Interval(0,0)) {return 0.0f;}
	/** Returns the Use Global Settings flag setting. */
	virtual int GetUseGlobal() {return 0;}
	/** Sets the lights Use Global Settings flag.
	\param a Nonzero indicates the light uses the global settings; zero indicates the
	light uses its own settings. */
	virtual void SetUseGlobal(int a) {}
	/** Returns the lights Cast Shadows flag.
	\return  Nonzero indicates the light casts shadows; otherwise 0. */
	virtual int GetShadow() {return 0;}
	/** Sets the lights Cast Shadows flag.
	\param a Nonzero indicates the light casts shadows; zero indicates the light does
	not cast shadows. */
	virtual void SetShadow(int a) {}
	/** Retrieves the type of shadows used by the light - mapped or raytraced.
	\return  One of the following values: 
	-1: if the Shadow Generator is NULL. (R3 only). 
	0: if the light uses Shadow Maps. 
	1: if the light uses Raytraced Shadows. 
	0xffff: for any other Shadow Generators. (R3 only). */
	virtual int GetShadowType() {return 0;}
	/** Sets the type of shadows used by the light - mapped or raytraced.
	\param a One of the following values: 
	0: This value plugs in a Shadow Map Generator\n
	1: This value plugs in a Raytraced Shadow Generator. 
	Any other value is a NOOP. */
	virtual void SetShadowType(int a) {}
	/** Returns the lights Absolute Map Bias setting.
	\return  Nonzero indicates Absolute Map Bias is on; zero indicates it is
	off. */
	virtual int GetAbsMapBias() {return 0;}
	/** Sets the lights Absolute Map Bias setting.
	\param a Nonzero indicates Absolute Map Bias is on; zero indicates it is off. */
	virtual void SetAbsMapBias(int a) {}
	/** Returns the lights Overshoot on / off setting. Nonzero indicates overshoot
	is on; otherwise 0. */
	virtual int GetOvershoot() {return 0;}
	/** Sets the lights Overshoot on / off setting.
	\param a Nonzero indicates overshoot is on; otherwise 0. */
	virtual void SetOvershoot(int a) {}
	/** Returns the lights Projector on / off setting. Nonzero indicates this light
	projects an image; otherwise 0. */
	virtual int GetProjector() {return 0;}
	/** Sets the lights projector on / off setting.
	\param a Nonzero indicates this light projects an image; otherwise 0. */
	virtual void SetProjector(int a) {}
	/** Returns the list of names of items included or excluded by this light. See
	Class NameTab. */
	virtual ExclList* GetExclList() {return NULL;}
	/** Returns TRUE if the light's name list is of items to be included by the
	light. Returns FALSE if the list is of items to exclude from the light. */
	virtual BOOL Include() {return FALSE;}
	/** Returns the map used by a projector light. */
	virtual Texmap* GetProjMap() {return NULL;}
	/** Sets the image(s) used by the projector light.
	\param pmap The map to use. */
	virtual void SetProjMap(Texmap* pmap) {}
	/**		Updates the display of the light's target distance in the light's rollup
	page.
	\param t The time to retrieve the distance.
	\param inode The light node. */
	virtual void UpdateTargDistance(TimeValue t, INode* inode) {}
};
/*------------------------------------------------------------------- 
HelperObject:
---------------------------------------------------------------------*/

/** This is used as a base class to create helper object plug-ins. It simply
	provides implementations for a few of the methods of Animatable and Object.
	\see  Class Object, Class Animatable.
	 */
class HelperObject : public Object {
public:
	/** Implemented by the System. 
	Returns the super class ID of this plug-in type: HELPER_CLASS_ID */
	SClass_ID SuperClassID() { return HELPER_CLASS_ID; }
	/** Implemented by the System. 
	Returns 0 to indicate this object type may not be rendered. */
	int IsRenderable() { return(0); }
	/** Implemented by the System. 
	Sets the default node name to "Helper". */
	virtual void InitNodeName(MSTR& s) { s = _M("Helper"); }
	/** Implemented by the System. 
	Returns TRUE to indicate the object color is used for display. */
	virtual int UsesWireColor() { return FALSE; }   // TRUE if the object color is used for display
	virtual BOOL NormalAlignVector(TimeValue t,Point3 &pt, Point3 &norm) {pt=Point3(0,0,0);norm=Point3(0,0,-1);return TRUE;}
};


/// \defgroup Grid_Types Grid Types
//@{
#define GRID_PLANE_NONE		-1
#define GRID_PLANE_TOP		0
#define GRID_PLANE_LEFT		1
#define GRID_PLANE_FRONT	2
#define GRID_PLANE_BOTTOM	3
#define GRID_PLANE_RIGHT	4
#define GRID_PLANE_BACK		5
//@}

/** This is a base class used to create construction grid objects. It implements a
	few of the methods of Animatable and Object and provides a few for working with
	construction grids.  
	\see  Class HelperObject, Class INode,  Class Object,  Class ViewExp, Class Matrix3
	*/
class ConstObject : public HelperObject {
private:
	bool m_transient;
	bool m_temporary;	// 030730  --prs.
public:
	ConstObject() { m_transient = m_temporary = false; }

	// Override this function in HelperObject!
	/** Implemented by the System. 
	Returns 1 to indicate this object is a construction grid object. */
	int IsConstObject() { return 1; }

	// Methods specific to construction grids:
	/** This method returns the construction grid transformation matrix. This is
	the world space orientation and position of the construction plane.
	\param t The time to retrieve the matrix.
	\param inode The node in the scene corresponding to the construction grid object.
	\param vpt The viewport the TM is being returned for. Certain construction grid
	objects might have a different plane for different viewports.
	\param tm The transform matrix for this view is returned here. */
	virtual void GetConstructionTM( TimeValue t, INode* inode, ViewExp *vpt, Matrix3 &tm ) = 0;     // Get the transform for this view
	virtual void SetConstructionPlane(int which, int notify=TRUE) = 0;
	virtual int  GetConstructionPlane(void) = 0;
	/** This method is specific to construction grids. The system calls this method
	to retrieve the snap dimension of the grid. In the 3ds Max user interface
	for the construction grid helper object there is a spinner for 'Spacing'.
	This is the spacing for the grid. When GetSnaps() is called the
	Point3 returned will have this value in all three axes. This value is used,
	for example, when you create a box or other primitive and are setting the
	height dimension.
	\param t The time to retrieve the snap values. */
	virtual Point3 GetSnaps( TimeValue t ) = 0;    // Get snap values
	virtual void SetSnaps(TimeValue t, Point3 p) = 0;

	virtual BOOL NormalAlignVector(TimeValue t,Point3 &pt, Point3 &norm) {pt=Point3(0,0,0);norm=Point3(0,0,-1);return TRUE;}

	//JH 09/28/98 for design ver
	bool IsTransient()const {return m_transient;}
	void SetTransient(bool state = true) {m_transient = state;}

	// grid bug fix, 030730  --prs.
	bool IsTemporary() const { return m_temporary; }
	void SetTemporary(bool state = true) { m_temporary = state; }

	//JH 11/16/98
	virtual void SetExtents(TimeValue t, Point3 halfbox)=0;
	virtual Point3 GetExtents(TimeValue t)=0;
	//JH 09/28/98 for design ver
	//	bool IsImplicit()const {return m_implicit;}
	//	void SetImplicit(bool state = true) {m_implicit = state;}

};

/*------------------------------------------------------------------- 
GeomObject: these are the Renderable objects.  
---------------------------------------------------------------------*/

/** This is the base class for the creation of Geometric Object plug-ins. This
	class represents an object that has geometry and is renderable.  
	\see  Class Object, Class Mesh*/
class GeomObject : public Object {
public:         
	virtual void InitNodeName(MSTR& s) { s = _M("Object"); }
	SClass_ID SuperClassID() { return GEOMOBJECT_CLASS_ID; }

	virtual int IsRenderable() { return(1); }               

	// If an object creates different  meshes depending on the 
	// particular instance (view-dependent) it should return 1.
	/** If an object creates different meshes depending on the particular instance
	(view-dependent) it should return nonzero; otherwise 0. */
	virtual int IsInstanceDependent() { return 0; }

	// GetRenderMesh should be implemented by all renderable GeomObjects.
	// set needDelete to TRUE if the render should delete the mesh, FALSE otherwise
	// Primitives that already have a mesh cached can just return a pointer
	// to it (and set needDelete = FALSE).
	/** This method should be implemented by all renderable GeomObjects. It
	provides a mesh representation of the object for use by the renderer.
	Primitives that already have a mesh cached can just return a pointer to it
	(and set needDelete to FALSE). 
	Implementations of this method which take a
	long time should periodically call View::CheckForRenderAbort() to
	see if the user has canceled the render. If canceled, the function can
	either return NULL, or return a non null pointer with the appropriate value
	for needDelete. (If needDelete is TRUE a non-null mesh will
	be deleted.)
	\param t The time to get the mesh.
	\param inode The node in the scene.
	\param view If the renderer calls this method it will pass the view information here.
	See Class View.
	\param needDelete Set to TRUE if the renderer should delete the mesh, FALSE otherwise.
	\return  A pointer to the mesh object. */
	CoreExport virtual Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);

	// Objects may now supply multiple render meshes ( e.g. particle systems). If this function
	// returns a positive number, then GetMultipleRenderMesh and GetMultipleRenderMeshTM will be 
	// called for each mesh, instead of calling GetRenderMesh // DS 5/10/00
	/**		Objects may supply multiple render meshes ( e.g. particle systems). If this
	method returns a positive number, then GetMultipleRenderMesh and
	GetMultipleRenderMeshTM will be called for each mesh, instead of
	calling GetRenderMesh.
	\return  The number of render meshes, or 0 to indicate that multiple meshes
	aren't supported. */
	virtual int NumberOfRenderMeshes() { return 0; } // 0 indicates multiple meshes not supported.

	// For multiple render meshes, this method must be implemented. 
	// set needDelete to TRUE if the render should delete the mesh, FALSE otherwise
	// meshNumber specifies which of the multiplie meshes is being asked for.// DS 5/10/00
	/**		For multiple render meshes, this method must be implemented. set
	needDelete to TRUE if the render should delete the mesh, FALSE
	otherwise.
	\param t The time at which to obtain the mesh.
	\param inode The pointer to the node.
	\param view A reference to the view.
	\param needDelete TRUE if the mesh needs to be deleted, otherwise FALSE.
	\param meshNumber Specifies which of the multiplie meshes is being asked for. */
	virtual	Mesh* GetMultipleRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete, int meshNumber) { return NULL; }

	// For multiple render meshes, this method must be implemented. 
	// meshTM should be returned with the transform defining the offset of the particular mesh in object space.
	// meshTMValid should contain the validity interval of meshTM // DS 5/10/00
	/** For multiple render meshes, this method must be implemented.
	\param t The time at which to obtain the mesh.
	\param inode The pointer to the node.
	\param view A reference to the view.
	\param meshNumber Specifies which of the multiplie meshes is being asked for.
	\param meshTM Should be returned with the transform defining the offset of the particular
	mesh in object space.
	\param meshTMValid Should contain the validity interval of meshTM. */
	virtual void GetMultipleRenderMeshTM(TimeValue t, INode *inode, View& view, int meshNumber, 
		Matrix3& meshTM, Interval& meshTMValid) { return;  }

	// If this returns NULL, then GetRenderMesh will be called
	/** This method provides a patch mesh representation of the object for use by the
	renderer. If this method returns NULL, then GetRenderMesh() will be
	called.
	\param t The time to get the patch mesh.
	\param inode The node in the scene.
	\param view If the renderer calls this method it will pass the view information here. See Class View.
	\param needDelete Set to TRUE if the renderer should delete the patch mesh, FALSE otherwise.
	\return  A pointer to the patch mesh. See Class PatchMesh. */
	CoreExport virtual PatchMesh* GetRenderPatchMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);

	CoreExport Class_ID PreferredCollapseType();

	/**	Returns TRUE if this object can do displacement mapping; otherwise FALSE. */
	virtual BOOL CanDoDisplacementMapping() { return 0; }

private:
};


//-- Particle Systems ---------------------------------------------------

// A force field can be applied to a particle system by a SpaceWarp.
// The force field provides a function of position in space, velocity
// and time that gives a force.
// The force is then used to compute an acceleration on a particle
// which modifies its velocity. Typically, particles are assumed to
// to have a normalized uniform mass==1 so the acceleration is F/M = F.
/** A Space Warp modifier usually uses an instance of this class and implements the
	Force() method. The force field is then applied to the particle system
	when the particle system is bound to the Space Warp. This class is similar in
	some respects to the Deformer class as used by a modifier. The difference is
	that a deformer modifies the points of the object. A force field is really an
	acceleration - it modifies velocity rather than position. 
	The force field provides a function of position in space, velocity and time
	that gives a force. The force is then used to compute an acceleration on a
	particle which modifies its velocity. Typically, particles are assumed to have
	a normalized uniform mass equal to 1 so the acceleration is F/M = F.  
	\see  Class Point3, Class WSMObject.*/
class ForceField : public InterfaceServer {
public:
	/** This method is called to compute a force on the particle based on its
	position, velocity and the current time.
	\param t The time to compute the force.
	\param pos The current position of the particle.
	\param vel The current velocity of the particle.
	\param index The index of the particle being forced.
	\return  The force vector as a Point3. This vector is added to the
	velocity. */
	virtual Point3 Force(TimeValue t,const Point3 &pos, const Point3 &vel, int index)=0;
	virtual void SetRandSeed(int seed) {}
	/**	This method is callled to delete this instance of the ForceField. This
	should be called, for example, by developers who use the
	WSMObject::GetForceField() method. */
	virtual void DeleteThis() {}
};

// A collision object can be applied to a particle system by a SpaceWarp.
// The collision object checks a particle's position and velocity and
// determines if the particle will colide with it in the next dt amount of
// time. If so, it modifies the position and velocity.

/** A collision object can be applied to a particle system by a Space Warp. The
	collision object checks a particle's position and velocity and determines if
	the particle will collide with it in the next dt amount of time. If so,
	it modifies the position and velocity.  
	\see  Class Point3
	*/
class CollisionObject : public InterfaceServer {
public:
	// Check for collision. Return TRUE if there was a collision and the position and velocity have been modified.
	/** This method checks a particles position and velocity to determine if there
	was be a collision between the particle and this collision object. If there
	was a collision, the particles position and velocity should be
	modified. 
	The plug-in may compute a line segment between where the particle is now,
	and where it will be in dt amount of time. The plug-in then checks
	if the line segment intersects the collision object. If so, it would
	compute the resulting position, and modify the velocity vector vel
	to point in the new direction (presumably reflected across the surface
	normal). 
	3ds Max 3.0 introduced interparticle collision (where particles may collide
	with other particles). In order to implement interparticle collision (IPC)
	in the presence of collision objects, it became necessary to generalize the
	operation of the deflectors so that they didn't always work in discrete
	time intervals. 
	That is, in the general case of an unidentified number of particles
	interacting in an unspecified way, it was necessary to allow everything
	involved in that system to update to specified times without moving
	past that time. 
	In the absence of IPC enabled, the particle system calls the bound
	collision objects with the parameter UpdatePastCollide == TRUE, and
	the deflector checks all collisions, updates particles based on their
	collisions with deflectors and the ensuing, remaining time intervals in
	dt subsequent to the collisions. 
	In the presence of IPC that won't work. When IPC is active, all particles
	need to be updated to the time of the first collision in the system in
	dt, whether that collision be between particles or between particles
	and deflectors. Thus, in the presence of IPC, all particle updates to bound
	deflectors are called with UpdatePastCollide == FALSE. In that case,
	the collision objects return both the position and velocity of the updated
	particles and the time at which the collision occurred. 
	All such times are compared, along with all possible internally calculated
	IPC event times. If there are any nonnegative times returned greater than
	or equal to zero, all particle states are reverted to their states at the
	beginning of the interval and then updated to the precise minimum time
	returned as the earliest collision. And then everything starts up
	again trying to update itself to the next integral time, when control can
	pass back to whatever is asking the particles to update themselves. If
	there are other collisions in that time, it happens again. 
	This whole set of operations happens after any true returns from the
	trilinear sort/sweep correlator that looks for the possibility of
	collisions. If there are no possible collisions, everything proceeds
	through a complete interval normally.
	\param t The time to check for collision.
	\param pos The position of the particle to check and potentially modify.
	\param vel The velocity vector of the particle to check and potentially modify.
	\param dt This is an increment of time - the step size. The method checks if the
	particle will collide in this amount of time.
	\param index The index of the particle being collided.
	\param ct An array of floating point times at which the collisions occurred.
	\param UpdatePastCollide This is a flag to tell the collision object to update the particle past the
	collision to the remainder of input dt or to output the state of the
	particle at the collision. In the presence of interparticle collision
	enable, we have to update to the times of collisions and then retest from
	there. See the Remarks.
	\return  TRUE if there was a collision and the position and velocity have
	been modified; otherwise FALSE. */
	virtual BOOL CheckCollision(TimeValue t,Point3 &pos, Point3 &vel, float dt,int index, float *ct=NULL, BOOL UpdatePastCollide=TRUE)=0;
	/**	This method provides a way of identifying the 'parent' Deflector for a
	CollisionObject available to a particle system. This must be implemented by
	all Deflectors. It returns the object pointer to the Deflector from which
	the Collision object is derived. */
	virtual Object *GetSWObject()=0;
	virtual void SetRandSeed(int seed) {}
	virtual void DeleteThis() {}
};

/** \defgroup Returne_valuse_of_ParticleCenter___ Returne valuse of ParticleCenter() 
* value indicating where the particle geometry (mesh) lies in relation to the particle position.*/
//@{
#define PARTCENTER_HEAD		1  //!< Particle geometry lies behind the particle position
#define PARTCENTER_CENTER	2  //!< Particle geometry is centered around particle position
#define PARTCENTER_TAIL		3  //!< Particle geometry lies in front of the particle position
//@}

// The particle system class derived from GeomObject and still has
// GEOMOBJECT_CLASS_ID as its super class.
//
// Given an object, to determine if it is a ParticleObject, call
// GetInterface() with the ID  I_PARTICLEOBJ or use the macro
// GetParticleInterface(anim) which returns a ParticleObject* or NULL.

/** This is the base class for creating particle system plug-ins. 
Many particle systems may be derived from class SimpleParticle instead
of this class. See Class SimpleParticle for more details. 
Note: This class is derived from GeomObject and still has
GEOMOBJECT_CLASS_ID as its super class. To determine if an object is a
ParticleObject, call: 
Animatable::GetInterface() with the ID I_PARTICLEOBJ or use the
macro: 
GetParticleInterface(anim) where anim is the object in question.
This will return a ParticleObject* or NULL. See
Class Animatable . 
Note: See the method Animatable::GetProperty() for details on choosing
the method used to evaluate the particle system during motion blur rendering.

\see Class Animatable, Class GeomObject, Class ForceField, Class CollisionObject, Class ShadeContext
*/
class ParticleObject : public GeomObject {
public:
	BOOL IsParticleSystem() {return TRUE;}

	/** This method is called to add the force field object passed to the list of
	force field objects operating on this particle system.
	\param ff Points to an instance of a ForceField object.
	\par Sample Code:
	\code
	void SimpleParticle::ApplyForceField(ForceField *ff)
	{
		fields.Append(1,&ff);
	}
	\endcode */
	virtual void ApplyForceField(ForceField *ff)=0;
	/** This method is called to add the collision object passed to the list of
	collision objects operating on this particle system.
	\param co Points to an instance of a collision object.
	\return  If a particle does not support this method it should return FALSE;
	otherwise return TRUE.
	\par Sample Code:
	\code
	BOOL SimpleParticle::ApplyCollisionObject(CollisionObject *co)
	{
		cobjs.Append(1,&co);
		return TRUE;
	}
	\endcode  */
	virtual BOOL ApplyCollisionObject(CollisionObject *co)=0; // a particle can choose no to support this and return FALSE

	// A particle object IS deformable, but does not let itself be
	// deformed using the usual GetPoint/SetPoint methods. Instead
	// a space warp must apply a force field to deform the particle system.
	/** Implemented by the System. 
	This method returns TRUE to indicate it is deformable. A particle object is
	deformable, but does not let itself be deformed using the usual
	GetPoint() / SetPoint() methods. Instead a space warp must
	apply a force field to deform the particle system. */
	int IsDeformable() {return TRUE;} 

	// Particle objects don't actually do a shallow copy and therefore 
	// cannot be cached.
	/** Implemented by the System. 
	This method returns FALSE to indicate the object cannot be cached. Particle
	objects don't perform a shallow copy and therefore cannot be cached. */
	BOOL CanCacheObject() {return FALSE;}


	/**	Implemented by the System. 
	This method is inherited from Class Object.
	This is a default implementation provided for particle systems.
	\param t The time to compute the normal align vector.
	\param pt The point of intersection.
	\param norm The normal at the point of intersection.
	\return  TRUE if this method is implemented to return the normal align
	vector; otherwise FALSE. */
	virtual BOOL NormalAlignVector(TimeValue t,Point3 &pt, Point3 &norm) {pt=Point3(0,0,0);norm=Point3(0,0,-1);return TRUE;}


	// These methods provide information about individual particles
	/**	Returns the position of the specified particle in world space at the time
	passed. 
	The Particle Age texture map and the Particle Motion Blur texture map use
	this method.
	\param t The time to return the particle position.
	\param i The index of the particle.
	\note When a texture map calls these methods, the particle index i
	is passed to the texmap in the data member ShadeContext::mtlNum. The
	particle systems encode the index of the particle associated with the face
	of the particle mesh being shaded into the mtlNum. For instance,
	once the particle system generates a mesh to be rendered, every face of the
	mesh corresponds to a particle. This isn't a one-to-one correspondance
	because there are more faces than particles (if the particles are
	represented as tetrahedrons there are four faces per particle). When a
	texture map or material that is shading a mesh generated by a particle
	system wants to know which particle the face is associated with it gets
	this info out of the ShadeContext::mtlNum. 
	For example, here is a fragment of the code from the Particle Age texture
	map where it evaluates the color of the point being shaded:
	\code
	AColor PartAgeTex::EvalColor(ShadeContext& sc)
	{
		...
	// Evaluate...
		Object *ob = sc.GetEvalObject();
		if (ob && ob->IsParticleSystem()) {
			ParticleObject *obj = (ParticleObject*)ob;
			TimeValue t = sc.CurTime();
			TimeValue age = obj->ParticleAge(t,sc.mtlNum);
			TimeValue life = obj->ParticleLife(t,sc.mtlNum);
			...etc.
		}
	} 
	\endcode */
	virtual Point3 ParticlePosition(TimeValue t,int i) {return Point3(0,0,0);}
	/**		Returns the velocity of the specified particle at the time passed (in 3ds
	Max units per tick). This is specified as a vector. The Particle Age
	texture map and the Particle Motion Blur texture map use this method.
	\param t The time to return the particle velocity.
	\param i The index of the particle. */
	virtual Point3 ParticleVelocity(TimeValue t,int i) {return Point3(0,0,0);}
	/**		Returns the world space size of the specified particle in at the time
	passed. 
	The Particle Age texture map and the Particle Motion Blur texture map use
	this method.
	\param t The time to return the particle size.
	\param i The index of the particle. */
	virtual float ParticleSize(TimeValue t,int i) {return 0.0f;}
	/**		Returns a value indicating where the particle geometry (mesh) lies in
	relation to the particle position. 
	This is used by Particle Motion Blur for example. It gets the point in
	world space of the point it is shading, the size of the particle from
	ParticleSize(), and the position of the mesh from
	ParticleCenter(). Given this information, it can know where the
	point is, and it makes the head and the tail more transparent.
	\param t The time to return the particle center.
	\param i The index of the particle.
	\return  One of the following: 
	\ref PARTCENTER_HEAD 
	The particle geometry lies behind the particle position. 
	\ref PARTCENTER_CENTER 
	The particle geometry is centered around particle position. 
	\ref PARTCENTER_TAIL 
	The particle geometry lies in front of the particle position. */
	virtual int ParticleCenter(TimeValue t,int i) {return PARTCENTER_CENTER;}
	/**	Returns the age of the specified particle -- the length of time it has been
	'alive'. 
	The Particle Age texture map and the Particle Motion Blur texture map use
	this method.
	\param t Specifies the time to compute the particle age.
	\param i The index of the particle. */
	virtual TimeValue ParticleAge(TimeValue t, int i) {return -1;}
	/**		Returns the life of the particle -- the length of time the particle will be
	'alive'. 
	The Particle Age texture map and the Particle Motion Blur texture map use
	this method.
	\param t Specifies the time to compute the particle life span.
	\param i The index of the particle. */
	virtual TimeValue ParticleLife(TimeValue t, int i) {return -1;}

	// This tells the renderer if the ParticleObject's topology doesn't change over time
	// so it can do better motion blur.  This means the correspondence of vertex id to
	// geometrical vertex must be invariant.
	/**		If a particle system has a fixed number of particles of fixed topology,
	then it can return TRUE for this method, and the renderer will then compute
	the image motion blur velocities based on the vertex motions, giving motion
	blur for rotating particles etc. If the particle system is topology-varying
	it should return FALSE. */
	virtual BOOL HasConstantTopology() { return FALSE; }
};

//----------------------------------------------------------------------


/*------------------------------------------------------------------- 
ShapeObject: these are the open or closed hierarchical shape objects.  
---------------------------------------------------------------------*/

class PolyShape;
class BezierShape;
class MeshCapInfo;
class PatchCapInfo;
class ShapeHierarchy;

/// \defgroup GenerateMesh_Options GenerateMesh Options
//@{
#define GENMESH_DEFAULT -1	//!< Use whatever is stored in the ShapeObject's UseViewport flag
#define GENMESH_VIEWPORT 0
#define GENMESH_RENDER 1
//@}

/** \defgroup number_of_ShapeObject_references_subanims number of ShapeObject references/subanims
 * The number of ShapeObject references/subanims are set to the number of references and subanims 
 * in the ShapeObject class, you can use them to make your code more bullet-proof should the number 
 * of references change in the future. See maxsdk/include/splshape.h for an example of how they can be used. 
 */
//@{
#define SHAPE_OBJ_NUM_REFS 1
#define SHAPE_OBJ_NUM_SUBS 1
//@}

/// \defgroup Parameter_block_reference_indices Parameter block reference indices
//@{
#define USERPBLOCK SHAPE_OBJ_NUM_REFS		//!< User's parameter block
#define IPBLOCK (SHAPE_OBJ_NUM_REFS + 1)	//!< Interpolations parameter block
//@}

class IParamBlock;
class IParamMap;

#define SHAPE_RECT_RENDERPARAMS_PROPID PROPID_USER+10

//! Rectangular Shape Render Parameters API
//! This interface gives access to the new Renderable SPline parameters for Rectangular Shapes
//! The IShapeRectRenderParams interface can be retrieved like this:
//! IShapeRectRenderParams* rparams = (IShapeRectRenderParams*)obj->GetProperty(SHAPE_RECT_RENDERPARAMS_PROPID);
//! Note that this interface contains VPT and non VPT parameters. The non VPT parameters are used for the renderer
//! and the viewport in case GetViewportOrRenderer() returns true, or if GetViewportOrRenderer() returns false and GetUseViewPort() returns false.
//! Otherwise the non vpt settings only control the mesh for the renderer, not the viewport.
class IShapeRectRenderParams : public AnimProperty
{
protected :
	ShapeObject *mShape;
public: 
	//! Constructor
	//! \param so - ShapeObject that publishes this interface
	IShapeRectRenderParams(ShapeObject *so) : mShape(so)
	{
	}

	//! Gets the Rectangular setting of the shape
	//! \param t - Time to get the value for
	//! \return Rectangular setting of the shape
	CoreExport BOOL GetRectangular(TimeValue t) const;
	//! Sets the Rectangular setting of the shape
	//! \param t - Time to set the value for
	//! \param rectangular - if true, shape is rectangular, if false radial
	CoreExport void SetRectangular(TimeValue t, BOOL rectangular);

	//! Gets the Width of the rectangular section of the shape
	//! \param t - Time to get the value for
	//! \return Width of the rectangular section of the shape
	CoreExport float GetWidth(TimeValue t) const;
	//! Sets the Width of the rectangular section of the shape
	//! \param t - Time to set the value for
	//! \param width - Width Value (must be positive)
	CoreExport void SetWidth(TimeValue t, float width);

	//! Gets the Length of the rectangular section of the shape
	//! \param t - Time to get the value for
	//! \return Length of the rectangular section of the shape
	CoreExport float GetLength(TimeValue t) const;
	//! Sets the Length of the rectangular section of the shape
	//! \param t - Time to set the value for
	//! \param length - Length Value (must be positive)
	CoreExport void SetLength(TimeValue t, float length);

	//! Gets the Angle of the rectangular section of the shape
	//! \param t - Time to get the value for
	//! \return Angle of the rectangular section of the shape
	CoreExport float GetAngle2(TimeValue t) const;
	//! Sets the Angle of the rectangular section of the shape
	//! \param t - Time to set the value for
	//! \param angle - Angle Value for rectangular shape
	CoreExport void SetAngle2(TimeValue t, float angle);

	//! Gets the AspectLock parameter of the rectangular section of the shape
	//! \param t - Time to get the value for
	//! \return AspectLock parameter of the rectangular section of the shape
	CoreExport BOOL GetAspectLock(TimeValue t) const;
	//! Sets the AspectLock parameter of the rectangular section of the shape
	//! \param t - Time to set the value for
	//! \param aspectLock - TRUE if aspect is locked, FALSE otherwise
	CoreExport void SetAspectLock(TimeValue t, BOOL aspectLock);

	//! Gets the Rectangular setting of the shape for the viewport
	//! \param t - Time to get the value for
	//! \return Rectangular setting of the shape for the viewport
	CoreExport BOOL GetVPTRectangular(TimeValue t) const;
	//! Sets the Rectangular parameter of the rectangular section of the shape for the viewport
	//! \param t - Time to set the value for
	//! \param rectangular - if true, shape is rectangular, if false radial
	CoreExport void SetVPTRectangular(TimeValue t, BOOL rectangular);

	//! Gets the Width of the rectangular section of the shape for the viewport
	//! \param t - Time to get the value for
	//! \return Width of the rectangular section of the shape for the viewport
	CoreExport float GetVPTWidth(TimeValue t) const;
	//! Sets the Width of the rectangular section of the shape for the viewport
	//! \param t - Time to set the value for
	//! \param width - Width Value (must be positive)
	CoreExport void SetVPTWidth(TimeValue t, float width);

	//! Gets the Length of the rectangular section of the shape for the viewport
	//! \param t - Time to get the value for
	//! \return Length of the rectangular section of the shape for the viewport
	CoreExport float GetVPTLength(TimeValue t) const;
	//! Sets the Length of the rectangular section of the shape for the viewport
	//! \param t - Time to set the value for
	//! \param length - Length Value (must be positive)
	CoreExport void SetVPTLength(TimeValue t, float length);

	//! Gets the Angle of the rectangular section of the shape for the viewport
	//! \param t - Time to get the value for
	//! \return Angle of the rectangular section of the shape for the viewport
	CoreExport float GetVPTAngle2(TimeValue t) const;
	//! Sets the Angle of the rectangular section of the shape for the viewport
	//! \param t - Time to set the value for
	//! \param angle - Angle Value for rectangular shape
	CoreExport void SetVPTAngle2(TimeValue t, float angle);

	//! Gets the AspectLock parameter of the rectangular section of the shape for the viewport
	//! \param t - Time to get the value for
	//! \return AspectLock parameter of the rectangular section of the shape for the viewport
	CoreExport BOOL GetVPTAspectLock(TimeValue t) const;
	//! Sets the AspectLock parameter of the rectangular section of the shape for the viewport
	//! \param t - Time to set the value for
	//! \param aspectLock - TRUE if aspect is locked, FALSE otherwise
	CoreExport void SetVPTAspectLock(TimeValue t, BOOL aspectLock);

	//! Gets the AutoSmooth parameter of the shape
	//! \param t - Time to get the value for
	//! \return AutoSmooth parameter of the shape
	CoreExport BOOL GetAutosmooth(TimeValue t) const;
	//! Sets the AutoSmooth parameter of the shape for the viewport
	//! \param t - Time to set the value for
	//! \param autosmooth - TRUE if resulting mesh should be autosmoothed, FALSE otherwise
	CoreExport void SetAutosmooth(TimeValue t, BOOL autosmooth);

	//! Gets the AutoSmooth Threshold of the shape
	//! \param t - Time to get the value for
	//! \return AutoSmooth Threshold of the shape
	CoreExport float GetAutosmoothThreshold(TimeValue t) const;
	//! Sets the AutoSmooth Threshold of the shape 
	//! \param t - Time to set the value for
	//! \param threshold - threashold angle in radians
	CoreExport void SetAutosmoothThreshold(TimeValue t, float threshold);

	//! ID of this AnimProperty
	//! \return ID of this AnimProperty
	DWORD ID() { return SHAPE_RECT_RENDERPARAMS_PROPID;}
};

/** ShapeObjects are open or closed hierarchical shapes made up of one or more
pieces. This base class defines a set of methods that plug-in shapes must
implement. 
Note: Many plug-in shapes may be derived from
Class SimpleSpline rather than this
class and have fewer methods to implement. See that class for more details. 
Any classes subclassing
off of ShapeObject should be sure to call the ShapeObject constructor in their
constructor, in order to properly initialize the fields contained in the
ShapeObject. This is the thickness field, which specifies the
thickness of the mesh generated from the shape at rendering time. For
example:
\code
LinearShape::LinearShape() : ShapeObject()
{
	...
}
\endcode
Also, the ShapeObject contains Load and Save methods, which handle the
storage of the data contained within the ShapeObject. In order to properly
store this information, classes which subclass off of ShapeObject need to call
the ShapeObject Load and Save methods before storing their information. For
example: 
\code
IOResult LinearShape::Save(ISave *isave)
{
	IOResult res = ShapeObject::Save(isave);
	if(res != IO_OK)
		return res;
	...
}

IOResult LinearShape::Load(ILoad *iload)
{
	IOResult res = ShapeObject::Load(iload);
	if(res != IO_OK)
		return res;
	...
}
\endcode
The number of ShapeObject references/subanims are defined as
SHAPE_OBJ_NUM_REFS and SHAPE_OBJ_NUM_SUBS in /include/object.h
and are set to the number of references and subanims in the ShapeObject class,
you can use them to make your code more bullet-proof should the number of
references change in the future. See maxsdk/include/splshape.h for an example
of how they can be used.  
 \see  Class GeomObject, Class PolyShape
 */
class ShapeObject : public GeomObject
{
	friend class SObjRenderingDlgProc;
	friend class ShapePostLoadCallback;
	friend class IShapeRectRenderParams;
	IObjParam *ip;
	IParamBlock *sopblock;	// New for r4, renderable version parameter block
	static IParamMap *pmap;
	int loadVersion;
	// Display mesh cache stuff
	Mesh meshCache;
	Interval cacheValid;
	int cacheType;

public:
	CoreExport ShapeObject();
	CoreExport ~ShapeObject();	// Must call this on destruction

	// from InterfaceServer
	CoreExport virtual BaseInterface* GetInterface(Interface_ID iid);

	// from GeomObject
	CoreExport virtual void* GetInterface(ULONG id);

	// display functions from BaseObject
	CoreExport virtual bool RequiresSupportForLegacyDisplayMode() const;
	CoreExport virtual bool UpdateDisplay(
		unsigned long renderItemCategories, 
		const MaxSDK::Graphics::MaterialRequiredStreams& materialRequiredStreams, 
		TimeValue t);

	virtual BOOL IsShapeObject() { return TRUE; }

	/** Computes the intersection point of the ray passed and the shape. 
	\note This method has a default implementation and
	it is not necessary to define this method in classes derived from
	ShapeObject.
	\param t The time to compute the intersection.
	\param ray Ray to intersect.
	\param at The point of intersection.
	\param norm The surface normal at the point of intersection.
	\return  Nonzero if a point of intersection was found; otherwise 0. */
	virtual int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm) {return FALSE;}
	virtual void InitNodeName(MSTR& s) { s = _M("Shape"); }
	SClass_ID SuperClassID() { return SHAPE_CLASS_ID; }
	CoreExport virtual int IsRenderable();
	/**	In order to simplify things for subclasses of ShapeObject, this method is
	now available. It should be called whenever the ShapeObject-based object is
	copied. It takes care of copying all the data to the ShapeObject from
	another ShapeObject-based object 
	Implemented by the System.
	\param from The ShapeObject to copy from. */
	CoreExport virtual void CopyBaseData(ShapeObject &from);
	// Access methods
	/** Implemented by the System. 
	Returns the shape's thickness setting.
	\param t The time to obtain the thickness.
	\param ivalid The validity interval. */
	CoreExport float GetThickness(TimeValue t, Interval &ivalid);
	/**	This method returns the number of sides for the cross-section of the
	rendering mesh version of the shape for the specified time.
	\param t The time to obtain the thickness.
	\param ivalid The validity interval. */
	CoreExport int GetSides(TimeValue t, Interval &ivalid);
	/**	This method returns the angle that the cross-section of the rendering mesh
	will be rotated to, for the specified time.
	\param t The time to obtain the thickness.
	\param ivalid The validity interval. */
	CoreExport float GetAngle(TimeValue t, Interval &ivalid);
	/**	This method returns the thickness of the viewport version of the rendering
	mesh. This is not an animated parameter. */
	CoreExport float GetViewportThickness();
	/**	This method returns the number of sides for the cross-section for the
	viewport version of the rendering mesh. This is not an animated parameter.
	*/
	CoreExport int GetViewportSides();
	/**	This method returns the angle that the cross-section of the viewport
	version of the rendering mesh will be rotated to. This is not an animated
	parameter. */
	CoreExport float GetViewportAngle();
	/**	Implemented by the System. 
	The ShapeObject class now has a "renderable" flag contained within it.
	Access to this is via this method and SetRenderable(). If this is set to
	TRUE and the node is set to renderable, the spline will be rendered. This
	defaults to FALSE. */
	CoreExport BOOL GetRenderable();
	/**	Implemented by the System. 
	Returns TRUE if the generate UVs switch is on; FALSE if off. */
	CoreExport BOOL GetGenUVs();
	/**	This method returns TRUE if the "Display Render Mesh" switch is on. FALSE
	when the switch is off. */
	CoreExport BOOL GetDispRenderMesh();
	/**	This method returns TRUE if the "Use Viewport Settings" switch is on. FALSE
	when the switch is off. */
	CoreExport BOOL GetUseViewport();
	/**	This method returns the value of the Viewport/Render switch and either
	returns GENMESH_VIEWPORT or GENMESH_RENDER. */
	CoreExport BOOL GetViewportOrRenderer();
	/** Implemented by the System. 
	Sets the thickness setting of the shape to the specified value.
	\param t The time at which to set the thickness.
	\param thick The new thickness setting for the shape. */
	CoreExport void SetThickness(TimeValue t, float thick);
	/**	This method allows you to set the number of sides for the rendering mesh
	version of the shape for the specified time. The allowable ranges for this
	parameter are 3-100.
	\param t The time at which to set the number of sides.
	\param s The number of sides you wish to set. */
	CoreExport void SetSides(TimeValue t, int s);
	/**	This method allows you to set the cross-section rotation angle for the
	rendering mesh version of the shape, in degrees, for the specified time.
	\param t The time at which to set the angle.
	\param a The angle you wish to set, in degrees. */
	CoreExport void SetAngle(TimeValue t, float a);
	CoreExport void SetViewportThickness(float thick);
	/**	This method allows you to set the number of sides for the viewport version
	of the rendering mesh. This is not an animated parameter.
	\param s The number of viewport sides you wish to set. */
	CoreExport void SetViewportSides(int s);
	/**	This method allows you to set the angle that the cross-section of the
	viewport version of the rendering mesh will be rotated to, in degrees. This
	is not an animated parameter.
	\param a The viewport angle you wish to set, in degrees. */
	CoreExport void SetViewportAngle(float a);
	/**	Implemented by the System. 
	Sets the rendering flag to the specified value.
	\param sw TRUE for on; FALSE for off. */
	CoreExport void SetRenderable(BOOL sw);
	/**	Implemented by the System. 
	Pass TRUE to set the generate UVs switch to on; FALSE to set it off.
	\param sw TRUE for on; FALSE for off. */
	CoreExport void SetGenUVs(BOOL sw);
	/**	This method allows you to turn the "Display Render Mesh" switch on or off.
	\param sw TRUE or FALSE to set or unset the "Display Render Mesh" switch. */
	CoreExport void SetDispRenderMesh(BOOL sw);
	/**	This method allows you to turn the "Use Viewport Settings" switch on or
	off.
	\param sw TRUE or FALSE to set or unset the "Use Viewport Settings" switch. */
	CoreExport void SetUseViewport(BOOL sw);
	/**	This method allows you to set the value of the Viewport/Render switch.
	\param sw Set this parameter to GENMESH_VIEWPORT or GENMESH_RENDER. */
	CoreExport void SetViewportOrRenderer(BOOL sw);
	CoreExport virtual Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete);
	/**	Returns information on the rendering mesh. 
	Implemented by the System.
	\param t The time to get the information.
	\param inode The node associated with the mesh.
	\param view Describes properties of the view associated with the render. See Class View.
	\param nverts The number of vertices in the render mesh.
	\param nfaces The number of faces in the render mesh. */
	CoreExport virtual void GetRenderMeshInfo(TimeValue t, INode *inode, View& view, int &nverts, int &nfaces);	// Get info on the rendering mesh
	/**	This method will generate a mesh based on either the viewport or rendering
	parameters for the specified time.
	\param t The time at which to generate the mesh.
	\param option The option can be either GENMESH_VIEWPORT, GENMESH_RENDER, or
	GENMESH_DEFAULT. When using the default definition the mesh generator will
	use whatever is in the Viewport/Render switch in the parameter block.
	\param mesh A pointer to a Mesh object. If this is set to NULL, the mesh will be
	generated and cached, but not returned. */
	CoreExport virtual void GenerateMesh(TimeValue t, int option, Mesh *mesh);
	/**		This method is used by the Summary Info and Object Properties dialogs to inform
	the user how many vertices or CVs are in the object. The method is passed a
	TimeValue and a curve index; if the curve index is \<0, the function should
	return the number of vertices/CVs in the entire shape. Otherwise, it should
	return the number of vertices/CVs in the specified curve.
	\param t The time at which the number of vertices is to be computed.
	\param curve  The curve index. See note above. */
	virtual int NumberOfVertices(TimeValue t, int curve = -1) { return 0; }	// Informational only, curve = -1: total in all curves
	/** Returns the number of polygons in the shape. */
	virtual int NumberOfCurves()=0;                 // Number of curve polygons in the shape
	/** This method is called to determine if the specified curve of the shape is
	closed at the time passed.
	\param t The time to check.
	\param curve The index of the curve to check.
	\return  TRUE if the curve is closed; otherwise FALSE. */
	virtual BOOL CurveClosed(TimeValue t, int curve)=0;     // Returns TRUE if the curve is closed
	/** This method returns a point interpolated on the entire curve. This method
	returns the point but you don't know which segment the point falls on. See
	method InterpPiece3D().
	\param t The time to evaluate.
	\param curve The index of the curve to evaluate.
	\param param The 'distance' along the curve where 0 is the start and 1 is the end.
	\param ptype The parameter type for spline interpolation. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_ptypes_for_shape.html">List of Parameter Types for
	Shape Interpolation</a>.
	\return  The interpolated point on the curve. */
	virtual Point3 InterpCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE)=0;    // Interpolate from 0-1 on a curve
	/** This method returns a tangent vector interpolated on the entire curve. Also
	see method TangentPiece3D().
	\param t The time at which to evaluate the curve.
	\param curve The index of the curve to evaluate.
	\param param The 'distance' along the curve where 0.0 is the start and 1.0 is the end. 
	int ptype=PARAM_SIMPLE\n\n
	The parameter type for spline interpolation. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_ptypes_for_shape.html">List of Parameter Types for
	Shape Interpolation</a>.
	\return  The tangent vector */
	virtual Point3 TangentCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE)=0;   // Get tangent at point on a curve
	/** Returns the length of the specified curve. 
	Note: This method makes no allowance for non-uniform scaling in the object
	transform. To do that, see the following code fragment (os is the
	ObjectState with the shape object and xfm is the NodeTM of
	the shape object node).
	\code
	if (os.obj->SuperClassID() == SHAPE_CLASS_ID)
	{
		ShapeObject *sobj;
		sobj = (ShapeObject *) os.obj;
		int cct = sobj->NumberOfCurves();
		PolyShape workShape;
		sobj->MakePolyShape(ip->GetTime(), workShape);
		workShape.Transform(xfm);
		float len = 0.0f;
		for (int i=0; i<cct; i++)
			len += workShape.lines[i].CurveLength();
	}
	\endcode
	\param t The time at which to compute the length.
	\param curve The index of the curve. */
	virtual float LengthOfCurve(TimeValue t, int curve)=0;  // Get the length of a curve
	/** Returns the number of sub-curves in a curve.
	\param t The time at which to check.
	\param curve The index of the curve. */
	virtual int NumberOfPieces(TimeValue t, int curve)=0;   // Number of sub-curves in a curve
	/** This method returns the interpolated point along the specified sub-curve
	(segment). For example consider a shape that is a single circle with four
	knots. If you called this method with curve=0 and piece=0 and param=0.0
	you'd get back the point at knot 0. If you passed the same parameters
	except param=1.0 you'd get back the point at knot 1.
	\param t The time to evaluate the sub-curve.
	\param curve The curve to evaluate.
	\param piece The segment to evaluate.
	\param param The position along the curve to return where 0.0 is the start and 1.0 is the end.
	\param ptype The parameter type for spline interpolation. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_ptypes_for_shape.html">List of Parameter Types for
	Shape Interpolation</a>.
	\return  The point in world space. */
	virtual Point3 InterpPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE)=0; // Interpolate from 0-1 on a sub-curve
	/** Returns the tangent vector on a sub-curve at the specified 'distance' along
	the curve.
	\param t The time to evaluate the sub-curve.
	\param curve The curve to evaluate.
	\param piece The sub-curve (segment) to evaluate.
	\param param The position along the curve to return where 0 is the start and 1 is the end.
	\param ptype The parameter type for spline interpolation. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_ptypes_for_shape.html">List of Parameter Types for
	Shape Interpolation</a>.
	\return  The tangent vector. */
	virtual Point3 TangentPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE)=0;        // Get tangent on a sub-curve
	/**	This method provides access to the material IDs of the shape. It returns
	the material ID of the specified segment of the specified curve of this
	shape at the time passed. There is a default implementation so there is no
	need to implement this method if the shape does not support material
	IDs. 
	Note: typedef unsigned short MtlID;
	\param t The time to evaluate the sub-curve.
	\param curve The zero based index of the curve to evaluate.
	\param piece The sub-curve (segment) to evaluate. */
	virtual MtlID GetMatID(TimeValue t, int curve, int piece) { return 0; }
	/** This method is called to determine if the shape can be converted to a
	bezier representation.
	\return  TRUE if the shape can turn into a bezier representation; otherwise
	FALSE. */
	virtual BOOL CanMakeBezier() { return FALSE; }                  // Return TRUE if can turn into a bezier representation
	/** Creates the bezier representation of the shape.
	\param t The time to convert.
	\param shape The bezier representation is stored here. */
	virtual void MakeBezier(TimeValue t, BezierShape &shape) {}     // Create the bezier representation
	/** This method is called to prepare the shape for lofting, extrusion, etc.
	This methods looks at the shape organization, and puts together a shape
	hierarchy. This provides information on how the shapes are nested.
	\param t The time to organize the curves.
	\param hier This class provides information about the hierarchy. See Class ShapeHierarchy. */
	virtual ShapeHierarchy &OrganizeCurves(TimeValue t, ShapeHierarchy *hier=NULL)=0;       // Ready for lofting, extrusion, etc.
	/** Create a PolyShape representation with optional fixed steps.
	\param t The time to make the PolyShape.
	\param shape The PolyShape representation is stored here.
	\param steps  The number of steps between knots. Values \>=0 indicates the use of fixed steps:\n\n
	PSHAPE_BUILTIN_STEPS\n
	Use the shape's built-in steps/adaptive settings (default).\n\n
	PSHAPE_ADAPTIVE_STEPS\n
	Force adaptive steps.\n
	\param optimize  If TRUE intermediate steps are removed from linear segments. */
	virtual void MakePolyShape(TimeValue t, PolyShape &shape, int steps = PSHAPE_BUILTIN_STEPS, BOOL optimize = FALSE)=0;   // Create a PolyShape representation with optional fixed steps & optimization
	/** This method generates a mesh capping info for the shape.
	\param t The time to create the cap info.
	\param capInfo The cap info to update.
	\param capType See \ref capTypes.
	\return  Nonzero if the cap info was generated; otherwise zero. */
	virtual int MakeCap(TimeValue t, MeshCapInfo &capInfo, int capType)=0;  // Generate mesh capping info for the shape
	/** This method creates a patch cap info out of the shape. Only implement this
	method if CanMakeBezier() returns TRUE.
	\param t The time to create the cap info.
	\param capInfo The cap info to update.
	\return  Nonzero if the cap info was generated; otherwise zero. */
	virtual int MakeCap(TimeValue t, PatchCapInfo &capInfo) { return 0; }	// Only implement if CanMakeBezier=TRUE -- Gen patch cap info
	/**	This method is called to attach the shape of attachNode to
	thisNode at the specified time. If any endpoints of the curves in
	the shape being attached are within the threshold distance to endpoints of
	an existing curve, and the weld flag is TRUE, they should be welded.
	\param t The time to attach.
	\param thisNode This is the node associated with this shape object.
	\param attachNode The node of the shape to attach.
	\param weldEnds If TRUE the endpoints of the shape should be welded together (based on the
	threshold below). If FALSE no welding is necessary.
	\param weldThreshold If any endpoints of the curves in the shape being attached are within this
	threshold distance to endpoints of an existing curve, and the weld flag is
	TRUE, they should be welded
	\return  Return TRUE if attached; otherwise FALSE. */
	virtual BOOL AttachShape(TimeValue t, INode *thisNode, INode *attachNode, BOOL weldEnds=FALSE, float weldThreshold=0.0f) { return FALSE; }	// Return TRUE if attached
	// UVW Mapping switch access
	virtual BOOL HasUVW() { return GetGenUVs(); }
	virtual BOOL HasUVW (int mapChannel) { return (mapChannel==1) ? HasUVW() : FALSE; }
	virtual void SetGenUVW(BOOL sw) { SetGenUVs(sw); }
	virtual void SetGenUVW (int mapChannel, BOOL sw) { if (mapChannel==1) SetGenUVW (sw); }
	// These handle loading and saving the data in this class. Should be called
	// by derived class BEFORE it loads or saves any chunks
	/** Implemented by the System. 
	This method handles the storage of the data contained within the
	ShapeObject. In order to properly store this information, classes which
	subclass off of ShapeObject need to call this methods before storing their
	information.
	\param isave An interface for saving data. See Class ISave. */
	CoreExport virtual IOResult Save(ISave *isave);
	/** Implemented by the System. 
	This method handles the loading of the data contained within the
	ShapeObject. In order to properly load this information, classes which
	subclass off of ShapeObject need to call this methods before loading their
	information.
	\param iload An interface for loading data. See Class ILoad. */
	CoreExport virtual IOResult Load(ILoad *iload);		

	/** Implemented by the System. 
	This is an implementation of the Object method. It simply returns
	splineShapeClassID. */
	CoreExport virtual Class_ID PreferredCollapseType();
	/**	Implemented by the System. 
	This is an implementation of the Object method. It fills in the
	property fields with the number of vertices and curves in the shape. */
	CoreExport virtual BOOL GetExtendedProperties(TimeValue t, MSTR &prop1Label, MSTR &prop1Data, MSTR &prop2Label, MSTR &prop2Data);
	/**	Implemented by the System. 
	Objects derived from this class which have RescaleWorldUnits methods
	implemented need to call this method. The following example is the
    SplineShape implementation of this method from core.
	\code
	void SplineShape::RescaleWorldUnits(float f)
	{
		if (TestAFlag(A_WORK1))
			return;
	// Call the base class's rescale (this sets the A_WORK1 flag)
		ShapeObject::RescaleWorldUnits(f);
	// Now rescale stuff inside our data structures
		Matrix3 stm = ScaleMatrix(Point3(f, f, f));
		shape.Transform(stm);
	}
	\endcode
	Note that the A_WORK1 flags is tested first to be sure it isn't processing
	the rescale twice. The code then calls ShapeObject::RescaleWorldUnits,
	which sets the A_WORK1 flag and performs the necessary rescale methods for
	all references for the object, and scales the renderable thickness value.
	\param f The parameter to scale. */
	CoreExport virtual void RescaleWorldUnits(float f);
	// New reference support for r4
	/**		This method will notify the Shape Object of changes in values in its parameter
	block. The ShapeObject's parameter block is reference number zero. If
	subclasses implement this method, they should pass any messages referring to
	the ShapeObject's parameter block to it. For example:\n\n
	// If this isn't one of our references, pass it on to the
	ShapeObject...\n\n
	if(hTarget == GetReference(0))\n\n
	return ShapeObject::NotifyRefChanged(\n\n
	changeInt, hTarget, partID, message);\n\n
	This is a vital part of the mechanism; When a parameter in the parameter block
	changes, the ShapeObject must be able to flush its cached mesh which will no
	longer be valid.
	\param changeInt This is the interval of time over which the message is active.
	\param hTarget This is the handle of the reference target the message was sent by. The
	reference maker uses this handle to know specifically which reference target
	sent the message.
	\param partID This contains information specific to the message passed in. Some messages
	don't use the partID at all. See the section \ref Reference_Messages
	for more information about the meaning of the partID for some common messages.
	\param message The msg parameter passed into this method is the specific message which needs
	to be handled. See \ref Reference_Messages.
	\return  The return value from this method is of type RefResult. This is
	usually REF_SUCCEED indicating the message was processed. Sometimes, the
	return value may be REF_STOP. This return value is used to stop the
	message from being propagated to the dependents of the item. */
	CoreExport virtual RefResult ShapeObject::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message );
	/**	This method allows the ShapeObject to return a pointer to its parameter
	block. Any subclasses implementing this method must pass on the call if it
	indicates the ShapeObject's reference. For example:
	\code
	>RefTargetHandle SomeShape::GetReference(int i) {
		If(i == 0) return ShapeObject::GetReference(i);
	}
	\endcode
	\param i The reference handle to retrieve.
	\return  The handle to the Reference Target. */
	CoreExport virtual RefTargetHandle GetReference(int i);
	/**	This method sets the ShapeObject's parameter block pointer. Any subclasses
	implementing this method must pass on the call to the ShapeObject if it
	refers to index 0. For example: 
	void SomeShape::SetReference(int i, RefTargetHandle rtarg) {\n\n
	if(i == 0) ShapeObject::SetReference(i, rtarg);\n\n
	}
	\param i The virtual array index of the reference to store.
	\param rtarg The reference handle to store. */
protected:
	CoreExport virtual void SetReference(int i, RefTargetHandle rtarg);
public:
	/**	This method returns the ShapeObject's animatable pointer. Derived classes
	implementing this method must pass on references to index 0 to the
	ShapeObject. For example:: 
	Animatable* SomeShape::SubAnim(int i) {\n\n
	if(i == 0) return ShapeObject::SubAnim(i);\n\n
	}
	\param i This is the index of the sub-anim to return. */
	CoreExport virtual Animatable* SubAnim(int i);
	/**	This method returns the name of the animatable's name. Derived classes
	implementing this method must pass on references to index 0 to the
	ShapeObject. For example: 
	MSTR SomeShape::SubAnimName(int i) {\n\n
	if(i == 0) return ShapeObject::SubAnimName(i);\n\n
	}
	\param i This is the index of the sub-anim's name to return. */
	CoreExport virtual MSTR SubAnimName(int i);
	/**	This method returns the parameter dimension of the parameter whose index is
	passed.
	\param pbIndex The index of the parameter to return the dimension of.
	\return  Pointer to a ParamDimension. */
	CoreExport ParamDimension *GetParameterDim(int pbIndex);
	/** This method returns the name of the parameter whose index is passed.
	\param pbIndex The index of the parameter to return the dimension of. */
	CoreExport MSTR GetParameterName(int pbIndex);
	/**	This method remaps references at load time so that files saved from
	previous versions of 3ds Max get their references adjusted properly to
	allow for the new ShapeObject reference. If derived classes implement this
	method, they must properly pass on the call to the ShapeObject's code. An
	example from the SplineShape code: 
	int SplineShape::RemapRefOnLoad(int iref) {\n\n
	// Have the ShapeObject do its thing first...\n\n
	iref = ShapeObject::RemapRefOnLoad(iref); \n\n
	 if(loadRefVersion == ES_REF_VER_0)\n\n
	return iref+1;\n\n
	 return iref;\n\n
	}\n\n
	Note that the SplineShape first calls ShapeObject's remapper, then applies
	its remapping operation to the index returned by the ShapeObject code.
	IMPORTANT NOTE: For this remapping to operate properly, the derived class
	MUST call ShapeObject::Save as the first thing in its ::Save method, and
	must call ShapeObject::Load as the first thing in its ::Load method. This
	allows the ShapeObject to determine file versions and the need for
	remapping references.
	\param iref The input index of the reference.
	\return  The output index of the reference. */
	CoreExport virtual int RemapRefOnLoad(int iref);
	
	/** The ShapeObject makes 1 reference; this is where it tells the system. Any
	derived classes implementing this method must take this into account when
	returning the number of references they make. A good idea is to implement
	NumRefs in derived classes as: 
	Int SomeShape::NumRefs() {\n\n
	return myNumRefs + ShapeObject::NumRefs();\n\n
	} */
	virtual int NumRefs() {return 1;}
	virtual int NumSubs() {return 1;}
	/**	This method allows the ShapeObject to create its new "Rendering" rollup. To
	use it, the derived class simply calls it first thing in its own
	BeginEditParams method. An example from the SplineShape code:\n\n
	void SplineShape::BeginEditParams(IObjParam *ip, ULONG flags,Animatable
	*prev )\n\n
	{\n\n
	ShapeObject::BeginEditParams(ip, flags, prev);\n\n
	 // ...\n\n
	}
	\param ip The interface pointer passed to the plug-in.
	\param flags The flags passed along to the plug-in in
	Animatable::BeginEditParams().
	\param prev The pointer passed to the plug-in in Animatable::BeginEditParams(). */
	CoreExport void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
	
	/**	Similarly to BeginEditParams, this method allows the ShapeObject to remove
	its "Rendering" rollup. A derived class simply calls this first thing in
	its own EndEditParams. An example from the SplineShape code: 
	void SplineShape::EndEditParams( IObjParam *ip, ULONG flags,Animatable
	*next )\n\n
	{\n\n
	ShapeObject::EndEditParams(ip, flags, next);\n\n
	// ...\n\n
	}
	\param ip The interface pointer passed to the plug-in.
	\param flags The flags passed along to the plug-in in
	Animatable::BeginEditParams().
	\param prev The pointer passed to the plug-in in Animatable::BeginEditParams().
	*/
	CoreExport void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	
	// Get the validity of the ShapeObject parts
	/**	This method gets the validity interval for the ShapeObject's internal
	parameters only. It DOES NOT include those of the derived classes. So, if
	you called this method on a ShapeObject that was a circle with an animated
	radius, you wouldn't see the effect of the animated radius on the interval
	- All you'd see would be the interval of the ShapeObject's rendering
	parameters. To get the entire ShapeObject's interval, you would call
	ShapeObject::ObjectShapeObjValidity(t).
	\param t The time about which the interval is computed. */
	CoreExport Interval GetShapeObjValidity(TimeValue t);
	// The following displays the shape's generated mesh if necessary
	/**	This method displays the shape's generated mesh if necessary. Objects
	derived from ShapeObject will want to have the ShapeObject code display the
	rendering mesh in the viewport; this method will do that for them. Simply
	set the viewport transform and call this method. An example from the
	SplineShape code: 
	int SplineShape::Display(TimeValue t, INode *inode, ViewExp* vpt, int
	flags)\n\n
	{\n\n
	Eval(t);\n\n
	GraphicsWindow *gw = vpt-\>getGW();\n\n
	gw-\>setTransform(inode-\>GetObjectTM(t));\n\n
	ShapeObject::Display(t, inode, vpt, flags);\n\n
	// ...\n\n
	}\n\n
	If the ShapeObject's "Display Render Mesh" switch is off, it will do
	nothing. Otherwise, it will display the proper mesh as specified by its
	parameter block.
	\param t The time to display the object.
	\param inode The node to display.
	\param vpt An interface pointer that may be used to call methods associated with the viewports.
	\param flags See \ref Display_Flags.
	\return  The return value is not currently used. */
	CoreExport int Display(TimeValue t, INode *inode, ViewExp* vpt, int flags);
	
	// Get the bounding box for the shape, if it's active.  Leaves bbox unchanged if not.
	/**	This method returns a bounding box for the shape, if it's active, if the
	"Display Render Mesh" switch is on. It is necessary to include this box
	when computing the bounding box for a shape, otherwise the viewport display
	will not work properly.
	\param t The time to get the bounding box.
	\param tm 
	The points of ShapeObject are transformed by this matrix prior to the
	bounding box computations. */
	CoreExport virtual Box3 GetBoundingBox(TimeValue t, Matrix3 *tm=NULL);
	
	/**	This method is very important - It causes the ShapeObject to flush its
	cached rendering mesh. Most objects have their own "InvalidateGeomCache"
	methods; simply call this when a shape derived from ShapeObject changes and
	it will ensure that the rendering mesh is regenerated the next time it is
	evaluated. Failure to call this method will result in improper rendering
	mesh updates. */
	CoreExport virtual void InvalidateGeomCache();

	//! \brief Associates data passed in to given id
	/*! Retains and associates passed in AnimProperty data to the id passed in by client.
	\param id An id used to identify the property.
	\param data A pointer (needs to be castable to AnimProperty *) that contains the 
	data to be kept.
	\return returns 1
	*/
	int SetProperty(ULONG id, void *data) 
	{
		AnimProperty *prop = (AnimProperty *)GetProperty(id);
		if (prop) prop = (AnimProperty*)data;
		else 
		{
			prop = (AnimProperty *)data;
			aprops.Append(1, &prop);
		}
		return 1;
	}

	//! \brief Retrieves data associated with id.
	/*! Retrieves the AnimProperty data associated with the passed in id.
	\param id Id that is associated with desired data.
	\return The associated data (castable to AnimProperty *), or NULL if no 
	data is associated with this id. */
	void *GetProperty(ULONG id) 
	{ 	
		for(int i = 0;i<aprops.Count();i++)
			if (aprops[i] && aprops[i]->ID() == id )
				return aprops[i];
		return NULL;
	}

	// Get/Set the UsePhyicalScaleUVs flag.  When true, the UV's
	// assigned to renderable spline are scaled to the size
	// of the object.
	/** Queries whether if real world texture size is used or not. */
	CoreExport BOOL GetUsePhysicalScaleUVs();
	/** Sets whether real world texture size is used or not. */
	CoreExport void SetUsePhysicalScaleUVs(BOOL flag);

private:

	/*! Adjusts spinner to correctly reflect current aspect ratio of viewport or renderer.
	\param hWnd Handle to spinner whose aspect has changed.
	*/
	void SetAspect(HWND hWnd);
	/*!Calculates aspect ratio of viewport or renderer settings.
	\param t time value which calculation is made at
	\param viewport control parameter to indicate whether viewport or renderer is desired
	\return length/width calculated aspect ratio
	*/
	float GetAspect(TimeValue t, BOOL viewport);
	void CheckAspectLock(HWND hWnd);
	/*!An internal callback method for when the aspect ratio of the viewport or renderer changes.
	\param hWnd Handle to spinner whose aspect has changed.
	*/
	void OnAspectChange(HWND hWnd);
	void OnLengthWidthChange(HWND hWnd, BOOL lengthChange);
	/*! Internal method for determining whether viewport or render params should be used.
	\return TRUE if viewport params should be used, FALSE otherwise
	*/
	BOOL UseViewOrRenderParams();
	BOOL CanLockAspect(BOOL vpt);
};

// Set ShapeObject's global Constant Cross-Section angle threshold (angle in radians) --
// Used for renderable shapes.
CoreExport void SetShapeObjectCCSThreshold(float angle);

/*------------------------------------------------------------------- 
WSMObject : This is the helper object for the WSM modifier
---------------------------------------------------------------------*/

/**	This class is a base class used to derived the helper object for a space warp
	modifier (WSM Modifier). 
	World Space Object plug-ins use a Super Class ID of WSM_OBJECT_CLASS_ID.
	\see  Class Object, Class SimpleWSMObject, Class ForceField, Class CollisionObject
 */
class WSMObject : public Object {
public:                                         
	/** Implemented by the System. 
	Returns the super class ID of this plug-in type:
	WSM_OBJECT_CLASS_ID. */
	SClass_ID SuperClassID() { return WSM_OBJECT_CLASS_ID; }                
	/** When the user binds a node to a space warp, a new modifier must be created
	and added to the node's WSM derived object. This method creates the new
	modifier.
	\param node The node of the WSMObject.
	\return  A pointer to the new modifier. */
	virtual Modifier *CreateWSMMod(INode *node)=0;
	
	/**	This is a method of Object. Below is shown the default
	implementation provided by this class. */
	virtual int UsesWireColor() { return FALSE; }   // TRUE if the object color is used for display
	
	/**	This is a method of Object. Below is shown the default
	implementation provided by this class. */
	virtual BOOL NormalAlignVector(TimeValue t,Point3 &pt, Point3 &norm) {pt=Point3(0,0,0);norm=Point3(0,0,-1);return TRUE;}
	
	/**	Returns TRUE if spacewarp or collision object supports Dynamics; otherwise
	FALSE. */
	virtual BOOL SupportsDynamics() { return FALSE; } // TRUE if spacewarp or collision object supports Dynamics

	/**	Returns a pointer to a ForceField. This pointer can be used during
	dynamics calculations, but should not be hung on to after that. For
	example, you shouldn't have the pointer long enough for it to be possible
	for the user to delete the space warp object. When you're done using the
	ForceField call its DeleteThis() method. This method may be
	called several times on the same space warp object with different
	INode* if it is instanced.
	\param node This is the space warp object's node. */
	virtual ForceField *GetForceField(INode *node) {return NULL;}
		
	/**	This method returns the collision object for the WSM. This works just like
	GetForceField() documented above.
	\param node This is the space warp object's node. */
	virtual CollisionObject *GetCollisionObject(INode *node) {return NULL;}		

	CoreExport void* GetInterface(ULONG id);
};


//---------------------------------------------------------------------------------


class ControlMatrix3;

// Used with EnumModContexts()

/** Callback object used with Modifier::EnumModContexts(). The proc()
	method is called by the system.  
	\see  Class ModContext
	*/
class ModContextEnumProc: public MaxHeapOperators {
public:
	/** Destructor */
	virtual ~ModContextEnumProc() {;}
	/** This is called by Modifier::EnumModContexts().
	\param mc The ModContext.
	\return  Return FALSE to stop, TRUE to continue. 
	\sa  Modifier::EnumModContexts(),
	Modifier. */
	virtual BOOL proc(ModContext *mc)=0;  // Return FALSE to stop, TRUE to continue.
};

/*------------------------------------------------------------------- 
Modifier: these are the ObjectSpace and World Space modifiers: They are 
subclassed off of BaseObject so that they can put up a graphical 
representation in the viewport. 
---------------------------------------------------------------------*/

/** This is the class from which you may derive Object Space and Space Warp (World
	Space) Modifier plug-ins. This class is subclassed off of BaseObject so
	the modifier can put up a graphical representation in the viewport to use as a
	gizmo.
	\par Method Groups:
	See <a href="class_modifier_groups.html">Method Groups for Class Modifier</a>. 
	\see  Class BaseObject, Class ObjectState, Class ModContext, Class ModContextEnumProc, 
	\see Class Interval, Class ISave,  Class ILoad,  Class Class_ID*/
class Modifier : public BaseObject {
	friend class ModNameRestore;
	MSTR modName;
public:

	CoreExport Modifier();
	CoreExport ~Modifier();

	/** Implemented by the System. 
	Returns the name of the modifier. */
	CoreExport virtual MSTR GetName();
	/** Implemented by the System. 
	Sets the name of the modifier to the name passed.
	\param n Specifies the name to set. */
	CoreExport virtual void SetName(MSTR n);

	SClass_ID SuperClassID() { return OSM_CLASS_ID; }

	// Disables all mod apps that reference this modifier _and_ have a select
	// anim flag turned on.
	/** This method is used internally. */
	void DisableModApps() { NotifyDependents(FOREVER,PART_OBJ,REFMSG_DISABLE); }
	/** This method is used internally. */
	void EnableModApps() {  NotifyDependents(FOREVER,PART_OBJ,REFMSG_ENABLE); }

	// This disables or enables the mod. All mod apps referencing will be affected.
	/** Implemented by the System. 
	This disables the modifier in the history browser (modifier stack). */
	void DisableMod() { 
		SetAFlag(A_MOD_DISABLED);
		NotifyDependents(FOREVER, (PartID)(PART_ALL|PART_OBJECT_TYPE),REFMSG_CHANGE); 
	}
	/** Implemented by the System. 
	This enables the modifier in the history browser (modifier stack). */
	void EnableMod() {      
		ClearAFlag(A_MOD_DISABLED);
		NotifyDependents(FOREVER, (PartID)(PART_ALL|PART_OBJECT_TYPE), REFMSG_CHANGE); 
	}
	/** Implemented by the System. 
	This returns the status (enabled or disabled) of the modifier in the
	history browser.
	\return  Nonzero if enabled; otherwise 0. 
	  */
	int IsEnabled() { return !TestAFlag(A_MOD_DISABLED); }

	// Same as above but for viewports only
	/** Implemented by the System. 
	Disables the modifier in the viewports (it remains active in the renderer
	unless DisableMod() above is used). */
	void DisableModInViews() { 
		SetAFlag(A_MOD_DISABLED_INVIEWS);
		NotifyDependents(FOREVER, (PartID)(PART_ALL|PART_OBJECT_TYPE), REFMSG_CHANGE); 
	}
	/** Implemented by the System. 
	Enables the modifier in the viewports. */
	void EnableModInViews() {      
		ClearAFlag(A_MOD_DISABLED_INVIEWS);
		NotifyDependents(FOREVER,(PartID)(PART_ALL|PART_OBJECT_TYPE), REFMSG_CHANGE); 
	}
	/** Implemented by the System. 
	Returns nonzero if the modifier is enabled in the view ports; otherwise
	zero. */
	int IsEnabledInViews() { return !TestAFlag(A_MOD_DISABLED_INVIEWS); }

	// Same as above but for renderer only
	/** Implemented by the System. 
	This turns off the modifier in the renderer 
	  */
	void DisableModInRender() { 
		SetAFlag(A_MOD_DISABLED_INRENDER);
		NotifyDependents(FOREVER,(PartID)(PART_ALL|PART_OBJECT_TYPE), REFMSG_CHANGE); 
	}
	/** Implemented by the System. 
	This turns on the modifier in the renderer 
	  */
	void EnableModInRender() {      
		ClearAFlag(A_MOD_DISABLED_INRENDER);
		NotifyDependents(FOREVER,(PartID)(PART_ALL|PART_OBJECT_TYPE), REFMSG_CHANGE); 
	}
	/** Implemented by the System. 
	This returns the status (enabled or disabled) of the modifier in the
	renderer.
	\return  Nonzero if enabled; otherwise 0. 
	  */
	int IsEnabledInRender() { return !TestAFlag(A_MOD_DISABLED_INRENDER); }

	/** This method returns the validity interval of a modifier. It is simply the
	combination of the validity of all the modifier's parameters. It's used to
	determine when to cache in the pipeline, but is not directly responsible
	for determining when ModifyObject() is called. ModifyObject()
	is called when the pipeline needs to be evaluated either because someone
	sent a \ref REFMSG_CHANGE message or the validity of the object does not
	include the current time.
	\par If a modifier is not animated it's OK to simply return FOREVER from
	this method. In the case where the modifier changes because a user changes
	a non-animated control in the user interface (for instance a check box),
	you can cause reevaluation by notifying your dependents of the change, i.e.:
	\code
	NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
	\endcode
	\param t The time to calculate the Interval.
	\sa  Advanced Topics on
	<a href="ms-its:3dsmaxsdk.chm::/start_conceptual_overview.html#heading_08">Intervals</a>.
	*/
	CoreExport virtual Interval LocalValidity(TimeValue t);
	/** These are channels that the modifier needs in order to perform its
	modification. This should at least include the channels specified in
	ChannelsChanged() but may include more. 
	Note that ChannelsUsed() is called many times but the channels
	returned should not change on the fly.
	\return  The channels required. See \ref objectChannels.
	\par Sample Code:
	\code
	{ return GEOM_CHANNEL|TOPO_CHANNEL; }
	\endcode */
	virtual ChannelMask ChannelsUsed()=0;
	/** These are the channels that the modifier actually modifies. Note that
	ChannelsChanged() is called many times but the channels returned
	should not change on the fly.
	\return  The channels that are changed. See \ref objectChannels */
	virtual ChannelMask ChannelsChanged()=0;
	// this is used to invalidate cache's in Edit Modifiers:
	/** This method is called when an item in the modifier stack before this
	modifier sends a \ref REFMSG_CHANGE message via
	NotifyDependents(). 
	Consider the following example: Assume the modifier stack contains a Sphere
	Object, then a Bend, then a Edit Mesh. The Edit Mesh modifier does not have
	a reference to the Bend or the Sphere because it does not officially depend
	on these items. However it does depend on them in a certain sense, because
	it modifies the data that these items produce. So, if they change it may
	affect the modifier. A modifier may build a cache based on its input
	object. The modifier needs a way to know when to discard this cache because
	the input object has changed. Whenever one of the items before this
	modifier in the stack sends out a \ref REFMSG_CHANGE message via
	NotifyDependents() to indicate it has changed this method is called.
	The modifier may respond in a way appropriate to it, for example by
	discarding its cache of the input object. 
	It is not legal, to issue a NotifyDependent()'s in the
	NotifyInputChanged() method of a modifier, without checking for
	reentrancy. Imagine, you have an instanced modifier applied to the same
	object in the stack. Sending a refmsg from the NotifyInputChanged
	method will casue an endles loop. Simply putting a guard in, that checks
	for reentrancy should get rid of the problem.
	\param changeInt This is the interval from the message. It is reserved for future use - now
	it will always be FOREVER.
	\param partID This is the partID from the message.
	\param message This is the message sent.
	\param mc The ModContext for the pipeline that changed. If a modifier is applied to
	multiple objects, then there are ModApps in each pipeline that it is
	applied to. These ModApps are pointing to the same modifier. Consider the
	following example: Say you apply a Bend modifier to a Sphere, a Cylinder
	and a Box object. There are three ModApps but only one Bend modifier. Then
	you go to the Sphere and adjust its Radius. This will cause
	NotifyInputChanged() to be called on the Bend because the Bend's
	input changed. However only one of its inputs changed - only the Sphere
	changed and not the Cylinder or the Box. Therefore
	NotifyInputChanged() will be called once, and the ModContext passed
	in will be for the Sphere's changed pipeline. It is possible that all three
	objects could change at the same time. If an instanced float controller was
	assigned to the radius, width, and height - one parameter for each object -
	then the controller was adjusted in the function curve editor, all three
	items would change. In this case NotifyInputChanged() would be
	called three times on the Bend. Once for each pipeline, once with each
	ModContext. */
	virtual void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) {}

	// This method indicates if the modifier changes the selection type channel or not.
	// If a modifier wants to change dynamically if it changes the subobj sel type
	// or not, it can overwrite this method.
	// ChannelsChanged() however can not be dynamically implemented.
	/**	If a modifier want to make it possible to sitch dynamically between
	changing the selection type that flows up the stack, or leaving it like it
	is, it can overwrite this. The default implementation indicates that it
	changes the selection type, if the SUBSEL_TYPE_CHANNEL is part of
	ChannelsChanged(). Note that ChannelsChanged() can not
	dynamically changed for various reasons. */
	virtual bool ChangesSelType(){ return ChannelsChanged()&SUBSEL_TYPE_CHANNEL ? true : false;}

	// These call ChannelsUsed/Changed() but also OR in GFX_DATA_CHANNEL as appropriate.
	/** Returns the same value as ChannelsUsed() above except
	GFX_DATA_CHANNEL will be ORed in if the TOPO_CHANNEL or the
	TEXMAP_CHANNEL are being used. */
	CoreExport ChannelMask TotalChannelsUsed();
	/** Returns the same value as ChannelsChanged() above
	except GFX_DATA_CHANNEL will be ORed in if the TOPO_CHANNEL,
	the TEXMAP_CHANNEL , or the VERTCOLOR_CHANNEL are being
	changed. */
	CoreExport ChannelMask TotalChannelsChanged();

	// This is the method that is called when the modifier is needed to 
	// apply its effect to the object. Note that the INode* is always NULL
	// for object space modifiers.
	/** This is the method that actually modifies the input object. This method is
	responsible for altering the object and then updating the validity interval
	of the object to reflect the validity of the modifier.
	\param t The time at which the modification is being done.
	\param mc A reference to the ModContext.
	\param os The object state flowing through the pipeline. This contains a pointer to
	the object to modify.
	\param node The node the modifier is applied to. This parameter is always NULL for
	Object Space Modifiers and non-NULL for World Space Modifiers (Space
	Warps). This is because a given WSM is only applied to a single node at a
	time whereas an OSM may be applied to several nodes. This may be used for
	example by particle system space warps to get the transformation matrix of
	the node at various times.
	\sa The topic on <a href="ms-its:3dsmaxsdk.chm::/mods_modifiers.html">Modifiers</a> in the
	Programmers Guide. */
	virtual void ModifyObject(TimeValue t, ModContext &mc, ObjectState* os, INode *node)=0;

	// this should return FALSE for things like edit modifiers
	/** This method is no longer used. */
	virtual int NeedUseSubselButton() { return 1; }

	// Modifiers that place a dependency on topology should return TRUE
	// for this method. An example would be a modifier that stores a selection
	// set base on vertex indices.
	/** Modifiers that place a dependency on topology should return TRUE for this
	method. An example would be a modifier that stores a selection set base on
	vertex indices. This modifier depends on the indices being intact for it to
	operate correctly.
	\param mc Reference to the ModContext.  
	\return  TRUE if the modifier depends on topology; otherwise FALSE. */
	virtual BOOL DependOnTopology(ModContext &mc) {return FALSE;}

	// this can return:
	//   DEFORM_OBJ_CLASS_ID -- not really a class, but so what
	//   MAPPABLE_OBJ_CLASS_ID -- ditto
	//   TRIOBJ_CLASS_ID
	//   BEZIER_PATCH_OBJ_CLASS_ID
	/** This is the type of object that the modifier knows how to modify. Simple
		modifiers that just modify points of an object can operate on generic
		'Deformable' objects. Deformable objects are any type of object that has
		points. A modifier could also work on a particular type of object such as a
		TriObject or PatchObject.
		\return  The Class_ID of the item. You can request any Class_ID for your
		input type. For example, Class_ID(OMNI_LIGHT_CLASS_ID, 0). See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_class_ids.html">List of Class_IDs</a>. */
	virtual Class_ID InputType()=0;

	virtual void ForceNotify(Interval& i) 
	{NotifyDependents(i,ChannelsChanged(),REFMSG_CHANGE );}

	/** When a 3ds Max file is being saved, this method is called so that the
	modifier can save the localData structure that is hung off each ModContext.
	If the modifier doesn't store any data in the ModContext it can ignore this
	method.
	\param isave You may use this pointer to call methods of ISave to write data.
	\param ld Pointer to the LocalModData for the modifier.
	\return  One of the following values: IO_OK, IO_ERROR. */
	virtual IOResult SaveLocalData(ISave *isave, LocalModData *ld) { return IO_OK; }  
	/** When a 3ds Max file is being loaded, this method is called so that the
	modifier can load the LocalModData structure that is hung off each
	ModContext. If the modifier doesn't store any data in the ModContext it can
	ignore this method.
	\param iload You may use this pointer to call methods of ILoad to read data.
	\param pld A pointer to a pointer in the ModContext. The modifier must set this
	pointer to point at a new LocalModData derived class.
	\return  One of the following values: IO_OK, IO_ERROR. */
	virtual IOResult LoadLocalData(ILoad *iload, LocalModData **pld) { return IO_OK; }  

	// These handle loading and saving the modifier name. Should be called
	// by derived class BEFORE it loads or saves any chunks
	/** Implemented by the System. 
	This method handles saving the modifier name. This method should be called
	by the derived class BEFORE it saves any chunks. See the sample code below.
	\param isave You may use this pointer to call methods of ISave to write data.
	\return  One of the following values: IO_OK, IO_ERROR.
	\par Sample Code:
	\code
	IOResult DispMod::Save(ISave *isave)
	{
	// First save the modifier name by calling the base class version.
		Modifier::Save(isave);
	// Then save this modifiers data.
		isave->BeginChunk(BMIO_CHUNK);
		bi.Save(isave);
		isave->EndChunk();
		return IO_OK;
	}
	\endcode  */
	CoreExport IOResult Save(ISave *isave);
	/** Implemented by the System.
	\param iload You may use this pointer to call methods of ILoad to read data.
	\return  One of the following values: IO_OK, IO_ERROR. */
	CoreExport IOResult Load(ILoad *iload);

	// This will call proc->proc once for each application of the modifier.
	/** Implemented by the System. 
	This method will call the callback object proc method once for each
	application of the modifier.
	\param proc The callback object whose proc method is called.
	\sa  Class ModContextEnumProc. */
	CoreExport void EnumModContexts(ModContextEnumProc *proc);

	// This method will return the IDerivedObject and index of this modifier 
	// for a given modifier context.
	/**	This method will retrieve the IDerivedObject and index of this modifier for
	a given modifier context.
	\param mc Points to the ModContext for the modifier.
	\param derObj A pointer to the IDerivedObject is returned here.
	\param modIndex The zero based index of the modifier in the derived object is returned here. */
	CoreExport void GetIDerivedObject(ModContext *mc, IDerivedObject *&derObj, int &modIndex);

	// In case the modifier changes the object type (basically the os->obj pointer in ModifyObject)
	// *and* changes the ExtensionChannel, it has to overwrite this method and copy only the channels
	// that it doesn't modify/added already to the new object.
	/**	In case the modifier changes the object type (basically the os-\>obj
	pointer in ModifyObject) *and* changes the ExtensionChannel, it has to
	overwrite this method and copy only the channels that it doesn't
	modify/added already to the new object.
	\param fromObj 
	\param toObj */
	CoreExport virtual void CopyAdditionalChannels(Object *fromObj, Object *toObj) { toObj->CopyAdditionalChannels(fromObj);}

	// Animatable Overides...
	CoreExport SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
	CoreExport MSTR SvGetName(IGraphObjectManager *gom, IGraphNode *gNode, bool isBeingEdited);
	CoreExport bool SvCanSetName(IGraphObjectManager *gom, IGraphNode *gNode);
	CoreExport bool SvSetName(IGraphObjectManager *gom, IGraphNode *gNode, MSTR &name);
	CoreExport bool SvHandleDoubleClick(IGraphObjectManager *gom, IGraphNode *gNode);
	CoreExport COLORREF SvHighlightColor(IGraphObjectManager *gom, IGraphNode *gNode);
	CoreExport bool SvIsSelected(IGraphObjectManager *gom, IGraphNode *gNode);
	CoreExport MultiSelectCallback* SvGetMultiSelectCallback(IGraphObjectManager *gom, IGraphNode *gNode);
	CoreExport bool SvCanSelect(IGraphObjectManager *gom, IGraphNode *gNode);
	CoreExport bool SvCanInitiateLink(IGraphObjectManager *gom, IGraphNode *gNode);
	CoreExport bool SvCanConcludeLink(IGraphObjectManager *gom, IGraphNode *gNode, IGraphNode *gNodeChild);
	CoreExport bool SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild);
	CoreExport bool SvCanRemoveThis(IGraphObjectManager *gom, IGraphNode *gNode);
	CoreExport bool SvRemoveThis(IGraphObjectManager *gom, IGraphNode *gNode);

private:
};

/** This is a base class developers creating object space modifiers may derives
	their plug-ins from. It simply provides a default implementation of
	SuperClassID().  
	 \see  Class Modifier, Class SimpleMod.*/
class OSModifier : public Modifier {
public:
	/** Implemented by the System. 
	Returns the Super Class ID of this plug-in type: OSM_CLASS_ID. */
	SClass_ID SuperClassID() { return OSM_CLASS_ID; }
};

/** This is a base class for creating world space modifiers. It simply provides a
	default implementation of SuperClassID(). 
	World Space Modifier plug-ins use a Super Class ID of WSM_CLASS_ID. <br>
	\see  Class Modifier, SimpleWSMMod.
	*/
class WSModifier : public Modifier {
public:
	/** Implemented by the System. 
	Returns the Super Class ID of this plug-in type: WSM_CLASS_ID. */
	SClass_ID SuperClassID() { return WSM_CLASS_ID; }
};

CoreExport void MakeHitRegion(HitRegion& hr, int type, int crossing, int epsi, IPoint2 *p);

/** This is a callback used to draw a series of lines. A developer would implement
a class derived from this one that has a GraphicsWindow pointer within
it. This is the base class used by DrawLineProc and BoxLineProc.
Below is the code from DrawLineProc showing how this is used: 
class DrawLineProc:public PolyLineProc {\n\n
GraphicsWindow *gw;\n\n
\n\n
DrawLineProc() { gw = NULL; }\n\n
DrawLineProc(GraphicsWindow *g) { gw = g; }\n\n
int proc(Point3 *p, int n)\n\n
{ gw-\>polyline(n, p, NULL, NULL, 0, NULL); return 0; }\n\n
void SetLineColor(float r, float g, float b)\n\n
{gw-\>setColor(LINE_COLOR,r,g,b);}\n\n
};\n\n
Typically a developer would not have to use this class, but would rather use
Class DrawLineProc, and
Class BoxLineProc.  
\see  Class DrawLineProc, Class BoxLineProc, Class GraphicsWindow.
*/
class PolyLineProc: public MaxHeapOperators {
public:
	/** Destructor */
	virtual ~PolyLineProc() {;}
	/** Draws a polyline between the specified points.
	\param p Array of vertices of the polyline.
	\param n Number of vertices in the polyline. */
	virtual int proc(Point3 *p, int n)=0;
	/** Sets the color used to draw the polyline.
	\param r Red component in the range 0.0 to 1.0
	\param g Green component in the range 0.0 to 1.0
	\param b Blue component in the range 0.0 to 1.0 */
	virtual void SetLineColor(float r, float g, float b) {}
	virtual void SetLineColor(Point3 c) {}
	virtual void Marker(Point3 *p,MarkerType type) {}
};
#pragma warning(pop)

/** This class provides a simplified way to draw a connected series of lines to the
GraphicsWindow passed to the class constructor.  
\see  Class PolyLineProc, Class GraphicsWindow
*/
class DrawLineProc : public PolyLineProc {
	GraphicsWindow *gw;
public:
	/** Constructor. The graphics window pointer is set to NULL. */
	DrawLineProc() { gw = NULL; }
	/** Constructor. The graphics window pointer is set to g. */
	DrawLineProc(GraphicsWindow *g) { gw = g; }
	/** Implemented by the System. 
	Draws a connected series of lines between the points to the GraphicsWindow
	passed to the class constructor.
	\param p Array of vertices of the polyline.
	\param n Number of vertices in the polyline. */
	int proc(Point3 *p, int n) { gw->polyline(n, p, NULL, NULL, 0, NULL); return 0; }
	/** Implemented by the System. 
	Sets the color used to draw the lines.
	\param r Red component in the range 0.0 to 1.0
	\param g Green component in the range 0.0 to 1.0
	\param b Blue component in the range 0.0 to 1.0 */
	void SetLineColor(float r, float g, float b) {gw->setColor(LINE_COLOR,r,g,b);}
	void SetLineColor(Point3 c) {gw->setColor(LINE_COLOR,c);}
	void Marker(Point3 *p,MarkerType type) {gw->marker(p,type);}
};


/** This class provides a bounding box and a matrix. It will compute a bounding box
	based on a set of points passed to it after these points have been transformed
	by the matrix. All methods of this class are implemented by the system.  
	\see  Class PolyLineProc
	*/
class BoxLineProc : public PolyLineProc {
	Box3 box;
	Matrix3 *tm;
public:
	/** Constructor. The bounding box is set to empty. */
	BoxLineProc() { box.Init();}
	/** Constructor. The bounding box is set to empty and the matrix
	is initialized to the matrix passed. */
	BoxLineProc(Matrix3* m) { tm = m;  box.Init(); }
	/** Returns the computed box. */
	Box3& Box() { return box; }
	/** Implemented by the System. 
	This takes the points passed and expands the bounding box to include the
	points after they have been transformed by the matrix.
	\param p The points to include
	\param n Number of points.
	\return  Always returns zero. */
	CoreExport int proc(Point3 *p, int n);
	CoreExport void Marker(Point3 *p,MarkerType type);
};


// Apply the PolyLineProc to each edge (represented by an array of Point3's) of the box
// after passing it through the Deformer def.
CoreExport void DoModifiedBox(Box3& box, Deformer &def, PolyLineProc& lp);
CoreExport void DoModifiedLimit(Box3& box, float z, int axis, Deformer &def, PolyLineProc& lp);
CoreExport void DrawCenterMark(PolyLineProc& lp, Box3& box );

// Some functions to draw mapping icons
CoreExport void DoSphericalMapIcon(BOOL sel,float radius, PolyLineProc& lp);
CoreExport void DoCylindricalMapIcon(BOOL sel,float radius, float height, PolyLineProc& lp);
CoreExport void DoPlanarMapIcon(BOOL sel,float width, float length, PolyLineProc& lp);

//---------------------------------------------------------------------
// Data structures for keeping log of hits during sub-object hit-testing.
//---------------------------------------------------------------------

class HitLog;

/** This class provides a data structure used during sub-object hit-testing.
\par Data Members:
INode *nodeRef;\n\n
Points the node that was hit.\n\n
ModContext *modContext;\n\n
Points to the ModContext of the modifier.\n\n
DWORD distance;\n\n
The 'distance' of the hit. To classify as a hit, the sub-object component must
be within some threshold distance of the mouse. This distance is recorded in
the hit record so that the closest of all the hits can be identified. What the
distance actually represents depends on the rendering level of the viewport.
For wireframe modes, it refers to the distance in the screen XY plane from the
mouse to the sub-object component. In a shaded mode, it refers to the Z depth
of the sub-object component. In both cases, smaller values indicate that the
sub-object component is 'closer' to the mouse cursor.\n\n
ulong hitInfo;\n\n
A general unsigned long value. Most modifiers will just need this to identity
the sub-object element. The edit mesh modifier uses the value to store the
index of the vertex or face that was hit for example.\n\n
HitData *hitData;\n\n
In case 4 bytes is not enough space to identity the sub-object element, this
pointer is available. To use this, a developer would define a class derived
from HitData that would contain the necessary data. The HitData class has one
member function, a virtual destructor, so the derived class can be properly
deleted when the HitRecord instance is deleted.  
\see  Class HitLog, Class HitData
*/
class HitRecord: public MaxHeapOperators {
	friend class HitLog;    
	HitRecord *next;
public:         
	INode *nodeRef;
	ModContext *modContext;
	DWORD distance;
	ulong hitInfo;
	HitData *hitData;
	/** Constructor. The following initialization is performed:
	next = NULL; modContext = NULL; distance = 0; hitInfo = 0; hitData =
	NULL; */
	HitRecord() { next = NULL; modContext = NULL; distance = 0; hitInfo = 0; hitData = NULL;}
	/** Constructor. The data members are initialized to the data passed.
	*/
	HitRecord(INode *nr, ModContext *mc, DWORD d, ulong inf, HitData *hitdat) {
		next = NULL;
		nodeRef = nr; modContext = mc; distance = d; hitInfo = inf; hitData = hitdat;
	}               
	/** Constructor. The data members are initialized to the data passed.
	*/
	HitRecord(HitRecord *n,INode *nr, ModContext *mc, DWORD d, ulong inf, HitData *hitdat) {
		next = n;
		nodeRef = nr; modContext = mc; distance = d; hitInfo = inf; hitData = hitdat;
	}               
	/** Implemented by the System.\n\n
	Each HitRecord maintains a pointer to another HitRecord. This
	method returns the next hit record. */
	HitRecord *     Next() { return next; }
	/** Implemented by the System.\n\n
	Destructor. If HitData has been allocated, it is deleted as well. */
	CoreExport ~HitRecord();
};                                      

/** This class provides a data structure for keeping a log of hits during
sub-object hit-testing. It provides a list of HitRecords that may be
added to and cleared. A developer may also request the 'closest' hit record in
the list. 
\see  Class HitRecord, Class HitData*/
class HitLog: public MaxHeapOperators {
	HitRecord *first;
	int hitIndex;
	bool hitIndexReady;			// CAL-07/10/03: hitIndex is ready to be increased.
public:
	/** Constructor. The list of HitRecords is set to NULL. */
	HitLog()  { first = NULL; hitIndex = 0; hitIndexReady = false; }
	/** Destructor. Clears the hit log. */
	~HitLog() { Clear(); }
	/** Clears the log of hits by deleting the list of HitRecords. */
	CoreExport void Clear();
	CoreExport void ClearHitIndex(bool ready = false)		{ hitIndex = 0; hitIndexReady = ready; }
	CoreExport void IncrHitIndex()		{ if (hitIndexReady) hitIndex++; else hitIndexReady = true; }
	/** Implemented by the System. 
	Returns the first HitRecord. */
	HitRecord* First() { return first; }
	/** Implemented by the System. 
	Returns the HitRecord that was 'closest' to the mouse position when
	hit testing was performed. This is the HitRecord with the minimum
	distance. */
	CoreExport HitRecord* ClosestHit();
	/** Implemented by the System. 
	This method is called to log a hit. It creates a new HitRecord
	object using the data passed and adds it to the hit log.
	\param nr The node that was hit.
	\param mc The ModContext of the modifier.
	\param dist The 'distance' of the hit. What the distance actually represents depends on
	the rendering level of the viewport. For wireframe modes, it refers to the
	distance in the screen XY plane from the mouse to the sub-object component.
	In a shaded mode, it refers to the Z depth of the sub-object component. In
	both cases, smaller values indicate that the sub-object component is
	'closer' to the mouse cursor.
	\param info Identifies the sub-object component that was hit.
	\param hitdat If the info field is insufficient to indicate the sub-object component that
	was hit, pass an instance of the HitData class that contains the
	needed information. */
	CoreExport void LogHit(INode *nr, ModContext *mc, DWORD dist, ulong info, HitData *hitdat = NULL);
};


// Creates a new empty derived object, sets it to point at the given
// object and returns a pointer to the derived object.
/** Creates a new empty derived object, sets it to point at the given
object and returns a pointer to the derived object.
\param obj object reference of the derived object will point at this object.
\return  A pointer to the derived object. */
CoreExport Object *MakeObjectDerivedObject(Object *obj);

/// \defgroup Category_strings_for_space_warp_objects Category strings for space warp objects
//@{
#define SPACEWARP_CAT_GEOMDEF		1
#define SPACEWARP_CAT_MODBASED		2
#define SPACEWARP_CAT_PARTICLE		3
//@}

CoreExport MCHAR *GetSpaceWarpCatString(int id);

// ObjectConverter - allows users to register methods to (for example)
// allow Max to convert TriObjects directly into their plug-in object type.

/** This virtual class is implemented by applications that want to supply a
conversion method from one object type to another. A typical use would be to
support conversion of a native 3ds Max type (such as TriObject) to a plug-in's
object type. There are a set of global functions that can be used with this
class. These are documented at the bottom of the topic. One of these is called
to register the ObjectConverter with the system. 
Note that the registered object converters are called from the methods: 
Object::CanConvertToType and Object::ConvertToType.\n\n
So for individual objects to support these, they'll need to add the line\n\n
if (Object::CanConvertToType(obtype)) return 1;\n\n
to the end of their CanConvertToType methods and\n\n
if (Object::CanConvertToType(obtype))\n\n
return Object::ConvertToType(t, obtype);\n\n
to the end of their ConvertToType methods.  
\see Class InterfaceServer, Class Class_ID, Class Object, Class TriObject, Class PatchObject
*/
class ObjectConverter : public InterfaceServer {
public:
	/** This method returns the Class ID of the object this converter
	converts from. */
	virtual Class_ID ConvertsFrom ()=0;
	/** This method returns the Class ID of the object this converter
	converts to. */
	virtual Class_ID ConvertsTo ()=0;
	// NOTE: There's a serious problem in that this method does not accept a TimeValue.
	// See below for details.
	/** This method actually performs the conversion, creating and
	returning a new object with the class ID specified in ConvertsTo().
	\param from Points to the object to convert. */
	virtual Object *Convert (Object *from)=0;
	/** This should delete the ObjectConverter if necessary.  */
	virtual void DeleteThis () { }
};

// There was a problem in the above ObjectConverter class in that its Convert
// method doesn't accept a time parameter.  It's too late to change that, so we've
// implemented the following interface to supply the correct method.  Users'
// ObjectConverters should subclass off of both ObjectConverter and
// ITimeBasedConverter.  They should implement the GetInterface method as
// follows:
//BaseInterface *MyConverter::GetInterface (Interface_ID id) {
//if (id == INTERFACE_TIME_BASED_CONVERTER) return (ITimeBasedConverter *)this;
//return ObjectConverter::GetInterface (id);
//}
// They should then implement ConvertWithTime properly, and use Convert only
// to call ConvertWithTime with a time of GetCOREInterface()->GetTime().
// Convert should not be called (and won't be called by the system if this
// interface is properly set up).

#define INTERFACE_TIME_BASED_CONVERTER Interface_ID(0x1e064bad,0x716643db)

class ITimeBasedConverter : public BaseInterface {
public:
	Interface_ID GetID () { return INTERFACE_TIME_BASED_CONVERTER; }
	// This is the method that should be used to do the right conversion:
	virtual Object *ConvertWithTime (TimeValue t, Object *from)=0;
};

/** Registers an object converter with the system.
\param conv to the ObjectConverter instance to register.
\return  Returns true if the converter could be added; false if not. */
CoreExport bool RegisterObjectConverter (ObjectConverter *conv);
/** Indicates if a TriObject can convert to the specified class ID. 
Note: this actually checks if an Editable Mesh object can convert to the
specified type, since an Editable Mesh is what you get when you call
CreateNewTriObject ().\n\n
This method may be used in an object's CanConvertToType() and
ConvertToType() methods. If your object supports conversion to a
TriObject, but doesn't support conversion to the given class ID, you can use
this method to find out if TriObjects can be used as an "intermediary". If so,
you can construct a temporary TriObject, convert it to the given class ID, and
call the temporary TriObject's DeleteThis() method.
\param to Class ID to convert to.
\return  Nonzero if the TriObject can be converted to the specified objec type;
otherwise zero. */
CoreExport int CanConvertTriObject (Class_ID to);
/** Indicates if a PatchObject can convert to the specified class ID.
\param to Class ID to convert to.
\return  Nonzero if the PatchObject can be converted to the specified objec
type; otherwise zero. */
CoreExport int CanConvertPatchObject (Class_ID to);
/** Indicates if a SplineObject can convert to the specified class ID.
\param to Class ID to convert to.
\return  Nonzero if the SplineObject can be converted to the specified objec
type; otherwise zero. */
CoreExport int CanConvertSplineShape (Class_ID to);
/** This method will register the object passed as the editable tri object.
\param triob object to register as the editable tri object. */
CoreExport void RegisterStaticEditTri (Object *triob);
/** Registers a class ID of an object that the user can collapse other objects to.
The type will only appear if the current object returns nonzero from
CanConvertTo(cid).
\param cid class ID the object will collapse to.
\param name name of the collapse-to object type (such as "Editable Poly").
\param canSelfConvert whether an object should be allowed to collapse to itself. (false is
generally preferred, so that the collapse-to menu only has relevant entries.)
*/
CoreExport void RegisterCollapseType (Class_ID cid, MSTR name, bool canSelfConvert=false);

// Developers have to return a class derived from this class with implementations for 
// all memberfunctions when implementing subobjects for obejcts and modifiers (see GetSubObjType())


/** Developers have to return a class derived from this class with implementations
for all the methods when implementing sub-objects for objects and modifiers
(see BaseObject::GetSubObjType()).  
\see  Class InterfaceServer, Class BaseObject, Class MaxIcon, Class GenSubObjType
*/
class ISubObjType : public InterfaceServer
{
public:
	/** Returns a pointer to the icon for this sub-object type. This
	icon appears in the stack view beside the name. */
	virtual MaxIcon *GetIcon()=0;
	/** Returns the name of this sub-object type that appears in the
	stack view. */
	virtual MCHAR *GetName()=0;
};

// Generic implementation for subobject types. This SubObjectType will either use the 
// SubObjectIcons_16i.bmp and  SubObjectIcons_16a.bmp bitmaps in the UI directory 
// (for the GenSubObjType(int idx) constructor), or any other bmp file that is specified
// in the GenSubObjType(MCHAR *nm, MCHAR* pFilePrefix, int idx) constructor. The 
// bitmap files have to reside in the UI directory.

/** This class provides a generic implementation for sub-object types. Instead of
having to create a sub-class of ISubObjType the constructors of this
class may be used to initialize private data members of the class. Then
implementations of the GetName() and GetIcon() methods of
ISubObjType are provided which simply return the data members. 
This SubObjectType will either use the subObjectIcons_16i.bmp and
SubObjectIcons_16a.bmp bitmaps in the UI/icons directory (for the
GenSubObjType(int idx) constructor), or any other bmp file that is
specified in the GenSubObjType(MCHAR *nm, MCHAR* pFilePrefix, int idx)
constructor. The bitmap files have to reside in the UI/icons
directory. 
All methods of this class are implemented by the System. 
\see  Class ISubObjType, Class BaseObject, Class MaxIcon*/
class GenSubObjType : public ISubObjType {
	MSTR name;
	MaxIcon *mIcon;
	int mIdx;
	MSTR mFilePrefix;

public:
	/** Constructor. The private data members are inialized to the values
	passed and the corresponding Get methods of this class will return these data
	members.
	\param nm The name for this sub-object type.
	\param pFilePrefix The BMP imagelist file name prefix for this sub-object type. This is the file
	name, without the extension, and with the assumption that the file is in the
	ui/icons directory. For example specifying _M("SubObjectIcons") for this
	parameter indicates the file UI/icons/SubObjectIcons_16i.bmp if the
	small icons are in use and SubObjectIcons_24i.bmp if the large icons are
	in use.
	\param idx This is the one based index into the image list of the icon to use. */
	GenSubObjType(MCHAR *nm, MCHAR* pFilePrefix, int idx) : name(nm), mIcon(NULL), mIdx(idx), mFilePrefix(pFilePrefix) {}

	/** This constructor assumes that the icons are in either
	UI/icons/SubObjectIcons_16i.bmp or SubObjectIcons_24i.bmp
	depending on which size icons are in use by the system. In this case only the
	index into the image list is required.
	\param idx This is the one based index into the image list of the icon to use. */
	GenSubObjType(int idx) : mIcon(NULL), mIdx(idx), mFilePrefix(_M("SubObjectIcons")) {}

	/** Destructor. */
	CoreExport ~GenSubObjType();
	/** Sets the name for this sub-object type.
	\param nm The name to set. */
	void SetName(MCHAR *nm){name = nm;}
	/** Returns the name for this sub-object type. This is the
	implementation of the ISubObjType method. */
	MCHAR *GetName() { return name;}
	/** Returns the icon for this sub-object type. This is the
	implementation of the ISubObjType method. */
	CoreExport MaxIcon *GetIcon(); 
};

