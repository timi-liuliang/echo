/**********************************************************************
 *<
	FILE: mtl.h

	DESCRIPTION: Material and texture class definitions

	CREATED BY: Don Brittain

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "export.h"
#include "tab.h"
#include "matrix3.h"
#include "BaseInterface.h"

// forward declarations
class MtlBase;

#define UVSOURCE_MESH		0	// use UVW coords from a standard map channel
#define UVSOURCE_XYZ		1	// compute UVW from object XYZ
#define UVSOURCE_MESH2		2	// use UVW2 (vertexCol) coords
#define UVSOURCE_WORLDXYZ	3	// use world XYZ as UVW
#ifdef GEOREFSYS_UVW_MAPPING
#define UVSOURCE_GEOXYZ		4	// generate planar uvw mapping from geo referenced world xyz on-the-fly
#endif
#define UVSOURCE_FACEMAP	5	// use "face map" UV coords
#define UVSOURCE_HWGEN		6	// use hardware generated texture coords

// texture class definition
/*! \sa  Class MtlMakerCallback, Class Material, Class Matrix3.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class describes a texture used by the interactive renderer. This includes
all the information about the mapping channel, tiling, etc. A table of these is
maintained by the <b>texture</b> data member of class <b>Material</b>.\n\n
There are data members related to maps which specify how the texture should be
applied. These are specified independently for color and alpha and include a
scale. For example, for normal multiplication (modulation) application of a
texture, the entries would be:\n\n
<b>colorOp = GW_TEX_MODULATE</b>\n\n
<b>colorAlphaSource = GW_TEX_TEXTURE</b>\n\n
<b>colorScale = GW_TEX_SCALE_1X</b>\n\n
<b>alphaOp = GW_TEX_MODULATE</b>\n\n
<b>alphaAlphaSource = GW_TEX_TEXTURE</b>\n\n
<b>alphaScale = GW_TEX_SCALE_1X</b>\n\n
For applying a texture with alpha blending, the entry would be:\n\n
<b>colorOp = GW_TEX_ALPHA_BLEND</b>\n\n
<b>colorAlphaSource = GW_TEX_TEXTURE</b>\n\n
<b>colorScale = GW_TEX_SCALE_1X</b>\n\n
<b>alphaOp = GW_TEX_LEAVE</b>\n\n
<b>alphaAlphaSource = GW_TEX_TEXTURE</b>\n\n
<b>alphaScale = GW_TEX_SCALE_1X</b>\n\n
For applying an opacity map, the entry would be:\n\n
<b>colorOp = GW_TEX_LEAVE</b>\n\n
<b>colorAlphaSource = GW_TEX_TEXTURE</b>\n\n
<b>colorScale = GW_TEX_SCALE_1X</b>\n\n
<b>alphaOp = GW_TEX_REPLACE</b>\n\n
<b>alphaAlphaSource = GW_TEX_TEXTURE</b>\n\n
<b>alphaScale = GW_TEX_SCALE_1X</b>
\par Data Members:
<b>int useTex;</b>\n\n
Indicates if the material uses textures. Nonzero indicates textures are
used.\n\n
<b>int faceMap;</b>\n\n
Indicates if the material is face mapped. Nonzero indicates it is.\n\n
<b>DWORD_PTR textHandle;</b>\n\n
The texture handle.\n\n
<b>int uvwSource;</b>\n\n
The UVW source used.\n\n
<b>UVSOURCE_MESH</b>\n\n
Use UVW coordinates from a standard map channel.\n\n
<b>UVSOURCE_XYZ</b>\n\n
Compute UVW from object XYZ.\n\n
<b>UVSOURCE_MESH2</b>\n\n
Use UVW2 (Vertex Color) coordinates.\n\n
<b>UVSOURCE_WORLDXYZ</b>\n\n
Use World XYZ as UVW.\n\n
<b>int mapChannel;</b>\n\n
The mapping channel used.\n\n
<b>Matrix3 textTM;</b>\n\n
The texture transformation matrix.\n\n
<b>UBYTE tiling[3];</b>\n\n
The UVW tiling. One of the following values:\n\n
<b>GW_TEX_REPEAT</b>\n\n
<b>GW_TEX_MIRROR</b>\n\n
<b>GW_TEX_NO_TILING</b>\n\n
<b>UBYTE colorOp; </b>\n\n
The color texture operation. One of the following values:\n\n
<b>GW_TEX_LEAVE</b>\n\n
Use the source pixel value\n\n
<b>GW_TEX_REPLACE</b>\n\n
Use the texture pixel value\n\n
<b>GW_TEX_MODULATE</b>\n\n
Multiply the source with the texture\n\n
<b>GW_TEX_ADD</b>\n\n
Add the source and texture \n\n
<b>GW_TEX_ADD_SIGNED</b>\n\n
Add the source and texture with an 0.5 subtraction\n\n
<b>GW_TEX_SUBTRACT</b>\n\n
Subtract the source from the texture\n\n
<b>GW_TEX_ADD_SMOOTH</b>\n\n
Add the source and the texture then subtract their product\n\n
<b>GW_TEX_ALPHA_BLEND</b>\n\n
Alpha blend the texture with the source\n\n
<b>GW_TEX_PREMULT_ALPHA_BLEND</b>\n\n
Alpha blend the source with a premultiplied alpha\n\n
<b>UBYTE colorAlphaSource;</b>\n\n
The color blend alpha source. One of the following values:\n\n
<b>GW_TEX_ZERO</b>\n\n
Use no alpha value\n\n
<b>GW_TEX_SOURCE</b>\n\n
Use the source alpha\n\n
<b>GW_TEX_TEXTURE</b>\n\n
Use the texture alpha\n\n
<b>GW_TEX_CONSTANT</b>\n\n
Use a constant BGRA color as an alpha\n\n
<b>GW_TEX_PREVIOUS</b>\n\n
Use the previous texture stage alpha\n\n
<b>UBYTE colorScale;</b>\n\n
The color scale factor. One of the following values:\n\n
<b>GW_TEX_SCALE_1X</b>\n\n
Multiply the tex op result by 1\n\n
<b>GW_TEX_SCALE_2X</b>\n\n
Multiply the tex op result by 2\n\n
<b>GW_TEX_SCALE_4X</b>\n\n
Multiply the tex op result by 4\n\n
<b>UBYTE alphaOp;</b>\n\n
The alpha texture operation. One of the following values:\n\n
<b>GW_TEX_LEAVE</b>\n\n
Use the source pixel value\n\n
<b>GW_TEX_REPLACE</b>\n\n
Use the texture pixel value\n\n
<b>GW_TEX_MODULATE</b>\n\n
Multiply the source with the texture\n\n
<b>GW_TEX_ADD</b>\n\n
Add the source and texture \n\n
<b>GW_TEX_ADD_SIGNED</b>\n\n
Add the source and texture with an 0.5 subtraction\n\n
<b>GW_TEX_SUBTRACT</b>\n\n
Subtract the source from the texture\n\n
<b>GW_TEX_ADD_SMOOTH</b>\n\n
Add the source and the texture then subtract their product\n\n
<b>GW_TEX_ALPHA_BLEND</b>\n\n
Alpha blend the texture with the source\n\n
<b>GW_TEX_PREMULT_ALPHA_BLEND</b>\n\n
Alpha blend the source with a premultiplied alpha\n\n
<b>UBYTE alphaAlphaSource;</b>\n\n
The alpha blend alpha source. One of the following values:\n\n
<b>GW_TEX_ZERO</b>\n\n
Use no alpha value\n\n
<b>GW_TEX_SOURCE</b>\n\n
Use the source alpha\n\n
<b>GW_TEX_TEXTURE</b>\n\n
Use the texture alpha\n\n
<b>GW_TEX_CONSTANT</b>\n\n
Use a constant BGRA color as an alpha\n\n
<b>GW_TEX_PREVIOUS</b>\n\n
Use the previous texture stage alpha\n\n
<b>UBYTE alphaScale;</b>\n\n
The alpha scale factor. One of the following values:\n\n
<b>GW_TEX_SCALE_1X</b>\n\n
Multiply the tex op result by 1\n\n
<b>GW_TEX_SCALE_2X</b>\n\n
Multiply the tex op result by 2\n\n
<b>GW_TEX_SCALE_4X</b>\n\n
Multiply the tex op result by 4  */
class  TextureInfo : public BaseInterfaceServer {
public:
	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> useTex   = 1;</b>\n\n
	<b> faceMap   = 0;</b>\n\n
	<b> textHandle   = 0;</b>\n\n
	<b> uvwSource  = UVSOURCE_MESH;</b>\n\n
	<b> mapChannel  = 1;</b>\n\n
	<b> tiling[0] = tiling[1] = tiling[2] = GW_TEX_REPEAT;</b>\n\n
	<b> colorOp   = GW_TEX_MODULATE;</b>\n\n
	<b> colorAlphaSource = GW_TEX_TEXTURE;</b>\n\n
	<b> colorScale  = GW_TEX_SCALE_1X;</b>\n\n
	<b> alphaOp   = GW_TEX_LEAVE;</b>\n\n
	<b> alphaAlphaSource = GW_TEX_TEXTURE;</b>\n\n
	<b> alphaScale  = GW_TEX_SCALE_1X;</b> */
	DllExport TextureInfo();
	/*! \remarks Destructor. */
	virtual ~TextureInfo() {};
	DllExport TextureInfo& operator=(const TextureInfo &from);
	
	int			useTex;
	int			faceMap;
	DWORD_PTR	textHandle;  // texture handle
	int 		uvwSource;  
	int         mapChannel;
	Matrix3 	textTM;  // texture transform
	UBYTE 		tiling[3]; // u,v,w tiling:  GW_TEX_REPEAT, GW_TEX_MIRROR, or GW_TEX_NO_TILING
	UBYTE		colorOp;	// color texture operation
	UBYTE		colorAlphaSource;	// color blend alpha source
	UBYTE		colorScale;	// color scale factor
	UBYTE		alphaOp;	// alpha texture operation
	UBYTE		alphaAlphaSource;	// alpha blend alpha source
	UBYTE		alphaScale;	// alpha scale factor
};

// main material class definition
/*! \sa  Class TextureInfo, Template Class Tab, Class Point3, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_rendering_limits.html">Rendering Limits</a>.\n\n
\par Description:
This class describes the properties of a material used by the interactive
renderer.
\par Data Members:
<b>Point3 Ka;</b>\n\n
Ambient color setting. The values for x, y, and z range from 0.0 to 1.0. These
correspond to red, green, and blue respectively.\n\n
<b>Point3 Kd;</b>\n\n
Diffuse color setting. The values for x, y, and z range from 0.0 to 1.0. These
correspond to red, green, and blue respectively.\n\n
<b>Point3 Ks;</b>\n\n
Specular color setting. The values for x, y, and z range from 0.0 to 1.0. These
correspond to red, green, and blue respectively.\n\n
<b>float shininess;</b>\n\n
Shininess setting. This value ranges from 0.0 to 1.0.\n\n
<b>float shinStrength;</b>\n\n
Shininess strength setting. This value ranges from 0.0 to 1.0.\n\n
<b>float opacity;</b>\n\n
Opacity setting. This value ranges from 0.0 to 1.0.\n\n
<b>float selfIllum;</b>\n\n
Self Illumination setting. This value ranges from 0.0 to 1.0.\n\n
<b>int dblSided;</b>\n\n
Double sided material setting. Nonzero indicates double sided material;
otherwise single sided.\n\n
<b>int shadeLimit;</b>\n\n
Shading limit setting. See
<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_rendering_limits.html">Rendering Limits</a>.\n\n
<b>Tab\<TextureInfo\> texture;</b>\n\n
The table of textures used by this material.  */
class  Material : public BaseInterfaceServer {
protected:

public:
	/*! \remarks Class constructor. The data members are initialized as
	follows:\n\n
	<b> Ka[0] = Ka[1] = Ka[2] = 0.3f;</b>\n\n
	<b> Kd[0] = Kd[1] = Kd[2] = 0.9f;</b>\n\n
	<b> Ks[0] = Ks[1] = Ks[2] = 0.9f;</b>\n\n
	<b> shininess  = 10.0f;</b>\n\n
	<b> shinStrength = 1.0f;</b>\n\n
	<b> opacity   = 1.0f;</b>\n\n
	<b> selfIllum  = 0.0f;</b>\n\n
	<b> dblSided  = 0;</b>\n\n
	<b> shadeLimit  = 3;</b>\n\n
	<b> TextureInfo texInfo;</b>\n\n
	<b> texture.Append(1, \&texInfo);</b> */
	DllExport Material();
	/*! \remarks Class destructor. */
	virtual ~Material() {};
	/*! \brief Copy constructor. 
		\param [in] from The object that will be copied.
	*/
	DllExport Material(const Material &from);
	/*! \brief Assignment operator.
		\param [in] from The object that will be copied.
	*/
	DllExport Material& operator=(const Material &from);

    Point3		Ka;
    Point3		Kd;
    Point3		Ks;
    float		shininess;
    float		shinStrength;
    float		opacity;
	float		selfIllum;
	int			dblSided;
	int			shadeLimit;
	Tab<TextureInfo> texture;
	DllExport BaseInterface *GetInterface(Interface_ID id);
private:
	void CopyFrom(const Material &from);
};
