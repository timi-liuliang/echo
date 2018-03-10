/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApexDefs.h"
#if defined(APEX_CUDA_SUPPORT) //&& !defined(__CUDACC__)

#include "ApexCudaTest.h"
#include "ApexCudaWrapper.h"
#include "PsFile.h"
#include <cuda.h>
#include "NiModule.h"
#include "ApexSDKHelpers.h"

# define CUT_SAFE_CALL(call)  { CUresult ret = call;                         \
		if( CUDA_SUCCESS != ret ) {                                              \
			APEX_INTERNAL_ERROR("Cuda Error %d", ret);                         \
			PX_ASSERT(!ret); } }

#define ALIGN_OFFSET(offset, alignment) (offset) = ((offset) + (alignment) - 1) & ~((alignment) - 1)

#define WRITE_SCALAR(val) mMemBuf.alignWrite(4); mMemBuf.write(&val, sizeof(val));

#define WRITE_ALIGN_ARRAY(ptr, size, align) { PxU32 nsz = size; \
	mMemBuf.alignWrite(4); mMemBuf.write(&nsz, sizeof(nsz)); mMemBuf.alignWrite(align); mMemBuf.write(ptr, nsz); }

#define WRITE_ARRAY(ptr, size) WRITE_ALIGN_ARRAY(ptr, size, 4)

#define WRITE_STRING(str) { mMemBuf.alignWrite(4); str.serialize(mMemBuf); }

#define READ_SCALAR(val) mMemBuf->alignRead(4); mMemBuf->read(&val, sizeof(val));

#define READ_STRING(str) { mMemBuf->alignRead(4); str.deserialize(*mMemBuf); }

namespace physx
{
namespace apex
{

	ApexCudaTestKernelContextReader::ApexCudaTestKernelContextReader(const char* path, NiApexScene* scene)
		:	mMemBuf(NULL)
		,	mHeadCudaObj(NULL)
		,	mFunc(NULL)
		,	mApexScene(scene)
		,	mCuStream(NULL)
		,	mTmpArray(*scene, __FILE__, __LINE__)
		,	mCopyQueue(*(scene->getTaskManager()->getGpuDispatcher()))
		,	mCudaArrayCount(0)
		,	mCudaArrayList(NULL)
	{
		FILE* loadFile;
		loadFile = fopen(path, "rb");
		if (loadFile)
		{
			PxU32 serviceInfo[5];

			fread(serviceInfo, sizeof(PxU32), 5, loadFile);
			if (serviceInfo[0] != ApexCudaTestFileVersion)
			{
				PX_ASSERT(!"Unknown version of cuda context file");
			}
			fseek(loadFile, 0, 0);

			mMemBuf = PX_NEW(PxMemoryBuffer)(serviceInfo[1]);
			mMemBuf->initWriteBuffer(serviceInfo[1]);
			mCudaObjOffset = serviceInfo[3];
			mParamOffset = serviceInfo[4];
			fread((void*)mMemBuf->getWriteBuffer(), 1, serviceInfo[1], loadFile);
			
			// Header
			mMemBuf->seekRead(serviceInfo[2]);
			READ_STRING(mName);
			READ_STRING(mModuleName);
			READ_SCALAR(mFrame);
			READ_SCALAR(mCallPerFrame);

			READ_SCALAR(mFuncInstId);
			READ_SCALAR(mSharedSize);
			READ_SCALAR(mBlockDim.x);
			READ_SCALAR(mBlockDim.y);
			READ_SCALAR(mBlockDim.z);
			READ_SCALAR(mGridDim.x);
			READ_SCALAR(mGridDim.y);
			mGridDim.z = 0;
			READ_SCALAR(mKernelType);
			READ_SCALAR(mThreadCount[0]);
			READ_SCALAR(mThreadCount[1]);
			READ_SCALAR(mThreadCount[2]);
			READ_SCALAR(mBlockCountY);

			NiModuleScene* moduleScene = scene->getNiModuleScene(mModuleName.c_str());
			if (moduleScene)
			{
				mHeadCudaObj = static_cast<ApexCudaObj*>(moduleScene->getHeadCudaObj());
			}

			ApexCudaObj* obj = mHeadCudaObj;
			while(obj)
			{
				if (obj->getType() == ApexCudaObj::FUNCTION)				
				{				
					if (ApexSimpleString(DYNAMIC_CAST(ApexCudaFunc*)(obj)->getName()) == mName)
					{
						mFunc = DYNAMIC_CAST(ApexCudaFunc*)(obj);
						break;
					}
				}
				obj = obj->next();
			}
		}
	}

	ApexCudaTestKernelContextReader::~ApexCudaTestKernelContextReader()
	{
		if (mMemBuf)
		{
			PX_DELETE(mMemBuf);
		}
		if (mCudaArrayList)
		{
			PX_DELETE_ARRAY(mCudaArrayList);
		}
	}

	bool ApexCudaTestKernelContextReader::runKernel()
	{
		if (mFunc)
		{
			//launch1			
			ApexCudaFuncParams params;
			int* tmp = NULL;
			int itmp = 0;

			physx::PxScopedCudaLock _lock_(*mApexScene->getTaskManager()->getGpuDispatcher()->getCudaContextManager());
						
			mFunc->setParam(params, tmp);	// profile buffer (NULL)
			mFunc->setParam(params, itmp);	// kernelID (0)

			switch(mKernelType)
			{
			case apexCudaTest::KT_SYNC :
				PX_ASSERT(!"Not implemented!");
				break;			
			case apexCudaTest::KT_FREE2D :
				mFunc->setParam(params, mThreadCount[0]);
				mFunc->setParam(params, mThreadCount[1]);
				break;
			case apexCudaTest::KT_FREE3D :
				mFunc->setParam(params, mThreadCount[0]);
				mFunc->setParam(params, mThreadCount[1]);
				mFunc->setParam(params, mThreadCount[2]);
				mFunc->setParam(params, mBlockCountY);
				break;
			case apexCudaTest::KT_BOUND :
			case apexCudaTest::KT_FREE :
				mFunc->setParam(params, mThreadCount[0]);
				break;
			default :
				PX_ASSERT(!"Wrong kernel type");
			}

			loadContext(params);

			void *config[5] = {
				CU_LAUNCH_PARAM_BUFFER_POINTER, params.mParams,
				CU_LAUNCH_PARAM_BUFFER_SIZE,    &params.mOffset,
				CU_LAUNCH_PARAM_END
			};
			PX_ASSERT(mFuncInstId < mFunc->mFuncInstCount);
			CUT_SAFE_CALL(cuLaunchKernel(mFunc->mFuncInstData[mFuncInstId].mCuFunc, (physx::PxU32)mGridDim.x, (physx::PxU32)mGridDim.y, 1, (physx::PxU32)mBlockDim.x, (physx::PxU32)mBlockDim.y, (physx::PxU32)mBlockDim.z, mSharedSize, (CUstream)mCuStream, 0, (void **)config));

			mTmpArray.copyDeviceToHostQ(mCopyQueue);
			mCopyQueue.flushEnqueued();

			//copy mOutArrayRefs to host
			PxU32 outArrayRefsOffset = 0;
			for (PxU32 i = 0; i < mOutArrayRefs.size(); i++)
			{
				if (mOutArrayRefs[i].cudaArray != NULL)
				{
					outArrayRefsOffset += mOutArrayRefs[i].size;
				}
			}
			Array <PxU8> outArrayRefsBuffer(outArrayRefsOffset);
			outArrayRefsOffset = 0;
			for (PxU32 i = 0; i < mOutArrayRefs.size(); i++)
			{
				if (mOutArrayRefs[i].cudaArray != NULL)
				{
					mOutArrayRefs[i].cudaArray->copyToHost((CUstream)mCuStream, outArrayRefsBuffer.begin() + outArrayRefsOffset);
					outArrayRefsOffset += mOutArrayRefs[i].size;
				}
			}

			CUT_SAFE_CALL(cuStreamSynchronize((CUstream)mCuStream));

			for (PxU32 i = 0; i < mTexRefs.size(); i++)
			{
				if (mTexRefs[i].cudaTexRef)
				{
					mTexRefs[i].cudaTexRef->unbind();
				}
			}
			for (PxU32 i = 0; i < mSurfRefs.size(); i++)
			{
				if (mSurfRefs[i].cudaSurfRef)
				{
					mSurfRefs[i].cudaSurfRef->unbind();
				}
			}

			bool isOk = true;
			for (PxU32 i = 0; i < mOutMemRefs.size() && isOk; i++)
			{
				isOk = compare(
					(const PxU8*)mTmpArray.getPtr() + mOutMemRefs[i].bufferOffset,
					(const PxU8*)mOutMemRefs[i].gpuPtr,
					mOutMemRefs[i].size,
					mOutMemRefs[i].fpType,
					mOutMemRefs[i].name.c_str());
			}
			outArrayRefsOffset = 0;
			for (PxU32 i = 0; i < mOutArrayRefs.size() && isOk; i++)
			{
				if (mOutArrayRefs[i].cudaArray != NULL)
				{
					PxU32 fpType;
					switch (mOutArrayRefs[i].cudaArray->getFormat())
					{
					case CU_AD_FORMAT_HALF:
						fpType = 2;
						break;
					case CU_AD_FORMAT_FLOAT:
						fpType = 4;
						break;
					default:
						fpType = 0;
						break;
					};
					isOk = compare(
						outArrayRefsBuffer.begin() + outArrayRefsOffset,
						mOutArrayRefs[i].bufferPtr,
						mOutArrayRefs[i].size,
						fpType,
						mOutArrayRefs[i].name.c_str());
					outArrayRefsOffset += mOutArrayRefs[i].size;
				}
			}
			return isOk;
		}

		APEX_DEBUG_WARNING("can't find kernel '%s'", mName.c_str());
		return false;
	}

    class Float16Compressor
    {
        union Bits
        {
            float f;
            PxI32 si;
            PxU32 ui;
        };

        static int const shift = 13;
        static int const shiftSign = 16;

        static PxI32 const infN = 0x7F800000; // flt32 infinity
        static PxI32 const maxN = 0x477FE000; // max flt16 normal as a flt32
        static PxI32 const minN = 0x38800000; // min flt16 normal as a flt32
        static PxI32 const signN = 0x80000000; // flt32 sign bit

        static PxI32 const infC = infN >> shift;
        static PxI32 const nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
        static PxI32 const maxC = maxN >> shift;
        static PxI32 const minC = minN >> shift;
        static PxI32 const signC = signN >> shiftSign; // flt16 sign bit

        static PxI32 const mulN = 0x52000000; // (1 << 23) / minN
        static PxI32 const mulC = 0x33800000; // minN / (1 << (23 - shift))

        static PxI32 const subC = 0x003FF; // max flt32 subnormal down shifted
        static PxI32 const norC = 0x00400; // min flt32 normal down shifted

        static PxI32 const maxD = infC - maxC - 1;
        static PxI32 const minD = minC - subC - 1;

    public:
        static float decompress(PxU16 value)
        {
            Bits v;
            v.ui = value;
            PxI32 sign = v.si & signC;
            v.si ^= sign;
            sign <<= shiftSign;
            v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
            v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);
            Bits s;
            s.si = mulC;
            s.f *= v.si;
            PxI32 mask = -(norC > v.si);
            v.si <<= shift;
            v.si ^= (s.si ^ v.si) & mask;
            v.si |= sign;
            return v.f;
        }
    };

	bool ApexCudaTestKernelContextReader::compare(const PxU8* resData, const PxU8* refData, size_t size, PxU32 fpType, const char* name)
	{
		char str[4096];
		bool isOk = true;
		switch (fpType)
		{
		case 2:
			for (PxU32 j = 0; j < size && isOk; j += 2)
			{
				float ref = Float16Compressor::decompress(*reinterpret_cast<const PxU16*>(refData + j));
				float res = Float16Compressor::decompress(*reinterpret_cast<const PxU16*>(resData + j));
				isOk = physx::PxAbs(res - ref) <= 2.5e-3 * physx::PxMax(2.f, physx::PxAbs(res + ref));
				if (!isOk)
				{
					sprintf(str, "data mismatch at %d (%f != %f) in kernel '%s' param '%s'", (j / 2), res, ref, mName.c_str(), name);
					dumpParams(str);
					APEX_DEBUG_WARNING(str);
				}
			}
			break;
		case 4:
			for (PxU32 j = 0; j < size && isOk; j += 4)
			{
				float ref = *reinterpret_cast<const float*>(refData + j);
				float res = *reinterpret_cast<const float*>(resData + j);
				isOk = physx::PxAbs(res - ref) <= 2.5e-7 * physx::PxMax(2.f, physx::PxAbs(res + ref));
				if (!isOk)
				{
					sprintf(str, "data mismatch at %d (%f != %f) in kernel '%s' param '%s'", (j / 4), res, ref, mName.c_str(), name);
					dumpParams(str);
					APEX_DEBUG_WARNING(str);
				}
			}
			break;
		case 8:
			for (PxU32 j = 0; j < size && isOk; j += 8)
			{
				double ref = *reinterpret_cast<const double*>(refData + j);
				double res = *reinterpret_cast<const double*>(resData + j);
				isOk = physx::PxAbs(res - ref) <= 2.5e-14 * physx::PxMax(2., physx::PxAbs(res + ref));
				if (!isOk)
				{
					sprintf(str, "data mismatch at %d (%lf != %lf) in kernel '%s' param '%s'", (j / 8), res, ref, mName.c_str(), name);
					dumpParams(str);
					APEX_DEBUG_WARNING(str);
				}
			}
			break;
		default:
			for (PxU32 j = 0; j < size && isOk; j += 4)
			{
				int ref = *reinterpret_cast<const int*>(refData + j);
				int res = *reinterpret_cast<const int*>(resData + j);
				isOk = (res == ref);
				if (!isOk)
				{
					sprintf(str, "data mismatch at %d (%d != %d) in kernel '%s' param '%s'", (j / 4), res, ref, mName.c_str(), name);
					dumpParams(str);
					APEX_DEBUG_WARNING(str);
				}
			}
			break;
		};
		return isOk;
	}

	void ApexCudaTestKernelContextReader::dumpParams(char* str)
	{
		size_t len = strlen(str);
		str += len;
		*str++ = '\n';
		sprintf(str, "blockDim = (%d, %d, %d) GridDim = (%d, %d, %d) threadCount = (%d, %d, %d)", mBlockDim.x, mBlockDim.y, mBlockDim.z, mGridDim.x, mGridDim.y, mGridDim.z, mThreadCount[0], mThreadCount[1], mThreadCount[2]);
		for (PxU32 i = 0; i < mParamRefs.size(); ++i)
		{
			size_t len = strlen(str);
			str += len;
			*str++ = '\n';
			sprintf(str, "arg '%s' = 0x%x", mParamRefs[i].name.c_str(), mParamRefs[i].value);
		}
	}

	void ApexCudaTestKernelContextReader::loadContext(ApexCudaFuncParams& params)
	{
		PxU32 n;
		PxU32 cudaMemOffset = 0;

		//Read cuda objs
		mMemBuf->seekRead(mCudaObjOffset);
		READ_SCALAR(n)
		mCudaArrayList = PX_NEW(ApexCudaArray)[n];
		mCudaArrayCount = 0;
		for (PxU32 i = 0; i < n; i++)
		{
			PxU32 t;
			READ_SCALAR(t);
			switch(t)
			{
			case apexCudaTest::OBJ_TYPE_TEX_REF_MEM:
				loadTexRef(cudaMemOffset, false);
				break;
			case apexCudaTest::OBJ_TYPE_CONST_MEM:
				loadConstMem();
				break;
			case apexCudaTest::OBJ_TYPE_SURF_REF:
				loadSurfRef();
				break;
			case apexCudaTest::OBJ_TYPE_TEX_REF_ARR:
				loadTexRef(cudaMemOffset, true);
				break;
			default:
				PX_ASSERT(!"Wrong type");
				return;
			}
		}


		//Read call params
		mMemBuf->seekRead(mParamOffset);
		READ_SCALAR(n);
		PxU32 cudaMemOffsetPS = 0;
		for (PxU32 i = 0; i < n; i++)
		{
			cudaMemOffsetPS += getParamSize();
			ALIGN_OFFSET(cudaMemOffsetPS, APEX_CUDA_TEX_MEM_ALIGNMENT);
		}

		PxU32 arrSz = physx::PxMax(cudaMemOffset + cudaMemOffsetPS, 4U);
		mTmpArray.reserve(arrSz, ApexMirroredPlace::CPU_GPU);
		mTmpArray.setSize(arrSz);	

		mMemBuf->seekRead(this->mParamOffset + sizeof(n));
		for (PxU32 i = 0; i < n; i++)
		{
			loadParam(cudaMemOffset, params);
		}

		for (PxU32 i = 0; i < mInMemRefs.size(); i++)
		{
			memcpy(mTmpArray.getPtr() + mInMemRefs[i].bufferOffset, mInMemRefs[i].gpuPtr, mInMemRefs[i].size);
		}

		if (cudaMemOffset > 0)
		{
			mCopyQueue.reset((CUstream)mCuStream, 1);
			mTmpArray.copyHostToDeviceQ(mCopyQueue, cudaMemOffset);
			mCopyQueue.flushEnqueued();
		}
		for (PxU32 i = 0; i < mInArrayRefs.size(); i++)
		{
			if (mInArrayRefs[i].cudaArray != NULL)
			{
				mInArrayRefs[i].cudaArray->copyFromHost((CUstream)mCuStream, mInArrayRefs[i].bufferPtr);
			}
		}

		for (PxU32 i = 0; i < mTexRefs.size(); i++)
		{
			if (mTexRefs[i].cudaTexRef)
			{
				if (mTexRefs[i].memRefIdx != PxU32(-1))
				{
					const apexCudaTest::MemRef& memRef = mInMemRefs[ mTexRefs[i].memRefIdx ];
					mTexRefs[i].cudaTexRef->bindTo(mTmpArray.getGpuPtr() + memRef.bufferOffset, memRef.size);
				}
				else if (mTexRefs[i].cudaArray != NULL)
				{
					mTexRefs[i].cudaTexRef->bindTo(*mTexRefs[i].cudaArray);
				}
			}
		}
		for (PxU32 i = 0; i < mSurfRefs.size(); i++)
		{
			if (mSurfRefs[i].cudaArray != NULL)
			{
				mSurfRefs[i].cudaSurfRef->bindTo(*mSurfRefs[i].cudaArray, mSurfRefs[i].flags);
			}
		}
	}

	ApexCudaArray* ApexCudaTestKernelContextReader::loadCudaArray()
	{
		PxU32 format, numChannels, width, height, depth, flags;
		READ_SCALAR(format);
		READ_SCALAR(numChannels);
		READ_SCALAR(width);
		READ_SCALAR(height);
		READ_SCALAR(depth);
		READ_SCALAR(flags);

		CUDA_ARRAY3D_DESCRIPTOR desc;
		desc.Format = CUarray_format(format);
		desc.NumChannels = numChannels;
		desc.Width = width;
		desc.Height = height;
		desc.Depth = depth;
		desc.Flags = flags;

		ApexCudaArray* cudaArray = &mCudaArrayList[mCudaArrayCount++];
		cudaArray->create(desc);

		return cudaArray;
	}

	void ApexCudaTestKernelContextReader::loadTexRef(PxU32& memOffset, bool bBindToArray)
	{
		ApexSimpleString name;
		READ_STRING(name);

		TexRef texRef;
		texRef.memRefIdx = PxU32(-1);
		texRef.cudaArray = NULL;
		if (bBindToArray)
		{
			texRef.cudaArray = loadCudaArray();
			const PxU32 size = PxU32(texRef.cudaArray->getByteSize());

			mMemBuf->alignRead(4);
			mInArrayRefs.pushBack( ArrayRef(name.c_str(), texRef.cudaArray, mMemBuf->getReadLoc(), size) );
			mMemBuf->advanceReadLoc(size);
		}
		else
		{
			PxU32 size;
			READ_SCALAR(size);
			if (size > 0)
			{
				texRef.memRefIdx = mInMemRefs.size();

				mMemBuf->alignRead(4);
				mInMemRefs.pushBack( apexCudaTest::MemRef(mMemBuf->getReadLoc(), size, 0, memOffset) );
				mMemBuf->advanceReadLoc(size);

				memOffset += size; ALIGN_OFFSET(memOffset, APEX_CUDA_TEX_MEM_ALIGNMENT);
			}
		}

		//Find texture
		for (ApexCudaObj* obj = mHeadCudaObj; obj; obj = obj->next())
		{
			if (obj->getType() == ApexCudaObj::TEXTURE && strcmp(obj->getName(), name.c_str()) == 0)
			{
				texRef.cudaTexRef = DYNAMIC_CAST(ApexCudaTexRef*)(obj);
				mTexRefs.pushBack(texRef);
				break;
			}
		}
	}

	void ApexCudaTestKernelContextReader::loadSurfRef()
	{
		ApexSimpleString name;
		PxU32 flags;
		READ_STRING(name);
		READ_SCALAR(flags);

		SurfRef surfRef;
		surfRef.flags = ApexCudaMemFlags::Enum(flags);
		surfRef.cudaArray = loadCudaArray();
		const PxU32 size = PxU32(surfRef.cudaArray->getByteSize());

		if (surfRef.flags & ApexCudaMemFlags::IN)
		{
			mMemBuf->alignRead(4);
			mInArrayRefs.pushBack( ArrayRef(name.c_str(), surfRef.cudaArray, mMemBuf->getReadLoc(), size) );
			mMemBuf->advanceReadLoc(size);
		}
		if (surfRef.flags & ApexCudaMemFlags::OUT)
		{
			mMemBuf->alignRead(4);
			mOutArrayRefs.pushBack( ArrayRef(name.c_str(), surfRef.cudaArray, mMemBuf->getReadLoc(), size) );
			mMemBuf->advanceReadLoc(size);
		}

		//Find surface
		for (ApexCudaObj* obj = mHeadCudaObj; obj; obj = obj->next())
		{
			if (obj->getType() == ApexCudaObj::SURFACE && strcmp(obj->getName(), name.c_str()) == 0)
			{
				surfRef.cudaSurfRef = DYNAMIC_CAST(ApexCudaSurfRef*)(obj);
				mSurfRefs.pushBack(surfRef);
				break;
			}
		}
	}

	void ApexCudaTestKernelContextReader::loadConstMem()
	{
		PxU32 size;
		ApexSimpleString name;
		READ_STRING(name);
		READ_SCALAR(size);

		//Load const mem
		ApexCudaObj* obj = mHeadCudaObj;
		while(obj)
		{
			if (obj->getType() == ApexCudaObj::CONST_STORAGE)				
			{
				ApexCudaConstStorage* constMem = DYNAMIC_CAST(ApexCudaConstStorage*)(obj);
				if (ApexSimpleString(constMem->getName()) == name)
				{
					PX_ASSERT(constMem->mHostBuffer != 0);
					PX_ASSERT(constMem->mHostBuffer->getSize() >= size);
					void* hostPtr = reinterpret_cast<void*>(constMem->mHostBuffer->getPtr());

					mMemBuf->read(hostPtr, size);
					CUT_SAFE_CALL(cuMemcpyHtoDAsync(constMem->mDevPtr, hostPtr, size, NULL));
					break;
				}
			}
			obj = obj->next();
		}
	}

	PxU32 ApexCudaTestKernelContextReader::getParamSize()
	{
		ApexSimpleString name;
		PxU32 size, align, intent;
		PxI32 dataOffset;
		READ_STRING(name);
		READ_SCALAR(align);
		READ_SCALAR(intent);
		READ_SCALAR(dataOffset);
		READ_SCALAR(size);
		if (size > 0)
		{
			mMemBuf->alignRead(align);
			mMemBuf->advanceReadLoc(size);
		
			if ((intent & 3) == 3)
			{
				mMemBuf->alignRead(align);
				mMemBuf->advanceReadLoc(size);
			}
			if (intent & 3)
			{
				return size;
			}
		}
		return 0;
	}

	void ApexCudaTestKernelContextReader::loadParam(PxU32& memOffset, ApexCudaFuncParams& params)
	{
		ParamRef paramRef;
		PxU32 size, align, intent;
		PxI32 dataOffset;
		READ_STRING(paramRef.name);
		READ_SCALAR(align);
		READ_SCALAR(intent);
		READ_SCALAR(dataOffset);
		READ_SCALAR(size);
		if (size > 0)
		{
			if (!intent)	// scalar param
			{
				paramRef.value = *(PxU32*)(mMemBuf->getReadLoc());
				mParamRefs.pushBack(paramRef);

				mFunc->setParam(params, align, size, (void*)(mMemBuf->getReadLoc()));
				mMemBuf->advanceReadLoc(size);
			}
			else
			{
				mMemBuf->alignRead(align);
				mInMemRefs.pushBack(apexCudaTest::MemRef(mMemBuf->getReadLoc(), size, dataOffset, memOffset));
				if (intent & 0x01)	// input intent
				{
					mMemBuf->advanceReadLoc(size);
				}
				if (intent & 0x02)	// output intent
				{
					mMemBuf->alignRead(align);
					mOutMemRefs.pushBack(apexCudaTest::MemRef(mMemBuf->getReadLoc(), size, dataOffset, memOffset, intent >> 2));
					mOutMemRefs.back().name = paramRef.name;
					mMemBuf->advanceReadLoc(size);
				}
				void* ptr = mTmpArray.getGpuPtr() + memOffset - dataOffset;
				mFunc->setParam(params, align, sizeof(void*), &ptr);
				memOffset += size; ALIGN_OFFSET(memOffset, APEX_CUDA_TEX_MEM_ALIGNMENT);
			}
		}
		else
		{
			void* ptr = NULL;//mTmpArray.getGpuPtr() + memOffset - dataOffset;
			mFunc->setParam(params, align, sizeof(void*), &ptr);
		}
	}
	
	ApexCudaTestKernelContext::ApexCudaTestKernelContext(const char* path, const char* functionName, const char* moduleName, PxU32 frame, PxU32 callPerFrame, 
		bool isWriteForNonSuccessfulKernel, bool isContextForSave)
		:	mVersion(ApexCudaTestFileVersion)
		,	mFrame(frame)
		,	mCallPerFrame(callPerFrame)
		,	mPath(path)
		,	mName(functionName)
		,	mModuleName(moduleName)
		,	mCudaObjsCounter(0)
		,	mCallParamsCounter(0)
		,	mIsCompleteContext(false)
		,	mIsWriteForNonSuccessfulKernel(isWriteForNonSuccessfulKernel)
		,	mIsContextForSave(isContextForSave)
	{
		PxU32 writeLoc;
		// service info
		mMemBuf.setEndianMode(PxMemoryBuffer::ENDIAN_LITTLE);
		mMemBuf.write(&mVersion, sizeof(PxU32));		// Version of format
		mMemBuf.seekWrite(2 * sizeof(PxU32));			// Space for file size
		writeLoc = 32;									// Offset for header block
		mMemBuf.write(&writeLoc, sizeof(PxU32));
		
		// header info
		mMemBuf.seekWrite(writeLoc);		
		WRITE_STRING(mName)								// Name of function
		WRITE_STRING(mModuleName)						// Name of module
		WRITE_SCALAR(frame)								// Current frame
		WRITE_SCALAR(callPerFrame)						// Call of kernel per current frame

		writeLoc = mMemBuf.tellWrite();
		writeLoc += 12 * sizeof(PxU32);					// Space for cuda kernel parameters

		mCudaObjsOffset = writeLoc;						// Offset for cuda objects block
		mMemBuf.seekWrite(3 * sizeof(PxU32));	
		mMemBuf.write(&mCudaObjsOffset, sizeof(PxU32));

		writeLoc = mCudaObjsOffset + sizeof(PxU32);		// Space for N of cuda objs
		mMemBuf.seekWrite(writeLoc);
	}

	ApexCudaTestKernelContext::~ApexCudaTestKernelContext()
	{
	}

	PX_INLINE PxU32 ApexCudaTestKernelContext::advanceMemBuf(PxU32 size, PxU32 align)
	{
		PxU32 writeLoc = mMemBuf.tellWrite();
		ALIGN_OFFSET(writeLoc, align);
		const PxU32 ret = writeLoc;
		writeLoc += size;
		mMemBuf.seekWrite(writeLoc);
		return ret;
	}
	PX_INLINE void ApexCudaTestKernelContext::copyToMemBuf(const apexCudaTest::MemRef& memRef)
	{
		CUT_SAFE_CALL(cuMemcpyDtoHAsync(
			(void*)(mMemBuf.getWriteBuffer() + memRef.bufferOffset), CUdeviceptr((const PxU8*)memRef.gpuPtr + memRef.dataOffset), memRef.size, CUstream(mCuStream))
		);
	}
	PX_INLINE void ApexCudaTestKernelContext::copyToMemBuf(const ArrayRef& arrayRef)
	{
		ApexCudaArray cudaArray;
		cudaArray.assign(arrayRef.cuArray, false);
		cudaArray.copyToHost((CUstream)mCuStream, (void*)(mMemBuf.getWriteBuffer() + arrayRef.bufferOffset));
	}

	void ApexCudaTestKernelContext::completeCudaObjsBlock()
	{
		PxU32 writeLoc = mMemBuf.tellWrite();
		mMemBuf.seekWrite(4 * sizeof(PxU32));			// Offset for call param block
		mMemBuf.write(&writeLoc, sizeof(PxU32));
		mCallParamsOffset = writeLoc;

		mMemBuf.seekWrite(mCudaObjsOffset);				// Write N of cuda objs
		mMemBuf.write(&mCudaObjsCounter, sizeof(PxU32));

		writeLoc += sizeof(PxU32);						// Space for N of call params
		mMemBuf.seekWrite(writeLoc);
	}

	void ApexCudaTestKernelContext::completeCallParamsBlock()
	{
		PxU32 writeLoc = mMemBuf.tellWrite();
		mMemBuf.seekWrite(mCallParamsOffset);			// Write N of call params
		mMemBuf.write(&mCallParamsCounter, sizeof(PxU32));
		mMemBuf.seekWrite(writeLoc);
	}

	void ApexCudaTestKernelContext::setFreeKernel(PxU32 threadCount)
	{
		PxU32 writeLoc = mMemBuf.tellWrite();
		mMemBuf.seekWrite(mCudaObjsOffset - 5 * sizeof(PxU32));
		PxU32 tmp = apexCudaTest::KT_FREE;
		mMemBuf.write(&tmp, sizeof(tmp));		
		mMemBuf.write(&threadCount, sizeof(threadCount));
		tmp = 0;
		mMemBuf.write(&tmp, sizeof(tmp));		
		mMemBuf.write(&tmp, sizeof(tmp));		
		mMemBuf.write(&tmp, sizeof(tmp));		
		mMemBuf.seekWrite(writeLoc);
	}
	void ApexCudaTestKernelContext::setFreeKernel(PxU32 threadCountX, PxU32 threadCountY)
	{
		PxU32 writeLoc = mMemBuf.tellWrite();
		mMemBuf.seekWrite(mCudaObjsOffset - 5 * sizeof(PxU32));
		PxU32 tmp = apexCudaTest::KT_FREE2D;
		mMemBuf.write(&tmp, sizeof(tmp));		
		mMemBuf.write(&threadCountX, sizeof(threadCountX));
		mMemBuf.write(&threadCountY, sizeof(threadCountY));
		tmp = 0;
		mMemBuf.write(&tmp, sizeof(tmp));		
		mMemBuf.write(&tmp, sizeof(tmp));		
		mMemBuf.seekWrite(writeLoc);
	}
	void ApexCudaTestKernelContext::setFreeKernel(PxU32 threadCountX, PxU32 threadCountY, PxU32 threadCountZ, PxU32 blockCountY)
	{
		PxU32 writeLoc = mMemBuf.tellWrite();
		mMemBuf.seekWrite(mCudaObjsOffset - 5 * sizeof(PxU32));
		PxU32 tmp = apexCudaTest::KT_FREE3D;
		mMemBuf.write(&tmp, sizeof(tmp));		
		mMemBuf.write(&threadCountX, sizeof(threadCountX));
		mMemBuf.write(&threadCountY, sizeof(threadCountY));
		mMemBuf.write(&threadCountZ, sizeof(threadCountZ));
		mMemBuf.write(&blockCountY, sizeof(blockCountY));
		mMemBuf.seekWrite(writeLoc);
	}
	void ApexCudaTestKernelContext::setBoundKernel(PxU32 threadCount)
	{
		PxU32 writeLoc = mMemBuf.tellWrite();
		mMemBuf.seekWrite(mCudaObjsOffset - 5 * sizeof(PxU32));
		PxU32 tmp = apexCudaTest::KT_BOUND;
		mMemBuf.write(&tmp, sizeof(tmp));		
		mMemBuf.write(&threadCount, sizeof(threadCount));
		tmp = 0;
		mMemBuf.write(&tmp, sizeof(tmp));		
		mMemBuf.write(&tmp, sizeof(tmp));		
		mMemBuf.write(&tmp, sizeof(tmp));		
		mMemBuf.seekWrite(writeLoc);
	}
	void ApexCudaTestKernelContext::setSyncKernel()
	{
		PxU32 writeLoc = mMemBuf.tellWrite();
		mMemBuf.seekWrite(mCudaObjsOffset - 5 * sizeof(PxU32));
		PxU32 tmp = apexCudaTest::KT_SYNC;
		mMemBuf.write(&tmp, sizeof(tmp));
		mMemBuf.seekWrite(writeLoc);
	}

	void ApexCudaTestKernelContext::setSharedSize(PxU32 size)
	{
		PxU32 writeLoc = mMemBuf.tellWrite();
		mMemBuf.seekWrite(mCudaObjsOffset - 11 * sizeof(PxU32));
		mMemBuf.write(&size, sizeof(int));
		mMemBuf.seekWrite(writeLoc);
	}
	void ApexCudaTestKernelContext::setFuncInstId(int id)
	{
		PxU32 writeLoc = mMemBuf.tellWrite();
		mMemBuf.seekWrite(mCudaObjsOffset - 12 * sizeof(PxU32));
		mMemBuf.write(&id, sizeof(int));
		mMemBuf.seekWrite(writeLoc);
	}

	void ApexCudaTestKernelContext::setBlockDim(PxU32 x, PxU32 y, PxU32 z)
	{
		PxU32 writeLoc = mMemBuf.tellWrite();
		mMemBuf.seekWrite(mCudaObjsOffset - 10 * sizeof(PxU32));
		mMemBuf.write(&x, sizeof(int));
		mMemBuf.write(&y, sizeof(int));
		mMemBuf.write(&z, sizeof(int));
		mMemBuf.seekWrite(writeLoc);
	}

	void ApexCudaTestKernelContext::setGridDim(PxU32 x, PxU32 y)
	{
		PxU32 writeLoc = mMemBuf.tellWrite();
		mMemBuf.seekWrite(mCudaObjsOffset - 7 * sizeof(PxU32));
		mMemBuf.write(&x, sizeof(int));
		mMemBuf.write(&y, sizeof(int));
		mMemBuf.seekWrite(writeLoc);
	}

	void ApexCudaTestKernelContext::addParam(const char* name, PxU32 align, const void *val, size_t size, int memRefIntent, int dataOffset, PxU32 fpType)
	{
		if (val == 0)
		{
			//handle NULL-ptr case
			size = 0;
			dataOffset = 0;
		}
		PxU32 sz = (PxU32)size;
		mCallParamsCounter++;
		ApexSimpleString tName(name);
		WRITE_STRING(tName);
		WRITE_SCALAR(align);
		PxU32 intent = (physx::PxU32)memRefIntent;
		intent += fpType << 2;
		WRITE_SCALAR(intent);
		WRITE_SCALAR(dataOffset);
		if (memRefIntent == 0)
		{
			WRITE_ALIGN_ARRAY(val, sz, align);
		}
		else
		{
			WRITE_SCALAR(sz);
			if (sz > 0)
			{
				if (memRefIntent & ApexCudaMemFlags::IN)
				{
					const PxU32 offset = advanceMemBuf(sz, align);
					apexCudaTest::MemRef memRef(val, size, dataOffset, offset);
					copyToMemBuf(memRef);
				}
				if (memRefIntent & ApexCudaMemFlags::OUT)
				{
					const PxU32 offset = advanceMemBuf(sz, align);
					apexCudaTest::MemRef memRef(val, size, dataOffset, offset);
					mMemRefs.pushBack(memRef);
				}
			}
		}
	}

	void ApexCudaTestKernelContext::startObjList()
	{
	}
	void ApexCudaTestKernelContext::finishObjList()
	{
		completeCudaObjsBlock();
	}

	PxU32 ApexCudaTestKernelContext::addCuArray(CUarray cuArray)
	{
		ApexCudaArray cudaArray;
		cudaArray.assign(cuArray, false);

		const CUDA_ARRAY3D_DESCRIPTOR& desc = cudaArray.getDesc();
		PxU32 format = PxU32(desc.Format);
		PxU32 numChannels = PxU32(desc.NumChannels);
		PxU32 width = PxU32(desc.Width);
		PxU32 height = PxU32(desc.Height);
		PxU32 depth = PxU32(desc.Depth);
		PxU32 flags = PxU32(desc.Flags);

		WRITE_SCALAR(format);
		WRITE_SCALAR(numChannels);
		WRITE_SCALAR(width);
		WRITE_SCALAR(height);
		WRITE_SCALAR(depth);
		WRITE_SCALAR(flags);

		return PxU32(cudaArray.getByteSize());
	}

	void ApexCudaTestKernelContext::addTexRef(const char* name, const void* mem, size_t size, CUarray arr)
	{
		PX_ASSERT(!mIsCompleteContext);
		const PxU32 objType = (arr != NULL) ? apexCudaTest::OBJ_TYPE_TEX_REF_ARR : apexCudaTest::OBJ_TYPE_TEX_REF_MEM;
		mCudaObjsCounter++;
		WRITE_SCALAR(objType);
		ApexSimpleString tName(name);
		WRITE_STRING(tName);
		if (arr != NULL)
		{
			const PxU32 sz = addCuArray(arr);
			const PxU32 offset = advanceMemBuf(sz);
			ArrayRef arrayRef(arr, offset);
			copyToMemBuf(arrayRef);
		}
		else
		{
			const PxU32 sz = PxU32(size);
			WRITE_SCALAR(sz);
			if (sz > 0)
			{
				const PxU32 offset = advanceMemBuf(sz);
				apexCudaTest::MemRef memRef(mem, size, 0, offset);
				copyToMemBuf(memRef);
			}
		}
	}

	void ApexCudaTestKernelContext::addSurfRef(const char* name, CUarray arr, ApexCudaMemFlags::Enum flags)
	{
		PX_ASSERT(!mIsCompleteContext);
		const PxU32 objType = apexCudaTest::OBJ_TYPE_SURF_REF;
		mCudaObjsCounter++;
		WRITE_SCALAR(objType);
		ApexSimpleString tName(name);
		WRITE_STRING(tName);
		const PxU32 intent = flags;
		WRITE_SCALAR(intent);

		const PxU32 sz = addCuArray(arr);
		if (intent & ApexCudaMemFlags::IN)
		{
			const PxU32 offset = advanceMemBuf(sz);
			ArrayRef arrayRef(arr, offset);
			copyToMemBuf(arrayRef);
		}
		if (intent & ApexCudaMemFlags::OUT)
		{
			const PxU32 offset = advanceMemBuf(sz);
			ArrayRef arrayRef(arr, offset);
			mArrayRefs.pushBack(arrayRef);
		}
	}

	void ApexCudaTestKernelContext::addConstMem(const char* name, const void* mem, size_t size)
	{
		PX_ASSERT(!mIsCompleteContext);
		const PxU32 objType = apexCudaTest::OBJ_TYPE_CONST_MEM;
		mCudaObjsCounter++;
		WRITE_SCALAR(objType);
		ApexSimpleString cmName(name);
		WRITE_STRING(cmName);
		WRITE_ARRAY(mem, (PxU32)size);
	}

	void ApexCudaTestKernelContext::copyMemRefs()
	{
		for (PxU32 i = 0; i < mMemRefs.size(); i++)
		{
			copyToMemBuf(mMemRefs[i]);
		}
		mMemRefs.clear();
	}

	void ApexCudaTestKernelContext::copyArrayRefs()
	{
		for (PxU32 i = 0; i < mArrayRefs.size(); i++)
		{
			copyToMemBuf(mArrayRefs[i]);
		}
		mArrayRefs.clear();
	}

	void ApexCudaTestKernelContext::setKernelStatus()
	{
		if (mIsWriteForNonSuccessfulKernel)
		{
			int cuResult = cuCtxSynchronize();//= cudaPeekAtLastError();
			//cudaDeviceSynchronize();
		
			if (cuResult)
			{
				mErrorCode += 'E';
				mErrorCode += ApexSimpleString((physx::PxU32)cuResult, 3);
				saveToFile();
				APEX_INTERNAL_ERROR("Cuda Error %d", cuResult);
			}
			else if (mIsContextForSave)
			{
				copyMemRefs();
				copyArrayRefs();
			}
		}
		else
		{
			copyMemRefs();
			copyArrayRefs();
		}
	}

	bool ApexCudaTestKernelContext::saveToFile()
	{
		if (!mIsContextForSave && mErrorCode.size() == 0)
		{
			return false;
		}
		if (!mIsCompleteContext)
		{
			completeCallParamsBlock();

			PxU32 writeLoc = mMemBuf.tellWrite();
			mMemBuf.seekWrite(sizeof(PxU32));				// Write size of file
			mMemBuf.write(&writeLoc, sizeof(PxU32));
			mIsCompleteContext = true;

			mMemBuf.seekWrite(writeLoc);
		}
		
		ApexSimpleString path(mPath);
		path += mName;
		path += '_';
		path += ApexSimpleString(mCallPerFrame, 3);
		path += ApexSimpleString(mFrame, 5);
		path += mErrorCode;
		FILE* saveFile;
		physx::shdfnd::fopen_s(&saveFile,path.c_str(),"wb");
		if (saveFile)
		{
			fwrite(mMemBuf.getWriteBuffer(), mMemBuf.getWriteBufferSize(), 1, saveFile);
			return !fclose(saveFile);
		}
		
		return false;
	}

	
	ApexCudaTestManager::ApexCudaTestManager()
		:	mCurrentFrame(0)
		,	mMaxSamples(0)
		,	mFramePeriod(0)		
		,	mCallPerFrameMaxCount(1)
		,	mIsWriteForNonSuccessfulKernel(false)
	{
	}

	ApexCudaTestManager::~ApexCudaTestManager()
	{
		for (PxU32 i = 0; i < mContexts.size(); i++)
		{
			PX_DELETE(mContexts[i]);
		}
	}

	void ApexCudaTestManager::setWriteForFunction(const char* functionName, const char* moduleName)
	{
		if (strcmp(functionName, "*") == 0)
		{
			//Add all function registered in module
			NiModuleScene* moduleScene = mApexScene->getNiModuleScene(moduleName);
			ApexCudaObj* obj = NULL;
			if (moduleScene)
			{
				obj = static_cast<ApexCudaObj*>(moduleScene->getHeadCudaObj());
			}
			while(obj)
			{
				if (obj->getType() == ApexCudaObj::FUNCTION)				
				{
					const char* name = DYNAMIC_CAST(ApexCudaFunc*)(obj)->getName();
					if (mKernels.find(KernelInfo(name, moduleName)) == mKernels.end())
					{
						mKernels.pushBack(KernelInfo(name, moduleName));
					}
				}
				obj = obj->next();
			}
		}
		else
		{
			ApexSimpleString fName(moduleName);
			fName += '_';
			fName += ApexSimpleString(functionName);
			mKernels.pushBack(KernelInfo(fName.c_str(), moduleName));
		}
	}

	bool ApexCudaTestManager::runKernel(const char* path)
	{
		if (mApexScene)
		{			
			ApexCudaTestKernelContextReader contextReader(path, mApexScene);
			return contextReader.runKernel();
		}
		return false;
	}

	void ApexCudaTestManager::nextFrame()
	{
		mCurrentFrame++;

		if (mContexts.size() > 0)
		{
			for (PxU32 i = 0; i < mContexts.size(); i++)
			{
				mContexts[i]->saveToFile();
				PX_DELETE(mContexts[i]);
			}
			mContexts.clear();

			for (PxU32 i = 0; i < mKernels.size(); i++)
			{
				mKernels[i].callCount = 0;
			}
		}
	}

	ApexCudaTestKernelContext* ApexCudaTestManager::isTestKernel(const char* functionName, const char* moduleName)
	{
		KernelInfo* kernel = NULL;
		if  (	mContexts.size() < mMaxSamples 
			&&	(	mSampledFrames.find(mCurrentFrame) != mSampledFrames.end()
				||	mFramePeriod && (mCurrentFrame % mFramePeriod) == 0
				)
			&&	(kernel = mKernels.find(KernelInfo(functionName, moduleName))) != mKernels.end()
			&&  (kernel->callCount < mCallPerFrameMaxCount)
			)
		{
			mContexts.pushBack(PX_NEW(ApexCudaTestKernelContext)(mPath.c_str(), functionName, moduleName, mCurrentFrame, ++(kernel->callCount), mIsWriteForNonSuccessfulKernel, true));
			return mContexts.back();
		}
		else if (mIsWriteForNonSuccessfulKernel && (mKernels.size() == 0 || (mKernels.find(KernelInfo(functionName, moduleName)) != mKernels.end())))
		{
			mContexts.pushBack(PX_NEW(ApexCudaTestKernelContext)(mPath.c_str(), functionName, moduleName, mCurrentFrame, 0, true, false));
			return mContexts.back();
		}
		return NULL;
	}
}
} // namespace physx::apex

#endif
