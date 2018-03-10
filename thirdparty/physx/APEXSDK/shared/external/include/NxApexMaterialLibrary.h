/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef _NX_APEX_MATERIAL_LIBRARY_H_
#define _NX_APEX_MATERIAL_LIBRARY_H_

#include "PsShare.h"
#include "foundation/PxVec3.h"

/**
	Texture map types.  Currently the NxApexMaterialLibrary interface only supports one map of each type.
*/
enum NxApexTextureMapType
{
	DIFFUSE_MAP = 0,
	BUMP_MAP,
	NORMAL_MAP,

	TEXTURE_MAP_TYPE_COUNT
};


/**
	Texture pixel format types, a supported by the Destruction tool's bmp, dds, and tga texture loaders.
*/
enum NxApexPixelFormat
{
	PIXEL_FORMAT_UNKNOWN,

	PIXEL_FORMAT_RGB,
	PIXEL_FORMAT_BGR_EXT,
	PIXEL_FORMAT_BGRA_EXT,
	PIXEL_FORMAT_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	PIXEL_FORMAT_COMPRESSED_RGBA_S3TC_DXT3_EXT,
	PIXEL_FORMAT_COMPRESSED_RGBA_S3TC_DXT5_EXT,
};


/**
	Basic interface to query generic texture map data.
*/
class NxApexTextureMap
{
public:

	/** The texture pixel format, supported by the Destruction tool's texture loaders. */
	virtual NxApexPixelFormat	getPixelFormat() const = 0;

	/** The horizontal texture map size, in pixels. */
	virtual physx::PxU32				getWidth() const = 0;

	/** The vertical texture map size, in pixels. */
	virtual physx::PxU32				getHeight() const = 0;

	/** The number of color/alpha/etc. channels. */
	virtual physx::PxU32				getComponentCount() const = 0;

	/** The size, in bytes, of the pixel buffer. */
	virtual physx::PxU32				getPixelBufferSize() const = 0;

	/** The beginning address of the pixel buffer. */
	virtual physx::PxU8*				getPixels() const = 0;
};


/**
	Basic interface to query generic render material data.
*/
class NxApexMaterial
{
public:

	/** The material's name, used by the named resource provider. */
	virtual const char*			getName() const = 0;

	/** Access to the material's texture maps, indexed by NxApexTextureMapType. */
	virtual NxApexTextureMap*	getTextureMap(NxApexTextureMapType type) const = 0;

	/** The ambient lighting color of the material. */
	virtual const physx::PxVec3&		getAmbient() const = 0;

	/** The diffuse lighting color of the material. */
	virtual const physx::PxVec3&		getDiffuse() const = 0;

	/** The specular lighting color of the material. */
	virtual const physx::PxVec3&		getSpecular() const = 0;

	/** The opacity of the material. */
	virtual physx::PxF32				getAlpha() const = 0;

	/** The shininess (specular power) of the material. */
	virtual physx::PxF32				getShininess() const = 0;
};


/**
	Material library skeleton interface.
*/
class NxApexMaterialLibrary
{
public:

	/** Saves the material to an physx::PxFileBuf. */
	virtual void			serialize(physx::PxFileBuf& stream) const = 0;

	/** Loads material from an physx::PxFileBuf. */
	virtual void			deserialize(physx::PxFileBuf& stream) = 0;

	/**
		Query a material by name.
		If the material already exists, it is returned and 'created' is set to false.
		If the material did not exist before, it is created, returned, and 'created' is set to true.
	*/
	virtual NxApexMaterial*	getMaterial(const char* materialName, bool& created) = 0;

	/**
		Remove and delete named material.
		Returns true if the material was found, false if it was not.
	*/
	virtual bool			deleteMaterial(const char* materialName) = 0;
};


#endif // #ifndef _NX_APEX_MATERIAL_LIBRARY_H_
