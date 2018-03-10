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
#include "polyshp.h"	// Need this for PolyLine class
#include "GraphicsConstants.h"

// forward declarations
class ViewExp;
class IObjParam;

/// \defgroup Point_flags Point flags for PolyShape representation
//@{
#define BEZ_SHAPE_KNOT			(1<<0)	//!< A knot point
#define BEZ_SHAPE_INTERPOLATED	(1<<1)	//!< An interpolated point between two knots
//@}


/// \defgroup Line_types Line types
//@{
#define LTYPE_CURVE 0	//!< Specifies that the segment should interpolate based on the bezier handles for the segment.
#define LTYPE_LINE 1	//!< Specifies that the segment should go straight from knot to knot and ignore the bezier handles.
//@}

/// \defgroup Compound_line_types Compound line types
//@{
#define CURVE_CURVE (LTYPE_CURVE | (LTYPE_CURVE<<2))
#define LINE_CURVE (LTYPE_LINE | (LTYPE_CURVE<<2))
#define CURVE_LINE (LTYPE_CURVE | (LTYPE_LINE<<2))
#define LINE_LINE (LTYPE_LINE | (LTYPE_LINE<<2))
//@}

/// \defgroup Knot_types Knot types
//@{
/**  Produces the bezier handles automatically to produce a smooth curve. 
With this knot type the bezier handles are invisible.*/
#define KTYPE_AUTO 0	
/** Produces a sharp corner.*/
#define KTYPE_CORNER 1
/**This knot type produces bezier handles that are collinear 
 * (the bezier vectors coming out of the knot are collinear). These handles are 
 * constrained to be collinear so if the user moves one handle 
 * the other will move to remain collinear. Also if the user moves 
 * the handle towards or away from the knot the other handle will 
 * move the corresponding amount in the proper scale.*/
#define KTYPE_BEZIER 2
/** This knot type has bezier handles but they are not constrained to be opposite each other.*/
#define KTYPE_BEZIER_CORNER (KTYPE_BEZIER | KTYPE_CORNER)
#define KTYPE_RESET	4
//@}

/** \defgroup Parameter_types Parameter types.
 * These are not currently used and are reserved for future use. */
//@{
#define PARM_UNIFORM		0
#define PARM_ARCLENGTH		1
#define PARM_CENTRIPETAL	2
#define PARM_CUSTOM			3
//@}

/// \defgroup DrawPhase_values DrawPhase values
//@{
#define DRAW_IDLE 0
#define DRAW_INITIAL_MOUSE_DOWN 1
#define DRAW_FREEMOVE_POINT 2
#define DRAW_FREEMOVE_POINT_MOUSE_DOWN 3	//!< Inserting's initial click inside spline
#define DRAW_INITIAL_BEZ_ADJ 11
#define DRAW_DRAGGING_VECTOR 22
//@}

/// \defgroup Parameter_types_for_shape_interpolation Parameter types for shape interpolation (Must match interpolation types in object.h)
//@{
#define SPLINE_INTERP_SIMPLE 0		//!< Parameter space based on segments
#define SPLINE_INTERP_NORMALIZED 1	//!< Parameter space normalized to curve length
//@}

class Spline3D;
class SplineKnotAssy;
class SplineKnot;			  

/** Provides the vertex-level point information for the Spline3D class. 
 * 'aux' fields in spline are available in 3 channels:
 * 0: Used in capping process
 * 1: Used to track topology changes in spline editing
 * 2: Available to user */
class SplinePoint: public MaxHeapOperators {
	friend class Spline3D;
	friend class SplineKnotAssy;
	friend class SplineKnot;

	private:
		Point3 point;
		int aux[3];
	public:
		CoreExport SplinePoint();
		CoreExport SplinePoint(Point3 &p, int a1 = -1, int a2 = -1, int a3 = -1);
		CoreExport SplinePoint& operator=(SplinePoint &fromSP);
		inline Point3& GetPoint() { return point; }
		inline int GetAux(int index) { return aux[index]; }
		inline void SetPoint(const Point3 &p) { point = p; }
		inline void SetAux(int index, int a) { aux[index] = a; }
	};


#define SPLINE_MATID_SHIFT	16		//!< The mat ID is stored in the HIWORD of the knot flags
#define SPLINE_MATID_MASK	0xFFFF	//!< The mat ID is stored in the HIWORD of the knot flags
//watje
#define SEGMENT_VISIBLE		(1<<0)
#define SPLINEKNOT_NO_SNAP	(1<<1)	//!< Suppresses snapping to knot if set

/** Primarily for internal use, and is designed for the 
 *	new "connect copy" feature of spline segments. The method XFormVerts() 
 *	normally operates on the selected knots (or the knots of selected spline 
 *	segments), but if the knots have the SPLINEKNOT_ADD_SEL flag set, and the 
 *	SplineShape also has the ES_ADDED_SELECT flag set, then those additional 
 *	knots will be affected as well. */
#define SPLINEKNOT_ADD_SEL	(1<<2)	//!< Additional selection for transformation

/** Used for the internal storage of spline knot assemblies
 * in the Spline3D class*/
class SplineKnotAssy: public MaxHeapOperators {
	friend class Spline3D;
	friend class SplineKnot;

	private:
		int ktype;			// Knot type
		int ltype;			// Line type
		float du;			// Parameter value
		SplinePoint inVec;	// The in vector
		SplinePoint knot;	// The knot
		SplinePoint outVec;	// The out vector
		DWORD flags;
	public:
		CoreExport SplineKnotAssy();
		CoreExport SplineKnotAssy(int k, int l, Point3 p, Point3 in, Point3 out, int a1= -1, int a2= -1, int a3= -1, int Ia1= -1, int Ia2= -1, int Ia3= -1, int Oa1= -1, int Oa2= -1, int Oa3= -1, DWORD f=0);
		CoreExport SplineKnotAssy(int k, int l, SplinePoint p, SplinePoint in, SplinePoint out, DWORD f=0);
		CoreExport SplineKnotAssy(SplineKnot &k);
		inline	int		Ktype() { return ktype; }
		inline	void	SetKtype(int t) { ktype=t; }
		inline	int		Ltype() { return ltype; }
		inline	void	SetLtype(int t) { ltype=t; }
		inline	Point3	Knot() { return knot.point; }
		inline	void	SetKnot(const Point3 &p) { knot.point=p; }
		inline	Point3	InVec() { return inVec.point; }
		inline	void	SetInVec(const Point3 &p) { inVec.point=p; }
		inline	Point3	OutVec() { return outVec.point; }
		inline	void	SetOutVec(const Point3 &p) { outVec.point=p; }
		inline	float	GetParm() { return du; }
		inline	void	SetParm(float p) { du = p; }
		inline	MtlID	GetMatID() {return (int)((flags>>SPLINE_MATID_SHIFT)&SPLINE_MATID_MASK);}
		inline	void    SetMatID(MtlID id) {flags &= 0xFFFF; flags |= (DWORD)(id<<SPLINE_MATID_SHIFT);}
		
		/** Allow access as if the in/knot/out components are contained vertices.  
		 * \param index 0=inVec 1=knot 2=outVec  
		 * \param which 0=aux1 1=aux2 2=aux3*/
		CoreExport int	GetAux(int index, int which);

		/** Allow access as if the in/knot/out components are contained vertices.  
		 * \param index 0=inVec 1=knot 2=outVec  
		 * \param which 0=aux1 1=aux2 2=aux3*/
		CoreExport void	SetAux(int index, int which, int value);

		/** Allow access as if the in/knot/out components are contained vertices.  
		 * \param index 0=inVec 1=knot 2=outVec  
		 * \param which 0=aux1 1=aux2 2=aux3*/
		CoreExport Point3 GetVert(int index);

		/** Allow access as if the in/knot/out components are contained vertices.  
		 * \param index 0=inVec 1=knot 2=outVec  
		 * \param which 0=aux1 1=aux2 2=aux3*/
		CoreExport void SetVert(int index, const Point3 &p);

		inline	SplinePoint GetKnot() { return knot; }
		inline	SplinePoint GetInVec() { return inVec; }
		inline	SplinePoint GetOutVec() { return outVec; }
		inline	void SetKnot(SplinePoint &sp) { knot = sp; }
		inline	void SetInVec(SplinePoint &sp) { inVec = sp; }
		inline	void SetOutVec(SplinePoint &sp) { outVec = sp; }
		inline  DWORD GetFlags() { return flags; }

//watje		
		inline	BOOL	IsHidden() {return (flags&SEGMENT_VISIBLE);}
		inline	void    Hide() { flags |= (DWORD)(SEGMENT_VISIBLE);}
		inline	void    Unhide() { flags &= (DWORD)(~SEGMENT_VISIBLE);}
//TH 8/22/00
		inline	BOOL	IsNoSnap() {return (flags&SPLINEKNOT_NO_SNAP);}
		inline	void    SetNoSnap() { flags |= (DWORD)(SPLINEKNOT_NO_SNAP);}
		inline	void    ClearNoSnap() { flags &= (DWORD)(~SPLINEKNOT_NO_SNAP);}
// CAL-05/23/03
		inline	BOOL	GetFlag (DWORD fl) { return (flags & fl) ? TRUE : FALSE; }
		inline	void	SetFlag (DWORD fl, BOOL val=TRUE) { if (val) flags |= fl; else flags &= ~fl; }
		inline	void	ClearFlag (DWORD fl) { flags &= ~fl; }
	};

/** Describes a single knot in a spline. It is used by plug-ins to 
 * get and set knot information in the Spline3D class.
 * This is primarily here for backward-compatibility with versions prior to MAXr3.
 * In 3ds Max 2.0 and later there are methods which provide full access to the
 * private data members of the class.
 * All methods of this class are implemented by the system.
 * To understand this class better, picture a bezier spline with three knots, 
 * going from left to right: \nA---\>AB----BA\<---B---\>BC----CB\<---C
 * The knot points are A, B and C. The vectors are labeled the same as patch
 * vectors (AB is the vector from A going toward B, the vector from B to A is
 * labeled BA, and so on).
 * In this diagram, AB is the OUT vector for knot A. BA is the IN vector for knot
 * B. BC is the OUT vector for knot B, and CB is the IN vector for knot C. Because
 * this is an open spline, knot A doesn't use its IN vector, and knot C doesn't
 * use its OUT vector.
 * The IN and OUT terminology is based on the way a spline flows from the first
 * knot to the last. If the spline is reversed, the IN and OUT sense is reversed,
 * as well.
 * Regarding the vectors, the only difference between a circle and a square is
 * that the square has vectors that are at the same location as the knot point (in
 * other words, zero length vectors) causing sharp corners. The circle uses
 * vectors which cause each segment to bulge to form a quarter-circle.
 * Take a look at the /MAXSDK/SAMPLES/OBJECTS/NGON.CPP source file for an
 * example of how the vectors are generated to form a linear NGON versus a
 * circular one. 
 * \see Spline3D
 */
class SplineKnot: public MaxHeapOperators {
	friend class Spline3D;
	friend class SplineKnotAssy;

	int ktype;
	int ltype;
	Point3 point;
	Point3 inVec;
	Point3 outVec;
	int aux;		// Used for capping
	int aux2;		// Used to track topo changes in spline editing
	int aux3;		// User aux field
	int inAux;
	int inAux2;
	int inAux3;
	int outAux;
	int outAux2;
	int outAux3;
	DWORD flags;
public:
	CoreExport SplineKnot();
	
	/** Constructor. The data members are initialized to the values passed. 
	 * \param k Knot type
	 * \param l Line type
	 * \param p Point Location
	 * \param in In vector
	 * \param out Out vector
	 * \param a1 Used for capping. An integer value which may be used for temporary storage of data
	 * associated with the knot. This data will be overwritten by the internal
	 * EditSpline code.
	 * \param a2 Used to track topo changes in spline editing.
	 * \param a3 User aux field.
	 * \param Ia1, Ia2, Ia3, Oa1, Oa2, Oa3, f
	 */
	CoreExport SplineKnot(int k, int l, Point3 p, Point3 in, Point3 out, int a1= -1, int a2= -1, int a3= -1, int Ia1= -1, int Ia2= -1, int Ia3= -1, int Oa1= -1, int Oa2= -1, int Oa3= -1, DWORD f=0);
	CoreExport SplineKnot(SplineKnotAssy &k);

	/** Returns the knot type.
	 * \see Knot_types */
	inline	int		Ktype() { return ktype; }

	/**	Sets the knot type.
	 * \param t The type to set.
	 * \see Knot_types*/
	inline	void	SetKtype(int t) { ktype=t; }

	/** Returns the line type.
	 * \see Line_types*/
	inline	int		Ltype() { return ltype; }

	/**	Sets the line type.
	 * \param t The type to set. 
	 * \see Line_types*/
	inline	void	SetLtype(int t) { ltype=t; }

	/**	Provides access to the first integer chunk of 
	 * auxiliary data for the knot. */
	inline	int		Aux() { return aux; }

	/**	Sets the first integer of auxiliary data for the knot.
	 * \param a The value to be set to. */
	inline	void	SetAux(int a) { aux=a; }

	/**	Provides access to the second integer of auxiliary data for the knot. */
	inline	int		Aux2() { return aux2; }

	/**	Sets the second integer of auxiliary data for the knot.
	 * \param a The value to be set to. */
	inline	void	SetAux2(int a) { aux2=a; }

	inline	int		Aux3() { return aux3; }
	inline	void	SetAux3(int a) { aux3=a; }
	inline	int		InAux() { return inAux; }
	inline	void	SetInAux(int a) { inAux=a; }
	inline	int		InAux2() { return inAux2; }
	inline	void	SetInAux2(int a) { inAux2=a; }
	inline	int		InAux3() { return inAux3; }
	inline	void	SetInAux3(int a) { inAux3=a; }
	inline	int		OutAux() { return outAux; }
	inline	void	SetOutAux(int a) { outAux=a; }
	inline	int		OutAux2() { return outAux2; }
	inline	void	SetOutAux2(int a) { outAux2=a; }
	inline	int		OutAux3() { return outAux3; }
	inline	void	SetOutAux3(int a) { outAux3=a; }

	/**	Returns the point location for the knot. */
	inline	Point3	Knot() { return point; }

	/**	Sets the point location for the knot.
	 * \param p The point to be set. */
	inline	void	SetKnot(Point3 p) { point=p; }

	/**	Returns the in vector for the knot. */
	inline	Point3	InVec() { return inVec; }

	/**	Sets the in vector for the knot.
	 * \param p The in vector to be set. */
	inline	void	SetInVec(Point3 p) { inVec=p; }

	/**	Returns the out vector for the knot. */
	inline	Point3	OutVec() { return outVec; }

	/**	Sets the out vector for the knot.
	 * \param p The out vector to set. */
	inline	void	SetOutVec(Point3 p) { outVec=p; }

	inline	MtlID	GetMatID() {return (int)((flags>>SPLINE_MATID_SHIFT)&SPLINE_MATID_MASK);}
	inline	void    SetMatID(MtlID id) {flags &= 0xFFFF; flags |= (DWORD)(id<<SPLINE_MATID_SHIFT);}
	inline  DWORD	GetFlags() { return flags; }

//watje
	inline	BOOL	IsHidden() {return (flags&SEGMENT_VISIBLE);}
	inline	void    Hide() { flags |= (DWORD)(SEGMENT_VISIBLE);}
	inline	void    Unhide() { flags &= (DWORD)(~SEGMENT_VISIBLE);}
//TH 8/22/00
	inline	BOOL	IsNoSnap() {return (flags&SPLINEKNOT_NO_SNAP);}
	inline	void    SetNoSnap() { flags |= (DWORD)(SPLINEKNOT_NO_SNAP);}
	inline	void    ClearNoSnap() { flags &= (DWORD)(~SPLINEKNOT_NO_SNAP);}
// CAL-05/23/03
	inline	BOOL	GetFlag (DWORD fl) { return (flags & fl) ? TRUE : FALSE; }
	inline	void	SetFlag (DWORD fl, BOOL val=TRUE) { if (val) flags |= fl; else flags &= ~fl; }
	inline	void	ClearFlag (DWORD fl) { flags &= ~fl; }
	};

/// \internal \defgroup Private_spline_flags Private spline flags
//@{
#define SPLINE_CLOSED	(1<<0)
#define SPLINE_ORTHOG   (1<<1)
//@}

/** General-purpose 3D spline class. The BezierShape class has a list of
 * these splines that make up the bezier shape. Methods of this class are used to
 * access the properties of the spline. All methods of this class are implemented
 * by the system.
 * \see SplineKnot, PolyLine*/
class Spline3D: public MaxHeapOperators {
    friend class BezierShape;
    friend class SplineShape;
    friend class Railing; 
private:
	static	int			splineCount;	// Number of splines in the system
			int			parmType;		// Interpolation parameter type	(needed?)
			int			knotCount;		// Number of points in spline

			int			flags;			// Private flags
			int			iCur;			// Current editing point
			int			Interact(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3* mat, IObjParam* ip=NULL );	// Handle mouse interaction
			float		cachedLength;
			BOOL		cacheValid;
			SplineKnotAssy*	knots;		// Knot assembly array
			float *		lengths;		// Cached lengths
			float *		percents;		// Cached length percentages
			int			drawPhase;		// Drawing phase
			int			editMode;		// 1 if editing, 0 otherwise

			// Creation settings
			int			initialType;	// Knot type at initial click
			int			dragType;		// Knot type at drag
			
			BOOL		selfInt;		// Cached self-intersection flag
			BOOL		siCacheValid;	// Self-intersection cache valid?
			Box3		bbox;			// Cached bounding box
			BOOL		bboxValid;		// Bounding box valid?
			BOOL		paramsValid;	// Knot parameter values valid?
			BOOL		bezPointsValid;	// Bezier points valid?
			BOOL		clockwise;		// Clockwise cache
			BOOL		cwCacheValid;	// Clockwise cache valid?
			PolyLine	polyLine;		// Polyline cache
			BOOL		plineOpt;
			int			plineSteps;
			BOOL		plineCacheValid;	// Polyline cache valid?
protected:
	CoreExport		void		Allocate(int count);
	CoreExport		void		ChordParams();							// Compute chord length params
	CoreExport		void		UniformParams();						// Compute uniform params
	CoreExport		void		CentripetalParams();					// Compute centripetal params
	CoreExport		void		LinearFwd(int i);
	CoreExport		void		LinearBack(int i);
	CoreExport		void		ContinFwd(int i);
	CoreExport		void		ContinBack(int i);
	CoreExport		void		HybridPoint(int i);
	CoreExport		void		CompCornerBezPoints(int n);
	CoreExport		void		CompAdjBesselBezPoints(int i);
	CoreExport		void		BesselStart(int i);
	CoreExport		void		BesselEnd(int i);
	CoreExport		void		NaturalFwd(int i);
	CoreExport		void		NaturalBack(int i);
public:

	/** Constructor. 
	 * \param itype Knot type at initial click.
	 * \param dtype Knot type at drag.
	 * \param ptype Interpolation parameter type
	 */
	CoreExport		Spline3D(int itype = KTYPE_CORNER,int dtype = KTYPE_BEZIER,int ptype = PARM_UNIFORM);
	CoreExport		Spline3D(Spline3D& fromSpline);

	/** Destructor. */
	CoreExport		virtual ~Spline3D();

	/** Assignment operator. */
	CoreExport		Spline3D& 	operator=(Spline3D& fromSpline);

	/** Assignment operator. This generates a PolyLine from the
	 * spline, where points are added in between the knots on the spline. For
	 * example if the steps value was 5, it will interpolate 5 points in between
	 * each knot on the spline. */
	CoreExport		Spline3D& 	operator=(PolyLine& fromLine);

	/** Clears out the spline. It frees the knots
	 * attributes array and the bezier points array. */
	CoreExport		void		NewSpline();

	/** \internal This method is used internally. */
	inline			int			ParmType() { return parmType; };

	/** Returns the Knot (point) count. */
	inline			int			KnotCount() { return knotCount; }					
	
	/** Returns the private spline flags.
	 * SPLINE_CLOSED
	 * This indicates if the spline is closed or not. */
	inline			int			Flags() { return flags; }

	/** Returns the number of line segments in the spline. For
	 * example if you have a 4 knot spline that is open you'll get 3 segments. */
	CoreExport		int			Segments();

	/** Returns the closed status. Nonzero if closed, zero if not closed.	*/
	inline			int			Closed() { return (flags & SPLINE_CLOSED) ? 1:0; }

	/** \internal This method is used internally. */
	CoreExport		int			ShiftKnot(int where,int direction);						// Shove array left or right 1,
	
	/** Add a knot to the spline at the specified location.
	 * \param k The knot to be added.
	 * \param where = -1,  The location to add the knot. a negative value indicates the end of the spline.
	 * \return  Nonzero on success, zero otherwise.
	 */
	CoreExport		int			AddKnot(SplineKnot &k,int where = -1);					// Add a knot to the spline
	
	/**	Sets the 'i-th' knot object which contain the knot point, in and out 
	 * vectors, knot and line types and auxiliary values.
	 * \param i Specifies the value to be set to.
	 * \param k The knot to be set. */
	CoreExport		void		SetKnot(int i, SplineKnot &k);
	
	/**	Returns the 'i-th' knot object which contain the knot point, in
	 * and out vectors, knot and line types and auxiliary values.
	 * \param i Specifies the knot to be retrieved. */
	CoreExport		SplineKnot	GetKnot(int i);

	/** Delete the specified knot.
	 * \param where The location of the knot to be deleted.
	 * \return  Nonzero if the knot was deleted, zero otherwise. */
	CoreExport		int			DeleteKnot(int where);									// Delete the specified knot
	
	/** Used internally by the free form line object
	 * SPLINE.CPP. This method allows the user to use the mouse to create a
	 * line. See the sample code in /MAXSDK/SAMPLES/OBJECTS/SPLINE.CPP for
	 * an example of this method in use.\n\n
	 * New for 3ds Max 2.0 is an additional parameter is a pointer to an
	 * IObjParam object, which is used to access the
	 * DisplayTempPrompt() mechanism. The parameter is optional; omitting
	 * it allows the spline operation to work as in 3ds Max 1.x. Adding the
	 * parameter causes the spline to display the delta, distance and angle of the
	 * current segment being edited. */
	CoreExport		int			Create(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3* mat, IObjParam *ip=NULL);	// Create the spline
	
	/** Used internally by the free form line object
	 * SPLINE.CPP. See the sample code in
	 * /MAXSDK/SAMPLES/OBJECTS/SPLINE.CPP for an example of this method in
	 * use. */
	CoreExport		int			StartInsert(ViewExp *vpt,int msg, int point, int flags, IPoint2 theP, Matrix3* mat, int where );	// Start an insertion operation on the spline
	
	/** \internal This method is used internally. */
	CoreExport		int			SetParam(int index,float param);		// Set custom param value
	
	/** \internal This method is used internally. */
	CoreExport		float		GetParam(int index);					// Get param value
	
	/** Returns the knot type from the specified knot. 
	 * \param index The index of the knot type to return. 
	 * \see Knot_types*/	
	inline			int			GetKnotType(int index) { return knots[index].ktype; }
	
	/** Sets the knot type of the specified knot.
	 * \param index The knot to be set.
	 * \param type The Spline knot type
	 * \return  Nonzero if set, zero otherwise.
	 * \see Knot_types */
	
	CoreExport		int			SetKnotType(int index,int type);		// Set the knot type
	/** Returns the type of line segment between knots for the specified segment.
	 * \param index The index of the segment whose line type will be returned.
	 * \return  Spline line types
	 * \see Line_types */
	
	inline			int			GetLineType(int index) { return knots[index].ltype; }
	
	/** Sets the line type of the specified segment.
	 * \param index The index of the segment.
	 * \param type The Spline line type
	 * \return  Nonzero if set, zero otherwise.
	 * \see Line_types */
	CoreExport		int			SetLineType(int index,int type);		// Set the line type
	
	/** \internal This method is used internally. */
	virtual			void		CustomParams() { UniformParams(); }		// Replace this as needed
	
	/** \internal This method is used internally. */
	CoreExport		void		CompParams();							// Compute param values
	
	/** This method should be called whenever you finish changing
	 * points on the spline. This updates all the information internal to the
	 * spline needed to calculate all the bezier handles. */
	CoreExport		void		ComputeBezPoints();
	
	/** Finds segment and parameter for whole-curve parameter. */
	CoreExport		void		FindSegAndParam(float u, int ptype, int &seg, float &param);
	CoreExport		void		RefineCurve(float u, int ptype=SPLINE_INTERP_SIMPLE);
	CoreExport		void		RefineSegment(int segment, float t, int ptype=SPLINE_INTERP_SIMPLE);
	
	/** \deprecated Developers should use InterpBezier3D() instead
  	 * of this method and just use the x and y values returned. */
	CoreExport		Point2		InterpBezier(IPoint2 *bez, float t);
	
	/** Returns a point interpolated on a segment between two knots.
	 * \param segment The index of the segment to interpolate.
	 * \param t A value in the range of 0.0 to 1.0. 0 is the first knot and 1 is the second knot.
	 * \param ptype = SPLINE_INTERP_SIMPLE;  The spline type to use.
	 * \return  The interpolated point. */
	CoreExport		Point3		InterpBezier3D(int segment, float t, int ptype=SPLINE_INTERP_SIMPLE);
	
	/** Returns a point interpolated on the entire curve. However, does not tell 
	 * which segment the point falls on. Typically the method  InterpBezier3D() 
	 * will give better control of the curve as it interpolates a bezier segment.
	 * \param u A value in the range of 0.0 to 1.0 for the entire curve.
	 * \param ptype = SPLINE_INTERP_SIMPLE; The spline type to use.
	 * \return  The interpolated point. */
	CoreExport		Point3		InterpCurve3D(float u, int ptype=SPLINE_INTERP_SIMPLE);
	
	/** Returns a tangent vector interpolated on a segment between two knots.
	 * \param segment The index of the segment.
	 * \param t A value in the range of 0.0 to 1.0. 0 is the first knot and 1 is the second knot.
	 * \param ptype = SPLINE_INTERP_SIMPLE;  The spline type to use.
	 * \return  The tangent vector. */
	CoreExport		Point3		TangentBezier3D(int segment, float t, int ptype=SPLINE_INTERP_SIMPLE);
	
	/** Returns a tangent vector interpolated on the entire curve.
	 * \param u A value in the range of 0.0 to 1.0 for the entire curve.
	 * \param ptype = SPLINE_INTERP_SIMPLE; The spline type to use.
	 * \return  The tangent vector. */
	CoreExport		Point3		TangentCurve3D(float u, int ptype=SPLINE_INTERP_SIMPLE);
	
	/** \deprecated This method should not be used. */
	CoreExport		Point3		AverageTangent(int i);
	
	/** \deprecated This method should not be used. */
	CoreExport		void		MakeBezCont(int i);
	
	/** \deprecated This method should not be used. */
	CoreExport		void		RedistTangents(int i, Point3 d);
	
	/** deprecated  This method should not be used. */
	CoreExport		void		FixAdjBezTangents(int i);
	
	/** \deprecated This method should not be used. */
	CoreExport		void		DrawCurve(GraphicsWindow *gw, Material *mtl);
	
	/** \deprecated This method should not be used. */
	inline			void		SetEditMode(int mode) { editMode = mode ? 1:0; }
	
	/** Returns nonzero if the knot type is KTYPE_AUTO. Zero otherwise.
	 * \param i The index of the knot. */
	CoreExport		int			IsAuto(int i);
	
	/** Returns nonzero if the knot type is KTYPE_BEZIER. Zero otherwise.
	 * \param i	The index of the knot. */
	CoreExport		int			IsBezierPt(int i);
	
	/** Returns nonzero if the knot type is KTYPE_CORNER, zero otherwise.
	 * \param i	The index of the knot. */
	CoreExport		int			IsCorner(int i);
	
	/** \internal This method is used internally. */
	CoreExport		Point3		GetDragVector(ViewExp *vpt,IPoint2 p,int i,Matrix3* mat);
	
	/** \internal This method is used internally. */
	CoreExport		int			AppendPoint(ViewExp *vpt,const Point3& p, int where = -1);
	
	/** \internal This method is used internally. */
	CoreExport		int			DrawPhase() { return drawPhase; }
	
	/** \internal This method is used internally. */
	CoreExport		int			GetiCur() { return iCur; }
	
	/** Returns the bounding box of the curve in the space specified
	 * by the tm in the input parameters.
	 * \param t	This parameter is not used.
	 * \param tm The tm to transform the points by prior to computing the bounding box.
	 * \param box The bounding box */
	CoreExport		void		GetBBox(TimeValue t,  Matrix3& tm, Box3& box);
	
	/** \internal This method is used internally. */
	CoreExport		IPoint2		ProjectPoint(ViewExp *vpt, Point3 fp, Matrix3 *mat);
	
	/** \internal This method is used internally. */
	CoreExport		Point3		UnProjectPoint(ViewExp *vpt, IPoint2 p, Matrix3 *mat);
	
	/** \internal This method is used internally. */
	CoreExport		void		Snap(GraphicsWindow *gw, SnapInfo *snap, IPoint2 *p, Matrix3 &tm);

	/** \internal This method is used internally to save the class data from
	disk. */
	CoreExport		IOResult 	Save(ISave *isave);
	
	/** \internal This method is used internally to load the class data from
	disk. */
	CoreExport		IOResult	Load(ILoad *iload);
	
	/** Opens or closes the spline. If the
	 * optional parameter is not specified it is closed.
	 * \param flag = 1. Nonzero to close; zero to open.
	 * \return  Nonzero if changed, zero if not changed. */
	CoreExport		int			SetClosed(int flag = 1);
	
	/** Sets the spline to open.
	 * \return  Nonzero if changed, zero if not changed. */
	CoreExport		int			SetOpen();
	
	/** Displays data about the specified knot using
	 * DebugPrints(). See
	 * <a href="ms-its:3dsmaxsdk.chm::/debug_debugging.html">Debugging</a>.
	 * \param where	The index of the knot. */
	CoreExport		void		Dump(int where);
	
	/**	Returns the 'i-th' in vector position in absolute coordinates.
	 * \param i	The vector position to be retrieved. */
	CoreExport 		Point3		GetInVec(int i);

	/**	Sets the 'i-th' in vector position in absolute coordinates.
	 * \param i	The position to be altered.
	 * \param p	The value to be set in absolute coordinates. */
	CoreExport		void		SetInVec(int i, const Point3 &p);
	
	/**	Returns the 'i-th' in vector position relative to	the knot point.
	 * \param i	The position to be returned. */
	CoreExport		Point3		GetRelInVec(int i);
	
	/**	Sets the 'i-th' bezier in vector position relative to the knot	point.
	 * \param i	The vector to be set.
	 * \param p	The vector data to set, relative to the knot point. */
	CoreExport		void		SetRelInVec(int i, const Point3 &p);
	
	/**	Returns the 'i-th' knot point.
	 * \param i	The knot point to be returned. */
	CoreExport		Point3		GetKnotPoint(int i);
	
	/**	Sets the 'i-th' knot point to the specified value.
	 * \param i The knot point to be set.\n\n
	 * \param p	The value to be set to. */
	CoreExport		void		SetKnotPoint(int i, const Point3 &p);
	
	/**	Returns the 'i-th' out vector position in absolute coordinates.
	 * \param i	The out vector point to be returned. */
	CoreExport		Point3		GetOutVec(int i);
	
	/**	Sets the 'i-th' out vector position in absolute	coordinates.
	 * \param i The out vector point to get.
	 * \param p	The out vector to set in absolute coordinates. */
	CoreExport		void		SetOutVec(int i, const Point3 &p);
	
	/**	Returns the 'i-th' out vector position relative to the knot point.
	 * \param i	Specifies the point to get. */
	CoreExport		Point3		GetRelOutVec(int i);
	
	/**	Sets the 'i-th' out vector position relative to the	knot point.
	 * \param i	Specifies the point to be set.
	 * \param p	The out vector position to set relative to the knot point. */
	CoreExport		void		SetRelOutVec(int i, const Point3 &p);
	
	// The following methods return absolute coords for the bezier vertices
	
	/** Returns an item in the in vector. Each control point is made up of 
	 * three points. The in vector coming off the bezier control point, 
	 * the knot point itself, and the out vector. There are these three 
	 * points for every control point. This method will return any item in this list.
	 * \param i	The index into the vertex list. */
	CoreExport		Point3		GetVert(int i);
	
	/** \deprecated This method should not be used. */
	CoreExport		void		SetVert(int i, const Point3& p);
	
	/** Returns the number of vertices. This is always the number of
	knots times 3. */	
	inline			int			Verts() { return knotCount*3; }
	
	// The following methods get/set the knot aux fields based on knot index
	// These are here for backward compatibility with MAXr2
	
	/**	Returns the auxiliary data associated with the specified knot. This is
	 * used internally for tracking topological changes to the spline during
	 * editing. Developers can use it for temporary purposes but it will be
	 * altered by the EditableSpline (SplineShape) code.
	 * \param knot The knot from whom the data is retrieved. */
	CoreExport		int			GetAux(int knot);
	
	/**	Sets the first integer auxiliary data associated with the specified knot.
	 * \param knot The knot whose auxiliary will be set.
	 * \param value	The value to be set to. */
	CoreExport		void		SetAux(int knot, int value);
	
	/**	Returns the second integer auxiliary data associated with the specified	knot
	 * \param knot The knot from whom the data is retrieved. */
	CoreExport		int			GetAux2(int knot);
	
	/**	Sets the second integer auxiliary data associated with the specified knot.
	 * \param knot	The knot whose auxiliary data will be set.
	 * \param value	The value to set to. */
	CoreExport		void		SetAux2(int knot, int value);
	
	/**	Returns the third integer auxiliary data associated with the specified
	 * knot. This field is available for any use.
	 * \param knot The knot whose auxiliary data is returned. */
	CoreExport		int			GetAux3(int knot);
	
	/**	Sets the third integer auxiliary data associated with the specified knot.
	 * This field is available for any use.
	 * \param knot The knot whose auxiliary data will be set.
	 * \param value	The value to set to. */
	CoreExport		void		SetAux3(int knot, int value);
	
	// The following methods are new to MAXr3 and get/set aux fields for in/knot/out
	// knot: knot index
	// which: 0=aux1 1=aux2 2=aux3
	
	/**	Returns the specified integer auxiliary data associated with the specified knot.
	 * \param knot The knot whose auxiliary data is returned.
	 * \param which	The auxiliary field. 0=aux1, 1=aux2 and 2=aux3. */
	CoreExport		int			GetKnotAux(int knot, int which);
	
	/**	Sets the specified integer auxiliary data associated with the specified	knot.
	 * \param knot Specifies the knot whose auxiliary data will be set.
	 * \param which The auxiliary field. 0=aux1, 1=aux2 and 2=aux3 .
	 * \param value	The value to set to. */
	CoreExport		void		SetKnotAux(int knot, int which, int value);
	
	/**	Returns the specified integer auxiliary data associated with the specified in vector.
	 * \param knot Specifies the knot whose auxiliary data will be returned.
	 * \param which The auxiliary field. 0=aux1, 1=aux2 and 2=aux3. */
	CoreExport		int			GetInAux(int knot, int which);
	
	/**	Sets the specified integer auxiliary data associated with the specified in vector.
	 * \param knot The knot whose auxiliary data will be set.
	 * \param which The auxiliary field. The auxiliary field. 0=aux1, 1=aux2 and 2=aux3.
	 * \param value	The value to set to. */
	CoreExport		void		SetInAux(int knot, int which, int value);
	
	/**	Returns the specified integer auxiliary data associated with the specified out vector.
	 * \param knot	The knot whose auxiliary data will be returned.
	 * \param which The auxiliary field. 0=aux1, 1=aux2 and 2=aux3.*/
	CoreExport		int			GetOutAux(int knot, int which);
	
	/**	Sets the specified integer auxiliary data associated with the specified	out vector.
	 * \param knot The knot whose auxiliary data will be set.
	 * \param which The auxiliary field. 0=aux1, 1=aux2 and 2=aux3.
	 * \param value	The value to be set to. */
	CoreExport		void		SetOutAux(int knot, int which, int value);
	
	// The following methods get/set the aux fields	based on bezier vertex index
	// i: bezier vertex index
	// which: 0=aux1 1=aux2 2=aux3
	
	/**	Returns the specified integer auxiliary data associated with the specified
	 * bezier vertex.
	 * \param i	The zero based bezier vertex index.
	 * \param which The auxiliary field. 0=aux1, 1=aux2 and 2=aux3.*/
	CoreExport		int			GetVertAux(int i, int which);
	
	/**	Sets the specified integer auxilliary data associated with the specified
	 * bezier vertex.
	 * \param i	The zero based bezier vertex index.
	 * \param which The auxiliary field. 0=aux1, 1=aux2 and 2=aux3.
	 * \param value	The value to be set to. */
	CoreExport		void		SetVertAux(int i, int which, int value);
	
	// The following methods get/set the material ID for a spline segment
	
	/**	Returns the material ID for the specified spline segment.
	 * \param seg The zero based index of the segment. */
	CoreExport		MtlID		GetMatID(int seg);
	
	/**	Sets the material ID for the specified spline segment.
	 * \param seg The zero based index of the segment.
	 * \param id The material ID to be set to. */
	CoreExport		void		SetMatID(int seg, MtlID id);

	/** Returns the length of the spline. */
	CoreExport		float		SplineLength();

	/**	Returns the length of the specified segment of this spline.
	 * \param seg The zero based index of the segment to check. */
	CoreExport		float		SegmentLength(int seg);
	
	/** Transforms the points of the spline	into another space 
	 * defined by the specified transformation matrix.
	 * \param tm The pointer to the transformation matrix. */
	CoreExport		void		Transform(Matrix3 *tm);

	/** Reverses all the points of the spline.
	 * \param keepZero Defaults to FALSE in 
	 * order to retain backwards compatibility.
	 * Setting it to TRUE insures that a closed spline will have the same vertex as
	 * its first point when it is reversed. The parameter is ignored on open splines.*/
	CoreExport		void		Reverse(BOOL keepZero = FALSE);
	
	/** Appends the specified spline onto the end of this
	 * one. The splines should both be opened.
	 * \param spline The spline to append.
	 * \param weldCoincidentFirstVertex Defaults to TRUE. Set this to TRUE to 
	 * weld coincident first vertices. Setting it to FALSE will disable welding.
	Returns TRUE if first point auto-welded	*/
	CoreExport		BOOL		Append(Spline3D *spline, BOOL weldCoincidentFirstVertex=TRUE);	

	/** Takes the specified spline and puts it on the front of this spline.
	 * \param spline The spline to perpend. 
	 * \return true if last point auto-welded, false otherwise. */
	CoreExport		BOOL		Prepend(Spline3D *spline, BOOL weldCoincidentLastVertex=TRUE);
	
	/** \returns true if the spline is clockwise in the XY plane (it
	 * ignores Z), false otherwise. This call is meaningless if the shape self intersects. */
	CoreExport		BOOL		IsClockWise();
	
	/** \returns true if the spline intersects itself in the XY plane
	 * (it ignores Z), false otherwise. */
	CoreExport		BOOL		SelfIntersects();

	/** Tells if the input spline intersects the specified spline
	 * in the XY plane (ignoring Z) or not.
	 * \param spline The pointer to the spline to check. 
	 * \return true if this spline intersects the specified spline
	 * in the XY plane (ignoring Z), false otherwise.*/
	CoreExport		BOOL		IntersectsSpline(Spline3D *spline);
	
	/** Tells if the specified 2-D point is surrounded (contained within) this spline or not.
	 * \param p	The point to check. 
	 * \return true if the specified point is surrounded (contained within) the current spline, false otherwise.*/
	CoreExport		BOOL		SurroundsPoint(Point2 p);

	/** Creates a PolyLine from this spline given a steps setting
	 * and an optimize parameter.
	 * Note the following constraints on this method. When a ShapeObject is asked to
	 * output a PolyShape with a given number of steps and FALSE is specified for
	 * optimization, it must output a PolyLine with [steps * pieces + pieces + 1]
	 * vertices if it's an open shape and [steps * pieces + pieces] vertices if it's closed.
	 * \param line The result is stored here. 
	 * \param steps Defaults to -1. The number of steps between knots in the spline.
	 * \param optimize Defaults to FALSE. If TRUE, linear segments between control 
	 * points in the spline will not generate steps in between. It will just be one line segment. */
	CoreExport		void		MakePolyLine(PolyLine &line, int steps = -1, BOOL optimize = FALSE);

	/** This method makes sure the shape has flushed out any cached
	 * data it may have had. */
	CoreExport		void		InvalidateGeomCache();

	/**	This method allows this Spline3D to create a map of smoothing groups that
	 * eases the creation of meshes. It fills in a developer supplied IntTab with 
	 * smoothing groups for each segment of the spline. A spline with 4 segments 
	 * will cause the IntTab to be set to 4 entries, for example.
	 * Five smoothing groups are used for this operation, 1\<\<0 through 1\<\<4.
	 * Once you have them, you can shift them as needed for your application. The
	 * smoothing groups are set up so that segments connected by knots with
	 * KTYPE_SMOOTH or KTYPE_BEZIER types are smoothed together.
	 * \param map A table of integers. See Template Class Tab. */
	CoreExport		void		GetSmoothingMap(IntTab &map);
	};

