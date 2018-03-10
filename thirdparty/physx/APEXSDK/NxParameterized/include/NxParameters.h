/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_PARAMETERS_H
#define NX_PARAMETERS_H

#include "foundation/PxMat33.h"
#include "PxMat34Legacy.h"

#include "PsMutex.h"
#include "PsAllocator.h"
#include "PsShare.h"

#include "NxParameterized.h"
#include "NxParametersTypes.h"

#define NX_PARAM_PLACEMENT_NEW(p, T)        new(p) T

namespace NxParameterized
{
	typedef physx::MutexT<physx::RawAllocator> MutexType;

	const char *typeToStr(DataType type);
	DataType strToType(const char *str);

	PX_INLINE static bool IsAligned(const void *p, physx::PxU32 border)
	{
		return !(reinterpret_cast<size_t>(p) % border); //size_t == uintptr_t
	}

	PX_INLINE static bool IsAligned(physx::PxU32 x, physx::PxU32 border)
	{
		return !(x % border);
	}

    // Used for associating useful info with parameters (e.g. min, max, step, etc...)
    class HintImpl : public Hint
    {
    public:

        HintImpl();
        HintImpl(const char *name, physx::PxU64 value);
        HintImpl(const char *name, physx::PxF64 value);
        HintImpl(const char *name, const char *value);
        virtual ~HintImpl();

        void init(const char *name, physx::PxU64 value, bool static_allocation = false);
        void init(const char *name, physx::PxF64 value, bool static_allocation = false);
        void init(const char *name, const char *value, bool static_allocation = false);
        void cleanup();

        const char *name(void) const { return(mName); }
        DataType type(void) const { return(mType); }

		physx::PxU64 asUInt(void) const;
		physx::PxF64 asFloat(void) const;
		const char *asString(void) const;
		bool setAsUInt(physx::PxU64 v);

     private:

        bool mStaticAllocation;
        char *mName;
        DataType mType;

        union
        {
            physx::PxU64 mUIntValue;
            physx::PxF64 mFloatValue;
            char *mStringValue;
        };
    };


    class DefinitionImpl : public Definition
    {
    public:

        DefinitionImpl(Traits &traits, bool staticAlloc = true);
        DefinitionImpl(const char *name, DataType t, const char *structName, Traits &traits, bool staticAlloc = true);
        virtual ~DefinitionImpl();

        // can be used instead of the constructors and destructor
        void init(const char *name, DataType t, const char *structName, bool static_allocation = false);
        void cleanup(void);
        void destroy();

        const char *name(void) const 
		{ 
			return(mName); 
		}
        const char *longName(void) const { return(mLongName); }
        const char *structName(void) const { return(mStructName); }
        DataType type(void) const { return(mType); }
		const char* typeString() const { return typeToStr(mType); }

        physx::PxI32 arrayDimension(void) const;
        physx::PxI32 arraySize(physx::PxI32 dimension = 0) const;
        bool arraySizeIsFixed(void) const;
        bool setArraySize(physx::PxI32 size); // -1 if the size is not fixed

        bool isIncludedRef(void) const;

        bool isLeaf(void) const { return(type() != TYPE_STRUCT && type() != TYPE_ARRAY); }

        const Definition *parent(void) const { return(mParent); }
        const Definition *root(void) const;

        // Only used with parameters of TYPE_STRUCT or TYPE_ARRAY
        physx::PxI32 numChildren(void) const;
        const Definition *child(physx::PxI32 index) const;
        const Definition *child(const char *name, physx::PxI32 &index) const; // only used with TYPE_STRUCT
        void setChildren(Definition **children, physx::PxI32 n);
        void addChild(Definition *child);

        physx::PxI32 numHints(void) const;
        const Hint *hint(physx::PxI32 index) const;
        const Hint *hint(const char *name) const;
        void setHints(const Hint **hints, physx::PxI32 n);
        void addHint(Hint *hint);

        physx::PxI32 numEnumVals(void) const;
		physx::PxI32 enumValIndex( const char * enum_val ) const;

        // The pointers returned by enumVal() are good for the lifetime of the DefinitionImpl object.
        const char *enumVal(physx::PxI32 index) const;
        void setEnumVals(const char **enum_vals, physx::PxI32 n);
        void addEnumVal(const char *enum_val);


		physx::PxI32 numRefVariants(void) const;
		physx::PxI32 refVariantValIndex( const char * ref_val ) const;

		// The pointers returned by refVariantVal() are good for the lifetime of the DefinitionImpl object.
		const char * refVariantVal(physx::PxI32 index) const;
        void setRefVariantVals(const char **ref_vals, physx::PxI32 n);
        void addRefVariantVal(const char *ref_val);

		physx::PxU32 alignment(void) const;
		void setAlignment(physx::PxU32 align);

		physx::PxU32 padding(void) const;
		void setPadding(physx::PxU32 align);

		void setDynamicHandleIndicesMap(const physx::PxU8 *indices, physx::PxU32 numIndices);
		const physx::PxU8 * getDynamicHandleIndicesMap(physx::PxU32 &outNumIndices) const;

		bool isSimpleType(bool simpleStructs, bool simpleStrings) const;

    private:

        enum { MAX_NAME_LEN = 256 };

        void setDefaults(void);

        bool mStaticAllocation;

        const char * mName;
        const char * mLongName;
        const char * mStructName;
        DataType mType;
        physx::PxI32 mArraySize;

        DefinitionImpl *mParent;

        physx::PxI32 mNumChildren;
        Definition **mChildren;

        physx::PxI32 mNumHints;
        HintImpl **mHints;

        physx::PxI32 mNumEnumVals;
        char **mEnumVals;

		physx::PxI32 mNumRefVariants;
        char **mRefVariantVals;

		Traits *mTraits;
		bool mLongNameAllocated;

		physx::PxU32 mAlign;
		physx::PxU32 mPad;

		physx::PxU32 mNumDynamicHandleIndices;
		const physx::PxU8 *mDynamicHandleIndices;
    };

	// Used by generated code
	struct ParamLookupNode
	{
		NxParameterized::DataType type;
		bool isDynamicArrayRoot;
		size_t offset;
		const size_t* children;
		int numChildren;
	};

	//Constructs temporary object and extract vptr
	template<typename T, size_t T_align>
	const char* getVptr()
	{
		char buf[T_align + sizeof(T)];

		// We want all isAllocated false
		memset(buf, false, sizeof(buf));

		// Align
		char* bufAligned = (char*)((size_t)(buf + T_align) & ~(T_align - 1));

		// Call "fast" constructor (buf and refCount != NULL)
		// which does not allocate members
		T* tmp = NX_PARAM_PLACEMENT_NEW(bufAligned, T)(0, reinterpret_cast<void*>(16), reinterpret_cast<physx::PxI32*>(16));

		// vptr is usually stored at the beginning of object...
		const char* vptr = *reinterpret_cast<const char**>(tmp);

		//Cleanup
		tmp->~T();

		return vptr;
	}

class NxParameters : public NxParameterized::Interface
{
public:

    NxParameters(Traits *traits, void *buf = 0, physx::PxI32 *refCount = 0); //Long form is used for inplace objects
    virtual ~NxParameters();

	// placement delete
	virtual void destroy();
	virtual void initDefaults(void) { };
	virtual void initRandom(void);

	virtual const char * className(void) const { return mClassName; }
	virtual void setClassName(const char *name);

	virtual const char * name(void) const { return mName; }
	virtual void setName(const char *name);

	virtual physx::PxU32 version(void) const { return 0; }
	virtual physx::PxU16 getMajorVersion(void) const;
	virtual physx::PxU16 getMinorVersion(void) const;

	virtual const physx::PxU32 * checksum(physx::PxU32 &bits) const
	{
		bits = 0;
		return 0;
	}

    physx::PxI32 numParameters(void);
    const Definition *parameterDefinition(physx::PxI32 index);

    const Definition *rootParameterDefinition(void);
	const Definition *rootParameterDefinition(void) const;

    // Given a long name like "mystruct.somearray[10].foo", it will return
    // a handle to that specific parameter.  The handle can then be used to
    // set/get values, as long as it's a handle to a leaf node.
    virtual ErrorType getParameterHandle(const char *long_name, Handle &handle) const;
	virtual ErrorType getParameterHandle(const char *long_name, Handle &handle);

	virtual void setSerializationCallback(SerializationCallback *cb, void *userData = NULL);
	virtual ErrorType callPreSerializeCallback() const;

	ErrorType initParamRef(const Handle &handle, const char *chosenRefStr = 0, bool doDestroyOld = false);

    // These functions wrap the raw(Get|Set)XXXXX() methods.  They deal with
    // error handling and casting.
    ErrorType getParamBool(const Handle &handle, bool &val) const;
    ErrorType setParamBool(const Handle &handle, bool val);
    ErrorType getParamBoolArray(const Handle &handle, bool *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamBoolArray(const Handle &handle, const bool *array, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType getParamString(const Handle &handle, const char *&val) const;
    ErrorType setParamString(const Handle &handle, const char *val);
    ErrorType getParamStringArray(const Handle &handle, char **array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamStringArray(const Handle &handle, const char **array, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType getParamEnum(const Handle &handle, const char *&val) const;
    ErrorType setParamEnum(const Handle &handle, const char *val);
    ErrorType getParamEnumArray(const Handle &handle, char **array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamEnumArray(const Handle &handle, const char **array, physx::PxI32 n, physx::PxI32 offset= 0);

	ErrorType getParamRef(const Handle &handle, NxParameterized::Interface *&val) const;
    ErrorType setParamRef(const Handle &handle, NxParameterized::Interface * val, bool doDestroyOld = false);
    ErrorType getParamRefArray(const Handle &handle, NxParameterized::Interface **array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamRefArray(const Handle &handle, /*const*/ NxParameterized::Interface **array, physx::PxI32 n, physx::PxI32 offset = 0, bool doDestroyOld = false);

	ErrorType getParamI8(const Handle &handle, physx::PxI8 &val) const;
    ErrorType setParamI8(const Handle &handle, physx::PxI8 val);
    ErrorType getParamI8Array(const Handle &handle, physx::PxI8 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamI8Array(const Handle &handle, const physx::PxI8 *val, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType getParamI16(const Handle &handle, physx::PxI16 &val) const;
    ErrorType setParamI16(const Handle &handle, physx::PxI16 val);
    ErrorType getParamI16Array(const Handle &handle, physx::PxI16 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamI16Array(const Handle &handle, const physx::PxI16 *val, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType getParamI32(const Handle &handle, physx::PxI32 &val) const;
    ErrorType setParamI32(const Handle &handle, physx::PxI32 val);
    ErrorType getParamI32Array(const Handle &handle, physx::PxI32 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamI32Array(const Handle &handle, const physx::PxI32 *val, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType getParamI64(const Handle &handle, physx::PxI64 &val) const;
    ErrorType setParamI64(const Handle &handle, physx::PxI64 val);
    ErrorType getParamI64Array(const Handle &handle, physx::PxI64 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamI64Array(const Handle &handle, const physx::PxI64 *val, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType getParamU8(const Handle &handle, physx::PxU8 &val) const;
    ErrorType setParamU8(const Handle &handle, physx::PxU8 val);
    ErrorType getParamU8Array(const Handle &handle, physx::PxU8 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamU8Array(const Handle &handle, const physx::PxU8 *val, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType getParamU16(const Handle &handle, physx::PxU16 &val) const;
    ErrorType setParamU16(const Handle &handle, physx::PxU16 val);
    ErrorType getParamU16Array(const Handle &handle, physx::PxU16 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamU16Array(const Handle &handle, const physx::PxU16 *array, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType getParamU32(const Handle &handle, physx::PxU32 &val) const;
    ErrorType setParamU32(const Handle &handle, physx::PxU32 val);
    ErrorType getParamU32Array(const Handle &handle, physx::PxU32 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamU32Array(const Handle &handle, const physx::PxU32 *array, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType getParamU64(const Handle &handle, physx::PxU64 &val) const;
    ErrorType setParamU64(const Handle &handle, physx::PxU64 val);
    ErrorType getParamU64Array(const Handle &handle, physx::PxU64 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamU64Array(const Handle &handle, const physx::PxU64 *array, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType getParamF32(const Handle &handle, physx::PxF32 &val) const;
    ErrorType setParamF32(const Handle &handle, physx::PxF32 val);
    ErrorType getParamF32Array(const Handle &handle, physx::PxF32 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamF32Array(const Handle &handle, const physx::PxF32 *array, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType getParamF64(const Handle &handle, physx::PxF64 &val) const;
    ErrorType setParamF64(const Handle &handle, physx::PxF64 val);
    ErrorType getParamF64Array(const Handle &handle, physx::PxF64 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamF64Array(const Handle &handle, const physx::PxF64 *array, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType setParamVec2(const Handle &handle, const physx::PxVec2 &val);
    ErrorType getParamVec2(const Handle &handle, physx::PxVec2 &val) const;
    ErrorType getParamVec2Array(const Handle &handle, physx::PxVec2 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamVec2Array(const Handle &handle, const physx::PxVec2 *array, physx::PxI32 n, physx::PxI32 offset= 0);

	ErrorType setParamVec3(const Handle &handle, const physx::PxVec3 &val);
    ErrorType getParamVec3(const Handle &handle, physx::PxVec3 &val) const;
    ErrorType getParamVec3Array(const Handle &handle, physx::PxVec3 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamVec3Array(const Handle &handle, const physx::PxVec3 *array, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType setParamVec4(const Handle &handle, const physx::PxVec4 &val);
    ErrorType getParamVec4(const Handle &handle, physx::PxVec4 &val) const;
    ErrorType getParamVec4Array(const Handle &handle, physx::PxVec4 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamVec4Array(const Handle &handle, const physx::PxVec4 *array, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType setParamQuat(const Handle &handle, const physx::PxQuat &val);
    ErrorType getParamQuat(const Handle &handle, physx::PxQuat &val) const;
    ErrorType getParamQuatArray(const Handle &handle, physx::PxQuat *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamQuatArray(const Handle &handle, const physx::PxQuat *array, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType setParamMat33(const Handle &handle, const physx::PxMat33 &val);
    ErrorType getParamMat33(const Handle &handle, physx::PxMat33 &val) const;
    ErrorType getParamMat33Array(const Handle &handle, physx::PxMat33 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamMat33Array(const Handle &handle, const physx::PxMat33 *array, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType setParamMat34(const Handle &handle, const physx::PxMat34Legacy &val);
    ErrorType getParamMat34(const Handle &handle, physx::PxMat34Legacy &val) const;
    ErrorType getParamMat34Array(const Handle &handle, physx::PxMat34Legacy *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamMat34Array(const Handle &handle, const physx::PxMat34Legacy *array, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType setParamMat34(const Handle &handle, const physx::PxMat44 &val);
    ErrorType getParamMat34(const Handle &handle, physx::PxMat44 &val) const;
    ErrorType getParamMat34Array(const Handle &handle, physx::PxMat44 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamMat34Array(const Handle &handle, const physx::PxMat44 *array, physx::PxI32 n, physx::PxI32 offset= 0);

	ErrorType setParamMat44(const Handle &handle, const physx::PxMat44 &val);
    ErrorType getParamMat44(const Handle &handle, physx::PxMat44 &val) const;
    ErrorType getParamMat44Array(const Handle &handle, physx::PxMat44 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamMat44Array(const Handle &handle, const physx::PxMat44 *array, physx::PxI32 n, physx::PxI32 offset= 0);

	ErrorType setParamBounds3(const Handle &handle, const physx::PxBounds3 &val);
    ErrorType getParamBounds3(const Handle &handle, physx::PxBounds3 &val) const;
    ErrorType getParamBounds3Array(const Handle &handle, physx::PxBounds3 *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
    ErrorType setParamBounds3Array(const Handle &handle, const physx::PxBounds3 *array, physx::PxI32 n, physx::PxI32 offset= 0);

	ErrorType setParamTransform(const Handle &handle, const physx::PxTransform &val);
	ErrorType getParamTransform(const Handle &handle, physx::PxTransform &val) const;
	ErrorType getParamTransformArray(const Handle &handle, physx::PxTransform *array, physx::PxI32 n, physx::PxI32 offset = 0) const;
	ErrorType setParamTransformArray(const Handle &handle, const physx::PxTransform *array, physx::PxI32 n, physx::PxI32 offset= 0);

    ErrorType valueToStr(const Handle &array_handle, char *str, physx::PxU32 n, const char *&ret);
    ErrorType strToValue(Handle &handle,const char *str,const char **endptr); // assigns this string to the value

    ErrorType resizeArray(const Handle &array_handle, physx::PxI32 new_size);
    ErrorType getArraySize(const Handle &array_handle, physx::PxI32 &size, physx::PxI32 dimension = 0) const;
	ErrorType swapArrayElements(const Handle &array_handle, physx::PxU32 firstElement, physx::PxU32 secondElement);

    bool equals(const NxParameterized::Interface &obj, Handle* handleOfInequality, physx::PxU32 numHandlesOfInequality, bool doCompareNotSerialized = true) const;

    bool areParamsOK(Handle *invalidHandles = NULL, physx::PxU32 numInvalidHandles = 0);

	ErrorType copy(const NxParameterized::Interface &other);

	ErrorType clone(NxParameterized::Interface *&obj) const;

    static physx::PxVec2 		getVec2(const char *str, const char **endptr);
    static physx::PxVec3 		getVec3(const char *str, const char **endptr);
    static physx::PxVec4 		getVec4(const char *str, const char **endptr);
    static physx::PxQuat 		getQuat(const char *str, const char **endptr);
    static physx::PxMat33		getMat33(const char *str, const char **endptr);
    static physx::PxMat34Legacy getMat34(const char *str, const char **endptr);
    static physx::PxMat44 		getMat44(const char *str, const char **endptr);
    static physx::PxBounds3     getBounds3(const char *str, const char **endptr);
    static physx::PxTransform   getTransform(const char *str, const char **endptr);

	static physx::PxVec2		init(physx::PxF32 x,physx::PxF32 y);
	static physx::PxVec3		init(physx::PxF32 x,physx::PxF32 y,physx::PxF32 z);
	static physx::PxVec4		initVec4(physx::PxF32 x,physx::PxF32 y,physx::PxF32 z,physx::PxF32 w);
	static physx::PxQuat		init(physx::PxF32 x,physx::PxF32 y,physx::PxF32 z,physx::PxF32 w);
	static physx::PxMat33		init(physx::PxF32 _11,physx::PxF32 _12,physx::PxF32 _13,physx::PxF32 _21,physx::PxF32 _22,physx::PxF32 _23,physx::PxF32 _31,physx::PxF32 _32,physx::PxF32 _33);
	static physx::PxMat34Legacy	init(physx::PxF32 _11,physx::PxF32 _12,physx::PxF32 _13,physx::PxF32 _21,physx::PxF32 _22,physx::PxF32 _23,physx::PxF32 _31,physx::PxF32 _32,physx::PxF32 _33,physx::PxF32 x,physx::PxF32 y,physx::PxF32 z);
	static physx::PxMat44		init(physx::PxF32 _11,physx::PxF32 _12,physx::PxF32 _13,physx::PxF32 _14,physx::PxF32 _21,physx::PxF32 _22,physx::PxF32 _23,physx::PxF32 _24,physx::PxF32 _31,physx::PxF32 _32,physx::PxF32 _33,physx::PxF32 _34,physx::PxF32 _41,physx::PxF32 _42,physx::PxF32 _43,physx::PxF32 _44);
	static physx::PxBounds3		init(physx::PxF32 minx,physx::PxF32 miny,physx::PxF32 minz,physx::PxF32 maxx,physx::PxF32 maxy,physx::PxF32 maxz);
	static physx::PxTransform	init(physx::PxF32 x,physx::PxF32 y,physx::PxF32 z,physx::PxF32 qx,physx::PxF32 qy,physx::PxF32 qz,physx::PxF32 qw);

	static physx::PxI32 MultIntArray(const physx::PxI32 *array, physx::PxI32 n);
	static ErrorType resizeArray(Traits *parameterizedTraits,
                                 void *&buf,
                                 physx::PxI32 *array_sizes,
                                 physx::PxI32 dimension,
                                 physx::PxI32 resize_dim,
                                 physx::PxI32 new_size,
								 bool doFree,
                                 physx::PxI32 element_size,
                                 physx::PxU32 element_align,
								 bool &isMemAllocated);
	static void recursiveCopy(const void *src,
							  const physx::PxI32 *src_sizes,
							  void *dst,
							  const physx::PxI32 *dst_sizes,
							  physx::PxI32 dimension,
							  physx::PxI32 element_size,
							  physx::PxI32 *indexes = NULL,
							  physx::PxI32 level = 0);

	virtual void getVarPtr(const Handle &, void *&ptr, size_t &offset) const
	{
		PX_ALWAYS_ASSERT();
		ptr = 0;
		offset = 0;
	}

	Traits * getTraits(void) const
	{
		return mParameterizedTraits;
	}

	virtual bool checkAlignments() const;

protected:

	void *getVarPtrHelper(const ParamLookupNode *rootNode, void *paramStruct, const Handle &handle, size_t &offset) const;

	static void destroy(NxParameters *obj, NxParameterized::Traits *traits, bool doDeallocateSelf, physx::PxI32 *refCount, void *buf);

    // All classes deriving from NxParameterized must overload this function.  It
    // returns the parameter definition tree.  The root node must be a struct
    // with an empty string for its name.
	virtual const Definition *getParameterDefinitionTree(void) {return NULL;}
	virtual const Definition *getParameterDefinitionTree(void) const {return NULL;}

	ErrorType releaseDownsizedParameters(const Handle &handle, int newSize, int oldSize);
	ErrorType initNewResizedParameters(const Handle &handle, int newSize, int oldSize);
	virtual ErrorType rawResizeArray(const Handle &handle, int new_size);
	virtual ErrorType rawGetArraySize(const Handle &array_handle, int &size, int dimension) const;
	virtual ErrorType rawSwapArrayElements(const Handle &array_handle, unsigned int firstElement, unsigned int secondElement);

    // The methods for the types that are supported by the class deriving from
    // NxParameterized must be overloaded.
    virtual ErrorType rawSetParamBool(const Handle &handle, bool val);
    virtual ErrorType rawGetParamBool(const Handle &handle, bool &val) const;
    virtual ErrorType rawGetParamBoolArray(const Handle &handle, bool *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamBoolArray(const Handle &handle, const bool *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawGetParamString(const Handle &handle, const char *&val) const;
    virtual ErrorType rawSetParamString(const Handle &handle, const char *val);
	virtual ErrorType rawGetParamStringArray(const Handle &handle, char **array, physx::PxI32 n, physx::PxI32 offset) const;
	virtual ErrorType rawSetParamStringArray(const Handle &handle, const char **array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawGetParamEnum(const Handle &handle, const char *&val) const;
    virtual ErrorType rawSetParamEnum(const Handle &handle, const char *val);
	virtual ErrorType rawGetParamEnumArray(const Handle &handle, char **array, physx::PxI32 n, physx::PxI32 offset) const;
	virtual ErrorType rawSetParamEnumArray(const Handle &handle, const char **array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawGetParamRef(const Handle &handle, NxParameterized::Interface *&val) const;
    virtual ErrorType rawSetParamRef(const Handle &handle, NxParameterized::Interface * val);
	virtual ErrorType rawGetParamRefArray(const Handle &handle, NxParameterized::Interface **array, physx::PxI32 n, physx::PxI32 offset) const;
	virtual ErrorType rawSetParamRefArray(const Handle &handle, /*const*/ NxParameterized::Interface **array, physx::PxI32 n, physx::PxI32 offset);

	virtual ErrorType rawGetParamI8(const Handle &handle, physx::PxI8 &val) const;
    virtual ErrorType rawSetParamI8(const Handle &handle, physx::PxI8 val);
    virtual ErrorType rawGetParamI8Array(const Handle &handle, physx::PxI8 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamI8Array(const Handle &handle, const physx::PxI8 *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawGetParamI16(const Handle &handle, physx::PxI16 &val) const;
    virtual ErrorType rawSetParamI16(const Handle &handle, physx::PxI16 val);
    virtual ErrorType rawGetParamI16Array(const Handle &handle, physx::PxI16 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamI16Array(const Handle &handle, const physx::PxI16 *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawGetParamI32(const Handle &handle, physx::PxI32 &val) const;
    virtual ErrorType rawSetParamI32(const Handle &handle, physx::PxI32 val);
    virtual ErrorType rawGetParamI32Array(const Handle &handle, physx::PxI32 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamI32Array(const Handle &handle, const physx::PxI32 *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawGetParamI64(const Handle &handle, physx::PxI64 &val) const;
    virtual ErrorType rawSetParamI64(const Handle &handle, physx::PxI64 val);
    virtual ErrorType rawGetParamI64Array(const Handle &handle, physx::PxI64 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamI64Array(const Handle &handle, const physx::PxI64 *array, physx::PxI32 n, physx::PxI32 offset);


    virtual ErrorType rawGetParamU8(const Handle &handle, physx::PxU8 &val) const;
    virtual ErrorType rawSetParamU8(const Handle &handle, physx::PxU8 val);
    virtual ErrorType rawGetParamU8Array(const Handle &handle, physx::PxU8 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamU8Array(const Handle &handle, const physx::PxU8 *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawGetParamU16(const Handle &handle, physx::PxU16 &val) const;
    virtual ErrorType rawSetParamU16(const Handle &handle, physx::PxU16 val);
    virtual ErrorType rawGetParamU16Array(const Handle &handle, physx::PxU16 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamU16Array(const Handle &handle, const physx::PxU16 *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawGetParamU32(const Handle &handle, physx::PxU32 &val) const;
    virtual ErrorType rawSetParamU32(const Handle &handle, physx::PxU32 val);
    virtual ErrorType rawGetParamU32Array(const Handle &handle, physx::PxU32 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamU32Array(const Handle &handle, const physx::PxU32 *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawSetParamU64(const Handle &handle, physx::PxU64 val);
    virtual ErrorType rawGetParamU64(const Handle &handle, physx::PxU64 &val) const;
    virtual ErrorType rawGetParamU64Array(const Handle &handle, physx::PxU64 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamU64Array(const Handle &handle, const physx::PxU64 *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawGetParamF32(const Handle &handle, physx::PxF32 &val) const;
    virtual ErrorType rawSetParamF32(const Handle &handle, physx::PxF32 val);
    virtual ErrorType rawGetParamF32Array(const Handle &handle, physx::PxF32 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamF32Array(const Handle &handle, const physx::PxF32 *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawGetParamF64(const Handle &handle, physx::PxF64 &val) const;
    virtual ErrorType rawSetParamF64(const Handle &handle, physx::PxF64 val);
    virtual ErrorType rawGetParamF64Array(const Handle &handle, physx::PxF64 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamF64Array(const Handle &handle, const physx::PxF64 *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawSetParamVec2(const Handle &handle,physx::PxVec2 val);
    virtual ErrorType rawGetParamVec2(const Handle &handle,physx::PxVec2 &val) const;
    virtual ErrorType rawGetParamVec2Array(const Handle &handle,physx::PxVec2 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamVec2Array(const Handle &handle, const physx::PxVec2 *array, physx::PxI32 n, physx::PxI32 offset);

	virtual ErrorType rawSetParamVec3(const Handle &handle,physx::PxVec3 val);
    virtual ErrorType rawGetParamVec3(const Handle &handle,physx::PxVec3 &val) const;
    virtual ErrorType rawGetParamVec3Array(const Handle &handle,physx::PxVec3 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamVec3Array(const Handle &handle, const physx::PxVec3 *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawSetParamVec4(const Handle &handle,physx::PxVec4 val);
    virtual ErrorType rawGetParamVec4(const Handle &handle,physx::PxVec4 &val) const;
    virtual ErrorType rawGetParamVec4Array(const Handle &handle,physx::PxVec4 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamVec4Array(const Handle &handle, const physx::PxVec4 *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawSetParamQuat(const Handle &handle,physx::PxQuat val);
    virtual ErrorType rawGetParamQuat(const Handle &handle,physx::PxQuat &val) const;
    virtual ErrorType rawGetParamQuatArray(const Handle &handle,physx::PxQuat *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamQuatArray(const Handle &handle, const physx::PxQuat *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawSetParamMat33(const Handle &handle,physx::PxMat33 val);
    virtual ErrorType rawGetParamMat33(const Handle &handle,physx::PxMat33 &val) const;
    virtual ErrorType rawGetParamMat33Array(const Handle &handle,physx::PxMat33 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamMat33Array(const Handle &handle, const physx::PxMat33 *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawSetParamMat34(const Handle &handle,physx::PxMat34Legacy val);
    virtual ErrorType rawGetParamMat34(const Handle &handle,physx::PxMat34Legacy &val) const;
	virtual ErrorType rawGetParamMat34Array(const Handle &handle,physx::PxMat34Legacy *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamMat34Array(const Handle &handle, const physx::PxMat34Legacy *array, physx::PxI32 n, physx::PxI32 offset);

    virtual ErrorType rawSetParamMat44(const Handle &handle,physx::PxMat44 val);
    virtual ErrorType rawGetParamMat44(const Handle &handle,physx::PxMat44 &val) const;
    virtual ErrorType rawGetParamMat44Array(const Handle &handle,physx::PxMat44 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamMat44Array(const Handle &handle, const physx::PxMat44 *array, physx::PxI32 n, physx::PxI32 offset);

	virtual ErrorType rawSetParamBounds3(const Handle &handle,physx::PxBounds3 val);
    virtual ErrorType rawGetParamBounds3(const Handle &handle,physx::PxBounds3 &val) const;
    virtual ErrorType rawGetParamBounds3Array(const Handle &handle,physx::PxBounds3 *array, physx::PxI32 n, physx::PxI32 offset) const;
    virtual ErrorType rawSetParamBounds3Array(const Handle &handle, const physx::PxBounds3 *array, physx::PxI32 n, physx::PxI32 offset);

	virtual ErrorType rawSetParamTransform(const Handle &handle,physx::PxTransform val);
	virtual ErrorType rawGetParamTransform(const Handle &handle,physx::PxTransform &val) const;
	virtual ErrorType rawGetParamTransformArray(const Handle &handle,physx::PxTransform *array, physx::PxI32 n, physx::PxI32 offset) const;
	virtual ErrorType rawSetParamTransformArray(const Handle &handle, const physx::PxTransform *array, physx::PxI32 n, physx::PxI32 offset);

	// WARNING!
	// Binary deserializer relies on layout of fields
	// If you change anything be sure to update it as well

	Traits *mParameterizedTraits;
	SerializationCallback *mSerializationCb;
	void *mCbUserData;
	void *mBuffer;
	physx::PxI32 *mRefCount;
	const char *mName;
	const char *mClassName;
	bool mDoDeallocateSelf; //if true - memory should be deallocated in destroy()
	bool mDoDeallocateName; //if true - mName is in inplace-buffer and should not be freed
	bool mDoDeallocateClassName; //see comment for mDoDeallocateName

private:

	void initRandom(NxParameterized::Handle& handle);

    bool equals(const NxParameterized::Interface &obj,
                Handle &param_handle,
				Handle *handlesOfInequality,
				physx::PxU32 numHandlesOfInequality,
				bool doCompareNotSerialized) const;

	bool checkAlignments(Handle &param_handle) const;

    bool areParamsOK(Handle &handle, Handle *invalidHandles, physx::PxU32 numInvalidHandles, physx::PxU32 &numRemainingHandles);

	ErrorType copy(const NxParameterized::Interface &other,
				   Handle &param_handle);

	// recursively call pre serialization callback
	ErrorType callPreSerializeCallback(Handle& handle) const;

    ErrorType checkParameterHandle(const Handle &handle) const;

};

} // end of namespace

#endif // NX_PARAMETERS_H
