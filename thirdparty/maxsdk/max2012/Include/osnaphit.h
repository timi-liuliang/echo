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
// FILE:        osnaphit.h
// DESCRIPTION: Defines the classes which are passed from the osnaps 
//              to the manager.
// AUTHOR:      John Hutchinson
// HISTORY:     December 9, 1996
//**************************************************************************/
#pragma once
#include "coreexp.h"
#include "maxheap.h"
#include "baseinterface.h"
#include "point3.h"
#include "ipoint3.h"
#include "box2.h"
#include "box3.h"

// forward declarations
class Osnap;
class HitMesh;
class OsnapMarker;
class ViewExp;
class INode;

/*! \sa  Class IOsnapManager, Class Osnap,  Class HitMesh,  Class Point3, Class IPoint3,  The Advanced Topics section on
<a href="ms-its:3dsmaxsdk.chm::/nodes_snapping.html">Snapping</a>.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class encapsulates the data required to record a snapped point. Typically
a plug-in creates instances of this class and records them with the
<b>OsnapManager</b>. The manager is responsible for freeing the memory
associated with recorded hits. All the methods of this class are implemented by
the system. If a snap plugin needs to record additional data for its hits, it
should derive from this class and provide a clone method which copies this
additional data and calls the base classes clone method. */
class OsnapHit : public BaseInterfaceServer
{
	friend class OsnapManager;
	friend class Osnap;
	friend class TypedHit;
public:
	/*! \remarks Constructor. The data members are initialized to the values
	passed.
	\par Parameters:
	<b>Point3 p3</b>\n\n
	The point that was hit in object space.\n\n
	<b>Osnap* s</b>\n\n
	Points to the Osnap instance which made this hit.\n\n
	<b>int sub</b>\n\n
	The sub-snap index which made this hit.\n\n
	<b>HitMesh *m</b>\n\n
	Points to the mesh used to hilite the topology that was hit. */
	CoreExport OsnapHit(Point3 p3, Osnap* s, int sub, HitMesh *m);
	CoreExport OsnapHit(Point3 pt);
	/*! \remarks Constructor. The data members are initialized from the
	<b>OsnapHit</b> passed.
	\par Parameters:
	<b>const OsnapHit\& h</b>\n\n
	The data members are copied from this <b>OsnapHit</b>. */
	CoreExport OsnapHit(const OsnapHit& h);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const OsnapHit\& h</b>\n\n
	The <b>OsnapHit</b> to assign. */
	CoreExport virtual OsnapHit& operator=(const OsnapHit& h);
	/*! \remarks Destructor. If a <b>HitMesh</b> was allocated it is deleted.
	*/
	CoreExport virtual ~OsnapHit();
	/*! \remarks Returns a pointer to a new instance of the <b>OsnapHit</b>
	class and initializes it with this instance. Developers deriving from this
	class should overide this method. */
	CoreExport virtual OsnapHit* clone();

	/*! \remarks This method is used internally. Sets the data members
	associated with screen hit data.
	\par Parameters:
	<b>IPoint3 screen3</b>\n\n
	The hit location in screen space.\n\n
	<b>int len</b>\n\n
	The distance from the cursor. */
	void setscreendata(IPoint3 screen3, int len);

	//////////////////////////////////////////////////////////////////////
	// Display Methods
	//////////////////////////////////////////////////////////////////////
	/*! \remarks This display method is implemented by the system and used
	internally.
	\par Parameters:
	<b>ViewExp *vpt</b>\n\n
	The viewport to display in.\n\n
	<b>TimeValue t</b>\n\n
	The current time.\n\n
	<b>Point3 color</b>\n\n
	The color to draw it in.\n\n
	<b>int markersize</b>\n\n
	The relative size of the icon.\n\n
	<b>boolean markers = TRUE</b>\n\n
	Controls whether or not the hit icon is drawn.\n\n
	<b>boolean hilite = TRUE</b>\n\n
	Controls whether or not the mesh part of the hit is drawn. */
	CoreExport virtual boolean display(ViewExp *vpt, TimeValue t, Point3 color,  \
		int markersize, boolean markers = TRUE, boolean hilite = TRUE);
	/*! \remarks This method is not currently used. */
	CoreExport void erase(ViewExp *vpt, TimeValue t) const; // the hit can erase itself
	/*! \remarks Implemented by the system.\n\n
	This method determines the damage rectangle for this hit.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to compute the rectangle.\n\n
	<b>ViewExp *vpt</b>\n\n
	The viewport in which to compute the rectangle.\n\n
	<b>Rect *rect</b>\n\n
	Points to storage for the computed result.\n\n
	<b>int marksize</b>\n\n
	The size of the icon. */
	CoreExport void GetViewportRect(TimeValue t, ViewExp* vpt, Rect* rect, int marksize) const;

	//////////////////////////////////////////////////////////////////////
	// Accessor Methods
	//////////////////////////////////////////////////////////////////////
	/*! \remarks Returns the hit location in object space. */
	CoreExport Point3 GetHitpoint(){return hitpoint;};
	/*! \remarks Returns the hit location in world space. */
	CoreExport Point3 GetWorldHitpoint(){return worldpoint;};
	/*! \remarks Returns the hit location in screen space. <b>IPoint3.z</b> is
	the depth in screen space. */
	CoreExport IPoint3 GetHitscreen(){return m_hitscreen;};
	/*! \remarks Returns the sub-snap index which made this hit. */
	CoreExport int GetSubsnap(){return subsnap;}
	CoreExport POINT GetCursor(){return m_cursor;}
	/*! \remarks Returns a pointer to the node which got hit. */
	INode* GetNode(){return node;}
	/*! \remarks This method is used internally for debugging and is not
	implemented for plug-in use.
	\par Operators:
	*/
	void Dump()const;

	//////////////////////////////////////////////////////////////////////
	// Operator Methods
	//////////////////////////////////////////////////////////////////////
	//define comparators so we can sort a list of these
	/*! \remarks This comparison operator is used in sorting.
	\par Parameters:
	<b>OsnapHit\& hit</b>\n\n
	The <b>OsnapHit</b> to compare.
	\return  TRUE if the distance from the cursor is less than <b>hit</b>;
	otherwise checks if the depth in Z space is less than <b>hit</b>: If so
	TRUE; otherwise FALSE. */
	CoreExport BOOL operator<(OsnapHit& hit);
	/*! \remarks This comparison operator is used in sorting.
	\par Parameters:
	<b>OsnapHit\& hit</b>\n\n
	The <b>OsnapHit</b> to compare.
	\return  TRUE if the distance from the cursor is less than <b>hit</b>;
	otherwise checks if the depth in Z space is less than <b>hit</b>: If so
	TRUE; otherwise FALSE. */
	CoreExport BOOL operator>(OsnapHit& hit);

public:
	/*! \remarks This method is not currently implemented. */
	void Update(TimeValue t);
	/*! \remarks Implemented by the System.\n\n
	This method updates the internal data to reflect a change in time. For
	example, if a hit is recorded on the endpoint of a particular edge of a
	mesh, the node moving would invalidate the hit data and a call to this
	method would be required before using its data.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to reevaluate it.
	\return  The updated point in world space. */
	CoreExport Point3 ReEvaluate(TimeValue t);
	virtual bool IsWorldSpaceHit(){return false;}


private: //data
	Point3 hitpoint; // the hit location in object space
	Point3 worldpoint; // the hit location in world space
	IPoint3 m_hitscreen; // the hit location in screen space
	POINT m_cursor;//the position of the cursor when this guy was recorded
	int m_len; //The distace from the cursor
	int m_z_depth; //The depth in z space
	BOOL m_complete; //indicates whether the screendata has been set
	Osnap*  snap; //the snap which made this hit
	int subsnap; //the subsnap index that made this hit
	OsnapMarker *m_pmarker; //a pointer to this snaps marker

	INode* node;// the node which got hit
	HitMesh *hitmesh;//a mesh used to hilite the topolgy we hit

	ViewExp *m_vpt;//the viewport which was active

	BOOL m_invisible;//this guy won't display itself
};

//a class to hold a list of object space points for highlighting the geometry
/*! \sa  Class Osnap,  The Advanced Topics section on
<a href="ms-its:3dsmaxsdk.chm::/nodes_snapping.html">Snapping</a>..\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is a class to hold a list of object space points for highlighting the
geometry associated with a hit. One oddity of this class is that you need to
allocate one point more than the number of points you actually need. This
additional storage is used for clipping against the viewport at display time.
For example the "endpoint" snap would alocate a three-point hitmesh to
highlight the edge which was hit (See the example below) .\n\n
The class has data members for the number of points and a pointer to the actual
<b>Point3</b> list.\n\n
In practice, developers need only use the first two methods shown below. <br>
*/
class HitMesh: public MaxHeapOperators 
{
private:
	int m_numverts;
	Point3 *m_pverts;
	static long m_cref;
public:
	/*! \remarks Constructor. The number of points is set to zero and the
	point list is set to NULL. */
	CoreExport HitMesh();
	/*! \remarks Destructor. If point list is allocated it is freed. */
	CoreExport ~HitMesh();
	/*! \remarks Constructor. The number of points and the points are
	initialized from the <b>HitMesh</b> passed.
	\par Parameters:
	<b>const HitMesh\& h</b>\n\n
	The <b>HitMesh</b> to init from. */
	CoreExport HitMesh(const HitMesh& h);
	/*! \remarks Constructor. The number of vertices is set to the value
	<b>n</b> passed and the points array is allocated with that size. This is
	the constructor that plugin developers will typically use in conjunction
	with the <b>setVert()</b> method described below.
	\par Parameters:
	<b>int n</b>\n\n
	The number of points to allocate. */
	CoreExport HitMesh(int n);
	/*! \remarks Access operator. Return the 'i-th' point.
	\par Parameters:
	<b>int i</b>\n\n
	Specifies the point to return. */
	Point3 operator[](int i){return m_pverts[i];}
	/*! \remarks Returns the number of points in this <b>HitMesh</b>. */
	int getNumVerts(){return m_numverts;}
 	/*! \remarks Sets the number of vertices. This frees any existing verts
 	and allocates new ones.
 	\par Parameters:
 	<b>int n</b>\n\n
 	The number of points to allocate. */
 	CoreExport void setNumVerts(int n);     
	/*! \remarks Sets the 'i-th' vertex to the specified value.
	\par Parameters:
	<b>int i</b>\n\n
	The vertex to set.\n\n
	<b>const Point3 \&xyz</b>\n\n
	The value to set.\n\n
	<b>In practice, developers need only use the two previous methods. The
	following code segment from the mesh snap exemplifies their use.</b>\n\n
\code
//add the hit points based on the active subsnaps
if(GetActive(EDGE_SUB))  // The edge snapping is active and we have a hit on the edge defined by from and to.
{
	HitMesh *hitmesh = new HitMesh(3);			  //Allocate one more than we need.
	hitmesh->setVert(0, from);
	hitmesh->setVert(1, to);
	float dap = Length(cursor - sf2);
	assert(Length(st2 - sf2)>=0.0f);
	float pct = (float)sqrt(fabs(dap*dap - distance*distance)) / Length(st2 - sf2);
	Point3 cand;
	float pctout = gw->interpWorld(&xyz[0],&xyz[1],pct,&cand);
	theman->RecordHit(new EdgeHit(cand, this, EDGE_SUB, hitmesh, ifrom, ito, pct));
}
\endcode  */
	void setVert(int i, const Point3 &xyz){ m_pverts[i] = xyz; }	
	/*! \remarks Returns the 'i-th' vertex. */
	Point3& getVert(int i){return m_pverts[i];}
	/*! \remarks Returns a pointer to the array of points.
	\par Operators:
	*/
	Point3* getVertPtr(){return m_pverts;}
};


class TypedHit: public OsnapHit
{
public:
	TypedHit(Point3 pt);
	virtual boolean display(ViewExp *vpt, TimeValue t, Point3 color,  \
		int markersize, boolean markers = TRUE, boolean hilite = TRUE);
	virtual bool IsWorldSpaceHit(){return true;}
};
