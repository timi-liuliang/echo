/**********************************************************************
 	FILE: omanapi.h

	DESCRIPTION:  Defines an interface to the osnapmanager class

	CREATED BY: John Hutchinson
	HISTORY: May 14, 1997
	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "baseinterface.h"
#include "point3.h"
#include "matrix3.h"

// forward declarations
class Osnap;
class OsnapHit;
class ViewExp;
class INode;
class Point3;
class IPoint3;
struct SnapInfo;

// Flag masks for user settings
#define SHOW_PREVIEW  			(1<<0)
#define HIGHLIGHT_GEOM  		(1<<1)
#define HIGHLIGHT_POINT  		(1<<2)
#define SHOW_CURSOR					(1<<3)
#define XYZ_CONSTRAINT			(1<<5)
#define SNAP_FROZEN					(1<<15)
#define DISPLAY_RUBBER_BAND	(1<<20)

// Flag masks for internal stuff
#define BUFFER_HOLDING  		(1<<6)

  
#define IID_IOsnapManager Interface_ID(0x5ba68f3, 0x490c28a2)
/*! \sa  Class OsnapHit, Class INode, Class ViewExp, Class Matrix3, The Advanced Topics section on
<a href="ms-its:3dsmaxsdk.chm::/nodes_snapping.html">Snapping</a>.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class provides an interface to the OsnapManager. Developers who implement
osnaps need to record hits with the osnap manager. Developers implementing
command modes are responsible for getting the snap preview done and may be
responsible for initializing and closing point sequences. See the Advanced
Topics section on
<a href="ms-its:3dsmaxsdk.chm::/nodes_snapping.html">Snapping</a> for more
details.  */
class IOsnapManager :  public BaseInterface
{
public:
	/*! \remarks This method is used internally but may be called to determine
	if the <b>OsnapManager</b> is currently on. */
	virtual BOOL getactive() const =0;
	/*! \remarks This method is used internally but may be called to determine
	if the <b>OsnapManager</b> will use the system level axis constraints when
	performing translation. */
	virtual BOOL getAxisConstraint()=0;
	/*! \remarks When a plugin osnap finds a hit, it should call this method
	to record it with the manager. This will enter the hit in a stack which is
	sorted based on proximity to the cursor position. When multiple hits are
	recorded with the manager, the user has the ability to cycle throught the
	stack.
	\par Parameters:
	<b>OsnapHit* somehit</b>\n\n
	A pointer to the hit to record. The Osnap plugin should instantiate the
	hits and the manager is responsible for freeing them. */
	virtual void RecordHit(OsnapHit* somehit)=0;
	/*! \remarks Some snaps only make sense relative to a reference point.
	Consider, for example, the tangent snap which looks for points of tangency
	on splines. You can only find a tangent point relative to a second point.
	As objects are created and manipulated, the OsnapManager maintains a list
	of the Points that have been input to the current command mode. This method
	tells you if the first point has been recorded. */
	virtual BOOL OKForRelativeSnap()=0;
	/*! \remarks This method tells you if the point on the top of the
	reference stack was actually snapped. */
	virtual BOOL RefPointWasSnapped()=0;
	/*! \remarks This method retrieves the Point on the top of the reference
	stack. The point returned is in world space. Note that calling this method
	when the stack is empty will crash the program. Remember to call
	<b>OKForRelativeSnap()</b> first. Here are the first few lines of the
	tangent snap's main method.\n\n
	\code
	void ShapeSnap::Snap(Object* pobj, IPoint2 *p, TimeValue t) {
		if(!theman->OKForRelativeSnap())
			return;
		// Get the reference point
		Point3 relpoint(theman->GetRefPoint()); //the last point the user clicked.
		// Transform the reference point into the node's coordinate system
		Matrix3 tm = theman->GetObjectTM();
		relpoint = Inverse(tm) * relpoint;
		// . . .
	}
	\endcode
	\par Parameters:
	<b>BOOL top = TRUE</b>\n\n
	The default is to return the top of the stack, i.e. the last point which
	was input to the command mode. If you pass FALSE, the bottom of the stack
	will be returned. */
	virtual Point3 GetRefPoint(BOOL top = TRUE)=0;
	/*! \remarks This method is used internally. This method tells if the any
	hits were recorded with the OsnapManager during the last scene traversal.
	*/
	virtual BOOL IsHolding()=0;
	/*! \remarks This method is used internally. It returns the current hit
	i.e. the hit which the manager is currently displaying. */
	virtual OsnapHit &GetHit()=0;
	/*! \remarks Returns a pointer to a <b>ViewExp</b>. This is only valid for
	the duration of the current scene traversal. It is guaranteed to be valid
	while in a plugin's <b>Snap()</b> method. */
	virtual ViewExp* GetVpt()=0;
	/*! \remarks Returns a pointer to the node which is currently being
	snapped. */
	virtual INode* GetNode()=0;
	/*! \remarks Returns the current snap strength. This is the radius of a
	circular area about the cursor. It is the plugin's responsibility to
	determine if any "interesting" parts of the object lie within this area.
	For testing points, see the method
	<b>OsnapManager::CheckPotentialHit()</b>. */
	virtual int GetSnapStrength()=0;
	/*! \remarks Returns the object transformation matrix of the node which is
	currently being snapped. */
	virtual Matrix3 GetObjectTM()=0;
	/*! \remarks Returns the animation time of the current scene traversal. */
	virtual TimeValue GetTime()=0;
	/*! \remarks This method is not currently used. The method transforms a
	point in the current node's object space into screen space.
	\par Parameters:
	<b>Point3 *inpt</b>\n\n
	A pointer to the object space point.\n\n
	<b>IPoint3 *outpt</b>\n\n
	A pointer to storage for the screen space point. */
	virtual void wTranspoint(Point3 *inpt, IPoint3 *outpt)=0;
	/*! \remarks This method may be called to clear out the OsnapManager's
	reference point stack. Typically this is handled internally. However,
	objects which implement their own creation processes may need to call this
	method upon completion of a creation cycle. */
	virtual void Reset() =0;
	virtual BOOL TestAFlag(int mask)=0;
	virtual Point3 GetCurrentPoint()=0;
};

#define IID_IOsnapManager7 Interface_ID(0x795e7eae, 0x48e02d13)
class IOsnapManager7 :  public IOsnapManager
{
public:
	// --- Snap Preview Radius management
	// This is the radius of the circular area around the mouse cursor in which 
	// the snap point has to be in order to register a hit with the Snap Manager.
	// From all the registered hits, the Snap Manager determines the closest to
	// the mouse cursor and asks it to highlight ("preview" itself)
	virtual int GetSnapPreviewRadius() const = 0; 
	virtual void SetSnapPreviewRadius(int nVal) = 0; 

	// --- Snap Radius management
	// This is the radius of the circular area around the mouse cursor in which 
	// the highlighted snap point has to fall into for the transformed entity to 
	// be automatically snapped to that snap point. Note that the entity will 
	// always snap to the highlighted snap point when the user starts or finishes 
	// transforming the entity
	virtual int GetSnapRadius() const = 0; 
	virtual void SetSnapRadius(int nVal) = 0; 

	// --- Rubber-Band Display management methods
	// These methods allow the toggling of the rubber-band display when the user
	// transforms an entity. Note that the rubber-band is not displayed during
	// object creation
	virtual bool GetDisplaySnapRubberBand() const = 0;
	virtual void SetDisplaySnapRubberBand(bool bDisplayRubberBand) = 0;

	// Use Axis Constraint
	virtual void SetAxisConstraint(BOOL bUseAxisConstraint) = 0;

	// Read-Only access to the SnapInfo data. 
	// Returns false if it could not satisfy the request
	virtual bool GetSnapInfo(SnapInfo& si) const = 0;

	// Returns a pointer to the Osnap object that is current override snap object
	// The active snap type of the returned snap object is the exact snap override type.
	// NULL is returned when the snap system is not in override mode.
	virtual const Osnap* GetSnapOverride() const = 0;

	// for MXS exposure
	virtual void SetAFlag(int mask) = 0;
	virtual void ClearAFlag(int mask) = 0;
	virtual void setactive(boolean state) = 0;
	virtual void sethilite(Point3 col) = 0;
	virtual Point3 gethilite() = 0;
	virtual void setmarksize(int val) = 0;
	virtual int getmarksize() = 0;
	virtual BOOL SnapToFrozen() = 0;
	virtual void SetSnapToFrozen(BOOL state = TRUE) = 0;
	virtual Osnap* Enumerate(int FLAGS) const = 0;
	virtual DWORD GetSnapFlags() = 0;

	CoreExport static IOsnapManager7* GetInstance();
};


