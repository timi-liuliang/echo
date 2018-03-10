/**********************************************************************
 *<
	FILE: IDX8PixelShader.h

	DESCRIPTION: DirectX 8 Pixel Shader Interface Definition

	CREATED BY: Nikolai Sander and Norbert Jeske

	HISTORY: Created 9/27/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#pragma once
#include "baseinterface.h"
#include <d3dx9.h>

// forward declarations
class ID3DGraphicsWindow;
class IDX8VertexShader;
class Material;
class INode;

#define DX8_PIXEL_SHADER_INTERFACE_ID Interface_ID(0x56df1953, 0xc6121a3)

/*! \sa  Class BaseInterface,  Class Material, Class INode\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The abstract interface to the Direct-3D Pixel Shader architecture.  */
class IDX8PixelShader : public BaseInterface
{
public:
	/*! \remarks This method returns the interface ID of the class.
	\par Default Implementation:
	<b>{ return DX8_PIXEL_SHADER_INTERFACE_ID; }</b> */
	virtual Interface_ID GetID() { return DX8_PIXEL_SHADER_INTERFACE_ID; }

	// Confirm that the Direct3D Device can handle this PixelShader
	/*! \remarks This method will confirm that the Direct3D Device can handle
	this PixelShader.
	\par Parameters:
	<b>ID3DGraphicsWindow *gw</b>\n\n
	A pointer to the Direct-3D Graphics Window. */
	virtual HRESULT ConfirmDevice(ID3DGraphicsWindow *gw) = 0;

	// Confirm that an associated VertexShader will work with this PixelShader
	/*! \remarks This method will confirm that an associated VertexShader will
	work with this PixelShader.
	\par Parameters:
	<b>IDX8VertexShader *pvs</b>\n\n
	A pointer to the vertex shader to check for. */
	virtual HRESULT ConfirmVertexShader(IDX8VertexShader *pvs) = 0;

	// Load PixelShader instructions and textures.  PixelShader instructions
	// should be loaded once and shared among all the nodes using this
	// PixelShader.  In addition, any textures necessary for the PixelShader
	// effect should be loaded once and shared among all the nodes using this
	// PixelShader.
	/*! \remarks This method will load the PixelShader instructions and
	textures. PixelShader instructions should be loaded once and shared among
	all the nodes using this PixelShader. In addition, any textures necessary
	for the PixelShader effect should be loaded once and shared among all the
	nodes using this PixelShader.
	\par Parameters:
	<b>Material *mtl</b>\n\n
	A pointer to the pixel shader material.\n\n
	<b>INode *node</b>\n\n
	A pointer to the node. */
	virtual HRESULT Initialize(Material* mtl, INode* node) = 0;

	// Number of passes for the effect this PixelShader creates.  Note that
	// this value will depend on the hardware currently in use.
	/*! \remarks This method returns the number of passes for the effect this
	PixelShader creates. Note that this value will depend on the hardware
	currently in use. */
	virtual int GetNumMultiPass() = 0;

	// Retrieve the PixelShader handle for the specified pass for use in GFX
	/*! \remarks This method returns the PixelShader handle for the specified
	pass for use in GFX.
	\par Parameters:
	<b>int numPass</b>\n\n
	The pass for which to return the pixelshader handle. */
	virtual DWORD GetPixelShaderHandle(int numPass) = 0;

	// Set the PixelShader for the specified pass.  This call will be made at
	// least once per object to set the per object data for the PixelShader
	// such as the PixelShader constants.
	/*! \remarks This method allows you to set the PixelShader for the
	specified pass. This call will be made at least once per object to set the
	per object data for the PixelShader such as the PixelShader constants.
	\par Parameters:
	<b>ID3DGraphicsWindow *gw</b>\n\n
	A pointer to the Direct-3D Graphics Window.\n\n
	<b>int numPass</b>\n\n
	The pass for which to set the pixel shader. */
	virtual HRESULT SetPixelShader(ID3DGraphicsWindow *gw, int numPass) = 0;
};
