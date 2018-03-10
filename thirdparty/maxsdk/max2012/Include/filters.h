//-----------------------------------------------------------------------------
// --------------------
// File ....: Filters.h
// --------------------
// Author...: Gus Grubba
// Date ....: September 1995
//
// History .: Sep, 07 1995 - Started
//
//-----------------------------------------------------------------------------
#pragma once
#include "FLTExport.h"
#include "maxheap.h"
#include "fltapi.h"
#include "tvnode.h"
#include "maxapi.h"
#include "bitmap.h"

//-- How long can a filter name be
#define MAXFILTERNAME  MAX_PATH
#define MAXRESOURCE    MAX_PATH

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- Frame Range
//
   
/*! \sa  Class ImageFilterInfo.\n\n
\par Description:
This class describes a range of frames and provides methods to access the
first, last, current and number of frames in the range. All methods of this
class are implemented by the system.  */
class FrameRange: public MaxHeapOperators {

	  int start;
	  int end;
	  int current;
	  
	public:
	
	  FLTExport       FrameRange  ( ) {start = end = current  = 0;}
	  FLTExport       ~FrameRange ( ) {};

	  /*! \remarks Returns the first frame number of this range. */
	  FLTExport int   First       ( ) {   return (start); }
	  /*! \remarks Returns the last frame number of this range. */
	  FLTExport int   Last        ( ) {   return (end); }
	  /*! \remarks Returns the number of frames in this range. */
	  FLTExport int   Count       ( ) {   return (end - start + 1); }
	  /*! \remarks Returns the current frame of this range. */
	  FLTExport int   Current     ( ) {   return (current);   }
	  /*! \remarks Returns the elapsed time of this range. */
	  FLTExport int   Elapsed     ( ) {   return (current -   start); }

	  /*! \remarks Sets the first frame number for this range to the specified
	  value.
	  \par Parameters:
	  <b>int u</b>\n\n
	  Specifies the new first frame number. */
	  FLTExport void  SetFirst    ( int u ) { start = u; }
	  /*! \remarks Sets the last frame number for this range to the specified
	  value.
	  \par Parameters:
	  <b>int u</b>\n\n
	  Specifies the new last frame number. */
	  FLTExport void  SetLast     ( int u ) { end = u; }
	  /*! \remarks Sets the current frame number for this range to the
	  specified value.
	  \par Parameters:
	  <b>int u</b>\n\n
	  Specifies the new current frame number. */
	  FLTExport void  SetCurrent  ( int u ) { current = u; }


};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//--   Forward Reference

class ImageFilter;
class FilterManager;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- Time Change Notification (R2)

/*! \sa  Class TimeChangeCallback,  Class ImageFilter.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class provides a callback when the user moves the Video Post time slider.
This happens internally, developers must only respond to the
<b>FLT_TIMECHANGED</b> message. See \ref imageFilterRelatedMessages.
\par Data Members:
<b>BOOL set;</b>\n\n
Indicate the callback is register with 3ds Max.\n\n
<b>ImageFilter *filter;</b>\n\n
Points to the filter who's notified on the time change.  */
class TimeChange : public TimeChangeCallback {
   public:
	  BOOL set;
	  /*! \remarks Constructor. The data member <b>set</b> is made FALSE. */
	  TimeChange () { set = FALSE; }
	  ImageFilter *filter;
	  /*! \remarks This method is called the when
	       the current animation time is changed
	  \par Parameters:
	  <b>TimeValue t</b>\n\n
	  The current time (position of the time slider). */
	  void TimeChanged(TimeValue t);
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//--   Filter Info
//
   
enum MaskType {
   MASK_R = 0,  
   MASK_G, 
   MASK_B, 
   MASK_A, 
   MASK_L, 
   MASK_Z, 
   MASK_MTL_ID,
   MASK_NODE_ID

};

#define NUMMASKFLAGS (MASK_NODE_ID - MASK_R) + 1

/*! \sa  Class ImageFilter, Class BitmapInfo, Class FrameRange, Class ITrackViewNode, Class Class_ID.\n\n
\par Description:
This class provides information to an image filter plug-in. This is information
such as state of any masks used, and the various frame ranges for the video
post queue. It is analogous to the BitmapInfo class in the Bitmap Manager.
\par Data Members:
<b>BOOL maskenabled;</b>\n\n
TRUE if the filter has a mask; otherwise FALSE.\n\n
<b>BOOL evCopy;</b>\n\n
This is used internally as a flag indicating this object is a temporary copy,
and not the real thing. It is only an issue when filters have Track %View
Nodes. This is only used internally.\n\n
<b>BOOL invertedmask;</b>\n\n
TRUE if the mask is inverted; otherwise FALSE.\n\n
<b>BitmapInfo mask;</b>\n\n
The image used as the mask.\n\n
<b>WORD maskflag;</b>\n\n
This is used internally. It indicates what part of the mask image is used to
create the grayscale mask. It may be one of the following values: <b>MASK_R,
MASK_G, MASK_B, MASK_A, MASK_L, MASK_Z, MASK_MTL_ID, MASK_NODE_ID</b>.\n\n
<b>BitmapInfo imgQueue;</b>\n\n
This is a BitmapInfo that holds information about the current Video Post main
queue image buffer. This can be used to get Video Post's (or the target
image's) resolution, etc.\n\n
<b>FrameRange QueueRange;</b>\n\n
This defines the entire Video Post Queue range. This is the range defined
between VP Start Time and VP End Time in the video post user interface.\n\n
<b>FrameRange ExecutionRange;</b>\n\n
When the queue is executed, this is the range of frames being rendered.\n\n
<b>FrameRange FilterRange;</b>\n\n
The FilterRange is where this filter starts and ends.  */
class ImageFilterInfo: public MaxHeapOperators {

	  //-- Name of the filter used internally for identitification.

	  MCHAR            name[MAXFILTERNAME];

	  //-- Filters may want to identify themselves by something more  
	  //   specific than their names. Specially filters that give names
	  //   to parameter sets. If "resource" below is not empty, it
	  //   will be used to identify the filter in the Video Post Queue.
	  //   This is saved along with everything else by the host (Max).
	  //   If all the filter needs is a resource to identify a set of
	  //   parameters, this will sufice.

	  MCHAR            resource[MAXRESOURCE];

	  //-- Plug-In Parameter Block ------------------------------------------
	  //
	  //    No direct access to clients. Use the  methods in the  filter class.
	  //

	  void             *pidata;
	  DWORD            pisize;

	  //-- New R2 Stuff

	  MCHAR       *userlabel;    //-- Optional label given by user
	  ITrackViewNode *node;         //-- TV Node (if any)
	  Class_ID    nodeid;        //-- TV Node ID (if any);

	  int            flttype;

   public:

	  FLTExport        ImageFilterInfo                  ( );
	  FLTExport       ~ImageFilterInfo                  ( );

	  //-- Mask Information -------------------------------------------------

	  BOOL                         maskenabled,evCopy;
	  BOOL                         invertedmask;
	  BitmapInfo                   mask;
	  WORD                         maskflag;
	  
	  //-- This is a BitmapInfo that holds information about the current 
	  //   Video Post main queue Image buffer. This can be used to get
	  //   VP's (or target image) resolution, etc. To make an analogy, if
	  //   this was a BitmapIO plug-in, this is the BitmapInfo given as
	  //   the argument. This used primarilly at the time the filter
	  //   receives the "Setup()" call as at render time, all this can be
	  //   found in srcmap.

	  BitmapInfo                   imgQueue;

	  //-- Internal Helpers -------------------------------------------------

	  FLTExport void              SetName         ( const MCHAR *n );
	  /*! \remarks Filters may want to identify themselves by something more
	  specific than their names when they appear in the video post queue. By
	  default, the name of a filter is used to identify it in the video post
	  queue, ie the Negative filter appears as Negative. Some filters may want
	  a more descriptive name to appear. For instance a gradient filter that
	  allows the user to save named settings may want the name of the set to
	  appear rather than simply the name of the filter itself. Thus, "Flowing
	  gradient - Red to Blue" may appear rather than "Gradient". This method is
	  available for filters that that give such names to parameter sets. If not
	  empty, the resource name will be used to identify the filter in the Video
	  Post Queue. This is saved along with everything else by the system (3ds
	  Max).
	  \par Parameters:
	  <b>const MCHAR *n</b>\n\n
	  The name to appear, instead of the filter name, in the video post queue.
	  */
	  FLTExport void              SetResource     ( const MCHAR *n );
	  /*! \remarks Returns the name of the filter.
	  \par Operators:
	  */
	  FLTExport const MCHAR      *Name           ( )   { return    (const MCHAR *)name;}
	  /*! \remarks Returns the resource name. */
	  FLTExport const MCHAR      *Resource       ( )   { return    (const MCHAR *)resource;}
	  
	  //-- Plug-In Parameter Block ------------------------------------------
	  
	  FLTExport void              *GetPiData      ( ) { return pidata; }
	  FLTExport void              SetPiData       ( void    *ptr ) { pidata = ptr; }
	  FLTExport DWORD            GetPiDataSize   ( )   { return    pisize; }
	  FLTExport void              SetPiDataSize   ( DWORD s ) { pisize = s; }
	  FLTExport void              ResetPiData     ( );
	  FLTExport BOOL              AllocPiData     ( DWORD size  );

	  /*! \remarks Assignment operator.
	  \par Parameters:
	  <b>ImageFilterInfo \&from</b>\n\n
	  The source ImageFilterInfo. */
	  FLTExport ImageFilterInfo &operator= (  ImageFilterInfo &from );
	  
	  //-- Load/Save
	  
	  FLTExport IOResult         Save            ( ISave *isave );
	  FLTExport IOResult         Load            ( ILoad *iload, Interface *max );
	  
	  //-- Execution  Info ---------------------------------------------------
	  //
	  //    12/06/95 - GG
	  //
	  //    QueueRange defines    the entire Video Post Queue range. Execution
	  //    is only the portion being rendered. This is, unless   the user    selects
	  //    a "range", the same as QueueRange. FilterRange is where this  filter
	  //    starts    and ends.
	  //
	  //    Video Post Queue
	  //
	  //              1         2         3         4         5
	  //    0----|----|----|----|----|----|----|----|----|----|----|----|----|---- ...
	  //
	  //    Video Post spans from 0 to 49 (QueueRange) Start: 0  End: 49
	  //
	  //    qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq
	  //
	  //    User executes a "range" from 10 to 30 (Execution Range) Start: 10 End: 30
	  //
	  //                  uuuuuuuuuuuuuuuuuuuuu
	  //
	  //    This filter appears in the queue from 5 to 35 (Filter Range) Start: 5 End: 35
	  //
	  //           fffffffffffffffffffffffffffffff        

	  FrameRange                   QueueRange;              //-- Entire Video Post Queue
	  FrameRange                   ExecutionRange;          //-- Segement being rendered
	  FrameRange                   FilterRange;             //-- Filter Segment
	  
	  //----------------------------------------------------------------------
	  //-- R2 Stuff Below ----------------------------------------------------
	  //----------------------------------------------------------------------
   
	  //-- Trackview Node Functions ------------------------------------------

	  /*! \remarks	  This method is used to return the Track %View node for this filter.
	  Because Video Post Filter plug-ins have a short life, in other words,
	  they are only loaded when they are actually needed and deleted right
	  after, the Track %View node information is kept in the
	  <b>ImageFilterInfo</b> class kept by Video Post for each filter event. */
	  FLTExport ITrackViewNode   *Node ( )            { return node; }
	  /*! \remarks	  This method sets the Track %View node associated with this ImageFilter.
	  \par Parameters:
	  <b>ITrackViewNode *n</b>\n\n
	  The Track %View node to set. */
	  FLTExport void          SetNode (ITrackViewNode *n) { node = n;      }

	  /*! \remarks	  Returns the Class_ID of the Track %View node (if any). */
	  FLTExport Class_ID         NodeID      ( )            { return nodeid;}
	  /*! \remarks	  Sets the stored Class_ID of the Track %View node (if any).
	  \par Parameters:
	  <b>Class_ID id</b>\n\n
	  The id to set. */
	  FLTExport void          SetNodeID   ( Class_ID id )   { nodeid = id;  }

	  //-- Optional Label given by user while adding or editing a filter. This label
	  //   replaces the filter's name in Video Post's tracks for easier identification.

	  /*! \remarks	  Returns the optional label entered by the user while adding or editing a
	  filter. This label replaces the filter's name in Video Post's tracks in
	  Track %View for easier identification. This is the name that is entered
	  in the 'Edit Filter Event' dialog Filter Plug-In Label field. The label
	  defaults to Unnamed in which case the Filter's name appears (for example
	  'Negative'). */
	  FLTExport MCHAR            *UserLabel     ( )         { return userlabel; }

	  //-- Used by VP to update the label. Not to be used by filters.

	  FLTExport void          SetUserLabel   ( MCHAR *l) { userlabel = l; }

	  //-- Used to determine what type of filter this is at "Setup" time.

	  #define  FLT_FILTER  0
	  #define  FLT_LAYER   1

	  /*! \remarks	  This method is used so dual mode filters can detect what mode they are
	  running in (as a filter or as a compositor). In the 3ds Max 1.x SDK,
	  filters that ran both as filters and layers had no way to determine what
	  mode they were running while in "Setup" mode (in <b>ShowControl()</b>).
	  At run time they would check for a foreground bitmap. If it was NULL,
	  they were to assume they were running as simple filters. Now this method
	  may be used to determine what mode they are running in.
	  \return  One of the following values:\n\n
	  <b>FLT_FILTER</b>\n\n
	  <b>FLT_LAYER</b> */
	  FLTExport int           FilterType     ( )   { return flttype; }
	  /*! \remarks	  This method is used internally. */
	  FLTExport void          SetFilterType  ( int type ) { flttype = type; }

};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//--   Filter Plug-Ins Handler
//
   
/*! \sa  Class ImageFilter, Class ClassDesc, \ref imageFilterCapabilityFlags.\n\n
\par Description:
This class provides methods that access properties of the image filter plug-in.
All methods of this class are implemented by the system.  */
class FLT_FilterHandler: public MaxHeapOperators {

	  //-- Name and Capabilities  ------------------------
	  
	  MCHAR         fltDescription[MAXFILTERNAME];    
	  DWORD         fltCapability;

	  //-- DLL    Handler ----------------------------------
	  
	  ClassDesc *cd;
	  
   public:

	  /*! \remarks Constructor. The capability flags are set to 0, the
	  description string is set to NULL, and the class descriptor is set to
	  NULL. */
	  FLT_FilterHandler();
	  
	  /*! \remarks Sets the description string associated with a ImageFilter
	  plug-in.
	  \par Parameters:
	  <b>const MCHAR *d = NULL</b>\n\n
	  The string to set.
	  \return  The string that was set. */
	  FLTExport MCHAR           *Description        ( const MCHAR  *d = NULL  );

	  /*! \remarks Sets the class descriptor associated with the ImageFilter.
	  \par Parameters:
	  <b>ClassDesc *dll</b>\n\n
	  The class descriptor to set. */
	  FLTExport void             SetCD              ( ClassDesc *dll )    { cd = dll;}
	  /*! \remarks Returns a pointer to the class descriptor. */
	  FLTExport ClassDesc       *GetCD              ( )                   { return    cd;}

	  /*! \remarks Sets the capability flag. The flags passed are ORed into
	  the existing flags.
	  \par Parameters:
	  <b>DWORD cap</b>\n\n
	  The flags to set. */
	  FLTExport void             SetCapabilities  ( DWORD cap )      { fltCapability |= cap;}
	  /*! \remarks Returns the capability flags. */
	  FLTExport DWORD            GetCapabilities  ( )                { return    (fltCapability);}
	  /*! \remarks Tests the flags passed to see if they are set in the
	  capability flags.
	  \par Parameters:
	  <b>DWORD cap</b>\n\n
	  The flags to test.
	  \return  TRUE if the flags are set; otherwise FALSE. */
	  FLTExport BOOL             TestCapabilities ( DWORD cap )      { return    (fltCapability  & cap);}

};

//-----------------------------------------------------------------------------
//--   Messages    sent back by various    (client)    methods

/*! \defgroup imageFilterRelatedMessages ImageFilter-Related Messages
Sent by the plug-in to notify host of current progress. The host should
return TRUE if it's ok to continue or FALSE to abort process. Messages can be sent 
using SendMessage() as follows:
\code
LRESULT SendMessage(
  HWND hwnd,		// handle of destination window
  UINT uMsg,		// message to send
  WPARAM wParam,	// first message parameter
  LPARAM lParam		// second message parameter
);
\endcode
/sa Class ImageFilter.
*/
//@{
#define    FLT_PROGRESS    WM_USER + 0x20	//!< wParam: Current lParam: Total
/*! Sent by the plug-in to check for process interruption. The host should
return FALSE (by setting *lParam)if it's ok to continue or TRUE to abort process. */
#define  FLT_CHECKABORT WM_USER + 0x21		//!< wParam: 0 lParam: BOOL*
/*! Sent by the plug-in to display an optional textual message (for progress report). 
\code
BOOL ImageFilter_Negative::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
  switch (message) {
    case FLT_UNDO:
    undo = true;
    break;
  case FLT_TIMECHANGED:
    // . . .
\endcode
*/
#define  FLT_TEXTMSG    WM_USER + 0x22		//!< wParam: 0 lParam: LPCMSTR
/*! Sent by the host TO the plug-in to notify the time has changed (the user
moved the time slider in Max). */
#define    FLT_TIMECHANGED WM_USER + 0x23	//!< wParam: 0 lParam: TimeValue t
/*! Sent by 3ds Max TO the plug-in to notify that an Undo operation has been done.
The plugin will set some boolean internally and wait for the next WM_PAINT message
to update any spinners or other values that may have been undone. The filter manager
sends this message (if you register for the notification with RegisterTVNodeNotify())
and an undo operation was performed. */
#define    FLT_UNDO     WM_USER + 0x24		//!< wParam: 0 lParam: 0
//@}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//--   List of Filter  Plug-Ins
//
   
/*! \sa  Class FilterHandler, Template Class Tab, \ref imageFilterCapabilityFlags.\n\n
\par Description:
This class provides methods to access the table of image filters. Methods are
provided to find the index in the table of the filter using it description
string, check the capabilities of any filter in the table, and to create
instances of any filters in the table. All methods of this class are
implemented by the system.  */
class FLT_FilterList: public   Tab<FLT_FilterHandler> {

	  BOOL        listed;
	   
   public:

	  /*! \remarks Constructor. The flag that indicates the list of filters
	  has been built is set to FALSE. */
	  FLT_FilterList        ( )           { listed    = FALSE;    }

	  /*! \remarks Sets the state to indicate the list of filter plug-ins has
	  been built.
	  \par Parameters:
	  <b>BOOL f</b>\n\n
	  TRUE indicates the list has been built; FALSE indicates this list has not
	  been built.
	  \return  The state that was set is returned. */
	  BOOL        Listed    ( BOOL    f)  { listed    = f; return (listed);};
	  /*! \remarks Returns TRUE if the list of filter plug-ins has been built;
	  otherwise FALSE. */
	  BOOL        Listed    ( )           { return    (listed);};

	  /*! \remarks Returns the index in the filter table of the image filter
	  whose description string is passed.
	  \par Parameters:
	  <b>const MCHAR *name</b>\n\n
	  The Description string of the filter to find.
	  \return  If found, the index in the table is returned; otherwise -1. */
	  FLTExport int    FindFilter              ( const MCHAR *name );
	  /*! \remarks Returns the capability flags associated with the image
	  filter whose description is passed.
	  \par Parameters:
	  <b>const MCHAR *name</b>\n\n
	  The Description string of the filter.
	  \return  The capability flags. If not found, 0 is returned. */
	  FLTExport DWORD  GetFilterCapabilities   ( const MCHAR *name );

	  //-- This Creates   an  Instance    - Make sure to  "delete"    it  after   use.

	  /*! \remarks This method creates an instance of the ImageFilter class.
	  This method will call <b>Create()</b> on the class descriptor. Make sure
	  to delete this instance after use.
	  \par Parameters:
	  <b>const MCHAR *d</b>\n\n
	  The Description string of the filter to create.
	  \return  An instance of the ImageFilter class. The developer is
	  responsible for deleting this instance. */
	  FLTExport ImageFilter *CreateFilterInstance(const MCHAR *d);

};

//-----------------------------------------------------------------------------
//-- Undo Notification

/*! \sa  Class TVNodeNotify.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class can be used so an <b>ImageFilter</b> plug-in can get notified on a
change to one of its Track %View Nodes.\n\n
This class provides an implementation of the <b>NotifyRefChanged()</b> method
of class <b>TVNodeNotify</b>. The constructor of this class stores a window
handle. Usually this is the control dialog window handle of the ImageFilter
plug-in using this class. Upon receipt of a message via
<b>TVNodeNotify::NotifyRefChanged()</b> this implementation sends a
<b>FLT_UNDO</b> message to the ImageFilter control dialog window proc and
invalidates the window. Most filters will set a flag indicating that an undo
has occurred when they get the <b>FLT_UNDO</b> message, and actually update the
UI controls when they process the <b>WM_PAINT</b> message. This is because the
<b>FLT_UNDO</b> message may be sent many time and the controls shouldn't be
updated each time (as they might appear to 'flicker'). See the code for the
Negative filter in <b>/MAXSDK/SAMPLES/FILTERS/NEGATIVE/NEGATIVE.CPP</b> for
details.\n\n
All methods of this class are implemented by the system.  */
#pragma warning(push)
#pragma warning(disable:4100)
class UndoNotify : public TVNodeNotify {
   HWND hWnd;
public:
   /*! \remarks Constructor.
   \par Parameters:
   <b>HWND hwnd</b>\n\n
   The control dialog window handle where the <b>FLT_UNDO</b> message will be
   sent. This window handle is also passed to <b>InvalidateRectangle()</b> so a
   <b>WM_PAINT</b> message will be sent. */
   UndoNotify (HWND hwnd) {hWnd = hwnd;}
   RefResult NotifyRefChanged (Interval changeInt, RefTargetHandle hTarget, 
		 PartID& partID,  RefMessage message) {
		 SendMessage(hWnd,FLT_UNDO,0,0);
		 InvalidateRect(hWnd,NULL,FALSE);
		 return(REF_SUCCEED);
   }
};
#pragma warning(pop)
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*! \defgroup imageFilterCapabilityFlags ImageFilter Capability Flags
It is valid for a plug-in to both Filter and Compositor. If both flags are
set, the user will be able to  select it from both the Filter list and from
the Compositor list. The plug-in will know it is running as a filter when
the foreground  map pointer is  NULL. */
//@{
#define IMGFLT_NONE              0		//!< None
#define IMGFLT_MASK              (1<<0)	//!< Supports Masking
/*! This informs the system to call the plug-ins ShowControl() method when the
user selects the Setup button. If the filter does not have a control panel do 
not set this bit and the setup button will be grayed out in the 3ds Max user
interface. */
#define IMGFLT_CONTROL           (1<<1)	//!< Plug-In has a Control Panel
#define IMGFLT_FILTER            (1<<2)	//!< Plug-In is a Filter
/*! If the plug-in is a layer type of filter, it should set this bit. */
#define IMGFLT_COMPOSITOR        (1<<3)	//!< Plug-In is a Compositor
/*! If this flag is NOT set, 3ds Max will avoid multithreading this plug-in. */
#define IMGFLT_THREADED          (1<<4)	//!< Thread aware plug-in
//@}

//-- Class ID's for various DLL's

#define NEGATIVECLASSID 0x4655434A
#define ALPHACLASSID    0x655434A4
#define ADDCLASSID      0x55434A46
#define BLURCLASSID     0x5434A465
#define CROSFADECLASSID 0x434A4655
#define GLOWCLASSID     0x35A46554
#define COOKIECLASSID   0x4A465543
#define WIPECLASSID     0xA4655434
#define FADECLASSID     0x4655434B
#define PDALPHACLASSID  0x655434B4

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//--   Image   Filter Class
//

/*! \sa  Class ImageFilterInfo, Class Bitmap, Class ITrackViewNode, Class TimeChange, Class UndoNotify, <a href="ms-its:3dsmaxsdk.chm::/bitmaps_root.html">Working with
Bitmaps</a>.\n\n
\par Description:
Image processing filter plug-ins are derived from <b>ImageFilter</b>. This
class has virtual methods the developer implements to provide information about
the plug-in version, and description. The developer also implements the
<b>Capability()</b> method to indicate the properties of the plug-in such as if
it is a one pass filter or compositor, and whether it has a control dialog to
be displayed.\n\n
The <b>Render()</b> method is the one that actually alters the source image to
perform the work of the application.\n\n
Filter plug-ins have access to several bitmaps associated with the video post
data stream. All filter plug-ins will have at least a pointer to data member
<b>srcmap</b>. This is Video Post's main image pipeline. Composition and
transition (layer) filters will also receive a second bitmap (<b>frgmap</b>)
which should be composited above the main bitmap (<b>srcmap)</b>. If
<b>mskmap</b> is not NULL, it will contain a pointer to a grayscale image to be
used as a mask for the process. Note that developers should not delete these
bitmaps as they are maintained by 3ds Max.\n\n
If a plug-in is both a filter and a compositor, the plug-in can tell if it is
running as a filter when the <b>frgmap</b> pointer is NULL.\n\n
Note: If changes are made to an <b>ImageFilter</b> plug-in, the system will not
automatically put up the 'The scene has been modified. Do you want to save your
changes?' dialog if the user attempts to exit without saving. So that your
plug-in does not go unsaved, you should call the following global function if
you make changes. This will indicate to the system that the save requester
needs to be brought up:\n\n
<b>void SetSaveRequired(int b=TRUE);</b>\n\n
Sets the 'save dirty bit'. This will indicate to the system that the save
requester needs to be presented to the user.
\par Method Groups:
See Method Groups for Class ImageFilter.
\par Data Members:
protected:\n\n
<b>BOOL interactive;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
TRUE if the setup dialog is interactive; otherwise FALSE.\n\n
<b>HWND vpSetuphWnd;</b>\n\n
The window handle of the video post setup dialog.\n\n
<b>HWND vphWnd;</b>\n\n
The window handle of the Video Post dialog.\n\n
<b>HWND dlghWnd;</b>\n\n
The window handle of the filter's setup dialogue when in "Interactive"
mode.\n\n
<b>Bitmap *srcmap</b>\n\n
The Source Bitmap (background). Note: The Video Post output resolution may be
retrieved using this pointer. The width is <b>srcmap-\>Width()</b> and the
height is <b>srcmap-\>Height()</b>.\n\n
<b>Bitmap *mskmap</b>\n\n
The Image Mask Bitmap (for grayscale masking). This bitmap is at the Video Post
output resolution size when the developer needs to access it in the
<b>Render()</b> method.\n\n
<b>Bitmap *frgmap</b>\n\n
The Foreground Bitmap (for layering/transitions). This bitmap is at the Video
Post output resolution size when the developer needs to access it in the
<b>Render()</b> method.\n\n
<b>ImageFilterInfo *ifi</b>\n\n
A pointer to an instance of the class that provides information about this
filter and the video post queue.\n\n
<b>TimeChange timeChange;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
This class maintains this time change object so it may send
<b>FLT_TIMECHANGED</b> messages.\n\n
<b>UndoNotify* undonotify;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
Points to an instance of the class that can be used so an <b>ImageFilter</b>
plug-in can get notified on a change to its Track %View Node.  */
#pragma warning(push)
#pragma warning(disable:4100)
class ImageFilter: public MaxHeapOperators {
   
   protected:

	  BOOL  interactive;
	  HWND  vpSetuphWnd,vphWnd,dlghWnd;

	  //-- Bitmap Pointers --------------------------------------------------
	  //
	  // All filters will have at least a pointer to "srcmap". This is VP's
	  // (or any other process') main image pipeline.
	  //
	  // Composition filters will also receive a second [frgmap] bitmap
	  // which should be composited above the main [srcmap] bitmap.
	  //
	  // If "mskmap" is not NULL, it will contain a pointer to a grayscale
	  // image to be used as a mask for the process.
	  //
	  // 12/06/95 - GG
	  //
	  // The srcmap (Background) is the Video Post queue bitmap. Use its
	  // methods to find out dimmensions (width, height, aspect ratio, etc.)
	  // If the queue is using Alpha channel, it will be noted in the bitmap
	  // flags (srcmap). The same is true for Z and G buffers. Again, simply
	  // use the bitmap methods to access these.
	  //
	  
	  Bitmap                      *srcmap;         //--   Source (Background)
	  Bitmap                      *mskmap;         //--   Mask (Grayscale Masking)
	  Bitmap                      *frgmap;         //--   Foreground (for layering/transitions)
	  
	  //-- Set    by  Host ----------------------------------
	  
	  ImageFilterInfo         *ifi;
	  
   public:
   
	  /*! \remarks Constructor. The <b>srcmap, mskmap</b> and <b>frgmap</b>
	  are set to NULL. The undo notify pointer is set to NULL and the
	  interactive flag is set to FALSE. */
	  FLTExport   ImageFilter     ( );
	  /*! \remarks Destructor. */
	  FLTExport virtual ~ImageFilter   ( );
	  
	  //-- Filter Info    ---------------------------------
	  
	  /*! \remarks Returns an ASCII description of the filter plug-in (i.e. "Convolution
	  Filter"). */
	  FLTExport virtual     const MCHAR    *Description      ( ) = 0; // ASCII description (i.e. "Convolution Filter")
	  /*! \remarks Returns the name of the plug-in's author. */
	  FLTExport virtual     const MCHAR    *AuthorName       ( ) = 0; // ASCII Author name
	  /*! \remarks Implemented by the Plug-In\n\n
	  Returns the plug-in ASCII Copyright message. */
	  FLTExport virtual     const MCHAR    *CopyrightMessage ( ) = 0; // ASCII Copyright message
	  /*! \remarks Returns the plug-in version number * 100 (i.e. v3.01 = 301). */
	  FLTExport virtual     UINT            Version          ( ) = 0; // Version number * 100 (i.e. v3.01 = 301)
	  /*! \remarks This method returns a set of flags that describe the capabilities of this
	  filter plug-in. These capabilities indicate if the plug-in is a filter,
	  compositor, or has a control panel. To create a flag, "OR" the
	  capabilities together, ie. (<b>IMGFLT_CONTROL | IMGFLT_COMPOSITOR</b>).
	  Note: It is valid for a plug-in to both a Filter and a Compositor. If
	  both flags are set, the user will be able to select it from both the
	  Filter list and from the Compositor list. The plug-in will know it is
	  running as a filter when the foreground map pointer, <b>frgmap</b>, is
	  NULL.
	  \return  See \ref imageFilterCapabilityFlags. */
	  FLTExport virtual     DWORD           Capability       ( ) = 0; // Returns capability flags (see above)

	  //-- Dialogs ----------------------------------------------------------
	  //
	  //    An About Box  is  mandatory. The  Control panel is optional and   its 
	  //    existence should be flagged   by  the Capability  flag above.
	  //

	  /*! \remarks This method is called by the system to display the About Box of the
	  plug-in. This dialog is mandatory so the developer must implement this
	  method.
	  \par Parameters:
	  <b>HWND hWnd</b>\n\n
	  The parent window handle of the dialog. */
	  FLTExport virtual     void            ShowAbout         ( HWND    hWnd ) =    0;
	  /*! \remarks This method is called by the system to display the control panel for the
	  plug-in. This control panel is optional and its existence should be
	  flagged by the capability flag returned from <b>Capability()</b>
	  (<b>IMGFLT_CONTROL</b>). If a plug-in does not have a control panel,
	  don't implement this method and let it default to FALSE.
	  \par Parameters:
	  <b>HWND hWnd</b>\n\n
	  The parent window handle of the dialog.
	  \return  TRUE if the user selects OK to exit the dialog, and FALSE if the
	  user selects Cancel.
	  \par Default Implementation:
	  <b>{ return FALSE; }</b> */
	  FLTExport virtual     BOOL            ShowControl       ( HWND    hWnd ) {    return FALSE; }

	  //-- Parameter  Setting (Host's Responsability) ----

	  FLTExport virtual     void            SetSource         ( Bitmap *map )     {srcmap = map;}
	  FLTExport virtual     void            SetForeground     ( Bitmap *map )     {frgmap = map;}
	  FLTExport virtual     void            SetMask           ( Bitmap *map )     {mskmap = map;}
	  FLTExport virtual     void            SetFilterInfo     ( ImageFilterInfo *i ) {ifi   = i;}

	  //-- Execution  ------------------------------------
	  //
	  //    The   "hWnd" argument is a    window handler  to  which
	  //    the   plug-in will be sending messages.

	  /*! \remarks This is the method the plug-in implements to alter the image. This method
	  performs the work of the filter or compositor.
	  \par Parameters:
	  <b>HWND hWnd</b>\n\n
	  The window handle to which the plug-in will be sending progress and abort
	  check messages.
	  \return  TRUE if the render was completed; otherwise FALSE (error or
	  canceled by user).
	  \par Sample Code:
	  Below is an example of a render loop through each horizontal band of the
	  image demonstrating the posting of messages. At the start of the loop the
	  progress and check abort messages are sent. The progress message updates
	  the Execute Video Post dialog with how much of the image has been
	  processed. The check abort message allows the plug-in to detect if the
	  user has canceled the operation.\n\n
	\code
	BOOL result = TRUE;
	BOOL abort = FALSE;
	for (int iy = 0; iy Height(); iy++)
	{
	// Progress Report
		SendMessage(hWnd,FLT_PROGRESS,iy,srcmap->Height()-1);
	// Check for Abort
		SendMessage(hWnd,FLT_CHECKABORT,0,(LPARAM)(BOOL )&abort);
		if (abort) {
			result = FALSE;
			break;
		}
	}
	
	return(result);
	\endcode
	  Messages are sent via the Window API <b>SendMessage()</b> function. See
	  \ref imageFilterRelatedMessages. */
	  FLTExport virtual     BOOL            Render            ( HWND    hWnd ) =    0;

	  //-- Max    Interface ----------------------------------------------------
	  //
	  //    Some of Max's core functions exported through the Interface class.
	  //

	  /*! \remarks Implemented by the System.\n\n
	  This method returns an interface pointer for calling methods implemented
	  in 3ds Max. See Class Interface. */
	  FLTExport virtual  Interface *Max  ( );

	  //-- Helpers --------------------------------------

	  /*! \remarks This method is used internally. */
	  FLTExport virtual  int     Lerp    (int a, int b, int l);
	  /*! \remarks This method is used internally. */
	  FLTExport virtual  int     Lerp    (int a, int b, float f);
	  
	  //-- Parameter  Block   Load and    Save ------------------------------------
	  //
	  //   The host will  call EvaluateConfigure() to determine the   buffer size
	  //   required by the plug-in.
	  //
	  //   SaveConfigure() will be called so the  plug-in can transfer    its
	  //   parameter block to the host ( ptr is a pre-allocated   buffer).
	  //
	  //   LoadConfigure() will be called so the  plug-in can load its    
	  //   parameter block back.
	  //   
	  //   Memory management is performed by the  host using standard
	  //   LocalAlloc() and   LocalFree().
	  //   
	  
	  /*! \remarks The system will call this method to determine the buffer size required by
	  the plug-in. The plug-in can save its parameter block in this buffer by
	  implementing the <b>SaveConfigure()</b> method.
	  \return  The number of bytes required by the plug-in's parameter block.
	  \par Default Implementation:
	  <b>{ return 0; }</b> */
	  FLTExport virtual  DWORD   EvaluateConfigure  ( )           { return 0; }
	  /*! \remarks This method will be called so the plug-in can load its parameter block.
	  Memory management is performed by 3ds Max using standard
	  <b>LocalAlloc()</b> and <b>LocalFree()</b>.
	  \par Parameters:
	  <b>void *ptr</b>\n\n
	  A pre-allocated buffer.
	  \return  TRUE if the data was loaded OK; otherwise FALSE.
	  \par Default Implementation:
	  <b>{ return (FALSE) };</b> */
	  FLTExport virtual  BOOL    LoadConfigure      ( void *ptr ) { return (FALSE); }
	  /*! \remarks This method will be called so the plug-in can transfer its parameter
	  block to the host.
	  \par Parameters:
	  <b>void *ptr</b>\n\n
	  A pre-allocated buffer the plug-in may write to.
	  \return  TRUE if the data was saved OK; otherwise FALSE.
	  \par Default Implementation:
	  <b>{ return (FALSE); }</b> */
	  FLTExport virtual  BOOL    SaveConfigure      ( void *ptr ) { return (FALSE); }

	  //-- Preview Facility -------------------------------------------------
	  //
	  //    This is used  by  plug-ins    that want to have   a preview bitmap while
	  //    displaying its control dialogue.
	  //
	  //    The   flag controls how   much of the queue   to  run:
	  //
	  //    PREVIEW_BEFORE - The queue is run up  to  the event before the    filter
	  //    calling it.
	  //
	  //    PREVIEW_UP ----- The queue is run up  to  the event (filter) calling
	  //    this function.
	  //
	  //    PREVIEW_WHOLE -- The whole queue is run   including events after
	  //    this filter.
	  //
	  //    The   given   frame   is  the Video Post  Queue   frame   number and not  Max's
	  //    frame number.
	  //
	  //
	  //    Parameters:
	  //
	  //    hWnd -    WIndow handle to send messages to. These are    the progress,
	  //    check for abort, text messages    etc. If the plug in wants to support
	  //    a cancel button   and progress bars   etc, it must handle these messages.
	  //    It is Ok to send a    NULL window handle in which case    nothing is checked.
	  //
	  //    back -    Pointer to a Bitmap pointer. If the Bitmap pointer  is  NULL,   a
	  //    new   bitmap is created   using   the given dimmensions. This pointer must be
	  //    NULL the first time this  function    is  called as the bitmap    must be
	  //    created by Video Post. Once   this function is called and a   bitmap is
	  //    returned, it  is  ok  to  call it again using this map.   In  this case, Video
	  //    Post will simply use it instead of creating a new one.    You must    delete
	  //    the   bitmap when done.
	  //
	  //    fore -    For layer plug-ins, this points to the  foreground image.   This is
	  //    only valid if flag    is  set to PREVIEW_BEFORE. In this case back will hold  
	  //    Video Post main   queue   and fore    will have the foreground image to be 
	  //    composited. This is usefull   if  you, a layer plug-in, want  to  collect the 
	  //    images    and run a real  time preview. If flag is not PREVIEW_BEFORE,    fore
	  //    will be a NULL pointer indicating there   is  no  bitmap.
	  //
	  //    frame - The desired frame. Make sure  you request a frame within  the
	  //    range your plug-in    is  active.
	  //
	  //    width & height - Self explanatory.
	  //
	  //    flag -    Explained above.
	  //

	  #ifndef PREVIEW_BEFORE
	  #define PREVIEW_BEFORE  1
	  #define PREVIEW_UP      2
	  #define PREVIEW_WHOLE   3
	  #endif

		/*! \remarks Implemented by the System.\n\n
		This method provides a preview facility for use by plug-ins. This can be used
		by plug-ins that want to have a preview bitmap while displaying a control
		dialog.\n\n
		Note: When you add a new Video Post event, an event object is created and it is
		added to the event queue when you hit the OK button. However, because the event
		is added to the queue only when you exit the dialogue, you cannot create a
		preview at that stage. Later, when you "Edit" the event, it is part of the
		queue and you can create a preview. Internally, there is no way for 3ds Max to
		tell if the "Setup" button was called from an "Add" event or from an "Edit"
		event. Plug-In developers can tell if they are in the "Add" event mode by
		looking at the return value from this method. It will be FALSE if in "Add" mode
		since the call will fail.
		\par Parameters:
		<b>HWND hWnd</b>\n\n
		This window handle will receive progress notification messages. The messages
		are defined in both BITMAP.H and FILTERS.H:\n\n
		<b>FLT_PROGRESS</b>\n\n
		wParam: Current\n\n
		lParam: Total\n\n
		<b>FLT_CHECKABORT</b>\n\n
		wParam: 0\n\n
		lParam: BOOL*\n\n
		<b>FLT_TEXTMSG</b>\n\n
		wParam: 0\n\n
		lParam: LPCMSTR\n\n
		<b>BMM_PROGRESS</b>\n\n
		wParam: Current\n\n
		lParam: Total\n\n
		<b>BMM_CHECKABORT</b>\n\n
		wParam: 0\n\n
		lParam: *BOOL\n\n
		<b>BMM_TEXTMSG</b>\n\n
		wParam: 0\n\n
		lParam: LPCMSTR\n\n
		<b>Bitmap **back</b>\n\n
		A pointer to the Bitmap Pointer (the Background). If the Bitmap pointer is
		NULL, a new bitmap is created using the given dimensions. This pointer must be
		NULL the first time this function is called as the bitmap must be created by
		Video Post. Once this function is called and a bitmap is returned, it is OK to
		call it again using this map. In this case, Video Post will simply use it
		instead of creating a new one. Note: You must NOT delete the bitmap when done
		-- Video Post will take care of it.\n\n
		<b>int frame</b>\n\n
		The desired frame in TICKS. Note that this is the position of the Video Post
		frame slider (in TICKS) and not the main 3ds Max frame slider. See the Advanced
		Topics section on
		<a href="ms-its:3dsmaxsdk.chm::/anim_time_functions.html">Time</a> for details
		on ticks.\n\n
		<b>int width</b>\n\n
		The desired width of the preview.\n\n
		<b>int height</b>\n\n
		The desired height of the preview.\n\n
		<b>Float aspect</b>\n\n
		The desired aspect ratio of the preview.\n\n
		<b>Bitmap **fore = NULL</b>\n\n
		A pointer to the Bitmap Pointer (the Foreground). For layer plug-ins, this
		points to the foreground image. This is only valid if flag is set to
		<b>PREVIEW_BEFORE</b>. In this case <b>back</b> will hold Video Post main queue
		and <b>fore</b> will have the foreground image to be composited. This is useful
		if you, a layer plug-in, want to collect the images and run a realtime preview.
		If flag is not <b>PREVIEW_BEFORE</b>, <b>fore</b> will be a NULL pointer
		indicating there is no bitmap.\n\n
		<b>DWORD flag = PREVIEW_UP</b>\n\n
		The flag controls how much of the queue to run. The options are:\n\n
		<b>PREVIEW_BEFORE</b>\n\n
		The queue is run up to the event before the filter calling it.\n\n
		<b>PREVIEW_UP</b>\n\n
		The queue is run up to the event (filter) calling this function.\n\n
		<b>PREVIEW_WHOLE</b>\n\n
		The whole queue is run including events after this filter.
		\return  TRUE if the creation was successful; otherwise FALSE. */
	  FLTExport virtual    BOOL CreatePreview  ( 
			HWND hWnd,                      //-- Window handle to send  messages    to
			Bitmap **back,                  //-- Pointer to Bitmap Pointer (Background)
			int frame,                      //-- Desired Frame
			int width,                      //-- Desired Width
			int height,                     //-- Desired Height
			float   aspect,                 //-- Desired Aspect Ratio
			Bitmap **fore   = NULL,         //-- Pointer to Bitmap Pointer (Foreground)
			DWORD   flag    = PREVIEW_UP );

	  //----------------------------------------------------------------------
	  //-- Channels Required
	  //
	  //    By setting this   flag,   the plug-in can request the host    to  generate
	  //    the   given   channels. Prior to Rendering,   the host    will scan the
	  //    plug-ins in the   chain   of  events and list all types of channels
	  //    being requested. The plug-in, at the  time of the Render()    call,   
	  //    will have access to these channels through    the channel interface
	  //    described in  Bitmap.h    - BitmapStorage.
	  //
	  //    The   generation of these channels should not, normally,  be  a 
	  //    default setting   for a   plug-in.    These   channels    are memory hungry   and
	  //    if the    plug-in won't use   it, it should not   ask for it. Normally
	  //    the   plug-in would ask   the user    which   channels    to  use and set only
	  //    the   proper flags.
	  //
	  
	  /*! \remarks If a filter wants to work with the G-buffer (geometry/graphics buffer) it
	  implements this method. It is used to indicate what image channels this
	  plug-in requires. Prior to rendering 3ds Max will scan the plug-ins in
	  the chain of events and find out all the channels being requested. At the
	  time the plug-in's <b>Render()</b> method is called, it will have access
	  to these channels. The methods of class Bitmap may be used to access the
	  channels.\n\n
	  Note: The generation of these channels should not normally be a default
	  setting for the plug-in. These channels are memory intensive and if the
	  plug-in won't use the channel it should not ask for it. Normally the
	  plug-in would ask the user which channels to use and only then set the
	  proper flags.
	  \return  See \ref gBufImageChannels.
	  \par Default Implementation:
	  <b>{ return BMM_CHAN_NONE; }</b> */
	  FLTExport virtual     DWORD   ChannelsRequired       ( ) {   return BMM_CHAN_NONE; }
	  

	  //----------------------------------------------------------------------
	  //-- R2 Stuff Below ----------------------------------------------------
	  //----------------------------------------------------------------------
   
	  TimeChange  timeChange;
	  UndoNotify* undonotify;

	  FLTExport virtual HWND  DlgHandle         ( void ) { return dlghWnd; }

	  //-- Filter Control Dialogue Interactivity -----------------------------

	  /*! \remarks Implemented by the System.\n\n
	  
	  This method may be used to allow a filter's dialog to operate
	  interactively. This means that a user can have the filter's control
	  dialog up and still operate 3ds Max and Track %View at the same time.
	  This method should be called from the filter's <b>Control()</b> method as
	  part of the <b>WM_INITDIALOG</b> code.\n\n
	  Note that even though this method can be called safely for any reason,
	  developers should only call it when using animated parameters. It doesn't
	  make sense to use it for filters with non-animated or no parameters. For
	  sample code using this method see
	  <b>/MAXSDK/SAMPLES/POSTFILTERS/NEGATIVE/NEGATIVE.CPP</b>.
	  \par Parameters:
	  <b>HWND hWnd</b>\n\n
	  The parent window handle.
	  \par Sample Code:
	\code 
	BOOL ImageFilter_Negative::Control(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
		switch (message) {
			case WM_INITDIALOG: {
	   //-- Make Dialogue Interactive
			MakeDlgInteractive(hWnd);
		... 
\endcode      */
	  FLTExport virtual void  MakeDlgInteractive   ( HWND hWnd );
	  /*! \remarks	  Implemented by the System.\n\n
	  Returns TRUE if the control dialog is interactive; otherwise FALSE. This
	  means a user can have the filter's control dialog up and still operate
	  3ds Max and Track %View at the same time. */
	  FLTExport virtual BOOL  IsInteractive     ( void ) { return interactive; }

	  //-- Trackview Node Functions ------------------------------------------

	  /*! \remarks	  Implemented by the System.\n\n
	  This method may be called to create a new Track %View Node. */
	  FLTExport virtual ITrackViewNode *CreateNode ( );
	  /*! \remarks	  Implemented by the System.\n\n
	  This method is used to return the Track %View node for this filter. */
	  FLTExport virtual ITrackViewNode *Node ( ) { return ifi->Node(); }

	  //-- FilterUpdate() ----------------------------------------------------
	  //
	  // Whenever a filter instance is created or updated (i.e. the user went,
	  // through the Filter Edit Control dialogue) this is call is issued to 
	  // the filter. The filter may use it to create/update its node controls.
	  //
	  // See example in negative.cpp.

	  /*! \remarks	  Whenever a filter instance is created or updated (i.e. the user went
	  through the Filter Edit Control dialog) this method is called. The filter
	  may use it to create or update its node controls. For an example see
	  <b>/MAXSDK/SAMPLES/POSTFILTERS/NEGATIVE/NEGATIVE.CPP</b>.
	  \par Default Implementation:
	  <b>{}</b> */
	  FLTExport virtual void  FilterUpdate   ( ) { }

};
#pragma warning(pop)
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//--   Main Filter Manager Class
//
//

class FilterManager: public MaxHeapOperators    {
   
	  MCHAR                   name[MAXFILTERNAME];
	  FLTInterface            *iface;
	  ImageFilterInfo         *ifi;
	  Interface               *max;

	  //-- General Private    Methods
	  
	  BOOL                    SetupPlugIn                 ( HWND hWnd, WORD   item );
	  void                    HandleMaskFile              ( HWND hWnd, WORD   item );
	  
	  //-- Image Filter   Private Methods
	  
	  int                     GetCurrentFilter            ( HWND hWnd, MCHAR *plugin  );
	  void                    HandleFilterDialogState     ( HWND hWnd );

   public:
   
	  FLTExport                   FilterManager           ( FLTInterface  *i);
	  FLTExport                   FilterManager           ( FLTInterface  *i,const    MCHAR   *name);
	  FLTExport                   ~FilterManager          ( );
	  
	  FLTExport FLTInterface  *iFace                      ( ) {   return iface;}
	  
	  void                        DoConstruct             ( FLTInterface  *i,const    MCHAR   *name);
	  
	  FLT_FilterList              fltList;
	  FLTExport void              ListFilters             ( );
	  
	  FLTExport HINSTANCE         AppInst                 ( );
	  FLTExport HWND              AppWnd                  ( );
	  FLTExport DllDir            *AppDllDir              ( );
	  FLTExport Interface         *Max                    ( ) {   return max; }
	  
	  //-- User Interface -------------------------------

	  INT_PTR                 ImageFilterControl      ( HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam );
	  
	  //-- This function will create a    mask bitmap based
	  //    on the    given   ImageFilterInfo class.
	  
	  Bitmap                      *ProcessMask                    ( HWND hWnd, ImageFilterInfo    *ii );
	  
	  //-- This function will list all    available filter 
	  //    plug-ins. The "item" argument defines an id 
	  //    for   a combo box to  receive the list whithin 
	  //    the   hWnd context. It returns the number of  
	  //    filters found.
	  
	  FLTExport int               GetFilterList                   ( HWND hWnd, int item );
	  FLTExport int               GetLayerList                    ( HWND hWnd, int item );
	  
	  //-- This runs  the show. Thew  window handle is used
	  //    to send progress messages back. See above the
	  //    discussion about messages. The    host should
	  //    check keyboard and    cancel buttons  and return
	  //    FALSE to a FLT_PROGRESS or FLT_CHECKABORT
	  //    message telling   the Plug-In to  cancel.
	  
	  FLTExport BOOL              RenderFilter     ( HWND hWnd, 
										  ImageFilterInfo *ii, 
										  Bitmap *map,
										  Bitmap *foreMap = NULL);
	  
	  //-- This will  bring   a full blown dialog giving  the
	  //    user an interface to select   and define a plug-
	  //    in filter. Returns    FALSE   if  the user    cancels.
	  
	  FLTExport BOOL              SelectImageFilter( HWND hWnd, ImageFilterInfo *ii    );
	  
	  //-- This will  fill out    the given combo box with a
	  //    list of available mask options
	  
	  FLTExport void              ListMaskOptions  ( HWND hWnd, int item);

	  //----------------------------------------------------------------------
	  //-- R2 Stuff Below ----------------------------------------------------
	  //----------------------------------------------------------------------
   
	  //-- Internal Use

	  FLTExport void              UpdateFilter     ( ImageFilterInfo *ii );

	  
};

//-----------------------------------------------------------------------------
//--   Forward References
//

extern FLTExport   void             OpenFLT         (  FLTInterface *i );
extern FLTExport   void             CloseFLT        (  );

//-----------------------------------------------------------------------------
//--   The Primary Filter Manager  Object
//
// TO  DO: Move    to  App data    structure?

extern FLTExport FilterManager *TheFilterManager; 

