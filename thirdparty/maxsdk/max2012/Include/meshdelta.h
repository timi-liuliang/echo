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
// FILE:        MeshDelta.h
// DESCRIPTION: 
// AUTHOR:      Steve Anderson
// HISTORY:     Created June 1998
//**************************************************************************/

#pragma once

#include "maxheap.h"
#include "export.h"
#ifndef __MESHADJ__
#include "meshadj.h"
#endif

// STEVE: MeshDeltas and/or MapDeltas lack the following clever features:
// - Realloc amounts: currently we realloc arrays with no extra room for growth (dv especially).

// These classes encompass the notion of a "Mesh Edit".  They are the principal means
// of keeping track of what's going on in the Edit Mesh modifier, and have many standard
// mesh edits available for use elsewhere.

// Principle is that, while these work as designed on the right mesh, they will give
// some result on the wrong mesh.

// Order of operations:
// Verts/TVerts/CVerts created or cloned.
// Faces created -- indices correspond to original vert list, then create, then clone.
// Face attributes & indices changed.
// Verts & faces deleted.

// "Channels" of data in MeshDelta -- different from PART_GEOM type channels!
#define MDELTA_VMOVE 0x0001
#define MDELTA_VCLONE 0x0004
#define MDELTA_VCREATE MDELTA_VCLONE // MDELTA_VCREATE is used only by MapDelta and not MeshDelta
#define MDELTA_VDELETE 0x0008
#define MDELTA_VDATA 0x0010
#define MDELTA_FREMAP 0x0020
#define MDELTA_FCHANGE 0x0040
#define MDELTA_FCREATE 0x0080
#define MDELTA_FDELETE 0x0100
#define MDELTA_FDATA 0x0200      // also used for per-face-data channel
#define MDELTA_NUMBERS 0x0400
#define MDELTA_FSMOOTH 0x0800
#define MDELTA_ALL 0xffff

/*! \sa  Class Mesh, Class Point3.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class represents the notion of a mesh edit vertex move. The public data
members provide the index of the vertex moved as well as the amount of the move
in X, Y, Z.
\par Data Members:
<b>DWORD vid;</b>\n\n
The id of the vertex moved.\n\n
<b>Point3 dv;</b>\n\n
The amount of the move.  */
class VertMove: public MaxHeapOperators {
public:
   DWORD vid;
   Point3 dv;

   /*! \remarks Constructor. */
   VertMove () {}
   /*! \remarks Constructor. The data members are initialized to the values
   passed. */
   VertMove (DWORD i, Point3 p) { vid=i; dv=p; }
   /*! \remarks Destructor. */
	~VertMove () {}
	/*! \remarks Assignment operator. */
   VertMove & operator= (const VertMove & from) { vid=from.vid; dv=from.dv; return (*this); }
};

/*! \sa  Class Mesh, Class Point3.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class represents the notion of a mesh edit UVW vertex assignment. The
public data members provide the index of the vertex as well as the UVWVert.
\par Data Members:
<b>DWORD vid;</b>\n\n
The index of the vertex.\n\n
<b>UVVert v;</b>\n\n
The UVW vertext.  */
class UVVertSet: public MaxHeapOperators {
public:
   DWORD vid;
   UVVert v;

   /*! \remarks Constructor. */
   UVVertSet () {}
   /*! \remarks Constructor. The data members are initalized to the values
   passed. */
   UVVertSet (DWORD i, UVVert p) { vid=i; v=p; }
   /*! \remarks Destructor. */
   ~UVVertSet () {}

	/*! \remarks Assignment operator. */
   UVVertSet & operator= (const UVVertSet & from) { vid=from.vid; v=from.v; return (*this); }
};

/*! \sa  Class MeshDelta , Class Face\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the notion of a face create within a MeshDelta. The
public data members provide the index of any original face the new face may be
based on as well as the new face itself. The virtue of providing the original
face information is that we may use it to track per-face data that isn't
contained within the face itself.\n\n
Note that this class relates closely to the new custom per-face data channels.
For details on the implementations see Class IFaceDataChannel, Class IFaceDataMgr.
\par Data Members:
<b>DWORD original;</b>\n\n
The face in the input mesh this new face should copy properties from. If
UNDEFINED, this face is assumed to be totally original and should not copy
properties from any faces in the input mesh.\n\n
<b>Face face;</b>\n\n
The new face.  */
class FaceCreate: public MaxHeapOperators {
public:
   DWORD original;
   Face face;

	/*! \remarks Constructor.\n\n
	Constructor. Initializes data members to the values passed.*/
   FaceCreate (DWORD f, const Face & fc) : original(f), face(fc) { }
	/*! \remarks Constructor.\n\n
	Constructor. Initializes face to the value passed, and initializes original to
	UNDEFINED.*/
   FaceCreate (const Face & fc) : original(UNDEFINED), face(fc) { }
	/*! \remarks Constructor.\n\n
	Constructor. Initializes original to the value passed. Does not initialize the
	face data member.*/
   FaceCreate (DWORD f) : original(f) { }
	/*! \remarks Constructor. Initializes original to UNDEFINED. Does not
	initialize the face data member.*/
   FaceCreate () : original(UNDEFINED) { }
	/*! \remarks Copy Constructor\n\n
	Constructor. Initializes data members to match those in fc.*/
   FaceCreate (const FaceCreate & fc) : original(fc.original), face(fc.face) { }
   /*! \remarks Assignment operator. Sets all data members to be equal to
   those in fc.
   \par Default Implementation:
   <b>{ original = fc.original; face=fc.face; return *this; }</b> */
   FaceCreate & operator= (const FaceCreate & fc) { original = fc.original; face=fc.face; return *this; }
};

#define FR_V0  1
#define FR_V1  2
#define FR_V2  4
#define FR_ALL 7

/*! \sa  Class Mesh, Class Face,  Class Point3.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class represents the notion of a mesh edit Face Remap, which changes one
or more of the verticies a face uses. It can also alter the visibiliy of the
face's edge, its hidden state and its material ID.
\par Data Members:
<b>DWORD f,</b>\n\n
Face being remapped. This is a zero based index into the Mesh's <b>faces</b>
array.\n\n
<b>DWORD flags</b>\n\n
Specifies which verticies to remap. One or more of the following values:\n\n
<b>FR_V0</b> - Remap the 0th vertex.\n\n
<b>FR_V1</b> - Remap the 1st vertex.\n\n
<b>FR_V2</b> - Remap the 2nd vertex.\n\n
<b>FR_ALL</b> - Remap all the vertices.\n\n
<b>DWORD v[3];</b>\n\n
Array of vertex indicies. These indicate which vertex is used by each specified
corner of the face being remapped.  */
class FaceRemap: public MaxHeapOperators {
public:
   DWORD f, flags, v[3];   // Face being remapped
   /*! \remarks Constructor. The <b>flags</b> and <b>f</b> are set to 0. */
   FaceRemap () { f=flags=0; }
   /*! \remarks Constructor.
   \par Parameters:
   <b>DWORD ff</b>\n\n
   The face to remap.\n\n
   <b>DWORD fl</b>\n\n
   The flags to set.\n\n
   <b>DWORD *vv</b>\n\n
   The array of vertex indicies. */
   DllExport FaceRemap (DWORD ff, DWORD fl, DWORD *vv);
   /*! \remarks Applies the vertex remapping to the given face based on the
   flags of this FaceRemap object.
   \par Parameters:
   <b>Face \&ff</b>\n\n
   The face whose vertices are remapped. */
   DllExport void Apply (Face &ff);
   /*! \remarks Applies the vertex remapping to the given map face based on
   the flags of this FaceRemap object.
   \par Parameters:
   <b>TVFace \&tf</b>\n\n
   The texture face whose tVerts are remapped by the verts of this FaceRemap
   object. The <b>v</b> data member used contains indices into the mesh
   object's <b>tVerts</b> array. */
   DllExport void Apply (TVFace & tf);
   /*! \remarks Assigns the flags and verts of this FaceRemap object to the
   FaceRemap passed.
   \par Parameters:
   <b>FaceRemap \&fr</b>\n\n
   The FaceRemap whose flags and verts are assigned. */
   DllExport void Apply (FaceRemap & fr);
   /*! \remarks Returns a new Face with the FaceRemap applied. */
   Face operator* (Face &ff) { Face nf=ff; Apply(nf); return nf; }
   /*! \remarks Returns a new TVFace with the FaceRemap applied. */
   TVFace operator* (TVFace & ff) { TVFace nf=ff; Apply(nf); return nf; }
};

// Attribute changes available for faces:
#define ATTRIB_EDGE_A      (1<<0)
#define ATTRIB_EDGE_B      (1<<1)
#define ATTRIB_EDGE_C      (1<<2)
#define ATTRIB_EDGE_ALL  7
#define ATTRIB_HIDE_FACE      (1<<3)
#define ATTRIB_MATID    (1<<4)

// Mat ID takes bits 5-21
#define ATTRIB_MATID_SHIFT 5
#define ATTRIB_MATID_MASK  0xffff

/*! \sa  Class Mesh, Class Face,  Class Point3.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class represents the notion of a mesh edit Face Change, which changes the
visibiliy of the face's edges, its hidden state and/or its material ID.
\par Data Members:
<b>DWORD f</b>\n\n
The face to change. This is a zero based index into the Mesh's <b>faces</b>
array.\n\n
<b>DWORD flags</b>\n\n
Specifies which attributes to change. One or more of the following values:\n\n
<b>ATTRIB_EDGE_A</b> - Alter edge between v0 and v1\n\n
<b>ATTRIB_EDGE_B</b> - Alter edge between v1 and v2\n\n
<b>ATTRIB_EDGE_C</b> - Alter edge between v2 and v0\n\n
<b>ATTRIB_EDGE_ALL</b> - Alter all the edges.\n\n
<b>ATTRIB_HIDE_FACE</b> - Alter the face hidden state.\n\n
<b>ATTRIB_MATID</b> - Alter the material ID.\n\n
<b>DWORD val;</b>\n\n
The value containing the face change information. Bits 0, 1, 2 hold the edge
visibility, bit 3 holds the hidden state, and bits 5-21 hold the material ID.
 */
class FaceChange: public MaxHeapOperators {
public:
   DWORD f, flags, val;
   /*! \remarks Constructor. The <b>flags</b> and <b>f</b> are set to 0. */
   FaceChange () { f=flags=0; }
   /*! \remarks Constructor. The data members are initialized to the values
   passed.
   \par Parameters:
   <b>DWORD ff</b>\n\n
   The index of the face to change. This is a zero based index into the Mesh's
   <b>faces</b> array.\n\n
   <b>DWORD fl</b>\n\n
   The flags to set.\n\n
   <b>DWORD v</b>\n\n
   The value to set. */
   FaceChange (DWORD ff, DWORD fl, DWORD v) { f=ff; flags=fl; val=v; }
   /*! \remarks Applies the face change to the Face passed using the flags of
   this FaceChange object to contol what's altered.
   \par Parameters:
   <b>Face \&ff</b>\n\n
   The face to change. */
   DllExport void Apply (Face &ff);
   /*! \remarks Updates the flags and val to the FaceChange passed using this
   FaceChange and its flags.
   \par Parameters:
   <b>FaceChange \&fa</b>\n\n
   The FaceChange object to alter. */
   DllExport void Apply (FaceChange & fa);
};

/*! \sa  Class Mesh, Class Face,  Class Point3.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class represents the notion of the edit mesh Face Smooth operation. This
updates the smoothing group infomation in the face.
\par Data Members:
<b>DWORD f</b>\n\n
The face to change. This is a zero based index into the Mesh's <b>faces</b>
array.\n\n
<b>DWORD mask</b>\n\n
The mask into the face of the smoothing groups.\n\n
<b>DWORD val;</b>\n\n
The smoothing group information to store.  */
class FaceSmooth: public MaxHeapOperators {
public:
   DWORD f, mask, val;
   /*! \remarks Constructor. The <b>flags</b> and <b>f</b> are set to 0. */
   FaceSmooth () { f=mask=0; }
   /*! \remarks Constructor. The data members are initalized to the values
   passed. */
   FaceSmooth (DWORD ff, DWORD mk, DWORD vl) { f=ff; mask=mk; val=vl; }
   /*! \remarks Applies this smoothing change to the given face.
   \par Parameters:
   <b>Face \&ff</b>\n\n
   The face to update. */
   DllExport void Apply (Face &ff);
   /*! \remarks Assigns the flags and val to the given FaceSmooth object from
   this one.
   \par Parameters:
   <b>FaceSmooth \&fs</b>\n\n
   The FaceSmooth object to alter. */
   DllExport void Apply (FaceSmooth & fs);
};

//STEVE: someday support applying a standard mapping to selected faces?
/*! \sa  Class Mesh, Class MeshDelta, Template Class Tab.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is an SDK class that represents some kind of change to a mesh map. This
"delta" can include changes in map vertices and/or faces. It is always a
subordinate part of a MeshDelta. Most of the time, the programmer does not need
to worry about this class directly, but can let the parent MeshDelta do most of
the work.\n\n
Note: You must <b>#include "MESHDLIB.H"</b> to use this class as it's not
included by default by <b>MAX.H</b>.
\par Method Groups:
See <a href="class_map_delta_groups.html">Method Groups for Class MapDelta</a>.
\par Data Members:
<b>DWORD vnum</b>\n\n
The expected number of vertices in the input mesh\n\n
<b>DWORD fnum;</b>\n\n
The expected number of faces in the input mesh\n\n
<b>Tab\<UVVertSet\> vSet;</b>\n\n
This data member stores changes in the mapping vertices given as input. See
class UVVertSet for more information. UVVertSets are stored in original map
vertex ID order, and there is never more than one UVVertSet per original map
vertex.\n\n
<b>Tab\<Point3\> vCreate;</b>\n\n
This data member stores mapping vertices newly created as part of the MapDelta.
These are stored in the order created.\n\n
<b>Tab\<TVFace\> fCreate;</b>\n\n
This data member stores map faces newly created as part of the MapDelta. These
are stored in the order created.\n\n
<b>Tab\<FaceRemap\> fRemap;</b>\n\n
This data member stores changes in which map vertices are used by existing map
faces. See Class FaceRemap for more
information. These are stored in original face order, and there is never more
than one per original face.  */
class MapDelta: public MaxHeapOperators {
public:
   DWORD vnum, fnum;
   Tab<UVVertSet> vSet;
   Tab<Point3> vCreate;
   Tab<TVFace> fCreate; // New texture vert faces -- matches master MeshDelta fCreate in size.
   Tab<FaceRemap> fRemap;  // ordered list of faces using at least one new vertex.

   /*! \remarks Constructor. Initializes the MapDelta with empty tables and
   0's for input mesh map size. */
   MapDelta () { vnum=0; fnum=0; }
	/*! \remarks Clears out all existing map changes. Zeroes all the vCreate,
	vSet, etc tables. */
   DllExport void ClearAllOps ();

   // Bookkeeping:
	/*! \remarks Returns the number of map vertex sets that would be applied to a
	map with the specified number of map vertices. (If that number equals this
	MapDelta's vnum, this is simply vSet.Count().)
	\par Parameters:
	<b>DWORD inVNum</b>\n\n
	The number of vertices in the input map we're inquiring about. */
   DllExport int NumVSet (DWORD inVNum);
   /*! \remarks Sets the number of map vertices in the input map. NOTE that if
   n is less than the current vnum, the data relating to the extra vertices
   will be lost. (That is, if one of your map vertex sets is applied to map
   vertex 32, and you SetInVNum to 30, that vertex set will be lost, and will
   not be recovered if you later SetInVNum to 35.) It is NOT necessary to call
   this method before applying this MapDelta to a smaller than expected Map.
   \par Parameters:
   <b>DWORD n</b>\n\n
   The number of vertices expected from the input mesh. */
   DllExport void SetInVNum (DWORD n);
   /*! \remarks Sets the number of map faces in the input map. NOTE that if n
   is less than the current fnum, the data relating to the extra faces will be
   lost. (That is, if one of your face remaps is applied to face 32, and you
   SetInFNum to 30, that face remap will be lost, and will not be recovered if
   you later SetInFNum to 35.) It is NOT necessary to call this method before
   applying this MapDelta to a smaller than expected Map.
   \par Parameters:
   <b>DWORD n</b>\n\n
   The number of faces expected from the input mesh. */
   DllExport void SetInFNum (DWORD n);
   /*! \remarks Returns the number of vertices in the output map, assuming
   that the input map is of the expected (vnum) size. */
   DWORD outVNum () { return vnum + vCreate.Count(); }
	/*! \remarks Returns the number of vertices in the output map, assuming that
	the input map has the specified number of map vertices.
	\par Parameters:
	<b>DWORD inVNum</b>\n\n
	The number of map vertices expected in the input map. */
   DWORD outVNum (DWORD inVNum) { return inVNum + vCreate.Count(); }
   /*! \remarks Indicates whether the specified map vertex was created in this
   MapDelta.
   \par Parameters:
   <b>DWORD i</b>\n\n
   The index in the output of the map vertex. */
   bool IsCreate (DWORD i) { int j=i-vnum; return ((j>=0) && (j<vCreate.Count())); }
   /*! \remarks Obtains the index of the vSet entry that relates to this
   vertex.
   \par Parameters:
   <b>DWORD i</b>\n\n
   The map vertex index.
   \return  The index in the vSet array of the map vertex set corresponding to
   this vertex, or UNDEFINED if this map vertex has not been modified. */
   DllExport DWORD SetID (DWORD i);

   // Topological ops:
   /*! \remarks Creates new map vertices.
   \par Parameters:
   <b>UVVert *v</b>\n\n
   A pointer to an array of UVVerts representing the new map vertices.\n\n
   <b>int num</b>\n\n
   The size of the UVVert array.
   \return  The index (in the output map) of the first of these new map
   vertices. */
   DllExport DWORD VCreate (UVVert *v, int num=1);
   /*! \remarks Creates new map faces.\n\n
   Note: MapDeltas must be kept up to date with the parent MeshDelta in all new
   face creations. See the MeshDelta method CreateDefaultMapFaces for details.
   \par Parameters:
   <b>TVFace *f</b>\n\n
   A pointer to an array of map faces to be added to the MapDelta.\n\n
   <b>int num</b>\n\n
   The size of the map face array. */
   DllExport void FCreate (TVFace *f, int num=1);
   /*! \remarks Creates new "default" map faces, where all the corners are
   UNDEFINED. (These are later filled in by a call to the parent MeshDelta's
   FillInFaces method.)\n\n
   Note: MapDeltas must be kept up to date with the parent MeshDelta in all new
   face creations. See the MeshDelta method CreateDefaultMapFaces for details.
   \par Parameters:
   <b>int num</b>\n\n
   The number of default faces to create. */
   DllExport void FCreateDefault (int num=1);
   /*! \remarks Creates 2 new faces, forming a quad.\n\n
   Note: MapDeltas must be kept up to date with the parent MeshDelta in all new
   face creations. See the MeshDelta method CreateDefaultMapFaces for details.
   \par Parameters:
   <b>DWORD *t</b>\n\n
   A pointer to an array of 4 map vertices to be used as corners of the quad.
   */
   DllExport void FCreateQuad (DWORD *t);
	/*! \remarks Creates a new map face by copying (and optionally remapping) the
	face given.\n\n
	Note: MapDeltas must be kept up to date with the parent MeshDelta in all new
	face creations. See the MeshDelta method CreateDefaultMapFaces for details.
	\par Parameters:
	<b>TVFace \& tf</b>\n\n
	The map face we wish to clone. (This is typically generated by the OutFace
	method.)\n\n
	<b>DWORD remapFlags=0</b>\n\n
	<b>DWORD *v=NULL</b>\n\n
	If we wish to remap any of the corners of this map face while cloning, the
	appropriate flags and vertices should be passed in these last two arguments. v
	should point to an array of 3 map vertex indices, although the ones not marked
	as used by the remapFlags need not be set to anything in particular. See class
	FaceRemap for more information about face remapping. */
   DllExport void FClone (TVFace & tf, DWORD remapFlags=0, DWORD *v=NULL);
   /*! \remarks Adds a face remap to this MapDelta. If the face specified
   already has a remap record, the two are combined. If the face specified is a
   face created by this MapDelta, the remap is applied directly to the fCreate
   entry instead of being stored in fRemap.
   \par Parameters:
   <b>DWORD f</b>\n\n
   The face to remap.\n\n
   <b>DWORD flags</b>\n\n
   Face Remap flags - these indicate which vertices should be remapped. The
   possibilities are FR_V0 (1), FR_V1 (2), and FR_V2 (4). (See class FaceRemap
   for more information.)\n\n
   <b>DWORD *v</b>\n\n
   A pointer to the vertices to remap the face to use. Only the positions
   indicated in the remap flags need contain meaningful data.\n\n
   Note that the vertices indicated here must be indexed by their positions
   after all of the current MeshDelta's creates and clones, but before any
   vertex deletes - essentially input-based indexing. Vertex index values of 0
   through vnum-1 are considered to be the original mesh's vertices; values of
   vnum through vnum+vCreate.Count()-1 are considered to be this MeshDelta's
   newly created vertices; and values above this are cloned vertices. */
   DllExport void FRemap (DWORD f, DWORD flags, DWORD *v);  // Creates a (or modifies an existing) remap record.
   /*! \remarks Adds a face remap to this MapDelta. If the face specified in
   the FaceRemap already has a remap record, the two are combined. If the face
   specified is a face created by this MapDelta, the remap is applied directly
   to the fCreate entry instead of being stored in fRemap.
   \par Parameters:
   <b>FaceRemap \& fr</b>\n\n
   A FaceRemap that should be appended to this MapDelta. */
   void FRemap (FaceRemap & fr) { FRemap (fr.f, fr.flags, fr.v); }
   /*! \remarks Obtains the index of the fRemap entry that relates to this
   face.
   \par Parameters:
   <b>DWORD ff</b>\n\n
   The map face index.
   \return  If there is such an entry, the index is returned, so
   fRemap[RemapID(ff)].fid == ff. If there is no remap record for this map
   face, the method returns UNDEFINED. */
   DllExport DWORD RemapID (DWORD ff);
   /*! \remarks Tells whether the specified corner of the specified face has
   been remapped in this MapDelta.
   \par Parameters:
   <b>DWORD ff</b>\n\n
   The map face index.\n\n
   <b>DWORD vid</b>\n\n
   The corner of the face - 0, 1, or 2.
   \return  If this corner has been remapped, it returns the vertex it's been
   remapped to. Otherwise, it returns UNDEFINED. */
   DllExport DWORD IsRemapped (DWORD ff, DWORD vid);
   /*! \remarks Returns the specified map face as it would appear in the
   MapDelta output, taking into account any remaps.
   \par Parameters:
   <b>TVFace *mf</b>\n\n
   The input map face array.\n\n
   <b>DWORD f</b>\n\n
   The index of the face you want the output version of. */
   DllExport TVFace OutFace (TVFace *mf, DWORD ff);
   /*! \remarks Deletes the specified faces. This only affects the fCreate
   array, and should generally only be called by the parent MeshDelta's FDelete
   method to keep the face create arrays in sync.
   \par Parameters:
   <b>int offset</b>\n\n
   Indicates what position in the fdel array corresponds to the first created
   face. (This is necessary since the BitArrays handed to MeshDelta::FDelete
   generally are based on the indexing after the previous MeshDelta::fDelete is
   applied. So this value is less than fnum if there was some previous deletion
   of original faces in the MeshDelta.)\n\n
   <b>BitArray \& fdel</b>\n\n
   The faces to delete. The faces are indexed by their output mesh positions.
   */
   DllExport void FDelete (int offset, BitArray & fdel);

   // Geometric ops:
   /*! \remarks Sets an existing map vertex to the value given. (Note that if
   the same map vertex is set twice, the new set simply replaces the old one -
   there is never more than one UVVertSet in the vSet array for a single input
   map vertex.)
   \par Parameters:
   <b>int i</b>\n\n
   The index of the map vertex to set.\n\n
   <b>const UVVert \& p</b>\n\n
   The value to set the map vertex to. */
   DllExport void Set (DWORD i, const UVVert & p);
   DllExport void Set (BitArray & sel, const UVVert & p);

   // Uses:
   /*! \remarks Equality operator - makes this MapDelta just like the one
   given. */
   MapDelta & operator=(MapDelta & from) { CopyMDChannels (from, MDELTA_ALL); return *this; }
   /*! \remarks Appends the given MapDelta to the current one.
   \par Parameters:
   <b>MapDelta \& td</b>\n\n
   The MapDelta to append. This MapDelta may be modified to make it suitable,
   ie the vnum and fnum values will be set to the expected output of the
   current MapDelta if they don't already match. (This may result in the loss
   of some data - see "SetInVNum" and "SetInFNum" for more information.) */
   DllExport MapDelta & operator*=(MapDelta & from);
   /*! \remarks Changes the given map by this MapDelta, in the following
   manner:\n\n
   First, any maps that are supported by the MeshDelta but not by the mesh are
   assigned to the mesh in their default form. (Vertex colors are white, other
   maps are copies of the mesh vertices, and all have the same topology as the
   mesh.)\n\n
   Next, any UNDEFINED mapping verts in the MeshDelta are filled in by
   FillInFaces.\n\n
   Then the new vertices are added, creates first, followed by clones. The
   original vertices are then moved.\n\n
   The faces are then modified, by applying all the FaceRemaps, FaceChanges,
   and FaceSmooths to the appropriate faces. New faces (in fCreate) are
   appended to the end of the face list.\n\n
   Map changes are applied to all active maps, and map channels not supported
   by this MeshDelta are removed.\n\n
   After all that is done, the vertices and faces marked in the vDelete and
   fDelete arrays are deleted.\n\n
   Finally, the vertex data, vertex hide, and selections kept in the MeshDelta
   are applied to the result.
   \par Parameters:
   <b>UVVert *tv</b>\n\n
   The map vertex array to change. This should be allocated to handle all the
   new map vertices in the vCreate array.\n\n
   <b>TVFace *tf</b>\n\n
   The map face array to change. This should be allocated to handle all the new
   map faces in the fCreate array.\n\n
   <b>DWORD inVNum</b>\n\n
   The actual number of map vertices (which doesn't have to match this
   MapDelta's vnum) in the input map.\n\n
   <b>DWORD inFNum</b>\n\n
   The actual number of map faces (which doesn't have to match this MapDelta's
   fnum) in the input map. */
   DllExport void Apply (UVVert *tv, TVFace *tf, DWORD inVNum, DWORD inFNum);

   // Handy debugging output
   /*! \remarks Prints out all the changes in this MapDelta to the DebugPrint
   window in Developer Studio. */
   DllExport void MyDebugPrint ();

   // Backup stuff:
   /*! \remarks Indicates what parts of a MapDelta could be changed if this
   MapDelta were appended to it. This is useful when backing up the MapDelta
   for Restore Objects. For instance, if you had a MapDelta with lots of face
   remaps, and you wanted to compose it with one that only added map vertices,
   there would be no reason to back up the remaps for an undo.
   \return  Returns some combination of the following flags, corresponding to
   the data members that would be changed:\n\n
   <b>MDELTA_VMOVE</b>: Indicates that the vSet array will be altered by this
   MapDelta.\n\n
   <b>MDELTA_VCREATE</b>: Indicates that the vCreate array will be altered by
   this MapDelta.\n\n
   <b>MDELTA_FREMAP</b>: Indicates that the fRemap array will be altered by
   this MapDelta.\n\n
   <b>MDELTA_FCREATE</b>: Indicates that the fCreate array will be altered by
   this MapDelta. */
   DllExport DWORD ChangeFlags ();
   /*! \remarks Copies the specified parts of the MapDelta. (Useful in
   combination with ChangeFlags to create efficient Restore objects.)
   \par Parameters:
   <b>MapDelta \& from</b>\n\n
   The MapDelta to copy into this.\n\n
   <b>DWORD channels</b>\n\n
   Indicates the parts to copy - some combination of the following flags:\n\n
   <b>MDELTA_VMOVE</b>: Copy the vSet array.\n\n
   <b>MDELTA_VCREATE</b>: Copy the vCreate array.\n\n
   <b>MDELTA_FREMAP</b>: Copy the fRemap array.\n\n
   <b>MDELTA_FCREATE</b>: Copy the fCreate array. */
   DllExport void CopyMDChannels (MapDelta & from, DWORD channels);

   // Double-checking routines, good for after loading.
   // Return TRUE if already correct, FALSE if they had to make a correction.
   DllExport BOOL CheckOrder (); // Checks for out of order sets or remaps
   DllExport BOOL CheckFaces (); // Checks remaps & fCreates for out of bound map vert id's.
};

/*! \sa  Class Mesh, Class PerData, Class MeshDelta.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
VDataDelta is a way for a MeshDelta to keep track of per-vertex information.
Like vertex selection, this information is explicitly set after all topological
changes performed by the MeshDelta, so any active vertex data channels are
always set to the size <b>outVNum()</b> for the related <b>MeshDelta</b>.\n\n
An example of this being used is in Edit Mesh, where you can assign or modify
vertex weights. All these changes and assignments are stored in the Edit Mesh's
main MeshDelta.
\par Data Members:
<b>PerData *out;</b>\n\n
This is where the output vertex data is kept, if the given vertex data channel
has been activated.  */
class VDataDelta: public MaxHeapOperators {
public:
   PerData *out;

   /*! \remarks Constructor. The data member <b>out</b> is set to NULL. */
   VDataDelta () { out=NULL; }
   /*! \remarks Destructor. If <b>out</b> is allocated it is deleted. */
   DllExport ~VDataDelta ();
   /*! \remarks Sets the size of the output vertex data to the value
   specified.
   \par Parameters:
   <b>int nv</b>\n\n
   The number of elements to allocate.\n\n
   <b>BOOL keep=FALSE</b>\n\n
   If TRUE previous values are kept (copied to the new storage); otherwise they
   are discarded. */
   void SetVNum (int nv, BOOL keep=FALSE) { if (out) out->SetCount(nv,keep); }
   /*! \remarks If the output vertex data has not been allocated this method
   allocates it with the specified number of elements.
   \par Parameters:
   <b>int vnum</b>\n\n
   The number of elements to allocate.\n\n
   <b>int vdID</b>\n\n
   The channel to allocate. One of the following values:\n\n
   <b>VDATA_SELECT</b>\n\n
   <b>VDATA_WEIGHT</b> */
   DllExport void Activate (int vnum, int vdID);
   /*! \remarks Sets the specified number of elements of vertex data at the
   location passed.
   \par Parameters:
   <b>int where</b>\n\n
   The zero based index of the destination in the <b>out</b> array.\n\n
   <b>void *data</b>\n\n
   The source data.\n\n
   <b>int num=1</b>\n\n
   The number of elements to set. */
   DllExport void Set (int where, void *data, int num=1);
};

#pragma warning(push)
#pragma warning(disable:4100)

/*! \sa  Class Mesh, Class FaceChange, Class FaceRemap, Class FaceSmooth, Class VertMove, Class UVVertSet, Class MapDelta, Class VDataDelta, Class AdjEdgeList, Class AdjFaceList, Class MeshChamferData, Template Class Tab, Class BitArray.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class represents the notion of a mesh edit.\n\n
This is an SDK class that represent some kind of change to a mesh. This "delta"
can include topological, geometric, map, and/or selection changes. Most
standard mesh "edits" available in the Editable Mesh or Edit Mesh interface are
available through the MeshDelta SDK, giving developers a powerful way to
manipulate meshes while not having to "sweat the details" of maintaining maps
to match the mesh changes, updating edge selections, etc.\n\n
The MeshDelta members and methods make use of a number of mesh-related classes, including Class FaceChange, Class FaceRemap, Class FaceSmooth, Class VertMove, Class UVVertSet, Class MapDelta, Class VDataDelta, Class AdjEdgeList, Class AdjFaceList, Class MeshChamferData.\n\n
While we often talk about the characteristics of the "input mesh" that a
MeshDelta is based on, all MeshDeltas should be able to cope with any mesh.\n\n
Note: You must <b>#include "MESHDLIB.H"</b> to use this class as it's not
included by default by <b>MAX.H</b>.
\par Method Groups:
See <a href="class_mesh_delta_groups.html">Method Groups for Class MeshDelta</a>.
\par Data Members:
<b>DWORD vnum</b>\n\n
The expected number of vertices in the input mesh.\n\n
<b>DWORD fnum;</b>\n\n
The expected number of faces in the input mesh.\n\n
<b>Tab\<VertMove\> vMove;</b>\n\n
This data member stores movements of input vertices. Each VertMove consists of
a vertex ID indicating which vertex should be moved and a Point3 offset in
object space. VertMoves are stored in vertex ID order, and there is never more
than one VertMove per original vertex.\n\n
<b>Tab\<VertMove\> vClone;</b>\n\n
If the vClone[i] record has a vid of UNDEFINED, it's considered a "create", and
the coordinates of vClone[i].p are considered to be in object space. If
vClone[i].vid is not undefined, it's the index of a vertex "original" in the
input mesh, and vClone[i].p is treated as an offset from that vertex. If the
vertex is not present in the input mesh, i.e. vClone[i].vid \>= mesh::numVerts,
the clone will not be created in the output. All creates and clones are stored
in the order created..\n\n
<b>BitArray vDelete;</b>\n\n
This data member stores deletions of vertices in the input mesh. vDelete's size
is vnum.\n\n
<b>Tab\<FaceCreate\> fCreate;</b>\n\n
This data member stores faces newly created as part of the MeshDelta. These are
stored in the order created.\n\n
<b>Tab\<FaceRemap\> fRemap;</b>\n\n
This data member stores changes in which vertices are used by existing faces.
See class FaceRemap for more information. These are stored in original face
order, and there is never more than one per original face.\n\n
<b>Tab\<FaceChange\> fChange;</b>\n\n
This data member stores changes in input face characteristics, such as material
ID, edge visibility, and face hiding. See class FaceChange for more
information. These are stored in original face order, and there is never more
than one per original face.\n\n
<b>Tab\<FaceSmooth\> fSmooth;</b>\n\n
This data member stores changes in input face smoothing groups. See class
FaceSmooth for more information. These are stored in original face order, and
there is never more than one per original face.\n\n
<b>BitArray fDelete;</b>\n\n
This data member stores deletions of faces in the input mesh. The size of this
BitArray is fnum.\n\n
<b>BitArray vsel;</b>\n\n
This data member stores the vertex selection of the output mesh.\n\n
<b>BitArray esel;</b>\n\n
This data member stores the edge selection of the output mesh. As with class
Mesh's edgeSel data member, this information is indexed by side of face:
esel[ff*3+k] is the edge selection for face ff, side k.\n\n
<b>BitArray fsel;</b>\n\n
This data member stores the face selection of the output mesh.\n\n
<b>BitArray vhide;</b>\n\n
This data member stores the vertex hide information of the output mesh.\n\n
<b>MapDelta *map;</b>\n\n
Points to an array of MapDeltas which maintain any relevant changes to the
various map channels. The size of this array is always equal to
mapSupport.GetSize().\n\n
<b>BitArray mapSupport;</b>\n\n
Indicates which maps are supported by this MeshDelta.\n\n
<b>VDataDelta *vd;</b>\n\n
Points to an array of VDataDeltas which maintain any relevant changes to the
various vertex data channels. The size of this array is always equal to
vdSupport.GetSize().\n\n
<b>BitArray vdSupport;</b>\n\n
Indicates which vertex data channels are supported by this MeshDelta.  */
class MeshDelta : public BaseInterfaceServer {
   DWORD *vlut, *flut;
   int vlutSize, flutSize;

   MapDelta hmap[NUM_HIDDENMAPS];
   BitArray hmapSupport;

   // Internal methods:
   // Parts of Cut:
   DWORD FindBestNextFace (Mesh & m, Tab<DWORD> *vfac, Point3 *cpv, DWORD startV, Point3 & svP);
   DWORD FindOtherFace (DWORD ff, Tab<DWORD> * vfa, Tab<DWORD> * vfb);
public:
   DWORD vnum, fnum;

   Tab<VertMove> vMove; // Ordered list of moves to existing verts
   //Tab<Point3> vCreate;  // DO NOT USE!!!!!  Use of this data member was eliminated in 4.0
   Tab<VertMove> vClone;   // Creation-order list of cloned points.
   BitArray vDelete;

   Tab<FaceCreate> fCreate;         // New faces
   Tab<FaceRemap> fRemap;  // existing faces using new verts.  (Ordered list.)
   Tab<FaceChange> fChange;   // ordered list of face flag changes
   Tab<FaceSmooth> fSmooth;   // ordered list of face smoothing group changes
   BitArray fDelete; // Also applies to map faces.

   BitArray vsel, esel, fsel, vhide;   // Based on output mesh indexing.

   MapDelta *map;
   BitArray mapSupport;

   VDataDelta *vd;   // Based on output mesh indexing.
   BitArray vdSupport;

   /*! \remarks Constructor. Initializes the MeshDelta with NULL pointers and
   0's for input mesh size. */
   DllExport MeshDelta ();
   /*! \remarks Constructor. Initializes the MeshDelta to be based on the mesh
   given. MapDeltas and VDataDeltas are allocated as appropriate, and vnum and
   fnum are set. */
   DllExport MeshDelta (const Mesh & m);
   /*! \remarks Destructor. Frees all allocated memory, including the
   MapDeltas. */
   DllExport ~MeshDelta ();

   /*! \remarks Initializes the MeshDelta to the mesh given, setting map and
   vdata support as appropriate. Does NOT clear out existing changes.
   \par Parameters:
   <b>const Mesh \&m</b>\n\n
   The mesh to init from. */
   DllExport void InitToMesh (const Mesh & m);
   /*! \remarks Clears out all existing mesh changes. Zeroes all the vCreate,
   vMove, etc arrays, as well as those in the active MapDeltas. Does not clear
   memory. */
   DllExport void ClearAllOps ();
   /*! \remarks Sets the number of map channels used by the MeshDelta -
   allocates the "map" array.
   \par Parameters:
   <b>int num</b>\n\n
   The number of maps to allocate.\n\n
   <b>bool keep=TRUE</b>\n\n
   If TRUE any previous maps are kept; otherwise they are discarded. */
   DllExport void SetMapNum (int n, bool keep=TRUE);
   /*! \remarks Gets the number of map channels in the MeshDelta - equivalent
   to mapSupport.GetSize(). */
   int GetMapNum () { return mapSupport.GetSize(); }
   /*! \remarks Data accessor - gets the MapDelta for the specified map
   channel. Since in 4.0 we now have "hidden map channels" which are accessed
   by negative indices (-1 for MAP_SHADING, for example), data accessor methods
   like this one should be used instead of the actual arrays. (Hidden map
   channels are stored in a new private data member, not as part of the public
   map array.) */
   MapDelta & Map(int mp) { return (mp<0) ? hmap[-1-mp] : map[mp]; }
   /*! \remarks Indicates whether the specified map channel is supported by
   this MeshDelta. Since in 4.0 we now have "hidden map channels" which are
   accessed by negative indices (-1 for MAP_SHADING, for example), data
   accessor methods like this one should be used instead of the actual data
   members. (Hidden map channel support information is stored in a new private
   data member, not as part of the public mapSupport BitArray.) */
   bool getMapSupport (int mp) { return ((mp<0) ? hmapSupport[-1-mp] : mapSupport[mp]) ? true : false; }
   /*! \remarks Sets map support in this MeshDelta for the specified map
   channel. Since in 4.0 we now have "hidden map channels" which are accessed
   by negative indices (-1 for MAP_SHADING, for example), data accessor methods
   like this one should be used instead of the actual data members. (Hidden map
   channel support information is stored in a new private data member, not as
   part of the public mapSupport BitArray.) */
   void setMapSupport (int mp, bool val=true) { if (mp<0) hmapSupport.Set(-1-mp, val); else mapSupport.Set(mp, val); }
   /*! \remarks Indicates whether any map channel is supported by this
   MeshDelta. */
   bool hasMapSupport () { return !(mapSupport.IsEmpty() && hmapSupport.IsEmpty()); }
   /*! \remarks Sets the number of vertex data channels used by the MeshDelta
   - allocates the "vd" array.
   \par Parameters:
   <b>int num</b>\n\n
   The number of vertex data channels to allocate.\n\n
   <b>bool keep=TRUE</b>\n\n
   If TRUE any previous vertex data channels are kept; otherwise they are
   discarded. */
   DllExport void SetVDataNum (int size, bool keep=TRUE);
   /*! \remarks Gets the number of vertex data channels in the MeshDelta -
   equivalent to vdSupport.GetSize(). */
   int GetVDataNum () { return vdSupport.GetSize(); }
   /*! \remarks Indicates what data channels of a mesh would be changed by
   this MeshDelta. For instance, a MeshDelta with vertex moves but no other
   changes would return PART_GEOM|PART_SELECT. PART_GEOM represents the moves,
   and PART_SELECT represents the fact that MeshDeltas always overwrite
   selection info. Most of the changes in a MeshDelta will alter PART_TOPO.
   PART_VERTCOLOR and PART_TEXMAP may also be returned.\n\n
   This is especially useful for knowing what parts of a mesh to back up in a
   restore object for an undo/redo. (See the SDK implementation of Editable
   Mesh for an example of this.) Also, it can be used for invalidating
   temporary data, as in both Edit and Editable Mesh. */
   DllExport DWORD PartsChanged ();

   // The main work of a MeshDelta.
   /*! \remarks Changes the given mesh by this MeshDelta, in the following
   manner:\n\n
   First, any maps that are supported by the MeshDelta but not by the mesh are
   assigned to the mesh in their default form. (Vertex color channels are
   white, and other maps are copies of the mesh vertices. All have the same
   topology as the mesh.)\n\n
   Next, any UNDEFINED mapping verts in the MeshDelta are filled in by
   FillInFaces.\n\n
   Then the new vertices are added, creates first, followed by clones. The
   original vertices are then moved.\n\n
   The faces are then modified, by applying all the FaceRemaps, FaceChanges,
   and FaceSmooths to the appropriate faces. New faces (in fCreate) are
   appended to the end of the face list.\n\n
   Map changes are applied to all active maps, and map channels not supported
   by this MeshDelta are removed.\n\n
   After all that is done, the vertices and faces marked in the vDelete and
   fDelete arrays are deleted.\n\n
   Finally, the vertex data, vertex hide, and selections kept in the MeshDelta
   are applied to the result. */
   DllExport void Apply(Mesh& mesh);
   /*! \remarks Assignment operator - makes this MeshDelta just like the one
   given. */
   MeshDelta& operator=(MeshDelta& td) { CopyMDChannels (td, MDELTA_ALL); return *this; }
   /*! \remarks Appends the given MeshDelta to the current one.
   \par Parameters:
   <b>MeshDelta \& td</b>\n\n
   The MeshDelta to append. This MeshDelta may be modified to make it suitable,
   ie the vnum and fnum values will be set to the expected output of the
   current MeshDelta if they don't already match. (This may result in the loss
   of some data - see "SetInVNum" and "SetInFNum" for more information.) */
   MeshDelta& operator*=(MeshDelta& td) { Compose(td); return *this; }
	/*! \remarks Appends the given MeshDelta to the current one.
	\par Parameters:
	<b>MeshDelta \& td</b>\n\n
	The MeshDelta to append. This MeshDelta may be modified to make it suitable, ie
	the vnum and fnum values will be set to the expected output of the current
	MeshDelta if they don't already match. (This may result in the loss of some
	data - see "SetInVNum" and "SetInFNum" for more information.) */
   DllExport void Compose (MeshDelta & td);

   // Following give numbers of clones or deletions, given the input numbers.
   // (We can't delete vertex 10 on an 8-vert mesh; this counts the number of valid entries.)
   /*! \remarks Returns the number of vertex moves that would be applied to a
   mesh with the specified number of vertices. If that number equals this
   MeshDelta's vnum, this is simply vMove.Count().
   \par Parameters:
   <b>DWORD inVNum</b>\n\n
   The number of vertices in the input mesh we're inquiring about. */
   DllExport DWORD NumVMove (DWORD inVNum);
   /*! \remarks Returns the number of vertex clones \& creates that would be
   applied to a mesh with the specified number of vertices. If that number
   equals this MeshDelta's vnum, this is simply vClone.Count(). If, however,
   inVNum is lower than the expected vnum, some of the clones might be
   eliminated, reducing this number.
   \par Parameters:
   <b>DWORD inVNum</b>\n\n
   The number of vertices in the input mesh we're inquiring about. */
   DllExport DWORD NumVClone (DWORD inVNum);
   /*! \remarks Returns the number of vertex deletes that would be applied to
   a mesh with the specified number of vertices. If that number equals this
   MeshDelta's vnum, this is simply vDelete.NumberSet ().
   \par Parameters:
   <b>DWORD inVNum</b>\n\n
   The number of vertices in the input mesh we're inquiring about. */
   DllExport DWORD NumVDelete (DWORD inVNum);
   /*! \remarks Returns the number of face deletes that would be applied to a
   mesh with the specified number of faces. If inFNum equals this MeshDelta's
   fnum, this is simply fDelete.NumberSet ().
   \par Parameters:
   <b>DWORD inFNum</b>\n\n
   The number of faces in the input mesh we're inquiring about. */
   DllExport DWORD NumFDelete (DWORD inFNum);
   /*! \remarks Returns the number of face creates in this MeshDelta. */
   int NumFCreate () { return fCreate.Count(); }

   // Sets the size of the input object -- should be used only in multiplying MeshDeltas,
   // since it destroys records of changes to out-of-range components.
   // MeshDelta may be applied to mesh without using these.
   /*! \remarks Sets the number of faces in the input mesh. NOTE that if nface
   is less than the current fnum, the data relating to the extra faces will be
   lost. (That is, if one of your face remaps is applied to face 32, and you
   SetInFNum to 30, that face remap will be lost, and will not be recovered if
   you later SetInFNum to 35.) It is NOT necessary to call this method before
   applying this MeshDelta to a smaller than expected Mesh.
   \par Parameters:
   <b>int nface</b>\n\n
   The number of faces expected from the input mesh. */
   DllExport void SetInFNum (int nface);
   /*! \remarks Sets the number of vertices in the input mesh. NOTE that if nv
   is less than the current vnum, the data relating to the extra vertices will
   be lost. (That is, if one of your vertex moves is applied to vertex 32, and
   you SetInVNum to 30, that vertex move will be lost, and will not be
   recovered if you later SetInVNum to 35.) It is NOT necessary to call this
   method before applying this MeshDelta to a smaller than expected Mesh.
   \par Parameters:
   <b>int nv</b>\n\n
   The number of vertices expected from the input mesh. */
   DllExport void SetInVNum (int nv);

	/*! \remarks Returns the number of vertices in the output mesh, assuming that
	the input mesh is of the expected (vnum) size. */
   int outVNum () { return vnum + vClone.Count() - vDelete.NumberSet(); }
	/*! \remarks Returns the number of vertices in the output mesh, assuming that
	the input mesh has the specified number of vertices.
	\par Parameters:
	<b>int inVNum</b>\n\n
	The number of vertices expected in the input mesh. */
   int outVNum (int inVNum) { return inVNum + NumVClone(inVNum) - NumVDelete(inVNum); }
	/*! \remarks Returns the number of faces in the output mesh, assuming that the
	input mesh is of the expected (fnum) size. */
   int outFNum () { return fnum + fCreate.Count() - fDelete.NumberSet(); }
	/*! \remarks Returns the number of faces in the output mesh, assuming that the
	input mesh has the specified number of faces.
	\par Parameters:
	<b>DWORD inFNum</b>\n\n
	The number of faces expected in the input mesh. */
   int outFNum (int inFNum) { return inFNum + fCreate.Count() - NumFDelete(inFNum); }

   /*! \remarks To make things easy for developers, it's possible to create or
   remap mapping faces to use UNDEFINED mapping verts. This routine, which is
   called by Apply below, fills in those UNDEFINED mapping values with the
   mapping vertices used by neighboring faces, or, if necessary, by new mapping
   vertices. (These new mapping vertices are always (.5,.5,0) for regular map
   channels or (1,1,1) for the vertex color channel.)\n\n
   This process does not produce very good maps, but it allows a sort of
   minimal mapping support that prevents maps from being lost before the user
   can make their manual corrections. */
   DllExport void FillInFaces (Mesh & m); // Fills in undefined mapping face verts.
   /*! \remarks Activates the vertex color MapDelta, adding whatever new map
   verts or faces are needed to match the current MeshDelta. After calling this
   method, applying this MeshDelta to a mesh without vertex colors will result
   in a vertex color map with all white vertices and a topology identical to
   the mesh. */
   DllExport void AddVertexColors ();  // Adds vertex color mapdelta to match this meshdelta.
   /*! \remarks Activates the specified MapDelta, adding whatever new map
   verts or faces are needed to match the current MeshDelta. If mapID is 0, the
   standard vertex colors (white) will be applied (see AddVertexColors).
   Otherwise, after calling this method, applying this MeshDelta to a mesh
   without the specified map active will result in a map with the same topology
   as the mesh and UVVerts that are copies of the mesh vertices.
   \par Parameters:
   <b>int mapID</b>\n\n
   The map channel to add. 0 represents vertex colors, 1 is the original map
   channel (referred to in class Mesh by tVerts and tvFaces), and 2-99 are the
   new map channels (stored in meshes in the MeshMap class). */
   DllExport void AddMap (int mapID); // Adds mapdelta on specified channel to match this meshdelta.
   /*! \remarks Activates the specified vertex data channel, creating a
   default set of vertex data to match the output of the current MeshDelta.
   ("Default" values of vertex data depend on the channel, and are given by
   VDataDefault (vdChan).)
   \par Parameters:
   <b>int vdChan</b>\n\n
   The vertex data channel\n\n
   <b>Mesh *m=NULL</b>\n\n
   A pointer to the Mesh object. */
   DllExport void AddVertexData (int vdChan, Mesh *m=NULL);

   // Create lookup tables for fast conversion of pre- and post- vert/face indices.
   /*! \remarks Updates the MeshDelta's internal lookup tables, which make use
   of all delete and create records to set up a correspondence between output
   and input vertices and faces.
   \par Parameters:
   <b>int extraV</b>\n\n
   If nonzero, this indicates the number of extra spaces that should be added
   to the lookup table. This is useful for example if you want the lookup table
   to still be valid after the next \<extraV\> vertex creates or clones.\n\n
   <b>int extraF</b>\n\n
   If nonzero, this indicates the number of extra spaces that should be added
   to the lookup table. This is useful for example if you want the lookup table
   to still be valid after the next \<extraF\> face creates. */
   DllExport void UpdateLUTs (int extraV=0, int extraF=0);
   /*! \remarks Invalidates and clears the lookup tables. This usually only
   needs to be called internally. */
   DllExport void ClearLUTs ();

   // Following methods turn output indices to input indices.
   /*! \remarks Finds the input mesh index of the vertex with the specified
   output mesh index.\n\n
   Note that these indices are the same if there are no vertex deletes.
   \par Parameters:
   <b>DWORD i</b>\n\n
   The output mesh index.
   \return  The input mesh index of the same vertex. If the vertex specified is
   actually created by this MeshDelta, the return value would be vnum+i for
   vClone[i]. */
   DllExport DWORD VLut (DWORD i);
   /*! \remarks Finds the input mesh index of the face with the specified
   output mesh index.\n\n
   Note that these indices are the same if there are no face deletes.
   \par Parameters:
   <b>DWORD i</b>\n\n
   The output mesh index.
   \return  The input mesh index of the same face. If the face specified is
   actually created by this MeshDelta, the return value would be fnum+i for
   fCreate[i]. */
   DllExport DWORD FLut (DWORD i);
   // Following methods turn input indices to output indices.
   /*! \remarks Returns the index in the output mesh of the specified input
   vertex.\n\n
   Note that these indices are the same if there are no vertex deletes.
   \par Parameters:
   <b>DWORD i</b>\n\n
   The index of the vertex in the input mesh.
   \return  The output mesh index of the same vertex. If the vertex was deleted
   in this MeshDelta, UNDEFINED is returned. */
   DllExport DWORD PostVIndex (DWORD i);
   /*! \remarks Returns the index in the output mesh of the specified input
   face.\n\n
   Note that these indices are the same if there are no face deletes.
   \par Parameters:
   <b>DWORD i</b>\n\n
   The index of the face in the input mesh.
   \return  The output mesh index of the same face. If the face was deleted in
   this MeshDelta, UNDEFINED is returned. */
   DllExport DWORD PostFIndex (DWORD i);
   // Following operate on output indices
   /*! \remarks Indicates whether or not the specified vertex is created in
   this MeshDelta.
   \par Parameters:
   <b>DWORD i</b>\n\n
   The output-based vertex index. */
   bool IsVClone (DWORD i) { int j=VLut(i)-vnum; return ((j>=0) && (j<vClone.Count())); }
   /*! \remarks Tells you what input vertex the specified output vertex is a
   clone of.
   \par Parameters:
   <b>DWORD i</b>\n\n
   The output-based vertex index.
   \return  The index in the input mesh of the original vertex this one's a
   clone of. If this vertex is not a clone, UNDEFINED is returned. */
   DWORD VCloneOf (DWORD i) { int j=VLut(i)-vnum; return ((j>=0) && (j<vClone.Count())) ? vClone[j].vid : UNDEFINED; }
   // NOTE: vCreate array no longer used in 3.1!
   /*! \remarks No longer used.\n\n
   This method was used in 3.0 and 3.1 to indicate whether the specified vertex
   was created as a create, not a clone in this MeshDelta. In 4.0, we
   integrated the clone and create records together, and this method is now set
   to always return FALSE.
   \par Parameters:
   <b>DWORD i</b>\n\n
   The output-based vertex index. */
   bool IsVCreate (DWORD i) { return FALSE; }
   /*! \remarks Indicated whether the specified face was created in this
   MeshDelta.
   \par Parameters:
   <b>DWORD i</b>\n\n
   The face index in the output mesh. */
   bool IsFCreate (DWORD i) { int j=FLut(i)-fnum; return ((j>=0) && (j<fCreate.Count())); }
   /*! \remarks Obtains the index of the vMove entry that relates to this
   vertex.
   \par Parameters:
   <b>DWORD i</b>\n\n
   The input-based vertex index.
   \return  The index in the vMove array of the vertex move corresponding to
   this vertex, or UNDEFINED if this vertex has no move associated with it. */
   DllExport DWORD MoveID (DWORD i);

   // Basic topological operations:
   // Those that accept DWORD indices require post-operation indices.
   /*! \remarks Creates new vertices.
   \par Parameters:
   <b>Point3 *p</b>\n\n
   A pointer to an array of points representing the new vertices.\n\n
   <b>int num</b>\n\n
   The size of the point array.\n\n
   <b>BitArray *sel=NULL</b>\n\n
   If non-NULL, this points to a BitArray of size num that indicates which of
   these new vertices should be selected. (If NULL, none of the new vertices
   are selected.)\n\n
   <b>BitArray *hide=NULL</b>\n\n
   If non-NULL, this points to a BitArray of size num that indicates which of
   these new vertices should be hidden. (If NULL, none of the new vertices are
   hidden.)
   \return  The index (in the output mesh) of the first of these new vertices.
   */
   DllExport DWORD VCreate (Point3 *p, int num=1, BitArray *sel=NULL, BitArray *hide=NULL);
   /*! \remarks Clones some vertices.
   \par Parameters:
   <b>DWORD *v</b>\n\n
   A pointer to an array of indices of verts that should be cloned.\n\n
   <b>int num</b>\n\n
   The size of the array.
   \return  The index (in the output mesh) of the first of the clones. */
   DllExport DWORD VClone (DWORD *v, int num=1);
   /*! \remarks Clones some vertices.
   \par Parameters:
   <b>DWORD *v</b>\n\n
   A pointer to an array of ids of vertices that should be cloned.\n\n
   <b>Point3 *off</b>\n\n
   A pointer to an array of offsets for the clones.\n\n
   <b>int num</b>\n\n
   The size of the arrays.
   \return  The index (in the output mesh) of the first of the clones. */
   DllExport DWORD VClone (DWORD *v, Point3 *off, int num=1);
   /*! \remarks Clones some vertices.
   \par Parameters:
   <b>VertMove *vm</b>\n\n
   A pointer to an array of VertMoves indicating which vertices should be
   cloned and what offsets the clones should use.\n\n
   <b>int num</b>\n\n
   The size of the vm array.
   \return  The index (in the output mesh) of the first of the clones. */
   DllExport DWORD VClone (VertMove *vm, int num=1);
   /*! \remarks Clones a single vertex.
   \par Parameters:
   <b>DWORD v</b>\n\n
   The index (in the output mesh) of the vertex you wish to clone.
   \return  The index (in the output mesh) of the clone. */
   DWORD VClone (DWORD v) { return VClone (&v, 1); }
   /*! \remarks Clones and offsets a single vertex.
   \par Parameters:
   <b>DWORD v</b>\n\n
   The index (in the output mesh) of the vertex you wish to clone.\n\n
   <b>Point3 off</b>\n\n
   The desired offset from the original vertex.
   \return  The index (in the output mesh) of the clone. */
   DWORD VClone (DWORD v, Point3 off) { return VClone (&v, &off, 1); }
	/*! \remarks Returns the expected location in the output mesh of the specified
	vertex.
	\par Parameters:
	<b>Mesh \& m</b>\n\n
	The input mesh.\n\n
	<b>DWORD v</b>\n\n
	The vertex you want the output location of. This index is input-based - the
	vertex index in <b>m</b>, not in the output mesh. */
   DllExport Point3 OutVert (Mesh & m, DWORD v);
   /*! \remarks Deletes the specified vertices.
   \par Parameters:
   <b>DWORD *v</b>\n\n
   A pointer to an array of (output-based) ids of the vertices that should be
   deleted.\n\n
   <b>int num</b>\n\n
   The number of vertices to delete (the size of the v array). */
   DllExport void VDelete (DWORD *v, int num=1);
   /*! \remarks Deletes the specified vertices.
   \par Parameters:
   <b>BitArray \& vdel</b>\n\n
   A BitArray, of size OutVNum, indicating which of the vertices should be
   deleted. Vertices in this array are indexed by output mesh order. */
   DllExport void VDelete (BitArray & vdel);
   /*! \remarks Creates new faces.\n\n
   Note: MapDeltas must be kept up to date with all new face creations - see
   CreateDefaultMapFaces.
   \par Parameters:
   <b>Face *f</b>\n\n
   A pointer to an array of faces to be added to the MeshDelta.\n\n
   <b>int num</b>\n\n
   The size of the face array.
   \return  The index (in the output mesh) of the first of these new faces. */
   DllExport DWORD FCreate (Face *f, int num=1);
   /*! \remarks Creates new faces. This is what's used to add in face
   creations in the all-important Compose method. It assumes that the
   "originals" in the array of FaceCreates are post-indexed, and uses FLut and
   extracts originals for creates-of-creates as appropriate.\n\n
   Note: MapDeltas must be kept up to date with all new face creations - see
   CreateDefaultMapFaces.
   \par Parameters:
   <b>FaceCreate *f</b>\n\n
   A pointer to an array of face create records to be added to the
   MeshDelta.\n\n
   <b>int num</b>\n\n
   The size of the face create array.
   \return  The index (in the output mesh) of the first of these new faces. */
   DllExport DWORD FCreate (FaceCreate *f, int num=1);
   /*! \remarks Creates 2 new faces, forming a quad.\n\n
   Note: MapDeltas must be kept up to date with all new face creations - see
   CreateDefaultMapFaces.
   \par Parameters:
   <b>DWORD *v</b>\n\n
   A pointer to an array of 4 vertices to be used as corners of the quad.\n\n
   <b>DWORD smG=0</b>\n\n
   The smoothing group desired for the new faces.\n\n
   <b>MtlID matID=0</b>\n\n
   The material ID desired for the new faces.\n\n
   <b>int orig=UNDEFINED</b>\n\n
   This optional parameter sets the map faces to undefined.
   \return  The index (in the output mesh) of the first of these 2 new faces.
   */
   DllExport DWORD FCreateQuad (DWORD *v, DWORD smG=0, MtlID matID=0, int orig=UNDEFINED);
   /*! \remarks Creates a new face by copying an existing face. The result is
   put into the fCreate array and treated thereafter like a face create - this
   is different from vertex clones, which are maintained separately from vertex
   creates.\n\n
   Note: MapDeltas must be kept up to date with all new face creations - see
   CreateDefaultMapFaces.
   \par Parameters:
   <b>Face \& f</b>\n\n
   The face we wish to clone. (This is typically generated by the outFace
   method.)\n\n
   <b>DWORD ff</b>\n\n
   The (output-based) index of the face we're cloning. (This is used to copy
   face and edge selection.)\n\n
   <b>DWORD remapFlags=0</b>\n\n
   <b>DWORD *v=NULL</b>\n\n
   If we wish to remap any of the corners of this face while cloning, the
   appropriate flags and vertices should be passed in these last two arguments.
   v should point to an array of 3 vertex (output) indices, although the ones
   not marked as used by the remapFlags need not be set to anything in
   particular. See class FaceRemap for more information about face remapping.
   \return  The index (in the output mesh) of the new face. */
   DllExport DWORD FClone (Face & f, DWORD ff, DWORD remapFlags=0, DWORD *v=NULL);
	/*! \remarks MapDeltas must <b>always</b> keep their faces in sync with the
	parent MeshDelta. If the developer creates new faces, but doesn't want to go
	through the bother of figuring out exactly how the related map faces should
	look, this method may be used to create map faces with UNDEFINED verts. These
	can then be filled in automatically later. (See the MeshDelta FillInFaces
	method for details.)\n\n
	For <b>every</b> face created in the MeshDelta, either CreateDefaultMapFaces
	should be called, or map faces should be created in every active map channel,
	using MapDelta::FCreate and related methods.
	\par Parameters:
	<b>int num=1</b>\n\n
	The number of default faces we wish to create in each active map channel. */
   DllExport void CreateDefaultMapFaces (int num=1);
   /*! \remarks Adds face remaps to this MeshDelta. If the face specified in
   each FaceRemap already has a remap record, the two are combined. If the face
   specified is a face created by this MeshDelta, the remap is applied directly
   to the fCreate entry instead of being stored in fRemap.
   \par Parameters:
   <b>FaceRemap *f</b>\n\n
   A pointer to an array of FaceRemap that should be appended to this
   MeshDelta.\n\n
   Note that the faces and vertices in each FaceRemap must be indexed by their
   positions after all of the current MeshDelta's creates, clones, etc, but
   before any vertex or face deletes. Vertex index values of 0 through vnum-1
   are considered to be the original mesh's vertices; values above this are
   cloned or created vertices. Likewise, face index values of 0 through fnum-1
   are considered to be the original mesh faces, while fnum through
   fnum+fCreate.Count()-1 are this MeshDelta's face creates.\n\n
   <b>int num=1</b>\n\n
   The number of elements in the FaceRemap array. */
   DllExport void FRemap (FaceRemap *f, int num=1);
   /*! \remarks Adds a face remap to this MeshDelta. If the face specified
   already has a remap record, the two are combined. If the face specified is a
   face created by this MeshDelta, the remap is applied directly to the fCreate
   entry instead of being stored in fRemap.
   \par Parameters:
   <b>DWORD f</b>\n\n
   The face to remap.\n\n
   Note that this face must be indexed by its position after all of the current
   MeshDelta's face creates, but before any face deletes. Face index values of
   0 through fnum-1 are considered to be the original mesh faces, while fnum
   through fnum+fCreate.Count()-1 are this MeshDelta's face creates.\n\n
   <b>DWORD flags</b>\n\n
   Face Remap flags - these indicate which vertices should be remapped. The
   possibilities are FR_V0 (1), FR_V1 (2), and FR_V2 (4). (See class FaceRemap
   for more information.)\n\n
   <b>DWORD *v</b>\n\n
   A pointer to the vertices to remap the face to use. Only the positions
   indicated in the remap flags need contain meaningful data.\n\n
   Note that the vertices indicated here must be indexed by their positions
   after all of the current MeshDelta's creates and clones, but before any
   vertex deletes - essesntially input-based indexing. Vertex index values of 0
   through vnum-1 are considered to be the original mesh's vertices; values of
   vnum through vnum+vCreate.Count()-1 are considered to be this MeshDelta's
   newly created vertices; and values above this are cloned vertices. */
   DllExport void FRemap (DWORD f, DWORD flags, DWORD *v);
   /*! \remarks Obtains the index of the fRemap entry that relates to this
   face.
   \par Parameters:
   <b>DWORD ff</b>\n\n
   The input-based face index.
   \return  If there is such an entry, the index is returned, so
   fRemap[RemapID(ff)].fid == ff. If there is no remap record for this face,
   the method returns UNDEFINED. */
   DllExport DWORD RemapID (DWORD ff);
   /*! \remarks Tells whether the specified corner of the specified face has
   been remapped in this MeshDelta.
   \par Parameters:
   <b>DWORD ff</b>\n\n
   The input-based face index.\n\n
   <b>DWORD vid</b>\n\n
   The corner of the face - 0, 1, or 2.
   \return  If this corner has been remapped, it returns the vertex it's been
   remapped to. Otherwise, it returns UNDEFINED. */
   DllExport DWORD IsRemapped (DWORD ff, DWORD vid);
	/*! \remarks Returns the specified face as it would appear in the MeshDelta
	output. Face Changes, Smooths, and Remaps are applied.
	\par Parameters:
	<b>Mesh \& m</b>\n\n
	The input mesh.\n\n
	<b>DWORD f</b>\n\n
	The index of the face you want the output version of. This index is input-based
	- the face index in <b>m</b>, not in the output mesh. */
   DllExport Face OutFace (Mesh & m, DWORD ff);
   /*! \remarks Appends some face changes to the current MeshDelta. Face
   changes can encompass changes to face material IDs, edge visibility, or face
   hiding. See class FaceChange for more information.
   \par Parameters:
   <b>FaceChange *f</b>\n\n
   A pointer to an array of new face changes for this MeshDelta.\n\n
   <b>int num=1</b>\n\n
   The number of elements in the FaceChange array. */
   DllExport void FChange (FaceChange *f, int num=1);
   /*! \remarks Changes the characteristics of one face.
   \par Parameters:
   <b>DWORD f</b>\n\n
   The output-indexed face to change.\n\n
   <b>DWORD flags</b>\n\n
   <b>DWORD dat</b>\n\n
   These two parameters describe the change desired. See class FaceChange for a
   description of these flags. flags indicates which characteristics should be
   set, and dat includes the on-or-off state of each flag we're setting. So for
   example FChange (26, ATTRIB_EDGE_A| ATTRIB_EDGE_B, ATTRIB_EDGE_A) would set
   face 26 to have the first edge visible and the second invisible, without
   changing the existing visibility for the third edge. */
   DllExport void FChange (DWORD f, DWORD flags, DWORD dat);
   /*! \remarks Adds smoothing group changes to this MeshDelta. See class
   FaceSmooth for more information.
   \par Parameters:
   <b>FaceSmooth *f</b>\n\n
   A pointer to an array of smoothing change records. The face IDs in these
   records should be output-indexed.\n\n
   <b>int num=1</b>\n\n
   The number of elements in the FaceSmooth array. */
   DllExport void FSmooth (FaceSmooth *f, int num=1);
   /*! \remarks Changes the smoothing groups on the specified face.
   \par Parameters:
   <b>DWORD f</b>\n\n
   The output-based index of the face to change.\n\n
   <b>DWORD mask</b>\n\n
   The smoothing groups to change.\n\n
   <b>DWORD val</b>\n\n
   The smoothing group values. For instance, FSmooth (32, 7, 2) would set
   smoothing group 2 and clear groups 1 and 3 for face 32, since mask has bits
   0, 1, and 2 set but val only has bit 1 set. */
   DllExport void FSmooth (DWORD f, DWORD mask, DWORD val);
   /*! \remarks Sets the material ID for the specified face. If the face is
   created by this MeshDelta, the fCreate record is amended. If it's an input
   face, a FaceChange record is created or amended.
   \par Parameters:
   <b>DWORD f</b>\n\n
   The output-indexed face to change.\n\n
   <b>MtlID mt</b>\n\n
   The desired material ID. */
   void SetMatID (DWORD f, MtlID mt) { FChange (f, ATTRIB_MATID, mt<<ATTRIB_MATID_SHIFT); }
   /*! \remarks Sets the smoothing groups on the specified face.
   \par Parameters:
   <b>DWORD f</b>\n\n
   The output-based index of the face to change.\n\n
   <b>DWORD smG</b>\n\n
   The smoothing groups to set. All bits not set in this parameter are cleared.
   */
   void SetSmGroup (DWORD f, DWORD smG) { FSmooth (f, ~DWORD(0), smG); }
   /*! \remarks Sets the edge visibility for the specified side of the
   specified face. If the face is created by this MeshDelta, the fCreate record
   is amended. If it's an input face, a FaceChange record is created or
   amended.
   \par Parameters:
   <b>DWORD f</b>\n\n
   The output-indexed face to change.\n\n
   <b>DWORD ed</b>\n\n
   The side of the face to change (0, 1, or 2).\n\n
   <b>BOOL vis=TRUE</b>\n\n
   The desired visibility. */
   void SetEdgeVis (DWORD f, DWORD ed, BOOL vis=TRUE) { FChange (f, (1<<ed), vis?(1<<ed):0); }
   /*! \remarks Deletes the specified faces.
   \par Parameters:
   <b>DWORD *f</b>\n\n
   A pointer to an array of output-based indices of faces we wish to delete.
   Note that all the elements should be based on the output before any
   deletions occur. So if you wanted to delete what are currently faces 3 and
   5, you could pass an array with 3 and 5, you would <b>not</b> have to think,
   "Ah, 3 will be deleted, so I should use 4 instead of 5."\n\n
   <b>int num=1</b>\n\n
   The size of the array. */
   DllExport void FDelete (DWORD *f, int num=1);
   /*! \remarks Deletes the specified faces.
   \par Parameters:
   <b>BitArray \& fdel</b>\n\n
   The faces to delete. The faces are indexed by their output mesh positions.
   */
   DllExport void FDelete (BitArray & fdel);

   // Geometric ops:
   /*! \remarks Moves a single vertex. (Note that if the same vertex is moved
   twice, the new move is simply added to the old one - there is never more
   than one VertMove in the vMove array for a single input vertex.)
   \par Parameters:
   <b>int i</b>\n\n
   The index of the vertex in the output mesh.\n\n
   <b>const Point3 \& p</b>\n\n
   The vector to move the vertex by. */
   DllExport void Move (int i, const Point3 & p);
   /*! \remarks Moves the specified vertices. (Note that if the same vertex is
   moved twice, the new move is simply added to the old one - there is never
   more than one VertMove in the vMove array for a single input vertex.)
   \par Parameters:
   <b>BitArray \& sel</b>\n\n
   Indicates which vertices should be moved. Vertices are indexed based on the
   output mesh.\n\n
   <b>const Point3 \& p</b>\n\n
   The vector to move the vertices by. */
   DllExport void Move (BitArray & sel, const Point3 & p);
   /*! \remarks Adds in the specified vertex moves. (Note that if the same
   vertex is moved twice, the new move is simply added to the old one - there
   is never more than one VertMove in the vMove array for a single input
   vertex.)
   \par Parameters:
   <b>VertMove *vm</b>\n\n
   A pointer to an array of VertMoves to apply to this MeshDelta. Vertices are
   indexed based on the output mesh.\n\n
   <b>int num</b>\n\n
   The size of the VertMove array. */
   DllExport void Move (VertMove *vm, int num);

   // FOLLOWING TWO METHODS SHOULD NOT BE USED:
   DllExport void GetSavingPermutations (int & numCr, int & numCl, Tab<int> & vPermute, Tab<int> & vPReverse);
   DllExport void PermuteClonedVertices (Tab<int> & vPermute);

   // Gotta be able to save and load this complex thing...
   /*! \remarks Saves the MeshDelta to a 3ds Max file. */
   DllExport IOResult Save (ISave *isave);
   /*! \remarks Loads the MeshDelta from a 3ds Max file. */
   DllExport IOResult Load (ILoad *iload);

   // Handy debugging output
   /*! \remarks Prints out the MeshDelta to the DebugPrint window in Developer
   Studio.
   \par Parameters:
   <b>bool lut=FALSE</b>\n\n
   If TRUE, the vertex and face lookup tables are printed out.\n\n
   <b>bool mp=FALSE</b>\n\n
   Active map deltas are also printed out if this is TRUE. */
   DllExport void MyDebugPrint (bool lut=FALSE, bool mp=FALSE);

   // Backup-relevant characteristics:
   /*! \remarks Indicates what parts of a MeshDelta could be changed if this
   MeshDelta were appended to it. This is useful when backing up MeshDelta for
   Restore Objects. For instance, if you had a MeshDelta with lots of face
   smoothing changes, and you wanted to compose it with one that only moved
   vertices, there would be no reason to back up the smoothing changes for an
   undo.
   \par Parameters:
   <b>Tab\<DWORD\> *mChannels=NULL</b>\n\n
   If non-NULL, this points to a table that should be filled with change flags
   for the various map channels. The table is set to the number of map
   channels, and each DWORD in it is filled in by calling MapDelta::ChangeFlags
   on the appropriate map channel (or left at zero if the map channel is
   inactive.)
   \return  Returns some combination of the following flags, corresponding to
   the data members that would be changed:\n\n
   <b>MDELTA_VMOVE</b>\n\n
   <b>MDELTA_VCREATE</b>\n\n
   <b>MDELTA_VCLONE</b>\n\n
   <b>MDELTA_VDELETE</b>\n\n
   <b>MDELTA_VDATA</b>\n\n
   <b>MDELTA_FREMAP</b>\n\n
   <b>MDELTA_FCHANGE</b>\n\n
   <b>MDELTA_FCREATE</b>\n\n
   <b>MDELTA_FDELETE</b>\n\n
   <b>MDELTA_FDATA</b>\n\n
   <b>MDELTA_NUMBERS</b>\n\n
   <b>MDELTA_FSMOOTH</b>\n\n
   Note that in 4.0 and thereafter, the <b>MDELTA_VCREATE</b> and
   <b>MDELTA_VCLONE</b> flags are identical and represent the same information.
   (This was not true in 3.0 or 3.1.) */
   DllExport DWORD ChangeFlags (Tab<DWORD> *mChannels=NULL);
   /*! \remarks Copies the specified parts of the MeshDelta. (Useful in
   combination with ChangeFlags to create efficient Restore objects.)
   \par Parameters:
   <b>MeshDelta \& from</b>\n\n
   The MeshDelta to copy into this.\n\n
   <b>DWORD channels</b>\n\n
   Indicates the parts to copy - some combination of the following flags:\n\n
   <b>MDELTA_VMOVE</b>\n\n
   <b>MDELTA_VCREATE</b>\n\n
   <b>MDELTA_VCLONE</b>\n\n
   <b>MDELTA_VDELETE</b>\n\n
   <b>MDELTA_VDATA</b>\n\n
   <b>MDELTA_FREMAP</b>\n\n
   <b>MDELTA_FCHANGE</b>\n\n
   <b>MDELTA_FCREATE</b>\n\n
   <b>MDELTA_FDELETE</b>\n\n
   <b>MDELTA_FDATA</b>\n\n
   <b>MDELTA_NUMBERS</b>\n\n
   <b>MDELTA_FSMOOTH</b>\n\n
   Note that in 4.0 and thereafter, the <b>MDELTA_VCREATE</b> and
   <b>MDELTA_VCLONE</b> flags are identical and represent the same information.
   (This was not true in 3.0 or 3.1.)\n\n
   <b>Tab\<DWORD\> *mChannels=NULL</b>\n\n
   If non-NULL, this points to a table that contains channels to copy in the
   various map channels. The table should be of the size of the number of map
   channels. For each active map channel in from, the corresponding DWORD in
   this table is passed in MapDelta::CopyMDChannels to copy the relevant parts
   of the map. */
   DllExport void CopyMDChannels (MeshDelta & from, DWORD channels, Tab<DWORD> *mChannels=NULL);

   // Double-checking routines, good for after loading.
   // Returns TRUE if order was already correct, FALSE if it had to make a correction.
   DllExport BOOL CheckOrder ();
   DllExport BOOL CheckMapFaces ();

   // More complex operations, built on the list above.
   // Mesh given is expected to be result of the current MeshDelta.
   // Found in MDAppOps.cpp
   /*! \remarks Automatically generates smoothing groups for the selected
   faces. Existing smoothing groups are ignored. See the AutoSmooth feature in
   Edit Mesh for an example.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected.\n\n
   <b>BitArray sel</b>\n\n
   The faces to AutoSmooth.\n\n
   <b>float angle</b>\n\n
   The maximum angle between faces that should be smoothed together.\n\n
   <b>AdjFaceList *af=NULL</b>\n\n
   A pointer to the adjacent face list for this mesh. If NULL, the method
   constructs its own AdjFaceList.\n\n
   <b>AdjEdgeList *ae=NULL</b>\n\n
   A pointer to the adjacent edge list for this mesh. If NULL, the method
   constructs its own AdjEdgeList. */
   DllExport void AutoSmooth(Mesh &m, BitArray sel, float angle, AdjFaceList *af=NULL, AdjEdgeList *ae=NULL);
   /*! \remarks Moves the selected vertices along the directions given to
   produce the movement corresponding to the mouse drags of a Bevel operation
   in Editable Mesh. Note that this method only changes geometry.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected.\n\n
   <b>BitArray vset</b>\n\n
   The set of vertices to move.\n\n
   <b>float outline</b>\n\n
   The amount of outlining to do in this Bevel move.\n\n
   <b>Tab\<Point3\> *odir</b>\n\n
   The outline direction for each vertex. This should be given by
   MeshTempData::OutlineDir. See the Edit Mesh source for details.\n\n
   <b>float height</b>\n\n
   The amount of extrusion to do in this Bevel move.\n\n
   <b>Tab\<Point3\> *hdir</b>\n\n
   The extrusion direction for each vertex. This should be given by
   MeshTempData::EdgeExtDir or FaceExtDir. See the Edit Mesh source for
   examples. */
   DllExport void Bevel (Mesh & m, BitArray vset, float outline, Tab<Point3> *odir,
      float height, Tab<Point3> *hdir);
   /*! \remarks Create a polygon of any size. The polygon may be nonconvex,
   but should be (roughly) coplanar.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected.\n\n
   <b>int deg</b>\n\n
   The number of vertices used by this polygon (its degree).\n\n
   <b>int *v</b>\n\n
   The indices of the vertices in <b>Mesh m</b> that this polygon should
   use.\n\n
   <b>DWORD smG=0</b>\n\n
   The desired smoothing group for the new polygon.\n\n
   <b>MtlID matID=0</b>\n\n
   The desired material ID for the new polygon. */
   DllExport DWORD CreatePolygon (Mesh & m, int deg, int *v, DWORD smG=0, MtlID matID=0);
   /*! \remarks Deletes the specified vertices, along with any faces that used
   them.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected.\n\n
   <b>BitArray sel</b>\n\n
   The vertices to delete. Any face that uses any of the vertices selected here
   will also be deleted. */
   DllExport void DeleteVertSet (Mesh & m, BitArray sel);   // does delete faces
   /*! \remarks Deletes all faces using the specified edges. (Doesn't delete
   any verts.)
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected.\n\n
   <b>BitArray sel</b>\n\n
   The edges to delete the faces of. Edges are indexed by face*3+side. */
   DllExport void DeleteEdgeSet (Mesh & m, BitArray sel);   // doesn't delete verts
   /*! \remarks Deletes the specified faces. (Doesn't delete any verts.)
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected.\n\n
   <b>BitArray sel</b>\n\n
   The faces to delete. */
   DllExport void DeleteFaceSet (Mesh & m, BitArray sel);   // doesn't delete verts.
   /*! \remarks Deletes the current subobject selection. If m.selLevel is
   MESH_OBJECT, nothing is deleted. If it's MESH_VERTEX, the faces using the
   selected vertices are also deleted.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected. */
   DllExport void DeleteSelected (Mesh & m);
   /*! \remarks Deletes the vertices not in use by any faces.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected. */
   DllExport void DeleteIsoVerts (Mesh & m);
   /*! \remarks Flips the normal of the specified face (by switching the
   face's v[0] and v[1]). (Related map faces are also flipped.)
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected.\n\n
   <b>DWORD face</b>\n\n
   The face to flip. */
   DllExport void FlipNormal (Mesh & m, DWORD face);
   /*! \remarks Flattens the faces indicated into the same plane. The target
   plane is determined by the average of all the face centers and the average
   of all the face normals.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected.\n\n
   <b>BitArray sel</b>\n\n
   The faces to make coplanar. */
   DllExport void MakeSelFacesPlanar (Mesh &m, BitArray sel);
   /*! \remarks Flattens the vertices indicated into the same plane. The
   target plane is determined by the average position and normal of the
   vertices.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected.\n\n
   <b>BitArray sel</b>\n\n
   The vertices to make coplanar. */
   DllExport void MakeSelVertsPlanar (Mesh &m, BitArray sel);
   /*! \remarks Moves the vertices indicated into the specified plane. (The
   target plane is defined as all points which, when DotProd'd with N, return
   offset.) All vertices are moved along the normal vector N.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected.\n\n
   <b>BitArray sel</b>\n\n
   The vertices to move into the plane.\n\n
   <b>Point3 \& N</b>\n\n
   The unit normal to the plane.\n\n
   <b>float offset</b>\n\n
   The offset of the plane (also its distance from the origin). */
   DllExport void MoveVertsToPlane (Mesh & m, BitArray sel, Point3 & N, float offset);
   /*! \remarks Like the old standalone method "FitMeshIDsToMaterial", this
   method limits the material IDs to values between 0 and numMats-1. This is
   useful eg in matching the number of material Ids to the number of materials
   used on this node.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected.\n\n
   <b>int numMats</b>\n\n
   The number of material ids allowed. */
   DllExport void RestrictMatIDs (Mesh & m, int numMats);   // like "FitMeshIDsToMaterial".
   /*! \remarks Sets or clears face selection depending on whether they match
   a pattern of flags. This is pretty much only useful for selecting or
   deselecting hidden faces:\n\n
   <b> mdelta.SelectFacesByFlags(*mesh, FALSE, FACE_HIDDEN,
   FACE_HIDDEN); // deselects hidden faces.</b>
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh, which should match the output of the current MeshDelta, that
   should be affected.\n\n
   <b>BOOL onoff</b>\n\n
   Indicates whether faces should be selected or deselected if they match the
   flag pattern.\n\n
   <b>DWORD flagmask</b>\n\n
   Indicates whether faces should be selected or deselected if they match the
   flag pattern.\n\n
   <b>DWORD flags</b>\n\n
   Indicates whether faces should be selected or deselected if they match the
   flag pattern. */
   DllExport void SelectFacesByFlags (Mesh & m, BOOL onoff, DWORD flagmask, DWORD flags);
   // if adj is non-NULL, it uses it to set the "other side" visible too.
   DllExport void SetSingleEdgeVis (Mesh & m, DWORD ed, BOOL vis, AdjFaceList *adj=NULL);

   // Following will initialize to the mesh given: they can't be used to "add" ops to an existing MeshDelta.
   // (To add these ops, make a new MeshDelta, call one of the following, and append it to your previous one with Compose.)
   // Found in MDOps.cpp
   /*! \remarks Attaches another mesh to this one.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>Mesh \& attachment</b>\n\n
   The mesh this MeshDelta should attach.\n\n
   <b>Matrix3 \& relativeTransform</b>\n\n
   The transform taking the attachment mesh from its object space to ours.\n\n
   <b>int matOffset</b>\n\n
   The offset that should be applied to all the material IDs in the attachment.
   */
   DllExport void AttachMesh (Mesh & m, Mesh &attachment, Matrix3 & relativeTransform,
      int matOffset);
   /*! \remarks Splits the selected vertices into a separate vertex for every
   face that uses them.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray vset</b>\n\n
   The vertices that should be broken. */
   DllExport void BreakVerts (Mesh & m, BitArray vset);
   /*! \remarks Performs the topological changes needed for an edge chamfer,
   and creates the information necessary to do the geometric changes.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray eset</b>\n\n
   The edges to chamfer.\n\n
   <b>MeshChamferData \& mcd</b>\n\n
   A class in which the directions and limits of movement for all the vertices
   involved in the chamfer should be stored. See class MeshChamferData.\n\n
   <b>AdjEdgeList *ae=NULL</b>\n\n
   A pointer to the adjacent edge list for this mesh. If NULL, the method
   constructs its own AdjEdgeList. */
   DllExport void ChamferEdges (Mesh & m, BitArray eset, MeshChamferData &mcd, AdjEdgeList *ae=NULL);
   /*! \remarks Moves the relevant vertices to a specified chamfer value.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>MeshChamferData \& mcd</b>\n\n
   A class in which the directions and limits of movement for all the vertices
   involved in the chamfer have been stored by a previous ChamferVertices or
   ChamferEdges call. See class MeshChamferData.\n\n
   <b>float amount</b>\n\n
   The amount (in object space units) of chamfering to do. For vertex chamfers,
   this indicates how far along the edges each point should move. For edge
   chamfers, it represents how far along each face each edge should move.\n\n
   <b>AdjEdgeList *ae=NULL</b>\n\n
   A pointer to the adjacent edge list for this mesh. If NULL, the method
   constructs its own AdjEdgeList. */
   DllExport void ChamferMove (Mesh & m, MeshChamferData &mcd, float amount, AdjEdgeList *ae=NULL);
   /*! \remarks Performs the topological changes needed for a vertex chamfer,
   and creates the information necessary to do the geometric changes.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray vset</b>\n\n
   The vertices that should be chamfered.\n\n
   <b>MeshChamferData \& mcd</b>\n\n
   A class in which the directions and limits of movement for all the vertices
   involved in the chamfer should be stored. See class MeshChamferData.\n\n
   <b>AdjEdgeList *ae=NULL</b>\n\n
   A pointer to the adjacent edge list for this mesh. If NULL, the method
   constructs its own AdjEdgeList. */
   DllExport void ChamferVertices (Mesh & m, BitArray vset, MeshChamferData &mcd, AdjEdgeList *ae=NULL);
   /*! \remarks Clones the specified faces, along with the vertices and
   mapping vertices they use.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray fset</b>\n\n
   The faces that should be cloned. */
   DllExport void CloneFaces (Mesh & m, BitArray fset);
   /*! \remarks Clones the specified vertices. More efficient on an
   initialized MeshDelta than VClone, which has to be able to cope with
   existing complex MeshDeltas.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray vset</b>\n\n
   The vertices that should be cloned. */
   DllExport void CloneVerts (Mesh & m, BitArray vset);
   /*! \remarks Collapses the edges indicated down to a point.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray ecol</b>\n\n
   The edges to collapse.\n\n
   <b>AdjEdgeList *ae=NULL</b>\n\n
   A pointer to the adjacent edge list for this mesh. If NULL, the method
   constructs its own AdjEdgeList. */
   DllExport void CollapseEdges(Mesh &m, BitArray ecol, AdjEdgeList *ae=NULL);
   /*! \remarks Cuts the mesh from a point on one edge to a point on another,
   along a line drawn by looking at the mesh from a particular viewpoint. (See
   Edit Mesh's Cut feature for an illustration.)
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>DWORD ed1</b>\n\n
   The edge that the cut starts on. The edge is indexed by face*3+side, so the
   start vertex is m.faces[ed1/3].v[ed1%3], and the end vertex is
   m.faces[ed1/3].v[(ed1+1)%3].\n\n
   <b>float prop1</b>\n\n
   The position on the edge to start the cut from. 0 means the start vertex of
   the edge, and 1 means the end vertex.\n\n
   <b>DWORD ed2</b>\n\n
   The edge that the cut should end on.\n\n
   <b>float prop2</b>\n\n
   The position on the edge to finish the cut on. 0 means the start vertex of
   the edge, and 1 means the end vertex.\n\n
   <b>Point3 \&norm</b>\n\n
   The direction of view. The cut will take place on this "side" of the mesh,
   in the plane formed by this vector and the direction from the start to the
   end.\n\n
   <b>bool fixNeigbors=TRUE</b>\n\n
   Indicates whether the faces on the other side of each end of the cut should
   be split to prevent splits at the ends.\n\n
   <b>bool split=FALSE</b>\n\n
   Indicates whether the cut should actually split the mesh apart or just
   refine it by adding geometry. */
   DllExport DWORD Cut (Mesh & m, DWORD ed1, float prop1, DWORD ed2, float prop2,
      Point3 & norm, bool fixNeighbors=TRUE, bool split=FALSE);
   /*! \remarks Detaches a subset of the geometry from the mesh given, either
   separating it as a new element or creating a new mesh with it.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>Mesh *out</b>\n\n
   The new mesh into which the detached portion can be put. (If elem is TRUE,
   this is not used and may be NULL.)\n\n
   <b>BitArray fset</b>\n\n
   The selection to detach. If faces is TRUE, this is a face selection.
   Otherwise, it's a vertex selection.\n\n
   <b>BOOL faces</b>\n\n
   If TRUE, we should detach the selected faces; otherwise, we should detach
   the selected vertices.\n\n
   <b>BOOL del</b>\n\n
   Indicates whether the detached portion should be deleted from the original
   mesh.\n\n
   <b>BOOL elem</b>\n\n
   If TRUE, we're actually just detaching to an element, and the out mesh will
   not be used. */
   DllExport void Detach (Mesh & m, Mesh *out, BitArray fset, BOOL faces, BOOL del, BOOL elem);
   /*! \remarks Divides the specified edge, adding a point and dividing faces
   to match.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>DWORD ed</b>\n\n
   The edge to divide, indexed as face*3+side.\n\n
   <b>float prop=.5f</b>\n\n
   The proportion along the edge where the division should occur. 0 is the
   start vertex, m.faces[ed/3].v[ed%3], and 1 is the end vertex,
   m.faces[ed/3].v[(ed+1)%3].\n\n
   <b>AdjEdgeList *el=NULL</b>\n\n
   A pointer to the adjacent edge list for this mesh. If NULL, the method
   constructs its own AdjEdgeList if needed.\n\n
   <b>bool visDiag1=FALSE</b>\n\n
   Indicates whether the diagonal connecting the new point on this edge with
   the far corner of face ed/3 should be visible.\n\n
   <b>bool fixNeighbors=TRUE</b>\n\n
   Indicates whether the face on the other side of this edge, that is, the face
   using this edge that isn't ed/3, should be divided as well to prevent the
   introduction of a seam.\n\n
   <b>bool visDiag2=FALSE</b>\n\n
   Indicates whether the diagonal connecting the new point on this edge with
   the far corner of the face on the other side of the edge should be visible.
   (Not used if fixNeighbors is FALSE.)\n\n
   <b>bool split=FALSE</b>\n\n
   Indicates whether the method should create separate vertices for the two
   halves of the edge, splitting the mesh open along the diagonal(s). */
   DllExport void DivideEdge (Mesh & m, DWORD ed, float prop=.5f, AdjEdgeList *el=NULL,
      bool visDiag1=FALSE, bool fixNeighbors=TRUE, bool visDiag2=FALSE, bool split=FALSE);
   /*! \remarks Divides all the selected edges in half, creating new points
   and subdividing faces.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray eset</b>\n\n
   The edges to divide.\n\n
   <b>AdjEdgeList *ae=NULL</b>\n\n
   A pointer to the adjacent edge list for this mesh. If NULL, the method
   constructs its own AdjEdgeList. */
   DllExport void DivideEdges (Mesh & m, BitArray eset, AdjEdgeList *el=NULL);
   /*! \remarks Divides the selected face into 3, by introducing a new point
   on the face and splitting the original face along lines from the corners to
   the new point.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>DWORD f</b>\n\n
   The face to divide.\n\n
   <b>float *bary=NULL</b>\n\n
   A pointer to the barycentric coordinates of the new point on the face. If
   NULL, the center of the face is used.\n\n
   Barycentric coordinates on a triangle are a set of three numbers between 0
   and 1 that add up to 1. Any point on a triangle can be uniquely described by
   a set of these. The point corresponding to barycentric coordinates (a,b,c)
   on a face with corners A,B,C is a*A + b*B + c*C. */
   DllExport void DivideFace (Mesh & m, DWORD f, float *bary=NULL);
   /*! \remarks Divides the selected faces into 3, by creating their center
   points and splitting the original faces along lines from the corners to the
   center.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray fset</b>\n\n
   The faces to divide.\n\n
   <b>MeshOpProgress *mop=NULL</b>\n\n
   If non-NULL, this points to an implementation of class MeshOpProgress which
   can be used to interrupt the algorithm if it's taking too long. See class
   MeshOpProgress for details. */
   DllExport void DivideFaces (Mesh & m, BitArray fset, MeshOpProgress *mop=NULL);
   /*! \remarks Tessellates the mesh. This algorithm is exactly the one used
   in the Tessellate modifier, when operating on "Faces" (triangle icon) and in
   "Edge" type.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray fset</b>\n\n
   The faces to tessellate.\n\n
   <b>float tens</b>\n\n
   The tension for the edge tessellation. This value should be fairly small,
   between 0 and .5, and corresponds to the value in the Tessellate, Edit Mesh,
   or Editable Mesh UI's divided by 400.\n\n
   <b>AdjEdgeList *ae=NULL</b>\n\n
   A pointer to the adjacent edge list for this mesh. If NULL, the method
   constructs its own AdjEdgeList.\n\n
   <b>AdjFaceList *af=NULL</b>\n\n
   A pointer to the adjacent face list for this mesh. If NULL, the method
   constructs its own AdjFaceList.\n\n
   <b>MeshOpProgress *mop=NULL</b>\n\n
   If non-NULL, this points to an implementation of class MeshOpProgress which
   can be used to interrupt the algorithm if it's taking too long. See class
   MeshOpProgress for details. */
   DllExport void EdgeTessellate(Mesh &m, BitArray fset, float tens,
      AdjEdgeList *ae=NULL, AdjFaceList *af=NULL, MeshOpProgress *mop=NULL);
   /*! \remarks "Explodes" the mesh into separate elements.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>float thresh</b>\n\n
   The threshold angle between faces that indicates whether they should be in
   the same or different element.\n\n
   <b>bool useFaceSel=FALSE</b>\n\n
   Indicates whether the mesh's current face selection should be used or if the
   whole mesh should be exploded.\n\n
   <b>AdjFaceList *af=NULL</b>\n\n
   A pointer to the adjacent face list for this mesh. If NULL, the method
   constructs its own AdjFaceList. */
   DllExport void ExplodeFaces(Mesh &m, float thresh, bool useFaceSel=FALSE, AdjFaceList *af=NULL);
   /*! \remarks Performs the topological changes necessary to extrude the
   indicated edges. (The geometric component is handled later by the Bevel
   method.)
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray eset</b>\n\n
   The edges to extrude.\n\n
   <b>Tab\<Point3\> *edir=NULL</b>\n\n
   Fills in the directions for moving all the relevant vertices to handle the
   geometric part of the extrusion. See the Edit Mesh or Editable Mesh source
   in <b>MAXSDK/SAMPLES/MODIFIERS</b> and
   <b>MAXSDK/SAMPLES/MESH/EDITABLEMESH</b> to see how this is used. */
   DllExport void ExtrudeEdges (Mesh & m, BitArray eset, Tab<Point3> *edir=NULL);
   /*! \remarks Performs the topological changes necessary to extrude the
   indicated faces. (The geometric component is handled later by the Bevel
   method.)
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray fset</b>\n\n
   The faces that should be extruded.\n\n
   <b>AdjEdgeList *el=NULL</b>\n\n
   A pointer to the adjacent edge list for this mesh. If NULL, the method
   constructs its own AdjEdgeList. */
   DllExport void ExtrudeFaces (Mesh & m, BitArray fset, AdjEdgeList *el=NULL);
   /*! \remarks This method resets the vertex corners.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on. */
   DllExport void ResetVertCorners (Mesh & m);  // DO NOT USE.  Not relevant.
   /*! \remarks Resets all the vertex weights to 1.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on. */
   DllExport void ResetVertWeights (Mesh & m);
   /*! \remarks Sets the indicated faces to have face alpha.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray fset</b>\n\n
   The faces that should be affected.\n\n
   <b>float alpha</b>\n\n
   The amount of alpha\n\n
   <b>int mp=MAP_ALPHA</b>\n\n
   The map channel. Use 0 for normal vertex colors, MAP_SHADING for the
   illumination channel, and MAP_ALPHA for the alpha channel. */
   void SetFaceAlpha (Mesh &m, BitArray fset, float alpha, int mp=MAP_ALPHA) { SetFaceColors (m, fset, UVVert(alpha,alpha,alpha), mp); }
   /*! \remarks Sets the indicated vertices to have face alpha.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray vset</b>\n\n
   The vertices that should be affected.\n\n
   <b>float alpha</b>\n\n
   The amount of alpha\n\n
   <b>int mp=MAP_ALPHA</b>\n\n
   The map channel. Use 0 for normal vertex colors, MAP_SHADING for the
   illumination channel, and MAP_ALPHA for the alpha channel. */
   void SetVertAlpha (Mesh &m, BitArray vset, float alpha, int mp=MAP_ALPHA) { SetVertColors (m, vset, UVVert(alpha,alpha,alpha),mp); }
   /*! \remarks Sets the indicated faces to have vertex colors all equal to
   the color value specified. (This often involves creating new vertex color
   map vertices, so faces that neighbor the indicated faces are not affected.)
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray fset</b>\n\n
   The faces that should be affected.\n\n
   <b>VertColor vc</b>\n\n
   The desired color.\n\n
   <b>int mp=0</b>\n\n
   The map channel. Use 0 for normal vertex colors, MAP_SHADING for the
   illumination channel, and MAP_ALPHA for the alpha channel. */
   DllExport void SetFaceColors (Mesh &m, BitArray fset, VertColor vc, int mp=0);
   /*! \remarks Sets all vertex color map vertices associated with the
   indicated vertices to the specified color.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray vset</b>\n\n
   The vertices that should be affected. If more than one map vertex is used at
   this vertex, all of them have their colors set.\n\n
   <b>VertColor vc</b>\n\n
   The desired color.\n\n
   <b>int mp=0</b>\n\n
   The map channel. Use 0 for normal vertex colors, MAP_SHADING for the
   illumination channel, and MAP_ALPHA for the alpha channel. */
   DllExport void SetVertColors (Mesh &m, BitArray vset, VertColor vc, int mp=0);
   DllExport void SetVertCorners (Mesh &m, BitArray vset, float corner);   // DO NOT USE: Not relevant.
   /*! \remarks Sets the weights of the specified vertices. (These weight
   values are only used in MeshSmooth NURMS mode as of 3ds Max 3.0.)
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray vset</b>\n\n
   The vertices that should have their weights set.\n\n
   <b>float weight</b>\n\n
   The weight to set. */
   DllExport void SetVertWeights (Mesh &m, BitArray vset, float weight);
	/*! \remarks "Turns" the specified edge. Only works on edges that have a face
	on both sides. These two faces are considered as a quad, where this edge is the
	diagonal, and remapped so that the diagonal flows the other way, between the
	vertices that were opposite this edge on each face.
	\par Parameters:
	<b>Mesh \& m</b>\n\n
	The mesh this MeshDelta should be based on.\n\n
	<b>DWORD ed</b>\n\n
	The edge to turn, indexed as face*3+side.\n\n
	<b>AdjEdgeList *ae=NULL</b>\n\n
	A pointer to the adjacent edge list for this mesh. If NULL, the method
	constructs its own AdjEdgeList. */
   DllExport DWORD TurnEdge (Mesh & m, DWORD ed, AdjEdgeList *el=NULL);
   /*! \remarks Welds all vertices that are sufficiently close together.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray vset</b>\n\n
   The vertices that are candidates for being welded.\n\n
   <b>float thresh</b>\n\n
   The maximum distance (in object space units) between two vertices that will
   allow them to be welded.
   \return  Returns TRUE if any vertices were welded, FALSE if none were within
   threshold. */
   DllExport BOOL WeldByThreshold (Mesh & m, BitArray vset, float thresh);
   /*! \remarks Welds the specified vertices together into one vertex, no
   matter how far apart they are.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray vset</b>\n\n
   The vertices that should be welded.\n\n
   <b>Point3 *weldPoint=NULL</b>\n\n
   If non-NULL, this points to the location we'd like to put the weld result.
   (If NULL, the result is put at the average location of the selected
   vertices.) */
   DllExport void WeldVertSet (Mesh & m, BitArray vset, Point3 *weldPoint=NULL);
   /*! \remarks This method is designed for internal use, in UnifyNormals, but
   may also be called directly.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray fset</b>\n\n
   The faces that should be affected.\n\n
   <b>int face</b>\n\n
   The starting face to propagate normal directions from.\n\n
   <b>AdjFaceList \& af</b>\n\n
   The adjacent face list corresponding to the mesh. Required, can't be made
   locally, for efficiency's sake.\n\n
   <b>BitArray \& done</b>\n\n
   Keeps track of which faces have had their normals unified. Faces that are
   set here when the call is made will not be processed, and will not be
   crossed to reach other faces. Faces that are still clear upon completion
   were not processed, probably because they were on a separate element from
   "face".\n\n
   <b>BOOL bias=1</b>\n\n
   Used to keep track of whether the current face has been oriented correctly.
   For example, if the starting face was not selected in fset, and therefore
   doesn't have the right orientation, but you still want selected faces in the
   same element to be corrected, you would submit FALSE here. */
   DllExport void PropagateFacing (Mesh & m, BitArray & fset, int face,
      AdjFaceList &af, BitArray &done,BOOL bias=1);
   /*! \remarks Unifies normals on selected faces, making the normals
   consistent from face to face.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>BitArray fset</b>\n\n
   The faces that should be affected. Nonselected faces can be traversed by the
   algorithm, but they will not be corrected if their normals are pointing the
   "wrong" way.\n\n
   <b>AdjFaceList *af=NULL</b>\n\n
   A pointer to the adjacent face list corresponding to the mesh. If NULL, an
   adjacent face list is computed by the method. */
   DllExport void UnifyNormals (Mesh & m, BitArray fset, AdjFaceList *af=NULL);

   // In slicer.cpp:
   /*! \remarks Slices the mesh along the specified slicing plane.
   \par Parameters:
   <b>Mesh \& m</b>\n\n
   The mesh this MeshDelta should be based on.\n\n
   <b>Point3 N</b>\n\n
   The normal of the slice plane.\n\n
   <b>float off</b>\n\n
   These parameters define the slicing plane as all points p satisfying the
   equation DotProd(p,N) = off. N should be normalized.\n\n
   <b>bool sep=FALSE</b>\n\n
   Indicates whether the slice should separate the mesh into two separate
   elements (if TRUE) or just refine the existing mesh by splitting faces (if
   FALSE).\n\n
   <b>bool remove=FALSE</b>\n\n
   Indicates whether the slice should remove the portion of the mesh "below"
   the slicing plane, where "below" is defined as the area where DotProd (p,N)
   - off \< 0. If remove is TRUE, sep is ignored.\n\n
   <b>BitArray *fslice=NULL</b>\n\n
   A bit array containing the list of faces to slice.\n\n
   <b>AdjEdgeList *ae=NULL</b>\n\n
   A pointer to the adjacent edge list for this mesh. If NULL, the method
   constructs its own AdjEdgeList. */
   DllExport void Slice (Mesh & m, Point3 N, float off, bool sep=FALSE, bool remove=FALSE, BitArray *fslice=NULL, AdjEdgeList *ae=NULL);
};

// Following classes provide standard interface for modifiers and objects that
// use mesh deltas -- specifically Edit Mesh and Editable Mesh for now.

enum meshCommandMode { McmCreate, McmAttach, McmExtrude, McmBevel, McmChamfer,
      McmSlicePlane, McmCut, McmWeldTarget, McmFlipNormalMode, McmDivide, McmTurnEdge,McmEditSoftSelection };
enum meshButtonOp    { MopHide, MopUnhideAll, MopDelete, MopDetach, MopBreak, MopViewAlign,
      MopGridAlign, MopMakePlanar, MopCollapse, MopTessellate, MopExplode, MopSlice, MopWeld,
      MopRemoveIsolatedVerts, MopSelectOpenEdges, MopCreateShapeFromEdges, MopShowNormal,
      MopFlipNormal, MopUnifyNormal, MopAutoSmooth, MopVisibleEdge, MopInvisibleEdge, MopAutoEdge,
      MopAttachList, MopSelectByID, MopSelectBySG, MopClearAllSG, MopSelectByColor,
      MopCopyNS, MopPasteNS, MopEditVertColor, MopEditVertIllum };
enum meshUIParam { MuiSelByVert, MuiIgBack, MuiIgnoreVis, MuiSoftSel, MuiSSUseEDist,
      MuiSSEDist, MuiSSBack, MuiWeldBoxSize, MuiExtrudeType, MuiShowVNormals,
      MuiShowFNormals, MuiSliceSplit, MuiCutRefine, // end of integer values
      MuiPolyThresh, MuiFalloff, MuiPinch, MuiBubble, MuiWeldDist, MuiNormalSize,
      MuiDeleteIsolatedVerts // MuiDeleteIsolatedVerts is an integer value, but also supports float
      };

#define EM_MESHUIPARAM_LAST_INT MuiShowFNormals // must specify last integer param
#define EM_SL_OBJECT 0
#define EM_SL_VERTEX 1
#define EM_SL_EDGE 2
#define EM_SL_FACE 3
#define EM_SL_POLYGON 4
#define EM_SL_ELEMENT 5

/*! \sa  Class Mesh, Class MeshDelta.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is a virtual class which both Editable Mesh (class EditTriObject) and Edit
Mesh (class EditMeshMod) subclass off of. It contains interface methods for
editable meshes, much like the IMeshSelect class contains interface methods for
modifiers and objects that can select sub-object parts of meshes.  */
class MeshDeltaUser : public InterfaceServer {
public:
   /*! \remarks This method is used to notify the <b>MeshDeltaUser</b> that at
   least one of its associated <b>MeshDeltaUserData</b>s has changed.
   \par Parameters:
   <b>DWORD parts</b>\n\n
   This represents the parts of the local data that have changed. One or more
   of the following values:\n\n
   <b>PART_TOPO, PART_GEOM, PART_TEXMAP, PART_MTL, PART_SELECT,
   PART_SUBSEL_TYPE, PART_DISPLAY, PART_VERTCOLOR, PART_GFX_DATA</b> */
   virtual void LocalDataChanged (DWORD parts)=0;
   // start or stop interactive command mode, uses mode enum above
   /*! \remarks This method is used to start up one of the interactive command
   modes of the editable mesh.
   \par Parameters:
   <b>meshCommandMode mode</b>\n\n
   The mode to start. One of the following values:\n\n
   <b>McmCreate, McmAttach, McmExtrude, McmBevel, McmChamfer, McmSlicePlane,
   McmCut, McmWeldTarget, McmFlipNormalMode, McmDivide, McmTurnEdge</b> */
   virtual void ToggleCommandMode (meshCommandMode mode)=0;
   // perform button op, uses op enum above
   /*! \remarks This method performs the equivalent operation as a button
   press in the editable mesh UI.
   \par Parameters:
   <b>meshButtonOp opcode</b>\n\n
   One of the following values:\n\n
   <b>MopHide, MopUnhideAll, MopDelete, MopDetach, MopBreak, MopViewAlign,
   MopGridAlign, MopMakePlanar, MopCollapse, MopTessellate, MopExplode,
   MopSlice, MopWeld, MopShowNormal, MopAutoSmooth, MopRemoveIsolatedVerts,
   MopSelectOpenEdges, MopCreateShapeFromEdges, MopFlipNormal, MopUnifyNormal,
   MopVisibleEdge, MopInvisibleEdge, MopAutoEdge, MopAttachList, MopSelectByID,
   MopSelectBySG, MopClearAllSG, MopSelectByColor</b> */
   virtual void ButtonOp (meshButtonOp opcode)=0;

   // UI controls access
   /*! \remarks This method allows you to get the edit spline parameters from the command
   panel. Currently not in use.
   \par Parameters:
   <b>meshUIParam uiCode</b>\n\n
   One of the following values;\n\n
   <b>MuiSelByVert, MuiIgBack, MuiIgnoreVis, MuiSoftSel, MuiSSUseEDist,
   MuiSSEDist, MuiSSBack, MuiWeldBoxSize, MuiExtrudeType, MuiShowVNormals,
   MuiShowFNormals, MuiPolyThresh, MuiFalloff, MuiPinch, MuiBubble,
   MuiWeldDist, MuiNormalSize</b>\n\n
   <b>int \&ret</b>\n\n
   The returned value.
   \par Default Implementation:
   <b>{ }</b> */
   virtual void GetUIParam (meshUIParam uiCode, int & ret) { }
   /*! \remarks This method allows you to set the edit spline parameters from the command
   panel. Currently not in use.
   \par Parameters:
   <b>meshUIParam uiCode</b>\n\n
   One of the following values;\n\n
   <b>MuiSelByVert, MuiIgBack, MuiIgnoreVis, MuiSoftSel, MuiSSUseEDist,
   MuiSSEDist, MuiSSBack, MuiWeldBoxSize, MuiExtrudeType, MuiShowVNormals,
   MuiShowFNormals, MuiPolyThresh, MuiFalloff, MuiPinch, MuiBubble,
   MuiWeldDist, MuiNormalSize</b>\n\n
   <b>int val</b>\n\n
   The value to set.
   \par Default Implementation:
   <b>{ }</b> */
   virtual void SetUIParam (meshUIParam uiCode, int val) { }
   /*! \remarks This method allows you to get the edit spline parameters from the command
   panel. Currently not in use.
   \par Parameters:
   <b>meshUIParam uiCode</b>\n\n
   One of the following values;\n\n
   <b>MuiSelByVert, MuiIgBack, MuiIgnoreVis, MuiSoftSel, MuiSSUseEDist,
   MuiSSEDist, MuiSSBack, MuiWeldBoxSize, MuiExtrudeType, MuiShowVNormals,
   MuiShowFNormals, MuiPolyThresh, MuiFalloff, MuiPinch, MuiBubble,
   MuiWeldDist, MuiNormalSize</b>\n\n
   <b>float \&ret</b>\n\n
   The returned value.
   \par Default Implementation:
   <b>{ }</b> */
   virtual void GetUIParam (meshUIParam uiCode, float & ret) { }
   /*! \remarks This method allows you to set the edit spline parameters from the command
   panel. Currently not in use.
   \par Parameters:
   <b>meshUIParam uiCode</b>\n\n
   One of the following values;\n\n
   <b>MuiSelByVert, MuiIgBack, MuiIgnoreVis, MuiSoftSel, MuiSSUseEDist,
   MuiSSEDist, MuiSSBack, MuiWeldBoxSize, MuiExtrudeType, MuiShowVNormals,
   MuiShowFNormals, MuiPolyThresh, MuiFalloff, MuiPinch, MuiBubble,
   MuiWeldDist, MuiNormalSize</b>\n\n
   <b>float val</b>\n\n
   The value to set.
   \par Default Implementation:
   <b>{ }</b> */
   virtual void SetUIParam (meshUIParam uiCode, float val) { }
   virtual void UpdateApproxUI () { }

   // Should work on any local command mode.
   virtual void ExitCommandModes ()=0;

   virtual bool Editing () { return FALSE; } // returns TRUE iff between BeginEditParams, EndEditParams
   virtual DWORD GetEMeshSelLevel () { return EM_SL_OBJECT; }
   virtual void SetEMeshSelLevel (DWORD sl) { }

   // access to EditTriObject method for creating point controllers
   virtual void PlugControllersSel(TimeValue t,BitArray &set) { }
};
#pragma warning(pop)
/*! \sa  Class Mesh, Class MeshDelta, Class MeshDeltaUser, Class LocalModData, Class Matrix3, Class Point3,  Class Quat,  Class BitArray.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class provides a standard interface for modifiers and objects that use
mesh deltas -- specifically Edit Mesh and Editable Mesh.\n\n
Both Edit Mesh and Editable Mesh have a current "state", which can be modified
by MeshDeltas. In Editable Mesh, this "state" is an actual mesh, while in Edit
Mesh, this is one <b>MeshDelta</b> per <b>LocalModData</b>. This class provides
a standard interface to these: "MeshDeltaUser" and "MeshDeltaUserData".  */
class MeshDeltaUserData: public MaxHeapOperators {
public:
   /*! \remarks This method applies the MeshDelta. Everything that happens in
   Edit Mesh and Editable Mesh goes through this method. Note that in Edit
   Mesh, the MeshDeltaUserData (EditMeshData) is separate from the
   MeshDeltaUser (EditMeshMod), though in Editable Mesh, EditTriObject
   subclasses from both of them.\n\n
   There's essentially one mesh that can be edited per
   <b>MeshDeltaUserData</b>, so <b>ApplyMeshDelta</b> is the way to edit that
   mesh. ApplyMeshDelta typically handles adding Restore objects (if
   theHold.Holding()), clearing out any temporary local caches that are
   invalidated, and notifying the pipeline that the mesh has changed.
   \par Parameters:
   <b>MeshDelta \&md</b>\n\n
   The mesh delta to apply.\n\n
   <b>MeshDeltaUser *mdu</b>\n\n
   Points to the mesh delta user.\n\n
   <b>TimeValue t</b>\n\n
   The time to apply the mesh delta. */
   virtual void ApplyMeshDelta (MeshDelta & md, MeshDeltaUser *mdu, TimeValue t)=0;
   /*! \remarks Returns a pointer to the MeshDelta object for this application
   of the Edit Mesh modifier. This is only non-NULL in Edit Mesh.
   \par Default Implementation:
   <b>{ return NULL; }</b> */
   virtual MeshDelta *GetCurrentMDState () { return NULL; } // only non-null in Edit Mesh
   // functional interface to mesh ops
   virtual void MoveSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin)=0;
   virtual void RotateSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin)=0;
   virtual void ScaleSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin)=0;
   virtual void ExtrudeSelection(int level, BitArray* sel, float amount, float bevel, BOOL groupNormal, Point3* direction)=0;
};

// Constants used in Edit(able) Mesh's shortcut table - THESE MUST BE MATCHED to values in Editable Mesh's resources.
#define EM_SHORTCUT_ID 0x38ba1366

#define MDUID_EM_SELTYPE                   40001
#define MDUID_EM_SELTYPE_BACK              40002
#define MDUID_EM_SELTYPE_VERTEX            40003
#define MDUID_EM_SELTYPE_EDGE              40004
#define MDUID_EM_SELTYPE_FACE              40005
#define MDUID_EM_SELTYPE_POLYGON           40006
#define MDUID_EM_SELTYPE_ELEMENT           40007
#define MDUID_EM_SELTYPE_OBJ               40008
#define MDUID_EM_AUTOSMOOTH                40009
#define MDUID_EM_ATTACH                    40010
#define MDUID_EM_BREAK                     40011
#define MDUID_EM_IGBACK                    40012
#define MDUID_EM_BEVEL                     40013
#define MDUID_EM_CREATE                    40014
#define MDUID_EM_CUT                       40015
#define MDUID_EM_DIVIDE                    40016
#define MDUID_EM_EXTRUDE                   40017
#define MDUID_EM_FLIPNORM                  40018
#define MDUID_EM_SS_BACKFACE               40019
#define MDUID_EM_UNIFY_NORMALS             40020
#define MDUID_EM_HIDE                      40021
#define MDUID_EM_EDGE_INVIS                40022
#define MDUID_EM_IGNORE_INVIS                 40023
#define MDUID_EM_IGNORE_INVIS              40023
#define MDUID_EM_COLLAPSE                  40024
#define MDUID_EM_SHOWNORMAL                40025
#define MDUID_EM_SELOPEN                   40026
#define MDUID_EM_REMOVE_ISO                40027
#define MDUID_EM_SLICEPLANE                40028
#define MDUID_EM_SOFTSEL                   40029
#define MDUID_EM_SLICE                     40030
#define MDUID_EM_DETACH                    40031
#define MDUID_EM_TURNEDGE                  40032
#define MDUID_EM_UNHIDE                    40033
#define MDUID_EM_EDGE_VIS                  40034
#define MDUID_EM_SELBYVERT                 40035
#define MDUID_EM_AUTOEDGE                  40036
#define MDUID_EM_WELD                      40038
#define MDUID_EM_EXPLODE                   40039
#define MDUID_EM_CHAMFER                   40040
#define MDUID_EM_WELD_TARGET                     40041
#define MDUID_EM_ATTACH_LIST                     40042
#define MDUID_EM_VIEW_ALIGN                         40043
#define MDUID_EM_GRID_ALIGN                      40044
#define MDUID_EM_SPLIT                              40045
#define MDUID_EM_REFINE_CUTENDS                  40046
#define MDUID_EM_COPY_NAMEDSEL                   40047
#define MDUID_EM_PASTE_NAMEDSEL                  40048
#define MDUID_EM_MAKE_PLANAR                     40049
#define MDUID_EM_VERT_COLOR                         40050
#define MDUID_EM_VERT_ILLUM                         40051
#define MDUID_EM_FLIP_NORMAL_MODE                40052

/*! \remarks This global function is available in release 3.0 and later
only.\n\n
Finds a triangulation of an n-sided polygon using vertices in the specified
mesh. As long as the vertices are coplanar, this algorithm will find a proper
triangulation, even for nonconvex polygons.
\par Parameters:
<b>Mesh \&m</b>\n\n
The mesh containing the vertices used in the polygon.\n\n
<b>int deg</b>\n\n
The size of the polygon.\n\n
<b>int *vv</b>\n\n
The vertex indices of the polygon, in order around the perimeter. For instance,
if deg is 5 and w points to an array containing (3, 6, 8, 0, 7), the polygon is
presumed to have the outline described by m.verts[3], m.verts[6], m.verts[8],
m.verts[0], and m.verts[7].\n\n
<b>int *tri</b>\n\n
This is where the output is placed. Note that this should point to an array of
size at least (deg-2)*3, to hold all the triangles. The values placed in this
array are indices into the w array -- that is, given a 5-sided polygon, one
triangle in this list might be (0,2,3), indicating you should use the 0th, 2nd,
and 3rd elements of w to form the triangle. Put another way, to make a face
from the n'th triangle given by this array, you would set:\n\n
<b>f.v[0] = w[tri[n*3+0]];</b>\n\n
<b>f.v[1] = w[tri[n*3+1]];</b>\n\n
<b>f.v[2] = w[tri[n*3+2]];</b> */
DllExport void FindTriangulation (Mesh & m, int deg, int *vv, int *tri);


