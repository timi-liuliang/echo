/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_XML_SERIALIZER_H
#define NX_XML_SERIALIZER_H

//XML serialization (by John Ratcliff)

#include "NxSerializer.h"
#include "AbstractSerializer.h"

#include "PxIOStream.h"

namespace NxParameterized
{

struct traversalState;

bool isXmlFormat(physx::PxFileBuf &stream);

// XML serializer implementation
class XmlSerializer : public AbstractSerializer
{
	ErrorType peekNumObjects(char *data, physx::PxU32 len, physx::PxU32 &numObjects);

	Serializer::ErrorType peekClassNames(physx::PxFileBuf &stream, char **classNames, physx::PxU32 &numClassNames);

#ifndef WITHOUT_APEX_SERIALIZATION
	Serializer::ErrorType traverseParamDefTree(
		const Interface &obj,
		physx::PxIOStream &stream,
		traversalState &state,
		Handle &handle,
		bool printValues = true);

	Serializer::ErrorType emitElementNxHints(
		physx::PxIOStream &stream,
		Handle &handle,
		traversalState &state,
		bool &includedRef);

	Serializer::ErrorType emitElement(
		const Interface &obj,
		physx::PxIOStream &stream,
		const char *elementName,
		Handle &handle,
		bool includedRef,
		bool printValues,
		bool isRoot = false);
#endif

protected:

#ifndef WITHOUT_APEX_SERIALIZATION
	Serializer::ErrorType internalSerialize(physx::PxFileBuf &fbuf,const Interface **objs, physx::PxU32 n, bool doMetadata);
#endif

	Serializer::ErrorType internalDeserialize(physx::PxFileBuf &stream, DeserializedData &res, bool &doesNeedUpdate);

public:

	XmlSerializer(Traits *traits): AbstractSerializer(traits) {}

	~XmlSerializer() {}

	virtual void release(void)
	{
		this->~XmlSerializer();
		serializerMemFree(this,mTraits);
	}

	PX_INLINE static physx::PxU32 version()
	{
		return 0x00010000;
	}

	ErrorType peekNumObjectsInplace(const void * data, physx::PxU32 dataLen, physx::PxU32 & numObjects);

	ErrorType peekNumObjects(physx::PxFileBuf &stream, physx::PxU32 &numObjects);

	ErrorType deserializeMetadata(physx::PxFileBuf & /*stream*/, DeserializedMetadata & /*desData*/)
	{
		return Serializer::ERROR_NOT_IMPLEMENTED;
	}
};

} // namespace NxParameterized

#endif
