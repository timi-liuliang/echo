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
// FILE:        polyobj.h
// DESCRIPTION: Defines Polygon Mesh Object
// AUTHOR:      Steve Anderson
// HISTORY:     created June 1998
//**************************************************************************/

#pragma once

#include "meshlib.h"
#include "mnmath.h"
#include "snap.h"
#include "object.h"
#include "maxtess.h"

#ifdef POLY_LIB_EXPORTING
#define PolyLibExport __declspec( dllexport )
#else
#define PolyLibExport __declspec( dllimport )
#endif

#define POLY_MULTI_PROCESSING TRUE

extern PolyLibExport Class_ID polyObjectClassID;

#pragma warning(push)
#pragma warning(disable:4239 4100)

/*! \sa  Class GeomObject.,
Class MNMesh\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
3ds Max 4 introduces a new type of pipeline object, the polygon-based mesh
object, or <b>PolyObject</b> for short. This object is based on the
<b>MNMesh</b> which has been present in the SDK for some time and used for 3ds
Max effects like MeshSmooth, Boolean, and Connect.\n\n
PolyObjects are more restricted than TriObjects in that they only support
"manifold topology". That is to say, you can't create "rat's nest" meshes out
of polygon meshes. Each edge in a polygon mesh can be referenced only once on
each "side", with a well defined "outside" and "inside" direction for every
element of polygons. In TriObjects, you could create a mesh with 5 vertices and
3 faces: (0,1,2), (0,1,3), and (0,1,4). This would be illegal in PolyObjects
because the edge (0,1) is referenced in the (0-\>1) direction by three
different faces. (0,1,2), (1,0,3) would be a legal pair of faces, however,
because the (0,1) edge is referenced only once in each direction. This implies
also that the two faces have consistent normals - there's a well-defined inside
and outside at the edge. It's impossible to have neighboring faces have
inconsistent normals in a PolyObject, which is why the FlipNormals method in
Editable Poly only works on entire elements.\n\n
PolyObjects have a complete edge list present virtually all the time. They also
have full topological links - the edges reference the vertices and faces they
use, the faces reference the vertices and edges, and the vertices also
reference the edges and faces. This is somewhat wasteful of memory, but it
makes it very easy to navigate the mesh and do complex algorithms like
subdivision or Booleans. (Of course this also means there's more to keep track
of in these operations.)\n\n
Mesh objects flowing up the pipeline should be freely convertible between
TriObjects and PolyObjects. Virtually all data should be preserved in
converting back and forth between these types. Exceptions include PolyObject
edge data, such as crease values, which are lost upon conversion to TriObjects
(since TriObjects have no edge lists).\n\n
Virtually all the public methods of PolyObject are based on the equivalent
methods in TriObject. class PolyObject provides implementations of all the
required methods of Animatable, ReferenceMaker, ReferenceTarget, Base Object,
Object, and GeomObject. All methods of this class are implemented by the
system.\n\n

\par Data Members:
<b>MNMesh mm;</b>\n\n
See class MNMesh for information about
manipulating the mesh. */
class PolyObject: public GeomObject {
private:
	// Displacement approximation stuff:
	TessApprox mDispApprox;
	bool mSubDivideDisplacement;
	bool mDisableDisplacement;
	bool mSplitMesh;

protected:
	Interval geomValid;
	Interval topoValid;
	Interval texmapValid;
	Interval selectValid;
	Interval vcolorValid;
	DWORD validBits; // for the remaining constant channels
	PolyLibExport void CopyValidity(PolyObject *fromOb, ChannelMask channels);
	//  inherited virtual methods for Reference-management
	PolyLibExport RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message );
public:
	MNMesh mm;

	PolyLibExport PolyObject();
	PolyLibExport ~PolyObject();

	//  inherited virtual methods:

	// display functions from BaseObject
	PolyLibExport virtual bool RequiresSupportForLegacyDisplayMode() const;
	PolyLibExport virtual bool UpdateDisplay(
		unsigned long renderItemCategories, 
		const MaxSDK::Graphics::MaterialRequiredStreams& materialRequiredStreams, 
		TimeValue t);

	// From BaseObject
	PolyLibExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
	PolyLibExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
	PolyLibExport void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
	PolyLibExport CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; }
	
	PolyLibExport RefTargetHandle Clone(RemapDir& remap);
	

	// From Object			 
	PolyLibExport ObjectState Eval(TimeValue time);
	PolyLibExport Interval ObjectValidity(TimeValue t);
	PolyLibExport BOOL HasUVW();
	PolyLibExport BOOL HasUVW(int mapChannel);
	PolyLibExport Object *CollapseObject();
	// get and set the validity interval for the nth channel
	PolyLibExport Interval ChannelValidity (TimeValue t, int nchan);
	PolyLibExport Interval PartValidity (DWORD partIDs);
	PolyLibExport void SetChannelValidity (int i, Interval v);
	PolyLibExport void SetPartValidity (DWORD partIDs, Interval v);
	PolyLibExport void InvalidateChannels (ChannelMask channels);

	// Convert-to-type validity
	PolyLibExport Interval ConvertValidity(TimeValue t);

	// Deformable object procs	
	int IsDeformable() { return 1; }  
	PolyLibExport int NumPoints();
	PolyLibExport Point3 GetPoint(int i);
	PolyLibExport void SetPoint(int i, const Point3& p);
	PolyLibExport void PointsWereChanged();
	PolyLibExport void Deform (Deformer *defProc, int useSel=0);

// The following members have been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.
	/*! \remarks	Returns TRUE if the 'i-th' point is selected; otherwise FALSE.
	\par Parameters:
	<b> int i</b>\n\n
	The zero based index of the point to check. */
	BOOL IsPointSelected (int i) {
		if ((i<0) || (i>=mm.numv)) return false;
		return mm.v[i].FlagMatch (MN_DEAD|MN_SEL, MN_SEL);
	}
	/*! \remarks	Returns a floating point weighted point selection.
	\par Parameters:
	<b> int i</b>\n\n
	The zero based index of the point to check.
	\return  Returns 1.0f if selected and 0.0f if not. */
	float PointSelection (int i) {
		if ((i<0) || (i>=mm.numv)) return 0.0f;
		if (mm.v[i].GetFlag (MN_DEAD)) return 0.0f;
		if (mm.v[i].GetFlag (MN_SEL)) return 1.0f;
		float *vssel = mm.getVSelectionWeights();
		if (vssel) return vssel[i];
		else return 0.0f;
	}

	/*! \remarks	Retreives the number of faces and vertices of the polyginal mesh representation
	of the object. Note: Plug-In developers should use the global function
	GetPolygonCount(Object*, int\&, int\&) to retrieve the number f vertices and
	faces in an arbitrary object.\n\n
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to compute the number of faces and vertices.\n\n
	<b>int\& numFaces</b>\n\n
	The number of faces is returned here.\n\n
	<b>int\& numVerts</b>\n\n
	The number of vertices is returned here.\n\n
	\par Default Implementation:
	<b> { return TRUE; }</b> */
	BOOL PolygonCount(TimeValue t, int& numFaces, int& numVerts) {
		numFaces = 0;
		numVerts = 0;
		for (int i=0; i<mm.numf; i++) if (!mm.f[i].GetFlag (MN_DEAD)) numFaces++;
		for (int i=0; i<mm.numv; i++) if (!mm.v[i].GetFlag (MN_DEAD)) numVerts++;
		return TRUE;
	}

	// Mappable object procs
	int IsMappable() { return 1; }
	int NumMapChannels () { return MAX_MESHMAPS; }
	int NumMapsUsed () { return mm.numm; }
	PolyLibExport void ApplyUVWMap(int type, float utile, float vtile, float wtile,
		int uflip, int vflip, int wflip, int cap,const Matrix3 &tm,int channel=1);

	PolyLibExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm=NULL,BOOL useSel=FALSE );

	PolyLibExport int CanConvertToType(Class_ID obtype);
	PolyLibExport Object* ConvertToType(TimeValue t, Class_ID obtype);
	PolyLibExport void FreeChannels(ChannelMask chan);
	PolyLibExport Object *MakeShallowCopy(ChannelMask channels);
	PolyLibExport void ShallowCopy(Object* fromOb, ChannelMask channels);
	PolyLibExport void NewAndCopyChannels(ChannelMask channels);

	/*! \remarks This method returns the selection levels defined for class
	MNMesh: <b>MNM_SL_OBJECT</b> (0), <b>MNM_SL_VERTEX</b> (1),
	<b>MNM_SL_EDGE</b> (2), and <b>MNM_SL_FACE</b> (3). */
	PolyLibExport DWORD GetSubselState();
	/*! \remarks This method allows you to set the selection levels defined
	for class MNMesh: <b>MNM_SL_OBJECT</b> (0), <b>MNM_SL_VERTEX</b> (1),
	<b>MNM_SL_EDGE</b> (2), and <b>MNM_SL_FACE</b> (3).
	\par Parameters:
	<b>DWORD s</b>\n\n
	The selection level to set. */
	PolyLibExport void SetSubSelState(DWORD s);

	/*! \remarks Uses the <b>MNMesh::CheckAllData()</b> method, which uses
	<b>DebugPrint()</b> to give details about any errors that it finds. */
	PolyLibExport BOOL CheckObjectIntegrity();

	// From GeomObject
	PolyLibExport int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
	PolyLibExport ObjectHandle CreateTriObjRep(TimeValue t);  // obselete, do not use!!!  (Returns NULL.)
	PolyLibExport void GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box );
	PolyLibExport void GetLocalBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box );
	PolyLibExport Mesh* GetRenderMesh(TimeValue t, INode *inode, View &view,  BOOL& needDelete);

	PolyLibExport void TopologyChanged();

	/*! \remarks Accessor for the MNMesh mm data member. */
	MNMesh& GetMesh() { return mm; }

	// Animatable methods

	PolyLibExport void DeleteThis();
	void FreeCaches() {mm.InvalidateGeomCache(); mm.InvalidateTopoCache(false); }
	Class_ID ClassID() { return Class_ID(POLYOBJ_CLASS_ID,0); }
	void GetClassName(MSTR& s) { s = MSTR(_M("PolyMeshObject")); }
	void NotifyMe(Animatable* subAnim, int message) { UNUSED_PARAM(subAnim); UNUSED_PARAM(message);}
	int IsKeyable() { return 0;}
	int Update(TimeValue t) { UNUSED_PARAM(t); return 0; }
	//BOOL BypassTreeView() { return TRUE; }
	// This is the name that will appear in the history browser.
	MCHAR *GetObjectName() { return _M("PolyMesh"); }
	PolyLibExport void RescaleWorldUnits(float f);

	// IO
	PolyLibExport IOResult Save(ISave *isave);
	PolyLibExport IOResult Load(ILoad *iload);

	// Displacement mapping parameter access.
	// (PolyObjects don't do displacement mapping directly, but they
	// pass their settings on to TriObjects.)
	/*! \remarks Implementation of a class GeomObject method which indicates
	whether or not this object supports displacement mapping. It returns true
	as long as <b>GetDisplacementDisable()</b> returns false. */
	PolyLibExport BOOL CanDoDisplacementMapping();
	/*! \remarks Sets displacement subdivision parameters to match one of the
	standard displacement approximation presets.
	\par Parameters:
	<b>int preset</b>\n\n
	Should be 0 for low, 1 for medium, or 2 for high. */
	PolyLibExport void SetDisplacementApproxToPreset(int preset);

	// Accessor methods:
	/*! \remarks Disables displacement subdivision (without altering the
	parameters).
	\par Parameters:
	<b>bool disable</b>\n\n
	TRUE to disable; FALSE to enable. */
	void SetDisplacementDisable (bool disable) { mDisableDisplacement = disable; }
	/*! \remarks Sets most of the displacement parameters. See
	class TessApprox for details.
	\par Parameters:
	<b>TessApprox \& params</b>\n\n
	The tessellation approximation data. */
	void SetDisplacementParameters (TessApprox & params) { mDispApprox = params; }
	/*! \remarks Controls displacement subdivision splitting.
	\par Parameters:
	<b>bool split</b>\n\n
	TRUE to set; FALSE to unset. */
	void SetDisplacementSplit (bool split) { mSplitMesh = split; }
	/*! \remarks Turns displacement on or off.
	\par Parameters:
	<b>bool displace</b>\n\n
	TRUE to turn on; FALSE to turn off. */
	void SetDisplacement (bool displace) { mSubDivideDisplacement = displace; }

	/*! \remarks Indicates whether displacement subdivision is currently
	disabled. */
	bool GetDisplacementDisable () const { return mDisableDisplacement; }
	/*! \remarks Accesses most of the displacement parameters. See
	class TessApprox for details. */
	TessApprox GetDisplacementParameters () const { return mDispApprox; }
	/*! \remarks Indicates whether displacement subdivision splitting is on.
	*/
	bool GetDisplacementSplit () const { return mSplitMesh; }
	/*! \remarks Indicates whether displacement subdivision is on. */
	bool GetDisplacement () const { return mSubDivideDisplacement; }

	/*! \remarks Accessor method for the displacement subdivision parameters
	that can be quicker to use than <b>SetDisplacementParameters()</b> and
	<b>GetDisplacementParameters()</b>.\n\n
	  */
	TessApprox &DispParams() { return mDispApprox; }

	//! \brief Should reduce any derived display data to save memory, since the node wont be drawn until the user undhides it
	/*! This function should delete any derived data used to display the object such as gfx normals, direct mesh caches etc.  
	This is typicallly called when the user hides the node	or sets it as bounding box
	*/
	PolyLibExport void ReduceDisplayCaches();

	/*! \brief This returns whether the Graphics Cache for this object needs to be rebuilt
	\par Parameters:
		GraphicsWindow *gw the active graphics window \n
		Material *ma the material aray assigned to the mesh \n
		int numMat the number of materials in the material array \n
	*/
	PolyLibExport bool NeedGWCacheRebuilt(GraphicsWindow *gw, Material *ma, int numMat);
	
	/*! \brief This builds the graphics window cached mesh 
	\par Parameters:
	GraphicsWindow *gw the active graphics window \n
	Material *ma the material aray assigned to the mesh \n
	int numMat the number of materials in the material array \n
	BOOL threaded whether when building the cache it can use additional threads.  This is needed since the system may be creating many meshes at the same time\n
	*/
	PolyLibExport void BuildGWCache(GraphicsWindow *gw, Material *ma, int numMat,BOOL threaded);	


};

#pragma warning(pop)

// Regular PolyObject
/*! \remarks Gets the class descriptor for the PolyObject. */
PolyLibExport ClassDesc* GetPolyObjDescriptor();

// A new descriptor can be registered to replace the default
// tri object descriptor. This new descriptor will then
// be used to create tri objects.

/*! \remarks Allows a plugin to register an Editable Poly object. (This is
done by epoly.dlo in the standard 3ds Max distribution.) This is the object
which is collapsed to when the stack is collapsed. The default if no such
object is registered is to simply collapse to a PolyObject (which has no UI
parameters).
\par Parameters:
<b>ClassDesc* desc</b>\n\n
A pointer to the class descriptor. */
PolyLibExport void RegisterEditPolyObjDesc(ClassDesc* desc);
/*! \remarks Gets the class descriptor for the currently registered Editable Poly object. */
PolyLibExport ClassDesc* GetEditPolyObjDesc(); // Returns default of none have been registered

// Use this instead of new PolyObject. It will use the registered descriptor
// if one is registered, otherwise you'll get a default poly-object.
/*! \remarks Returns an Editable Poly object from the currently registered EPoly 
descriptor, cast as a PolyObject.  */
PolyLibExport PolyObject *CreateEditablePolyObject();

// Inter-object conversions:
PolyLibExport void ConvertPolyToPatch (MNMesh & from, PatchMesh & to, DWORD flags=0);
PolyLibExport void ConvertPatchToPoly (PatchMesh & from, MNMesh & to, DWORD flags=0);

