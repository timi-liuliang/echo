/**********************************************************************
 *<
	FILE: IHardwareShader.h

	DESCRIPTION: Hardware Shader Extension Interface class

	CREATED BY: Norbert Jeske

	HISTORY:

 *>	Copyright (c) 2000 - 2002, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "maxheap.h"
#include "maxtypes.h"
#include "TabTypes.h"
#include "GraphicsConstants.h"
#include "GraphicsTypes.h"
#include "baseinterface.h"

// forward declarations
class Mesh;
class MNMesh;
class GWFace;
class Point3;
class Material;
class BitArray;
class INode;

#define HARDWARE_SHADER_INTERFACE_ID Interface_ID(0x7bbd585b, 0x75bb641c)

#define GFX_MAX_COLORS 2

// Use of the following four classes, TriStrip, MeshData, WireMeshData, and
// MeshFaceData, has been added in 3ds max 4.2.  These classes were updated
// from their original (unused) definitions and use of the IHardwareShader
// class member functions that use these data classes was added in 3ds max 4.2.
// If your plugin utilizes these new mechanisms, be sure that your clients are
// aware that they must run your plugin with 3ds max version 4.2 or higher.
//
// For documentation on the use of these data classes, see the code of the
// HardwareShader plugin samples or contact Discreet.

class TriStrip: public MaxHeapOperators {
public:
	DWORD	rndMode;
	MtlID	mID;
	MtlID	maxID;
	DWORD	smGrp;
	int		clrMode;
	int		texMode;
	DWTab	v;
	DWTab	n;
	DWTab	c[GFX_MAX_COLORS];
	DWTab	tv[GFX_MAX_TEXTURES];

	void AddVertN(DWORD vtx, DWORD nor) { v.Append(1, &vtx, (v.Count()>>1)); n.Append(1, &nor, n.Count()/2); }
	void AddVertNC1(DWORD vtx, DWORD nor, DWORD col) { v.Append(1, &vtx, (v.Count()>>1)); n.Append(1, &nor, n.Count()/2); c[0].Append(1, &col, c[0].Count()/2); }

	void AddVertNT1(DWORD vtx, DWORD nor, DWORD tvtx) {
		int alloc = (v.Count()>>1)+1;
		v.Append(1, &vtx, alloc); n.Append(1, &nor, alloc);
		tv[0].Append(1, &tvtx, alloc);
	}
	void AddVertNC1T1(DWORD vtx, DWORD nor, DWORD col, DWORD tvtx) {
		int alloc = (v.Count()>>1)+1;
		v.Append(1, &vtx, alloc); n.Append(1, &nor, alloc); c[0].Append(1, &col, alloc);
		tv[0].Append(1, &tvtx, alloc);
	}
	void AddVertNT2(DWORD vtx, DWORD nor, DWORD tvtx1, DWORD tvtx2) {
		int alloc = (v.Count()>>1)+1;
		v.Append(1, &vtx, alloc); n.Append(1, &nor, alloc);
		tv[0].Append(1, &tvtx1, alloc); tv[1].Append(1, &tvtx2, alloc);
	}
	void AddVertNC1T2(DWORD vtx, DWORD nor, DWORD col, DWORD tvtx1, DWORD tvtx2) {
		int alloc = (v.Count()>>1)+1;
		v.Append(1, &vtx, alloc); n.Append(1, &nor, alloc); c[0].Append(1, &col, alloc);
		tv[0].Append(1, &tvtx1, alloc); tv[1].Append(1, &tvtx2, alloc);
	}
	void AddVertNT3(DWORD vtx, DWORD nor, DWORD tvtx1, DWORD tvtx2, DWORD tvtx3) {
		int alloc = (v.Count()>>1)+1;
		v.Append(1, &vtx, alloc); n.Append(1, &nor, alloc);
		tv[0].Append(1, &tvtx1, alloc); tv[1].Append(1, &tvtx2, alloc); tv[2].Append(1, &tvtx3, alloc);
	}
	void AddVertNC1T3(DWORD vtx, DWORD nor, DWORD col, DWORD tvtx1, DWORD tvtx2, DWORD tvtx3) {
		int alloc = (v.Count()>>1)+1;
		v.Append(1, &vtx, alloc); n.Append(1, &nor, alloc); c[0].Append(1, &col, alloc);
		tv[0].Append(1, &tvtx1, alloc); tv[1].Append(1, &tvtx2, alloc); tv[2].Append(1, &tvtx3, alloc);
	}
	void AddVertNT4(DWORD vtx, DWORD nor, DWORD tvtx1, DWORD tvtx2, DWORD tvtx3, DWORD tvtx4) {
		int alloc = (v.Count()>>1)+1;
		v.Append(1, &vtx, alloc); n.Append(1, &nor, alloc);
		tv[0].Append(1, &tvtx1, alloc); tv[1].Append(1, &tvtx2, alloc);
		tv[2].Append(1, &tvtx3, alloc); tv[3].Append(1, &tvtx4, alloc);
	}
	void AddVertNC1T4(DWORD vtx, DWORD nor, DWORD col, DWORD tvtx1, DWORD tvtx2, DWORD tvtx3, DWORD tvtx4) {
		int alloc = (v.Count()>>1)+1;
		v.Append(1, &vtx, alloc); n.Append(1, &nor, alloc); c[0].Append(1, &col, alloc);
		tv[0].Append(1, &tvtx1, alloc); tv[1].Append(1, &tvtx2, alloc);
		tv[2].Append(1, &tvtx3, alloc); tv[3].Append(1, &tvtx4, alloc);
	}

	/*! \remarks This method adds vertex color data to a vertex. This method is usually used together with
	AddVertN and AddVertexUV methods to set each vertex's data flexibly.
	\param i  The color channel index must be less than GFX_MAX_COLORS.\
	\param col The vertex color data. 
	*/
	void AddVertexCol(int i, DWORD col)
	{
		DbgAssert(i >= 0 && i < GFX_MAX_COLORS);
		c[i].Append(1, &col, (v.Count()>>1)+1);
	}

	/*! \remarks This method adds a set of 2d texture coordinate to a vertex. This method is usually used together with
	AddVertN and AddVertexCol methods to set each vertex's data flexibly.
	\param i  The texture coordinate index must be less than GFX_MAX_TEXTURES.\
	\param tx The texture coordinate data.
	 */
	void AddVertexUV(int i, DWORD tx)
	{
		DbgAssert(i >= 0 && i < GFX_MAX_TEXTURES);
		tv[i].Append(1, &tx, (v.Count()>>1)+1);
	}
};

typedef TriStrip *TriStripPtr;
typedef Tab<TriStripPtr> TriStripTab;

class MeshData: public MaxHeapOperators
{
public:
	DWORD_PTR	id;
	int			numFaces;
	GWFace		*faces;
	int			numStrips;
	int			startStrip;
	int			endStrip;
	TriStripTab	*strips;
	DWORD		mapFlags;
	int			numXYZ;
	Point3		*xyz;
	int			numNor;
	Point3		*nor;
	DWTab		*norIndex;
	Point3		*faceNor;
	int			numRGB[GFX_MAX_COLORS];
	Point3		*rgb[GFX_MAX_COLORS];
	int			numUVW[GFX_MAX_TEXTURES];
	Point3		*uvw[GFX_MAX_TEXTURES];
	int			numMtl;
	Material	*mtlArray;
	int			displayFlags;
	BitArray	*faceSel;
	BitArray	*edgeSel;

	MeshData()
	  : id(NULL),
		numFaces(0),
		faces(NULL),
		numStrips(0),
		startStrip(0),
		endStrip(0),
		strips(NULL),
		mapFlags(0),
		numXYZ(0),
		xyz(NULL),
		numNor(0),
		nor(NULL),
		norIndex(NULL),
		faceNor(NULL),
		numMtl(0),
		mtlArray(NULL),
		displayFlags(0),
		faceSel(NULL),
		edgeSel(NULL)
	{
		int kk;
		for (kk = 0; kk < GFX_MAX_COLORS; kk++) {
			numRGB[kk] = 0;
			rgb[kk] = NULL;
		}
		for (kk = 0; kk < GFX_MAX_TEXTURES; kk++) {
			numUVW[kk] = 0;
			uvw[kk] = NULL;
		}
	}
};

class WireMeshData: public MaxHeapOperators
{
public:
	DWORD_PTR	id;
	int			numFaces;
	GWFace		*faces;
	int			numXYZ;
	Point3		*xyz;
	int			displayFlags;
	BitArray	*faceSel;
	BitArray	*edgeSel;
	int			numMat;
	Material	*mtlArray;

	WireMeshData()
	  : id(NULL),
		numFaces(0),
		faces(NULL),
		numXYZ(0),
		xyz(NULL),
		displayFlags(0),
		faceSel(NULL),
		edgeSel(NULL),
		numMat(0),
		mtlArray(NULL)
	{}
};

class MeshFaceData: public MaxHeapOperators
{
public:
	DWORD_PTR	id;
	int			numFaces;
	GWFace		*faces;
	Point3		*faceNor;
	GWFace		*norFaces;
	DWORD		mapFlags;
	int			displayFlags;
	BitArray	*faceSel;
	BitArray	*edgeSel;
	int			numClrFaces[GFX_MAX_COLORS];
	GWFace		*clrFaces[GFX_MAX_COLORS];
	int			numTexFaces[GFX_MAX_TEXTURES];
	GWFace		*texFaces[GFX_MAX_TEXTURES];
	int			numXYZ;
	Point3		*xyz;
	int			numNor;
	Point3		*nor;
	DWTab		*norIndex;
	int			numRGB[GFX_MAX_COLORS];
	Point3		*rgb[GFX_MAX_COLORS];
	int			numUVW[GFX_MAX_TEXTURES];
	Point3		*uvw[GFX_MAX_TEXTURES];
	int			numMat;
	Material	*mtlArray;

	MeshFaceData()
	  : id(NULL),
		numFaces(0),
		faces(NULL),
		faceNor(NULL),
		norFaces(NULL),
		mapFlags(0),
		displayFlags(0),
		faceSel(NULL),
		edgeSel(NULL),
		numXYZ(0),
		xyz(NULL),
		numNor(0),
		nor(NULL),
		norIndex(NULL),
		numMat(0),
		mtlArray(NULL)
	{
		int kk;
		for (kk = 0; kk < GFX_MAX_COLORS; kk++) {
			numClrFaces[kk] = 0;
			clrFaces[kk] = NULL;
			numRGB[kk] = 0;
			rgb[kk] = NULL;
		}
		for (kk = 0; kk < GFX_MAX_TEXTURES; kk++) {
			numTexFaces[kk] = 0;
			texFaces[kk] = NULL;
			numUVW[kk] = 0;
			uvw[kk] = NULL;
		}
	}
};

// End of 3ds max 4.2 Extension


class IHardwareShader : public BaseInterface
{
public:
	virtual Interface_ID	GetID() { return HARDWARE_SHADER_INTERFACE_ID; }
	virtual Interface_ID	GetVSID() = 0;
	virtual Interface_ID	GetPSID() = 0;

	// Interface Lifetime
	virtual LifetimeType	LifetimeControl() { return noRelease; }

	// Start to draw object
	virtual void	StartObject(Mesh *mesh) = 0;
	virtual void	StartObject(MNMesh *mnmesh) = 0;

	// Initialize HardwareShader first time or frame-by-frame
	virtual void	InitializeShaders(Mesh *mesh, BaseInterface *pbvs, Material *mtlArray, int numMtl, GFX_ESCAPE_FN fn) = 0;
	virtual void	InitializeShaders(MNMesh *mnmesh, BaseInterface *pbvs, Material *mtlArray, int numMtl, GFX_ESCAPE_FN fn) = 0;

	// Try to draw as tristrips?
	virtual bool	CanTryStrips() = 0;

	// Retrive the current rendering mode
	virtual DWORD	GetRndMode() = 0;

	// Retrieve a Material
	virtual Material	*GetMaterial(int numMat) = 0;

	// Set a Material
    virtual void	SetMaterial(const Material &m, int index=0) = 0;

	// Retrieve number of passes for specified Material in Material array
	virtual int		GetNumMultiPass(int numMtl) = 0;

	// Set the number of the current pass
	virtual void	SetNumMultiPass(int numPass) = 0;

	// Draw 3D mesh as TriStrips
	virtual void	DrawMeshStrips(MeshData *data, GFX_ESCAPE_FN fn) = 0;

	// Draw 3D mesh as wireframe
	virtual void	DrawWireMesh(WireMeshData *data, GFX_ESCAPE_FN fn) = 0;

	// Draw 3D lines
	virtual void	StartLines(WireMeshData *data) = 0;
	virtual void	AddLine(DWORD *vert, int vis) = 0;
	virtual void	DrawLines() = 0;
	virtual void	EndLines(GFX_ESCAPE_FN fn) = 0;

	// Draw 3D triangles
	virtual void	StartTriangles(MeshFaceData *data) = 0;
	virtual void	AddTriangle(DWORD index, int *edgeVis) = 0;
	virtual void	DrawTriangles() = 0;
	virtual void	EndTriangles(GFX_ESCAPE_FN fn) = 0;

	// End of drawing object
	virtual void	EndObject(Mesh *mesh) = 0;
	virtual void	EndObject(MNMesh *mnmesh) = 0;
};

/*! \sa  Class Mesh, Class INode\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The abstract interface for a vertex shader.  */
class IVertexShader: public MaxHeapOperators
{
public:
	// Load VertexShader instructions, create any additional per vertex data on
	// a per node basis.  VertexShader instructions should be loaded once and
	// shared among all the nodes using this VertexShader.  Additional per
	// vertex data should be (re)created only when the associated node data
	// changes.  In addition, if there is sufficient memory, VertexBuffers can
	// be created and updated only when node data changes in order to improve
	// rendering performance.
	/*! \remarks This method loads VertexShader instructions, create any
	additional per vertex data on a per node basis. VertexShader instructions
	should be loaded once and shared among all the nodes using this
	VertexShader. Additional per vertex data should be (re)created only when
	the associated node data changes. In addition, if there is sufficient
	memory, VertexBuffers can be created and updated only when node data
	changes in order to improve rendering performance.
	\par Parameters:
	<b>Mesh *mesh</b>\n\n
	A pointer to the mesh to initialize.\n\n
	<b>INode *node</b>\n\n
	A pointer to the node to initialize. */
	virtual HRESULT Initialize(Mesh *mesh, INode *node) = 0;
	/*! \remarks This method loads VertexShader instructions, create any
	additional per vertex data on a per node basis. VertexShader instructions
	should be loaded once and shared among all the nodes using this
	VertexShader. Additional per vertex data should be (re)created only when
	the associated node data changes. In addition, if there is sufficient
	memory, VertexBuffers can be created and updated only when node data
	changes in order to improve rendering performance.
	\par Parameters:
	<b>MNMesh *mnmesh</b>\n\n
	A pointer to the MNMesh to initialize.\n\n
	<b>INode *node</b>\n\n
	A pointer to the node to initialize. */
	virtual HRESULT Initialize(MNMesh *mnmesh, INode *node) = 0;
};

