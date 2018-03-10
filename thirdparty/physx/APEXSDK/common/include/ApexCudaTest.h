/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEX_CUDA_TEST__
#define __APEX_CUDA_TEST__

#include "NxApexDefs.h"
#include "NxApexCudaTestManager.h"

#include "PxMemoryBuffer.h"
#include "ApexString.h"
#include "ApexMirroredArray.h"
#include "NiApexScene.h"

#include "ApexCudaDefs.h"

namespace physx
{
namespace apex
{

struct ApexCudaMemRefBase;
class ApexCudaObj;
class ApexCudaFunc;
struct ApexCudaFuncParams;
class ApexCudaTexRef;
class ApexCudaSurfRef;

const PxU32 ApexCudaTestFileVersion = 103;

namespace apexCudaTest
{

struct MemRef
{
	ApexSimpleString	name;
	const void*	gpuPtr;
	size_t		size;
	PxI32		dataOffset;
	PxU32		bufferOffset;	
	PxU32		fpType; // Floating point type, if 0 - not a float, else if 4 - float, else if 8 - double

	MemRef(const void* gpuPtr, size_t size, PxI32 dataOffset, PxU32 bufferOffset, PxU32 fpType = 0) 
		: gpuPtr(gpuPtr),  size(size), dataOffset(dataOffset), bufferOffset(bufferOffset), fpType(fpType) {}
};

enum ObjTypeEnum
{
	OBJ_TYPE_NONE = 0,
	OBJ_TYPE_TEX_REF_MEM = 1,
	OBJ_TYPE_CONST_MEM = 2,
	OBJ_TYPE_SURF_REF = 4,
	OBJ_TYPE_TEX_REF_ARR = 5
};

enum KernelTypeEnum
{
	KT_SYNC,
	KT_FREE,
	KT_FREE2D,
	KT_FREE3D,
	KT_BOUND
};

}

/** Read cuda kernel context from specified file. Run kernel ant compare output with results from file
*/
class ApexCudaTestKernelContextReader : public UserAllocated
{
	struct Dim3
	{
		int x,y,z;
	};

	struct TexRef
	{
		ApexCudaTexRef* cudaTexRef;
		PxU32 memRefIdx;
		ApexCudaArray* cudaArray;
	};

	struct SurfRef
	{
		ApexCudaSurfRef* cudaSurfRef;
		ApexCudaArray* cudaArray;
		ApexCudaMemFlags::Enum flags;
	};

	struct ArrayRef
	{
		ApexSimpleString	name;
		ApexCudaArray*		cudaArray;
		const PxU8*			bufferPtr;
		PxU32				size;

		ArrayRef(const char* name, ApexCudaArray* cudaArray, const PxU8* bufferPtr, PxU32 size)
		{
			this->name = name;
			this->cudaArray = cudaArray;
			this->bufferPtr = bufferPtr;
			this->size = size;
		}
	};

	struct ParamRef
	{
		ApexSimpleString name;
		PxU32 value;
	};

public:
	ApexCudaTestKernelContextReader(const char* path, NiApexScene* scene);
	~ApexCudaTestKernelContextReader();

	bool	runKernel();

private:
	ApexCudaArray* loadCudaArray();

	void	loadContext(ApexCudaFuncParams& params);
	void	loadTexRef(PxU32& memOffset, bool bBindToArray);
	void	loadSurfRef();
	void	loadConstMem();
	PxU32	getParamSize();
	void	loadParam(PxU32& memOffset, ApexCudaFuncParams& params);

	bool	compare(const PxU8* resData, const PxU8* refData, size_t size, PxU32 fpType, const char* name);
	void	dumpParams(char* str);

	PxMemoryBuffer*		mMemBuf;

	PxU32				mCudaObjOffset;
	PxU32				mParamOffset;

	int					mCuOffset;
	void*				mCuStream;

	ApexSimpleString	mName;
	ApexSimpleString	mModuleName;
	PxU32				mFrame;
	PxU32				mCallPerFrame;

	PxU32				mFuncInstId;
	PxU32				mSharedSize;
	Dim3				mBlockDim;
	Dim3				mGridDim;
	apexCudaTest::KernelTypeEnum mKernelType;
	PxU32				mThreadCount[3];
	PxU32				mBlockCountY;

	ApexCudaObj*		mHeadCudaObj;
	ApexCudaFunc*		mFunc;

	NiApexScene*		mApexScene;
	Array <PxU8*>		mArgSeq;
	ApexMirroredArray <PxU8>		mTmpArray;
	physx::PxGpuCopyDescQueue	mCopyQueue;

	Array <apexCudaTest::MemRef>	mInMemRefs;
	Array <apexCudaTest::MemRef>	mOutMemRefs;
	Array <ArrayRef>	mInArrayRefs;
	Array <ArrayRef>	mOutArrayRefs;

	Array <TexRef>		mTexRefs;
	Array <SurfRef>		mSurfRefs;

	PxU32				mCudaArrayCount;
	ApexCudaArray*		mCudaArrayList;

	Array <ParamRef>	mParamRefs;
};

/** Extract context data from CudaModuleScene about cuda kernel and save it to specified file
*/
class ApexCudaTestKernelContext : public UserAllocated
{
	struct ArrayRef
	{
		CUarray	cuArray;
		PxU32	bufferOffset;

		ArrayRef(CUarray cuArray, PxU32 bufferOffset)
		{
			this->cuArray = cuArray;
			this->bufferOffset = bufferOffset;
		}
	};

public:
	ApexCudaTestKernelContext(const char* path, const char* functionName, const char* moduleName, PxU32 frame, PxU32 callPerFrame, bool isWriteForNonSuccessfulKernel, bool isKernelForSave);
	~ApexCudaTestKernelContext();

	bool saveToFile();

	PX_INLINE void setCuStream(void* cuStream)
	{
		mCuStream = cuStream;
	}

	void				startObjList();
	void				finishObjList();

	void				setFreeKernel(PxU32 threadCount);
	void				setFreeKernel(PxU32 threadCountX, PxU32 threadCountY);
	void				setFreeKernel(PxU32 threadCountX, PxU32 threadCountY, PxU32 threadCountZ, PxU32 blockCountY);
	void				setBoundKernel(PxU32 threadCount);
	void				setSyncKernel();

	void				setBlockDim(PxU32 x, PxU32 y, PxU32 z);
	void				setGridDim(PxU32 x, PxU32 y);

	void				setSharedSize(PxU32 size);
	void				setFuncInstId(int id);

	void				addParam(const char* name, PxU32 align, const void *val, size_t size, int isMemRef = 0, int dataOffset = 0, PxU32 fpType = 0);
	void				addTexRef(const char* name, const void* mem, size_t size, CUarray arr);
	void				addSurfRef(const char* name, CUarray arr, ApexCudaMemFlags::Enum flags);
	void				addConstMem(const char* name, const void* mem, size_t size);
	void				setKernelStatus();

private:
	void				copyMemRefs();
	void				copyArrayRefs();

	PxU32				addCuArray(CUarray cuArray);

	void				completeCudaObjsBlock();
	void				completeCallParamsBlock();

	PX_INLINE PxU32		advanceMemBuf(PxU32 size, PxU32 align = 4);
	PX_INLINE void		copyToMemBuf(const apexCudaTest::MemRef& memRef);
	PX_INLINE void		copyToMemBuf(const ArrayRef& arrayRef);

	void*				mCuStream;

	PxU32				mVersion;
	PxU32				mFrame;
	PxU32				mCallPerFrame;
	ApexSimpleString	mName;
	ApexSimpleString	mErrorCode;
	ApexSimpleString	mModuleName;
	ApexSimpleString	mPath;
	PxMemoryBuffer		mMemBuf;

	PxU32				mCudaObjsOffset;
	PxU32				mCallParamsOffset;
	
	PxU32				mCudaObjsCounter;
	PxU32				mCallParamsCounter;

	Array <ArrayRef>	mArrayRefs;
	Array <apexCudaTest::MemRef> mMemRefs;

	bool				mIsCompleteContext;
	bool				mIsWriteForNonSuccessfulKernel;
	bool				mIsContextForSave;
};


/** Class get information what kernels should be tested and give directive for creation ApexCudaTestContext
 */
class ApexCudaTestManager : public NxApexCudaTestManager
{
	struct KernelInfo
	{
		ApexSimpleString functionName;
		ApexSimpleString moduleName;
		PxU32 callCount;
		
		KernelInfo(const char* functionName, const char* moduleName) 
			: functionName(functionName), moduleName(moduleName), callCount(0) {}

		bool operator!= (const KernelInfo& ki)
		{
			return this->functionName != ki.functionName || this->moduleName != ki.moduleName;
		}
	};

public:
	
	ApexCudaTestManager();
	virtual ~ApexCudaTestManager();

	PX_INLINE void setNiApexScene(NiApexScene* scene)
	{
		mApexScene = scene;
	}
	void nextFrame();
	ApexCudaTestKernelContext* isTestKernel(const char* functionName, const char* moduleName);

	// interface for NxApexCudaTestManager
	PX_INLINE void setWritePath(const char* path)
	{
		mPath = ApexSimpleString(path);
	}
	void setWriteForFunction(const char* functionName, const char* moduleName);

	PX_INLINE void setMaxSamples(PxU32 maxSamples)
	{
		mMaxSamples = maxSamples;
	}
	void setFrames(PxU32 numFrames, const PxU32* frames)
	{
		for(PxU32 i = 0; i < numFrames && mSampledFrames.size() < mMaxSamples; i++)
		{
			if (frames == NULL) // write next numFrames frames after current
			{
				mSampledFrames.pushBack(mCurrentFrame + i + 1);
			}
			else
			{
				mSampledFrames.pushBack(frames[i]);
			}
		}
	}
	void setFramePeriod(PxU32 period)
	{
		mFramePeriod = period;
	}
	void setCallPerFrameMaxCount(PxU32 cpfMaxCount)
	{
		mCallPerFrameMaxCount = cpfMaxCount;
	}
	void setWriteForNotSuccessfulKernel(bool flag)
	{
		mIsWriteForNonSuccessfulKernel = flag;
	}
/*	void setCallPerFrameSeries(PxU32 callsCount, const PxU32* calls)
	{
		for(PxU32 i = 0; i < callsCount && mSampledCallsPerFrame.size() < mCallPerFrameMaxCount; i++)
		{
			mSampledCallsPerFrame.pushBack(calls[i]);
		}
	}*/
	bool runKernel(const char* path);
	
private:
	NiApexScene*	mApexScene;
	PxU32	mCurrentFrame;
	PxU32	mMaxSamples;
	PxU32	mFramePeriod;
	PxU32	mCallPerFrameMaxCount;
	bool	mIsWriteForNonSuccessfulKernel;
	ApexSimpleString					mPath;
	Array <PxU32>						mSampledFrames;
	//Array <PxU32>						mSampledCallsPerFrame;
	Array <KernelInfo>					mKernels;
	Array <ApexCudaTestKernelContext*>	mContexts;
};

}
} // namespace physx::apex

#endif // __APEX_CUDA_TEST__
