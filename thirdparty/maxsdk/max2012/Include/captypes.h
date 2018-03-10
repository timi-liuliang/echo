/**********************************************************************
 *<
	FILE: captypes.h

	DESCRIPTION: Capping type defintions

	CREATED BY: Tom Hudson

	HISTORY: Created 12 October 1995

 *> Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "coreexp.h"
#include "PatchTypeConstants.h"
#include <WTypes.h>
#include "TabTypes.h"
#include "point3.h"

// forward declarations
class PolyShape;
class BezierShape;
class PatchMesh;
class Mesh;
class Matrix3;

// Capping types supported
/*! \defgroup capTypes Shape Capping Types */
//@{
/*! The capping code does the best job it can given this constraint, however it 
is possible to wind up with long sliver-like faces on the cap. This is referred
to as a morph cap because if you cap a shape using this method, it does not
generate any new vertices and you can then morph between shapes with the same
number of vertices. */
#define CAPTYPE_MORPH 0		//!< Uses the existing vertices in the PolyShape to generate the cap.
/*! This helps to break up the shape and helps reduce slivering. Grid capping
will generate a different number of vertices based on the shape and thus the
shapes are not morphable. */
#define CAPTYPE_GRID 1		//!< Generates new vertices in the interior of the shape in a grid pattern.
//@}

// Capping information classes:
// These classes provide information on how a cap is put together, based on the following:
//
// For Mesh caps, you get a list of created triangular faces, where the vertex indices are
// either original vertices on the PolyShape, newly created vertices inside the shape, or
// newly created vertices on the edge of a shape.  New vertices are only created for GRID
// type capping.  Cap info is always stored in unflipped form -- That is, faces are oriented
// in a counterclockwise order as viewed from the shape's "front", or positive Z.
//
// New free vertices are listed in the MeshCapInfo's "newVerts" table, a simple Point3 table.
// The "newVert" member of the MeshCapVert class points to the entry in the newVerts table.
//
// New edge vertices are stored with the polygon and segment number where they reside, along
// with a position on that segment (0-1) where they reside.  This information allows the cap
// user to divide adjacent faces as needed.
//
// For Patch caps, you can only cap using MORPH type capping.  GRID capping is meant for Mesh
// caps, where distorting a non-subdivided cap would result in serious surface discontinuities.
// Patches are automatically subdivided, so GRID capping is unnecessary there.
//

// CapFace flags 
#define CF_ABLINE (1<<0)
#define CF_BCLINE (1<<1)
#define CF_CALINE (1<<2)

class CapFace: public MaxHeapOperators {
	public:
		int va;	// Index of vertex a
		int vb;	// Index of vertex b
		int vc;	// Index of vertex c
		DWORD flags;
		CapFace() {}
		CapFace(int a, int b, int c, DWORD f) { va=a; vb=b; vc=c; flags=f; }
	};

// Mesh cap vertices:
// These can be original vertices from the PolyShape or new free vertices
// in the center of the PolyShape.

#define MCV_ORIGINAL	0
#define MCV_FREE		1

class MeshCapVert: public MaxHeapOperators {
	public:
		int type;		// See above
		int poly;		// The polygon number
		int index;		// The index of the vertex
		int newVert;	// The index of the new vertex
		MeshCapVert() {}
		MeshCapVert(int t, int p, int i, int nv=0) { type=t; poly=p; index=i; newVert=nv; }
	};

typedef Tab<CapFace> CapFaceTab;
typedef Tab<MeshCapVert> MeshCapVertTab;

// The information class for mesh capping (MORPH or GRID)

/*! \sa
<a href="ms-its:3dsmaxsdk.chm::/shps_shapes_and_splines.html">Working with
Shapes and Splines</a>.\n\n
\par Description:
The information class for mesh capping (MORPH or GRID). All the data members
and methods of this class are used internally. Developers must only declare an
instance of this class and then call <b>MakeCap()</b> on the shape.
\par Data Members:
<b>CapFaceTab faces;</b>\n\n
This is used internally.\n\n
<b>MeshCapVertTab verts;</b>\n\n
This is used internally.\n\n
<b>Point3Tab newVerts;</b>\n\n
This is used internally.  */
class MeshCapInfo: public MaxHeapOperators {
	public:
		CapFaceTab faces;
		MeshCapVertTab verts;
		Point3Tab newVerts;
		/*! \remarks Assignment operator used internally. */
		MeshCapInfo &operator=(MeshCapInfo& from) { faces=from.faces; verts=from.verts; newVerts=from.newVerts; return *this; }
		/*! \remarks This method is used internally. */
		CoreExport void Init(PolyShape* shape);
		/*! \remarks This method is used internally.
		\par Operators:
		*/
		CoreExport void FreeAll();
	};

// Support classes for MeshCapper

class PolyLine;

/*! \sa  Class MeshCapInfo.\n\n
\par Description:
A support class for the mesh capper. There is one of these for each polygon in
a shape to be capped. All methods of this class are implemented by the system.
\par Data Members:
<b>int numVerts;</b>\n\n
The number of vertices.\n\n
<b>int *verts;</b>\n\n
List of the verts in the mesh corresponding to the verts in the PolyLine (1 per
vertex).  */
class MeshCapPoly: public MaxHeapOperators {
	public:
		int numVerts;
		int *verts;	// List of verts in mesh corresponding to verts in the PolyLine (1 per vert)
		/*! \remarks Constructor. The <b>verts</b> pointer is set to NULL. */
		MeshCapPoly() { verts = NULL; }
		/*! \remarks This method is used internally. */
		CoreExport void Init(PolyLine &line);
		/*! \remarks Destructor. */
		CoreExport ~MeshCapPoly();
		/*! \remarks This establishes the corresponding Mesh vertex for a
		vertex in the PolyLine. This should be called for each vertex in the
		PolyLine.
		\par Parameters:
		<b>int index</b>\n\n
		The index into the shape.\n\n
		<b>int vertex</b>\n\n
		The corresponding index into the mesh. */
		CoreExport void SetVert(int index, int vertex);
	};

// This class is used to apply the MeshCapInfo data to a mesh -- It will modify the mesh as required to
// add the cap.  Simply fill in the vertices and faces bordering the cap, then call the CapMesh method.

/*! \sa  Class MeshCapInfo, Class PolyShape, Class MeshCapPoly.\n\n
\par Description:
This class is used to apply the <b>MeshCapInfo</b> data to a mesh. It will
modify the mesh as required to add the cap. Simply fill in the vertices and
faces bordering the cap, then call the <b>CapMesh()</b> method. All methods of
this class are implemented by the system.
\par Data Members:
<b>int numPolys;</b>\n\n
The number of polygons in the shape.\n\n
<b>MeshCapPoly *polys;</b>\n\n
One for each polygon in the shape.  */
class MeshCapper: public MaxHeapOperators {
	public:
		int numPolys;
		MeshCapPoly *polys;
		/*! \remarks Constructor. This gets the capper ready for the topology
		of the specified PolyShape.
		\par Parameters:
		<b>PolyShape \&shape</b>\n\n
		The PolyShape to initialize the cap data from. */
		CoreExport MeshCapper(PolyShape &shape);
		/*! \remarks Destructor.
		\par Operators:
		*/
		CoreExport ~MeshCapper();
		/*! \remarks Array access operator. Returns the MeshCapPoly as
		specified by the index. */
		CoreExport MeshCapPoly &operator[](int index);
		/*! \remarks This method is used to modify the mesh based on the
		MeshCapInfo, the flipped state, the smoothing group and an orientation
		matrix.
		\par Parameters:
		<b>Mesh \&mesh</b>\n\n
		The Mesh to modify.\n\n
		<b>MeshCapInfo \&capInfo</b>\n\n
		The MeshCapInfo describing the properties of the cap.\n\n
		<b>BOOL flip</b>\n\n
		A switch to indicate if the cap should be oriented normally or flipped.
		The MeshCapInfo is used for both the front and back side of a cap.
		Either the front or the back will have to be flipped. This switch is
		used to indicate if a particular one is flipped.\n\n
		<b>DWORD smooth</b>\n\n
		The smoothing group for all the faces in the cap.\n\n
		<b>Matrix3 *tm=NULL</b>\n\n
		The orientation matrix. A grid cap generates new vertices inside the
		shape that make up the grid. In the case of a SurfRev for example, the
		end cap might be rotated, or scaled in some manner. A matrix is
		required so the capper knows how to orient the vertices into the
		correct location. This matrix is ignored for non-grid capping.\n\n
		<b>int mtlID=-1</b>\n\n
		This parameter is available in release 2.0 and later only.\n\n
		Specifies the material ID of the faces that make up the cap. The
		default for this value is -1, which causes the capper to use material
		ID 0 for non-flipped caps and ID 1 for flipped caps. Note that these ID
		values are zero-based, while the UI display is 1-based.
		\return  Nonzero if the mesh was modified; otherwise zero. */
		CoreExport int CapMesh(Mesh& mesh, MeshCapInfo& capInfo, BOOL flip, DWORD smooth, Matrix3* tm = NULL, int mtlID = -1);
	};

// Patch capping

class CapPatch: public MaxHeapOperators {
	public:
		int type;		// PATCH_TRI or PATCH_QUAD
		int verts[4];
		int vecs[8];
		int interior[4];
		CapPatch() {}
		CapPatch(int va, int vab, int vba, int vb, int vbc, int vcb, int vc, int vca, int vac, int i1, int i2, int i3) {
			type=PATCH_TRI; verts[0]=va; verts[1]=vb; verts[2]=vc; vecs[0]=vab; vecs[1]=vba; vecs[2]=vbc, vecs[3]=vcb;
			vecs[4]=vca; vecs[5]=vac; interior[0]=i1; interior[1]=i2; interior[2]=i3; }
		CapPatch(int va, int vab, int vba, int vb, int vbc, int vcb, int vc, int vcd, int vdc, int vd, int vda, int vad, int i1, int i2, int i3, int i4) {
			type=PATCH_QUAD; verts[0]=va; verts[1]=vb; verts[2]=vc; verts[3]=vd; vecs[0]=vab; vecs[1]=vba; vecs[2]=vbc, vecs[3]=vcb;
			vecs[4]=vcd; vecs[5]=vdc; vecs[6]=vda, vecs[7]=vad;  interior[0]=i1; interior[1]=i2; interior[2]=i3; interior[3]=i4; }
	};

// Patch cap vertices:
// These can be original vertices from the BezierShape or new free vertices
// in the center of the BezierShape.

#define PCVERT_ORIGINAL	0
#define PCVERT_FREE		1

/*! \sa \ref capTypes.\n
\par Description:
Patch cap vertices. These can be original vertices from the BezierShape or new
free vertices in the center of the BezierShape. All methods of this class are
implemented by the system.
\par Data Members:
<b>int type;</b>\n\n
Capping types supported. See \ref capTypes.\n\n
<b>int poly;</b>\n\n
The polygon number (ORIGINAL or EDGE)\n\n
<b>int index;</b>\n\n
The index of the vertex (ORIGINAL) or the segment for the EDGE vertex  */
class PatchCapVert: public MaxHeapOperators {
	public:
		int type;
		int poly;		// The polygon number (ORIGINAL or EDGE)
		int index;		// The index of the vertex (ORIGINAL) or the segment for the EDGE vertex
		/*! \remarks Constructor. No initialization is performed. */
		PatchCapVert() {}
		/*! \remarks Constructor. The data members are initialized to the
		values passed. */
		PatchCapVert(int t, int p, int i) { type=t; poly=p; index=i; }
	};

// Patch cap vectors:
// When a patch cap is generated, new interior vectors will be generated within the patch, and patch
// edges within the cap will have new vectors.  Patch edges along the edges of the originating bezier
// shape will use existing vectors.  This class provides information on which is which.

#define PCVEC_ORIGINAL	0
#define PCVEC_NEW		1

/*! class PatchCapVec
\par Description:
Patch cap vectors. When a patch cap is generated, new interior vectors will be
generated within the patch, and patch edges within the cap will have new
vectors. Patch edges along the edges of the originating bezier shape will use
existing vectors. This class provides information on which is which. All
methods of this class are implemented by the system.
\par Data Members:
<b>int type;</b>\n\n
Capping types supported. See \ref capTypes.\n\n
<b>int poly;</b>\n\n
Polygon number for ORIGINAL\n\n
<b>int index;</b>\n\n
Index for ORIGINAL or into newVecs table.  */
class PatchCapVec: public MaxHeapOperators {
	public:
		int type;		// See above
		int poly;		// Polygon number for ORIGINAL
		int index;		// Index for ORIGINAL or into newVecs table (see below)
		/*! \remarks Constructor. No initialization is performed. */
		PatchCapVec() {}
		/*! \remarks Constructor. The data members are initialized to the
		values passed. */
		PatchCapVec(int t, int p, int i) { type=t; poly=p; index=i; }
	};

typedef Tab<CapPatch> CapPatchTab;
typedef Tab<PatchCapVert> PatchCapVertTab;
typedef Tab<PatchCapVec> PatchCapVecTab;

// The information class for patch capping

/*! \sa
<a href="ms-its:3dsmaxsdk.chm::/shps_shapes_and_splines.html">Working with
Shapes and Splines</a>.\n\n
\par Description:
This is the information class for patch capping. All methods of this class are
implemented by the system. Developers must only declare an instance of this
class and then call <b>MakeCap()</b> on the shape.
\par Data Members:
<b>CapPatchTab patches;</b>\n\n
This is used internally.\n\n
<b>PatchCapVertTab verts;</b>\n\n
This is used internally.\n\n
<b>PatchCapVecTab vecs;</b>\n\n
This is used internally.\n\n
<b>Point3Tab newVerts;</b>\n\n
This is used internally.\n\n
<b>Point3Tab newVecs;</b>\n\n
This is used internally.  */
class PatchCapInfo: public MaxHeapOperators {
	public:
		CapPatchTab patches;
		PatchCapVertTab verts;
		PatchCapVecTab vecs;
		Point3Tab newVerts;
		Point3Tab newVecs;
		/*! \remarks Assignment operator used internally. */
		PatchCapInfo &operator=(PatchCapInfo &from) { patches=from.patches; verts=from.verts; vecs=from.vecs; newVerts=from.newVerts; newVecs=from.newVecs; return *this; }
		/*! \remarks This method is used internally. */
		CoreExport void Init(BezierShape *shape);
		/*! \remarks This method is used internally.
		\par Operators:
		*/
		CoreExport void FreeAll();
	};

// Support classes for MeshCapper

class Spline3D;

/*! \sa  Class Spline3D, <a href="ms-its:3dsmaxsdk.chm::/shps_shapes_and_splines.html">Working with
Shapes and Splines</a>.\n\n
\par Description:
A support class for the PatchCapper. All methods of this class are implemented
by the system.
\par Data Members:
<b>int numVerts;</b>\n\n
The number of vertices.\n\n
<b>int numVecs;</b>\n\n
The number of vectors.\n\n
<b>int *verts;</b>\n\n
List of verts in patch mesh corresponding to verts in the spline (1 per
vert)\n\n
<b>int *vecs;</b>\n\n
List of vecs in patch mesh corresponding to vecs in the spline (1 per vector)
 */
class PatchCapPoly: public MaxHeapOperators {
	public:
		int numVerts;
		int numVecs;
		int *verts;	// List of verts in patch mesh corresponding to verts in the spline (1 per vert)
		int *vecs;	// List of vecs in patch mesh corresponding to vecs in the spline (1 per vector)
		/*! \remarks Constructor. The <b>verts</b> and <b>vecs</b> are set to
		NULL. */
		PatchCapPoly() { verts = vecs = NULL; }
		/*! \remarks This method is used internally. */
		CoreExport void Init(Spline3D &spline);
		/*! \remarks Destructor. The <b>verts</b> and <b>vecs</b> are freed.
		*/
		CoreExport ~PatchCapPoly();
		/*! \remarks This establishes the corresponding mesh vertex for a
		vertex in the PolyLine. This should be called for each vertex in the
		PolyLine.
		\par Parameters:
		<b>int index</b>\n\n
		The index into the shape.\n\n
		<b>int vertex</b>\n\n
		The corresponding index into the mesh. */
		CoreExport void SetVert(int index, int vertex);
		/*! \remarks This establishes the corresponding mesh vertex for a
		vector in the PolyLine. This should be called for each vertex in the
		PolyLine.
		\par Parameters:
		<b>int index</b>\n\n
		The index into the shape.\n\n
		<b>int vector</b>\n\n
		The corresponding index into the patch. */
		CoreExport void SetVec(int index, int vector);
	};

// This class is used to apply the PatchCapInfo data to a PatchMesh -- It will modify the mesh as required to
// add the cap.  Simply fill in the vertices, vectors and patches bordering the cap, then call the CapPatch method.

/*! \sa  Class PatchCapPoly, Class BezierShape, Class PatchMesh, <a href="ms-its:3dsmaxsdk.chm::/shps_shapes_and_splines.html">Working with
Shapes and Splines</a>.\n\n
\par Description:
This class is used to apply the <b>PatchCapInfo</b> data to a <b>PatchMesh</b>.
It will modify the mesh as required to add the cap. Simply fill in the
vertices, vectors and patches bordering the cap, then call the CapPatch method.
All methods of this class are implemented by the system.
\par Data Members:
<b>int numPolys;</b>\n\n
The number of polygons.\n\n
<b>PatchCapPoly *polys;</b>\n\n
A pointer to the PatchCapPoly.  */
class PatchCapper: public MaxHeapOperators {
	public:
		int numPolys;
		PatchCapPoly *polys;
		/*! \remarks Constructor.
		\par Parameters:
		<b>BezierShape \&shape</b>\n\n
		See Class BezierShape. */
		CoreExport PatchCapper(BezierShape &shape);
		/*! \remarks Destructor. */
		CoreExport ~PatchCapper();
		/*! \remarks Array access operator. */
		CoreExport PatchCapPoly &operator[](int index);
		/*! \remarks This method is used to modify the mesh based on the
		PatchCapInfo, the flipped state, the smoothing group and an orientation
		matrix.
		\par Parameters:
		<b>PatchMesh \&mesh</b>\n\n
		The PatchMesh to modify.\n\n
		<b>PatchCapInfo \&capInfo</b>\n\n
		The PatchCapInfo describing the cap. See
		Class PatchCapInfo.\n\n
		<b>BOOL flip</b>\n\n
		A switch to indicate if the cap should be oriented normally or flipped.
		The PatchCapInfo is used for both the front and back side of a cap.
		Either the front or the back will have to be flipped. This switch is
		used to indicate if a particular one is flipped.\n\n
		<b>DWORD smooth</b>\n\n
		The smoothing group for all the faces in the cap.\n\n
		<b>Matrix3 *tm=NULL</b>\n\n
		The orientation matrix. A grid cap generates new vertices inside the
		shape that make up the grid. In the case of a SurfRev for example, the
		end cap might be rotated, or scaled in some manner. A matrix is
		required so the capper knows how to orient the vertices into the
		correct location. This matrix is ignored for non-grid capping.\n\n
		<b>int mtlID=-1</b>\n\n
		Specifies the material ID of the patches that make up the cap. The
		default for this value is -1, which causes the capper to use material
		ID 0 for non-flipped caps and ID 1 for flipped caps. Note that these ID
		values are zero-based, while the UI display is 1-based.
		\return  Nonzero if the mesh was modified; otherwise zero.
		\par Operators:
		*/
		CoreExport int CapPatchMesh(PatchMesh& mesh, PatchCapInfo& capInfo, BOOL flip, DWORD smooth, Matrix3* tm = NULL, int mtlID = -1);
	};


