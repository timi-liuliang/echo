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
#include "channels.h"
#include "snap.h"
#include "ioapi.h"
#include "export.h"
#include "vedge.h"  //DS
#include "utillib.h"
#include "tab.h"
#include "baseinterface.h"
#include "channels.h"
#include "MeshFaceFlagConstants.h"
#include "GraphicsConstants.h"
#include "assert.h"
#include "bitarray.h"
#include "TabTypes.h"
#include "Strip.h"
#include "box3.h"
#include "point3.h"
#include "UVWMapper.h"
#include "UVWMapTypes.h"

// forward declarations
class GraphicsWindow;
class Material;
class HitRegion;
class IPoint2;

/** An argument for the mapChan parameter in Mesh::setVCDisplayData(). 
 * If mapChan parameter is set to MESH_USE_EXT_CVARRAY then the data in 
 * VCArray and TVFace is stored for internal use and consequent drawing. */
#define MESH_USE_EXT_CVARRAY	(-32767)

#define MESH_MULTI_PROCESSING	TRUE		//!< TRUE turns on mp vertex transformation

#define MESH_CAGE_BACKFACE_CULLING			//!< for "cage" orange gizmo meshes in EMesh, EPoly, Mesh Select, etc.

class ISave;
class ILoad;
class IHardwareShader;
class TriStrip;
class MeshNormalSpec;

#define NEWMESH

/** \internal The 'rendered' normal class. RNormals are stored as unit vectors in
object space. An instance of this class is maintained by the RVertex
class since the RVertex is what gets rendered (lit), and the lighting
methods need to know about the normals. All methods of this class are
implemented by the system. 
Note:This class is used internally by 3ds Max. Developers who need to
compute face and vertex normals for a mesh should instead refer to the Advanced
Topics section "Computing Face and Vertex Normals".
\see Class Mesh, Class RVertex, Class GraphicsWindow.
*/
class RNormal: public MaxHeapOperators {
	public:
		/**  Constructor. The smoothing group and material index are
		set to zero. */
		RNormal()	{ smGroup = mtlIndex = 0; }
		/** Sets the normal to the specified value.
		\param nor The normal value to set. */
		void		setNormal(const Point3 &nor) { normal = nor; }
		/** Adds the specified value to the value of the normal.
		\param nor The normal value to add to the existing value. */
		void		addNormal(const Point3 &nor) { normal += nor; }	
		/** Converts the normal to a unit vector. */
		void		normalize(void) 	{ normal = Normalize(normal); }
		/** Returns the normal. */
		Point3 &	getNormal(void) 	{ return normal; }
      inline const Point3& getNormal() const { return normal; }
		/** Sets the smoothing group to the specified value.
		\param g The smoothing group value to set. */
		void		setSmGroup(DWORD g)	{ smGroup = g; }
		/** ORs the specified smoothing group value to the existing
		value.
		\param g The smoothing group bits to set. */
		void		addSmGroup(DWORD g) { smGroup |= g; }
		/** Returns the smoothing group value. */
		DWORD		getSmGroup(void)	{ return smGroup; }
		/** Sets the material index to the specified value.
		\param i The material index value to set. */
		void		setMtlIndex(MtlID i){ mtlIndex = i; }
		/** Returns the material index. */
		MtlID		getMtlIndex(void)	{ return mtlIndex; }
		/** Sets the RGB value.
		\param clr The RGB color to set. */
		void		setRGB(Point3 &clr)	{ rgb = clr; };
		/** Returns the RGB value. */
		Point3 &	getRGB(void)		{ return rgb; }
      inline const Point3& getRGB() const { return rgb; }
		
	private:	
		/** The normal as a unit vector. Note that if you set this normal, you should call
		the method normalize() if you are not sure the normal is already unit
		length. */
		Point3		normal;	  
		/** The smoothing group. Normals are attached to smoothing groups in the sense that
		one vertex, if it is shared by two triangles with different smoothing groups,
		will have two normals associated with it -- one for each smoothing group. */
		DWORD		smGroup;    
		/** The material index. If one vertex is shared between two materials, there will
		be two normals. Therefore the material index is stored with the normal as
		well. 
		Note: typedef unsigned short MtlID; */
		MtlID		mtlIndex;   
		/** The RGB value.  */
		Point3		rgb;	   
	};					   

/** \internal A RVertex is a rendered vertex. A vertex becomes a RVertex after
it has been transformed. A RVertex has a position (x, y, z coordinate)
that is stored in device coordinates. They are stored in the data member
fPos[3]. 
One vertex in a Mesh can be shared between many different smoothing
groups. In the 3ds Max Mesh database, the vertices are shared, however
the normals are not. This is why an RVertex has a RNormal data
member. For example, if you had a sphere that had the top and bottom
hemi-spheres smoothed separately (i.e. not smoothed across the equator), then
the vertices across the equator would have two RNormals for each
RVertex while the other vertices would have one. There may be as many
RNormals as there are smoothing groups colliding at a vertex. However,
it is by far the most common case to have one, and anything other than one or
two is very rare. 
For purposes of smoothing, as many RNormals are allocated as required
and are stored in this class. RNormals are kept in this RVertex
class since the RVertex is what gets rendered (lit). When you light a
vertex you need to know its normal direction. Thus the RNormal(s) are
stored in this class (using data member rn or *ern). 
All methods of this class are implemented by the system. 
Note:This class is used internally by 3ds Max. Developers who need to
compute face and vertex normals for a mesh should instead refer to the Advanced
Topics section Computing Face and Vertex Normals.
\see  Class RNormal, Class Mesh, Class GraphicsWindow. 
 */
class RVertex: public MaxHeapOperators {
	public:
      enum { 
         MAX_NUM_NORMALS  = 0xff
      };

   public:
		/**  Constructor. The flags are set to zero and the ern
		 * pointer is set to NULL. */
		RVertex()	{ rFlags = 0; ern = NULL; }

		/** Destructor. */
		DllExport ~RVertex();	
		
		/**  The flags contain the clip flags, the number of normals at the vertex, and the
		 * number of normals that have already been rendered. These are used internally.
		 * For example, the clipping flags are used to see if the RVertex can be either
		 * trivially accepted or rejected when rendering. */
		DWORD		rFlags;     

		/**  \deprecated This position is no longer used. */
		int			pos[3];	
		/**  If used, a single RNormal will be stored here. */
		RNormal		rn;
		/**  In some cases, there may be two or more RNormals per vertex. If this is the
		 * case, these 'extra' RNormals are allocated and the pointer to the memory is
		 * stored here. If these are used, then data member rn is not used
		 * (rn is copied into ern[0]). 
		 */
		RNormal 	*ern;		 
	};					  

/** This class represents a single triangular face. The class maintains three
indices into the vertex list for the face, a 32-bit smoothing group for the
face, and 32-bits of face flags. The flags also store information about the
visibility of the face, the visibility of the three edges, and whether or not
the face has texture vertices present. The most significant 16-bits of the face
flags store the material index. All methods of this class are implemented by
the system.
\see  Class Mesh. */
class Face: public MaxHeapOperators {	
	public:
		/** These are zero-based indices into a mesh object's array of vertices. */
		DWORD	v[3];
		/** Smoothing group bits for the face. 
		 * Each bit of this 32 bit value represents membership in a smoothing group. The
		 * least significant bit represents smoothing group #1 while the most significant
		 * bit represents group #32. If two adjacent faces are assigned the same smoothing
		 * group bit, the edge between them is rendered smoothly. 
		 */
		DWORD	smGroup;

		/**	The Face Flags:
		Can be from the following list:
		\li The flags in \ref Edge_visibility_flags 
		\li The FACE_HIDDEN flag (Face visibility bit: if the bit is 1, the face is hidden). 
		\li The Mesh::mapSupport() function (should be used instead of the obsolete HAS_TVERTS flag).
		\li \ref Material_ID_Masks to access the Material ID (which is stored in the HIWORD of the face flags).
		*/
		DWORD	flags;

		/** Constructor. The smoothing groups and face flags are
		initialized to zero. */
		Face()	{ smGroup = flags = 0; }

		/** Retrieves the zero based material ID for this face. Note:
		typedef unsigned short MtlID; */
		MtlID	getMatID() { return (int)( ( flags >> FACE_MATID_SHIFT ) & FACE_MATID_MASK); }

		/** Sets the material ID for this face.
		\param id Specifies the zero based material index. */

		void    setMatID(MtlID id) {flags &= 0xFFFF; flags |= (DWORD)(id<<FACE_MATID_SHIFT);}
		/** Sets the smoothing group bits for this face.
		\param i Specifies the smoothing group bits for this face. */
		void	setSmGroup(DWORD i) { smGroup = i; }

		/** Returns the smoothing group bits for this face. */
		DWORD	getSmGroup(void)	{ return smGroup; }

		/** Sets the vertices of this face.
		\param vrt An array of the 3 vertices to store. These are zero based indices into
		the mesh object's array of vertices. */
		DllExport void	setVerts(DWORD *vrt);

		/** Sets the vertices of this face. The specified indexes are
		zero based indices into the mesh object's array of vertices.
		\param a Specifies the first vertex. 
		\param b Specifies the second vertex. 
		\param c Specifies the third vertex. */
		void	setVerts(int a, int b, int c)  { v[0]=a; v[1]=b; v[2]=c; }

		/** Sets the visibility of the specified edge.
		\param edge Specifies the edge to set the visibility of. You may use 0, 1, or 2. 
		\param visFlag One of the following values: 
		\li EDGE_VIS 
		Sets the edge as visible. 
		\li EDGE_INVIS 
		Sets the edge as invisible. */
		DllExport void	setEdgeVis(int edge, int visFlag);

		/** Sets the visibility of the all the edges.
		\param va Specifies the visibility for edge 0. Use either EDGE_VIS or
		EDGE_INVIS. 
		\param vb Specifies the visibility for edge 1. Use either EDGE_VIS or
		EDGE_INVIS. 
		\param vc Specifies the visibility for edge 2. Use either EDGE_VIS or
		EDGE_INVIS. */
		DllExport void    setEdgeVisFlags(int va, int vb, int vc); 

		/** Retrieves the edge visibility for the specified edge.
		\param edge Specifies the edge.
		\return  Nonzero if the edge is visible, zero if the edge is invisible.
		*/
		int		getEdgeVis(int edge){ return flags & (VIS_BIT << edge); }

		/** Returns the index into the mesh vertex array of the
		specified vertex.
		\param index Specifies the vertex to retrieve. You may use 0, 1 or 2. */
		DWORD	getVert(int index)	{ return v[index]; }

		/** Retrieves a pointer to the vertex array.
		\return  A pointer to the vertex array. */
		DWORD *	getAllVerts(void)	{ return v; }

		/** Determines if the face is hidden or visible.
		\return  TRUE if the face is hidden; otherwise FALSE. */
		BOOL	Hidden() {return flags&FACE_HIDDEN?TRUE:FALSE;}

		/** Hides this face (makes it invisible in the viewports). */
		void	Hide() {flags|=FACE_HIDDEN;}

		/** Shows this face (makes it visible in the viewports). */
		void	Show() {flags&=~FACE_HIDDEN;}

		/** Sets the hidden state of this face.
		\param hide Specifies the hidden state for the face. Pass TRUE to hide the face;
		FALSE to show it. */
		void	SetHide(BOOL hide) {if (hide) Hide(); else Show();}

		BOOL	InForeground() {return flags&FACE_INFOREGROUND?TRUE:FALSE;}
		void	PushToForeground() {flags|=FACE_INFOREGROUND;}
		void	PushToBackground() {flags&=~FACE_INFOREGROUND;}
		void	SetInForegound(BOOL foreground) {if (foreground) PushToForeground(); else PushToBackground();}

		BOOL	IsBackFacing() {return flags&FACE_BACKFACING?TRUE:FALSE;}
		void	SetBackFacing() {flags|=FACE_BACKFACING;}
		void	SetFrontFacing() {flags&=~FACE_BACKFACING;}
		void	SetBackFacing(BOOL backFacing) {if (backFacing) SetBackFacing(); else SetFrontFacing();}



		/** Returns the first vertex in the face that isn't v0 or v1.
		\param v0 The zero based index of one of the vertices to check. 
		\param v1 The zero based index of the other vertex to check.
		\return  The zero based index of the vertex found in the Mesh's vertex
		list. */
		DllExport DWORD GetOtherIndex (DWORD v0, DWORD v1);

		/** Returns the index of the edge in the face that goes from v0 to v1, or
		v1 to v0.
		\param v0 The zero based index of the vertex at one end of the edge. 
		\param v1 The zero based index of the vertex at the other end of the edge.
		\return  The zero based index of the edge found in the Mesh's edge
		list. 
		  */
		DllExport DWORD GetEdgeIndex (DWORD v0, DWORD v1);

		/** Indicates order in which vertices v0 and v1 appear in the face.
		\param v0 One vertex on this face. 
		\param v1 Another vertex on this face.
		\return  1 if v1 follows v0 in sequence (This includes e.g. when
		Face::v[2] == v0 and Face::v[0] == v1.) 
		-1 if v0 follows v1 in sequence 
		0 if v0 or v1 are not on the face. */
		DllExport int Direction (DWORD v0, DWORD v1);

		/** Returns the index of the specified vertex in this face's vertex list
		(0, 1 or 2). If not found 3 is returned.
		\param v0 The zero based index of the vertex to check. */
		DllExport DWORD GetVertIndex (DWORD v0);

		/** This method switches v0,v1 if needed to put them in face-order. If v0
		and v1 are in the order in which they appear in the face, or if one or
		both of them are not actually on the face, nothing happens. If however
		v0 follows v1, the values of the parameters are switched, so that they
		are then in the correct order for this face.
		Sample Code:
		\code
		Face & f = mesh.faces[edge.f[0]];
		DWORD v0 = edge.v[0];
		DWORD v1 = edge.v[1];
		// Switch v0, v1 if needed to match orientation in selected face.
		f.OrderVerts(v0,v1);
		\endcode
		\param v0 One vertex on this face. 
		\param v1 Another vertex on this face. */
		DllExport void OrderVerts (DWORD & v0, DWORD & v1);	// switches v0,v1 if needed to put them in face-order.
	};


// This is used both for UVWs and color verts
/** This class is used for texture faces as well as vertex colors. The class
maintains an array of three indices into the object's tVerts array. See
the Mesh class for details on how its array of TVFaces and tVerts relate. All
methods of this class are implemented by the system.
\see  Class Mesh. */
class TVFace: public MaxHeapOperators {
public:
	/** These are indices into the mesh object's tVerts array. */
	DWORD	t[3]; 

	/** Constructor. No initialization is done. */
	TVFace() {}

	/** Constructor.
	\param a Specifies the index into the tVerts array for vertex 0. 
	\param b Specifies the index into the tVerts array for vertex 1. 
	\param c Specifies the index into the tVerts array for vertex 2. */
	TVFace(DWORD a, DWORD b, DWORD c) {t[0]=a; t[1]=b; t[2]=c;}
	
	/** Sets the texture vertices.
	\param vrt An array of indices into the tVerts array for vertices 0, 1, and 2.
	*/
	DllExport void	setTVerts(DWORD *vrt);

	/** Sets the textured vertices.
	\param a Specifies the index into the tVerts array for vertex 0. 
	\param b Specifies the index into the tVerts array for vertex 1. 
	\param c Specifies the index into the tVerts array for vertex 2. */
	void	setTVerts(int a, int b, int c)  { t[0]=a; t[1]=b; t[2]=c; }	

	/** Retrieves one of the texture vertices.
	\param index Specifies the index of the texture vertex to retrieve. You may use 0, 1 or
	2.
	\return  The texture vertex. */
	DWORD	getTVert(int index)	{ return t[index]; }

	/** Returns a pointer to the array of texture vertices. 
	  */
	DWORD *	getAllTVerts(void)	{ return t; }

	/** Returns the index of the specified texture vertex in this texture face's
	vertex list (0, 1 or 2). If not found 3 is returned.
	\param v0 The zero based index of the texture vertex to check. */
	DllExport DWORD GetVertIndex (DWORD v0);

	/** Returns the first texture vertex in this texture face that isn't v0 or v1.
	\param v0 The zero based index of one of the vertices to check. 
	\param v1 The zero based index of the other vertex to check. */
	DllExport DWORD GetOtherIndex (DWORD v0, DWORD v1);

	/** Indicates the order in which vertices v0 and v1 appear in the texture face.
	\param v0 One vertex on this texture face. 
	\param v1 Another vertex on this texture face.
	\return  1 if v1 follows v0 in sequence. 
	-1 if v0 follows v1 in sequence. 
	0 if v0 or v1 are not on the face. */
	DllExport int Direction (DWORD v0, DWORD v1);

	/**	This method switches v0,v1 if needed to put them in face-order. If v0 and
	v1 are in the order in which they appear in the texture face, or if one or
	both of them are not actually on the texture face, nothing happens. If
	however v0 follows v1, the values of the parameters are switched, so that
	they are then in the correct order for this texture face.
	\param v0 One vertex on this texture face. 
	\param v1 Another vertex on this texture face. */
	DllExport void OrderVerts (DWORD & v0, DWORD & v1);	// switches v0,v1 if needed to put them in face-order.
};


// MeshMap stuff:

#define MAX_MESHMAPS 100			//!< The higher limit of the mapping channels (map channel can be from 0 to MAX_MESHMAPS - 1)

/// \defgroup Mapping_Flags Mapping Flags
//@{
#define MESHMAP_USED 0x0001			//!< Indicates this mapping channel is actually used (carries mapping information).
#define MESHMAP_TEXTURE 0x0002		//!< Indicates this is a texture mapping channel.
#define MESHMAP_VERTCOLOR 0x0004	//!< Indicates this is a vertex color channel.
#define MESHMAP_USER 0x0100			//!< Indicates the channel is used for a developer purpose
//@}



/// \defgroup Hidden_Map_Channels___Numbers Hidden Map Channels / Numbers
//@{
/** The number of "Hidden" or negative-indexed maps in objects which support hidden maps.
 * indexes for hidden maps are -1-(their position in the hidden map array).
 * (These negative indexes are valid for methods such as mapSupport, mapVerts, etc.)
 */
#define NUM_HIDDENMAPS 2
/** The shading (or illumination) map. */
#define MAP_SHADING -1
/** The Alpha channel map. Note that only the x (or u) coordinate of the map vertices is currently used. */
#define MAP_ALPHA -2
//#define MAP_NORMALS -3
//@}

/** In 3ds Max 3.0 and later the user may work with more than 2 mapping channels.
When the mapping channel is set to a value greater than 1 (by using a UVWMap
Modifier for example) then an instance of this class is allocated for each
channel up to the value specified. It maintains the mapping information for a
single channel. 
An array of instances of this class is carried by the Mesh class in the public
data member: 
MeshMap *maps; 
All methods of this class are implemented by the system.
*/
class MeshMap: public MaxHeapOperators {
public:
	/** One or more of the flags in \Mapping_Flags.*/
	DWORD flags;
	/** Array of texture vertices. This stores the UVW coordinates for the mapping
	channel. Note: typedef Point3 UVVert; */
	UVVert *tv;
	/** The texture vertex faces. There needs to be one TVFace for every face in the
	Mesh, but there can be three indices into the UVVert array that are any
	UV's. */
	TVFace *tf;
	/** The number of elements in the UVVert array. */
	int vnum;
	/** The number of elements in the TVFace array. */
	int fnum;

	/** Constructor. The flags are cleared, the vertex and face
	numbers are set to 0, and the tv and tf pointers are set to NULL. */
	MeshMap () { flags=0x0; tv=NULL; tf=NULL; vnum = fnum = 0; }

	/** Destructor. If the tv and tf arrays are allocated they are
	deleted. */
	DllExport ~MeshMap ();

	/** Returns the number of UVVerts. */
	int getNumVerts () { return vnum; }

	/** Sets the number of UVVerts allocated to the specified value.
	\param vn The new number of UVVerts to allocate. 
	\param keep	If TRUE, any previously allocated UVVerts are maintained (up to the maximum
	set by vn). If FALSE they are discarded. Defaults to False.*/
	DllExport void setNumVerts (int vn, BOOL keep=FALSE);

	/** Returns the number of TVFaces. */
	int getNumFaces () { return fnum; }

	/** Set the number of TVFaces allocated to the specified value.
	\param fn The new number of TVFaces to allocate. 
	\param keep If TRUE any previously allocated TVFaces are maintained (up to the maximum set
	by fn). If FALSE they are discarded. Defaults to FALSE. */
	DllExport void setNumFaces (int fn, BOOL keep=FALSE, int oldCt=0);

	/** Clears (deletes) the tv and tf arrays and sets
	the counts to zero. */
	DllExport void Clear ();

	/** This method returns a BitArray with size vnum, where isolated
	(unused) vertices are selected. */
	DllExport BitArray GetIsoVerts ();

	/** This method is used to delete vertices from a mesh map.
	\param set The array of bits where mapping vertices you want to delete are set. 
	\param delFace This is an optional parameter. If non-NULL, it's filled with a record of
	which faces, if any, were using the specified map verts and should
	therefore be deleted or considered invalid. (Note: in normal usage, it's
	preferable to remove any compromised faces _before_ deleting vertices, so
	this parameter would rarely be used). Defaults to NULL */
	DllExport void DeleteVertSet (BitArray set, BitArray *delFace=NULL);

	/** This method is used to delete faces from a mesh map.
	NOTE: The number and arrangement of faces in a MeshMap should always agree
	with the "parent" mesh. It's safest in most cases to just let this be
	handled by Mesh::DeleteFaceSet(). 
	\param set This is a list of mapping faces to delete. 
	\param isoVert If non-NULL, this BitArray is filled with a list of map vertices that were
	used by the deleted faces but not by any remaining faces. (This is a list
	of "newly isolated" map vertices). Defaults to NULL. 
	*/
	DllExport void DeleteFaceSet (BitArray set, BitArray *isoVert=NULL);

	/** Sets the specified flag(s).
	\param fl The flags to set. See the public data member flags above. */
	void SetFlag (DWORD fl) { flags |= fl; }

	/** Clears the specified flag(s).
	\param fl The flags to clear. See the public data member flags above. */
	void ClearFlag (DWORD fl) { flags &= ~fl; }

	/** Returns TRUE if the specified flag(s) are set; otherwise
	FALSE.
	\param fl The flags to check. See the public data member flags above. */
	BOOL GetFlag (DWORD fl) { return (flags & fl) ? TRUE : FALSE; }

	/** Returns TRUE if this mapping channel is being used; otherwise
	FALSE. */
	BOOL IsUsed () const { return (flags & MESHMAP_USED) ? TRUE : FALSE; }

	/** Exchanges the data between this MeshMap object and the
	specified one. The flags, vnum and fnum values are
	exchanged. The UVVert and TVFace pointers are swapped.
	\param from The MeshMap instance to swap with. */
	DllExport void SwapContents (MeshMap & from);

	/** Assignment operator.
	\param from The MeshMap to assign. */
	DllExport MeshMap & operator= (MeshMap & from);
};

// Usually returns TEXMAP_CHANNEL or VERTCOLOR_CHANNEL:
/** Returns the Channel ID of the map channel. if mp\>=1, this always returns
TEXMAP_CHANNEL. For mp\<1, including the hidden map channels, this is
currently always VERTCOLOR_CHANNEL. In the future it may include map
channels that are actually part of GEOM_CHANNEL or something.
\param mp The map channel. */
DllExport DWORD MapChannelID (int mp);

// Usually returns TEXMAP_CHAN_NUM, etc:
/** Similar to MapChannelID, but this returns the CHAN_NUM
version:TEXMAP_CHAN_NUM, VERTCOLOR_CHAN_NUM, etc.
\param mp The map channel.  */
DllExport int MapChannelNum (int mp);


/** This class can be used to store a bit per map channel,
 * including both negative and positive map channels, and it works with any
 * size index.
 */ 
class MapBitArray: public MaxHeapOperators {
private:
	bool mTexture, mColor;
	BitArray mTextureFlip, mColorFlip;

public:

	/// Constructor.  Sets all channels to false.
	MapBitArray () : mTexture(false), mColor(false) { }


	/// Constructor.  Sets all channels to the given default value.
	MapBitArray (bool defaultValue)
		: mTexture(defaultValue), mColor(defaultValue) { }

	/// Constructor.  Sets all texture channels (1 and above) to the texture default,
	/// and all color channels (0 and below) to the color default.
	MapBitArray (bool textureDefault, bool colorDefault)
		: mTexture(textureDefault), mColor(colorDefault) { }

	/// Returns the highest index that this MapBitArray could have a non-default value for.
	/// All indices above this are guaranteed to have value "TextureDefault()".
	int Largest() const { return (mTextureFlip.GetSize()>1) ? mTextureFlip.GetSize()-1 : 0; }

	/// Returns the lowest index that this MapBitArray could have a non-default value for.
	/// All indices below this are guaranteed to have value "ColorDefault()".
	int Smallest() const { return (mColorFlip.GetSize()>1) ? 1-mColorFlip.GetSize() : 0; }

	/// Sets the value of the given channel.
	DllExport void Set (int mapChannel, bool val=true);

	/// Clears the value of the given channel.
	void Clear (int mapChannel) { Set (mapChannel, false); }

	/// Gets the value of the given channel.  (If this particular channel has
	/// not been set before, the default for the channel will be returned.)
	DllExport bool Get (int mapChannel) const;

	/// Returns the default value for texture channels, channels 1 and above.
	bool TextureDefault () const { return mTexture; }

	/// Returns the default value for color channels, channels 0 and below.
	bool ColorDefault () const { return mColor; }

	/// Inverts the value of all texture channels (1 and above).
	void InvertTextureChannels () { mTexture=!mTexture; }

	/// Inverts the value of all color channels (0 and below).
	void InvertColorChannels () { mColor=!mColor; }

	/// Inverts all channel values.
	void InvertAll () { mColor = !mColor; mTexture = !mTexture; }

	/// Saves data to stream.
	DllExport IOResult Save(ISave* isave);

	/// Loads data from stream.
	DllExport IOResult Load(ILoad* iload);

	/// Indexing operator
	bool operator[](int i) const { return Get(i); }

	/// Comparison operator.
	DllExport BOOL operator==(const MapBitArray& b) const;

	/// Inequality operator.
	BOOL operator!=(const MapBitArray& b) const { return !(*this == b); }

	/// Assignment operator
	DllExport MapBitArray& operator=(const MapBitArray& b);
};

// Following is used for arbitrary per-element info in meshes, such as weighted verts
// or weighted vert selections.  Methods are deliberately made to look like Tab<> methods.

// For per-vertex info: set a maximum, and reserve first ten channels
// for Discreet's use only.
#define MAX_VERTDATA 100	//!< The maximum channel index for developers' defined data.
#define VDATA_USER 10		//!< Third parties should use this channel or higher.

/** \defgroup Vertex_Data_Index_Options Vertex Data Index Options
 * The following are the vertex data channel index values for use with 
 * the vertex data methods of class Mesh, class EPoly, and class MNMesh.
 * Indices of important per-vertex data
 */
//@{
#define VDATA_SELECT  0		//!< The vertex soft selection data. This is index 0.
#define VDATA_WEIGHT  1		//!< The vertex weight data. This is index 1.
#define VDATA_ALPHA   2		//!< Vertex Alpha values
#define VDATA_CORNER  3		//!< Cornering values for subdivision use 
//@}

// Related constants:
#define MAX_WEIGHT ((float)1e5)
#define MIN_WEIGHT ((float)1e-5)

/** \defgroup Data_Types_for_Mash_Vertices Data Types for Mash Vertices
 * There is currently only one type of data supported for mesh vertices: 
 * floating point values. This however might be extended in the future.
 */
//@{
#define PERDATA_TYPE_FLOAT 0	//!< The floating point data type.
//@}

// Vertex-specific methods:
/** Returns the type of data supported, i.e.
PERDATA_TYPE_FLOAT.
\deprecated \param vdID This parameter is ignored. */
DllExport int VertexDataType (int vdID);

/** Returns a pointer to a default floating point value for the
specified channel.
\param vdID One of the values from \ref Vertex_Data_Index_Options 
 */
DllExport void *VertexDataDefault (int vdID);

/** This class is used for per -'something' floating-point information. For
example, it is used with Meshes to keep track of such per-vertex
information as weighted (Affect Region or Soft) selections and vertex weights.
It is used in MNMesh to store per-edge data (edge weights). 
Currently there's only one "type" of data supported, floating point values, but
this may be extended in the future. PerData arrays in Meshes and MNMeshes
cannot be reserved for plug-ins at this time; 3ds Max maintains the list in
MESH.H of the reserved vertex data channels, and in MNMESH.H for the MNEdge
data channels. 
The methods of this class are deliberately made to look like Tab\<\> methods.
All methods of this class are implemented by the system.
\see  Class Mesh. 
*/
class PerData: public MaxHeapOperators {
public:
	/** The number of elements of per-vertex data. */
	int dnum;
	/** The type of data held by this class. See \ref Data_Types_for_Mash_Vertices .*/
	int type;
	/** The number of elements currently allocated in the data array. */
	int alloc;
	/** Points to the actual data. */
	void *data;

	/** Constructor. The number of elements is set to 0, the type is
	set to 0 and the data pointer is set to NULL. */
	PerData () { data=NULL; dnum=0; alloc=0; type=0; }
	/** Constructor. 
	\param n 
	The number of elements to allocate. 
	\param tp 
	The type to set. */
	PerData (int n, int tp) { data=NULL; dnum=0; alloc=0; type=tp; setAlloc (n, FALSE); }
	/** Destructor. Any allocated data is freed and the count and
	type are set to 0. */
	~PerData () { Clear (); }

	// Following only depend on type:
	/** Allocates and returns a pointer to an array of floats of the
	specified size.
	\param num The number of floats to allocate. */
	DllExport void *AllocData (int num);

	/** Deletes the specified array of floats.
	\param addr Pointer to the array of floats to free. */
	DllExport void FreeData (void *addr);

	/** Returns the number of bytes used by the base data type for
	the vertex data. This is only implemented for a type of
	VDATA_TYPE_FLOAT in which case it returns sizeof(float).
	Other cases simply return 0. */
	DllExport int DataSize ();

	/** Returns the address of the specified element in the array
	passed.
	\param ptr The array whose at-th element address is returned. 
	\param at The zero based index of the element. */
	void *Addr (void *ptr, int at) { BYTE *vd=(BYTE *)ptr; return (void *)(vd+at*DataSize()); }

	/** Returns the address of the specified element in the
	data array.
	\param at The zero based index of the element. */
	void *Addr (int at) { return Addr(data,at); }

	/** Copies the specified number of elements between the two data
	arrays.
	\param to Points to the destination data array. 
	\param from Points to the source data array. 
	\param num The number of elements to copy. */
	DllExport void CopyData (void *to, void *from, int num=1);

	/** Copies the specified number of elements between the two
	specified locations in the data array.
	\param to The zero based index into the data array of the destination. 
	\param from The zero based index into the data array of the source. 
	\param num The number of elements to copy. */
	void CopyData ( int to,  int from, int num=1) { CopyData (Addr(to), Addr(from), num); }

	/** Computes the weighted sum of the arguments passed. This is
	effectivly c = a*prop1 + b*prop2. 
	This is used, for example, in splitting an edge, where we would want to
	interpolate the vertex weight values from the edge's endpoints to create
	the weight for the new vertex.
	\param to A pointer to the location in which the result should be stored. 
	\param fr1 A pointer to the first value to be summed. 
	\param prop1 The weight given to the first value. 
	\param fr2 A pointer to the second value. 
	\param prop2 The weight given to the second value. */
	DllExport void WeightedSum (void *to, void *fr1, float prop1, void *fr2, float prop2);

	/** Computes the weighted sum of the arguments passed. This is
	similar to the method above except to, fr1, and fr2 are indices of the
	values in the PerData array. That is, PerData::WeightedSum (c, a, prop1, b,
	prop2), where a, b, and c are ints between 0 and PerData::dnum-1, is
	equivalent to the call PerData::WeightedSum (PerData::Addr(c),
	PerData::Addr(a), prop1, PerData::Addr(b), prop2).
	\param to The index in the PerData array of the location in which the result should
	be stored. 
	\param fr1 The index of the first value to be summed in the PerData array. 
	\param prop1 The weight given to the first value. 
	\param fr2 The index of the second value to be summed in the PerData array. 
	\param prop2 The weight given to the second value. */
	void WeightedSum (int to, int fr1, float prop1, int fr2, float prop2) { WeightedSum (Addr(to), Addr(fr1), prop1, Addr(fr2), prop2); }

	/** Sets the number of elements allocated in the data
	array.
	\param num The number of elements to allocate. 
	\param keep	If TRUE previous values are kept (copied to the new storage); otherwise
	they are discarded. Defaults to TRUE.*/
	DllExport void setAlloc (int num, BOOL keep=TRUE);

	/** Sets the number of elements allocated in the data
	array and sets the dnum member to num.
	\param num The number of elements to allocate. 
	\param keep If TRUE previous values are kept (copied to the new storage); otherwise
	they are discarded. Defaults to FALSE.*/
	void SetCount (int num, BOOL keep = FALSE) { setAlloc (num, keep); dnum=num; }

	/** Reduces the size of the data array to contain
	dnum elements. */
	void Shrink () { if (alloc>dnum) setAlloc(dnum); }

	/** Returns the number of elements used (dnum) */
	int Count () { return dnum; }

	/** Clears (deletes) any allocated data and sets the count and
	type to 0. */
	DllExport void Clear ();

	/** Removes any element whose corresponding element in the
	BitArray is not set.
	\param del Specifies which elements to delete. Data elelemts corresponding to bits
	that are on remain; for bits that are off the elements are deleted. */
	DllExport void DeleteSet (BitArray del);

	/** Deletes the specifiec number of elements from the specified
	location in the data array.
	\param at The location to delete elements. 
	\param num The number of elements to delete. */
	DllExport void Delete (int at, int num);

	/** Inserts the specified number of data elements into the
	specified location in the data array.
	\param at The zero based index of the location for the insert. 
	\param num The number of elements to insert. 
	\param el The data to insert. */
	DllExport void Insert (int at, int num, void *el);

	/** Appends the specified elements to the data array.
	\param num The number of elements to append. 
	\param el The data to append. */
	DllExport void Append (int num, void *el);

	/** Inserts the specified number of elements into the data
	array at the given location.
	\param at The zero based index of the location to insert the data. 
	\param num The number of elements to insert. 
	\param el The data to insert.*/
	DllExport void InsertCopies (int at, int num, void *el);

	/** Appends the specified number of elements to the data
	array.
	\param num The number of elements to append. 
	\param el The data to append. */
	DllExport void AppendCopies (int num, void *el);

	/** Swaps the contents of this PerData object and the specified
	one.
	\param from The object to swap with. */
	DllExport void SwapContents (PerData & from);

	/** Assignment operator.
	\param from The VertexData source. */
	DllExport PerData & operator= (const PerData & from);
	DllExport void MyDebugPrint ();
};

// Mesh::flags definitions
/** \defgroup MeshVertexFaceFlags Mesh, Vertex and Face Flags
The following represent flags describing various flags that can be set on the mesh, vertices and faces.  Not all
flags can be applied to each sub object type.
*/
//@{
#define MESH_EDGE_LIST     (1<<1)			//!< \deprecated This flag is obsolete.
#define MESH_LOCK_RENDDATA (1<<2)			//!< Setting this flag prevents render data from being deleted (except when the mesh is deleted).
#define MESH_SMOOTH_BIT1   (1<<3)
#define MESH_SMOOTH_BIT2   (1<<4)
#define MESH_SMOOTH_BIT3   (1<<5)
#define MESH_SMOOTH_BIT4   (1<<6)
#define MESH_SMOOTH_MASK   0x78				//!< Flag mask for SMOOTH_BIT's 1 thru 4
#define MESH_BEEN_DSP	   (1<<9)			//!< \internal Mesh? flag that marks the mesh as has been drawn I think
/**  Having this bit set to 1 indicates that the smooth faces with selection-color outlines should be displayed.
Having this reset to 0 means the transparent shaded faces should be displayed. */
#define MESH_SMOOTH_SUBSEL (1<<10)			
#define MESH_FACENORMALSINVALID  (1<<11)	//!< Mesh flag that Signals that face normals should be rebuilt
#define MESH_CACHEINVALID  (1<<12)			//!< Mesh, vertices and Face flag Signals that the they have changed and RVertices normals should be rebuilt 
#define MESH_BACKFACESCOMPUTED (1<<16)		//!< Mesh flag to determine whether the back facing attribute on the face list has been computed or not
#define MESH_PARTIALCACHEINVALID (1<<17)	//!< Mesh flag that means that the mesh has only set some of the vertices as invalid and not to reprocess the entire mesh just the vertices that changed
//@}

// For internal use only!
#define MESH_TEMP_1 (1<<13)
#define MESH_TEMP_2 (1<<14)
#define MESH_DONTTRISTRIP (1<<15)	//!< \internal Mesh flag to determine whether to use tristripping or not

class StripData: public MaxHeapOperators {
public:
	int ct;
	DWORD f[6];
	void AddFace(DWORD face)
		{ if(ct < 6) f[ct++] = face; }
};

typedef int (*INTRFUNC)();

DllExport void setMeshIntrFunc(INTRFUNC fn);


/** This class allows access to the sub-object hit records used in Mesh hit
testing. All methods of this class are implemented by the system.
\see  Class Mesh, Class BitArray. 
 */
class MeshSubHitRec: public MaxHeapOperators {
	private:		
		MeshSubHitRec *next;
	public:
		/** The distance of the hit. If the user is in wireframe mode, this is the distance
		in pixels to the item that was hit. If the user is in shaded mode, this is the
		Z depth distance. Smaller numbers indicate a closer hit. 
		*/
		DWORD	dist;
		/** The index of the sub-object component. For example, if faces were being hit
		tested, this would be the index of the mesh's BitArray faceSel. For
		edges, this is the index into the edgeSel BitArray, where the index is
		3*faceIndex+edgeIndex. 
		*/
		int		index;
		/** \deprecated These are not currently used.*/
		DWORD	flags;

		/** Constructor. The data members are initialized to the data
		members passed. */
		MeshSubHitRec(DWORD dist, int index, MeshSubHitRec *next) 
			{this->dist = dist; this->index = index; this->next = next;}

		/** Constructor. The data members are initialized to the data
		members passed. */
		MeshSubHitRec(DWORD dist, int index, DWORD flags, MeshSubHitRec *next) 
			{this->dist = dist; this->index = index; this->next = next;this->flags = flags;}

		/** Returns the next mesh sub hit record. */
		MeshSubHitRec *Next() { return next; }		
	};

/** This class describes a list of sub-object hit records. All methods of this
class are implemented by the system.  
\see  Class MeshSubHitRec. */
class SubObjHitList: public MaxHeapOperators {
	private:
		MeshSubHitRec *first;
	public:
		/** Constructor. The list is set to NULL. */
		SubObjHitList() { first = NULL; }

		/** Destructor. All the hit records are deleted. */
		DllExport ~SubObjHitList();

		/** Returns the first item in the hit list. */
		MeshSubHitRec *First() { return first; }

		/** Allocates and adds a new hit record to the list.
		\param dist The distance of the hit. 
		\param index The index of the hit. */
		DllExport void AddHit( DWORD dist, int index );
	};



/// \defgroup Flags_for_Sub_Object_Hit_Test Flags for Sub Object Hit Test
/// These are the same bits used for object level.
//@{

#define SUBHIT_SELONLY		(1<<0)	//!< Selected only.
#define SUBHIT_UNSELONLY	(1<<2)	//!< Unselected only.
#define SUBHIT_ABORTONHIT	(1<<3)	//!< Abort hit testing on the first hit found.
/** This treats selected items as solid and unselected items as not solid. 
 * Treating an item as solid means the face will be hit if the mouse is anywhere 
 * inside the face region and not just over a visible edge.
 */
#define SUBHIT_SELSOLID		(1<<4)

#define SUBHIT_USEFACESEL	(1<<23)	//!< When this bit is set, the sel only and unsel only tests will use the faces selection when doing a vertex level hit test
#define SUBHIT_VERTS		(1<<24)	//!< Hit test vertices.
#define SUBHIT_FACES		(1<<25)	//!< Hit test faces.
#define SUBHIT_EDGES		(1<<26)	//!< Hit test edges.
#define SUBHIT_TYPEMASK		(SUBHIT_VERTS|SUBHIT_FACES|SUBHIT_EDGES)
//@}

/** \defgroup Mesh_Selection_Level_Bits Mesh Selection Level Bits
 * These are the list of arguments for Mesh::selLevel current level of selection. 
 * When all the bits are 0, the object is at object level selection. */
//@{
#define MESH_OBJECT		(1<<0)	//!< Object level.
#define MESH_VERTEX		(1<<1)	//!< Vertex level.
#define MESH_FACE		(1<<2)	//!< Face level.
#define MESH_EDGE		(1<<3)	//!< Edge level.
//@}

/** \defgroup Mesh_Normal_Display_Flags Mesh Normal Display Flags
 * These bits Control the display of surface normals on the mesh object. 
 * Note that there may be more than one normal per vertex if faces 
 * that share the vertex are in non-overlapping smoothing groups. 
 * In this case, all normals associated with the given vertex are drawn. 
 */
//@{
#define MESH_DISP_NO_NORMALS		0		//!< Turn off normal display.
#define MESH_DISP_FACE_NORMALS		(1<<0)	
#define MESH_DISP_VERTEX_NORMALS	(1<<1)
//@}


class MeshOpProgress;
class UVWMapper;

/** A developer may derive a class from this class, put any required data in it,
and then hang this data off a Mesh. This is done using the methods of Class
Mesh: 
void SetRenderData(MeshRenderData *p); 
Sets the mesh render data hung off this Mesh. 
MeshRenderData * GetRenderData(); 
Gets a pointer to the MeshRenderData.  
\see Class Mesh. 
*/
class MeshRenderData: public MaxHeapOperators {
	public:
	/** Destructor. */
	virtual ~MeshRenderData() {;}

	/** Deletes this instance of the class. */
	virtual void DeleteThis()=0;
	};

class AdjFaceList;

namespace MaxGraphics {
	class IMeshInternal;
}

/** The Mesh class is provided for use by plug-ins and is used by the system. It is
the data structure for meshes in the system and maintains pointers to the
vertices, faces, texture vertices, etc. It provides methods to handle most of
the functionality that procedural objects must implement. All methods of this
class are implemented by the system. See the page Working with Meshes.
Note: There is a set of classes for working with parts of a mesh such as its
face structure, element structure, and cluster structure. For details see:
Class AdjEdgeList, Class AdjFaceList, Class FaceElementList, Class FaceClusterList.
\see  Class BitArray, Class Face,  Class TVFace,  Class Point3, Class TriObject
\see Class RNormal, Class RVertex, Class TriObject, Class MeshMap, 
*/
class Mesh : public BaseInterfaceServer {
	friend class Face;
	friend class MeshAccess;
	friend class HardwareMesh;
	friend class MaxGraphics::IMeshInternal;
	friend void gfxCleanup(void *data);

	private:

#if MESH_MULTI_PROCESSING
		static int		refCount;
		static HANDLE	xfmThread;
		static HANDLE	xfmMutex;
		static HANDLE	xfmStartEvent;
		static HANDLE	xfmEndEvent;
		friend DWORD WINAPI xfmFunc(LPVOID ptr);
		static HANDLE	fNorThread;
		static HANDLE	fNorMutex;
		static HANDLE	fNorStartEvent;
		static HANDLE	fNorEndEvent;
		friend DWORD WINAPI fNorFunc(LPVOID ptr);

		static HANDLE	workThread;
		static HANDLE	workMutex;
		static HANDLE	workStartEvent;
		static HANDLE	workEndEvent;
		friend DWORD WINAPI workFunc(LPVOID ptr);
#endif
		// derived data-- can be regenerated
		RVertex*        rVerts;         // instance specific.
		GraphicsWindow* cacheGW;        // identifies rVerts cache
		Point3*         faceNormal;     // object space--depends on geom+topo
		Box3            bdgBox;         // object space--depends on geom+topo
		int             numVisEdges;    // depends on topo 
		int             edgeListHasAll; // depends on topo
		VEdge*          visEdge;        // depends on topo 	
 
 		// Vertex and face work arrays -- for snap code
		int			snapVCt;
		int			snapFCt;
		char*          snapV;
		char*          snapF;

		// A simple edge list used so we can draw edges fast in object or vertex sub object mode
		Tab<DWORD>	mDisplayEdges;
		//these are counter of various derived data so we can track when to delete them when they become unused
		UINT		mDXMeshCacheDrawCount;  //this the number of times that the direct mesh cache has been used to draw the mesh exclusively
		UINT		mRedrawCountFromLoad;  //this is the number of times the mesh has been redrawn from load.  This is used to prevent an initial memory spike on load
											//this will be set to -1 if the mesh was created and not loaded

		// Reserved maps for special purposes, such as vertex shading.
		MeshMap hmaps[NUM_HIDDENMAPS];

		// -------------------------------------
		//
		long   		flags;		  	// work flags- 

		float 		norScale;	    // scale of normals -- couldn't this be done
		 							// automatically relative to bdgBox?




		// Rolf: these are instance specific and should be pulled out of here,
		// and just passed in from the Node.
		BYTE		dspNormals;    // display surface normals--- put in flags?
		BYTE		dspAllEdges;   // shows hidden edges  ---- put in flags?
		BYTE		dspVertTicks;  // shows vertex ticks

		//  This only draws the selected edges used to draw them over the directX cache 
		/**	This draws the selected edges of the mesh.  Backface culling is not supported
		\param gw The graphics window associated with the draw.
		*/
		void	DrawOnlySelectedEdges(GraphicsWindow *gw);

		//  This only draws the selected faces used to draw them over the directX cache 
		/**	This draws the selected faces of the mesh.  Backface culling is not supported
		\param gw The graphics window associated with the draw.
		*/
		void	DrawOnlySelectedFaces(GraphicsWindow *gw);

		int 		renderFace(GraphicsWindow *gw, DWORD index, int *custVis=NULL);
		int			renderEdge(GraphicsWindow *gw, DWORD face, DWORD edge);
		int			renderFaceVerts(GraphicsWindow *gw, DWORD index);
		void		renderStrip(GraphicsWindow *gw, Strip *s);
		void 		render3DFace(GraphicsWindow *gw, DWORD index, int *custVis=NULL);
		void		render3DFaceVerts(GraphicsWindow *gw, DWORD index);
		void		render3DStrip(GraphicsWindow *gw, Strip *s);
		void		render3DWireStrip(GraphicsWindow *gw, Strip *s);
		BOOL		CanDrawStrips(DWORD rndMode, Material *mtl, int numMtls);
		BOOL		NormalsMatchVerts();
		void		checkRVertsAlloc(void);
		void  		calcNormal(int i);
		void		buildFaceNormals();		// calcs just the face normals
		void		setCacheGW(GraphicsWindow *gw)	{ cacheGW = gw; }
		GraphicsWindow *getCacheGW(void)			{ return cacheGW; }

		// New Mesh routines to drive HardwareShaders
		bool		CanDrawTriStrips(DWORD rndMode, int numMtls, Material *mtl);
		bool		BuildTriStrips(DWORD rndMode, int numMtls, Material *mtl);
		void		TriStripify(DWORD rndMode, int numTex, TVFace *tvf[], TriStrip *s, StripData *sd, int vtx);
		void 		Draw3DTriStrips(IHardwareShader *phs, int numMat, Material *ma);
		void 		Draw3DWireTriStrips(IHardwareShader *phs, int numMat, Material *ma);
		void 		Draw3DVisEdgeList(IHardwareShader *phs, DWORD flags);
		int			render3DTriStrips(IHardwareShader *phs, int kmat, int kstrips);
		int			render3DWireTriStrips(IHardwareShader *phs, int kmat, int kstrips);
		int 		render3DFaces(IHardwareShader *phs, DWORD index, int *custVis=NULL);

		void 		freeVerts();
		void        freeVertCol();
		void  		freeFaces();
		void		freeFaceNormals();  
		void  		freeRVerts(BOOL forceDelete=FALSE);
		void  		freeTVerts();
		void  		freeTVFaces();
		void  		freeVCFaces();
		void		freeSnapData();
		int			buildSnapData(GraphicsWindow *gw,int verts,int edges);
		void		ComputeBackFaceFlags(GraphicsWindow *gw);

		//Just some temporary structs to hold a list of only changed faces and verts.  They are used when building normals.
		BitArray	mInvalidVert;
		BitArray	mInvalidFace;
		Tab<int>	mInvalidVertList;
		Tab<int>	mInvalidFaceList;
		//Frees one rvert in the list
		void		FreeRVert(int index);
		void		ComputeFaceAngle(int index, float *angles);
		void		ComputeRVert(int index, float *faceAngles);

	public:

		
		// Topology
		int			numVerts;	//!< Number of vertices. 
		int	 		numFaces;	//!< Number of faces. 
		Face *		faces;		//!< Number of faces. 

		// Geometry
		Point3 *	verts;		//!< Array of vertex coordinates.

		// Texture Coord assignment 
		int			numTVerts;	//!< Number of texture vertices. 
		/** The array of texture vertices. This stores the UVW coordinates. For a 2D
			mapping only two of them are used, i.e. UV, VW, or WU. This just provides
			greater flexibility so the user can choose to use UV, VW, or WU. 
			Note: typedef Point3 UVVert; */
		UVVert *	tVerts;
		/** The array of texture faces. There needs to be one TVFace for every face,
			but there can be three indices into the UVVert array that are any UVs. Each
			face of the object can have its own mapping. 
		*/
		TVFace *	tvFace;  	 

		// Color per vertex
		int numCVerts;			//!< Number of color vertices. 
		VertColor * vertCol;	//!< Array of color vertices. 
		TVFace *    vcFace;		//!< Array of color per vertex faces. 

		/** Storage for the current mapping channel to use for vertex colors (default =
		0). */
		int			curVCChan;	
		/** Points to storage for a possible external color array (default = NULL). This
		can be either an external array or one of the mapping channels. See the method
		Mesh::setVCDisplayData(); */
		VertColor *	curVCArray;	
		TVFace	  * curVCFace;	//!< possible external face array (default = NULL)

		/** When 3ds Max is rendering the color values come from this variable. This array
		defaults to the internal vertCol but can be set to an external array, or
		a map channel. See the method Mesh::setVCDisplayData(); */
		VertColor * vertColArray;
		
		/** When 3ds Max is rendering the vertex color lookup comes from this structure.
		This defaults to the vcFace data but if a mapping channel is used for
		color lookup, its TVFace structure is used.*/
		TVFace *	vcFaceData;

		// More maps:
		/** The number of maps supported by the mesh. By default this is 2 but may be
		changed with the multiple map functions in this class. */
		int numMaps;
		/** When the number of mapping channels is set to a value greater than 1 then an
		instance of this class is allocated for each channel up to numMaps. An
		instance maintains the mapping information for a single channel. */
		MeshMap *maps;

		
		/** This bit array indicates if a particular vertex data channel is supported in
		this mesh. If the bit is set the channel is supported. 
		*/
		BitArray vdSupport;
		/** The array of PerData objects which maintain and provide access to the floating
		point vertex data. There is one of these for each supported channel. The first
		two PerData objects in this array are used internally by 3ds Max.*/
		PerData *vData;

		// Material assignment
		MtlID		mtlIndex;		//!< object material

		// Selection
		/** Indicates the selected vertices. There is one bit for each vertex. Bits that
		are 1 indicate the vertex is selected. */
		BitArray	vertSel; 
		/** Indicates the selected faces. There is one bit for each face. Bits that are 1
		indicate the face is selected. */
		BitArray	faceSel; 
		/** Indicates the selected edges. There is one bit for each edge of each face. Bits
		that are 1 indicate the edge is selected. The edge is identified by
		3*faceIndex + edgeIndex. */
		BitArray	edgeSel;
		/** Hidden flags for vertices.*/
		BitArray	vertHide;

		/** Display attribute flags. They control various aspect of the Mesh objects display and may be one or more
		of the following values: 
		\li DISP_VERTTICKS - Display vertices as small tick marks. 
		\li DISP_SELVERTS - Display selected vertices. 
		\li DISP_SELFACES - Display selected faces. 
		\li DISP_SELEDGES - Display selected edges. 
		\li DISP_SELPOLYS - Display selected polygons. Polygons are defined as
		adjacent triangles with hidden edges. A selected face would show all edges
		regardless of if they were hidden edges. A polygon would only show the edges of
		the polygon that were not hidden.*/
		DWORD		dispFlags;

		/** Selection level flags. This is the current level of selection. When all the bits are 0, the object is
		at object level selection. The selection level bits are: 
		\li MESH_OBJECT- Object level. 
		\li MESH_VERTEX - Vertex level. 
		\li MESH_FACE - Face level. 
		\li MESH_EDGE - Edge level.*/
		DWORD		selLevel;

		// true if normals have been built for the current mesh
		// SCA revision for Max 5.0 - value of 0 for unbuilt, 1 for built according to legacy 4.0 scheme,
		// and 2 for built according to new scheme - this is so that we refresh normals if the user changes
		// the "Use Legacy 4.0 Vertex Normals" checkbox.
		/** Nonzero if normals have been built for the current mesh; 0 otherwise.*/
		int			normalsBuilt;

		/** Points to the render data used by the renderer.*/
		MeshRenderData*	 renderData;  

		// derived data-- can be regenerated
		StripTab* stab;        //!< depends on topo
		DWTab     norInd;      //!< indirection array for fast normal lookup
		int       normalCount; //!< total number of normals
		Point3*   gfxNormals;  //!< flattened list of normals pointers back in the rverts data.  We need to get rid of this or something this is big memory eater

		//Builds the GFX Normals
		void		BuildGFXNormals();

		
		int			numTexCoords[GFX_MAX_TEXTURES]; //!< Derived arrays to contain generated texture coordinates.
		Point3 *	texCoords[GFX_MAX_TEXTURES];	//!< Derived arrays to contain generated texture coordinates.

		/// Derived table of TriStrips, depends on topology
		Tab<TriStrip *>	*tstab;

		/** Constructor. Initializes the mesh object. The mesh counts
		are set to 0 and its pointers are set to NULL. */
		DllExport Mesh();
		/** Constructor. The mesh is initialized equal to
		fromMesh. */
		DllExport Mesh(const Mesh& fromMesh);
		/** Destructor. Frees any allocated arrays (faces, verts,
		tverts, tvfaces). */
		DllExport ~Mesh(); 
		/** \internal Initializes the mesh object. The mesh counts are set to 0
		and its pointers are set to NULL. Note: This method is not intended to
		be called by developers. It is used internally. */
		DllExport void Init();
		/** 
 
		This method deletes this mesh. */
		DllExport void DeleteThis();

		/** Assignment operator. Note: This operator does not copy
		the rVerts array. This means that developers who have specified
		normals will have to explicitly copy them after a mesh assignment is
		done using this operator. This is because rVerts are
		instance-specific. In general, normals are computed from smoothing
		groups, and hence are "generated data". In the case where normals are
		specified, 3ds Max stores the data in the rVerts array so as to
		not waste space. The way 3ds Max uses the mesh = operator assumes that
		the rVerts are not copied, and thus developers must do the
		copying themselves.
		\param fromMesh Specifies the mesh to copy. */
		DllExport Mesh& operator=(const Mesh& fromMesh);
		
		/** Sets the number of geometric vertices in the mesh.
		\param ct Specifies the number of vertices. 
		\param keep Specifies if the previous vertices should be kept. If TRUE the previous
		vertices are kept; otherwise they are discarded. Defaults to FALSE.
		\param synchSel	This parameter is available in release 2.0 and later only. 
		If TRUE the selection set BitArrays are resized to fit the number of
		vertices; otherwise they are left unaltered. Defaults to TRUE.
		\return  TRUE if storage was allocated and the number of vertices was
		set; otherwise FALSE. */
		DllExport BOOL 	setNumVerts(int ct, BOOL keep=FALSE, BOOL synchSel=TRUE);
		/** Returns the number of vertices. */
		int				getNumVerts(void) const	{ return numVerts; }
		
		/** Sets the number of faces in the mesh.
		\param ct Specifies the number of faces. 
		\param keep	Specifies if the previous faces should be kept. If TRUE the previous
		faces are kept; otherwise they are discarded. Defaults to FALSE.
		\param synchSel If TRUE the selection set BitArrays are resized to fit the number of
		faces; otherwise they are left unaltered. Defaults to TRUE.
		\return  TRUE if storage was allocated and the number of faces was set;
		otherwise FALSE. */
		DllExport BOOL	setNumFaces(int ct, BOOL keep=FALSE, BOOL synchSel=TRUE);

		/** Returns the number of faces in the mesh. */
		int				getNumFaces(void) const{ return numFaces; }
		
		// Original mapping coordinates (map channel 1)
		/** Sets the number of texture vertices (in mapping channel
		1).
		\param ct Specifies the number of texture vertices. 
		\param keep	Specifies if the previous texture vertices should be kept. If TRUE the
		previous texture vertices are kept; otherwise they are discarded. Defaults to FALSE.
		\return  TRUE if storage was allocated and the number of texture
		vertices was set; otherwise FALSE. */
		DllExport BOOL	setNumTVerts(int ct, BOOL keep=FALSE);

		/** Returns the number of texture vertices (in mapping
		channel 1). */
		int				getNumTVerts(void) const { return numTVerts; }

		/** Sets the number of TVFaces. This method is automatically
		called if you set the number of faces to keep these two in sync
		(because the number of TVFaces should be the same as the number of
		faces). The following rules apply: 
		If you have no TVFaces and keep is TRUE then the TVFaces array
		stays empty. 
		If you have no TVFaces and keep is FALSE they are allocated. 
		If you have TVFaces and ct = 0 then the TVFaces are freed.
		\param ct The number of TVFaces. 
		\param BOOL keep Specifies if the old faces should be kept. 
		\param oldCt The length of the existing TVFaces array.
		\return  TRUE if storage has been allocated and the number is set;
		otherwise FALSE. */
		DllExport BOOL 	setNumTVFaces(int ct, BOOL keep=FALSE, int oldCt=0);

		// Color per vertex array (map channel 0)
		// these methods only affect the vertColArray, even when the vertex colors
		// come from a different array (as set by the setCVertArray method below)
		/** Sets the number of color per vertex vertices.
		\param ct The number of color vertices to set. 
		\param keep	If TRUE previous values are kept; otherwise they are discarded.
		\return  TRUE if the value was set; otherwise FALSE. */
		DllExport BOOL 	setNumVertCol(int ct,BOOL keep=FALSE);
		/** Returns the number of color per vertex vertices. */
		int             getNumVertCol() const {return numCVerts;}
		/** Sets the number of color per vertex faces.
		\param ct The number of color per vertex faces to set. 
		\param keep	Specifies if the old faces should be kept if the array is being
		resized. If FALSE they are freed. 
		\param oldCt The length of the existing VCFaces array.
		\return  TRUE if storage has been allocated and the number is set;
		otherwise FALSE. */
		DllExport BOOL 	setNumVCFaces(int ct, BOOL keep=FALSE, int oldCt=0);

		// To use a different source array for displaying vertex color data:
		//  -- to use a different map channel, call with args: mapChanNum, NULL, NULL
		//  -- to use an external array, call with: MESH_USE_EXT_CVARRAY, vcArray, face_data_if_available
		//     (if no face array is supplied, then we will use the internal vertex color face array)
		//  -- to revert to the internal color vert array, call with "0 , NULL, NULL" (or no args)

		// This method would typically be called right before display, as with a node display callback, 
		// or through an extension object.
		/** This method would typically be called right before display, as with a
		node display callback, or through an extension object. If mapChan
		parameter is set to MESH_USE_EXT_CVARRAY then the data in VCArray and
		TVFace is stored for internal use and consequent drawing. If the arrays
		are NULL then the internal source is used. 
		\param mapChan the mapping channel to use. 
		\param VCArray An external array hosting the vertex colors 
		\param VCF An external array of TVFace indexing into the color array 
		  */
		DllExport void	setVCDisplayData(int mapChan = 0, VertColor *VCArray=NULL, TVFace *VCf=NULL);

		// For mp in following: 0=vert colors, 1=original TVerts, 2&up = new map channels
		/** Set the number of texture maps used by this Mesh. Note that this call
		is made automatically if Mesh::setMapSupport() is called.
		\param ct The number of texture maps to use. This is a value between 2 and
		MAX_MESHMAPS-1. 
		\param keep	TRUE to keep the old mapping information after the resize; FALSE to
		discard it. */
		DllExport void setNumMaps (int ct, BOOL keep=FALSE);

		/**	Returns the number of mapping channels in use. */
		int getNumMaps () const { return numMaps; }

		/** Returns TRUE if the specified mapping channel is supported; otherwise
		FALSE.
		\param mp Specifies the channel. See the page "List of Mapping Channel Index Values". */
		DllExport BOOL mapSupport (int mp) const;

		/**	Sets whether the specified mapping channels is supported or not.
		\param mp Specifies the channel. See the page "List of Mapping Channel Index Values". 
		\param support TRUE to indicate the channel is supported; otherwise FALSE. */
		DllExport void setMapSupport (int mp, BOOL support=TRUE);

		/**	Sets the number of texture or vertex color vertices for the specified
		mapping channel of this mesh.
		\param mp Specifies the channel. See the page "List of Mapping Channel Index Values".
		\param ct The number of vertices to allocate. 
		\param keep	If TRUE previous values are kept; otherwise they are discarded. */
		DllExport void setNumMapVerts (int mp, int ct, BOOL keep=FALSE);

		/**	Returns the number of texture or vertex color vertices for the
		specified channel of this mesh.
		\param mp Specifies the channel. See "List of Mapping Channel Index Values". */
		DllExport int getNumMapVerts (int mp) const;
		/**	Sets the number of texture or vertex color faces for the specified
		channel of this mesh.
		\param mp Specifies the channel. See the page "List of Mapping Channel Index Values". 
		\param ct The number of faces to allocate. 
		\param keep	If TRUE previous values are kept; otherwise they are discarded. */
		DllExport void setNumMapFaces (int mp, int ct, BOOL keep=FALSE, int oldCt=0);

		/** Returns a pointer to the list of UVVerts for the specified
		channel of this mesh.
		\param mp Specifies the channel. See the page "List of Mapping Channel Index Values".  */
		DllExport UVVert *mapVerts (int mp) const;

		/** Returns a pointer to the list of TVFaces for the specified
		channel of this mesh.
		\param mp Specifies the channel. See the page "List of Mapping Channel Index Values".  */
		DllExport TVFace *mapFaces (int mp) const;

		/** Sets a single texture or vertex color value for the specified channel
		of this mesh.
		\param mp Specifies the channel. See the page "List of Mapping Channel Index Values".  
		\param i The zero based index of the vertex to set. 
		\param xyz The value to set. */
		void		setMapVert (int mp, int i, const UVVert&xyz) { if (mapVerts(mp)) mapVerts(mp)[i] = xyz; }

		/** Applies a simple planar mapping to the specified channel. This is done
		by copying the mesh topology and vertex locations into the map.
		\param mp Specifies the channel. See the page "List of Mapping Channel Index Values".  */
		DllExport void MakeMapPlanar (int mp);	// Copies mesh topology, vert locations into map.
		
		/** Returns a BitArray with a bit set for each isolated vertex
		(un-referenced by any face) for the specified channel.
		\param mp Specifies the channel. See the page "List of Mapping Channel Index Values".  */
		DllExport BitArray GetIsoMapVerts (int mp);

		/** Deletes the map vertices indicated.
		\param mp Specifies the channel. See the page "List of Mapping Channel Index Values".  
		\param set Indicates which map verts should be deleted. set.GetSize() should equal
		this mesh's getNumMapVerts(mp). 
		\param fdel If non-NULL, this points to a BitArray that will be filled in with the
		faces that will need to be deleted or have new map verts assigned
		because they used a map vert that was deleted. (The size will be set to
		this mesh's numFaces.) */
		DllExport void DeleteMapVertSet (int mp, BitArray set, BitArray *fdel=NULL);

		/** This method deletes each isolated vertex (un-referenced by any face)
		for the all active maps. */
		DllExport void DeleteIsoMapVerts ();	//	 do all active maps

		/** This method deletes each isolated vertex (un-referenced by any face)
		for the specified channel.
		\param mp Specifies the channel. See the page "List of Mapping Channel Index Values".  The default value of -1 indicates to
		do all active maps. */
		DllExport void DeleteIsoMapVerts (int mp);

		/** Deallocates the texture or vertex color vertices for the specified
		channel of this mesh.
		\param mp Specifies the channel. See the page "List of Mapping Channel Index Values".  */
		DllExport void freeMapVerts (int mp);

		/** Deallocates the texture or vertex color faces for the specified channel
		of this mesh.
		\param mp Specifies the channel. See the page "List of Mapping Channel Index Values".  */
		DllExport void freeMapFaces (int mp);

		/** This method returns the map for the specified map channel.
		\param mp 
		The map channel. */
		MeshMap & Map(int mp) { return (mp<0) ? hmaps[-1-mp] : maps[mp]; }

		/** Sets the number of channels of vertex data used by the mesh.
		\param ct The number of elements of vertex data to set. 
		\param keep	If TRUE any old vertex data is kept; otherwise it is discarded. */
		DllExport void setNumVData (int ct, BOOL keep=FALSE);

		/** Returns the number of vertex data channels maintained by this mesh. */
		int getNumVData () const { return vdSupport.GetSize(); }

		/** Returns TRUE if the specified channel of vertex data is available for
		this mesh; otherwise FALSE.
		\param vd The vertex data channel. See the page "List of Vertex	Data Index Options". */
		DllExport BOOL vDataSupport (int vd) const;

		/** Sets if the specified channel of vertex data is supported by this mesh.
		\param vd The vertex data channel. See the page "List of Vertex	Data Index Options". 
		\param support TRUE to indicate the channel is supported; FALSE to indicate it's not.
		If TRUE is specified then numVerts elements are allocated (if
		needed). If FALSE is specified the data for the channel is freed. */
		DllExport void setVDataSupport (int vd, BOOL support=TRUE);

		/** Returns a pointer to the vertex data for the specified channel or NULL
		if the channel is not supported. If supported then the size of this
		array is numVerts.
		\param vd The vertex data channel. See the page "List of Vertex	Data Index Options".*/
		void *vertexData (int vd) const { return vDataSupport(vd) ? vData[vd].data : NULL; }

		/** Returns a pointer to the floating point vertex data for the specified
		channel of this mesh or NULL if the channel is not supported. If
		supported then the size of this array is numVerts.
		\param vd The vertex data channel. See the page "List of Vertex	Data Index Options".*/
		float *vertexFloat (int vd) const { return (float *) vertexData (vd); }

		/** Deallocates the vertex data for the specified chanel.
		\param vd The vertex data channel. See the page "List of Vertex	Data Index Options". */
		DllExport void freeVData (int vd);

		/** Deallocates the vertex data from all the channels and sets the number
		of supported channels to 0. */
		DllExport void freeAllVData ();

		// Two specific vertex scalars.
		/** Returns a pointer to the floating point vertex weight data. */
		float *getVertexWeights () { return vertexFloat(VDATA_WEIGHT); }

		/** Sets the channel support for the vertex weights channel
		(VDATA_WEIGHT). */
		void SupportVertexWeights () { setVDataSupport (VDATA_WEIGHT); }

		/** Clears (deallocates) the vertex weights channel data. */
		void ClearVertexWeights() { setVDataSupport (VDATA_WEIGHT, FALSE); }

		/** Deallocates the vertex weights channel data (same as
		ClearVertexWeights() above). */
		void freeVertexWeights () { freeVData (VDATA_WEIGHT); }

		/**Returns a pointer to the floating point vertex selection weights data.
		*/
		float *getVSelectionWeights () { return vertexFloat(VDATA_SELECT); }

		/** Sets the channel support for the vertex weights channel
		(VDATA_SELECT). */
		void SupportVSelectionWeights () { setVDataSupport (VDATA_SELECT); }

		/** Clears (deallocates) the vertex selection weights channel data. */
		void ClearVSelectionWeights() { setVDataSupport (VDATA_SELECT, FALSE); }

		/** Deallocates the vertex selection weights channel data (same as
		ClearVSelectionWeights() above). */
		void freeVSelectionWeights () { freeVData (VDATA_SELECT); }

		// these flags are restricted to 4 bits and force the topology (strips & edges)
		// to be invalidated when they change.  Used by primitives with smoothing checkboxes
		/** This method should be called when the user has clicked on the 'Smooth'
		check box in a procedural object. It invalidates the appropriate caches
		of the mesh so the display is updated properly. If this method is not
		called, the internal topology cache might prevent the mesh from
		appearing changed.
		\param f Nonzero indicates smoothed; zero unsmoothed. */
		DllExport void  setSmoothFlags(int f);

		/** Returns the state of the smooth flags. See setSmoothFlags()
		above. */
		DllExport int   getSmoothFlags();

		/** Sets a single vertex in the verts array.
		\param i A zero based index into the verts array of the vertex to
		store. 
		\param xyz Specifies the coordinate of the vertex. */
		void		setVert(int i, const Point3 &xyz)	{ verts[i] = xyz; }

		/** Sets a single vertex in the verts array.
		\param i A zero based index into the verts array of the vertex to
		store. 
		\param x 
		Specifies the X coordinate of the vertex. 
		\param y 
		Specifies the Y coordinate of the vertex. 
		\param z 
		Specifies the Z coordinate of the vertex. */
		void		setVert(int i, float x, float y, float z)	{ verts[i].x=x; verts[i].y=y; verts[i].z=z; }
		/** Sets a single texture vertex in the tVerts array.
		\param i 
		A zero based index into the tVerts array of the texture vertex
		to store. 
		\param xyz 
		Specifies the coordinate of the vertex. */
		void		setTVert(int i, const UVVert &xyz)	{ tVerts[i] = xyz; }
		/** Sets a single texture vertex in the tVerts array.
		\param i 
		A zero based index into the tVerts array of the texture vertex
		to store. 
		\param x 
		Specifies the X coordinate of the texture vertex. 
		\param y 
		Specifies the Y coordinate of the texture vertex. 
		\param z 
		Specifies the Z coordinate of the texture vertex. */
		void		setTVert(int i, float x, float y, float z)	{ tVerts[i].x=x; tVerts[i].y=y; tVerts[i].z=z; }
		
		/** Sets a single 'rendered' normal in the rVerts
		array of RVertex instances.
		\param i 
		A zero based index into the rVerts array of the normal to
		store. 
		\param xyz 
		The normal to store in device coordinates. This should be a unit
		vector. */
		DllExport void		setNormal(int i, const Point3 &xyz); 

		/** Returns the 'i-th' 'rendered' normal from the
		rVerts array.
		\param i 
		A zero based index into the rVerts array of the normal to get.
		*/
		DllExport Point3 &	getNormal(int i) const; // mjm - made const - 2.16.99

		/** Sets the 'i-th' face normal.
		\param i A zero based index into the face normal array of the normal to
		store. 
		\param xyz 
		The face normal to store. This should be a unit vector. */
		void		setFaceNormal(int i, const Point3 &xyz) { faceNormal[i] =  xyz; }

		/** Returns the 'i-th' face normal.
		\param i Specifies the index of the face normal to retrieve. */
		Point3 &	getFaceNormal(int i) { return faceNormal[i]; }

		/** Returns the 'i-th' vertex.
		\param i Specifies the index of the vertex to retrieve. */
		Point3 &	getVert(int i)		{ return verts[i];  }

		/** Returns a pointer to the 'i-th' vertex.
		\param i Specifies the index of the vertex address to retrieve. */
		Point3 *	getVertPtr(int i)	{ return verts+i; }

		/** Returns the 'i-th' texture vertex.
		\param i Specifies the index of the texture vertex to retrieve. */
		UVVert &	getTVert(int i)		{ return tVerts[i];  }

		/** Returns a pointer to the 'i-th' texture vertex.
		\param i Specifies the index of the texture vertex address to retrieve. */
		UVVert *	getTVertPtr(int i)	{ return tVerts+i; }

		/** This method returns the 'i-th' RVertex.
		\param i Specifies the index of the RVertex to retrieve. */
		RVertex &	getRVert(int i)		{ return rVerts[i]; }

		/** This method returns a pointer to the 'i-th' RVertex.
		\param i Specifies the index of the RVertex to retrieve. */
		RVertex *	getRVertPtr(int i)	{ return rVerts+i; }

		
		/** \deprecated This method is no longer used. */
		void		setMtlIndex(MtlID	i)	{ mtlIndex = i; }

		/** \deprecated This method is no longer used. */
		MtlID		getMtlIndex(void) 		{ return mtlIndex; }

		// Face MtlIndex access methods;
	    /** Retrieves the zero based material index of the 'i-th'
	    face.
	    \param i Specifies the face index. This is the zero based index into the
	    faces array. */
	    DllExport MtlID		getFaceMtlIndex(int i);

		/** Sets the material index of the 'i-th' face.
		\param i Specifies the face index. This is the zero based index into the
		faces array. 
		\param id The material index for the 'i-th' face. */
		DllExport void		setFaceMtlIndex(int i, MtlID id); 	
		
		/** This method resolves the normals on the RVertex
		array. If the Mesh already has normals at each vertex, the normal is
		just moved to the RVertex array. See
		Class RVertex and
		Class RNormal. 
		If you are creating a Mesh by hand, after you are done
		specifying all the vertices and faces, this method should be called.
		This allocates the RVertex and RNormal database for the
		Mesh. This will allow you to query the Mesh and ask about
		normals on the vertices. Also, if you deform a Mesh (i.e. take one of
		the vertices and move it), you should call this method again. Actually,
		if you are only moving one normal you only need to smooth the polygons
		that share the vertex. However, there is no method to smooth a subset
		of a Mesh, you either have to do it by hand or call this method
		to smooth the entire Mesh. 
		This method also builds the face normals for the mesh. */
		DllExport void		buildNormals();			// calcs face and vertex normals

		/** This method is similar to buildNormals() above,
		but ignores the material index (mtlIndex). In other words, the
		difference between this and buildNormals() is that it doesn't
		look at the mtlIndex of the faces: normals of faces with the
		same smoothing group are averaged regardless. */
		DllExport void 		buildRenderNormals();	// like buildNormals, but ignores mtlIndex

		// checkNormals can be used to build the normals and allocate RVert space 
		// only if necessary.  This is a very cheap call if the normals are already calculated.
		// When illum is FALSE, only the RVerts allocation is checked (since normals aren't
		// needed for non-illum rendering).  When illum is TRUE, normals will also be built, if
		// they aren't already.  So, to make sure normals are built, call this with illum=TRUE.
		/** This method can be used to build the normals and allocate
		RVert space only if necessary. This is a very inexpensive call
		if the normals are already calculated. When illum is FALSE, only
		the RVerts allocation is checked (since normals aren't needed
		for non-illum rendering). When illum is TRUE, normals will also
		be built, if they aren't already. So, to make sure normals are built,
		call this with illum=TRUE.
		\param illum 
		If TRUE then normals are built. If FALSE then only the RVert
		array is allocated. */
		DllExport void		checkNormals(BOOL illum);

		/** Renders this Mesh using the specified graphics window and
		array of materials. 
		Note: If a display routine makes multiple calls to this method you need
		to have called: 
		gw-\>setMaterial(inode-\>Mtls()[0]); 
		before calling Mesh::render(). If you don't then you may get the
		wrong material for material ID 0.
		\param gw 
		Points to the graphics window to render to. 
		\param ma 
		The list of materials to use to render the mesh. 
		See Class Material and the topic "Class INode - Material methods". 
		\param rp 
		Specifies the rectangular region to render. If the mesh should be
		rendered to the entire viewport pass NULL. 
		\param compFlags 
		One or more of the following flags: 
		\li COMP_TRANSFORM 
		Forces recalculation of the model to screen transformation; otherwise
		attempt to use the cache. 
		\li COMP_IGN_RECT 
		Forces all polygons to be rendered; otherwise only those intersecting
		the box will be rendered. 
		\li COMP_LIGHTING 
		Forces re-lighting of all vertices (as when a light moves); otherwise
		only re-light moved vertices 
		\li COMP_ALL 
		All of the above flags. 
		\li COMP_OBJSELECTED 
		If this bit is set then the node being displayed by this mesh is
		selected. Certain display flags only activate when this bit is set. 
		\param numMat
		The number of materials for the mesh. 
		\para mpi This pointer to an InterfaceServer can be used to get hold of the
		IXTCAccess pointer. 
		And IXTCAccess interface can also be obtained from the object by
		calling Object::GetInterface 
		(IXTCACCESS_INTERFACE_ID).
		\par Sample Code:
		The following code shows this method being used to render the mesh as
		part of the BaseObject::Display() method: 
		\code
		int SimpleObject::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
		{
			if (!OKtoDisplay(t)) 
				return 0;
			GraphicsWindow *gw = vpt->getGW();
			Matrix3 mat = inode->GetObjectTM(t);
			UpdateMesh(t);								  // UpdateMesh() just calls BuildMesh()
			gw->setTransform(mat);
			mesh.render(gw, inode->Mtls(), (flags&USE_DAMAGE_RECT) ? &vpt->GetDammageRect() : NULL, COMP_ALL, inode->NumMtls());
			return(0);
		}
		\endcode */
		DllExport void		render(GraphicsWindow *gw, Material *ma, RECT *rp, int compFlags, int numMat=1, InterfaceServer *pi = NULL);

		/** Checks the given HitRecord hr to see if it
		intersects this Mesh object.
		\param gw 
		Points to the graphics window to check. 
		\param ma 
		The list of materials for the mesh. 
		\param hr 
		This describes the properties of a region used for the hit testing. See
		Class HitRegion. 
		\param abortOnHit
		If nonzero, the hit testing is complete after any hit; otherwise all
		hits are checked. 
		\param numMat 
		The number of materials for the mesh.
		\return  TRUE if the item was hit; otherwise FALSE. */
		DllExport BOOL		select(GraphicsWindow *gw, Material *ma, HitRegion *hr, int abortOnHit = FALSE, int numMat=1);
		
		/** Checks to see if there is a snap point near the given
		mouse point.
		\param gw 
		The graphics window in which to check. 
		\param snap 
		This structure describes the snap settings used, and the results of the
		snap test. See the title "Structure	SnapInfo". 
		\param p 
		The mouse point to check. 
		\param tm 
		The object transformation matrix. This is the transformation to place
		the object into the world coordinate system.
		\par Sample Code:
		// Checks to see if there is a snap point near the given mouse
		point. 
		\code
		void TestObject::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt)
		{
		// Grab the object TM
			Matrix3 tm = inode->GetObjectTM(t);
		// Grab the graphics window
			GraphicsWindow *gw = vpt->getGW();
		// Make sure our mesh is up to date
			UpdateMesh(t);		  // UpdateMesh() just calls BuildMesh()
		// Set the transform in the GW
			gw->setTransform(tm);
		// Let the mesh do the work...
			mesh.snap( gw, snap, p, tm );
		}
		\endcode */
		DllExport void		snap(GraphicsWindow *gw, SnapInfo *snap, IPoint2 *p, Matrix3 &tm);
		/** This method
		may be called to perform sub-object hit testing on this mesh.
		\param gw 
		The graphics window associated with the viewport the mesh is being hit tested
		in. 
		\param ma 
		The list of materials for the mesh. See Class Material and the topic "Class INode - Material methods". 
		\param hr 
		This describes the properties of a region used for the hit testing. See
		Class HitRegion. 
		\param flags 
		Flags for sub object hit testing. One or more of the flags in \ref Flags_for_Sub_Object_Hit_Test
		\param hitList 
		The results are stored here. See Class SubObjHitList. 
		\param numMat 
		The number of materials for the mesh.
		\return  TRUE if the item was hit; otherwise FALSE. */
		DllExport BOOL 		SubObjectHitTest(GraphicsWindow *gw, Material *ma, HitRegion *hr,
								DWORD flags, SubObjHitList& hitList, int numMat=1 );

		//  Controls the display of surface normals on the mesh object.
		/** Note that there may be more than one normal per vertex if faces that
			share the vertex are in non-overlapping smoothing groups. In this case,
			all normals associated with the given vertex are drawn.
		\param b Nonzero to display the normals; zero to turn off normals display.
			This can be a combination of MESH_DISP_FACE_NORMALS and
			MESH_DISP_VERTEX_NORMALS. (The arguments may be or'ed together
			to display both.) For backwards compatibility,
			MESH_DISP_FACE_NORMALS is defined to be 1, so sending in
			TRUE will turn on display of face normals, as before.
		\param sc This specifies the length that should be used (in world units) to
			display the normals. Since all normals start out with length equal to 1 they
			probably would be too small to see unless they were scaled */
		void		displayNormals(int b, float sc)
		{
			DbgAssert(b <= 0xff);
			DbgAssert(b >= 0);
			dspNormals = (BYTE)b; 
			if(sc != 0.0f) 
				norScale = sc;
		}
		/** Controls the display of hidden edges of this mesh object.
		This just sets the Boolean in the mesh that controls whether "hidden"
		edges (for instance the diagonals on the sides of a cube) are
		displayed.
		\param b 
		Nonzero to display all the hidden edges; otherwise zero. */
		void		displayAllEdges(int b)
		{
			DbgAssert(b <= 0xff);
			DbgAssert(b >= 0);
			dspAllEdges = (BYTE)b;
		}
		/** Computes the bounding box of the Mesh. If surface normals
		are displayed, they are taken into account in the computation of the
		box. The bounding box is stored with the Mesh object, use
		getBoundingBox() to retrieve it. */
		DllExport void		buildBoundingBox(void);

		/** Retrieves the bounding box of the mesh object.
		\param tm The optional TM allows the box to be calculated in any space. NOTE:
		This computation will be slower because all the points must be
		transformed.
		\return  The bounding box of the Mesh. */
		DllExport Box3 		getBoundingBox(Matrix3 *tm=NULL); // RB: optional TM allows the box to be calculated in any space.
		                                              // NOTE: this will be slower because all the points must be transformed.
		
		// Cache invalidation
		/** Call this method after the geometry of this Mesh has
		changed. It invalidates the bounding box, and tosses out the cached
		normals and edge list. */
		DllExport void 		InvalidateGeomCache();

		/** Call this method after you alter vertex or face lists or
		revise edge visibility flags. It will invalidate the edge and strip
		database maintained by the mesh. */
		DllExport void 		InvalidateTopologyCache();

		/** Cleans up the allocated arrays. This frees the Faces,
		Verts, TVerts, TVFaces, FaceMtlIndexList and invalidates the geometry
		cache. */
		DllExport void 		FreeAll(); //DS

		DllExport void      ZeroTopologyCache(); // RB set pointers to NULL but don't delete from mem.

		// edge list functions		
		/** \internal This method is used internally. */
		DllExport void		EnableEdgeList(int e);

		/** \internal This method is used internally. */
		DllExport void     	BuildVisEdgeList();

		/** \internal This is used internally. */
		DllExport void 		DrawVisEdgeList(GraphicsWindow *gw, DWORD flags);
		DllExport void 		Draw3DVisEdgeList(GraphicsWindow *gw, DWORD flags);

		/** \internal This is used internally. */
		DllExport void		HitTestVisEdgeList(GraphicsWindow *gw, int abortOnHit ); // RB

		/** \internal This is used internally. */
		DllExport void		InvalidateEdgeList(); // RB

		// strip functions				
		/** It builds the strips database inside the mesh. See the method
		BuildStripsAndEdges() below. */
		DllExport BOOL     	BuildStrips();
		/** \internal This method is used internally. */
		DllExport void		Stripify(Strip *s, StripData *sd, int vtx);
		DllExport void		Stripify(Strip *s, StripData *sd, int vtx, DWTab& v, DWTab& n, DWTab& tv);
		/** \internal This method is used internally. */
		DllExport void		getStripVertColor(GraphicsWindow *gw, int cv, int flipped, MtlID mID, DWORD smGroup, Point3 &rgb);
		/** \internal This method is used internally. */
		DllExport void		getStripNormal(int cv, MtlID mID, DWORD smGroup, Point3 &nor);
		DllExport int		getStripNormalIndex(int cv, MtlID mID, DWORD smGroup);
		/** \internal This method is used internally. */
		DllExport BOOL		getStripTVert(GraphicsWindow *gw, int cv, int ctv, Point3 &uvw, int texNum = 0);
		/** \internal This method is used internally. */
		DllExport void 		DrawStrips(GraphicsWindow *gw, Material *ma, int numMat);
		/** \internal This method is used internally. */
		DllExport void 		Draw3DStrips(GraphicsWindow *gw, Material *ma, int numMat);
		DllExport void 		Draw3DWireStrips(GraphicsWindow *gw, Material *ma, int numMat);

		/** This method invalidates the strips database. */
		DllExport void		InvalidateStrips();

		/** This method builds the strips and edges database inside the mesh. When
		developers create a new mesh for display then this method should be
		called. See the section on Stripping in the Advanced Topics section
		"Working with Meshes" for details on this method. */
		DllExport void		BuildStripsAndEdges();

		// functions for use in data flow evaluation
		/** Makes a copy of the specified channels of the specified Mesh
		object's data structures (but not all the data in these structures) into this
		Mesh. For example the verts, tVerts, tvFaces, ... are not copied.
		\param amesh 
		Specifies the source Mesh to copy. 
		\param  channels  
		Specifies the channels to copy. See \ref ObjectChannels. */
      DllExport void       ShallowCopy(Mesh *amesh, ChannelMask channels);

		/** Makes a complete copy of the specified channels of the specified
		Mesh object (its data structures and all the data in these structures) into
		this Mesh.
		\param amesh 
		Specifies the source Mesh to copy. 
		\param channels 
		Specifies the channels to copy. See \ref ObjectChannels. */
      DllExport void       DeepCopy(Mesh *amesh, ChannelMask channels);

		/** This method replaces the specified channels with newly allocated
		copies.
		\param channels 
		Specifies the channels to copy. See \ref ObjectChannels. */
      DllExport void       NewAndCopyChannels(ChannelMask channels);

		/** Release the memory associated with the specified channels. For
		example if the TOPO_CHANNEL is specified the faces are freed, if the
		GEOM_CHANNEL is specified the vertices are freed, etc.
		\param channels 
		Specifies the channels to free. Channels not specified are left intact. See \ref ObjectChannels. 
		\param zeroOthers
		If nonzero then the various pointers are set to NULL and their counts are
		zeroed. For example faces, verts, tVerts, and tvFace are set to
		NULL and numFaces, numVerts and numTVerts are set to 0. If this
		is passed as 0, these pointers and counts are left unaltered. */
      DllExport void       FreeChannels(ChannelMask channels, int zeroOthers=1);

		// Mesh flags
		/** Sets the state of the mesh flags.
		\param f 
		Specifies the flags to set. See \ref MeshVertexFaceFlags*/
		void		SetFlag(DWORD f) { flags |= f; }
		/** Returns the state of the specified mesh flags. See
		Data Members: above for a list of the mesh flags.
		\param f 
		Specifies the flags to retrieve. See \ref MeshVertexFaceFlags*/
		DWORD		GetFlag(DWORD f) { return flags & f; }
		/** Sets the state of the specified mesh flags to 0.
		\param f 
		Specifies the flags to clear. See \ref MeshVertexFaceFlags*/
		void		ClearFlag(DWORD f) { flags &= ~f; }

		// Display flags
		/** Sets the state of the display flags. See Data
		Members: above for a list of the display flags.
		\param f 
		Specifies the flags to set. */
		void		SetDispFlag(DWORD f) { dispFlags |= f; }
		/** Returns the state of the specified display flags. See
		Data Members: above for a list of the display flags.
		\param f 
		Specifies the flags to retrieve. */
		DWORD		GetDispFlag(DWORD f) { return dispFlags & f; }
		/** Sets the state of the specified display flags to 0.
		\param f 
		Specifies the flags to clear. */
		void		ClearDispFlag(DWORD f) { dispFlags &= ~f; }

		// Selection access
		/** Retrieves the bits representing the vertex selection
		status. See Data Members: above.
		\return  The vertex selection status. */
		BitArray& 	VertSel() { return vertSel;  }	

		/** Retrieves the bits representing the face selection
		status. 
		\return  The face selection status. */
		BitArray& 	FaceSel() { return faceSel;  }	

		// Constructs a vertex selection list based on the current selection level.
		/** Constructs a vertex selection list based on the current
		selection level. For example if the selection level is at object level
		all the bits are set (vertices, faces and edges). If the selection
		level is at vertex level only the selected vertex bits are set.
		\return  A BitArray reflecting the current selection level.
		\par Sample Code:
		\code
		BitArray sel = mesh->VertexTempSel();
		for ( int i = 0; i < mesh->getNumVerts(); i++ )
		{
			if ( sel[i] ) {
				...
			}
		}
		\endcode */
		DllExport BitArray 	VertexTempSel();

		DllExport IOResult Save(ISave* isave);
		DllExport IOResult Load(ILoad* iload);

		// RB: added so all objects can easily support the GeomObject method of the same name.
		/** Calculates the intersection of the specified ray with
		this mesh object. This allows Mesh objects to easily implement the
		Object::IntersectRay() method.
		\param ray 
		Specifies the origin and direction of the ray to intersect with the
		mesh. See Class Ray. 
		\param at 
		The computed point of intersection on the surface of the mesh. 
		\param norm 
		The face normal at the point of intersection (at).
		\return  Nonzero if the ray intersected the mesh object; otherwise 0.
		Note that this method ignores backfaces when computing the result. */
		DllExport int IntersectRay(Ray& ray, float& at, Point3& norm);

		/** Calculates the intersection of the specified ray with this mesh object.
		This new version also returns the face index that was intersected and
		the barycentric coordinates of that face. 
		Barycentric coordinates are the coordinates relative to the triangular
		face. The barycentric coordinates of a point p relative to a
		triangle describe that point as a weighted sum of the vertices of the
		triangle. If the barycentric coordinates are b0, b1, and
		b2, then: 
		p = b0*p0 + b1*p1 + b2*p2; 
		where p0, p1, and p2 are the vertices of the
		triangle. The Point3 returned by this method has the barycentric
		coordinates stored in its three coordinates. These coordinates are
		relative to the triangular face that was intersected. These barycentric
		coordinates can be used to interpolate any quantity whose value is
		known at the vertices of the triangle.
		\param ray 
		Specifies the origin and direction of the ray to intersect with the
		mesh. See Class Ray. 
		\param at 
		The computed point of intersection on the surface of the mesh. 
		\param norm 
		The face normal at the point of intersection (at). 
		\param fi 
		The face index of the face that was intersected is returned here. 
		\param bary 
		The barycentric coordinates of the face that was hit.
		\return  Nonzero if the ray intersected the mesh object; otherwise 0.
		*/
		DllExport int IntersectRay(Ray& ray, float& at, Point3& norm, DWORD &fi, Point3 &bary);

		// RB: I couldn't resist adding these <g>
		/** Performs a boolean union operation.
		\param mesh 
		Specifies the mesh to use as the other operand of the boolean
		operation.
		\return  A new Mesh resulting from the boolean operation. If the
		operation fails an empty Mesh is returned. */
		DllExport Mesh operator+(Mesh &mesh);  // Union

		/** Performs a boolean difference operation.
		\param mesh 
		Specifies the mesh to use as the other operand of the boolean
		operation.
		\return  A new Mesh resulting from the boolean operation. If the
		operation fails an empty Mesh is returned. */
		DllExport Mesh operator-(Mesh &mesh);  // Difference
		/** Performs a boolean intersection operation.
		\param mesh 
		Specifies the mesh to use as the other operand of the boolean
		operation.
		\return  A new Mesh resulting from the boolean operation. If the
		operation fails an empty Mesh is returned. */
		DllExport Mesh operator*(Mesh &mesh);  // Intersection
		DllExport void MyDebugPrint ();

		/** \internal This method is used internally. */
		DllExport void WeldCollinear(BitArray &set);

		/** Allows this Mesh to be reduced in complexity by reducing
		the number of faces based on a surface normal threshold. Adjacent faces
		whose difference in surface normal angle falls below the specified
		threshold will be collapsed into a single triangle. The Mesh may also
		have its edge visibility set based on a surface normal threshold.
		\param normThresh 
		When the angle between adjacent surface normals is less than this value
		the optimization is performed. This angle is specified in radians. 
		\param edgeThresh 
		When the angle between adjacent surface normals is less than this value
		the auto edge is performed (if the OPTIMIZE_AUTOEDGE flag is
		set). This angle is specified in radians. 
		\param bias 
		When optimizing mesh objects, as the optimization increases, you can
		get lots of long skinny 'degenerate' triangles (that cause rendering
		artifacts). Increasing the bias parameter keeps triangles from becoming
		degenerate. The range of values is from 0 to 1 (where 0 turns bias
		off). Values close to 1 reduce the amount of optimization in favor of
		maintaining equilateral triangles. 
		\param maxEdge 
		This parameter is available in release 2.0 and later only. 
		This will prevent the optimize function from creating edges longer than
		this value. If this parameter is \<=0 no limit is placed on the length
		of the edges. 
		\param flags 
		These flags control the optimization. Specify zero or more of the
		\ref Optimization_Flags
		\param prog 
		A callback used for lengthy optimize operations. See
		Class MeshOpProgress. */
		DllExport void Optimize(
			float normThresh, float edgeThresh, 
			float bias, float maxEdge, DWORD flags, 
			MeshOpProgress *prog=NULL);

		/** This method may be called to map this Mesh with UVW mapping coordinates.
		\param type 
		The mapping type. One of the following values: 
		\li MAP_PLANAR 
		\li MAP_CYLINDRICAL 
		\li MAP_SPHERICAL 
		\li MAP_BALL 
		\li MAP_BOX 
		\param utile 
		Number of tiles in the U direction. 
		\param vtile 
		Number of tiles in the V direction. 
		\param wtile 
		Number of tiles in the W direction. 
		\param uflip 
		If nonzero the U values are mirrored. 
		\param vflip 
		If nonzero the V values are mirrored. 
		\param wflip 
		If nonzero the W values are mirrored. 
		\param cap 
		This is used with MAP_CYLINDRICAL. If nonzero, then any face normal that
		is pointing more vertically than horizontally will be mapped using planar
		coordinates. 
		\param tm 
		This defines the mapping space. As each point is mapped, it is multiplied by
		this matrix, and then it is mapped. 
		\param channel
		This parameter is available in release 2.0 and later only. 
		This indicates which channel the mapping is applied to -- channel==1
		corresponds to the original texture mapping channel. */
		DllExport void ApplyUVWMap(int type,
			float utile, float vtile, float wtile,
			int uflip, int vflip, int wflip, int cap,
			const Matrix3 &tm, int channel=1);
		DllExport void ApplyMapper (UVWMapper & map, int channel=1);

		/** Flips the surface normal of the 'i-th' face (this just
		rearranges the indices for the face structure). This also makes sure
		the edge flags are rearranged as well. If there are UV coordinates
		they are rearranged appropriately.
		\param i 
		The index of the face to flip. */
		DllExport void FlipNormal(int i);
		/** Unifies the surfaces normals of this Mesh. This may be
		for selected faces, or the entire mesh.
		\param selOnly 
		If TRUE only the selected faces are unified. */
		DllExport void UnifyNormals(BOOL selOnly);
		/** Performs an auto smooth on the mesh, setting the
		smoothing groups based on the surface normals.
		\param angle 
		The minimum angle between surface normals for smoothing to be applied,
		in radians. 
		\param useSel 
		If TRUE only the selected faces are smoothed. 
		\param preventIndirectSmoothing 
		This parameter is available in release 2.0 and later only. 
		TRUE to turn on; FALSE to leave off. This matches the option in the
		Smooth Modifier UI -- use this to prevent smoothing 'leaks" when using
		this method. If you use this method, and portions of the mesh that
		should not be smoothed become smoothed, then try this option to
		see if it will correct the problem. Note that the problem it corrects
		is rare, and that checking this slows the Auto Smooth process. */
		DllExport void AutoSmooth(float angle,BOOL useSel,BOOL preventIndirectSmoothing=FALSE);

		/** \internal This method is used internally. */
		DllExport Edge *MakeEdgeList(int *edgeCount, int flagdbls=0);

		/** This method removes faces from the face list with the
		FACE_WORK flag set.
		\return  The number of faces deleted. */
		DllExport int DeleteFlaggedFaces(); // deletes all faces with FACE_WORK flag set
				
		
		// deletes all selected elements of the current selection level
		/** Deletes all selected elements of the current selection level. */
		DllExport void DeleteSelected();	
		
		// Deletes vertices as specified by the bit array
		/** Deletes the vertices as specified by the BitArray.
		\param set 
		Set of bits to indicate the vertices to delete. */
		DllExport void DeleteVertSet(BitArray set);
		
		// Deletes faces as specified by the bit array. If isoVert is non
		// null then it will be setup to flag vertices that were isolated
		// by the face deletetion. This set can then be passed to
		// DeleteVertSet to delete isolated vertices.
		/** Deletes faces as specified by the BitArray.
		\param set 
		Set of bits to indicate the faces to delete. 
		\param isoVert 
		If non NULL then this method will be setup to flag vertices that were
		isolated by the face deletion. This set can then be passed to
		DeleteVertSet() to delete isolated vertices. */
		DllExport void DeleteFaceSet(BitArray set, BitArray *isoVert=NULL);

		// Returns TRUE if an equivalent face already exists.
		/** This method may be called to determine if an equivalent
		face already exists.
		\param v0 
		Index of the first vertex. 
		\param v1 
		Index of the second vertex. 
		\param v2 
		Index of the third vertex.
		\return  TRUE if an equivalent face already exists; otherwise FALSE. */
		DllExport BOOL DoesFaceExist(DWORD v0, DWORD v1, DWORD v2);

		// Removes faces that have two or more equal indices.
		// Returns TRUE if any degenerate faces were found
		/** Removes faces that have two or more equal indices.
		\return  TRUE if any degenerate faces were found; otherwise FALSE. */
		DllExport BOOL RemoveDegenerateFaces();

		// Removes faces that have indices that are out of range
		// Returns TRUE if any illegal faces were found
		/** Removes faces that have indices that are out of range
		\return  TRUE if any illegal faces were found; otherwise FALSE. */
		DllExport BOOL RemoveIllegalFaces();

		/** 
 
		This method returns the normal of the specified face. If nrmlize
		is TRUE, it makes this normal unit length. Otherwise, it's the edge
		cross-product length, which is actually 2 times the area of the face.
		\param fi 
		Specifies the face whose normal is returned. 
		\param nrmlize
		Use TRUE to make the normal unit length. */
		DllExport Point3 FaceNormal (DWORD fi, BOOL nrmlize=FALSE);
		/** 
 
		This method returns the center of the specified face.
		\param fi 
		Specifies the face whose center is returned. */
		DllExport Point3 FaceCenter (DWORD fi);
		/** Returns the angle between two face surface normals in
		radians.
		\param f0 
		Index of the first face. 
		\param f1 
		Index of the second face. */
		DllExport float AngleBetweenFaces(DWORD f0, DWORD f1);

		// Compute the barycentric coords of a point in the plane of
		// a face relative to that face.
		/** Computes and returns the barycentric coordinates of a
		point in the plane of a face relative to that face.
		\param face 
		The index of the face to check. 
		\param p 
		The input point.
		\return  The point p barycentric coordinates. If the point
		p is inside the face the returned values will sum to one. Note:
		If the face (or set of 3 points) is degenerate, ie if it has a zero
		length normal vector ((p1-p0)^(p2-p0)), the methods return
		Point3(-1,1,1). */
		DllExport Point3 BaryCoords(DWORD face, Point3 p);

		// Some edge operations
		/** Divides the edge, creating a new point. The face directly using this
		edge (face edge/3) is also divided in two, and other faces using the
		edge may optionally be split to use the split edges.
		\param edge 
		The edge to divide. 
		\param prop 
		The proportion along the edge to make the division. An edge can be
		expressed as ff*3+ee, where ff is a face using this edge and ee
		represents which pair of vertices the edge is between,
		faces[ff]-\>v[ee] and faces[ff]-\>v[(ee+1)%3]. The new point is created
		at (1-prop) times the first vertex plus prop times the second. prop may
		vary from 0 to 1. prop=.5 gives the same result that DivideEdge (DWORD
		edge) would generate. 
		\param visDiag1  
		Indicates whether the "diagonal" used to split the primary face this
		edge is on (edge/3) is visible or not. 
		\param fixNeighbors  
		Indicates whether other faces using this edge should also be split to
		use the two new edges, or if they should be left as they were. In a
		typical mesh, there is one other face using this edge, the face on the
		"other side" of the edge. If fixNeighbors is FALSE, the "other side" in
		this case would still use the original edge, while the face on this
		side would be split to use the two new edges. This would create a
		"hole" in the mesh. 
		\param visDiag2 
		Indicates whether the "diagonals" used to split other faces using this
		edge are visible or not. This argument is not used if fixneighbors is
		FALSE. */
		DllExport void DivideEdge(DWORD edge, float prop=.5f, bool visDiag1=TRUE,
			bool fixNeighbors=TRUE, bool visDiag2=TRUE);
		/**  Cuts a face into three faces, arranged as a quad and a triangle, by
		slicing from a point on one edge to a point on another.
		\param face 
		The face to be divided. 
		\param e1,e2 
		The index of edges to be cut. For instance, if you wanted to cut from
		the edge between v[0] and v[1], to the edge between v[2] and v[0], you
		would use e1=0 and e2=2. 
		\param prop1
		The proportion along edge e1 to start cutting. 
		\param prop2 
		The proportion along edge e2 to stop cutting. 
		\param fixNeighbors 
		Indicates whether faces also using the cut edges should be split to use
		the new, subdivided edges. 
		bool split=FALSE 
		Indicates that the triangle and quad created by this action should use
		different vertices. If TRUE, the vertices created by the cut are
		duplicated, with one set being used for faces on one side and the other
		set being used by faces on the other side. */
		DllExport void DivideFace(DWORD face, DWORD e1, DWORD e2, 
			float prop1=.5f, float prop2=.5f, bool fixNeighbors=TRUE, bool split=FALSE);
		/** Turns an edge. The quadrilateral formed by the triangles on either side
		of this edge essentially has its diagonal switched.
		\param edge 
		The edge to be turned. 
		\param otherEdge 
		If non-NULL, this should point to a variable in which the index of the
		"other side" of this edge should be stored. In essence, the two sides
		of an edge used by two faces, f1 and f2, are stored in two different
		locations, f1*3+e1 and f2*3+e2, where e1, e2 are 0, 1, or 2. This
		argument is provided so you have easy access to the other side of the
		edge, if desired, to make easy selection or visibility changes to the
		edge or other changes to the faces on both sides. */
		DllExport void TurnEdge (DWORD edge, DWORD *otherEdge=NULL);

		// Tessellation
		/**  Tessellates the mesh (or only the selected faces) using the face/center method.
		This means each face is subdivided by lines from a new vertex at the center to
		the original vertices.
		\param ignoreSel  
		If TRUE the entire mesh is tesselated; otherwise only the selected faces. */
		DllExport void FaceCenterTessellate(BOOL ignoreSel=FALSE, MeshOpProgress *mop=NULL);
		/** 		Edge tessellates the mesh using the specified tension parameter. This method can
		operate on the entire mesh or only the selection set. Edge tessellation means
		that faces are internally subdivided, with additional faces generated from a
		new vertex in the middle of each face edge.
		\param tens 
		The tension setting. This value can range from -100.0 to 100.0. This value
		matches the parameter in the Editable Mesh user interface when tessellating
		faces. 
		\param ignoreSel 
		If TRUE the entire mesh is tesselated; otherwise only the selected faces. */
		DllExport void EdgeTessellate(float tens,BOOL ignoreSel=FALSE, MeshOpProgress *mop=NULL);

		// Extrudes selected faces. Note that this is just a topological
		// change. The new extruded faces do not change position but
		// are left on top of the original faces.
		// If doFace is FALSE then selected edges are extruded.
		/** Extrudes the selected faces. Note that this is just a topological
		change. The new extruded faces do not change position but are left on
		top of the original faces.
		\param doFace 
		If TRUE the faces are extruded. If FALSE then the selected edges are
		extruded. */
		DllExport void ExtrudeFaces(BOOL doFace=TRUE);

		// Indents selected faces, in a manner consistent with the outlining used in Bevel.
		// Added by SteveA for Shiva, 6/98
		/**  Indents the selected faces, in a manner consistent with the outlining
		used in Bevel.
		\param amount 
		The amount to indent. */
		DllExport void IndentSelFaces (float amount);

		// Splits verts specified in bitarray so that they are only
		// used by a single face
		/** Splits the vertices specified in the BitArray so that they are only
		used by a single face.
		\param set 
		This array of bits, one per vertex in the mesh. If the bit is set, the
		corresponding vertex in the mesh is copied as required so it is only
		used by a single face. If the bit is not set the vertex is ignored. */
		DllExport void BreakVerts(BitArray set);

		// Deletes verts that aren't used by any faces
		/** This method returns a BitArray (of size numVerts), where
		isolated verts are selected. */
		DllExport BitArray GetIsoVerts ();
		/** Deletes the vertices that aren't used by any faces. */
		DllExport void DeleteIsoVerts ();

		// Clone faces (and verts used by those faces)
		/**  Creates a copy of the faces and verts used by those faces as specified
		by the BitArray passed. If texture faces and vertex color faces
		are present they are cloned as well.
		\param fset 
		There is one bit in this array for each face in the mesh. If the bit is
		set, the corresponding face in the mesh is cloned. If the bit is zero
		the face is not cloned. */
		DllExport void CloneFaces(BitArray fset);
		/** 
 
		Sets bits for all faces in the same polygon with face f. Faces
		already selected in set will not be processed -- so if f
		is "set", nothing happens. 
		The definition of a polygon is all faces sharing invisible edges with
		edge angles below "thresh".
		\param f 
		Specifies which face to evaluate -- the zero based index into the
		faces array. 
		\param set 
		Specifies which faces are not processed. One bit for each face with set
		bits not considered. 
		\param thresh 
		The angle in radians which is the threshold for defining a polygon. A
		polygon is all faces sharing invisible edges with edge angles below
		this angle. 
		\param ignoreVisEdges 
		If TRUE, the edge visibility is ignored but the threshhold is still
		relevant. 
		\param af 
		This adjacent face list can be passed if there's one handy; otherwise a
		new one will be computed by the method. */
		DllExport void PolyFromFace (DWORD f, BitArray &set, float thresh, BOOL ignoreVisEdges, AdjFaceList *af=NULL);
		/**  This method sets bits for all faces in the same "element", or connected
		component, with face f. Faces already selected in set
		will be considered "walls" for this processing and will not be
		evaluated.
		\param f 
		Specifies which face to evaluate -- the zero based index into the
		faces array. 
		\param set 
		Specifies which faces are considered as barriers to the element and are
		not processed. One bit for each face with set bits not considered. 
		\param af  
		This adjacent face list can be passed if there's one handy; otherwise a
		new one will be computed by the method. */
		DllExport void ElementFromFace (DWORD f, BitArray &set, AdjFaceList *af=NULL);
		/** When faces are deleted in Edit or Editable Mesh, we often want to be
		able to delete the verts that are isolated by this action. This method
		generates the list of verts that are used only by the specified set of
		faces.
		\param fset 
		This method finds those vertices used only by the faces indicated in
		this BitArray. 
		\param vset 
		This BitArray is completely overwritten with the result, and will be
		set to the right size (numVerts) if needed. */
		DllExport void FindVertsUsedOnlyByFaces (BitArray & fset, BitArray & vset);
		/** This method fills in a BitArray with the edges in the mesh that are
		"open" or "one-sided". (This is the same as the Edit Mesh "Select Open
		Edges" function.)
		\param edges 
		This BitArray will be set to size numFaces*3, and only the open edge
		bits will be set. */
		DllExport void FindOpenEdges (BitArray & edges);
		/** This method calculates, for each vertex, the sum of the angles of this
		vertex's corner in each face it's on. So for instance, a point lying in
		the middle of a grid would always have vertex angle 2*PI, whereas a
		corner of a box would only have 3*PI/2.
		\param vang 
		This should be a pointer to an array of size numVerts. The vertex angle
		for each vertex is put in this array (in radians). 
		\param set  
		If non-NULL, this points to a BitArray describing which vertices to
		find angles of. If only some bits are set, some computation time is
		saved. If NULL, all vertices' angles are found. */
		DllExport void FindVertexAngles (float *vang, BitArray *set=NULL);

		// used by the renderer
		/** Sets the mesh render data hung off this Mesh. This method
		and GetRenderData() allow the renderer to 'hang' data on a mesh.
		This data can be anything the renderer wants. The data will
		automatically be deleted when the mesh is deleted via the
		DeleteThis() method.
		\param p 
		\see Class MeshRenderData. */
		void  SetRenderData(MeshRenderData *p) {renderData = p; } 
		/** Returns the mesh render data hung off this Mesh. See
		Class MeshRenderData. This
		method and SetRenderData() allow the renderer to 'hang' data on
		a mesh. This data can be anything the renderer wants. The data will
		automatically be deleted when the mesh is deleted via 
		DeleteThis() . */
		MeshRenderData * GetRenderData() { return renderData; }

		// Quick access to specified normal interface.
		DllExport void ClearSpecifiedNormals ();
		DllExport MeshNormalSpec *GetSpecifiedNormals ();
		DllExport void SpecifyNormals ();

		// Copy only vertices and faces - no maps, selection, per-vertex data, etc.
		DllExport void CopyBasics (const Mesh & from);

		// --- from InterfaceServer
		DllExport BaseInterface* GetInterface(Interface_ID id);

      // TO DO: OLP This should probably be put in an interface.
      //  Check if a face's cache is invalid
      /**   The function first checks if the whole mesh or the face have been flagged as
         cache invalid (MESH_CACHEINVALID). If none of these is true, it then cycles through the face's
         vertices and returns true as soon as it finds a vertex flagged with MESH_CACHEINVALID.
         \pre faceIndex must be a valid index in the faces (f) array.
         \param faceIndex The index of the face for which you want to determine cache validity
         \return true if the face is invalid, else false.
      */
      DllExport bool isFaceCacheInvalid(int faceIndex);

      //  Invalidates a single vertex's cache indicating it needs to be rebuilt.
      /**   The function flags the specified vertex with the MESH_CACHEINVALID flag, telling the rebuild
         functions that this vertex, the faces surrounding it and the other vertices of those faces
         need to have their normals recomputed. It also sets normalsBuilt to 0 to trigger normals' rebuild.
         This function should be called when one to several vertices have been modified. If all or almost
         all vertices of the mesh have been invalidated, it's more efficient to call InvalidateGeomCache() instead.
         \pre vertex must be a valid index in the vertex (v) array.
         \param vertex The index of the vertex that needs to be invalidated.
      */
      DllExport void InvalidateVertexCache(int vertex);

      /**  Holds flags specific to the corresponding vertex in the verts array.*/
      long *      vFlags;

		//  This will set the mesh or as static or not.  As static mesh has no animated channels
		/**	This will set the mesh or as static or not.  As static mesh has no animated channels.  This allows 
		use to certain display optimization since we know the mesh is not changing.
		\param staticMesh Whether the mesh is to tagged static or not.
		*/
		DllExport void SetStaticMesh(bool staticMesh);

		//  This will force all the derived data used to display the mesh to be deleted
		/**	This will force all the derived data used to display the mesh to be deleted.  This data can be things 
			like the gfx normals, the direct x mesh cache, tri stripping data etc.
		*/
		DllExport void ReduceDisplayCaches();

		/**  This returns whether the Graphics Cache for this object needs to be rebuilt
		\param gw the active graphics window \n
		\param ma the material aray assigned to the mesh \n
		\param numMat the number of materials in the material array \n
		*/
		DllExport bool NeedGWCacheRebuilt(GraphicsWindow *gw,Material *ma, int numMat);
		/**  This builds the graphics window cached mesh 
		\param gw the active graphics window \n
		\param ma the material aray assigned to the mesh \n
		\param numMat the number of materials in the material array \n
		\param threaded whether when building the cache it can use additional threads.  This is needed since the system may be creating many meshes at the same time\n
		*/
		DllExport void BuildGWCache(GraphicsWindow *gw,Material *ma, int numMat,BOOL threaded);


		
	};



/// \defgroup Optimization_Flags Optimization Flags
//@{
/** Specifies that faces won't be collapsed across a material boundary. */
#define OPTIMIZE_SAVEMATBOUNDRIES		(1<<0)	
/** Specifies that faces won't be collapsed across a dissimilar smoothing group boundary. */
#define OPTIMIZE_SAVESMOOTHBOUNDRIES	(1<<1)	 
/** Specifies that the edge visibility should be set automatically based on the 
angle between adjacent surface normals. This will only set edges as invisible 
- it will not set edges as visible. */
#define OPTIMIZE_AUTOEDGE				(1<<2)
//@}

/** \internal This is used internally. */
DllExport void setUseVisEdge(int b);
/** \internal This is used internally. */
DllExport int getUseVisEdge();

#define SMALL_VERTEX_DOTS	0
#define LARGE_VERTEX_DOTS	1

/// \defgroup Vertex_Dot_Types Vertex Dot Types
/// Vertex dot types with radius from 2 to 7.
//@{
#define VERTEX_DOT2	0		//!< equivalent to SMALL_VERTEX_DOTS
#define VERTEX_DOT3	1		//!< equivalent to LARGE_VERTEX_DOTS
#define VERTEX_DOT4	2
#define VERTEX_DOT5	3
#define VERTEX_DOT6	4
#define VERTEX_DOT7	5
//@}

/// \defgroup Handle_Box_Types Handle Box Types
/// Handle box types with radius from 2 to 7
//@{
#define HANDLE_BOX2	0
#define HANDLE_BOX3	1
#define HANDLE_BOX4	2
#define HANDLE_BOX5	3
#define HANDLE_BOX6	4
#define HANDLE_BOX7	5
//@}

// CAL-05/07/03: get vertex dot marker from vertex dot type
#define VERTEX_DOT_MARKER(vtype) (MarkerType)(vtype - VERTEX_DOT2 + DOT2_MRKR)

// CAL-05/07/03: get vertex dot marker from vertex dot type
#define HANDLE_BOX_MARKER(htype) (MarkerType)(htype - HANDLE_BOX2 + BOX2_MRKR)

DllExport void setUseVertexDots(int b);
DllExport int getUseVertexDots();

DllExport void setVertexDotType(int t);
DllExport int getVertexDotType();

DllExport void setHandleBoxType(int t);
DllExport int getHandleBoxType();

DllExport void setDisplayBackFaceVertices(int b);
DllExport int getDisplayBackFaceVertices();

// Steve Anderson - new for Max 5.0 - we have a better way of evaluating
// vertex normals now, weighting the contribution of each face by the angle
// the face makes at that vertex, but users might want to access the old way.
// This is controlled by the "Use Legacy R4 Vertex Normals" checkbox
// in the General Preferences dialog.
class VertexNormalsCallback: public MaxHeapOperators {
public:
	virtual ~VertexNormalsCallback() {;}
	virtual void SetUseFaceAngles (bool value) = 0;
};

class VertexNormalsControl: public MaxHeapOperators {
	bool mUseFaceAngles;
	Tab<VertexNormalsCallback *> mCallbacks;
public:
	DllExport VertexNormalsControl () : mUseFaceAngles(true) { }

	DllExport void RegisterCallback (VertexNormalsCallback *pCallback);
	DllExport void UnregisterCallback (VertexNormalsCallback *pCallback);

	DllExport void SetUseFaceAngles (bool value);
	DllExport bool GetUseFaceAngles ();
};

DllExport VertexNormalsControl *GetVertexNormalsControl ();

// a callback to update progress UI while doing a
// lengthy operation to a mesh
/** A callback used while doing a lengthy operation to a mesh. A developer creates
an instance of this class and passes a pointer to it into the
CalcBoolOp() or Optimize() function.  
\see  Class Mesh. 
*/
class MeshOpProgress: public MaxHeapOperators {
	public:
		/** Destructor. */
		virtual ~MeshOpProgress() {;}

		// called once with the total increments
		/** This method is called once with the total number of increments.
		\param total 
		The total number of increments. */
		virtual void Init(int total)=0;

		// Called to update progress. % done = p/total
		/** This method is called over and over with a new value for p.
		The percentage complete is p/total.
		\param p 
		The number completed so far.
		\return  If TRUE processing will continue. If FALSE processing is
		aborted. */
		virtual BOOL Progress(int p)=0;
	};

/// \defgroup Boolean_Operations_for_Meshes Boolean Operations for Meshes
//@{
#define MESHBOOL_UNION 				1
#define MESHBOOL_INTERSECTION  		2
#define MESHBOOL_DIFFERENCE 		3
//@}

//
// mesh = mesh1 op mesh2
// If tm1 or tm2 are non-NULL, the points of the corresponding
// mesh will be transformed by these tm before the bool op
// The mesh will be transformed back by either Inverse(tm1) or
// Inverse(tm2) depending whichInv (0=>tm1, 1=>tm2)
// unless whichInv is -1 in which case it will not be transformed
// back.
//
/** Matrix3 *tm2 = NULL, int whichInv = 0, int weld = TRUE); 
Note: This method is still in the SDK, but it is now obselete. Calls to
CalcBoolOp() should be replaced with calls to the new MNMesh form
of Boolean. Please see the method MNMesh::MakeBoolean in
Class MNMesh for details. 
This function stores the result of a boolean operation between mesh1 and
mesh2 into mesh. This operation may be a union, intersection or
difference. If tm1 or tm2 are non-NULL, the points of the
corresponding mesh will be transformed by these matrices before the boolean
operation. The mesh will be transformed back by either Inverse(tm1) or
Inverse(tm2) depending on whichInv (a value of 0 will use
tm1, a value of 1 will use tm2, unless whichInv is -1 in
which case it will not be transformed back).
\param mesh 
The result of the boolean operation is stored here. mesh = mesh1 op
mesh2. 
\param mesh1 
The first operand. 
\param mesh2 
The second operand. 
\param op 
The boolean operation. One of the values from \ref Boolean_Operations_for_Meshes
\param prog = NULL 
A callback to display a progress. See
Class MeshOpProgress. 
\param tm1 = NULL 
If non-NULL then the points of mesh1 will transformed by this matrix before the
boolean operation. 
\param tm2 = NULL 
If non-NULL then the points of mesh2 will transformed by this matrix before the
boolean operation. 
\param whichInv = 0 
If 0, the resulting mesh will be transformed by Inverse(tm1). If 1, the
resulting mesh will be transformed by Inverse(tm2). If -1, the mesh will not be
transformed back. 
\param weld = TRUE 
If TRUE, the vertices of the resulting mesh are welded.
\return  Nonzero if the operation completed successfully; otherwise zero. */
DllExport int CalcBoolOp(
	Mesh &mesh, Mesh &mesh1, Mesh &mesh2, int op,
	MeshOpProgress *prog = NULL,
	Matrix3 *tm1 = NULL,
	Matrix3 *tm2 = NULL,
	int whichInv = 0,
	int weld = TRUE);


// Combines two meshes. The matrix and whichInv parameters have
// the same meaning as they do for the CalcBoolOp above.
/** This function is available in release 2.0 and later only. 
This function may be used to simply combine two meshes into one.
\param mesh 
The result of the combine operation is stored here. mesh =
mesh1+mesh2. 
\param mesh1 
The first operand. 
\param mesh2 
The second operand. 
\param tm1 
If non-NULL then the points of mesh1 will transformed by this matrix before the
boolean operation. 
\param tm2 
If non-NULL then the points of mesh2 will transformed by this matrix before the
boolean operation. 
\param whichInv 
If 0, the resulting mesh will be transformed by Inverse(tm1). If 1, the
resulting mesh will be transformed by Inverse(tm2). If -1, the mesh will not be
transformed back. */
DllExport void CombineMeshes(
		Mesh &mesh,Mesh &mesh1,Mesh &mesh2,
		Matrix3 *tm1=NULL, Matrix3 *tm2=NULL, int whichInv=0);

/** Slices a single mesh.  The Point3 N and the float offset define a
 * slicing plane (by DotProd (N,X) = offset).  Default behavior is to
 * split faces that cross the plane, producing 1-2 new faces on each side
 * and a new vert in the middle of each edge crossing the plane.  split
 * means to add 2 different but coincident points to the top and bottom
 * sets of faces, splitting the mesh into two meshes.  remove means to
 * delete all faces & verts below the plane.*/
DllExport void SliceMesh (Mesh & mesh,
						  Point3 N, float off, bool split=FALSE, bool remove=FALSE);

// Handy utilities to go with meshes:

DllExport Mesh * CreateNewMesh();

