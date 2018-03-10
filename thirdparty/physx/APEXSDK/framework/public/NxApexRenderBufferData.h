/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_RENDER_BUFFER_DATA_H
#define NX_APEX_RENDER_BUFFER_DATA_H

/*!
\file
\brief classes NxApexRenderSemanticData, NxApexRenderBufferData, NxApexModuleSpecificRenderBufferData
*/

#include "NxApexUsingNamespace.h"
#include "NxApexRenderDataFormat.h"
#include "NxApexSDK.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

/**
\brief Describes the data location and layout of a single semantic
*/
class NxApexRenderSemanticData
{
public:
	const void*					data;	//!< Pointer to first semantic data element
	physx::PxU32				stride;	//!< Semantic stride, in bytes
	void*						ident;	//!< Used for custom semantics only: value retrieved from NRP
	NxRenderDataFormat::Enum	format;	//!< Format used for this semantic
	/**
	\brief When this class is used for mesh authoring, this is the format of the source (user's) buffer.
		If different from 'format', and not NxRenderDataFormat::UNSPECIFIED, the data will be converted to
		from srcFormat to format (if a converter exists).
		If srcFormat = NxRenderDataFormat::UNSPECIFIED (default value), it is assumed that srcFormat = format.
	*/
	NxRenderDataFormat::Enum	srcFormat;
	/**
	\brief Copy this buffer from the authoring asset to the asset.

	This will only be used at render mesh asset creation time.
	*/
	bool						serialize;
public:
	PX_INLINE NxApexRenderSemanticData(void)
	{
		data   = 0;
		stride = 0;
		ident  = 0;
		format = NxRenderDataFormat::UNSPECIFIED;
		srcFormat = NxRenderDataFormat::UNSPECIFIED;
		serialize = false;
	}
};

/**
\brief Describes all of the semantics of a render buffer
*/
template < class SemanticClass, class SemanticEnum>
class NxApexRenderBufferData
{
public:
	PX_INLINE NxApexRenderBufferData(void)
	{
		m_numCustomSemantics = 0;
		m_customSemanticData = 0;
	}

	/**
	\brief Returns the properties of a base semantic
	*/
	PX_INLINE const NxApexRenderSemanticData& getSemanticData(SemanticEnum semantic) const
	{
		PX_ASSERT(semantic < SemanticClass::NUM_SEMANTICS);
		return m_semanticData[semantic];
	}

	/**
	\brief Returns the number of custom semantics
	*/
	PX_INLINE physx::PxU32 getNumCustomSemantics() const
	{
		return m_numCustomSemantics;
	}

	/**
	\brief Returns the properties of a custom semantic
	*/
	PX_INLINE const NxApexRenderSemanticData& getCustomSemanticData(physx::PxU32 index) const
	{
		PX_ASSERT(index < m_numCustomSemantics);
		return m_customSemanticData[index];
	}

	/**
	\brief Set a specific set of custom semantics on this data buffer.

	If there are named custom semantics in a NxRenderMeshAsset, its NxRenderMeshActor will
	call this function to set those semantics on this buffer.
	*/
	PX_INLINE void setCustomSemanticData(NxApexRenderSemanticData* data, physx::PxU32 num)
	{
		m_numCustomSemantics = num;
		m_customSemanticData = data;
	}

	/**
	\brief Set the properties (pointer and stride) of a base semantic.
	*/
	PX_INLINE void setSemanticData(SemanticEnum semantic, const void* data, physx::PxU32 stride, NxRenderDataFormat::Enum format, NxRenderDataFormat::Enum srcFormat = NxRenderDataFormat::UNSPECIFIED)
	{
		PX_ASSERT(semantic < SemanticClass::NUM_SEMANTICS);
		NxApexRenderSemanticData& sd = m_semanticData[semantic];
		sd.data   = data;
		sd.stride = stride;
		sd.format = format;
		sd.srcFormat = srcFormat == NxRenderDataFormat::UNSPECIFIED ? format : srcFormat;
	}

private:
	NxApexRenderSemanticData  m_semanticData[SemanticClass::NUM_SEMANTICS]; //!< Base semantics for this buffer type
	physx::PxU32			  m_numCustomSemantics;							//!< Number of custom semantics
	NxApexRenderSemanticData* m_customSemanticData;							//!< An array of custom semantics
};

/**
\brief Describes data semantics provided by a particular module
*/
class NxApexModuleSpecificRenderBufferData
{
public:
	NxAuthObjTypeID           moduleId;						//!< Unique module identifier, from NxModule::getModuleID()
	NxApexRenderSemanticData* moduleSpecificSemanticData;   //!< Pointer to an array of custom semantics
	physx::PxU32              numModuleSpecificSemantics;   //!< Count of custom semantics provided by this module

public:
	PX_INLINE NxApexModuleSpecificRenderBufferData(void)
	{
		moduleId                   = 0;
		moduleSpecificSemanticData = 0;
		numModuleSpecificSemantics = 0;
	}
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_APEX_RENDER_BUFFER_DATA_H
