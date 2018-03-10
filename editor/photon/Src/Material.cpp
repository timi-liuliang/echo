/*=============================================================================
	Material.cpp: Material classes implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "stdafx.h"
#include "Material.h"
#include "Importer.h"

namespace Lightmass
{
	//----------------------------------------------------------------------------
	//	Material base class
	//----------------------------------------------------------------------------
	void FBaseMaterial::Read( FILE* FileHandle, FScene& Scene)
	{
		fread( (FBaseMaterialData*)this, sizeof(FBaseMaterialData), 1, FileHandle);
	}

	void FBaseMaterial::Import( class FLightmassImporter& Importer )
	{
		verify(Importer.ImportData((FBaseMaterialData*)this));
	}

	//----------------------------------------------------------------------------
	//	Material class
	//----------------------------------------------------------------------------
	void FMaterial::Read( FILE* FileHandle, FScene& Scene)
	{
		FBaseMaterial::Read( FileHandle, Scene);

		// read the shared data structure
		fread( (FMaterialData*)this, sizeof(FMaterialData), 1, FileHandle);

		// import the actual material samples...
		INT ReadSize;

		// Transmission
		ReadSize = TransmissionSize * TransmissionSize;
		if (ReadSize > 0)
		{
			MaterialTransmission.Init(TF_ARGB16F, TransmissionSize, TransmissionSize);
			fread( MaterialTransmission.GetData(), ReadSize * sizeof(FFloat16Color), 1, FileHandle);
		}

		// Emissive
		ReadSize = EmissiveSize * EmissiveSize;
		if (ReadSize > 0)
		{
			MaterialEmissive.Init(TF_ARGB16F, EmissiveSize, EmissiveSize);
			fread(MaterialEmissive.GetData(), ReadSize * sizeof(FFloat16Color), 1, FileHandle);
		}
		
		// Diffuse
		ReadSize = DiffuseSize * DiffuseSize;
		if (ReadSize > 0)
		{
			MaterialDiffuse.Init(TF_ARGB16F, DiffuseSize, DiffuseSize);
			fread(MaterialDiffuse.GetData(), ReadSize * sizeof(FFloat16Color), 1, FileHandle);
		}
		else
		{
			// Opaque materials should always import diffuse
			// check(BlendMode != BLEND_Opaque && BlendMode != BLEND_Masked && BlendMode != BLEND_SoftMasked);
		}

		// Normal
		ReadSize = NormalSize * NormalSize;
		if (ReadSize > 0)
		{
			MaterialNormal.Init(TF_ARGB16F, NormalSize, NormalSize);
			fread(MaterialNormal.GetData(), ReadSize * sizeof(FFloat16Color), 1, FileHandle);
		}
	}

	void FMaterial::Import( class FLightmassImporter& Importer )
	{
		// import super class
		FBaseMaterial::Import(Importer);

		// import the shared data structure
		verify(Importer.ImportData((FMaterialData*)this));

		// import the actual material samples...
		INT ReadSize;

		// Emissive
		ReadSize = EmissiveSize * EmissiveSize;
		checkf(ReadSize > 0, TEXT("Failed to import emissive data!"));
		MaterialEmissive.Init(TF_ARGB16F, EmissiveSize, EmissiveSize);
		verify(Importer.Read(MaterialEmissive.GetData(), ReadSize * sizeof(FFloat16Color)));

		// Diffuse
		ReadSize = DiffuseSize * DiffuseSize;
		if (ReadSize > 0)
		{
			MaterialDiffuse.Init(TF_ARGB16F, DiffuseSize, DiffuseSize);
			verify(Importer.Read(MaterialDiffuse.GetData(), ReadSize * sizeof(FFloat16Color)));
		}
		else
		{
			// Opaque materials should always import diffuse
			check(BlendMode != BLEND_Opaque && BlendMode != BLEND_Masked && BlendMode != BLEND_SoftMasked);
		}
		// Specular
		ReadSize = SpecularSize * SpecularSize;
		if (ReadSize > 0)
		{
			MaterialSpecular.Init(TF_ARGB16F, SpecularSize, SpecularSize);
			verify(Importer.Read(MaterialSpecular.GetData(), ReadSize * sizeof(FFloat16Color)));
		}
		else
		{
			// Opaque materials should always import specular
			check(BlendMode != BLEND_Opaque && BlendMode != BLEND_Masked && BlendMode != BLEND_SoftMasked);
		}
		// Transmission
		ReadSize = TransmissionSize * TransmissionSize;
		if (ReadSize > 0)
		{
			MaterialTransmission.Init(TF_ARGB16F, TransmissionSize, TransmissionSize);
			verify(Importer.Read(MaterialTransmission.GetData(), ReadSize * sizeof(FFloat16Color)));
		}
		else
		{
			// Materials with a translucent blend mode should always import transmission
			check((BlendMode != BLEND_Translucent) && (BlendMode != BLEND_Additive) && (BlendMode != BLEND_Modulate) && (BlendMode != BLEND_ModulateAndAdd) && (BlendMode != BLEND_DitheredTranslucent));
		}

		// Normal
		ReadSize = NormalSize * NormalSize;
		if( ReadSize > 0 )
		{
			MaterialNormal.Init(TF_ARGB16F, NormalSize, NormalSize);
			verify(Importer.Read(MaterialNormal.GetData(), ReadSize * sizeof(FFloat16Color)));
		}
	}

}	// namespace Lightmass


