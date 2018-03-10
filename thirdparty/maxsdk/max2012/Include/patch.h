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
// FILE:        patch.h
// DESCRIPTION: Main include file for bezier patches
// AUTHOR:      Tom Hudson
// HISTORY:     Created June 21, 1995
//              June 17, 1997 TH -- Added second texture mapping channel
//              12-10-98 Peter Watje added hide interior edge support and hidding patches
//              12-31-98 Peter Watje added hook patches, patch extrusion and bevels
//**************************************************************************/

#pragma once

#include "maxheap.h"
#include "coreexp.h"
#include "meshlib.h"
#include "hitdata.h"
#include "maxtess.h"
#include "PatchTypeConstants.h"

// Uncomment the following to check for missed triangular patch 'aux' computation
//#define CHECK_TRI_PATCH_AUX

// Value for undefined patches and vertices
#define PATCH_UNDEFINED -1

// TH 5/17/99 -- Commented out MULTI_PROCESSING, it wasn't being used and was causing
// am obscure memory leak (Defect 180889)
//#define MULTI_PROCESSING	TRUE		// TRUE turns on mp vertex transformation

class HookPoint: public MaxHeapOperators 
	{
public:
	int upperPoint, lowerPoint;
	int upperVec,lowerVec;
	int upperHookVec, lowerHookVec;
	int hookPoint;
	int upperPatch, lowerPatch, hookPatch;
	int hookEdge, upperEdge, lowerEdge;
	};

class ExtrudeData: public MaxHeapOperators
{
public:
	int u,l,uvec,lvec;
//3-10-99 watje
	Point3 edge;
	Point3 bevelDir;
};

class ISave;
class ILoad;
class PatchMesh;

#define NEWPATCH

class PRVertex: public MaxHeapOperators {
	public:
		PRVertex()	{ rFlags = 0; /*ern = NULL;*/ }
		CoreExport ~PRVertex();	

		DWORD		rFlags;     
		int			pos[3];	
	};					  

// Patch vector flags
#define PVEC_INTERIOR	(1<<0)
#define PVEC_INTERIOR_MASK	0xfffffffe

// Vector flag processing tables
#define NUM_PATCH_VEC_FLAGS 1
const DWORD PatchVecFlagMasks[] = {1};
const int PatchVecFlagShifts[] = {0};

// Patch vectors

/*! \sa  Class PatchMesh, <a href="ms-its:3dsmaxsdk.chm::/patches_root.html">Working with Patches</a>.\n\n
\par Description:
This class represents a patch vector. This can be either an interior vector or
an edge vector. All methods of this class are implemented by the system.
\par Data Members:
<b>Point3 p;</b>\n\n
The vertex location.\n\n
<b>int vert;</b>\n\n
The vertex which owns this vector.\n\n
<b>IntTab patches;</b>\n\n
These are the patches that share this vector. If the edge is open there will
only be one patch. In this case <b>patches[0]</b> will be used and
<b>patches[1]</b> will be -1. Note: As of R4.0 the previous array of [2] has
been replaced with the IntTab because vectors can now be used by more than two
patches.\n\n
<b>DWORD flags;</b>\n\n
The patch vector flag:\n\n
<b>PVEC_INTERIOR</b>\n\n
This indicates the vector is an interior vector. These are the three vectors
inside a tri patch or the four inside a quad patch.\n\n
<b>int aux1;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
Used to track topology changes during editing (Edit Patch).\n\n
<b>int aux2;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
Used to track topology changes during editing (PatchMesh).  */
class PatchVec: public MaxHeapOperators {
	public:
		Point3 p;			// Location
		int vert;			// Vertex which owns this vector
		IntTab patches;		// List of patches using this vector
		DWORD flags;
		int aux1;			// Used to track topo changes during editing (Edit Patch)
		int aux2;			// Used to track topo changes during editing (PatchMesh)
		/*! \remarks Constructor. The location is set to 0,0,0. The vertex
		owner is set to undefined. The patches using the vector is set to
		undefined. The flags are set to 0. */
		CoreExport PatchVec();
		/*! \remarks Constructor. The data members are initialized to those of
		the <b>from</b> patch vector.
		\par Parameters:
		<b>PatchVec \&from</b>\n\n
		The vector to copy from. */
		CoreExport PatchVec(PatchVec &from);
		/*! \remarks Resets the data members. The vertex owner is set to
		undefined. The patches using the vector is set to undefined. */
		void ResetData() { vert = PATCH_UNDEFINED; patches.Delete(0,patches.Count());}
		/*! \remarks Adds the specified patch to this vector table.
		\par Parameters:
		<b>int index</b>\n\n
		The index in the <b>PatchMesh</b> class patches table (<b>patches</b>)
		of the patch to add.
		\return  Returns TRUE if the patch was added; otherwise FALSE. */
		CoreExport BOOL AddPatch(int index);
		/*! \remarks Assignment operator.
		\par Parameters:
		<b>PatchVec\& from</b>\n\n
		The patch vector to copy from. */
		CoreExport PatchVec& operator=(PatchVec& from);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Transforms the vertex location p by the specified matrix.
		\par Parameters:
		<b>Matrix3 \&tm</b>\n\n
		The matrix to transform the vertex. */
		void Transform(Matrix3 &tm) { p = p * tm; }

		/*! \remarks This is used internally to save the data to the .MAX
		file. */
		CoreExport IOResult Save(ISave* isave);
		/*! \remarks This is used internally to load the data from the .MAX
		file.
		\par Operators:
		*/
		CoreExport IOResult Load(ILoad* iload);
	};

// Patch vertex flags
#define PVERT_COPLANAR	(1<<0)
#define PVERT_CORNER (0)
#define PVERT_TYPE_MASK 0xfffffffe
//watje 12-10-98
#define PVERT_HIDDEN	(1<<1)
#define PVERT_HIDDEN_MASK 0xfffffffd
// CAL-04/28/03
#define PVERT_RESET		(1<<2)

// Vertex flag processing tables
#define NUM_PATCH_VERT_FLAGS 2
const DWORD PatchVertFlagMasks[] = {1, 1};
const int PatchVertFlagShifts[] = {0, 1};

// Patch vertex

/*! \sa  Class PatchMesh, Template Class Tab, <a href="ms-its:3dsmaxsdk.chm::/patches_root.html">Working with Patches</a>.
\par Description:
This class stores the information associated with a patch vertex and provides
methods to access the data associated with this vertex. All methods of this
class are implemented by the system.
\par Data Members:
<b>Point3 p;</b>\n\n
The vertex location.\n\n
<b>IntTab vectors;</b>\n\n
The list of vectors attached to this vertex. There can be any number of vectors
attached to a vertex. For example consider the north pole of a sphere made from
a set of triangular patches. If there were 16 patches meeting at this point
there would be 16 vectors. The table contains the indices of these vectors.
This is set up automatically when a developer calls <b>buildLinkages()</b>.\n\n
Note: <b>typedef Tab\<int\> IntTab;</b>\n\n
Note that the methods below allow a developer to manipulate the tables of this
class. Developers must be careful when doing so as it is easy to corrupt the
patch data structures. It may be easier for developers to manipulate the
patches, delete vertices, etc., by manipulating them in the <b>PatchMesh</b>
and then call <b>buildLinkages()</b> again. The methods below do work however
and may be used.\n\n
<b>IntTab patches;</b>\n\n
The list of patches using this vertex.\n\n
<b>IntTab edges;</b>\n\n
This data member is available in release 4.0 and later only.\n\n
The list of edges using this vertex. This table will be set up automatically
when a developer calls <b>buildLinkages()</b>.\n\n
<b>DWORD flags;</b>\n\n
The patch vertex flags\n\n
<b>PVERT_COPLANAR</b>\n\n
This constrains things such that this vertex and all the vectors that are
attached to it are coplanar. If this is set, and you call
<b>ApplyConstraints()</b>, the system will adjust the vectors to the average
plane that they are in and then constrain them to it.\n\n
<b>PVERT_CORNER</b>\n\n
The vertex is a corner.\n\n
<b>PVERT_HIDDEN</b>\n\n
The vertex is hidden.\n\n
<b>int aux1;</b>\n\n
Used to track topo changes during editing (Edit Patch).\n\n
<b>int aux2;</b>\n\n
Used to track topology changes during editing (PatchMesh).  */
class PatchVert: public MaxHeapOperators {
	public:
		Point3 p;			// Location
		IntTab vectors;		// List of vectors attached to this vertex
		IntTab patches;		// List of patches using this vertex
		IntTab edges;		// List of edges using this vertex
		DWORD flags;
		int aux1;			// Used to track topo changes during editing (Edit Patch)
		int aux2;			// Used to track topo changes during editing (PatchMesh)
		/*! \remarks Constructor. The location is set to 0,0,0. The flags are
		set to 0. */
		CoreExport PatchVert();
		/*! \remarks Constructor. The data members are copied from the from
		<b>PatchVert</b>.
		\par Parameters:
		<b>PatchVert \&from</b>\n\n
		The source PatchVert. */
		CoreExport PatchVert(PatchVert &from);
		/*! \remarks Destructor. Deletes the elements from the <b>vectors</b>
		table and <b>patches</b> table. */
		~PatchVert() { ResetData(); }
		/*! \remarks Assignment operator.
		\par Parameters:
		<b>PatchVert\& from</b>\n\n
		The patch vertex to copy from. */
		CoreExport PatchVert& operator=(PatchVert& from);
		/*! \remarks This method deletes the elements from the <b>vectors</b>
		table and <b>patches</b> table. */
		CoreExport void ResetData();
		/*! \remarks Returns the index in this classes <b>vectors</b> table of
		the vector whose index is passed. If not found, -1 is returned.
		\par Parameters:
		<b>int index</b>\n\n
		The index in the <b>PatchMesh</b> class vectors table (<b>vecs</b>) of
		the vector to find. */
		CoreExport int FindVector(int index);
		/*! \remarks Adds the specified vector to this vector table.
		\par Parameters:
		<b>int index</b>\n\n
		The index in the <b>PatchMesh</b> class vectors table (<b>vecs</b>) of
		the vector to add. */
		CoreExport void AddVector(int index);
		/*! \remarks Deletes the specified vector from this vector table.
		\par Parameters:
		<b>int index</b>\n\n
		The index in the <b>PatchMesh</b> class vectors table (<b>vecs</b>) of
		the vector to delete. */
		CoreExport void DeleteVector(int index);
		/*! \remarks Returns the index in this classes <b>patches</b> table of
		the patch whose index is passed. If not found, -1 is returned.
		\par Parameters:
		<b>int index</b>\n\n
		The index in the <b>PatchMesh</b> class patches table (<b>patches</b>)
		of the patch to find. */
		CoreExport int FindPatch(int index);
		/*! \remarks Adds the specified patch to this vector table.
		\par Parameters:
		<b>int index</b>\n\n
		The index in the <b>PatchMesh</b> class patches table (<b>patches</b>)
		of the patch to add. */
		CoreExport void AddPatch(int index);
		/*! \remarks Deletes the patch specified by the index.
		\par Parameters:
		<b>int index</b>\n\n
		The index in the <b>PatchMesh</b> class patches table (<b>patches</b>)
		of the patch to delete. */
		CoreExport void DeletePatch(int index);
		/*! \remarks Returns the index in this classes <b>edges</b> table of
		the patch whose index is passed. If not found, -1 is returned.
		\par Parameters:
		<b>int index</b>\n\n
		The index in the <b>PatchMesh</b> class edges table (<b>edges</b>) of
		the edge to find. */
		CoreExport int FindEdge(int index);
		/*! \remarks Adds the specified edge to this vector table.
		\par Parameters:
		<b>int index</b>\n\n
		The index in the <b>PatchMesh</b> class edges table (<b>edges</b>) of
		the edge to add. */
		CoreExport void AddEdge(int index);
		/*! \remarks Deletes the edge specified by the index.
		\par Parameters:
		<b>int index</b>\n\n
		The index in the <b>PatchMesh</b> class edges table (<b>edges</b>) of
		the edge to delete. */
		CoreExport void DeleteEdge(int index);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Transform the vertex by the specified matrix.
		\par Parameters:
		<b>Matrix3 \&tm</b>\n\n
		The matrix which transforms the point. */
		void Transform(Matrix3 &tm) { p = p * tm; }

//watje  12-10-98
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the hidded state of the vertex.
		\par Parameters:
		<b>BOOL sw = TRUE</b>\n\n
		TRUE to set to hidden; FALSE for visible. */
		CoreExport void SetHidden(BOOL sw = TRUE)
			{
			if(sw)
				flags |= PVERT_HIDDEN;
			else
				flags &= ~PVERT_HIDDEN;
			}
//watje  12-10-98
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns TRUE if the vertex is hidden; otherwise FALSE. */
		BOOL IsHidden() { return (flags & PVERT_HIDDEN) ? TRUE : FALSE; }

		/*! \remarks This method is used internally to save the class data to
		disk storage. */
		CoreExport IOResult Save(ISave* isave);
		/*! \remarks This method is used internally to load the class data
		from disk storage.
		\par Operators:
		*/
		CoreExport IOResult Load(ILoad* iload);
	};

/*! \sa  Class PatchMesh, Template Class Tab, <a href="ms-its:3dsmaxsdk.chm::/patches_root.html">Working with Patches</a>.
\par Description:
This class is available in release 4.0 and later only.\n\n
This class stores the texture vertex information associated with a patch and
provides methods to access this. All methods of this class are implemented by
the system.
\par Data Members:
<b>UVVert p;</b>\n\n
The texture vertex location.\n\n
<b>int aux1;</b>\n\n
Used to track topology changes during editing (Edit Patch).  */
class PatchTVert: public MaxHeapOperators {
	public:
		UVVert p;			// Location
		int aux1;			// Used to track topo changes during editing (Edit Patch)
		/*! \remarks Constructor. The UVVert is set to 0,0,0. The aux member is set to
		-1. */
		CoreExport PatchTVert() { p = UVVert(0,0,0); aux1 = -1; }
		/*! \remarks Constructor. The UVVert is set to the provided uvw
		parameters passed to the method. The aux member is set to -1.
		\par Parameters:
		<b>float u, float v, float w;</b>\n\n
		The u, v, and w values for the texture vertex.
		\par Operators:
		*/
		CoreExport PatchTVert(float u, float v, float w) { p=UVVert(u,v,w);	aux1 = -1; }
		/*! \remarks Conversion operator. Returns a reference to UVVert p. */
		CoreExport operator UVVert&() { return p; }
		/*! \remarks Assignment operator.
		\par Parameters:
		<b>UVVert\& from</b>\n\n
		The texture vertex to copy from. */
		CoreExport PatchTVert& operator=(const UVVert &from) { p=from; return *this; }
	};

/*! \sa  Class PatchMesh, <a href="ms-its:3dsmaxsdk.chm::/patches_root.html">Working with Patches</a>.\n\n
\par Description:
This class describes a patch edge using the vertices at the edge ends, and the
indices of the patches sharing the edge. All methods of this class are
implemented by the system.
\par Data Members:
<b>int v1;</b>\n\n
Index of the first vertex.\n\n
<b>int vec12;</b>\n\n
Vector from <b>v1</b> to <b>v2</b>.\n\n
<b>int vec21;</b>\n\n
Vector from <b>v2</b> to <b>v1</b>.\n\n
<b>int v2;</b>\n\n
Index of second vertex.\n\n
<b>IntTab patches;</b>\n\n
Index of the patches using this edge. If the edge is only used by one patch,
patches[1] will be less than zero. Note: Previous to R4.0 two separate integer
variables (<b>patch1</b> and <b>patch2</b>) were used.\n\n
<b>int aux1;</b>\n\n
This is used to track topology changes during editing (Edit Patch).\n\n
<b>int aux2;</b>\n\n
This is used to track topology changes during editing (PatchMesh).  */
class PatchEdge: public MaxHeapOperators {
	public:
		int v1;		// Index of first vertex
		int vec12;	// Vector from v1 to v2
		int vec21;	// Vector from v2 to v1
		int v2;		// Index of second vertex
		IntTab patches;	// List of patches using this edge
		int aux1;	// Used to track topo changes during editing (Edit Patch)
		int aux2;	// Used to track topo changes during editing (PatchMesh)
		/*! \remarks Constructor. The data members are initialized as
		undefined. */
		CoreExport PatchEdge();
		/*! \remarks Copy Constructor. The data members are initialized from the
		<b>PatchEdge</b> passed. */
		CoreExport PatchEdge(const PatchEdge &from);
		/*! \remarks Constructor. The data members are initialized to the
		values passed. */
		CoreExport PatchEdge(int v1, int vec12, int vec21, int v2, int p1, int p2, int aux1=-1, int aux2=-1);
		// Dump the patch edge structure via DebugPrints
		/*! \remarks You may call this method to dump the patch edge structure
		via <b>DebugPrint()</b>. See
		<a href="ms-its:3dsmaxsdk.chm::/debug_debugging.html">Debugging</a>. */
		CoreExport void Dump();
		/*! \remarks This is used internally to save the data to the .MAX
		file. */
		CoreExport IOResult Save(ISave* isave);
		/*! \remarks This is used internally to load the data from the .MAX
		file. */
		CoreExport IOResult Load(ILoad* iload);
	};

// Patch Flags:
// WARNING:  If you add flags here, you'll need to update the table below...
#define PATCH_AUTO			(1<<0)	// Interior verts computed automatically if set
#define PATCH_MANUAL		(0)		// Interior verts stored in 'interior' array
#define PATCH_INTERIOR_MASK 0xfffffffe
//watje 12-10-98
#define PATCH_HIDDEN		(1<<1)  //patch is hidden

//watje new patch mapping
#define PATCH_LINEARMAPPING		(1<<2)  //patch uses the old liunear mapping scheme else use the new mapping

#define PATCH_USE_CURVED_MAPPING_ON_VERTEX_COLOR		(1<<3)  //patch will use the new curved mapping for vertex colors also

// The mat ID is stored in the HIWORD of the patch flags
#define PATCH_MATID_SHIFT	16
#define PATCH_MATID_MASK	0xFFFF

// Patch flag processing tables
#define NUM_PATCH_PATCH_FLAGS 5
const DWORD PatchPatchFlagMasks[] = {1, 1, 1, 1, 0xffff};
const int PatchPatchFlagShifts[] = {0, 1, 2, 3, 16};

/*! \sa  Class PatchMesh, Class TVPatch, <a href="ms-its:3dsmaxsdk.chm::/patches_root.html">Working with Patches</a>.\n\n
\par Description:
A <b>PatchMesh</b> is made up of a series of <b>Patch</b> objects derived from
this class. This is similar to the way faces relate to a mesh. All methods of
this class are implemented by the system.
\par Data Members:
<b>int type;</b>\n\n
The patch type. One of the following values:\n\n
<b>PATCH_UNDEF</b>\n\n
Undefined.\n\n
<b>PATCH_TRI</b>\n\n
Triangular patch.\n\n
<b>PATCH_QUAD</b>\n\n
Quadrilateral patch.\n\n
<b>int v[4];</b>\n\n
A patch can have three or four vertices based on the <b>type</b>. The corner
vertices on a patch are referred to as <b>a</b>, <b>b</b>, <b>c</b>, (and if
it's a quad patch <b>d</b>). These are ordered <b>a, b, c, d</b> going
counter-clockwise around the patch.\n\n
<b>int vec[8];</b>\n\n
A patch can have six or eight vector points. The vectors are referred to as
follows: <b>ab</b> is the vector coming out of vertex <b>a</b> towards
<b>b</b>. <b>ba</b> is the one coming out of <b>b</b> towards <b>a</b>.
<b>bc</b> is the vector coming out of <b>b</b> towards <b>c</b>. <b>cb</b> is
the one coming out of <b>c</b> towards <b>b</b>, and so on.\n\n
<b>int interior[4];</b>\n\n
A patch can have three or four interior vertices.\n\n
<b>Point3 aux[9];</b>\n\n
This is used internally for triangular patches only (degree 4 control
points).\n\n
<b>int edge[4];</b>\n\n
Pointers into the edge list. There can be three or four depending on the patch
<b>type</b>.\n\n
<b>DWORD smGroup;</b>\n\n
The smoothing group. This defaults to 1. All patches are smoothed in a
<b>PatchMesh</b>.\n\n
<b>DWORD flags;</b>\n\n
Patch Flags. The following value may be set.\n\n
<b>PATCH_AUTO</b>\n\n
Interior vertices are computed automatically if this flag is set (and normally
it is set). An example of when this flag would not be set is if you were
creating a primitive using patches that needed to make special interior control
points to create the shape. In this case you'd clear this flag and then put
whatever values you needed into the <b>vec</b> array.\n\n
<b>PATCH_HIDDEN</b>\n\n
The patch is hidden.\n\n
<b>PATCH_LINEARMAPPING</b>\n\n
This option is available in release 4.0 and later only.\n\n
The patch is using the old linear mapping scheme.\n\n
<b>PATCH_USE_CRUVED_MAPPING_ON_VERTEX_COLOR</b>\n\n
This option is available in release 4.0 and later only.\n\n
The patch is using the new curved mapping for vertex colors also.\n\n
<b>int aux1;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is used to track topology changes during editing (Edit Patch).\n\n
<b>int aux2;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is used to track topology changes during editing (PatchMesh).  */
class Patch : public BaseInterfaceServer {	
	public:
		int type;			// See types, above
		int	v[4];			// Can have three or four vertices
		int	vec[8];			// Can have six or eight vector points
		int	interior[4];	// Can have one or four interior vertices
		Point3 aux[9];		// Used for triangular patches only -- Degree 4 control points
		int	edge[4];		// Pointers into edge list -- Can have three or four
		DWORD	smGroup;	// Defaults to 1 -- All patches smoothed in a PatchMesh
		DWORD	flags;		// See flags, above
		int aux1;			// Used to track topo changes during editing (Edit Patch)
		int aux2;			// Used to track topo changes during editing (PatchMesh)

#ifdef CHECK_TRI_PATCH_AUX
		Point3 auxSource[9];
		CoreExport void CheckTriAux(PatchMesh *pMesh);
#endif //CHECK_TRI_PATCH_AUX

		/*! \remarks Constructor. The type is set to undefined, the smooth
		group is set to 1, and the flags indicate automatic interior points.
		Note: This constructor does not allocate arrays. Use
		<b>SetType(type)</b>. */
		CoreExport Patch();	// WARNING: This does not allocate arrays -- Use SetType(type) or Patch(type)
		CoreExport Patch(int type);
		/*! \remarks Constructor. The <b>this</b> pointer is set to the
		<b>fromPatch</b>. */
		CoreExport Patch(Patch& fromPatch);
		/*! \remarks Destructor. */
		CoreExport ~Patch();
		/*! \remarks This method is used by the constructors internally.
		Developers should not call this method. */
		CoreExport void Init();
		/*! \remarks The vertices are copied from the array passed. Based on
		the patch type either three or four vertices are copied.
		\par Parameters:
		<b>int *vrt</b>\n\n
		The vertices are set to these values. These values are indices into the
		<b>v</b> array. */
		CoreExport void	setVerts(int *vrt);
		/*! \remarks Sets the vertices for Tri Patch to those passed.
		\par Parameters:
		<b>int a, int b, int c</b>\n\n
		The vertices to set. <b>v[0]=a; v[1]=b; v[2]=c;</b>These values are
		indices into the <b>v</b> array. */
		void	setVerts(int a, int b, int c)  { assert(type == PATCH_TRI); v[0]=a; v[1]=b; v[2]=c; }
		/*! \remarks Sets the vertices for a Quad Patch to those passed.
		\par Parameters:
		<b>int a, int b, int c, int d</b>\n\n
		The vertices to set. <b>v[0]=a; v[1]=b; v[2]=c; v[3] = d;</b> These
		values are indices into the <b>v</b> array. */
		void	setVerts(int a, int b, int c, int d)  { assert(type == PATCH_QUAD); v[0]=a; v[1]=b; v[2]=c; v[3]=d; }
		/*! \remarks Sets the vectors for a Tri Patch to those passed.
		\par Parameters:
		<b>int ab, int ba, int bc, int cb, int ca, int ac;</b>\n\n
		The vectors to set. <b>vec[0]=ab; vec[1]=ba; vec[2]=bc;</b>\n\n
		<b>vec[3]=cb; vec[4]=ca; vec[5]=ac;</b>These values are indices into
		the <b>vec</b> array. */
		void	setVecs(int ab, int ba, int bc, int cb, int ca, int ac) {
			assert(type == PATCH_TRI);
			vec[0]=ab; vec[1]=ba; vec[2]=bc; vec[3]=cb; vec[4]=ca; vec[5]=ac;
			}
		/*! \remarks Sets the vectors for a Quad patch to those passed.
		\par Parameters:
		<b>int ab, int ba, int bc, int cb,</b>\n\n
		<b>int cd, int dc, int da, int ad</b>;\n\n
		The vectors to set. <b>vec[0]=ab; vec[1]=ba; vec[2]=bc;
		vec[3]=cb;</b>\n\n
		<b>vec[4]=cd; vec[5]=dc; vec[6]=da, vec[7]=ad;</b> These values are
		indices into the <b>vec</b> array. */
		void	setVecs(int ab, int ba, int bc, int cb, int cd, int dc, int da, int ad) {
			assert(type == PATCH_QUAD);
			vec[0]=ab; vec[1]=ba; vec[2]=bc; vec[3]=cb; vec[4]=cd; vec[5]=dc; vec[6]=da, vec[7]=ad;
			}
		/*! \remarks Sets the interior vertex values for a Tri Patch.
		\par Parameters:
		The interior values to set: <b>interior[0]=a; interior[1]=b;
		interior[2]=c;</b> These values are indices into the <b>interior</b>
		array. */
		void	setInteriors(int a, int b, int c) {
			assert(type == PATCH_TRI);
			interior[0]=a; interior[1]=b; interior[2]=c;
			}
		/*! \remarks Sets the interior vertex values for a Tri Patch.
		\par Parameters:
		<b>int a, int b, int c, int d</b>\n\n
		The interior values to set: <b>interior[0]=a; interior[1]=b;
		interior[2]=c; interior[3]=d;</b> These values are indices into the
		<b>interior</b> array. */
		void	setInteriors(int a, int b, int c, int d) {
			assert(type == PATCH_QUAD);
			interior[0]=a; interior[1]=b; interior[2]=c; interior[3]=d;
			}
		/*! \remarks Returns the vertex specified by the index.
		\par Parameters:
		<b>int index</b>\n\n
		The vertex to retrieve. This value may be 0, 1, 2 (or 3 if it's a quad
		patch). */
		int		getVert(int index)	{ return v[index]; }
		/*! \remarks Returns a pointer to the vertex array. */
		int *	getAllVerts(void)	{ return v; }
		MtlID	getMatID() {return (int)((flags>>FACE_MATID_SHIFT)&FACE_MATID_MASK);}
		void    setMatID(MtlID id) {flags &= 0xFFFF; flags |= (DWORD)(id<<FACE_MATID_SHIFT);}
		Point3	getUVW(int index) const;		// UVW of a Triangle's i-th vertex
		Point2	getUV(int index) const;			// UV of a Quadrilateral's i-th vertex
		bool	getVertUVW(int vert, Point3 &uvw) const;	// UVW of a Triangle's vertex
		bool	getVertUV(int vert, Point2 &uv) const;		// UV of a Quadrilateral's vertex
		Point3 BicubicSurface(PatchMesh *pMesh, const float *uu, const float *vv);
		/*! \remarks Triangle patch interpolator. This method returns a point
		on the surface of the patch based on the specified u, v and w values.
		The u, v, w values are barycentric coordinates. u+v+w = 1.0. If u is 1,
		and v and w are 0, the point is at the first vertex. If u is 0, v is 1,
		and w is 0, then the point is at the second vertex. If u and v are 0
		and w is 1 then the point is at the third vertex. Varying positions
		between these values represent different positions on the patch.
		\par Parameters:
		<b>PatchMesh *pMesh</b>\n\n
		Points to the <b>PatchMesh</b> to interpolate.\n\n
		<b>float u, float v, float w</b>\n\n
		The barycentric coordinates.
		\return  A point on the surface of the patch. */
		CoreExport Point3 interp(PatchMesh *pMesh, float u, float v, float w);	// Triangle
		/*! \remarks Quadrilateral patch interpolator. This method returns a
		point on the surface of the patch based on the specified u and v
		values.
		\par Parameters:
		<b>PatchMesh *pMesh</b>\n\n
		Points to the <b>PatchMesh</b> to interpolate.\n\n
		<b>float u</b>\n\n
		The u value in the range 0.0 to 1.0. This defines the distance along
		one axis of the patch.\n\n
		<b>float v</b>\n\n
		The v value in the range 0.0 to 1.0. This defines the distance along
		the other axis of the patch.
		\return  A point on the surface of the patch. */
		CoreExport Point3 interp(PatchMesh *pMesh, float u, float v);			// Quadrilateral
		CoreExport Point3 WUTangent(PatchMesh *pMesh, float u, float v, float w);	// Triangle WU Tangent
		CoreExport Point3 UVTangent(PatchMesh *pMesh, float u, float v, float w);	// Triangle UV Tangent
		CoreExport Point3 VWTangent(PatchMesh *pMesh, float u, float v, float w);	// Triangle VW Tangent
		CoreExport Point3 UTangent(PatchMesh *pMesh, float u, float v);	// Quadrilateral U Tangent
		CoreExport Point3 VTangent(PatchMesh *pMesh, float u, float v);	// Quadrilateral V Tangent
		CoreExport Point3 Normal(PatchMesh *pMesh, float u, float v, float w);	// Triangle Surface Normal
		CoreExport Point3 Normal(PatchMesh *pMesh, float u, float v);			// Quadrilateral Surface Normal
		/*! \remarks This method is used internally. It compute the degree-4
		alias control points. */
		CoreExport void ComputeAux(PatchMesh *pMesh, int index);
		CoreExport void ComputeAux(PatchMesh *pMesh);	// Do all degree-4 points
		/*! \remarks Whenever you are done working on a <b>PatchMesh</b>, this
		method should be called. If the interior vertices of the patch are
		automatic it will update them to correctly match the changes to the
		other vectors. This computes interior vertices considering this patch
		only.
		\par Parameters:
		<b>PatchMesh* pMesh</b>\n\n
		Points to the <b>PatchMesh</b> to compute the interior vertices of. */
		CoreExport void computeInteriors(PatchMesh* pMesh);
		/*! \remarks Sets the type of the patch to either Tri or Quad and
		optionally resets the arrays.
		\par Parameters:
		<b>int type</b>\n\n
		The patch type. One of the following values:\n\n
		<b>PATCH_TRI</b> - Triangular Patch\n\n
		<b>PATCH_QUAD</b> - Quadrilateral Patch\n\n
		<b>BOOL init = FALSE</b>\n\n
		If TRUE the arrays are reset to undefined; otherwise they are left
		unchanged. Normally this is set to FALSE. */
		CoreExport void SetType(int type, BOOL init = FALSE);
		/*! \remarks Assignment operator.
		\par Parameters:
		<b>Patch\& from</b>\n\n
		The patch to copy from. */
		CoreExport Patch& operator=(Patch& from);
		/*! \remarks Sets the flag controlling if interior vertices are
		computed automatically.
		\par Parameters:
		<b>BOOL sw = TRUE</b>\n\n
		TRUE to set; FALSE to clear. */
		CoreExport void SetAuto(BOOL sw = TRUE);
		/*! \remarks Returns TRUE if the <b>PATCH_AUTO</b> flag is set;
		otherwise FALSE. */
		BOOL IsAuto() { return (flags & PATCH_AUTO) ? TRUE : FALSE; }

//watje 12-10-98
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the hidden state of the patch.
		\par Parameters:
		<b>BOOL sw = TRUE</b>\n\n
		TRUE to hide; FALSE to unhide. */
		CoreExport void SetHidden(BOOL sw = TRUE);
//watje 12-10-98
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns TRUE if the patch is hidden; otherwise FALSE. */
		BOOL IsHidden() { return (flags & PATCH_HIDDEN) ? TRUE : FALSE; }
		// Tell the caller which edge uses the two supplied vert indexes (-1 if error)
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method provides an easy way to find out which edge of a patch uses
		two vertices. Simply supply the two vertices of the desired edge and
		call this method.
		\par Parameters:
		<b>int v1, int v2</b>\n\n
		The index of the two vertices.
		\return  The index of the edge within the patch (0 - 2 for triangular
		patches, 0 - 3 for quad patches).\n\n
		CAUTION: Be sure to check the return value before using it for an index
		- If the two vertices supplied are not used as an edge on this patch,
		-1 is returned. */
		int WhichEdge(int v1, int v2);
		// Tell the caller which vertex uses the supplied vert index (-1 if error)
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method provides an easy way to find out which corner of a patch
		uses a given vertex. Simply supply the vertex index and call this
		method.
		\par Parameters:
		<b>int v</b>\n\n
		The index of the vertex.
		\return  The corner index of the given vertex within the patch (0 - 2
		for triangular patches, 0 - 3 for quad patches).\n\n
		CAUTION: Be sure to check the return value before using it for an index
		- If the vertex supplied is not used as a corner on this patch, -1 is
		returned. */
		int WhichVert(int v);

		// Dump the patch mesh structure via DebugPrints

		/*! \remarks This may be called to dump the patch mesh structure via
		<b>DebugPrint()</b>. See
		<a href="ms-its:3dsmaxsdk.chm::/debug_debugging.html">Debugging</a>.
		\par Operators:
		*/
		CoreExport void Dump();

		CoreExport IOResult Save(ISave* isave);
		CoreExport IOResult Load(ILoad* iload);
	};

// Separate class for patch texture verts
/*! \sa  Class Patch, Class PatchMesh, <a href="ms-its:3dsmaxsdk.chm::/patches_root.html">Working with Patches</a>.\n\n
\par Description:
This is a texture vertex patch structure. This is similar to the <b>TVFace</b>
class used with a <b>Mesh</b>. All methods of this class are implemented by the
system.
\par Data Members:
<b>int tv[4];</b>\n\n
Texture vertices. There are always four here, even for Tri Patches. These are
indices in the PatchMesh's <b>tVerts</b> array.\n\n
<b>int handles[8];</b>\n\n
The UVW vertices for the handles.\n\n
<b>int interiors[4];</b>\n\n
The UVW interior handles.  */
class TVPatch: public MaxHeapOperators {	
	public:
		int	tv[4];			// Texture verts (always 4 here, even for tri patches)
//watje new patch mapping
		int handles[8];
		int interiors[4];

		/*! \remarks Constructor. */
		CoreExport TVPatch();
		/*! \remarks Copy Constructor. The <b>this</b> pointer is set to the
		<b>fromPatch</b>. */
		CoreExport TVPatch(const TVPatch& fromPatch);
		/*! \remarks Performs initialization by setting the texture vertices
		to 0. */
		CoreExport void Init();
		/*! \remarks Sets the specified number of texture vertices.
		\par Parameters:
		<b>int *vrt</b>\n\n
		The array of verts to set.\n\n
		<b>int count</b>\n\n
		The number to set. */
		CoreExport void setTVerts(int *vrt, int count);
		/*! \remarks Sets the texture vertices for a Quad Patch.
		\par Parameters:
		<b>int a, int b, int c, int d = 0</b>\n\n
		The vertices to set: <b>tv[0]=a; tv[1]=b; tv[2]=c; tv[3]=d;</b> */
		CoreExport void setTVerts(int a, int b, int c, int d = 0);

//watje new patch mapping
//sets the indices of the patch handles
		/*! \remarks Sets the specified number of texture (UVW) handles.
		\par Parameters:
		<b>int *vrt</b>\n\n
		The array of handles to set.\n\n
		<b>int count</b>\n\n
		The number to set. */
		CoreExport void setTHandles(int *vrt, int count);
		/*! \remarks Sets the texture (UVW) handles for a Quad Patch.
		\par Parameters:
		<b>int a, int b, int c, int d, int e, int f, int g = 0, int h =
		0</b>\n\n
		The handles to set: <b>handles[0]=a; handles[1]=b; handles[2]=c;
		handles[3]=d; handles[4]=e; handles[5]=f; handles[6]=g;
		handles[7]=h;</b> */
		CoreExport void setTHandles(int a, int b, int c, int d ,
								   int e, int f, int g = 0 , int h = 0);
//sets the indices of the patch interior handles
		/*! \remarks Sets the specified number of interior texture (UVW)
		handles.
		\par Parameters:
		<b>int *vrt</b>\n\n
		The array of interior handles to set.\n\n
		<b>int count</b>\n\n
		The number to set. */
		CoreExport void setTInteriors(int *vrt, int count);
		/*! \remarks Sets the interior texture (UVW) handles for a Quad Patch.
		\par Parameters:
		<b>int a, int b, int c, int d = 0</b>\n\n
		The interior handles to set: <b>interiors[0]=a; interiors[1]=b;
		interiors[2]=c; interiors[3]=d;</b> */
		CoreExport void setTInteriors(int a, int b, int c, int d = 0);

		/*! \remarks Returns the texture vertex specified by the index.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the texture vertex to return. */
		int		getTVert(int index)	{ return tv[index]; }
		/*! \remarks Returns a pointer to the array of texture vertices. */
		int *	getAllTVerts(void)	{ return tv; }
		/*! \remarks Assignment operator.
		\par Parameters:
		<b>TVPatch\& from</b>\n\n
		The texture vertex patch to copy from. */
		CoreExport TVPatch& operator=(const TVPatch& from);

		/*! \remarks This method is used internally in saving to the MAX file.
		*/
		CoreExport IOResult Save(ISave* isave);
		/*! \remarks This method is used internally in loading from the MAX
		file.
		\par Operators:
		*/
		CoreExport IOResult Load(ILoad* iload);
	};

CoreExport void setPatchIntrFunc(INTRFUNC fn);

// Special types for patch vertex hits -- Allows us to distinguish what they hit on a pick
#define PATCH_HIT_PATCH		0
#define PATCH_HIT_EDGE		1
#define PATCH_HIT_VERTEX	2
#define PATCH_HIT_VECTOR	3
#define PATCH_HIT_INTERIOR	4

/*! \sa  Class PatchMesh.\n\n
\par Description:
This class represents a single hit record for sub-patch level hit testing. All
methods of this class are implemented by the system.
\par Data Members:
<b>DWORD dist;</b>\n\n
The distance of the hit. If the user is in wireframe mode, this is the distance
in pixels to the item that was hit. If the user is in shaded mode, this is the
Z depth distance. Smaller numbers indicate a closer hit.\n\n
<b>PatchMesh *patch;</b>\n\n
The PatchMesh associated with this sub-patch hit.\n\n
<b>int index;</b>\n\n
The index of the sub-object component. For example, if vertices were being hit
tested, this would be the index into the vertex table.\n\n
<b>int type;</b>\n\n
The type of the hit. One of the following values:\n\n
<b>PATCH_HIT_PATCH</b>\n\n
<b>PATCH_HIT_EDGE</b>\n\n
<b>PATCH_HIT_VERTEX</b>\n\n
<b>PATCH_HIT_VECTOR</b>\n\n
<b>PATCH_HIT_INTERIOR</b>  */
class PatchSubHitRec: public MaxHeapOperators {
	private:		
		PatchSubHitRec *next;
	public:
		DWORD	dist;
		PatchMesh *patch;
		int		index;
		int		type;

		/*! \remarks Constructor. The data members are set to the values
		passed. */
		PatchSubHitRec( DWORD dist, PatchMesh *patch, int index, int type, PatchSubHitRec *next ) 
			{ this->dist = dist; this->patch = patch; this->index = index; this->type = type; this->next = next; }

		/*! \remarks Returns the next sub hit record. */
		PatchSubHitRec *Next() { return next; }		
	};

/*! \sa  Class PatchMesh, Class PatchSubHitRec.\n\n
\par Description:
This class describes a list of sub-patch hit records. Methods are available to
return the first PatchSubHitRec in the list, and to add hits to the list. All
methods of this class are implemented by the system.  */
class SubPatchHitList: public MaxHeapOperators {
	private:
		PatchSubHitRec *first;
	public:
		/*! \remarks Constructor. The first sub hit is set to NULL. */
		SubPatchHitList() { first = NULL; }
		/*! \remarks Destructor. The list of patch hits are deleted. */
		CoreExport ~SubPatchHitList();

		/*! \remarks Returns the first sub hit record. */
		PatchSubHitRec *First() { return first; }
		/*! \remarks Creates a new sub hit record and adds it to the list.
		\par Parameters:
		<b>DWORD dist</b>\n\n
		The distance of the hit. If the user is in wireframe mode, this is the
		distance in pixels to the item that was hit. If the user is in shaded
		mode, this is the Z depth distance. Smaller numbers indicate a closer
		hit.\n\n
		<b>PatchMesh *patch</b>\n\n
		The PatchMesh associated with this sub-patch hit.\n\n
		<b>int index</b>\n\n
		The index of the sub-object component. For example, if vertices were
		being hit tested, this would be the index into the vertex table.\n\n
		<b>int type</b>\n\n
		The type of the hit. One of the following values:\n\n
		<b>PATCH_HIT_PATCH</b>\n\n
		<b>PATCH_HIT_EDGE</b>\n\n
		<b>PATCH_HIT_VERTEX</b>\n\n
		<b>PATCH_HIT_VECTOR</b>\n\n
		<b>PATCH_HIT_INTERIOR</b> */
		CoreExport void AddHit( DWORD dist, PatchMesh *patch, int index, int type );
	};


// Special storage class for hit records so we can know which object was hit
/*! \sa  Class HitData, Class PatchMesh.\n\n
\par Description:
This is a storage class for hit records so the system can know which patch was
hit. All methods of this class are implemented by the system.
\par Data Members:
<b>PatchMesh *patch;</b>\n\n
The PatchMesh associated with this hit record.\n\n
<b>int index;</b>\n\n
The index of the sub-object component. For example, if vertices were being hit
tested, this would be the index into the vertex table.\n\n
<b>int type;</b>\n\n
Types for patch vertex hits.\n\n
<b>PATCH_HIT_PATCH</b>\n\n
<b>PATCH_HIT_EDGE</b>\n\n
<b>PATCH_HIT_VERTEX</b>\n\n
<b>PATCH_HIT_VECTOR</b>\n\n
<b>PATCH_HIT_INTERIOR</b>  */
class PatchHitData : public HitData {
	public:
		PatchMesh *patch;
		int index;
		int type;
		/*! \remarks Constructor. The data members are initialized to the
		values passed. */
		PatchHitData(PatchMesh *patch, int index, int type)
			{ this->patch = patch; this->index = index; this->type = type; }
		~PatchHitData() {}
	};

// Flags for sub object hit test

// NOTE: these are the same bits used for object level.
#define SUBHIT_PATCH_SELONLY	(1<<0)
#define SUBHIT_PATCH_UNSELONLY	(1<<2)
#define SUBHIT_PATCH_ABORTONHIT	(1<<3)
#define SUBHIT_PATCH_SELSOLID	(1<<4)

#define SUBHIT_PATCH_VERTS		(1<<24)
#define SUBHIT_PATCH_VECS		(1<<25)
#define SUBHIT_PATCH_PATCHES	(1<<26)
#define SUBHIT_PATCH_EDGES		(1<<27)
#define SUBHIT_PATCH_TYPEMASK	(SUBHIT_PATCH_VERTS|SUBHIT_PATCH_VECS|SUBHIT_PATCH_EDGES|SUBHIT_PATCH_PATCHES)
#define SUBHIT_PATCH_IGNORE_BACKFACING (1<<28)



// Selection level bits.
#define PATCH_OBJECT		(1<<0)
#define PATCH_VERTEX		(1<<1)
#define PATCH_PATCH			(1<<2)
#define PATCH_EDGE			(1<<3)
#define PATCH_HANDLE		(1<<4)

// Types for Subdivision, below:
#define SUBDIV_EDGES 0
#define SUBDIV_PATCHES 1

// Relax defaults
#define DEF_PM_RELAX FALSE
#define DEF_PM_RELAX_VIEWPORTS	TRUE
#define DEF_PM_RELAX_VALUE	0.0f
#define DEF_PM_ITER	1
#define DEF_PM_BOUNDARY	TRUE
#define DEF_PM_SADDLE	FALSE

// PatchMesh flags
#define PM_HITTEST_REQUIRE_ALL (1<<0)	// Force faces to be hit only if all triangles are hit.  (Internal use.)

namespace MaxGraphics {
	class IPatchMeshInternal;
}

/*! \sa  Class Patch, Class PatchVec, Class PatchEdge, Class PatchVert, Class BitArray, Class TessApprox, Class PatchTVert, <a href="ms-its:3dsmaxsdk.chm::/patches_root.html">Working with
Patches</a>.\n\n
\par Description:
A patch mesh can be made up of any number of patches. Each of these patches can
be three or four sided. Each edge of a patch can only be used by either one
patch (which makes it an open edge) or two patches (which makes it a
transitional edge between the two).\n\n
All methods of this class are implemented by the system.
\par Method Groups:
See <a href="class_patch_mesh_groups.html">Method Groups for Class PatchMesh</a>.
\par Topology:
<b>int numVerts;</b>\n\n
The number of vertices.\n\n
<b>int numVecs;</b>\n\n
The number of vectors.\n\n
<b>int numPatches;</b>\n\n
The number of patches.\n\n
<b>int numEdges;</b>\n\n
The number of edges.\n\n
<b>Patch *patches;</b>\n\n
The list of patches.\n\n
<b>PatchVec *vecs;</b>\n\n
The list of PatchVecs.\n\n
<b>PatchEdge *edges;</b>\n\n
The list of PatchEdges.\n\n
<b>Tab\<HookPoint\> hooks;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This table is used internally
\par Geometry:
<b>PatchVert *verts;</b>\n\n
The list of PatchVerts.
\par Texture Coordinate Assignment:
<b>Tab\<int\> numTVerts;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
A table containing the number of texture vertices for each channel.\n\n
<b>Tab\<PatchTVert *\> tVerts;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
A table containing pointers to the texture vertices for each channel.\n\n
Previous to R4.0 this was a Tab\<UVVert *\>.\n\n
<b>Tab\<TVPatch *\> tvPatches;  </b>\n\n
This data member is available in release 3.0 and later only.\n\n
A table containing pointers to the texture vertex patches for each channel.
\par Material Assignment:
<b>DWORD mtlIndex;</b>\n\n
The object level material.
\par Selection:
<b>BitArray vertSel;</b>\n\n
The selected vertices.\n\n
<b>BitArray edgeSel;</b>\n\n
The selected edges.\n\n
<b>BitArray patchSel;</b>\n\n
The selected patches.\n\n
<b>int bezVecVert;</b>\n\n
This is used internally.\n\n
<b>DWORD dispFlags;</b>\n\n
The display attribute flags. See \ref patchDisplayFlags.\n\n
<b>DWORD selLevel;</b>\n\n
The current selection level. One of the following values:\n\n
<b>PATCH_OBJECT</b> - Object level.\n\n
<b>PATCH_VERTEX</b> - Vertex level.\n\n
<b>PATCH_PATCH</b> - Patch level.\n\n
<b>PATCH_EDGE</b> - Edge level.\n\n
<b>int cacheSteps;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
The meshSteps used for the cache.\n\n
<b>BOOL cacheAdaptive;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
The adaptive switch used for the mesh cache.\n\n
<b>Tab\<Point3\> extrudeDeltas;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This data member is for internal use only.\n\n
<b>Tab\<ExtrudeData\> extrudeData;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This data member is for internal use only.\n\n
<b>BitArray bevelEdges;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This data member is for internal use only.\n\n
<b>Tab\<float\> edgeDistances;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This data member is for internal use only.  */
class PatchMesh : public BaseInterfaceServer {
	friend class Patch;
	friend class MaxGraphics::IPatchMeshInternal;

	private:
#if MULTI_PROCESSING
		static int		refCount;
		static HANDLE	xfmThread;
		static HANDLE	xfmMutex;
		static HANDLE	xfmStartEvent;
		static HANDLE	xfmEndEvent;
		friend DWORD WINAPI xfmFunc(LPVOID ptr);
#endif
		// derived data-- can be regenerated
		PRVertex 		*rVerts;		// <<< instance specific.
		PRVertex 		*rVecs;			// <<< instance specific.
		GraphicsWindow 	*cacheGW;  		// identifies rVerts cache
		Box3			bdgBox;			// object space--depends on geom+topo
 
		// The number of interpolations this patch will use for mesh conversion
		int			meshSteps;
//3-18-99 watje to support render steps
		int			meshStepsRender;
		BOOL		showInterior;
		BOOL		usePatchNormals;	// CAL-05/15/03: use true patch normals. (FID #1760)

		BOOL		adaptive;
		// GAP tessellation
		TessApprox	viewTess;	// tessellation control for the interactive renderer
		TessApprox	prodTess;	// tessellation control for the production renderer
		TessApprox	dispTess;	// displacment tessellation control for the production renderer
		BOOL		mViewTessNormals;	// use normals from the tesselator
		BOOL		mProdTessNormals;	// use normals from the tesselator
		BOOL		mViewTessWeld;	// Weld the mesh after tessellation
		BOOL		mProdTessWeld;	// Weld the mesh after tessellation

 		// Vertex and patch work arrays -- for snap code
		int			snapVCt;
		int			snapPCt;
		char		*snapV;
		char		*snapP;

		// -------------------------------------
		//
		DWORD  		flags;		  	// work flags- 

		// Hidden Map Channels
		// Texture Coord assignment 
		Tab<int> numHTVerts;
		Tab<PatchTVert *> htVerts;
		Tab<TVPatch *> htvPatches;  	 

		// relax options
		BOOL relax;
		BOOL relaxViewports;
		float relaxValue;
		int relaxIter;
		BOOL relaxBoundary;
		BOOL relaxSaddle;
		
		void SetFlag(DWORD fl, bool val=TRUE) { if (val) flags |= fl; else flags &= ~fl; }
		void ClearFlag(DWORD fl) { flags &= ~fl; }
		bool GetFlag(DWORD fl) const { return (flags & fl) ? true : false; }

		int 		renderPatch( GraphicsWindow *gw, int index);
		int 		renderEdge( GraphicsWindow *gw, int index, HitRegion *hr);
		void		checkRVertsAlloc(void);
		void		setCacheGW(GraphicsWindow *gw)	{ cacheGW = gw; }
		GraphicsWindow *getCacheGW(void)			{ return cacheGW; }

		void 		freeVerts();
		void 		freeTVerts(int channel=0);
		void 		freeVecs();
		void  		freePatches();
		void  		freeTVPatches(int channel=0);
		void  		freeEdges();
		void  		freeRVerts();
		void		freeSnapData();
		int			buildSnapData(GraphicsWindow *gw,int verts,int edges);

		// Mesh caches
		Mesh		unrelaxedMesh;	// Unrelaxed
		Mesh		relaxedMesh;	// Relaxed

		// CAL-03/06/03: Store the mapping of the faces on the cached mesh to the patches. (FID #832)
		Tab<int>	mappingFaceToPatch;

	public:
		// Topology
		int			numVerts;
		int			numVecs;
		int	 		numPatches;
		int			numEdges;
		Patch *		patches;
		PatchVec *	vecs;
		PatchEdge *	edges;
		Tab<HookPoint> hooks;

//watje 4-16-99 to handle hooks and changes in topology
		Tab<Point3> hookTopoMarkers;
		Tab<Point3> hookTopoMarkersA;
		Tab<Point3> hookTopoMarkersB;
		CoreExport int HookFixTopology() ;

		// Normals
		Point3 *	normals;
		BOOL		normalsBuilt;

		// Geometry
		PatchVert *	verts;

		// Texture Coord assignment 
		Tab<int> numTVerts;
		Tab<PatchTVert *> tVerts;
		Tab<TVPatch *> tvPatches;  	 

		// Material assignment
		MtlID		mtlIndex;     // object material

		// Selection
		BitArray	vecSel;  		// selected vectors // CAL-06/10/03: (FID #1914)
		BitArray	vertSel;  		// selected vertices
		BitArray	edgeSel;  		// selected edges
		BitArray	patchSel;  		// selected patches

		// If hit bezier vector, this is its info:
		int bezVecVert;

		// Display attribute flags
		DWORD		dispFlags;

		// Selection level
		DWORD		selLevel;

		// Mesh cache flags
		int cacheSteps;		// meshSteps used for the cache
		BOOL cacheAdaptive;	// adaptive switch used for cache
		BOOL unrelaxedMeshValid;
		BOOL relaxedMeshValid;

		/*! \remarks Constructor. Initializes data members (see <b>Init()</b>
		below). */
		CoreExport PatchMesh();
		/*! \remarks Constructor. This <b>PatchMesh</b> is initialized from
		the specified <b>PatchMesh</b>. */
		CoreExport PatchMesh(PatchMesh& fromPatch);

		/*! \remarks Initializes the data members to default values:\n\n
		<b>meshSteps  = 5;</b>\n\n
		<b>adaptive  = FALSE;</b>\n\n
		<b>rVerts   = NULL;</b>\n\n
		<b>cacheGW   = NULL;</b>\n\n
		<b>numVerts   = 0;</b>\n\n
		<b>numVecs   = 0;</b>\n\n
		<b>numPatches   = 0;</b>\n\n
		<b>numEdges  = 0;</b>\n\n
		<b>patches   = NULL;</b>\n\n
		<b>edges   = NULL;</b>\n\n
		<b>numTVerts.ZeroCount();</b>\n\n
		<b>tvPatches.ZeroCount();</b>\n\n
		<b>tVerts.ZeroCount();</b>\n\n
		<b>vecs   = NULL;</b>\n\n
		<b>verts    = NULL;</b>\n\n
		<b>mtlIndex   = 0;</b>\n\n
		<b>flags    = 0;</b>\n\n
		<b>snapVCt   = 0;</b>\n\n
		<b>snapPCt   = 0; </b>\n\n
		<b>snapV   = NULL;</b>\n\n
		<b>snapP   = NULL;</b>\n\n
		<b>dispFlags = DISP_LATTICE;</b>\n\n
		<b>selLevel = PATCH_OBJECT;</b>\n\n
		<b>bezVecVert  = -1;</b>\n\n
		<b>bdgBox.Init();</b>\n\n
		<b>cacheSteps = -9999;</b>\n\n
		<b>cacheAdaptive = -9999;</b>\n\n
		<b>prodTess.u = 5;    </b>\n\n
		<b>prodTess.v = 5;    </b>\n\n
		<b>prodTess.dist = 2.0f;  </b>\n\n
		<b>prodTess.ang = 10.0f;  </b>\n\n
		<b>prodTess.edge = 1.0f;  </b>\n\n
		<b>prodTess.view = TRUE;  </b>\n\n
		<b>dispTess.type = TESS_CURVE; </b>\n\n
		<b>dispTess.v = 5;    </b>\n\n
		<b>dispTess.dist = 2.0f;  </b>\n\n
		<b>dispTess.ang = 10.0f;  </b>\n\n
		<b>dispTess.edge = 1.0f;  </b>\n\n
		<b>dispTess.view = TRUE;  </b>\n\n
		<b>viewTess.u = 5;  </b>\n\n
		<b>viewTess.v = 5;  </b>\n\n
		<b>viewTess.view = FALSE; </b>\n\n
		<b>viewTess.ang = 20.0f; </b>\n\n
		<b>viewTess.dist = 10.0f; </b>\n\n
		<b>viewTess.edge = 10.0f; </b> */
		CoreExport void Init();

		/*! \remarks Destructor. Frees up allocated arrays. */
		CoreExport ~PatchMesh();

		/*! \remarks Assignment operator from another <b>PatchMesh</b>. */
		CoreExport PatchMesh& 		operator=(PatchMesh& fromPatchMesh);
		/*! \remarks Assignment operator. This operator will do the conversion
		from a <b>Mesh</b> to a <b>PatchMesh</b>. Note that this can get very
		slow if there are a lot of faces in the mesh. When the system does the
		conversion is must do a fair amount of work with its interior
		connection lists. It builds a data base of what is connected to what
		and makes sure that the <b>PatchMesh</b> doesn't have any places where
		an edge is used by more than two patches. Also, for every face in the
		mesh it generates a triangular patch. Therefore use this method with
		some caution as it can create some very complex <b>PatchMesh</b>
		objects. */
		CoreExport PatchMesh& 		operator=(Mesh& fromMesh);

		// The following is similar to operator=, but just takes the major components,
		// not the display flags, selection level, etc.
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		The following is similar to operator=, but just takes the major
		components, not the display flags, selection level, etc.
		\par Parameters:
		<b>PatchMesh \&fromPatchMesh</b>\n\n
		The patch mesh to copy from. */
		CoreExport void CopyPatchDataFrom(PatchMesh &fromPatchMesh);

		/*! \remarks Sets the number of vertices in the patch mesh.
		\par Parameters:
		<b>int ct</b>\n\n
		The new number of vertices.\n\n
		<b>BOOL keep = FALSE</b>\n\n
		If TRUE any old vertices are copied to the new array; otherwise they
		are freed.
		\return  TRUE if the number of vertices was allocated; otherwise FALSE.
		*/
		CoreExport BOOL	setNumVerts(int ct, BOOL keep = FALSE);
		/*! \remarks Returns the number of vertices in the patch mesh. */
		int				getNumVerts(void)	{ return numVerts; }
		
		/*! \remarks Sets the number of vectors.
		\par Parameters:
		<b>int ct</b>\n\n
		The new vector count.\n\n
		<b>BOOL keep = FALSE</b>\n\n
		If TRUE any previous vectors are copied; otherwise they are freed.
		\return  TRUE if the number was allocated and set; otherwise FALSE. */
		CoreExport BOOL	setNumVecs(int ct, BOOL keep = FALSE);
		/*! \remarks Returns the number of vectors. */
		int				getNumVecs(void)	{ return numVecs; }
		
		/*! \remarks Sets the number of patches.
		\par Parameters:
		<b>int ct</b>\n\n
		The new patch count.\n\n
		<b>BOOL keep = FALSE</b>\n\n
		If TRUE any previous patches are copied; otherwise they are freed.
		\return  TRUE if the number was allocated and set; otherwise FALSE. */
		CoreExport BOOL	setNumPatches(int ct, BOOL keep = FALSE);
		/*! \remarks Returns the number of patches. */
		int				getNumPatches(void)		{ return numPatches; }

		/*! \remarks Sets the number of edges.
		\par Parameters:
		<b>int ct</b>\n\n
		The new edge count.\n\n
		<b>BOOL keep = FALSE</b>\n\n
		If TRUE any previous edges are copied; otherwise they are freed.
		\return  TRUE if the number was allocated and set; otherwise FALSE. */
		CoreExport BOOL	setNumEdges(int ct, BOOL keep = FALSE);
		/*! \remarks Returns the number of edges. */
		int				getNumEdges(void)		{ return numEdges; }
		
		/*! \remarks Sets the 'i-th' vertex.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the vertex to set.\n\n
		<b>const Point3 \&xyz</b>\n\n
		The vertex location. */
		void		setVert(int i, const Point3 &xyz)	{ verts[i].p = xyz; }
		/*! \remarks Sets the 'i-th' vertex.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the vertex to set.\n\n
		<b>float x, float y, float z</b>\n\n
		The vertex location. */
		void		setVert(int i, float x, float y, float z)	{ verts[i].p.x=x; verts[i].p.y=y; verts[i].p.z=z; }
		/*! \remarks Sets the 'i-th' vector.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the vector to set.\n\n
		<b>const Point3 \&xyz</b>\n\n
		The vector to set. */
		void		setVec(int i, const Point3 &xyz)	{ vecs[i].p = xyz; }
		/*! \remarks Sets the 'i-th' vector.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the vector to set.\n\n
		<b>float x, float y, float z</b>\n\n
		The vector values to set. */
		void		setVec(int i, float x, float y, float z)	{ vecs[i].p.x=x; vecs[i].p.y=y; vecs[i].p.z=z; }
		
		/*! \remarks Returns the 'i-th' vertex.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the vertex to retrieve. */
		PatchVert &	getVert(int i)					{ return verts[i];  }
		/*! \remarks Returns the address of the 'i-th' vertex.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the vertex. */
		PatchVert *	getVertPtr(int i)				{ return verts+i; }
		/*! \remarks Returns the 'i-th' vector.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the vector to retrieve. */
		PatchVec &	getVec(int i)					{ return vecs[i];  }
		/*! \remarks Returns the address of the 'i-th' vector.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the vector. */
		PatchVec *	getVecPtr(int i)				{ return vecs+i; }
		/*! \remarks This method is not currently used. */
		PRVertex &	getRVert(int i)					{ return rVerts[i]; }
		/*! \remarks This method is not currently used. */
		PRVertex *	getRVertPtr(int i)				{ return rVerts+i; }
		PRVertex &	getRVec(int i)					{ return rVecs[i]; }
		PRVertex *	getRVecPtr(int i)				{ return rVecs+i; }
		
		// Two versions of following methods, to cope with necessary change in map indexing between 2.5 and 3.
		// Old TV/VC methods are given with "TV" in the name.  For these methods, channel 0 is the original map
		// channel, 1, while any nonzero channel is vertex colors.  (No higher channels!)
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Sets the number of mapping verticies in the original TV map or vertex
		color channel.
		\par Parameters:
		<b>int mp</b>\n\n
		Specifies the channel. If 0, the number of vertices in the original
		mapping channel (map channel 1) is set. If nonzero, the number of
		vertices in the vertex color channel (map channel 0) is set.\n\n
		<b>int ct</b>\n\n
		The number of map vertices desired.\n\n
		<b>BOOL keep=FALSE</b>\n\n
		If TRUE, any existing mapping verts are copied over into the new array.
		\return  TRUE on success; otherwise FALSE. */
		BOOL	setNumTVertsChannel(int mp, int ct, BOOL keep=FALSE) { return setNumMapVerts (mp?0:1, ct, keep); }
		/*! \remarks Sets the number of mapping verts in the original mapping
		channel (channel 1).
		\par Parameters:
		<b>int ct</b>\n\n
		The number of map vertices desired.\n\n
		<b>BOOL keep = FALSE</b>\n\n
		If TRUE, any existing mapping verts are copied over into the new array.
		\return  TRUE if successful, FALSE if unsuccessful. */
		BOOL setNumTVerts(int ct, BOOL keep=FALSE) { return setNumMapVerts (1, ct, keep); }
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Returns the number of mapping verts in the original map or vertex
		colors.
		\par Parameters:
		<b>int mp</b>\n\n
		If 0, the number of vertices in the original mapping channel (map
		channel 1) is returned. If nonzero, the number of vertices in the
		vertex color channel (map channel 0) is returned. */
		int	getNumTVertsChannel(int mp) const { return numTVerts[mp?0:1]; }
		/*! \remarks Returns the number of mapping vertices in the original
		mapping channel (channel 1). */
		int getNumTVerts() const { return getNumMapVerts(1); }

		// New methods have "Map" in the name, and accept normal Object-level map indexing: 0 is VC channel, 1 or more
		// are map channels.
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the number of mapping verts in the specified mapping channel.
		\par Parameters:
		<b>int mp</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int ct</b>\n\n
		The number of mapping verts desired.\n\n
		<b>BOOL keep=FALSE</b>\n\n
		If TRUE any old vertices are copied to the new array; otherwise they
		are freed.
		\return  TRUE if the number of vertices was allocated; otherwise FALSE.
		*/
		CoreExport BOOL setNumMapVerts (int mp, int ct, BOOL keep = FALSE);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the number of mapping verts in the specified mapping channel.
		\par Parameters:
		<b>int mp</b>\n\n
		In this method, 0 is the vertex color channel, and channels 1 through
		MAX_MESHMAPS-1 are the map channels. */
		CoreExport int getNumMapVerts (int mp) const;
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		Returns a pointer to the list of PatchTVerts for the specified channel
		of this patch.
		\par Parameters:
		<b>int mp</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels. */
		CoreExport PatchTVert *mapVerts (int mp) const;
		CoreExport TVPatch *mapPatches (int mp) const;

		// These are parallel to patches
		// These are called from setNumPatches() to maintain the same count.
		//
		// If they are NULL and keep = TRUE they stay NULL.
		// If they are NULL and keep = FALSE they are allocated (3D verts also init themselves from the main vert array)
		// If they are non-NULL and ct = 0 they are set to NULL (and freed)
		// Old version: nonzero = vc channel
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Sets the number of map patches in the original TV or vertex color
		channels. (Note that <b>setNumMapPatches()</b> is called from
		<b>setNumPatches()</b>, so this doesn't need to be called separately
		once a map channel is active.)\n\n
		Note:\n\n
		If the map patches are NULL and keep = TRUE they stay NULL.\n\n
		If the map patches are NULL and keep = FALSE they are allocated, and
		map verts also init themselves from the main vert array.\n\n
		If the map patches are non-NULL and ct = 0 they are set to NULL (and
		freed)
		\par Parameters:
		<b>int channel</b>\n\n
		If 0, the number of map patches in the original map channel are set. If
		nonzero, the number of map patches in the vertex color channel is
		set.\n\n
		<b>int ct</b>\n\n
		The number of map patches desired -- should match the number of
		patches.\n\n
		<b>BOOL keep=FALSE</b>\n\n
		The keep flag. See above.\n\n
		<b>int oldCt=0</b>\n\n
		The old number of patches. This is important for determining how much
		to copy over when keep is TRUE.
		\return  TRUE if storage has been allocated and the number is set;
		otherwise FALSE. */
		BOOL setNumTVPatchesChannel(int channel, int ct, BOOL keep=FALSE, int oldCt=0) { return setNumMapPatches (channel?0:1, ct, keep, oldCt); }
		/*! \remarks Sets the number of map patches in the original TV
		channel. (Note that <b>setNumMapPatches()</b> is called from
		<b>setNumPatches()</b>, so this doesn't need to be called separately
		once a map channel is active.)\n\n
		Note:\n\n
		If the map patches are NULL and keep = TRUE they stay NULL.\n\n
		If the map patches are NULL and keep = FALSE they are allocated, and
		map verts also init themselves from the main vert array.\n\n
		If the map patches are non-NULL and ct = 0 they are set to NULL (and
		freed)
		\par Parameters:
		<b>int ct</b>\n\n
		The number of map patches desired -- should match the number of
		patches.\n\n
		<b>BOOL keep=FALSE</b>\n\n
		If TRUE, existing map patches are copied into the new map patch array.
		oldCt should specify how many patches were around previously.\n\n
		<b>int oldCt=0</b>\n\n
		The old number of patches. This is important for determining how much
		to copy over when keep is TRUE.
		\return  TRUE if storage has been allocated and the number is set;
		otherwise FALSE. */
		BOOL setNumTVPatches(int ct, BOOL keep=FALSE, int oldCt=0) { return setNumMapPatches (1, ct, keep, oldCt); }
		// New version: 0 = vc channel
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the number of map patches in the specified map channel. (Note that
		this is called from <b>setNumPatches()</b>, so it doesn't need to be
		called separately once a map channel is active.)\n\n
		Note:\n\n
		If the map patches are NULL and keep = TRUE they stay NULL.\n\n
		If the map patches are NULL and keep = FALSE they are allocated, and
		map verts also init themselves from the main vert array.\n\n
		If the map patches are non-NULL and ct = 0 they are set to NULL (and
		freed).
		\par Parameters:
		<b>int channel</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int ct</b>\n\n
		The number of map patches desired -- should match the number of
		patches.\n\n
		<b>BOOL keep=FALSE</b>\n\n
		If TRUE, existing map patches are copied into the new map patch array.
		oldCt should specify how many patches were around previously.\n\n
		<b>int oldCt=0</b>\n\n
		The old number of patches. This is important for determining how much
		to copy over when keep is TRUE.
		\return  TRUE if storage has been allocated and the number is set;
		otherwise FALSE. */
		CoreExport BOOL 	setNumMapPatches (int channel, int ct, BOOL keep=FALSE, int oldCt=0);

		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Sets the 'i-th' map vertex of the specified channel.
		\par Parameters:
		<b>int channel</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int i</b>\n\n
		The index of the map vertex to set.\n\n
		<b>const UVVert \&xyz</b>\n\n
		The value to set. */
		void		setTVertChannel(int channel, int i, const UVVert &xyz)	{ tVerts[channel?0:1][i] = xyz; }
		/*! \remarks Sets the 'i-th' map vertex.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the map vertex to set.\n\n
		<b>const UVVert \&xyz</b>\n\n
		The value to set. */
		void		setTVert(int i, const UVVert &xyz)	{ tVerts[1][i] = xyz; }
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Sets the 'i-th' map vertex of the specified channel.
		\par Parameters:
		<b>int channel</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int i</b>\n\n
		The index of the map vertex to set.\n\n
		<b>float x, float y, float z</b>\n\n
		The values to set. */
		void		setTVertChannel(int channel, int i, float x, float y, float z)	{ tVerts[channel?0:1][i].p.x=x; tVerts[channel?0:1][i].p.y=y; tVerts[channel?0:1][i].p.z=z; }
		/*! \remarks Sets the 'i-th' map vertex.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the map vertex to set.\n\n
		<b>float x, float y, float z</b>\n\n
		The values to set. */
		void		setTVert(int i, float x, float y, float z)	{ tVerts[1][i].p.x=x; tVerts[1][i].p.y=y; tVerts[1][i].p.z=z; }
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the 'i-th' map patch.
		\par Parameters:
		<b>int channel</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int i</b>\n\n
		The index of the map vertex to set.\n\n
		<b>TVPatch \&tvp</b>\n\n
		The map patch to set. */
		void		setTVPatchChannel(int channel, int i, TVPatch &tvp)	{ tvPatches[channel?0:1][i] = tvp; }
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the specified map patch.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the texture patch to set.\n\n
		<b>TVPatch \&tvp</b>\n\n
		The map patch to set. */
		void		setTVPatch(int i, TVPatch &tvp)	{ tvPatches[1][i] = tvp; }
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Previous to R4, this method returned an UVVert\&.\n\n
		Returns the 'i-th' map vertex of the specified channel.
		\par Parameters:
		<b>int channel</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int i</b>\n\n
		The index of the map vertex to retrieve. */
		PatchTVert &	getTVertChannel(int channel, int i)	{ return tVerts[channel?0:1][i];  }
		/*! \remarks Previous to R4, this method returned an UVVert\&.\n\n
		Returns the 'i-th' map vertex.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the map vertex to retrieve. */
		PatchTVert &	getTVert(int i)	{ return tVerts[1][i];  }
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Previous to R4, this method returned an UVVert*.\n\n
		Returns a pointer to the 'i-th' map vertex of the specified channel.
		\par Parameters:
		<b>int channel</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int i</b>\n\n
		The index of the texture vertex. */
		PatchTVert *	getTVertPtrChannel(int channel, int i)	{ return tVerts[channel?0:1]+i; }
		/*! \remarks Previous to R4, this method returned an UVVert*.\n\n
		Returns a pointer to the 'i-th' map vertex.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the map vertex. */
		PatchTVert *	getTVertPtr(int i)	{ return tVerts[1]+i; }
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Returns the 'i-th' texture patch structure of the specified channel.
		\par Parameters:
		<b>int channel</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int i</b>\n\n
		The index of the TVPatch. */
		TVPatch &	getTVPatchChannel(int channel, int i)	{ return tvPatches[channel?0:1][i];  }
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Returns the 'i-th' texture patch from map channel 1.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the TVPatch. */
		TVPatch &	getTVPatch(int i)	{ return tvPatches[1][i];  }

		// New map methods: for these, channel 0 is v.c. channel, and anything higher is a map channel.
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the specified mapping vertex in the channel passed.
		\par Parameters:
		<b>int mp</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int i</b>\n\n
		The zero based index of the vert to set.\n\n
		<b>const UVVert \&xyz</b>\n\n
		The vert to set. */
		void setMapVert (int mp, int i, const UVVert &xyz) { mapVerts(mp)[i] = xyz; }
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the specified mapping vertex in the channel passed.
		\par Parameters:
		<b>int mp</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int i</b>\n\n
		The zero based index of the vert to set.\n\n
		<b>float x</b>\n\n
		The x coordinate of the vert to set.\n\n
		<b>float y</b>\n\n
		The y coordinate of the vert to set.\n\n
		<b>float z</b>\n\n
		The z coordinate of the vert to set. */
		void setMapVert (int mp, int i, float x, float y, float z) { Point3 MV(x,y,z); setMapVert (mp, i, MV); }
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the specified mapping patch in the channel passed.
		\par Parameters:
		<b>int mp</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int i</b>\n\n
		The zero based index of the TVPatch to set.\n\n
		<b>const TVPatch \&tvp</b>\n\n
		The map patch to set. */
		void setMapPatch (int mp, int i, const TVPatch &tvp) { mapPatches(mp)[i] = tvp; }
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Previous to R4, this method returned an UVVert\&.\n\n
		Returns a reference to the specified mapping vert from the specified
		channel.
		\par Parameters:
		<b>int mp</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int i</b>\n\n
		The zero based index of the vert to get. */
		PatchTVert & getMapVert (int mp, int i) { return mapVerts(mp)[i]; }
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Previous to R4, this method returned an UVVert*.\n\n
		Returns a pointer to the specified mapping vert from the specified
		channel.
		\par Parameters:
		<b>int mp</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int i</b>\n\n
		The zero based index of the vert to get. */
		PatchTVert * getMapVertPtr (int mp, int i) { return mapVerts(mp) + i; }
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns a reference to the specified map patch from the specified
		channel.
		\par Parameters:
		<b>int mp</b>\n\n
		The mapping channel. In this method, 0 is the vertex color channel, and
		channels 1 through MAX_MESHMAPS-1 are the map channels.\n\n
		<b>int i</b>\n\n
		The zero based index of the map patch to get. */
		TVPatch & getMapPatch (int mp, int i) { return mapPatches(mp)[i];  }
		/*! \remarks This method is no longer used. */
		void		setMtlIndex(MtlID i)	{ mtlIndex = i; }
		/*! \remarks This method is no longer used. */
		MtlID		getMtlIndex(void) 		{ return mtlIndex; }
	    CoreExport MtlID		getPatchMtlIndex(int i);
		CoreExport void		setPatchMtlIndex(int i, MtlID id); 	

		// Automatically update all the adjacency info, etc.
		// Returns TRUE if patch mesh is valid, FALSE if it's not!
		// MAXr4: New option, can update linkages for single new patch by supplying
		// the 'patch' index.  Recomputes linkages for entire patch mesh if no index supplied.
		/*! \remarks This is an important method to call after putting
		together a <b>PatchMesh</b>. This method does the work to figure out
		how the <b>PatchMesh</b> is connected together, one patch to another.
		It determines which edges are used by which patches and so on.
		\par Parameters:
		<b>int patch</b>\n\n
		This option is available in release 4.0 and later only.\n\n
		This optional parameter allows you to only update adjacency information
		for a single new patch. If the plugin code is adding new patches and
		not removing any others then you can call this method with the new
		patch index and the various adjacency information in vertices, vectors,
		and edges will be built. If you call this method for the entire object
		(no parameter or <b>patch</b> \< 0) it will destroy all adjacency
		information and rebuild it from scratch.
		\return  TRUE if the patch mesh is valid, FALSE if it is not. */
		CoreExport BOOL		buildLinkages(int patch=-1);
		
		// Compute the interior bezier points for each patch in the mesh
		/*! \remarks This method computes the interior bezier points for each
		patch in the mesh. This method should be called after any modifications
		have been made to alter the <b>PatchMesh</b>(for example changes to
		point positions). If there are any automatic patches this will compute
		the interior vectors. */
		CoreExport void		computeInteriors();

		// Compute the degree-4 bezier points for each triangular patch in the mesh
		CoreExport void		computeAux();

		/*! \remarks Renders a patch mesh using the specified graphics window
		and array of materials.
		\par Parameters:
		<b>GraphicsWindow *gw</b>\n\n
		Points to the graphics window to render to.\n\n
		<b>Material *ma</b>\n\n
		The list of materials to use to render the patch.\n\n
		<b>RECT *rp</b>\n\n
		Specifies the rectangular region to render. If the patch mesh should be
		rendered to the entire viewport pass NULL.\n\n
		<b>int compFlags</b>\n\n
		One or more of the following flags:\n\n
		<b>COMP_TRANSFORM</b>\n\n
		Forces recalculation of the model to screen transformation; otherwise
		attempt to use the cache.\n\n
		<b>COMP_IGN_RECT</b>\n\n
		Forces all polygons to be rendered; otherwise only those intersecting
		the box will be rendered.\n\n
		<b>COMP_LIGHTING</b>\n\n
		Forces re-lighting of all vertices (as when a light moves); otherwise
		only re-light moved vertices\n\n
		<b>COMP_ALL</b>\n\n
		All of the above flags.\n\n
		<b>COMP_OBJSELECTED</b>\n\n
		If this bit is set then the node being displayed by this mesh is
		selected. Certain display flags only activate when this bit is set.\n\n
		<b>COMP_OBJFROZEN</b>\n\n
		If this bit is set then the node being displayed by this mesh is
		frozen.\n\n
		<b>int numMat=1</b>\n\n
		The number of materials supported. */
		CoreExport void		render(GraphicsWindow *gw, Material *ma, RECT *rp, int compFlags, int numMat=1);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will render a 'gizmo' version of the PatchMesh and is used
		primarily by Editable Patch and Edit Patch to facilitate the Show End
		Result feature.
		\par Parameters:
		<b>GraphicsWindow *gw</b>\n\n
		The graphics window associated with the viewport the patch mesh gizmo
		should be shown in. */
		CoreExport void		renderGizmo(GraphicsWindow *gw);
		/*! \remarks Checks the given HitRecord <b>hr</b> to see if it
		intersects the patch mesh object.
		\par Parameters:
		<b>GraphicsWindow *gw</b>\n\n
		Points to the graphics window to check.\n\n
		<b>Material *ma</b>\n\n
		The list of materials for the patch mesh.\n\n
		<b>HitRegion *hr</b>\n\n
		This describes the properties of a region used for the hit testing. See
		Class HitRegion.\n\n
		<b>int abortOnHit = FALSE</b>\n\n
		If nonzero, the hit testing is complete after any hit; otherwise all
		hits are checked.\n\n
		<b>int numMat=1</b>\n\n
		The number of materials supported.
		\return  TRUE if the item was hit; otherwise FALSE. */
		CoreExport BOOL		select(GraphicsWindow *gw, Material *ma, HitRegion *hr, int abortOnHit=FALSE, int numMat=1);
		/*! \remarks Checks to see if there is a snap point near the given
		mouse point.
		\par Parameters:
		<b>GraphicsWindow *gw</b>\n\n
		The graphics window in which to check.\n\n
		<b>SnapInfo *snap</b>\n\n
		This structure describes the snap settings used, and the results of the
		snap test. See Structure SnapInfo.\n\n
		<b>IPoint2 *p</b>\n\n
		The mouse point to check.\n\n
		<b>Matrix3 \&tm</b>\n\n
		The object transformation matrix. This is the transformation to place
		the object into the world coordinate system. */
		CoreExport void		snap(GraphicsWindow *gw, SnapInfo *snap, IPoint2 *p, Matrix3 &tm);
		/*! \remarks This method may be called to perform sub-object hit
		testing of the patch mesh.
		\par Parameters:
		<b>GraphicsWindow *gw</b>\n\n
		The graphics window associated with the viewport the patch mesh is
		being hit tested in.\n\n
		<b>Material *ma</b>\n\n
		The list of materials for the patch mesh.\n\n
		<b>HitRegion *hr</b>\n\n
		This describes the properties of a region used for the hit testing. See
		Class HitRegion.\n\n
		<b>DWORD flags</b>\n\n
		Flags for sub object hit testing. One or more of the following
		values:\n\n
		<b>SUBHIT_PATCH_SELONLY</b>\n\n
		Selected only.\n\n
		<b>SUBHIT_PATCH_UNSELONLY</b>\n\n
		Unselected only.\n\n
		<b>SUBHIT_PATCH_ABORTONHIT</b>\n\n
		Abort hit testing on the first hit found.\n\n
		<b>SUBHIT_PATCH_SELSOLID</b>\n\n
		This treats selected items as solid and unselected items as not solid.
		Treating an item as solid means the patch will be hit if the mouse is
		anywhere inside the patch region and not just over a visible edge.\n\n
		<b>SUBHIT_PATCH_VERTS</b>\n\n
		Hit test vertices.\n\n
		<b>SUBHIT_PATCH_VECS</b>\n\n
		Hit test vectors.\n\n
		<b>SUBHIT_PATCH_PATCHES</b>\n\n
		Hit test patches.\n\n
		<b>SUBHIT_PATCH_EDGES</b>\n\n
		Hit test edges.\n\n
		<b>SubPatchHitList\& hitList</b>\n\n
		See Class SubPatchHitList.\n\n
		<b>int numMat=1</b>\n\n
		The number of materials for the mesh.
		\return  TRUE if the item was hit; otherwise FALSE. */
		CoreExport BOOL 	SubObjectHitTest(GraphicsWindow *gw, Material *ma, HitRegion *hr,
								DWORD flags, SubPatchHitList& hitList, int numMat=1 );

		/*! \remarks Computes the bounding box of the patch mesh. The bounding
		box is stored with the patch mesh object, use <b>getBoundingBox()</b>
		to retrieve it. */
		CoreExport void		buildBoundingBox(void);
		/*! \remarks Retreives the bounding box of the patch mesh object.
		\par Parameters:
		<b>Matrix3 *tm=NULL</b>\n\n
		The optional TM allows the box to be calculated in any space. */
		CoreExport Box3		getBoundingBox(Matrix3 *tm=NULL); // RB: optional TM allows the box to be calculated in any space.
		                                              // NOTE: this will be slower becuase all the points must be transformed.
		CoreExport void		GetDeformBBox(Box3& box, Matrix3 *tm=NULL, BOOL useSel=FALSE);
		
		/*! \remarks This method should be called when the <b>PatchMesh</b>
		changes. It invalidates the caches of the patch mesh. */
		CoreExport void 	InvalidateGeomCache();
		CoreExport void		InvalidateMesh();		// Also invalidates relaxed mesh
		CoreExport void		InvalidateRelaxedMesh();
		/*! \remarks Frees everything from the patch mesh. */
		CoreExport void 	FreeAll(); //DS
				
		// functions for use in data flow evaluation
		/*! \remarks This method is used internally in data flow evaluation. */
		CoreExport void 	ShallowCopy(PatchMesh *amesh, ChannelMask channels);
		/*! \remarks This method is used internally in data flow evaluation. */
		CoreExport void 	DeepCopy(PatchMesh *amesh, ChannelMask channels);
		/*! \remarks This method is used internally in data flow evaluation. */
		CoreExport void   NewAndCopyChannels(ChannelMask channels);
		/*! \remarks This method is used internally in data flow evaluation. */
		CoreExport void 	FreeChannels(ChannelMask channels, int zeroOthers=1);

		// Display flags
		/*! \remarks Sets the state of the specified display flags.
		\par Parameters:
		<b>DWORD f</b>\n\n
		The flags to set. See \ref patchDisplayFlags. */
		void		SetDispFlag(DWORD f) { dispFlags |= f; }
		/*! \remarks Returns the state of the specified display flags.
		\par Parameters:
		<b>DWORD f</b>\n\n
		The flags to get. See \ref patchDisplayFlags. */
		DWORD		GetDispFlag(DWORD f) { return dispFlags & f; }
		/*! \remarks Clears the specified display flags.
		\par Parameters:
		<b>DWORD f</b>\n\n
		The flags to clear. See \ref patchDisplayFlags. */
		void		ClearDispFlag(DWORD f) { dispFlags &= ~f; }

		// Selection access
		BitArray& 	VecSel() { return vecSel; }		// CAL-06/10/03: (FID #1914)
		/*! \remarks Returns the bits representing the vertex selection
		status. See the Data Members above. See
		Class BitArray. */
		BitArray& 	VertSel() { return vertSel; }
		/*! \remarks Returns the bits representing the edge selection status.
		See the Data Members above. See Class BitArray. */
		BitArray& 	EdgeSel() { return edgeSel; }
		/*! \remarks Returns the bits representing the patch selection status.
		See the Data Members above. See Class BitArray. */
		BitArray& 	PatchSel() { return patchSel; }

		// Constructs a vertex selection list based on the current selection level.
		/*! \remarks Constructs a vertex selection list based on the current
		selection level. For example if the selection level is at object level
		all the bits are set. If the selection level is at vertex level only
		the selected vertex bits are set. See Class BitArray. */
		CoreExport BitArray 	VertexTempSel();

		// Apply the coplanar constraints to the patch mesh
		// (Optionally only apply it to selected vertices)
		/*! \remarks This method may be called to apply the coplanar
		constraints to the patch mesh. The constraints may optionally only
		apply to selected vertices. There is a flag that may be set for a patch
		vertex (<b>PVERT_COPLANAR</b>). For example, you can set this flag to
		make a vertex coplanar with its vectors. If this is done, then when
		this method is called, the patch code will then go through the
		<b>PatchMesh</b> and find the average plane that is used by the vertex
		and all the vectors associated with it. It will then constrain all the
		vectors to lie in this plane (by rotating them so that they lie on the
		plane). In this way there will be a consistent transition between the
		patches sharing the vertex.
		\par Parameters:
		<b>BOOL selOnly = FALSE</b>\n\n
		If TRUE the constraints are only applied to the selected vertices;
		otherwise all vertices. */
		CoreExport void ApplyConstraints(BOOL selOnly = FALSE);

		// Create triangular or quadrilateral patch
		/*! \remarks Create a quadrilateral patch given a patch
		index and a list of all the vertices, vectors, interiors, and a smoothing
		group.
		\par Parameters:
		<b>int index -</b> The index of the patch to create (0\>= index \<
		<b>numPatches</b>).\n\n
		<b>int va -</b> The first vertex.\n\n
		<b>int vab -</b> Vector ab.\n\n
		<b>int vba -</b> Vector ba.\n\n
		<b>int vb -</b> The second vertex.\n\n
		<b>int vbc -</b> Vector bc.\n\n
		<b>int vcb -</b> Vector cb.\n\n
		<b>int vc -</b> The third vertex.\n\n
		<b>int vcd -</b> Vector cd.\n\n
		<b>int vdc -</b> Vector dc.\n\n
		<b>int vd -</b> The fourth vertex.\n\n
		<b>int vda -</b> Vector da.\n\n
		<b>int vad -</b> Vector ad.\n\n
		<b>int i1 -</b> Interior 1.\n\n
		<b>int i2 -</b> Interior 2.\n\n
		<b>int i3 -</b> Interior 3.\n\n
		<b>int i4 -</b> Interior 4.\n\n
		<b>DWORD sm -</b> The smoothing group.
		\return  TRUE if the patch was created; otherwise FALSE. */
		CoreExport BOOL MakeQuadPatch(int index, int va, int vab, int vba, int vb, int vbc, int vcb, int vc, int vcd, int vdc, int vd, int vda, int vad, int i1, int i2, int i3, int i4, DWORD sm);
		/*! \remarks Create a triangular patch given a patch
		index and a list of all the vertices, vectors, interiors, and a smoothing
		group.
		\par Parameters:
		<b>int index -</b> The index of the patch to create (0\>= index \<
		numPatches).\n\n
		<b>int va -</b> The first vertex.\n\n
		<b>int vab -</b> Vector ab.\n\n
		<b>int vba -</b> Vector ba.\n\n
		<b>int vb -</b> The second vertex.\n\n
		<b>int vbc -</b> Vector bc.\n\n
		<b>int vcb -</b> Vector cb.\n\n
		<b>int vc -</b> The third vertex.\n\n
		<b>int vca -</b> Vector ca.\n\n
		<b>int vac -</b> Vector ac.\n\n
		<b>int i1 -</b> Interior 1.\n\n
		<b>int i2 -</b> Interior 2.\n\n
		<b>int i3 -</b> Interior 3.\n\n
		<b>DWORD sm -</b> The smoothing group.
		\return  TRUE if the patch was created; otherwise FALSE. */
		CoreExport BOOL MakeTriPatch(int index, int va, int vab, int vba, int vb, int vbc, int vcb, int vc, int vca, int vac, int i1, int i2, int i3, DWORD sm);

		// Get/Set mesh steps, adaptive switch
		/*! \remarks Sets the number of steps along each edge that determines
		how fine the mesh is generated off the patch.
		\par Parameters:
		<b>int steps</b>\n\n
		The number of steps to set. */
		CoreExport void SetMeshSteps(int steps);
		/*! \remarks Returns the number of mesh steps. */
		CoreExport int GetMeshSteps();
#ifndef NO_OUTPUTRENDERER
//3-18-99 watje to support render steps
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the Surface Render Steps setting.
		\par Parameters:
		<b>int steps</b>\n\n
		The value to set. */
		CoreExport void SetMeshStepsRender(int steps);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the Surface Render Steps setting. */
		CoreExport int GetMeshStepsRender();
#endif // NO_OUTPUTRENDERER
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the 'Show Interior Edges' value.
		\par Parameters:
		<b>BOOL si</b>\n\n
		TRUE for on; FALSE for off. */
		CoreExport void SetShowInterior(BOOL si);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the 'Show Interior Edge' setting; TRUE if on; FALSE if off. */
		CoreExport BOOL GetShowInterior();

		CoreExport void SetUsePatchNormals(BOOL usePatchNorm);
		CoreExport BOOL GetUsePatchNormals();

		/*! \remarks This is currently not used. Reserved for future use. */
		CoreExport void SetAdaptive(BOOL sw);
		/*! \remarks This is currently not used. Reserved for future use. */
		CoreExport BOOL GetAdaptive();

		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Sets the tesselation approximation object used for viewport rendering.
		\par Parameters:
		<b>TessApprox tess</b>\n\n
		The tesselation approximation object to be used for viewport rendering.
		*/
		CoreExport void SetViewTess(TessApprox tess);
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Returns the tesselation approximation object used for rendering in the
		viewports. */
		CoreExport TessApprox GetViewTess();
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Sets the tesselation approximation object used for production
		rendering.
		\par Parameters:
		<b>TessApprox tess</b>\n\n
		The tesselation approximation object to be used for production
		rendering. */
		CoreExport void SetProdTess(TessApprox tess);
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		Returns the tesselation approximation object used for production
		rendering. */
		CoreExport TessApprox GetProdTess();
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the tesselation approximation object used for display in the
		viewports.
		\par Parameters:
		<b>TessApprox tess</b>\n\n
		The tesselation approximation object to be used for the viewports. */
		CoreExport void SetDispTess(TessApprox tess);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the tesselation approximation object used for display in the
		viewports. */
		CoreExport TessApprox GetDispTess();
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns TRUE if normals are used from the viewport tesselator;
		otherwise FALSE. */
		CoreExport BOOL GetViewTessNormals();
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets if normals are used from the viewport tesselator.
		\par Parameters:
		<b>BOOL use</b>\n\n
		TRUE to use normals; FALSE to not use them. */
		CoreExport void SetViewTessNormals(BOOL use);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns TRUE if normals are used from the production renderer
		tesselator; otherwise FALSE. */
		CoreExport BOOL GetProdTessNormals();
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets if normals are used from the production renderer tesselator. */
		CoreExport void SetProdTessNormals(BOOL use);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns TRUE if the viewport mesh is welded after tesselation;
		otherwise FALSE. */
		CoreExport BOOL GetViewTessWeld();
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets if the viewport mesh is welded after tesselation; otherwise FALSE.
		\par Parameters:
		<b>BOOL weld</b>\n\n
		TRUE to weld; FALSE to not weld. */
		CoreExport void SetViewTessWeld(BOOL weld);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns TRUE if the production renderer mesh is welded after
		tesselation; otherwise FALSE. */
		CoreExport BOOL GetProdTessWeld();
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets if the production renderer mesh is welded after tesselation;
		otherwise FALSE.
		\par Parameters:
		<b>BOOL weld</b>\n\n
		TRUE to weld; FALSE to not weld. */
		CoreExport void SetProdTessWeld(BOOL weld);

		// Find the edge index for a given vertex-vector-vector-vertex sequence
		/*! \remarks This method is used internally. */
		int GetEdge(int v1, int v12, int v21, int v2, int p);
		// Find the edge indices for two given vertices
		CoreExport Tab<int> GetEdge(int v1, int v2) const;

		// Find all of the patch indices for two given vertices
		CoreExport Tab<int> GetPatches(int v1, int v2) const;

		// find the indices for the patches that depend on the given vertex
		CoreExport Tab<int> GetPatches(int vert) const;

		// find the indices for the edges that depend on the given vertex
		CoreExport Tab<int> GetEdges(int vert) const;

		// find the indices for the vectors that depend on the given vertex
		CoreExport Tab<int> GetVectors(int vert) const;


		// Apply mapping to the patch mesh
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Set the number of texture maps used by this PatchMesh.
		\par Parameters:
		<b>int ct</b>\n\n
		The number to use. This is a value between 2 and
		<b>MAX_MESHMAPS-1</b>.\n\n
		<b>BOOL keep=TRUE</b>\n\n
		TRUE to keep the old mapping information after the resize; FALSE to
		discard it. */
		CoreExport void setNumMaps (int ct, BOOL keep=TRUE);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the number of mapping channels in use. */
		int getNumMaps () { return numTVerts.Count(); }
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will allocate mapping channels as needed. If the map
		channel is already present, no action is taken. Otherwise, the
		additional channels are created.
		\par Parameters:
		<b>int chan</b>\n\n
		Specifies which channel. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_mapping_channel_index_values.html">List of
		Mapping Channel Index Values</a>. If zero (special vertex color
		channel) and <b>init</b> is TRUE, all vertex colors are initialized to
		white (1,1,1).\n\n
		<b>BOOL init=TRUE</b>\n\n
		If TRUE, the channel is initialized to match the PatchMesh's structure.
		*/
		CoreExport void setMapSupport(int chan, BOOL init=TRUE);	// Make sure map support is there for this channel, optionally init
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns TRUE if the specified mapping channel is supported; otherwise
		FALSE.
		\par Parameters:
		<b>int mp</b>\n\n
		Specifies which channel. See
		<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_mapping_channel_index_values.html">List of
		Mapping Channel Index Values</a>. */
		BOOL getMapSupport (int mp) { return ((mp<tvPatches.Count()) && tvPatches[mp]) ? TRUE : FALSE; }
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the maximum number of possible mapping channels. */
		int NumMapChannels () { return MAX_MESHMAPS; }
		/*! \remarks This method may be called
		to map this <b>PatchMesh</b> with UVW mapping coordinates.
		\par Parameters:
		<b>int type</b>\n\n
		The mapping type. One of the following values:\n\n
		<b>MAP_PLANAR</b>\n\n
		<b>MAP_CYLINDRICAL</b>\n\n
		<b>MAP_SPHERICAL</b>\n\n
		<b>MAP_BALL</b>\n\n
		<b>MAP_BOX</b>\n\n
		<b>float utile</b>\n\n
		Number of tiles in the U direction.\n\n
		<b>float vtile</b>\n\n
		Number of tiles in the V direction.\n\n
		<b>float wtile</b>\n\n
		Number of tiles in the W direction.\n\n
		<b>int uflip</b>\n\n
		If nonzero the U values are mirrored.\n\n
		<b>int vflip</b>\n\n
		If nonzero the V values are mirrored.\n\n
		<b>int wflip</b>\n\n
		If nonzero the W values are mirrored.\n\n
		<b>int cap</b>\n\n
		This is used with <b>MAP_CYLINDRICAL</b>. If nonzero, then any patch normal
		that is pointing more vertically than horizontally will be mapped using planar
		coordinates.\n\n
		<b>const Matrix3 \&tm</b>\n\n
		This defines the mapping space. As each point is mapped, it is multiplied by
		this matrix, and then it is mapped.\n\n
		<b>int channel=1</b>\n\n
		This parameter is available in release 2.0 and later only.\n\n
		This indicates which channel the mapping is applied to -- <b>channel==1</b>
		corresponds to the original texture channel. Note that this is a change from
		what it meant before release 3.0. Previously channel 1 referred to the color
		per vertex channel (and this parameter defaulted to 0). */
		CoreExport void ApplyUVWMap(int type,
			float utile, float vtile, float wtile,
			int uflip, int vflip, int wflip, int cap,
			const Matrix3 &tm,int channel=1);

		// Tag the points in the patch components to record our topology (This stores
		// identifying values in the various aux2 fields in the Patch)
		// This info can be used after topology-changing operations to remap information
		// tied to vertices, edges and patches.
		// Returns TRUE if tagged successfully
		/*! \remarks This method tags the points in the patch components to
		record our topology (this stores identifying values in the various aux2
		fields in the Patch). This information can be used after
		topology-changing operations to remap information tied to vertices,
		edges and patches.
		\return  Returns TRUE if tagged successfully; otherwise FALSE. */
		CoreExport BOOL RecordTopologyTags();

		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Transforms the vertices and vectors of the patch mesh, re- computes the
		interior bezier points for each patch in the mesh and invalidates the
		geometry cache.
		\par Parameters:
		<b>Matrix3 \&tm</b>\n\n
		The matrix to transform with. */
		CoreExport void Transform(Matrix3 &tm);

		// Weld the vertices
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method is used internally. */
		CoreExport BOOL Weld(float thresh, BOOL weldIdentical=FALSE, int startVert=0);

		// weld one selected vertex to another selected vertex
		CoreExport BOOL PatchMesh::Weld(int fromVert, int toVert);

		// Weld selected edges
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will weld any edges which are selected and have the same
		endpoints.
		\return  TRUE if any welding took place, otherwise FALSE. */
		CoreExport BOOL WeldEdges();

		// General-purpose deletion
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This is a method which may be used to delete sets of verts or patches.
		\par Parameters:
		<b>BitArray \&delVerts</b>\n\n
		A bit array with bits set for verts to delete.\n\n
		<b>BitArray \&delPatches</b>\n\n
		A bit array with bits set for patches set to delete. */
		CoreExport void DeletePatchParts(BitArray &delVerts, BitArray &delPatches);

		// Clone specified patch geometry (or selected patches if 'patches' == NULL)
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will copy the patches specified by the BitArray, or by the
		patch selection set if the BitArray <b>patches</b> pointer is NULL.
		This method is used by Editable Patch and Edit Patch to facilitate the
		shift-copy operations.
		\par Parameters:
		<b>BitArray *patches</b>\n\n
		The array containing the series of selected patches. */
		CoreExport void ClonePatchParts(BitArray *patches = NULL);

		// Subdivision
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Subdivides the selected edge or patch.
		\par Parameters:
		<b>int type</b>\n\n
		One of the following values:\n\n
		<b>SUBDIV_EDGES</b>\n\n
		Subdivides an edge.\n\n
		<b>SUBDIV_PATCHES</b>\n\n
		Subdivides an entire patch\n\n
		<b>BOOL propagate</b>\n\n
		TRUE to propogate; FALSE to not propogate. */
		CoreExport void Subdivide(int type, BOOL propagate);

		// Add patch to selected single-patch edges
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Add a patch of the desired type to each selected edge that doesn't have
		two patches attached.
		\par Parameters:
		<b>int type</b>\n\n
		One of the following values:\n\n
		<b>PATCH_TRI</b>\n\n
		<b>PATCH_QUAD</b> */
		CoreExport void AddPatch(int type);

		// Hooks a vertex to a patch edge
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method is obsolete and should not be used. */
		CoreExport int AddHook();
		//tries to add hook patch at the specified vert
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method is obsolete and should not be used. */
		CoreExport int AddHook(int index);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method is obsolete and should not be used. */
		CoreExport int AddHook(int vertIndex, int segIndex) ;

		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method is obsolete and should not be used. */
		CoreExport int RemoveHook();
		//goes through and looks for invalid hooks and tries to fix them used when topology changes
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method is used internally. */
		CoreExport int UpdateHooks();

		Tab<Point3> extrudeDeltas;
		Tab<ExtrudeData> extrudeData;
		Tab<Point3> edgeNormals;
		Tab<int> newEdges;
		Tab<int> newVerts;
//creates initial extrude faces and temporary data used in move normal
//type = PATCH_PATCH or PATCH_EDGE only
//edgeClone: Only for edge mode, clones edges before extrusion
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method will create all the necessary geometry for an extrusion
		operation.
		\par Parameters:
		<b>int type</b>\n\n
		This option is available in release 4.0 and later only.\n\n
		Specifies the extrusion type, either <b>PATCH_PATCH</b> or
		<b>PATCH_EDGE</b>. If the extrusion type is set to <b>PATCH_EDGE</b>,
		then the <b>edgeClone</b> paramter will tell the function to clone the
		selected edges prior to creating the extrusion geometry.\n\n
		<b>BOOL edgeClone</b>\n\n
		This option is available in release 4.0 and later only.\n\n
		The edge clone flag. If set to TRUE the function will clone the
		selected edges prior to creating the exstrusion geometry. */
		CoreExport void CreateExtrusion(int type = PATCH_PATCH, BOOL edgeClone=FALSE);
//computes the average normals of the selected patches or selected edges
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method will allow you to obtain the average normal of all selected
		patches or edges.
		\par Parameters:
		<b>int type</b>\n\n
		This option is available in release 4.0 and later only.\n\n
		Specifies if the average normal return is based on all selected
		patches, using <b>PATCH_PATCH,</b> or all selected edges using
		<b>PATCH_EDGE</b>. */
		CoreExport Point3 AverageNormals(int type = PATCH_PATCH);
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method is used internally. */
		CoreExport Point3 PatchNormal(int index);
		CoreExport void BuildPatchNormals();		// Only builds normals if necessary
		CoreExport void InvalidatePatchNormals();
		CoreExport Point3 EdgeNormal(int index);
		/*! \remarks		This method is used internally. */
		CoreExport void MoveNormal(float amount, BOOL useLocalNorms, int type/* = PATCH_PATCH*/);
		// Flip normal of indicated patch, or selected patches if -1, or all patches if -2
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method flips the normal of the specified patch. This is done by
		reordering the vertices. If the PatchMesh has textures assigned to it
		then the texture patches are processed as well.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the patch for which you want to flip the normal. */
		CoreExport void FlipPatchNormal(int index);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method makes sure the patches in the operation set are all facing
		the same direction. This is determined by the vertex order around the
		edges of the patch.
		\par Parameters:
		<b>BOOL useSel</b>\n\n
		If this parameter is set to TRUE, the operation set is the set of
		selected patches as indicated by the <b>patchSel</b> BitArray. If this
		parameter is set to FALSE, all patches are processed. */
		CoreExport void UnifyNormals(BOOL useSel);
//creates temporary data used in Bevel
		BitArray bevelEdges;
		Tab<float> edgeDistances;
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method is used internally. */
		CoreExport void CreateBevel();
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method is used internally. */
		CoreExport void Bevel(float amount, int smoothStart, int smoothEnd);
//computes the bevel direction of patch based on which edges are open
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		This method is for internal use. */
		Point3 GetBevelDir(int patchVertID);

		// Attach a second PatchMesh, adjusting materials
		CoreExport void Attach(PatchMesh *attPatch, int mtlOffset);

		// Change the interior type of a patch or selected patches (index < 0)
		CoreExport void ChangePatchInterior(int index, int type);

		// Change the type of a vertex or selected vertices (index < 0)
		CoreExport void ChangeVertType(int index, int type);

		CoreExport BOOL SelVertsSameType();	// Are all selected vertices the same type?
		CoreExport BOOL SelPatchesSameType();	// Are all selected patches the same type?

		// CAL-04/28/03: reset vertex tangents (FID #827)
		CoreExport BOOL ResetVertexTangents(int index);
		CoreExport BOOL ResetVertexTangents(bool useSel=true, const BitArray *vSel=NULL);

		// CAL-04/23/03: patch smooth (FID #1419)
		CoreExport BOOL PatchSmoothVector(bool useSel=true, const BitArray *vSel=NULL);
		CoreExport BOOL PatchSmoothVertex(bool useSel=true, const BitArray *vSel=NULL);
		CoreExport BOOL PatchSmoothEdge(bool useSel=true, const BitArray *eSel=NULL);
		CoreExport BOOL PatchSmoothPatch(bool useSel=true, const BitArray *pSel=NULL);

		// CAL-04/23/03: Shrink/Grow, Edge Ring/Loop selection. (FID #1419)
		CoreExport void ShrinkSelection(int type);
		CoreExport void GrowSelection(int type);
		CoreExport void SelectEdgeRing(BitArray &eSel);
		CoreExport void SelectEdgeLoop(BitArray &eSel);

		// Dump the patch mesh structure via DebugPrints
		/*! \remarks This method may be called to dump the patch mesh
		structure via <b>DebugPrint()</b>. See
		<a href="ms-its:3dsmaxsdk.chm::/debug_debugging.html">Debugging</a>. */
		CoreExport void Dump();
#ifdef CHECK_TRI_PATCH_AUX
		CoreExport void CheckTriAux();
#endif //CHECK_TRI_PATCH_AUX

		// Ready the mesh cache
		CoreExport void PrepareMesh();
		CoreExport void PrepareUnrelaxedMesh();

		// Get the Mesh version
		CoreExport Mesh& GetMesh();
		CoreExport Mesh& GetUnrelaxedMesh();

		// Actual mesh tessellation used by above methods; optionally allows calling
		// routine to propegate selection from the patch to the mesh.
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will produce the mesh version of the PatchMesh.
		\par Parameters:
		<b>Mesh\& msh</b>\n\n
		The mesh in which the resulting mesh should be stored.\n\n
		<b>DWORD convertFlags</b>\n\n
		The flags modifying the mesh process.\n\n
		<b>PATCH_CONVERT_KEEPSEL</b>\n\n
		This flag indicates that the subobject vertex and patch selections in
		the PatchMesh should be converted to subobject vertex and face
		selections in the mesh.\n\n
		<b>PATCH_CONVERT_USESOFTSEL</b>\n\n
		This flag indicates that soft selections should be used in the mesh to
		interpolate between selected and nonselected vertices. (Soft Selections
		cannot be based on edge or patch selection in the PatchMesh, only
		vertex selection.) */
		CoreExport void ComputeMesh (Mesh & m, DWORD convertFlags);

		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Calculates the intersection of the specified ray with this patch mesh
		object. This method calls the method of the same name on the Mesh
		cache.
		\par Parameters:
		<b>Ray\& ray</b>\n\n
		Specifies the origin and direction of the ray to intersect with the
		patch mesh. See Class Ray.\n\n
		<b>float\& at</b>\n\n
		The computed point of intersection on the surface of the patch
		mesh.\n\n
		<b>Point3\& norm</b>\n\n
		The face normal at the point of intersection (<b>at</b>).
		\return  Nonzero if the ray intersected the mesh object; otherwise 0.
		Note that this method ignores backfaces when computing the result. */
		CoreExport int IntersectRay(Ray& ray, float& at, Point3& norm);

		CoreExport IOResult Save(ISave* isave);
		CoreExport IOResult Load(ILoad* iload);

		// TH 6/12/00 -- Added AutoSmooth method for Smooth modifier
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will perform automatic smoothing on the patch mesh. The
		smoothing angle is determined for each patch by computing the normals
		at each corner of the patch (using the corner vertex and the two edge
		vectors connected to that vertex), then averaging the normals of the
		two corners of each edge to arrive at a normal for that edge. These
		normals are used to determine whether adjacent patches are within the
		auto-smooth threshold angle.
		\par Parameters:
		<b>float angle</b>\n\n
		The minimum angle between surface normals for smoothing to be applied,
		in radians.\n\n
		<b>BOOL useSel</b>\n\n
		If this parameter is set to TRUE then only the selected patches are
		smoothed.\n\n
		<b>BOOL preventIndirectSmoothing</b>\n\n
		TRUE to turn on; FALSE to leave it off. This matches the option in the
		Smooth Modifier UI -- use this to prevent smoothing 'leaks" when using
		this method. If you use this method, and portions of the patch mesh
		that should not be smoothed become smoothed, then try this option to
		see if it will correct the problem. Note that the problem it corrects
		is rare, and that checking this slows the automatic smoothing process.
		*/
		CoreExport void AutoSmooth(float angle,BOOL useSel,BOOL preventIndirectSmoothing);

		// Change/Get the mapping type of a patch or selected patches (index < 0)
//watje new patch mapping
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method changes the mapping of a patch or all selected patches to
		linear.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the patch for which to change the mapping to linear. A
		value \< 0 indicates all selected patches are to be changed to linear
		mapping. */
		CoreExport void ChangePatchToLinearMapping(int index);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method changes the mapping of a patch or all selected patches to
		curved.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the patch for which to change the mapping to curved. A
		value \< 0 indicates all selected patches are to be changed to curved
		mapping. */
		CoreExport void ChangePatchToCurvedMapping(int index);
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will check if one or all selected patches have linear
		mapping applied.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the patch for which to check if mapping is linear. A value
		\< 0 indicates all selected patches are checked for linear mapping.
		\return  TRUE if the specified patch or selected patches have linear
		mapping applied, otherwise FALSE. */
		CoreExport BOOL ArePatchesLinearMapped(int index); 
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will check if one or all selected patches have curved
		mapping applied.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the patch for which to check if mapping is curved. A value
		\< 0 indicates all selected patches are checked for curved mapping.
		\return  TRUE if the specified patch or selected patches have curved
		mapping applied, otherwise FALSE. */
		CoreExport BOOL ArePatchesCurvedMapped(int index); 
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method examines the selected edges and will return TRUE if all
		selected edges are used by only one single patch. If the method returns
		FALSE, there are no edges selected or any of the selected edges are
		used by more than one single patch. */
		CoreExport BOOL SingleEdgesOnly();	// Returns TRUE if all selected edges are used by only 1 edge
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method will return a BitArray that defines the group of patches
		defining an element that contains the patch indicated by the specified
		index. A PatchMesh element is any set of patches sharing common
		vertices.
		\par Parameters:
		<b>int index</b>\n\n
		The patch index for which to return the element. */
		CoreExport BitArray& GetElement(int index);

		// --- from InterfaceServer
		CoreExport BaseInterface* GetInterface(Interface_ID id);

	

// soft selection support
// should these be	private?:

//5-25-00 support for soft selections (tb)
		private:

		int     mVertexWeightSelectLevel;
		float * mpVertexWeights;
		int		numVertexWeights;
		int   * mpVertexEdgeDists;
		float * mpVertexDists;

		public:
		// NOTE: There is no int GetVertexWeightCount(); but there should be.  Developers can
		// generally count on the number of weights being = numVerts + numVecs.  The order in
		// the array is all the vertices, then all the vectors.
		CoreExport void  SetVertexWeightCount( int i ); // destroys existing weights, sets all weights = 0.0.
		CoreExport void  SetVertexWeight( int i, float w ) { assert( mpVertexWeights ); if ( i >= numVertexWeights ) return; mpVertexWeights[i] = w; }
		CoreExport float VertexWeight( int i ) { if ( !mpVertexWeights ) return 0.0f; if ( i >= numVertexWeights ) return 0.0f; return mpVertexWeights[i]; }
		CoreExport bool  VertexWeightSupport() { if ( mpVertexWeights ) return true; return false; }
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method provides direct access to the vertex weights array and is
		included to match a similar function in the Mesh class.
		\return  A pointer to the vertex weights array.
		\par Default Implementation:
		<b>{ return mpVertexWeights; }</b> */
		CoreExport float *GetVSelectionWeights() { return mpVertexWeights; }
		/*! \remarks This method is available in release 4.0 and later
		only.\n\n
		This method allocates a vertex weights array, if none is currently
		allocated. This method is included to match a similar function in the
		Mesh class. */
		CoreExport void  SupportVSelectionWeights();		// Allocate a weight table if none 
		CoreExport int   VertexWeightSelectLevel() { return mVertexWeightSelectLevel; } 

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

		CoreExport Point3 VertexNormal( int vIndex ); 

		CoreExport BOOL Relaxing();		// returns TRUE if Relax && RelaxValue != 0 && RelaxIter != 0
		CoreExport BOOL SetRelax(BOOL v);			// All "Set" ops return TRUE if option changed
		CoreExport BOOL SetRelaxViewports(BOOL v);
		CoreExport BOOL SetRelaxValue(float v);
		CoreExport BOOL SetRelaxIter(int v);
		CoreExport BOOL SetRelaxBoundary(BOOL v);
		CoreExport BOOL SetRelaxSaddle(BOOL v);
		CoreExport BOOL GetRelax();
		CoreExport BOOL GetRelaxViewports();
		CoreExport float GetRelaxValue();
		CoreExport int GetRelaxIter();
		CoreExport BOOL GetRelaxBoundary();
		CoreExport BOOL GetRelaxSaddle();
};

//!@{
//! \name Conversion flags
//! \brief These are used in conversion methods in core\converters.cpp and in poly\converters.cpp.
#define CONVERT_KEEPSEL					0x0001
#define CONVERT_USESOFTSEL			0x0002
#define CONVERT_SEL_LEVEL				0x0004
#define CONVERT_PATCH_USEQUADS	0x0010
#define CONVERT_NO_RELAX				0x0020
//!@} 

// Conversion methods:
CoreExport void ConvertMeshToPatch (Mesh &m, PatchMesh &pm, DWORD flags=0);
CoreExport void ConvertPatchToMesh (PatchMesh &pm, Mesh &m, DWORD flags=0);

//! \brief This method converts a Patch to Trimesh, similarly to the method ConvertPatchToMesh(). 
/*! However, the new method also stores the mapping from the Mesh's faces to the PatchMesh's 
patches in the 'mapping' parameter.
\param[in] pm - The patch to convert
\param[in] m - The mesh object we set our converted TriMesh onto.
\param[out] mapping - After conversion, this array will be filled a mapping from the Mesh's faces to the PatchMeshs faces.
\param[in] flags - One of the ConversionFlags */
CoreExport void ConvertPatchToMeshWithMapping (PatchMesh &pm, Mesh &m, Tab<int> *mapping, DWORD flags=0);
CoreExport void RelaxMesh(Mesh &mesh, float value, int iter, BOOL boundary, BOOL saddle);

