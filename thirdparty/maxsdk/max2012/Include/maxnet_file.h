//-----------------------------------------------------------------------------
// ------------------------
// File ....: maxnet_file.h
// ------------------------
// Author...: Gus J Grubba
// Date ....: March 2000
// O.S. ....: Windows 2000
//
// *** Obsolete ***
// 
//-----------------------------------------------------------------------------

#pragma once
#include <WTypes.h>
#include "strbasic.h"
#include "maxheap.h"
#include "network\MaxNetExport.h"
// forward declarations
struct AlertData;
class CJobText;
struct SceneInfo;
struct Job;
struct NetworkStatus;
struct ConfigurationBlock;
struct WeekSchedule;
struct JobServer;
struct JOBFRAMES;

#define SRVTOMAX				_M("ToMax.txt")
#define APPTOSRV				_M("ToServer.txt")
#define JOBDESCRP				_M("Job.txt")

//---------------------------------------------------------
//-- Messages from MAX to Server

#define	FROMMAX_READY			_M("ready")
#define	FROMMAX_FRAME_COMPLETE	_M("complete")
#define	FROMMAX_FRAME_ERROR		_M("error")
#define	FROMMAX_BUSY			_M("busy")
#define	FROMMAX_GOING_DOWN		_M("end")

//---------------------------------------------------------
//-- Messages from Server to Max

#define	FROMSRV_NEWFRAME		_M("frame")
#define	FROMSRV_CANCEL			_M("cancel")

//-----------------------------------------------------------------------------
//-- Sections
//

#ifdef	_MXNTF_DEFINE_

#define MXNTF_SERVERINFO			_M("ServerInfo")
#define MXNTF_VERSION				_M("Version")
#define MXNTF_SRV_PID				_M("ServerPID")
#define MXNTF_SRV_TOTAL_FRAMES		_M("TotalFrames")
#define MXNTF_SRV_TOTAL_TIME		_M("TotalTime")
#define MXNTF_SRV_INDEX				_M("Index")
#define MXNTF_CONFIG_BLOCK			_M("Configuration")
#define MXNTF_TOTAL_MEMORY			_M("TotalMemory")
#define MXNTF_TOTAL_CPU				_M("NumberCpus")
#define MXNTF_OS_MAJOR				_M("OSMajor")
#define MXNTF_OS_MINOR				_M("OSMinor")
#define MXNTF_OS_BUILD				_M("OSBuild")
#define MXNTF_OS_PLATFORM_ID		_M("OSPlatformID")
#define MXNTF_OS_CSDV				_M("OSCSDV")
#define MXNTF_TEMP_DIR				_M("TempDir")
#define MXNTF_WORK_DISK				_M("WorkDisk")
#define MXNTF_AVAILABLE_DISKS		_M("AvailableDisks")
#define MXNTF_DISK_SPACE			_M("DiskSpace")
#define MXNTF_MAC					_M("MAC")
#define MXNTF_NETSTATUS				_M("NetworkStatus")
#define MXNTF_DROPPED_PACKETS		_M("DroppedPackets")
#define MXNTF_BAD_PACKETS			_M("BadPackets")
#define MXNTF_TCP_REQUESTS			_M("TCPRequests")
#define MXNTF_UDP_REQUESTS			_M("UDPRequests")
#define MXNTF_BOOT_TIME				_M("BootTime")
#define MXNTF_JOB					_M("Job")
#define MXNTF_FLAGS					_M("Flags")
#define MXNTF_HJOB					_M("HJOB")
#define MXNTF_JOB_NAME				_M("JobName")
#define MXNTF_JOB_FIRST_FRAME		_M("FirstFrame")
#define MXNTF_JOB_LAST_FRAME		_M("LastFrame")
#define MXNTF_STEP					_M("Step")
#define MXNTF_JOB_SUBMISSION		_M("Submission")
#define MXNTF_JOB_START				_M("Start")
#define MXNTF_JOB_END				_M("End")
#define MXNTF_JOB_FRAMES_COMPLETE	_M("FramesComplete")
#define MXNTF_JOB_SERVER_COUNT		_M("ServerCount")
#define MXNTF_JOB_PRIORITY			_M("JobPriority")
#define MXNTF_JOB_ARCHSIZE			_M("ArchivedSize")
#define MXNTF_JOB_UNARCHSIZE		_M("UnarchivedSize")
#define MXNTF_ALERTS				_M("Alerts")
#define MXNTF_NOTIFICATIONS			_M("Notifications")
#define MXNTF_ENABLED				_M("Enabled")
#define MXNTF_USER_NAME				_M("UserName")
#define MXNTF_COMPUTER_NAME			_M("ComputerName")
#define MXNTF_JOB_TEXT				_M("JobText")
#define MXNTF_COUNT					_M("Count")
#define MXNTF_JOB_TEXTITEM			_M("JobTextItem")
#define MXNTF_JOB_TEXT_TEXT			_M("JobTextText")
#define MXNTF_JOB_TEXT_OUTPUT		_M("JobTextOutput")
#define MXNTF_JOB_TEXT_OUTPUT_FILE	_M("JobTextOutputData")
#define MXNTF_R_ELEMENT				_M("RenderElement")
#define MXNTF_RE_ENABLED			_M("RenderElementEnabled")
#define MXNTF_RE_ACTIVE				_M("RenderElementsActive")
#define MXNTF_RE_FLT_ENABLED		_M("RenderElementFilterEnabled")
#define MXNTF_RE_ATM_ENABLED		_M("RenderElementAtmEnabled")
#define MXNTF_RE_SHD_ENABLED		_M("RenderElementShadowEnabled")
#define MXNTF_RE_NAME				_M("RenderElementName")
#define MXNTF_RE_OUTPUT				_M("RenderElementOutput")
#define MXNTF_JOB_FRAMES			_M("JobFrames")
#define MXNTF_JOB_FRAME_NO			_M("Frame")
#define MXNTF_JOB_SERVERS			_M("JobServers")
#define MXNTF_JOB_SERVER_NO			_M("Server")
#define MXNTF_WIDTH					_M("Width")
#define MXNTF_HEIGHT				_M("Height")
#define MXNTF_PIXELASPECT			_M("PixelAspect")
#define MXNTF_INIT					_M("Init")
#define MXNTF_GAMMACORRECTION		_M("UseGammaCorrection")
#define MXNTF_INPUTGAMMA			_M("InputGamma")
#define MXNTF_OUTPUTGAMMA			_M("OutputGamma")
#define MXNTF_CAMERA				_M("Camera")
#define MXNTF_SCENE_INFO			_M("SceneInfo")
#define MXNTF_NO_OBJECTS			_M("NumberOfObjects")
#define MXNTF_NO_FACES				_M("NumberOfFaces")
#define MXNTF_NO_LIGHTS				_M("NumberOfLights")
#define MXNTF_SCENE_FLAGS			_M("SceneFlags")
#define MXNTF_SCENE_START			_M("SceneStart")
#define MXNTF_SCENE_END				_M("SceneEnd")
#define MXNTF_JOB_STATE				_M("JobState")
#define MXNTF_NO_JOBS				_M("NumberOfJobs")
#define MXNTF_JOB_STATE_N			_M("Job")
#define MXNTF_INI_GENERAL			_M("General")
#define MXNTF_INI_MGRPORT			_M("ManagerPort")
#define MXNTF_INI_SRVPORT			_M("ServerPort")
#define MXNTF_INI_MAXBLOCK			_M("MaxBlockSize")
#define MXNTF_INI_NETMASK			_M("NetworkMask")
#define MXNTF_INI_MANAGER			_M("Manager")
#define MXNTF_INI_QM				_M("QueueManager")
#define MXNTF_INI_MGRAPP			_M("ManagerApp")
#define MXNTF_INI_SRVAPP			_M("ServerApp")
#define MXNTF_INI_INIT				_M("Init")
#define MXNTF_INI_WX				_M("WindowX")
#define MXNTF_INI_WY				_M("WindowY")
#define MXNTF_INI_WW				_M("WindowW")
#define MXNTF_INI_WH				_M("WindowH")
#define MXNTF_INI_SR				_M("SplitterRoll")
#define MXNTF_INI_SC				_M("SplitterColumn")
#define MXNTF_INI_CLI_VFB			_M("UseVFB")
#define MXNTF_INI_SRV_TABS			_M("ServerTabs")
#define MXNTF_INI_AUTO_CONN			_M("AutoConnect")
#define MXNTF_INI_AUTO_REFRESH		_M("AutoRefresh")
#define MXNTF_INI_RETRYFSERVERS		_M("RetryFailedServers")
#define MXNTF_INI_SRVCOOLOFF		_M("ServerCoolOffTime")
#define MXNTF_INI_CLIENTNOTIFYDELAY	_M("ClientNotificationDelay")
#define MXNTF_INI_RETRYCOUNT		_M("RetryCount")
#define MXNTF_INI_TIMEBRETRY		_M("TimeBetweenRetries")
#define MXNTF_INI_MAXLOADTIME		_M("MAXLoadTimeout")
#define MXNTF_INI_MAXRENDERTIME		_M("MAXRenderTimeout")
#define MXNTF_INI_MAXUNLOADTIME		_M("MAXUnloadTimeout")
#define MXNTF_INI_MAXCONCASSIGN		_M("MaxConcurrentAssignments")
#define MXNTF_INI_TIME				_M("Timers")
#define MXNTF_INI_ACKTIMEOUT		_M("AckTimeout")
#define MXNTF_INI_ACKRETRY			_M("AckRetries")
#define MXNTF_INI_FASTACKTIMEOUT	_M("FastAckTimeout")
#define MXNTF_INI_LOG				_M("Log")
#define MXNTF_INI_MAXSCRRENLN		_M("MaxLogScreenLines")
#define MXNTF_INI_ERRORSCREEN		_M("LogErrorsToScreen")
#define MXNTF_INI_WARNSCREEN		_M("LogWarningsToScreen")
#define MXNTF_INI_INFOSCREEN		_M("LogInfoToScreen")
#define MXNTF_INI_DEBUGSCREEN		_M("LogDebugToScreen")
#define MXNTF_INI_DEBUGEXSCREEN		_M("LogDebugExToScreen")
#define MXNTF_INI_ERRORFILE			_M("LogErrorsToFile")
#define MXNTF_INI_WARNFILE			_M("LogWarningsToFile")
#define MXNTF_INI_INFOFILE			_M("LogInfoToFile")
#define MXNTF_INI_DEBUGFILE			_M("LogDebugToFile")
#define MXNTF_INI_DEBUGEXFILE		_M("LogDebugExToFile")
#define MXNTF_INI_SERVER			_M("Server")
#define MXNTF_INI_AUTOSEARCH		_M("AutoSearchManager")
#define MXNTF_INI_MGRNAME			_M("ManagerName")
#define MXNTF_WEEK_SCHEDULE			_M("WeekSchedule")
#define MXNTF_ATT_PRIORITY			_M("AttendedPriority")
#define MXNTF_UTT_PRIORITY			_M("UnattendedPriority")
#define MXNTF_WEEK_DAY				_M("Day")
#define MXNTF_INI_CLIENT			_M("ClientSettings")
#define MXNTF_INI_ALERT_COMPLETION	_M("AlertCompletion")
#define MXNTF_INI_ALERT_FAILURE		_M("AlertFailure")
#define MXNTF_INI_ALERT_NTH			_M("AlertEveryNth")
#define MXNTF_INI_ALERT_PROGRESS	_M("AlertProgress")
#define MXNTF_INI_ALERTS			_M("Alerts")
#define MXNTF_INI_AUTOCONNECT		_M("AutoConnect")
#define MXNTF_INI_IGNORESCENE		_M("IgnoreScenePath")
#define MXNTF_INI_INCLUDEMAPS		_M("IncludeMaps")
#define MXNTF_INI_SKIPOUTPUT		_M("SkipOutputTest")
#define MXNTF_INI_USEALLSERVERS		_M("UseAllServers")

#endif

//-----------------------------------------------------------------------------
//-- File Class
//

class MAXNETEXPORT MaxNetFile : public MaxHeapOperators {

protected:

	MCHAR	line[1024];
	FILE*	f;
	bool	read;

public:

	MaxNetFile		( );
	virtual	~MaxNetFile		( );

	virtual bool	OpenRead	(const MCHAR* name);
	virtual bool	OpenWrite	(const MCHAR* name);
	virtual void	Close		( );					//-- Optional (Destructor calls it)
	void			Reset		( );					//-- fseek 0 seek_set

	MCHAR*	FirstBlank		(MCHAR* line);
	MCHAR*	FirstNonblank	(MCHAR* line);
	MCHAR*	NextValue		(MCHAR* ln);
	bool	LocateSection	(MCHAR* section);
	MCHAR*	LocateValue		(MCHAR* section, MCHAR* var, bool rescan = true);
	MCHAR*	GetLine			(MCHAR* destination = 0 );

	bool	ReadFilename	(MCHAR* ptr, MCHAR* filename);

	bool	Write			(MCHAR* line);
	bool	WriteHeader		(MCHAR* hdr);
	bool	WritePair		(MCHAR* var, float value);
	bool	WritePair		(MCHAR* var, int value);
	bool	WritePair		(MCHAR* var, bool value);
	bool	WritePair		(MCHAR* var, short value);
	bool	WritePair		(MCHAR* var, DWORD value);
	bool	WritePair		(MCHAR* var, size_t value);
	bool	WritePair		(MCHAR* var, MCHAR* value);
	bool	WritePair		(MCHAR* var, SYSTEMTIME* value);
	bool	WritePairHex	(MCHAR* var, DWORD value);

	bool	ReadPair		(MCHAR* section, MCHAR* var, float* value);
	bool	ReadPair		(MCHAR* section, MCHAR* var, int* value);
	bool	ReadPair		(MCHAR* section, MCHAR* var, bool* value);
	bool	ReadPair		(MCHAR* section, MCHAR* var, short* value);
	bool	ReadPair		(MCHAR* section, MCHAR* var, DWORD* value);
	bool	ReadPair		(MCHAR* section, MCHAR* var, size_t* value);
	bool	ReadPair		(MCHAR* section, MCHAR* var, MCHAR* value);
	bool	ReadPair		(MCHAR* section, MCHAR* var, SYSTEMTIME* value);
	bool	ReadPairHex		(MCHAR* section, MCHAR* var, DWORD* value);
};

//-----------------------------------------------------------------------------
//-- Network Engine Config
//
class MAXNETEXPORT MaxNetEngine : public MaxNetFile {

	public:

		bool	WriteNetStatus		(NetworkStatus* status);
		bool	WriteCfg			(ConfigurationBlock *cfg);
		bool	WriteAlertData		(AlertData *alerts);
		bool	WriteJobText		(CJobText& jobText);
		bool	WriteSceneInfo		(SceneInfo* info);
		bool	WriteWeekSchedule	(WeekSchedule* ws);

		bool	ReadNetStatus		(NetworkStatus* status);
		bool	ReadCfg				(ConfigurationBlock *cfg);
		bool	ReadAlertData		(AlertData *alerts);
		int		ReadJobTextCount	( );
		bool	ReadJobText			(CJobText& jobText);
		bool	ReadSceneInfo		(SceneInfo* info);
		bool	ReadWeekSchedule	(WeekSchedule* ws);

};

//! Network Job
class MAXNETEXPORT MaxNetJob : public MaxNetEngine {
	public:
		bool	WriteJob		(Job* job, CJobText& jobText);
		bool	ReadJob			(Job* job, CJobText& jobText);
		bool	WriteServers	(int count, const JobServer* servers);
		bool	ReadServers		(int count, JobServer* servers);
		bool	WriteFrames		(int count, const JOBFRAMES* frames);
		bool	ReadFrames		(int count, JOBFRAMES* frames);
};

//! Message File From Max to Server
enum max_msg
{
	MAX_MESSAGE_NOTHING=0,
	MAX_MESSAGE_READY,
	MAX_MESSAGE_BUSY,
	MAX_MESSAGE_FRAME_COMPLETE,
	MAX_MESSAGE_FRAME_ERROR,
	MAX_MESSAGE_GOING_DOWN
};

class MAXNETEXPORT MsgFromMax : public MaxNetFile {
	public:
		//-- Server uses this to peek/read messages
		max_msg	Message			(MCHAR* file, MCHAR* err = 0);
		//-- MAX uses this to write out messages
		void	WriteMessage	(max_msg message, MCHAR* err = 0);
};

//! Message File From Server to Max
enum srv_msg
{
	SRV_MESSAGE_NOTHING=0,
	SRV_MESSAGE_NEWFRAME,
	SRV_MESSAGE_CANCEL
};

class MAXNETEXPORT MsgFromSrv : public MaxNetFile {
	public:
		//-- MAX uses this to peek/read messages
		srv_msg	Message			(MCHAR* file, int* arg);
		//-- Server uses this to write out messages
		void	WriteMessage	(srv_msg message, int arg = 0);
};