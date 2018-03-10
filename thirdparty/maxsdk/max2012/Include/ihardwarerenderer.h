/**********************************************************************
 *<
	FILE: IHardwareRenderer.h

	DESCRIPTION: Hardware Renderer Extension Interface class

	CREATED BY: Norbert Jeske

	HISTORY:

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "ihardwareshader.h"
#include "HWMesh.h"

// forward declarations
struct D3DXCOLOR;
class IHardwareMesh;
class IHardwareMaterial;
class HardwareMaterial;
class Matrix3;

#define HARDWARE_RENDERER_INTERFACE_ID Interface_ID(0x7f78405c, 0x43894b27)

#define HARDWARE_RENDERER_ST_INTERFACE_ID Interface_ID(0x14716452, 0x65f35430)

#define D3DHWMESHCACHEMANAGER_INTERFACE_ID Interface_ID(0x7f7840ef, 0x11894b11)

/*!
\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
This interface provides access to the DirectX interfaces on the drawing thread
of max. This allows creation and loading of various DirectX objects such as
Textures. This class has many interfaces not currently implemented, although
they will compile and link correctly. This is due to support for DirectX 9,
meaning that when a compatible driver is released these methods will be
implemented. The documentation for this class only deals with methods that
developers can and would normally use.\n\n
The basic idea of this class is to provide the developer with access to the
DirectX device without actually giving the developer total control, and thus
possibility to de-stable the 3ds Max Viewports.\n\n
A pointer to this class can be obtained with the following code snippet: -\n\n
<b>ViewExp *pview = GetCOREInterface()-\>GetActiveViewport();</b>\n\n
<b>GraphicsWindow *gw = pview-\>getGW();</b>\n\n
<b>IHardwareRenderer * phr = (IHardwareRenderer
*)gw-\>GetInterface(HARDWARE_RENDERER_INTERFACE_ID);</b>\n\n
For an example usage of this class see
<b>MAXSDK/SAMPLES/HardwareShaders/LightMap/Lightmap.cpp</b>\n\n
   */
class IHardwareRenderer : public IHardwareShader
{
public:
	virtual Interface_ID	GetID() { return HARDWARE_RENDERER_INTERFACE_ID; }

	// Interface Lifetime
	virtual LifetimeType	LifetimeControl() { return noRelease; }

	// Device Information
	virtual int		GetMaxTexStages() = 0;

	// Active Viewport Windows
	virtual int		GetNumActiveWindows() = 0;

	// Screen Update
	virtual bool	UpdateScreen(GFX_ESCAPE_FN fn) = 0;

	// Depth Peeling for Transparency
	virtual bool	CanDepthPeel() = 0;
	virtual void	CreateLayer(int layerNum) = 0;
	virtual void	BlendLayers() = 0;

	// Start and End of Drawing a Node
	virtual void	StartNode() = 0;
	virtual void	EndNode(GFX_ESCAPE_FN fn) = 0;

	// Material Settings
	virtual void	SetColor(ColorType t, D3DXCOLOR *pClr) = 0;
	virtual void	SetMaterial(HardwareMaterial *pMtl) = 0;

	virtual D3DXCOLOR	*GetDiffuseColor() = 0;

	//return Display All Triangle Edges in DirectX mode for objects that are not selected and in the modifier panel
	//if this is the case all edges will be drawn
	virtual bool	DisplayAllTriangleEdges() = 0;
	virtual void	SetDisplayAllTriangleEdges(bool displayAllEdges) = 0;

	// Cached Mesh Data
	virtual bool	CanCacheMeshData() = 0;

	// Draw a 3ds max Mesh
	//
	//	type = D3DPRIMITIVETYPE type of primitive
	//	nPrim = number of primitives
	//	pAttr = array of length nPrim indicating which primitives share
	//		attributes, e.g. color, material, effect
	//	nComp = number of separate vertex data arrays
	//	pUse = array of length nComp indicating D3DDECLUSAGE of each data array
	//	ppInd = array of nComp arrays each the same length and long enough
	//		to handle nPrim, the connectivity data for the IndexBuffer
	//	fComp = bit flags for number of float components in each data array,
	//		e.g. 0x00 = 4 floats, 0x01 = 1 float, 0x02 = 2 floats, and
	//		0x03 = 3 floats, enough space for 16 flags
	//	pLen = array of nComp values giving lengths of data arrays
	//	ppData = array of nComp float arrays with data, each being 1X, 2X, 3X,
	//		or 4X the length specified in pLen depending on the fComp bit flag
	//	attr = which attribute set to draw

	virtual void	DrawMesh(DWORD type, DWORD nPrim, DWORD *pAttr,
		DWORD nComp, DWORD *pUse, DWORD **ppInd, DWORD fComp, DWORD *pLen,
		float **ppData,	DWORD attr, GFX_ESCAPE_FN fn) = 0;

	// Build a D3DXMesh from a 3ds max Mesh
	//
	//	Returns a pointer to a GFX D3DXMesh class that wraps the D3DXMesh
	//	object

	virtual DWORD_PTR	BuildMesh(DWORD nFaces, DWORD *pAttr, DWORD nComp,
		DWORD *pUse, DWORD **ppInd, DWORD fComp, DWORD *pLen, float **ppData,
		GFX_ESCAPE_FN fn) = 0;

	virtual DWORD_PTR	BuildMesh(IHardwareMesh *pHWMesh,
		GFX_ESCAPE_FN fn) = 0;

	// Draw a D3DXMesh or part of a D3DXMesh
	//
	//	pMesh = pointer to a GFX D3DMesh class
	//	attr = attribute value to match with primitive attributes

	virtual void	DrawMesh(DWORD_PTR pMesh, DWORD attr,
		GFX_ESCAPE_FN fn) = 0;

	// Load a D3DXMesh from a .X file
	//
	//	Returns a pointer to a GFX D3DXMesh class that wraps the D3DXMesh
	//	object

	virtual DWORD_PTR	LoadMesh(LPCTSTR filename) = 0;

	// Save a D3DXMesh to a .X file
	//
	//	pMesh = pointer to a GFX D3DMesh class

	virtual void	SaveMesh(LPCTSTR filename, DWORD_PTR pMesh) = 0;

	// Free a D3DXMesh
	//
	//	pMesh = pointer to a GFX D3DMesh class

	virtual void	FreeMesh(DWORD_PTR pMesh) = 0;

	// Build a GFX IndexedBuffer, a VertexBuffer and IndexBuffer pair
	//
	//	Returns a pointer to a GFX D3DIndexedBuffer class

	virtual DWORD_PTR	BuildIndexedBuffer(DWORD type, DWORD nPrim,
		DWORD *pAttr, DWORD nComp, DWORD *pUse, DWORD **ppInd, DWORD fComp,
		DWORD *pLen, float **ppData, GFX_ESCAPE_FN fn) = 0;

	// Draw an IndexedBuffer or part of an IndexedBuffer
	virtual void	DrawIndexedBuffer(DWORD_PTR pBuffer, DWORD attr,
		GFX_ESCAPE_FN fn) = 0;

	// Free an IndexedBuffer
	virtual void	FreeIndexedBuffer(DWORD_PTR pBuffer) = 0;

	// Build a D3DTexture from a 3ds max texture
	//
	//	bmi = 3ds max bitmap texture
	//	mipLevels = number of MipMap levels to build
	//	usage = type, e.g. Texture, RenderTarget, DepthStencil
	//	format = pixel format, e.g. A8R8G8B8
	//
	//	Returns a pointer to a GFX D3DTexture class
	/*! \remarks This will create a DX texture stored in local storage. The
	pointer returned can be used in IHardwareMaterial::SetTexture method. An
	example of using this method can be seen in the Lightmap sample.\n\n

	\par Parameters:
	<b>BITMAPINFO *bmi</b>\n\n
	A pointer to the bitmap from which the texture will be created\n\n
	<b>UINT miplevels</b>\n\n
	The number of miplevels to create\n\n
	<b>DWORD usage</b>\n\n
	The usage falg is the same as for D3DXCreateTexture - see the DirectX
	documentation for more information\n\n
	<b>DWORD format</b>\n\n
	The pixel format for the texture */
	virtual DWORD_PTR	BuildTexture(BITMAPINFO *bmi, UINT mipLevels,
		DWORD usage, DWORD format) = 0;

	// Load a D3DTexture from a file, includes CubeMaps
	//
	//	Returns a pointer to a GFX D3DTexture class
	/*! \remarks This simple loads a texture from the supplied filename using the
	default options for D3DXCreateTextureFromFile. Please refer to the DirectX
	documentation for further information. */
	virtual DWORD_PTR	LoadTexture(LPCTSTR filename) = 0;

	// Save a D3DTexture to a file, includes CubeMaps
	virtual void	SaveTexture(LPCTSTR filename, DWORD_PTR pTex) = 0;

	// Free a D3DTexture, includes CubeMaps
	virtual void	FreeTexture(DWORD_PTR pTex) = 0;

	// Build a D3DCubeMap from a set of D3DTextures
	//
	//	size = edge size, e.g. width and height
	//	mipLevels = number of MipMap levels to build
	//	usage = type, e.g. Texture, RenderTarget, DepthStencil
	//	format = pixel format, e.g. A8R8G8B8
	//	pTex = array of six D3DTextures to use for the faces
	//
	//	Returns a pointer to a GFX D3DCubeMap class

	virtual DWORD_PTR	BuildCubeMap(UINT size, UINT mipLevels, DWORD usage,
		DWORD format, DWORD_PTR *pTex) = 0;

	// Set a face of a D3DCubeMap
	//
	//	pCubeMap = pointer to a GFX D3DCubeMap class
	//	face = face to set, e.g. positive X
	//	mipLevel = number of MipMap levels to build
	//	pTex = D3DTexture to use for the face

	virtual void	SetCubeMapFace(DWORD_PTR pCubeMap, DWORD face,
		UINT mipLevel, DWORD_PTR pTex) = 0;

	// Build a D3DVertexDeclaration
	//
	//	nComp = number of separate vertex data components
	//	pUse = array of length nComp indicating D3DDECLUSAGE of each data array
	//	fComp = bit flags for number of float components in each data array,
	//		e.g. 0x00 = 4 floats, 0x01 = 1 float, 0x02 = 2 floats, and
	//		0x03 = 3 floats, enough space for 16 flags
	//
	//	Returns a pointer to a GFX D3DVertexDecl class

	virtual DWORD_PTR	BuildVertexDecl(DWORD nComp, DWORD *pUse,
		DWORD fComp) = 0;

	// Set a D3DVertexDeclaration
	virtual void	SetVertexDecl(DWORD_PTR pVertexDecl) = 0;

	// Free a D3DVertexDeclaration
	virtual void	FreeVertexDecl(DWORD_PTR pVertexDecl) = 0;

	// Build a D3DVertexShader
	//
	//	code = string with Shader instructions
	//
	//	Returns a pointer to a GFX D3DVertexShader class

	virtual DWORD_PTR	BuildVertexShader(LPCSTR code) = 0;

	// Load a D3DVertexShader from a file
	//
	//	Returns a pointer to a GFX D3DVertexShader class

	virtual DWORD_PTR	LoadVertexShader(LPCTSTR filename) = 0;

	// Save a D3DVertexShader to a file
	virtual void	SaveVertexShader(LPCTSTR filename, DWORD_PTR pVertexShader) = 0;

	// Set a D3DVertexShader
	virtual void	SetVertexShader(DWORD_PTR pVertexShader) = 0;

	// Set D3DVertexShader Constants
	//
	//	nReg is the number of the register where the data loading starts,
	//	type is the D3DXPARAMETERTYPE of the parameter, pData is the actual
	//	data and nData is the number of four 32 bit sets of data
	//
	virtual void	SetVertexShaderConstant(DWORD_PTR pVertexShader,
		DWORD nReg, DWORD type, void *pData, DWORD nData) = 0;

	// Free a D3DVertexShader
	virtual void	FreeVertexShader(DWORD_PTR pVertexShader) = 0;

	// Build a D3DPixelShader
	//
	//	code = string with Shader instructions
	//
	//	Returns a pointer to a GFX D3DPixelShader class

	virtual DWORD_PTR	BuildPixelShader(LPCSTR code) = 0;

	// Load a D3DPixelShader
	//
	//	Returns a pointer to a GFX D3DPixelShader class

	virtual DWORD_PTR	LoadPixelShader(LPCTSTR filename) = 0;

	// Save a D3DPixelShader to a file
	virtual void	SavePixelShader(LPCTSTR filename, DWORD_PTR pPixelShader) = 0;

	// Set a D3DPixelShader
	virtual void	SetPixelShader(DWORD_PTR pPixelShader) = 0;

	// Set D3DPixelShader Constants
	//
	//	nReg is the number of the register where the data loading starts,
	//	type is the D3DXPARAMETERTYPE of the parameter, pData is the actual
	//	data and nData is the number of four 32 bit sets of data
	//
	virtual void	SetPixelShaderConstant(DWORD_PTR pPixelShader,
		DWORD nReg, DWORD type, void *pData, DWORD nData) = 0;

	// Free a D3DPixelShader
	virtual void	FreePixelShader(DWORD_PTR pPixelShader) = 0;

	// Build a D3DXEffect
	//
	//	Returns a pointer to a GFX D3DEffect class

	virtual	DWORD_PTR	BuildEffect(LPCSTR code) = 0;

	// Load a D3DXEffect from a file
	virtual DWORD_PTR	LoadEffect(LPCTSTR filename) = 0;

	// Save a D3DXEffect to a file
	virtual void	SaveEffect(LPCTSTR filename, DWORD_PTR pEffect) = 0;

	// Set D3DXEffect Parameters
	//
	//	name is the name of the parameter, type is the D3DXPARAMETERTYPE of the
	//	parameter, pData is the actual data
	//
	virtual void	SetEffectParameter(DWORD_PTR pEffect, LPCTSTR name,
		DWORD type, void *pData) = 0;

	// Set D3DXEffect Technique
	//
	//	name is the name of the technique
	//
	virtual void	SetEffectTechnique(DWORD_PTR pEffect, LPCTSTR name) = 0;

	// Begin a D3DXEffect
	virtual void	BeginEffect(DWORD_PTR pEffect) = 0;

	// Set a D3DXEffect Pass
	virtual void	SetEffectPass(DWORD_PTR pEffect, DWORD nPass) = 0;

	// End a D3DXEffect
	virtual void	EndEffect(DWORD_PTR pEffect) = 0;

	// Free a D3DXEffect
	virtual void	FreeEffect(DWORD_PTR pEffect) = 0;

	// Begin D3DXEffect access
	//
	//	The D3DXEffect associated with the GFX D3DEffect is returned
	//
	virtual DWORD_PTR	BeginEffectAccess(DWORD_PTR pEffect, GFX_ESCAPE_FN fn) = 0;

	// End D3DXEffect access
	virtual void	EndEffectAccess(GFX_ESCAPE_FN fn) = 0;

	// Begin Direct3D Device access
	//
	//	The Direct3DDevice is returned
	//
	virtual DWORD_PTR	BeginDeviceAccess(GFX_ESCAPE_FN fn) = 0;

	// End Direct3D Device access
	virtual void	EndDeviceAccess(GFX_ESCAPE_FN fn) = 0;

	//! \brief This tells the mesh / mnmesh classes whether a shader has performed the drawing already so it can simply ignore the draw call.
	/*! When a shader draws an object, in certain situations 3ds max will also try to draw the object. 
      This can produce a "double draw" situation.
      This method used to query if the object has been drawn by the shader already. If it has you can simply ignore 
      your drawing code. However if you draw any support objects or subobject data, these can still be drawn, 
      you simply want to avoid drawing the actual triangles of the object.
		\return true if the object has already been drawn, false if not.
	*/
	virtual bool ObjectDrawnByShader() = 0;

	//! \brief This method lets you update the vertex positions of the DirectX mesh cache
	/*!	This allow you to keep the directx mesh cache around without having to delete it on geometric changes.
		\param[in] pMesh  this is the handle of the directx mesh
		\param[in] vertexList  this is a pointer to an array containing the new vertex positions
		\param[in] gfxNormalList  this is a pointer to an array containing the new vertex normals. This is the gfxnormals ie teh flattened list of the render normals
		\param[in] faceNormalList  this is a pointer to an array containing the new face normals. This is to handle face with no smoothing group which default to the face normal instead of the render normals
		\param[in] changedVerts  this is a bitarray that marks all the vertices that have changed.  This lets us know which sectors of the mesh we don;t have to change thus making the operation faster
		\param[in] count  this is the number of vertices in the vertexList
	*/
	virtual void DXCacheUpdateVertexBuffer(DWORD_PTR pMesh, Point3 *vertexList,Point3 *gfxNormalList, Point3 *faceNormalList, BitArray &changedVerts, int count) = 0;

};

class IHardwareRendererST : public IHardwareRenderer
{
public:
	virtual Interface_ID	GetID() { return HARDWARE_RENDERER_ST_INTERFACE_ID; }
};


/*!
\par Description:
<b>This class is only available in release 10 or later.</b>\n\n
This interface provides access to the D3D HWMeshCache Manager.  This allows
nodes or any other "thing" that needs to display to register a HW Mesh with the manager
these hardware meshes are sorted and displayed by material to maximize performance and minimize
state switch
\n\n
<b>ViewExp *pview = GetCOREInterface()-\>GetActiveViewport();</b>\n\n
<b>GraphicsWindow *gw = pview-\>getGW();</b>\n\n
<b>ID3DHWMeshCacheManager * manager = (ID3DHWMeshCacheManager*)gw-\>GetInterface(D3DHWMESHCACHEMANAGER_INTERFACE_ID);</b>\n\n
\n\n
*/
class ID3DHWMeshCacheManager : public BaseInterface
{
public:

	virtual Interface_ID	GetID() { return D3DHWMESHCACHEMANAGER_INTERFACE_ID; }

	// Interface Lifetime
	virtual LifetimeType	LifetimeControl() { return noRelease; }

	//
	//! \brief This resets the list.  It removes all the D3D meshes from the manager.
	/*!	It removes all the D3D meshes from the manager.  This should be called whenever the manager
		needs to flushed for instance file resest, load etc.
	\param[in] BOOL clearBlock  - this will also remove any material list that are present.  Typically you do not want to do this so on the next add the data struct will not need to be rebukt
	*/
	virtual void D3DHWMeshCacheManagerReset(BOOL clearBlock = FALSE) = 0;

	//! \brief This method lets user add a D3D mesh to the manager to be drawn
	/*!	This method lets add a D3D mesh to the manager to be drawn.  It will be drawn batched based on the material supplied
		in the current display mode.  This returns TRUE if the node was succefully added else FALSE
	\param[in] INode *node this is the inode that owns this d3d mesh, it can be null
	\param[in] pMesh  this is the handle of the directx mesh which is a pointer to a D3DXMesh
	\param[in] Material *mtl this is the material for this mesh and can be null
	\param[in] HardwareMaterial *pHWMat this is the hwmaterial for this mesh and must be supplied
	\param[in] Point3 wireframeColor this is color of the wireframe mesh since it can be different than the shaded diffuse color
	\param[in] Matrix3 *tmOverRide this is the world space transform for the mesh, if null the gw transform will be used
	\param[in] attr this is the attribute of this particular mesh typically it is the material ID
	*/
	virtual BOOL D3DHWMeshCacheManagerAddHWMesh( INode *node, GFX_MESH::IHWDrawMesh *pMesh, Material *mtl, HardwareMaterial *pHWMat, Point3 wireframeColor, Matrix3 *tmOverRide, DWORD attr, DWORD renderLimit) = 0;

	//! \brief This allows you to set a render limit for the HW Mesh cache manager
	/*!
	This allows you to set a render limit for the HW Mesh cache so when you add a mesh to the 
	manager you can filter it by the display limit.  This is useful for when the display is 
	swapping draw modes and you only want to batch a certain mode.

	\param[in] DWORD limit this is this the limit filter so only objects matching this limit will be added
	If this is set 0 there will be no filtering
	*/
	virtual void D3DHWMeshCacheManagerSetRenderLimit(DWORD limit) = 0;

	//! \brief This allows you to get a render limit for the HW Mesh cache manger
	/*!
	This allows you to get a render limit for the HW Mesh cache so when you add a mesh to the 
	manager you can filter it by the display limit.  This is useful for when the display is 
	swapping draw modes and you only want to batch a certain mode.
	*/
	virtual DWORD D3DHWMeshCacheManagerGetRenderLimit() = 0;

	//! \brief This method lets user add a HWMaterial
	/*!	This method lets add a  user add a HWMaterial.  This is useful if you are preprocessing materials so
		you can add them as you process them
	\param[in] HardwareMaterial *pHWMat this is the hwmaterial for this mesh and must be supplied
	\param[in] Point3 wireframeColor this is color of the wireframe mesh since it can be different than the shaded diffuse color
	*/
	virtual void D3DHWMeshCacheManagerRegisterMaterial(HardwareMaterial *mtl, Point3 wireframeColor) = 0;

	//! \brief This method lets the user remove add a D3D mesh from the manager 
	/*!	This method lets the user remove add a D3D mesh from the manager 
	\param[in] pMesh  this is the handle of the directx mesh to be removed
	*/
//	virtual void D3DHWMeshCacheManagerDeleteHWMesh(DWORD_PTR pMesh) = 0;
	virtual void D3DHWMeshCacheManagerDeleteHWMesh(GFX_MESH::IHWDrawMesh * pMesh) = 0;

	//! \brief This method draws the D3D HW Meshes
	/*!	This method draws the D3D HW Meshes
	\param[in] GraphicsWindow *gw which is the graphics window to draw to
	*/
	virtual void D3DHWMeshCacheManagerDisplay(GraphicsWindow *gw) = 0;

	virtual int D3DHWMeshCacheManagerTotalBlocksDrawn() = 0;
};



