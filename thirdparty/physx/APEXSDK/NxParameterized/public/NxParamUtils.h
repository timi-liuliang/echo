/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_PARAM_UTILS_H
#define NX_PARAM_UTILS_H

#include "NxParameterized.h"

// utility methods to operate on NxParameterized data.

namespace NxParameterized
{

/**
\brief Recursively finds a parameter with the given name

This method will recursively search through not only this parameterized but all referenced
parameterized objects as well.  It sets the handle and returns the NxParameterized::Interface in which the name was found.

\param i the parameterized object that will be searched

\param longName	contains the name of the parameter to be found
the longName will work with arrays, structs, and included references

\param outHandle will contain the output handle that provides read-only access to the specified parameter

\returns the NxParameterized::Interface pointer in which the parameter is contained (this could be different than the top level NxParameterized::Interface if the parameter is contained in an included reference)
*/
PX_INLINE const Interface * findParam(const Interface &i,const char *longName, Handle &outHandle);


/**
\brief Recursively finds a parameter with the given name

This method will recursively search through not only this parameterized but all referenced
parameterized objects as well.  It sets the handle and returns the NxParameterized::Interface in which the name was found.

\param i the parameterized object that will be searched

\param longName	contains the name of the parameter to be found
the longName will work with arrays, structs, and included references

\param outHandle will contain the output handle that provides read-write access to the specified parameter

\returns the NxParameterized::Interface pointer in which the parameter is contained (this could be different than the top level NxParameterized::Interface if the parameter is contained in an included reference)
*/
PX_INLINE Interface * findParam(Interface &i,const char *longName, Handle &outHandle);

/**
\brief Container for results of getParamList
*/
struct ParamResult
{
public:
	/**
	\brief Constructor
	*/
	ParamResult(const char *name,
				const char *longName,
				const char *className,
				const char *instanceName,
				const Handle &handle,
				physx::PxI32 arraySize,
				DataType type)
		: mArraySize(arraySize),
		mInstanceName(instanceName),
		mClassName(className),
		mName(name),
		mLongName(longName),
		mHandle(handle),
		mDataType(type)
	{}

	/**
	\brief size of array (if parameter is array)
	*/
	physx::PxI32 mArraySize;

	/**
	\brief Name of parameter's parent object
	*/
	const char *mInstanceName;

	/**
	\brief Name of NxParameterized-class of parameter's parent object
	*/
	const char *mClassName;

	/**
	\brief The name of the parameter
	*/
	const char *mName;

	/**
	\brief The fully qualified 'long' name of the parameter
	*/
	const char *mLongName;

	/**
	\brief Use this handle to access the parameter in question
	*/
	Handle mHandle;

	/**
	\brief The type of parameter
	*/
	DataType mDataType;
};

/**
\brief A helper method to retrieve the list of all parameters relative to an interface.

\param [in]  i the input interface 
\param [in]  className is an optional class name to match to.  If this is null, it will return all parameters.
\param [in]  paramName is an optional parameter name to match to.  If this is null, it will return all parameters.
\param [out] count the number of parameters found
\param [in]  recursive if true the search will recurse into every included reference, not just the top parameterized class
\param [in]  classesOnly if true the search will only match against class names
\param [in]  traits typically the APEX traits class, used to allocate the ParamResult, see NxApexSDK::getParameterizedTraits()

\note The return pointer is allocated by the NxParameterized Traits class and should be freed by calling releaseParamList()

*/
PX_INLINE const ParamResult * getParamList(const Interface &i,
								 const char *className,	// name of class to match
								 const char *paramName, // name of parameter to match
								 physx::PxU32 &count,
								 bool recursive,
								 bool classesOnly,
								 NxParameterized::Traits *traits);

/// helper function to free the parameter list returned from getParamList
PX_INLINE void				releaseParamList(physx::PxU32 resultCount,const ParamResult *results,NxParameterized::Traits *traits);

/// helper function to get an NxParameterized array size
PX_INLINE bool getParamArraySize(const Interface &pm, const char *name, physx::PxI32 &arraySize);

/// helper function to resize an NxParameterized array
PX_INLINE bool resizeParamArray(Interface &pm, const char *name, physx::PxI32 newSize);

/**
\brief Callback container for getNamedReferences
*/
class NamedReferenceInterface
{
public:
	/**
	\brief Callback

	Calls back to the user with any named reference (not included references) in the NxParameterized::Interface
	If the user returns a NULL pointer, than the original named reference is left alone.
	If the user returns a non-NULL pointer, than the named reference is replaced with the 'const char *' returned.
	*/
	virtual const char * namedReferenceCallback(const char *className,const char *namedReference,Handle &handle) = 0;
};

/// Calls back for every non-included named reference.
PX_INLINE physx::PxU32 getNamedReferences(const Interface &i,
										  NamedReferenceInterface &namedReference,
										  bool recursive);

/**
\brief Callback container for getReferences
*/
class ReferenceInterface
{
public:
	/**
	\brief Callback

	Calls back to the user with any reference (named or included or both) in the NxParameterized::Interface.
	*/
	virtual void referenceCallback(Handle &handle) = 0;
};

/// Calls back for every reference (named or included or both).
PX_INLINE void getReferences(const Interface &iface,
										  ReferenceInterface &cb,
										  bool named,
										  bool included,
										  bool recursive);

/// helper function to get an NxParameterized value
PX_INLINE bool getParamBool(const Interface &pm, const char *name, bool &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamBool(Interface &pm, const char *name, bool val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamString(const Interface &pm, const char *name, const char *&val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamString(Interface &pm, const char *name, const char *val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamEnum(const Interface &pm, const char *name, const char *&val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamEnum(Interface &pm, const char *name, const char *val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamRef(const Interface &pm, const char *name, NxParameterized::Interface *&val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamRef(Interface &pm, const char *name, NxParameterized::Interface *val, bool doDestroyOld = false) ;

/// helper function to init an NxParameterized value
PX_INLINE bool initParamRef(Interface &pm, const char *name, const char *className, bool doDestroyOld = false);
/// helper function to init an NxParameterized value
PX_INLINE bool initParamRef(Interface &pm, const char *name, const char *className, const char *objName, bool doDestroyOld = false);

/// helper function to get an NxParameterized value
PX_INLINE bool getParamI8(const Interface &pm, const char *name, physx::PxI8 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamI8(Interface &pm, const char *name, physx::PxI8 val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamI16(const Interface &pm, const char *name, physx::PxI16 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamI16(Interface &pm, const char *name, physx::PxI16 val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamI32(const Interface &pm, const char *name, physx::PxI32 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamI32(Interface &pm, const char *name, physx::PxI32 val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamI64(const Interface &pm, const char *name, physx::PxI64 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamI64(Interface &pm, const char *name, physx::PxI64 val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamU8(const Interface &pm, const char *name, physx::PxU8 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamU8(Interface &pm, const char *name, physx::PxU8 val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamU16(const Interface &pm, const char *name, physx::PxU16 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamU16(Interface &pm, const char *name, physx::PxU16 val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamU32(const Interface &pm, const char *name, physx::PxU32 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamU32(Interface &pm, const char *name, physx::PxU32 val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamU64(const Interface &pm, const char *name, physx::PxU64 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamU64(Interface &pm, const char *name, physx::PxU64 val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamF32(const Interface &pm, const char *name, physx::PxF32 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamF32(Interface &pm, const char *name, physx::PxF32 val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamF64(const Interface &pm, const char *name, physx::PxF64 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamF64(Interface &pm, const char *name, physx::PxF64 val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamVec2(const Interface &pm, const char *name, physx::PxVec2 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamVec2(Interface &pm, const char *name, const physx::PxVec2 &val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamVec3(const Interface &pm, const char *name, physx::PxVec3 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamVec3(Interface &pm, const char *name, const physx::PxVec3 &val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamVec4(const Interface &pm, const char *name, physx::PxVec4 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamVec4(Interface &pm, const char *name, const physx::PxVec4 &val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamQuat(const Interface &pm, const char *name, physx::PxQuat &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamQuat(Interface &pm, const char *name, const physx::PxQuat &val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamMat33(const Interface &pm, const char *name, physx::PxMat33 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamMat33(Interface &pm, const char *name, const physx::PxMat33 &val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamMat34(const Interface &pm, const char *name, physx::PxMat44 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamMat34(Interface &pm, const char *name, const physx::PxMat44 &val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamMat44(const Interface &pm, const char *name, physx::PxMat44 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamMat44(Interface &pm, const char *name, const physx::PxMat44 &val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamBounds3(const Interface &pm, const char *name, physx::PxBounds3 &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamBounds3(Interface &pm, const char *name, const physx::PxBounds3 &val) ;

/// helper function to get an NxParameterized value
PX_INLINE bool getParamTransform(const Interface &pm, const char *name, physx::PxTransform &val);
/// helper function to set an NxParameterized value
PX_INLINE bool setParamTransform(Interface &pm, const char *name, const physx::PxTransform &val) ;

} // namespace NxParameterized


#include "NxParamUtils.inl"

#endif // NX_PARAM_UTILS_H
