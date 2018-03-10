/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#define NOMINMAX
#include <stdio.h>

#include "NxRenderMesh.h"
#include "foundation/PxVec3.h"
#include "PxFileBuf.h"
#include "PsAllocator.h"
#include "PsShare.h"
#include "ApexMaterial.h"

// Local utilities

PX_INLINE bool pathsMatch(const char* pathA, const char* pathB, int length)
{
	while (length-- > 0)
	{
		char a = *pathA++;
		char b = *pathB++;
		if (a == '\\')
		{
			a = '/';
		}
		if (b == '\\')
		{
			b = '/';
		}
		if (a != b)
		{
			return false;
		}
		if (a == '\0')
		{
			return true;
		}
	}

	return true;
}

struct MaterialNameComponents
{
	const char* path;
	int pathLen;
	const char* filename;
	int filenameLen;
	const char* ext;
	int extLen;
	const char* name;
	int nameLen;

	bool	operator <= (const MaterialNameComponents& c) const
	{
		if (pathLen > 0 && !pathsMatch(path, c.path, pathLen))
		{
			return false;
		}
		if (filenameLen > 0 && strncmp(filename, c.filename, (physx::PxU32)filenameLen))
		{
			return false;
		}
		if (extLen > 0 && strncmp(ext, c.ext, (physx::PxU32)extLen))
		{
			return false;
		}
		return 0 == strncmp(name, c.name, (physx::PxU32)nameLen);
	}
};

PX_INLINE void decomposeMaterialName(MaterialNameComponents& components, const char* materialName)
{
	components.path = materialName;
	components.pathLen = 0;
	components.filename = materialName;
	components.filenameLen = 0;
	components.ext = materialName;
	components.extLen = 0;
	components.name = materialName;
	components.nameLen = 0;

	if (materialName == NULL)
	{
		return;
	}

	const int len = (int)strlen(materialName);

	// Get name - will exclude any '#' deliniator
	components.name += len;
	while (components.name > materialName)
	{
		if (*(components.name - 1) == '#')
		{
			break;
		}
		--components.name;
	}
	components.nameLen = len - (int)(components.name - materialName);
	if (components.name == materialName)
	{
		return;
	}

	// Get extension - will include '.'
	components.ext = components.name;
	while (components.ext > materialName)
	{
		if (*(--components.ext) == '.')
		{
			break;
		}
	}
	if (components.ext != materialName)
	{
		components.extLen = (int)(components.name - components.ext) - 1;
	}

	// Get filename
	components.filename = components.ext;
	while (components.filename > materialName)
	{
		if (*(components.filename - 1) == '/' || *(components.filename - 1) == '\\')
		{
			break;
		}
		--components.filename;
	}
	if (components.filename != materialName)
	{
		components.filenameLen = (int)(components.ext - components.filename);
	}

	// Get path
	components.path = materialName;
	components.pathLen = (int)(components.filename - materialName);
}

PX_INLINE void copyToStringBufferSafe(char*& buffer, int& bufferSize, const char* src, int copySize)
{
	if (copySize >= bufferSize)
	{
		copySize = bufferSize - 1;
	}
	memcpy(buffer, src, (physx::PxU32)copySize);
	buffer += copySize;
	bufferSize -= copySize;
}

struct ApexDefaultMaterialLibraryVersion
{
	enum
	{
		Initial = 0,
		AddedBumpMapType,
		AddedMaterialNamingConvention,
		RemovedMaterialNamingConvention,

		Count,
		Current = Count - 1
	};
};


PX_INLINE void serialize_string(physx::PxFileBuf& stream, const std::string& string)
{
	const physx::PxU32 length = (physx::PxU32)string.length();
	stream.storeDword(length);
	stream.write(string.c_str(), length);
}

PX_INLINE void deserialize_string(physx::PxFileBuf& stream, std::string& string)
{
	const physx::PxU32 length = stream.readDword();
	char* cstr = (char*)PxAlloca(length + 1);
	stream.read(cstr, length);
	cstr[length] = '\0';
	string = cstr;
}


// ApexDefaultTextureMap functions

ApexDefaultTextureMap::ApexDefaultTextureMap() :
	mPixelFormat(PIXEL_FORMAT_UNKNOWN),
	mWidth(0),
	mHeight(0),
	mComponentCount(0),
	mPixelBufferSize(0),
	mPixelBuffer(NULL)
{
}

ApexDefaultTextureMap& ApexDefaultTextureMap::operator = (const ApexDefaultTextureMap& textureMap)
{
	mPixelFormat = textureMap.mPixelFormat;
	mWidth = textureMap.mWidth;
	mHeight = textureMap.mHeight;
	mComponentCount = textureMap.mComponentCount;
	mPixelBufferSize = textureMap.mPixelBufferSize;
	mPixelBuffer = new physx::PxU8[mPixelBufferSize];
	memcpy(mPixelBuffer, textureMap.mPixelBuffer, mPixelBufferSize);
	return *this;
}

ApexDefaultTextureMap::~ApexDefaultTextureMap()
{
	unload();
}

void
ApexDefaultTextureMap::build(NxApexPixelFormat format, physx::PxU32 width, physx::PxU32 height, physx::PxU32* fillColor)
{
	physx::PxU8 fillBuffer[4];
	int componentCount;

	switch (format)
	{
	case PIXEL_FORMAT_RGB:
		componentCount = 3;
		if (fillColor != NULL)
		{
			fillBuffer[0] = (*fillColor >> 16) & 0xFF;
			fillBuffer[1] = (*fillColor >> 8) & 0xFF;
			fillBuffer[2] = (*fillColor) & 0xFF;
		}
		break;
	case PIXEL_FORMAT_BGR_EXT:
		componentCount = 3;
		if (fillColor != NULL)
		{
			fillBuffer[0] = (*fillColor) & 0xFF;
			fillBuffer[1] = (*fillColor >> 8) & 0xFF;
			fillBuffer[2] = (*fillColor >> 16) & 0xFF;
		}
		break;
	case PIXEL_FORMAT_BGRA_EXT:
		componentCount = 4;
		if (fillColor != NULL)
		{
			fillBuffer[0] = (*fillColor) & 0xFF;
			fillBuffer[1] = (*fillColor >> 8) & 0xFF;
			fillBuffer[2] = (*fillColor >> 16) & 0xFF;
			fillBuffer[3] = (*fillColor >> 24) & 0xFF;
		}
		break;
	default:
		return;	// Not supported
	}

	unload();

	mPixelBufferSize = componentCount * width * height;
	mPixelBuffer = new physx::PxU8[mPixelBufferSize];
	mPixelFormat = format;
	mComponentCount = (physx::PxU32)componentCount;
	mWidth = width;
	mHeight = height;

	if (fillColor != NULL)
	{
		physx::PxU8* write = mPixelBuffer;
		physx::PxU8* writeStop = mPixelBuffer + mPixelBufferSize;
		physx::PxU8* read = fillBuffer;
		physx::PxU8* readStop = fillBuffer + componentCount;
		while (write < writeStop)
		{
			*write++ = *read++;
			if (read == readStop)
			{
				read = fillBuffer;
			}
		}
	}
}

void ApexDefaultTextureMap::unload()
{
	mPixelFormat = PIXEL_FORMAT_UNKNOWN;
	mWidth = 0;
	mHeight = 0;
	mComponentCount = 0;
	mPixelBufferSize = 0;
	delete [] mPixelBuffer;
	mPixelBuffer = NULL;
}

void ApexDefaultTextureMap::serialize(physx::PxFileBuf& stream) const
{
	stream.storeDword((physx::PxU32)mPixelFormat);
	stream.storeDword(mWidth);
	stream.storeDword(mHeight);
	stream.storeDword(mComponentCount);
	stream.storeDword(mPixelBufferSize);
	if (mPixelBufferSize != 0)
	{
		stream.write(mPixelBuffer, mPixelBufferSize);
	}
}

void ApexDefaultTextureMap::deserialize(physx::PxFileBuf& stream, physx::PxU32 /*version*/)
{
	unload();
	mPixelFormat = (NxApexPixelFormat)stream.readDword();
	mWidth = stream.readDword();
	mHeight = stream.readDword();
	mComponentCount = stream.readDword();
	mPixelBufferSize = stream.readDword();
	if (mPixelBufferSize != 0)
	{
		mPixelBuffer = new physx::PxU8[mPixelBufferSize];
		stream.read(mPixelBuffer, mPixelBufferSize);
	}
}


// ApexDefaultMaterial functions
ApexDefaultMaterial::ApexDefaultMaterial() :
	mAmbient(0.0f),
	mDiffuse(0.0f),
	mSpecular(0.0f),
	mAlpha(0.0f),
	mShininess(0.0f)
{
	for (physx::PxU32 i = 0; i < TEXTURE_MAP_TYPE_COUNT; ++i)
	{
		mTextureMaps[i] = NULL;
	}
}

ApexDefaultMaterial& ApexDefaultMaterial::operator = (const ApexDefaultMaterial& material)
{
	mName = material.mName;

	for (physx::PxU32 i = 0; i < TEXTURE_MAP_TYPE_COUNT; ++i)
	{
		if (material.mTextureMaps[i])
		{
			mTextureMaps[i] = new ApexDefaultTextureMap(*material.mTextureMaps[i]);
		}
		else
		{
			mTextureMaps[i] = NULL;
		}
	}

	mAmbient = material.mAmbient;
	mDiffuse = material.mDiffuse;
	mSpecular = material.mSpecular;
	mAlpha = material.mAlpha;
	mShininess = material.mShininess;

	return *this;
}

ApexDefaultMaterial::~ApexDefaultMaterial()
{
	unload();
}

void ApexDefaultMaterial::setName(const char* name)
{
	mName = name;
}

bool ApexDefaultMaterial::setTextureMap(NxApexTextureMapType type, ApexDefaultTextureMap* textureMap)
{
	if (type < 0 || type >= TEXTURE_MAP_TYPE_COUNT)
	{
		return false;
	}

	delete mTextureMaps[type];

	mTextureMaps[type] = textureMap;

	return true;
}

void ApexDefaultMaterial::unload()
{
	mName.clear();

	for (physx::PxU32 i = 0; i < TEXTURE_MAP_TYPE_COUNT; ++i)
	{
		delete mTextureMaps[i];
		mTextureMaps[i] = NULL;
	}

	mAmbient = physx::PxVec3(0.0f);
	mDiffuse = physx::PxVec3(0.0f);
	mSpecular = physx::PxVec3(0.0f);
	mAlpha = 0.0f;
	mShininess = 0.0f;
}


void ApexDefaultMaterial::serialize(physx::PxFileBuf& stream) const
{
	serialize_string(stream, mName);

	for (physx::PxU32 i = 0; i < TEXTURE_MAP_TYPE_COUNT; ++i)
	{
		if (mTextureMaps[i] == NULL)
		{
			stream.storeDword((physx::PxU32)0);
		}
		else
		{
			stream.storeDword((physx::PxU32)1);
			mTextureMaps[i]->serialize(stream);
		}
	}

	stream.storeFloat(mAmbient.x);
	stream.storeFloat(mAmbient.y);
	stream.storeFloat(mAmbient.z);
	stream.storeFloat(mDiffuse.x);
	stream.storeFloat(mDiffuse.y);
	stream.storeFloat(mDiffuse.z);
	stream.storeFloat(mSpecular.x);
	stream.storeFloat(mSpecular.y);
	stream.storeFloat(mSpecular.z);
	stream.storeFloat(mAlpha);
	stream.storeFloat(mShininess);
}

void ApexDefaultMaterial::deserialize(physx::PxFileBuf& stream, physx::PxU32 version)
{
	unload();

	deserialize_string(stream, mName);

	if (version < ApexDefaultMaterialLibraryVersion::AddedBumpMapType)
	{
		for (physx::PxU32 i = 0; i < 2; ++i)
		{
			const physx::PxU32 pointerIsValid = stream.readDword();
			if (pointerIsValid)
			{
				mTextureMaps[i] = new ApexDefaultTextureMap();
				mTextureMaps[i]->deserialize(stream, version);
			}
		}
		mTextureMaps[2] = mTextureMaps[1];
		mTextureMaps[1] = NULL;
	}
	else
	{
		for (physx::PxU32 i = 0; i < TEXTURE_MAP_TYPE_COUNT; ++i)
		{
			const physx::PxU32 pointerIsValid = stream.readDword();
			if (pointerIsValid)
			{
				mTextureMaps[i] = new ApexDefaultTextureMap();
				mTextureMaps[i]->deserialize(stream, version);
			}
		}
	}

	mAmbient.x = stream.readFloat();
	mAmbient.y = stream.readFloat();
	mAmbient.z = stream.readFloat();
	mDiffuse.x = stream.readFloat();
	mDiffuse.y = stream.readFloat();
	mDiffuse.z = stream.readFloat();
	mSpecular.x = stream.readFloat();
	mSpecular.y = stream.readFloat();
	mSpecular.z = stream.readFloat();
	mAlpha = stream.readFloat();
	mShininess = stream.readFloat();
}

NxApexTextureMap* ApexDefaultMaterial::getTextureMap(NxApexTextureMapType type) const
{
	if (type < 0 || type >= TEXTURE_MAP_TYPE_COUNT)
	{
		return NULL;
	}

	return mTextureMaps[type];
}


// ApexDefaultMaterialLibrary functions
ApexDefaultMaterialLibrary::ApexDefaultMaterialLibrary()
{
}

ApexDefaultMaterialLibrary& ApexDefaultMaterialLibrary::operator = (const ApexDefaultMaterialLibrary& materialLibrary)
{
	mMaterials.resize(materialLibrary.getMaterialCount());
	for (physx::PxU32 i = 0; i < materialLibrary.getMaterialCount(); ++i)
	{
		ApexDefaultMaterial* material = materialLibrary.getMaterial(i);
		PX_ASSERT(material != NULL);
		mMaterials[i] = new ApexDefaultMaterial(*material);
	}

	return *this;
}

ApexDefaultMaterialLibrary::~ApexDefaultMaterialLibrary()
{
	unload();
}

void ApexDefaultMaterialLibrary::unload()
{
	const physx::PxU32 size = (physx::PxU32)mMaterials.size();
	for (physx::PxU32 i = 0; i < size; ++i)
	{
		delete mMaterials[i];
		mMaterials[i] = NULL;
	}
	mMaterials.resize(0);
}

ApexDefaultMaterial* ApexDefaultMaterialLibrary::getMaterial(physx::PxU32 materialIndex) const
{
	if (materialIndex >= mMaterials.size())
	{
		return NULL;
	}

	return mMaterials[materialIndex];
}

void ApexDefaultMaterialLibrary::merge(const ApexDefaultMaterialLibrary& materialLibrary)
{
	for (physx::PxU32 i = 0; i < materialLibrary.getMaterialCount(); ++i)
	{
		ApexDefaultMaterial* material = materialLibrary.getMaterial(i);
		PX_ASSERT(material != NULL);
		const physx::PxU32 size = (physx::PxU32)mMaterials.size();
		physx::PxU32 j = 0;
		for (; j < size; ++j)
		{
			if (!strcmp(mMaterials[j]->getName(), material->getName()))
			{
				break;
			}
		}
		if (j == size)
		{
			ApexDefaultMaterial* newMaterial = new ApexDefaultMaterial(*material);
			mMaterials.push_back(newMaterial);
		}
	}
}

void ApexDefaultMaterialLibrary::serialize(physx::PxFileBuf& stream) const
{
	stream.storeDword((physx::PxU32)ApexDefaultMaterialLibraryVersion::Current);

	const physx::PxU32 size = (physx::PxU32)mMaterials.size();
	stream.storeDword(size);
	for (physx::PxU32 i = 0; i < size; ++i)
	{
		mMaterials[i]->serialize(stream);
	}
}

void ApexDefaultMaterialLibrary::deserialize(physx::PxFileBuf& stream)
{
	unload();

	physx::PxU32 version = stream.readDword();

	physx::PxU32 size = stream.readDword();
	mMaterials.resize(size);
	for (physx::PxU32 i = 0; i < size; ++i)
	{
		mMaterials[i] = new ApexDefaultMaterial();
		mMaterials[i]->deserialize(stream, version);
	}

	if (version >= ApexDefaultMaterialLibraryVersion::AddedMaterialNamingConvention  && version < ApexDefaultMaterialLibraryVersion::RemovedMaterialNamingConvention)
	{
		stream.readDword();	// Eat naming convention
	}
}

NxApexMaterial* ApexDefaultMaterialLibrary::getMaterial(const char* materialName, bool& created)
{
	physx::PxI32 index = findMaterialIndex(materialName);
	if (index >= 0)
	{
		created = false;
		return mMaterials[(physx::PxU32)index];
	}

	ApexDefaultMaterial* newMaterial = new ApexDefaultMaterial();
	newMaterial->setName(materialName);
	mMaterials.push_back(newMaterial);
	created = true;
	return newMaterial;
}

bool ApexDefaultMaterialLibrary::deleteMaterial(const char* materialName)
{
	physx::PxI32 index = findMaterialIndex(materialName);
	if (index < 0)
	{
		return false;
	}

	ApexDefaultMaterial* material = mMaterials[(physx::PxU32)index];
	delete material;

	mMaterials[(physx::PxU32)index] = mMaterials[mMaterials.size() - 1];
	mMaterials.resize(mMaterials.size() - 1);

	return true;
}

physx::PxI32 ApexDefaultMaterialLibrary::findMaterialIndex(const char* materialName)
{
	const char blank[] = "";
	materialName = materialName ? materialName : blank;

	const physx::PxU32 size = (physx::PxU32)mMaterials.size();
	physx::PxI32 index = 0;
	for (; index < (physx::PxI32)size; ++index)
	{
		const char* existingMaterialName = mMaterials[(physx::PxU32)index]->getName() ? mMaterials[(physx::PxU32)index]->getName() : blank;
		if (!strcmp(existingMaterialName, materialName))
		{
			return index;
		}
	}

	return -1;
}
