/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "PxTaskManager.h"
#include "PxTask.h"
#include "PxSpuTask.h"
#include "PxCpuDispatcher.h"
#include "PxSpuDispatcher.h"
#include "PxGpuDispatcher.h"

#include "PsThread.h"
#include "PsAtomic.h"
#include "PsMutex.h"
#include "PsHashMap.h"
#include "PsArray.h"
#include "PsFoundation.h"

#if PX_SUPPORT_VISUAL_DEBUGGER
#include "PxProfileScopedEvent.h"
#include "PxProfileEventSender.h"
#include "PxProfileEventNames.h"
#include "PxProfileZone.h"
#include "PxProfileZoneManager.h"
#endif

#define DOT_LOG 0

// for information on generating tasks graphs see this wiki page
// https://wiki.nvidia.com/engwiki/index.php/PhysX/sdk/InternalDoc_Example_TaskGraph
#if DOT_LOG
#include "stdio.h"
#define LOG_FRAME_NUM 60
static int  framenum;
static FILE *logfile;

#if defined(PX_PS3)
static const char* logFilename = "/app_home/pxtask-graph.txt";
static __thread physx::PxBaseTask* currentTask;
#else
static const char* logFilename = "pxtask-graph.txt";
__declspec(thread) static physx::PxBaseTask* currentTask;
#endif

#endif

#define LOCK()  shdfnd::Mutex::ScopedLock __lock__(mMutex)

namespace physx
{
    const int EOL = -1;
	typedef shdfnd::HashMap<const char *, PxTaskID> PxTaskNameToIDMap;

	struct PxTaskDepTableRow
	{
		PxTaskID    mTaskID;
		int       mNextDep;
	};
	typedef shdfnd::Array<PxTaskDepTableRow> PxTaskDepTable;

	class PxTaskTableRow
	{
	public:
		PxTaskTableRow() : mRefCount( 1 ), mStartDep(EOL), mLastDep(EOL) {}
		void addDependency( PxTaskDepTable& depTable, PxTaskID taskID )
		{
			int newDep = (int) depTable.size();
			PxTaskDepTableRow row;
			row.mTaskID = taskID;
			row.mNextDep = EOL;
			depTable.pushBack( row );

			if( mLastDep == EOL )
			{
				mStartDep = mLastDep = newDep;
			}
			else
			{
				depTable[ (PxU32)mLastDep ].mNextDep = newDep;
				mLastDep = newDep;
			}
		}

		PxTask *    mTask;
		volatile int mRefCount;
		PxTaskType::Enum mType;
		int       mStartDep;
		int       mLastDep;
	};
	typedef shdfnd::Array<PxTaskTableRow> PxTaskTable;


/* Implementation of PxTaskManager abstract API */
class PxTaskMgr : public PxTaskManager, public shdfnd::UserAllocated
{
	PX_NOCOPY(PxTaskMgr)
public:
	PxTaskMgr(PxCpuDispatcher*, PxGpuDispatcher*, PxSpuDispatcher*);
	~PxTaskMgr();

	void     setCpuDispatcher( PxCpuDispatcher& ref )
	{
		mCpuDispatcher = &ref;
	}

	void     setSpuDispatcher( PxSpuDispatcher& ref )
	{
		mSpuDispatcher = &ref;
	}

	void     setGpuDispatcher( PxGpuDispatcher& ref )
	{
		mGpuDispatcher = &ref;
	}

	void     initializeProfiling( physx::PxProfileZoneManager& ref );

	PxCpuDispatcher* getCpuDispatcher() const
	{
		return mCpuDispatcher;
	}

	PxGpuDispatcher* getGpuDispatcher() const
	{
		return mGpuDispatcher;
	}

	PxSpuDispatcher* getSpuDispatcher() const		
	{
		return mSpuDispatcher;
	}

	void	resetDependencies();
	void	startSimulation();
	void	stopSimulation();
	void	taskCompleted( PxTask& task );

	PxTaskID  getNamedTask( const char *name );
	PxTaskID  submitNamedTask( PxTask *task, const char *name, PxTaskType::Enum type = PxTaskType::TT_CPU );
	PxTaskID  submitUnnamedTask( PxTask& task, PxTaskType::Enum type = PxTaskType::TT_CPU );
	PxTask*   getTaskFromID( PxTaskID );

	bool    dispatchTask( PxTaskID taskID, bool gpuGroupStart );
	bool    resolveRow( PxTaskID taskID, bool gpuGroupStart );

	void    release();

	void	finishBefore( PxTask& task, PxTaskID taskID );
	void	startAfter( PxTask& task, PxTaskID taskID );

	void	addReference( PxTaskID taskID );
	void	decrReference( PxTaskID taskID );
	PxI32	getReference( PxTaskID taskID ) const;

	void	decrReference( PxLightCpuTask& lighttask );
	void	addReference( PxLightCpuTask& lighttask );	

	void	decrReference(PxSpuTask& spuTask);

	void	emitStartEvent( PxBaseTask& basetask, PxU32 threadId=0);
	void	emitStopEvent( PxBaseTask& basetask, PxU32 threadId=0);

	PxCpuDispatcher           *mCpuDispatcher;
	PxGpuDispatcher           *mGpuDispatcher;	
	PxSpuDispatcher			*mSpuDispatcher;
	PxTaskNameToIDMap              mName2IDmap;
	volatile int			 mPendingTasks;
    shdfnd::Mutex            mMutex;

	PxTaskDepTable				 mDepTable;
	PxTaskTable				 mTaskTable;

	shdfnd::Array<PxTaskID>	 mStartDispatch;

#if PX_SUPPORT_VISUAL_DEBUGGER
	PxProfileZone*	   	mProfileZone;
#endif

#if DOT_LOG
	static void debugGraphEnd();
	static void debugGraphEdge(PxBaseTask* prev, PxU32 prevIndex, PxU32 prevType, PxBaseTask* next, PxU32 nextIndex, PxU32 nextType, PxU32 weight);
	static void debugGraphBegin(const char* filename);
#endif
	};

PxTaskManager* PxTaskManager::createTaskManager( PxCpuDispatcher* cpuDispatcher, PxGpuDispatcher* gpuDispatcher, PxSpuDispatcher* spuDispatcher )
{
	return PX_NEW(PxTaskMgr)(cpuDispatcher, gpuDispatcher, spuDispatcher);
}

PxTaskMgr::PxTaskMgr( PxCpuDispatcher* cpuDispatcher, PxGpuDispatcher* gpuDispatcher, PxSpuDispatcher* spuDispatcher )
	: mCpuDispatcher( cpuDispatcher )
    , mGpuDispatcher( gpuDispatcher )
	, mSpuDispatcher( spuDispatcher )
	, mPendingTasks( 0 )
	, mDepTable(PX_DEBUG_EXP("PxTaskDepTable"))
	, mTaskTable(PX_DEBUG_EXP("PxTaskTable"))	
	, mStartDispatch(PX_DEBUG_EXP("StartDispatch"))
#if PX_SUPPORT_VISUAL_DEBUGGER
	, mProfileZone(0)
#endif
{
}

#if PX_SUPPORT_VISUAL_DEBUGGER
void PxTaskMgr::initializeProfiling(physx::PxProfileZoneManager& zoneMgr)
{
	if (!mProfileZone)
	{
		mProfileZone = &physx::PxProfileZone::createProfileZone(&shdfnd::getFoundation(), "PxTaskManager");
		zoneMgr.addProfileZone(*mProfileZone);
	}
}
#else
void PxTaskMgr::initializeProfiling(physx::PxProfileZoneManager&) {}
#endif

#if DOT_LOG
void PxTaskMgr::debugGraphBegin(const char* filename)
{
	logfile = fopen(filename, "w");
	
	if (logfile)
	{
		fprintf(logfile, "digraph tasks {\n");
		fprintf(logfile, "dpi=300;\n");
		fprintf(logfile, "node [width=.3, height=0.8 style=\"rounded, filled\"];");
	}
}
void PxTaskMgr::debugGraphEnd()
{
	if (logfile)
	{
		fprintf(logfile, "}\n");
		fclose(logfile);
		logfile = NULL;
	}
}

void PxTaskMgr::debugGraphEdge(PxBaseTask* prev, PxU32 prevIndex, PxU32 prevType, PxBaseTask* next, PxU32 nextIndex, PxU32 nextType, PxU32 weight)
{
	PX_ASSERT(next);

	enum Type
	{
		eCpuNode,
		eSpuNode,
		eJoinNode
	};

	if (logfile)
	{
		// lock
		PxTaskMgr& mgr = static_cast<PxTaskMgr&>(*next->getTaskManager());
		shdfnd::Mutex::ScopedLock lock(mgr.mMutex);

		// check both task and their task manager is valid
		if (prev && prev->mTm)
			fprintf(logfile, "{node [shape=%s,label=\"%s\"] t%d%d};\n", (prevType==eSpuNode)?"box,fillcolor=lightblue":"ellipse,fillcolor=lightgrey", prev->getName(), PxUnionCast<PxU32>(prev), prevIndex);
		
		if (next && next->mTm)
			fprintf(logfile, "{node [shape=%s,label=\"%s\"] t%d%d};\n", (nextType==eSpuNode)?"box,fillcolor=lightblue":"ellipse,fillcolor=lightgrey", next->getName(), PxUnionCast<PxU32>(next), nextIndex);

		if (weight > 0 && prev && next)
			fprintf(logfile, "t%d%d->t%d%d [weight=%d];\n", PxUnionCast<PxU32>(prev), prevIndex, PxUnionCast<PxU32>(next), nextIndex, weight);
	}
}
#endif


PxTaskMgr::~PxTaskMgr()
{
}

void PxTaskMgr::release()
{
#if PX_SUPPORT_VISUAL_DEBUGGER
	if (mProfileZone)
	{
		mProfileZone->release();
	}
#endif
	PX_DELETE(this);
}

void PxTaskMgr::decrReference(PxLightCpuTask& lighttask)
{
#if DOT_LOG	
	PxU32 weight = 1;
#endif

	/* This does not need a lock! */
	if (!physx::shdfnd::atomicDecrement(&lighttask.mRefCount))
	{
#if DOT_LOG
		++weight;
#endif
		PX_ASSERT(mCpuDispatcher);
		if (mCpuDispatcher)
		{
			mCpuDispatcher->submitTask(lighttask);
		}
		else
		{
			lighttask.release();
		}
	}

#if DOT_LOG	
	debugGraphEdge(currentTask, 0, 0, &lighttask, 0, 0, weight);
#endif
}

void PxTaskMgr::addReference(PxLightCpuTask& lighttask)
{
	/* This does not need a lock! */
	physx::shdfnd::atomicIncrement(&lighttask.mRefCount);
}

void PxTaskMgr::decrReference(PxSpuTask& spuTask)
{
#if defined(PX_PS3)

#if DOT_LOG
	PxU32 weight = 1;
#endif

	if( !physx::shdfnd::atomicDecrement( &spuTask.mRefCount ) )
	{
		PX_ASSERT( mSpuDispatcher);
		if( mSpuDispatcher )
		{
			mSpuDispatcher->submitTask(spuTask);

#if DOT_LOG		
			++weight;
			for (PxU32 i=0; i < spuTask.getSpuCount(); ++i)
				debugGraphEdge(&spuTask, i, 1, spuTask.mCont, 0, 0, 2);
#endif
		}
		else
		{
			shdfnd::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Attempting to run a PxSpuTask: %s without a PxSpuDispatcher present.", spuTask.getName());

			spuTask.release();
		}
	}

#if DOT_LOG
	for (int i=0; i < spuTask.getSpuCount(); ++i)
		debugGraphEdge(currentTask, 0, 0, &spuTask, i, 1, weight);
#endif

#else
	PX_UNUSED(spuTask);
#endif // PX_PS3
}

void PxTaskMgr::emitStartEvent(PxBaseTask& basetask, PxU32 threadId)
{
#if DOT_LOG
	currentTask = &basetask;
#endif

	PxBaseTask* tmp = &basetask;
	PX_UNUSED(tmp);
	PX_UNUSED(threadId);

	/* This does not need a lock! */
#if PX_SUPPORT_VISUAL_DEBUGGER
	//PX_COMPILE_TIME_ASSERT(sizeof(PxProfileEventId::mEventId) == sizeof(PxBaseTask::mEventID));
	if (mProfileZone)
	{
		PxProfileEventSender* sender = mProfileZone;
		if (sender)
		{
			if (basetask.mEventID == 0xFFFF)
			{
				basetask.mEventID = mProfileZone->getEventIdForName(basetask.getName());
			}

			PxProfileEventId id(basetask.mEventID, true);
			sender->startEvent(id, static_cast<PxU64>(reinterpret_cast <size_t>(this)), threadId);
		}
	}
#endif
}

void PxTaskMgr::emitStopEvent(PxBaseTask& basetask, PxU32 threadId)
{
	PxBaseTask* tmp = &basetask;
	PX_UNUSED(tmp);
	PX_UNUSED(threadId);

	/* This does not need a lock! */
#if PX_SUPPORT_VISUAL_DEBUGGER
	//PX_COMPILE_TIME_ASSERT(sizeof(PxProfileEventId::mEventId) == sizeof(PxBaseTask::mEventID));
	if (mProfileZone)
	{
		PxProfileEventSender* sender = mProfileZone;
		if (sender)
		{
			if (basetask.mEventID == 0xFFFF)
			{
				basetask.mEventID = mProfileZone->getEventIdForName(basetask.getName());
			}

			PxProfileEventId id(basetask.mEventID, true);
			if (basetask.mProfileStat)
			{
				sender->eventValue(id, static_cast<PxU64>(reinterpret_cast <size_t>(this)), basetask.mProfileStat);
			}
			sender->stopEvent(id, static_cast<PxU64>(reinterpret_cast <size_t>(this)), threadId);
		}
	}
#endif
}

/*
 * Called by the owner (Scene) at the start of every frame, before
 * asking for tasks to be submitted.
 */
void PxTaskMgr::resetDependencies()
{
#if DOT_LOG
	if( logfile )
	{
		debugGraphEnd();
	}
	if( framenum++ == LOG_FRAME_NUM )
	{
		debugGraphBegin(logFilename);
	}
#endif

	PX_ASSERT( !mPendingTasks ); // only valid if you don't resubmit named tasks, this is true for the SDK
    PX_ASSERT( mCpuDispatcher );
    mTaskTable.clear();
    mDepTable.clear();
    mName2IDmap.clear();
    mPendingTasks = 0;
}

/* 
 * Called by the owner (Scene) to start simulating the task graph.
 * Dispatch all tasks with refCount == 1
 */
void PxTaskMgr::startSimulation()
{
    PX_ASSERT( mCpuDispatcher );

	if( mGpuDispatcher )
	{
		mGpuDispatcher->startSimulation();
	}

	/* Handle empty task graph */
	if( mPendingTasks == 0 )
    {

		return;
    }

    bool gpuDispatch = false;
    for( PxTaskID i = 0 ; i < mTaskTable.size() ; i++ )
    {
		if(	mTaskTable[ i ].mType == PxTaskType::TT_COMPLETED )
		{
			continue;
		}
		if( !shdfnd::atomicDecrement( &mTaskTable[ i ].mRefCount ) )
		{
			mStartDispatch.pushBack(i);
		}
	}
	for( PxU32 i=0; i<mStartDispatch.size(); ++i)
	{
		gpuDispatch |= dispatchTask( mStartDispatch[i], gpuDispatch );
	}
	//mStartDispatch.resize(0);
	mStartDispatch.forceSize_Unsafe(0);

    if( mGpuDispatcher && gpuDispatch )
	{
        mGpuDispatcher->finishGroup();
	}
}

void PxTaskMgr::stopSimulation()
{
	if( mGpuDispatcher )
	{
		mGpuDispatcher->stopSimulation();
	}
}

PxTaskID PxTaskMgr::getNamedTask( const char *name )
{
	const PxTaskNameToIDMap::Entry *ret;
    {
        LOCK();
		ret = mName2IDmap.find( name );
    }
    if( ret )
	{
        return ret->second;
	}
    else
	{
        // create named entry in task table, without a task
        return submitNamedTask( NULL, name, PxTaskType::TT_NOT_PRESENT );
}
}

PxTask* PxTaskMgr::getTaskFromID( PxTaskID id )
{
	LOCK(); // todo: reader lock necessary?
	return mTaskTable[ id ].mTask;
}


/* If called at runtime, must be thread-safe */
PxTaskID PxTaskMgr::submitNamedTask( PxTask *task, const char *name, PxTaskType::Enum type )
{
    if( task )
    {
        task->mTm = this;
        task->submitted();
    }

    LOCK();

	const PxTaskNameToIDMap::Entry *ret = mName2IDmap.find( name );
    if( ret )
    {
		PxTaskID prereg = ret->second;
		if( task )
		{
			/* name was registered for us by a dependent task */
			PX_ASSERT( !mTaskTable[ prereg ].mTask );
			PX_ASSERT( mTaskTable[ prereg ].mType == PxTaskType::TT_NOT_PRESENT );
			mTaskTable[ prereg ].mTask = task;
			mTaskTable[ prereg ].mType = type;
			task->mTaskID = prereg;
		}
		return prereg;
    }
    else
    {
        shdfnd::atomicIncrement(&mPendingTasks);
        PxTaskID id = (PxTaskID) mTaskTable.size();
        mName2IDmap[ name ] = id;
        if( task )
		{
            task->mTaskID = id;
		}
        PxTaskTableRow r;
        r.mTask = task;
        r.mType = type;
#if DOT_LOG
		if( logfile )
		{
			if( type == PxTaskType::TT_GPU )
			{
				fprintf(logfile, "{node [shape=box,label=\"%s\"] t%d0};\n", task->getName(), PxUnionCast<PxU32>(task));
			}
			else if (type == PxTaskType::TT_NOT_PRESENT)
			{
				fprintf(logfile, "{node [shape=invhouse,label=\"%s\"] t%d0};\n", name, PxUnionCast<PxU32>(task));
			}
			else
			{
				fprintf(logfile, "{node [label=\"%s\"] t%d0};\n", task->getName(), PxUnionCast<PxU32>(task));
			}
		}
#endif
		mTaskTable.pushBack(r);
        return id;
    }
}

/*
 * Add an unnamed task to the task table
 */
PxTaskID PxTaskMgr::submitUnnamedTask( PxTask& task, PxTaskType::Enum type )
{
    shdfnd::atomicIncrement(&mPendingTasks);

	task.mTm = this;
    task.submitted();
    
	LOCK();
    task.mTaskID = (PxTaskID) mTaskTable.size();
    PxTaskTableRow r;
    r.mTask = &task;
    r.mType = type;
#if DOT_LOG
	if( logfile )
	{
		if( type == PxTaskType::TT_GPU )
		{
			fprintf(logfile, "{node [shape=box,label=\"%s\"] t%d0};\n", task.getName(), PxUnionCast<PxU32>(&task));
		}
		else
		{
			fprintf(logfile, "{node [label=\"%s\"] t%d0};\n", task.getName(), PxUnionCast<PxU32>(&task));
		}
	}
#endif
    mTaskTable.pushBack(r);
    return task.mTaskID;
}


/* Called by worker threads (or cooperating application threads) when a
 * PxTask has completed.  Propogate depdenencies, decrementing all
 * referenced tasks' refCounts.  If any of those reach zero, activate
 * those tasks.
 */
void PxTaskMgr::taskCompleted( PxTask& task )
{
    LOCK();
    if( resolveRow( task.mTaskID, false ) )
	{
        mGpuDispatcher->finishGroup();
	}
}

/* ================== Private Functions ======================= */

/*
 * Add a dependency to force 'task' to complete before the
 * referenced 'taskID' is allowed to be dispatched.
 */
void PxTaskMgr::finishBefore( PxTask& task, PxTaskID taskID )
{
    LOCK();
	PX_ASSERT( mTaskTable[ taskID ].mType != PxTaskType::TT_COMPLETED );

#if DOT_LOG
	if( logfile )
	{
		fprintf(logfile, "t%d0->t%d0;\n", PxUnionCast<PxU32>(&task), PxUnionCast<PxU32>(mTaskTable[ taskID ].mTask));
	}
#endif

    mTaskTable[ task.mTaskID ].addDependency( mDepTable, taskID );
	shdfnd::atomicIncrement( &mTaskTable[ taskID ].mRefCount );
}


/*
 * Add a dependency to force 'task' to wait for the referenced 'taskID'
 * to complete before it is allowed to be dispatched.
 */
void PxTaskMgr::startAfter( PxTask& task, PxTaskID taskID )
{
    LOCK();
	PX_ASSERT( mTaskTable[ taskID ].mType != PxTaskType::TT_COMPLETED );

#if DOT_LOG
	if( logfile )
	{
		fprintf(logfile, "t%d0->t%d0;\n",	PxUnionCast<PxU32>(mTaskTable[ taskID ].mTask), PxUnionCast<PxU32>(&task));
	}
#endif

    mTaskTable[ taskID ].addDependency( mDepTable, task.mTaskID );
	shdfnd::atomicIncrement( &mTaskTable[ task.mTaskID ].mRefCount );
}


void PxTaskMgr::addReference( PxTaskID taskID )
{
    LOCK();
    shdfnd::atomicIncrement( &mTaskTable[ taskID ].mRefCount );
}

/*
 * Remove one reference count from a task.  Intended for use by the
 * GPU dispatcher, to remove reference counts when CUDA events are
 * resolved.  Must be done here to make it thread safe.
 */
void PxTaskMgr::decrReference( PxTaskID taskID )
{
    LOCK();

#if DOT_LOG	
	debugGraphEdge(currentTask, 0, 0, mTaskTable[ taskID ].mTask, 0, 0, 1);
#endif

    if( !shdfnd::atomicDecrement( &mTaskTable[ taskID ].mRefCount ) )
    {
        if( dispatchTask( taskID, false ) )
        {
            mGpuDispatcher->finishGroup();
        }
    }
}

PxI32 PxTaskMgr::getReference(PxTaskID taskID) const
{
	return mTaskTable[ taskID ].mRefCount;
}

/*
 * A task has completed, decrement all dependencies and submit tasks
 * that are ready to run.  Signal simulation end if ther are no more
 * pending tasks.
 */
bool PxTaskMgr::resolveRow( PxTaskID taskID, bool gpuGroupStart )
{
    int depRow = mTaskTable[ taskID ].mStartDep;

	PxU32 streamIndex = 0;
	bool syncRequired = false;
	if( mTaskTable[ taskID ].mTask )
	{
		streamIndex = mTaskTable[ taskID ].mTask->mStreamIndex;
	}

    while( depRow != EOL )
    {
        PxTaskDepTableRow& row = mDepTable[ (PxU32)depRow ];
        PxTaskTableRow& dtt = mTaskTable[ row.mTaskID ];

		// pass stream index to (up to one) dependent GPU task
		if( dtt.mTask && dtt.mType == PxTaskType::TT_GPU && streamIndex )
		{
			if( dtt.mTask->mStreamIndex )
			{
				PX_ASSERT( dtt.mTask->mStreamIndex != streamIndex );
				dtt.mTask->mPreSyncRequired = true;
			}
			else if( syncRequired )
			{
				dtt.mTask->mPreSyncRequired = true;
			}
			else
			{
				dtt.mTask->mStreamIndex = streamIndex;
				/* only one forward task gets to use this stream */
				syncRequired = true;
			}
		}

        if( !shdfnd::atomicDecrement( &dtt.mRefCount ) )
		{
			gpuGroupStart |= dispatchTask( row.mTaskID, gpuGroupStart );
		}

        depRow = row.mNextDep;
    }

    shdfnd::atomicDecrement( &mPendingTasks );
    return gpuGroupStart;
}

/*
 * Submit a ready task to its appropriate dispatcher.
 */
bool PxTaskMgr::dispatchTask( PxTaskID taskID, bool gpuGroupStart )
{
	LOCK(); // todo: reader lock necessary?
    PxTaskTableRow& tt = mTaskTable[ taskID ];

    // prevent re-submission
    if( tt.mType == PxTaskType::TT_COMPLETED )
    {
		shdfnd::getFoundation().error(PX_WARN, "PxTask dispatched twice");
        return false;
    }

    switch ( tt.mType )
    {
    case PxTaskType::TT_CPU:
        mCpuDispatcher->submitTask( *tt.mTask );
        break;

	case PxTaskType::TT_GPU:
#if defined(PX_WINDOWS)
        if( mGpuDispatcher )
        {
			if( !gpuGroupStart )
			{
				mGpuDispatcher->startGroup();
			}
			mGpuDispatcher->submitTask( *tt.mTask );
			gpuGroupStart = true;
		}
		else
#endif
		{
			shdfnd::getFoundation().error(PX_WARN, "No GPU dispatcher");
		}
		break;

    case PxTaskType::TT_NOT_PRESENT:
		/* No task registered with this taskID, resolve its dependencies */
		PX_ASSERT(!tt.mTask);
		//shdfnd::getFoundation().error(PX_INFO, "unregistered task resolved");
        gpuGroupStart |= resolveRow( taskID, gpuGroupStart );
		break;
	case PxTaskType::TT_COMPLETED:
    default:
        shdfnd::getFoundation().error(PX_WARN, "Unknown task type");
        gpuGroupStart |= resolveRow( taskID, gpuGroupStart );
        break;
    }

    tt.mType = PxTaskType::TT_COMPLETED;
    return gpuGroupStart;
}

} // end physx namespace
