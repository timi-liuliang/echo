/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "ApexVertexFormat.h"
#include "NiApexSDK.h"

#include <NxParamArray.h>

namespace physx
{
namespace apex
{

// Local functions and definitions

PX_INLINE char* apex_strdup(const char* input)
{
	if (input == NULL)
	{
		return NULL;
	}

	size_t len = strlen(input);

	char* result = (char*)PX_ALLOC(sizeof(char) * (len + 1), PX_DEBUG_EXP("apex_strdup"));
#ifdef WIN32
	strncpy_s(result, len + 1, input, len);
#else
	strncpy(result, input, len);
#endif

	return result;
}

PX_INLINE physx::PxU32 hash(const char* string)
{
	// "DJB" string hash
	physx::PxU32 h = 5381;
	char c;
	while ((c = *string++) != '\0')
	{
		h = ((h << 5) + h) ^ c;
	}
	return h;
}

struct SemanticNameAndID
{
	SemanticNameAndID(const char* name, NxVertexFormat::BufferID id) : m_name(name), m_id(id)
	{
		PX_ASSERT(m_id != 0 || strcmp(m_name, "SEMANTIC_INVALID") == 0);
	}
	const char*					m_name;
	NxVertexFormat::BufferID	m_id;
};

#define SEMANTIC_NAME_AND_ID( name )	SemanticNameAndID( name, (NxVertexFormat::BufferID)hash( name ) )

static const SemanticNameAndID sSemanticNamesAndIDs[] =
{
	SEMANTIC_NAME_AND_ID("SEMANTIC_POSITION"),
	SEMANTIC_NAME_AND_ID("SEMANTIC_NORMAL"),
	SEMANTIC_NAME_AND_ID("SEMANTIC_TANGENT"),
	SEMANTIC_NAME_AND_ID("SEMANTIC_BINORMAL"),
	SEMANTIC_NAME_AND_ID("SEMANTIC_COLOR"),
	SEMANTIC_NAME_AND_ID("SEMANTIC_TEXCOORD0"),
	SEMANTIC_NAME_AND_ID("SEMANTIC_TEXCOORD1"),
	SEMANTIC_NAME_AND_ID("SEMANTIC_TEXCOORD2"),
	SEMANTIC_NAME_AND_ID("SEMANTIC_TEXCOORD3"),
	SEMANTIC_NAME_AND_ID("SEMANTIC_BONE_INDEX"),
	SEMANTIC_NAME_AND_ID("SEMANTIC_BONE_WEIGHT"),
	SEMANTIC_NAME_AND_ID("SEMANTIC_DISPLACEMENT_TEXCOORD"),
	SEMANTIC_NAME_AND_ID("SEMANTIC_DISPLACEMENT_FLAGS"),

	SemanticNameAndID("SEMANTIC_INVALID", (NxVertexFormat::BufferID)0)
};


// NxVertexFormat implementation
void ApexVertexFormat::reset()
{
	if (mParams != NULL)
	{
		mParams->winding = 0;
		mParams->hasSeparateBoneBuffer = 0;
	}
	clearBuffers();
}

void ApexVertexFormat::setWinding(NxRenderCullMode::Enum winding)
{
	mParams->winding = winding;
}

void ApexVertexFormat::setHasSeparateBoneBuffer(bool hasSeparateBoneBuffer)
{
	mParams->hasSeparateBoneBuffer = hasSeparateBoneBuffer;
}

NxRenderCullMode::Enum ApexVertexFormat::getWinding() const
{
	return (NxRenderCullMode::Enum)mParams->winding;
}

bool ApexVertexFormat::hasSeparateBoneBuffer() const
{
	return mParams->hasSeparateBoneBuffer;
}

const char* ApexVertexFormat::getSemanticName(NxRenderVertexSemantic::Enum semantic) const
{
	PX_ASSERT((physx::PxU32)semantic < NxRenderVertexSemantic::NUM_SEMANTICS);
	return (physx::PxU32)semantic < NxRenderVertexSemantic::NUM_SEMANTICS ? sSemanticNamesAndIDs[semantic].m_name : NULL;
}

NxVertexFormat::BufferID ApexVertexFormat::getSemanticID(NxRenderVertexSemantic::Enum semantic) const
{
	PX_ASSERT((physx::PxU32)semantic < NxRenderVertexSemantic::NUM_SEMANTICS);
	return (physx::PxU32)semantic < NxRenderVertexSemantic::NUM_SEMANTICS ? sSemanticNamesAndIDs[semantic].m_id : (BufferID)0;
}

NxVertexFormat::BufferID ApexVertexFormat::getID(const char* name) const
{
	if (name == NULL)
	{
		return (BufferID)0;
	}
	const BufferID id = hash(name);
	return id ? id : (BufferID)1;	// We reserve 0 for an invalid ID
}

physx::PxI32 ApexVertexFormat::addBuffer(const char* name)
{
	if (name == NULL)
	{
		return -1;
	}

	const BufferID id = getID(name);

	physx::PxI32 index = getBufferIndexFromID(id);
	if (index >= 0)
	{
		return index;
	}

	physx::PxI32 semantic = 0;
	for (; semantic < NxRenderVertexSemantic::NUM_SEMANTICS; ++semantic)
	{
		if (getSemanticID((NxRenderVertexSemantic::Enum)semantic) == id)
		{
			break;
		}
	}
	if (semantic == NxRenderVertexSemantic::NUM_SEMANTICS)
	{
		semantic = NxRenderVertexSemantic::CUSTOM;
	}

	NxParameterized::Handle handle(*mParams);
	mParams->getParameterHandle("bufferFormats", handle);

	mParams->getArraySize(handle, index);

	mParams->resizeArray(handle, index + 1);

	NxParameterized::Handle elementHandle(*mParams);
	handle.getChildHandle(index, elementHandle);
	NxParameterized::Handle subElementHandle(*mParams);
	elementHandle.getChildHandle(mParams, "name", subElementHandle);
	mParams->setParamString(subElementHandle, name);
	elementHandle.getChildHandle(mParams, "semantic", subElementHandle);
	mParams->setParamI32(subElementHandle, semantic);
	elementHandle.getChildHandle(mParams, "id", subElementHandle);
	mParams->setParamU32(subElementHandle, (physx::PxU32)id);
	elementHandle.getChildHandle(mParams, "format", subElementHandle);
	mParams->setParamU32(subElementHandle, (physx::PxU32)NxRenderDataFormat::UNSPECIFIED);
	elementHandle.getChildHandle(mParams, "access", subElementHandle);
	mParams->setParamU32(subElementHandle, (physx::PxU32)NxRenderDataAccess::STATIC);
	elementHandle.getChildHandle(mParams, "serialize", subElementHandle);
	mParams->setParamBool(subElementHandle, true);

	return index;
}

bool ApexVertexFormat::bufferReplaceWithLast(physx::PxU32 index)
{
	PX_ASSERT((physx::PxI32)index < mParams->bufferFormats.arraySizes[0]);
	if ((physx::PxI32)index < mParams->bufferFormats.arraySizes[0])
	{
		NxParamArray<VertexFormatParametersNS::BufferFormat_Type> bufferFormats(mParams, "bufferFormats", reinterpret_cast<NxParamDynamicArrayStruct*>(&mParams->bufferFormats));
		bufferFormats.replaceWithLast(index);
		return true;
	}

	return false;
}

bool ApexVertexFormat::setBufferFormat(physx::PxU32 index, NxRenderDataFormat::Enum format)
{
	if (index < getBufferCount())
	{
		mParams->bufferFormats.buf[index].format = format;
		return true;
	}

	return false;
}

bool ApexVertexFormat::setBufferAccess(physx::PxU32 index, NxRenderDataAccess::Enum access)
{
	if (index < getBufferCount())
	{
		mParams->bufferFormats.buf[index].access = access;
		return true;
	}

	return false;
}

bool ApexVertexFormat::setBufferSerialize(physx::PxU32 index, bool serialize)
{
	if (index < getBufferCount())
	{
		mParams->bufferFormats.buf[index].serialize = serialize;
		return true;
	}

	return false;
}

const char* ApexVertexFormat::getBufferName(physx::PxU32 index) const
{
	return index < getBufferCount() ? (const char*)mParams->bufferFormats.buf[index].name : NULL;
}

NxRenderVertexSemantic::Enum ApexVertexFormat::getBufferSemantic(physx::PxU32 index) const
{
	return index < getBufferCount() ? (NxRenderVertexSemantic::Enum)mParams->bufferFormats.buf[index].semantic : NxRenderVertexSemantic::NUM_SEMANTICS;
}

NxVertexFormat::BufferID ApexVertexFormat::getBufferID(physx::PxU32 index) const
{
	return index < getBufferCount() ? (BufferID)mParams->bufferFormats.buf[index].id : (BufferID)0;
}

NxRenderDataFormat::Enum ApexVertexFormat::getBufferFormat(physx::PxU32 index) const
{
	return index < getBufferCount() ? (NxRenderDataFormat::Enum)mParams->bufferFormats.buf[index].format : NxRenderDataFormat::UNSPECIFIED;
}

NxRenderDataAccess::Enum ApexVertexFormat::getBufferAccess(physx::PxU32 index) const
{
	return index < getBufferCount() ? (NxRenderDataAccess::Enum)mParams->bufferFormats.buf[index].access : NxRenderDataAccess::ACCESS_TYPE_COUNT;
}

bool ApexVertexFormat::getBufferSerialize(physx::PxU32 index) const
{
	return index < getBufferCount() ? mParams->bufferFormats.buf[index].serialize : false;
}

physx::PxU32 ApexVertexFormat::getBufferCount() const
{
	return (physx::PxU32)mParams->bufferFormats.arraySizes[0];
}

physx::PxU32 ApexVertexFormat::getCustomBufferCount() const
{
	PX_ASSERT(mParams != NULL);
	PxU32 customBufferCount = 0;
	for (PxI32 i = 0; i < mParams->bufferFormats.arraySizes[0]; ++i)
	{
		if (mParams->bufferFormats.buf[i].semantic == NxRenderVertexSemantic::CUSTOM)
		{
			++customBufferCount;
		}
	}
	return customBufferCount;
}

physx::PxI32 ApexVertexFormat::getBufferIndexFromID(BufferID id) const
{
	for (physx::PxI32 i = 0; i < mParams->bufferFormats.arraySizes[0]; ++i)
	{
		if (mParams->bufferFormats.buf[i].id == (physx::PxU32)id)
		{
			return i;
		}
	}

	return -1;
}



// ApexVertexFormat functions

ApexVertexFormat::ApexVertexFormat()
{
	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	mParams = DYNAMIC_CAST(VertexFormatParameters*)(traits->createNxParameterized(VertexFormatParameters::staticClassName()));
	mOwnsParams = mParams != NULL;
}

ApexVertexFormat::ApexVertexFormat(VertexFormatParameters* params) : mParams(params), mOwnsParams(false)
{
}

ApexVertexFormat::ApexVertexFormat(const ApexVertexFormat& f) : NxVertexFormat(f)
{
	NxParameterized::Traits* traits = NiGetApexSDK()->getParameterizedTraits();
	mParams = DYNAMIC_CAST(VertexFormatParameters*)(traits->createNxParameterized(VertexFormatParameters::staticClassName()));
	mOwnsParams = mParams != NULL;
	if (mParams)
	{
		copy(f);
	}
}

ApexVertexFormat::~ApexVertexFormat()
{
	if (mOwnsParams && mParams != NULL)
	{
		mParams->destroy();
	}
}

bool ApexVertexFormat::operator == (const NxVertexFormat& format) const
{
	if (getWinding() != format.getWinding())
	{
		return false;
	}

	if (hasSeparateBoneBuffer() != format.hasSeparateBoneBuffer())
	{
		return false;
	}

	if (getBufferCount() != format.getBufferCount())
	{
		return false;
	}

	for (physx::PxU32 thisIndex = 0; thisIndex < getBufferCount(); ++thisIndex)
	{
		BufferID id = getBufferID(thisIndex);
		const physx::PxI32 thatIndex = format.getBufferIndexFromID(id);
		if (thatIndex < 0)
		{
			return false;
		}
		if (getBufferFormat(thisIndex) != format.getBufferFormat((physx::PxU32)thatIndex))
		{
			return false;
		}
		if (getBufferAccess(thisIndex) != format.getBufferAccess((physx::PxU32)thatIndex))
		{
			return false;
		}
	}

	return true;
}

void ApexVertexFormat::copy(const ApexVertexFormat& other)
{
	reset();

	setWinding(other.getWinding());
	setHasSeparateBoneBuffer(other.hasSeparateBoneBuffer());

	for (physx::PxU32 i = 0; i < other.getBufferCount(); ++i)
	{
		const char* name = other.getBufferName(i);
		const physx::PxU32 index = (physx::PxU32)addBuffer(name);
		setBufferFormat(index, other.getBufferFormat(i));
		setBufferAccess(index, other.getBufferAccess(i));
		setBufferSerialize(index, other.getBufferSerialize(i));
	}
}

void ApexVertexFormat::clearBuffers()
{
	if (mParams)
	{
		NxParameterized::Handle handle(*mParams);

		mParams->getParameterHandle("bufferFormats", handle);
		handle.resizeArray(0);
	}
}


}
} // end namespace physx::apex
