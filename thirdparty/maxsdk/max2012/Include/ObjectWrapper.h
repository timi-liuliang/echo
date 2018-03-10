/**********************************************************************
 *<
	FILE:			ObjectWrapper.h
	DESCRIPTION:	Geometry object abstraction layer
	CREATED BY:		Michaelson Britt
	HISTORY:		Created Friday, April 18, 2003

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/


#pragma once

#include "maxheap.h"
#include "triobj.h"
#include "polyobj.h"
#include "meshadj.h"
#include "patchobj.h"

class HitListWrapper;


//-----------------------------------------------------------------------------
// struct GenFace
// Represents a "face" for any type of object

//! A generalized representation of a geometric or texture face as an array of vertex indices.
struct GenFace: public MaxHeapOperators {
	//! The number of vertices in the face
	unsigned short numVerts; 
	//! An array of vertex indices for the face
	DWORD* verts;
};

//! A generalized representation of a geometric or texture edge as a pair of vertex indices and face indices.
struct GenEdge: public MaxHeapOperators {
	//! The indices of the two faces bordering the edge
	DWORD f[2];
	//! The indices of the two vertices spanned by the edge
	DWORD v[2];
};

//! A generic set of parameters defining how soft selection weights are calculated.
struct GenSoftSelData: public MaxHeapOperators {
	/*! If TRUE, soft selection weights are enabled and active, otherwise FALSE if soft selection is not active. */
	BOOL useSoftSel; //whether soft selection is active
	/*! If TRUE, the distance between vertices is computed along edges. If FALSE, it's computed directly through space.*/
	BOOL useEdgeDist;
	
	/*! This indicates the maximum number of edges the algorithm may travel along in finding the distance 
	between vertices. (Maximum path length.)\n\n
	WARNING: If useEdgeDist is FALSE, this is an n-squared algorithm: it compares every vertex not in the cluster 
	with every vertex in it. If useEdgeDist is TRUE, the time it takes is proportional to the number of verts in the 
	cluster multiplied by edgeIts. */
	int edgeIts;

	/*! If TRUE, vertices with a normal that points more than 90 degrees away from the average normal of the
	selection are not given any partial selections. They're either 1 if selected or 0 otherwise. */
	BOOL ignoreBack;

	/*! The limit distance of the effect. If distance > falloff, the function will always return 0. */
	float falloff;
	/*! Use this to affect the tangency of the curve near distance=0. Positive values produce a pointed tip, with a 
	negative slope at 0, while negative values produce a dimple, with positive slope */
	float pinch;
	/*! Use this to change the curvature of the function. A value of 1.0 produces a half-dome. As you reduce 
	this value, the sides of the dome slope more steeply. Negative values lower the base of the curve below 0. */
	float bubble;
	//float* weights; //the soft selection values

	//! Constructor; initializes all values to zero, except the falloff, which is initialized to a default of 20.0
	CoreExport GenSoftSelData(); //a struct with a constructor :)
};

//! Returns TRUE if the input soft selection parameters are identical, FALSE otherwise
BOOL operator==(GenSoftSelData& a, GenSoftSelData& b);


//-----------------------------------------------------------------------------
// class ObjectWrapper
//
/*! \sa Class Mesh, Class MNMesh, Class PatchMesh
	\par Description:
	ObjectWrapper providing a common interface to Mesh, MNMesh, and PatchMesh objects.\n
	All methods of this class are implemented by the system.\n\n
	The ObjectWrapper provides a common interface to three common classes of geometry objects,
	Mesh, MNMesh and PatchMesh, and is helpful when writing reusable code that is not type-dependant.
	Specifically, the ObjectWrapper is intended for use in Modifier plug-ins, when a developer 
	wants to avoid writing three versions of code for the three object types. Note that objects 
	of other types, such as NURBS and Particle Systems, are not directly supported by the 
	ObjectWrapper, although the wrapper can attempt to convert them to a supported type.\n\n
	The ObjectWrapper defines an abstracted concept of vertices, faces, and edges. Vertices 
	are defined as points in space; for PatchMesh objects, these are the patch knots. Faces 
	are defined as either triangles, polygons or patches, according to the object type. For 
	MNMesh objects, no access is provided to the flag values of the vertices. For PatchMesh 
	objects, no access is provided to the patch handles; methods which set the position of 
	a vertex will move the patch knot and attempt to maintain the position of the handles 
	relative to the knot.\n\n
	ObjectWrappers do not have a class ID or a class descriptor, and are not part of the 
	reference hierarchy. They can be instantiated like primitive types using new() and delete(). 
	The Init() method assigns a specific object to the wrapper after the wrapper is created.\n\n
	Note that using ObjectWrappers may incur a small performance penalty, because every method 
	of the class must branch into the appropriate type-specific method, and is therefore slower 
	than calling type-specific methods directly.
*/
//-----------------------------------------------------------------------------
class ObjectWrapper: public MaxHeapOperators {
	protected:
		TimeValue t;

		//FIXME: change the objectWrapper to store both the mesh and the object
		union {
			Mesh* mesh;
			MNMesh* poly;
			PatchMesh* patch;
		};
		union {
			MeshTempData* meshTempData;
			MNTempData* polyTempData;
		};
		BOOL isCopy;

		Point3* verts;
		GenFace* faces;
		GenEdge* edges;
		GeomObject* collapsedObj;

		BitArray faceSel, vertSel;
		GenSoftSelData softSelData;

		int invalidMask;

	public:

		//! The type of the underlying object, triObject for Mesh, polyObject for MNMesh, and patchObject for PatchMesh
		enum { empty=0, triObject, polyObject, patchObject } type;
		enum { noneEnable=0, triEnable=1, polyEnable=2, patchEnable=4, allEnable=0x7 };

		//! Constructor. The wrapper is set to an empty, uninitialized state
		CoreExport ObjectWrapper();
		//! Destructor. Calls the Release() method to free any memory used by the wrapper
		CoreExport ~ObjectWrapper() {Release();}
		//! For internal use only
		CoreExport void		Zero();

		/*! \remarks Assign an object to the ObjectWrapper. If the object is not a supported type
		(specified by the enable flags), it will be converted, and a copy of the converted 
		object is held internally.
		\param t - The current slider time
		\param os - The object
		\param copy - True to make a duplicate of the object, False otherwise
		\param enable - Flags indicating which types should be supported. The object 
			will be converted to the native type (and a copy maintained) unless it is one of 
			these supported types. For Mesh objects, set the bit ObjectWrapper::triEnable. 
			For MNMesh objects, set the bit ObjectWrapper::polyEnable. For Patch objects, set
			the bit ObjectWrapper::patchEnable. 
		\param nativeType - Indicates which type to convert to, if the object is not already
			a supported type. For Mesh, pass ObjectWrapper::triObject. For MNMesh, pass 
			ObjectWrapper::polyObject. For PatchMesh, pass ObjectWrapper::patchObject.
		\return True if the operation succeeds, False otherwise. */
		CoreExport BOOL		Init( TimeValue t, ObjectState& os, BOOL copy=FALSE, int enable=allEnable, int nativeType=polyObject );

		/*! \remarks Release internal data, and reset to an empty state. If the underlying 
		object was copied or converted to a different type by the Init() method, then the 
		ObjectWrapper maintains a copy internally, consuming a significant amount of memory. 
		Calling this method is important to free the internal copy. */
		CoreExport void		Release(); //frees the collapsed object, if any

		//! \remarks Indicates whether the wrapper is currently empty
		//! \return True if no object is assigned to the wrapper, False otherwise
		CoreExport BOOL	IsEmpty() {return type==empty;}

		//! \remarks Indicates the type of the underlying object
		//! \return For Mesh, returns ObjectWrapper::triObject. For MNMesh, returns ObjectWrapper::polyObject. For PatchMesh, returns ObjectWrapper::patchObject.
		CoreExport int		Type() {return type;}

		//! \remarks Returns the Mesh object pointer, or NULL if the underlying object is not type Mesh
		CoreExport Mesh*	GetTriMesh() {return type==triObject? mesh:NULL;}
		//! \remarks Returns the Mesh object pointer, or NULL if the underlying object is not type MNMesh
		CoreExport MNMesh*	GetPolyMesh() {return type==polyObject? poly:NULL;}
		//! \remarks Returns the PatchMesh object pointer, or NULL if the underlying object is not type PatchMesh
		CoreExport PatchMesh* GetPatchMesh() {return type==patchObject? patch:NULL;}

		enum	{ iv_none=0x000, iv_verts=0x001, iv_faces=0x002, iv_edges=0x004 };
		/*! \remarks Clear the validity mask for verts, faces, or edges. The validity mask applies when
		calling the methods Verts(), Faces() and Edges(). These methods maintain internal copies
		of data from the underlying object. Clearing the validity flags ensures that the next call
		to one of these methods will allocate a new, updated list instead of returning a stale list.
		The validity flag for a channel should be cleared whenever the channel is modifier. However,
		ObjectWrapper methods will mark the flags automatically, so it is not necessary to call 
		this method unless the underlying object is being modified outside of the ObjectWrapper.
		\param iInvalid - The validity flags to clear. To clear the vertex channel, pass 
			ObjectWrapper::iv_verts. To clear the face channel, pass ObjectWrapper::iv_faces. To clear 
			the edge channel, pass ObjectWrapper::iv_edges. */
		CoreExport void		Invalidate( int iInvalid );

		//! \remarks Calls the method InvalidateGeomCache() on the underlying object
		CoreExport void		InvalidateGeomCache();

	//-- Geom vert support

		//! \remarks Returns the number of geometric vertices
		CoreExport int		NumVerts(); //geom verts
		//! \remarks Returns the number of geometric faces
		CoreExport int		NumFaces(); //geom faces
		//! \remarks Returns the number of geometry edges
		CoreExport int		NumEdges(); //geom edges

		/*! \remarks Returns an array of the geometric vertices. Note that this method incurs a memory usage 
		penalty because it allocates an internal cache. If this method is called when the vertex
		channel has been invalidated, it will update the cache, which incurs an additional performance penalty */
		CoreExport Point3*	Verts(); //geom verts
		/*! \remarks Returns an array of the geometric faces. Note that this method incurs a memory usage penalty because it 
		allocates an internal cache. If this method is called when the face channel has been invalidated, it will 
		update the cache, which incurs an additional performance penalty.
		\sa Class GenFace */
		CoreExport GenFace* Faces(); //geom faces
		/*! \remarks Returns an array of the geometric edges. Note that this method incurs a memory usage penalty because 
		it allocates an internal cache. If this method is called when the edge channel has been invalidated, it 
		will update the cache, which incurs an additional performance penalty.
		\sa Class GenEdge */
		CoreExport GenEdge* Edges(); //geom edges

		//! \remarks Returns an individual geometric vertex
		//! \param index - The index of the vertex to retrieve
		CoreExport Point3*	GetVert( int index );
		//! \remarks Sets the position of a geometric vertex
		//! \param index - The index of the vertex to set
		//! \param p - The position of the vertex
		CoreExport void		SetVert( int index, Point3 &p );
		//! \remarks Sets the number of geometric verts in the object
		//! \param num - The vertex count
		CoreExport void		SetNumVerts( int num );

		//! \remarks Returns an individual geometric face
		//! \param index - The index of the face to retrieve
		CoreExport GenFace	GetFace( int index );
		//! \remarks Sets the data for geometric face
		//! \param index - The index of the face to set
		//! \param face - The data for the face
		CoreExport void		SetFace( int index, GenFace &face );
		//! \remarks Sets the number of geometric faces in the object
		//! \param num - The face count
		CoreExport void		SetNumFaces( int num );

		//! \remarks Returns an individual geometric edge
		//! \param index - The index of the edge to retrieve
		CoreExport GenEdge	GetEdge( int index );
		//! \remarks Returns the number of edges bordering a specified geometric face
		//! \param faceIndex - The index of the face
		CoreExport int		NumFaceEdges( int faceIndex );
		//! \remarks Returns the index number of an edge bordering a specified geometric face
		//! \param faceIndex - The index of the face
		//! \param edgeIndex - The index of the edge relative to the face
		CoreExport int		GetFaceEdgeIndex( int faceIndex, int edgeIndex );

		//! \remarks For MNMesh objects, this calls MNMesh::GetVertexSpace(). For other types, this calls MatrixFromNormal() on the vertex normal.
		//! \param index - The index of the vertex
		//! \param[out] tm - The vertex space matrix
		CoreExport void		GetVertexSpace( int index, Matrix3 & tm );
		//! \remarks Returns the geometric normal of a vertex
		//! \param index - The index of the vertex
		CoreExport Point3*	GetVertexNormal( int index );

		CoreExport BOOL		GetFaceHidden( int faceIndex );
		CoreExport void		SetFaceHidden( int faceIndex, BOOL hidden );

	//-- Map channel support

		//! \remarks Returns TRUE if the specified mapping channel is supported; otherwise FALSE.
		//! \sa List of Mapping Channel Index Values.
		//! \param mapChannel - The index of the map channel
		CoreExport BOOL		GetChannelSupport( int mapChannel );
		//! \remarks Sets whether the specified mapping channel is supported or not.
		//! \sa List of Mapping Channel Index Values.
		//! \param mapChannel - The index of the map channel
		//! \param b - TRUE to indicate the channel is supported; otherwise FALSE.
		CoreExport void		SetChannelSupport( int mapChannel, BOOL b );

		//! \remarks Returns the number of texture or vertex color vertices for the specified mapping channel.
		//! \param channel - The index of the map channel
		CoreExport int		NumMapVerts( int channel );
		//! \remarks Sets the number of texture or vertex color vertices for the specified mapping channel.
		//! \param channel - The index of the map channel
		//! \param num - The number of vertices to allocate.
		CoreExport void		SetNumMapVerts( int channel, int num );

		//! \remarks Returns a single texture or vertex color value for the specified mapping channel.
		//! \param channel - The index of the map channel
		//! \param index - The index of the texture vertex
		CoreExport Point3*	GetMapVert( int channel, int index );
		//! \remarks Sets a single texture or vertex color value for the specified mapping channel.
		//! \param channel - The index of the map channel
		//! \param index - The index of the texture vertex
		//! \param p - The value to set
		CoreExport void		SetMapVert( int channel, int index, Point3 &p );

		//! \remarks Returns an individual texture mapping face for the specified mapping channel
		//! \sa GenFace
		//! \param channel - The index of the map channel
		//! \param index - The index of the texture face
		CoreExport GenFace	GetMapFace( int channel, int index );
		//! \remarks Sets an individual texture mapping face for the specified mapping channel
		//! \sa Class GenFace
		//! \param channel - The index of the map channel
		//! \param index - The index of the texture face
		//! \param face - The data for the face
		CoreExport void		SetMapFace( int channel, int index, GenFace &face );

	//-- Material ID support
		//! \remarks Returns the zero based material ID for the specified face
		//! \param index - The index of the face
		CoreExport MtlID	GetMtlID( int index ); // face index
		//! \remarks Sets the material ID for the specified face
		//! \param index - The index of the face
		//! \param mtlID - The zero based material ID
		CoreExport void		SetMtlID( int index, MtlID mtlID ); // face index

	//-- Smoothing Group support
	
		//! \remarks Returns the smoothing group bits for the specified face
		//! \param index - The index of the face
		CoreExport DWORD	GetSmoothingGroup( int index ); // face index
		//! \remarks Sets the smoothing group bits for the specified face
		//! \param index - The index of the face
		//! \param smGrp - The smoothing group bits
		CoreExport void		SetSmoothingGroup( int index, DWORD smGrp ); // face index

	//-- Sub-object selection support

		//! \brief Selection enum values.
		enum {
			//! indicates the object selection level.
			SEL_OBJECT, 
			//! indicates the face selection level.
			SEL_VERT, 
			//! indicates the vertex selection level.
			SEL_FACE,
			//! indicates the element selection level.
			SEL_ELEM
		}; //selection enum values

		//! \remarks Returns the current level of selection for the object.
		//!See the selection enum values above
		CoreExport int		GetSelLevel(); //returns a selection enum value
		//! \remarks Sets the current level of selection for the object.
		//! \param selLevel - The selection level. See the selection enum values above
		CoreExport void		SetSelLevel(int selLevel); //selLevel is a selection enum value

		//! \remarks Returns the bits representing the vertex selection status
		CoreExport BitArray& GetVertSel();
		//! \remarks Sets the bits representing the vertex selection status
		//! \param sel - The selection bits
		CoreExport void		SetVertSel( BitArray& );

		//! \remarks Returns the bits representing the face selection status
		CoreExport BitArray& GetFaceSel();
		//! \remarks Sets the bits representing the face selection status
		//! \param sel - The selection bits
		CoreExport void		SetFaceSel( BitArray& sel );

		/*! \remarks Returns the bits representing the element selection status. For all 
		currently supported types, this is identical to the face selection, 
		because the underlying types do not natively support the element selection level. */
		CoreExport BitArray& GetElemSel();

		/*! \remarks Sets the bits representing the element selection status. For all currently supported
		types, this is identical to setting the face selection, because the underlying types 
		do not natively support the element selection level.
		\param sel - The selection bits */
		CoreExport void		SetElemSel( BitArray& sel );

		//! \remarks Returns the bits representing the specified selection status
		//! \param selLevel - The selection level. See the selection enum values above
		CoreExport BitArray* GetSel( int selLevel ); //gets the selection of the given type
		//! \remarks Returns the bits representing the specified selection status
		//! \param selLevel - The selection level. See the selection enum values above
		//! \param sel - The selection bits
		CoreExport void		SetSel( int selLevel, BitArray& sel ); //sets the selection of the given type

		/*!Converts the bits representing a face selection into bits representing an element selection,
		and expands the selection accordingly. If any face in an element is selected according 
		the input bits, then all faces within that element will be toggled as selected in the output 
		bits. This method does not change the selection on the object; the methods SetFaceSel() 
		or SetElemSel() are needed to apply a selection after it is calculated
		\param[out] elemSel - The output selection bits for the element level
		\param faceSel - The input selection bits for the face level */
		CoreExport void		FaceToElemSel( BitArray& elemSel, BitArray& faceSel ); //convert face to elem selection

	//-- Selection methods

		/*! \remarks Grows the selection for the specified level. The perimeter of the selection is
		expanded by one sub object. The resulting selection is stored in the output bits;
		This method does not change the selection on the object
		\param selLevel - The selection level. See the selection enum values above
		\param[out] newSel - The output selection bits */
		CoreExport void		GrowSelection( int selLevel, BitArray& newSel );

		/*! \remarks Shrinks the selection for the specified level. The perimeter of the
		selection is reduced by one sub object. The resulting selection is stored in the
		output bits; This method does not change the selection on the object
		\param selLevel - The selection level. See the selection enum values above
		\param[out] newSel - The output selection bits */
		CoreExport void		ShrinkSelection( int selLevel, BitArray& newSel );

	//-- Soft selection support

		//! \remarks Returns the soft selection parameters
		//! \sa GenSoftSelData
		//! \param[out] softSelData - The selection parameters
		CoreExport void		GetSoftSel( GenSoftSelData& softSelData );
		//! \remarks Sets the soft selection parameters. Note that UpdateSoftSel() may be needed to recalculate the soft selection weights.
		//! \sa GenSoftSelData
		//! \param softSelData - The selection parameters
		CoreExport void		SetSoftSel( GenSoftSelData& softSelData );
		//! \remarks Recalculates the soft selection weights. Use this after setting the soft selection parameters.
		CoreExport void		UpdateSoftSel();

		/*! \remarks Returns the vertex soft selection weights. If cached, the cache is returned.
		Otherwise a cache is allocated and computed from the current object and the soft selection parameters. */
		CoreExport float*	SoftSelWeights();
		/*! \remarks Returns the viewport display color corresponding to the given vertex soft selection weight.
		\param f - The selection weight, ranging from 0 to 1 */
		CoreExport Point3	SoftSelColor( float f );

	//-- Hit testing and ray intersection
		enum { SUBHIT_IGNOREBACKFACING=(1<<5) }; //other SUBHIT_... values defined in Mesh.h are also supported

		//! \remarks Performs a viewport hit testing operation against the object
		//! \param gw - The graphics window associated with the viewport the object is being hit tested in.
		//! \param ma - The list of materials for the mesh
		//! \param hr - This describes the properties of a region used for the hit testing. See Class HitRegion.
		//! \param flags - Flags specifying how the hit testing is performed. Accepted values are SUBHIT_IGNOREBACKFACING to specify the ignore backfacing mode, plus the following values defined in Mesh.h: SUBHIT_UNSELONLY, SUBHIT_ABORTONHIT, SUBHIT_SELSOLID, and SUBHIT_USEFACESEL
		//! \param[out] hitList - The hit list object to hold the results of the hit test operation. See Class HitListWrapper.
		//! \param numMat - The number of materials for the mesh.
		//! \param mat - The object-to-world transform matrix for the object. This needed to calculate backface culling if the underlying object is type Mesh and the flag ObejctWrapper:: SUBHIT_IGNOREBACKFACING is specified
		//!return TRUE if any hits were found, FALSE otherwise
		CoreExport BOOL		SubObjectHitTest(
								int selLevel, GraphicsWindow* gw, Material* ma, HitRegion *hr, 
								DWORD flags, HitListWrapper& hitList, int numMat=1, Matrix3 *mat = NULL );

		//! \remarks Intersects a ray against the object, returning the face and barycentric coordinates of the hit point.
		//! \param ray - The ray to cast
		//! \param[out] intersectAt - The distance travelled by the ray to the intersection point.
		//! \param[out] norm - The surface normal at the intersection point.
		//! \param[out] fi - The face index of the intersected face
		//! \param[out] bary - The barycentric coordinates of the hit point. For MNMesh and PatchMesh object, there may be more than three coordinate values
		//!return Nonzero if an intersection was found, zero otherwise
		CoreExport int		IntersectRay(  Ray& ray, float& intersectAt, Point3& norm, int& fi, FloatTab& bary  );

		//! \remarks Renders the object using the specified graphics window and array of materials.
		//! \param gw - Points to the graphics window to render to.
		//! \param ma - The list of materials to use to render the mesh.
		//! \param rp - Specifies the rectangular region to render. If the mesh should be rendered to the entire viewport pass NULL
		//! \param compFlags - One or more of the following flags defined in Mesh.h: COMP_TRANSFORM, COMP_IGN_RECT, COMP_LIGHTING, COMP_ALL or COMP_OBJSELECTED
		//! \param numMat - The number of materials for the mesh
		CoreExport void		Render(GraphicsWindow *gw, Material *ma, RECT *rp, int compFlags, int numMat = 1);

	//-- Enum conversion

		/*! \remarks Converts the ObjectWrapper selection level enum values into the type-specific equivalents. 
		For instance, an input of ObjectWrapper::SEL_VERT (integer value 1) would return MESH_VERTEX 
		(integer value 2) if the underlying object was type Mesh. */
		CoreExport int		SelLevel( int selLevel ); //generic enum to object-specific enum //FIXME: should be private?
		/*! \remarks Converts the ObjectWrapper hit level enum values into the type-specific equivalents. 
		For instance, an input of ObjectWrapper::SEL_VERT (integer value 1) would return SUBHIT_VERTS 
		(integer value 1<< 25) if the underlying object was type Mesh. */
		CoreExport int		HitLevel( int selLevel ); //generic enum to object-specific hit level enum //FIXME: should be private?

};


//-----------------------------------------------------------------------------
// class HitListWrapper

class HitListWrapper: public MaxHeapOperators {
	protected:
		friend class ObjectWrapper;
		union {
			SubObjHitList* meshHitList;
			SubObjHitList* polyHitList;
			SubPatchHitList* patchHitList;
		};
		union {
			MeshSubHitRec* curMeshHitRec;
			MeshSubHitRec* curPolyHitRec;
			PatchSubHitRec* curPatchHitRec;
		};

	public:
		/*! The type of the underlying object against which the hit testing occurred;
			triObject for Mesh, polyObject for MNMesh, and patchObject for PatchMesh. 
			The empty type indicates that no hit testing has been performed yet */
		enum { empty=0, triObject, polyObject, patchObject } type;

		/*! \remarks Constructor. This does not call the Init() method, so wrapper must still be initialized before it can be used. */
		CoreExport HitListWrapper() {type=empty;}
		/*! \remarks Destructor. Deallocates used memory by calling Release() */
		CoreExport ~HitListWrapper();

		/*! \remarks Sets the first hit entry in the list as the current hit
		\return FALSE if no hit entries are in the list, otherwise TRUE */
		CoreExport bool GoToFirst();
		/*! \remarks Sets the current hit to the next entry in the list
		\return FALSE if the operation fails because no entries remain the list, otherwise TRUE */
		CoreExport bool GoToNext();

		/*! \remarks The distance of the current hit. If the user is in wireframe mode, this is the 
		distance in pixels to the item that was hit. If the user is in shaded mode, this is the Z depth
		distance. Smaller numbers indicate a closer hit. 
		\return The distance of the current hit. */
		CoreExport DWORD GetDist();

		/*! \remarks The index of the sub-object component of the current hit. For example,
		if faces were being hit tested, this would be the index of the face which was hit.
		\return The index of the sub-object component of the current hit.  */
		CoreExport int GetIndex();

		/*! \remarks Initializes the wrapper to the specified object type, allocating the 
		type-specific hit list used by the wrapper. This must be called before the wrapper
		can store any hit test results.  
		\param type - one of the type enum values*/
		CoreExport void Init( int type );
		/*! \remarks Releases the type-specific hit list used by the wrapper. This is normally called by the destructor. */
		CoreExport void Release();
};


