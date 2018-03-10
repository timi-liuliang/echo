/**********************************************************************
*<
FILE: DeprecatedRealtimeMaterialTexturemapShaderAPI

DESCRIPTION:	Main Include for deprecated DirectX and MS Effect related code
				
CREATED BY:		Neil Hazzard

HISTORY:		June 2010

*>	Copyright (c) 2010, All Rights Reserved.
**********************************************************************/
#pragma once

#include "maxheap.h"

//! \deprecated Shader generation is now handled by MetaSL and the mental mill compiler
/*! Shader generators were built on top of Ashli, a shader compiler from AMD ATI.  Now all shader
generation in 3ds Max is performed by mental mill.  To get similar data being generated based on 
actual supported shader code, the developer is advised to use the IEffectFile interface
*/
class MAX_DEPRECATED IHLSLCodeGenerator: public MaxHeapOperators{

public:
	enum CodeVersion {
		PS_1_X,
		PS_2_0,
		PS_3_0,
		PS_2_X,
	};

	//! Access to the factory class
	DllExport static IHLSLCodeGenerator * GetHLSLCodeGenerator();

	//!Generate HLSL Code based on the material
	/*! This will create HLSL code based on the material and lights in the scene.  It will generate code for Blinn and ONB style
	specular component.  It will support Ambient,Diffuse, specular, Self Illumination, opacity, bump, reflection maps.
	\param *mtl The material source
	\param lights A table of lights to used in the HLSL generation
	\param &transp True if there is a transparent component
	\returns A string containing the code.
	*/
	virtual MCHAR * GenerateFragmentCode(StdMat2 * mtl, Tab <INode *> lights, bool & transp) = 0;

	//! Generates a basic Vertex shader to support the pixel shader
	/*\
	\return A string containing the code
	*/
	virtual MCHAR * GenerateVertexCode()=0;

	//! Generates an Effect File based on the material supplied.
	/*! This is similar to GenerateFragmentCode, but will optimize the HLSL code to assembler, based on a pass compiler.
	The effect file generated will only represent what is currently seen in the viewport
	\param *mtl The material that will be used to generate the %effect
	\param lights An array of lights to be used by the shader
	\param code The Shader Model to compile to
	\param &transp  Tells the code if transparency is used.  This can be used to setup specific RenderStates
	\param instSize The maximum number of instructions
	\param useLPRT This is not needed by the developer.  It is used in conjunction with Ashli
	\return A string containing the %effect file.  This can be loaded in the DirectX 9 Shader material
	*/
	virtual MCHAR * GenerateEffectFile(StdMat2 * mtl,Tab <INode *> lights,CodeVersion code, bool & transp, int instSize, bool useLPRT = false) =0;

	//! Get the texture map used by the specified mapName
	/*! When the material tree is processed, a texture map can provide its only HLSL code and texture samplers.  This function
	will return the texmap of these maps and the default diffuse, bump etc..
	\param *mat The standard material whose textures are to be queried
	\param *mapName This is the name parsed out in the %effect file
	\param &bump Allows special handling of the bump map
	\returns A pointer to the specified texture map
	*/
	virtual Texmap* GetShaderDefinedTexmap(StdMat2 * mat, MCHAR * mapName, bool & bump)=0;

//	virtual ~IHLSLCodeGenerator() {}
	virtual void Release() = 0;
};

//! \deprecated Shader generation is now handled by MetaSL and the mental mill compiler
/*! Shader generators were built on top of Ashli, a shader compiler from AMD ATI.  Now all shader
generation in 3ds Max is performed by mental mill.  To get similar data being generated based on 
actual supported shader code, the developer is advised to use the IEffectFile interface
*/
class MAX_DEPRECATED IHLSLCodeGenerator2
{
public:
	enum CodeVersion {
		PS_1_X,
		PS_2_0,
		PS_3_0,
		PS_2_X,
	};

	//! Generates an Effect File based on the material supplied.
	/*! This is similar to GenerateFragmentCode, but will optimize the HLSL code to assembler, based on a pass compiler.
	This version of the function will generate code for all maps whether they are visible in the viewport or not.
	\param *mtl The material that will be used to generate the %effect
	\param lights An array of lights to be used by the shader
	\param code The Shader Model to compile to
	\param &transp  Tells the code if transparency is used.  This can be used to setup specific RenderStates
	\param instSize The maximum number of instructions
	\param useLPRT This is not needed by the developer.  It is used in conjunction with Ashli
	\return A string containing the %effect file.  This can be loaded in the DirectX 9 Shader material
	*/
	virtual MCHAR * GenerateEffectFile2(StdMat2 * mtl,Tab <INode *> lights,CodeVersion code, bool & transp, int instSize, bool useLPRT = false) =0;

	//!Generate HLSL Code based on the material
	/*! This will create HLSL code based on the material and lights in the scene.  It will generate code for Blinn and ONB style
	specular component.  It will support Ambient,Diffuse, specular, Self Illumination, opacity, bump, reflection maps.
	This version of the function will generate code for all maps whether they are visible in the viewport or not.
	\param *mtl The material source
	\param lights A table of lights to used in the HLSL generation
	\param &transp True if there is a transparent component
	\returns A string containing the code.
	*/
	virtual MCHAR * GenerateFragmentCode2(StdMat2 * mtl, Tab <INode *> lights, bool & transp) = 0;

	//! Access to the factory class
	DllExport static IHLSLCodeGenerator2 * GetHLSLCodeGenerator2();

};

//!\deprecated
#define HLSL_TEXTURE_INTERFACE Interface_ID(0xc2b2185, 0x635556bf)
#pragma deprecated ("HLSL_TEXTURE_INTERFACE")

//! \deprecated
class MAX_DEPRECATED TextureData: public MaxHeapOperators{
public:
	//! UI Name to appear in the Effect file
	MCHAR UIName[25];

	//! Sampler name is used as the filename for the texture, in the %effect file.  It is also used as a lookup for the texmap
	MCHAR SamplerName[25];

	//! The sub map number the texture refers to - the calling code will use texmap->GetSubTexmap(SubMapNum) to access it.
	int SubMapNum;
};

//typedef Tab < TextureData> TexDataList;
//#pragma deprecated ("TextureData")
//!\deprecated This  is no longer used as a method to build a shade tree in 3ds Max
/*! With the introduction of MetaSL as the core shading language, existing ways to build a shade tree have
been deprecated.  To build shaders of this form the developer is advised to use MetaSL and corresponding
MetaSL workflows
*/
class  MAX_DEPRECATED IHLSLTexmap : public BaseInterface
{
public:

	//! Specifies that the Texmap will provide HLSL code for the DX Standard Material
	/*! 
	\return True if the Texmap supports HLSL code generation.
	*/
	virtual bool DoesSupportHLSL(){return false;}
	
	//! Allow texture to provide an hlsl representation of the texture.
	/*! 
	\param *code A pointer to a buffer to store the HLSL code
	\param *entryPoint A pointer to a buffer to store the name of the function
	*/
	virtual void GetTexmapHLSLFunction(MCHAR * code, MCHAR * entryPoint){ UNUSED_PARAM(code); UNUSED_PARAM(entryPoint); };

	//! Provide access to any textures used in the HLSL code
	/*!
	This is used to store data to allow the HLSL compiler to find the sub maps used by the texture map.
	This also included a UI name for the %effect file generation
	\param *list A pointer to a TexDataList that is used to store texture info
	*/
	virtual void GetTextureData(LPVOID list){ UNUSED_PARAM(list); };

};