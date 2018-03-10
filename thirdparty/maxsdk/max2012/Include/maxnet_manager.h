//-----------------------------------------------------------------------------
// ---------------------------
// File ....: maxnet_manager.h
// ---------------------------
// Author...: Gus J Grubba
// Date ....: February 2000
// O.S. ....: Windows 2000
//
// History .: Feb, 15 2000 - Created
//
// 3D Studio Max Network Rendering Classes
// 
//-----------------------------------------------------------------------------

#pragma once
#include "network\MaxNetExport.h"
#include "network\MaxNet.h"
#include "strbasic.h"
#include "maxheap.h"
#include "maxnet_job.h"
// forward declarations
struct ManagerInfo;
struct ClientInfo;
struct JobServer;
struct JOBFRAMES;
struct ServerList;
struct NetworkStatus;

//-------------------------------------------------------------------
//-- Global Server State
//

#define JOB_STATE_COMPLETE	0
#define JOB_STATE_WAITING	1
#define JOB_STATE_BUSY		2
#define JOB_STATE_ERROR		3
#define JOB_STATE_SUSPENDED	4

/*! \sa Class MaxNetManager, Structure Job
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is used by the Network Rendering API to store information the state of a job.
*/
struct JobList: public MaxHeapOperators {
	/*! The structure containing the job details. */
	Job		job;
	/*! The handle to the job. */
	HJOB		hJob;
	/*! The current state of the job, which is one of the following values:
	- JOB_STATE_COMPLETE \n
	The job is complete.
	- JOB_STATE_WAITING \n
	The job is waiting to be rendered.
	- JOB_STATE_BUSY \n
	The job is busy rendering.
	- JOB_STATE_ERROR \n
	The job experienced an error.
	- JOB_STATE_SUSPENDED \n
	The job is suspended.
	*/
	WORD		state;
};

//-----------------------------------------------------------------------------
//-- MaxNetCallBack
//
//	Note: Return as soon as possible from these calls. They block the API thread
//  and nothing will happen until you return. If you have to do some processing,
//  post a message to your own code and return immediately. Also note that these
//  calls may come from a separate thread than your main process thread. 
//

/*! \sa  Class MaxNetManager\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
If you want to use the call back mechanisms provided by the MaxNetManager
class, you create your own class derived from MaxNetCallBack and pass it as the
argument for <b>MaxNetManager::SetCallBack()</b>. All methods are optional. You
need only to implement those you want.  */
#pragma warning(push)
#pragma warning(disable:4100)
class MAXNETEXPORT MaxNetCallBack: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~MaxNetCallBack() {;}
		//-- Return "true" to cancel, "false" to continue
		/*! \remarks This method is called whenever a lengthy operation is
		under way. This includes large block transfers, file transfers, etc.
		\par Parameters:
		<b>int total</b>\n\n
		A total amount of information to process.\n\n
		<b>int current</b>\n\n
		From the total amount, this is the current position.
		\return  TRUE if you want to cancel the operation, otherwise FALSE. */
		virtual bool Progress		(int total, int current){return false;}
		/*! \remarks This method is called to provide a textual message
		regarding the current process which is under way (connecting to a
		Manager, waiting for a reply, etc.)
		\par Parameters:
		<b>const MCHAR* message</b>\n\n
		The text message. */
		virtual void ProgressMsg	(const MCHAR *message){;}
		//-- Notifies the Manager Went Down
		/*! \remarks This method is called for messages which are sent to
		indicate that the Manager was shut down and is no longer available to
		service any requests. */
		virtual void ManagerDown	( ){;}
		//-- Notifies something has changed (new job, new server, new frame, etc.)
		/*! \remarks This method is called to indicate something has changed.
		This message is sent whenever a new job is sent to the queue, a Server
		changed its state, an error occurred, etc. This allows you to be kept
		up to date without the need to poll the Manager from time to time. Note
		that these messages are queued up. That is, if 10 jobs are deleted at
		once or any number of changes occur within a short period, you will
		only receive one <b>Update()</b> call. */
		virtual void Update			( ){;}
		//-- Notifies someone wants control of the queue; Send grant control msg to manager;
		/*! \remarks This method is called if someone requests control over
		the queue. If you do not have control over the queue, you do not need
		to respond to this message. If you have control over the queue and do
		nothing when this call is received, the Manager will timeout and take
		control over the queue. If you have control over the queue, you should
		respond to this message using
		<b>MaxNetManager::GrantManagerControl()</b> passing either a true
		value, indicating you are granting control, or a false value,
		indicating you do not want to relinquish control.
		\par Parameters:
		<b>MCHAR* station</b>\n\n
		The name of the computer requesting control. */
		virtual void QueryControl	( MCHAR* station ){;}
		//-- Notifies someone has taken control of the queue (Another QueueManager for instance)
		/*! \remarks This method is called in order to notify that someone
		took control of the queue. */
		virtual void QueueControl	( ){;}
};
#pragma warning(pop)
//-----------------------------------------------------------------------------
//-- Manager Session Class
//

/*! \sa  Class MaxNet, Class MaxNetCallBack, Class CJobText, Class ManagerInfo, Class ClientInfo, Class JobList, Class Job, Class HSERVER, Class JOBFRAMES, Class JobServer, Class ServerList, Class WeekSchedule, Class NetworkStatus
\par Description:
This class is available in release 4.0 and later only.\n\n
The MaxNetManager class provides all the methods to interact with the network
rendering functions provided and acts as your primary interface. The API
provided through the MaxNetManager allows clients to connect to the Network
Rendering Manager and perform any and all functions available. It encapsulates
all the networking details leaving the client code to concentrate on whatever
it needs to do. The API handles all networking code and the intricacies of the
communication protocols used by the lower layers. This class is derived from
the MaxNet class which is solely used for exception handling as shown
below.\n\n
\par Sample Code:
<b>try {</b>\n\n
<b>// the code being tried</b>\n\n
<b>} catch (MaxNet* maxerr) {</b>\n\n
<b>// handle the error</b>\n\n
<b>// do NOT delete maxerr</b>\n\n
<b>// use maxerr-\>GetErrorText() to get the error description</b>\n\n
<b>// use maxerr-\>GetError() to get the error code</b>\n\n
<b>}</b>
\par Method Groups:
See <a href="class_max_net_manager_groups.html">Method Groups for Class MaxNetManager</a>.
*/
class MAXNETEXPORT MaxNetManager : public MaxNet {

	public:
		/*! \remarks Destructor. */
		virtual ~MaxNetManager() {;}

		//-- Optional Call Back
		
		/*! \remarks Sets a callback method to receive information updates
		about various asynchronous events from the MaxNet API. It is not
		required to set up this callback.
		\par Parameters:
		<b>MaxNetCallBack* mgr</b>\n\n
		Points to a MaxNetCallBack object. */
		virtual void	SetCallBack				( MaxNetCallBack* cb )=0;

		//-- Session
		/*! \remarks This method will broadcast a message to the local area network in
		order to look for a Manager. If a Manager is found, the method returns its name
		in <b>manager</b>.
		\par Parameters:
		<b>short port</b>\n\n
		Specifies which port will be used to access the Manager. Unless there is a
		specific reason to use a particular port, use the default
		<b>DF_MGRPORT</b>.\n\n
		<b>char* manager</b>\n\n
		A string representing the name of the Manager will be put into this variable.
		The variable itself should be <b>MAX_PATH</b> in size.\n\n
		<b>char* netmask</b>\n\n
		Specifies which local area network mask should be used for the scope of the
		broadcast. The default of 255.255.255.0 should work for most networks that are
		not divided into subnets.
		\return  true if a Manager is found, otherwise false. */
		virtual bool	FindManager				( short port, char* manager, char* netmask = "255.255.255.0" )=0;
		/*! \remarks This method allows you to connect to a Network Rendering
		Manager.
		\par Parameters:
		<b>short port</b>\n\n
		Specifies which port will be used to access the Manager. Unless there
		is a specific reason to use a particular port, use the default
		<b>DF_MGRPORT</b>.\n\n
		<b>char* manager</b>\n\n
		A string representing the name or IP number of the Manager you want to
		connect to. The network name requires the network to have some form of
		name to address translation. It is recommended to use the name provided
		by <b>MaxNetManager::FindManager()</b>.\n\n
		<b>bool enable_callback = false</b>\n\n
		This parameter enables or disables asynchronous messages from the
		Manager. If you connect to the Manager in order to collect information
		about the network queue then it is recommended to enable (set to true)
		this parameter in order to keep your lists updated. Otherwise you have
		to poll the Manager at various times to check for changes. When enabled
		calls will be received though the <b>MaxNetCallBack</b> mechanism
		informing you whenever a new job has been completed, an error has
		occurred, a Server has changed, etc. */
		virtual void	Connect					( short port, char* manager = NULL, bool enable_callback = false )=0;
		/*! \remarks This method will cause a disconnect from the currently
		connected Manager. */
		virtual void	Disconnect				( )=0;
		/*! \remarks This method allows basic information about the Manager to
		be collected.
		\par Parameters:
		<b>ManagerInfo* info</b>\n\n
		A pointer to the Manager information. */
		virtual void	GetManagerInfo			( ManagerInfo* info )=0;
		/*! \remarks This method will shut down the Manager and shut off the
		entire system.
		\return  TRUE if successful, otherwise FALSE. A reason for failure
		might be because this method is called without having the proper
		Manager rights such as operating in read only mode. Further explanation
		can be found in the <b>TakeManagerControl()</b> method. */
		virtual bool	KillManager				( )=0;
		/*! \remarks This method toggles updates from the Manager. For this
		method to function you will need to enable the <b>enable_callback</b>
		in <b>MaxNetManager::Connect()</b>. The use of this method allows you
		to temporarily disable updates from the Manager which might be useful
		in a situation when you are submitting many jobs at once or executing
		any other large number of changes. Instead of receiving updates for all
		changes, you would temporarily disable the callbacks so you could
		execute your many changes and when done, re-enable the callbacks.
		\par Parameters:
		<b>bool enable</b>\n\n
		TRUE or FALSE to enable or disable, respectively. */
		virtual void	EnableUpdate			( bool enable = true )=0;
		
		/*! \remarks This method queries the Manager to check if you can take
		control of the queue. If no one has the queue control, it will
		immediately return true. If someone else has control, the Manager will
		ask the controlling client if it wants to relinquish control. If
		relinquished, the method returns true, if control is not relinquished
		the method will return false.
		\par Parameters:
		<b>bool wait</b>\n\n
		This parameter can be set to true in order to wait for an answer in
		case someone has control over the queue, causing this method not to
		return until it receives an answer from the controlling client. If
		there is no response from the controlling client, the method will time
		out in 10 seconds after which it will return true to allow a request
		for queue control.
		\return  TRUE if allowed to request queue control, otherwise FALSE. */
		virtual bool	QueryManagerControl		( bool wait )=0;
		/*! \remarks This method allows the acquisition of control of the
		queue.
		\return  TRUE if control is granted, otherwise FALSE. */
		virtual bool	TakeManagerControl		( )=0;
		/*! \remarks This method allows you to issue a grant or deny response
		to a MaxNetCallBack message. If you are the controlling client while
		another client wants control of the queue by calling the
		<b>QueryManagerControl()</b> method you will receive a message through
		the <b>MaxNetCallBack</b> mechanism. If you do not respond, control
		will be taken away from you automatically after 10 seconds and granted
		to the requesting client.
		\par Parameters:
		<b>bool grant</b>\n\n
		This parameter allows you to respond TRUE to relinquish control of the
		queue or FALSE if you do not want to relinquish control. */
		virtual void	GrantManagerControl		( bool grant )=0;
		/*! \remarks This method allows you to temporarily lock the queue
		control when performing a series of changes and don't want to get
		interrupted. While the queue control is locked, no queries are made.
		They all return false to prevent anyone from taking control of the
		queue. This method can only be called if you already have queue
		control. Please, do not forget to unlock the queue control after you
		are done performing your changes.
		\par Parameters:
		<b>bool lock</b>\n\n
		Set this parameter to TRUE if you want to lock the queue or FALSE if
		you want to unlock the queue.
		\return  TRUE if queue control could be locked, otherwise FALSE. */
		virtual bool	LockControl				( bool lock  )=0;
		/*! \remarks This method returns the number of clients currently
		connected to the Manager. */
		virtual int		GetClientCount			( )=0;
		/*! \remarks This method allows you to list all the clients currently
		connected to the Manager.
		\par Parameters:
		<b>int start</b>\n\n
		The first client in the list to return.\n\n
		<b>int end</b>\n\n
		The last client in the list to return. If you want the entire list of
		clients at once set the <b>start</b> and <b>end</b> to 0 and -1,
		respectively.\n\n
		<b>ClientInfo* clientList</b>\n\n
		The array to receive the list of clients. This array should be large
		enough to receive the number of clients requested.
		\return  The actual number of clients inserted in <b>clientList</b>.
		This could be either equal to the amount requested or less (if some
		client disconnected from the Manager). */
		virtual int		ListClients				( int start, int end, ClientInfo* clientList )=0;

		//-- Jobs
		
		/*! \remarks This method returns the number of jobs in the queue. */
		virtual int		GetJobCount				( )=0;
		/*! \remarks This method allows you to list all the jobs in the queue.
		\par Parameters:
		<b>int start</b>\n\n
		The first job in the list to return.\n\n
		<b>int end</b>\n\n
		The last job in the list to return. If you want the entire list of jobs
		at once set the <b>start</b> and <b>end</b> to 0 and -1,
		respectively.\n\n
		<b>JobList* jobList</b>\n\n
		The array to receive the list of jobs. This array should be large
		enough to receive the number of jobs requested.
		\return  The actual number of jobs inserted in <b>jobList</b>. This
		could be either equal to the amount requested or less. */
		virtual int		ListJobs				( int start, int end, JobList* jobList )=0;
		/*! \remarks This method allows you to get a single record for the
		jobList given the specified job handle.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The job handle.\n\n
		<b>JobList* jobList</b>\n\n
		A pointer to a JobList structure for the received job. */
		virtual void	GetJob					( HJOB hJob, JobList* jobList )=0;
		/*! \remarks This method allows you to get an individual job
		description structure.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The job handle.\n\n
		<b>Job* job</b>\n\n
		A pointer to a Job structure for the received job. */
		virtual void	GetJob					( HJOB hJob, Job* job )=0;
		/*! \remarks Use this method to request the CJobText for a particular
		job. (See the CJobText class description for an explanation). Some job
		information are random both in number as they are in size. The job
		description structure (Job) will only show static elements. In order to
		get dynamic elements or elements with variable length, the CJobText
		class is used.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The job handle.\n\n
		<b>CjobText\& jobText</b>\n\n
		A reference to a CJobText class to receive the information.\n\n
		<b>int count</b>\n\n
		The number of elements you are interested in receiving. The number of
		elements can be found in Job.jobtextcount. */
		virtual void	GetJobText				( HJOB hJob, CJobText& jobText, int count )=0;
		/*! \remarks This method submits changes to an existing job. Once you
		collect a job, you can change settings in both the Job structure as in
		the CJobText elements and send it back so the changes can be
		applied.\n\n
		Note that you must use a <b>Getjob()</b> and <b>SetJob()</b>
		combination in order to make sure all the elements are correct. The
		Manager will automatically complete some of the structure members. It
		is not possible to create a new structure, fill in the data, and submit
		it. You can only do that when submitting a new job, which is then
		handled by a different set of methods.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The job handle.\n\n
		<b>Job* job</b>\n\n
		A pointer to the job description.\n\n
		<b>CJobText\& jobText</b>\n\n
		A reference to a CJobText class with the description of elements.\n\n
		<b>bool reset</b>\n\n
		This flag indicates whether or not the job is started from scratch. If
		set to FALSE it will continue from the current stage. */
		virtual void	SetJob					( HJOB hJob, Job* job, CJobText& jobText, bool reset )=0;
		/*! \remarks This method returns the job priority value for the
		specified Job.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The handle to the job for which to obtain its priority */
		virtual int		GetJobPriority			( HJOB hJob )=0;
		/*! \remarks This method allows you to set the priority of a specified
		job.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The handle to the job for which to obtain its priority\n\n
		<b>int priority</b>\n\n
		The priority value you want to assign to the job.
		\return  TRUE if the priority as set successfully, otherwise FALSE */
		virtual bool	SetJobPriority			( HJOB hJob, int priority )=0;
		/*! \remarks This method allows you to set the job order for a
		specific job.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The handle to the job for which to obtain its priority\n\n
		<b>DWORD count</b>\n\n
		The job order index. */
		virtual void	SetJobOrder				( HJOB* hJob, DWORD count )=0;
		/*! \remarks This method will delete a job from the queue permanently
		and remove all files related to the job.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The job handle. */
		virtual void	DeleteJob				( HJOB hJob )=0;
		/*! \remarks This method will suspend a specific job. This method is
		the opposite of <b>ActivateJob()</b>.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The job handle */
		virtual void	SuspendJob				( HJOB hJob )=0;
		/*! \remarks This method will activate a specific job. This method is
		the opposite of <b>SuspendJob()</b>.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The job handle */
		virtual void	ActivateJob				( HJOB hJob )=0;
		/*! \remarks This method will return the number of Servers assigned to
		a given job.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The job handle */
		virtual int		GetJobServersCount		( HJOB hJob )=0;
		/*! \remarks This method will return a list of the Servers assigned to
		a given job.
		\par Parameters:
		<b>int start</b>\n\n
		The first Server in the list to return.\n\n
		<b>int end</b>\n\n
		The last Server in the list to return. If you want the entire list of
		servers at once set the <b>start</b> and <b>end</b> to 0 and -1,
		respectively.\n\n
		<b>HJOB hJob</b>\n\n
		The job handle.\n\n
		<b>JobServer* servers</b>\n\n
		The array to receive the list of Servers. This array should be large
		enough to receive the number of Servers requested.
		\return  The actual number of Servers inserted in <b>servers</b>. This
		could be either equal to the amount requested or less. */
		virtual int		GetJobServers			( int start, int end, HJOB hJob, JobServer* servers )=0;
		/*! \remarks This method allows you to obtain the textual status of a
		specific Server for a given job. The JobServer structure return by
		<b>GetJobServers()</b> will only describe the status of a Server using
		a flag. If the flag shows the Server status is "Error", you can use
		this method to receive a more descriptive text message, like "Could not
		write to d:/path/file.tga".
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The job handle.\n\n
		<b>HSERVER hServer</b>\n\n
		The Server handle.\n\n
		<b>MCHAR* status_text</b>\n\n
		A pointer to a string to receive the message. This string should be
		<b>MAX_PATH</b> long. */
		virtual void	GetJobServerStatus		( HJOB hJob, HSERVER hServer, MCHAR* status_text )=0;
		/*! \remarks This method will suspend a specific Server for a given
		job. The Server will stop working with the given job and start working
		on another one, provided another job exists. Use the
		<b>AssignJobServer()</b> to reactive it.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The job handle.\n\n
		<b>HSERVER hServer</b>\n\n
		The Server handle. */
		virtual void	SuspendJobServer		( HJOB hJob, HSERVER hServer )=0;
		/*! \remarks This method can be used to assign a Server to a given
		job.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The job handle.\n\n
		<b>HSERVER hServer</b>\n\n
		The Server handle. */
		virtual void	AssignJobServer			( HJOB hJob, HSERVER hServer )=0;
		/*! \remarks This method will return the number of frames for a given
		job.
		\par Parameters:
		<b>HJOB hJob</b>\n\n
		The job handle. */
		virtual int		GetJobFramesCount		( HJOB hJob )=0;
		/*! \remarks This method will return a list of frames for a given job.
		\par Parameters:
		<b>int start</b>\n\n
		The first frame in the list to return.\n\n
		<b>int end</b>\n\n
		The last frame in the list to return. If you want the entire list of
		frames at once set the <b>start</b> and <b>end</b> to 0 and -1,
		respectively.\n\n
		<b>HJOB hJob</b>\n\n
		The job handle.\n\n
		<b>JOBFRAMES* frames</b>\n\n
		The array to receive the list of frames with information for each
		individual frame. This array should be large enough to receive the
		number of frames requested.
		\return  The actual number of frames inserted in <b>frames</b>. This
		could be either equal to the amount requested or less. */
		virtual int		GetJobFrames			( int start, int end, HJOB hJob, JOBFRAMES* frames )=0;
		/*! \remarks This method will return the log file for a given job.
		\par Parameters:
		<b>int start</b>\n\n
		The first log file line to return (base zero, 0 is the first line).\n\n
		<b>int count</b>\n\n
		The number of lines (rows) to return. If you want the whole file at
		once, set <b>start</b> to 0 and <b>count</b> to -1. Alternatively, if
		you want any lines added since the last time you call, set <b>start</b>
		to the last line you collected + 1 and <b>count</b> to -1.\n\n
		<b>HJOB hJob</b>\n\n
		The job handle.\n\n
		<b>MCHAR** buffer</b>\n\n
		A pointer to a MCHAR array to receive the lines of the log file. The
		buffer will be allocated so it will accommodate the incoming data. This
		array should be freed using <b>LocalFree()</b> when you are finished.
		\return  The size of the allocated buffer. This is the size of the
		entire buffer including the last NULL terminating byte. If
		<b>GetJobLog()</b> returns 0, it means there are no new log file lines
		available. */
		virtual int		GetJobLog				( int start, int count, HJOB hJob, MCHAR** buffer )=0;
		
		/*! \remarks When submitting a job, you can use this method to find
		out if the Manager can write a given output image file. This is usually
		the case when you have the output image file set to a local drive. The
		other participants in the network rendering may not be able to "see"
		this path and they will eventually fail. This test is not guaranteed as
		the Servers may have a different set of rights than the Manager, in
		which case the Manager might fail the write test while the Servers
		would have no problem otherwise.
		\par Parameters:
		<b>MCHAR* output</b>\n\n
		The output image file name you want to check. This is the full path and
		filename (i.e. d:/badpath/file.tga).\n\n
		<b>MCHAR* err</b>\n\n
		A MCHAR string to receive the error message if one exists. This will
		explain why the test failed (such as path not found, access denied,
		etc.)
		\return  TRUE if the Manager could write to the given path, otherwise
		FALSE. */
		virtual bool	CheckOutputVisibility	( MCHAR* output, MCHAR* err )=0;
		/*! \remarks This method allows you to assign a new job to the network
		queue.
		\par Parameters:
		<b>Job* job</b>\n\n
		The job structure containing the information about the job. See the Job
		structure description for an explanation.\n\n
		<b>MCHAR* archive</b>\n\n
		The full path and filename of the archive containing the job files.
		This is the "*.maz" file created by the <b>Maz()</b> function in the
		API.\n\n
		<b>HSERVER* servers</b>\n\n
		An array containing the Servers assigned to this job. If the job flag
		is set to "use all Servers" and <b>job.servercount</b> is zero, this
		argument can be NULL (ignored). Otherwise it should be an array
		<b>job.servercount * sizeof(HSERVERS)</b> long with the list of Servers
		to assign to this job.\n\n
		<b>CJobText\& jobtext</b>\n\n
		A reference to a CJobText class with the proper elements.\n\n
		<b>DWORD blocksize</b>\n\n
		An optional alternate block size to use for network transfers. If
		<b>blocksize</b> is set to zero, the API will use the default
		<b>DF_READCHUNK</b>. You may want to set this to something smaller if
		you are running over slow connections such as a modem connection. You
		may want to make it larger if you have a high performance network. This
		number will determine how large of a block of data to send at once to
		the Manager. */
		virtual void	AssignJob				( Job* job, MCHAR* archive, HSERVER* servers, CJobText& jobtext, DWORD blocksize = 0 )=0;
		

		//-- Servers (Global)

		/*! \remarks This method will return the number of Servers registered
		with the Manager. */
		virtual int		GetServerCount			( )=0;
		/*! \remarks This method allow you to list the Servers registered with
		the Manager.
		\par Parameters:
		<b>int start</b>\n\n
		The first Server to return.\n\n
		<b>int end</b>\n\n
		The last Server to return. If you want the whole list at once, set
		<b>start</b> to 0 and <b>end</b> to -1.\n\n
		<b>ServerList* serverList</b>\n\n
		The array to receive the list of Servers. This array should be large
		enough to receive the number of Servers requested.
		\return  The actual number of Servers inserted in <b>serverList</b>.
		This could be either equal to the amount requested or less. */
		virtual int		ListServers				( int start, int end, ServerList* serverList )=0;
		/*! \remarks This method allows you to get a Server and retrieve a
		single record in the serverList given\n\n
		the Server handle.
		\par Parameters:
		<b>HSERVER hServer</b>\n\n
		The handle to the Server.\n\n
		<b>ServerList* serverList</b>\n\n
		A pointer to the ServerList in which to retrieve the Server. */
		virtual void	GetServer				( HSERVER hServer, ServerList* serverList )=0;
		/*! \remarks This method allows you to delete a Server from the
		Manager's Server list. You can not delete an active Server (i.e. a
		Server which is currently working on a job).
		\par Parameters:
		<b>HSERVER* hServer</b>\n\n
		The Server handle.
		\return  TRUE if the server is successfully deleted, otherwise FALSE.
		*/
		virtual bool	DeleteServer			( HSERVER hServer )=0;
		/*! \remarks The Manager keeps a performance index for each Server.
		This index is computed based on the Server performance while rendering
		frames. All factors are taken into consideration such as the time it
		takes to load a job, the time it takes to process requests, the time it
		takes to render a frame, the memory and CPU load, etc. This index is in
		turn used internally to determine the best distribution of workload.
		You can use this method to reset a Server's performance index.
		\par Parameters:
		<b>HSERVER* hServer</b>\n\n
		The Server handle.
		\return  TRUE if resetting the Server's performance index was
		successful, otherwise FALSE. */
		virtual bool	ResetServerIndex		( HSERVER hServer )=0;
		/*! \remarks This method allows you to obtain a given Server's weekly
		schedule structure.
		\par Parameters:
		<b>HSERVER* hServer</b>\n\n
		The Server handle.\n\n
		<b>WeekSchedule* schedule</b>\n\n
		A pointer to a WeekSchedule structure to receive the schedule. */
		virtual void	GetWeekSchedule			( HSERVER hServer, WeekSchedule* schedule )=0;
		/*! \remarks This method allows you to set a given Server's weekly
		schedule structure.
		\par Parameters:
		<b>HSERVER* hServer</b>\n\n
		The Server handle.\n\n
		<b>WeekSchedule* schedule</b>\n\n
		A pointer to a WeekSchedule structure with the new weekly schedule. */
		virtual void	SetWeekSchedule			( HSERVER hServer, WeekSchedule* schedule )=0;
		/*! \remarks This method allows you to obtain the network status for a
		given server. This method mostly serves as means to check network
		diagnostics.
		\par Parameters:
		<b>HSERVER* hServer</b>\n\n
		The Server handle.\n\n
		<b>NetworkStatus* net_stat</b>\n\n
		A pointer to a NetworkStatus structure to receive the status data. */
		virtual void	GetServerNetStat		( HSERVER hServer, NetworkStatus* net_stat )=0;
		
		/*! \remarks This method will return the number of Server groups. */
		virtual int		GetServerGroupCount		( )=0;
		/*! \remarks This method will return the number of Servers for a given
		Server group.
		\par Parameters:
		<b>int group</b>\n\n
		The zero based index into the Server group list. */
		virtual int		GetServerGroupXCount	( int group )=0;
		/*! \remarks This method allows you to obtain a Server group.
		\par Parameters:
		<b>int group</b>\n\n
		The zero based index into the Server group list.\n\n
		<b>int count</b>\n\n
		The number of Servers to send, in order to define the size of the
		<b>grplist</b>.\n\n
		<b>HSERVER* grplist</b>\n\n
		The array in which the list of Servers will be returned. This array
		should be large enough to accommodate <b>count</b> Servers.\n\n
		<b>MCHAR* name</b>\n\n
		The name of the Server group. This string must be at least
		<b>MAX_PATH</b> long.
		\return  The number of Servers collected. */
		virtual int		GetServerGroup			( int group, int count, HSERVER* grplist, MCHAR* name )=0;
		/*! \remarks This method allows you to submit (create) a new Server
		group.
		\par Parameters:
		<b>int count</b>\n\n
		The number of Servers in the list.\n\n
		<b>HSERVER* grplist</b>\n\n
		The array containing <b>count</b> Servers.\n\n
		<b>MCHAR* name</b>\n\n
		The name of the server group. */
		virtual void	NewServerGroup			( int count, HSERVER* grplist, MCHAR* name )=0;
		/*! \remarks This method allows you to delete a given Server group.
		\par Parameters:
		<b>int group</b>\n\n
		The zero based index of the Server group to delete. */
		virtual void	DeleteServerGroup		( int group )=0;
		

};

//-----------------------------------------------------------------------------
//-- Interface Class

class MaxNetworkInterface: public MaxHeapOperators {
	public:
		virtual ~MaxNetworkInterface() {;}
		virtual	bool	GetCurrentRenderer		( MCHAR* name, DWORD* id1, DWORD* id2)=0;
};

//This class will have more methods with each version.
//Version number indicates which methods are supported by a given implementation
class MaxNetworkInterface2 : public MaxNetworkInterface {
	protected:
		int version;
	public:
		MaxNetworkInterface2() { version = 6010; }  //version 6.0.1.0
		int		GetMAXVersion()						{return version;}
		void	SetMAXVersion(int version)			{this->version = version;}

		//-- version 6.0.1.0 - begin
		virtual BOOL	GetRendMultiThread()		{return FALSE;}
		virtual BOOL	GetRendSimplifyAreaLights()	{return FALSE;}
		virtual BOOL	GetUseAdvLight()			{return FALSE;}
		virtual BOOL	GetCalcAdvLight()			{return FALSE;}
		//-- version 6.0.1.0 - end

		//-- version 7.0.0.0 - begin
		//-- ADD VERSION 7 METHODS HERE
		//-- version 7.0.0.0 - end
};

//Uses interface version 1
MAXNETEXPORT void AssignJobEx(
	MaxNetManager* mgr, 
	MaxNetworkInterface* maxIface,
	Job* job,
	MCHAR* archive,
	HSERVER* servers,
	CJobText& jobtext,
	DWORD blocksize = 0 );

//Uses interface version 2
MAXNETEXPORT void AssignJobEx(
	MaxNetManager* mgr, 
	MaxNetworkInterface2* maxIface,
	Job* job,
	MCHAR* archive,
	HSERVER* servers,
	CJobText& jobtext,
	DWORD blocksize = 0 );


//-- EOF: maxnet_manager.h ----------------------------------------------------
