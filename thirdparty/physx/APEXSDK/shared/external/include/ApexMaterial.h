/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include <stdio.h>
#include <string>
#include "PsShare.h"
#include "NxRenderMesh.h"
#include "NxApexMaterialLibrary.h"
#include "foundation/PxVec3.h"
#include <vector>

/**
	A generic texture map.  Loads from a variety of file formats, but is stored in a unified basic format.
	May be (de)serialized from/to an physx::PxFileBuf.
*/
class ApexDefaultTextureMap : public NxApexTextureMap
{
public:
	ApexDefaultTextureMap();
	ApexDefaultTextureMap(const ApexDefaultTextureMap& textureMap)
	{
		*this = textureMap;
	}
	ApexDefaultTextureMap&	operator = (const ApexDefaultTextureMap& textureMap);
	virtual					~ApexDefaultTextureMap();

	void					build(NxApexPixelFormat format, physx::PxU32 width, physx::PxU32 height, physx::PxU32* fillColor = NULL);

	/** Deallocates all buffers and sets all values to the default constructor values. */
	void					unload();

	/** Saves the generic texture data to an physx::PxFileBuf. */
	void					serialize(physx::PxFileBuf& stream) const;

	/** Loads generic texture data from an physx::PxFileBuf. */
	void					deserialize(physx::PxFileBuf& stream, physx::PxU32 version);

	// NxApexTexture API
	NxApexPixelFormat		getPixelFormat() const
	{
		return mPixelFormat;
	}
	physx::PxU32					getWidth() const
	{
		return mWidth;
	}
	physx::PxU32					getHeight() const
	{
		return mHeight;
	}
	physx::PxU32					getComponentCount() const
	{
		return mComponentCount;
	}
	physx::PxU32					getPixelBufferSize() const
	{
		return mPixelBufferSize;
	}
	physx::PxU8*					getPixels() const
	{
		return mPixelBuffer;
	}

protected:

	NxApexPixelFormat	mPixelFormat;
	physx::PxU32				mWidth;
	physx::PxU32				mHeight;
	physx::PxU32				mComponentCount;
	physx::PxU32				mPixelBufferSize;
	physx::PxU8*				mPixelBuffer;
};


class ApexDefaultMaterial : public NxApexMaterial
{
public:

	ApexDefaultMaterial();
	ApexDefaultMaterial(const ApexDefaultMaterial& material)
	{
		*this = material;
	}
	ApexDefaultMaterial&	operator = (const ApexDefaultMaterial& material);
	virtual					~ApexDefaultMaterial();

	/** Sets the name of the material, for lookup by the named resource provider. */
	void					setName(const char* name);

	/** Sets one of the material's texture maps (diffuse or normal) */
	bool					setTextureMap(NxApexTextureMapType type, ApexDefaultTextureMap* textureMap);

	/** Sets the ambient lighting color. */
	void					setAmbient(const physx::PxVec3& ambient)
	{
		mAmbient = ambient;
	}

	/** Sets the diffuse lighting color. */
	void					setDiffuse(const physx::PxVec3& diffuse)
	{
		mDiffuse = diffuse;
	}

	/** Sets the specular lighting color. */
	void					setSpecular(const physx::PxVec3& specular)
	{
		mSpecular = specular;
	}

	/** Sets material's opacity. */
	void					setAlpha(physx::PxF32 alpha)
	{
		mAlpha = alpha;
	}

	/** Sets the material's shininess (specular power). */
	void					setShininess(physx::PxF32 shininess)
	{
		mShininess = shininess;
	}

	/** Deallocates all buffers and sets all values to the default constructor values. */
	void					unload();

	/** Saves the material to an physx::PxFileBuf. */
	void					serialize(physx::PxFileBuf& stream) const;

	/** Loads material from an physx::PxFileBuf. */
	void					deserialize(physx::PxFileBuf& stream, physx::PxU32 version);

	// NxApexMaterial API
	const char*				getName() const
	{
		return mName.c_str();
	}
	NxApexTextureMap*		getTextureMap(NxApexTextureMapType type) const;
	const physx::PxVec3&			getAmbient() const
	{
		return mAmbient;
	}
	const physx::PxVec3&			getDiffuse() const
	{
		return mDiffuse;
	}
	const physx::PxVec3&			getSpecular() const
	{
		return mSpecular;
	}
	physx::PxF32					getAlpha() const
	{
		return mAlpha;
	}
	physx::PxF32					getShininess() const
	{
		return mShininess;
	}

private:

	std::string				mName;

	ApexDefaultTextureMap*	mTextureMaps[TEXTURE_MAP_TYPE_COUNT];

	physx::PxVec3					mAmbient;
	physx::PxVec3					mDiffuse;
	physx::PxVec3					mSpecular;
	physx::PxF32					mAlpha;
	physx::PxF32					mShininess;
};


class ApexDefaultMaterialLibrary : public NxApexMaterialLibrary
{
public:

	ApexDefaultMaterialLibrary();
	ApexDefaultMaterialLibrary(const ApexDefaultMaterialLibrary& materialLibrary)
	{
		*this = materialLibrary;
	}
	ApexDefaultMaterialLibrary&	operator = (const ApexDefaultMaterialLibrary& material);
	virtual						~ApexDefaultMaterialLibrary();

	/** Deallocates all buffers and sets all values to the default constructor values. */
	void						unload();

	/** Returns the number of materials in the library */
	physx::PxU32						getMaterialCount() const
	{
		return (physx::PxU32)mMaterials.size();
	}

	/**
		Access to the materials by index.
		Valid range of materialIndex is 0 to getMaterialCount()-1.
	*/
	ApexDefaultMaterial*		getMaterial(physx::PxU32 materialIndex) const;

	/**
		Remove and delete named material.
		Returns true if the material was found, false if it was not.
	*/
	virtual bool				deleteMaterial(const char* materialName);

	/**
		Adds the materials from the given materialLibrary, which
		aren't already in this material library.  (Based upon name.)
	*/
	void						merge(const ApexDefaultMaterialLibrary& materialLibrary);

	// NxApexMaterialLibrary API

	/** Saves the material to an physx::PxFileBuf. */
	void						serialize(physx::PxFileBuf& stream) const;

	/** Loads material from an physx::PxFileBuf. */
	void						deserialize(physx::PxFileBuf& stream);

	NxApexMaterial*				getMaterial(const char* materialName, bool& created);

	/* Returns -1 if the material is not found */
	physx::PxI32						findMaterialIndex(const char* materialName);

private:

	std::vector<ApexDefaultMaterial*>	mMaterials;
};


#endif // #ifndef _MATERIAL_H_
