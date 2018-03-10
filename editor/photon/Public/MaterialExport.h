/*=============================================================================
	MaterialExport.h: Material export data definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

#pragma pack(push, 1)

//----------------------------------------------------------------------------
//	Helper definitions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//	Mesh export file header
//----------------------------------------------------------------------------
struct FMaterialFileHeader
{
	/** FourCC cookie: 'MTRL' */
	UINT		Cookie;
	FGuid		FormatVersion;
};

//----------------------------------------------------------------------------
//	Base material
//----------------------------------------------------------------------------
struct FBaseMaterialData
{
	FGuid		Guid;
};

//----------------------------------------------------------------------------
//	Material data, builds upon FBaseMaterialData
//----------------------------------------------------------------------------
/**
 *	Material blend mode.
 *	MUST MATCH UE3 EXACTLY!!!
 */
enum EBlendMode
{
    BLEND_Opaque            =0,
    BLEND_Masked            =1,
    BLEND_Translucent       =2,
    BLEND_Additive          =3,
    BLEND_Modulate          =4,
	BLEND_ModulateAndAdd    =5,
    BLEND_SoftMasked        =6,
    BLEND_AlphaComposite    =7,
	BLEND_DitheredTranslucent=8,
    BLEND_MAX               =9,
};

struct FMaterialData
{
	/** The BLEND mode of the material */
	EBlendMode BlendMode;
	/** Whether the material is two-sided or not */
	BITFIELD bTwoSided:1;
	/** Whether the material should cast shadows as masked even though it has a translucent blend mode. */
	BITFIELD bCastShadowAsMasked:1;
	/** Scales the emissive contribution for this material. */
	FLOAT EmissiveBoost;
	/** Scales the diffuse contribution for this material. */
	FLOAT DiffuseBoost;
	/** Scales the specular contribution for this material. */
	FLOAT SpecularBoost;
	/** The clip value for masked rendering */
	FLOAT OpacityMaskClipValue;

	/** 
	 *	The sizes of the material property samples
	 */
	INT EmissiveSize;
	INT DiffuseSize;
	INT SpecularSize;
	INT TransmissionSize;
	INT NormalSize;

	FMaterialData() : 
		  EmissiveBoost(1.0f)
		, DiffuseBoost(1.0f)
		, SpecularBoost(1.0f)
		, EmissiveSize(0)
		, DiffuseSize(0)
		, SpecularSize(0)
		, TransmissionSize(0)
		, NormalSize(0)
	{
	}
};

#pragma pack(pop)

}	// namespace Lightmass
