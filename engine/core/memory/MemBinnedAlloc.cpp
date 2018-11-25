#include "MemBinnedAlloc.h"
#include "MemAllocDef.h"

#if (ECHO_MEMORY_ALLOCATOR  == ECHO_MEMORY_ALLOCATOR_BINNED)

#include <stddef.h>
#include <iostream>
#include <assert.h>
#include <limits>

#ifdef ECHO_PLATFORM_WINDOWS
#include <Windows.h>
#include <Psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

#ifdef ECHO_PLATFORM_IOS
#include <mach/mach.h>
#include <mach/mach_host.h>
#endif

#ifdef ECHO_PLATFORM_ANDROID
#include "unistd.h"
#include <jni.h>
#endif

#include "Foundation/Thread/Threading.h"
#include "Memory/MemBinnedAlloc.h"
#include "Memory/MemManager.h"

namespace Echo
{
    #define STATS 0

#define USE_INTERNAL_LOCKS
#define CACHE_FREED_OS_ALLOCS

#ifdef USE_INTERNAL_LOCKS
#	define USE_COARSE_GRAIN_LOCKS
#endif



#if defined CACHE_FREED_OS_ALLOCS
#	define MAX_CACHED_OS_FREES (32)
#	define MAX_CACHED_OS_FREES_BYTE_LIMIT (4*1024*1024)
#endif

#if defined USE_INTERNAL_LOCKS && !defined USE_COARSE_GRAIN_LOCKS
#	define USE_FINE_GRAIN_LOCKS
#endif

    //template< class T > inline T Align( const T Ptr, int Alignment )
    //{
    //	return (T)(((size_t)Ptr + Alignment - 1) & ~(Alignment-1));
    //}

	static MallocInterface* g_binned_malloc = 0;

    inline unsigned int FloorLog2(unsigned int Value)
    {

        unsigned int pos = 0;
        if (Value >= 1<<16) { Value >>= 16; pos += 16; }
        if (Value >= 1<< 8) { Value >>=  8; pos +=  8; }
        if (Value >= 1<< 4) { Value >>=  4; pos +=  4; }
        if (Value >= 1<< 2) { Value >>=  2; pos +=  2; }
        if (Value >= 1<< 1) {				pos +=  1; }
        return (Value == 0) ? 0 : pos;
    }

    inline unsigned int CountLeadingZeros(unsigned int Value)
    {
        if (Value == 0) return 32;
        return 31 - FloorLog2(Value);
    }


    inline unsigned int CeilLogTwo( unsigned int Arg )
    {
        int Bitmask = ((int)(CountLeadingZeros(Arg) << 26)) >> 31;
        return (32 - CountLeadingZeros(Arg - 1)) & (~Bitmask);
    }
  
	/**
	 * 分配器接口
	 */
    class MallocInterface
    {
    public:
        MallocInterface(){}
        virtual ~MallocInterface(){}
        
        virtual void* Malloc( size_t Size, unsigned int Alignment ) = 0;
        virtual void* Realloc( void* Ptr, size_t NewSize, unsigned int Alignment) = 0;
        virtual void Free( void* Ptr ) = 0;
        virtual void CheckLeak() = 0;
        
        // 替换全局operator new & delete
        void* operator new( size_t size ) { return ::malloc(size); }
        void* operator new[]( size_t size ) { return ::malloc(size); }
        void operator delete( void* ptr ) { ::free(ptr); }
        void operator delete[]( void* ptr ) { ::free(ptr); }
        
    };
    
	/**
	 * 分配器实现
	 */
    class MallocBinned : public MallocInterface
    {
    private:    
        // Counts.
        enum { POOL_COUNT = 42 };

        /** Maximum allocation for the pooled allocator */
        enum { EXTENED_PAGE_POOL_ALLOCATION_COUNT = 2 };
        enum { MAX_POOLED_ALLOCATION_SIZE   = 32768+1 };
        enum { PAGE_SIZE_LIMIT = 65536 };
        // BINNED_ALLOC_POOL_SIZE can be increased beyond 64k to cause binned malloc to allocate
        // the small size bins in bigger chunks. If OS Allocation is slow, increasing
        // this number *may* help performance but YMMV.
        enum { BINNED_ALLOC_POOL_SIZE = 65536 };

        // Forward declares.
        struct SFreeMem;
        struct SPoolTable;

        // Memory pool info. 32 bytes.
        struct SPoolInfo
        {
            union //32Byte 
            {
                struct
                {
                    /** Number of allocated elements in this pool, when counts down to zero can free the entire pool. */
                    unsigned short			Taken;		// 2
                    /** Index of pool. Index into MemSizeToPoolTable[]. Valid when < MAX_POOLED_ALLOCATION_SIZE, MAX_POOLED_ALLOCATION_SIZE is OsTable.
                    When AllocSize is 0, this is the number of pages to step back to find the base address of an allocation. See FindPoolInfoInternal()
                    */
                    unsigned short			TableIndex; // 4
                    /** Number of bytes allocated */
                    unsigned int			AllocSize;	// 8
                    /** Pointer to first free memory in this pool or the OS Allocation Size in bytes if this allocation is not binned*/
                    SFreeMem*		FirstMem;   // 12/16
                    SPoolInfo*		Next;		// 16/24
                    SPoolInfo**		PrevLink;	// 20/32
                };

                /** Explicit padding for 32 bit builds */
                unsigned char Padding[32]; // 32
            };

            void SetAllocationSizes( unsigned int InBytes, size_t InOsBytes, unsigned int InTableIndex, unsigned int SmallAllocLimt )
            {
                TableIndex=InTableIndex;
                AllocSize=InBytes;
                if (TableIndex == SmallAllocLimt)
                {
                    FirstMem=(SFreeMem*)InOsBytes;
                }
            }

            unsigned int GetBytes() const
            {
                return AllocSize;
            }

            size_t GetOsBytes( unsigned int InPageSize, unsigned int SmallAllocLimt ) const
            {
                if (TableIndex == SmallAllocLimt)
                {
                    return (size_t)FirstMem;
                }
                else
                {
                    return Align(AllocSize, InPageSize);
                }
            }

            void Link( SPoolInfo*& Before )
            {
                if( Before )
                {
                    Before->PrevLink = &Next;
                }
                Next     = Before;
                PrevLink = &Before;
                Before   = this;
            }

            void Unlink()
            {
                if( Next )
                {
                    Next->PrevLink = PrevLink;
                }
                *PrevLink = Next;
            }
        };

        /** Information about a piece of free memory. 8 bytes */
        struct SFreeMem
        {
            /** Next or MemLastPool[], always in order by pool. */
            SFreeMem*	Next;
            /** Number of consecutive free blocks here, at least 1. */
            unsigned int		NumFreeBlocks;
        };

        /** Default alignment for binned allocator */
        enum { DEFAULT_BINNED_ALLOCATOR_ALIGNMENT = sizeof(SFreeMem) };

#ifdef CACHE_FREED_OS_ALLOCS
        /**  */
        struct SFreePageBlock
        {
            void*				Ptr;
            size_t				ByteSize;

            SFreePageBlock()
            {
                Ptr = NULL;
                ByteSize = 0;
            }
        };
#endif

        /** Pool table. */
        struct SPoolTable
        {
            SPoolInfo*			FirstPool;
            SPoolInfo*			ExhaustedPool;
            unsigned int		BlockSize;
#ifdef USE_FINE_GRAIN_LOCKS
            Mutex               CriticalSection;
#endif
#if STATS
            /** Number of currently active pools */
            unsigned int				NumActivePools;

            /** Largest number of pools simultaneously active */
            unsigned int				MaxActivePools;

            /** Number of requests currently active */
            unsigned int				ActiveRequests;

            /** High watermark of requests simultaneously active */
            unsigned int				MaxActiveRequests;

            /** Minimum request size (in bytes) */
            unsigned int				MinRequest;

            /** Maximum request size (in bytes) */
            unsigned int				MaxRequest;

            /** Total number of requests ever */
            unsigned long long			TotalRequests;

            /** Total waste from all allocs in this table */
            unsigned long long			TotalWaste;
#endif
            SPoolTable()
                : FirstPool(NULL)
                , ExhaustedPool(NULL)
                , BlockSize(0)
#if STATS
                , NumActivePools(0)
                , MaxActivePools(0)
                , ActiveRequests(0)
                , MaxActiveRequests(0)
                , MinRequest(0)
                , MaxRequest(0)
                , TotalRequests(0)
                , TotalWaste(0)
#endif
            {

            }
        };

        /** Hash table struct for retrieving allocation book keeping information */
        struct PoolHashBucket
        {
            size_t			Key;
            SPoolInfo*		FirstPool;
            PoolHashBucket* Prev;
            PoolHashBucket* Next;

            PoolHashBucket()
            {
                Key=0;
                FirstPool=NULL;
                Prev=this;
                Next=this;
            }

            void Link( PoolHashBucket* After )
            {
                Link(After, Prev, this);
            }

            static void Link( PoolHashBucket* Node, PoolHashBucket* Before, PoolHashBucket* After )
            {
                Node->Prev=Before;
                Node->Next=After;
                Before->Next=Node;
                After->Prev=Node;
            }

            void Unlink()
            {
                Next->Prev = Prev;
                Prev->Next = Next;
                Prev=this;
                Next=this;
            }
        };

        unsigned long long TableAddressLimit;



        Mutex	AccessGuard;

        // PageSize dependent constants
        unsigned long long MaxHashBuckets;
        unsigned long long MaxHashBucketBits;
        unsigned long long MaxHashBucketWaste;
        unsigned long long MaxBookKeepingOverhead;
        /** Shift to get the reference from the indirect tables */
        unsigned long long PoolBitShift;
        unsigned long long IndirectPoolBitShift;
        unsigned long long IndirectPoolBlockSize;
        /** Shift required to get required hash table key. */
        unsigned long long HashKeyShift;
        /** Used to mask off the bits that have been used to lookup the indirect table */
        unsigned long long PoolMask;
        unsigned long long BinnedSizeLimit;
        unsigned long long BinnedOSTableIndex;

        // Variables.
        SPoolTable  PoolTable[POOL_COUNT];
        SPoolTable	OsTable;
        SPoolTable	PagePoolTable[EXTENED_PAGE_POOL_ALLOCATION_COUNT];
        SPoolTable* MemSizeToPoolTable[MAX_POOLED_ALLOCATION_SIZE+EXTENED_PAGE_POOL_ALLOCATION_COUNT];

        PoolHashBucket* HashBuckets;
        PoolHashBucket* HashBucketFreeList;

        unsigned int	PageSize;

#ifdef CACHE_FREED_OS_ALLOCS
        SFreePageBlock	FreedPageBlocks[MAX_CACHED_OS_FREES];
        unsigned int	FreedPageBlocksNum;
        unsigned int	CachedTotal;
#endif

#if STATS
        size_t		OsCurrent;
        size_t		OsPeak;
        size_t		WasteCurrent;
        size_t		WastePeak;
        size_t		UsedCurrent;
        size_t		UsedPeak;
        size_t		CurrentAllocs;
        size_t		TotalAllocs;
        /** OsCurrent - WasteCurrent - UsedCurrent. */
        size_t		SlackCurrent;
        double		MemTime;
#endif

        // Implementation.
        void OutOfMemory(unsigned long long Size, unsigned int Alignment=0)
        {
            // this is expected not to return
            //FPlatformMemory::OnOutOfMemory(Size, Alignment);
        }

        inline void TrackStats(SPoolTable* Table, size_t Size)
        {
#if STATS
            // keep track of memory lost to padding
            Table->TotalWaste += Table->BlockSize - Size;
            Table->TotalRequests++;
            Table->ActiveRequests++;
            Table->MaxActiveRequests = Max(Table->MaxActiveRequests, Table->ActiveRequests);
            Table->MaxRequest = Size > Table->MaxRequest ? Size : Table->MaxRequest;
            Table->MinRequest = Size < Table->MinRequest ? Size : Table->MinRequest;
#endif
        }

        /**
        * Create a 64k page of SPoolInfo structures for tracking allocations
        */
        SPoolInfo* CreateIndirect()
        {
            assert(IndirectPoolBlockSize * sizeof(SPoolInfo) <= PageSize);
            SPoolInfo* Indirect = (SPoolInfo*)BinnedAllocFromOS( (size_t)(IndirectPoolBlockSize * sizeof(SPoolInfo)) );
            if( !Indirect )
            {
                OutOfMemory(IndirectPoolBlockSize * sizeof(SPoolInfo));
            }
            memset(Indirect, 0, (size_t)IndirectPoolBlockSize*sizeof(SPoolInfo));
            //STAT(OsPeak = std::max(OsPeak, OsCurrent += Align(IndirectPoolBlockSize * sizeof(SPoolInfo), PageSize)));
            //STAT(WastePeak = std::max(WastePeak, WasteCurrent += Align(IndirectPoolBlockSize * sizeof(SPoolInfo), PageSize)));
            return Indirect;
        }

        /**
        * Gets the SPoolInfo for a memory address. If no valid info exists one is created.
        * NOTE: This function requires a mutex across threads, but its is the callers responsibility to
        * acquire the mutex before calling
        */
        inline SPoolInfo* GetPoolInfo( size_t Ptr )
        {
            if (!HashBuckets)
            {
                // Init tables.
                HashBuckets = (PoolHashBucket*)BinnedAllocFromOS(Align((size_t)MaxHashBuckets*sizeof(PoolHashBucket), PageSize));

                for (unsigned int i=0; i<MaxHashBuckets; ++i)
                {
                    new (HashBuckets+i) PoolHashBucket();
                }
            }

            size_t Key=Ptr>>HashKeyShift;
            size_t Hash=Key&(MaxHashBuckets-1);
            size_t PoolIndex=((size_t)Ptr >> PoolBitShift) & PoolMask;

            PoolHashBucket* collision=&HashBuckets[Hash];
            do
            {
                if (collision->Key==Key || !collision->FirstPool)
                {
                    if (!collision->FirstPool)
                    {
                        collision->Key=Key;
                        InitializeHashBucket(collision);
                        //CA_ASSUME(collision->FirstPool);
                    }
                    return &collision->FirstPool[PoolIndex];
                }
                collision=collision->Next;
            } while (collision!=&HashBuckets[Hash]);
            //Create a new hash bucket entry
            PoolHashBucket* NewBucket=CreateHashBucket();
            NewBucket->Key=Key;
            HashBuckets[Hash].Link(NewBucket);
            return &NewBucket->FirstPool[PoolIndex];
        }

        inline SPoolInfo* FindPoolInfo(size_t Ptr1, size_t& AllocationBase)
        {
            unsigned short NextStep = 0;
            size_t Ptr=Ptr1&~((size_t)PageSize-1);
            for (unsigned int i=0, n=(BINNED_ALLOC_POOL_SIZE/PageSize)+1; i<n; ++i)
            {
                SPoolInfo* Pool = FindPoolInfoInternal(Ptr, NextStep);
                if (Pool)
                {
                    AllocationBase=Ptr;
                    //assert(Ptr1 >= AllocationBase && Ptr1 < AllocationBase+Pool->GetBytes());
                    return Pool;
                }
                Ptr = ((Ptr-(PageSize*NextStep))-1)&~((size_t)PageSize-1);
            }
            AllocationBase=0;
            return NULL;
        }

        inline SPoolInfo* FindPoolInfoInternal(size_t Ptr, unsigned short& JumpOffset)
        {
            assert(HashBuckets);

            unsigned int Key=Ptr>>HashKeyShift;
            unsigned int Hash=Key&(MaxHashBuckets-1);
            unsigned int PoolIndex=((size_t)Ptr >> PoolBitShift) & PoolMask;
            JumpOffset=0;

            PoolHashBucket* collision=&HashBuckets[Hash];
            do
            {
                if (collision->Key==Key)
                {
                    if (!collision->FirstPool[PoolIndex].AllocSize)
                    {
                        JumpOffset = collision->FirstPool[PoolIndex].TableIndex;
                        return NULL;
                    }
                    return &collision->FirstPool[PoolIndex];
                }
                collision=collision->Next;
            } while (collision!=&HashBuckets[Hash]);

            return NULL;
        }

        /**
        *	Returns a newly created and initialized PoolHashBucket for use.
        */
        inline PoolHashBucket* CreateHashBucket()
        {
            PoolHashBucket* bucket=AllocateHashBucket();
            InitializeHashBucket(bucket);
            return bucket;
        }

        /**
        *	Initializes bucket with valid parameters
        *	@param bucket pointer to be initialized
        */
        inline void InitializeHashBucket(PoolHashBucket* bucket)
        {
            if (!bucket->FirstPool)
            {
                bucket->FirstPool=CreateIndirect();
            }
        }

        /**
        * Allocates a hash bucket from the free list of hash buckets
        */
        PoolHashBucket* AllocateHashBucket()
        {
            if (!HashBucketFreeList)
            {
                HashBucketFreeList=(PoolHashBucket*)BinnedAllocFromOS(PageSize);
                //STAT(OsPeak = std::max(OsPeak, OsCurrent += PageSize));
                //STAT(WastePeak = std::max(WastePeak, WasteCurrent += PageSize));
                for (size_t i=0, n=(PageSize/sizeof(PoolHashBucket)); i<n; ++i)
                {
                    HashBucketFreeList->Link(new (HashBucketFreeList+i) PoolHashBucket());
                }
            }
            PoolHashBucket* NextFree=HashBucketFreeList->Next;
            PoolHashBucket* Free=HashBucketFreeList;
            Free->Unlink();
            if (NextFree==Free)
            {
                NextFree=NULL;
            }
            HashBucketFreeList=NextFree;
            return Free;
        }

        SPoolInfo* AllocatePoolMemory(SPoolTable* Table, unsigned int PoolSize, unsigned short TableIndex)
        {
            // Must create a new pool.
            unsigned int Blocks = PoolSize / Table->BlockSize;
            unsigned int Bytes = Blocks * Table->BlockSize;
            size_t OsBytes=Align(Bytes, PageSize);
            assert(Blocks >= 1);
            assert(Blocks * Table->BlockSize <= Bytes && PoolSize >= Bytes);

            // Allocate memory.
            SFreeMem* Free = NULL;
            size_t ActualPoolSize; //TODO: use this to reduce waste?
            Free = (SFreeMem*)OSAlloc(OsBytes, ActualPoolSize);

            assert(!((size_t)Free & (PageSize-1)));
            if( !Free )
            {
                OutOfMemory(OsBytes);
            }

            // Create pool in the indirect table.
            SPoolInfo* Pool;
            {
#ifdef USE_FINE_GRAIN_LOCKS
                MutexLock PoolInfoLock(AccessGuard);
#endif
                Pool = GetPoolInfo((size_t)Free);
                for (size_t i=(size_t)PageSize, Offset=0; i<OsBytes; i+=PageSize, ++Offset)
                {
                    SPoolInfo* TrailingPool = GetPoolInfo(((size_t)Free)+i);
                    assert(TrailingPool);
                    //Set trailing pools to point back to first pool
                    TrailingPool->SetAllocationSizes(0, 0, Offset, (unsigned int)BinnedOSTableIndex);
                }
            }

            // Init pool.
            Pool->Link( Table->FirstPool );
            Pool->SetAllocationSizes(Bytes, OsBytes, TableIndex, (unsigned int)BinnedOSTableIndex);
            //STAT(OsPeak = std::max(OsPeak, OsCurrent += OsBytes));
            //STAT(WastePeak = std::max(WastePeak, WasteCurrent += OsBytes - Bytes));
            Pool->Taken		 = 0;
            Pool->FirstMem   = Free;

#if STATS
            Table->NumActivePools++;
            Table->MaxActivePools = sdt::max(Table->MaxActivePools, Table->NumActivePools);
#endif
            // Create first free item.
            Free->NumFreeBlocks = Blocks;
            Free->Next       = NULL;

            return Pool;
        }

        inline SFreeMem* AllocateBlockFromPool(SPoolTable* Table, SPoolInfo* Pool)
        {
            // Pick first available block and unlink it.
            Pool->Taken++;
            assert(Pool->TableIndex < BinnedOSTableIndex); // if this is false, FirstMem is actually a size not a pointer
            assert(Pool->FirstMem);
            assert(Pool->FirstMem->NumFreeBlocks > 0);
            assert(Pool->FirstMem->NumFreeBlocks <= PAGE_SIZE_LIMIT);
            SFreeMem* Free = (SFreeMem*)((unsigned char*)Pool->FirstMem + --Pool->FirstMem->NumFreeBlocks * Table->BlockSize);
            if( !Pool->FirstMem->NumFreeBlocks )
            {
                Pool->FirstMem = Pool->FirstMem->Next;
                if( !Pool->FirstMem )
                {
                    // Move to exhausted list.
                    Pool->Unlink();
                    Pool->Link( Table->ExhaustedPool );
                }
            }
            //STAT(UsedPeak = std::max(UsedPeak, UsedCurrent += Table->BlockSize));
            return Free;
        }

        /**
        * Releases memory back to the system. This is not protected from multi-threaded access and it's
        * the callers responsibility to Lock AccessGuard before calling this.
        */
        void FreeInternal( void* Ptr )
        {
            //MEM_TIME(MemTime -= FPlatformTime::Seconds());
            //STAT(CurrentAllocs--);

            size_t BasePtr;
            SPoolInfo* Pool = FindPoolInfo((size_t)Ptr, BasePtr);
            assert(Pool);
            assert(Pool->GetBytes() != 0);
            if( Pool->TableIndex < BinnedOSTableIndex )
            {
                SPoolTable* Table=MemSizeToPoolTable[Pool->TableIndex];
#ifdef USE_FINE_GRAIN_LOCKS
                MutexLock TableLock(Table->CriticalSection);
#endif
#if STATS
                Table->ActiveRequests--;
#endif
                // If this pool was exhausted, move to available list.
                if( !Pool->FirstMem )
                {
                    Pool->Unlink();
                    Pool->Link( Table->FirstPool );
                }

                // Free a pooled allocation.
                SFreeMem* Free		= (SFreeMem*)Ptr;
                Free->NumFreeBlocks	= 1;
                Free->Next			= Pool->FirstMem;
                Pool->FirstMem		= Free;
                //STAT(UsedCurrent -= Table->BlockSize);
			
#if defined(ECHO_PLATFORM_WINDOWS) && defined(_DEBUG)
				memset(Free + 1, 0, Table->BlockSize - sizeof(SFreeMem)); 
#endif

                // Free this pool.
                assert(Pool->Taken >= 1);
                if( --Pool->Taken == 0 )
                {
#if STATS
                    Table->NumActivePools--;
#endif
                    // Free the OS memory.
                    size_t OsBytes = Pool->GetOsBytes(PageSize, (unsigned int)BinnedOSTableIndex);
                    //STAT(OsCurrent -= OsBytes);
                    //STAT(WasteCurrent -= OsBytes - Pool->GetBytes());
                    Pool->Unlink();
                    Pool->SetAllocationSizes(0, 0, 0, (unsigned int)BinnedOSTableIndex);
                    OSFree((void*)BasePtr, OsBytes);
                }
            }
            else
            {
                // Free an OS allocation.
                assert(!((size_t)Ptr & (PageSize-1)));
                size_t OsBytes = Pool->GetOsBytes(PageSize, (unsigned int)BinnedOSTableIndex);
                //STAT(UsedCurrent -= Pool->GetBytes());
                //STAT(OsCurrent -= OsBytes);
                //STAT(WasteCurrent -= OsBytes - Pool->GetBytes());
                OSFree(Ptr, OsBytes);
            }

			memx().on_delete(Ptr);

            //MEM_TIME(MemTime += FPlatformTime::Seconds());
        }

        void PushFreeLockless(void* Ptr)
        {
#ifdef USE_COARSE_GRAIN_LOCKS
            MutexLock ScopedLock(AccessGuard);
#endif

            FreeInternal(Ptr);
        }

        /**
        * Clear and Process the list of frees to be deallocated. It's the callers
        * responsibility to Lock AccessGuard before calling this
        */
        void FlushPendingFrees()
        {

        }

        inline void OSFree(void* Ptr, size_t Size)
        {
#ifdef CACHE_FREED_OS_ALLOCS
#ifdef USE_FINE_GRAIN_LOCKS
            MutexLock MainLock(AccessGuard);
#endif
            if ((CachedTotal + Size > MAX_CACHED_OS_FREES_BYTE_LIMIT) || (Size > BINNED_ALLOC_POOL_SIZE))
            {
                BinnedFreeToOS(Ptr);
                return;
            }
            if (FreedPageBlocksNum >= MAX_CACHED_OS_FREES)
            {
                //Remove the oldest one
                void* FreePtr = FreedPageBlocks[FreedPageBlocksNum-1].Ptr;
                CachedTotal -= FreedPageBlocks[FreedPageBlocksNum-1].ByteSize;
                --FreedPageBlocksNum;
                BinnedFreeToOS(FreePtr);
            }
            FreedPageBlocks[FreedPageBlocksNum].Ptr = Ptr;
            FreedPageBlocks[FreedPageBlocksNum].ByteSize = Size;
            CachedTotal += Size;
            ++FreedPageBlocksNum;
#else
            (void)Size;
            BinnedFreeToOS(Ptr);
#endif
        }

        inline void* OSAlloc(size_t NewSize, size_t& OutActualSize)
        {
#ifdef CACHE_FREED_OS_ALLOCS
            {
#ifdef USE_FINE_GRAIN_LOCKS
                // We want to hold the lock a little as possible so release it
                // before the big call to the OS
                MutexLock MainLock(AccessGuard);
#endif
                for (unsigned int i=0; i < FreedPageBlocksNum; ++i)
                {
                    // is it possible (and worth i.e. <25% overhead) to use this block
                    if (FreedPageBlocks[i].ByteSize >= NewSize && FreedPageBlocks[i].ByteSize * 3 <= NewSize * 4)
                    {
                        void* Ret=FreedPageBlocks[i].Ptr;
                        OutActualSize=FreedPageBlocks[i].ByteSize;
                        CachedTotal-=FreedPageBlocks[i].ByteSize;
                        FreedPageBlocks[i]=FreedPageBlocks[--FreedPageBlocksNum];
                        return Ret;
                    }
                };
            }
            OutActualSize=NewSize;
            void* Ptr = BinnedAllocFromOS(NewSize);
            if (!Ptr)
            {
                //Are we holding on to much mem? Release it all.
                FlushAllocCache();
                Ptr = BinnedAllocFromOS(NewSize);
            }
            return Ptr;
#else
            (void)OutActualSize;
            return BinnedAllocFromOS(NewSize);
#endif
        }

#ifdef CACHE_FREED_OS_ALLOCS
        void FlushAllocCache()
        {
#ifdef USE_FINE_GRAIN_LOCKS
            MutexLock MainLock(AccessGuard);
#endif
            for (int i=0, n=FreedPageBlocksNum; i<n; ++i)
            {
                //Remove allocs
                BinnedFreeToOS(FreedPageBlocks[i].Ptr);
                FreedPageBlocks[i].Ptr = NULL;
                FreedPageBlocks[i].ByteSize = 0;
            }
            FreedPageBlocksNum = 0;
            CachedTotal = 0;
        }
#endif

    public:
        // InPageSize - First parameter is page size, all allocs from BinnedAllocFromOS() MUST be aligned to this size
        // AddressLimit - Second parameter is estimate of the range of addresses expected to be returns by BinnedAllocFromOS(). Binned
        // Malloc will adjust it's internal structures to make look ups for memory allocations O(1) for this range.
        // It's is ok to go outside this range, look ups will just be a little slower
        MallocBinned(unsigned int InPageSize, unsigned long long AddressLimit)
            :	TableAddressLimit(AddressLimit)
            ,	HashBuckets(NULL)
            ,	HashBucketFreeList(NULL)
            ,	PageSize		(InPageSize)
#ifdef CACHE_FREED_OS_ALLOCS
            ,	FreedPageBlocksNum(0)
            ,	CachedTotal(0)
#endif
#if STATS
            ,	OsCurrent		( 0 )
            ,	OsPeak			( 0 )
            ,	WasteCurrent	( 0 )
            ,	WastePeak		( 0 )
            ,	UsedCurrent		( 0 )
            ,	UsedPeak		( 0 )
            ,	CurrentAllocs	( 0 )
            ,	TotalAllocs		( 0 )
            ,	SlackCurrent	( 0 )
            ,	MemTime			( 0.0 )
#endif
        {
            assert(!(PageSize & (PageSize - 1)));
            assert(!(AddressLimit & (AddressLimit - 1)));
            assert(PageSize <= 65536); // There is internal limit on page size of 64k
            assert(AddressLimit > PageSize); // assert to catch 32 bit overflow in AddressLimit
            assert(sizeof(SPoolInfo) == 32);

            /** Shift to get the reference from the indirect tables */
            PoolBitShift = CeilLogTwo(PageSize);
            IndirectPoolBitShift = CeilLogTwo(PageSize/sizeof(SPoolInfo));
            IndirectPoolBlockSize = PageSize/sizeof(SPoolInfo);

            MaxHashBuckets = AddressLimit >> (IndirectPoolBitShift+PoolBitShift);
            MaxHashBucketBits = CeilLogTwo((unsigned int)MaxHashBuckets);
            MaxHashBucketWaste = (MaxHashBuckets*sizeof(PoolHashBucket))/1024;
            MaxBookKeepingOverhead = ((AddressLimit/PageSize)*sizeof(PoolHashBucket))/(1024*1024);
            /**
            * Shift required to get required hash table key.
            */
            HashKeyShift = PoolBitShift+IndirectPoolBitShift;
            /** Used to mask off the bits that have been used to lookup the indirect table */
            PoolMask =  ( ( 1 << ( HashKeyShift - PoolBitShift ) ) - 1 );
            BinnedSizeLimit = PAGE_SIZE_LIMIT/2;
            BinnedOSTableIndex = BinnedSizeLimit+EXTENED_PAGE_POOL_ALLOCATION_COUNT;

            assert((BinnedSizeLimit & (BinnedSizeLimit-1)) == 0);


            // Init tables.
            OsTable.FirstPool = NULL;
            OsTable.ExhaustedPool = NULL;
            OsTable.BlockSize = 0;

            /** The following options are not valid for page sizes less than 64k. They are here to reduce waste*/
            PagePoolTable[0].FirstPool = NULL;
            PagePoolTable[0].ExhaustedPool = NULL;
            PagePoolTable[0].BlockSize = PageSize == PAGE_SIZE_LIMIT ? (unsigned int)( BinnedSizeLimit+(BinnedSizeLimit/2) ) : 0;

            PagePoolTable[1].FirstPool = NULL;
            PagePoolTable[1].ExhaustedPool = NULL;
            PagePoolTable[1].BlockSize = PageSize == PAGE_SIZE_LIMIT ? (unsigned int)(PageSize+BinnedSizeLimit) : 0;

            // Block sizes are based around getting the maximum amount of allocations per pool, with as little alignment waste as possible.
            // Block sizes should be close to even divisors of the POOL_SIZE, and well distributed. They must be 16-byte aligned as well.
            static const unsigned int BlockSizes[POOL_COUNT] =
            {
                8,		16,		32,		48,		64,		80,		96,		112,
                128,	160,	192,	224,	256,	288,	320,	384,
                448,	512,	576,	640,	704,	768,	896,	1024,
                1168,	1360,	1632,	2048,	2336,	2720,	3264,	4096,
                4672,	5456,	6544,	8192,	9360,	10912,	13104,	16384,
                21840,	32768
            };

            for( unsigned int i = 0; i < POOL_COUNT; i++ )
            {
                PoolTable[i].FirstPool = NULL;
                PoolTable[i].ExhaustedPool = NULL;
                PoolTable[i].BlockSize = BlockSizes[i];
#if STATS
                PoolTable[i].MinRequest = PoolTable[i].BlockSize;
#endif
            }

            for( unsigned int i=0; i<MAX_POOLED_ALLOCATION_SIZE; i++ )
            {
                unsigned int Index = 0;
                while( PoolTable[Index].BlockSize < i )
                {
                    ++Index;
                }
                assert(Index < POOL_COUNT);
                MemSizeToPoolTable[i] = &PoolTable[Index];
            }

            MemSizeToPoolTable[BinnedSizeLimit] = &PagePoolTable[0];
            MemSizeToPoolTable[BinnedSizeLimit+1] = &PagePoolTable[1];

            assert(MAX_POOLED_ALLOCATION_SIZE - 1 == PoolTable[POOL_COUNT - 1].BlockSize);
        }

    public:
        //static MallocBinned* BaseAllocator();

        void* BinnedAllocFromOS( size_t Size );

        void BinnedFreeToOS( void* Ptr );
		void CheckLeak()
		{
		}

        virtual ~MallocBinned()
        {

		}

        /**
        * Returns if the allocator is guaranteed to be thread-safe and therefore
        * doesn't need a unnecessary thread-safety wrapper around it.
        */
        virtual bool IsInternallyThreadSafe() const
        {
#ifdef USE_INTERNAL_LOCKS
            return true;
#else
            return false;
#endif
        }

        /**
        * Malloc
        */
        virtual void* Malloc( size_t Size, unsigned int Alignment )
        {
#ifdef USE_COARSE_GRAIN_LOCKS
            MutexLock ScopedLock(AccessGuard);
#endif

           // FlushPendingFrees();

            // Handle DEFAULT_ALIGNMENT for binned allocator.
            if (Alignment == DEFAULT_ALIGNMENT)
            {
                Alignment = DEFAULT_BINNED_ALLOCATOR_ALIGNMENT;
            }
            assert(Alignment <= PageSize);
            Alignment = std::max<unsigned int>(Alignment, DEFAULT_BINNED_ALLOCATOR_ALIGNMENT);
            Size = std::max<size_t>(Alignment, Align(Size, Alignment));


            //STAT(CurrentAllocs++);
            //STAT(TotalAllocs++);
            SFreeMem* Free;
            if( Size < BinnedSizeLimit )
            {
                // Allocate from pool.
                SPoolTable* Table = MemSizeToPoolTable[Size];
#ifdef USE_FINE_GRAIN_LOCKS
                MutexLock TableLock(Table->CriticalSection);
#endif
                assert(Size <= Table->BlockSize);

                TrackStats(Table, Size);

                SPoolInfo* Pool = Table->FirstPool;
                if( !Pool )
                {
                    Pool = AllocatePoolMemory(Table, BINNED_ALLOC_POOL_SIZE/*PageSize*/, Size);
                }

                Free = AllocateBlockFromPool(Table, Pool);
            }
            else if ( ((Size >= BinnedSizeLimit && Size <= PagePoolTable[0].BlockSize) ||
                (Size > PageSize && Size <= PagePoolTable[1].BlockSize))
                && Alignment == DEFAULT_BINNED_ALLOCATOR_ALIGNMENT )
            {
                // Bucket in a pool of 3*PageSize or 6*PageSize
                unsigned int BinType = Size < PageSize ? 0 : 1;
                unsigned int PageCount = 3*BinType + 3;
                SPoolTable* Table = &PagePoolTable[BinType];
#ifdef USE_FINE_GRAIN_LOCKS
                MutexLock TableLock(Table->CriticalSection);
#endif
                assert(Size <= Table->BlockSize);

                TrackStats(Table, Size);

                SPoolInfo* Pool = Table->FirstPool;
                if( !Pool )
                {
                    Pool = AllocatePoolMemory(Table, PageCount*PageSize, static_cast<unsigned short>(BinnedSizeLimit+BinType) );
                }

                Free = AllocateBlockFromPool(Table, Pool);
            }
            else
            {
                // Use OS for large allocations.
                size_t AlignedSize = Align(Size,PageSize);
                size_t ActualPoolSize; //TODO: use this to reduce waste?
                Free = (SFreeMem*)OSAlloc(AlignedSize, ActualPoolSize);
                if( !Free )
                {
                    OutOfMemory(AlignedSize);
                }
                assert(!((size_t)Free & (PageSize-1)));

                // Create indirect.
                SPoolInfo* Pool;
                {
#ifdef USE_FINE_GRAIN_LOCKS
                    MutexLock PoolInfoLock(AccessGuard);
#endif
                    Pool = GetPoolInfo((size_t)Free);
                }

                Pool->SetAllocationSizes(Size, AlignedSize, static_cast<unsigned int>(BinnedOSTableIndex), static_cast<unsigned int>(BinnedOSTableIndex) );
                //STAT(OsPeak = std::max(OsPeak, OsCurrent += AlignedSize));
                //STAT(UsedPeak = std::max(UsedPeak, UsedCurrent += Size));
                //STAT(WastePeak = std::max(WastePeak, WasteCurrent += AlignedSize - Size));
            }

			memx().on_new(Free, Size, 3);
            return Free;
        }

        /**
        * Realloc
        */
        virtual void* Realloc( void* Ptr, size_t NewSize, unsigned int Alignment)
        {
            // Handle DEFAULT_ALIGNMENT for binned allocator.
            if (Alignment == DEFAULT_ALIGNMENT)
            {
                Alignment = DEFAULT_BINNED_ALLOCATOR_ALIGNMENT;
            }
            assert(Alignment <= PageSize);
            Alignment = std::max<unsigned int>(Alignment, DEFAULT_BINNED_ALLOCATOR_ALIGNMENT);
            if (NewSize)
            {
                NewSize = std::max<size_t>(Alignment, Align(NewSize, Alignment));
            }


            size_t BasePtr;
            void* NewPtr = Ptr;
            if( Ptr && NewSize )
            {
                SPoolInfo* Pool = FindPoolInfo((size_t)Ptr, BasePtr);

                if( Pool->TableIndex < BinnedOSTableIndex )
                {
                    // Allocated from pool, so grow or shrink if necessary.
                    assert(Pool->TableIndex > 0); // it isn't possible to allocate a size of 0, Malloc will increase the size to DEFAULT_BINNED_ALLOCATOR_ALIGNMENT
                    if( NewSize>MemSizeToPoolTable[Pool->TableIndex]->BlockSize || NewSize<=MemSizeToPoolTable[Pool->TableIndex-1]->BlockSize )
                    {
                        NewPtr = Malloc( NewSize, Alignment );
                        memcpy( NewPtr, Ptr, std::min<size_t>( NewSize, MemSizeToPoolTable[Pool->TableIndex]->BlockSize ) );
                        Free( Ptr );
                    }
                }
                else
                {
                    // Allocated from OS.
                    assert(!((size_t)Ptr & (PageSize-1)));
                    if( NewSize > Pool->GetOsBytes(PageSize, (unsigned int)BinnedOSTableIndex) || NewSize * 3 < Pool->GetOsBytes(PageSize, (unsigned int)BinnedOSTableIndex) * 2 )
                    {
                        // Grow or shrink.
                        NewPtr = Malloc( NewSize, Alignment );
                        memcpy( NewPtr, Ptr, std::min<size_t>(NewSize, Pool->GetBytes()) );
                        Free( Ptr );
                    }
                    else
                    {
                        // Keep as-is, reallocation isn't worth the overhead.
                        //STAT(UsedCurrent += NewSize - Pool->GetBytes());
                        //STAT(UsedPeak = std::max(UsedPeak, UsedCurrent));
                        //STAT(WasteCurrent += Pool->GetBytes() - NewSize);
                        Pool->SetAllocationSizes(NewSize, Pool->GetOsBytes(PageSize, (unsigned int)BinnedOSTableIndex), (unsigned int)BinnedOSTableIndex, (unsigned int)BinnedOSTableIndex);
                    }
                }
            }
            else if( Ptr == NULL )
            {
                NewPtr = Malloc( NewSize, Alignment );
            }
            else
            {
                Free( Ptr );
                NewPtr = NULL;
            }



            return NewPtr;
        }

        /**
        * Free
        */
        virtual void Free( void* Ptr )
        {
            if( !Ptr )
            {
                return;
            }

            PushFreeLockless(Ptr);
        }

        /**
        * If possible determine the size of the memory allocated at the given address
        *
        * @param Original - Pointer to memory we are asserting the size of
        * @param SizeOut - If possible, this value is set to the size of the passed in pointer
        * @return true if succeeded
        */
        virtual bool GetAllocationSize(void *Original, size_t &SizeOut)
        {
            if (!Original)
            {
                return false;
            }
            size_t BasePtr;
            SPoolInfo* Pool = FindPoolInfo((size_t)Original, BasePtr);
            SizeOut = Pool->TableIndex < BinnedSizeLimit ? MemSizeToPoolTable[Pool->TableIndex]->BlockSize : Pool->GetBytes();
            return true;
        }

        /**
        * Validates the allocator's heap
        */
        virtual bool ValidateHeap()
        {
#ifdef USE_COARSE_GRAIN_LOCKS
            MutexLock ScopedLock(AccessGuard);
#endif
            for( int i = 0; i < POOL_COUNT; i++ )
            {
                SPoolTable* Table = &PoolTable[i];
#ifdef USE_FINE_GRAIN_LOCKS
                MutexLock TableLock(Table->CriticalSection);
#endif
                for( SPoolInfo** PoolPtr = &Table->FirstPool; *PoolPtr; PoolPtr = &(*PoolPtr)->Next )
                {
                    SPoolInfo* Pool = *PoolPtr;
                    assert(Pool->PrevLink == PoolPtr);
                    assert(Pool->FirstMem );
                    for( SFreeMem* Free = Pool->FirstMem; Free; Free = Free->Next )
                    {
                        assert(Free->NumFreeBlocks > 0);
                    }
                }
                for( SPoolInfo** PoolPtr = &Table->ExhaustedPool; *PoolPtr; PoolPtr = &(*PoolPtr)->Next )
                {
                    SPoolInfo* Pool = *PoolPtr;
                    assert(Pool->PrevLink == PoolPtr);
                    assert(!Pool->FirstMem);
                }
            }

            return( true );
        }


        /** Called once per frame, gathers and sets all memory allocator statistics into the corresponding stats. */
        virtual void UpdateStats()
        {
#if STATS
            size_t	LocalOsCurrent = 0;
            size_t	LocalOsPeak = 0;
            size_t	LocalWasteCurrent = 0;
            size_t	LocalWastePeak = 0;
            size_t	LocalUsedCurrent = 0;
            size_t	LocalUsedPeak = 0;
            size_t	LocalCurrentAllocs = 0;
            size_t	LocalTotalAllocs = 0;
            size_t	LocalSlackCurrent = 0;

            {
#ifdef USE_INTERNAL_LOCKS
                MutexLock ScopedLock( AccessGuard );
#endif
                UpdateSlackStat();

                // Copy memory stats.
                LocalOsCurrent = OsCurrent;
                LocalOsPeak = OsPeak;
                LocalWasteCurrent = WasteCurrent;
                LocalWastePeak = WastePeak;
                LocalUsedCurrent = UsedCurrent;
                LocalUsedPeak = UsedPeak;
                LocalCurrentAllocs = CurrentAllocs;
                LocalTotalAllocs = TotalAllocs;
                LocalSlackCurrent = SlackCurrent;
            }

            SET_MEMORY_STAT( STAT_Binned_OsCurrent, LocalOsCurrent );
            SET_MEMORY_STAT( STAT_Binned_OsPeak, LocalOsPeak );
            SET_MEMORY_STAT( STAT_Binned_WasteCurrent, LocalWasteCurrent );
            SET_MEMORY_STAT( STAT_Binned_WastePeak, LocalWastePeak );
            SET_MEMORY_STAT( STAT_Binned_UsedCurrent, LocalUsedCurrent );
            SET_MEMORY_STAT( STAT_Binned_UsedPeak, LocalUsedPeak );
            SET_MEMORY_STAT( STAT_Binned_CurrentAllocs, LocalCurrentAllocs );
            SET_MEMORY_STAT( STAT_Binned_TotalAllocs, LocalTotalAllocs );
            SET_MEMORY_STAT( STAT_Binned_SlackCurrent, LocalSlackCurrent );
#endif
        }

        /** Writes allocator stats from the last update into the specified destination. */
        //virtual void GetAllocatorStats( FGenericMemoryStats& out_Stats );

        /**
        * Dumps allocator stats to an output device. Subclasses should override to add additional info
        *
        * @param Ar	[in] Output device
        */
        virtual void DumpAllocatorStats( const std::string& logfile )
        {
        }

        virtual const char * GetDescriptiveName() { return ("binned"); }

        
        
    protected:

        void UpdateSlackStat()
        {
#if	STATS
            size_t LocalWaste = WasteCurrent;
            double Waste = 0.0;
            for( int PoolIndex = 0; PoolIndex < POOL_COUNT; PoolIndex++ )
            {
                Waste += ( ( double )PoolTable[PoolIndex].TotalWaste / ( double )PoolTable[PoolIndex].TotalRequests ) * ( double )PoolTable[PoolIndex].ActiveRequests;
                Waste += PoolTable[PoolIndex].NumActivePools * ( BINNED_ALLOC_POOL_SIZE - ( ( BINNED_ALLOC_POOL_SIZE / PoolTable[PoolIndex].BlockSize ) * PoolTable[PoolIndex].BlockSize ) );
            }
            LocalWaste += ( unsigned int )Waste;
            SlackCurrent = OsCurrent - LocalWaste - UsedCurrent;
#endif // STATS
        }

        ///////////////////////////////////
        //// API platforms must implement
        ///////////////////////////////////
        /**
        * @return the page size the OS uses
        *	NOTE: This MUST be implemented on each platform that uses this!
        */
        unsigned int BinnedGetPageSize();


    };

    class MallocDebug : public MallocInterface
    {
        // Tags.
        enum { MEM_PreTag  = 0xf0ed1cee };
        enum { MEM_PostTag = 0xdeadf00f };
        enum { MEM_Tag     = 0xfe       };
        enum { MEM_WipeTag = 0xcd       };
        
        // Alignment.
        enum { ALLOCATION_ALIGNMENT = 16 };
        
        // Number of block sizes to collate (in steps of 4 bytes)
        enum { MEM_SizeMax = 128 };
        enum { MEM_Recent = 5000 };
        enum { MEM_AgeMax = 80 };
        enum { MEM_AgeSlice = 100 };
        
    private:
        // Structure for memory debugging.
        struct MemDebug
        {
            size_t		Size;
            int		RefCount;
            int*		PreTag;
            MemDebug*	Next;
            MemDebug**	PrevLink;
        };
        
        MemDebug*	GFirstDebug;
        
        /** Total size of allocations */
        size_t		TotalAllocationSize;
        /** Total size of allocation overhead */
        size_t		TotalWasteSize;
        
        static const unsigned int AllocatorOverhead = sizeof(MemDebug) + sizeof(MemDebug*) + sizeof(int) + ALLOCATION_ALIGNMENT + sizeof(int);
        
    public:
        // FMalloc interface.
        MallocDebug()
        :	GFirstDebug( NULL )
        ,	TotalAllocationSize( 0 )
        ,	TotalWasteSize( 0 )
        {}
        
        /**
         * Malloc
         */
        virtual void* Malloc( size_t Size, unsigned int Alignment )
        {
            assert(Alignment == DEFAULT_ALIGNMENT && "Alignment currently unsupported in this allocator");
            MemDebug* Ptr = NULL;
            Ptr = (MemDebug*)malloc( AllocatorOverhead + Size );
            assert(Ptr);
            unsigned char* AlignedPtr = Align( (unsigned char*) Ptr + sizeof(MemDebug) + sizeof(MemDebug*) + sizeof(int), ALLOCATION_ALIGNMENT );
            Ptr->RefCount = 1;
        
            Ptr->Size = Size;
            Ptr->Next = GFirstDebug;
            Ptr->PrevLink = &GFirstDebug;
            Ptr->PreTag = (int*) (AlignedPtr - sizeof(int));
            *Ptr->PreTag = MEM_PreTag;
            *((MemDebug**)(AlignedPtr - sizeof(int) - sizeof(MemDebug*)))	= Ptr;
            *(int*)(AlignedPtr+Size) = MEM_PostTag;
            memset( AlignedPtr, MEM_Tag, Size );
            if( GFirstDebug )
            {
                assert(GFirstDebug->PrevLink == &GFirstDebug);
                GFirstDebug->PrevLink = &Ptr->Next;
            }
            GFirstDebug = Ptr;
            TotalAllocationSize += Size;
            TotalWasteSize += AllocatorOverhead;
            assert(!(size_t(AlignedPtr) & ((size_t)0xf)));
            return AlignedPtr;
        }
    
	/**
	 * Realloc
	 */
        virtual void* Realloc( void* InPtr, size_t NewSize, unsigned int Alignment )
        {
            if( InPtr && NewSize )
            {
                MemDebug* Ptr = *((MemDebug**)((unsigned char*)InPtr - sizeof(int) - sizeof(MemDebug*)));
                assert((Ptr->RefCount==1));
                void* Result = Malloc( NewSize, Alignment );
                memcpy( Result, InPtr, std::min<size_t>(Ptr->Size,NewSize) );
                Free( InPtr );
                return Result;
            }
            else if( InPtr == NULL )
            {
                return Malloc( NewSize, Alignment );
            }
            else
            {
                Free( InPtr );
                return NULL;
            }
        }

        /**
         * Free
         */
        virtual void Free( void* InPtr )
        {
            if( !InPtr )
            {
                return;
            }

            MemDebug* Ptr = *((MemDebug**)((unsigned char*)InPtr - sizeof(int) - sizeof(MemDebug*)));

            assert(Ptr->RefCount==1);
            assert(*Ptr->PreTag==MEM_PreTag);
            assert(*(int*)((unsigned char*)InPtr+Ptr->Size)==MEM_PostTag);

            TotalAllocationSize -= Ptr->Size;
            TotalWasteSize -= AllocatorOverhead;

            memset( InPtr, MEM_WipeTag, Ptr->Size );
            Ptr->Size = 0;
            Ptr->RefCount = 0;

            assert(Ptr->PrevLink);
            assert(*Ptr->PrevLink==Ptr);
            *Ptr->PrevLink = Ptr->Next;
            if( Ptr->Next )
            {
                Ptr->Next->PrevLink = Ptr->PrevLink;
            }

            free( Ptr );
        }

        virtual bool GetAllocationSize(void *Original, size_t &SizeOut)
        {
            if( !Original )
            {
                SizeOut = 0;
            }
            else
            {
                const MemDebug* Ptr = *((MemDebug**)((unsigned char*)Original - sizeof(int) - sizeof(MemDebug*)));
                SizeOut = Ptr->Size;
            }

            return true;
        }


        virtual bool ValidateHeap()
        {
            for( MemDebug** Link = &GFirstDebug; *Link; Link=&(*Link)->Next )
            {
                assert(*(*Link)->PrevLink==*Link);
            }

            return( true );
        }

    };

    class MallocAsny: public MallocInterface
    {
        Mutex           m_Mutex;
        MallocInterface* m_MI;
    public:
        MallocAsny(MallocInterface* MI):m_MI(MI){};
        ~MallocAsny(){};
        
        virtual void* Malloc( size_t Size, unsigned int Alignment )
        {
            MutexLock autoLock(m_Mutex);
            return m_MI->Malloc(Size, Alignment);
        }
        
        virtual void Free( void* Ptr )
        {
            if(Ptr)
            {
                MutexLock autoLock(m_Mutex);
                m_MI->Free(Ptr);
            }
        }
        
    };


	MallocInterface* CreateBinnedMalloc()
    {
        static Mutex G_MB_Mutex;
          
        MutexLock lock(G_MB_Mutex);
        
		if (g_binned_malloc)
			return g_binned_malloc;
             
#ifdef ECHO_PLATFORM_IOS
        
        // get free memory
        vm_size_t PageSize;
        host_page_size(mach_host_self(), &PageSize);
        
        vm_statistics Stats;
        mach_msg_type_number_t StatsSize = sizeof(Stats);
        host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&Stats, &StatsSize);
        
        unsigned long long MemoryLimit = std::min<unsigned long long>( (unsigned long long)(1) << CeilLogTwo(Stats.free_count * PageSize), 0x100000000);
        
        g_binned_malloc = new MallocBinned(PageSize, MemoryLimit);
        
#elif defined(ECHO_PLATFORM_WINDOWS)
        
        PERFORMANCE_INFORMATION PerformanceInformation = {0};
        ::GetPerformanceInfo( &PerformanceInformation, sizeof(PerformanceInformation) );
        
        
        g_binned_malloc = new MallocBinned((unsigned int)(PerformanceInformation.PageSize&0xffffffff), (unsigned long long )(0xffffffff)+1);

//#elif defined(ECHO_PLATFORM_ANDROID)
//
//        int NumPhysPages = sysconf(_SC_PHYS_PAGES);
//		int PageSize = sysconf(_SC_PAGESIZE);
//
//		unsigned long long TotalPhysical = NumPhysPages * PageSize;
//
//		unsigned long long TotalPhysicalGB = (TotalPhysical + 1024 * 1024 * 1024 - 1) / 1024 / 1024 / 1024;
//
//        unsigned long long MemoryLimit = std::min<unsigned long long>( unsigned long long(1) << CeilLogTwo(TotalPhysical), 0x100000000);
//
//	    G_MB = new MallocBinned(PageSize, MemoryLimit);
#else
        g_binned_malloc = new MallocBinned(4096, 0x100000000);
        
#endif
        
       // G_MB = new MallocAsny(G_MB);
        

		return g_binned_malloc;
    }
    
	// 分配内存
    void* MallocBinnedMgr::Malloc(size_t count, unsigned int aligment)
    {	
        if(!g_binned_malloc)
        {
            CreateBinnedMalloc();
        }

		MallocInterface* mallocInterface = (MallocInterface*)g_binned_malloc;
        return mallocInterface->Malloc(count, aligment);
    }

	// 重新分配
    void* MallocBinnedMgr::Realloc( void* Ptr, size_t NewSize, unsigned int Alignment )
    {		
        if(!g_binned_malloc)
        {
            CreateBinnedMalloc();
        }

		MallocInterface* mallocInterface = (MallocInterface*)g_binned_malloc;
        return mallocInterface->Realloc(Ptr, NewSize, Alignment);
    }

	// 释放内存
    void MallocBinnedMgr::Free(void* ptr)
    {	
        if(!g_binned_malloc)
        {
            CreateBinnedMalloc();
        }

		MallocInterface* mallocInterface = (MallocInterface*)g_binned_malloc;
        mallocInterface->Free(ptr);
    }

    void MallocBinnedMgr::CheckLeak()
    {	
        if(!g_binned_malloc)
        {
			return;
        }

		MallocInterface* mallocInterface = (MallocInterface*)g_binned_malloc;
        mallocInterface->CheckLeak();
    }

	void MallocBinnedMgr::ReplaceInstance(MallocInterface* mallocInterface)
	{
		g_binned_malloc = mallocInterface;
	}

	MallocInterface* MallocBinnedMgr::CreateInstance()
	{
		if (NULL == g_binned_malloc)
			CreateBinnedMalloc();
		return g_binned_malloc;
	}

	void MallocBinnedMgr::ReleaseInstance()
	{
		if (NULL == g_binned_malloc)
			return;
		delete g_binned_malloc;
		g_binned_malloc = NULL;
	}


    void* MallocBinned::BinnedAllocFromOS( size_t Size )
    {
#ifdef ECHO_PLATFORM_WINDOWS
        return VirtualAlloc( NULL, Size, MEM_COMMIT, PAGE_READWRITE );
#else
        return valloc(Size);

#endif
    }
    
    void MallocBinned::BinnedFreeToOS( void* Ptr )
    {
#ifdef ECHO_PLATFORM_WINDOWS
        VirtualFree( Ptr, 0, MEM_RELEASE );
#else
        free(Ptr);
#endif
    }
static memx::memx* g_memx = NULL;

memx::memx& memx()
{
	if (NULL == g_memx)
	{
		g_memx = new memx::memx;
		g_memx->init("memx.xml");
	}
	assert(g_memx != (void*)(-1));
	assert(g_memx);
	return *g_memx;
}

void release_memx()
{
	if (NULL == g_memx)
		return;
	delete g_memx;
	g_memx = (memx::memx*)(-1);
}


    
}//Echo
#endif
