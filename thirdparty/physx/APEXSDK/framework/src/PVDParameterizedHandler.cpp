/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "PVDParameterizedHandler.h"

#ifndef WITHOUT_PVD

#include "PVDBinding.h"
#include "PvdDataStream.h"
#include "NxParameterized.h"
#include "NxParameters.h"
#include "PxMat34Legacy.h"


using namespace physx::shdfnd;
using namespace physx::debugger;
using namespace physx::debugger::comm;


#define SET_PROPERTY_VALUE 	\
	if (pvdAction != PvdAction::DESTROY) \
	{\
		ok = propertyHandle.getParam(val) == NxParameterized::ERROR_NONE; \
		if (ok)\
		{\
			if (isArrayElement)\
				mPvdStream->appendPropertyValueData(DataRef<const PxU8>((const PxU8*)&val, sizeof(val)));\
			else\
				mPvdStream->setPropertyValue(pvdInstance, propertyName, val);\
		}\
	}\


namespace PVD
{



bool PvdParameterizedHandler::createClass(const NamespacedName& className)
{
	bool create = !mCreatedClasses.contains(className.mName);
	if (create)
	{
		mPvdStream->createClass(className);
		mCreatedClasses.insert(className.mName);
	}

	return create;
}



bool PvdParameterizedHandler::getPvdType(const NxParameterized::Definition& def, NamespacedName& pvdTypeName)
{
	NxParameterized::DataType paramType = def.type();

	bool ok = true;
	switch(paramType)
	{
	case NxParameterized::TYPE_BOOL :
		pvdTypeName = getPvdNamespacedNameForType<bool>();
		break;

	case NxParameterized::TYPE_STRING :
		pvdTypeName = getPvdNamespacedNameForType<const char*>();
		break;

	case NxParameterized::TYPE_I8 :
		pvdTypeName = getPvdNamespacedNameForType<PxI8>();
		break;

	case NxParameterized::TYPE_I16 :
		pvdTypeName = getPvdNamespacedNameForType<PxI16>();
		break;

	case NxParameterized::TYPE_I32 :
		pvdTypeName = getPvdNamespacedNameForType<PxI32>();
		break;

	case NxParameterized::TYPE_I64 :
		pvdTypeName = getPvdNamespacedNameForType<PxI64>();
		break;

	case NxParameterized::TYPE_U8 :
		pvdTypeName = getPvdNamespacedNameForType<PxU8>();
		break;

	case NxParameterized::TYPE_U16 :
		pvdTypeName = getPvdNamespacedNameForType<PxU16>();
		break;

	case NxParameterized::TYPE_U32 :
		pvdTypeName = getPvdNamespacedNameForType<PxU32>();
		break;

	case NxParameterized::TYPE_U64 :
		pvdTypeName = getPvdNamespacedNameForType<PxU64>();
		break;

	case NxParameterized::TYPE_F32 :
		pvdTypeName = getPvdNamespacedNameForType<PxF32>();
		break;

	case NxParameterized::TYPE_F64 :
		pvdTypeName = getPvdNamespacedNameForType<PxF64>();
		break;

	case NxParameterized::TYPE_VEC2 :
		pvdTypeName = getPvdNamespacedNameForType<PxVec2>();
		break;

	case NxParameterized::TYPE_VEC3 :
		pvdTypeName = getPvdNamespacedNameForType<PxVec3>();
		break;

	case NxParameterized::TYPE_VEC4 :
		pvdTypeName = getPvdNamespacedNameForType<PxU16>();
		break;

	case NxParameterized::TYPE_QUAT :
		pvdTypeName = getPvdNamespacedNameForType<PxQuat>();
		break;

	case NxParameterized::TYPE_MAT33 :
		pvdTypeName = getPvdNamespacedNameForType<PxMat33>();
		break;

	case NxParameterized::TYPE_MAT34 :
		pvdTypeName = getPvdNamespacedNameForType<PxMat34Legacy>();
		break;

	case NxParameterized::TYPE_BOUNDS3 :
		pvdTypeName = getPvdNamespacedNameForType<PxBounds3>();
		break;

	case NxParameterized::TYPE_MAT44 :
		pvdTypeName = getPvdNamespacedNameForType<PxMat44>();
		break;

	case NxParameterized::TYPE_POINTER :
		pvdTypeName = getPvdNamespacedNameForType<VoidPtr>();
		break;

	case NxParameterized::TYPE_TRANSFORM :
		pvdTypeName = getPvdNamespacedNameForType<PxTransform>();
		break;

	case NxParameterized::TYPE_REF :
	case NxParameterized::TYPE_STRUCT :
		pvdTypeName = getPvdNamespacedNameForType<ObjectRef>();
		break;

	case NxParameterized::TYPE_ARRAY:
		{
			PX_ASSERT(def.numChildren() > 0);
			const NxParameterized::Definition* arrayMemberDef = def.child(0);

			ok = getPvdType(*arrayMemberDef, pvdTypeName);

			// array of strings is not supported by pvd
			if (arrayMemberDef->type() == NxParameterized::TYPE_STRING)
			{
				ok = false;
			}

			break;
		}

	default:
		ok = false;
		break;
	};

	return ok;
}


size_t PvdParameterizedHandler::getStructId(void* structAddress, const char* structName, bool deleteId)
{
	StructId structId(structAddress, structName);

	size_t pvdStructId = 0;
	if (mStructIdMap.find(structId) != NULL)
	{
		pvdStructId = mStructIdMap[structId];
	}
	else
	{
		PX_ASSERT(!deleteId);

		 // addresses are 4 byte aligned, so this id is probably not used by another object
		pvdStructId = mNextStructId++;
		pvdStructId = (pvdStructId << 1) | 1;

		mStructIdMap[structId] = pvdStructId;
	}

	if (deleteId)
	{
		mStructIdMap.erase(structId);
	}

	return pvdStructId;
}


const void* PvdParameterizedHandler::getPvdId(const NxParameterized::Handle& handle, bool deleteId)
{
	void* retVal = 0;
	NxParameterized::DataType type = handle.parameterDefinition()->type();

	switch(type)
	{
	case NxParameterized::TYPE_REF:
		{
			// references use the referenced interface pointer as ID
			NxParameterized::Interface* paramRef = NULL;
			handle.getParamRef(paramRef);
			retVal = (void*)paramRef;
			break;
		}

	case NxParameterized::TYPE_STRUCT:
		{
			// structs use custom ID's, because two structs can have the same location if
			// a struct contains another struct as its first member
			NxParameterized::NxParameters* param = (NxParameterized::NxParameters*)(handle.getInterface());
			if (param != NULL)
			{
				// get struct address
				size_t offset = 0;
				void* structAddress = 0;
				param->getVarPtr(handle, structAddress, offset);

				// create an id from address and name
				retVal = (void*)getStructId(structAddress, handle.parameterDefinition()->longName(), deleteId);
			}
			break;
		}

	default:
		break;
	}

	return retVal;
}


bool PvdParameterizedHandler::setProperty(const void* pvdInstance, NxParameterized::Handle& propertyHandle, bool isArrayElement, PvdAction::Enum pvdAction)
{
	const char* propertyName = propertyHandle.parameterDefinition()->name();
	NxParameterized::DataType propertyType = propertyHandle.parameterDefinition()->type();

	bool ok = true;
	switch(propertyType)
	{
	case NxParameterized::TYPE_BOOL :
		{
			bool val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_STRING :
		{
			if (isArrayElement)
			{
				// pvd doesn't support arrays of strings
				ok = false;
			}
			else
			{
				const char* val;
				ok = propertyHandle.getParamString(val) == NxParameterized::ERROR_NONE;
				if (ok)
				{
					mPvdStream->setPropertyValue(pvdInstance, propertyName, val);
				}
			}
			
			break;
		}

	case NxParameterized::TYPE_I8 :
		{
			PxI8 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_I16 :
		{
			PxI16 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_I32 :
		{
			PxI32 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_I64 :
		{
			PxI64 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_U8 :
		{
			PxU8 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_U16 :
		{
			PxU16 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_U32 :
		{
			PxU32 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_U64 :
		{
			PxU64 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_F32 :
		{
			PxF32 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_F64 :
		{
			PxF64 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_VEC2 :
		{
			PxVec2 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_VEC3 :
		{
			PxVec3 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_VEC4 :
		{
			PxVec4 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_QUAT :
		{
			PxQuat val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_MAT33 :
		{
			PxMat33 val;
			SET_PROPERTY_VALUE;
			break;
		}
/*
	case NxParameterized::TYPE_MAT34 :
		{
			PxMat34Legacy val;
			SET_PROPERTY_VALUE;
			break;
		}
*/
	case NxParameterized::TYPE_BOUNDS3 :
		{
			PxBounds3 val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_MAT44 :
		{
			PxMat44 val;
			SET_PROPERTY_VALUE;
			break;
		}

		/*
	case NxParameterized::TYPE_POINTER :
		{
			void* val;
			SET_PROPERTY_VALUE;
			break;
		}
		*/

	case NxParameterized::TYPE_TRANSFORM :
		{
			PxTransform val;
			SET_PROPERTY_VALUE;
			break;
		}

	case NxParameterized::TYPE_STRUCT:
		{
			const void* pvdId = getPvdId(propertyHandle, pvdAction == PvdAction::DESTROY);

			if (pvdId != 0)
			{
				if (!mInstanceIds.contains(pvdId))
				{
					// create pvd instance for struct
					mInstanceIds.insert(pvdId);
					NamespacedName structName(APEX_PVD_NAMESPACE, propertyHandle.parameterDefinition()->structName());
					mPvdStream->createInstance(structName, pvdId);
					mPvdStream->setPropertyValue(pvdInstance, propertyName, DataRef<const PxU8>((const PxU8*)&pvdId, sizeof(NxParameterized::Interface*)), getPvdNamespacedNameForType<ObjectRef>());
				}

				// recursively update struct properties
				updatePvd(pvdId, propertyHandle, pvdAction);

				if (pvdAction == PvdAction::DESTROY)
				{
					// destroy pvd instance of struct
					mPvdStream->destroyInstance(pvdId);
					mInstanceIds.erase(pvdId);
				}
			}
			break;
		}

	case NxParameterized::TYPE_REF:
		{
			const void* pvdId = getPvdId(propertyHandle, pvdAction == PvdAction::DESTROY);

			if (pvdId != 0)
			{
				// get a handle in the referenced parameterized object
				NxParameterized::Handle refHandle = propertyHandle;
				propertyHandle.getChildHandle(0, refHandle);
				NxParameterized::Interface* paramRef = NULL;
				ok = refHandle.getParamRef(paramRef) == NxParameterized::ERROR_NONE;

				if (ok)
				{
					if (!mInstanceIds.contains(pvdId))
					{
						// create a pvd instance for the reference
						mInstanceIds.insert(pvdId);
						NamespacedName refClassName(APEX_PVD_NAMESPACE, paramRef->className());
						mPvdStream->createInstance(refClassName, pvdId);
						mPvdStream->setPropertyValue(pvdInstance, propertyName, DataRef<const PxU8>((const PxU8*)&pvdId, sizeof(NxParameterized::Interface*)), getPvdNamespacedNameForType<ObjectRef>());
					}

					// recursivly update pvd instance of the referenced object
					refHandle = NxParameterized::Handle(paramRef);
					updatePvd(pvdId, refHandle, pvdAction);

					if (pvdAction == PvdAction::DESTROY)
					{
						// destroy pvd instance of reference
						mPvdStream->destroyInstance(pvdId);
						mInstanceIds.erase(pvdId);
					}
				}
			}
			break;
		}

	case NxParameterized::TYPE_ARRAY:
		{
			const NxParameterized::Definition* def = propertyHandle.parameterDefinition();
			PX_ASSERT(def->numChildren() > 0);

			const NxParameterized::Definition* arrayMemberDef = def->child(0);
			NxParameterized::DataType arrayMemberType = arrayMemberDef->type();

			PX_ASSERT(def->arrayDimension() == 1);
			PxI32 arraySize = 0;
			propertyHandle.getArraySize(arraySize);

			if (arraySize > 0)
			{
				if (arrayMemberType == NxParameterized::TYPE_STRUCT || arrayMemberType == NxParameterized::TYPE_REF)
				{
					for (PxI32 i = 0; i < arraySize; ++i)
					{
						NxParameterized::Handle childHandle(propertyHandle);
						propertyHandle.getChildHandle(i, childHandle);

						const void* pvdId = getPvdId(childHandle, pvdAction == PvdAction::DESTROY);

						// get the class name of the member
						NamespacedName childClassName(APEX_PVD_NAMESPACE, "");
						if (arrayMemberType == NxParameterized::TYPE_STRUCT)
						{
							childClassName.mName = childHandle.parameterDefinition()->structName();
						}
						else if (arrayMemberType == NxParameterized::TYPE_REF)
						{
							// continue on a handle in the referenced object
							NxParameterized::Interface* paramRef = NULL;
							ok = childHandle.getParamRef(paramRef) == NxParameterized::ERROR_NONE;
							PX_ASSERT(ok);
							if (!ok)
							{
								break;
							}
							childHandle = NxParameterized::Handle(paramRef);
							childClassName.mName = paramRef->className();
						}

						if (!mInstanceIds.contains(pvdId))
						{
							// create pvd instance for struct or ref and add it to the array
							mInstanceIds.insert(pvdId);
							mPvdStream->createInstance(childClassName, pvdId);
							mPvdStream->pushBackObjectRef(pvdInstance, propertyName, pvdId);
						}
						
						// recursively update the array member
						updatePvd(pvdId, childHandle, pvdAction);

						if (pvdAction == PvdAction::DESTROY)
						{
							// destroy pvd instance for struct or ref
							mPvdStream->removeObjectRef(pvdInstance, propertyName, pvdId); // might not be necessary
							mPvdStream->destroyInstance(pvdId);
							mInstanceIds.erase(pvdId);
						}
					}
				}
				else if (pvdAction != PvdAction::DESTROY)
				{
					// for arrays of simple types just update the property values
					NamespacedName pvdTypeName;
					if (getPvdType(*def, pvdTypeName))
					{
						mPvdStream->beginSetPropertyValue(pvdInstance, propertyName, pvdTypeName);
						for (PxI32 i = 0; i < arraySize; ++i)
						{
							NxParameterized::Handle childHandle(propertyHandle);
							propertyHandle.getChildHandle(i, childHandle);

							setProperty(pvdInstance, childHandle, true, pvdAction);
						}
						mPvdStream->endSetPropertyValue();
					}
				}
			}
			break;
		}

	default:
		ok = false;
		break;
	};

	return ok;
}



void PvdParameterizedHandler::initPvdClasses(const NxParameterized::Definition& paramDefinition, const char* className)
{
	NamespacedName pvdClassName(APEX_PVD_NAMESPACE, className);

	// iterate all properties
	const int numChildren = paramDefinition.numChildren();
	for (int i = 0; i < numChildren; i++)
	{
		const NxParameterized::Definition* childDef = paramDefinition.child(i);

		const char* propertyName = childDef->name();
		NxParameterized::DataType propertyDataType = childDef->type();
		

		// First, recursively create pvd classes for encountered structs
		//
		// if it's an array, continue with its member type, and remember that it's an array
		bool isArray = false;
		if (propertyDataType == NxParameterized::TYPE_ARRAY)
		{
			PX_ASSERT(childDef->numChildren() > 0);

			const NxParameterized::Definition* arrayMemberDef = childDef->child(0);
			if (arrayMemberDef->type() == NxParameterized::TYPE_STRUCT)
			{
				NamespacedName memberClassName(APEX_PVD_NAMESPACE, arrayMemberDef->structName());
				if (createClass(memberClassName))
				{
					// only recurse if this we encounter the struct the first time and a class has been created
					initPvdClasses(*arrayMemberDef, memberClassName.mName);
				}
			}

			isArray = true;
		}
		else if (propertyDataType == NxParameterized::TYPE_STRUCT)
		{
			// create classes for structs
			// (doesn't work for refs, looks like Definitions don't contain the Definitions of references)

			NamespacedName childClassName(APEX_PVD_NAMESPACE, childDef->structName());
			if (createClass(childClassName))
			{
				// only recurse if this we encounter the struct the first time and a class has been created
				initPvdClasses(*childDef, childClassName.mName);
			}
		}


		// Then, create the property
		NamespacedName typeName;
		if (!childDef->hint("NOPVD") && getPvdType(*childDef, typeName))
		{
			mPvdStream->createProperty(pvdClassName, propertyName, "", typeName, isArray ? PropertyType::Array : PropertyType::Scalar);
		}
	}
}


void PvdParameterizedHandler::updatePvd(const void* pvdInstance, NxParameterized::Handle& paramsHandle, PvdAction::Enum pvdAction)
{
	// iterate all properties
	const int numChildren = paramsHandle.parameterDefinition()->numChildren();
	for (int i = 0; i < numChildren; i++)
	{
		paramsHandle.set(i);

		if (!paramsHandle.parameterDefinition()->hint("NOPVD"))
		{
			setProperty(pvdInstance, paramsHandle, false, pvdAction);
		}
		paramsHandle.popIndex();
	}
}

}


#endif