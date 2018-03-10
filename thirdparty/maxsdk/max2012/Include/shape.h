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
// FILE:        shape.h
// DESCRIPTION: Defines Basic BezierShape Object
// AUTHOR:      Tom Hudson
// HISTORY:     created 23 February 1995
//**************************************************************************/

#pragma once

#include "maxheap.h"
#include "shphier.h"
#include "spline3d.h"
#include "shpsels.h"	// Shape selection classes
#include "hitdata.h"
#include "TabTypes.h"
#include "GraphicsConstants.h"

// forward declarations
class BezierShape;
class ModContext;

// Parameters for BezierShape::PerformTrimOrExtend
#define SHAPE_TRIM 0
#define SHAPE_EXTEND 1

//watje class contains the data for the bind points
class bindShape: public MaxHeapOperators
{
public:
	int pointSplineIndex;      // spline that is to be bound
	int segSplineIndex;        // spline that contains the segment to bind to
	int seg;					// seg that the point is bound to
	Point3 bindPoint,segPoint;  // posiiton in space of the start seg point and the bind point
	BOOL isEnd;                 // is the bound point the end or beginning of the spline
};


/*! \sa  Class BezierShape.\n\n
\par Description:
This class contains a hit record for sub-shape hit testing. All methods of this
class are implemented by the system.
\par Data Members:
<b>DWORD dist;</b>\n\n
The distance of the hit. If the user is in wireframe mode, this is the distance
in pixels to the item that was hit. If the user is in shaded mode, this is the
Z depth distance. Smaller numbers indicate a closer hit.\n\n
<b>BezierShape* shape;</b>\n\n
The shape that was hit.\n\n
<b>int poly;</b>\n\n
The polygon that was hit.\n\n
<b>int index;</b>\n\n
The index of the sub-object component that was hit.  */
class ShapeSubHitRec: public MaxHeapOperators {
	private:
		ShapeSubHitRec *next;
	public:
		DWORD	dist;
		BezierShape*	shape;
		int		poly;
		int		index;
		/*! \remarks Constructor. The data members are initialized to the
		values passed. */
		ShapeSubHitRec( DWORD dist, BezierShape *shape, int poly, int index, ShapeSubHitRec *next )
			{ this->dist = dist; this->shape = shape; this->poly = poly; this->index = index; this->next = next; }

		/*! \remarks Returns the next sub hit record. */
		ShapeSubHitRec *Next() { return next; }
	};

/*! class SubShapeHitList
\par Description:
This class provides methods for accessing the first hit list record and adding
hits to the list. All methods of this class are implemented by the system. <br>
*/
class SubShapeHitList: public MaxHeapOperators {
	private:
		ShapeSubHitRec *first;
	public:
		/*! \remarks Constructor. The list is set to NULL. */
		SubShapeHitList() { first = NULL; }
		/*! \remarks Destructor. The list is deleted. */
		CoreExport ~SubShapeHitList();	

		/*! \remarks Returns the first hit in the list. */
		ShapeSubHitRec *First() { return first; }
 		/*! \remarks Adds a hit record to the list.
 		\par Parameters:
 		<b>DWORD dist</b>\n\n
 		The distance of the hit. If the user is in wireframe mode, this is the
 		distance in pixels to the item that was hit. If the user is in shaded
 		mode, this is the Z depth distance. Smaller numbers indicate a closer
 		hit.\n\n
 		<b>BezierShape *shape</b>\n\n
 		The shape that was hit.\n\n
 		<b>int poly</b>\n\n
 		The polygon that was hit.\n\n
 		<b>int index</b>\n\n
 		The index of the sub-object component that was hit. */
 		CoreExport void AddHit( DWORD dist, BezierShape *shape, int poly, int index );  
		int Count() {
			int count = 0;
			ShapeSubHitRec *ptr = first;
			while(ptr) {
				count++;
				ptr = ptr->Next();
				}
			return count;
			}
	};

// Special storage class for hit records so we can know which object was hit
/*! \sa  Class HitData.\n\n
\par Description:
This is a storage class for hit records used in hit testing to know which
specific shape object was hit. All methods of this class are implemented by the
system.
\par Data Members:
<b>BezierShape *shape;</b>\n\n
The shape that was hit.\n\n
<b>int poly;</b>\n\n
The polygon of the shape that was hit.\n\n
<b>int index;</b>\n\n
The index of the sub-object entity that was hit.  */
class ShapeHitData : public HitData {
	public:
		BezierShape *shape;
		int poly;
		int index;
		/*! \remarks Constructor. The data members are initialized to the
		values passed. */
		ShapeHitData(BezierShape *shape, int poly, int index)
			{ this->shape = shape; this->poly = poly; this->index = index; }
		~ShapeHitData() {}
	};


// Callback used for retrieving other shapes in the current editing context
/*! \sa  Class BezierShape, Class ModContext.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class has a method used for retrieving other shapes in the current editing
context. This class provides a way for the
<b>BezierShape::PerformTrimOrExtend</b> method to access the shapes being
trimmed.  */
class ShapeContextCallback: public MaxHeapOperators {
	public:
		/*! \remarks This method will be called with a ModContext pointer; the
		function should return the shape for that context. This is only used in
		modifier applications, where more than one shape object is being
		modified. See <b>/MAXSDK/SAMPLES/MODIFIERS/EDITSPL.CPP</b> for an
		example of its use.
		\par Parameters:
		<b>ModContext *context</b>\n\n
		Points to the ModContext for the shape the modifier is applied to.
		\return  A pointer to the BezierShape for the context. */
		virtual BezierShape *GetShapeContext(ModContext* context) = 0;
	};

class ShapeObject;

/*! \sa  Class BezierShape, Template Class Tab, Class BitArray.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class has data members and methods used to build and store topology
information on a BezierShape. This class is used with the
<b>BezierShape::GetTopology()</b> method.
\par Data Members:
<b>BOOL ready;</b>\n\n
TRUE if the data has been built; otherwise FALSE.\n\n
<b>IntTab kcount;</b>\n\n
A table of integers containing the knot count for each bezier spline in the
shape.\n\n
<b>BitArray closed;</b>\n\n
A bit array containing a 1 for each closed spline or a 0 for each open one in
the shape.  */
class BezierShapeTopology: public MaxHeapOperators {
	public:
		BOOL ready;
		IntTab kcount;
		BitArray closed;
		/*! \remarks Constructor. The data members are initialized as
		follows:\n\n
		<b>ready = FALSE;</b> */
		BezierShapeTopology() { ready = FALSE; }
		/*! \remarks Builds the topology data for the specified shape.
		\par Parameters:
		<b>BezierShape \&shape</b>\n\n
		The shape whose topology data will be built. */
		CoreExport void Build(BezierShape &shape);
		/*! \remarks Assignment operator. */
		CoreExport int operator==(const BezierShapeTopology& t);
		/*! \remarks Used internally to save the shape topology data. */
		CoreExport IOResult Save(ISave *isave);
		/*! \remarks Used internally to load the shape topology data. */
		CoreExport IOResult Load(ILoad *iload);
	};

// The following {class, member, macro, flag} has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.

// CAL-04/12/01: Use area selection to jump between splines in soft selection
// To be merged into BezierShape in the future.
// Two public data members, mUseAreaSelect & mAreaSelect, in the interface extension class.
// Set mUseAreaSelect to TRUE to enable the use of area selection to jump between splines
// when doing soft selection with edge distance enabled. mAreaSelect is the distance
// allowed to jump from one node in a spline to another node in another spline.
#define BEZIER_SHAPE_INTERFACE_EX41 Interface_ID(0x23526cd1, 0x695d7e4e)
#define GET_BEZIER_SHAPE_INTERFACE_EX41() \
	(BezierShapeInterface_Ex41*) GetInterface(BEZIER_SHAPE_INTERFACE_EX41)

/*!  \n\n
class BezierShapeInterface_Ex41 : public BaseInterface\n\n

\par Description:
This class is available in release 4.2 and later only.\n\n
Extension to Class BezierShape to use area selection to jump between splines in
soft selection. Contains two public data members, <b>mUseAreaSelect</b> and
<b>mAreaSelect</b>, in the interface extension class. Set <b>mUseAreaSelect</b>
to TRUE to enable the use of area selection to jump between splines when doing
soft selection with edge distance enabled. <b>mAreaSelect</b> is the distance
allowed to jump from one node in a spline to another node in another
spline.\n\n
<b>BOOL mUseAreaSelect  </b>\n\n
TRUE if use area selection\n\n
<b>float mAreaSelect </b>\n\n
Radius of area selection   */
class BezierShapeInterface_Ex41 : public BaseInterface
{
public:
	BOOL mUseAreaSelect;		// TRUE if use area selection
	float mAreaSelect;			// Radius of area selection

	/*! \remarks Returns a pointer to the Base Interface for the interface ID
	passed.\n\n

	\par Parameters:
	<b>Interface_ID id</b>\n\n
	The unique ID of the interface to get\n\n
	  */
	BaseInterface*	GetInterface(Interface_ID id) { if (id == BEZIER_SHAPE_INTERFACE_EX41) return (BaseInterface*)this; else return NULL; }
	/*! \remarks This method returns the unique interface ID.\n\n
	  */
	Interface_ID	GetID() { return BEZIER_SHAPE_INTERFACE_EX41; }
	/*! \remarks This method can be used as a direct interface delete request.
	*/
	void			DeleteInterface();
};
// End of 3ds max 4.2 Extension

/*! \sa  Class ShapeObject, Class Spline3D, Class PatchCapInfo, Class ShapeVSel, Class ShapeSSel, Class ShapePSel, Class Material, <a href="ms-its:3dsmaxsdk.chm::/shps_shapes_and_splines.html">Working with
Shapes and Splines</a>.\n\n
\par Description:
Defines a basic bezier shape object. The <b>BezierShape</b> is effectively a
collection of Bezier Splines. For example the 3ds Max Donut object has two
splines in a hierarchy to make a shape. The <b>BezierShape</b> contains these
splines.
\par Method Groups:
See <a href="class_bezier_shape_groups.html">Method Groups for Class BezierShape</a>.
\par Data Members:
<b>PatchCapInfo patchCap;</b>\n\n
Patch capping cache (mesh capping and hierarchy caches stored in PolyShape
cache)\n\n
<b>BOOL patchCapCacheValid;</b>\n\n
Indicates if the patch cap is valid or not.\n\n
<b>Spline3D **splines;</b>\n\n
A pointer to the list of splines.\n\n
<b>int splineCount;</b>\n\n
The number of splines in this shape.\n\n
<b>int steps;</b>\n\n
Number of steps (a value of -1 will use adaptive).\n\n
<b>BOOL optimize;</b>\n\n
Setting this to TRUE optimizes linear segments\n\n
<b>ShapeVSel vertSel;</b>\n\n
The selected vertices.\n\n
<b>ShapeSSel segSel;</b>\n\n
The selected segments.\n\n
<b>ShapePSel polySel;</b>\n\n
The selected polygons.\n\n
<b>int bezVecPoly;</b>\n\n
This is used internally in hit testing.\n\n
<b>int bezVecVert;</b>\n\n
This is used internally in hit testing.\n\n
<b>DWORD selLevel;</b>\n\n
Selection level.\n\n
<b>SHAPE_OBJECT</b> - Object level selection.\n\n
<b>SHAPE_SPLINE</b> - Spline level selection (a single polygon within the
shape).\n\n
<b>SHAPE_SEGMENT</b> - Segment level selection.\n\n
<b>SHAPE_VERTEX</b> - Vertex level selection.\n\n
<b>DWORD dispFlags;</b>\n\n
Display attribute flags. See
<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_beziershape_display_flags.html">List of BezierShape
Display Flags</a>.\n\n
<b>Tab\<bindShape\> bindList;</b>\n\n
The table of bind points. See Template Class Tab.\n\n */
class BezierShape : public BaseInterfaceServer {
 		Box3			bdgBox;			// object space--depends on geom+topo
		static int shapeCount;			// Number of shape objects in the system!
		PolyShape pShape;				// PolyShape cache
		int pShapeSteps;				// Number of steps in the cache
		BOOL pShapeOptimize;			// TRUE if cache is optimized
		BOOL pShapeCacheValid;			// TRUE if the cache is current
		int *vertBase;					// Cache giving vert start index for each spline
		int totalShapeVerts;			// Total number of verts in the shape (cached)
		int *knotBase;					// Cache giving knot start index for each spline
		int totalShapeKnots;			// Total number of knots in the shape (cached)
		BOOL topoCacheValid;			// TRUE if topology cache is valid
		BezierShapeTopology topology;	// Topology cache
		
		// CAL-03/25/01: use area selection to jump between splines in soft selection
		//				 Make new APIs to access these in the future.
		// BOOL mUseAreaSelect;			// TRUE if use area selection
		// float mAreaSelect;				// Radius of area selection

		// CAL-04/12/01: move the above two private data members into interface extension
		//				 and use the following private methods to access them
		BOOL UseAreaSelect();
		void SetUseAreaSelect( BOOL useAreaSelect );

		float AreaSelect();
		void SetAreaSelect( float aresSelect );

		// CAL-03/25/01: Remove these friend declaration once their access to area selection
		//				 variables are replaced by the new APIs.
		friend INT_PTR CALLBACK SplineSelectDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

	public:
		
		// Patch capping cache (mesh capping and hierarchy caches stored in PolyShape cache)
		PatchCapInfo patchCap;
		BOOL patchCapCacheValid;

		// The list of splines
		Spline3D **splines;
		int splineCount;

		int steps;						// Number of steps (-1 = adaptive)
		BOOL optimize;					// TRUE optimizes linear segments

		// Selection
		ShapeVSel	vertSel;  		// selected vertices
		ShapeSSel	segSel;  		// selected segments
		ShapePSel	polySel;  		// selected polygons

		// If hit bezier vector, this is its info:
		int bezVecPoly;
		int bezVecVert;

		// Selection level
		DWORD		selLevel;

		// Display attribute flags
		DWORD		dispFlags;

		/*! \remarks The data members are initialized as follows:\n\n
		<b>splines = NULL;</b>\n\n
		<b>splineCount = 0;</b>\n\n
		<b>dispFlags = 0;</b>\n\n
		<b>selLevel = SHAPE_OBJECT;</b>\n\n
		<b>bdgBox.Init();</b>\n\n
		<b>bezVecPoly = -1;</b>\n\n
		<b>bezVecVert = -1;</b>\n\n
		<b>vertSel.Empty();</b>\n\n
		<b>segSel.Empty();</b>\n\n
		<b>polySel.Empty();</b>\n\n
		<b>optimize = FALSE;</b>\n\n
		<b>steps = 5;</b> */
		CoreExport BezierShape();
		/*! \remarks Constructor. The shape is initialized based on the
		<b>fromShape</b>. */
		CoreExport BezierShape(BezierShape& fromShape);

		/*! \remarks Initializes the <b>BezierShape</b>. The data members are
		initialized as follows:\n\n
		<b>splines = NULL;</b>\n\n
		<b>splineCount = 0;</b>\n\n
		<b>dispFlags = 0;</b>\n\n
		<b>selLevel = SHAPE_OBJECT;</b>\n\n
		<b>bdgBox.Init();</b>\n\n
		<b>bezVecPoly = -1;</b>\n\n
		<b>bezVecVert = -1;</b>\n\n
		<b>vertSel.Empty();</b>\n\n
		<b>segSel.Empty();</b>\n\n
		<b>polySel.Empty();</b>\n\n
		<b>optimize = FALSE;</b>\n\n
		<b>steps = 5;</b> */
		CoreExport void		Init();

		/*! \remarks Destructor. All the splines in this shape are cleared. */
		CoreExport ~BezierShape();

		/*! \remarks Assignment operator. */
		CoreExport BezierShape& 		operator=(BezierShape& fromShape);
		/*! \remarks Assignment operator. */
		CoreExport BezierShape& 		operator=(PolyShape& fromShape);
		/*! \remarks Returns a copy of the 'i-th' vertex of the specified spline.
		\par Parameters:
		<b>int poly</b>\n\n
		The index into the splines list where <b>poly \>= 0</b> and <b>poly \<
		splineCount</b>.\n\n
		<b>int i</b>\n\n
		The index of the control point in the spline. */
		CoreExport Point3	GetVert(int poly, int i);
		/*! \remarks Sets the 'i-th' vertex of the specified spline.
		\par Parameters:
		<b>int poly</b>\n\n
		The index into the splines list where <b>poly \>= 0</b> and <b>poly \<
		splineCount</b>.\n\n
		<b>int i</b>\n\n
		The index of the vertex in the spline.\n\n
		<b>const Point3 \&xyz</b>\n\n
		The point to set. */
		CoreExport void		SetVert(int poly, int i, const Point3 &xyz);
		
		/*! \remarks This is used internally to render the shape. */
		CoreExport void		Render(GraphicsWindow *gw, Material *ma, RECT *rp, int compFlags, int numMat);
		CoreExport void		RenderGizmo(GraphicsWindow *gw);
		/*! \remarks This is used internally to hit test the shape. */
		CoreExport BOOL		Select(GraphicsWindow *gw, Material *ma, HitRegion *hr, int abortOnHit = FALSE);
		/*! \remarks This is used internally to snap to the shape. */
		CoreExport void		Snap(GraphicsWindow *gw, SnapInfo *snap, IPoint2 *p, Matrix3 &tm);
		// See polyshp.h for snap flags
		/*! \remarks This is used internally to snap to the shape. */
		CoreExport void		Snap(GraphicsWindow *gw, SnapInfo *snap, IPoint2 *p, Matrix3 &tm, DWORD flags);
		/*! \remarks This method is used internally to perform sub-object hit
		testing of the shape. */
		CoreExport BOOL 	SubObjectHitTest(GraphicsWindow *gw, Material *ma, HitRegion *hr,
								DWORD flags, SubShapeHitList& hitList );

		/*! \remarks Computes the bounding box of the splines making up this
		bezier shape object. The result is stored in the <b>bdgBox</b> data
		member. */
		CoreExport void		BuildBoundingBox(void);
		/*! \remarks Returns the bounding box of the splines making up this
		bezier shape object. The optional TM allows the box to be calculated in
		any space.
		\par Parameters:
		<b>Matrix3 *tm=NULL</b>\n\n
		The matrix to transform the points before computing the bounding box.
		*/
		CoreExport Box3		GetBoundingBox(Matrix3 *tm=NULL); // RB: optional TM allows the box to be calculated in any space.
				                                              // NOTE: this will be slower becuase all the points must be transformed.
		
		/*! \remarks This method should be called when a shape changes. It
		invalidates the caches of the shape. This resets the bounding box, and
		removes the hierarchy, cap and shape caches. */
		CoreExport void		InvalidateGeomCache();
		/*! \remarks This method is used internally. */
		CoreExport void		InvalidateCapCache();
				
		// functions for use in data flow evaluation
		/*! \remarks This method is used internally in data flow evaluation */
		CoreExport void 	ShallowCopy(BezierShape *ashape, ChannelMask channels);
		/*! \remarks This method is used internally in data flow evaluation */
		CoreExport void 	DeepCopy(BezierShape *ashape, ChannelMask channels);
		/*! \remarks This method is used internally in data flow evaluation */
		CoreExport void	NewAndCopyChannels(ChannelMask channels);
		/*! \remarks This method is used internally in data flow evaluation */
		CoreExport void 	FreeChannels(ChannelMask channels, int zeroOthers=1);

		// Display flags
		/*! \remarks Sets the state of the specified display flags.
		\par Parameters:
		<b>DWORD f</b>\n\n
		The flags to set. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_beziershape_display_flags.html">List of
		BezierShape Display Flags</a>. */
		CoreExport void		SetDispFlag(DWORD f);
		/*! \remarks Returns the state of the specified display flags.
		\par Parameters:
		<b>DWORD f</b>\n\n
		The flags to get. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_beziershape_display_flags.html">List of
		BezierShape Display Flags</a>. */
		CoreExport DWORD	GetDispFlag(DWORD f);
		/*! \remarks Clears the specified display flags.
		\par Parameters:
		<b>DWORD f</b>\n\n
		The flags to clear. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_beziershape_display_flags.html">List of
		BezierShape Display Flags</a>. */
		CoreExport void		ClearDispFlag(DWORD f);
		/*! \remarks Constructs a vertex selection list based on the current selection
		level for the specified spline. For example if the selection level is at object
		level all the bits are set. If the selection level is at vertex level only the
		selected vertex bits are set. See Class BitArray.
		\par Parameters:
		<b>int poly</b>\n\n
		The index into the <b>splines</b> data member.\n\n
		<b>int level = -1</b>\n\n
		This allows a selction level to be optionally specified. For example if the
		selection level is at object level all the bits are set. If the selection level
		is at vertex level only the selected vertex bits are set. One of the following values:\n\n
		<b>SHAPE_OBJECT</b>\n\n
		<b>SHAPE_SPLINE</b>\n\n
		<b>SHAPE_SEGMENT</b>\n\n
		<b>SHAPE_VERTEX</b> */
		CoreExport BitArray VertexTempSel(int poly, int level = -1);
		// Constructs a vertex selection list for all polys based on the current selection level
		// This is a bitarray with a bit for each vertex in each poly in the shape.  If
		// includeVecs is set, it will set the bits for vectors associated with selected knots.
		// Specify a poly number or -1 for all.  Can specify selection level optionally
		/*! \remarks This method looks at the selection state for the selection level
		and sets the appropriate bits based on the selection set <b>unless</b>
		<b>forceSel</b> is TRUE. If <b>forceSel</b> is TRUE, it acts as if every item
		in the selection set for the specified spline was set. It's an easy way to
		select entire splines.\n\n
		See Class BitArray.
		\par Parameters:
		<b>int poly = -1</b>\n\n
		The index into the <b>splines</b> data member (-1 means use all).\n\n
		<b>BOOL includeVecs = FALSE</b>\n\n
		If set, this method will set the bits for vectors associated with selected
		knots.\n\n
		<b>int level = 0</b>\n\n
		This allows a selction level to be optionally specified. For example if the
		selection level is at object level all the bits are set. If the selection level
		is at vertex level only the selected vertex bits are set. One of the following values:\n\n
		<b>SHAPE_OBJECT</b>\n\n
		<b>SHAPE_SPLINE</b>\n\n
		<b>SHAPE_SEGMENT</b>\n\n
		<b>SHAPE_VERTEX</b>\n\n
		<b>BOOL forceSel = FALSE</b>\n\n
		If TRUE, it acts as if every item in the selection set for the specified spline
		was set. It's an easy way to select entire splines. For example, selecting
		spline 2 regardless of the spline-level selection state:\n\n
		<b>VertexTempSelAll(2, FALSE, SHAPE_SPLINE, TRUE);</b>\n\n
		This just builds the selection set as if spline 2 was completely selected.
		Otherwise, the spline bits would have only been selected if	BezierShape::polySel.sel[2] was set.\n\n
		Incidentally, the following calls will do exactly the same thing:\n\n
		<b>VertexTempSelAll(2, FALSE, SHAPE_VERTEX, TRUE);</b>\n\n
		<b>VertexTempSelAll(2, FALSE, SHAPE_SEGMENT, TRUE);</b>\n\n
		For what it's worth, the SHAPE_SPLINE version is the most efficient. */
		CoreExport BitArray	VertexTempSelAll(int poly = -1, BOOL includeVecs = FALSE, int level = 0, BOOL forceSel = FALSE);

		//! \brief Constructs a vertex selection list of the specified poly.
		/*! \remarks Constructs a vertex selection list of the specified poly based on the 
			setting of the specified flag on the vertex. The implementation calls GetFlag() 
			on each SplineKnotAssy checking the specified flag value. 
			\param poly - Specifies the polygon to construct the list from.
			\param flag - The flag value each vertex will be tested for. */
		CoreExport BitArray VertexFlagSel(int poly, DWORD flag);

		/*! \remarks Saves the shape data to the .MAX file. */
		CoreExport IOResult Save(ISave* isave);
		/*! \remarks Loads the shape data from the .MAX file. */
		CoreExport IOResult Load(ILoad* iload);

		// BezierShape-specific methods

		/*! \remarks Returns the number of splines in this shape. */
		inline int SplineCount() { return splineCount; }
		/*! \remarks Returns a pointer to the spline specified by the index
		passed.
		\par Parameters:
		<b>int index</b>\n\n
		Specifies which spline to return. This is an index into the
		<b>splines</b> data member. */
		CoreExport Spline3D* GetSpline(int index);
		/*! \remarks Creates and adds an empty spline to the shape.
		\par Parameters:
		<b>int itype = KTYPE_CORNER</b>\n\n
		The initial knot type you get when you click and release the mouse
		during spline creation. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_spline_knot_types.html">List of Knot
		Types</a>.\n\n
		<b>int dtype = KTYPE_BEZIER</b>\n\n
		The drag knot type used when you click and drag to create a vertex
		during spline creation. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_spline_knot_types.html">List of Knot
		Types</a>.\n\n
		<b>int ptype = PARM_UNIFORM</b>\n\n
		This parameter is not used. Let it default to <b>PARM_UNIFORM</b>.
		\return  A pointer to the newly created spline. */
		CoreExport Spline3D* NewSpline(int itype = KTYPE_CORNER,int dtype = KTYPE_BEZIER,int ptype = PARM_UNIFORM);
		/*! \remarks Add an existing spline to this shape as the last one in
		the list. Note that this copies only the pointer, it <b>does not</b>
		copy the entire spline, so <b>do not</b> delete the spline anywhere
		else. This will do it when it's done with it.
		\par Parameters:
		<b>Spline3D* spline</b>\n\n
		The spline to add.
		\return  A pointer to the spline passed or NULL if the call failed. */
		CoreExport Spline3D* AddSpline(Spline3D* spline);
		/*! \remarks Deletes the specified spline.
		\par Parameters:
		<b>int index</b>\n\n
		The index into the splines data member.
		\return  Nonzero on success; otherwise zero. */
		CoreExport int DeleteSpline(int index);
		/*! \remarks Inserts the specified spline into the spline list at the
		location passed.
		\par Parameters:
		<b>Spline3D* spline</b>\n\n
		The spline to add.\n\n
		<b>int index</b>\n\n
		The index in to the splines data member indicating where to insert the
		spline.
		\return  Nonzero on success; otherwise zero. */
		CoreExport int InsertSpline(Spline3D* spline, int index);
		/*! \remarks This method deletes every spline in the shape. */
		CoreExport void NewShape();
		/*! \remarks Returns the total number of vertices in the entire shape.
		*/
		CoreExport int GetNumVerts();
		/*! \remarks Returns the total number of segments in the entire shape.
		*/
		CoreExport int GetNumSegs();
		/*! \remarks Computes the bounding box of this shape.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		This parameter is not used.\n\n
		<b>Box3\& box</b>\n\n
		The result is stored here.\n\n
		<b>Matrix3 *tm</b>\n\n
		The points of each spline in this shape are deformed using this
		matrix.\n\n
		<b>BOOL useSel</b>\n\n
		If TRUE the box is computed about the selected vertices only; otherwise
		all points. */
		CoreExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );
		/*! \remarks This is a very important call to make. When you are done
		adding polygons to the shape you should call this method. This method
		updates a set of embedded selection set data within the shape. This
		selection set data tells what polygons are selected, what segments are
		selected, and what control points are selected. This resets the sizes
		of the selection bit arrays for this shape.
		\par Parameters:
		<b>BOOL save = FALSE</b>\n\n
		This parameter is available in release 4.0 and later only.\n\n
		This optional parameter preserves the selection set information when
		set to TRUE, and erases it when set to FALSE. The default, FALSE, is
		present for backward compatibility, where there was no argument. */
		CoreExport void UpdateSels(BOOL save=FALSE);
		/*! \remarks For each spline in this shape, this method sets the
		corresponding bit if the shape is closed and clears the bit is the
		shape is open.
		\par Parameters:
		<b>BitArray\& array</b>\n\n
		The BitArray to update. */
		CoreExport void GetClosures(BitArray& array);
		/*! \remarks Sets the closed state of each spline in this shape based
		on the BitArray passed.
		\par Parameters:
		<b>BitArray\& array</b>\n\n
		Indicates which shapes should be closed: 1 = closed; 0 = open. */
		CoreExport void SetClosures(BitArray& array);
		/*! \remarks This is used for hit testing. This method returns a value
		from 0.0 to 1.0 that tells you where a hit was found on a particular
		segment. Sample code that uses this is the edit spline modifier in
		<b>/MAXSDK/SAMPLES/MODIFIERS/EDITSPL.H</b>. This allows a refinement of
		the hit testing.
		\par Parameters:
		<b>int poly</b>\n\n
		The index of the spline.\n\n
		<b>int segment</b>\n\n
		The index of the segment.\n\n
		<b>GraphicsWindow *gw</b>\n\n
		The graphics window where the hit test was done.\n\n
		<b>Material *ma</b>\n\n
		The list of materials.\n\n
		<b>HitRegion *hr</b>\n\n
		The hit region. See Class HitRegion.\n\n
		<b>int ptype = PARAM_SIMPLE</b>\n\n
		This parameter is available in release 4.0 and later only.\n\n
		This allows the caller to get the location on the segment in either
		parameter space or normalized distance space. Both return values of
		0-1. For proper backward compatibility, the default parameter type is
		<b>PARAM_SIMPLE</b>. The other option is <b>PARAM_NORMALIZED</b>. */
		CoreExport float FindSegmentPoint(int poly, int segment, GraphicsWindow *gw, Material *ma, HitRegion *hr, int ptype = PARAM_SIMPLE);
		/*! \remarks Reverses the spline whose index is passed.
		\par Parameters:
		<b>int poly</b>\n\n
		The spline to reverse.\n\n
		<b>BOOL keepZero = FALSE</b>\n\n
		This optional parameter is available in release 2.0 and later only.\n\n
		This parameter defaults to FALSE in order to retain backwards
		compatibility. Setting it to TRUE insures that a closed spline will
		have the same vertex as its first point when it is reversed. The
		parameter is ignored on open splines. */
		CoreExport void Reverse(int poly, BOOL keepZero = FALSE);
		/*! \remarks Reverses the splines of this shape if the corresponding
		bit in <b>reverse</b> is set.
		\par Parameters:
		<b>BitArray \&reverse</b>\n\n
		If the bit is set the spline is reversed; otherwise it is left
		alone.\n\n
		<b>BOOL keepZero = FALSE</b>\n\n
		This optional parameter is available in release 2.0 and later only.\n\n
		This parameter defaults to FALSE in order to retain backwards
		compatibility. Setting it to TRUE insures that a closed spline will
		have the same vertex as its first point when it is reversed. The
		parameter is ignored on open splines. */
		CoreExport void Reverse(BitArray &reverse, BOOL keepZero = FALSE);
		/*! \remarks This methods looks at the shape organization, and puts
		together a shape hierarchy. This provides information on how the shapes
		are nested. For example on a donut object with two circles, this method
		determines which circle is inside the other one.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		This parameter is not used.\n\n
		<b>ShapeHierarchy *hier = NULL</b>\n\n
		If non-NULL the result is store here (in addition to being returned).
		See Class ShapeHierarchy.
		\return  The result is returned here. */
		CoreExport ShapeHierarchy &OrganizeCurves(TimeValue t, ShapeHierarchy *hier = NULL);
		/*! \remarks Makes a <b>PolyShape</b> from this shape.
		\par Parameters:
		<b>PolyShape \&pshp</b>\n\n
		The results are stored here.\n\n
		<b>int steps = -1</b>\n\n
		The number of steps between knots in the spline.\n\n
		<b>BOOL optimize = FALSE</b>\n\n
		If TRUE, linear segments between control points in the spline will not generate
		steps in between. It will just be one line segment. */
		CoreExport void MakePolyShape(PolyShape &pshp, int steps = -1, BOOL optimize = FALSE);
		/*! \remarks Sets the specified vertex of the specified poly as the
		first vertex. On an open polygon this has to be one of the end control
		points. On a closed shape it doesn't matter.
		\par Parameters:
		<b>int poly</b>\n\n
		The poly to update.\n\n
		<b>int vertex</b>\n\n
		The vertex to make first. */
		CoreExport void MakeFirst(int poly, int vertex);
		/*! \remarks Transforms the points of each poly in the shape by the
		matrix passed.
		\par Parameters:
		<b>Matrix3 \&tm</b>\n\n
		The transformation matrix. */
		CoreExport void Transform(Matrix3 &tm);
		/*! \remarks This lets you add another shape to this one. */
		CoreExport BezierShape& operator+=(BezierShape& from);
		/*! \remarks		This method is used for adding the splines from one BezierShape to
		another, with a weld threshold that will weld endpoints of the new
		splines onto endpoints of existing splines. Calling this method will
		cause the splines of the "from" shape to be added to those of the
		shape. If any endpoints in the "from" shape are within the specified
		weld threshold, they will be automatically welded.
		\par Parameters:
		<b>BezierShape \&from</b>\n\n
		The shape whose splines are added.\n\n
		<b>float weldThreshold</b>\n\n
		The endpoint weld threshold. */
		CoreExport void AddAndWeld(BezierShape &from, float weldThreshold);
		/*! \remarks This method is used internally. */
		CoreExport void ReadyCachedPolyShape();
		/*! \remarks This method is passed a capping information structure and
		it will compute the information it needs to make a mesh cap for this shape.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		This parameter is not used.\n\n
		<b>MeshCapInfo \&capInfo</b>\n\n
		The capping information. See Class MeshCapInfo.\n\n
		<b>int capType</b>\n\n
		See \ref capTypes.
		\return  Nonzero if the method succeeded; otherwise zero. */
		CoreExport int MakeCap(TimeValue t, MeshCapInfo &capInfo, int capType);
		/*! \remarks This method is passed a capping information structure and
		it will compute the information it needs to make a patch cap for this
		shape.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		This parameter is not used.\n\n
		<b>PatchCapInfo \&capInfo</b>\n\n
		The capping information. See Class PatchCapInfo.
		\return  Nonzero if the method succeeded; otherwise zero. */
		CoreExport int MakeCap(TimeValue t, PatchCapInfo &capInfo);
		/*! \remarks This method is used internally by the BezierShape code.
		When you call <b>BezierShape::MakeCap(TimeValue t, PatchCapInfo
		\&capInfo)</b>, if the patch cap data isn't cached, this method is
		called to build it. Calling it is not normally necessary, or a good
		idea -- it builds the PatchCapInfo data structure regardless of whether
		it's cached or not. Just call the <b>MakeCap()</b> method to get the
		cap information and the caching is done automatically. */
		CoreExport int ReadyPatchCap();
		
		// The following copies the shapes, selection sets and any caches from the source object.
		// It does NOT copy selection level info or display info.
		/*! \remarks This method copies the shapes, selection sets and any
		caches from the source object. It does <b>not</b> copy selection level,
		or display information.
		\par Parameters:
		<b>BezierShape \&fromShape</b>\n\n
		The shape to copy from. */
		CoreExport void CopyShapeDataFrom(BezierShape &fromShape);

		// The following methods provide an easy way to derive a simple index for any
		// vert in any spline in the shape, and turn that index back into a poly/vert pair
		/*! \remarks This method is used internally. */
		CoreExport void PrepVertBaseIndex();	// Used internally automatically
		/*! \remarks This method provides an easy way to derive a simple index
		for any vertex in any spline in the shape.
		\par Parameters:
		<b>int poly</b>\n\n
		The zero based index of the spline.\n\n
		<b>int vert</b>\n\n
		The zero based index of the vertex.
		\return  A zero based index for vertex. */
		CoreExport int GetVertIndex(int poly, int vert);
		/*! \remarks This method takes a vertex index and turns it back into a
		poly / vertex pair (see <b>GetVertIndex()</b> above).
		\par Parameters:
		<b>int index</b>\n\n
		The input index.\n\n
		<b>int \&polyOut</b>\n\n
		The output poly.\n\n
		<b>int \&vertOut</b>\n\n
		The output vertex. */
		CoreExport void GetPolyAndVert(int index, int &polyOut, int &vertOut);
		/*! \remarks Returns the total number of verticies in the shape. */
		CoreExport int GetTotalVerts();	// Total number of verts in the shape
		CoreExport Point3	GetVert( int index ); // returns a point using global indexing scheme
		CoreExport void		SetVert( int index , const Point3 &xyz); // sets a point using global indexing scheme

		// The following methods provide an easy way to derive a simple index for any
		// knot in any spline in the shape, and turn that index back into a poly/knot pair
		/*! \remarks This method is used internally, automatically. */
		CoreExport void PrepKnotBaseIndex();	// Used internally automatically
		/*! \remarks This method returns an index for any knot in any spline
		in the shape.
		\par Parameters:
		<b>int poly</b>\n\n
		The input poly number.\n\n
		<b>int knot</b>\n\n
		The input know number.
		\return  The zero based index of the knot. */
		CoreExport int GetKnotIndex(int poly, int knot);
		/*! \remarks This method computes a poly / knot pair from an index
		(see <b>GetKnotIndex()</b> above).
		\par Parameters:
		<b>int index</b>\n\n
		The input knot index.\n\n
		<b>int \&polyOut</b>\n\n
		The output index of the poly it is a part of.\n\n
		<b>int \&knotOut</b>\n\n
		The output knot number. */
		CoreExport void GetPolyAndKnot(int index, int &polyOut, int &knotOut);
		/*! \remarks Returns the total number of knots in the shape. */
		CoreExport int GetTotalKnots();	// Total number of knots in the shape

		// The following functions delete the selected items, returning TRUE if any were
		// deleted, or FALSE if none were deleted.
		/*! \remarks Deletes the selected vertices for the specified poly in
		the shape.
		\par Parameters:
		<b>int poly</b>\n\n
		The zero based index of the polygon.
		\return  TRUE if any were deleted; FALSE if none were deleted. */
		CoreExport BOOL DeleteSelVerts(int poly);	// For single poly
		/*! \remarks Deletes the selected polygons for the specified poly in
		the shape.
		\par Parameters:
		<b>int poly</b>\n\n
		The zero based index of the polygon.
		\return  TRUE if any were deleted; FALSE if none were deleted. */
		CoreExport BOOL DeleteSelSegs(int poly);	// For single poly
		/*! \remarks Deletes the selected vertices for all polys in the shape.
		\return  TRUE if any were deleted; FALSE if none were deleted. */
		CoreExport BOOL DeleteSelectedVerts();	// For all polys
		/*! \remarks Deletes the selected segments for all polys in the shape.
		\return  TRUE if any were deleted; FALSE if none were deleted. */
		CoreExport BOOL DeleteSelectedSegs();
		/*! \remarks Deletes the selected polygons for all polys in the shape.
		\return  TRUE if any were deleted; FALSE if none were deleted. */
		CoreExport BOOL DeleteSelectedPolys();

		// Copy the selected geometry (segments or polys), reversing if needed.
		// Returns TRUE if anything was copied
		/*! \remarks Copies the selected geometry (segments or polys),
		reversing if needed.
		\par Parameters:
		<b>BOOL reverse=FALSE</b>\n\n
		TRUE to reverse; FALSE to leave alone.
		\return  Returns TRUE if anything was copied. */
		CoreExport BOOL CloneSelectedParts(BOOL reverse=FALSE);

		// Tag the points in the spline components to record our topology (This stores
		// identifying values in the Spline3D's Knot::aux fields for each control point)
		// This info can be used after topology-changing operations to remap information
		// tied to control points.
		// Returns FALSE if > 32767 knots or polys (can't record that many)
		// 
		// MAXr3: Optional channel added.  0=aux2, 1=aux3
		/*! \remarks Tags the points in the spline components to record the
		topology of the shape. (This stores identifying values in the
		Spline3D's Knot::aux fields for each control point). This info can be
		used after topology-changing operations to remap information tied to
		control points.
		\par Parameters:
		<b>int channel=0</b>\n\n
		This parameter is available in release 3.0 and later only.\n\n
		Specifies which auxiliary channel. One of the following values:\n\n
		<b>0</b>=aux2\n\n
		<b>1</b>=aux3
		\return  Returns FALSE if \> 32767 knots or polys (it can't record that
		many). */
		CoreExport BOOL RecordTopologyTags(int channel=0);

		// Support for interpolating along the shape's splines
		/*! \remarks This method returns a point interpolated on the specified
		spline on the entire curve. This method returns the point but you don't
		know which segment the point falls on. See method
		<b>InterpPiece3D()</b>.
		\par Parameters:
		<b>int poly</b>\n\n
		The zero based index of the spline.\n\n
		<b>float param</b>\n\n
		The position along the curve to return where 0 is the start and 1 is
		the end.\n\n
		<b>int ptype=PARAM_SIMPLE</b>\n\n
		The parameter type for spline interpolation. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_ptypes_for_shape.html">List of Parameter Types
		for Shape Interpolation</a>.
		\return  The interpolated point on the curve. */
		CoreExport Point3 InterpCurve3D(int poly, float param, int ptype=PARAM_SIMPLE);
		/*! \remarks This method returns a tangent vector interpolated on the
		entire curve of the specified spline. Also see method
		<b>TangentPiece3D()</b>.
		\par Parameters:
		<b>int poly</b>\n\n
		The zero based index of the spline.\n\n
		<b>float param</b>\n\n
		The position along the curve to return where 0 is the start and 1 is
		the end.\n\n
		<b>int ptype=PARAM_SIMPLE</b>\n\n
		The parameter type for spline interpolation. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_ptypes_for_shape.html">List of Parameter Types
		for Shape Interpolation</a>.
		\return  The tangent vector */
		CoreExport Point3 TangentCurve3D(int poly, float param, int ptype=PARAM_SIMPLE);
		/*! \remarks This method returns the interpolated point along the
		specified sub-curve (segment) for the specified spline. For example
		consider a shape that is a single circle with four knots. If you called
		this method with curve=0 and piece=0 and param=0.0 you'd get back the
		point at knot 0. If you passed the same parameters except param=1.0
		you'd get back the point at knot 1.
		\par Parameters:
		<b>int poly</b>\n\n
		The zero based index of the spline.\n\n
		<b>int piece</b>\n\n
		The sub-curve (segment) to evaluate.\n\n
		<b>float param</b>\n\n
		The position along the curve to return where 0 is the start and 1 is
		the end.\n\n
		<b>int ptype=PARAM_SIMPLE</b>\n\n
		The parameter type for spline interpolation. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_ptypes_for_shape.html">List of Parameter Types
		for Shape Interpolation</a>.
		\return  The point in world space. */
		CoreExport Point3 InterpPiece3D(int poly, int piece, float param, int ptype=PARAM_SIMPLE);
		/*! \remarks Returns the tangent vector of the specified spline on a
		sub-curve at the specified 'distance' along the curve.
		\par Parameters:
		<b>int poly</b>\n\n
		The zero based index of the spline.\n\n
		<b>int piece</b>\n\n
		The sub-curve (segment) to evaluate.\n\n
		<b>float param</b>\n\n
		The position along the curve to return where 0 is the start and 1 is
		the end.\n\n
		<b>int ptype=PARAM_SIMPLE</b>\n\n
		The parameter type for spline interpolation. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_ptypes_for_shape.html">List of Parameter Types
		for Shape Interpolation</a>. */
		CoreExport Point3 TangentPiece3D(int poly, int piece, float param, int ptype=PARAM_SIMPLE);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the material ID for the specified spline and segment of this
		shape.
		\par Parameters:
		<b>int poly</b>\n\n
		The zero based index of the spline.\n\n
		<b>int piece</b>\n\n
		The zero based index of the segment of the spline. */
		CoreExport MtlID GetMatID(int poly, int piece);
		/*! \remarks Returns the length of the specified spline.
		\par Parameters:
		<b>int poly</b>\n\n
		The index of the spline to check. */
		CoreExport float LengthOfCurve(int poly);

		// Get information on shape topology
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Retrieves information on the shape topology.
		\par Parameters:
		<b>BezierShapeTopology \&topo</b>\n\n
		The object which is updated with the shape data. See
		Class BezierShapeTopology. */
		CoreExport void GetTopology(BezierShapeTopology &topo);

		// Perform a trim or extend
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method provides a way for a BezierShape to trim and extend
		splines. This method is meant to perform as part of a mouse-centered
		operation -- Click on the part of the spline and pass the hitrecord,
		viewport and mouse point to the trim function.
		\par Parameters:
		<b>IObjParam *ip</b>\n\n
		The interface pointer. See Class Interface.\n\n
		<b>ViewExp *vpt</b>\n\n
		The viewport the user clicked in. See
		Class ViewExp.\n\n
		<b>ShapeHitData *hit</b>\n\n
		The hit record for the selection. See
		Class ShapeHitData.\n\n
		<b>IPoint2 \&m</b>\n\n
		The point the user clicked on in the viewport. See
		Class IPoint2.\n\n
		<b>ShapeContextCallback \&cb</b>\n\n
		The callback object. See
		Class ShapeContextCallback.\n\n
		<b>int trimType</b>\n\n
		Specifies if the operation is a Trim or an Extend. One of the following
		values:\n\n
		<b>SHAPE_TRIM</b>\n\n
		<b>SHAPE_EXTEND</b>\n\n
		<b>int trimInfinite</b>\n\n
		This is set to TRUE for infinite projections.
		\return  This returns TRUE if the trim or extend was performed. */
		CoreExport BOOL PerformTrimOrExtend(IObjParam *ip, ViewExp *vpt, ShapeHitData *hit, IPoint2 &m, ShapeContextCallback &cb, int trimType, int trimInfinite);

		CoreExport BOOL SelVertsSameType();	// Are all selected vertices the same type?
		CoreExport BOOL SelSegsSameType();	// Are all selected segments the same type?
		CoreExport BOOL SelSplinesSameType();	// Are all segments in selected splines the same type?

//watje 1-10-98 additonal changes

		Tab<bindShape> bindList;		//list of bind points
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method binds a knot to a segment. A bind acts as a constraint, it
		constrains the first point or the end point of a spline to the mid
		point of a segment.
		\par Parameters:
		<b>BOOL isEnd</b>\n\n
		Specifies whether the first or last point is bound. TRUE for the end;
		FALSE for the start.\n\n
		<b>int segIndex</b>\n\n
		The index of the segment to be bound to.\n\n
		<b>int splineSegID</b>\n\n
		The index of the spline that contains the segment.\n\n
		<b>int splinePointID</b>\n\n
		The index of spline that is being bound. */
		CoreExport void BindKnot(BOOL isEnd, int segIndex, int splineSegID, int splinePointID); // binds a knot to a seg ment
		/*! \remarks		This method unbinds the specified spline.
		\par Parameters:
		<b>int splineID</b>\n\n
		The index of spline that is being bound.\n\n
		<b>BOOL isEnd</b>\n\n
		Specifies whether the first or last point is unbound. TRUE for the end; FALSE	for the start. */
		CoreExport BOOL UnbindKnot(int splineID, BOOL isEnd);  //unbind a knot 
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method needs to be called when the topology changes to update the
		bind list.
		\par Parameters:
		<b>BOOL useGeomTracking=FALSE</b>\n\n
		This parameter is available in release 4.0 and later only.\n\n
		This allows the update to be dependant on previous recorded topology or
		geometry. If this flag is set to TRUE it uses the old method of using
		the geometry to rebuild the binds, else it uses the new method which
		uses the aux flags of the splines which store the old topology indices.
		Normally this will be FALSE since most of the times the topology
		tracking is more accurate, it should be set to TRUE when there is no
		initial topology to work from for instance when attaching or detaching
		geometry. */
		CoreExport void UpdateBindList(BOOL useGeometricTracking = FALSE);	//when topology changes this needs to be called to update the bind list
		/*! \remarks		Hides the selected segments.
		\return  TRUE if any were hidden. FALSE if none were hidden. */
		CoreExport BOOL HideSelectedSegs();  // hide selected segs
		/*! \remarks		Hides the segments attached to the selected vertices.
		\return  TRUE if any were hidden. FALSE if none were hidden. */
		CoreExport BOOL HideSelectedVerts(); // hide segs attached to selected verts
		/*! \remarks		Hides the segments attached to the selected splines.
		\return  TRUE if any were hidden. FALSE if none were hidden. */
		CoreExport BOOL HideSelectedSplines(); // hide segs attached to selected splines
		/*! \remarks		Unhides all the segments in the shape.
		\return  TRUE if any were unhidden. FALSE if none were unhidden. */
		CoreExport BOOL UnhideSegs();          //unhide all segs

		CoreExport int  UnselectHiddenVerts();
		CoreExport int  UnselectHiddenSegs();
		CoreExport int  UnselectHiddenSplines();

		float * mpVertexWeights;
		int numVertexWeights;
		int   * mpVertexEdgeDists;
		float * mpVertexDists;

		public:
		CoreExport void  SetVertexWeightCount( int i ); // destroys existing weights, sets all weights = 0.0.
		CoreExport void  SetVertexWeight( int i, float w ) { assert( mpVertexWeights ); if ( i >= numVertexWeights ) return; mpVertexWeights[i] = w; }
		CoreExport float VertexWeight( int i ) { if ( !mpVertexWeights ) return 0.0f; if ( i >= numVertexWeights ) return 0.0f; return mpVertexWeights[i]; }
		CoreExport bool  VertexWeightSupport() { if ( mpVertexWeights ) return true; return false; }
		CoreExport float *GetVSelectionWeights() { return mpVertexWeights; }
		CoreExport void SupportVSelectionWeights();		// Allocate a weight table if none 

        float mFalloff, mPinch, mBubble;
		int   mEdgeDist, mUseEdgeDist, mAffectBackface, mUseSoftSelections;

		CoreExport int  UseEdgeDists( );
		CoreExport void SetUseEdgeDists( int edgeDist );

		CoreExport int  EdgeDist( );
		CoreExport void SetEdgeDist( int edgeDist );

		CoreExport int  UseSoftSelections();
		CoreExport void SetUseSoftSelections( int useSoftSelections );

		CoreExport int AffectBackface( );
		CoreExport void SetAffectBackface( int affectBackface );

		CoreExport float Falloff( );
		CoreExport void SetFalloff( float falloff );

		CoreExport float Pinch( );
		CoreExport void SetPinch( float pinch );

		CoreExport float Bubble( );
		CoreExport void SetBubble( float bubble );

		CoreExport void InvalidateVertexWeights();

		CoreExport void UpdateVertexDists();
		CoreExport void UpdateEdgeDists( );
		CoreExport void UpdateVertexWeights();

		//! \brief Delete multiple splines at once.
		/*!	Deleting multiple splines at once is faster than deleting one by one by calling DeleteSpline(), because the internal bookkeeping only needs to be done once.

				\param[in] indices Point to an array of indices to delete.  Indices should be unique, and sorted in ascending order.
				\param[in] n Number of entries in the indices array.
				\return false if any of the vertex indices are invalid.
		*/
		CoreExport bool DeleteSplines(const unsigned int* indices, unsigned int n);
	};

