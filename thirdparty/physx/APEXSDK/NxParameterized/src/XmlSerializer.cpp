/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "PsShare.h"
#include "PsArray.h"
#include "foundation/PxMath.h"

#include "PsFastXml.h"
#include "PxIOStream.h"

#include "NxSerializer.h"
#include "XmlSerializer.h"
#include "NxParameterized.h"
#include "NxParameters.h"
#include "NxParameterizedTraits.h"
#include "NxTraitsInternal.h"
#include "XmlDeserializer.h"

#define PRINT_ELEMENT_HINTS 0
#define PRINT_ELEMENTS_WITHIN_EMPTY_ARRAYS 0

#define UNOPTIMIZED_XML 0

namespace NxParameterized
{

static const char indentStr[] = "  ";

struct traversalState
{
	traversalState()
	{
		indent[0] = 0;
		indentLen = 0;
		level = 0;
	}

	void incLevel()
	{
		physx::string::strncat_s(indent, (physx::PxI32)strlen(indent) + (physx::PxI32)strlen(indentStr) + 1, indentStr, strlen(indentStr));
		level++;
	}

	void decLevel()
	{
		level--;
		indentLen = (sizeof(indentStr) - 1) * level;
		if(indentLen < sizeof(indent))
			indent[indentLen] = 0;
	}

	char indent[4096];
	physx::PxU32 indentLen;
	physx::PxI32 level;
};

Serializer::ErrorType XmlSerializer::peekNumObjects(char *data, physx::PxU32 len, physx::PxU32 &numObjects)
{
	//FIXME: this code is not robust

	data[len-1] = 0;

	const char *root = ::strstr(data, "<NxParameters");
	if( !root )
		return Serializer::ERROR_MISSING_ROOT_ELEMENT;

	const char *numObjectsString = ::strstr(root, "numObjects");
	if( !numObjectsString )
		return Serializer::ERROR_INVALID_ATTR;

	numObjectsString = ::strstr(numObjectsString, "\"");
	if( !numObjectsString )
		return Serializer::ERROR_INVALID_ATTR;

	numObjects = strtoul(numObjectsString + 1, 0, 0); //Skip leading quote
	return Serializer::ERROR_NONE;
}

class InputDataFromPxFileBuf: public physx::PxInputData
{
public:
	InputDataFromPxFileBuf(physx::PxFileBuf& fileBuf): mFileBuf(fileBuf) {}

	// physx::PxInputData interface
	virtual physx::PxU32	getLength() const
	{
		return mFileBuf.getFileLength();
	}

	virtual void	seek(physx::PxU32 offset)
	{
		mFileBuf.seekRead(offset);
	}

	virtual physx::PxU32	tell() const
	{
		return mFileBuf.tellRead();
	}

	// physx::PxInputStream interface
	virtual physx::PxU32 read(void* dest, physx::PxU32 count)
	{
		return mFileBuf.read(dest, count);
	}

	PX_NOCOPY(InputDataFromPxFileBuf)
private:
	physx::PxFileBuf& mFileBuf;
};

Serializer::ErrorType XmlSerializer::peekClassNames(physx::PxFileBuf &stream, char **classNames, physx::PxU32 &numClassNames)
{
	class ClassNameReader: public physx::FastXml::Callback
	{
		physx::PxU32 mDepth;
		physx::PxU32 mNumObjs;

		char **mClassNames;
		physx::PxU32 mNumClassNames;

		Traits *mTraits;

	public:
		ClassNameReader(char **classNames, physx::PxU32 &numClassNames, Traits *traits)
			: mDepth(0), mNumObjs(0), mClassNames(classNames), mNumClassNames(numClassNames), mTraits(traits) {}

		physx::PxU32 numObjs() const { return mNumObjs; }

		bool processComment(const char * /*comment*/) { return true; }

		bool processClose(const char * /*element*/,physx::PxU32 /*depth*/,bool & /*isError*/)
		{
			--mDepth;
			return true;
		}

		bool processElement(
			const char *elementName,
			const char  * /*elementData*/,
			const physx::FastXml::AttributePairs& attr,
			physx::PxI32 /*lineno*/)
		{
			//Ignore top-level element
			if( 0 == strcmp(elementName, "NxParameters") )
				return true;

			++mDepth;

			if( 1 != mDepth || 0 != strcmp(elementName, "value") )
				return true;

			//Top-level <value> => read className
			mClassNames[mNumObjs] = mTraits->strdup( attr.get("className") );
			++mNumObjs;

			return mNumObjs < mNumClassNames;
		}

		void *allocate(physx::PxU32 size) { return ::malloc(size); }
		void deallocate(void *ptr) { ::free(ptr); };
	};

	ClassNameReader myReader(classNames, numClassNames, mTraits);
	physx::FastXml *xmlParser = physx::createFastXml(&myReader);

	InputDataFromPxFileBuf inputData(stream);
	xmlParser->processXml(inputData);
	numClassNames = myReader.numObjs();

	return Serializer::ERROR_NONE;
}

Serializer::ErrorType XmlSerializer::peekNumObjectsInplace(const void * data, physx::PxU32 dataLen, physx::PxU32 & numObjects)
{
	if ( !dataLen || ! data )
		return ERROR_STREAM_ERROR;

	char hdr[100];
	physx::PxU32 len = physx::PxMin<physx::PxU32>(dataLen, sizeof(hdr) - 1);
	physx::string::strncpy_s(hdr, len+1, (const char *)data, len);

	return peekNumObjects(hdr, len, numObjects);
}

Serializer::ErrorType XmlSerializer::peekNumObjects(physx::PxFileBuf &stream, physx::PxU32 &numObjects)
{
	//FIXME: this code is not robust

	char hdr[100];
	physx::PxU32 len = stream.peek(hdr, sizeof(hdr));

	return peekNumObjects(hdr, len, numObjects);
}

#ifndef WITHOUT_APEX_SERIALIZATION

static void storeVersionAndChecksum(physx::PxIOStream &stream, const Interface *obj)
{
	physx::PxU16 major = obj->getMajorVersion(),
		minor = obj->getMinorVersion(); 

	stream << " version=\"" << major << '.' << minor << '"';

	physx::PxU32 bits;
	const physx::PxU32 *checksum = obj->checksum(bits);

	physx::PxU32 u32s = bits / 32;
	PX_ASSERT( 0 == bits % 32 );

	stream << " checksum=\"";
	for(physx::PxU32 i = 0; i < u32s; ++i)
	{
		char hex[20];
		physx::string::sprintf_s(hex, sizeof(hex), "0x%x", checksum[i]);
		stream << hex;
		if( u32s - 1 != i )
			stream << ' ';
	}
	stream << '"';
}

static bool IsSimpleType(const Definition *d)
{
	//We do not consider strings simple because it causes errors with NULL and ""
	switch ( d->type() )
	{
	case TYPE_ARRAY:
	case TYPE_STRUCT:
	case TYPE_REF:
	case TYPE_STRING:
	case TYPE_ENUM:
		return false;
	default:
		PX_ASSERT( d->numChildren() == 0 );
		return true;
	}
}

static bool IsSimpleStruct(const Definition *pd)
{
	bool ret = true;

	physx::PxI32 count = pd->numChildren();
	for (physx::PxI32 i=0; i<count; i++)
	{
		const Definition *d = pd->child(i);
		if ( !IsSimpleType(d) )
		{
			ret = false;
			break;
		}
	}

	return ret;
}

static bool DoesNeedQuote(const char *c)
{
	bool ret = false;
	while ( *c )
	{
		if ( *c == 32 || *c == ',' || *c == '<' || *c == '>' || *c == 9 )
		{
			ret = true;
			break;
		}
		c++;
	}

	return ret;
}
	
Serializer::ErrorType XmlSerializer::traverseParamDefTree(
	const Interface &obj,
	physx::PxIOStream &stream,
	traversalState &state,
	Handle &handle,
	bool printValues)
{
	bool isRoot = !handle.numIndexes() && 0 == state.level;

	if( !handle.numIndexes() )
	{
		NX_PARAM_ERR_CHECK_RETURN( obj.getParameterHandle("", handle), Serializer::ERROR_UNKNOWN );

		if( isRoot )
		{
			NX_ERR_CHECK_RETURN( emitElement(obj, stream, "value", handle, false, true, true) );
			stream << "\n";

			state.incLevel();
		}
	}

	const Definition *paramDef = handle.parameterDefinition();

	if( !paramDef->hint("DONOTSERIALIZE") )
	{

#	if PRINT_ELEMENT_HINTS
	bool includedRef = false;

	NX_ERR_CHECK_RETURN( emitElementNxHints(stream, handle, state, includedRef) );
#	else
	bool includedRef = paramDef->isIncludedRef();
#	endif

	switch( paramDef->type() )
	{
	case TYPE_STRUCT:
		{
			stream << state.indent;
			NX_ERR_CHECK_RETURN( emitElement(obj, stream, "struct", handle, false, true) );
			stream << "\n";

			state.incLevel();
			for(physx::PxI32 i = 0; i < paramDef->numChildren(); ++i)
			{
				handle.set(i);
				NX_ERR_CHECK_RETURN( traverseParamDefTree(obj, stream, state, handle, printValues) );
				handle.popIndex();
			}

			state.decLevel();

			stream << state.indent << "</struct>\n";

			break;
		}

	case TYPE_ARRAY:
		{
			stream << state.indent;
			NX_ERR_CHECK_RETURN( emitElement(obj, stream, "array", handle, false, true) );

			physx::PxI32 arraySize;
			NX_PARAM_ERR_CHECK_RETURN( handle.getArraySize(arraySize), Serializer::ERROR_INVALID_ARRAY );

			if( arraySize)
				stream << "\n";

			state.incLevel();

			if ( arraySize > 0 )
			{
#if UNOPTIMIZED_XML
				for(physx::PxI32 i = 0; i < arraySize; ++i)
				{
					handle.set(i);
					NX_ERR_CHECK_RETURN( traverseParamDefTree(obj, stream, state, handle, printValues) );
					handle.popIndex();
				}
#else
				handle.set(0);
				const Definition *pd = handle.parameterDefinition();
				handle.popIndex();
				switch ( pd->type() )
				{
					case TYPE_STRUCT:
						{
							if ( IsSimpleStruct(pd) )
							{
								for(physx::PxI32 i = 0; i < arraySize; ++i)
								{

									if ( (i&3) == 0 )
									{
										if ( i )
											stream << "\n";
										stream << state.indent;
									}

									handle.set(i);

									for( physx::PxI32 j=0; j<pd->numChildren(); j++ )
									{
										if (pd->child(j)->hint("DONOTSERIALIZE"))
											continue;

										handle.set(j);

										char buf[512];
										const char *str = 0;
										NX_PARAM_ERR_CHECK_RETURN( handle.valueToStr(buf, sizeof(buf), str), Serializer::ERROR_VAL2STRING_FAILED );

										stream << str;

										if ( (j+1) < pd->numChildren() )
										{
											stream << " ";
										}

										handle.popIndex();
									}

									if ( (i+1) < arraySize )
									{
										stream << ",";
									}

									handle.popIndex();
								} //i
								stream << "\n";
							}
							else
							{
								for(physx::PxI32 i = 0; i < arraySize; ++i)
								{
									handle.set(i);
									NX_ERR_CHECK_RETURN( traverseParamDefTree(obj, stream, state, handle, printValues) );
									handle.popIndex();
								}
							}
						}
						break;

					case TYPE_REF:
						for(physx::PxI32 i = 0; i < arraySize; ++i)
						{
							handle.set(i);
							NX_ERR_CHECK_RETURN( traverseParamDefTree(obj, stream, state, handle, printValues) );
							handle.popIndex();
						}
						break;

					case TYPE_BOOL:
						{
							bool v = false;
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamBoolArray(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << (physx::PxU32)v;
								if ( (i+1) < arraySize )
								{
									stream << " ";
								}
								if ( ((i+1)&63) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
						break;

    				case TYPE_I8:
						{
							physx::PxI8 v = 0;
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamI8Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v;
								if ( (i+1) < arraySize )
								{
									stream << " ";
								}
								if ( ((i+1)&63) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
    					break;

    				case TYPE_I16:
						{
							physx::PxI16 v = 0;
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamI16Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v;
								if ( (i+1) < arraySize )
								{
									stream << " ";
								}
								if ( ((i+1)&31) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
    					break;

    				case TYPE_I32:
						{
							physx::PxI32 v = 0;
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamI32Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v;
								if ( (i+1) < arraySize )
								{
									stream << " ";
								}
								if ( ((i+1)&31) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
    					break;

    				case TYPE_I64:
						{
							physx::PxI64 v = 0;
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamI64Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v;
								if ( (i+1) < arraySize )
								{
									stream << " ";
								}
								if ( ((i+1)&31) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
    					break;

    				case TYPE_U8:
						{
							physx::PxU8 v = 0;
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamU8Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v;
								if ( (i+1) < arraySize )
								{
									stream << " ";
								}
								if ( ((i+1)&63) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
    					break;

    				case TYPE_U16:
						{
							physx::PxU16 v = 0;
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamU16Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v;
								if ( (i+1) < arraySize )
								{
									stream << " ";
								}
								if ( ((i+1)&63) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
    					break;

    				case TYPE_U32:
						{
							physx::PxU32 v = 0;
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamU32Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v;
								if ( (i+1) < arraySize )
								{
									stream << " ";
								}
								if ( ((i+1)&31) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
    					break;

    				case TYPE_U64:
						{
							physx::PxU64 v = 0;
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamU64Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v;
								if ( (i+1) < arraySize )
								{
									stream << " ";
								}
								if ( ((i+1)&31) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
    					break;

    				case TYPE_F32:
						{
							physx::PxF32 v = 0;
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamF32Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v;
								if ( (i+1) < arraySize )
								{
									stream << " ";
								}
								if ( ((i+1)&31) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
    					break;

    				case TYPE_F64:
						{
							physx::PxF64 v = 0;
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamF64Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v;
								if ( (i+1) < arraySize )
								{
									stream << " ";
								}
								if ( ((i+1)&31) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
    					break;

					case TYPE_VEC2:
						{
							physx::PxVec2 v(0,0);
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamVec2Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v.x << " " << v.y;
								if ( (i+1) < arraySize )
								{
									stream << ", ";
								}
								if ( ((i+1)&15) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
						break;

					case TYPE_VEC3:
						{
							physx::PxVec3 v(0,0,0);
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamVec3Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v.x << " " << v.y << " " << v.z;
								if ( (i+1) < arraySize )
								{
									stream << ", ";
								}
								if ( ((i+1)&15) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
						break;

					case TYPE_VEC4:
						{
							physx::PxVec4 v(0,0,0,0);
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamVec4Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v.x << " " << v.y << " " << v.z << " " << v.w;
								if ( (i+1) < arraySize )
								{
									stream << ", ";
								}
								if ( ((i+1)&15) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
						break;

					case TYPE_QUAT:
						{
							physx::PxQuat v(0,0,0,1);
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamQuatArray(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v.x << " " << v.y << " " << v.z << " " << v.w;
								if ( (i+1) < arraySize )
								{
									stream << ", ";
								}
								if ( ((i+1)&15) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
						break;

					case TYPE_MAT33:
						{
							physx::PxMat33 m = physx::PxMat33::createIdentity();
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamMat33Array(&m,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								const physx::PxF32 *f = (const physx::PxF32 *)m.front();
								stream << f[0] << " " << f[1] << " " << f[2] << " " ;
								stream << f[3] << " " << f[4] << " " << f[5] << " ";
								stream << f[6] << " " << f[7] << " " << f[8];
								if ( (i+1) < arraySize )
								{
									stream << ", ";
								}
								if ( ((i+1)&15) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
						break;

					case TYPE_MAT34:
						{
							physx::PxMat44 m = physx::PxMat44::createIdentity();
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamMat34Array(&m,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								physx::PxF32 f[9];
								physx::PxMat34Legacy(m).M.getColumnMajor(f);
								physx::PxVec3 t = physx::PxMat34Legacy(m).t;
								stream << f[0] << " " << f[1] << " " << f[2] << " " ;
								stream << f[3] << " " << f[4] << " " << f[5] << " ";
								stream << f[6] << " " << f[7] << " " << f[8] << " ";
								stream << t.x << " " << t.y << " " << t.z;
								if ( (i+1) < arraySize )
								{
									stream << ", ";
								}
								if ( ((i+1)&15) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
						break;

					case TYPE_BOUNDS3:
						{
							physx::PxBounds3 v;
							v.minimum = physx::PxVec3(0.0f);
							v.maximum = physx::PxVec3(0.0f);
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamBounds3Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								stream << v.minimum.x << " " << v.minimum.y << " " << v.minimum.z << " " << v.maximum.x << " " << v.maximum.y << " " << v.maximum.z;
								if ( (i+1) < arraySize )
								{
									stream << ", ";
								}
								if ( ((i+1)&15) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
						break;

					case TYPE_MAT44:
						{
							physx::PxMat44 v = physx::PxMat44::createIdentity();
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamMat44Array(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								const physx::PxF32 *f = (const physx::PxF32 *)v.front();
								stream << f[0] << " " << f[1] << " " << f[2] << " " << f[3] << " ";
								stream << f[4] << " " << f[5] << " " << f[6] << " " << f[7] << " ";
								stream << f[8] << " " << f[9] << " " << f[10] << " " << f[11] << " ";
								stream << f[12] << " " << f[13] << " " << f[14] << " " << f[15];
								if ( (i+1) < arraySize )
								{
									stream << ", ";
								}
								if ( ((i+1)&15) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
						break;

					case TYPE_TRANSFORM:
						{
							physx::PxTransform v = physx::PxTransform::createIdentity();
							stream << state.indent;
							for (physx::PxI32 i=0; i<arraySize; i++)
							{
								NX_PARAM_ERR_CHECK_RETURN( handle.getParamTransformArray(&v,1,i), Serializer::ERROR_ARRAY_INDEX_OUT_OF_RANGE );
								const physx::PxF32 *f = (const physx::PxF32 *)&v;
								stream << f[0] << " " << f[1] << " " << f[2] << " " << f[3] << " ";
								stream << f[4] << " " << f[5] << " " << f[6] << " ";
								if ( (i+1) < arraySize )
								{
									stream << ", ";
								}
								if ( ((i+1)&15) == 0 )
								{
									stream << "\n";
									stream << state.indent;
								}
							}
							stream << "\n";
						}
						break;

					default:
						for(physx::PxI32 i = 0; i < arraySize; ++i)
						{
							handle.set(i);
							NX_ERR_CHECK_RETURN( traverseParamDefTree(obj, stream, state, handle, printValues) );
							handle.popIndex();
						}
						break;
				}
#endif
			}

#	if PRINT_ELEMENTS_WITHIN_EMPTY_ARRAYS
			if( arraySize == 0 )
			{
				handle.set(0);
				NX_ERR_CHECK_RETURN( traverseParamDefTree(obj, stream, state, handle, false) );
				handle.popIndex();
			}
#	endif

			state.decLevel();
			if( arraySize)
				stream << state.indent;

			stream << "</array>\n";

			break;
		}

	case TYPE_REF:
		{
			stream << state.indent;
			NX_ERR_CHECK_RETURN( emitElement(obj, stream, "value", handle, includedRef, printValues) );

			if( printValues && includedRef )
			{
				stream << state.indent << "\n";

				Interface *refObj = 0;
				NX_PARAM_ERR_CHECK_RETURN( handle.getParamRef(refObj), Serializer::ERROR_UNKNOWN );

				if( refObj )
				{
					Handle refHandle(refObj);
					state.incLevel();
					NX_ERR_CHECK_RETURN( traverseParamDefTree(*refObj, stream, state, refHandle) );
					state.decLevel();
					stream << state.indent;
				}
			}

			stream << "</value>\n";

			break;
		}

	case TYPE_POINTER:
		//Don't do anything with pointer
		break;

	default:
		{
			stream << state.indent;
			NX_ERR_CHECK_RETURN( emitElement(obj, stream, "value", handle, includedRef, printValues) );

			char buf[512];
			const char *str = 0;
			if(	printValues )
				NX_PARAM_ERR_CHECK_RETURN( handle.valueToStr(buf, sizeof(buf), str), Serializer::ERROR_VAL2STRING_FAILED );

			if( str )
				stream << str;

			stream << "</value>\n";

			break;
		} //default
	} //switch

	} //DONOTSERIALIZE

	if( isRoot )
	{
		state.decLevel();
		stream << "</value>\n";
	}

	return Serializer::ERROR_NONE;
}

Serializer::ErrorType XmlSerializer::emitElementNxHints(
	physx::PxIOStream &stream,
	Handle &handle,
	traversalState &state,
	bool &includedRef)
{
	const Definition *paramDef = handle.parameterDefinition();

	for(physx::PxI32 j = 0; j < paramDef->numHints(); ++j)
	{
		if( 0 == j )
			stream << "\n";

		const Hint *hint = paramDef->hint(j);

		stream << state.indent << "<!-- " << hint->name() << ": ";

		if( hint->type() == TYPE_STRING )
			stream << hint->asString() ;
		else if( hint->type() == TYPE_U64 )
			stream << hint->asUInt() ;
		else if( hint->type() == TYPE_F64 )
			stream << hint->asFloat() ;

		stream << " -->\n";
	}

	includedRef = paramDef->isIncludedRef();

	return Serializer::ERROR_NONE;
}

Serializer::ErrorType XmlSerializer::emitElement(
	const Interface &obj,
	physx::PxIOStream &stream,
	const char *elementName,
	Handle &handle,
	bool includedRef,
	bool printValues,
	bool isRoot)
{
	const Definition *paramDef = handle.parameterDefinition();

	DataType parentType = TYPE_UNDEFINED;

	if( paramDef->parent() )
		parentType = paramDef->parent()->type();

	stream << '<' << elementName;

	if( isRoot )
	{
		stream << " name=\"\""
			<< " type=\"Ref\""
			<< " className=\"" << obj.className() << "\"";

		const char *objectName = obj.name();
		if( objectName )
			stream << " objectName=\"" << objectName << "\"";

		if( isRoot ) //We only emit version info for root <struct>
			storeVersionAndChecksum(stream, &obj);
	}
	else
	{
		if( parentType != TYPE_ARRAY )
		{
			const char *name = paramDef->name();
			stream << " name=\"" << (name ? name : "") << "\"";
		}
	}

	switch( paramDef->type() )
	{
	case TYPE_STRUCT:
		break;

	case TYPE_ARRAY:
		{
			physx::PxI32 arraySize;
			NX_PARAM_ERR_CHECK_RETURN( handle.getArraySize(arraySize), Serializer::ERROR_INVALID_ARRAY );
			stream << " size=\"" << arraySize << '"';
			handle.set(0);
			const Definition *pd = handle.parameterDefinition();
			handle.popIndex();
			stream << " type=\"" << typeToStr(pd->type()) << '"';
			// ** handle use case for simple structs written out flat..
#if !UNOPTIMIZED_XML
			if ( pd->type() == TYPE_STRUCT && IsSimpleStruct(pd) )
			{
				stream << " structElements=\"";
				const physx::PxI32 count = pd->numChildren();

				// find how many of them need serialization
				physx::PxI32 serializeCount = 0;
				for (physx::PxI32 i=0; i<count; i++)
				{
					const Definition *d = pd->child(i);
					if (d->hint("DONOTSERIALIZE") == NULL)
					{
						serializeCount++;
					}
				}

				for (physx::PxI32 i=0; i<count; i++)
				{
					const Definition *d = pd->child(i);
					if (d->hint("DONOTSERIALIZE"))
						continue;

					stream << d->name();
					stream << "(";
					stream << typeToStr(d->type());
					stream << ")";
					if ( (i+1) < serializeCount )
					{
						stream<<",";
					}
				}
				stream << "\"";
			}
#endif
			//
			break;
		}

	case TYPE_REF:
		{
			stream << " type=\"" << typeToStr(paramDef->type()) << '"';

			Interface *paramPtr = 0;
			if( printValues )
				NX_PARAM_ERR_CHECK_RETURN( handle.getParamRef(paramPtr), Serializer::ERROR_UNKNOWN );

			stream << " included=\"" << ( includedRef ? "1" : "0" ) << "\"";

			if( !printValues || !paramPtr )
			{
				stream << " classNames=\"";
				for(physx::PxI32 i = 0; i < paramDef->numRefVariants(); ++i)
				{
					const char *ref = paramDef->refVariantVal(i);
					if ( DoesNeedQuote(ref) )
						stream << "%20" << ref << "%20" << " ";
					else
						stream << ref << " ";
				}
				stream << '"';

				break;
			}

			stream << " className=\"" << paramPtr->className() << '"';

			const char *objectName = paramPtr->name();
			if( objectName )
				stream << " objectName=\"" << objectName << "\"";

			if( includedRef )
				storeVersionAndChecksum(stream, paramPtr);

			break;
		}

	case TYPE_STRING:
	case TYPE_ENUM:
		{
			const char *val;
			NX_PARAM_ERR_CHECK_RETURN( handle.getParamString(val), Serializer::ERROR_UNKNOWN );

			//Make a note if value is NULL
			if( !val )
				stream << " null=\"1\"";
		}

		//Fall-through to default

	default:
		stream << " type=\"" << typeToStr(paramDef->type()) << "\"";
		break;
	} //switch

	stream << '>';

	return Serializer::ERROR_NONE;
}

Serializer::ErrorType XmlSerializer::internalSerialize(physx::PxFileBuf &fbuf, const Interface **objs, physx::PxU32 n, bool doMetadata)
{
	PX_UNUSED(doMetadata);

	physx::PxIOStream stream(fbuf, fbuf.getFileLength());
	stream.setBinary(false);

	physx::PxU32 minor = version() & 0xffffUL,
		major = version() >> 16;

	stream << "<!DOCTYPE NxParameters>\n"
		<< "<NxParameters "
		<< "numObjects=\"" << n << "\" "
		<< "version=\"" << major << '.' << minor << "\" "
		<< ">\n";

	for(physx::PxU32 i = 0; i < n; ++i)
	{
		const Interface &obj = *objs[i];
		Handle handle(obj);

		traversalState state;
		NX_ERR_CHECK_RETURN( traverseParamDefTree(obj, stream, state, handle) );
	}

	stream << "</NxParameters>\n";

	return Serializer::ERROR_NONE;
}

#endif

Serializer::ErrorType XmlSerializer::internalDeserialize(
	physx::PxFileBuf &stream,
	Serializer::DeserializedData &res,
	bool & /*doesNeedUpdate*/)
{
	XmlDeserializer *d = XmlDeserializer::Create(mTraits, XmlSerializer::version());
	physx::FastXml *xmlParser = physx::createFastXml(d);
	InputDataFromPxFileBuf inputData(stream);
	if( xmlParser && !xmlParser->processXml(inputData) )
	{
		Serializer::ErrorType err = d->getLastError();
		if( Serializer::ERROR_NONE == err ) //Proper error code not set?
		{
			DEBUG_ALWAYS_ASSERT(); //XmlDeserializer should set explicit error codes
			err = Serializer::ERROR_UNKNOWN;
		}

		xmlParser->release();

		d->releaseAll();
		d->destroy();

		return err;
	}

	if ( xmlParser )
		xmlParser->release();

	res.init(mTraits, d->getObjs(), d->getNobjs());

	d->destroy();
	
	return Serializer::ERROR_NONE;
}
bool isXmlFormat(physx::PxFileBuf &stream)
{
	// if it is at least 32 bytes long and the first 32 byte are all ASCII, then consider it potentially valid XML

	if( stream.getFileLength() < 32 )
		return false;

	char hdr[32];
	stream.peek(hdr, sizeof(hdr));

	for(size_t i = 0; i < sizeof(hdr); ++i)
	{
		char c = hdr[i];
		if( !(c == '\r' || c == '\t' || c == '\n' || ( c >= 32 && c < 127)) )
			return false;
	}

	const char *magic = "<!DOCTYPE NxParameters>";
	return 0 == ::strncmp(hdr, magic, strlen(magic));
}

} // namespace NxParameterized

