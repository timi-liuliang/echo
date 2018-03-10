/*=============================================================================
	Material.h: Material classes.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#pragma once

// @lmtodo: This seems wrong to me to be needed (scene uses mesh, not the other way around), and is only needed for the MAX_TEXCOORDS
#include "..\Public\MaterialExport.h"
#include "..\Public\SceneExport.h"

namespace Lightmass
{
	//----------------------------------------------------------------------------
	//	Material base class
	//----------------------------------------------------------------------------

	class FBaseMaterial : public FBaseMaterialData
	{
	public:
		virtual void    Read( FILE* FileHandle, FScene& Scene);
		virtual void	Import( class FLightmassImporter& Importer );
	};


	//----------------------------------------------------------------------------
	//	Material class
	//----------------------------------------------------------------------------
	class FMaterial : public FBaseMaterial, public FMaterialData
	{
	public:
		virtual void    Read( FILE* FileHandle, FScene& Scene);
		virtual void	Import( class FLightmassImporter& Importer );

		/** Sample the various properties of this material at the given UV */
		inline void SampleEmissive(const FVector2D& UV, FLinearColor& Emissive, FLOAT& OutEmissiveBoost) const
		{
			Emissive = MaterialEmissive.Sample(UV);
			OutEmissiveBoost = EmissiveBoost;
		}
		inline void SampleDiffuse(const FVector2D& UV, FLinearColor& Diffuse, FLOAT& OutDiffuseBoost) const
		{
			Diffuse = MaterialDiffuse.Sample(UV);
			OutDiffuseBoost = DiffuseBoost;
		}
		inline void SampleSpecularAndPower(const FVector2D& UV, FLinearColor& SpecularAndPower, FLOAT& OutSpecularBoost) const
		{
			SpecularAndPower = MaterialSpecular.Sample(UV);
			OutSpecularBoost = SpecularBoost;
		}
		inline FLinearColor SampleTransmission(const FVector2D& UV) const
		{
			return MaterialTransmission.Sample(UV);
		}
		inline void SampleNormal(const FVector2D& UV, FVector4& Normal) const
		{
			Normal = MaterialNormal.SampleNormal(UV);
			Normal.W = 0.0f;
			Normal = Normal.SafeNormal();
			if( Normal.SizeSquared() < KINDA_SMALL_NUMBER )
			{
				Normal.Set( 0.0f, 0.0f, 1.0f, 0.0f );
			}
		}

	protected:
		FTexture2D MaterialEmissive;
		FTexture2D MaterialDiffuse;
		FTexture2D MaterialSpecular;
		FTexture2D MaterialTransmission;
		FTexture2D MaterialNormal;
	};

}	// namespace Lightmass


