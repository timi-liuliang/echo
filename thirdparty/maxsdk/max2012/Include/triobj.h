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
// FILE:        triobj.h
// DESCRIPTION: Defines Triangle Mesh Object
// AUTHOR:      Dan Silva
// HISTORY:     created 9 September 1994
//**************************************************************************/

#pragma once

#include "meshlib.h"
#include "snap.h"
#include "maxtess.h"
#include "object.h"

#define TRI_MULTI_PROCESSING TRUE

extern CoreExport Class_ID triObjectClassID;

#pragma warning(push)
#pragma warning(disable:4239)

/*! \sa  Class GeomObject, Class ClassDesc, Class Mesh, Class TessApprox.\n\n
\par Description:
This class represents a renderable, deformable, triangle mesh object. All
procedural objects must be able to convert themselves to TriObjects. This class
provides implementations of all the required methods of Animatable,
ReferenceMaker, ReferenceTarget, Base Object, Object, and GeomObject. All
methods of this class are implemented by the system.
\par Data Members:
<b>Mesh mesh;</b>\n\n
This is the mesh of the TriObject. See Class Mesh
for methods to manipulate this mesh.\n\n
The following data members are used by the Displacement Mapping mechanism in
3ds Max.\n\n
<b>TessApprox mDispApprox;</b>\n\n
The object which describes the properties of the tesselation approximation of
the mesh.\n\n
<b>bool mSubDivideDisplacement;</b>\n\n
The subdivision displacement flag. When TRUE, displacement mapping mechanism
subdivides mesh faces to accurately displace the map, using the method and
settings you specify in the Subdivision Presets and Subdivision Method group
boxes. When FALSE, the modifier applies the map by moving vertices in the mesh,
the way the Displace modifier does.\n\n
<b>bool mDisableDisplacement;</b>\n\n
TRUE to disable displacement mapping; FALSE to enable it.\n\n
<b>bool mSplitMesh;</b>\n\n
The split mesh flag. This flag affects texture mapping as done by the
displacement mapping mechanism. When on, the modifier splits the mesh into
individual faces before displacing them: this helps preserve texture mapping.
When off, the modifier uses an internal method to assign texture mapping.
Default=On.  */
class TriObject: public GeomObject {
	protected:
		Interval geomValid;
		Interval topoValid;
		Interval texmapValid;
		Interval selectValid;
		Interval vcolorValid;
		Interval gfxdataValid;
		ChannelMask validBits; // for the remaining constant channels
		CoreExport void CopyValidity(TriObject *fromOb, ChannelMask channels);
#if TRI_MULTI_PROCESSING
		static int		refCount;
		static HANDLE	defThread;
		static HANDLE	defMutex;
		static HANDLE	defStartEvent;
		static HANDLE	defEndEvent;
		friend DWORD WINAPI defFunc(LPVOID ptr);
#endif	
		//  inherited virtual methods for Reference-management
		CoreExport RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message );
	public:
		Mesh  mesh;
		TessApprox mDispApprox;
		bool mSubDivideDisplacement;
		bool mDisableDisplacement;
		bool mSplitMesh;

		CoreExport TriObject();
		CoreExport ~TriObject();

		//  inherited virtual methods:

		// display functions from BaseObject
		CoreExport virtual bool RequiresSupportForLegacyDisplayMode() const;
		CoreExport virtual bool UpdateDisplay(
			unsigned long renderItemCategories, 
			const MaxSDK::Graphics::MaterialRequiredStreams& materialRequiredStreams, 
			TimeValue t);

		// from InterfaceServer
		CoreExport virtual BaseInterface* GetInterface(Interface_ID iid);

		//from animatable
		CoreExport void* GetInterface(ULONG id);
		CoreExport void ReleaseInterface(ULONG id,void *i);

		// From BaseObject
		CoreExport int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		CoreExport int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);
		CoreExport void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt);
		CoreExport CreateMouseCallBack* GetCreateMouseCallBack();
		CoreExport RefTargetHandle Clone(RemapDir& remap);
		
		// From Object			 
		CoreExport ObjectState Eval(TimeValue time);
		CoreExport Interval ObjectValidity(TimeValue t);
		CoreExport BOOL HasUVW();
		CoreExport BOOL HasUVW (int mapChannel);

		// get and set the validity interval for the nth channel
	   	CoreExport Interval ChannelValidity(TimeValue t, int nchan);
		CoreExport void SetChannelValidity(int i, Interval v);
		CoreExport void InvalidateChannels(ChannelMask channels);

		// Convert-to-type validity
		CoreExport Interval ConvertValidity(TimeValue t);

		// Deformable object procs	
		int IsDeformable() { return 1; }  
		int NumPoints() { return mesh.getNumVerts(); }
		Point3 GetPoint(int i) { return mesh.getVert(i); }
		void SetPoint(int i, const Point3& p) { mesh.setVert(i,p); }

		CoreExport BOOL IsPointSelected (int i);
		CoreExport float PointSelection (int i);

		// Mappable object procs
		int IsMappable() { return 1; }
		int NumMapChannels () { return MAX_MESHMAPS; }
		int NumMapsUsed () { return mesh.getNumMaps(); }
		void ApplyUVWMap(int type, float utile, float vtile, float wtile,
			int uflip, int vflip, int wflip, int cap,const Matrix3 &tm,int channel=1) {
				mesh.ApplyUVWMap(type,utile,vtile,wtile,uflip,vflip,wflip,cap,tm,channel); }
				
        CoreExport BOOL PolygonCount(TimeValue t, int& numFaces, int& numVerts);
		void PointsWereChanged(){ mesh.InvalidateGeomCache(); }
		CoreExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm=NULL,BOOL useSel=FALSE );
		CoreExport void Deform(Deformer *defProc, int useSel);

		CoreExport int CanConvertToType(Class_ID obtype);
		CoreExport Object* ConvertToType(TimeValue t, Class_ID obtype);
		CoreExport void FreeChannels(ChannelMask chan);
		CoreExport Object *MakeShallowCopy(ChannelMask channels);
		CoreExport void ShallowCopy(Object* fromOb, ChannelMask channels);
		CoreExport void NewAndCopyChannels(ChannelMask channels);

		CoreExport DWORD GetSubselState();
		CoreExport void SetSubSelState(DWORD s);

		CoreExport BOOL CheckObjectIntegrity();

		// From GeomObject
		CoreExport int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm);
		CoreExport ObjectHandle CreateTriObjRep(TimeValue t);  // for rendering, also for deformation		
		CoreExport void GetWorldBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box );
		CoreExport void GetLocalBoundBox(TimeValue t, INode *inode, ViewExp* vpt, Box3& box );
		CoreExport Mesh* GetRenderMesh(TimeValue t, INode *inode, View &view,  BOOL& needDelete);
		
		// for displacement mapping
		CoreExport BOOL CanDoDisplacementMapping();
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns a reference to the <b>mDispApprox</b> data member. */
		CoreExport TessApprox& DisplacmentApprox() { return mDispApprox; }
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns a reference to the boolean <b>mSubDivideDisplacement</b> data
		member. */
		CoreExport bool& DoSubdivisionDisplacment() { return mSubDivideDisplacement; }
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns a reference to the boolean <b>mSplitMesh</b> data member. */
		CoreExport bool& SplitMeshForDisplacement() { return mSplitMesh; }
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method is used internally to set the <b>mDispApprox</b> data
		member to one of the low/medium/high subdivision presets. */
		CoreExport void SetDisplacmentApproxToPreset(int preset);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the <b>mDisableDisplacement</b> data member to the given state.
		\par Parameters:
		<b>BOOL disable</b>\n\n
		TRUE to disable; FALSE to enable. */
		CoreExport void DisableDisplacementMapping(BOOL disable);

		CoreExport void TopologyChanged();

		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns a reference to the <b>mesh</b> data member of this TriObject.
		*/
		Mesh& GetMesh() { return mesh; }

		// Animatable methods

		CoreExport void DeleteThis();
		void FreeCaches() {mesh.InvalidateGeomCache(); }
		Class_ID ClassID() { return Class_ID(TRIOBJ_CLASS_ID,0); }
		void GetClassName(MSTR& s) { s = MSTR(_M("TriObject")); }
		void NotifyMe(Animatable* subAnim, int message) { UNUSED_PARAM(subAnim); UNUSED_PARAM(message);}
		int IsKeyable() { return 0;}
		int Update(TimeValue t) { UNUSED_PARAM(t); return 0; }
		//BOOL BypassTreeView() { return TRUE; }
		// This is the name that will appear in the history browser.
		MCHAR *GetObjectName() { return _M("Mesh"); }

		CoreExport void RescaleWorldUnits(float f);

		// IO
		CoreExport IOResult Save(ISave *isave);
		CoreExport IOResult Load(ILoad *iload);

		// TriObject-specific methods
		//! \brief Should reduce any derived display data to save memory, since the node wont be drawn until the user undhides it
		/*! This function should delete any derived data used to display the object such as gfx normals, direct mesh caches etc.  
		This is typicallly called when the user hides the node	or sets it as bounding box
		*/
		CoreExport void ReduceDisplayCaches();
		
		/*! \brief This returns whether the Graphics Cache for this object needs to be rebuilt
		\par Parameters:
		GraphicsWindow *gw the active graphics window \n
		Material *ma the material aray assigned to the mesh \n
		int numMat the number of materials in the material array \n
		*/
		CoreExport bool NeedGWCacheRebuilt(GraphicsWindow *gw, Material *ma, int numMat);

		/*! \brief This builds the graphics window cached mesh 
		\par Parameters:
		GraphicsWindow *gw the active graphics window \n
		Material *ma the material aray assigned to the mesh \n
		int numMat the number of materials in the material array \n
		BOOL threaded whether when building the cache it can use additional threads.  This is needed since the system may be creating many meshes at the same time\n
		*/
		CoreExport void BuildGWCache(GraphicsWindow *gw, Material *ma, int numMat,BOOL threaded);
	};

#pragma warning(pop)

CoreExport void SetDisplacmentPreset(int preset, TessApprox approx);

// Regular TriObject
/*! \remarks Returns a pointer to the class descriptor for the regular,
standard TriObject. */
CoreExport ClassDesc* GetTriObjDescriptor();

// A new decsriptor can be registered to replace the default
// tri object descriptor. This new descriptor will then
// be used to create tri objects.

/*! \remarks A new descriptor can be registered to replace the default
TriObject descriptor. This new descriptor will then be used to create
TriObjects.
\par Parameters:
<b>ClassDesc* desc</b>\n\n
The class descriptor to replace the default TriObject descriptor. */
CoreExport void RegisterEditTriObjDesc(ClassDesc* desc);
/*! \remarks Returns a pointer to the class descriptor for the editable
TriObject. It returns the default if none has been registered. See below. */
CoreExport ClassDesc* GetEditTriObjDesc(); // Returns default of none have been registered

// Use this instead of new TriObject. It will use the registered descriptor
// if one is registered, otherwise you'll get a default tri-object.
/*! \remarks This method is used to create a new TriObject. Use this instead
of <b>new TriObject</b>. It will use the registered descriptor if one is
registered, otherwise you'll get a default TriObject. */
CoreExport TriObject *CreateNewTriObject();

#include "XTCObject.h"

const Class_ID kTriObjNormalXTCID = Class_ID(0x730a33d7, 0x27246c55);

// The purpose of this class is to remove specified Mesh normals
// after modifiers which would invalidate them.
class TriObjectNormalXTC : public XTCObject
{
public:
	TriObjectNormalXTC () { }

	Class_ID ExtensionID () { return kTriObjNormalXTCID; }
 	CoreExport XTCObject *Clone();

	ChannelMask DependsOn () { return PART_TOPO|PART_GEOM; }
	ChannelMask ChannelsChanged () { return 0; }

	CoreExport void PreChanChangedNotify (TimeValue t, ModContext &mc, ObjectState *os,
		INode *node, Modifier *mod, bool bEndOfPipeline);
	CoreExport void PostChanChangedNotify (TimeValue t, ModContext &mc, ObjectState *os,
		INode *node, Modifier *mod, bool bEndOfPipeline);
	
	CoreExport void DeleteThis ();
};


