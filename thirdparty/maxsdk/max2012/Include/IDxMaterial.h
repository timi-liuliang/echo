/**********************************************************************
*<
FILE: IDxMaterial.h

DESCRIPTION:	Interface for accessing the bitmaps used by effects in the DxMaterial

CREATED BY:		Neil Hazzard

HISTORY:		Created:  07/08/03


*>	Copyright (c) 2003, All Rights Reserved.
**********************************************************************/
#pragma once

#include "iFnPub.h"
#include "AssetManagement/AssetUser.h"

// forward declarations
class IEffectParser;
class IParameterManager;
class ILightManager;

typedef interface ID3DXEffect * LPD3DXEFFECT;
typedef LPCSTR D3DXHANDLE;
typedef D3DXHANDLE *LPD3DXHANDLE;


#define IDXMATERIAL_INTERFACE Interface_ID(0x55b3201d, 0x29ab7fc3)
#define IDXMATERIAL2_INTERFACE Interface_ID(0x54e4202f, 0x30ab6bc4)
#define IDXMATERIAL3_INTERFACE Interface_ID(0xde24e62f, 0x1a8fd412)

//! \brief An interface class to manage access to various parameters used by DirectX effects.
/*! \par Description:
 The Effect file has many internal presentations and as such the DxMaterial does not always store a 
 one to one look up.  Many parameters are stored in a ParamBlock and can be accessed that way.  
 The paramblock index for the Effect is located at 0 (the material maintains 5 ParamBlocks) and hosts 
 all the parameters with the param name being that of the parameter in the effect File.\n\n 
 However not all entries in the paramblock match the parameter type of the effect file directly. 
 One such example is the light data, which in the paramblock will use a TYPE_INTLISTBOX, where in the 
 Effect file it is a vector.\n\n 
 To overcome this, the interface provides direct access to the lights and textures used by the effect. 
 It is possible to both get and set the bitmaps, but only read access is available for the lights. */
class IDxMaterial : public FPMixinInterface
{

public:
	enum LightSemantics
	{
		LIGHT_COLOR,
		LIGHT_DIRECTION,
		LIGHT_POSITION,
		LIGHT_FALLOFF,
		LIGHT_HOTSPOT,
		LIGHT_ATTENUATION
	};

	//!Reloads the current active effect
	virtual void ReloadDXEffect()=0;

	//!Returns the number of bitmaps used by the currently loaded effect
	/*!
	The number of bitmaps returned will be equal to the number of bitmap entries found in the effect file.  This
	will include bitmap slots that haven't been assigned, so it is important to check the return value from the
	various Bitmap related functions that take an index.
	\return The number of bitmaps
	*/
	virtual int GetNumberOfEffectBitmaps()=0;

	//!Gets the bitmap used by the effect 
	/*!
	\param index The index of the bitmap to retrieve
	\return A PBBitmap pointer for the bitmap used
	*/
	virtual PBBitmap * GetEffectBitmap(int index)=0;

	//!Set the bitmap used by the effect
	/*!
	\param index The index of the bitmap to set
	\param *bmap A PBBitmap pointer for the bitmap to set
	*/
	virtual void SetEffectBitmap(int index,PBBitmap * bmap)=0;

	//!Get the Dx Effect file AssetUser
	/*!This can also be accessed via the paramblock, but it is simply provided for completeness
	\return The effect file in use
	*/
	virtual const MaxSDK::AssetManagement::AssetUser&  GetEffectFile() = 0;
	
	//!Set the Dx Effect file AssetUser
	/*!This can also be accessed via the paramblock, but it is simply provided for completeness
	\param file The effect file to set
	*/
	virtual void SetEffectFile(const MaxSDK::AssetManagement::AssetUser& file) = 0;
	
	//! Get the bitmap used for the software rendering overrride
	/*! This can also be set by the Paramblock, it is just used for completeness
	return The Bitmap used
	*/
	virtual PBBitmap * GetSoftwareRenderBitmap() = 0;

	//!Set the bitmap to be used by the Renderer.
	/*! This can also be set by the Paramblock, it is just used for completeness
	\param *bmap A PBBitmap specifiying the bitmap to use
	*/
	virtual void SetSoftwareRenderBitmap(PBBitmap * bmap) = 0;

	//************************************************************************************
	// The following method are not exposed through function publishing
	//************************************************************************************

	//!Get the number of light based parameters
	/*!This will return the number of parameters that are light based, even Light Color.  Care needs to be taken with Light Color
	as it could also have a Color Swatch associated with it, so could already have been exported as part of the ParamBlock.  
	\return The number of light based parameters
	*/
	virtual int GetNumberOfLightParams()=0;

	//!The actual node used by the parameter
	/*!This represent the light node used by the parameter.  Care needs to taken as this could be a NULL pointer.  There are two reason for this.
	The first is that the LightColor Semantic is stored internally as a LightElement, but the writer of the Effect file may not have specified 
	a parentID 	for the light, this will result in a NULL.  Secondly if the user has not actually assigned a light via the UI drop down list,
	then again this will again result in a NULL pointer.
	\param index The index of the light to return.
	\return The INode for the active light.
	*/
	virtual INode * GetLightNode(int index)=0;

	//!The name of the parameter in the Effect file
	/*
	 \param index The index of the light to retrieve
	 \return A MCHAR* containing the name
	 */
	virtual MCHAR * GetLightParameterName(int index)=0;

	//!The light semantic as defined in the effect file
	/*
	 \param index THe index of the light to retrieve	
	 \return The semantic represented as a LightSemantics
	 */
	virtual LightSemantics GetLightSemantic(int index)=0;

};

/*! \par Description
	This includes a new interface to help extract mapping and texture usage. With the new UI
	and general updates to the DirectX 9 Shader, the developer can access the mapping data directly
	from the paramblock hosting the effect parameters
*/
class IDxMaterial2 : public IDxMaterial
{

public:
	enum BitmapTypes
	{
		UNKNOWN_MAP,
		DIFFUSE_MAP,
		BUMP_MAP,
		SPECULAR_MAP,
		OPACITY_MAP,
		DISPLACEMENT_MAP,
		LIGHTMAP_MAP,
		NORMAL_MAP,
		REFLECTION_MAP
	};

	//! Get the Mapping channel defined for the texture 
	/*! The mapping channel as specified in the Effect file.  If no mapping is defined then this will be mapping channel 0
	
	\param index The index of the bitmap to access
	\return The mapping channel for the texture
	*/
	virtual int GetBitmapMappingChannel(int index) = 0;

	//! Get the usage of the bitmap
	/*! This is typically the usuage as seen from 3dsmax.  The usage is based on the Semantic used in the %effect file
	This would be used in conjunction with GetNumberOfEffectBitmaps
	\param index The index of the bitmap to query
	\return The usage of the bitmap
	*/
	virtual BitmapTypes GetBitmapUsage(int index) = 0;

	//! Set the mapping channel for the bitmap
	/*!This would be used in conjunction with GetNumberOfEffectBitmaps
	\param bitmapIndex The index of the bitmap
	\param mapChannel The mapping channel value
	*/
	virtual void SetMappingChannel(int bitmapIndex, int mapChannel)=0;

};

class IDxMaterial3 : public IDxMaterial2
{
public:
	//! Access to the current active parser for the loaded %effect file
	/*!
	\return The current parser
	*/
	virtual IEffectParser * GetCurrentParser()=0;

	//! Get the active light node based on the D3DXHANDLE
	/*!
	\param handle The handle based on the effect file
	\return The light node
	*/
	virtual INode * GetLightNode(D3DXHANDLE handle)=0;

	//! Get the active Parameter Manager
	/*!
	\return The current parameter manager owned by the material
	*/
	virtual IParameterManager* GetCurrentParameterManager() =0;

	//! Get total number of Techniques
	/*! Returns the total number of validated techniques used by the DirectX Shader material
	\return The number of techniques
	*/
	virtual int GetNumberOfTechniques() =0;

	//! Get the technique name
	/* Returns the technique name based on the index supplied
	\param index The index of the technique
	\return The name of the technique
	*/
	virtual const MCHAR * GetTechniqueName(int index)=0;

	//! Get the active technique name
	/* Returns the name of the current active technique
	\return The technique name
	*/
	virtual const MCHAR * GetCurrentTechniqueName()=0;

	//! Get the active light manager
	/*! Provides access to the light manager maintained by the material.  This provides access to various 
	light data like position, color and attenuation
	\return The current active light manager
	*/
	virtual ILightManager * GetCurrentLightManager() =0;

};

