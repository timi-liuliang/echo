/**********************************************************************
 *<
	FILE: meshadj.h

	DESCRIPTION: Adjacency list for meshes.

	CREATED BY: Rolf Berteig

	HISTORY:	Extended for Shiva by: Steve Anderson

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include <WTypes.h>
#include "maxheap.h"
#include "export.h"
#include "point3.h"
#include "TabTypes.h"
#include "mesh.h"

// forward declarations
class Face;
class AdjFaceList;
class BitArray;
class Mesh;

#ifndef UNDEFINED
#define UNDEFINED	0xffffffff
#endif



/*! \sa  Class AdjEdgeList, Class Mesh.\n\n
\par Description:
This class describes a single edge of a mesh object that is adjacent to a
vertex. This is an edge that is coming out of the vertex. This is used in
adjacency lists.
\par Data Members:
<b>DWORD f[2];</b>\n\n
The indices into the meshes face table of the two faces that share this
edge.\n\n
<b>DWORD v[2];</b>\n\n
The indices into the meshes vertex table of the two vertices of this edge. <br>
*/
class MEdge: public MaxHeapOperators {
public:
	DWORD f[2];
	DWORD v[2];

	/*! \remarks Returns the index of the edge in the face on side
	<b>side</b>. So: given a Mesh mesh and an MEdge *me, int eid =
	me-\>EdgeIndex (mesh.faces, 0); then mesh.faces[me-\>f[0]].v[eid] and
	mesh.faces[me-\>f[0]].v[(eid+1)%3] are the endpoints of the edge.\n\n
	In particular, mesh.edgeSel[me-\>f[0]*3+eid] tells whether this edge is
	selected.
	\par Parameters:
	<b>Face *faces</b>\n\n
	The list of faces from the mesh.\n\n
	<b>int side</b>\n\n
	Either 0 or 1, indicating whether we should find this result for the face
	on side 0 or on side 1. */
	DllExport int EdgeIndex(Face *faces,int side);
	/*! \remarks Returns TRUE if this edge is selected on either side; or
	FALSE if it is not selected on either.
	\par Parameters:
	<b>Face *faces</b>\n\n
	The list of faces from the mesh.\n\n
	<b>BitArray \&esel</b>\n\n
	The edge selection BitArray from the mesh. */
	DllExport BOOL Selected (Face *faces,BitArray &esel);
	/*! \remarks Returns TRUE if this edge is visible on either side; or FALSE
	if it is not visible on either.
	\par Parameters:
	<b>Face *faces</b>\n\n
	The list of faces from the mesh. */
	DllExport BOOL Visible (Face *faces);
	/*! \remarks Returns TRUE if all the faces using this edge are hidden;
	otherwise FALSE.
	\par Parameters:
	<b>Face *faces</b>\n\n
	The list of faces from the mesh. */
	DllExport BOOL Hidden (Face *faces);
	/*! \remarks Returns the center of this edge.
	\par Parameters:
	<b>Point3 *verts</b>\n\n
	The list of vertices from the mesh. */
	DllExport Point3 Center(Point3* verts);
	/*! \remarks Returns TRUE if one (or both) of the faces sharing the edge
	is selected; otherwise FALSE.
	\par Parameters:
	<b>BitArray \&fsel</b>\n\n
	The face selection bit array. */
	DllExport BOOL AFaceSelected(BitArray &fsel);
	/*! \remarks Returns a point suitable for use in standard tessellation.
	\par Parameters:
	<b>Mesh *mesh</b>\n\n
	A pointer to the associated mesh.\n\n
	<b>AdjFaceList *af</b>\n\n
	A pointer to the associated AdjFaceList.\n\n
	<b>float tens</b>\n\n
	The tension parameter, as seen in the Tessellate modifier. */
	DllExport Point3 ButterFlySubdivide (Mesh *mesh,AdjFaceList *af,float tens);
	/*! \remarks	This method creates a map vertex for the middle of this edge using the
	Butterfly tessellation scheme. Designed to create map vertices to go with
	the vertex created by <b>ButterFlySubdivide()</b>.
	\par Parameters:
	<b>Mesh *mesh</b>\n\n
	A pointer to the associated mesh.\n\n
	<b>AdjFaceList *af</b>\n\n
	A pointer to the associated AdjFaceList.\n\n
	<b>float tens</b>\n\n
	The tension parameter, as seen in the Tessellate modifier.\n\n
	<b>int mp</b>\n\n
	The map channel we want to get a result for.\n\n
	<b>bool \& seam</b>\n\n
	If this is set to true by the algorithm, then there's a mapping seam on
	this edge, and different map vertices should be used on each side.\n\n
	<b>UVVert \& side2</b>\n\n
	If there's a seam, this contains the mapping result for the second side.
	The return value matches the mapping scheme on face f[0]; side2 matches the
	scheme on face f[1].
	\return  The desired mapping coordinates. */
	DllExport UVVert ButterFlyMapSubdivide (Mesh *mesh,AdjFaceList *af,float tens, int mp, bool & seam, UVVert & side2);
	/*! \remarks Returns the index of the other vertex using this edge.
	\par Parameters:
	<b>DWORD vv</b>\n\n
	The index of a vertex using the edge. */
	DWORD OtherVert (DWORD vv) { return (v[0] == vv) ? v[1] : v[0]; }
	/*! \remarks Returns the index of the other face using this edge.
	\par Parameters:
	<b>DWORD ff</b>\n\n
	The index of a face using this edge. */
	DWORD OtherFace (DWORD ff) { return (f[0] == ff) ? f[1] : f[0]; }
};

/*! \sa  Class DWORDTab, Template Class Tab, Class MEdge,  Class Mesh.\n\n
\par Description:
This class represents an edge adjacency list for meshes. For any given vertex
in a mesh this class has a table of DWORDs. These DWORDs are indices into the
edge table (<b>Tab\<MEdge\> edge</b>). The <b>edges</b> table stores the edges
adjacent to the vertex. So, each vertex has a list of indices into the edge
list that give it the list of edges adjacent to the vertex. All methods of the
class are implemented by the system.
\par Data Members:
<b>DWORDTab *list;</b>\n\n
This is an array of DWORDTabs, one per vertex. The Tab is a list of indices
into the edge list, one for each edge adjacent to the vertex.\n\n
<b>Tab\<MEdge\> edges;</b>\n\n
The table of edges.\n\n
<b>int nverts;</b>\n\n
The size of <b>list</b>.  */
class AdjEdgeList: public MaxHeapOperators {
public:
	DWORDTab *list; 	// 1 DWORDTab per vertex. The Tab is a list of indices into the edge list, 1 for each edge adjacent to the vertex
	Tab<MEdge> edges;	// Table of edges
	int nverts;			// size of 'list'.

	/*! \remarks Constructor. Builds an adjacency list from the specified
	mesh. This class require the mesh to be constructed so that each edge has
	exactly one or two faces.
	\par Parameters:
	<b>Mesh\& amesh</b>\n\n
	The mesh to build the adjacency list from. */
	DllExport AdjEdgeList(Mesh& amesh);
	/*! \remarks Destructor. Deletes the list. */
	DllExport ~AdjEdgeList();

	/*! \remarks This is used internally.
	\par Operators:
	*/
	DllExport void AddEdge( DWORD fi, DWORD v1, DWORD v2 );
	/*! \remarks Array access operator. Returns the 'i-th' list element. */
	DWORDTab& operator[](int i) { return list[i]; }
	/*! \remarks Finds the edge in the edge table that has the two specified
	vertices.
	\par Parameters:
	<b>DWORD v0, DWORD v1</b>\n\n
	The vertices.
	\return  The index into the edge table. */
	DllExport int FindEdge(DWORD v0, DWORD v1);
	/*! \remarks This method is used internally as part of the Optimize
	modifier. */
	DllExport int FindEdge(DWORDTab& vmap,DWORD v0, DWORD v1);		
	/*! \remarks This method is used internally as part of the Optimize
	modifier. */
	DllExport void TransferEdges(DWORD from,DWORD to,DWORD except1,DWORD except2,DWORD del);
	/*! \remarks This method is used internally as part of the Optimize
	modifier. */
	DllExport void RemoveEdge(DWORD from,DWORD e);
	/*! \remarks	This method simply calls <b>OrderVertEdges()</b> on all the vertices.
	\par Parameters:
	<b>Face *faces</b>\n\n
	A pointer to the faces for this mesh. */
	DllExport void OrderAllEdges (Face *faces);	// only affects order in each vert's list.
	/*! \remarks	Each vertex has a list of edges in the data member AdjEdgeList::list. This
	method reorders the elements of that list so that the edges are in order
	going around the vertex. The direction should be counterclockwise as seen
	from outside the mesh surface, though this necessarily breaks down with
	some rats' nest situations.
	\par Parameters:
	<b>DWORD v</b>\n\n
	The vertex whose edges should be ordered.\n\n
	<b>Face *faces</b>\n\n
	A pointer to the faces for this mesh.\n\n
	<b>Tab\<DWORD\> *flist=NULL</b>\n\n
	If non-NULL, this points to an array where the faces using this vertex
	should be stored (in order). */
	DllExport void OrderVertEdges (DWORD v, Face *faces, Tab<DWORD> *flist=NULL);
	/*! \remarks	This places a list of all faces using this vertex in flist. The faces are
	in no particular order.
	\par Parameters:
	<b>DWORD v</b>\n\n
	The vertex to check.\n\n
	<b>Tab\<DWORD\> \& flist</b>\n\n
	The table of faces. */
	DllExport void GetFaceList (DWORD v, Tab<DWORD> & flist);

	DllExport void MyDebugPrint ();

	/*! \remarks	Appends the specified edge to the specified vertex.
	\par Parameters:
	<b>DWORD v</b>\n\n
	The vertex the edge is added to.\n\n
	<b>DWORD e</b>\n\n
	The edge to add. */
	void AddEdgeToVertex(DWORD v,DWORD e) { list[v].Append (1, &e); }
};

/*! \sa  Class AdjFaceList.\n\n
\par Description:
This class describes a single face for use in AdjFaceLists.
\par Data Members:
<b>DWORD f[3];</b>\n\n
The indices of the faces adjacent to this one. These are indices into the mesh
face table.  */
class AdjFace: public MaxHeapOperators {
public:
	DWORD f[3];
	/*! \remarks Constructor. The face indices are set to <b>UNDEFINED</b>. */
	AdjFace() {f[0] = f[1] = f[2] = UNDEFINED;}
};

/*! \sa  Class AdjFace, Class AdjEdgeList.\n\n
\par Description:
This class represents a face adjacency list for meshes. This class require the
mesh to be constructed so that each edge has exactly one or two faces. It will
work with other meshes but may give misleading results (developers may wish to
call <b>mesh.RemoveDegenerateFaces()</b> to attempt to correct the mesh to work
with this class). All methods of this class are implemented by the system.
\par Data Members:
<b>Tab\<AdjFace\> list;</b>\n\n
The table of adjacent faces, one for each face.  */
class AdjFaceList: public MaxHeapOperators {
public:
	Tab<AdjFace> list;

	/*! \remarks Access operator. Returns the set of adjacent face for the
	i-th face. */
	AdjFace& operator[](int i) {return list[i];}
	/*! \remarks Constructor. The mesh and edge list passed define the mesh
	used to construct the face list. */
	DllExport AdjFaceList(Mesh& mesh,AdjEdgeList& el);
};

/*! \sa  Class AdjEdgeList, Class AdjFaceList.\n\n
\par Description:
This class may be used to access the various elements that compose a mesh.
Given a Mesh object <b>mesh</b>, the calls below allow a developer to access
the element number for a particular face, and the total number of elements in
the mesh.\n\n
<b>AdjEdgeList ae(mesh);</b>\n\n
<b>AdjFaceList af(mesh, ae);</b>\n\n
<b>FaceElementList elem(mesh, af);</b>\n\n
All methods of this class are implemented by the system.
\par Data Members:
<b>DWORDTab elem;</b>\n\n
This gives the element number for a particular face, i.e. <b>elem[i]</b> gives
the element number for face <b>i</b>.\n\n
<b>DWORD count;</b>\n\n
The total number of elements.  */
class FaceElementList: public MaxHeapOperators {
public:
	// For each face, which element is it in
	DWORDTab elem;
	DWORD count;
	/*! \remarks Constructor.
	\par Parameters:
	<b>Mesh \&mesh</b>\n\n
	The mesh that the element list is being built for.\n\n
	<b>AdjFaceList\& af</b>\n\n
	The face list for the mesh.
	\par Operators:
	*/
	DllExport FaceElementList(Mesh &mesh,AdjFaceList& af);
	/*! \remarks Accesses the i-th element of the table. */
	DWORD operator[](int i) {return elem[i];}
};

/*! \sa  Class AdjFaceList, Class BitArray.\n\n
\par Description:
This is a list of face "clusters" for a given mesh. A typical application would
be in Edit(able) Mesh, where the user has selected two separate groups of faces
on different parts of the mesh and wants to extrude them both, or rotate both
around their local centers. Each "cluster" is a contiguous group of selected
faces. Like AdjEdgeLists and AdjFaceLists, this class is only defined in
relation to some mesh.\n\n
This class may be used to group faces together based on the angle between their
normals or by their selection status.\n\n
All methods of this class are implemented by the system. Note that the
functionality provided by this class is not available in the 1.0 release of the
SDK. Later releases (1.1, 1.2, etc) do support it.
\par Data Members:
<b>DWORDTab clust;</b>\n\n
The cluster number (id), one for each face. Non-selected faces have
<b>UNDEFINED</b> for their id.\n\n
The cluster IDs of all the faces -- this table has size <b>mesh::numFaces</b>.
<b>clust[i]</b> is <b>UNDEFINED</b> if face <b>i</b> is not in any cluster (ie
is unselected).\n\n
<b>DWORD count;</b>\n\n
The number of clusters.  */
class FaceClusterList: public MaxHeapOperators {
public:
	// Cluster #, one for each face - non-selected faces have UNDEFINED for their id.
	DWORDTab clust;
	DWORD count;

	// This version separates cluster also using a minimum angle and optionally the selection set
	/*! \remarks Constructor. This version separates clusters using a minimum
	angle and optionally the selection set. A developer creates one of these
	cluster lists by specifying the mesh, the face list and an angle. What is
	built is a cluster number for each face identifying what cluster it is
	in.\n\n
	For example, if you create one of these for a sphere and set the angle
	threshold to 90 degrees, you would get back one cluster, and the cluster id
	for everything would be 0. If you ran it on a box, and you set the angle to
	\< 90 degrees, you would get back 6 ids. Two faces in the box would have id
	0, two would have id 1, etc.
	\par Parameters:
	<b>Mesh *mesh</b>\n\n
	The mesh to create the list for.\n\n
	<b>AdjFaceList\& adj</b>\n\n
	The face list for this mesh.\n\n
	<b>float angle</b>\n\n
	The maximum angle (in radians) that can be used in joining adjacent faces
	into the same cluster.\n\n
	<b>BOOL useSel</b>\n\n
	If FALSE, selection is ignored and all faces are grouped into clusters by
	angle. If TRUE, only selected faces are grouped into clusters, but angle is
	still relevant. Non-selected faces will have <b>UNDEFINED</b> for their id.
	*/
	DllExport FaceClusterList(Mesh *mesh, AdjFaceList& adj,float angle,BOOL useSel=TRUE);		
	// Uses selection set
	/*! \remarks Constructor. This version separates clusters using the
	selection set. In this case a cluster is defined as a set of faces that are
	selected and are adjacent. For example you could have a sphere with some
	faces selected on one side, and another group of faces selected on the
	other side. Each group of adjacent and selected faces would comprise
	clusters within the mesh. This is used for example by the axis tripods in
	3ds Max where each selected group of faces gets their own coordinate
	system.\n\n
	In this case the unselected faces will not be in any cluster. These store
	the value <b>UNDEFINED</b> for their id.
	\par Parameters:
	<b>BitArray\& fsel</b>\n\n
	This bit array defines the face selected state that the clusters will be
	grouped by. Each bit in the bit array corresponds to the parallel index in
	the mesh face table.\n\n
	<b>AdjFaceList\& adj</b>\n\n
	The face list for this mesh. */
	DllExport FaceClusterList (BitArray& fsel, AdjFaceList& adj);		
	/*! \remarks Access operator. Returns the cluster ID for face <b>i</b>. */
	DWORD operator[](int i) { return clust[i]; }
	/*! \remarks	Creates a list of cluster IDs for vertices.
	\par Parameters:
	<b>Mesh \&mesh</b>\n\n
	The mesh associated with this FaceClusterList.\n\n
	<b>Tab\<DWORD\> \&vclust</b>\n\n
	This is where the output goes: vclust is set to size mesh.numVerts, and the
	value of each entry in this table tells which cluster the vertex has been
	assigned to, based on the faces it's on. If vertex "v" is not in any
	clusters (ie none of the faces that use it are in any clusters), vclust[v]
	is UNDEFINED.\n\n
	In cases where a vertex is in two clusters, the larger face index is
	dominant. (In other words, if a vertex 6 is on faces 2 and 7, which are in
	two separate clusters, and face 9, which isn't in any cluster, it gets its
	cluster ID from face 7. This can happen if two selection regions touch at a
	vertex instead of along an edge.) */
	DllExport void MakeVertCluster(Mesh &mesh, Tab<DWORD> & vclust);
	/*! \remarks	Computes average normals and centers for all face clusters. Within a
	cluster, normals are weighted by the area of the face -- a face twice as
	big contributes twice as much to the cluster normal. (Mathematically, we
	just total up the non-normalized cross-products of each face, which are
	equivalent to 2*(area)*(face normal). Then we normalize the cluster total.)
	Face centers are directly averaged, without weighting.
	\par Parameters:
	<b>Mesh \&mesh</b>\n\n
	The mesh associated with this FaceClusterList.\n\n
	<b>Tab\<Point3\> \&norm</b>\n\n
	The average normal table to store the results in. This is set to size
	FaceClusterList::count, the number of clusters.\n\n
	<b>Tab\<Point3\> \&ctr</b>\n\n
	The average center table to store the results in. This is set to size
	FaceClusterList::count, the number of clusters. */
	DllExport void GetNormalsCenters (Mesh &mesh, Tab<Point3> & norm, Tab<Point3> & ctr);
	/*! \remarks	Each face cluster is a set of faces connected by shared edges. This method
	finds a cluster's boundary, which can be expressed as a sequence of edges
	on faces in the cluster (where the other side of each edge has no face or
	has a face not in this cluster). If there is more than one boundary, as for
	instance in the faces that make up the letter "o" in a ShapeMerge with
	Text, both boundaries are returned in no particular order.
	\par Parameters:
	<b>DWORD clustID</b>\n\n
	The cluster to get the border of.\n\n
	<b>AdjFaceList \&af</b>\n\n
	The adjacent face list associated with this FaceClusterList.\n\n
	<b>Tab\<DWORD\> \&cbord</b>\n\n
	The table where the output goes. If there are no borders (as for instance
	in a sphere with all faces selected), it remains empty. Otherwise, this is
	filled with a series of edge indices, then an UNDEFINED to mark the end of
	each border. So for instance if this cluster represents the front face of a
	default box, cbord will contain 4 edge indices and an UNDEFINED. If the
	cluster represents all the side faces of a cylinder, but not the top or
	bottom, there are two borders: on 24-sided cylinder, you'd get the 24 edge
	indices representing the bottom lip of the cylinder, then an UNDEFINED,
	then the 24 edge indices representing the top lip, followed by another
	UNDEFINED. (As elsewhere, edges are indexed by face*3+eid, where face is
	the face (in the cluster) the edge is on, and eid is the index of the edge
	on that face.) */
	DllExport void GetBorder (DWORD clustID, AdjFaceList & af, Tab<DWORD> & cbord);
	/*! \remarks	This creates "outline" directions for the vertices on the edge of the
	clusters. These are used in Edit(able) Mesh's new "Bevel" operation (when
	you Bevel by "Group"). These vectors, which are all perpendicular to the
	cluster normals, point in the direction and speed vertices must travel in
	order to move the edges out at a consistent rate without changing the shape
	of the outline of the cluster.\n\n
	To see how this works, create a Prism in 3ds Max ("Extended Primitives")
	with dimensions like 20 x 40 x 40 x 40, and apply an Edit Mesh. Select all
	the faces on the top of the prism, and spin the Bevel spinner up and down.
	Notice that the vertex at the sharpest point moves faster than the other 2,
	but that the edges all remain parallel to their original positions. The
	essence of Outlining is that the edges move at a constant rate, and the
	vertices move faster or slower to make this happen. (This strategy is also
	used in the Bevel and Path Bevel modifiers.)
	\par Parameters:
	<b>Mesh \& m</b>\n\n
	The mesh associated with this FaceClusterList\n\n
	<b>AdjFaceList \&af</b>\n\n
	The adjacent face list associated with this FaceClusterList\n\n
	<b>Tab\<Point3\> \&cnorms</b>\n\n
	The cluster normals, as computed by GetNormalsCenters\n\n
	<b>Tab\<Point3\> \&odir</b>\n\n
	A table to put the outline direction result in. This is set to size
	mesh.numVerts. Entries for vertices that are not on a cluster border are
	all (0,0,0). Entries for cluster border vertices are scaled, such that if
	you move all vertices the specified amount, each cluster's border edges
	will move by one 3ds Max unit.
	\par Operators:
	*/
	DllExport void GetOutlineVectors (Mesh & m, AdjFaceList & af, Tab<Point3> & cnorms, Tab<Point3> & odir);
};

/*! \sa  Class Mesh.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is a list of edge "clusters" for a given mesh. A typical application would
be in Edit(able) Mesh, where the user has selected a two separate groups of
edges on different parts of the mesh and wants to extrude them both, or rotate
both around their local centers. Each "cluster" is a contiguous group of
selected edges. Like AdjEdgeLists and AdjFaceLists, this class is only defined
in relation to some mesh.\n\n
Note: for construction of this list, an edge is considered selected on both
sides if it's selected on either. If you select the diagonal on top of a box,
you probably only selected one of (face 2, edge 2 = 8) or (face 3, edge 2 =
11). But edges 8 and 11 will both be in the same cluster.
\par Data Members:
<b>DWORDTab clust;</b>\n\n
The cluster IDs of all the edges -- this table has size mesh::numFaces*3.
clust[i] is UNDEFINED if edge i is not in any cluster (ie is totally
unselected).\n\n
<b>DWORD count;</b>\n\n
The number of clusters.  */
class EdgeClusterList: public MaxHeapOperators {
public:
	DWORDTab clust;
	DWORD count;

	/*! \remarks Constructor.\n\n
	Creates an edge cluster list from the current selection. All adjacent
	selected edges are grouped into the same cluster.
	\par Parameters:
	<b>Mesh \&mesh</b>\n\n
	The mesh associated with this EdgeClusterList.\n\n
	<b>BitArray \&esel</b>\n\n
	The bit array containing the edge selection data for the mesh.\n\n
	<b>AdjEdgeList \&adj</b>\n\n
	The adjacent edge list for the mesh. */
	DllExport EdgeClusterList(Mesh &mesh,BitArray &esel,AdjEdgeList &adj);
	/*! \remarks Returns the cluster ID for face f, edge e.
	\par Parameters:
	<b>int f</b>\n\n
	The index of the face in the mesh.\n\n
	<b>int e</b>\n\n
	The index of the edge in the mesh. */
	DWORD ID(int f, int e) {return clust[f*3+e];}
	/*! \remarks Access operator. Returns the cluster ID for edge i (indexed
	as 3*face+edge). */
	DWORD operator[](int i) {return clust[i];}
	/*! \remarks Creates a list of cluster IDs for vertices.
	\par Parameters:
	<b>Mesh \&mesh</b>\n\n
	The mesh associated with this EdgeClusterList.\n\n
	<b>Tab\<DWORD\> \& vclust</b>\n\n
	This is where the output goes: vclust is set to size mesh.numVerts, and the
	value of each entry in this table tells which cluster the vertex has been
	assigned to, based on the edges it's on. If vertex "v" is not in any
	clusters (ie none of the edges that use it are in any clusters), vclust[v]
	is UNDEFINED. */
	DllExport void MakeVertCluster (Mesh &mesh, Tab<DWORD> & vclust);
	/*! \remarks This method extracts normal and center information for the
	various clusters of the mesh.
	\par Parameters:
	<b>Mesh \&mesh</b>\n\n
	The mesh to evaluate.\n\n
	<b>Tab\<Point3\> \& norm</b>\n\n
	This table has its sizes set to the number of clusters in the cluster list.
	Normals are computed as the normalized average of the area-normal vectors
	of all faces in the cluster.\n\n
	<b>Tab\<Point3\> \& ctr</b>\n\n
	This table has its sizes set to the number of clusters in the cluster list.
	Centers are the average location of the face centers or edge centers --
	thus a point on three faces or edges in the same cluster has more weight
	than a point on one face in the cluster. */
	DllExport void GetNormalsCenters (Mesh &mesh, Tab<Point3> & norm, Tab<Point3> & ctr);
};

/*! \sa  Class Mesh, Class MeshDelta, Template Class Tab.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class contains all the data needed to move points as the user drags a
chamfer. It's created by the topological change that happens at the start of
the chamfer. It is used to maintain chamfer information between several
<b>MeshDelta</b> methods. The strategy is this: The chamfer operation is
divided into two parts, the topological change and a later geometric change.
(This works well for EditableMesh, where the topology change is completed
first, then apply a series of geometry changes as the user spins a spinner or
drags a mouse. Each geometry change is undone before the next is applied, but
the topology change only happens once.)\n\n
This class is filled in by the topological change with the "directions" for all
the geometric and mapping vert changes:
\par Data Members:
<b>Tab\<Point3\> vdir;</b>\n\n
This table contains the directions of movement for each vertex, scaled in such
a manner to produce a consistent chamfer.\n\n
<b>Tab\<float\> vmax;</b>\n\n
This table contains the limits of motion for each vertex - stopping the
vertices at these limits will prevent them from crossing each other or over far
edges.\n\n
<b>Tab\<UVVert\> *mdir;</b>\n\n
For each active map channel <b>mp</b>, <b>mdir[mp]</b> represents the
directions of movement of the map verts for that channel. (Map verts need to be
moved as well, otherwise the maps get distorted.)  */
class MeshChamferData: public MaxHeapOperators {
	Tab<UVVert> hmdir[NUM_HIDDENMAPS];
public:
	Tab<Point3> vdir;
	Tab<float> vmax;
	Tab<UVVert> *mdir;

	/*! \remarks Constructor. This sets <b>mdir</b> to NULL. */
	MeshChamferData () { mdir=NULL; }
	/*! \remarks Constructor.
	\par Parameters:
	<b>const Mesh \&m</b>\n\n
	The mesh to init this MeshChamferData object from, allocating the vertex
	and mapping vertex tables as appropriate. */
	MeshChamferData (const Mesh & m) { mdir=NULL; InitToMesh(m); }
	/*! \remarks Destructor. If <b>mdir</b> is allocted it is seleted. */
	DllExport ~MeshChamferData ();

	/*! \remarks This method sets up a <b>MeshChamferData</b> based on a given
	mesh, allocating the vertex and mapping vertex tables as appropriate.
	\par Parameters:
	<b>const Mesh \&m</b>\n\n
	The Mesh to init from. */
	DllExport void InitToMesh (const Mesh & m);
	/*! \remarks This method simply allocates the vdir and vmax tables, and
	initializes the new members of vmax to 0. This method can be applied to an
	existing <b>MeshChamferData</b> to reflect an increase in vertices by
	VClone or VCreate operations.
	\par Parameters:
	<b>int nv</b>\n\n
	The number of verts\n\n
	<b>bool keep=TRUE</b>\n\n
	TRUE to keep if resized; FALSE to discard.\n\n
	<b>int resizer=0</b>\n\n
	The number of elements the vdir and vmax tables are resized beyond their
	current size.\n\n
	  */
	DllExport void setNumVerts (int nv, bool keep=TRUE, int resizer=0);
	Tab<UVVert> & MDir (int mp) { return (mp<0) ? hmdir[-1-mp] : mdir[mp]; }
};

// The following function has been added
// in 3ds max 4.2.  If your plugin utilizes this new
// mechanism, be sure that your clients are aware that they
// must run your plugin with 3ds max version 4.2 or higher.
/*! \remarks This function uses calls to DebugPrint() to output all the data
in the specified MeshChamferData to the DebugPrint buffer during debug runs. It
is available for programmers' use, providing easy access to MeshChamferData
during development. It ought to be removed for release builds.
\par Parameters:
<b>MeshChamferData \& mcd</b>\n\n
The MeshChamferData we want to investigate.\n\n
<b>int mapNum</b>\n\n
The number of map channels in the Mesh associated with this MeshChamferData.
(For historical reasons, this information is not kept in the MeshChamferData
class.) Generally this is retrieved with a call to Mesh::getNumMaps(). */
DllExport void MeshChamferDataDebugPrint (MeshChamferData & mcd, int mapNum);
// End of 3ds max 4.2 Extension

// following is never saved: it's a collection of all the temporary data you might want to cache about a mesh.
// Extrusion types:
#define MESH_EXTRUDE_CLUSTER 1
#define MESH_EXTRUDE_LOCAL 2

/*! \sa  Class Mesh, Class AdjEdgeList, Class FaceClusterList, Template Class Tab.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is a class for caching winged edge lists, face adjacency lists, face and
edge clusters, vertex normals, and other derived data about a mesh.\n\n
There is a <b>SetMesh()</b> method to set the current mesh that the TempData is
based on, then there's a series of methods to update the cache and return some
sort of derived data. All of these methods follow the form:\n\n
<b>DerivedData *MeshTempData::DData (parameters);</b>\n\n
DerivedData is the container for the derived data requested (often a simple
table, though there are some specialized classes returned from some methods).
If the data has already been computed, the parameters are ignored and the
cached data is returned. Otherwise, the data is computed from the parameters
and the current mesh.\n\n
There are no procedures in place to detect changes in parameters or the mesh
since the last time a method was called, so it's the calling routine's
responsibility to free invalid structures. If you know that only certain
pipeline channel, such as <b>GEOM_CHANNEL</b>, have changed, you can use the
<b>Invalidate(DWORD partsChanged)</b> method. (<b>GEOM_CHANNEL</b> would free
the distances-to-selected-vertices, for example, but not the <b>Adjacent Edge
List</b>.)\n\n
In particular, there is no way for the <b>MeshTempData</b> to know when its
mesh pointer is no longer valid, so it's vital that the calling routine clear
the mesh (with <b>SetMesh(NULL)</b>) or stop using the <b>MeshTempData</b> when
this happens.\n\n
All data members are private. They basically consist of a series of pointers
which are initialized to NULL and then filled with allocated derived data as
requested. There is also a NULL-initialized, private mesh pointer which is set
with <b>SetMesh()</b>.\n\n
Editable Mesh and Edit Mesh both use this class to hold all the varieties of
temporary, cached data they create -- examples are vertex normals and face
clusters. This is called "ETTempData" in Editable Mesh and "EMTempData" in Edit
Mesh.\n\n
To use MeshTempData, just set it to your mesh and start asking for stuff:\n\n
<b>MyAlgorithm (Mesh *m) {</b>\n\n
<b>MeshTempData mtd(m);</b>\n\n
<b>// Get Adjacent Edge List.</b>\n\n
<b>AdjEdgeList ae = mtd.AdjEList ();</b>\n\n
<b>}</b>
\par Method Groups:
See <a href="class_mesh_temp_data_groups.html">Method Groups for Class MeshTempData</a>.
*/
class MeshTempData : public BaseInterfaceServer {
private:
	AdjEdgeList *adjEList;
	AdjFaceList *adjFList;
	EdgeClusterList *edgeCluster;
	FaceClusterList *faceCluster;
	Tab<DWORD> *vertCluster;
	Tab<Point3> *normals;
	Tab<Point3> *centers;
	Tab<Point3> *vnormals;
	Tab<Tab<float> *> *clustDist;
	Tab<float> *selDist;
	Tab<float> *vsWeight;
	MeshChamferData *chamData;

	Tab<Point3> *extDir;
	Tab<Point3> *outlineDir;

	Mesh *mesh;

	//this is used to keep track of the selDist data.  We used to compute all the seldist data which
	//was really slow.  We now only track the selDist that fall within  this limit which can make things
	//much faster.
	float mFalloffLimit;

public:
	/*! \remarks Constructor. Sets all data members to NULL. */
	DllExport MeshTempData ();
	/*! \remarks Constructor. Sets the internal mesh pointer to the mesh
	passed.
	\par Parameters:
	<b>Mesh *m</b>\n\n
	The mesh to set. */
	DllExport MeshTempData (Mesh *m);
	/*! \remarks Destructor. Frees all cached data. */
	DllExport ~MeshTempData ();

	/*! \remarks Sets the internal mesh pointer to <b>m</b>.
	\par Parameters:
	<b>Mesh *m</b>\n\n
	Points to the mesh to set. */
	void SetMesh (Mesh *m) { mesh = m; }

	/*! \remarks Returns an adjacent edge list. See class <b>AdjEdgeList</b>
	for more information. If cached, the cache is returned. Otherwise a cache
	is allocated and computed from the current mesh. */
	DllExport AdjEdgeList *AdjEList ();
	/*! \remarks Returns an adjacent face list. See class <b>AdjFaceList</b>
	for more information. If cached, the cache is returned. Otherwise a cache
	is allocated and computed from the current mesh. */
	DllExport AdjFaceList *AdjFList ();
	/*! \remarks Returns a face cluster list, which groups selected faces into
	"clusters" for transformation. See class <b>FaceClusterList</b> for more
	information. If cached, the cache is returned. Otherwise a cache is
	allocated and computed from the current mesh. */
	DllExport FaceClusterList *FaceClusters ();
	/*! \remarks Returns an edge cluster list, which groups selected edges
	into "clusters" for transformation. See class <b>EdgeClusterList</b> for
	more information. If cached, the cache is returned. Otherwise a cache is
	allocated and computed from the current mesh. */
	DllExport EdgeClusterList *EdgeClusters ();
	/*! \remarks Returns an index of which cluster, if any, each vertex is in.
	If cached, the cache is returned. Otherwise a cache is allocated and
	computed from the current mesh and the parameter.
	\par Parameters:
	<b>DWORD sl</b>\n\n
	Selection level. This should be either <b>MESH_EDGE</b> or
	<b>MESH_FACE</b>, to indicate whether the vertex cluster information should
	be based on edge or face clusters. Note that this parameter is ignored if
	there's already a vertex cluster cache.
	\return  A table of DWORD's is returned, one for each vertex. If
	(*VertexClusters(sl))[i] is UNDEFINED, vertex i is not in any cluster.
	Otherwise, the value for vertex i is the cluster index. */
	DllExport Tab<DWORD> *VertexClusters (DWORD sl);
	/*! \remarks Returns average normals for each cluster. If cached, the
	cache is returned. Otherwise a cache is allocated and computed from the
	current mesh and the parameter. Note that cluster centers and normals are
	computed and cached at the same time, when you call either method.
	\par Parameters:
	<b>DWORD sl</b>\n\n
	Selection level. This should be either MESH_EDGE or MESH_FACE, to indicate
	whether the clusters we're talking about are the edge or face clusters.
	Note that this parameter is ignored if there's already a cluster normal
	cache.
	\return  A table of Point3's is returned, one for each cluster. The values
	are already normalized to length 1. */
	DllExport Tab<Point3> *ClusterNormals (DWORD sl);
	/*! \remarks Returns mean centers for each cluster. If cached, the cache
	is returned. Otherwise a cache is allocated and computed from the current
	mesh and the parameter. Note that cluster centers and normals are computed
	and cached at the same time, when you call either method.
	\par Parameters:
	<b>DWORD sl</b>\n\n
	Selection level. This should be either MESH_EDGE or MESH_FACE, to indicate
	whether the clusters we're talking about are the edge or face clusters.
	Note that this parameter is ignored if there's already a cluster center
	cache.
	\return  A table of Point3's is returned, one for each cluster. */
	DllExport Tab<Point3> *ClusterCenters (DWORD sl);
	/*! \remarks Uses the current cluster center and normal caches to return
	the "objectspace to clusterspace" transform. This is the tranform of the
	"local" axis in moving edge or face clusters in Edit(able) Mesh. If the
	cluster centers \& normals have not been cached, the identity matrix is
	returned; thus the control over whether this is an edge or face cluster is
	handled by the last call to ClusterCenters or ClusterNormals.
	\par Parameters:
	<b>int clust</b>\n\n
	The cluster you want the transform for. */
	DllExport Matrix3 ClusterTM (int clust);
	/*! \remarks Returns a table of local average normals for vertices. This
	is equivalent to the average normals computed by the standalone
	function:\n\n
	<b>void AverageVertexNormals(Mesh \& mesh, Tab\<Point3\> \&
	vnormals)</b>\n\n
	If cached, the cache is returned. Otherwise a cache is allocated and
	computed from the current mesh. */
	DllExport Tab<Point3> *VertexNormals ();
	/*! \remarks Returns Vertex Selection weights (for affect region). If
	cached, the cache is returned. Otherwise a cache is allocated and computed
	from the current mesh and the parameters. Weights are based on an Affect
	Region type falloff from the current selection.
	\par Parameters:
	<b>BOOL useEdgeDist</b>\n\n
	If useEdgeDist is TRUE, the distance between vertices is computed along
	edges. If FALSE, it's computed directly through space.\n\n
	<b>int edgeIts</b>\n\n
	This indicates the maximum number of edges the algorithm may travel along
	in finding the distance between vertices. (Maximum path length.)\n\n
	WARNING: If useEdgeDist is FALSE, this is an n-squared algorithm: it
	compares every vertex not in the cluster with every vertex in it. If
	useEdgeDist is TRUE, the time it takes is proportional to the number of
	verts in the cluster times edgeIts.\n\n
	<b>BOOL ignoreBack</b>\n\n
	If TRUE, vertices with a normal (as computed in VertexNormals) that points
	more than 90 degrees away from the average normal of theselection are not
	given any partial selections. They're either 1 if selected or 0
	otherwise.\n\n
	<b>float falloff</b>\n\n
	The limit distance of the effect. If distance \> falloff, the function will
	always return 0.\n\n
	<b>float pinch</b>\n\n
	Use this to affect the tangency of the curve near distance=0. Positive
	values produce a pointed tip, with a negative slope at 0, while negative
	values produce a dimple, with positive slope.\n\n
	<b>float bubble</b>\n\n
	Use this to change the curvature of the function. A value of 1.0 produces a
	half-dome. As you reduce this value, the sides of the dome slope more
	steeply. Negative values lower the base of the curve below 0.
	\return  Returns a table of float values, one per vertex, that are 1.0 if
	the vertex is in the current selection, 0.0 if it's more than falloff
	distance (or more than <b>edgeIts</b> edges, if (<b>useEdgeDist</b>)), and
	<b>AffectRegionFunction((*SelectionDist(useEdgeDist, edgeIts)), falloff,
	pinch, bubble)</b> otherwise. */
	DllExport Tab<float> *VSWeight (BOOL useEdgeDist, int edgeIts,
									BOOL ignoreBack, float falloff, float pinch, float bubble);

	//! \brief This computes the distances of the unselected vertices from the selected vertices
	/*!	Computes the current distance of each vertex from the current selection. If cached, the 
	    cache is returned. Otherwise a cache is allocated and computed from the current mesh and 
		the parameters. The term "Selected verts" below refers to the vertices that are selected 
		in the mesh's current selection level. (See the Mesh method GetTempSel for details.)
		\param[in] useEdgeDist If TRUE, the distance between vertices is computed along edges. If FALSE, it's computed directly through space. 
		\param[in] edgeIts This indicates the maximum number of edges the algorithm may travel 
					along in finding the distance between vertices. (Maximum path length.).
					WARNING: If useEdgeDist is FALSE, this is an n-squared algorithm: it compares 
					every nonselected vertex with every selected one within the falloffLimit. If useEdgeDist is TRUE, 
					the time it takes is proportional to the number of selected vertices times 
					edgeIts. 
		\param[in] falloffLimit = -1.0f this limits the number of unselected vertices to check against
		           Only vertices that fall within the selected vertice bounding box + this limit will be
				   computed.  If this value is -1.0 all vertcies will be computed
		\return A table consisting of one float value per vertex. If this value is 0, the vertex is either 
					selected or on top of a selected vertex. Otherwise it represents the distance to the closest 
					selected vertex. If useEdgeDist is TRUE, values of -1.0 are returned for vertices with no edgeIts-length 
					path to a selected vertex.
	*/
	DllExport Tab<float> *SelectionDist (BOOL useEdgeDist, int edgeIts, float falloffLimit = -1.0f);
	/*! \remarks Computes the current distance of each vertex from the
	specifed cluster. If cached, the cache is returned. Otherwise a cache is
	allocated and computed from the current mesh and the parameters.
	\par Parameters:
	<b>DWORD sl</b>\n\n
	Indicates whether we should use edges (MESH_EDGE) or faces (MESH_FACE) to
	construct the clusters, if needed.\n\n
	<b>int clustId</b>\n\n
	The index of the cluster we're weasuring distance from.\n\n
	<b>BOOL useEdgeDist</b>\n\n
	If useEdgeDist is TRUE, the distance between vertices is computed along
	edges. If FALSE, it's computed directly through space.\n\n
	<b>int edgeIts</b>\n\n
	This indicates the maximum number of edges the algorithm may travel along
	in finding the distance between vertices. (Maximum path length.)\n\n
	WARNING: If <b>useEdgeDist</b> is FALSE, this is an n-squared algorithm: it
	compares every vertex not in the cluster with every vertex in it. If
	useEdgeDist is TRUE, the time it takes is proportional to the number of
	verts in the cluster times edgeIts.
	\return  A table consisting of one float value per vertex. If this value is
	0, the vertex is either selected or on top of a vertex in the cluster.
	Otherwise it represents the distance to the closest selected vertex. If
	<b>useEdgeDist</b> is TRUE, values of -1.0 are returned for vertices with
	no edgeIts-length path to a vertex in the cluster. */
	DllExport Tab<float> *ClusterDist (DWORD sl, int clustId, BOOL useEdgeDist, int edgeIts);
	/*! \remarks Returns the direction each vertex should be going, after a
	topological edge extrusion, to handle the geometric extrusion. This should
	be obtained after applying a <b>MeshDelta::ExtrudeEdges()</b> to the mesh
	to obtain valid results. If cached, the cache is returned. Otherwise a
	cache is allocated and computed from the current mesh and the parameters.
	\par Parameters:
	<b>Tab\<Point3\> *edir</b>\n\n
	This should be the edge direction table filled out by
	MeshDelta::ExtrudeEdges. It is necessary.\n\n
	<b>int extrusionType</b>\n\n
	This is one of <b>MESH_EXTRUDE_CLUSTER</b> or <b>MESH_EXTRUDE_LOCAL</b>, to
	indicate whether vertices should move according to cluster or local face
	normals.
	\return  A table of Point3's, one per vertex, representing the direction
	each vertex should move for further extrusion. The size of each nonzero
	entry is set to 1. */
	DllExport Tab<Point3> *EdgeExtDir (Tab<Point3> *edir, int extrusionType);
	/*! \remarks Returns the direction each vertex should be going, after a
	topological face extrusion, to handle the geometric extrusion. This should
	be obtained after applying a MeshDelta::ExtrudeFaces to the mesh to obtain
	valid results. If cached, the cache is returned. Otherwise a cache is
	allocated and computed from the current mesh and the parameters.
	\par Parameters:
	<b>int extrusionType</b>\n\n
	This is one of <b>MESH_EXTRUDE_CLUSTER</b> or <b>MESH_EXTRUDE_LOCAL</b>, to
	indicate whether vertices should move according to cluster or local face
	normals.
	\return  A table of Point3's, one per vertex, representing the direction
	each vertex should move for further extrusion. The size of each nonzero
	entry is set to 1. */
	DllExport Tab<Point3> *FaceExtDir (int extrusionType);
	/*! \remarks This computes nothing; it merely returns the current
	extrusion direction cache, if any. The extrusion direction is controlled by
	the first call to EdgeExtDir or FaceExtDir since the last invalidation. If
	cached, the cache is returned. Otherwise a cache is allocated and computed
	from the current mesh and the parameters. */
	Tab<Point3> *CurrentExtDir () { return extDir; }
	/*! \remarks This produces the "Outline" direction of all vertices, based
	on the current face selection. "Outlining" is the direction vertices move
	to move edges of the current face selection outward at a constant rate.
	They are not set to length 1, but rather to whatever "rate" best makes the
	outline edges movemost consistently, without changing their angles.
	\par Parameters:
	<b>int extrusionType</b>\n\n
	This is one of MESH_EXTRUDE_CLUSTER or MESH_EXTRUDE_LOCAL, to indicate
	whether vertices should move according to cluster or local face normals. */
	DllExport Tab<Point3> *OutlineDir (int extrusionType);

	/*! \remarks Returns the cache of a ChamferData for use in the MeshDelta
	methods,\n\n
	<b>void ChamferEdges (Mesh \& m, BitArray eset, MeshChamferData \&mcd,
	AdjEdgeList *ae=NULL);</b>\n\n
	<b>void ChamferMove (Mesh \& m, MeshChamferData \&mcd, float amount,
	AdjEdgeList *ae=NULL);</b>\n\n
	<b>void ChamferVertices (Mesh \& m, BitArray vset, MeshChamferData \&mcd,
	AdjEdgeList *ae=NULL);</b>\n\n
	Unlike other MeshTempData methods, this method makes no calculations based
	on the current mesh, but merely supplies a memory cache. */
	DllExport MeshChamferData *ChamferData();

	/*! \remarks Invalidates all data based on the specified part of the mesh.
	In the following chart, the columns represent the channels GEOM_CHANNEL
	(G), TOPO_CHANNEL (T), SELECT_CHANNEL (S), and SUBSEL_TYPE_CHANNEL (U).\n\n
	X's indicate dependency of the specified data cache on the given
	channel.\n\n
	<b>Method to get cache G T S U</b>\n\n
	<b>AdjEList X</b>\n\n
	<b>AdjFList X</b>\n\n
	<b>FaceClusters X X</b>\n\n
	<b>EdgeClusters X X</b>\n\n
	<b>VertexClusters X X X</b>\n\n
	<b>ClusterCenters X X X X</b>\n\n
	<b>ClusterNormals X X X X</b>\n\n
	<b>VertexNormals X X</b>\n\n
	<b>SelectionDist X X X X</b>\n\n
	<b>ClusterDist X X X X</b>\n\n
	<b>VSWeight X X X X</b>\n\n
	The extrusion direction methods could also be said to be dependent on all
	four channels, but is currently handled separately in freeBevelInfo.
	ChamferData is handled in freeChamferData, and is not based on the\n\n
	cached mesh.\n\n
	Sample use: Suppose you use a MeshDelta to modify a mesh, twice:\n\n
	<b>DoStuffToMesh (Mesh \& m) {</b>\n\n
	<b>MeshTempData foo;</b>\n\n
	<b>foo.SetMesh (\&m);</b>\n\n
	<b>MeshDelta md(m);</b>\n\n
	<b>md.Op1 (m, foo.AdjEList());  // insert op of choice here</b>\n\n
	<b>md.Apply (m);</b>\n\n
	<b>foo.Invalidate (md.PartsChanged ());</b>\n\n
	<b>md.ClearAllOps ();</b>\n\n
	<b>md.Op2 (m, foo.VSWeights ());</b>\n\n
	<b>md.Apply (m);</b>\n\n
	<b>foo.Invalidate (md.PartsChanged ());</b>\n\n
	<b>}</b>\n\n
	Only the parts of foo that are dependent on what was changed by the first
	meshdelta are freed. The other parts, if any, remain cached for further
	operations.
	\par Parameters:
	<b>DWORD part</b>\n\n
	One or more of the following channels:\n\n
	<b>GEOM_CHANNEL, TOPO_CHANNEL, SELECT_CHANNEL, SUBSEL_TYPE_CHANNEL</b> */
	DllExport void Invalidate (DWORD part);
	/*! \remarks Uncaches (frees) the distance dependent data returned by
	VSWeight, SelectionDist, and ClusterDist. */
	DllExport void InvalidateDistances ();
	/*! \remarks Frees the VSWeight data. This is useful, e.g., if the mesh
	has not changed, but you wish to change the falloff, pinch, or bubble
	parameters to get new vertex selection weights. */
	DllExport void InvalidateAffectRegion ();
	/*! \remarks Mainly for internal use, this frees just the cluster distance
	data. */
	DllExport void freeClusterDist ();
	/*! \remarks Frees only the extrusion direction data. */
	DllExport void freeBevelInfo ();
	/*! \remarks Frees only the chamfer data structure. */
	DllExport void freeChamferData();
	/*! \remarks Frees all cached data. */
	DllExport void freeAll ();
};

// CAL-04/30/01: soft selection parameters.
#define SOFTSEL_MIN_FALLOFF		0.0f
#define SOFTSEL_MAX_FALLOFF		999999.0f
#define SOFTSEL_DEFAULT_FALLOFF	20.0f

#define SOFTSEL_MIN_PINCH		-1000.0f
#define SOFTSEL_MAX_PINCH		1000.0f
#define SOFTSEL_DEFAULT_PINCH	0.0f

#define SOFTSEL_MIN_BUBBLE		-1000.0f
#define SOFTSEL_MAX_BUBBLE		1000.0f
#define SOFTSEL_DEFAULT_BUBBLE	0.0f

/*! \remarks This function is available in release 3.0 and later only.\n\n
This is the standard affect region function, based on a distance and the three
affect region parameters (same as the editable mesh).\n\n
This function is a cubic curve which returns 1 at distance 0, 0 if distance is
greater than falloff, and other values for distance between 0 and falloff. To
"see" this function graphed, look at the curve in the Soft Selection parameters
in Edit Mesh, Editable Mesh, Mesh Select, or Volume Select. This function
currently is constructed as follows:\n\n
\code
float u = ((falloff - dist)/falloff);
float u2 = u*u, s = 1.0f-u;
return (3*u*bubble*s + 3*u2*(1.0f-pinch))*s + u*u2;"
\endcode 
\par Parameters:
<b>float dist</b>\n\n
The distance to the selection. The method for computing this distance is up to
the developer; for example in Mesh Select, it's the distance to the nearest
selected vertex, while in Volume Select (with a box or sphere selection region)
it's the distance to the selection volume.\n\n
<b>float falloff</b>\n\n
The limit distance of the effect. If distance \> falloff, the function will
always return 0.\n\n
<b>float pinch</b>\n\n
Use this to affect the tangency of the curve near distance=0. Positive values
produce a pointed tip, with a negative slope at 0, while negative values
produce a dimple, with positive slope.\n\n
<b>float bubble</b>\n\n
Use this to change the curvature of the function. A value of 1.0 produces a
half-dome. As you reduce this value, the sides of the dome slope more steeply.
Negative values lower the base of the curve below 0.
\return  Returns the strength of the Affect Region function at the given
distance. (In selection modifiers, this is the "soft selection" amount, the
amount it's considered selected. A vertex at a distance with a return value of
.25, for instance, will be affected 1/4 as strongly in a deformation as a fully
selected vertex.) */
DllExport float AffectRegionFunction (float dist, float falloff, float pinch, float bubble);
DllExport Point3 SoftSelectionColor (float selAmount);
/*! \remarks This function is available in release 3.0 and later only.\n\n
This function creates a matrix with the normal as a Z-axis. The X and Y axes
are chosen arbitrarily.
\par Parameters:
<b>Point3\& normal</b>\n\n
The input normal is specified here.\n\n
<b>Matrix3\& mat</b>\n\n
The output matrix. */
DllExport void MatrixFromNormal (Point3& normal, Matrix3& mat);
/*! \remarks This function is available in release 3.0 and later only.\n\n
This function creates vertex normals that are weighted averages of faces using
each vertexSmoothing groups are not used in these computations -- the normals
are those you would expect with a totally smooth mesh.
\par Parameters:
<b>Mesh \& mesh</b>\n\n
The mesh whose average vertex normals are computed.\n\n
<b>Tab\<Point3\> \& vnormals</b>\n\n
The output vertex normals. . This will be set to size <b>mesh.numVerts</b>. */
DllExport void AverageVertexNormals (Mesh & mesh, Tab<Point3> & vnormals);
/*! \remarks This function is available in release 3.0 and later only.\n\n
This function computes and returns the average normal of a group of selected
vertices.
\par Parameters:
<b>Mesh\& mesh</b>\n\n
The mesh to check. The function uses <b>mesh.vertSel</b> to check for selected
verts. */
DllExport Point3 AverageSelVertNormal (Mesh& mesh);
/*! \remarks This function is available in release 3.0 and later only.\n\n
This function computes and returns the average center of a group of selected
vertices.
\par Parameters:
<b>Mesh\& mesh</b>\n\n
The mesh to check. The function uses <b>mesh.vertSel</b> to check for selected
verts. */
DllExport Point3 AverageSelVertCenter (Mesh& mesh);
/*! \remarks This function is available in release 3.0 and later only.\n\n
Removes hidden faces from the <b>mesh.faceSel</b> selection array.
\par Parameters:
<b>Mesh\& mesh</b>\n\n
The mesh to check. */
DllExport void DeselectHiddenFaces (Mesh &mesh);
/*! \remarks This function is available in release 3.0 and later only.\n\n
This function removes edges on hidden faces from the <b>mesh.edgeSel</b>
selection array.
\par Parameters:
<b>Mesh\& mesh</b>\n\n
The mesh to check. */
DllExport void DeselectHiddenEdges (Mesh &mesh);
/*! \remarks This function is available in release 3.0 and later only.\n\n
This function hides vertices that are only used by hidden faces. If
<b>alsoHide</b> has size <b>mesh.numVerts</b>, it is used to indicate other
vertices that should also be hidden. Note that passing <b>mesh.vertHide</b> as
<b>alsoHide</b> will NOT WORK, as mesh.vertHide is overwritten before alsoHide
is read.
\par Parameters:
<b>Mesh \&mesh</b>\n\n
The mesh to check.\n\n
<b>BitArray \&alsoHide</b>\n\n
If specified, this is used to indicates other vertices that should also be
hidden. */
DllExport void HiddenFacesToVerts (Mesh &mesh, BitArray alsoHide);
/*! \remarks This function is available in release 3.0 and later only.\n\n
This function computes distances from selected vertices (as indicated by
<b>mesh.VertexTempSel()</b>) to non-selected ones.\n\n
NOTE: This is an order-of-n-squared algorithm. Each unselected vert is compared
with each selected vert. So, if you have a mesh with 1000 sel verts and 1000
unsel verts, this involves a million compares.
\par Parameters:
<b>Mesh \& mesh</b>\n\n
The mesh to check.\n\n
<b>float *selDist</b>\n\n
This is assumed to be a float array of size <b>mesh.numVerts</b>. It is set to
-1 for all verts if there is no selection. Otherwise, selected vertices have a
value of 0, and nonselected vertices have the distance to the nearest selected
vertex. */
DllExport void SelectionDistance (Mesh & mesh, float *selDist, float falloffLimit = -1.0f);
/*! \remarks This function is available in release 3.0 and later only.\n\n
This function computes distances from selected vertices (as indicated by
<b>mesh.VertexTempSel()</b>) to non-selected ones along edge paths.
<b>selDist</b> is assumed to be a float array of size <b>mesh.numVerts</b>.
<b>selDist</b> is set to -1 for all verts if there is no selection. Otherwise,
selected vertices have <b>selDist</b> value 0; non-selected vertices that are
"iters" or fewer edges away from a selected vertex are assigned the shortest
edge-path distance to a selected vertex; and non-selected vertices that are
more than <b>iters</b> edges away are set to -1. The <b>AdjEdgeList</b> is
computed by the algorithm if the one passed is NULL; otherwise you can save
time by passing a cached one in.\n\n
This is NOT an n-squared algorithm like the one above. It's more a sort of
order-of-n-times-(iters-squared).
\par Parameters:
<b>Mesh \& mesh</b>\n\n
The mesh to check.\n\n
<b>float *selDist</b>\n\n
An array of floats of size <b>mesh.numVerts</b>.\n\n
<b>int iters</b>\n\n
If 0, Selection Distance is computed from each vertex to the nearest selected
vertex, regardless of topology. This is a VERY EXPENSIVE ALGORITHM, which takes
almost 4 times as long for twice as many vertices. If iters is non-zero, it
represents the number of edges one should "travel" in trying to find the
nearest selected vertex -- this means that it only takes twice as long for
twice as many verts. (This is like the Edge Distance parameter in EMesh's Soft
Selection dialog.) If iters is 0, ae is irrelevant and may be left as NULL. If
iters is nonzero, an Adjacent Edge List is required, and will be computed
internally from the mesh if ae is NULL. (If you've got an AdjEdgeList for this
mesh handy, pass it in, otherwise don't worry about it.)\n\n
Note also that if iters is nonzero, the distance is computed along the edges,
not directly through space. If there is no selected vertex within an
iters-length path, a vertex is assigned a 0 selection value.\n\n
<b>AdjEdgeList *ae=NULL</b>\n\n
The optional adjacent edge list. */
DllExport void SelectionDistance (Mesh & mesh, float *selDist, int iters, AdjEdgeList *ae=NULL);
/*! \remarks This function is available in release 3.0 and later only.\n\n
Computes distances from nonselected vertices in the mesh to each of the vertex
clusters. This is a VERY EXPENSIVE ALGORITHM, which takes almost 4 times as
long for twice as many vertices.\n\n
Preparation for this method would typically look like:\n\n
<b>// given Mesh msh, FaceClusterList fclust:</b>\n\n
<b>Tab\<DWORD\> vclust;</b>\n\n
<b>fclust.MakeVertCluster (msh, vclust);</b>\n\n
<b>Tab\<float\> ** clustDist;</b>\n\n
<b>clustDist = new (Tab\<float\> *)[fclust.count];</b>\n\n
<b>for (int i=0; i\<fclust.count; i++) clustDist[i] = new Tab\<float\>;</b>\n\n
<b>ClustDistances (mesh, fclust.count, vclust.Addr(0), clustDist);</b>\n\n
Then (*clustDist[c])[v] would give the distance from vertex v in the mesh to
the vertices of cluster c.
\par Parameters:
<b>Mesh \&mesh</b>\n\n
The mesh the clusters are based on.\n\n
<b>DWORD numClusts</b>\n\n
The number of clusters in this mesh.\n\n
<b>DWORD *vclust</b>\n\n
A pointer to an array of vertex cluster IDs. Typically this is a pointer to the
data in the table created by EdgeClusterList::GetVertClusters or
FaceClusterList::GetVertClusters.\n\n
<b>Tab\<float\> **clustDist</b>\n\n
This is an array of \<numClusts\> pointers to tables that will be used to store
distances from various clusters. Each table will be set to the ize of
mesh.numVerts and filled with distances to the cluster that table represents.
*/
DllExport void ClustDistances (Mesh & mesh, DWORD numClusts, DWORD *vclust,
							   Tab<float> **clustDist);
/*! \remarks This function is available in release 3.0 and later only.\n\n
Computes distances from nonselected vertices in the mesh to each of the vertex
clusters. Unlike the other version which doesn't have an iters or ae parameter,
this is a linear algorithm which computes distance along a finite number of
edges.\n\n
Preparation for this method would typically look like:\n\n
<b>// given Mesh msh, FaceClusterList fclust:</b>\n\n
<b>Tab\<DWORD\> vclust;</b>\n\n
<b>fclust.MakeVertCluster (msh, vclust);</b>\n\n
<b>Tab\<float\> ** clustDist;</b>\n\n
<b>clustDist = new (Tab\<float\> *)[fclust.count];</b>\n\n
<b>for (int i=0; i\<fclust.count; i++) clustDist[i] = new Tab\<float\>;</b>\n\n
<b>ClustDistances(mesh, fclust.count, vclust.Addr(0), clustDist);</b>\n\n
Then (*clustDist[c])[v] would give the distance from vertex v in the mesh to
the vertices of cluster c.
\par Parameters:
<b>Mesh \& mesh</b>\n\n
The mesh the clusters are based on.\n\n
<b>DWORD numClusts</b>\n\n
The number of clusters in this mesh.\n\n
<b>DWORD *vclust</b>\n\n
A pointer to an array of vertex cluster IDs. Typically this is a pointer to the
data in the table created by EdgeClusterList::GetVertClusters or
FaceClusterList::GetVertClusters.\n\n
<b>Tab\<float\> **clustDist</b>\n\n
This is an array of \<numClusts\> pointers to tables that will be used to store
distances from various clusters. Each table will be set to the ize of
mesh.numVerts and filled with distances to the cluster that table
represents.\n\n
<b>int iters</b>\n\n
The maximum number of edges to travel along looking for a vertex in the given
cluster.\n\n
<b>AdjEdgeList *ae=NULL</b>\n\n
Edge length computations require an adjacent edge list. If you don't pass one
in this parameter, it'll have to construct its own from the mesh. */
DllExport void ClustDistances (Mesh & mesh, DWORD numClusts, DWORD *vclust,
							   Tab<float> **clustDist, int iters, AdjEdgeList *ae=NULL);

// Mesh Selection Converter Interface - by Steve Anderson, October 2002
// For accurate selections of polygons and elements in Meshes in window region selections,
// (as well as correct by-vertex selections of edges and faces in window regions)
// we need to know not only which components were selected, but also which were culled,
// for instance by virtue of being back-facing.  We want to select only elements in which
// all faces were in the selection region, but we shouldn't eliminate elements just because 
// some of their faces were culled.

// So we provide this Mesh Selection Converter interface, which automatically stores culling
// information in a BitArray when Mesh::SubObjectHitTest is called.  Then this can be used
// to handle level conversions in Editable Mesh, Edit Mesh, and Mesh Select 
// SelectSubComponent routines.

#define MESHSELECTCONVERT_INTERFACE Interface_ID(0x3da7dd5, 0x7ecf0391)

// Selection converter flags
#define MESH_SELCONV_REQUIRE_ALL 0x01	// Set during Mesh hit-testing (with window region)

class MeshSelectionConverter : public BaseInterface {
public:
	DllExport MeshSelectionConverter ();

	// Our interesting methods:
	/*! \remarks Converts a face selection to an element selection.\n\n
	NOTE: This method can only produce accurate results with culling if we are\n\n
	certain that the face selection corresponds to the results of the last call
	to\n\n
	the owner Mesh's SubObjectHitTest method. (To turn off culling, use GetCull
	().ClearAll(). Culling is not relevant if the MESH_SELCONV_REQUIRE_ALL flag\n\n
	is turned off.)
	\par Parameters:
	<b>Mesh \& mesh</b>\n\n
	The mesh that owns this interface\n\n
	<b>AdjFaceList *af</b>\n\n
	A pointer to an accurate AdjFaceList based on this mesh. (This is used to\n\n
	construct elements.)\n\n
	<b>BitArray \& faceSel</b>\n\n
	The face selection representing actual hits in the viewport.\n\n
	<b>BitArray \& elementSel</b>\n\n
	This is where the output Element selection is stored.\n\n
	  */
	DllExport void FaceToElement (Mesh & mesh, AdjFaceList *af, BitArray & faceSel, BitArray & elementSel);
	/*!   \remarks Converts a face selection to a polygon selection.\n\n
	NOTE: This method can only produce accurate results with culling if we are\n\n
	certain that the face selection corresponds to the results of the last call
	to\n\n
	the owner Mesh's SubObjectHitTest method. (To turn off culling, use GetCull
	().ClearAll(). Culling is not relevant if the MESH_SELCONV_REQUIRE_ALL flag\n\n
	is turned off.)
	\par Parameters:
	<b>Mesh \& mesh</b>\n\n
	The mesh that owns this interface\n\n
	<b>AdjFaceList *af</b>\n\n
	A pointer to an accurate AdjFaceList based on this mesh. (This is used to\n\n
	construct elements.)\n\n
	<b>BitArray \& faceSel</b>\n\n
	The face selection representing actual hits in the viewport.\n\n
	<b>BitArray \& polygonSel</b>\n\n
	This is where the output polygon selection is stored\n\n
	<b>float faceThresh</b>\n\n
	The angle threshold between faces, in radians, that would mark them as\n\n
	belonging to separate polygons.\n\n
	<b>bool ignoreVisEdge=false</b>\n\n
	If true, this tells the system to ignore visible edges in constructing\n\n
	polygons, and instead rely entirely on the face threshold to distinguish
	them.\n\n
	  */
	DllExport void FaceToPolygon (Mesh & mesh, AdjFaceList *af, BitArray & faceSel, BitArray & polygonSel,
		float faceThresh, bool ignoreVisEdge=false);
	/*! \remarks Converts a vertex selection to a "By Vertex" edge selection.\n\n
	NOTE: This method can only produce accurate results with culling if we are\n\n
	certain that the vertex selection corresponds to the results of the last
	call\n\n
	to the owner Mesh's SubObjectHitTest method. (To turn off culling, use GetCull
	().ClearAll(). Culling is not relevant if the MESH_SELCONV_REQUIRE_ALL flag\n\n
	is turned off.)\n\n

	\par Parameters:
	<b>Mesh \& mesh</b>\n\n
	The mesh that owns this interface\n\n
	<b>BitArray \& vertexSel</b>\n\n
	The face selection representing actual hits in the viewport\n\n
	<b>BitArray \& edgeSel</b>\n\n
	The face selection representing actual hits in the viewport\n\n
	  */
	DllExport void VertexToEdge (Mesh & mesh, BitArray & vertexSel, BitArray & edgeSel);
	DllExport void VertexToFace (Mesh & mesh, BitArray & vertexSel, BitArray & faceSel);

	// Note - we can't just combine VertexToFace and FaceToPolygon, because we won't have accurate Face culling info.
	/*!   \remarks Converts a vertex selection to a "By Vertex" polygon
	selection.\n\n
	NOTE: This method can only produce accurate results with culling if we are\n\n
	certain that the vertex selection corresponds to the results of the last
	call\n\n
	to the owner Mesh's SubObjectHitTest method. (To turn off culling, use GetCull
	().ClearAll(). Culling is not relevant if the MESH_SELCONV_REQUIRE_ALL flag\n\n
	is turned off.)\n\n
	(Note that we can't just combine VertexToFace and FaceToPolygon to obtain
	the\n\n
	results of this method, because we won't have accurate Face culling info.)
	\par Parameters:
	<b>Mesh \& mesh</b>\n\n
	The mesh that owns this interface\n\n
	<b>AdjFaceList *af</b>\n\n
	A pointer to an accurate AdjFaceList based on this mesh. (This is used to\n\n
	construct elements.)\n\n
	<b>BitArray \&vertexSel</b>\n\n
	The vertex selection representing actual hits in the viewport.\n\n
	<b>BitArray \& polygonSel</b>\n\n
	This is where the output polygon selection is stored\n\n
	<b>float faceThresh</b>\n\n
	The angle threshold between faces, in radians, that would mark them as\n\n
	belonging to separate polygons.\n\n
	<b>bool ignoreVisEdge=false</b>\n\n
	If true, this tells the system to ignore visible edges in constructing\n\n
	polygons, and instead rely entirely on the face threshold to distinguish
	them.\n\n
	  */
	DllExport void VertexToPolygon (Mesh & mesh, AdjFaceList *af, BitArray & vertexSel, BitArray & polygonSel,
		float faceThresh, bool ignoreVisEdge=false);
	/*!   \remarks Converts a vertex selection to a "By Vertex" element
	selection.\n\n
	NOTE: This method can only produce accurate results with culling if we are\n\n
	certain that the vertex selection corresponds to the results of the last
	call\n\n
	to the owner Mesh's SubObjectHitTest method. (To turn off culling, use GetCull
	().ClearAll(). Culling is not relevant if the MESH_SELCONV_REQUIRE_ALL flag\n\n
	is turned off.)\n\n
	(Note that we can't just combine VertexToFace and FaceToElement to obtain
	the\n\n
	results of this method, because we won't have accurate Face culling info.)
	\par Parameters:
	<b>Mesh \& mesh</b>\n\n
	The mesh that owns this interface\n\n
	<b>AdjFaceList *af</b>\n\n
	A pointer to an accurate AdjFaceList based on this mesh. (This is used to\n\n
	construct elements.)\n\n
	<b>BitArray \&vertexSel</b>\n\n
	The vertex selection representing actual hits in the viewport.\n\n
	<b>BitArray \&elementSel</b>\n\n
	This is where the output element selection is stored\n\n
	  */
	DllExport void VertexToElement (Mesh & mesh, AdjFaceList *af, BitArray & vertexSel, BitArray & elementSel);

	// Access which subobject components were culled in last Mesh hit-testing.
	/*! \remarks Accesses the BitArray describing which subobject components were
	culled in\n\n
	last Mesh hit-testing. In its usual usage, this data will only be set by\n\n
	Mesh::SubObjectHitTest, and will only be used by the other methods of\n\n
	MeshSelectionConverter. This information does not affect selection
	conversion\n\n
	if the MESH_SELCONV_REQUIRE_ALL flag is not set. */
	BitArray & GetCull () { return mCull; }

	void SetFlag (DWORD f, bool value=true) { if (value) mFlags |= f; else mFlags &= ~f; }
	void ClearFlag(DWORD f) { mFlags &= ~f; }
	bool GetFlag (DWORD f) { return (mFlags&f) ? true : false; }

	// From BaseInterface:
	Interface_ID GetID() {return MESHSELECTCONVERT_INTERFACE;}
	DllExport void DeleteInterface();
	DllExport BaseInterface* GetInterface(Interface_ID id);
	DllExport BaseInterface* CloneInterface(void* remapDir = NULL);

private:
	DWORD mFlags;
	BitArray mCull;
	DWORD mCullLevel;
};


