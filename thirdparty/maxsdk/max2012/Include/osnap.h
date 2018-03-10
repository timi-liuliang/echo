/**********************************************************************
 *<
	FILE: osnap.h

	DESCRIPTION:  Classes for Osnaps

	CREATED BY: John Hutchinson

	HISTORY: December 9, 1996
 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "export.h"
#include "maxheap.h"
#include "tab.h"
#include "maxtypes.h"
#include "point2.h"
#include "strclass.h"

// forward declarations
class Point3;
class Object;
class IPoint2;
class INode;
class ViewExp;
struct SnapInfo;
class GraphicsWindow;
class ObjectState;

#define RESET 0
#define NEXT 1
#define ICON_WIDTH 32

#define HILITE_NORMAL	(1<<0)
#define HILITE_BOX		(1<<1)
#define HILITE_NODE		(1<<2)//Not implemented
#define HILITE_CROSSHAIR (1<<3)

#define OSNAP_STATE	_M("ObjectSnapPluginState")

//Some types
class Candidate: public MaxHeapOperators
{
public:
	Candidate(){};
	Candidate(Point3* p, int s, int num=0, ...);
	void SetMeshPoint(int which, const Point3 p);
	~Candidate();
	Point3 *pt;
	int type;
	int count;
	Point3 *meshverts;
};

typedef Tab<Candidate *> CandidateTab;
typedef	void (*SnapCallback) (Object* pobj, IPoint2 *p) ;

class OsnapMarker; 
class HitMesh;
class IOsnapManager;
class MXS_IOsnap; // LAM: added 2/6/01 for MXS osnap exposure

//The osnap class
//===========================================================================

/*! \sa  Class OsnapHit, Class IOsnapManager, Class OsnapMarker, Structure SnapInfo, The Advanced Topics section on
<a href="ms-its:3dsmaxsdk.chm::/nodes_snapping.html">Snapping</a>.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is the base class for the derivation of a new object snap plug-ins.\n\n
Conceptually, the osnap class represents a "rule" for locating points in an
object's local space. Typically, an instance of this class will only make sense
for certain object types. It's the job of the <b>ValidInput()</b> method to
filter out uninteresting nodes in the scene. When the scene is traversed, each
object which passes the input test will be passed into the <b>Snap()</b>
method. This method is the workhorse of object snap plug-ins and is responsible
for computing, allocating and recording its hits.\n\n
For convenience, an object snap plug-in may encompass multiple sub-rules. For
example, the shape snap contains rules for computing both tangents and
perpendicular points on splines. Therefore many of the methods have an index
argument which identifies which sub-snap it applies to.\n\n
For sample code see <b>/MAXSDK/SAMPLES/SNAPS/SPHERE/SPHERE.CPP</b>. */
#pragma warning(push)
#pragma warning(disable:4100)
class Osnap: public MaxHeapOperators {
	friend class OsnapHit;
	friend class MXS_IOsnap; // LAM: added 2/6/01 for MXS osnap exposure
	friend class OsnapManager;
	friend class OSnapDecorator;

protected:
	BOOL *m_active;
	DllExport void _Snap(INode* inode, TimeValue t, ViewExp* vpt, IPoint2* p, SnapInfo* snap);
	DllExport Point3 _ReEvaluate(TimeValue t, OsnapHit *hit);
	/*! \remarks Implemented by the system.\n\n
	Returns TRUE if any of the sub-snaps are active; otherwise FALSE. */
	DllExport boolean IsActive();
	/*! \remarks Implemented by the system.\n\n
	Sets the indexed sub-snap to the specified state. */
	DllExport void SetActive(int index, boolean state);
	/*! \remarks Implemented by the system.\n\n
	Returns TRUE if any of the indexed sub-snap is active. */
	DllExport boolean GetActive(int index);

	GraphicsWindow *m_hitgw;

	//Point management.
	//Some osnaps may want to maintain a list of potential hit points. 
	//Note that the points should be passed in in modeling space
	DllExport void AddCandidate(Point3 *pt, int type = -1, int num = 0,...);
	DllExport Point3 *GetCandidatePoint(int index);
	DllExport void GetCandidateMesh(int index, HitMesh *m);
	DllExport int GetCandidateType(int index);
	DllExport void ClearCandidates();
	DllExport int NumCandidates(){return point_candidates.Count();}
	DllExport virtual Point3 ReEvaluate(TimeValue t, OsnapHit *hit, Object* pobj);

	//Note: the following version uses an index into the candidate list
	DllExport BOOL CheckPotentialHit(int ptindex, Point2 cursor);
	//Note: the following version uses a remote matrix of points
	DllExport BOOL CheckPotentialHit(Point3 *p, int ptindex, Point2 cursor);
	int m_baseindex;//an index into the tool array

public:

	/*! \remarks Constructor. The <b>OsnapManger</b> interface pointer is
	initialized. */
	DllExport Osnap();
	/*! \remarks Destructor. Internal data structure to the class are freed.
	*/
	DllExport virtual ~Osnap();
	DllExport void Init();

	/*! \remarks Returns the number of sub-snaps this plug-in provides.
	\par Default Implementation:
	<b>{return 1;}</b> */
	virtual int numsubs(){return 1;}; //the number of subsnaps this guy has
	/*! \remarks Returns the category string for this plug-in. If the plug-in
	fails to overide this method, the snap will be added to the "standard" page
	of the UI.
	\par Default Implementation:
	<b>{return NULL;}</b> */
	DllExport virtual MCHAR *Category();//JH 01/04/98 {return NULL;}
	/*! \remarks Returns the Class_ID for this plug-in.
	\par Default Implementation:
	<b>{ return Class_ID( 0, 0); }</b> */
	virtual Class_ID ClassID() { return Class_ID( 0, 0); }
	/*! \remarks Developers have the option of placing all their code in a
	single <b>Snap()</b> method or of breaking it up into multiple callbacks.
	Developers wishing to use callbacks should overide this method to return
	TRUE. Note: if callbacks are used, the <b>Snap()</b> method is <b>not</b>
	called.
	\par Default Implementation:
	<b>{return FALSE;}</b> */
	virtual BOOL UseCallbacks(){return FALSE;}
	/*! \remarks Returns the number of callbacks used.
	\par Default Implementation:
	<b>{return 0;}</b> */
	virtual int NumCallbacks(){return 0;}
	/*! \remarks This method is provided for future use so that snaps can
	evaluate the object at arbitrary points in the pipeline. Returns TRUE if
	the object associated with the node passed is supported; otherwise FALSE.
	The default implementation calls <b>ValidInput()</b> and fills the storage
	pointed to by os with the object state at the end of the geometry pipeline.
	This is the same object state returned by EvalWorldState.
	\par Parameters:
	<b>INode *iNode</b>\n\n
	The node whose object being checked.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to check the object.\n\n
	<b>ObjectState *os</b>\n\n
	This pointer should be updated to the <b>ObjectState</b> of the object
	associated with the node by calling <b>INode::EvalWorldState()</b>.
	\par Default Implementation:
	<b>BOOL Osnap::GetSupportedObject(INode *inode, TimeValue t, ObjectState
	*os) {</b>\n\n
	<b> *os = inode-\>EvalWorldState(t);</b>\n\n
	<b> assert(os);</b>\n\n
	<b> Class_ID thistype = os-\>obj-\>ClassID();</b>\n\n
	<b> unsigned long supertype = os-\>obj-\>SuperClassID();</b>\n\n
	<b> return ValidInput(supertype,thistype)?TRUE:FALSE;</b>\n\n
	<b>}</b> */
	DllExport virtual BOOL GetSupportedObject(INode* iNode, TimeValue t, ObjectState* os);

	/*! \remarks Returns a pointer to the snap's name to be displayed in the
	user interface.
	\par Parameters:
	<b>int index</b>\n\n
	The index of the sub-snap whose name is returned. */
	virtual MSTR *snapname(int index)=0; // the snaps name to be displayed in the UI
	/*! \remarks Reserved for future use.
	\par Parameters:
	<b>int index</b>\n\n
	The index of the sub-snap whose name is returned.
	\par Default Implementation:
	<b>{return NULL;}</b> */
	virtual MSTR *tooltip(int index){return NULL;} // the snaps name to be displayed in the UI
	/*! \remarks This method is used to check if the object whose super class
	ID and class ID are passed is valid input for this object snap plug-in to
	snap to.
	\par Parameters:
	<b>SClass_ID scid</b>\n\n
	The Super Class ID to check.\n\n
	<b>Class_ID cid</b>\n\n
	The Class ID to check.
	\return  Returns TRUE if the object is OK to snap to; otherwise FALSE.
	\par Sample Code:
	<b>boolean SphereSnap::ValidInput(SClass_ID scid, Class_ID cid){</b>\n\n
	<b> boolean c_ok = FALSE, sc_ok = FALSE;</b>\n\n
	<b> sc_ok |= (scid == GEOMOBJECT_CLASS_ID)? TRUE : FALSE;</b>\n\n
	<b> c_ok |= (cid == Class_ID(SPHERE_CLASS_ID,0))? TRUE :
	FALSE;</b>\n\n
	<b> return sc_ok \&\& c_ok;</b>\n\n
	<b>}</b> */
	virtual boolean ValidInput(SClass_ID scid, Class_ID cid)=0;//the objects it supports 
	/*! \remarks This method should return a pointer to a (typically static) OsnapMarker.
	These markers define the identifying markers which get displayed in the
	viewports.
	\par Parameters:
	<b>int index</b>\n\n
	The subsnap whose marker the system requires.
	\return  A pointer to an OsnapMarker. If this method returns NULL, a
	default marker will be displayed. */
	virtual OsnapMarker *GetMarker(int index)=0; // single object might contain subsnaps
	/*! \remarks Returns a value to indicate the type of highlighting to be done for this
	snap. Typically, some part of the objects geometry is illumintated. In some
	cases it is desirable to illuminate the objects bounding box and
	occasionally to draw a world space crosshair. The default implementations
	should normally be used.
	\return  One or more of the following values:\n\n
	<b>HILITE_NORMAL</b>\n\n
	This is the default and indicates that some part of the objects geometry
	will be hilited. The description of this geometry is recorded in the
	hitmesh member of the class OsnapHit.\n\n
	<b>HILITE_BOX</b>\n\n
	This is return value indicates that the objects bounding box should be
	drawn as the result of a hit on this object.\n\n
	<b>HILITE_CROSSHAIR</b>\n\n
	Reserved for grid snapping. This return value indicates that a world space
	crosshair should be drawn through the hitpoint.
	\par Default Implementation:
	<b>{return HILITE_NORMAL;}</b> */
	virtual WORD HiliteMode(){return HILITE_NORMAL;}
	// UI methods
	/*! \remarks This method is reserved for future use.
	\par Default Implementation:
	<b>{return TRUE;}</b> */
	virtual boolean BeginUI(HWND hwnd){return TRUE;}
	/*! \remarks This method is reserved for future use.
	\par Default Implementation:
	<b>{}</b> */
	virtual void EndUI(HWND hwnd){};
	/*! \remarks Returns a handle to a bitmap that contains the icons to be
	displayed for this snap. If there are N subsnaps, this bitmap should
	contain N icons. The size of an individual icon is 16x15 or 16x16. */
	virtual HBITMAP getTools()=0;
	/*! \remarks Returns a handle to a bitmap that contains the masks for the
	UI icons for this snap plug-in. */
	virtual HBITMAP getMasks()=0;
	/*! \remarks This is the workhorse of a snap plug-in. This method should compute and
	record hits on the given object.
	\par Parameters:
	<b>Object* pobj</b>\n\n
	A pointer to an object which passed the valid input test. Note that if this
	method is called, you can make certain assumption about the class of the
	object and do appropriate casting as needed.\n\n
	<b>IPoint2 *p</b>\n\n
	The cursor position.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to check.
	\par Default Implementation:
	<b>{}</b> */
	virtual void Snap(Object* pobj, IPoint2 *p, TimeValue t){};
	/*! \remarks Developers may overide this method to do additional hit testing on each
	object as an additional rejection criteria. The default implementation
	returns TRUE and consequently filters nothing. Note that if this method
	returns FALSE for a given object, the snap method will never be called for
	it.\n\n
	Note: Nodes are always trivially rejected if the cursor position does not
	fall within the screen space bounding box of the node.
	\par Parameters:
	<b>Object* pobj</b>\n\n
	A pointer to the object returned by GetSupportedObject.\n\n
	<b>IPoint2 *p</b>\n\n
	The cursor position.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to hittest.
	\return  Returns TRUE if the object is being hit and should be considered
	for snapping.
	\par Default Implementation:
	<b>{return TRUE;}</b> */
	virtual BOOL HitTest(Object* pobj, IPoint2 *p, TimeValue t){return TRUE;}
	/*! \remarks Returns the specified callback to be used for snapping.
	\par Parameters:
	<b>int sub</b>\n\n
	The sub-snap index.
	\return  Note the following typedef -- a <b>SnapCallback</b> is simply a
	pointer to a function passed two arguments:\n\n
	<b>typedef void (*SnapCallback) (Object* pobj, IPoint2 *p) ;</b>
	\par Default Implementation:
	<b>{ return NULL;}</b> */
	virtual SnapCallback GetSnapCallback( int sub){ return NULL;}
	/*! \remarks This method is no longer used. */
	virtual WORD AccelKey(int index)=0;


protected://data
	IOsnapManager *theman;
	// Holds the point candidates
 	CandidateTab point_candidates; 
};
#pragma warning(pop)

//-----------------------------------------------------------------------------
//	OSnapDecorator

/*!	\sa Class Osnap
	\par Description:
	This class extends\decorates class Osnap with a few usefull methods. 
	Object Snap plugins should continue to derive from class Osnap. Code that 
	wishes to work with these plugins can use them directly or create and work 
	with instances of this class */
class OSnapDecorator : public Osnap
{
	public:
		// Ctor\Dtor
		DllExport OSnapDecorator(Osnap* pOsnap);

		/*! \remarks Returns true if at least one of its snap types is active  */
		DllExport virtual bool IsActive() const;
		/*! \remarks Returns true if the snap identified by the index parameter is active */
		DllExport virtual bool IsActive(const int nSnapIdx) const;
		/*! \remarks Turns On the snap identified by the index parameter */
		DllExport virtual void Activate(const int nSnapIdx);
		/*! \remarks Turns Off the snap identified by the index parameter */
		DllExport virtual void Deactivate(const int nSnapIdx);

		// --- From Osnap
		DllExport virtual int numsubs(); 
		DllExport virtual MCHAR *Category();
		DllExport virtual Class_ID ClassID();
		DllExport virtual BOOL UseCallbacks();
		DllExport virtual int NumCallbacks();
		DllExport virtual BOOL GetSupportedObject(
			INode *iNode, TimeValue t, ObjectState *os);

		DllExport virtual MSTR *snapname(int index);
		DllExport virtual OsnapMarker *GetMarker(int index);
		DllExport virtual HBITMAP getTools();
		DllExport virtual HBITMAP getMasks();
		DllExport virtual WORD AccelKey(int index);
		DllExport virtual boolean ValidInput(SClass_ID scid, Class_ID cid);

		DllExport virtual boolean BeginUI(HWND hwnd);
		DllExport virtual void EndUI(HWND hwnd);
		DllExport virtual void Snap(Object* pobj, IPoint2 *p, TimeValue t);
		DllExport virtual BOOL HitTest(Object* pobj, IPoint2 *p, TimeValue t);
		DllExport virtual SnapCallback GetSnapCallback( int sub);

	protected:
		OSnapDecorator(); // not implemented

	private:
		Osnap*	mpOsnapImp;
};



