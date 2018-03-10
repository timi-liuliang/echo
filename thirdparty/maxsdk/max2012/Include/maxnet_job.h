//-----------------------------------------------------------------------------
// -----------------------
// File ....: maxnet_job.h
// -----------------------
// Author...: Gus J Grubba
// Date ....: March 2000
// O.S. ....: Windows 2000
//
// History .: March, 11 2000 - Created
//
// 3D Studio Max Network Rendering Classes - Job
// 
//-----------------------------------------------------------------------------

#pragma once
#include <WTypes.h>
#include "strbasic.h"
#include "maxheap.h"
#include "network\MaxNetExport.h"
#include "maxnet_types.h"

// forward declarations
class MaxNetManagerImp;


//-----------------------------------------------------------------------------
//-- Scene Info

//#define SCENE_SHADOWMAPPED	(1<<0)	Obsolete
//#define SCENE_RAYTRACED		(1<<1)	Obsolete
#define SCENE_VIDEOCOLORCHECK	(1<<2)
#define SCENE_TWOSIDED			(1<<3)
#define SCENE_RENDERHIDEN		(1<<4)
#define SCENE_RENDERATMOSPHER	(1<<5)
#define SCENE_SUPERBLACK		(1<<6)
//#define SCENE_RENDERALPHA		(1<<7)	Obsolete
#define SCENE_SERIALNUMBERING	(1<<8)
#define SCENE_DITHER256			(1<<9)
#define SCENE_DITHERTRUE		(1<<10)
#define SCENE_RENDERFIELDS		(1<<11)
#define SCENE_DISPLACEMENT		(1<<12)
#define SCENE_EFFECTS			(1<<13)
#define SCENE_FIELDORDER		(1<<14)		//-- 0 Even / 1 Odd

/*! \sa Class MaxNetManager, Structure MaxJob
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is used by the Network Rendering API to store information about the scene.
*/
struct SceneInfo: public MaxHeapOperators {
	/*! The number of objects in the scene. */
	int		objects;
	/*! The total number of faces in the scene.  */
	int		faces;
	/*! The total number of lights in the scene. */
	int		lights;
	/*! The scene start / end time. */
	int		start,end;
	/*! This variable contains all the flags relating to the scenes rendering options, which are following values:
	- SCENE_VIDEOCOLORCHECK \n
	The video color check flag.
	- SCENE_TWOSIDED \n
	The render two sided flag.
	- SCENE_RENDERHIDEN \n
	The render hidden objects flag.
	- SCENE_RENDERATMOSPHER \n
	The render atmospheric effects flag.
	- SCENE_SUPERBLACK \n
	The render super black flag.
	- SCENE_SERIALNUMBERING \n
	The serial numbering flag.
	- SCENE_DITHER256 \n
	The dither 256 color flag.
	- SCENE_DITHERTRUE \n
	The dither true color flag.
	- SCENE_RENDERFIELDS \n
	The render fields flag.
	- SCENE_DISPLACEMENT \n
	The render displacement flag.
	- SCENE_EFFECTS \n
	The render effects flag.
	- SCENE_FIELDORDER \n
	The field ordering, 0 for even and 1 for odd.
	*/
	DWORD	flags;
};

//-----------------------------------------------------------------------------
//-- Text Info
//

enum JOB_TEXT_TYPE
{
	JOB_TEXT_USER = 64,				//!< User Name
	JOB_TEXT_COMPUTER,				//!< Computer Name (Job Submission)
	JOB_TEXT_MANAGER_SHARE,			//!< Manager's share (where to find job - filled by Manager)
	JOB_TEXT_FRAMES,				//!< Frames for those "1,2,4,5-40" types. Otherwise frames are defined in Job
	JOB_TEXT_MAX_OUTPUT,			//!< Output image file name (MAX)
	JOB_TEXT_CMB_OUTPUT,			//!< Output image file name (Combustion)
	JOB_TEXT_RENDER_ELEMENT,		//!< Render Elements
	JOB_TEXT_CAMERA,				//!< Camera List
	JOB_TEXT_RENDER_SCENE_STATE,	//!< The scene state used for rendering
	JOB_TEXT_SCENE_STATES,			//!< The scene state list
	JOB_TEXT_BATCH_RENDER,			//!< The name of the batch render entry this render will use
	JOB_TEXT_PRE_REND_SCRIPT,		//!< Pre-Render Script filename
	JOB_TEXT_POST_REND_SCRIPT,		//!< Post-Render Script filename
	JOB_TEXT_RESERVED = 9999		//!< Unknown Type
};

/*! \sa Class MaxNetManager, Structure JobText
\remarks This structure is available in release 4.0 and later only. \n\n
This structure contains the actual information for the output text buffer.
*/
struct TextBufferOutput: public MaxHeapOperators {
	/*! The device flag will be set to TRUE if the output is sent to a device instead of a file. */
	bool	device;
	/*! The output gamma for the device, which is optional. If the gamma value is set to 0.0, this value will be ignored. */
	float	gamma;
	/*! The text output buffer. */
	MCHAR	data[256];
};

/*! \sa Class MaxNetManager, Structure JobText
\remarks This structure is available in release 4.0 and later only. \n\n
This structure contains the details on a specific Render Element for a job.
*/
struct JobRenderElement: public MaxHeapOperators {
	/*! This flag indicates if the Render Element is enabled or disabled. */
	bool	enabled;
	/*! This flag indicates if filters are enabled or disabled for the Render Element. */
	bool	filterenabled;
	/*! This flag indicates if atmospheric effects are enabled or disabled for the Render Effect. */
	bool	atmosphere_applied;
	/*! This flag indicates if shadows are to be applied for the Render Effect. */
	bool	shadows_applied;
	/*! The name of the Render Effect. */
	MCHAR	name[128];
	/*! The output path and file name of the resulting Render Element. */
	MCHAR	output[MAX_PATH];
};

/*! \sa Class MaxNetManager, Class CJobText, Structure TextBufferOutput, Structure JobRenderElements, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_job_text_types.html">List of Job Text Types</a>
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is used by the Network Rendering API to store textural information regarding a job.
*/
struct JobText: public MaxHeapOperators {
	/*! The %Job Text type. See the <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_job_text_types.html">List of Job Text Types</a> for details. */
	JOB_TEXT_TYPE type;
	union {
		/*! The text buffer contents. */
		MCHAR			text[256];
		/*! The output text buffer. */
		TextBufferOutput	output;
		/*! The Render Elements details of the job. */
		JobRenderElement	re;
	};
};

/*! \sa  MaxNetCallBack, JobText, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_job_text_types.html">List of Job Text Types</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The CJobText class stores job information which is of a dynamic nature or of
variable length.  */
class MAXNETEXPORT CJobText: public MaxHeapOperators {
	friend class MaxNetManagerImp;
	protected:
		void*	list;
	public:
				CJobText		( );
				/*! \remarks Destructor. The CJobText buffers will be
				deallocated. */
				~CJobText		( );
		
		/*! \remarks This method return the number of JobText buffers in the
		list.\n\n
		Note: Developers should use Job.jobtextcount to find out how many
		elements there are. */
		int			Count		( );
		/*! \remarks This method will add another JobText buffer.
		\par Parameters:
		<b>JobText* jt</b>\n\n
		A pointer to the JobText buffer to add.
		\return  The number of JobText buffers. */
		int			Add			(JobText* jt);
		/*! \remarks This method will delete one or a sequence of buffers.
		\par Parameters:
		<b>int idx</b>\n\n
		The position of the first index to be deleted.\n\n
		<b>int count</b>\n\n
		The number of entries to delete. */
		void		Delete		(int idx, int count = 1);
		/*! \remarks This method will reset and deallocate the CJobText
		buffers. */
		void		Reset		( );
		/*! \remarks This method will return a pointer to the actual JobText
		buffer. */
		JobText*	Buffer		( );
		/*! \remarks This method returns the total size of the JobText buffer.
		*/
		int			BufferSize	( );
		
		/*! \remarks This access operator returns a reference to a JobText
		entry.
		\par Parameters:
		<b>const int i</b>\n\n
		The index of the JobText buffer to return. */
		JobText& operator[](const int i);

		/*! \remarks This method allows you to search for a JobText entry by
		its type. Refer to the list of Job Text types for details.
		\par Parameters:
		<b>JOB_TEXT_TYPE tp</b>\n\n
		The Job Text type you wish to find.\n\n
		<b>int start</b>\n\n
		The start position from which to initiate the search process.
		\return  The index of the entry which was found, or -1 if not found. */
		int		FindJobText		(JOB_TEXT_TYPE tp, int start = 0);
		/*! \remarks This method retrieves an index based on it's Text Type.
		Refer to the <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_job_text_types.html">List of Job
		Text Types</a> for details.
		\par Parameters:
		<b>MCHAR* text</b>\n\n
		A pointer to the text buffer which will be filled in by the method.\n\n
		<b>JOB_TEXT_TYPE type</b>\n\n
		The Job Text type you wish to find.\n\n
		<b>int start</b>\n\n
		The start position from which to retrieve the text item.\n\n
		<b>int* idx</b>\n\n
		A pointer to the index found.
		\return  TRUE if the method was successful, otherwise FALSE. */
		bool	GetTextItem		(MCHAR* text, JOB_TEXT_TYPE type, int start = 0, int* idx = 0);
		/*! \remarks This method will retrieve the user name associated with a
		job.
		\par Parameters:
		<b>MCHAR* user</b>\n\n
		The user name which was retrieved.
		\return  TRUE if the user name was retrieved, otherwise FALSE. */
		bool	GetUser			(MCHAR* user);
		/*! \remarks This method will retrieve the computer name associated
		with a job.
		\par Parameters:
		<b>MCHAR* computer</b>\n\n
		The computer name which was retrieved.
		\return  TRUE if the computer name was retrieved, otherwise FALSE. */
		bool	GetComputer		(MCHAR* computer);
		/*! \remarks This method will retrieve the frame sequence (such as the
		"1,2,4,5-40" types).
		\par Parameters:
		<b>MCHAR *frames</b>\n\n
		The frames retrieved.
		\return  TRUE if the frame sequence string was retrieved, otherwise
		FALSE. */
		bool	GetFrames		(MCHAR* frames);
		/*! \remarks This method will retrieve the Manager's network share
		associated with a job.
		\par Parameters:
		<b>MCHAR* share</b>\n\n
		The network share which was retrieved.
		\return  TRUE if the network share was retrieved, otherwise FALSE.
		\par Operators:
		*/
		bool	GetShare		(MCHAR* share);

};

//---------------------------------------------------------
//-- Alert Notification
//

#define NOTIFY_FAILURE		(1<<0)
#define NOTIFY_PROGRESS		(1<<1)
#define NOTIFY_COMPLETION	(1<<2)

/*! \sa Class MaxNetManager, Structure Job
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is used by the Network Rendering API to store 
information about the alert notifications.
*/
struct AlertData: public MaxHeapOperators {
	/*! This flag specifies if the alerts are enabled. */
	bool		alertEnabled;
	/*! This bitmap represents the enabled alerts, which are following values:
	- NOTIFY_FAILURE \n
	Notify on failure.
	- NOTIFY_PROGRESS \n
	Notify on progress.
	- NOTIFY_COMPLETION \n
	Notify on completion.
	*/
	DWORD		notifications;
	/*! Use every n-th frame for a progress report. */
	int		nthFrames;				
};

//---------------------------------------------------------
//-- Job Flags

#define JOB_VP				(1<<0)		//!< Video Post (otherwise is Render Scene)
#define JOB_NONC			(1<<1)		//!< Non concurrent driver (Accom, AVI, etc.)
#define JOB_MAPS			(1<<2)		//!< Include Maps
#define JOB_NONSTOP			(1<<3)		//!< Uninterruptible Driver (AVI, FCL, etc.)
#define JOB_SKIPEXST		(1<<4)		//!< Skip Existing Frames
#define JOB_ALLSERVERS		(1<<5)		//!< Use All Available Servers
#define JOB_INACTIVE		(1<<6)		//!< Job is Suspended
#define JOB_COMPLETE		(1<<7)		//!< Job is Complete (Read Only)
#define JOB_IGNORESHARE		(1<<8)		//!< Ignore Manager's Job Share - Always request archives
#define JOB_SKIPOUTPUTTST	(1<<9)		//!< Skip output test (Server won't test the output path)
#define JOB_NONSEQFRAMES	(1<<10)		//!< Non sequential frames (1,3,5-10, etc.)
#define JOB_COMBUSTIONJOB	(1<<11)		//!< Combustion Job
#define JOB_NOTARCHIVED		(1<<12)		//!< Uncompressed File (not an archive)
#define JOB_VFB				(1<<13)		//!< Should the VFB be up?
#define JOB_RENDER_CROP		(1<<14)		//!< Partial Render (Crop / Zoom / Region / etc.)

#define JOB_ASSIGN_VP		JOB_VP		//-- Make compatible with legacy flag
#define JOB_ASSIGN_RND		0			//-- Make compatible with legacy flag

//---------------------------------------------------------
//-- Priority Level

#define _JOB_PRIORITY_CRITICAL	0
#define _JOB_DEFAULT_PRIORITY	50

//---------------------------------------------------------
//-- Job

#define _JOB_VERSION 399

/*! \sa Class MaxNetManager, Structure MaxJob
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is used by the Network Rendering API to enable or disable Render Elements.
*/
struct MaxJobRenderElements: public MaxHeapOperators {
	/*! This flag determines if render elements are enabled or not. */
	bool		enabled;
};

/*! \sa Class MaxNetManager, Structure Job, Structure SceneInfo, Structure MaxJobRenderElements
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is used by the Network Rendering API to store information about a 3ds Max specific job.
*/
struct MaxJob: public MaxHeapOperators {
	/*! This flag determines if the structure is valid. */
	bool		init;				//-- Is structure valid?
	/*! This flag determines is gamma correction is used. */
	bool		gammacorrection;		//-- Use gamma correction?
	/*! The input gamma value for maps. */
	float		gammavaluein;			//-- Input Gamma (Maps)
	/*! The output gamma value for output images. */
	float		gammavalueout;			//-- Output Gamma (Output Image)
	/*! The pixel aspect ratio. */
	float		pixelaspect;			//-- Pixel Aspect Ratio
	/*! Camera */
	char		camera[128];			//-- Camera
	/*! The scene information data structure. */
	SceneInfo	sceneInfo;			//-- Scene Info
	/*! The render elements data structure. */
	MaxJobRenderElements	re;			//-- Render Elements
	/*! Reserved for future use. */
	char		reserved[64];
};

/*! \sa Class MaxNetManager, Structure Job
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is used by the Network Rendering API to store information about a Combustion specific job.
*/
struct CombustionJob: public MaxHeapOperators {
	/*! This flag determines if the structure is valid. */
	bool		init;
	/*! Reserved for future use. */
	char		reserved[128];
};

/*! \sa Class MaxNetManager, Structure AlertData, Structure MaxJob, Structure CombustionJob
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is used by the Network Rendering API to store information about a job.
*/
struct Job: public MaxHeapOperators {
	/*! The size of the structure, being sizeof(Job). */
	DWORD		size;
	/*! The structure version information, defined by _JOB_VERSION. */
	DWORD		version;
	/*! The server Process ID which is used by 3ds max to check server's health. */
	DWORD		server_pid;
	/*! The job flags, defined as the following values:
	- JOB_VP \n
	Video Post (otherwise is Render Scene).
	- JOB_NONC \n
	Non concurrent driver (Accom DDR, AVI, etc.)
	- JOB_MAPS \n
	The Include Maps flag.
	- JOB_NONSTOP \n
	Uninterruptible driver (AVI, FLC, etc.)
	- JOB_SKIPEXST   \n
	Skip Existing Frames.
	- JOB_ALLSERVERS \n
	Allow the use of all available servers.
	- JOB_INACTIVE \n
	This flag indicates the job is suspended
	- JOB_COMPLETE \n
	This read-only flag indicates that the job is complete.
	- JOB_IGNORESHARE \n
	Ignore the Manager's job share, always request archives.
	- JOB_SKIPOUTPUTTST \n
	This flag indicates that the server should not test the output path.
	- JOB_NONSEQFRAMES \n
	Non sequential frames such as 1,3,5-10, etc.
	- JOB_COMBUSTIONJOB \n
	This flag indicates the job is a Combustion specific job.
	- JOB_NOTARCHIVED \n
	This flag indicates an uncompressed file (i.e. not an archive).
	- JOB_ASSIGN_VP \n
	This is a legacy support flag, defined as JOB_VP.
	- JOB_ASSIGN_RND \n
	This is a legacy support flag.
	*/
	DWORD		flags;
	/*! Handle to the job, assigned by the Manager when a job is created/submitted. This handle will be read-only after its creation. */
	HJOB		hJob;
	/*! The name of the job. */
	char		name[MAX_PATH];
	/*! This variable is used internally when transferring an archive and specifies its size. */
	DWORD		filesize;			//!< Used internally when transferring archive (size of archive file)
	/*! This variable is used internally when transferring an archive and specifies its uncompressed size. */
	DWORD		filesizeextracted;		//!< Used internally when transferring archive (size of expanded archive)
	/*! The system time, set when a job is created/submitted. */
	SYSTEMTIME	submission;
	/*! The system time, set when a job starts.*/
	SYSTEMTIME	startjob;
	/*! The system time, set when a job is completed. */
	SYSTEMTIME	endjob;
	/*! The number of servers defined for the job (can be 0 if JOB_ALLSERVERS is set). */
	int		servercount;
	/*! The alert notification data structure. */
	AlertData	alerts;
	/*! The number of JobTextInfo records. */
	int		jobtextcount;
	/*! The first frame in the range. */
	int		firstframe;
	/*! The last frame in the range. */
	int		lastframe;
	/*! The frame step value (i.e. every n-th frame). */
	int		step;
	/*! The frame output dimensions (width / height). */
	int		width,height;
	/*! The number of frames completed. */
	int		frames_completed;
	/*! The job priority level. */
	char		priority;
	/*! Reserved for future use. */
	char		reserved[32];
	union {
		/*! Specific information about a job pertaining to 3ds Max. */
		MaxJob			maxJob;			//-- MAX Specific
		/*! Specific information about a job pertaining to Combustion. */
		CombustionJob	combustionJob;	//-- Combustion Specific
	};
};


										
