//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//

#pragma once

#include "maxheap.h"
#include "shphier.h"
#include "captypes.h"
#include "shpsels.h"	// Shape selection classes
#include "templt.h"
#include "maxtypes.h"
#include "channels.h" // for ChannelMask typedef
#include "TabTypes.h" // cannot forward declare IntTab
#include "GraphicsConstants.h"

// forward declarations
class GraphicsWindow;
class Material;
class HitRegion;
struct SnapInfo;
class IPoint2;


/// \defgroup Snap_flags Snap Flags
//@{
#define PSHAPE_SNAP_IGNORELAST	(1<<0)
#define PSHAPE_SNAP_NOEDGES		(1<<1)
//@}


/** \defgroup Parameter_types_for_shape_interpolation Parameter Types for Shape Interpolation*/
// (Must match interpolation types in object.h)
//@{
#define POLYSHP_INTERP_SIMPLE 0		//!< Parameter space based on segments
#define POLYSHP_INTERP_NORMALIZED 1	//!< Parameter space normalized to curve length
//@}

// Flags for sub object hit test

class Spline3D;

//--------------------------------------------------------------
// Capping classes, etc.

/// \defgroup CapVert_flags CapVert Flags
//@{
#define CAPVERT_VISEDGE (1<<0)
//@}

class CapVert: public MaxHeapOperators {
	public:
		int vert;
		DWORD flags;
		float ang;
		CapVert *prev,*next;
		CapVert *smaller,*bigger;
		CapVert() { vert=0; flags = 0; ang = 0.0f; prev=next=smaller=bigger = NULL; }
	};


/** \defgroup Predefined_PolyPt_flags Predefined PolyPt Flags
 * Developers can use bits 0-7 of the flags for their own use. 
 * Bits 8 and above are used by the system. If you are converting 
 * some other type of shape or spline to a PolyShape (and thus 
 * PolyLines) you can set certain flags to make things operate
 * smoother. These flags are described here.*/
//@{
/** Indicates if this point in the PolyLine corresponds to a knot in
 * the original spline. For example if you had a circle that was a bezier spline
 * it should have four of the points in the PolyLine designated as
 * POLYPT_KNOT. These are the points at the 12 o'clock, 3 o'clock, 6
 * o'clock and 9 o'clock positions. Then all the other points would be
 * POLYPT_INTERPOLATED. This is used to make capping more efficient, for
 * example, the system generally tries to attach to a knot when making 
 * connections between polygons.*/
#define POLYPT_KNOT 		(1<<8)	

/** Indicates that the point is an interpolated point and not a knot. 
 * Can be  used to make capping more efficient.
 * Use it if your class converts to a PolyLine. For example, the system 
 * generally tries to attach to a knot when making connections between polygons*/
#define POLYPT_INTERPOLATED (1<<9)	

/** Indicates that the point is part of a smooth transition. If you convert
 * to a PolyLine, use this bit to control smoothing of the resulting shape. 
 * Having this bit set to 1 means that any mesh generated will share smoothing 
 * across the edge. For example, all the points on a curved section of a 
 * spline between knots will have this flag set. Depending on
 * the knot type (if it's a corner or bezier corner) this smooth flag might
 * not be set. If this shape is then extruded or lofted this information is used
 * to determine smoothing groups. As a conclusion, if this flag is set it will 
 * generate a smooth transition. If this bit is not set, it will generate a sharp corner.*/
#define POLYPT_SMOOTH		(1<<10) 

/** The segment that starts with this point is selected. This is used in the
 * drawing routines so that PolyShapes generated from BezierShapes will still have
 * the selected segments drawn in the selected color.*/
#define POLYPT_SEG_SELECTED	(1<<11)	

#define POLYPT_BRIDGE		(1<<16)	//!< \internal Span between two polygons. Used internally by capping code.
#define POLYPT_SPLICE		(1<<17)	//!< \internal Point is endpoint of a bridge. Used internally by capping code.
#define POLYPT_VISEDGE		(1<<18)	//!< \internal Segment should be visible on mesh. Used internally by capping code.
#define POLYPT_NO_SPLICE	(1<<19)	//!< \internal Don't allow a bridge at this point. Used internally by capping code.
#define POLYPT_INVIS_EDGE	(1<<20)	//!< \internal Force segment to be invisible on capping. Used internally by capping code
#define POLYPT_NO_SNAP		(1<<21) //!< \internal Suppress snapping when set. Used internally by capping code.
//@}

/** \defgroup Predefined_PolyPt_flags2 Predefined PolyPt flags2
 * Flags2 field in PolyPt contains material IDs. They are stored in the HIWORD of this flag. */
//@{
#define POLYPT_MATID_SHIFT	16		
#define POLYPT_MATID_MASK	0xFFFF	
//@}


/** Represents a single point of a PolyLine. All methods of this
 * class are implemented by the system. 
 * \see PolyLine*/
class PolyPt: public MaxHeapOperators {
	public:
		Point3 p;		//!< The location of the point
		DWORD flags;	//!< See \ref Predefined_PolyPt_flags
		DWORD flags2;	//!< See \ref Predefined_PolyPt_flags2
		int aux;		//!< Auxiliary data attached to this point (usually mesh vertex number for capping)
		
		/** Constructor. All data members are initiated to their corresponding zero values. */
		PolyPt() { p = Point3(0,0,0); flags = 0; flags2 = 0; aux = 0; }

		/** Constructor. All data members are initialized to the input data. */
		PolyPt(Point3 ip, DWORD f=0, int a=0, DWORD f2=0) { p = ip; flags = f; aux = a; flags2 = f2;}
		
		/** Gets the material ID on a per-segment basis within the
		 * spline or polyline. */
		inline	MtlID	GetMatID() {return (int)((flags2>>POLYPT_MATID_SHIFT)&POLYPT_MATID_MASK);}
		
		/** Sets the material ID on a per-segment basis within the
		 * spline or polyline.
		 * \param id The ID to set to. */
		inline	void    SetMatID(MtlID id) {flags2 &= 0xFFFF; flags2 |= (DWORD)(id<<POLYPT_MATID_SHIFT);}
	};


/// \internal \defgroup Option_for_PolyLine__Cap3DS___PolyShape__Make3DSCap Option for PolyLine::Cap3DS / PolyShape::Make3DSCap
//@{
#define CAP3DS_OPT_CLOSEST_BRIDGE (1<<0)	//!< Bridge polys at closest point
//@}


/// \defgroup PolyLine_Flags PolyLine Flags
//@{
/** Indicates the polyline is closed.*/
#define POLYLINE_CLOSED			(1<<0)		 
/** Ignore self-intersections. This is used internally in the patch capping to
indicate that this line always returns FALSE from the self intersects test.
Normally this should not be set.
*/
#define POLYLINE_NO_SELF_INT	(1<<1)		 
//@}

/** Describes a single polygon in a PolyShape using linear
 * segments. All methods of this class are implemented by the system.
\par Method Groups:
 * \see PolyPt, PolyShape
 */
class PolyLine: public MaxHeapOperators {
	public:
		int numPts;			//!< The number of points in the polyline.
		PolyPt *pts;		//!< Pointer to the actual points in the polyline.
		DWORD flags;		//!< See \ref PolyLine_Flags
		Box3 bdgBox;		//!< The bounding box of the polyline.
		float cachedLength;	//!< The length of the polyline.
		float *lengths;		//!< Cached lengths for each point
		float *percents;	//!< Cached percentages for each point
		BOOL cacheValid;	//!< Indicates if the cache is valid.

		/** Constructor. The number of points is set to zero, the
		 * points array is set to NULL, the flags are set to 0, the cache validity
		 * is set to FALSE and the bounding box is set to empty. */
		CoreExport PolyLine();
		
		/** Constructor. The PolyLine is initialized using	the input object "from". */
		CoreExport PolyLine(PolyLine& from);

		/** Destructor. The array of points is freed. */
		CoreExport ~PolyLine();
		
		/** Initializes the PolyLine. The array of points (pts) is freed,
		 * the number of points is set to 0, and the cache validity is set to
		 * FALSE. */
		CoreExport void Init();

		/** Marks the polyline as closed by setting the POLYLINE_CLOSED flag to one. */
		void Close() { flags |= POLYLINE_CLOSED; }

		/** Tells if the polyline is closed or not. 
		 * \return true if polyline is closed, false if it is not.*/
		CoreExport BOOL IsClosed();

		/** Marks the polyline as open by resetting the POLYLINE_CLOSED flag to zero */
		void Open() { flags &= ~POLYLINE_CLOSED; }

		/** Tells if the polyline is open or not. 
		 * \return true if polyline is open, false if it is not.*/
		CoreExport BOOL IsOpen();

		/** Sets the POLYLINE_NO_SELF_INT flag. */		
		void SetNoSelfInt() { flags |= POLYLINE_NO_SELF_INT; }
	
		/** Returns true if the POLYLINE_NO_SELF_INT flag is set, false if it is not. */
		BOOL IsNoSelfInt() { return (flags & POLYLINE_NO_SELF_INT) ? TRUE : FALSE; }

		/** Returns the number of vertices (points) in the polyline.*/
		int Verts() { return numPts; }

		/** Returns the number of segments (edges between vertices)	of the polyline. */
		CoreExport int Segments();

		/** Sets the number of points in the polyline.
		 * \param count The number to set.
		 * \param keep If set to true, any existing points are copied to the new array. If not, they
		 * are freed. Defaults to true.
		 * \return  true if the number of points was set, false otherwise. */
		CoreExport BOOL SetNumPts(int count, BOOL keep = TRUE);

		/** Appends the specified point to the polyline. This adds
		 * the point to the end of the points list.
		 * \param p The point to appended. */
		CoreExport void Append(PolyPt& p);

		/** Inserts the specified point at the location passed.
		 * \param where	The "pts" array index indicating where to insert the point.
		 * \param p	The point to insert. */
		CoreExport void Insert(int where, PolyPt& p);

		/** Deletes the specified point.
		 * \param where The "pts" array index indicating which point to delete. */
		CoreExport void Delete(int where);

		/** Reverses the order of the points in the polyline.
		 * \param keepZero	Defaults to false. This should normally be passed 
		 * as TRUE. If TRUE, and the polyline is closed, this method will make 
		 * sure that vertex zero is the same on the	reversed version as on the 
		 * non-reversed version. Otherwise if passed as	FALSE the last vertex 
		 * becomes the first vertex, and the first vertex becomes the last. This
		 * is an important distinction for the lofter because it always wants 
		 * vertex zero to be in the same place. */
		CoreExport void Reverse(BOOL keepZero=FALSE);
	
		/** Assignment operator. */
		CoreExport PolyLine& operator=(PolyLine& from);
		/** Assignment operator. This generates a polyline from the
		spline. */
		CoreExport PolyLine& operator=(Spline3D& from);
		/** Array access operator. Returns the specified point in the
		<b>pts</b> array. */
		CoreExport PolyPt& operator[](int index) { return pts[index]; }

		/** Computes the bounding box of the polyline. The result is
		 * stored in the bdgBox data member. */
		CoreExport void BuildBoundingBox(void);

		/** Ensures that the calling PolyLine object has flushed out any 
		 * cached data it may have had. This resets the bounding box size
		 * and sets the cache validity to false. This should be called when any
		 * points have been changed in the polyline. */
		CoreExport void InvalidateGeomCache();

		/** Returns the bound box of the polyline. The optional TM
		 * allows the box to be calculated in any space.
		 * \param tm The points of the polyline are multiplied by this 
		 * matrix before the box computation. Defaults to NULL.*/
		CoreExport Box3 GetBoundingBox(Matrix3 *tm=NULL); 

		/** \internal This method is used internally. */
		CoreExport void Render(GraphicsWindow* gw, Material* ma, RECT *rp, int compFlags, int numMat);

		/**	This renders the polyline to the display.
		 * \param gw Points to the graphics window to render to.
		 * \param ma The list of materials to use to render the polyline. 
		 * See Class Material, Class INode - Material methods.
		 * \param numMat The number of materials for the polyline.
		 * \param colorSegs if this is set material is used to determine what colors 
		 * to render the polyline. Otherwise the standard sel/subsel colors are used
		 * \param segsel this is the segment sel bitarray used to draw the selected segments
		 * \param useStartSegments = TRUE if this is set the start/endsegmens will be used 
		 * around the polyline draw.  If you are drawing several polylines in a row it is 
		 * faster to set this to false and use start/endsegments around the entire draw 
		 * instead of each individual polyline
		 */
		CoreExport void Render(GraphicsWindow* gw, Material* ma, int numMat, BOOL colorSegs, BitArray &segsel, BOOL useStartSegments = TRUE);
		
		/** \internal This method is used internally. */
		CoreExport BOOL Select(GraphicsWindow* gw, Material* ma, HitRegion* hr, int abortOnHit = FALSE);
		
		/** \internal This method is used internally. */
		CoreExport void Snap(GraphicsWindow* gw, SnapInfo* snap, IPoint2* p, Matrix3 &tm, DWORD flags);
		
		/** Transforms the points of the polyline by the specified matrix.
		 * \param tm The matrix used to transform the points. */
		CoreExport void Transform(Matrix3 &tm);

		/** Called to dump the polyline structure via DebugPrint(). See
		 * <a href="ms-its:3dsmaxsdk.chm::/debug_debugging.html">Debugging</a>.
		 * \param title This title string is displayed using a DebugPrint()
		 * before the rest of the data. */
		CoreExport void Dump(MCHAR *title = NULL);

		/** \internal This method is used internally as part of the capping
		 * mechanism and should not be used. */
		CoreExport void SpliceLine(int where, PolyLine &source, int splicePoint);

		/** Tells if the line segment defined between the two input points
		 * intersects this PolyLine object or not.
		 * \param p1,p2	The endpoints of the line to check.
		 * \param findAll TRUE to find all intersections. FALSE to find only the first
		 * intersection. Defaults to FALSE.
		 * \param cb A pointer to an IntersectionCallback3D class. 
		 * \returns true if the line segment defined between points
		 * p1 and p2 intersects this PolyLine, false otherwise.
		 */
		CoreExport BOOL HitsSegment(Point2 p1, Point2 p2, BOOL findAll = FALSE, IntersectionCallback3D *cb = NULL);

		/** \internal This method is used internally. */
		CoreExport int Cap3DS(CapVert *capverts, MeshCapInfo &capInfo, DWORD options = 0);

		/** Tells if the input PolyLine intersects this PolyLine or not.
		 * \param line	The input PolyLine to check.
		 * \param findAll TRUE to find all intersections. FALSE to find 
		 * only the first intersection.
		 * \param cb A pointer to an IntersectionCallback3D class. 
		 * \return true if the input PolyLine line intersects
		 * this PolyLine, false otherwise.
		 */
		CoreExport BOOL HitsPolyLine(PolyLine &line, BOOL findAll = FALSE, IntersectionCallback3D *cb = NULL);

		/** Returns TRUE if the specified point is surrounded
		 * (contained within) this PolyLine. This method should only be called on
		 * closed PolyLines.
		 * \param point	The point to check if it is inside the polyline or not. */
		CoreExport BOOL SurroundsPoint(Point2 &point);

		/** Returns a point interpolated on a segment between two points.
		 * \param segment The index of the segment to interpolate.\n\n
		 * \param t	A value in the range of 0.0 to 1.0. 0 is the first point and 1 is the
		 * second point.
		 * \return  The interpolated point. */
		CoreExport Point3 InterpPiece3D(int segment, float t);

		/** Returns a point interpolated on the entire curve. This method returns a point 
		 * but does not tell on which segment the point falls.
		 * \param u	A value in the range of 0.0 to 1.0 for the entire curve.
		 * \param ptype The parameter type for interpolation. The default value, 
		 * POLYSHP_INTERP_SIMPLE, is based on segments (rather than the entire curve). 
		 * See \ref Parameter_types_for_shape_interpolation 
		 * \return The interpolated point. */
		CoreExport Point3 InterpCurve3D(float u, int ptype=POLYSHP_INTERP_SIMPLE);

		/** Returns a tangent vector interpolated on a segment between two points.
		 * \param segment The index of the segment.
		 * \param t	A value in the range of 0.0 to 1.0. 0 is the first point and 1 
		 * is the second point.
		 * \return The tangent vector. */
		CoreExport Point3 TangentPiece3D(int segment, float t);

		/** Returns a tangent vector interpolated on the entire curve.
		 * \param u	A value in the range of 0.0 to 1.0 for the entire curve.
		 * \param ptype The parameter type for interpolation. The default value, 
		 * POLYSHP_INTERP_SIMPLE, is based on segments (rather than the entire curve). 
		 * See \ref Parameter_types_for_shape_interpolation 
		 * \return The tangent vector. */
		CoreExport Point3 TangentCurve3D(float u, int ptype=POLYSHP_INTERP_SIMPLE);

		/**	Returns the material ID for the specified segment.
		 * \param segment The zero based index of the segment. */
		CoreExport MtlID GetMatID(int segment);
		/** Returns the length of the calling PolyLine object. */
		CoreExport float CurveLength();

		/** Returns true if the polyline is clockwise in the XY plane
		 * (ignoring the Z dimension), false otherwise. If the calling PolyLine object self
		 * intersects, the result is meaningless. */
		CoreExport BOOL IsClockWise();
		
		/** Returns TRUE if the polyline intersects itself in the XY
		 * plane (ignoring the Z dimension), false otherwise.
		 * \param findAll true to find all self intersections, false 
		 * to find only the first self intersection. Defaults to false.
		 * \param cb Pointer to an IntersectionCallback3D class. Defaults to NULL.
		 */
		CoreExport BOOL	SelfIntersects(BOOL findAll = FALSE, IntersectionCallback3D *cb = NULL);		// 2D!

		CoreExport void GetSmoothingMap(IntTab& map);
		// IO
		CoreExport IOResult Save(ISave* isave);
		CoreExport IOResult Load(ILoad* iload);

		/**	Lets the developer transfer the content of a line object to another object
		 * without suffering from any copy penalties.  This is useful to transfer polylines
		 * to a new array, for instance.
		 * \param[in, out] line PolLine object to swap with.
		 */
		CoreExport void Swap(PolyLine& line);
	};

#define CAPFACE_AB	(1<<0)
#define CAPFACE_BC	(1<<1)
#define CAPFACE_CA	(1<<2)

class ShapeObject;

/** A multi-polygon shape class. This class is used in the caching of bezier
shapes. This is used for doing a one time interpolation of a bezier shape into
a form that is the same shape but doesn't require any further interpolation. In
this way the system can do the complex calculations once, store the shape into
this PolyShape representation, and not have to go through the cubic
spline calculations to figure out where the points are in the future. This
class maintains an array of PolyLines.
As an example, this is used in the Extrude modifier. First, it
generates a PolyShape from the bezier shape it is extruding. Next, this
PolyShape is used to generate the mesh.
\see  Class ShapeObject, Class PolyLine, Class ShapeVSel, Class ShapeSSel, Class ShapePSel, Class ShapeHierarchy.\n\n
*/
class PolyShape: public MaxHeapOperators {
	public:
		int numLines;				//!< The number of lines in the polyshape.
		PolyLine *lines;			//!< The array of lines.
		DWORD flags;				//!< Currently not used.
		Box3 bdgBox;				//!< Stores the bounding box surrounding each PolyLine in the shape.

		/// \name Selections
		//@{
		ShapeVSel	vertSel;  		//!< selected vertices
		ShapeSSel	segSel;  		//!< selected segments
		ShapePSel	polySel;  		//!< selected polygons
		//@}

		/** The selection level. One of the following values:
		 * \ref Selection_Level_Flags
		 */
		DWORD		selLevel;		

		/** Display Attribute Flags.
		 * \li DISP_VERTTICKS - Display vertices as ticks.
		 * \li DISP_SELVERTS - Display selected vertices.
		 * \li DISP_SELSEGMENTS - Display selected segments.
		 * \li DISP_SELPOLYS - Display selected polygons.
		 */
		DWORD		dispFlags;		

		/// \internal \name Capping Caches
		//@{
		MeshCapInfo morphCap;			//!< The morph cap information.
		BOOL morphCapCacheValid;		//!< Determines if the morph cap is valid.
		MeshCapInfo gridCap;			//!< The grid cap information.
		BOOL gridCapCacheValid;			//!< Determines if the grid cap is valid.
		PatchCapInfo patchCap;			//!< The patch cap information.
		BOOL patchCapCacheValid;		//!< Determines if the patch cap is valid.
		ShapeHierarchy cachedHier;		//!< Hierarchy cache.
		BOOL hierCacheValid;			//!< Determines if the hierarchy cache is valid.
		//@}

		/** Constructor. The number of lines is set to 0, the lines
		 * array is set to NULL, the flags and display flags are set to 0, the
		 * selection level is set to SHAPE_OBJECT, the geometry cache is
		 * invalidated, and the masterObject is set to NULL. */
		CoreExport PolyShape();

		/** Constructor. The PolyShape is initialized from the
		 * specified PolyShape. */
		CoreExport PolyShape(PolyShape& from);

		/** Destructor. */
		CoreExport ~PolyShape();

		/** \deprecated Used by constructors. This function is a special version used by the
		 * constructors and should not be called by plug-in developers. If you
		 * need to clear out a PolyShape use NewShape().*/
		CoreExport void Init();	

		/** This deletes all the lines from the PolyShape and
		 * clears the shape out. Use this method instead of Init() function.
		 */
		CoreExport void NewShape();	

		/** Sets the number of polygons used by the poly shape.
		 * \param count The number of lines.\n\n
		 * \param keep If set to true, any old lines are copied to the new storage. They are
		 * freed otherwise. Defaults to true.
		 * \return true if the number of lines were set, false otherwise. */
		CoreExport BOOL SetNumLines(int count, BOOL keep = TRUE);

		/** Creates a new PolyLine and appends it to the end
		 * of the list of lines maintained by this PolyShape.
		 * \return The address of the newly added line. */
		CoreExport PolyLine* NewLine();

		/** Appends the specified PolyLine to the end of the
		 * lines list.
		 * \param l The PolyLine to be appended to the caller object. */
		CoreExport void Append(PolyLine &l);

		/** Inserts the specified PolyLine at the input location
		 * \param where The index into the lines list specifying where to insert the
		 * PolyLine.
		 * \param l The PolyLine to be inserted. */
		CoreExport void Insert(int where, PolyLine& l);

		/** Deletes the specified PolyLine from the lines
		 * list.
		 * \param where The index into the lines list specifying which line to delete. */
		CoreExport void Delete(int where);

		/** Assignment operator. */
		CoreExport PolyShape& operator=(PolyShape& from);

		/** Assignment operator. Note that this operator does not
		 * offer as much control as calling the method on the <b>BezierShape</b>
		 * itself named MakePolyShape(). That version allows you to specify
		 * the number of steps and an optimize parameter. These options are not
		 * available on this simple assignment operator. */
		CoreExport PolyShape& operator=(BezierShape& from);

		/** Builds a bounding box surrounding every line in the lines
		 * list. The bounding box is returned through the bdgBox data
		 * member. */
		CoreExport void BuildBoundingBox(void);

		/** Invalidates the cache of each line in the
		 * PolyShape. The bounding box is set to empty. This method also
		 * invalidates the capping caches.
		 * \param unused This parameter is not used. */
		CoreExport void InvalidateGeomCache(BOOL unused);

		/** Invalidates the morph, and grid cap caches. */
		CoreExport void InvalidateCapCache();

		/** Returns the bounding box of the <b>PolyLines</b> in this
		 * PolyShape. The optional TM allows the box to be calculated in
		 * any space.
		 * \param tm The points of the PolyLines in this PolyShape are
		 * transformed by this matrix prior to the bounding box computations. 
		 * optional TM allows the box to be calculated in any space.*/
		CoreExport Box3 GetBoundingBox(Matrix3 *tm=NULL);

		/** Computes the bounding box of this PolyShape.
		 * \param t The time at which to evaluate the bounding box.
		 * \param box The result is stored here.
		 * \param tm The points of each PolyLine in this PolyShape are
		 * deformed using this matrix.\n\n
		 * \param useSel If set to true, the box is computed about the 
		 * selected vertices only. All points will be used for this otherwise. */
		CoreExport void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel );

		/** \internal This method is used internally. */
		CoreExport void Render(GraphicsWindow* gw, Material* ma, RECT *rp, int compFlags, int numMat);

		/** \internal This method is used internally. */
		CoreExport BOOL Select(GraphicsWindow* gw, Material* ma, HitRegion* hr, int abortOnHit = FALSE);

		/** \internal This method is used internally. */
		CoreExport void Snap(GraphicsWindow* gw, SnapInfo* snap, IPoint2* p, Matrix3 &tm);

		/** \internal This method is used internally. */
		CoreExport void Snap(GraphicsWindow* gw, SnapInfo* snap, IPoint2* p, Matrix3 &tm, DWORD flags);

		/** Transforms the vertices of each PolyLine in this
		 * PolyShape by the specified matrix.
		 * \param tm The transformation matrix. */
		CoreExport void Transform(Matrix3 &tm);

		/** This method may be called to fill in the MeshCapInfo
		 * passed with the appropriate capping information. See
		 * <a href="ms-its:3dsmaxsdk.chm::/patches_root.html">Working with
		 * Patches</a> for more details on capping. This method is used for
		 * meshes. The method below is used for patches.
		 * \param tThis should be passed as the current time. You may retrieve this using
		 * Interface::GetTime() (See Class Interface).
		 * \param capInfo This information is filled in by this method. (See
		 * Class MeshCapInfo).
		 * \param capType The cap type. See \ref capTypes.
		 * \return  Nonzero if the cap info was set up successfully. Zero otherwise. */
		CoreExport int MakeCap(TimeValue t, MeshCapInfo &capInfo, int capType);

		/** This method may be called to fill in the
		 * PatchCapInfo passed with the appropriate capping information.
		 * This method is used for patches. Note that it is generally not
		 * recommended to try to create patch caps from PolyShapes. The
		 * patch cap uses bezier information, so it is much better to use a
		 * BezierShape to make a patch cap. It is very inefficient to do
		 * this with a PolyShape.
		 * \param t This should be passed as the current time. You may retrieve this using
		 * Interface::GetTime() (See Class Interface).
		 * \param capInfo This information is filled in by this method. (See
		 * Class PatchCapInfo).
		 * \return  Nonzero if the cap info was set up successfully. Zero otherwise */
		CoreExport int MakeCap(TimeValue t, PatchCapInfo &capInfo);

		/** \internal This method is used internally. */
		CoreExport int Make3DSCap(MeshCapInfo &capInfo, DWORD options = 0);

		/** \internal This method is used internally. */
		CoreExport int MakeGridCap(MeshCapInfo &capInfo);

		/** This may be called to display information about the
		 * PolyShape to the debug window via DebugPrint(). See
		 * <a href="ms-its:3dsmaxsdk.chm::/debug_debugging.html">Debugging</a>.
		 * \param title A string to be display before the PolyShape data is displayed.
		 */
		CoreExport void Dump(MCHAR *title = NULL);

		/** \internal This method is used internally. */
		CoreExport void UpdateCachedHierarchy();

		/** This methods looks at the shape organization, and puts
		 * together a shape hierarchy. This provides information on how the shapes
		 * are nested. For example on a donut object with two circles, this method
		 * determines which circle is inside the other one.
		 * \param t This parameter is not used.
		 * \param hier If non-NULL the result is store here (in addition to being returned).
		 * (See Class ShapeHierarchy).
		 * \return  The result is store here. */
		CoreExport ShapeHierarchy &OrganizeCurves(TimeValue t, ShapeHierarchy *hier = NULL);

		/** This is a very important call to make. Whenever you have
		 * changed the PolyShape, for example after you are done adding
		 * polygons to the shape or have changed the number of points in a shape,
		 * you should call this method. This method updates a set of embedded
		 * selection set data within the shape. This selection set data tells what
		 * polygons are selected, what segments are selected, and what control
		 * points are selected. This resets the sizes of the selection bit arrays
		 * for this shape. */
		CoreExport void UpdateSels();

		/** Reverses the PolyLine whose index is passed.
		 * \param poly The spline to reverse.
		 * \param keepZero This should normally be passed as "true". If set to true,
		 * and the polyline is closed, this method will make sure that vertex zero 
		 * is the same on the reversed version as on the non-reversed version. 
		 * Otherwise, if set to false, the last vertex becomes the first vertex, 
		 * and the first vertex becomes the last. This is an important distinction 
		 * for the lofter because it always wants vertex zero to be in the same place. */
		CoreExport void Reverse(int poly, BOOL keepZero=FALSE);
		
		/** Reverses the splines of this shape if the corresponding
		 * bit in <b>reverse</b> is set. The BitArray has one bit for each
		 * polyline.
		 * \param reverse If the bit is set the spline is reversed; otherwise it is left
		 * alone.
		 * \paramkeepZero=FALSE</b>\n\n
		 * This should normally be passed as TRUE. If set to true, and the polyline is
		 * closed, this method will make sure that vertex zero is the same on the
		 * reversed version as on the non-reversed version. Otherwise, if set to
		 * false, the last vertex becomes the first vertex, and the first vertex
		 * becomes the last. This is an important distinction for the lofter
		 * because it always wants vertex zero to be in the same place. */
		CoreExport void Reverse(BitArray &reverse, BOOL keepZero=FALSE);

		/** Returns the material ID for the specified segment of the specified
		 * poly.
		 * \param poly The zero based index of the poly.
		 * \param piece The zero based index of the segment. */
		CoreExport MtlID GetMatID(int poly, int piece);
		/** Constructs a vertex selection list based on the current
		 * selection level of the specified poly. For example if the selection
		 * level is at object level all the bits are set. If the selection level
		 * is at vertex level only the selected vertex bits are set. See
		 * Class BitArray.
		 * \param poly The poly whose selection level is constructed. */
		CoreExport BitArray	VertexTempSel(int poly);

		/** \internal This method is used internally in data flow evaluation in the	pipeline. */
		CoreExport void ShallowCopy(PolyShape* ashape, ChannelMask channels);

		/** \internal This method is used internally in data flow evaluation in the	pipeline. */
		CoreExport void DeepCopy(PolyShape* ashape, ChannelMask channels);

		/** \internal This method is used internally in data flow evaluation in the	pipeline. */
		CoreExport void NewAndCopyChannels(ChannelMask channels);

		/** \internal This method is used internally in data flow evaluation in the	pipeline. */
		CoreExport void FreeChannels(ChannelMask channels, int zeroOthers=1);
		
		
		/** \internal This method is used internally in saving to the MAX file.	*/
		CoreExport IOResult Save(ISave *isave);

		/** \internal This method is used internally in loading from the MAX file. */
		CoreExport IOResult Load(ILoad *iload);
		
		/**	Deletes multiple lines at once. This is faster than deleting one by one using
		 * Delete(), because the internal bookkeeping only needs to be done once.
		 * Lines are deleted in reverse order, from highest index to lowest index.  
		 * If the array of indices is not properly sorted (in ascending order), or if the
		 * indices are not unique, the results will be undefined: the wrong lines may be 
		 * deleted, or more likely 'false' will be returned, leaving you
		 * with a ShapeObject in an undefined state.
		 * \param indices Point to an array of indices to delete.  Indices should be unique, 
		 * and sorted in ascending order.
		 * \paramNumber of entries in the indices array.
		 * \return true if the delete operations succeeded, false if any of the indices are invalid.
		 */
		CoreExport bool Delete(const unsigned int* indices, unsigned int n);

		/**	Append multiple lines at once. This is faster than appending one by one 
		 * using Append(), because the internal bookkeeping only needs to be done once.
		 * \param lines Array of lines to append. 
		 * \param n Number of entries in the lines array.
		 * \param useSwap Should PolyLine::Swap() be used instead of operator = ?  Using Swap()
		 * is faster, and should be used if you are just going to discard the contents of the lines array.
		 */
		CoreExport void Append(PolyLine* lines, unsigned int n, bool useSwap = false);
	};

