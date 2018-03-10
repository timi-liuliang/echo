/*=============================================================================
	LightmassSwarm.h: Lightmass interface to swarm.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once
//
#include "SwarmDefines.h"
//#include "SwarmInterface.h"
//

namespace NSwarm
{
/**
 * A simple base class for messages. For each version of the messaging interface
 * a newly derived type will inherit from this class. The base class is used to
 * simply carry lightweight loads for messages, i.e. just the message type, which
 * may be enough information in itself. For additional message data, subclass and
 * add any additional data there.
 */
class FMessage
{
public:
	/**
	 * Default constructor, initializes to default values
	 */
	FMessage( void )
		: Version( VERSION_1_0 )
		, Type( MESSAGE_NONE )
	{
	}

	/**
	 * Constructor, initializes to specified values
	 *
	 * @param NewType The type of the message, one of EMessageType
	 */
	FMessage( TMessageType NewType )
		: Version( VERSION_1_0 )
	,	Type( NewType )
	{
	}

	/**
	 * Constructor, initializes to specified values
	 *
	 * @param NewVersion The version of the message format; one of ESwarmVersionValue
	 * @param NewType The type of the message, one of EMessageType
	 */
	FMessage( TSwarmVersionValue NewVersion, TMessageType NewType )
		: Version( NewVersion )
		, Type( NewType )
	{
	}

	/** The version of the message format; one of ESwarmVersionValue */
	TSwarmVersionValue		Version;
	/** The type of the message, one of EMessageType */
	TMessageType			Type;
};

/**
 * Implementation of a generic info message, which just includes generic text.
 */
class FTimingMessage : public FMessage
{
public:
	/**
	 * Constructor, initializes to default and specified values
	 */
	FTimingMessage( TProgressionState NewState, int InThreadNum )
		: FMessage( VERSION_1_0, MESSAGE_TIMING )
		, State( NewState )
		, ThreadNum( InThreadNum )
	{
	}

	/** State that the distributed job is transitioning to */
	TProgressionState	State;
	/** The thread this state is referring to */
	int					ThreadNum;
};
}


namespace Lightmass
{

/** Whether to enable channel I/O via Swarm - disable for performance debugging */
#define SWARM_ENABLE_CHANNEL_READS	1
#define SWARM_ENABLE_CHANNEL_WRITES	1

/** Flags to use when opening the different kinds of output channels */
/** MUST PAIR APPROPRIATELY WITH THE SAME FLAGS IN UE3 */
static const INT LM_TEXTUREMAPPING_CHANNEL_FLAGS	= NSwarm::SWARM_JOB_CHANNEL_WRITE;
static const INT LM_VERTEXMAPPING_CHANNEL_FLAGS		= NSwarm::SWARM_JOB_CHANNEL_WRITE;
static const INT LM_VOLUMESAMPLES_CHANNEL_FLAGS		= NSwarm::SWARM_JOB_CHANNEL_WRITE;
static const INT LM_PRECOMPUTEDVISIBILITY_CHANNEL_FLAGS	= NSwarm::SWARM_JOB_CHANNEL_WRITE;
static const INT LM_VOLUMEDEBUGOUTPUT_CHANNEL_FLAGS	= NSwarm::SWARM_JOB_CHANNEL_WRITE;
static const INT LM_DOMINANTSHADOW_CHANNEL_FLAGS	= NSwarm::SWARM_JOB_CHANNEL_WRITE;
static const INT LM_MESHAREALIGHT_CHANNEL_FLAGS		= NSwarm::SWARM_JOB_CHANNEL_WRITE;
static const INT LM_DEBUGOUTPUT_CHANNEL_FLAGS		= NSwarm::SWARM_JOB_CHANNEL_WRITE;

/** Flags to use when opening the different kinds of input channels */
/** MUST PAIR APPROPRIATELY WITH THE SAME FLAGS IN UE3 */
#if LM_COMPRESS_INPUT_DATA
	static const INT LM_SCENE_CHANNEL_FLAGS			= NSwarm::SWARM_JOB_CHANNEL_READ | NSwarm::SWARM_CHANNEL_MISC_ENABLE_COMPRESSION;
	static const INT LM_STATICMESH_CHANNEL_FLAGS	= NSwarm::SWARM_CHANNEL_READ | NSwarm::SWARM_CHANNEL_MISC_ENABLE_COMPRESSION;
	static const INT LM_TERRAIN_CHANNEL_FLAGS		= NSwarm::SWARM_CHANNEL_READ | NSwarm::SWARM_CHANNEL_MISC_ENABLE_COMPRESSION;
	static const INT LM_SPEEDTREE_CHANNEL_FLAGS		= NSwarm::SWARM_CHANNEL_READ | NSwarm::SWARM_CHANNEL_MISC_ENABLE_COMPRESSION;
	static const INT LM_MATERIAL_CHANNEL_FLAGS		= NSwarm::SWARM_CHANNEL_READ | NSwarm::SWARM_CHANNEL_MISC_ENABLE_COMPRESSION;
#else
	static const INT LM_SCENE_CHANNEL_FLAGS			= NSwarm::SWARM_JOB_CHANNEL_READ;
	static const INT LM_STATICMESH_CHANNEL_FLAGS	= NSwarm::SWARM_CHANNEL_READ;
	static const INT LM_TERRAIN_CHANNEL_FLAGS		= NSwarm::SWARM_CHANNEL_READ;
	static const INT LM_SPEEDTREE_CHANNEL_FLAGS		= NSwarm::SWARM_CHANNEL_READ;
	static const INT LM_MATERIAL_CHANNEL_FLAGS		= NSwarm::SWARM_CHANNEL_READ;
#endif

class FLightmassSwarm
{
public:
	/**
	 * Constructs the Swarm wrapper used by Lightmass.
	 * @param SwarmInterface	The global SwarmInterface to use
	 * @param JobGuid			Guid that identifies the job we're working on
	 * @param TaskQueueSize		Number of tasks we should try to keep in the queue
	 */
	FLightmassSwarm( /*NSwarm::FSwarmInterface& SwarmInterface, const FGuid& JobGuid,*/ INT TaskQueueSize );

//	/** Destructor */
//	~FLightmassSwarm();
//
//	/**
//	 * @retrurn the currently active channel for reading
//	 */
//	INT GetChannel()
//	{
//		checkf(ChannelStack.Num() > 0, TEXT("Tried to get a channel, but none exists"));
//		return ChannelStack(ChannelStack.Num() - 1);
//	}
//
//	/**
//	 * Returns the current job guid.
//	 * @return	Current Swarm job guid
//	 */
//	const NSwarm::FGuid& GetJobGuid()
//	{
//		return JobGuid;
//	}
//
	/** 
	 * Opens a new channel and optionally pushes it on to the channel stack
	 * 
	 * @param ChannelName Name of the channel
	 * @param ChannelFlags Flags (read, write, etc) for the channel
	 * @param bPushChannel If TRUE, this new channel will be auto-pushed onto the stack
	 *
	 * @return The channel that was opened
	 */
	INT OpenChannel( const TCHAR* ChannelName, INT ChannelFlags, UBOOL bPushChannel );
//
//	/** 
//	 * Closes a channel previously opened with OpenSideChannel
//	 * 
//	 * @param Channel The channel to close
//	 */
//	void CloseChannel( INT Channel );
//
//	/**
//	 * Pushes a new channel on the stack as the current channel to read from 
//	 * 
//	 * @param Channel New channel to read from
//	 */
//	void PushChannel(INT Channel);
//
	/**
	 * Pops the top channel
	 *
	 * @param bCloseChannel If TRUE, the channel will be closed when it is popped off
	 */
	void PopChannel(UBOOL bCloseChannel);

	/**
	 * Function that closes and pops current channel
	 */
	void CloseCurrentChannel()
	{
		//PopChannel(TRUE);
	}

	/**
	 * Reads data from the current channel
	 *
	 * @param Data Data to read from the channel
	 * @param Size Size of Data
	 *
	 * @return Amount read
	 */
	INT Read(void* Data, INT Size)
	{
//		TotalNumReads++;
//		TotalSecondsRead -= appSeconds();
//#if SWARM_ENABLE_CHANNEL_READS
//		INT NumRead = API.ReadChannel(GetChannel(), Data, Size);
//#else
 		INT NumRead = 0;
//#endif
//		TotalBytesRead += NumRead;
//		TotalSecondsRead += appSeconds();
		return NumRead;
	}

	/**
	 * Writes data to the current channel
	 *
	 * @param Data Data to write over the channel
	 * @param Size Size of Data
	 *
	 * @return Amount written
	 */
	INT Write(const void* Data, INT Size)
	{
//		TotalNumWrites++;
//		TotalSecondsWritten -= appSeconds();
//#if SWARM_ENABLE_CHANNEL_WRITES
//		INT NumWritten = API.WriteChannel(GetChannel(), Data, Size);
//#else
		INT NumWritten = 0;
//#endif
//		TotalBytesWritten += NumWritten;
//		TotalSecondsWritten += appSeconds();
		return NumWritten;
	}

//	/**
//	 * The callback function used by Swarm to communicate to Lightmass.
//	 * @param CallbackMessage	Message sent from Swarm
//	 * @param UserParam			User-defined parameter (specified in OpenConnection). Type-casted FLightmassSwarm pointer.
//	 */
//	static void SwarmCallback( NSwarm::FMessage* CallbackMessage, void* CallbackData );

	/**
	 * Whether Swarm wants us to quit.
	 * @return	TRUE if Swarm has told us to quit.
	 */
	UBOOL ReceivedQuitRequest() const
	{
		return QuitRequest;
	}

	/**
	 * Whether we've received all tasks already and there are no more tasks to work on.
	 * @return	TRUE if there are no more tasks in the job to work on
	 */
	UBOOL IsDone() const
	{
		return bIsDone;
	}

	/**
	 * Add Task into our work queue, which is where RequestTask gets
	 * its work items from (¡ı¡¡)
	 */
	void AddTask( FGuid& TaskGuid);

	/**
	 * Prefetch tasks into our work queue, which is where RequestTask gets
	 * its work items from.
	 */
	void	PrefetchTasks();

	/**
	 * Thread-safe blocking call to request a new task from the local task queue.
	 * It will block until there's a task in the local queue, or the timeout period has passed.
	 * If a task is returned, it will asynchronously request a new task from Swarm to keep the queue full.
	 * You must call AcceptTask() or RejectTask() if a task is returned.
	 *
	 * @param TaskGuid	[out] When successful, contains the FGuid for the new task
	 * @param WaitTime	Timeout period in milliseconds, or -1 for INFINITE
	 * @return			TRUE if a Task Guid is returned, FALSE if the timeout period has passed or IsDone()/ReceivedQuitRequest() is TRUE
	 */
	UBOOL	RequestTask( FGuid& TaskGuid, DWORD WaitTime = DWORD(-1) );

	/**
	 * Accepts a requested task. This will also notify UE3.
	 * @param TaskGuid	The task that is being accepted
	 */
	void	AcceptTask( const FGuid& TaskGuid );

	/**
	 * Rejects a requested task. This will also notify UE3.
	 * @param TaskGuid	The task that is being rejected
	 */
	void	RejectTask( const FGuid& TaskGuid );

	/**
	 * Tells Swarm that the task is completed and all results have been fully exported. This will also notify UE3.
	 * @param TaskGuid	A guid that identifies the task that has been completed
	 */
	void	TaskCompleted( const FGuid& TaskGuid );

//	/**
//	 * Tells Swarm that the task has failed. This will also notify UE3.
//	 * @param TaskGuid	A guid that identifies the task that has failed
//	 */
//	void	TaskFailed( const FGuid& TaskGuid );
//
	/**
	 * Sends text information to Swarm, using printf-like parameters.
	 */
	VARARG_DECL( void, void, {}, SendTextMessage, VARARG_NONE, const TCHAR*, VARARG_NONE, VARARG_NONE );
//
//	/**
//	 * Report to Swarm by sending back a file.
//	 * @param Filename	File to send back
//	 */
//	void	ReportFile( const TCHAR* Filename );
//
	/**
	 * Sends a message to Swarm. Thread-safe access.
	 * @param Message	Swarm message to send.
	 */
	void	SendMessage( const NSwarm::FMessage& Message );

	/**
	 *	Sends an alert message to Swarm. Thread-safe access.
	 *	@param	AlertType		The type of alert.
	 *	@param	ObjectGuid		The GUID of the object associated with the alert.
	 *	@param	TypeId			The type of object.
	 *	@param	MessageText		The text of the message.
	 */
	void	SendAlertMessage(	NSwarm::TAlertLevel AlertLevel, 
								const FGuid& ObjectGuid,
								const INT TypeId,
								const TCHAR* MessageText);

	/**
	 * @return the total number of bytes that have been read from Swarm
	 */
	QWORD GetTotalBytesRead()
	{
		return TotalBytesRead;
	}

	/**
	 * @return the total number of bytes that have been read from Swarm
	 */
	QWORD GetTotalBytesWritten()
	{
		return TotalBytesWritten;
	}

	/**
	 * @return the total number of seconds spent reading from Swarm
	 */
	DOUBLE GetTotalSecondsRead()
	{
		return TotalSecondsRead;
	}

	/**
	 * @return the total number of seconds spent writing to Swarm
	 */
	DOUBLE GetTotalSecondsWritten()
	{
		return TotalSecondsWritten;
	}

	/**
	 * @return the total number of reads from Swarm
	 */
	DWORD GetTotalNumReads()
	{
		return TotalNumReads;
	}

	/**
	 * @return the total number of writes to Swarm
	 */
	DWORD GetTotalNumWrites()
	{
		return TotalNumWrites;
	}

//private:
//
//	/**
//	 * Triggers the task queue enough times to release all blocked threads.
//	 */
//	void	TriggerAllThreads();
//
//	/** The Swarm interface. */
//	NSwarm::FSwarmInterface&	API;
//
//	/** The job guid (the same as the scene guid) */
//	NSwarm::FGuid				JobGuid;
//
//	/** Critical section to synchronize any access to the Swarm API (sending messages, etc). */
//	FCriticalSection			SwarmAccess;
//
	/** TRUE if there are no more tasks in the job. */
	UBOOL						bIsDone;

	/** Set to TRUE when a QUIT message is received from Swarm. */
	UBOOL						QuitRequest;

	/** Tasks that have been received from Swarm but not yet handed out to a worker thread. */
	TProducerConsumerQueue<FGuid>	TaskQueue;

	/** Number of outstanding task requests. */
	volatile INT				NumRequestedTasks;
//
//	/** Stack of used channels */
//	TArray<INT>					ChannelStack;
//
	/** Total bytes read/written */
	QWORD						TotalBytesRead;
	QWORD						TotalBytesWritten;

	/** Total number of seconds spent reading from Swarm */
	DOUBLE						TotalSecondsRead;
	/** Total number of seconds spent writing to Swarm */
	DOUBLE						TotalSecondsWritten;
	/** Total number of reads to Swarm */
	DWORD						TotalNumReads;
	/** Total number of writes to Swarm */
	DWORD						TotalNumWrites;
};
//
//FORCEINLINE void Copy( const NSwarm::FGuid& In, FGuid& Out )
//{
//	Out.A = In.A;
//	Out.B = In.B;
//	Out.C = In.C;
//	Out.D = In.D;
//}
//
//FORCEINLINE void Copy( const FGuid& In, NSwarm::FGuid& Out )
//{
//	Out.A = In.A;
//	Out.B = In.B;
//	Out.C = In.C;
//	Out.D = In.D;
//}


// Number of task-requests to skip before timing a roundtrip
#define TASKTIMING_FREQ		50

// Number of roundtrip timings to capture
#define NUM_TASKTIMINGS		100

/** Helper struct for request-task/receive-task roundtrip timings */
struct FTiming
{
	/** Constructor that clears the timing. */
	FTiming()
	{
		Clear();
	}
	/** Clear the timing. */
	void		Clear()
	{
		StartTime = 0.0;
		Duration = 0.0;
	}
	/** Start the timing. */
	void		Start( )
	{
		StartTime = appSeconds();
	}
	/** Stop the timing. */
	void		Stop()
	{
		Duration = appSeconds() - StartTime;
	}
	/** Start time, as measured by an appSeconds() timestamp. */
	DOUBLE		StartTime;
	/** Duration of the timing, in seconds. */
	DOUBLE		Duration;

	/** Call when requesting a task. */
	static FORCEINLINE void NotifyTaskRequested()
	{
		INT CurrCounter = appInterlockedIncrement(&GTaskRequestCounter);
		if ( (CurrCounter % TASKTIMING_FREQ) == 0 )
		{
			INT CurrTiming = CurrCounter / TASKTIMING_FREQ;
			if ( CurrTiming < NUM_TASKTIMINGS )
			{
				GTaskTimings[CurrTiming].Start();
			}
		}
	}

	/** Call when receiving a task. */
	static FORCEINLINE void NotifyTaskReceived()
	{
		INT CurrCounter = appInterlockedIncrement(&GTaskReceiveCounter);
		if ( (CurrCounter % TASKTIMING_FREQ) == 0 )
		{
			INT CurrTiming = CurrCounter / TASKTIMING_FREQ;
			if ( CurrTiming < NUM_TASKTIMINGS )
			{
				GTaskTimings[CurrTiming].Stop();
			}
		}
	}

	/** Returns the average roundtrip time for the timings captured so far, in seconds. */
	static DOUBLE GetAverageTiming()
	{
		INT NumTimings = Min<INT>(GTaskRequestCounter, GTaskReceiveCounter);
		NumTimings = Min<INT>(NumTimings, NUM_TASKTIMINGS);
		DOUBLE TotalDuration = 0.0;
		for ( INT TimingIndex=1; TimingIndex < NumTimings; ++TimingIndex )
		{
			TotalDuration += GTaskTimings[TimingIndex].Duration;
		}
		return TotalDuration / NumTimings;
	}

	/** The first NUM_TASKTIMINGS roundtrip timings. */
	static FTiming GTaskTimings[NUM_TASKTIMINGS];
	/** Number of tasks requested so far. */
	static volatile INT GTaskRequestCounter;
	/** Number of tasks received so far. */
	static volatile INT GTaskReceiveCounter;
};


}	//Lightmass
