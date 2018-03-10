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
// DESCRIPTION: General function to wrap the Dx SDK.  They support delayed loading
//				so as to remove the dependency on DX DLLs esp Dx10
// AUTHOR: Neil Hazzard - created Oct.31.06
//***************************************************************************/

#pragma once

#include "baseinterface.h"
#include <d3dx10.h>
#include <dxgi.h>
#include "MaxHeap.h"

#define D3D10_HARDWARE_RENDERER_INTERFACE_ID Interface_ID(0x7f23405c, 0x43314b27)
#define D3D10_GRAPHICS_WINDOW_INTERFACE_ID Interface_ID(0x7f12345e, 0x23564c43)
#define D3D10_SHADER_RESOURCE_INTERFACE_ID Interface_ID(0x4f3254ab, 0x2653de42)

//forward declaration
class INode;
class Mesh;
class MNMesh;
class Material;
class IHardwareMesh;

//! A D3D light, representing the light in the scene.
/*! 
\brief This structure is used to define the light data that is passed to the GPU.  There is a one to one match between
this, and the storage defined on GPU.
*/
struct D3D10SceneLight : public MaxHeapOperators
{
	/*! %Light position*/
	D3DXVECTOR4 m_Position;
	/*! %Light direction*/
	D3DXVECTOR4 m_Direction;
	/*! %Light diffuse colour component*/
	D3DXVECTOR4 m_Diffuse;
	/*! %Light specular colour component*/
	D3DXVECTOR4 m_Specular;
	/*! %Light ambient colour component*/
	D3DXVECTOR4 m_Ambient;
	/*!Attenuation data - currently this is not used, and is set to 1.0f*/
	D3DXVECTOR4 m_Attenuation;
	//! General info about the light
	/*!
	The vector contains the following data\n
	x = enabled/disabled\n
	y = type(0=spot,1=omni)\n 
	z = light falloff in radians\n
	w = light hotspot in radians
	*/
	D3DXVECTOR4 m_LightData;	
};

typedef D3D10SceneLight* lpD3D10SceneLight;

//! THis is work in progress and will be activated in another code drop
/*!
	This interface is used typically by the mesh and mnmesh to draw hardware optimized objects.
*/
class ID3D10HardwareRenderer : public BaseInterface
{
public:
	virtual Interface_ID GetID() { return D3D10_HARDWARE_RENDERER_INTERFACE_ID; }

	virtual bool BuildCachedMesh(IHardwareMesh *pHWMesh)=0;
	virtual bool DrawCachedMesh(DWORD_PTR pdwHWMesh, DWORD dwAttrib)=0;
	virtual void ReleaseCachedMesh(DWORD_PTR pdwHWMesh) = 0;

	virtual bool CanCacheMesh()=0;
	virtual bool ObjectDrawnByShader()=0;
	virtual void StartObject() =0;
	virtual void EndObject()=0;
	virtual int	 GetNumActiveWindows()= 0;

	virtual void SetMaterial(const Material & mtl)=0; // Yi GONG, May18th 2007 to fix defect 918155
	virtual void SetShaderResource(Material * gfxMat, Mesh * mesh, int mtlNum) =0;
	virtual void SetShaderResource(Material * gfxMat, MNMesh * mesh, int mtlNum)=0;

};

//! Main entry point to access the D3D 10 GFX layer
/*!
*/
class ID3D10GraphicsWindow : public BaseInterface
{
public:

	virtual Interface_ID GetID() { return D3D10_GRAPHICS_WINDOW_INTERFACE_ID; }

	/*! Access to the main D3D 10 device
	\returns The main D3D 10 device used by the GFX layer
	*/
	virtual ID3D10Device * GetDevice() = 0;

	/*! Access the World Matrix
	\return The World Matrix
	*/
	virtual D3DXMATRIX	GetWorldXform() const = 0;

	/*! Access the %View Matrix
	\return The %View Matrix
	*/
	virtual D3DXMATRIX	GetViewXform() const = 0;

	/*! Access the Projection Matrix
	\return The World Matrix
	*/
	virtual D3DXMATRIX	GetProjXform()const = 0;

	/*! Access the current viewport width and height
	\param *width A pointer to the buffer to receive the width value
	\param *height A pointer to the buffer to receive the height value
	*/
	virtual void GetWindowDimension(int * width, int * height)= 0;

	/*!Access the light used in the viewport.  Currently there are a total of 8 lights used, 
	\param index The index of the light to access
	\return A pointer to the light structure.  This can be NULL if an invalid index is used.
	*/
	virtual D3D10SceneLight * GetLight(int index)=0;

};

//! Interface used for drawing D3D 10 style hardware shaders
/*! A material would implement this interface to enable realtime shader drawing. 
The GFX layer will use this interface to communicate to the material, and control the draw sequence.
*/
class ID3D10ShaderResource : public BaseInterface
{
public:
	virtual Interface_ID GetID() { return D3D10_SHADER_RESOURCE_INTERFACE_ID; }

	//!Called at the beginning of the object draw sequence
	/*!Allows the material to setup all shader resources, prior to rendering.
	\param pD3D10gw A pointer the D3D10 GFX layer
	*/
	virtual void InitializeDrawSequence(ID3D10GraphicsWindow* pD3D10gw)=0;

	//! Determines  whether the shader is peforming its own rendering
	/*! The GFX layer will use this to determine whether the material is performing the rendering of the object. It
	will set a flag in the GFX layer which will be used else where, in particular by the mesh/mnmesh classes, to avoid 
	double drawing of the object.
	\return True/false depending on whether the shader is active.
	*/
	virtual bool IsShaderResourceActive() =0;

	//!Signal a draw request
	/*! The GFX layer will call this method to signify that the object needs to be drawn
	\param *node The actual node being rendered
	\param *mesh The actual mesh being rendered
	\param mtlNum The material number, in case of a multi material
	*/
	virtual void DrawShaderResource(INode * node, Mesh * mesh, int mtlNum) =0;

	//!Signal a draw request
	/*! The GFX layer will call this method to signify that the object needs to be drawn
	\param *node The actual node being rendered
	\param *mesh The actual mesh being rendered
	\param mtlNum The material number, in case of a multi material
	*/
	virtual void DrawShaderResource(INode * node, MNMesh * mesh, int mtlNum) =0;

};

