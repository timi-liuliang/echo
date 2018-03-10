/**********************************************************************
 *<
	FILE: IGameMaterial.h

	DESCRIPTION: Material interfaces for IGame

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: created 02/02/02

	IGame Version: 1.122

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
/*!\file IGameMaterial.h
\brief IGame material and texture Interfaces.
*/
#pragma once

#include "..\stdmat.h"
#include "..\TextureMapIndexConstants.h"
#include "IGameProperty.h"
#include "IGameType.h"

class IGameBitmapTex;
class IGameTextureMap;
class IGameUVGen;
class IGameFX;


//!Simple wrapper for 3ds Max materials
/*! IGameMaterial is IGame Wrapper around a basic Material.  It provides access to the basic material properties and 
Bitmap Textures used by the material.  Any material will be wrapped in this class, but only Standard Material is directly supported
with API access to the properties.  If the material is not directly supported then the data can be access via the IPropertyContainer
interface.
\sa IGameProperty, IGameScene
*/
class IGameMaterial : public IExportEntity
{
public:
	//! Check if the material is a Multi Material type 
	/*! This could be for a Blend or Mix material
	\return TRUE if is Multi material
	*/
	virtual bool IsMultiType()=0;

	//! Check if the material is a SubObject style Multi Material
	/*! This could be for 3ds Max's Multi Subobject material
	\return TRUE is a Subobject material
	*/
	virtual bool IsSubObjType()=0;

	//! Get the material name as seen in the Material Editor
	/*!
	\return The name of the material
	*/
	virtual MCHAR * GetMaterialName()=0;

	//! Get the material class as seen in the Material Editor
	/*!
	\return The name of the material class
	*/
	virtual MCHAR * GetMaterialClass()=0;

	//! Get the number of sub materials
	/*! The value is used by IGameScene::GetSubMaterial
	\return The number of Sub material
	\sa IGameScene::GetSubMaterial
	*/
	virtual int GetSubMaterialCount()=0;

	//! Access to any sub material.
	/*! The sub material is any material used by a multi material. For example, a Top/Bottom material the sub materials
	would be the top and bottom
	\param index Index into the submaterial
	\return A Pointer to a IGameMaterial
	\sa IGameMaterial, IGameScene::GetRootMaterial
	*/
	virtual IGameMaterial * GetSubMaterial(int index)  =0;

	//! Get the MatID for subobject materials 
	/*! This value represents the MatID used on objects to define what faces receive this material
	\param subIndex The index of the submaterial to retrieve 
	\return The MatID of the material
	*/
	virtual int GetMaterialID(int subIndex)=0;

	//! Get the Ambient Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetAmbientData()=0;

	//! Get the Diffuse Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetDiffuseData()=0;

	//! Get the Emissive Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetEmissiveData()=0;

	//! Get the Specular Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetSpecularData()=0;

	//! Get the Opacity Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetOpacityData()=0;

	//! Get the Glossiness Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetGlossinessData()=0;

	//! Get the Specular Level Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetSpecularLevelData()=0;

	//! Get the Emissive Amount Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty *  GetEmissiveAmtData()=0;

	//! Get the number of Textures used by the material
	/*!
	\return The texture count.
	*/
	virtual int GetNumberOfTextureMaps()=0;

	//!Access to the actual Texture Map
	/*!
	\param index The index to the  Texture Map to retrieve
	\return A pointer to a IGameTextureMap
	*/
	virtual IGameTextureMap * GetIGameTextureMap(int index)  =0;

	//! Access to the actual 3ds Max material definition
	/*! This allows developer access to the complete 3ds Max material if further data access is needed
	\return A pointer to a standard max Mtl class
	*/
	virtual Mtl * GetMaxMaterial()=0;

	//! Access the D3D effect interface for the material
	/*! This can be NULL if the material does not support effects - currently only the StdMtl2 and DxMaterial are supported
	*/
	virtual IGameFX* GetIGameFX()=0;

};


//!Simple wrapper for 3ds Max textures
/*! A generic class that wraps all the 3ds Max texture maps.  This class directly supports the Bitmap Texture.  This can be
tested for by calling IExportEntity::IsEntitySupported.  If it is not supported then access to the paramblocks, if defined can be obtained
by using the properties interface. The usual texture map properties including coordinate rollout access are provided.
*/

class IGameTextureMap : public IExportEntity
{
public:

	//! Get the texture map name as seen in the Material Editor
	/*! The name of the TextureMap as seen in the material editor/material browser.
	\returns The name of the texture map
	*/
	virtual MCHAR * GetTextureName() = 0;

	//! Get the Texture class as seen in the Material Editor
	/*!
	\return The name of the texture class
	*/
	virtual MCHAR * GetTextureClass()=0;


	//!Access to the actual 3ds Max definition
	/*! This allows the developer to get hold of extra data such as Texture Transforms, specified from the 
	Coordinates rollout.  3ds Max method of GetUVGen or GetXYZGen can be used for more advanced access
	\return A pointer to a max class Texmap
	*/
	virtual Texmap * GetMaxTexmap() = 0;
	
	//!Access to the Coordinate Rollout
	/*!If the developer needs access to the transforms applied to the texture, then this can be accessed here.
	\returns A pointer to IGameUVGen
	*/
	virtual IGameUVGen * GetIGameUVGen()=0;

	//!Get the map channel being used by the texture 
	/*! 
	\return The map channel. Valid value is 1..99
	*/
	virtual int GetMapChannel() = 0;

	//! Get the slot that the bitmap was found in.  
	/*! It uses the standard 3ds Max convention ID_BU  for bump etc..  If this is -1 then it means either the hosting material 
	was  not a standard material and the channel conversion could not be performed based on the active shader.
	\return The Slot definition.  Return -1 for an unsupported material.
	*/
	virtual int GetStdMapSlot()=0;

	//! Get the filename of the bitmap used by the Bitmap Texture
	/*!
	\return The name of bitmap file
	*/
	virtual MCHAR * GetBitmapFileName()=0;

	//! Get the Clip U Data from a the Bitmap Texture
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetClipUData()=0;

	//! Get the Clip V Data from a the Bitmap Texture
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetClipVData()=0;

	//! Get the Clip H Data from a the Bitmap Texture
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetClipHData()=0;

	//! Get the Clip W Data from a the Bitmap Texture
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetClipWData()=0;


};

//! class IGameBitmapTex
/*! An IGame wrapper class for basic Bitmap Texture access. Properties such as tiling are also provided
\sa IGameMaterial, IGameProperty
*/
/*
class IGameBitmapTex : public IExportEntity
{
public:

	virtual BitmapTex* GetMaxBitmapTex()=0;
};
*/



//!Simple wrapper for UVGen type data
/*!This is basically a helper class to access some data from the Coordinate Rollout panel.  All data is extracted
from the paramblock, and access is provided by support methods that handle the Property Container Access.  This data
is used to extract the actual Matrix used.  However it can be animated, so using this data the matrix can be recontructed.
\sa IGameBitmapTex, IGameProperty
*/
class IGameUVGen : public IExportEntity
{
public:

	//! Get the U Offset Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetUOffsetData() = 0;

	//! Get the V Offset Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetVOffsetData() = 0;

	//! Get the U Tiling Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetUTilingData() = 0;

	//! Get the V Tiling Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetVTilingData() = 0;

	//! Get the U Angle Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetUAngleData() = 0;
	
	//! Get the V Angle Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetVAngleData() = 0;

	//! Get the W Angle Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetWAngleData() = 0;

	//! Get the actual UV transform.
	/*! The UV transform that is the result of the Coordinate data.  
	\returns A GMatrix representation of the matrix
	*/
	virtual GMatrix GetUVTransform() = 0;

};


