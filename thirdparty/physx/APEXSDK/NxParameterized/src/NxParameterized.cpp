/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Px.h>

#if !defined(PX_PS4)
#pragma warning(disable:4996 4310)
#endif

#include "PsShare.h"
#include "foundation/PxAssert.h"
#include "PxAsciiConversion.h"
#include "NxParameterized.h"
#include "NxParameters.h"
#include "NxTraitsInternal.h"
#include "PsString.h"
#include "foundation/PxIntrinsics.h"
#include "PsMathUtils.h"
#include "foundation/PxTransform.h"
#include "foundation/PxMath.h"

namespace NxParameterized
{

#define NX_ERR_CHECK_RETURN(x) if(x!=ERROR_NONE) return(x);
#define NX_BOOL_RETURN(x) if((x)!=ERROR_NONE) return(false);

#define DEBUG_ASSERT(x)
//#define DEBUG_ASSERT(x) PX_ASSERT(x)

char Spog[] = "test";

#define COND_DUP(str) (mStaticAllocation) ? (char *)(str) : local_strdup(str)

static PX_INLINE void* allocAligned(NxParameterized::Traits* t, physx::PxU32 size, physx::PxU32 align)
{
	void* buf = t->alloc(size, align);
	if( (size_t)buf & (align - 1) )
	{
		t->free(buf);
		return 0;
	}

	return buf;
}

static PX_INLINE physx::PxF64 RandomF64()
{
	return (physx::PxF64)rand() / RAND_MAX;
}

static PX_INLINE physx::PxF32 RandomF32()
{
	return (physx::PxF32)RandomF64();
}

static PX_INLINE physx::PxU32 RandomIdx(physx::PxU32 m, physx::PxU32 M)
{
	return physx::PxU32(m + RandomF64() * (M - m) + 0.99); // FIXME: round
}

static PX_INLINE physx::PxVec2 RandomVec2()
{
	return physx::PxVec2(RandomF32(), RandomF32());
}

static PX_INLINE physx::PxVec3 RandomVec3()
{
	return physx::PxVec3(RandomF32(), RandomF32(), RandomF32());
}

static PX_INLINE physx::PxVec4 RandomVec4()
{
	return physx::PxVec4(RandomF32(), RandomF32(), RandomF32(), RandomF32());
}

static PX_INLINE physx::PxQuat RandomQuat()
{
	return physx::PxQuat(RandomF32(), RandomF32(), RandomF32(), RandomF32());
}

static PX_INLINE physx::PxU32 RandomU32()
{
	return (physx::PxU32)rand();
}

static PX_INLINE physx::PxU64 RandomU64()
{
	physx::PxU32 u32s[2];
	u32s[0] = RandomU32();
	u32s[1] = RandomU32();

	return *(physx::PxU64*)&u32s[0];
}

static PX_INLINE bool notEqual(const char *a, const char *b)
{
	if(a == NULL && b == NULL)
		return(false);

	return (a == NULL && b != NULL)
		|| (a != NULL && b == NULL)
		|| 0 != strcmp(a, b);
}

static PX_INLINE bool notEqual(physx::PxU8 a,physx::PxU8 b)
{
	return a!=b;
}
static PX_INLINE bool notEqual(physx::PxU16 a,physx::PxU16 b)
{
	return a!=b;
}

static PX_INLINE bool notEqual(physx::PxU32 a,physx::PxU32 b)
{
	return a!=b;
}

static PX_INLINE bool notEqual(physx::PxU64 a,physx::PxU64 b)
{
	return a!=b;
}

static PX_INLINE bool notEqual(physx::PxI8 a,physx::PxI8 b)
{
	return a!=b;
}
static PX_INLINE bool notEqual(physx::PxI16 a,physx::PxI16 b)
{
	return a!=b;
}

static PX_INLINE bool notEqual(physx::PxI32 a,physx::PxI32 b)
{
	return a!=b;
}

static PX_INLINE bool notEqual(physx::PxI64 a,physx::PxI64 b)
{
	return a!=b;
}

static PX_INLINE bool notEqual(physx::PxF32 a,physx::PxF32 b)
{
	return a!=b;
}

static PX_INLINE bool notEqual(physx::PxF64 a,physx::PxF64 b)
{
	return a!=b;
}

static PX_INLINE bool notEqual(bool a,bool b)
{
	return a!=b;
}

static PX_INLINE bool notEqual(const physx::PxVec2 &a,const physx::PxVec2 &b)
{
	return a!=b;
}

static PX_INLINE bool notEqual(const physx::PxVec3 &a,const physx::PxVec3 &b)
{
	return a!=b;
}

static PX_INLINE bool notEqual(const physx::PxVec4 &a,const physx::PxVec4 &b)
{
	return a!=b;
}

// This was formerly in the PxQuat.h file but was removed due to other code bases requiring it to be gone
static PX_INLINE bool operator!=( const physx::PxQuat& a, const physx::PxQuat& b )
{
	return a.x != b.x
		|| a.y != b.y
		|| a.z != b.z
		|| a.w != b.w;
}

static PX_INLINE bool notEqual(const physx::PxQuat &a,physx::PxQuat &b)
{
	return a != b;
}

static PX_INLINE bool notEqual(const physx::PxMat33 &a,physx::PxMat33 &b)
{
	const physx::PxReal *qa = a.front();
	const physx::PxReal *qb = b.front();
	return qa[0] != qb[0] || qa[1] != qb[1] || qa[2] != qb[2] || qa[3] != qb[3] ||
		   qa[4] != qb[4] || qa[5] != qb[5] || qa[6] != qb[6] || qa[7] != qb[7] ||
		   qa[8] != qb[8];
}

/*static PX_INLINE bool notEqual(const physx::PxMat34Legacy &a,physx::PxMat34Legacy &b)
{
	physx::PxF32 qa[9];
	physx::PxF32 qb[9];
	a.M.getColumnMajor(qa);
	b.M.getColumnMajor(qb);
	return a.t != b.t || qa[0] != qb[0] || qa[1] != qb[1] || qa[2] != qb[2] || qa[3] != qb[3] ||
		qa[4] != qb[4] || qa[5] != qb[5] || qa[6] != qb[6] || qa[7] != qb[7] ||
		qa[8] != qb[8];
}*/

static PX_INLINE bool notEqual(const physx::PxMat44 &a,physx::PxMat44 &b)
{
	return a.column0.w != b.column0.w || a.column0.x != b.column0.x || a.column0.y != b.column0.y || a.column0.z != b.column0.z ||
		a.column1.w != b.column1.w || a.column1.x != b.column1.x || a.column1.y != b.column1.y || a.column1.z != b.column1.z ||
		a.column2.w != b.column2.w || a.column2.x != b.column2.x || a.column2.y != b.column2.y || a.column2.z != b.column2.z ||
		a.column3.w != b.column3.w || a.column3.x != b.column3.x || a.column3.y != b.column3.y || a.column3.z != b.column3.z;
}

static PX_INLINE bool notEqual(const physx::PxBounds3 &a,const physx::PxBounds3 &b)
{
	return a.minimum != b.minimum || a.maximum != b.maximum;
}

static PX_INLINE bool notEqual(const physx::PxTransform &a,const physx::PxTransform &b)
{
	return a.p != b.p || a.q != b.q;
}

#define CHECK_FINITE(t) { \
	const t* _fs = (const t*)&val; \
	for(size_t _j = 0; _j < sizeof(val) / sizeof(t); ++_j) \
	{ \
		if( physx::PxIsFinite(_fs[_j]) ) continue; \
		char _longName[256]; \
		handle.getLongName(_longName, sizeof(_longName)); \
		NX_PARAM_TRAITS_WARNING(mParameterizedTraits, "%s: setting non-finite floating point value", _longName); \
		break; \
	} \
}

#define CHECK_FINITE_ARRAY(t) { \
	for(physx::PxI32 _i = 0; _i < physx::PxI32(n); ++_i) \
	{ \
		const t* _fs = (const t*)(&array[_i]); \
		for(size_t _j = 0; _j < sizeof(array[0]) / sizeof(t); ++_j) \
		{ \
			if( physx::PxIsFinite(_fs[_j]) ) continue; \
			char _longName[256]; \
			handle.getLongName(_longName, sizeof(_longName)); \
			NX_PARAM_TRAITS_WARNING(mParameterizedTraits, "%s[%d]: setting non-finite floating point value", _longName, (int)_i); \
			break; \
		} \
	} \
}

#define CHECK_F32_FINITE \
	CHECK_FINITE(physx::PxF32)

#define CHECK_F32_FINITE_ARRAY \
	CHECK_FINITE_ARRAY(physx::PxF32)

#define CHECK_F64_FINITE \
	CHECK_FINITE(physx::PxF64)

#define CHECK_F64_FINITE_ARRAY \
	CHECK_FINITE_ARRAY(physx::PxF64)

//******************************************************************************
//*** Local functions
//******************************************************************************

#if 0
static void *local_malloc(physx::PxU32 bytes)
{
	return(PX_ALLOC(bytes, PX_DEBUG_EXP("NxParameterized::local_malloc")));
}

static void *local_realloc(void *buf, physx::PxU32 bytes)
{
    return(NxGetApexAllocator()->realloc(buf, bytes));
}

static void local_free(void *buf)
{
    PX_FREE(buf);
}
#else

static void *local_malloc(physx::PxU32 bytes)
{
    return(malloc(bytes));
}

static void *local_realloc(void *buf, physx::PxU32 bytes)
{
    return(realloc(buf, bytes));
}

static void local_free(void *buf)
{
    free(buf);
}

#endif

static char *local_strdup(const char *str)
{
	if(str == NULL)
		return NULL;

	physx::PxU32 len = (physx::PxU32)strlen(str);

	char *result = (char *)local_malloc(sizeof(char) * (len + 1));
	physx::string::strncpy_s(result, len+1, str, len);
	return result;
}

static void local_strncpy(char *dst, const char *src, physx::PxU32 n)
{
	physx::string::strncpy_s(dst, n, src, strlen(src) + 1);
}


static void local_strncat(char *dst, const char *src, physx::PxU32 n)
{
	physx::string::strncat_s(dst, (physx::PxI32)strlen(dst) + (physx::PxI32)strlen(src) + 1,src, n);
}

static physx::PxI32 safe_strcmp(const char *str1, const char *str2)
{
	if( str1 != NULL && str2 != NULL )
		return strcmp(str1, str2);
	else if( str1 == NULL && str2 == NULL )
		return 0;
	else
		return -1;
}

void *dupBuf(void *buf, physx::PxU32 n)
{
    PX_ASSERT(buf != NULL);
    PX_ASSERT(n > 0);

    void *Ret = local_malloc(n);
    memcpy(Ret, buf, n);

    return(Ret);
}

DataType strToType(const char *str)
{
#   define NX_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
        if(!strcmp(str, #type_name)) \
            return(TYPE_##enum_name);

#   include "NxParameterized_types.h"

	if(!strcmp(str, "Pointer"))
		return(TYPE_POINTER);

    return(TYPE_UNDEFINED);
}

const char *typeToStr(DataType type)
{
    switch(type)
    {

#   define NX_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
        case TYPE_##enum_name : \
            return(#type_name);
#       include "NxParameterized_types.h"
		
		case TYPE_POINTER:
			return "Pointer";

		default:
			return NULL;
    }
}

//******************************************************************************
//*** class HintImpl
//******************************************************************************

HintImpl::HintImpl()
{
    mStaticAllocation = true;
}

HintImpl::HintImpl(const char *name, physx::PxU64 value)
{
    PX_ASSERT(name != NULL);

    init(name, value);
}

HintImpl::HintImpl(const char *name, physx::PxF64 value)
{
    PX_ASSERT(name != NULL);

    init(name, value);
}

HintImpl::HintImpl(const char *name, const char *value)
{
    PX_ASSERT(name != NULL);
    PX_ASSERT(value != NULL);

    init(name, value);
}

HintImpl::~HintImpl()
{
    cleanup();
}

void HintImpl::init(const char *name, physx::PxU64 value, bool static_allocation)
{
    PX_ASSERT(name != NULL);

    mStaticAllocation = static_allocation;
    mType = TYPE_U64;
    mName = COND_DUP(name);
    mUIntValue = value;
}

void HintImpl::init(const char *name, physx::PxF64 value, bool static_allocation)
{
    PX_ASSERT(name != NULL);

    mStaticAllocation = static_allocation;
    mType = TYPE_F64;
    mName = COND_DUP(name);
    mFloatValue = value;
}

void HintImpl::init(const char *name, const char *value, bool static_allocation)
{
    PX_ASSERT(name != NULL);
    PX_ASSERT(value != NULL);

    mStaticAllocation = static_allocation;
    mType = TYPE_STRING;
    mName = COND_DUP(name);
    mStringValue = COND_DUP(value);
}

void HintImpl::cleanup(void)
{
    PX_ASSERT(mName != NULL);

    if(!mStaticAllocation)
    {
        local_free(mName);

        if(type() == TYPE_STRING)
        {
            PX_ASSERT(mStringValue != NULL);
            local_free(mStringValue);
        }
    }
}

bool HintImpl::setAsUInt(physx::PxU64 v) 
{
	bool ret = false;
  PX_ASSERT(type() == TYPE_U64);
  if ( type() == TYPE_U64 )
  {
	mUIntValue = v;
	ret = true;
  }
  return ret;
}

physx::PxU64 HintImpl::asUInt(void) const
{
	PX_ASSERT(type() == TYPE_U64);
	return(mUIntValue);
}

physx::PxF64 HintImpl::asFloat(void) const
{
  PX_ASSERT(type() == TYPE_U64 || type() == TYPE_F64);
  return(type() == TYPE_U64 ? static_cast<physx::PxI64>(mUIntValue) : mFloatValue);
}

const char *HintImpl::asString(void)const
{
  PX_ASSERT(type() == TYPE_STRING);
  return(mStringValue);
}


//******************************************************************************
//*** class Handle
//******************************************************************************



//******************************************************************************
//*** class DefinitionImpl
//******************************************************************************

void DefinitionImpl::setDefaults(void)
{
    mName = NULL;

	mLongName = NULL;
	mLongNameAllocated = false;

	mStructName = NULL;

    mType = TYPE_UNDEFINED;

    mArraySize = 0;

    mParent = NULL;

    mNumChildren = 0;
    mChildren = NULL;

    mNumHints = 0;
    mHints = NULL;

    mEnumVals = NULL;
    mNumEnumVals = 0;

	mNumRefVariants = 0;
	mRefVariantVals = NULL;

	mNumDynamicHandleIndices = 0;
	mDynamicHandleIndices = NULL;

	mAlign = mPad = 0;
}

DefinitionImpl::DefinitionImpl(Traits &traits, bool staticAlloc)
{
    mStaticAllocation = staticAlloc;
	mTraits = &traits;
    setDefaults();
}

DefinitionImpl::DefinitionImpl(const char *name, DataType t, const char *structName, Traits &traits, bool staticAlloc)
{
    mStaticAllocation = staticAlloc;
	mTraits = &traits;
	mLongNameAllocated = false;
    init(name, t, structName, false);
}


DefinitionImpl::~DefinitionImpl()
{
    cleanup();
}

void DefinitionImpl::setAlignment(physx::PxU32 align)
{
	mAlign = align;
}

physx::PxU32 DefinitionImpl::alignment(void) const
{
	return mAlign;
}

void DefinitionImpl::setPadding(physx::PxU32 pad)
{
	mPad = pad;
}

physx::PxU32 DefinitionImpl::padding(void) const
{
	return mPad;
}

void DefinitionImpl::init(const char *name, DataType t, const char *structName, bool static_allocation)
{
	PX_UNUSED( static_allocation );

    cleanup();

	mName = name;
    mLongName = name;
	mStructName = structName;
	
    PX_ASSERT(t != TYPE_UNDEFINED);
    mType = t;
}

void DefinitionImpl::destroy(void)
{
	PX_ASSERT( !mStaticAllocation );

	if( !mStaticAllocation )
	{
		this->~DefinitionImpl();
		mTraits->free(this);
	}
}

void DefinitionImpl::cleanup(void)
{
    if(!mStaticAllocation)
    {
		if( mStructName )
			mTraits->free((void *)mStructName);

		mTraits->free((void *)mName);

        if(mChildren != NULL)
        {
           for(physx::PxI32 i=0; i < mNumChildren; ++i)
			   mChildren[i]->destroy();

		   local_free(mChildren);
		}

		if(mHints != NULL)
		{
			for(physx::PxI32 i=0; i < mNumHints; ++i)
			{
				mHints[i]->cleanup();
				mTraits->free(mHints[i]);
			}

            local_free(mHints);
		}

		if(mEnumVals != NULL)
		{
			for(physx::PxI32 i = 0; i < mNumEnumVals; ++i)
				local_free(mEnumVals[i]);

            local_free(mEnumVals);
		}

		if(mRefVariantVals != NULL)
		{
			for(physx::PxI32 i = 0; i < mNumRefVariants; ++i)
				local_free(mRefVariantVals[i]);

            local_free(mRefVariantVals);
        }
    }

	if(mLongNameAllocated && mLongName)
	{
		mTraits->strfree( (char *)mLongName );
		mLongName = NULL;
		mLongNameAllocated = false;
	}
    
    setDefaults();
}

const Definition *DefinitionImpl::root(void) const
{
    const Definition *root = this;
    while(root->parent() != NULL)
        root = root->parent();

    return(root);
}

physx::PxI32 DefinitionImpl::arrayDimension(void) const
{
    PX_ASSERT(type() == TYPE_ARRAY);

    physx::PxI32 Dim = 0;
    const Definition *Cur = this;
    for(;Cur->type() == TYPE_ARRAY; Cur = Cur->child(0))
    {
      PX_ASSERT(Cur != NULL);
      Dim++;
    }

    return(Dim);
}

physx::PxI32 DefinitionImpl::arraySize(physx::PxI32 dimension) const
{
    PX_ASSERT(type() == TYPE_ARRAY);
    PX_ASSERT(dimension >= 0);
    PX_ASSERT(dimension < arrayDimension());

    const Definition *Cur = this;
    for(physx::PxI32 i=0; i < dimension; ++i)
        Cur = Cur->child(0);

    if(Cur->type() != TYPE_ARRAY)
        return(-1);

	const DefinitionImpl *pd = static_cast<const DefinitionImpl *>(Cur);

    return( pd->mArraySize);
}

bool DefinitionImpl::arraySizeIsFixed(void) const
{
    PX_ASSERT(type() == TYPE_ARRAY);
    return(mArraySize > 0);
}

bool DefinitionImpl::setArraySize(physx::PxI32 size)
{
    PX_ASSERT(size >= -1);

    if(size < 0)
        return(false);

    mArraySize = size;

    return(true);
}

bool DefinitionImpl::isIncludedRef(void) const
{
	const Hint *h = hint("INCLUDED");
   	return h && h->type() == TYPE_U64 && h->asUInt();
}

physx::PxI32 DefinitionImpl::numChildren(void) const
{
    return(mNumChildren);
}

const Definition * DefinitionImpl::child(physx::PxI32 index) const
{
    PX_ASSERT(index >= 0);
    PX_ASSERT(index < numChildren());
    PX_ASSERT(type() == TYPE_STRUCT || type() == TYPE_ARRAY);

    if(index < 0 || index >= numChildren())
        return(NULL);

    return(mChildren[index]);
}

const Definition *  DefinitionImpl::child(const char *name, physx::PxI32 &index) const
{
    PX_ASSERT(name);
    PX_ASSERT(type() == TYPE_STRUCT);

    physx::PxI32 i;
    for(i=0; i < numChildren(); ++i)
        if(!strcmp(mChildren[i]->name(), name))
        {
            index = i;
            return(mChildren[i]);
        }

    return(NULL);
}


#define PUSH_TO_ARRAY(val, array, obj_type, num_var) \
    num_var++; \
    if(array == NULL) \
        array = (obj_type *)local_malloc(sizeof(obj_type)); \
    else \
        array = (obj_type *)local_realloc(array, num_var * sizeof(obj_type)); \
    PX_ASSERT(array != NULL); \
    array[num_var-1] = val;


static char *GenLongName(char *dst, 
                         physx::PxU32 n,
                         const char *parent_long_name,
                         DataType parent_type, 
                         const char *child_name)
{
    local_strncpy(dst, parent_long_name, n);

    switch(parent_type)
    {
        case TYPE_STRUCT:
            if(parent_long_name[0])
                local_strncat(dst, ".", n);
            local_strncat(dst, child_name, n);
            break;

        case TYPE_ARRAY:
            local_strncat(dst, "[]", n);
            break;

        default:
            PX_ASSERT((void *)"Shouldn't be here!" == NULL);
            break;
    }

    return(dst);
}

#define SET_ARRAY(type, src_array_var, dst_array_var, num_var) \
    PX_ASSERT(src_array_var != NULL); \
    PX_ASSERT(n > 0); \
    if(mStaticAllocation) \
    { \
        dst_array_var = (type *)src_array_var; \
        num_var = n; \
    } \
    else \
    { \
        PX_ASSERT(dst_array_var == NULL); \
        dst_array_var = (type *)dupBuf(src_array_var, sizeof(type) * n); \
        num_var = n; \
    }

void DefinitionImpl::setChildren(Definition **children, physx::PxI32 n)
{
    SET_ARRAY(Definition *, children, mChildren, mNumChildren);

	char tmpStr[MAX_NAME_LEN];

	for(physx::PxI32 i=0; i < n; ++i)
    {
        Definition *_child = children[i];
		DefinitionImpl *child = static_cast< DefinitionImpl *>(_child);

        PX_ASSERT(child->parent() == NULL); // Only one parent allowed

        GenLongName(tmpStr, 
                    MAX_NAME_LEN, 
                    mLongName, 
                    type(),
                    child->mName);

		child->mLongName = mTraits->strdup( tmpStr );
		child->mLongNameAllocated = true;

		PX_ASSERT( child != this );
        child->mParent = this;
    }
}

void DefinitionImpl::addChild(Definition *_child)
{
    PX_ASSERT(_child != NULL);
    PX_ASSERT(!mStaticAllocation);

	DefinitionImpl *child = static_cast< DefinitionImpl *>(_child);
	PX_ASSERT(child->mParent == NULL); // Only one parent allowed
	
	char tmpStr[MAX_NAME_LEN];

    GenLongName(tmpStr, 
                MAX_NAME_LEN, 
                mLongName, 
                type(), 
                child->mName);

	child->mLongName = mTraits->strdup( tmpStr );
	child->mLongNameAllocated = true;

	PX_ASSERT( child != this );
    child->mParent = this;
    PUSH_TO_ARRAY(_child, mChildren, Definition *, mNumChildren)
}


physx::PxI32 DefinitionImpl::numHints(void) const
{
    return(mNumHints);   
}

const Hint *DefinitionImpl::hint(physx::PxI32 index) const
{
    PX_ASSERT(index >= 0);
    PX_ASSERT(index < numHints());

	if( index >= numHints() )
	{
		return(NULL);
	}

    return(mHints[index]);
}

const Hint *DefinitionImpl::hint(const char *name) const
{
    PX_ASSERT(name != NULL);

    for(physx::PxI32 i=0; i < numHints(); ++i)
        if(!strcmp(mHints[i]->name(), name))
            return(mHints[i]);

    return(NULL);
}

void DefinitionImpl::setHints(const Hint **hints, physx::PxI32 n)
{
    SET_ARRAY(HintImpl *, hints, mHints, mNumHints);
}

void DefinitionImpl::addHint(Hint *_hint)
{
    PX_ASSERT(_hint != NULL);
	HintImpl *hint = static_cast< HintImpl *>(_hint);
    PUSH_TO_ARRAY(hint, mHints, HintImpl *, mNumHints)
}

physx::PxI32 DefinitionImpl::numEnumVals(void) const
{
   return(mNumEnumVals);
}

physx::PxI32 DefinitionImpl::enumValIndex( const char * enum_val ) const
{
	if(!enum_val)
		return(-1);

	for(physx::PxI32 i=0; i < numEnumVals(); ++i)
	{
        if( !strcmp( enumVal(i), enum_val ) )
		{
            return(i);
		}
	}

	return(-1);
}

const char *DefinitionImpl::enumVal(physx::PxI32 index) const
{
    PX_ASSERT(index >= 0);
    PX_ASSERT(index < numEnumVals());

    return(mEnumVals[index]);
}

void DefinitionImpl::setEnumVals(const char **enum_vals, physx::PxI32 n)
{
    SET_ARRAY(char *, enum_vals, mEnumVals, mNumEnumVals);
}

void DefinitionImpl::addEnumVal(const char *enum_val)
{
    PX_ASSERT(enum_val != NULL);

    char *NewEnumVal = COND_DUP(enum_val);
    PUSH_TO_ARRAY(NewEnumVal, mEnumVals, char *, mNumEnumVals)
}

physx::PxI32 DefinitionImpl::refVariantValIndex( const char * ref_val ) const
{
	if(!ref_val)
		return(-1);

	for(physx::PxI32 i=0; i < numRefVariants(); ++i)
	{
        if( !strcmp( refVariantVal(i), ref_val ) )
		{
            return(i);
		}
	}

	return(-1);
}

physx::PxI32 DefinitionImpl::numRefVariants(void) const
{
	return(mNumRefVariants);
}

const char *DefinitionImpl::refVariantVal(physx::PxI32 index) const
{
    PX_ASSERT(index >= 0);
    PX_ASSERT(index < numRefVariants());

    return(mRefVariantVals[index]);
}

void DefinitionImpl::setRefVariantVals(const char **ref_vals, physx::PxI32 n)
{
    SET_ARRAY(char *, ref_vals, mRefVariantVals, mNumRefVariants);
}

void DefinitionImpl::addRefVariantVal(const char *ref_val)
{
    PX_ASSERT(ref_val != NULL);

    char *NewEnumVal = COND_DUP(ref_val);
    PUSH_TO_ARRAY(NewEnumVal, mRefVariantVals, char *, mNumRefVariants)
}

void DefinitionImpl::setDynamicHandleIndicesMap(const physx::PxU8 *indices, physx::PxU32 numIndices)
{
	mNumDynamicHandleIndices = numIndices;
	mDynamicHandleIndices = indices;
}

const physx::PxU8 * DefinitionImpl::getDynamicHandleIndicesMap(physx::PxU32 &outNumIndices) const
{
	outNumIndices = mNumDynamicHandleIndices;
	return mDynamicHandleIndices;
}

bool DefinitionImpl::isSimpleType(bool simpleStructs, bool simpleStrings) const
{
	switch(mType)
	{
	case TYPE_STRUCT:
		if( !simpleStructs )
			return false;

		for(physx::PxI32 i = 0; i < mNumChildren; ++i)
		{
			if( !mChildren[i]->isSimpleType(simpleStructs, simpleStrings) )
				return false;
		}

		return true;

	case TYPE_ARRAY:
	case TYPE_REF:
		return false;

	default:
		PX_ASSERT( mNumChildren == 0 );
		return true;
	}
}

//******************************************************************************
//*** class NxParameterized
//******************************************************************************

NxParameters::NxParameters(Traits *traits, void *buf, physx::PxI32 *refCount)
{
	mParameterizedTraits = traits;

	if( buf )
	{
		mBuffer = buf;
		mRefCount = refCount;

		//Values of other fields are already deserialized
	}
	else
	{
		mName = mClassName = NULL;
		mDoDeallocateName = mDoDeallocateClassName = mDoDeallocateSelf = true;

		mSerializationCb = NULL;
		mCbUserData = NULL;

		mBuffer = NULL;
		mRefCount = NULL;

		//Other fields are used only for inplace objects => skip them
	}
}

NxParameters::~NxParameters()
{
	if( mClassName && mDoDeallocateClassName )
	{
		mParameterizedTraits->strfree( const_cast<char*>(mClassName) );
		mClassName = NULL;
	}

	if( mName && mDoDeallocateName )
	{
		mParameterizedTraits->strfree( const_cast<char*>(mName) );
		mName = NULL;
	}
}

// placement delete
void NxParameters::destroy()
{
	// We cache these fields here to avoid overwrite in destructor
	bool doDeallocateSelf = mDoDeallocateSelf;
	void *buf = mBuffer;
	physx::PxI32 *refCount = mRefCount;
	NxParameterized::Traits *traits = mParameterizedTraits;

	this->~NxParameters();

	destroy(this, traits, doDeallocateSelf, refCount, buf);
}

void NxParameters::destroy(NxParameters *obj, NxParameterized::Traits *traits, bool doDeallocateSelf, physx::PxI32 *refCount, void *buf)
{
	if( !doDeallocateSelf )
		return;

	if( !refCount ) //Ordinary object?
		{
		traits->free(obj);
			return;
		}

		//Inplace object => callback client

	traits->onInplaceObjectDestroyed(buf, obj);
	if( !traits->decRefCount(refCount) )
		traits->onAllInplaceObjectsDestroyed(buf);
}


physx::PxU16 NxParameters::getMajorVersion(void) const
{
	physx::PxU16 major = version() >> 16;
	return major;
}

physx::PxU16 NxParameters::getMinorVersion(void) const
{
	physx::PxU16 minor = version() & 0xffff;
	return minor;
}

void NxParameters::initRandom(void)
{
	Handle handle(*this, "");
	initRandom(handle);
}

void NxParameters::initRandom(NxParameterized::Handle& handle)
{
	NxParameterized::ErrorType error;

	const Definition* pd = handle.parameterDefinition();
	switch( pd->type() )
	{
	case TYPE_STRUCT:
		{
			for(physx::PxI32 i = 0; i < pd->numChildren(); ++i)
			{
				handle.set(i);
				initRandom(handle);
				handle.popIndex();
			}

			break;
		}

	case TYPE_ARRAY:
		{
			if (!pd->arraySizeIsFixed())
			{
				error = handle.resizeArray(physx::PxI32(10 * (double)rand() / RAND_MAX));
				DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
			}

			physx::PxI32 size;
			error = handle.getArraySize(size);
			DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );

			for(physx::PxI32 i = 0; i < size; ++i)
			{
				handle.set(i);
				initRandom(handle);
				handle.popIndex();
			}

			break;
		}

	case TYPE_REF:
		{
			if (!pd->numRefVariants())
				break; // Can't do anything without refVariants-hint

			physx::PxI32 refIdx = (physx::PxI32)RandomIdx(0u, (physx::PxU32)pd->numRefVariants() - 1);
			const char* className = pd->refVariantVal(refIdx);
			if( mParameterizedTraits->doesFactoryExist(className) ) {
				error = initParamRef(handle, className, true);
				DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );

				NxParameterized::Interface* obj = NULL;
				error = handle.getParamRef(obj);
				DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
			}

			break;
		}

	case TYPE_BOOL:
		error = handle.setParamBool( 0 == RandomU32() % 2 );
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_STRING:
		{
			char s[10];
			physx::PxI32 len = (physx::PxI32)( (size_t)rand() % sizeof(s) );
			for(physx::PxI32 i = 0; i < len; ++i)
				s[i] = 'a' + rand() % ('z' - 'a');
			s[len] = 0;

			error = handle.setParamString(s);
			DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
			break;
		}

	case TYPE_ENUM:
		{
			physx::PxI32 enumIdx = (physx::PxI32)RandomIdx(0u, (physx::PxU32)pd->numEnumVals()-1);
			error = handle.setParamEnum(pd->enumVal(enumIdx));
			DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
			break;
		}

	case TYPE_I8:
		error = handle.setParamI8((physx::PxI8)(RandomU32() & 0xff));
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_U8:
		error = handle.setParamU8((physx::PxU8)(RandomU32() & 0xff));
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_I16:
		error = handle.setParamI16((physx::PxI16)(RandomU32() & 0xffff));
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_U16:
		error = handle.setParamU16((physx::PxU16)(RandomU32() & 0xffff));
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_I32:
		error = handle.setParamI32((physx::PxI32)RandomU32());
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_U32:
		error = handle.setParamU32((physx::PxU32)RandomU32());
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_I64:
		error = handle.setParamI64((physx::PxI64)RandomU64());
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_U64:
		error = handle.setParamU64(RandomU64());
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_F32:
		error = handle.setParamF32(RandomF32());
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_F64:
		error = handle.setParamF64(RandomF64());
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_VEC2:
		error = handle.setParamVec2(RandomVec2());
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_VEC3:
		error = handle.setParamVec3(RandomVec3());
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_VEC4:
		error = handle.setParamVec4(RandomVec4());
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_QUAT:
		error = handle.setParamQuat(RandomQuat());
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_MAT33:
		error = handle.setParamMat33(physx::PxMat33(RandomVec3(), RandomVec3(), RandomVec3()));
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_MAT34:
		error = handle.setParamMat34(physx::PxMat34Legacy(physx::PxMat44(RandomVec4(), RandomVec4(), RandomVec4(), RandomVec4())));
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_BOUNDS3:
		error = handle.setParamBounds3(physx::PxBounds3(RandomVec3(), RandomVec3()));
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_MAT44:
		error = handle.setParamMat44(physx::PxMat44(RandomVec4(), RandomVec4(), RandomVec4(), RandomVec4()));
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;

	case TYPE_POINTER:
		// We can't init pointers :(
		break;

	case TYPE_TRANSFORM:
	{
		// PxTransform asserts if the quat isn't "sane"
		physx::PxQuat q;
		do { 
			q = RandomQuat();
		} while (!q.isSane());

		error = handle.setParamTransform(physx::PxTransform(RandomVec3(),q));
		DEBUG_ASSERT( NxParameterized::ERROR_NONE == error );
		break;
	}
	default:
		PX_ALWAYS_ASSERT();
	}

	PX_UNUSED(error); // Make compiler happy
}

void NxParameters::setClassName(const char *name) 
{ 
	if(mParameterizedTraits)
	{
		if( mClassName )
		{
			if( !strcmp( mClassName, name ) )
				return;

			if( mDoDeallocateClassName )
				mParameterizedTraits->strfree( const_cast<char*>(mClassName) );
		}

		mClassName = mParameterizedTraits->strdup(name);
	}
	else
	{
		mClassName = name;
	}

	mDoDeallocateClassName = true;
}

void NxParameters::setName(const char *name) 
{ 
	if(mParameterizedTraits)
	{
		if( mName )
		{
			if( !strcmp( mName, name ) )
				return;

			if( mDoDeallocateName )
				mParameterizedTraits->strfree( const_cast<char*>(mName) );
		}

		mName = mParameterizedTraits->strdup(name);
	}
	else
	{
		mName = name; 
	}

	mDoDeallocateName = true;
}

void NxParameters::setSerializationCallback(SerializationCallback *cb, void *userData)
{
	mSerializationCb = cb;
	mCbUserData = userData;
}

ErrorType NxParameters::callPreSerializeCallback() const
{
	if(mSerializationCb)
	{
		mSerializationCb->preSerialize(mCbUserData);
	}

	Handle handle(*this);

	NX_ERR_CHECK_RETURN( getParameterHandle("", handle) );

	return callPreSerializeCallback(handle);
}

physx::PxI32 NxParameters::numParameters(void) 
{
    return(rootParameterDefinition()->numChildren());
}

const Definition *NxParameters::parameterDefinition(physx::PxI32 index) 
{
    return(rootParameterDefinition()->child(index));
}

const Definition *NxParameters::rootParameterDefinition(void)
{
    return(getParameterDefinitionTree());
}

const Definition *NxParameters::rootParameterDefinition(void) const
{
    return(getParameterDefinitionTree());
}

ErrorType NxParameters::getParameterHandle(const char *long_name, Handle &handle) const
{
	PX_ASSERT( handle.getConstInterface() == this );
	
	if( rootParameterDefinition() == NULL )
	{
		handle.reset();
		return(ERROR_INVALID_CALL_ON_NAMED_REFERENCE);
	}

    return(handle.set(this,rootParameterDefinition(), long_name));
}

ErrorType NxParameters::getParameterHandle(const char *long_name, Handle &handle)
{
	PX_ASSERT( handle.getConstInterface() == this );

	if( rootParameterDefinition() == NULL )
	{
		handle.reset();
		return(ERROR_INVALID_CALL_ON_NAMED_REFERENCE);
	}

    return(handle.set(this,rootParameterDefinition(), long_name));
}

#ifndef PX_CHECKED
#	define CHECK_HANDLE
#	define CHECK_IS_SIMPLE_ARRAY(type_enum_name)
#else
#	define CHECK_HANDLE \
		{ \
	        ErrorType result = checkParameterHandle(handle); \
		    if(result != ERROR_NONE) \
			    return(result); \
		}
#	define CHECK_IS_SIMPLE_ARRAY(type_enum_name) \
		{ \
			PX_ASSERT(offset >= 0);\
	        PX_ASSERT(n >= 0);\
		    if(handle.parameterDefinition()->type() != TYPE_ARRAY) \
			    return(ERROR_NOT_AN_ARRAY); \
	        if(handle.parameterDefinition()->child(0)->type() != TYPE_##type_enum_name) \
		        return(ERROR_CAST_FAILED); \
			physx::PxI32 arraySize; \
	        ErrorType error; \
		    if((error = getArraySize(handle, arraySize)) != ERROR_NONE) \
			    return(error); \
	        if( offset + n > arraySize) \
		        return(ERROR_ARRAY_IS_TOO_SMALL); \
		}
#endif

template <class Type > ErrorType rawGetParam(const Handle &handle,Type &val,const NxParameters *parent)
{
	size_t offset;
	void *ptr=NULL;
	parent->getVarPtr(handle, ptr, offset);
	if ( ptr == NULL ) 
		return ERROR_TYPE_NOT_SUPPORTED;
	Type *var = (Type *)((char *)ptr);
	val = *var;
	return(ERROR_NONE);
}

template <class Type > ErrorType rawSetParam(const Handle &handle,const Type &val,NxParameters *parent)
{
	size_t offset;
	void *ptr=NULL;
	parent->getVarPtr(handle, ptr, offset);
	if(ptr == NULL)
		return(ERROR_INDEX_OUT_OF_RANGE);
	Type *Var = (Type *)((char *)ptr);
	*Var = val;
	return(ERROR_NONE);
}

template <class Type >ErrorType rawGetParamArray(const Handle &handle,Type *array, physx::PxI32 n, physx::PxI32 offset,const NxParameters *parent) 
{
	physx::PxI32 size;
	NX_ERR_CHECK_RETURN(handle.getArraySize(size));
	if( size < offset + n )
		return(ERROR_INDEX_OUT_OF_RANGE);

	if( n )
	{
		Handle memberHandle(handle);
		NX_ERR_CHECK_RETURN(memberHandle.set(offset));

		size_t tmp;
		void *ptr=NULL;
		parent->getVarPtr(memberHandle, ptr, tmp);
		if(ptr == NULL)
			return(ERROR_INDEX_OUT_OF_RANGE);

		::memcpy(array, ptr, n * sizeof(Type));
	}

	return(ERROR_NONE);
}

template <class Type> ErrorType rawSetParamArray(const Handle &handle, const Type *array, physx::PxI32 n, physx::PxI32 offset,NxParameters *parent)
{
	physx::PxI32 size;
	NX_ERR_CHECK_RETURN(handle.getArraySize(size));
	if( size < offset + n )
		return(ERROR_INDEX_OUT_OF_RANGE);

	if( n )
	{
		Handle memberHandle(handle);
		NX_ERR_CHECK_RETURN(memberHandle.set(offset));

		size_t tmp;
		void *ptr=NULL;
		parent->getVarPtr(memberHandle, ptr, tmp);
		if(ptr == NULL)
			return(ERROR_INDEX_OUT_OF_RANGE);

		::memcpy(ptr, array, n * sizeof(Type));
	}

	return(ERROR_NONE);
}

//******************************************************************************
//*** Bool
//******************************************************************************

ErrorType NxParameters::getParamBool(const Handle &handle, bool &val) const
{
    CHECK_HANDLE

    if(handle.parameterDefinition()->type() == TYPE_BOOL)
    {
		return rawGetParamBool(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamBool(const Handle &handle, bool val)
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_BOOL)
    {
        return rawSetParamBool(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamBoolArray(const Handle &handle, bool *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(BOOL)
    return(rawGetParamBoolArray(handle, array, n, offset));
}

ErrorType NxParameters::setParamBoolArray(const Handle &handle, const bool *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(BOOL)
    return(rawSetParamBoolArray(handle, array, n, offset));
}

//******************************************************************************
//*** String
//******************************************************************************

ErrorType NxParameters::getParamString(const Handle &handle, const char *&val) const
{
    CHECK_HANDLE

    if(handle.parameterDefinition()->type() == TYPE_ENUM)
    {
        return rawGetParamEnum(handle, val);
    }
    
	if(handle.parameterDefinition()->type() == TYPE_STRING)
    {
        return rawGetParamString(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamString(const Handle &handle, const char *val)
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_STRING)
    {
        return rawSetParamString(handle, val);
    }

	if(handle.parameterDefinition()->type() == TYPE_ENUM)
    {
        return rawSetParamEnum(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::rawGetParamStringArray(const Handle &handle, char **array, physx::PxI32 n, physx::PxI32 offset) const
{
	Handle memberHandle(handle);
	for(physx::PxI32 i=0; i < n; ++i)
	{
		ErrorType error;
		if((error = memberHandle.set(i + offset)) != ERROR_NONE)
			return(error);
		const char * tmp;
		if((error = rawGetParamString(memberHandle, tmp)) != ERROR_NONE)
			return(error);
		array[i] = const_cast<char*>(tmp);
		memberHandle.popIndex();
	}
	return(ERROR_NONE);
}


ErrorType NxParameters::getParamStringArray(const Handle &handle, char **array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(STRING)
    return(rawGetParamStringArray(handle, array, n, offset));
}

ErrorType NxParameters::rawSetParamStringArray(const Handle &handle, const char **array, physx::PxI32 n, physx::PxI32 offset)
{
	Handle memberHandle(handle);
	for(physx::PxI32 i=0; i < n; ++i)
	{
		ErrorType error;
		if((error = memberHandle.set(i + offset)) != ERROR_NONE)
			return(error);
		if((error = rawSetParamString(memberHandle, array[i] )) != ERROR_NONE)
			return(error);
		memberHandle.popIndex();
	}
	return(ERROR_NONE);
}

ErrorType NxParameters::setParamStringArray(const Handle &handle, const char **array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(STRING)
    return(rawSetParamStringArray(handle, array, n, offset));
}

//******************************************************************************
//*** Enum
//******************************************************************************

ErrorType NxParameters::getParamEnum(const Handle &handle, const char *&val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_STRING)
    {
        return rawGetParamString(handle, val);
    }

	if(handle.parameterDefinition()->type() == TYPE_ENUM)
    {
        return rawGetParamEnum(handle, val);
    }
    
    return(ERROR_CAST_FAILED);
}

// returns the actual enum string from the ParameterDefintion object, which should be good for the liftetime
// of the DefinitionImpl object.  Returns NULL if no such enum exists.
static const char* getEnumString(const Handle &handle, const char* str)
{
    PX_ASSERT(str != NULL);

    const Definition* paramDef = handle.parameterDefinition();
    PX_ASSERT(paramDef != NULL);
    PX_ASSERT(paramDef->type() == TYPE_ENUM);

    for (physx::PxI32 i = 0; i < paramDef->numEnumVals(); ++i)
        if(!strcmp(paramDef->enumVal(i), str))
            return(paramDef->enumVal(i));

    return(NULL);    
}

ErrorType NxParameters::setParamEnum(const Handle &handle, const char *val)
{
    CHECK_HANDLE

    val = getEnumString(handle, val);
    if(val == NULL)
        return(ERROR_INVALID_ENUM_VAL);

	if(handle.parameterDefinition()->type() == TYPE_ENUM)
    {
        return rawSetParamEnum(handle, val);
    }

	if(handle.parameterDefinition()->type() == TYPE_STRING)
    {
        return rawSetParamString(handle, val);
    }

	return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::rawGetParamEnumArray(const Handle &handle, char **array, physx::PxI32 n, physx::PxI32 offset) const
{
	Handle memberHandle(handle);
	for(physx::PxI32 i=0; i < n; ++i)
	{
		ErrorType error; 
		if((error = memberHandle.set(i + offset)) != ERROR_NONE)
			return(error);
		const char * tmp;
		if((error = rawGetParamEnum(memberHandle, tmp)) != ERROR_NONE)
			return(error);
		array[i] = const_cast<char*>(tmp);
		memberHandle.popIndex();
	}
	return(ERROR_NONE);
}

ErrorType NxParameters::rawSetParamEnumArray(const Handle &handle, const char **array, physx::PxI32 n, physx::PxI32 offset)
{
	Handle memberHandle(handle);
	for(physx::PxI32 i=0; i < n; ++i)
	{
		ErrorType error;
		if((error = memberHandle.set(i + offset)) != ERROR_NONE)
			return(error);
		if((error = rawSetParamEnum(memberHandle, array[i])) != ERROR_NONE)
			return(error);
		memberHandle.popIndex();
	}
	return(ERROR_NONE);
}

ErrorType NxParameters::getParamEnumArray(const Handle &handle, char **array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(ENUM)
    return(rawGetParamEnumArray(handle, array, n, offset));
}

ErrorType NxParameters::setParamEnumArray(const Handle &handle, const char **array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(ENUM)
    return(rawSetParamEnumArray(handle, array, n, offset));
}

//******************************************************************************
//*** Ref
//******************************************************************************

ErrorType NxParameters::initParamRef(const Handle &handle, const char *inChosenRefStr, bool doDestroyOld)
{
	CHECK_HANDLE

	NxParameterized::Interface *param = NULL;
	const char *chosenRefStr = inChosenRefStr;

	// create NxParam object (depends on if it's included or not)
	const Hint *hint = handle.parameterDefinition()->hint("INCLUDED");
	if (hint && hint->type() != TYPE_U64)
	{
		return(ERROR_INVALID_REFERENCE_INCLUDE_HINT);
	}

	if (hint != NULL && hint->asUInt() == 1)
	{
		// included

		if (chosenRefStr == 0 && handle.parameterDefinition()->numRefVariants() > 1)
		{
			return(ERROR_INVALID_REFERENCE_VALUE);
		}

		for (physx::PxI32 i = 0; i < handle.parameterDefinition()->numRefVariants(); i++)
		{
			if (!strcmp(handle.parameterDefinition()->refVariantVal(i), chosenRefStr))
			{
				// create an object of type chosenRefStr, somehow
				param = mParameterizedTraits->createNxParameterized(chosenRefStr);
				if (!param)
				{
					return ERROR_OBJECT_CONSTRUCTION_FAILED;
				}

				return setParamRef(handle, param, doDestroyOld);
			}
		}
		// PH: debug hint
		// If you land here, you should compare chosenRefStr and handle.mParameterDefinition.mRefVariantVals
		// to see why it couldn't find anything, and then fix the .pl
		return(ERROR_INVALID_REFERENCE_VALUE);
	}
	else
	{
		// not included, just create generic NxParameterized
		param = NX_PARAM_PLACEMENT_NEW(mParameterizedTraits->alloc(sizeof(NxParameters)), NxParameters)(mParameterizedTraits);
		if (!param)
		{
			return ERROR_OBJECT_CONSTRUCTION_FAILED;
		}

		if (chosenRefStr == 0)
		{
			param->setClassName(handle.parameterDefinition()->refVariantVal(0));
		}
		else
		{
			bool found = false;
			for (physx::PxI32 i = 0; i < handle.parameterDefinition()->numRefVariants(); i++)
			{
				if (!strcmp(handle.parameterDefinition()->refVariantVal(i), chosenRefStr))
				{
					param->setClassName(handle.parameterDefinition()->refVariantVal(i));
					found = true;
					break;
				}
			}
			if ( !found )
			{
				// ensure that we free this memory that we've allocated
				mParameterizedTraits->free(param);
				return(ERROR_INVALID_REFERENCE_VALUE);
			}
		}
		return setParamRef(handle, param, doDestroyOld);
	}
}

ErrorType NxParameters::getParamRef(const Handle &handle, NxParameterized::Interface *&val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_REF)
    {
        return rawGetParamRef(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamRef(const Handle &handle, NxParameterized::Interface *val, bool doDestroyOld)
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_REF)
    {
		if (doDestroyOld)
		{
			NxParameterized::Interface *param = NULL;

			getParamRef(handle, param);
			if(param)
			{
				param->destroy();
				param = NULL;
			}
		}
        return rawSetParamRef(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::rawGetParamRefArray(const Handle &handle, NxParameterized::Interface **array, physx::PxI32 n, physx::PxI32 offset) const
{
	Handle memberHandle(handle);
	for(physx::PxI32 i=0; i < n; ++i)
	{
		ErrorType error; 
		if((error = memberHandle.set(i + offset)) != ERROR_NONE)
			return(error);
		NxParameterized::Interface * tmp;
		if((error = rawGetParamRef(memberHandle, tmp)) != ERROR_NONE)
			return(error);
		array[i] = tmp;
		memberHandle.popIndex();
	}
	return(ERROR_NONE);
}

ErrorType NxParameters::rawSetParamRefArray(const Handle &handle,NxParameterized::Interface **array, physx::PxI32 n, physx::PxI32 offset)
{
	Handle memberHandle(handle);
	for(physx::PxI32 i=0; i < n; ++i) 
	{ 
		ErrorType error;
		if((error = memberHandle.set(i + offset)) != ERROR_NONE) 
			return(error); 
		if((error = rawSetParamRef(memberHandle, array[i])) != ERROR_NONE)
			return(error); 
		memberHandle.popIndex(); 
	} 

	return(ERROR_NONE);
}

ErrorType NxParameters::getParamRefArray(const Handle &handle, NxParameterized::Interface * *array, physx::PxI32 n, physx::PxI32 offset) const
{ 
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(REF)
    return(rawGetParamRefArray(handle, array, n, offset));
}

ErrorType NxParameters::setParamRefArray(const Handle &handle, /*const*/ NxParameterized::Interface **array, physx::PxI32 n, physx::PxI32 offset, bool doDestroyOld)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(REF)

	Handle memberHandle(handle);
	for(physx::PxI32 i = 0; i < n; ++i) 
	{ 
		ErrorType error;
		if((error = memberHandle.set(i + offset)) != ERROR_NONE)
		{
			return(error); 
		}

		if((error = setParamRef(memberHandle, array[i]), doDestroyOld) != ERROR_NONE)
		{
			return(error); 
		}
		memberHandle.popIndex(); 
	}
	return(ERROR_NONE);
}

//******************************************************************************
//*** I8
//******************************************************************************

ErrorType NxParameters::getParamI8(const Handle &handle, physx::PxI8 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_I8)
    {
        return rawGetParamI8(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamI8(const Handle &handle, physx::PxI8 val)
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_I8)
    {
        return rawSetParamI8(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamI8Array(const Handle &handle, physx::PxI8 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(I8)
    return(rawGetParamI8Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamI8Array(const Handle &handle, const physx::PxI8 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(I8)
    return(rawSetParamI8Array(handle, array, n, offset));
}

//******************************************************************************
//*** I16
//******************************************************************************

ErrorType NxParameters::getParamI16(const Handle &handle, physx::PxI16 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_I8)
    {
        physx::PxI8 tmp;
        ErrorType result = rawGetParamI8(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxI16)tmp;
        return(ERROR_NONE);
    }

	if(handle.parameterDefinition()->type() == TYPE_I16)
    {
        return rawGetParamI16(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamI16(const Handle &handle, physx::PxI16 val)
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_I16)
    {
        return rawSetParamI16(handle, val);
    }
	if(handle.parameterDefinition()->type() == TYPE_I8)
    {
        return rawSetParamI8(handle, (physx::PxI8)val);
    }
	if(handle.parameterDefinition()->type() == TYPE_U8)
    {
        return rawSetParamU8(handle, (physx::PxU8)val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamI16Array(const Handle &handle, physx::PxI16 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(I16)
    return(rawGetParamI16Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamI16Array(const Handle &handle, const physx::PxI16 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(I16)
    return(rawSetParamI16Array(handle, array, n, offset));
}

//******************************************************************************
//*** I32
//******************************************************************************

ErrorType NxParameters::getParamI32(const Handle &handle, physx::PxI32 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_I8)
    {
        physx::PxI8 tmp;
        ErrorType result = rawGetParamI8(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxI32)tmp;
        return(ERROR_NONE);
    }
	
	if(handle.parameterDefinition()->type() == TYPE_I16)
    {
        physx::PxI16 tmp;
        ErrorType result = rawGetParamI16(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxI32)tmp;
        return(ERROR_NONE);
    }

	if(handle.parameterDefinition()->type() == TYPE_I32)
    {
        physx::PxI32 tmp;
        ErrorType result = rawGetParamI32(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxI32)tmp;
        return(ERROR_NONE);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamI32(const Handle &handle, physx::PxI32 val)
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_I32)
    {
        return rawSetParamI32(handle, val);
    }
	if(handle.parameterDefinition()->type() == TYPE_I16)
    {
        return rawSetParamI16(handle, (physx::PxI16)val);
    }
	if(handle.parameterDefinition()->type() == TYPE_U16)
    {
        return rawSetParamU16(handle, (physx::PxU16)val);
    }
	if(handle.parameterDefinition()->type() == TYPE_I8)
    {
        return rawSetParamI8(handle, (physx::PxI8) val);
    }
	if(handle.parameterDefinition()->type() == TYPE_U8)
    {
        return rawSetParamU8(handle, (physx::PxU8) val);
    }


    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamI32Array(const Handle &handle, physx::PxI32 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(I32)
    return(rawGetParamI32Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamI32Array(const Handle &handle, const physx::PxI32 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(I32)
    return(rawSetParamI32Array(handle, array, n, offset));
}

//******************************************************************************
//*** I64
//******************************************************************************

ErrorType NxParameters::getParamI64(const Handle &handle, physx::PxI64 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_I8)
    {
        physx::PxI8 tmp;
        ErrorType result = rawGetParamI8(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxI64)tmp;
        return(ERROR_NONE);
    }
	
	if(handle.parameterDefinition()->type() == TYPE_I16)
    {
        physx::PxI16 tmp;
        ErrorType result = rawGetParamI16(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxI64)tmp;
        return(ERROR_NONE);
    }

	if(handle.parameterDefinition()->type() == TYPE_I32)
    {
        physx::PxI32 tmp;
        ErrorType result = rawGetParamI32(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxI64)tmp;
        return(ERROR_NONE);
    }

	if(handle.parameterDefinition()->type() == TYPE_I64)
    {
        physx::PxI64 tmp;
        ErrorType result = rawGetParamI64(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxI64)tmp;
        return(ERROR_NONE);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamI64(const Handle &handle, physx::PxI64 val)
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_I64)
    {
        return rawSetParamI64(handle, val);
    }
	if(handle.parameterDefinition()->type() == TYPE_U32)
    {
        return rawSetParamU32(handle, (physx::PxU32)val);
    }
	if(handle.parameterDefinition()->type() == TYPE_I32)
    {
        return rawSetParamI32(handle, (physx::PxI32)val);
    }
	if(handle.parameterDefinition()->type() == TYPE_I16)
    {
        return rawSetParamI16(handle, (physx::PxI16)val);
    }
	if(handle.parameterDefinition()->type() == TYPE_U16)
    {
        return rawSetParamU16(handle, (physx::PxU16) val);
    }
	if(handle.parameterDefinition()->type() == TYPE_I8)
    {
        return rawSetParamI8(handle, (physx::PxI8)val);
    }
	if(handle.parameterDefinition()->type() == TYPE_U8)
    {
        return rawSetParamU8(handle, (physx::PxU8)val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamI64Array(const Handle &handle, physx::PxI64 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(I64)
    return(rawGetParamI64Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamI64Array(const Handle &handle, const physx::PxI64 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(I64)
    return(rawSetParamI64Array(handle, array, n, offset));
}

//******************************************************************************
//*** U8
//******************************************************************************

ErrorType NxParameters::getParamU8(const Handle &handle, physx::PxU8 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_U8)
    {
        return rawGetParamU8(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamU8(const Handle &handle, physx::PxU8 val)
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_U8)
    {
        return rawSetParamU8(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamU8Array(const Handle &handle, const physx::PxU8 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(U8)
    return(rawSetParamU8Array(handle, array, n, offset));
}


//******************************************************************************
//*** U16
//******************************************************************************

ErrorType NxParameters::getParamU16(const Handle &handle, physx::PxU16 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_U8)
    {
        physx::PxU8 tmp;
        ErrorType result = rawGetParamU8(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxU16)tmp;
        return(ERROR_NONE);
    }

	if(handle.parameterDefinition()->type() == TYPE_U16)
    {
        physx::PxU16 tmp;
        ErrorType result = rawGetParamU16(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxU16)tmp;
        return(ERROR_NONE);
    }
	
	return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamU16(const Handle &handle, physx::PxU16 val)
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_U16)
    {
        return rawSetParamU16(handle, val);
    }
	if(handle.parameterDefinition()->type() == TYPE_U8)
    {
        return rawSetParamU8(handle, (physx::PxU8)val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamU16Array(const Handle &handle, physx::PxU16 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(U16)
    return(rawGetParamU16Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamU16Array(const Handle &handle, const physx::PxU16 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(U16)
    return(rawSetParamU16Array(handle, array, n, offset));
}

//******************************************************************************
//*** U32
//******************************************************************************

ErrorType NxParameters::getParamU32(const Handle &handle, physx::PxU32 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_U8)
    {
        physx::PxU8 tmp;
        ErrorType result = rawGetParamU8(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxU32)tmp;
        return(ERROR_NONE);
    }

	if(handle.parameterDefinition()->type() == TYPE_U16)
    {
        physx::PxU16 tmp;
        ErrorType result = rawGetParamU16(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxU32)tmp;
        return(ERROR_NONE);
    }

	if(handle.parameterDefinition()->type() == TYPE_U32)
    {
        physx::PxU32 tmp;
        ErrorType result = rawGetParamU32(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxU32)tmp;
        return(ERROR_NONE);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamU32(const Handle &handle, physx::PxU32 val)
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_U32)
    {
        return rawSetParamU32(handle, val);
    }
	if(handle.parameterDefinition()->type() == TYPE_U16)
    {
        return rawSetParamU16(handle, (physx::PxU16)val);
    }
	if(handle.parameterDefinition()->type() == TYPE_U8)
    {
        return rawSetParamU8(handle, (physx::PxU8)val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamU32Array(const Handle &handle, physx::PxU32 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(U32)
    return(rawGetParamU32Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamU32Array(const Handle &handle, const physx::PxU32 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(U32)
    return(rawSetParamU32Array(handle, array, n, offset));
}

//******************************************************************************
//*** U64
//******************************************************************************

ErrorType NxParameters::getParamU64(const Handle &handle, physx::PxU64 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_U8)
    {
        physx::PxU8 tmp;
        ErrorType result = rawGetParamU8(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxU64)tmp;
        return(ERROR_NONE);
    }

	if(handle.parameterDefinition()->type() == TYPE_U16)
    {
        physx::PxU16 tmp;
        ErrorType result = rawGetParamU16(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxU64)tmp;
        return(ERROR_NONE);
    }

	if(handle.parameterDefinition()->type() == TYPE_U32)
    {
        physx::PxU32 tmp;
        ErrorType result = rawGetParamU32(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxU64)tmp;
        return(ERROR_NONE);
    }

	if(handle.parameterDefinition()->type() == TYPE_U64)
    {
        physx::PxU64 tmp;
        ErrorType result = rawGetParamU64(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxU64)tmp;
        return(ERROR_NONE);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamU64(const Handle &handle, physx::PxU64 val)
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_U64)
    {
        return rawSetParamU64(handle, val);
    }

	if(handle.parameterDefinition()->type() == TYPE_U32)
    {
        return rawSetParamU32(handle, (physx::PxU32)val);
    }

	if(handle.parameterDefinition()->type() == TYPE_U16)
    {
        return rawSetParamU16(handle, (physx::PxU16)val);
    }

	if(handle.parameterDefinition()->type() == TYPE_U8)
    {
        return rawSetParamU8(handle, (physx::PxU8)val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamU64Array(const Handle &handle, physx::PxU64 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(U64)
    return(rawGetParamU64Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamU64Array(const Handle &handle, const physx::PxU64 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(U64)
    return(rawSetParamU64Array(handle, array, n, offset));
}

//******************************************************************************
//*** F32
//******************************************************************************

ErrorType NxParameters::getParamF32(const Handle &handle, physx::PxF32 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_F32)
    {
        return rawGetParamF32(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamF32(const Handle &handle, physx::PxF32 val)
{
    CHECK_HANDLE
    CHECK_F32_FINITE

	if(handle.parameterDefinition()->type() == TYPE_F32)
    {
        return rawSetParamF32(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamF32Array(const Handle &handle, physx::PxF32 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(F32)
    return(rawGetParamF32Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamF32Array(const Handle &handle, const physx::PxF32 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(F32)
    CHECK_F32_FINITE_ARRAY
    return(rawSetParamF32Array(handle, array, n, offset));
}

//******************************************************************************
//*** F64
//******************************************************************************

ErrorType NxParameters::getParamF64(const Handle &handle, physx::PxF64 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_F32)
    {
        physx::PxF32 tmp;
        ErrorType result = rawGetParamF32(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxF64)tmp;
        return(ERROR_NONE);
    }
	
	if(handle.parameterDefinition()->type() == TYPE_F64)
    {
        physx::PxF64 tmp;
        ErrorType result = rawGetParamF64(handle, tmp);
        if(result != ERROR_NONE)
            return(result);
        val = (physx::PxF64)tmp;
        return(ERROR_NONE);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamF64(const Handle &handle, physx::PxF64 val)
{
    CHECK_HANDLE
    CHECK_F64_FINITE

	if(handle.parameterDefinition()->type() == TYPE_F64)
    {
        return rawSetParamF64(handle, val);
    }
	else if(handle.parameterDefinition()->type() == TYPE_F32)
    {
		return rawSetParamF32(handle, (physx::PxF32)val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::setParamF64Array(const Handle &handle, const physx::PxF64 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(F64)
    CHECK_F64_FINITE_ARRAY
    return(rawSetParamF64Array(handle, array, n, offset));
}

/**
# When shrinking a dynamic array, the array may contain dynamically allocated Refs or Strings.
# It may also contain Structs that contain these items, we'll call them all "dynamic parameters".
#
# To handle this, we generate, for every dynamic array with dynamic parameters, handle indices in 
# the Parameter Definition that help the NxParameters::rawResizeArray() method find these parameters
# and destroy them quickly (without having to recursively traverse the parameters).
#
# The handle indices are layed out like this for the following struct:
# [ handleDepth0, 3, handleDepth1, 5 ] - handleDepth0 = handleDepth1 = 1 
# struct myStruct {
#   float a;
#   float b;
#   float c;
#   string myString;
#   float a;
#   ref myRef;
# }
#
# You can see that myString and myRef are the only two dynamically allocated members that need
# to be destroyed, so only their indices appear in the list.
# 
# Note: Currently, we only support 1D arrays with dynamic parameters in the top most struct.  
# array[i].myString is supported
# array[i].structa.myString is supported
# array[i].structa.structb.myString is NOT supported
*/
ErrorType NxParameters::releaseDownsizedParameters(const Handle &handle, int newSize, int oldSize)
{
	// if downsizing array, release dynamic parameters
	// for now, just do 1D arrays
	if( newSize >= oldSize )
	{
		return ERROR_NONE;
	}

	physx::PxU32 numIndices=0;
	Handle tmpHandle(handle);

	const physx::PxU8 *hIndices = tmpHandle.parameterDefinition()->getDynamicHandleIndicesMap(numIndices);

	if( numIndices )
	{
		// from array[new_size] to array[old_size]
		for( int i=newSize; i<oldSize; i++ )
		{
			tmpHandle.set( i );

			for( physx::PxU32 j=0; j<numIndices; )
			{
				// set the handle up to point to the right dynamic parameter
				physx::PxU8 indexDepth = hIndices[j];
				int k=0;
				for( ; k<indexDepth; k++ )
				{
					tmpHandle.set( hIndices[j+k+1] );
				}
				j += k + 1;
				
				// now we have a handle that's pointing to the dynamic parameter, release it
				DataType type = tmpHandle.parameterDefinition()->type();
				if( type == TYPE_STRING )
				{
					tmpHandle.setParamString( NULL );
				}
				else if( type == TYPE_REF )
				{
					NxParameterized::Interface * paramPtr = 0;
					tmpHandle.getParamRef( paramPtr );
					if( paramPtr )
					{
						paramPtr->destroy();
						tmpHandle.setParamRef( NULL );
					}
				}
				else if( type == TYPE_ENUM )
				{
					// nothing to do on a downsize
				}
				else
				{
					PX_ASSERT( 0 && "Invalid dynamically allocated type defined in Definition handle list" );
				}
				
				// reset the handle
				if( indexDepth > 0 )
				{
					tmpHandle.popIndex( indexDepth );
				}
			}

			tmpHandle.popIndex();
		}
	}

	return ERROR_NONE;
}



/**
# When growing a dynamic array, the array may contain Enums or Structs that contain enums.
#
# To handle this, we generate, for every dynamic array with enums, handle indices in 
# the Parameter Definition that help the NxParameters::rawResizeArray() method find these parameters
# and initialized them quickly (without having to recursively traverse the parameters).
#
# Note: Currently, we only support 1D arrays with enums in the top most struct.  
# array[i].enumA is supported
# array[i].structa.enumA is supported
# array[i].structa.structb.enumA is NOT supported
*/
ErrorType NxParameters::initNewResizedParameters(const Handle &handle, int newSize, int oldSize)
{
	// if downsizing array, release dynamic parameters
	// for now, just do 1D arrays
	if( newSize <= oldSize )
	{
		return ERROR_NONE;
	}

	physx::PxU32 numIndices=0;
	Handle tmpHandle(handle);

	const physx::PxU8 *hIndices = tmpHandle.parameterDefinition()->getDynamicHandleIndicesMap(numIndices);

	if( numIndices )
	{
		// from array[new_size] to array[old_size]
		for( int i=oldSize; i<newSize; i++ )
		{
			tmpHandle.set( i );

			for( physx::PxU32 j=0; j<numIndices; )
			{
				// set the handle up to point to the right dynamic parameter
				physx::PxU8 indexDepth = hIndices[j];
				int k=0;
				for( ; k<indexDepth; k++ )
				{
					tmpHandle.set( hIndices[j+k+1] );
				}
				j += k + 1;
				
				// now we have a handle that's pointing to the dynamic parameter, release it
				DataType type = tmpHandle.parameterDefinition()->type();
				if( type == TYPE_STRING || type == TYPE_REF )
				{
					// nothing to do on an array growth
				}
				else if( type == TYPE_ENUM )
				{
					// this is not the default value, but that's not available from here
					// we could possibly store the enum default values, or just always make them first
					tmpHandle.setParamEnum( tmpHandle.parameterDefinition()->enumVal(0) );
				}
				else if( type == TYPE_ARRAY )
				{
					// FIXME: we do not fully support arrays here, this is just a brain-dead stub

					const Definition *pd = 0;
					for(pd = tmpHandle.parameterDefinition(); pd->type() == TYPE_ARRAY; pd = pd->child(0))
					{
						PX_ASSERT( pd->numChildren() == 1 );
					}

					if( pd->type() != TYPE_STRING && pd->type() != TYPE_REF )
						PX_ASSERT( 0 && "Invalid dynamically allocated type defined in Definition handle list" );
				}
				else
				{
					PX_ASSERT( 0 && "Invalid dynamically allocated type defined in Definition handle list" );
				}
				
				// reset the handle
				if( indexDepth > 0 )
				{
					tmpHandle.popIndex( indexDepth );
				}
			}

			tmpHandle.popIndex();
		}
	}

	return ERROR_NONE;
}

ErrorType NxParameters::rawResizeArray(const Handle &handle, int new_size)
{
	size_t offset;
	ErrorType ret;
	void *ptr=NULL;

	Handle arrayRootHandle(handle);

	// We should consider storing alignment in dynamic array struct at some point
	physx::PxU32 align = arrayRootHandle.parameterDefinition()->alignment();
	if( 0 == align )
		align = 8; // Default alignment

	int dimension = 0;

	while(arrayRootHandle.parameterDefinition()->parent() &&
		arrayRootHandle.parameterDefinition()->parent()->type() == TYPE_ARRAY)
	{
		arrayRootHandle.popIndex();
		dimension++;
	}

	getVarPtr(arrayRootHandle, ptr, offset);
	if ( ptr == NULL ) 
		return ERROR_TYPE_NOT_SUPPORTED;

	DummyDynamicArrayStruct *dynArray = (DummyDynamicArrayStruct *)ptr;
	int old_size = dynArray->arraySizes[0];

	releaseDownsizedParameters(handle, new_size, old_size);

	ret = resizeArray(mParameterizedTraits,
					  dynArray->buf, 
					  dynArray->arraySizes, 
					  arrayRootHandle.parameterDefinition()->arrayDimension(), 
					  dimension, 
					  new_size,
					  dynArray->isAllocated,
					  dynArray->elementSize,
					  align,
					  dynArray->isAllocated); 

	initNewResizedParameters(handle, new_size, old_size);

	return ret;
}

ErrorType NxParameters::rawGetArraySize(const Handle &array_handle, int &size, int dimension) const
{
	size_t offset;
	void *ptr=NULL;
	getVarPtr(array_handle, ptr, offset);
	if ( ptr == NULL ) 
		return ERROR_TYPE_NOT_SUPPORTED;
	DummyDynamicArrayStruct *dynArray = (DummyDynamicArrayStruct *)ptr;
	size = dynArray->arraySizes[dimension];
	return(ERROR_NONE);
}

ErrorType NxParameters::rawSwapArrayElements(const Handle &array_handle, unsigned int firstElement, unsigned int secondElement)
{
	size_t offset = 0;
	void* ptr = NULL;
	Handle arrayRootHandle(array_handle);

	getVarPtr(arrayRootHandle, ptr, offset);
	if ( ptr == NULL ) 
		return ERROR_TYPE_NOT_SUPPORTED;

	DummyDynamicArrayStruct* dynArray = reinterpret_cast<DummyDynamicArrayStruct*>(ptr);

	const int elementSize = dynArray->elementSize;

	char tempDataStack[64];
	void* tempData = tempDataStack;

	void* tempDataHeap = NULL;
	if (elementSize > 64)
	{
		tempDataHeap = mParameterizedTraits->alloc((physx::PxU32)elementSize);
		tempData = tempDataHeap;
	}

	char* firstPtr = (char*)dynArray->buf + elementSize * firstElement;
	char* secondPtr = (char*)dynArray->buf + elementSize * secondElement;

	memcpy(tempData, firstPtr, (physx::PxU32)elementSize);
	memcpy(firstPtr, secondPtr, (physx::PxU32)elementSize);
	memcpy(secondPtr, tempData, (physx::PxU32)elementSize);

	if (tempDataHeap != NULL)
	{
		mParameterizedTraits->free(tempDataHeap);
	}

	return (ERROR_NONE);
}

#define ARRAY_HANDLE_CHECKS \
    if(!array_handle.isValid()) \
        return(ERROR_INVALID_PARAMETER_HANDLE); \
    if(array_handle.parameterDefinition()->type() != TYPE_ARRAY) \
        return(ERROR_NOT_AN_ARRAY); \

ErrorType NxParameters::resizeArray(const Handle &array_handle, physx::PxI32 new_size)
{
    ARRAY_HANDLE_CHECKS

    if(new_size < 0)
        return(ERROR_INVALID_ARRAY_SIZE);

    if(array_handle.parameterDefinition()->arraySizeIsFixed())
        return(ERROR_ARRAY_SIZE_IS_FIXED);

    return(rawResizeArray(array_handle, new_size));
}

ErrorType NxParameters::swapArrayElements(const Handle &array_handle, physx::PxU32 firstElement, physx::PxU32 secondElement)
{
	ARRAY_HANDLE_CHECKS;

	if (firstElement == secondElement)
		return (ERROR_NONE);

	int array_size = 0;
	rawGetArraySize(array_handle, array_size, 0);

	if (array_size < 1)
		return (ERROR_NONE);

	if (firstElement >= (unsigned int)array_size)
		return (ERROR_INDEX_OUT_OF_RANGE);

	if (secondElement >= (unsigned int)array_size)
		return (ERROR_INDEX_OUT_OF_RANGE);

	// well, maybe swapping will work on fixed size arrays as well...
	if (array_handle.parameterDefinition()->arraySizeIsFixed())
		return (ERROR_ARRAY_SIZE_IS_FIXED);

	return rawSwapArrayElements(array_handle, firstElement, secondElement);
}

ErrorType NxParameters::
    getArraySize(const Handle &array_handle, physx::PxI32 &size, physx::PxI32 dimension) const
{
    ARRAY_HANDLE_CHECKS

    if(dimension < 0 || dimension >= array_handle.parameterDefinition()->arrayDimension())
        return(ERROR_INVALID_ARRAY_DIMENSION);

    if(array_handle.parameterDefinition()->arraySizeIsFixed())
    {
        size = array_handle.parameterDefinition()->arraySize(dimension);
        return(ERROR_NONE);
    }

    return(rawGetArraySize(array_handle, size, dimension));
}

ErrorType NxParameters::callPreSerializeCallback(Handle& handle) const
{
	const Definition* def = handle.parameterDefinition();
	if (def->type() == TYPE_REF)
	{
		// don't preSerialize Named References
		const Hint *hint = def->hint("INCLUDED");
		if( hint && 
			hint->type() == TYPE_U64 && 
			hint->asUInt() == 1 )
		{
			// included
			NxParameterized::Interface* ref = NULL;
			NX_ERR_CHECK_RETURN( getParamRef(handle, ref) );
			if( ref )
			{
				NX_ERR_CHECK_RETURN( ref->callPreSerializeCallback() );
			}
		}
	}
	else if (def->type() == TYPE_ARRAY)
	{
		physx::PxI32 arraySize = 0;
		getArraySize(handle, arraySize);

		for (physx::PxI32 i = 0; i < arraySize; i++)
		{
			handle.set(i);

			NX_ERR_CHECK_RETURN( callPreSerializeCallback(handle) );

			handle.popIndex();
		}
	}
	else if (def->type() == TYPE_STRUCT)
	{
		const physx::PxI32 numChildren = def->numChildren();

		for (physx::PxI32 childIndex = 0; childIndex < numChildren; childIndex++)
		{
			handle.set(childIndex);

			NX_ERR_CHECK_RETURN( callPreSerializeCallback(handle) );

			handle.popIndex();
		}
	}

	return ERROR_NONE;
}

ErrorType NxParameters::checkParameterHandle(const Handle &handle) const
{
	PX_ASSERT( handle.getConstInterface() );
	PX_ASSERT( handle.getConstInterface() == this );
	if (!handle.isValid()) return(ERROR_INVALID_PARAMETER_HANDLE);
	if (handle.getConstInterface() == NULL ) return(ERROR_HANDLE_MISSING_INTERFACE_POINTER);
	if (handle.getConstInterface() != this ) return(ERROR_HANDLE_INVALID_INTERFACE_POINTER);
    if(handle.parameterDefinition()->root() != rootParameterDefinition())
        return(ERROR_PARAMETER_HANDLE_DOES_NOT_MATCH_CLASS);


    const Definition *ptr = rootParameterDefinition();
    for(physx::PxI32 i=0; i < handle.numIndexes(); ++i)
    {
        PX_ASSERT(ptr != NULL);

        switch(ptr->type())
        {
            case TYPE_STRUCT:
                if(handle.index(i) >= ptr->numChildren())
                    return(ERROR_INDEX_OUT_OF_RANGE);
                ptr = ptr->child(handle.index(i));
                break;

            case TYPE_ARRAY:
                {
                    physx::PxI32 size = ptr->arraySize();
                    Handle tmpHandle(handle);
					tmpHandle.popIndex(handle.numIndexes() - i);

                    if(size <= 0)
                        if(getArraySize(tmpHandle, size) != ERROR_NONE)
                            return(ERROR_INDEX_OUT_OF_RANGE);

                    if(handle.index(i) >= size)
                         return(ERROR_INDEX_OUT_OF_RANGE);

                    ptr = ptr->child(0);
                }
                break;

            default:
                PX_ALWAYS_ASSERT();
                ptr = NULL;
        }
    }

    return(ERROR_NONE);
}

ErrorType NxParameters::clone(NxParameterized::Interface *&nullDestObject) const
{
	PX_ASSERT(nullDestObject == NULL );
	nullDestObject = mParameterizedTraits->createNxParameterized(className(), version());
	if( !nullDestObject )
		return ERROR_OBJECT_CONSTRUCTION_FAILED;

	ErrorType err = nullDestObject->copy(*this);
	if( ERROR_NONE != err )
	{
		nullDestObject->destroy();
		nullDestObject = NULL;
	}

	return err;
}

ErrorType NxParameters::copy(const NxParameterized::Interface &other,
                             Handle &thisHandle)
{
   const Definition *paramDef = thisHandle.parameterDefinition();
   ErrorType error = ERROR_NONE;

    if( paramDef->type() == TYPE_STRUCT )
	{
        for(physx::PxI32 i=0; i < paramDef->numChildren(); ++i)
        {
            thisHandle.set(i);
			error = copy(other, thisHandle);
            NX_ERR_CHECK_RETURN(error);

            thisHandle.popIndex();
        }
        return(error);
	}
	else if( paramDef->type() == TYPE_ARRAY )
    {
        physx::PxI32 thisSize, otherSize;

		error = thisHandle.getArraySize(thisSize);
        NX_ERR_CHECK_RETURN(error);

		Handle otherHandle = thisHandle;
		otherHandle.setInterface(&other);
        error = otherHandle.getArraySize(otherSize);
        NX_ERR_CHECK_RETURN(error);

		thisHandle.setInterface(this);

		if(thisSize != otherSize)
		{
			error = thisHandle.resizeArray(otherSize);
			NX_ERR_CHECK_RETURN(error);
			thisSize = otherSize;
		}

        for(physx::PxI32 i=0; i < otherSize; ++i)
        {
            thisHandle.set(i);

			error = this->copy(other, thisHandle);
			NX_ERR_CHECK_RETURN(error);

            thisHandle.popIndex();
        }
        return(error);
    }

	Handle otherHandle = thisHandle; 
	otherHandle.setInterface(&other);
    switch(paramDef->type()) 
    {
		case TYPE_BOOL: 
			{ 
				bool otherVal; 
				error = otherHandle.getParamBool(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamBool(otherVal) );
			}

		case TYPE_I8: 
			{ 
				physx::PxI8 otherVal; 
				error = otherHandle.getParamI8(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamI8(otherVal) );
			}

		case TYPE_I16 : 
			{ 
				physx::PxI16 otherVal; 
				error = otherHandle.getParamI16(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamI16(otherVal) );
			}
		case TYPE_I32 : 
			{
				physx::PxI32 otherVal;
				error = otherHandle.getParamI32(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamI32(otherVal) );
			}
		case TYPE_I64 : 
			{
				physx::PxI64 otherVal;
				error = otherHandle.getParamI64(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamI64(otherVal) );
			}

		case TYPE_U8 : 
			{
				physx::PxU8 otherVal;
				error = otherHandle.getParamU8(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamU8(otherVal) );
			}
		case TYPE_U16 : 
			{
				physx::PxU16 otherVal;
				error = otherHandle.getParamU16(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamU16(otherVal) );
			}
		case TYPE_U32 : 
			{
				physx::PxU32 otherVal;
				error = otherHandle.getParamU32(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamU32(otherVal) );
			}
		case TYPE_U64 : 
			{
				physx::PxU64 otherVal;
				error = otherHandle.getParamU64(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamU64(otherVal) );
			}

		case TYPE_F32 : 
			{
				physx::PxF32 otherVal;
				error = otherHandle.getParamF32(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamF32(otherVal) );
			}
		case TYPE_F64 : 
			{
				physx::PxF64 otherVal;
				error = otherHandle.getParamF64(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamF64(otherVal) );
			}

		case TYPE_VEC2 : 
			{
				physx::PxVec2 otherVal;
				error = otherHandle.getParamVec2(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamVec2(otherVal) );
			}
		case TYPE_VEC3 : 
			{
				physx::PxVec3 otherVal;
				error = otherHandle.getParamVec3(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamVec3(otherVal) );
			}
		case TYPE_VEC4 : 
			{
				physx::PxVec4 otherVal;
				error = otherHandle.getParamVec4(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamVec4(otherVal) );
			}
		case TYPE_QUAT : 
			{
				physx::PxQuat otherVal;
				error = otherHandle.getParamQuat(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamQuat(otherVal) );
			}
		case TYPE_MAT33 : 
			{
				physx::PxMat33 otherVal;
				error = otherHandle.getParamMat33(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamMat33(otherVal) );
			}
		case TYPE_MAT34 : 
			{ 
				physx::PxMat44 otherVal;
				error = otherHandle.getParamMat34(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamMat34(otherVal) );
			}
		case TYPE_MAT44 : 
			{ 
				physx::PxMat44 otherVal;
				error = otherHandle.getParamMat44(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamMat44(otherVal) );
			}
		case TYPE_BOUNDS3 : 
			{ 
				physx::PxBounds3 otherVal;
				error = otherHandle.getParamBounds3(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamBounds3(otherVal) );
			}
		case TYPE_TRANSFORM : 
			{ 
				physx::PxTransform otherVal;
				error = otherHandle.getParamTransform(otherVal);
				NX_ERR_CHECK_RETURN( error );
				return( thisHandle.setParamTransform(otherVal) );
			}
        case TYPE_STRING:
            {
                const char *val1;
				error = otherHandle.getParamString(val1);
                NX_ERR_CHECK_RETURN(error);
				return(thisHandle.setParamString(val1));
            }

        case TYPE_ENUM:
            {
                const char *val1;
                error = otherHandle.getParamEnum(val1);
                NX_ERR_CHECK_RETURN(error);
				return val1 ? thisHandle.setParamEnum(val1) : ERROR_NONE;
            }

		case TYPE_REF:
			{
				NxParameterized::Interface *thisRef, *otherRef;
				error = thisHandle.getParamRef(thisRef);
				NX_ERR_CHECK_RETURN(error);
				
				error = otherHandle.getParamRef(otherRef);
				NX_ERR_CHECK_RETURN(error);

				if(thisRef)
				{
					thisRef->destroy();
					thisHandle.setParamRef(NULL);
				}

				if(otherRef)
				{
					error = thisHandle.initParamRef(otherRef->className(), true);
					NX_ERR_CHECK_RETURN(error);

					error = thisHandle.getParamRef(thisRef);
					NX_ERR_CHECK_RETURN(error);

					if(thisRef == NULL)
						return(ERROR_PARAMETER_HANDLE_NOT_INITIALIZED);

					return(thisRef->copy(*otherRef));
				}
			}
			break;

		case TYPE_POINTER:
			//Just don't do anything with pointers
			break;

        default:
            PX_ALWAYS_ASSERT();
            break;
    }

	return ERROR_NONE;
}

bool NxParameters::areParamsOK(Handle &handle, Handle *invalidHandles, physx::PxU32 numInvalidHandles, physx::PxU32 &numRemainingHandles)
{
	class Constraints
	{
		const Hint *min, *max, *multOf, *powOf;

		static physx::PxI64 abs(physx::PxI64 x)
		{
			return x >= 0 ? x : -x;
		}

	public:
		Constraints(const Hint *min_, const Hint *max_, const Hint *multOf_, const Hint *powOf_):
			min(min_), max(max_), multOf(multOf_), powOf(powOf_) {}

		bool isOK(physx::PxU64 val)
		{
			bool res = true;

			if( min )
				res = res && val >= min->asUInt();

			if( max )
				res = res && val <= max->asUInt();

			if( multOf )
				res = res && 0 == val % multOf->asUInt();

			if( powOf  )
			{
				//TODO: this is too slow
				physx::PxU64 base = powOf->asUInt(), acc = 1;
				while( acc < val )
					acc *= base;
				res = res && acc == base;
			}

			return res;
		}

		bool isOK(physx::PxI64 val)
		{
			bool res = true;

			if( min )
				res = res && val >= physx::PxI64(min->asUInt());

			if( max )
				res = res && val <= physx::PxI64(max->asUInt());

			if( multOf )
				res = res && 0 == val % physx::PxI64(multOf->asUInt());

			if( powOf  )
			{
				//TODO: this is too slow
				physx::PxI64 base = physx::PxI64(powOf->asUInt()), acc = 1;
				while( abs(acc) < val )
					acc *= base;
				res = res && abs(acc) == base;
			}

			return res;
		}

		bool isOK(physx::PxF32 val)
		{
			bool res = true;

			if( min )
				res = res && val >= physx::PxF32(min->asFloat());

			if( max )
				res = res && val <= physx::PxF32(max->asFloat());

			if( multOf )
				res = res && 0.0f == physx::mod(val, physx::PxF32(multOf->asFloat()));

			//TODO
			if( powOf)
			{
				PX_ALWAYS_ASSERT();
				return false;
			}

			return res;
		}

		bool isOK(physx::PxF64 val)
		{
			bool res = true;

			if( min )
				res = res && val >= min->asFloat();

			if( max )
				res = res && val <= max->asFloat();

			if( multOf )
				res = res && 0.0f == physx::mod((physx::PxF32)val, (physx::PxF32)multOf->asFloat());

			//TODO
			if( powOf )
			{
				PX_ALWAYS_ASSERT();
				return false;
			}

			return res;
		}
	};
	
	bool res = true;

    const Definition *pd = handle.parameterDefinition();

	Constraints con(pd->hint("min"), pd->hint("max"), pd->hint("multipleOf"), pd->hint("powerOf"));

	switch( pd->type() ) 
    {
        case TYPE_STRUCT:
            {
                for(physx::PxI32 i = 0; i < pd->numChildren(); ++i)
                {
                    handle.set(i);
                    res &= areParamsOK(handle, invalidHandles, numInvalidHandles, numRemainingHandles);
                    handle.popIndex();
                }
	            return res;
            }

        case TYPE_ARRAY:
            {
                physx::PxI32 arraySize = -1;
                NX_BOOL_RETURN( handle.getArraySize(arraySize) );

                for(physx::PxI32 i=0; i < arraySize; ++i)
                {
                    handle.set(i);
                    res &= areParamsOK(handle, invalidHandles, numInvalidHandles, numRemainingHandles);
                    handle.popIndex();
                }
	            return res;
            }

		case TYPE_U8:
		case TYPE_U16:
		case TYPE_U32:
		case TYPE_U64:
			{
				physx::PxU64 val;
				NX_BOOL_RETURN( handle.getParamU64(val) );
				res = con.isOK(val);
				if( !res && numRemainingHandles > 0 )
					invalidHandles[numRemainingHandles++ - 1] = handle;
				return res;
			}

		case TYPE_I8:
		case TYPE_I16:
		case TYPE_I32:
		case TYPE_I64:
			{
				physx::PxI64 val;
				NX_BOOL_RETURN( handle.getParamI64(val) );
				res = con.isOK(val);
				if( !res && numRemainingHandles > 0 )
					invalidHandles[numRemainingHandles++ - 1] = handle;
				return res;
			}

		case TYPE_F32:
			{
				physx::PxF32 val = -1;
				NX_BOOL_RETURN( handle.getParamF32(val) );
				res = con.isOK(val);
				if( !res && numRemainingHandles > 0 )
					invalidHandles[numRemainingHandles++ - 1] = handle;
				return res;
			}

		case TYPE_F64:
			{
				physx::PxF64 val = -1;
				NX_BOOL_RETURN( handle.getParamF64(val) );
				res = con.isOK(val);
				if( !res && numRemainingHandles > 0 )
					invalidHandles[numRemainingHandles++ - 1] = handle;
				return res;
			}

		default:
			return true;
	}
}

bool NxParameters::equals(const NxParameterized::Interface &obj,
						  Handle &param_handle,
						  Handle *handlesOfInequality,
						  physx::PxU32 numHandlesOfInequality,
						  bool doCompareNotSerialized) const
{
    const Definition *paramDef = param_handle.parameterDefinition();

	if (!doCompareNotSerialized && paramDef->hint("DONOTSERIALIZE") )
		return true;

	DataType type = paramDef->type();

    switch(type) 
    {
#define NX_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#define NX_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
	case TYPE_ ## enum_name: \
	{ \
		c_type a; \
		c_type b; \
		param_handle.setInterface(this); \
		NX_BOOL_RETURN( param_handle.getParam ## type_name(a) ); \
		param_handle.setInterface(&obj); \
		NX_BOOL_RETURN( param_handle.getParam ## type_name(b) ); \
		return !notEqual(a, b); \
	}
#include "NxParameterized_types.h"

		case TYPE_REF:
			{
				NxParameterized::Interface *val1, *val2;

				param_handle.setInterface( this );
				if(param_handle.getParamRef(val1) != ERROR_NONE)
					return(false);

				param_handle.setInterface( &obj );
				if(param_handle.getParamRef(val2) != ERROR_NONE)
					return(false);

				if(val1 == NULL && val2 == NULL)
					return(true);
				else if(val1 == NULL || val2 == NULL)
					return(false);

				return val2->equals(
					*val1,
					handlesOfInequality != NULL ? handlesOfInequality+1 : NULL,
					numHandlesOfInequality > 0 ? numHandlesOfInequality-1 : 0,
					doCompareNotSerialized );
			}

		case TYPE_STRUCT:
	        for(physx::PxI32 i = 0; i < paramDef->numChildren(); ++i)
		    {
			    param_handle.set(i);
				if (!equals(obj, param_handle, handlesOfInequality, numHandlesOfInequality, doCompareNotSerialized))
					return(false);
	            param_handle.popIndex();
		    }
	        return(true);

		case TYPE_ARRAY:
	    {
		    physx::PxU32 arraySize1, arraySize2;

			param_handle.setInterface (this);
		    if (param_handle.getArraySize((physx::PxI32&)arraySize1) != ERROR_NONE)
			    return(false);

			param_handle.setInterface (&obj);
		    if (param_handle.getArraySize((physx::PxI32&)arraySize2) != ERROR_NONE)
			    return(false);

	        if(arraySize1 != arraySize2)
		        return(false);

			if( arraySize1 > 100 && paramDef->isSimpleType(false, false) )
			{
				// Large array of simple types, fast path

				switch( type)
				{
				default:
					// Fall to slow path
					break;

#define NX_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#define NX_PARAMETERIZED_TYPES_NO_STRING_TYPES
#define NX_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
	case TYPE_ ## enum_name: { \
		physx::PxU32 byteSize = sizeof(c_type) * arraySize1; \
		c_type *data1 = (c_type *)mParameterizedTraits->alloc(byteSize), \
			*data2 = (c_type *)mParameterizedTraits->alloc(byteSize); \
		\
		param_handle.setInterface(this); \
		NX_BOOL_RETURN(param_handle.getParam ## type_name ## Array(data1, (physx::PxI32)arraySize1)); \
		\
		param_handle.setInterface(&obj); \
		NX_BOOL_RETURN(param_handle.getParam ## type_name ## Array(data2, (physx::PxI32)arraySize2)); \
		\
		int ret = memcmp(data1, data2, byteSize); \
		mParameterizedTraits->free(data1); \
		mParameterizedTraits->free(data2); \
		\
		return ret == 0; \
	}
#include "NxParameterized_types.h"

				}
			}

			// Array of aggregates, slow path

			param_handle.setInterface(this);

			for (physx::PxI32 i = 0; i < (physx::PxI32)arraySize1; ++i)
		    {
			    param_handle.set(i);
				if (!equals(obj, param_handle, handlesOfInequality, numHandlesOfInequality, doCompareNotSerialized))
					return(false);
				param_handle.popIndex();
			}

			return(true);
		}

		case TYPE_POINTER:
			return true;

        default:
			PX_ALWAYS_ASSERT();
            break;
    }

    return(false);
}
                   
ErrorType NxParameters::copy(const NxParameterized::Interface &other)
{
	if (this == &other)
		return(ERROR_NONE);

    if (rootParameterDefinition() != other.rootParameterDefinition())
		return(ERROR_PARAMETER_DEFINITIONS_DO_NOT_MATCH);

	// support empty, named references
	if (rootParameterDefinition() == NULL)
	{
		// the name or className could be NULL, strcmp doesn't like NULL strings...
		setClassName(other.className());
		setName(other.name());
		return(ERROR_NONE);
	}
	else
	{
		Handle handle (*this);
		if (getParameterHandle("", handle) != ERROR_NONE)
			return(ERROR_INVALID_PARAMETER_HANDLE);

		return(copy(other, handle));
	}
}

bool NxParameters::areParamsOK(Handle *invalidHandles, physx::PxU32 numInvalidHandles)
{
    Handle handle(*this);
	NX_BOOL_RETURN( getParameterHandle("", handle) );

	physx::PxU32 numRemainingHandles = numInvalidHandles;
	return areParamsOK(handle, invalidHandles, numInvalidHandles, numRemainingHandles);
}

bool NxParameters::equals(const NxParameterized::Interface &obj, Handle* handleOfInequality, physx::PxU32 numHandlesOfInequality, bool doCompareNotSerialized) const
{
	if( this == &obj )
		return(true);

    if(rootParameterDefinition() != obj.rootParameterDefinition())
        return(false);

	// support empty, named references
	if(rootParameterDefinition() == NULL)
	{
		// the name or className could be NULL, strcmp doesn't like NULL strings...
		return 0 == safe_strcmp(name(), obj.name()) && 0 == safe_strcmp(className(), obj.className());
	}

	// This should be a handle that can not set any values!
    Handle constHandle(*this);
	NX_BOOL_RETURN( getParameterHandle("", constHandle) );

	bool theSame = equals(obj, constHandle, handleOfInequality, numHandlesOfInequality, doCompareNotSerialized);

	if (!theSame && numHandlesOfInequality > 0)
		*handleOfInequality = constHandle;

	return theSame;
}

ErrorType NxParameters::valueToStr(const Handle &handle,
                                   char *buf,
                                   physx::PxU32 n,
                                   const char *&ret)
{
    PX_ASSERT(buf != NULL);
    PX_ASSERT(n > 0);

    if(!handle.isValid())
        return(ERROR_INVALID_PARAMETER_HANDLE);

    const Definition *paramDef = handle.parameterDefinition();

    ErrorType error = ERROR_TYPE_NOT_SUPPORTED;

    switch(paramDef->type())
    {
    	case TYPE_VEC2:
    		{
        		physx::PxVec2 val;
                if ((error = getParamVec2(handle, val)) == ERROR_NONE)
                {
					char f[2][physx::PxAsc::PxF32StrLen];
					physx::string::sprintf_s(buf, n,"%s %s",
						 physx::PxAsc::valueToStr(val.x, f[0], physx::PxAsc::PxF32StrLen),
						 physx::PxAsc::valueToStr(val.y, f[1], physx::PxAsc::PxF32StrLen));
    				ret = buf;
    			}
			}
    		break;
    	case TYPE_VEC3:
    		{
        		physx::PxVec3 val;
                if ((error = getParamVec3(handle, val)) == ERROR_NONE)
                {
					char f[3][physx::PxAsc::PxF32StrLen];
					physx::string::sprintf_s(buf, n,"%s %s %s",
						 physx::PxAsc::valueToStr(val.x, f[0], physx::PxAsc::PxF32StrLen),
						 physx::PxAsc::valueToStr(val.y, f[1], physx::PxAsc::PxF32StrLen),
						 physx::PxAsc::valueToStr(val.z, f[2], physx::PxAsc::PxF32StrLen));
    				ret = buf;
    			}
			}
    		break;
    	case TYPE_VEC4:
    		{
        		physx::PxVec4 val;
                if ((error = getParamVec4(handle, val)) == ERROR_NONE)
                {
					char f[4][physx::PxAsc::PxF32StrLen];
					physx::string::sprintf_s(buf, n,"%s %s %s %s",
						 physx::PxAsc::valueToStr(val.x, f[0], physx::PxAsc::PxF32StrLen),
						 physx::PxAsc::valueToStr(val.y, f[1], physx::PxAsc::PxF32StrLen),
						 physx::PxAsc::valueToStr(val.z, f[2], physx::PxAsc::PxF32StrLen),
						 physx::PxAsc::valueToStr(val.w, f[3], physx::PxAsc::PxF32StrLen));
    				ret = buf;
    			}
			}
    		break;
    	case TYPE_QUAT:
			{
				physx::PxQuat val;
				if ((error = getParamQuat(handle, val)) == ERROR_NONE)
				{
					physx::PxF32 quat[4];
					//val.getXYZW(quat);
					quat[0] = val.x; quat[1] = val.y; quat[2] = val.z; quat[3] = val.w;
					char f[4][physx::PxAsc::PxF32StrLen];
					physx::string::sprintf_s(buf, n,"%s %s %s %s",
						 physx::PxAsc::valueToStr(quat[0], f[0], physx::PxAsc::PxF32StrLen),
						 physx::PxAsc::valueToStr(quat[1], f[1], physx::PxAsc::PxF32StrLen),
						 physx::PxAsc::valueToStr(quat[2], f[2], physx::PxAsc::PxF32StrLen),
						 physx::PxAsc::valueToStr(quat[3], f[3], physx::PxAsc::PxF32StrLen));

					ret = buf;
				}
			}
    		break;
    	case TYPE_MAT33:
			{
				physx::PxMat33 val;
				if ((error = getParamMat33(handle, val)) == ERROR_NONE)
				{
					char f[9][physx::PxAsc::PxF32StrLen];
					const physx::PxReal *vals = val.front();
					physx::string::sprintf_s(buf, n,"%s %s %s  %s %s %s  %s %s %s",
						physx::PxAsc::valueToStr(vals[0], f[0], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[1], f[1], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[2], f[2], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[3], f[3], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[4], f[4], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[5], f[5], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[6], f[6], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[7], f[7], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[8], f[8], physx::PxAsc::PxF32StrLen));
					ret = buf;
				}
			}
    		break;
    	case TYPE_MAT34:
			{
				physx::PxMat34Legacy val;
				if ((error = getParamMat34(handle, val)) == ERROR_NONE)
				{
					char f[12][physx::PxAsc::PxF32StrLen];
					physx::PxF32 data[9];
					val.M.getColumnMajor(data);
					physx::string::sprintf_s(buf, n,"%s %s %s  %s %s %s  %s %s %s  %s %s %s",
						physx::PxAsc::valueToStr(data[0], f[0], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(data[1], f[1], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(data[2], f[2], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(data[3], f[3], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(data[4], f[4], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(data[5], f[5], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(data[6], f[6], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(data[7], f[7], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(data[8], f[8], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.t.x, f[9], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.t.y, f[10], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.t.z, f[11], physx::PxAsc::PxF32StrLen));
					ret = buf;
				}
			}
    		break;
    	case TYPE_MAT44:
			{
				physx::PxMat44 val;
				if ((error = getParamMat44(handle, val)) == ERROR_NONE)
				{
					char f[16][physx::PxAsc::PxF32StrLen];
					const physx::PxReal *vals = val.front();
					physx::string::sprintf_s(buf, n,"%s %s %s %s  %s %s %s %s  %s %s %s %s  %s %s %s %s",
						physx::PxAsc::valueToStr(vals[0], f[0], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[1], f[1], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[2], f[2], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[3], f[3], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[4], f[4], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[5], f[5], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[6], f[6], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[7], f[7], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[8], f[8], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[9], f[9], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[10], f[10], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[11], f[11], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[12], f[12], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[13], f[13], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[14], f[14], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(vals[15], f[15], physx::PxAsc::PxF32StrLen));
					ret = buf;
				}
			}
    		break;
    	case TYPE_BOUNDS3:
			{
				char f[6][physx::PxAsc::PxF32StrLen];
				physx::PxBounds3 val;
				if ((error = getParamBounds3(handle, val)) == ERROR_NONE)
				{
					physx::string::sprintf_s(buf, n,"%s %s %s  %s %s %s ",
						physx::PxAsc::valueToStr(val.minimum.x, f[0], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.minimum.y, f[1], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.minimum.z, f[2], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.maximum.x, f[3], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.maximum.y, f[4], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.maximum.z, f[5], physx::PxAsc::PxF32StrLen));
					ret = buf;
				}
			}
    		break;
    	case TYPE_TRANSFORM:
			{
				char f[7][physx::PxAsc::PxF32StrLen];
				physx::PxTransform val;
				if ((error = getParamTransform(handle, val)) == ERROR_NONE)
				{
					physx::string::sprintf_s(buf, n,"%s %s %s %s  %s %s %s ",
						physx::PxAsc::valueToStr(val.q.x, f[0], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.q.y, f[1], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.q.z, f[2], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.q.w, f[3], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.p.x, f[4], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.p.y, f[5], physx::PxAsc::PxF32StrLen),
						physx::PxAsc::valueToStr(val.p.z, f[6], physx::PxAsc::PxF32StrLen));
					ret = buf;
				}
			}
    		break;
        case TYPE_UNDEFINED:
        case TYPE_ARRAY:
        case TYPE_STRUCT:
			break;
        case TYPE_STRING:
            error = getParamString(handle, ret);
			break;
        case TYPE_ENUM:
            error = getParamEnum(handle, ret);
			break;
		case TYPE_REF:
		{
			const Hint *hint = paramDef->hint("INCLUDED");
			if( hint && hint->type() != TYPE_U64 )
				return(ERROR_INVALID_REFERENCE_INCLUDE_HINT);

			if( hint != NULL && hint->asUInt() == 1 )
			{
				// included, output the entire struct
			}
			else
			{
				//not included, get the "name" from the NxParameterized pointer
				NxParameterized::Interface *paramPtr = 0;
				ErrorType err = getParamRef(handle, paramPtr);
				if(err != ERROR_NONE)
				{
					return err;
				}

				ret = NULL;
				if(paramPtr)
				{
					ret = paramPtr->name();
				}
			}
			error = ERROR_NONE;
		}
		break;
		case TYPE_BOOL:
    		{
        		bool val;
                if ((error = getParamBool(handle, val)) == ERROR_NONE)
                {
					ret = physx::PxAsc::valueToStr(val, buf, n);
    			}
			}
			break;
		case TYPE_I8:
    		{
        		physx::PxI8 val;
                if ((error = getParamI8(handle, val)) == ERROR_NONE)
                {
					ret = physx::PxAsc::valueToStr(val, buf, n);
    			}
			}
			break;
		case TYPE_I16:
    		{
        		physx::PxI16 val;
                if ((error = getParamI16(handle, val)) == ERROR_NONE)
                {
                	ret = physx::PxAsc::valueToStr(val, buf, n);
    			}
			}
			break;
		case TYPE_I32:
    		{
        		physx::PxI32 val;
                if ((error = getParamI32(handle, val)) == ERROR_NONE)
                {
                	ret = physx::PxAsc::valueToStr(val, buf, n);
    			}
			}
			break;
		case TYPE_I64:
    		{
        		physx::PxI64 val;
                if ((error = getParamI64(handle, val)) == ERROR_NONE)
                {
           			ret = physx::PxAsc::valueToStr(val, buf, n);
    			}
			}
			break;
		case TYPE_U8:
    		{
        		physx::PxU8 val;
                if ((error = getParamU8(handle, val)) == ERROR_NONE)
                {
					ret = physx::PxAsc::valueToStr(val, buf, n);
    			}
			}
			break;
		case TYPE_U16:
    		{
        		physx::PxU16 val;
                if ((error = getParamU16(handle, val)) == ERROR_NONE)
                {
                	ret = physx::PxAsc::valueToStr(val, buf, n);
    			}
			}
			break;
		case TYPE_U32:
    		{
        		physx::PxU32 val;
                if ((error = getParamU32(handle, val)) == ERROR_NONE)
                {
                	ret = physx::PxAsc::valueToStr(val, buf, n);
    			}
			}
			break;
		case TYPE_U64:
    		{
        		physx::PxU64 val;
                if ((error = getParamU64(handle, val)) == ERROR_NONE)
                {
              		ret = physx::PxAsc::valueToStr(val, buf, n);
    			}
			}
			break;
		case TYPE_F32:
    		{
        		physx::PxF32 val;
                if ((error = getParamF32(handle, val)) == ERROR_NONE)
                {
                  physx::string::sprintf_s(buf, n,"%s",physx::PxAsc::valueToStr(val, buf, n));
    			  ret = buf;
    			}
			}
			break;
		case TYPE_F64:
    		{
        		physx::PxF64 val;
                if ((error = getParamF64(handle, val)) == ERROR_NONE)
                {
                	physx::string::sprintf_s(buf, n,"%s",physx::PxAsc::valueToStr(val, buf, n));
					ret = buf;
    			}
			}
			break;

		// Make compiler happy
		case TYPE_POINTER:
		case TYPE_LAST:
			break;
    }

    return error;
}

ErrorType NxParameters::strToValue(Handle &handle, const char *str, const char **endptr) // assigns this string to the value
{
	ErrorType ret = ERROR_NONE;

	const Definition *pd = handle.parameterDefinition();

	switch ( pd->type() )
	{
        case TYPE_UNDEFINED:
        case TYPE_ARRAY:
        case TYPE_STRUCT:    ret = ERROR_TYPE_NOT_SUPPORTED;                                  break;

		case TYPE_STRING:    ret = setParamString(handle,str);                                break;
        case TYPE_ENUM:      ret = setParamEnum(handle,str);                                  break;
		case TYPE_REF:       ret = ERROR_TYPE_NOT_SUPPORTED;                                  break;
		case TYPE_BOOL:	     ret = setParamBool(handle,physx::PxAsc::strToBool(str, endptr)); break;
        case TYPE_I8:  	     ret = setParamI8(handle,physx::PxAsc::strToI8(str, endptr));     break;
        case TYPE_I16:       ret = setParamI16(handle,physx::PxAsc::strToI16(str, endptr));   break;
        case TYPE_I32:       ret = setParamI32(handle,physx::PxAsc::strToI32(str, endptr));   break;
        case TYPE_I64:       ret = setParamI64(handle,physx::PxAsc::strToI64(str, endptr));   break;
        case TYPE_U8:        ret = setParamU8(handle,physx::PxAsc::strToU8(str, endptr));     break;
        case TYPE_U16:       ret = setParamU16(handle,physx::PxAsc::strToU16(str, endptr));   break;
        case TYPE_U32:       ret = setParamU32(handle,physx::PxAsc::strToU32(str, endptr));   break;
        case TYPE_U64:       ret = setParamU64(handle,physx::PxAsc::strToU64(str, endptr));   break;
        case TYPE_F32:       ret = setParamF32(handle,physx::PxAsc::strToF32(str, endptr));   break;
        case TYPE_F64:       ret = setParamF64(handle,physx::PxAsc::strToF64(str, endptr));   break;
        case TYPE_VEC2:      ret = setParamVec2(handle,getVec2(str, endptr));                 break;
        case TYPE_VEC3:      ret = setParamVec3(handle,getVec3(str, endptr));                 break;
        case TYPE_VEC4:      ret = setParamVec4(handle,getVec4(str, endptr));                 break;
        case TYPE_QUAT:      ret = setParamQuat(handle,getQuat(str, endptr));                 break;
        case TYPE_MAT33:     ret = setParamMat33(handle,getMat33(str, endptr));               break;
        case TYPE_MAT34:     ret = setParamMat34(handle,getMat34(str, endptr));               break;
        case TYPE_MAT44:     ret = setParamMat44(handle,getMat44(str, endptr));               break;
        case TYPE_BOUNDS3:   ret = setParamBounds3(handle,getBounds3(str, endptr));           break;
        case TYPE_TRANSFORM: ret = setParamTransform(handle,getTransform(str, endptr));       break;

		default:             ret = ERROR_TYPE_NOT_SUPPORTED;                                  break;
    }

    return ret;
}

physx::PxVec2 		NxParameters::getVec2(const char *str, const char **endptr)
{
	physx::PxVec2 ret(0,0);
	physx::PxAsc::strToF32s(&ret.x,2,str,endptr);

	return ret;
}

physx::PxVec3 		NxParameters::getVec3(const char *str, const char **endptr)
{
	physx::PxVec3 ret(0,0,0);
	physx::PxAsc::strToF32s(&ret.x,3,str,endptr);

	return ret;
}

physx::PxVec4 		NxParameters::getVec4(const char *str, const char **endptr)
{
	physx::PxVec4 ret(0,0,0,0);
	physx::PxAsc::strToF32s(&ret.x,4,str,endptr);

	return ret;
}

physx::PxQuat 		NxParameters::getQuat(const char *str, const char **endptr)
{
	physx::PxQuat ret;
	//ret.identity();
	physx::PxF32 quat[4];
	//ret.getXYZW(quat);
	physx::PxAsc::strToF32s(quat,4,str,endptr);
	//ret.setXYZW(quat);
	ret = physx::PxQuat(quat[0], quat[1], quat[2], quat[3]);

	return ret;
}

physx::PxMat33 		NxParameters::getMat33(const char *str, const char **endptr)
{
	physx::PxMat33 ret;
	physx::PxReal *vals = const_cast<physx::PxReal *>(ret.front());
	physx::PxAsc::strToF32s(vals,9,str,endptr);
	return ret;
}

physx::PxMat34Legacy 		NxParameters::getMat34(const char *str, const char **endptr)
{
	physx::PxMat34Legacy ret;
	ret.id();
	physx::PxF32 mat[12];
	mat[9] = mat[10] = mat[11] = 0;
	ret.M.getColumnMajor(mat);
	physx::PxAsc::strToF32s(mat,12,str,endptr);
	ret.M.setColumnMajor(mat);
	ret.t = physx::PxVec3(mat[9],mat[10],mat[11]);
	return ret;
}

physx::PxMat44 		NxParameters::getMat44(const char *str, const char **endptr)
{
	physx::PxMat44 ret;
	physx::PxReal *vals = const_cast<physx::PxReal *>(ret.front());
	physx::PxAsc::strToF32s(vals,16,str,endptr);
	return ret;
}

physx::PxBounds3    NxParameters::getBounds3(const char *str, const char **endptr)
{
	physx::PxBounds3 ret;
	ret.setEmpty();
	physx::PxAsc::strToF32s(&ret.minimum.x,6,str,endptr);
	return ret;
}

//******************************************************************************
//*** Vec2
//******************************************************************************

ErrorType NxParameters::setParamVec2(const Handle &handle, const physx::PxVec2 &val)
{
    CHECK_HANDLE
    CHECK_F32_FINITE

	if(handle.parameterDefinition()->type() == TYPE_VEC2)
    {
        return rawSetParamVec2(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamVec2(const Handle &handle, physx::PxVec2 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_VEC2)
    {
        return rawGetParamVec2(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamVec2Array(const Handle &handle, physx::PxVec2 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(VEC2)
    return(rawGetParamVec2Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamVec2Array(const Handle &handle, const physx::PxVec2 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(VEC2)
    CHECK_F32_FINITE_ARRAY
    return(rawSetParamVec2Array(handle, array, n, offset));
}

//******************************************************************************
//*** Vec3
//******************************************************************************

ErrorType NxParameters::setParamVec3(const Handle &handle, const physx::PxVec3 &val)
{
    CHECK_HANDLE
    CHECK_F32_FINITE

	if(handle.parameterDefinition()->type() == TYPE_VEC3)
    {
        return rawSetParamVec3(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamVec3(const Handle &handle, physx::PxVec3 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_VEC3)
    {
        return rawGetParamVec3(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamVec3Array(const Handle &handle, physx::PxVec3 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(VEC3)
    return(rawGetParamVec3Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamVec3Array(const Handle &handle, const physx::PxVec3 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(VEC3)
    CHECK_F32_FINITE_ARRAY
    return(rawSetParamVec3Array(handle, array, n, offset));
}

//******************************************************************************
//*** Vec4
//******************************************************************************

ErrorType NxParameters::setParamVec4(const Handle &handle, const physx::PxVec4 &val)
{
    CHECK_HANDLE
    CHECK_F32_FINITE

	if(handle.parameterDefinition()->type() == TYPE_VEC4)
    {
        return rawSetParamVec4(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamVec4(const Handle &handle, physx::PxVec4 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_VEC4)
    {
        return rawGetParamVec4(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamVec4Array(const Handle &handle, physx::PxVec4 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(VEC4)
    return(rawGetParamVec4Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamVec4Array(const Handle &handle, const physx::PxVec4 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(VEC4)
    CHECK_F32_FINITE_ARRAY
    return(rawSetParamVec4Array(handle, array, n, offset));
}

//******************************************************************************
//*** Quat
//******************************************************************************

ErrorType NxParameters::setParamQuat(const Handle &handle, const physx::PxQuat &val)
{
    CHECK_HANDLE
    CHECK_F32_FINITE

	if(handle.parameterDefinition()->type() == TYPE_QUAT)
    {
        return rawSetParamQuat(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamQuat(const Handle &handle, physx::PxQuat &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_QUAT)
    {
        return rawGetParamQuat(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamQuatArray(const Handle &handle, physx::PxQuat *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(QUAT)
    return(rawGetParamQuatArray(handle, array, n, offset));
}

ErrorType NxParameters::setParamQuatArray(const Handle &handle, const physx::PxQuat *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(QUAT)
    CHECK_F32_FINITE_ARRAY
    return(rawSetParamQuatArray(handle, array, n, offset));
}

//******************************************************************************
//*** Mat33
//******************************************************************************

ErrorType NxParameters::setParamMat33(const Handle &handle, const physx::PxMat33 &val)
{
    CHECK_HANDLE
    CHECK_F32_FINITE

    	if(handle.parameterDefinition()->type() == TYPE_MAT33)
    {
        return rawSetParamMat33(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamMat33(const Handle &handle, physx::PxMat33 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_MAT33)
    {
        return rawGetParamMat33(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamMat33Array(const Handle &handle, physx::PxMat33 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(MAT33)
    return(rawGetParamMat33Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamMat33Array(const Handle &handle, const physx::PxMat33 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(MAT33)
    CHECK_F32_FINITE_ARRAY
    return(rawSetParamMat33Array(handle, array, n, offset));
}

//******************************************************************************
//*** Mat34
//******************************************************************************

ErrorType NxParameters::setParamMat34(const Handle &handle, const physx::PxMat34Legacy &val)
{
    CHECK_HANDLE
    CHECK_F32_FINITE

   	if(handle.parameterDefinition()->type() == TYPE_MAT34)
    {
        return rawSetParamMat34(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamMat34(const Handle &handle, physx::PxMat34Legacy &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_MAT34)
    {
        return rawGetParamMat34(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamMat34Array(const Handle &handle, physx::PxMat34Legacy *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(MAT34)
    return(rawGetParamMat34Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamMat34Array(const Handle &handle, const physx::PxMat34Legacy *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(MAT34)
    CHECK_F32_FINITE_ARRAY
    return(rawSetParamMat34Array(handle, array, n, offset));
}

//******************************************************************************
//*** Mat34 (Mat44 args)
//******************************************************************************

ErrorType NxParameters::setParamMat34(const Handle &handle, const physx::PxMat44 &val)
{
    CHECK_HANDLE
    CHECK_F32_FINITE

   	if(handle.parameterDefinition()->type() == TYPE_MAT44)
    {
        return rawSetParamMat44(handle, val);
    }
	else if(handle.parameterDefinition()->type() == TYPE_MAT34)
    {
		physx::PxMat34Legacy mat34 = val;
        return rawSetParamMat34(handle, mat34);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamMat34(const Handle &handle, physx::PxMat44 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_MAT44)
    {
        return rawGetParamMat44(handle, val);
    }
	else if(handle.parameterDefinition()->type() == TYPE_MAT34)
    {
		physx::PxMat34Legacy mat34; 
        ErrorType e = rawGetParamMat34(handle, mat34);
		val = physx::PxMat44(mat34);
		return e;
    }

    return(ERROR_CAST_FAILED);
}

#if NOT_IMPLEMENTED
ErrorType NxParameters::getParamMat34Array(const Handle &handle, physx::PxMat44 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(MAT34)
    return(rawGetParamMat34Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamMat34Array(const Handle &handle, const physx::PxMat44 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(MAT34)
    CHECK_F32_FINITE_ARRAY
    return(rawSetParamMat34Array(handle, array, n, offset));
}
#endif

//******************************************************************************
//*** Mat44
//******************************************************************************

ErrorType NxParameters::setParamMat44(const Handle &handle, const physx::PxMat44 &val)
{
    CHECK_HANDLE
    CHECK_F32_FINITE

   	if(handle.parameterDefinition()->type() == TYPE_MAT44)
    {
        return rawSetParamMat44(handle, val);
    }
	else if(handle.parameterDefinition()->type() == TYPE_MAT34)
    {
		return setParamMat34(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamMat44(const Handle &handle, physx::PxMat44 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_MAT44)
    {
        return rawGetParamMat44(handle, val);
    }
	else if(handle.parameterDefinition()->type() == TYPE_MAT34)
    {
		return getParamMat34(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamMat44Array(const Handle &handle, physx::PxMat44 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(MAT44)
    return(rawGetParamMat44Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamMat44Array(const Handle &handle, const physx::PxMat44 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(MAT44)
    CHECK_F32_FINITE_ARRAY
    return(rawSetParamMat44Array(handle, array, n, offset));
}

//******************************************************************************
//*** Bounds3
//******************************************************************************

ErrorType NxParameters::setParamBounds3(const Handle &handle, const physx::PxBounds3 &val)
{
    CHECK_HANDLE
    CHECK_F32_FINITE

	if(handle.parameterDefinition()->type() == TYPE_BOUNDS3)
    {
        return rawSetParamBounds3(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamBounds3(const Handle &handle, physx::PxBounds3 &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_BOUNDS3)
    {
        return rawGetParamBounds3(handle, val);
    }

    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamBounds3Array(const Handle &handle, physx::PxBounds3 *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(BOUNDS3)
    return(rawGetParamBounds3Array(handle, array, n, offset));
}

ErrorType NxParameters::setParamBounds3Array(const Handle &handle, const physx::PxBounds3 *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(BOUNDS3)
    CHECK_F32_FINITE_ARRAY
    return(rawSetParamBounds3Array(handle, array, n, offset));
}

//******************************************************************************
//*** Transform
//******************************************************************************

ErrorType NxParameters::setParamTransform(const Handle &handle, const physx::PxTransform &val)
{
    CHECK_HANDLE
    CHECK_F32_FINITE

   	if(handle.parameterDefinition()->type() == TYPE_TRANSFORM)
    {
        return rawSetParamTransform(handle, val);
    }
    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamTransform(const Handle &handle, physx::PxTransform &val) const
{
    CHECK_HANDLE

	if(handle.parameterDefinition()->type() == TYPE_TRANSFORM)
    {
        return rawGetParamTransform(handle, val);
    }
    return(ERROR_CAST_FAILED);
}

ErrorType NxParameters::getParamTransformArray(const Handle &handle, physx::PxTransform *array, physx::PxI32 n, physx::PxI32 offset) const
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(TRANSFORM)
    return(rawGetParamTransformArray(handle, array, n, offset));
}

ErrorType NxParameters::setParamTransformArray(const Handle &handle, const physx::PxTransform *array, physx::PxI32 n, physx::PxI32 offset)
{
    CHECK_HANDLE
    CHECK_IS_SIMPLE_ARRAY(TRANSFORM)
    CHECK_F32_FINITE_ARRAY
    return(rawSetParamTransformArray(handle, array, n, offset));
}

physx::PxTransform    NxParameters::getTransform(const char *str, const char **endptr)
{
	physx::PxTransform ret;
	physx::PxAsc::strToF32s((physx::PxF32*)&ret,7,str,endptr);
	return ret;
}


//***********************************************
physx::PxVec2 NxParameters::init(physx::PxF32 x,physx::PxF32 y)
{
	physx::PxVec2 ret(x,y);
	return ret;
}

physx::PxVec3 NxParameters::init(physx::PxF32 x,physx::PxF32 y,physx::PxF32 z)
{
	physx::PxVec3 ret(x,y,z);
	return ret;
}

physx::PxVec4 NxParameters::initVec4(physx::PxF32 x,physx::PxF32 y,physx::PxF32 z,physx::PxF32 w)
{
	physx::PxVec4 ret(x,y,z,w);
	return ret;
}

physx::PxQuat NxParameters::init(physx::PxF32 x,physx::PxF32 y,physx::PxF32 z,physx::PxF32 w)
{
	physx::PxQuat ret;
	//ret.setXYZW(x,y,z,w);
	ret = physx::PxQuat(x,y,z,w);
	return ret;
}

physx::PxMat33 NxParameters::init(physx::PxF32 _11,physx::PxF32 _12,physx::PxF32 _13,physx::PxF32 _21,physx::PxF32 _22,physx::PxF32 _23,physx::PxF32 _31,physx::PxF32 _32,physx::PxF32 _33)
{
	physx::PxMat33 ret;

	ret.column0.x = _11;
	ret.column0.y = _21;
	ret.column0.z = _31;

	ret.column1.x = _12;
	ret.column1.y = _22;
	ret.column1.z = _32;

	ret.column2.x = _13;
	ret.column2.y = _23;
	ret.column2.z = _33;

	return ret;
}

physx::PxMat34Legacy NxParameters::init(physx::PxF32 _11,physx::PxF32 _12,physx::PxF32 _13,physx::PxF32 _21,physx::PxF32 _22,physx::PxF32 _23,physx::PxF32 _31,physx::PxF32 _32,physx::PxF32 _33,physx::PxF32 x,physx::PxF32 y,physx::PxF32 z)
{
	physx::PxMat34Legacy ret;
	ret.t = physx::PxVec3(x,y,z);
	physx::PxF32 mat[9] = { _11, _12, _13, _21, _22, _23, _31, _32, _33 };
	ret.M.setColumnMajor(mat);
	return ret;
}

physx::PxMat44 NxParameters::init(physx::PxF32 _11,physx::PxF32 _12,physx::PxF32 _13,physx::PxF32 _14,physx::PxF32 _21,physx::PxF32 _22,physx::PxF32 _23,physx::PxF32 _24,physx::PxF32 _31,physx::PxF32 _32,physx::PxF32 _33,physx::PxF32 _34,physx::PxF32 _41,physx::PxF32 _42,physx::PxF32 _43,physx::PxF32 _44)
{
	physx::PxMat44 ret;

	ret.column0.x = _11;
	ret.column0.y = _21;
	ret.column0.z = _31;
	ret.column0.w = _41;

	ret.column1.x = _12;
	ret.column1.y = _22;
	ret.column1.z = _32;
	ret.column1.w = _42;

	ret.column2.x = _13;
	ret.column2.y = _23;
	ret.column2.z = _33;
	ret.column2.w = _43;

	ret.column3.x = _14;
	ret.column3.y = _24;
	ret.column3.z = _34;
	ret.column3.w = _44;

	return ret;
}

physx::PxTransform NxParameters::init(physx::PxF32 x,physx::PxF32 y,physx::PxF32 z,physx::PxF32 qx,physx::PxF32 qy,physx::PxF32 qz,physx::PxF32 qw)
{
	return physx::PxTransform(physx::PxVec3(x,y,z), physx::PxQuat(qx,qy,qz,qw));
}


physx::PxBounds3 NxParameters::init(physx::PxF32 minx,physx::PxF32 miny,physx::PxF32 minz,physx::PxF32 maxx,physx::PxF32 maxy,physx::PxF32 maxz)
{
	physx::PxBounds3 ret;
	ret.minimum = physx::PxVec3(minx,miny,minz);
	ret.maximum = physx::PxVec3(maxx,maxy,maxz);
	return ret;
}



physx::PxI32 NxParameters::MultIntArray(const physx::PxI32 *array, physx::PxI32 n)
{
    PX_ASSERT(array != NULL);
    PX_ASSERT(n > 0);

    physx::PxI32 ret = array[0];

    for(physx::PxI32 i=1; i < n; ++i)
        ret *= array[i];

    return(ret);
}


ErrorType NxParameters::resizeArray(Traits *parameterizedTraits,
													 void *&buf, 
													 physx::PxI32 *array_sizes, 
													 physx::PxI32 dimension, 
													 physx::PxI32 resize_dim,
													 physx::PxI32 new_size,
													 bool doFree,
													 physx::PxI32 element_size,
													 physx::PxU32 element_align,
													 bool &isMemAllocated)
{
    PX_ASSERT(array_sizes != NULL);
    PX_ASSERT(dimension > 0);
    PX_ASSERT(resize_dim >= 0 && resize_dim < dimension);
    PX_ASSERT(new_size >= 0);
	PX_ASSERT(element_size > 0 );

    if(array_sizes[resize_dim] == new_size)
        return(ERROR_NONE); //isMemAllocated is unchanged

    physx::PxI32 newSizes[Handle::MAX_DEPTH];
    memcpy(newSizes, array_sizes, dimension * sizeof(physx::PxI32));

    newSizes[resize_dim] = new_size;

    physx::PxI32 currentNumElems = MultIntArray(array_sizes, dimension);
    physx::PxI32 newNumElems = MultIntArray(newSizes, dimension);

    if(newNumElems <= 0)
    {
        if(buf != NULL && doFree) 
            parameterizedTraits->free(buf);

        buf = NULL;
        goto no_error;
    }

    if(buf == NULL) 
    {
        if((buf = allocAligned(parameterizedTraits, (physx::PxU32)element_size * newNumElems, element_align)) == NULL) 
            return(ERROR_MEMORY_ALLOCATION_FAILURE);

		isMemAllocated = true;
        
        //initialize the array to 0's (for strings)
        memset(buf, 0, (physx::PxU32)element_size * newNumElems);

        goto no_error;
    }

    if(resize_dim == 0)
    {
        void *newBuf;
        char *newBufDataStart;
        size_t newBufDataSize;
        
        // alloc new buffer
        if((newBuf = allocAligned(parameterizedTraits, (physx::PxU32)element_size * newNumElems, element_align)) == NULL) 
            return(ERROR_MEMORY_ALLOCATION_FAILURE);

		isMemAllocated = true;
        
        // copy existing data to new buffer
		if(newNumElems < currentNumElems)
			memcpy(newBuf, buf, (physx::PxU32)element_size * newNumElems);
		else
			memcpy(newBuf, buf, (physx::PxU32)element_size * currentNumElems);
        
        // zero the new part of the array
        if(newNumElems > currentNumElems)
        {
            newBufDataStart = (char *)newBuf + (currentNumElems * element_size);
            newBufDataSize = (physx::PxU32)(newNumElems - currentNumElems) * element_size;
            memset(newBufDataStart, 0, newBufDataSize);
        }
        
        if( doFree )
        parameterizedTraits->free(buf);
        buf = newBuf;
    }
    else
    {
        void *newBuf = allocAligned(parameterizedTraits, (physx::PxU32)element_size * newNumElems, element_align);
        if(newBuf == NULL)
            return(ERROR_MEMORY_ALLOCATION_FAILURE);

		isMemAllocated = true;

        //initialize the array to 0's (for strings)
        memset(newBuf, 0, (physx::PxU32)element_size * newNumElems);

        recursiveCopy(buf, array_sizes, newBuf, newSizes, dimension, element_size);

		if( doFree )
        parameterizedTraits->free(buf);
        buf = newBuf;
    }
      

no_error:

    array_sizes[resize_dim] = new_size;
    return(ERROR_NONE);
}

void NxParameters::recursiveCopy(const void *src, 
								 const physx::PxI32 *src_sizes, 
								 void *dst, 
								 const physx::PxI32 *dst_sizes, 
								 physx::PxI32 dimension, 
								 physx::PxI32 element_size,
								 physx::PxI32 *indexes, 
								 physx::PxI32 level)
{
    physx::PxI32 srcSize = src_sizes[level];
    physx::PxI32 dstSize = dst_sizes[level];

    physx::PxI32 size = physx::PxMin(srcSize, dstSize);

    physx::PxI32 indexStore[Handle::MAX_DEPTH];

    if(indexes == NULL)
    {
        indexes = indexStore;
        memset(indexes, 0, Handle::MAX_DEPTH * sizeof(physx::PxI32));
    }

    if(level == dimension - 1)
    {
        physx::PxI32 srcIndex = indexes[0];
        physx::PxI32 dstIndex = indexes[0];

        for(physx::PxI32 i=1; i < dimension; ++i)
        {
            srcIndex = src_sizes[i] * (srcIndex) + indexes[i];
            dstIndex = dst_sizes[i] * (dstIndex) + indexes[i];
        }

		char *copy_dst = (char *)dst + dstIndex * element_size;
		char *copy_src = (char *)src + srcIndex * element_size;

        memcpy(copy_dst, copy_src, (physx::PxU32)element_size * size);
        return;
    }

    for(physx::PxI32 i=0; i < size; ++i)
    {
        indexes[level] = i;
        recursiveCopy(src, src_sizes, dst, dst_sizes, dimension, element_size, indexes, level + 1);
    }
}

ErrorType NxParameters::getParamU8Array(const Handle &handle, physx::PxU8 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	CHECK_HANDLE
	CHECK_IS_SIMPLE_ARRAY(U8)
	return(rawGetParamArray<physx::PxU8>(handle, array, n, offset,this));
}

ErrorType NxParameters::getParamF64Array(const Handle &handle, physx::PxF64 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	CHECK_HANDLE
	CHECK_IS_SIMPLE_ARRAY(F64)
	return(rawGetParamArray<physx::PxF64>(handle, array, n, offset,this));
}




ErrorType NxParameters::rawSetParamBool(const Handle &handle, bool val)
{
	return rawSetParam<bool>(handle,val,this);
}

ErrorType NxParameters::rawGetParamBool(const Handle &handle, bool &val) const
{
	return rawGetParam<bool>(handle,val,this);
}

ErrorType NxParameters::rawGetParamBoolArray(const Handle &handle, bool *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<bool>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamBoolArray(const Handle &handle, const bool *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<bool>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawGetParamString(const Handle &handle, const char *&val) const
{
	size_t offset;
	void *ptr=NULL;
	getVarPtr(handle, ptr, offset);
	if ( ptr == NULL )
		return ERROR_TYPE_NOT_SUPPORTED;
	DummyStringStruct *var = (DummyStringStruct *)(char *)ptr;
	val = var->buf;
	return(ERROR_NONE);
}

ErrorType NxParameters::rawSetParamString(const Handle &handle, const char *val)
{
	size_t offset;
	void *ptr=NULL;
	getVarPtr(handle, ptr, offset);
	if(ptr == NULL)
	{
		return(ERROR_INDEX_OUT_OF_RANGE);
	}

	DummyStringStruct *var = (DummyStringStruct *)(char *)ptr;
	if( var->isAllocated )
	{
		getTraits()->strfree((char *)var->buf);
	}

	var->buf = getTraits()->strdup(val);
	var->isAllocated = true;

	return(ERROR_NONE);
}

ErrorType NxParameters::rawGetParamEnum(const Handle &handle, const char *&val) const
{
	size_t offset;
	void *ptr=NULL;
	getVarPtr(handle, ptr, offset);
	if ( ptr == NULL )
		return ERROR_TYPE_NOT_SUPPORTED;
	const char * *var = (const char * *)((char *)ptr);
	val = *var;
	return(ERROR_NONE);
}

ErrorType NxParameters::rawSetParamEnum(const Handle &handle, const char *val)
{
	size_t offset;
	void *ptr=NULL;
	getVarPtr(handle, ptr, offset);
	if(ptr == NULL)
		return(ERROR_INDEX_OUT_OF_RANGE);
	const char * *Var = (const char * *)((char *)ptr);
	*Var = val;
	return(ERROR_NONE);
}

ErrorType NxParameters::rawGetParamRef(const Handle &handle, NxParameterized::Interface *&val) const
{
	size_t offset;
	void *ptr=NULL;
	getVarPtr(handle, ptr, offset);
	if ( ptr == NULL )
		return ERROR_TYPE_NOT_SUPPORTED;
	NxParameterized::Interface * *var = (NxParameterized::Interface * *)((char *)ptr);
	val = *var;
	return(ERROR_NONE);
}

ErrorType NxParameters::rawSetParamRef(const Handle &handle, NxParameterized::Interface * val)
{
	size_t offset;
	void *ptr=NULL;
	getVarPtr(handle, ptr, offset);
	if(ptr == NULL)
		return(ERROR_INDEX_OUT_OF_RANGE);

	NxParameterized::Interface * *Var = (NxParameterized::Interface * *)((char *)ptr);
	*Var = val;
	return(ERROR_NONE);
}

ErrorType NxParameters::rawGetParamI8(const Handle &handle, physx::PxI8 &val) const
{
	return rawGetParam<physx::PxI8>(handle,val,this);
}

ErrorType NxParameters::rawSetParamI8(const Handle &handle, physx::PxI8 val)
{
	return rawSetParam<physx::PxI8>(handle,val,this);
}

ErrorType NxParameters::rawGetParamI8Array(const Handle &handle, physx::PxI8 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxI8>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamI8Array(const Handle &handle, const physx::PxI8 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxI8>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawGetParamI16(const Handle &handle, physx::PxI16 &val) const
{
	return rawGetParam<physx::PxI16>(handle,val,this);
}

ErrorType NxParameters::rawSetParamI16(const Handle &handle, physx::PxI16 val)
{
	return rawSetParam<physx::PxI16>(handle,val,this);
}

ErrorType NxParameters::rawGetParamI16Array(const Handle &handle, physx::PxI16 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxI16>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamI16Array(const Handle &handle, const physx::PxI16 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxI16>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawGetParamI32(const Handle &handle, physx::PxI32 &val) const
{
	return rawGetParam<physx::PxI32>(handle,val,this);
}

ErrorType NxParameters::rawSetParamI32(const Handle &handle, physx::PxI32 val)
{
	return rawSetParam<physx::PxI32>(handle,val,this);
}

ErrorType NxParameters::rawGetParamI32Array(const Handle &handle, physx::PxI32 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxI32>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamI32Array(const Handle &handle, const physx::PxI32 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxI32>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawGetParamI64(const Handle &handle, physx::PxI64 &val) const
{
	return rawGetParam<physx::PxI64>(handle,val,this);
}

ErrorType NxParameters::rawSetParamI64(const Handle &handle, physx::PxI64 val)
{
	return rawSetParam<physx::PxI64>(handle,val,this);
}

ErrorType NxParameters::rawGetParamI64Array(const Handle &handle, physx::PxI64 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxI64>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamI64Array(const Handle &handle, const physx::PxI64 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxI64>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawGetParamU8(const Handle &handle, physx::PxU8 &val) const
{
	return rawGetParam<physx::PxU8>(handle,val,this);
}

ErrorType NxParameters::rawSetParamU8(const Handle &handle, physx::PxU8 val)
{
	return rawSetParam<physx::PxU8>(handle,val,this);
}

ErrorType NxParameters::rawGetParamU8Array(const Handle &handle, physx::PxU8 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxU8>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamU8Array(const Handle &handle, const physx::PxU8 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxU8>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawGetParamU16(const Handle &handle, physx::PxU16 &val) const
{
	return rawGetParam<physx::PxU16>(handle,val,this);
}

ErrorType NxParameters::rawSetParamU16(const Handle &handle, physx::PxU16 val)
{
	return rawSetParam<physx::PxU16>(handle,val,this);
}

ErrorType NxParameters::rawGetParamU16Array(const Handle &handle, physx::PxU16 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxU16>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamU16Array(const Handle &handle, const physx::PxU16 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxU16>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawGetParamU32(const Handle &handle, physx::PxU32 &val) const
{
	return rawGetParam<physx::PxU32>(handle,val,this);
}

ErrorType NxParameters::rawSetParamU32(const Handle &handle, physx::PxU32 val)
{
	return rawSetParam<physx::PxU32>(handle,val,this);
}

ErrorType NxParameters::rawGetParamU32Array(const Handle &handle, physx::PxU32 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxU32>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamU32Array(const Handle &handle, const physx::PxU32 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxU32>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawSetParamU64(const Handle &handle, physx::PxU64 val)
{
	return rawSetParam<physx::PxU64>(handle,val,this);
}

ErrorType NxParameters::rawGetParamU64(const Handle &handle, physx::PxU64 &val) const
{
	return rawGetParam<physx::PxU64>(handle,val,this);
}

ErrorType NxParameters::rawGetParamU64Array(const Handle &handle, physx::PxU64 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxU64>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamU64Array(const Handle &handle, const physx::PxU64 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxU64>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawGetParamF32(const Handle &handle, physx::PxF32 &val) const
{
	return rawGetParam<physx::PxF32>(handle,val,this);
}

ErrorType NxParameters::rawSetParamF32(const Handle &handle, physx::PxF32 val)
{
	return rawSetParam<physx::PxF32>(handle,val,this);
}

ErrorType NxParameters::rawGetParamF32Array(const Handle &handle, physx::PxF32 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxF32>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamF32Array(const Handle &handle, const physx::PxF32 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxF32>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawGetParamF64(const Handle &handle, physx::PxF64 &val) const
{
	return rawGetParam<physx::PxF64>(handle,val,this);
}

ErrorType NxParameters::rawSetParamF64(const Handle &handle, physx::PxF64 val)
{
	return rawSetParam<physx::PxF64>(handle,val,this);
}

ErrorType NxParameters::rawGetParamF64Array(const Handle &handle, physx::PxF64 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxF64>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamF64Array(const Handle &handle, const physx::PxF64 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxF64>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawSetParamVec2(const Handle &handle,physx::PxVec2 val)
{
	return rawSetParam<physx::PxVec2>(handle,val,this);
}

ErrorType NxParameters::rawGetParamVec2(const Handle &handle,physx::PxVec2 &val) const
{
	return rawGetParam<physx::PxVec2>(handle,val,this);
}

ErrorType NxParameters::rawGetParamVec2Array(const Handle &handle,physx::PxVec2 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxVec2>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamVec2Array(const Handle &handle, const physx::PxVec2 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxVec2>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawSetParamVec3(const Handle &handle,physx::PxVec3 val)
{
	return rawSetParam<physx::PxVec3>(handle,val,this);
}

ErrorType NxParameters::rawGetParamVec3(const Handle &handle,physx::PxVec3 &val) const
{
	return rawGetParam<physx::PxVec3>(handle,val,this);
}

ErrorType NxParameters::rawGetParamVec3Array(const Handle &handle,physx::PxVec3 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxVec3>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamVec3Array(const Handle &handle, const physx::PxVec3 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxVec3>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawSetParamVec4(const Handle &handle,physx::PxVec4 val)
{
	return rawSetParam<physx::PxVec4>(handle,val,this);
}

ErrorType NxParameters::rawGetParamVec4(const Handle &handle,physx::PxVec4 &val) const
{
	return rawGetParam<physx::PxVec4>(handle,val,this);
}

ErrorType NxParameters::rawGetParamVec4Array(const Handle &handle,physx::PxVec4 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxVec4>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamVec4Array(const Handle &handle, const physx::PxVec4 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxVec4>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawSetParamQuat(const Handle &handle,physx::PxQuat val)
{
	return rawSetParam<physx::PxQuat>(handle,val,this);
}

ErrorType NxParameters::rawGetParamQuat(const Handle &handle,physx::PxQuat &val) const
{
	return rawGetParam<physx::PxQuat>(handle,val,this);
}

ErrorType NxParameters::rawGetParamQuatArray(const Handle &handle,physx::PxQuat *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxQuat>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamQuatArray(const Handle &handle, const physx::PxQuat *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxQuat>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawSetParamMat33(const Handle &handle,physx::PxMat33 val)
{
	return rawSetParam<physx::PxMat33>(handle,val,this);
}

ErrorType NxParameters::rawGetParamMat33(const Handle &handle,physx::PxMat33 &val) const
{
	return rawGetParam<physx::PxMat33>(handle,val,this);
}

ErrorType NxParameters::rawGetParamMat33Array(const Handle &handle,physx::PxMat33 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxMat33>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamMat33Array(const Handle &handle, const physx::PxMat33 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxMat33>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawSetParamMat34(const Handle &handle,physx::PxMat34Legacy val)
{
	return rawSetParam<physx::PxMat34Legacy>(handle,val,this);
}

ErrorType NxParameters::rawGetParamMat34(const Handle &handle,physx::PxMat34Legacy &val) const
{
	return rawGetParam<physx::PxMat34Legacy>(handle,val,this);
}

ErrorType NxParameters::rawGetParamMat34Array(const Handle &handle,physx::PxMat34Legacy *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxMat34Legacy>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamMat34Array(const Handle &handle, const physx::PxMat34Legacy *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxMat34Legacy>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawSetParamMat44(const Handle &handle,physx::PxMat44 val)
{
	return rawSetParam<physx::PxMat44>(handle,val,this);
}

ErrorType NxParameters::rawGetParamMat44(const Handle &handle,physx::PxMat44 &val) const
{
	return rawGetParam<physx::PxMat44>(handle,val,this);
}

ErrorType NxParameters::rawGetParamMat44Array(const Handle &handle,physx::PxMat44 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxMat44>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamMat44Array(const Handle &handle, const physx::PxMat44 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxMat44>(handle,array,n,offset,this);
}


ErrorType NxParameters::rawSetParamBounds3(const Handle &handle,physx::PxBounds3 val)
{
	return rawSetParam<physx::PxBounds3>(handle,val,this);
}

ErrorType NxParameters::rawGetParamBounds3(const Handle &handle,physx::PxBounds3 &val) const
{
	return rawGetParam<physx::PxBounds3>(handle,val,this);
}

ErrorType NxParameters::rawGetParamBounds3Array(const Handle &handle,physx::PxBounds3 *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxBounds3>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamBounds3Array(const Handle &handle, const physx::PxBounds3 *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxBounds3>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamTransform(const Handle &handle,physx::PxTransform val)
{
	return rawSetParam<physx::PxTransform>(handle,val,this);
}

ErrorType NxParameters::rawGetParamTransform(const Handle &handle,physx::PxTransform  &val) const
{
	return rawGetParam<physx::PxTransform>(handle,val,this);
}

ErrorType NxParameters::rawGetParamTransformArray(const Handle &handle,physx::PxTransform *array, physx::PxI32 n, physx::PxI32 offset) const
{
	return rawGetParamArray<physx::PxTransform>(handle,array,n,offset,this);
}

ErrorType NxParameters::rawSetParamTransformArray(const Handle &handle, const physx::PxTransform *array, physx::PxI32 n, physx::PxI32 offset)
{
	return rawSetParamArray<physx::PxTransform>(handle,array,n,offset,this);
}

void *NxParameters::getVarPtrHelper(const ParamLookupNode *rootNode, void *paramStruct, const Handle &handle, size_t &offset) const
{
	const ParamLookupNode* curNode = rootNode;

	bool hasDynamicArray = false;
	offset = curNode->offset;

	void *ptr = const_cast<void *>(paramStruct);
	for(physx::PxI32 i = 0; i < handle.numIndexes(); ++i)
	{
		int index = handle.index(i);

		if (curNode->type == TYPE_ARRAY)
		{
			PX_ASSERT(curNode->numChildren);

			if (curNode->isDynamicArrayRoot)
			{
				ptr = ((DummyDynamicArrayStruct*)ptr)->buf;
				hasDynamicArray = true;
				if (ptr == NULL)
				{
					offset = 0;
					return 0;
				}
			}

			// don't get the next curNode until after we've checked that the "parent" is dynamic
			curNode = &rootNode[curNode->children[0]];

			size_t localOffset = index * curNode->offset;
			offset += localOffset;
			ptr = (char*)ptr + localOffset;
		}
		else
		{
			PX_ASSERT(index >= 0 && index < curNode->numChildren);
			curNode = &rootNode[curNode->children[index]];
			offset += curNode->offset;
			ptr = (char*)ptr + curNode->offset;
		}
	}

	if (hasDynamicArray)
	{
		offset = 0;
	}

	return ptr;
}

bool NxParameters::checkAlignments() const
{
	// support empty, named references
	if(rootParameterDefinition() == NULL)
	{
		return IsAligned(this, 8);
	}

    Handle constHandle(*this, "");
	if( !constHandle.isValid() )
	{
		return false;
	}

	return checkAlignments(constHandle);
}

bool NxParameters::checkAlignments(Handle &param_handle) const
{
    const Definition *paramDef = param_handle.parameterDefinition();

	physx::PxU32 align = paramDef->alignment();

	bool isDynamicArray = TYPE_ARRAY == paramDef->type() && !paramDef->arraySizeIsFixed();

	// For dynamic array alignment means alignment of it's first element
	if( !isDynamicArray )
	{
		size_t offset;
		void *ptr;
		getVarPtr(param_handle, ptr, offset);
		if( align > 0 && !IsAligned(ptr, align) )
		{
			return false;
		}
	}

	switch( paramDef->type() )
	{
	case TYPE_STRUCT:
		{
			for(physx::PxI32 i = 0; i < paramDef->numChildren(); ++i)
			{
				param_handle.set(i);
				if( !checkAlignments(param_handle) )
				{
					return false;
				}
				param_handle.popIndex();
			}
			break;
		}

	case TYPE_REF:
		{
			Interface *refObj;
			if( ERROR_NONE != param_handle.getParamRef(refObj) )
			{
				return false;
			}

			return 0 == refObj || refObj->checkAlignments();
		}

	case TYPE_ARRAY:
		{
			physx::PxI32 size;
			if( ERROR_NONE != param_handle.getArraySize(size) )
			{
				return false;
			}

			// See comment above
			if( isDynamicArray && align > 0 && size > 0 )
			{
				param_handle.set(0);

				size_t offset;
				void *ptr;
				getVarPtr(param_handle, ptr, offset);
				if( !IsAligned(ptr, align) )
				{
					return false;
				}

				param_handle.popIndex();
			}

			bool isSimpleType = paramDef->child(0)->isSimpleType();

			// Only check for first 10 elements if simple type
			size = physx::PxMin(size, isSimpleType ? 10 : PX_MAX_I32);

			for(physx::PxI32 i = 0; i < size; ++i)
			{
				param_handle.set(i);
				if( !checkAlignments(param_handle) )
				{
					return false;
				}
				param_handle.popIndex();
			}

			break;
		}

	default:
		break;
    }

    return(true);
}

} // namespace NxParameterized
