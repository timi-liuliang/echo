//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// FILE:        bitmap.h
// DESCRIPTION: 
// AUTHOR:      Tom Hudson
// HISTORY:     Sep, 01 1994 - Started serious coding
//            Oct, 18 1994 - First major revision for DLLs, restructuring
//            Jul, 10 1995 - Began working with it (Gus J Grubba)
//**************************************************************************/

#pragma once
#include "BMMExport.h"
#include <WTypes.h>
#include <vfw.h>
#include <CommDlg.h>
#include "maxheap.h"
#include "palutil.h"
#include "linklist.h"
#include "gbuf.h"
#include "pixelbuf.h"
#include "path.h"
#include "containers\array.h"
#include "assetmanagement\AssetType.h"
#include "assetmanagement\IAssetAccessor.h"
#include "assetmanagement\AssetUser.h"
#include "matrix3.h"
#include "GetCOREInterface.h"
#include "box2.h"

//-- Defines that may change with compiler
#define INTBITS (sizeof(int) * 8)

//-- Class ID's for various DLL's
#define IMGCLASSID      1
#define CYCLECLASSID    2
#define FLICCLASSID     5
#define TARGACLASSID    6
#define YUVCLASSID      7
#define FBCLASSID       8
#define WSDCLASSID      9
#define IFLCLASSID      10
#define BMPCLASSID      11
#define JPEGCLASSID     12
#define TARGAPLSCLASSID 13
#define AVICLASSID      14
#define RLACLASSID      15
#define RPFCLASSID		16
#define MPGCLASSID      17
// Class ID for the OpenEXR I/O plugin
#define	MAXOPENEXR_CLASSID		Class_ID(0x12f95b5e, 0x62a60b57)

//-- local definitions

class BitmapManager;
class BitmapManagerImp;
class BitmapProxyManager;
class BitmapStorage;
class BitmapFilter;
class BitmapDither;
class BitmapInfo;
class BitmapPicker;
class BitmapIO;
class Bitmap;

//-- Temporary definitions to make the compiler happy

class GraphicsWindow;
class BMMInterface;
class DllDir;

//-- External data 

extern int  TheSystemFrame;         // TO DO: Move to App data structure?

//-----------------------------------------------------------------------------
//-- The Primary Bitmap Manager Object
//

extern BMMExport BitmapManager *TheManager; // TO DO: Move to App data structure?

//-- Common Macros ------------------------------------------------------------

//-- Set up a NULL macro

#ifndef NULL
#define NULL (0)
#endif

//-----------------------------------------------------------------------------
//-- Image I/O History (Used by the File Picker Dialog ------------------------
//
class bmmHistoryList: public MaxHeapOperators {

	protected:

		MCHAR	title[MAX_PATH];
		MCHAR	initDir[MAX_PATH];
		HWND	hParent,hChild;
		int		listID;

		BMMExport void	StripSpaces	(MCHAR *string);

	public:

		BMMExport void	Init		( const MCHAR *title );
		BMMExport void	SetDlgInfo	( HWND hParent, HWND hChild, int ListID );
		BMMExport void	LoadDefaults( );
		BMMExport void	LoadList	( );
		BMMExport void	SaveList	( );
		BMMExport void	SetPath		( bool first = false );
		BMMExport void	NewPath		( const MCHAR *path );
		BMMExport MCHAR*DefaultPath	( ) { return initDir; }
		BMMExport void	SetPath		( const MCHAR *path );
		BMMExport int	Count		( );

};

//-- Generic bitmap information structure -------------------------------------

struct BMMImageInfo : public MaxHeapOperators {
   int   width,height;
   float aspect,gamma;
   DWORD flags;
};

//-- Basic bitmap types supported by Bitmap Manager
/*! \defgroup bitmapTypes Bitmap Types
See also: \ref BitmapStorage, \ref Color, \ref AColor, \ref AColor, \ref LogLUV32Pixel, \ref LogLUV24Pixel, \ref RealPixel
 */
//@{
#define BMM_NO_TYPE              0   //!<  Not allocated yet
#define BMM_LINE_ART             1   //!<  1-bit monochrome image
#define BMM_PALETTED             2   //!<  8-bit paletted image. Each pixel value is an index into the color table.
#define BMM_GRAY_8               3   //!<  8-bit grayscale bitmap.
#define BMM_GRAY_16              4   //!<  16-bit grayscale bitmap.
#define BMM_TRUE_16              5   //!<  16-bit true color image.
#define BMM_TRUE_32              6   //!<  32-bit color: 8 bits each for Red, Green, Blue, and Alpha.
#define BMM_TRUE_64              7   //!<  64-bit color: 16 bits each for Red, Green, Blue, and Alpha.
/*! This format uses a logarithmic encoding of luminance and U' and V' in the CIE perceptively uniform
space. It spans 38 orders of magnitude from 5.43571 to 1.84467 in steps of about 0.3% luminance steps. It
includes both positive and negative colors. A separate 16 bit channel is kept for alpha values. */
#define BMM_LOGLUV_32            13
/*! This format is similar to \ref BMM_LOGLUV_32 except is uses smaller values to give a span of 5 order of
magnitude from 1/4096 to 16 in 1.1% luminance steps. A separate 8 bit channel is kept for alpha values. */
#define BMM_LOGLUV_24            14
/*! This format is similar to \ref BMM_LOGUV_24, except the 8 bit alpha value is kept with the 24 bit 
color value in a single 32 bit word. */
#define BMM_LOGLUV_24A           15
#define BMM_REALPIX_32           16   //!<  The "Real Pixel" format.
#define BMM_FLOAT_RGBA_32        17   //!<  32-bit floating-point per component (non-compressed), RGB with or without alpha
#define BMM_FLOAT_GRAY_32        18   //!<  32-bit floating-point (non-compressed), monochrome/grayscale

//-- Information Only

#define BMM_TRUE_24              8    //!< 24-bit color: 8 bits each for Red, Green, and Blue. Cannot be written to.
#define BMM_TRUE_48              9    //!< 48-bit color: 16 bits each for Red, Green, and Blue. Cannot be written to.
#define BMM_YUV_422              10   //!< This is the YUV format - CCIR 601. Cannot be written to.
#define BMM_BMP_4                11   //!< Windows BMP 16-bit color bitmap.  Cannot be written to.
#define BMM_PAD_24               12   //!< Padded 24-bit (in a 32 bit register).  Cannot be written to.
/*! ONLY returned by the GetStoragePtr() method of BMM_FLOAT_RGBA_32 storage, NOT an actual storage type!
When GetStoragePtr() returns this type, the data should be interpreted as three floating-point values,
corresponding to Red, Green, and Blue (in this order). */
#define BMM_FLOAT_RGB_32         19
/*! ONLY returned by the GetAlphaStoragePtr() method of BMM_FLOAT_RGBA_32 or BMM_FLOAT_GRAY_32 storage,
NOT an actual storage type! When GetStorageAlphaPtr() returns this type, the data should be interpreted
as floating-point values one value per pixel, corresponding to Alpha. */
#define BMM_FLOAT_A_32           20
//@}

//-- Textual Limits

#define MAX_DESCRIPTION          256

//-- The number of bitmap formats supported internally

//#define BMM_FORMATS            6

//-- File types

//#define BMM_NOTYPE             0
//#define BMM_TARGA              1       // System Targa I/O driver
//#define BMM_GIF                2       // System GIF I.O driver

//-- Gamma limits

#define MINGAMMA                 0.2f
#define MAXGAMMA                 5.0f

/*! \defgroup bitmapOpenModeTypes Bitmap Open Mode Types */
//@{
#define BMM_NOT_OPEN             0	//!< Not opened yet
#define BMM_OPEN_R               1	//!< Read-only
#define BMM_OPEN_W               2	//!< Write-only. No reads will occur.
//@}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*! \defgroup bitmapErrorCodes Bitmap Error (result) Codes
The following error codes are used by the functions that use or return BMMRES types */

typedef unsigned short BMMRES;              //!< Traps the use of int or BOOL
//@{
#define BMMRES_SUCCESS                0		//!< Success - No error occurred.
#define BMMRES_ERRORTAKENCARE         1     //!< Error - Function has already taken action to process the error.
#define BMMRES_FILENOTFOUND           2		//!< The file being accessed was not found.
#define BMMRES_MEMORYERROR            3		//!< Insufficient memory for the requested operation.
#define BMMRES_NODRIVER               4     //!< Device driver responsible for image not present
#define BMMRES_IOERROR                5		//!< Input / Output error.
#define BMMRES_INVALIDFORMAT          6		//!< The file being access was not of the proper format for the requested operation.
#define BMMRES_CORRUPTFILE            7		//!< The file being accessed was corrupt.		
#define BMMRES_SINGLEFRAME            8     //!< Results from a goto request on a single frame image
#define BMMRES_INVALIDUSAGE           9     //!< Bad argument passed to function (Developer Mistake)
/*! This is returned if the user selects Retry from the 3ds Max Image IO Error dialog box. This dialog is
presented by the method BitmapIO::ProcessImageIOError(). */
#define BMMRES_RETRY                  10    //!< User selected "Retry" from error dialogue.
#define BMMRES_NUMBEREDFILENAMEERROR  11	//!< This can be passed as an errorcode to BitmapIO::ProcessImageIOError().
#define BMMRES_INTERNALERROR          12	//!< An internal error occurred.
#define BMMRES_BADFILEHEADER          13	//!< A file header error occurred.
#define BMMRES_CANTSTORAGE            14	//!< This is used internally.
#define BMMRES_BADFRAME               15    //!< Invalid Frame Number Requested
//@}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*! \defgroup bitmapIOCapabilityFlag BitmapIO Capability Flags
Returned by BitmapIO::Capability() */
//@{
#define BMMIO_NONE                         0   //!< Not defined yet

#define BMMIO_READER                   (1<<0)  //!< Reads image files
#define BMMIO_WRITER                   (1<<1)  //!< Writes image files
#define BMMIO_EXTENSION                (1<<2)  //!< Uses file extension (File Filter Type)
#define BMMIO_MULTIFRAME               (1<<3)  //!< "File" contains multiple frames (i.e. FLC, AVI)

#define BMMIO_THREADED                 (1<<4)  //!< Not used. All plugin are expected to be "thread aware".
#define BMMIO_RANDOM_ACCESS            (1<<5)  //!< Can read and/or write frames in any order
#define BMMIO_NON_CONCURRENT_ACCESS    (1<<6)  //!< Device cannot handle multiple, concurrent requests (FLC, AVI, VTR's, etc)

#define BMMIO_OWN_VIEWER               (1<<7)  //!< Driver has its own image viewer for its image type as opposed to using the generic Virtual Frame Buffer.

//-- Frame Buffers

#define BMMIO_FRAMEBUFFER              (1<<7)  //!< Frame Buffer Driver
#define BMMIO_GRABBER                  (1<<8)  //!< Device Grabs Video

/*! If the device is able to show its own image info dialogue this flag will be set. 
Otherwise, the host must use \ref BitmapManager::GetImageInfo() and display a generic info dialogue. */
#define BMMIO_INFODLG            		(1<<9) //!< Has Info Dialog

/*! If a driver cannot be started and stopped this flag should be set. This is an 'Uninterruptible Driver'.
For example the AVI, FLIC, etc. cannot stop and go and thus define this flag. */
#define BMMIO_UNINTERRUPTIBLE			(1<<10) //!< Uninterruptible Driver

/*! Drivers that may have a different image for a same given Max frame and same file/device
name should define this and implement the EvalMatch() method. */
#define BMMIO_EVALMATCH			(1<<11)	//!< EvalMatch() method has been implemented.

/*! If this flag is set, instead of calling the Load() method 3ds Max calls the GetImageName() method.
3ds Max will then process the given image name accordingly. The following flags relate to the plug-in's
control dialog. There is only a single call to the plug-in's control panel but the call specifies the nature
of the operation going on. It's up to the plugin to provide different interfaces if needed. If one control
serves two or more services, all the pertinent flags should be set. */
#define BMMIO_IFL                (1<<28) //!< Special IFL device

/*! There is only a single call to the plugin's control panel but the call specifies the nature of the
operation going on. It's up to the plugin to provide different interfaces if needed. If one control 
serves two or more services, all the pertinent flags should be set. */
#define BMMIO_CONTROLREAD        (1<<29) //!< Device Driver has Control Panel for Read Operations
#define BMMIO_CONTROLWRITE       (1<<30) //!< Device Driver has Control Panel for Write Operations
#define BMMIO_CONTROLGENERIC     (1<<31) //!< Device Driver has a generic Control Panel
//@}

//-----------------------------------------------------------------------------

/*! \defgroup bitmapCloseTypes Bitmap Close Types
 */

//@{
#define BMM_CLOSE_COMPLETE       0	//!< Close and save the image.
/*! Many bitmap loader / savers do not make use of these flags. An example of one that does is the FLC saver.
When a FLC file is closed, it goes through each image and calculates the palette. This can take a long time.
If the user cancels the operation they will not want to wait for this to happen. 
Thus this flag is passed to indicate that the user has canceled and this processing should not occur. */
#define BMM_CLOSE_ABANDON        1	//!< Close but abandon the image.
//@}

//-- Filter Types

/*! \defgroup bitmapFilterTypes Bitmap Filter Types
Specifies the type of filtering to perform. \n\n
The Pyramidal and Summed Area options provide two methods of pixel averaging that antialias the 
bitmaps in mapped materials. \n\n
Both methods require approximately the same rendering time. Summed-area filtering generally yields 
superior results but requires much more memory. Pyramidal filtering requires the program to allocate 
memory equal to approximately 133% of the size of the bitmap. By comparison, summed-area filtering 
requires the program to allocate approximately 400% of the size of the bitmap. \n\n
Use summed-area filtering only for smaller bitmaps, and avoid using any more such bitmaps in a scene 
than necessary. \n\n
Pyramidal filtering is quite adequate for most purposes. However, because it applies filtering 
as a function of distance, irregular antialiasing may occur on detailed texture maps that are 
applied to a plane receding into the distance. The effect of pyramidal filtering on extreme 
perspectives such as this is even more noticeable in animations, where portions of the texture 
map appear to "swim." */
//@{
#define BMM_FILTER_NONE          0	//!< Specifies no filtering should be performed.
#define BMM_FILTER_DUMMY         1	//!< No longer used.
#define BMM_FILTER_SUM           2	//!< Specifies summed area filtering.
#define BMM_FILTER_PYRAMID       3	//!< Specifies pyramidal filtering.
//@}

//-- Filter Flag values

#define BMM_FILTER_NOFLAGS       ((DWORD)0)
#define BMM_FILTER_LERP_WRAP     ((DWORD)(1<<0))
// Set when when the filter fails to build its lookup table, cleared otherwise
#define BMM_FILTER_BUILD_FAILED  ((DWORD)(1<<1))

//-- Dither Types

#define BMM_DITHER_NONE          0
#define BMM_DITHER_FLOYD         1

//-- Pyramidal filter information

#define MAX_PYRAMID_DEPTH        12
#define LAYER_DIM                MAX_PYRAMID_DEPTH+1

struct BMM_Pyramid: public MaxHeapOperators {
   WORD  dmax;
   void  *map[LAYER_DIM];
   void  *alpha[LAYER_DIM]; 
};

//-- Summed-area table information

struct BMM_SAT: public MaxHeapOperators {
   DWORD *sat_r,*sat_g,*sat_b,*sat_a;
};

/*! \defgroup bitmapCopyImageOptions Copy Image Operations */
//@{
#define COPY_IMAGE_CROP                 0  //!< Copy image to current map size w/cropping if necessary
/*! This is a resize from 50x50 to 150x150 using this option. \n\n
\image html copy_lo.gif */
#define COPY_IMAGE_RESIZE_LO_QUALITY    1  //!< Resize source image to destination map size (draft)
/*! This is a resize from 50x50 to 150x150 using this option. \n\n
\image html copy_hi.gif */
#define COPY_IMAGE_RESIZE_HI_QUALITY    2  //!< Resize source image to destination map size (final)
#define COPY_IMAGE_USE_CUSTOM           3  //!< Resize based on Image Input Options (BitmapInfo *)
//@}

// Class for storing a linked list of file extension strings
typedef LinkedEntryT<MSTR> MSTREntry; 
typedef LinkedListT<MSTR,MSTREntry> MSTRList;

//-- GRAINSTART
//-- Callback for Bitmap Effects
typedef BOOL (WINAPI* PBITMAP_FX_CALLBACK)( LPVOID lpparam, int done, int total, MCHAR* msg );
//-- GRAINEND

//-----------------------------------------------------------------------------
//-- I/O Handler
//
   
/*! \sa  Class BMM_IOList.\n\n
\par Description:
This class provides methods that access properties of the IO module. All
methods of this class are implemented by the system.  */
class BMM_IOHandler : public MaxHeapOperators 
{
   private:
	  //-- Name and Capabilities ------------------------
	  
	  MCHAR      ioShortDesc[MAX_DESCRIPTION]; 
	  MCHAR      ioLongDesc[MAX_DESCRIPTION];
	  DWORD      ioCapabilities;

	  //-- Extension for file types
	  
	  MSTRList   ioExtension;
	  
	  //-- DLL Handler ----------------------------------
	  
	  int	   dllNumber;
	  // the classID of the Class Descriptor that this points to.
	  Class_ID mClassID;
	  // the superclassID of the Class Descriptor that this points to.
	  SClass_ID mSuperID;
	  
   public:

	  /*! \remarks Constructor. The description string is set to NULL, the
	  capabilities are set to NONE, and the Class Descriptor is set to NULL. */
	  BMMExport BMM_IOHandler();
	  
	  /*! \remarks Returns the short description string for the IO module, and
	  optionally sets it to <b>d</b> if it is not NULL.
	  \par Parameters:
	  <b>const MCHAR *d = NULL</b>\n\n
	  The short description string to set. */
	  BMMExport MCHAR*           ShortDescription ( const MCHAR*  d = NULL );
	  /*! \remarks Returns the long description string for the IO module, and
	  optionally sets it to <b>d</b> if it is not NULL.
	  \par Parameters:
	  <b>const MCHAR *d = NULL</b>\n\n
	  The long description string to set. */
	  BMMExport MCHAR*           LongDescription  ( const MCHAR*  d = NULL );
	  /*! \remarks Returns the specified IO module file name extension, and
	  optionally sets it to <b>e</b> if it is not NULL.
	  \par Parameters:
	  <b>int index</b>\n\n
	  The index of the extension.\n\n
	  <b>const MCHAR *e = NULL</b>\n\n
	  The extension string. */
	  BMMExport MCHAR*           Extension        ( int index, const MCHAR*  e = NULL );
	  /*! \remarks Returns the number of file name extensions for the IO
	  module. */
	  BMMExport int              NumExtensions    ( )                { return ioExtension.Count(); }

	  /*! \brief This grabs the ClassID and the SuperClassID of Class Descriptor that gets passed in.
		It does not grab a hold of the Class Descriptor pointer itself.
	  	\Param dll - A pointer to the class descriptor. */
	  BMMExport void             SetCD            ( ClassDesc* dll );
	  /*! \remarks Returns the class descriptor for the IO module. */
	  BMMExport ClassDesc*       GetCD            ( );
	  BMMExport Class_ID         GetClassID       ( );
	  BMMExport SClass_ID        GetSuperID       ( );
	  BMMExport void             SetDllNumber     ( int num )        { dllNumber = num; }
	  BMMExport int              GetDllNum        ( )                { return dllNumber; }

	  /*! \remarks Sets the capabilities flag for the IO module.
	  \par Parameters:
	  <b>DWORD cap</b>\n\n
	  See \ref bitmapIOCapabilityFlag. */
	  BMMExport void             SetCapabilities  ( DWORD cap )      { ioCapabilities |= cap;};
	  /*! \remarks Returns the capabilities flag for the IO module.
	  \return  See \ref bitmapIOCapabilityFlag. */
	  BMMExport DWORD            GetCapabilities  ( )                { return (ioCapabilities);};
	  /*! \remarks Tests the capability flags passed.
	  \par Parameters:
	  <b>DWORD cap</b>\n\n
	  See  \ref bitmapIOCapabilityFlag.
	  \return  TRUE if the flags were set; otherwise FALSE.
	  \par Operators:
	  */
	  BMMExport BOOL             TestCapabilities ( DWORD cap )      { return ((ioCapabilities& cap) != 0);};
	  /*! \remarks Assignment operator. */
	  BMMExport BMM_IOHandler&   operator=(BMM_IOHandler& from);
};

//-----------------------------------------------------------------------------
//-- List of I/O Handlers
//

// Class for storing a linked list of Bitmap Manager BMM_IOHandler objects
typedef LinkedEntryT<BMM_IOHandler> BMM_IOHandlerEntry; 
typedef LinkedListT<BMM_IOHandler,BMM_IOHandlerEntry> BMM_IOHandlerList;

/*! \brief This class is used for storing a linked list of Bitmap Manager BMM_IOHandler objects. 
It provides methods for working with IO module devices derived from BitmapIO. All 
methods of this class are implemented by the system.  
\sa  Class BMM_IOHandlerList,  Class BitmapIO. */
class BMM_IOList: public BMM_IOHandlerList {

	  BOOL     listed;
	   
   public:

	  /*! \remarks Constructor. The flag that indicates that the list has been
	  built is set to FALSE. */
	  BMM_IOList          ( )                { listed = FALSE; }

	  /*! \remarks Sets the state to indicate the list of IO modules has been
	  built.
	  \par Parameters:
	  <b>BOOL f</b>\n\n
	  TRUE to indicate the list exists; otherwise FALSE. */
	  BOOL     Listed     ( BOOL f)          { listed = f; return (listed);};
	  /*! \remarks Indicates if the list of IO modules has been built. Returns
	  TRUE if it has; otherwise FALSE. */
	  BOOL     Listed     ( )                { return (listed);};

	  /*! \remarks Scans the list searching for a match for the description
	  string passed.
	  \par Parameters:
	  <b>const MCHAR *name</b>\n\n
	  The long description string of the device to find.
	  \return  The index in the IO list of the device. If not found, -1 is
	  returned. */
	  BMMExport int   FindDevice             ( const MCHAR *name);
	  /*! \remarks This method scans the IO module list searching for a match
	  for the extension in the filename passed.
	  \par Parameters:
	  <b>const MCHAR *name</b>\n\n
	  The filename to check.
	  \return  The index in the list of IO modules or -1 if not found. */
	  BMMExport int   FindDeviceFromFilename ( const MCHAR *name);
	  /*! \remarks Given a <b>BitmapInfo</b> record, this method finds the
	  device responsible for this image. If <b>bi.Name()</b> is empty, a device
	  is listed in <b>bi.Device()</b>. In that case we simply use the
	  <b>FindDevice()</b> function which returns an index to the device based
	  on a given device name.\n\n
	  If <b>bi.Name()</b> isn't empty, it means this is a file and we must
	  search for a device based on a filename. For that we use
	  <b>FindDeviceFromFilename()</b>. If we find a device, we take the
	  opportunity to fill in the device name in the <b>BitmapInfo</b> record
	  (using <b>SetDevice()</b>).
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  The BitmapInfo for the image whose device you wish to find.
	  \return  The index in the list of IO modules or -1 if not found. */
	  BMMExport int   ResolveDevice          ( BitmapInfo *bi   );
	  /*! \remarks Returns the capability flags of the device whose long
	  description string is passed.
	  \par Parameters:
	  <b>const MCHAR *name</b>\n\n
	  The long description flag.
	  \return  See  \ref bitmapIOCapabilityFlag. */
	  BMMExport DWORD GetDeviceCapabilities  ( const MCHAR *name);

	  //-- This Creates an Instance - Make sure to "delete" it after use.

	  /*! \remarks This creates an instance of the device plug-in class by
	  calling <b>Create()</b> on the class descriptor. Make sure to delete the
	  instance after use.
	  \par Parameters:
	  <b>const MCHAR *d</b>\n\n
	  The long description string.
	  \return  A pointer to an instance of the IO module. */
	  BMMExport BitmapIO *CreateDevInstance( const MCHAR *d );
	  /*! \remarks This creates an instance of the device plug-in class by
	  calling <b>Create()</b> on the class descriptor. Make sure to delete the
	  instance after use.
	  \par Parameters:
	  <b>int idx</b>\n\n
	  The index in the IO module list of the device.
	  \return  A pointer to an instance of the IO module. */
	  BMMExport BitmapIO *CreateDevInstance( int idx );

};



//-----------------------------------------------------------------------------
//-- RenderInfo Class
//

enum ProjectionType { ProjPerspective=0, ProjParallel=1 };

// WARNING: IF YOU CHANGE the RenderInfo data structure, the RPF writing code (rla.cpp ) has to
// be modified to read the old version as well as the new.  DS 9/7/00

/*! \sa  Class Ray, Class Point2,  Class Point3.\n\n
\par Description:
This class provides information about the rendering environment. All methods of
this class are implemented by the system.
\par Data Members:
<b>ProjectionType projType;</b>\n\n
The projection type: One of the following values:\n\n
<b>ProjPerspective</b>\n\n
<b>ProjParallel</b>\n\n
<b>float kx,ky;</b>\n\n
3D to 2D projection scale factor.\n\n
<b>float xc,yc;</b>\n\n
The screen origin.\n\n
<b>BOOL fieldRender;</b>\n\n
Indicates if the image is field rendered.\n\n
<b>BOOL fieldOdd;</b>\n\n
If TRUE, the first field is Odd lines.\n\n
<b>TimeValue renderTime[2];</b>\n\n
Render time for the 2 fields, if field rendering. If not, use
<b>renderTime[0]</b>.\n\n
<b>Matrix3 worldToCam[2];</b>\n\n
The world to camera transformation matrix; <b>worldToCam[0]</b> is for field 0,
<b>worldToCam[1]</b> is for field 1. Use <b>worldToCam[0]</b> if not field
rendering.\n\n
<b>Matrix3 camToWorld[2];</b>\n\n
The camera to world transformation matrix; <b>camToWorld[0]</b> is for field 0,
<b>camToWorld[1]</b> is for field 1. Use <b>camToWorld[0]</b> if not field
rendering.\n\n
<b>Rect region;</b>\n\n
This data member is available in release 4.0 and later only.\n\n
This rectangle holds the sub-region in the image that was rendered if the last
render was a region render. If it was not a region render then the rectangle is
empty.  */
class RenderInfo: public MaxHeapOperators {
   public:
   /*! \remarks Constructor. The data members are initialized as follows:\n\n
   <b> renderTime[0] = renderTime[1] = 0;</b>\n\n
   <b> worldToCam[0].IdentityMatrix();</b>\n\n
   <b> worldToCam[1].IdentityMatrix();</b>\n\n
   <b> camToWorld[0].IdentityMatrix();</b>\n\n
   <b> camToWorld[1].IdentityMatrix();</b>\n\n
   <b> fieldRender = fieldOdd = FALSE;</b>\n\n
   <b> projType = ProjPerspective;</b>\n\n
   <b> kx = ky = 1.0f;</b>\n\n
   <b> xc = yc = 400.0f;</b> */
   RenderInfo();
   ProjectionType projType;  
   float kx,ky;            // 3D to 2D projection scale factor 
   float xc,yc;           // screen origin
   BOOL fieldRender;    // field rendered?
   BOOL fieldOdd;         // if true, the first field is Odd lines
   // Render time and tranformations for the 2 fields, if field rendering. 
   // If not, use renderTime[0], etc.
   TimeValue renderTime[2]; 
   Matrix3 worldToCam[2];     
   Matrix3 camToWorld[2];
   Rect region;  // sub-region in image that was rendered if last render was a region render. Empty if not a region render. -- DS--7/13/00
   /*! \remarks Maps the specified world point to a screen point.
   \par Parameters:
   <b>Point3 p</b>\n\n
   The world point.\n\n
   <b>int field=0</b>\n\n
   The field order. This specifies which <b>camToWorld</b> matrix is used in
   the conversion.
   \return  The 2D screen point. */
   BMMExport Point2 MapWorldToScreen(Point3 p, int field=0) { return MapCamToScreen(worldToCam[field]*p);}
   /*! \remarks Maps the specified point in camera space to screen space and
   returns it.
   \par Parameters:
   <b>Point3 p</b>\n\n
   The point to convert. */
   BMMExport Point2 MapCamToScreen(Point3 p);  // map point in camera space to screen
   /*! \remarks Returns the viewing ray through the screen point, in camera
   space.
   \par Parameters:
   <b>Point2 p</b>\n\n
   The screen point. */
   BMMExport Ray MapScreenToCamRay(Point2 p);  // get viewing ray through screen point, in camera space
   /*! \remarks Returns the viewing ray through the specified screen point, in
   world space.
   \par Parameters:
   <b>Point2 p</b>\n\n
   The screen space point.\n\n
   <b>int field=0</b>\n\n
   The field order - specifies which <b>camToWorld</b> matrix is used in the
   conversion. */
   BMMExport Ray MapScreenToWorldRay(Point2 p, int field=0); // get viewing ray through screen point, in world space
   };

//-----------------------------------------------------------------------------
//-- Bitmap Info Class
//

/*! \defgroup bitmapFlags Bitmap Flags
 These flag bits describe properites of the bitmap */
//@{
#define MAP_NOFLAGS              ((DWORD)0)			//!< The bitmap has none of the characteristics below.
#define MAP_READY                ((DWORD)(1<<0))	//!< This bitmap has had memory allocated to it, or is accessible directly.
/*! This flag can be checked from a BitmapInfo instance (bi.Flags()&MAP_HAS_ALPHA)), but not from a Bitmap
instance (bmap->Flags() & MAP_HAS_ALPHA). Therefore if you have a Bitmap, use the Bitmap's HasAlpha()
method to see if the map has an alpha channel. */
#define MAP_HAS_ALPHA            ((DWORD)(1<<1))	//!< The bitmap has an alpha channel.
#define MAP_ALPHA_PREMULTIPLIED  ((DWORD)(1<<2))	//!< The bitmap has pre-multiplied alpha.
#define MAP_PALETTED             ((DWORD)(1<<3))	//!< The bitmap uses a palette (not true color).
#define MAP_FRAME_SYSTEM_LOCKED  ((DWORD)(1<<4))	//!< This flag is no longer used.
#define MAP_DITHERED             ((DWORD)(1<<5))	//!< The bitmap is dithered.
#define MAP_FLIPPED              ((DWORD)(1<<6))	//!< The bitmap is flipped horizontally
#define MAP_INVERTED             ((DWORD)(1<<7))	//!< The bitmap is flipped vertically

//#define MAP_CUSTOMSIZE           ((DWORD)(1<<8))     // Custom size for input
//#define MAP_RESIZE               ((DWORD)(1<<9))     // Resize when input

#define MAP_USE_SCALE_COLORS     ((DWORD)(1<<10))	//!< Scale colors when high dynamic range values are out of gamut

/*! When the proxy flag is set, the BitmapInfo is called a proxy info.
	The info for the proxy's original subject bitmap is called the subject info.
	The subject info is available from the proxy info via BitmapInfo::GetProxySubject().
	\see BitmapInfo::GetProxySubject() */
#define MAP_PROXY                ((DWORD)(1<<11))	//!< Indicates a proxy bitmap, reduced from original size / bit depth
/*! When the proxy request flag is set, the BitmapInfo is called a request info.
	A request info should contain settings for the original bitmap, without any subject info allocated.
	A request info passed to BitmapManager::Load() serves as a request to load proxy version of the original bitmap.
	The bitmap manager converts the request info to a proxy info with the complete proxy and subject settings. */
#define MAP_PROXYREQUEST         ((DWORD)(1<<12))	//!< Indicates a request to load proxy bitmap, reduced from original size / bit depth

#ifdef _ENABLE_BITMAP_PRINTING_
#define MAP_HAS_BGIMAGE          ((DWORD)(1<<29))    //!< internal use only
#endif
#define MAP_LEGAL_DELETE	     ((DWORD)(1<<30))    //!< internal use only
#define MAP_VIEW_FILTERED        ((DWORD)(1<<31))    //!< Test stuff

#define MAP_ALL_FLAGS            0xFFFFFFFF
//@}

//-----------------------------------------------------------------------------
//-- Messages sent back by various (client) methods

//-- Sent by the plug-in to notify host of current progress. The host should
//   return TRUE if it's ok to continue or FALSE to abort process.

#define BMM_PROGRESS   WM_USER + 0x120   //-- wParam: Current lParam: Total

//-- Sent by the plug-in to check for process interruption. The host should
//   return FALSE (by setting lParam) if it's ok to continue or TRUE to abort 
//   process.

#define BMM_CHECKABORT WM_USER + 0x121   //-- wParam: 0       lParam: *BOOL

//-- Sent by the plug-in to display an optional textual message (for progress
//   report).

#define BMM_TEXTMSG    WM_USER + 0x122   //-- wParam: 0       lParam: LPCMSTR

/*! \sa  Class Bitmap, <a href="ms-its:3dsmaxsdk.chm::/bitmaps_root.html">Working with Bitmaps</a>,
\ref bitmapErrorCodes.\n\n
\par Description:
This class describes the properties of a bitmap such as its path name or device
name, width, height, gamma, number of frames, etc. Methods are available to set
and retrieve these properties. All methods are implemented by the system unless
noted otherwise.
\par Method Groups:
See <a href="class_bitmap_info_groups.html">Method Groups for Class BitmapInfo</a>.
*/
class BitmapInfo: public MaxHeapOperators {
   
		//-- Image name in case of named images such as files -------

		MaxSDK::Util::Path mFilename;
		MaxSDK::AssetManagement::AssetUser mAssetUser;

		//-- Device name gotten from BMM_IOHandler::LongDescription() 

		MCHAR                    device[MAX_DESCRIPTION];

		//-- Window Handle to send BMM_UPDATE messages --------------

		HWND                     hWnd;

		//-- Plug-In Parameter Block --------------------------------

		void                    *pidata;
		DWORD                    pisize;

		//-- Proxy subject info -------------------------------------

		//! \brief Describes a proxy's full-size subject bitmap. Used when the info has the proxy flag toggled.
		/*! \see MAP_PROXY */
		BitmapInfo              *proxysubject;

		//-- Basic Image Data ---------------------------------------
		//
		//   When reading an image, or asking for image info, these
		//   fields will tell the user what the image is like.
		//
		//   When creating an image the user will set these fields to
		//   the desired parameters.
		//

		WORD                     width,height;
		float                    aspect,gamma;
		int                      fstart,fend;
		DWORD                    flags,type;

		//-- User data (what user wants) ----------------------------
		//
		//   Custom dimmensions, custom start and end point when
		//   reading sequence of imges, frame to fetch/save, etc.
		//

		WORD                     cwidth,cheight;   
		int                      custxpos,custypos;
		int                      start,end;        
		int                      step,preset_al;
		float                    custgamma;        

		#define BMM_UNDEF_FRAME   0x7FFF0000

/**	\defgroup bitmapCustomFlags Custom Bitmap Flags
These can be OR'd together. */
//@{
		#define BMM_CUSTOM_GAMMA        ((DWORD)(1 << 0))	//!< Custom gamma setting
		#define BMM_CUSTOM_SIZE         ((DWORD)(1 << 1))	//!< Custom size setting
		#define BMM_CUSTOM_RESFIT       ((DWORD)(1 << 2))	//!< Bitmap is to be resized
		#define BMM_CUSTOM_POS          ((DWORD)(1 << 3))	//!< Bitmap has a custom positioning
		#define BMM_CUSTOM_FILEGAMMA    ((DWORD)(1 << 4))	//!< Bitmap has a custom file gamma setting
		#define BMM_CUSTOM_IFLENUMFILES ((DWORD)(1 << 5))	//!< 
		#define BMM_CUSTOM_HDR_TYPE	((DWORD)(1 << 6))	//!< Internal Use Only.  Bitmap has custom hdr type that reads storage infor from BitMap Info and not from UI.	

//@}

/**	\defgroup bitmapAlignmentPositions Bitmap Alignment Positions
The following values define the nine possible alignment positions of a bitmap, 
numbered 0 to 8, left-to-right and top-to-bottom: \n\n \image html bit-pos.gif "Bitmap Alignment Positions"
*/
//@{
		#define BMM_CUSTOM_POSNW  0	//!< Top Left
		#define BMM_CUSTOM_POSN   1	//!< Top Center
		#define BMM_CUSTOM_POSNE  2	//!< Top Right
		#define BMM_CUSTOM_POSW   3	//!< Middle Left
		#define BMM_CUSTOM_POSCN  4	//!< Center
		#define BMM_CUSTOM_POSE   5	//!< Middle Right
		#define BMM_CUSTOM_POSSW  6	//!< Bottom Left
		#define BMM_CUSTOM_POSS   7	//!< Bottom Center
		#define BMM_CUSTOM_POSSE  8	//!< Bottom Right
//@}
		DWORD                    customflags;

		int                      fnumber;           //-- Defines frame to
												  //   read or write.

		//-- When reading a sequence of frames, loopflag indicates what to
		//   do when reading beyond the end of available frames. It 
		//   defaults to BMM_SEQ_WRAP.

		#define  BMM_SEQ_WRAP     0                  //-- Wraps around back to start point
		#define  BMM_SEQ_ERROR    1                  //-- Generates an error
		#define  BMM_SEQ_HOLD     2                  //-- Holds last frame when done
				
		WORD                     loopflag;         

		void                     doConstruct                 ( );
	  
   public:

		/*! \remarks Constructor. The following defaults are set by this
		constructor.\n\n
		The width = 640;\n\n
		The height = 480;\n\n
		The custom width = 320;\n\n
		The custom height = 200;\n\n
		The custom flags = BMM_CUSTOM_RESFIT | BMM_CUSTOM_FILEGAMMA;\n\n
		The custom gamma = 1.0f;\n\n
		The frame number = 0;\n\n
		The aspect ratio = 1.0f;\n\n
		The gamma setting = 1.0f;\n\n
		The name and device name are set to NULL.\n\n
		The looping flag = BMM_SEQ_WRAP; */
		BMMExport                BitmapInfo                  ( );
		/*! \remarks Constructor. The defaults are set as above excepting the bitmap
		name is set.
		\par Parameters:
		<b>MCHAR *n</b>\n\n
		The bitmap file name is set. */
		BMMExport                BitmapInfo                  ( const MCHAR *n );
		/*! \remarks Constructor. The defaults are set as above excepting the bitmap
		asset is set.
		\par Parameters:
		<b>AssetUser &asset</b>\n\n
		The bitmap file asset is set. */
		BMMExport                BitmapInfo                  ( const MaxSDK::AssetManagement::AssetUser &asset );
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Copy Constructor.
		\par Parameters:
		<b>const BitmapInfo \&bi</b>\n\n
		The BitmapInfo to copy from. */
		BMMExport                BitmapInfo                  ( const BitmapInfo &bi );
		BMMExport               ~BitmapInfo                  ( );

		//-- Bitmap Flags

		/*! \remarks Returns the flags of this BitmapInfo. See \ref bitmapFlags. */
		BMMExport DWORD          Flags                       ( ) { return (flags); }
		/*! \remarks Sets the flags for this BitmapInfo. These are bitwise
		OR-ed into the current flags.
		\par Parameters:
		<b>DWORD f</b>\n\n
		The flag bits to set. See \ref bitmapFlags.
		\return  The revised flags are returned. */
		BMMExport DWORD          SetFlags                    ( DWORD f ) { flags |=  f; return (flags); }
		BMMExport DWORD          ResetFlags                  ( DWORD f ) { flags &= ~f; return (flags); }
		/*! \remarks Determines if a set of flag bits are set.
		\par Parameters:
		<b>DWORD f</b>\n\n
		The flag bits to test. See \ref bitmapFlags.
		\return  TRUE if the bits are set; otherwise FALSE. */
		BMMExport BOOL           TestFlags                   ( DWORD f ) { return (flags & f); }

		//-- Generic Read

		/*! \remarks Returns the width (horizontal dimension) property of this
		BitmapInfo. */
		BMMExport WORD           Width                       ( ) { return (width); }
		/*! \remarks Returns the height (vertical dimension) setting of this
		BitmapInfo. */
		BMMExport WORD           Height                      ( ) { return (height); }
		/*! \remarks Returns the gamma setting property of this BitmapInfo. */
		BMMExport float          Gamma                       ( ) { return (gamma); }
		/*! \remarks Returns the aspect ratio property of this BitmapInfo. */
		BMMExport float          Aspect                      ( ) { return (aspect);}
		/*! \remarks Returns the type property of this BitmapInfo. See \ref bitmapTypes. */
		BMMExport int            Type                        ( ) { return (type); }
		/*! \remarks Returns the first frame property of this BitmapInfo. Note
		that for a multi-frame bitmap some sequences may start with something
		other than 0. */
		BMMExport int            FirstFrame                  ( ) { return (fstart); }
		/*! \remarks Returns the last frame property of this BitmapInfo. */
		BMMExport int            LastFrame                   ( ) { return (fend); }
		/*! \remarks Returns the total number of frames setting of this
		BitmapInfo. */
		BMMExport int            NumberFrames                ( ) { return (fend - fstart + 1); }
		/*! \remarks Returns the current frame setting of this BitmapInfo. */
		BMMExport int            CurrentFrame                ( ) { return (fnumber); }
		/*! \remarks When multi-frame BitmapIO loaders are reading a sequence
		of frames, this method is called to indicate what to do when reading
		beyond the end of available frames. The defaults is
		<b>BMM_SEQ_WRAP</b>.
		\return  One of the following values:\n\n
		<b>BMM_SEQ_WRAP</b>\n\n
		Wraps around back to start point.\n\n
		<b>BMM_SEQ_ERROR</b>\n\n
		Generates an error if reading goes beyond the end.\n\n
		<b>BMM_SEQ_PINGPONG</b>\n\n
		This causes the sequence to turn around and goes the other direction,
		back and forth.\n\n
		<b>BMM_SEQ_HOLD</b>\n\n
		When the last frame is reached it is held and used over and over. */
		BMMExport WORD           SequenceOutBound            ( ) { return (loopflag); }

		/*!
		 * \brief Returns the currently set path of the image file.
		 *
		 * \return The currently set path of the image file.
		 */
		/*! \remarks Returns the name property of this BitmapInfo. This is the
		full path name. See <b>MCHAR *Filename()</b> for just the file name. */
		BMMExport const MCHAR   *Name                        ( ) const;

		/*!
		 * \brief Returns the currently set path of the image file.
		 *
		 * \return The currently set path of the image file.
		 */
		BMMExport MaxSDK::Util::Path GetPathEx() const;

		//-- "Filename" returns just the name of image file

		/*! \remarks Returns just the file name of this BitmapInfo (not the
		entire path name). */
		BMMExport const MCHAR   *Filename                    ( );

		//-- "Device" is the device reponsible for producing this image.
		//   For file types, this is just informative. For non file types
		//   this is the way this image is identified. Therefore, it is
		//   important to save both name and device in order to properly
		//   identify an image.

		/*! \remarks Returns the device name responsible for producing this
		image. For file types, this is just informative. For non-file types
		(devices) this is the way this image is identified. Therefore, it is
		important to save both name and device in order to properly identify an
		image. */
		BMMExport const MCHAR   *Device                      ( ) { return (const MCHAR *)device; }

		//-- Compare Two Bitmaps

		/*! \remarks This method will compare names taking in consideration
		both file names and device names. As devices don't have a file name,
		this method will first determine what type of image this is, and then
		perform a proper comparison.
		\par Parameters:
		<b>BitmapInfo *bi</b>\n\n
		The other BitmapInfo with which to compare names.
		\return  TRUE if the BitmapInfos have the same name and device name;
		otherwise FALSE. */
		BMMExport BOOL           CompareName                 ( BitmapInfo *bi );

		//-- Copy Image info. Only name, device and image characteristics are
		//   copied. User info, such as Custom Width, etc. is not copied. 

		/*! \remarks Copies the image information of the <b>from</b>
		BitmapInfo to this bitmap. Only the name, device and image
		characteristics are copied. User info, such as Custom Width, etc. is
		not copied.\n\n
		The following properties of the <b>from</b> BitmapInfo are copied:\n\n
		<b>from-\>Name(),from-\>Device(),from-\>Width(), from-\>Height(),
		from-\>Aspect(), from-\>Gamma(),from-\>Type(), from-\>Flags(),
		from-\>FirstFrame(), from-\>LastFrame(), from-\>CurrentFrame(),
		from-\>GetCustomFlags()</b>
		\par Parameters:
		<b>BitmapInfo *from</b>\n\n
		The bitmap whose information will be copied. */
		BMMExport void           CopyImageInfo               ( BitmapInfo *from );

		//-- Generic Write

		/*! \remarks Sets the first frame property of this BitmapInfo.
		\par Parameters:
		<b>int f</b>\n\n
		Specifies the first frame setting.
		\return  The old (previous) first frame setting. */
		BMMExport int            SetFirstFrame               ( int m ) { int o = fstart;   fstart   = m; return (o);}
		/*! \remarks Sets the last frame property of this BitmapInfo.
		\par Parameters:
		<b>int f</b>\n\n
		Specifies the last frame.
		\return  The old (previous) frame setting. */
		BMMExport int            SetLastFrame                ( int s ) { int o = fend;     fend     = s; return (o);}
		/*! \remarks Sets the current frame setting of this BitmapInfo.
		\par Parameters:
		<b>int v</b>\n\n
		Specifies the current frame.
		\return  The old (previous) current frame setting. */
		BMMExport int            SetCurrentFrame             ( int v ) { int o = fnumber;  fnumber  = v; return (o);}
		/*! \remarks Sets the sequence out of bounds property of this
		BitmapInfo. When reading a sequence of frames, this specifies what to
		do when reading beyond the end of available frames.
		\par Parameters:
		<b>WORD s</b>\n\n
		One of the following values:\n\n
		<b>BMM_SEQ_WRAP</b>\n\n
		Wraps around back to start point.\n\n
		<b>BMM_SEQ_ERROR</b>\n\n
		Generates an error if reading goes beyond the end.\n\n
		<b>BMM_SEQ_PINGPONG</b>\n\n
		This causes the sequence to turn around and goes the other direction,
		back and forth.\n\n
		<b>BMM_SEQ_HOLD</b>\n\n
		When the last frame is reached it is held and used over and over.
		\return  The old (previous) value that was set. */
		BMMExport WORD           SetSequenceOutBound         ( WORD  c ) { WORD  o = loopflag; loopflag = c; return (o);}

		/*! \remarks Sets the width (horizontal dimension) property of this
		BitmapInfo.
		\par Parameters:
		<b>WORD w</b>\n\n
		Specifies the width setting in pixels.
		\return  The old (previously set) width of the bitmap. */
		BMMExport WORD           SetWidth                    ( WORD  s ) { WORD  o = width;    width    = s; return (o);}
		/*! \remarks Sets the height (vertical dimension) property of this
		BitmapInfo.
		\par Parameters:
		<b>WORD h</b>\n\n
		Specifies the height setting in pixels.
		\return  The old (previous) height setting. */
		BMMExport WORD           SetHeight                   ( WORD  u ) { WORD  o = height;   height   = u; return (o);}
		/*! \remarks Sets the gamma property of this BitmapInfo to the value
		passed.
		\par Parameters:
		<b>float g</b>\n\n
		Specifies the gamma setting.
		\return  The old (previous) gamma setting. */
		BMMExport float          SetGamma                    ( float c ) { float o = gamma;    gamma    = c; return (o);}
		/*! \remarks Set the aspect ratio property of this BitmapInfo to the
		specified value.
		\par Parameters:
		<b>float a</b>\n\n
		Specifies the aspect ratio setting.
		\return  The old (previous) aspect ratio of the bitmap. */
		BMMExport float          SetAspect                   ( float k ) { float o = aspect;   aspect   = k; return (o);}
		/*! \remarks Sets the type property of this BitmapInfo to the
		specified value.
		\par Parameters:
		<b>int t</b>\n\n
		Specifies the type of bitmap. See \ref bitmapTypes.
		\return  The old (previous) type setting. */
		BMMExport int            SetType                     ( int   s ) { int   o = type;     type     = s; return (o);}

		/*!
		 * \brief Set the path to the image file.
		 *
		 * \param path New path for the bitmap.
		 */
		BMMExport void SetPath(const MaxSDK::Util::Path &path);

		/*!
		 * \brief Set the path to the image file.
		 *
		 * This is a convenience method and calls SetPath under the hood.
		 * \param n New path for the bitmap.
		 */
		/*! \remarks Sets the name property of this BitmapInfo. When writing
		<b>n</b> should have a fully qualified filename. When reading, it only
		matters if the image is not in the MAP path. Note that a "feature" of
		the MAP path system is that if an image with same name is found more
		than once (in different paths), only the first one is seen.\n\n
		Note: If loading an image from a device, make sure the name is empty
		(<b>bi.SetName(_M(""));</b>). This is automatic if you use
		<b>BitmapManager::SelectDeviceInput()</b>. If you just create a
		<b>BitmapInfo</b> instance and set the device name by hand
		(<b>bi.SetDevice()</b>), this is also automatic as both name and device
		names are by default set to NULL (""). This is only a concern if you
		reuse a <b>BitmapInfo</b> class previously used for image files.
		\par Parameters:
		<b>const MCHAR *n</b>\n\n
		Specifies the name of the bitmap.
		\return  The old (previous) name that was set. */
		BMMExport const MCHAR   *SetName		( const MCHAR *n );
		
		//! \brief returns this bitmap's asset
		BMMExport const MaxSDK::AssetManagement::AssetUser& GetAsset() const;

		/*! \brief sets this bitmap's asset

		param[in] assetUser the new asset
		*/
		BMMExport void SetAsset( const MaxSDK::AssetManagement::AssetUser& assetUser ) ;

		/*! \remarks This method is used to set the device name.
		\par Parameters:
		<b>const MCHAR *d</b>\n\n
		The name to set.
		\return  The device name that was set. */
		BMMExport const MCHAR   *SetDevice                   ( const MCHAR *d );

		//-- Custom Input Processing

		/*! \remarks Returns the custom width setting of this BitmapInfo. */
		BMMExport WORD           CustWidth                   ( ) { return (cwidth);        }
		/*! \remarks Returns the custom height setting of this BitmapInfo. */
		BMMExport WORD           CustHeight                  ( ) { return (cheight);       }
		/*! \remarks Sets the custom width setting for this BitmapInfo.
		\par Parameters:
		<b>WORD w</b>\n\n
		The new custom width setting. */
		BMMExport void           SetCustWidth                ( WORD w ) { cwidth  = w;     }
		/*! \remarks Sets the custom height property of this BitmapInfo.
		\par Parameters:
		<b>WORD h</b>\n\n
		The new custom height setting. */
		BMMExport void           SetCustHeight               ( WORD h ) { cheight = h;     }
		/*! \remarks Returns the custom start frame property of this
		BitmapInfo. */
		BMMExport int            StartFrame                  ( ) { return (start);         }
		/*! \remarks Returns the custom end frame setting of this BitmapInfo..
		*/
		BMMExport int            EndFrame                    ( ) { return (end);           }
		/*! \remarks Sets the custom start frame property to the specified
		value.
		\par Parameters:
		<b>int s</b>\n\n
		Specifies the start frame setting. */
		BMMExport void           SetStartFrame               ( int s )  { start = s;      }
		/*! \remarks Sets the custom end frame property of this BitmapInfo.
		\par Parameters:
		<b>int e</b>\n\n
		The new end frame setting. */
		BMMExport void           SetEndFrame                 ( int e )  { end   = e;      }
		/*! \remarks Specifies the optional X coordinate (offset) property of
		this BitmapInfo. This specifies where to place the image if the image
		being copied from one Bitmap to another is smaller.
		\par Parameters:
		<b>int x</b>\n\n
		Specifies the custom X offset. */
		BMMExport void           SetCustomX                  ( int x ) { custxpos = x;    }
		/*! \remarks Sets the optional Y coordinate (offset) property of this
		BitmapInfo. This specifies where to place the image if the image being
		copied from one Bitmap to another is smaller.
		\par Parameters:
		<b>int y</b>\n\n
		Specifies the custom y offset. */
		BMMExport void           SetCustomY                  ( int y ) { custypos = y;    }
		/*! \remarks Returns the custom x offset setting of this BitmapInfo. */
		BMMExport int            GetCustomX                  ( ) { return custxpos;        }
		/*! \remarks Returns the custom Y offset setting of this BitmapInfo. */
		BMMExport int            GetCustomY                  ( ) { return custypos;        }
		/*! \remarks Sets a custom gamma setting of this BitmapInfo to the
		specified value.
		\par Parameters:
		<b>float g</b>\n\n
		Specifies the custom gamma setting. */
		BMMExport void           SetCustomGamma              ( float g ) { custgamma = g;  }
		/*! \remarks Returns the custom gamma setting of this BitmapInfo. */
		BMMExport float          GetCustomGamma              ( ) { return custgamma;       }
		/*! \remarks Returns the bitmap's currently used input gamma value. This is taken from either
		the image file's gamma, the system defaults, or the user override, according to the custom flags. */
		BMMExport float          GetEffectiveGamma           ( );
		/*! \remarks Sets the custom frame increment setting of this
		BitmapInfo.
		\par Parameters:
		<b>int s</b>\n\n
		Specifies the frame increment to use. */
		BMMExport void           SetCustomStep               ( int s ) { step = s;         }
		/*! \remarks Returns the custom frame step setting of this BitmapInfo.
		*/
		BMMExport int            GetCustomStep               ( ) { return step;            }
		/*! \remarks Establishes the optional alignment setting of this
		BitmapInfo. This specifies where to place the image if the image being
		copied from one Bitmap to another is smaller.
		\par Parameters:
		<b>int p</b>\n\n
		Specifies one of the following nine values that define the position of
		the bitmap:\n\n
		See \ref bitmapAlignmentPositions. */
		BMMExport void           SetPresetAlignment          ( int p ) { preset_al = p;    }
		/*! \remarks Returns the optional alignment setting of this
		BitmapInfo. This indicates where to place the image if the image being
		copied from one Bitmap to another is smaller.
		\return  See \ref bitmapAlignmentPositions. */
		BMMExport int            GetPresetAlignment          ( ) { return preset_al;       }

		//-- Custom Input Flags

		/*! \remarks Retrieves the custom flags setting of this BitmapInfo .
		See \ref bitmapFlags. */
		BMMExport DWORD          GetCustomFlags              ( ) { return (customflags);             }
		/*! \remarks Sets the custom flag(s) for this BitmapInfo.
		\par Parameters:
		<b>DWORD f</b>\n\n
		Specifies the custom flags. See \ref bitmapFlags. */
		BMMExport void           SetCustomFlag               ( DWORD f ) { customflags |=  f;        }
		/*! \remarks Clears the specified flag(s) of this BitmapInfo. See \ref bitmapFlags.
		\par Parameters:
		<b>DWORD f</b>\n\n
		Specifies the flag bits to reset. */
		BMMExport void           ResetCustomFlag             ( DWORD f ) { customflags &= ~f;        }
		/*! \remarks Tests the custom flags of this BitmapInfo. See \ref bitmapFlags.
		\par Parameters:
		<b>DWORD f</b>\n\n
		The flag bits to test.
		\return  Returns TRUE if the specified flags were set; otherwise FALSE.
		*/
		BMMExport BOOL           TestCustomFlags             ( DWORD f ) { return (customflags & f); }

		//-- Plug-In Parameter Block

		BMMExport void*			GetPiData                   ( ) { return pidata;          }
		BMMExport void          SetPiData                   ( void *ptr ) { pidata = ptr; }
		BMMExport DWORD         GetPiDataSize               ( ) { return pisize;          }
		BMMExport void          SetPiDataSize               ( DWORD s ) { pisize = s;     }
		BMMExport void          ResetPiData                 ( );
		BMMExport BOOL          AllocPiData                 ( DWORD size );

		//-- Proxy Subject info

		/*! \defgroup BitmapProxyInfoAccess Bitmap Proxy Subject Info
		For proxy bitmaps, the bitmap info describes the reduced-size proxy settings,
		while the info's proxy subject describes the full-size subject bitmap.
		Used when the info has the proxy flag toggled.
		\see MAP_PROXY
		*/
		//@{
		//! \brief Returns the subject info of the proxy.  When create is TRUE, the subject info will be allocated if it was null
		BMMExport BitmapInfo*	GetProxySubject             ( BOOL create=FALSE );
		BMMExport void          ResetProxySubject           ( ); //!< Deletes the subject info and sets it to null
		BMMExport BOOL          AllocProxySubject           ( ); //!< Creates the subject info. Info is initialized as per the default constructor
		//@}

		//-- Used to create Format Specific Parameter Block. Name and/or Device must be defined before using it.

		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		This method provides some access to device specific data (for instance
		the compression ratio in a JPEG file). This method will return a buffer
		containing a given device specific data (or NULL if the device
		referenced is unknown or doesn't have "specific data"). The buffer
		structure will depend on the device. For all drivers shipped with the
		SDK, this structure is defined in their header files (which must be
		included in the project for which this method is used). Internally,
		this method validates the driver, calls its <b>EvaluateConfigure()</b>
		method to define the buffer size, creates this buffer and, if the
		returned size is greater than zero, calls the driver's
		<b>SaveConfigure()</b> method in order to set default values. The
		developer may then change whatever they want, create and write a file
		using this <b>BitmapInfo</b> which includes the device's specific
		data.\n\n
		There is no need to free this buffer as this is handled by the
		<b>BitmapInfo</b> destructor.\n\n
		Note: The name and/or device properties must be defined before using
		this method. */
		BMMExport void*			CreateFmtSpecBlock			( void );

		//! \brief Assignment function. The data members of the specified BitmapInfo are copied to this BitmapInfo, with option to also copy the proxySubject info
		BMMExport void			Copy                        ( const BitmapInfo *from, BOOL copyProxySubject );
		BMMExport void          Copy                        ( const BitmapInfo *from ); //\\-- OBSOLETE --\\//
		/*! \remarks Assignment operator. The data members of the specified BitmapInfo
		are copied to this BitmapInfo.
		\par Parameters:
		<b>BitmapInfo \&from</b>\n\n
		The source BitmapInfo. */
		BMMExport BitmapInfo    &operator=                  ( const BitmapInfo &from );

		//-- Load/Save

		BMMExport IOResult       Save                        ( ISave *isave );
		BMMExport IOResult       Load                        ( ILoad *iload );

		BMMExport void           EnumAuxFiles                ( AssetEnumCallback& assetEnum, DWORD flags);
		  
		//-- Miscelaneous

		/*! \remarks Implemented by the System.\n\n
		This method is used to check the width, height, aspect ratio, and gamma
		settings to make sure they are within an acceptable range of values.
		The comparison is as follows:\n\n
		<b>if (width \< 1 ||</b>\n\n
		<b>height \< 1 ||</b>\n\n
		<b>aspect \<= 0.0 ||</b>\n\n
		<b>gamma \< MINGAMMA ||</b>\n\n
		<b>gamma \> MAXGAMMA)</b>\n\n
		<b>return (FALSE);</b>\n\n
		<b>else</b>\n\n
		<b>return (TRUE);</b>\n\n
		Where:\n\n
		<b>#define MINGAMMA 0.2f</b>\n\n
		<b>#define MAXGAMMA 5.0f</b>
		\return  TRUE if the BitmapInfo's settings are valid ; otherwise FALSE.
		*/
		BMMExport BOOL           Validate                    ( );
		/*! \remarks Returns the window handle to send progress or check abort
		messages to. */
		BMMExport HWND           GetUpdateWindow             ( )           { return hWnd; }
		/*! \remarks This is used internally - the system calls this method.
		This is how a window handle is sent down to device drivers and filters
		so they can send progress reports and check for cancel. */
		BMMExport void           SetUpdateWindow             ( HWND hwnd ) { hWnd = hwnd; }
		BMMExport DWORD          GetGChannels                ( );
		BMMExport DWORD          GetDeviceFlags              ( );
		BMMExport BOOL           FixDeviceName               ( ); //!< Sets the appropriate device name, as derived from the filename

};

//-----------------------------------------------------------------------------
//-- Bitmap I/O Class
//
//   None of these methods are to be used directly. Use the BitmapManager for
//   any image I/O.
//


/*! \sa  Class Bitmap, Class BitmapStorage, Class BitmapInfo, <a href="ms-its:3dsmaxsdk.chm::/bitmaps_root.html">Working with
Bitmaps</a>.\n\n
\par Description:
This is the base class used by developers creating image loader / saver
plug-ins. Developers implement pure virtual methods of this class to load the
image, open it for output, write to it, close it, and to provide information
about the image loader/saver they are creating. These are properties such as
the author name, copyright, image format description, filename extension(s)
used, and the capabilities of the image loader / saver.\n\n
When a BitmapIO derived image reader reads an image, it creates a storage class
that makes sense to it. For example, a paletted 8 bit is perfect for loading
GIF's but not for loading 32 bit Targas. The inverse is also true. There is no
point in creating a TRUE_64 storage to load a GIF. Because this is how image
buffers are managed, it is also important to note that if a developer writes an
image loader that creates images from scratch (a gradient generator for
instance), there is no need to have any real memory allocated. The plug-in
would simply derive a new type of <b>BitmapStorage</b> and provide the pixels
through the common methods (virtual buffer), creating them as they are
requested.
\par Method Groups:
See <a href="class_bitmap_i_o_groups.html">Method Groups for Class BitmapIO</a>.
\par Data Members:
protected:\n\n
<b>float gamma;</b>\n\n
The gamma setting.\n\n
<b>Bitmap *map;</b>\n\n
The Bitmap using this OUTPUT handler.\n\n
<b>BitmapStorage *storage;</b>\n\n
The storage used by this INPUT handler.\n\n
<b>int openMode;</b>\n\n
The mode that the IO module is open for. See \ref bitmapOpenModeTypes.\n\n
<b>BitmapIO *prevIO;</b>\n\n
A linked list pointer to the previous IO module for multiple outputs of a
single bitmap.\n\n
<b>BitmapIO *nextIO;</b>\n\n
A linked list pointer to the next IO module for multiple outputs of a single
bitmap.\n\n
<b>BitmapInfo bi;</b>\n\n
Describes the properties of the bitmap being handled by the instance of this
class.  */
#pragma warning(push)
#pragma warning(disable:4100)
class BitmapIO : public BaseInterfaceServer {
	private:
	  UWORD* outputGammaTab;   // this may be owned by gammaMgr
	  UWORD* privGammaTab;     // private gamma table owned by the BitmapIO.
		   
   protected:
	  float                   gamma;     
	  Bitmap                  *map;                        // The bitmap using this OUTPUT handler
	  BitmapStorage           *storage;                    // The storage used by this INPUT handler

	  int                      openMode;                   // See above
	  
	  //-- Linked list pointers for multiple output of a single bitmap

	  BitmapIO                *prevIO;                     
	  BitmapIO                *nextIO;

   public:

	  // Used by the subclassed BitmapIO's to get pixels for output with
	  // the appropriate output gamma correction.
	  /*! \remarks Implemented by the System.\n\n
	  This method is used by the subclassed BitmapIO to get pixels for output
	  with the appropriate output gamma correction.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>BMM_Color_64 *ptr</b>\n\n
	  Pointer to storage for the retrieved pixels. See
	  Structure BMM_Color_64.\n\n
	  <b>BOOL preMultAlpha=TRUE</b>\n\n
	  This parameter is available in release 3.0 and later only.\n\n
	  Setting this parameter to FALSE will cause pixels with non-premultiplied
	  alpha to be returned.
	  \return  Nonzero if the pixels were retrieved; otherwise zero. */
	  BMMExport  int           GetOutputPixels          ( int x,int y,int pixels,BMM_Color_64  *ptr, BOOL preMultAlpha=TRUE);
	  
	  // Used by the subclassed BitmapIO's to get pixels for output with
	  // the appropriate output gamma correction.
	  BMMExport  int           GetOutputPixels          ( int x,int y,int pixels,BMM_Color_fl  *ptr, BOOL preMultAlpha=TRUE);
	  
	  // Used by the subclassed BitmapIO's to get 32 bit pixels for output with
	  // the appropriate output gamma correction and dither. 
	  /*! \remarks Implemented by the System.\n\n
	  This method is used by the subclassed BitmapIO to get 32 bit pixels for
	  output with the appropriate output gamma correction and dither. Note that
	  this method works on only a single scanline of pixels at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>BMM_Color_32 *ptr</b>\n\n
	  Pointer to storage for the retrieved pixels. See
	  Structure BMM_Color_32.\n\n
	  <b>BOOL preMultAlpha=TRUE</b>\n\n
	  This parameter is available in release 3.0 and later only.\n\n
	  Setting this parameter to FALSE will cause pixels with non-premultiplied
	  alpha to be returned.
	  \return  Nonzero if the pixels were retrieved; otherwise zero. */
	  BMMExport  int           GetDitheredOutputPixels  ( int x,int y,int pixels,BMM_Color_32  *ptr, BOOL preMultAlpha=TRUE);

	  // Used by the subclassed BitmapIO's to get a DIB  for output with
	  // the appropriate output gamma correction. 
	  /*! \remarks Implemented by the System.\n\n
	  This method is used by the subclassed BitmapIO to get a DIB for output
	  with the appropriate output gamma correction.
	  \par Parameters:
	  <b>int depth = 24</b>\n\n
	  Specifies the depth of the DIB. This may be either 24 or 32. */
	  BMMExport  PBITMAPINFO   GetOutputDib             ( int depth = 24   );

	  // Used by the subclassed BitmapIO's to get a DIB  for output with
	  // the appropriate output gamma correction and dither
	  /*! \remarks Implemented by the System.\n\n
	  This methods is used by the subclassed BitmapIO to get a DIB for output
	  with the appropriate output gamma correction and dither.
	  \par Parameters:
	  <b>int depth = 24</b>\n\n
	  Specifies the depth of the DIB. This may be either 24 or 32. */
	  BMMExport  PBITMAPINFO   GetDitheredOutputDib      ( int depth = 24   );

	  /*! \remarks Implemented by the System.\n\n
	  Returns the output gamma setting. */
	  BMMExport  float         OutputGamma();

	  // If a BitmapIO wants to do its own dithering, it should call
	  // these to find out if dithering is wanted.  If it is a 24 bit or
	  // 32 bit format, it would usually just call GetDitheredOutputPixels instead.
	  /*! \remarks Implemented by the System.\n\n
	  If a BitmapIO wants to do its own dithering, it should call this method
	  to find out if dithering is wanted for true color images. If it is a 24
	  bit or 32 bit format, it would usually just call
	  <b>GetDitheredOutputPixels()</b> instead.
	  \return  TRUE if dithering is desired; otherwise FALSE. */
	  BMMExport  BOOL          DitherTrueColor();
	  /*! \remarks Implemented by the System.\n\n
	  If a BitmapIO wants to do its own dithering, it should call this method
	  to find out if dithering is wanted for paletted images.
	  \return  TRUE if dithering is desired; otherwise FALSE. */
	  BMMExport  BOOL          DitherPaletted();

	  // Calculate a color palette for output color packing: gamma corrects
	  /*! \remarks Calculate a color palette for output color packing for the
	  map that is using this output handler (this is the map pointed at by the
	  protected data member <b>Bitmap *map</b>). This method performs gamma
	  correction. See Class ColorPacker, 	  Class Quantizer.
	  \par Parameters:
	  <b>int palsize</b>\n\n
	  The size of the palette to compute.\n\n
	  <b>BMM_Color_48 *pal</b>\n\n
	  Storage for the palette.
	  \return  Nonzero if the palette was computed; otherwise zero. */
	  BMMExport  int           CalcOutputPalette(int palsize, BMM_Color_48 *pal);
   
	  BMMExport                BitmapIO                    ( );
	  BMMExport virtual                ~BitmapIO                    ( );
	  
	  BitmapInfo               bi;
   
	  /*! \remarks Implemented by the System.\n\n
	  Returns the open mode setting. See \ref bitmapOpenModeTypes.
	  */
	  inline    int            OpenMode                    ( ) { return (openMode); }
	  /*! \remarks Implemented by the System.\n\n
	  This method is used internally. */
	  inline    void           SetPrev                     ( BitmapIO *prev) { prevIO = prev; };
	  /*! \remarks Implemented by the System.\n\n
	  This method is used internally. */
	  inline    void           SetNext                     ( BitmapIO *next) { nextIO = next; };
	  /*! \remarks Implemented by the System.\n\n
	  This method is used internally. */
	  inline    BitmapIO      *Prev                        ( ) { return prevIO; };
	  /*! \remarks Implemented by the System.\n\n
	  This method is used internally. */
	  inline    BitmapIO      *Next                        ( ) { return nextIO; };
	  
	  /*! \remarks Implemented by the System.\n\n
	  Returns a pointer to the BitmapStorage for this image input handler. */
	  BMMExport BitmapStorage *Storage                     ( );
	  /*! \remarks Implemented by the System.\n\n
	  Returns a pointer to the Bitmap using this output handler. */
	  inline    Bitmap        *Map                         ( ) { return map; };

	  /*! \remarks Returns the number of filename extensions supported by this IO module.
	  For example the EPS plug-in supports "EPS" and "PS", and thus returns 2.
	  */
	  BMMExport virtual  int            ExtCount                    ( ) = 0;                                // Number of extemsions supported
	  /*! \remarks The extensions are accessed using a virtual array. This method returns
	  the 'i-th' filename extension supported by the IO module, (i.e. "EPS").
	  \par Parameters:
	  <b>int i</b>\n\n
	  Specifies which filename extension to return. */
	  BMMExport virtual  const MCHAR   *Ext                         ( int n ) = 0;                          // Extension #n (i.e. "3DS")
	  /*! \remarks Returns a long ASCII description of the image format (i.e. "Targa 2.0
	  Image File"). */
	  BMMExport virtual  const MCHAR   *LongDesc                    ( ) = 0;                                // Long ASCII description (i.e. "Targa 2.0 Image File")
	  /*! \remarks Returns a short ASCII description of the image format (i.e. "Targa"). */
	  BMMExport virtual  const MCHAR   *ShortDesc                   ( ) = 0;                                // Short ASCII description (i.e. "Targa")
	  /*! \remarks Returns the ASCII Author name of the IO module. */
	  BMMExport virtual  const MCHAR   *AuthorName                  ( ) = 0;                                // ASCII Author name
	  /*! \remarks Returns the ASCII Copyright message for the IO module. */
	  BMMExport virtual  const MCHAR   *CopyrightMessage            ( ) = 0;                                // ASCII Copyright message
	  /*! \remarks Returns the IO module version number * 100 (i.e. v3.01 = 301) */
	  BMMExport virtual  UINT           Version                     ( ) = 0;                                // Version number * 100 (i.e. v3.01 = 301)
	  
	  /*! \remarks Returns the IO module capability flags. These describe the capabilities
	  of the plug-in such as if it supports reading images, writing images,
	  multiple files, and whether it has its own information and control dialog
	  boxes. See  \ref bitmapIOCapabilityFlag. */
	  BMMExport virtual  int            Capability                  ( ) = 0;                                // Returns IO module ability flags (see above)
	  /*! \remarks This method is called to show the plug-in's "About" box. This is called,
	  for example, from the About button of the Add Image Input Event dialog in
	  Video Post.
	  \par Parameters:
	  <b>HWND hWnd</b>\n\n
	  The handle of the owner window. */
	  BMMExport virtual  void           ShowAbout                   ( HWND hWnd ) = 0;                      // Show DLL's "About..." box

	  //-- If the BMMIO_OWN_VIEWER flag is set, this method will be called
	  //   whenever the user wants to view an image for this device. This
	  //   is for devices which can "play" image sequences such as AVI's, FLIC's, etc.
	  //-- TH 2/26/96 -- Added BOOL return to indicate if view worked.  If it didn't,
	  //   it returns FALSE and the caller can view by the normal mechanism.

	  /*! \remarks If the <b>BMMIO_OWN_VIEWER</b> flag is set in the flags returned from the
	  <b>Capability()</b> method, this method will be called whenever the user
	  wants to view an image for this device. This is for devices which can
	  "play" image sequences such as AVIs, FLCs, etc.
	  \par Parameters:
	  <b>HWND hWnd</b>\n\n
	  The handle of the owner window.\n\n
	  <b>BitmapInfo *bi</b>\n\n
	  The bitmap to view.
	  \return  TRUE if the viewing the image was successful; otherwise FALSE.
	  \par Default Implementation:
	  <b>{ return FALSE; }</b> */
	  BMMExport virtual  BOOL           ShowImage                   ( HWND hWnd, BitmapInfo *bi ) { return FALSE; }

	  //-- Show DLL's Control Panel
	  //
	  //   If the user exists through an Ok, this function will return TRUE.
	  //   If the user cancels out, it will return FALSE. False indicates
	  //   nothing has changed so the system won't bother asking the plug-in
	  //   if it wants to save data.
	  //
	  //   This function is only called if the plug-in has defined it supports
	  //   it (through the Capability  flag above).  The flag will indicate to
	  //   the plug-in what operation is this control for (read, write, or
	  //   generic).
	  //
	  
	  /*! \remarks Displays the Control Panel of the IO module. This function is only called
	  if the plug-in has defined it supports it (through the Capability flag
	  returned from <b>Capability()</b>, ie. <b>BMMIO_CONTROLREAD</b>, etc.).
	  See  \ref bitmapIOCapabilityFlag.
	  \par Parameters:
	  <b>HWND hWnd</b>\n\n
	  The handle of the owner window.\n\n
	  <b>DWORD flag</b>\n\n
	  Indicates to the plug-in what operation the control is for (read, write,
	  or generic). See \ref bitmapIOCapabilityFlag.
	  \return  If the user exits the box through an OK, this function should
	  return TRUE. If the user cancels out, it will should FALSE. FALSE
	  indicates nothing has changed so the system won't bother asking the
	  plug-in if it wants to save data.
	  \par Default Implementation:
	  <b>{ return FALSE; }</b> */
	  BMMExport virtual  BOOL           ShowControl                 ( HWND hWnd, DWORD flag ) { return FALSE; }
	  
	  //-- Parameter Block Load and Save ------------------------------------
	  //
	  //  The host will call EvaluateConfigure() to determine the buffer size
	  //  required by the plug-in.
	  //
	  //  SaveConfigure() will be called so the plug-in can transfer its
	  //  parameter block to the host ( ptr is a pre-allocated buffer).
	  //
	  //  LoadConfigure() will be called so the plug-in can load its 
	  //  parameter block back.
	  //  
	  //  Memory management is performed by the host using standard
	  //  LocalAlloc() and LocalFree().
	  //  
	  
	  /*! \remarks This method is called by 3ds Max to determine the buffer size required by
	  the plug-in. The plug-in implements this method and returns the number of
	  bytes of configuration data it needs to save.
	  \return  The buffer size required by the plug-in (in bytes). */
	  BMMExport virtual  DWORD          EvaluateConfigure           ( ) = 0;
	  /*! \remarks This method is called by 3ds Max to allow the plug-in to load any
	  configuration data.
	  \par Parameters:
	  <b>void *ptr</b>\n\n
	  Pointer initialized to point to the previously saved configuration data.
	  \return  Returns TRUE if the data was loaded properly; otherwise FALSE.
	  */
	  BMMExport virtual  BOOL           LoadConfigure               ( void *ptr ) = 0;
	  /*! \remarks This method is called by 3ds Max to allow the plug-in to save any
	  configuration data.
	  \par Parameters:
	  <b>void *ptr</b>\n\n
	  Pointer initialized to a pre-allocated buffer where the plug-in may save
	  data.
	  \return  Returns TRUE if the data was saved; otherwise FALSE. */
	  BMMExport virtual  BOOL           SaveConfigure               ( void *ptr ) = 0;

	  //  Cfg methods provide access to the plug-in's default options, as saved in its .cfg file
	  //  ReadCfg() loads the default options into the parameter block
	  //  WriteCfg() saves the parameter block as the defaults

	  /*! \remarks This is the configuration file containing the default
	  options. */
	  BMMExport virtual  void           GetCfgFilename              ( MCHAR *filename ) {}
	  /*! \remarks Loads the defaults into  BitmapIO's parameter block.
	  \return  TRUE. */
	  BMMExport virtual  BOOL           ReadCfg                     ( ) { return TRUE; }
	  /*! \remarks Saves the parameter block as the new defaults. */
	  BMMExport virtual  void           WriteCfg                    ( ) {}


	  //-- Used internaly to make sure current block belongs to Plug-In
	  
	  BMMExport virtual  BOOL           ValidatePiData              ( BitmapInfo *bi );

	  //-- System Interface
	  
	  /*! \remarks Implemented by the System.\n\n
	  Returns the state of the silent mode flag. If this flag is TRUE the
	  plug-in should NOT post a dialog displaying any error messages. */
	  BMMExport BOOL           SilentMode                  ( );


	  //-- Calculate Desired Frame
	  //
	  //   This is for multiframe sequences. It processes the desired frame
	  //   based on user options. It is used at the Load() function to find
	  //   out which frame to load.
	  //
	  //   "fbi"    is the one passed to Load()
	  //   "frame"  is a pointer to an integer to receive the frame number
	  
	  /*! \remarks Implemented by the System.\n\n
	  This method is for use with multi-frame sequences. It processes the
	  desired frame based on the user options. For example the user can tell
	  the system to hold on the last frame of the sequence, loop back to the
	  beginning, or return an error. This method does all the checking
	  automatically based on the BitmapInfo object passed and computes the
	  proper frame number.
	  \par Parameters:
	  <b>BitmapInfo *fbi</b>\n\n
	  A pointer to the BitmapInfo that contains the user options. This is the
	  instance passed to <b>Load()</b>.\n\n
	  <b>int *frame</b>\n\n
	  A pointer to an integer to receive the frame number
	  \return  One of the following values:\n\n
	  <b>BMMRES_SUCCESS</b>\n\n
	  <b>BMMRES_BADFRAME</b> */
	  BMMExport BMMRES         GetFrame                    ( BitmapInfo *fbi, int *frame);
	  
	  //-- Critical Error Handling
	  
	  /*! \remarks Implemented by the System.\n\n
	  This method may be called to present the user with the 3ds Max Image IO
	  Error dialog box. The dialog displays the bitmap file name or device
	  name, and the specified error message. The user may choose Cancel or
	  Retry from the dialog. An appropriate value is returned based on the
	  users selection.\n\n
	  This method is used to handle hardware I/O errors automatically or
	  display the given string.\n\n
	  In this method, if Silent Mode is on (for example network rendering is
	  being done), no dialog is presented and <b>BMMRES_ERRORTAKENCARE</b> is
	  returned.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  A pointer to the BitmapInfo. This is used to retrieve the file or device
	  name (using <b>bi-\>Name()</b> or <b>bi-\>Device()</b>).\n\n
	  <b>MCHAR *string = NULL</b>\n\n
	  The error message to present in the dialog. If NULL this method will
	  query the operating system for the last I/O error and give its own
	  interpretation of the error. This will work for all "File Not Found",
	  "Permission Denied", etc. type errors.
	  \return  One of the following values:\n\n
	  <b>BMMRES_ERRORTAKENCARE</b>\n\n
	  Returned if the user selected OK from the dialog or the error was taken
	  care of (silent mode was on).\n\n
	  <b>BMMRES_ERRORRETRY</b>\n\n
	  The user has selected Retry from the dialog. */
	  BMMExport BMMRES         ProcessImageIOError         ( BitmapInfo *bi, MCHAR *string = NULL);
	  /*! \remarks Implemented by the System.\n\n
	  This method may be called to present the user with the 3ds Max Image IO
	  Error dialog box displaying the specified error message based on the
	  error code passed. The user may choose Cancel or Retry from the dialog.
	  An appropriate value is returned based on the users selection. If Silent
	  Mode is on (for example network rendering is being done) no dialog is
	  presented and <b>BMMRES_ERRORTAKENCARE</b> is returned.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  A pointer to the BitmapInfo. This is used to retrieve the file or device
	  name (using <b>bi-\>Name()</b> or <b>bi-\>Device()</b>).\n\n
	  <b>int errorcode</b>\n\n
	  The error code. Pass one of the following values and the string shown to
	  its right will be presented.\n\n
	  <b>BMMRES_INTERNALERROR -</b> Internal Error\n\n
	  <b>BMMRES_NUMBEREDFILENAMEERROR -</b> Error Creating Numbered File
	  Name.\n\n
	  <b>BMMRES_BADFILEHEADER -</b> Invalid Image File Header\n\n
	  <b>BMMRES_CANTSTORAGE -</b> Error Creating Image Storage\n\n
	  <b>BMMRES_MEMORYERROR -</b> Memory Error\n\n
	  <b>BMMRES_BADFRAME -</b> Invalid Frame Requested\n\n
	  Any other values produce - Unknown Error
	  \return  One of the following values:\n\n
	  <b>BMMRES_ERRORTAKENCARE</b>\n\n
	  Returned if the user selected Cancel from the dialog or the error was
	  taken care of (silent mode was on).\n\n
	  <b>BMMRES_ERRORRETRY</b>\n\n
	  The user has selected Retry from the dialog.
	  \par Sample Code:
		\code 		
		BMMRES BitmapIO_JPEG::Write(int frame)		
		{		
		//-- If we haven't gone through an OpenOutput(), leave		
			if (openMode != BMM_OPEN_W)		
				return		
					(ProcessImageIOError(&bi,BMMRES_INTERNALERROR));		
		//-- Resolve Filename --------------------------------		
			MCHAR filename[MAX_PATH];		
			if (frame == BMM_SINGLEFRAME) {		
				_tcscpy(filename,bi.Name());		
			}		
			else {		
				if (!BMMCreateNumberedFilename(bi.Name(),frame,filename)) {		
					return		
						(ProcessImageIOError(&bi,BMMRES_NUMBEREDFILENAMEERROR));		
				}		
			}		
		\endcode      */
	  BMMExport BMMRES         ProcessImageIOError         ( BitmapInfo *bi, int errorcode);
	  
	  //---------------------------------------------------------------------
	  //-- Channels Required (for Output)
	  //
	  //   By setting this flag, the plug-in can request the host to generate
	  //   the given channels. Prior to Rendering, the host will scan the
	  //   plug-ins in the chain of events and list all types of channels
	  //   being requested. The plug-in, at the time of the Write() call, will
	  //   have access to these channels through the channel interface
	  //   described below in BitmapStorage().
	  //
	  //   The generation of these channels should not, normally, be a 
	  //   default setting for a plug-in. These channels are memory hungry and
	  //   if the plug-in won't use it, it should not ask for it. Normally
	  //   the plug-in would ask the user which channels to save and set only
	  //   the proper flags.
	  //
	  
	  /*! \remarks These are the channels required for output. By setting this flag, the
	  plug-in can request that 3ds Max generate the given channels. Prior to
	  rendering, 3ds Max will scan the plug-ins in the chain of events and list
	  all types of channels being requested. The plug-in, at the time of the
	  <b>Write()</b> method, will have access to these channels through the
	  channel interface described in <b>BitmapStorage</b>.
	  \return  See \ref gBufImageChannels.
	  \par Default Implementation:
	  <b>{ return BMM_CHAN_NONE; }</b> */
	  BMMExport virtual  DWORD          ChannelsRequired            ( ) { return BMM_CHAN_NONE; }
	  
	  //-- Image Info
	  
	  /*! \remarks This method will display a dialog with information about the given bitmap
	  (either defined in <b>bi.Name()</b>/<b>bi.Device()</b> or explicitly in
	  the filename passed). The default method will retrieve image information
	  using the mandatory <b>GetImageInfo()</b> and display a generic
	  information dialog. If an image loader / writer wants to show its own
	  info dialog, perhaps showing an image property not found in the generic
	  dialog, it can implement its own function (and notify the system using
	  the <b>BMM_INFODLG</b> flag in the capabilities flag).
	  \par Parameters:
	  <b>HWND hWnd</b>\n\n
	  The parent window handle calling the dialog.\n\n
	  <b>BitmapInfo *bi</b>\n\n
	  Defines the name of the bitmap or device (unless specified below).\n\n
	  <b>const MCHAR *filename = NULL</b>\n\n
	  Specifies the filename to use explicitly.
	  \return  The result of the operation. See \ref bitmapErrorCodes. */
	  BMMExport virtual  BMMRES         GetImageInfoDlg             ( HWND hWnd, BitmapInfo *bi, const MCHAR *filename = NULL ) {return BMMRES_NODRIVER;}
	  /*! \remarks The BitmapIO module implements this method to initialize the
	  <b>BitmapInfo</b> instance passed in with information about the image.
	  This information might be obtained from read the image header for
	  example. The BitmapInfo passed contains the name of the image to get the
	  information about.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  A pointer to an instance of the class <b>BitmapInfo</b>.
	  \return  If an error occurs, the plug-in should process the error
	  (display a message if appropriate) and return
	  <b>BMMRES_ERRORTAKENCARE</b>. If everything went OK, the plug-in should
	  return <b>BMMRES_SUCCESS</b>.
	  \par Sample Code:
		\code
		bi->SetWidth(640);
		bi->SetHeight(480);
		bi->SetType(BMM_TRUE_24);
		bi->SetAspect(1.0f);
		bi->SetGamma(1.0f);
		bi->SetFirstFrame(0);
		bi->SetLastFrame(0);
		return BMM_SUCCESS;
		\endcode */
			  BMMExport virtual  BMMRES         GetImageInfo                (            BitmapInfo *bi ) = 0;
	  
	  //-- Image File Loaders (IFL handlers)
	  
	  /*! \remarks This method is implemented by image file loaders (IFL handlers). It is
	  called to update the filename passed based on the properties of the
	  BitmapInfo passed. See the implementation of this method in
	  <b>/MAXSDK/SAMPLES/IO/IFL.CPP</b>.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  Specifies the properties of the IFL sequence.\n\n
	  <b>MCHAR *filename</b>\n\n
	  The filename to update based on the properties of <b>bi</b>.
	  \return  See \ref bitmapErrorCodes. */
	  BMMExport virtual  BMMRES         GetImageName                ( BitmapInfo *bi, MCHAR *filename) {filename[0]=0; return (BMMRES_SUCCESS);}

	  //-- Image I/O (Not to use directly)
	  
	  /*! \remarks This is the method that is called to actually load and store the image.
	  This method usually creates the storage for the image data, allocates
	  storage space, and puts the image to the storage one scanline at a time.
	  This method also usually sets the <b>BitmapIO::openMode</b> flag to
	  <b>BMM_OPEN_R</b> to indicate the image is loaded and open for reading.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  Points to an instance of class BitmapInfo. This has the name of the
	  bitmap / device to load.\n\n
	  <b>Bitmap *map</b>\n\n
	  This points to the bitmap to be loaded.\n\n
	  <b>BMMRES *status</b>\n\n
	  The result of the bitmap load operation. See \ref bitmapErrorCodes.
	  \return  The BitmapStorage created to manages this bitmap.
	  \par Sample Code:
	  See the <b>Load()</b> method of <b>/MAXSDK/SAMPLES/IO/JPEG/JPEG.CPP</b>.
	  */
	  BMMExport virtual  BitmapStorage *Load                        ( BitmapInfo *bi, Bitmap *map, BMMRES *status ) = 0;      
		/*! \remarks This method opens the image for output and prepare to write to it. This is the
		time that the plug-in receives the information about what to write, the flags,
		etc. When the <b>Write()</b> method is called the only thing passed is the
		frame number.
		\par Parameters:
		<b>BitmapInfo *bi</b>\n\n
		The image information.\n\n
		<b>Bitmap *map</b>\n\n
		Points to the bitmap to save.
		\return  Returns nonzero if everything was OK; otherwise 0.
		\par Sample Code:
		See the <b>OpenOutput()</b> method of <b>/MAXSDK/SAMPLES/IO/JPEG/JPEG.CPP</b>.
		*/
	  BMMExport virtual  BMMRES         OpenOutput                  ( BitmapInfo *bi, Bitmap *map );
		/*! \remarks This method is called to write the image from the <b>BitmapStorage</b> to disk.
		The data member <b>bi</b> contains the relevant information (for example
		<b>bi.Name()</b> contains the name of the file to write).
		\par Parameters:
		<b>int frame</b>\n\n
		Specifies the frame to write. For single image formats this will be
		<b>BMM_SINGLEFRAME</b>.
		\return  Returns nonzero if everything was OK; otherwise 0.
		\par Sample Code:
		See the <b>Write()</b> method of <b>/MAXSDK/SAMPLES/IO/JPEG/JPEG.CPP</b>. */
	  BMMExport virtual  BMMRES         Write                       ( int frame );
	  /*! \remarks Closes the output file and saves or discards the changes based on the
	  flag passed.
	  \par Parameters:
	  <b>int flag</b>\n\n
	  See \ref bitmapCloseTypes.
	  \return  Returns nonzero if output was closed successfully; otherwise 0.
	  \par Sample Code:
	  See the <b>Close()</b> method of <b>/MAXSDK/SAMPLES/IO/WSD/WSD.CPP</b>.
	  */
	  BMMExport virtual  int            Close                       ( int flag );
	  /*! \remarks Implemented by the System.\n\n
	  This method is used internally.
	  \par Default Implementation:
	  <b>{ return NULL; }</b> */
	  BMMExport virtual  PAVIFILE       GetPaviFile                 ( ) { return NULL; }

	  // used internally to build output gamma table

	  /*! \remarks Implemented by the System.\n\n
	  This method is used internally to build the output gamma table. */
	  BMMExport void			InitOutputGammaTable(BitmapInfo*bi);

	  //-- Evaluate Matching Frame (R2)

	  /*! \remarks	  The bitmap manager caches images in order to speed its process. When a
	  new image is requested, if it's\n\n
	  already loaded, a pointer to it is passed around as opposed to loading an
	  entire new copy of it. It does so by comparing the image name and the
	  frame number requested (in case of multiframe files and/or devices).\n\n
	  This works fine in most cases, however consider the following scenario.
	  The Accom device generates images just as if you were loading Targa
	  files. The comparison explained above works fine. The problem however, is
	  that within the Accom private setup, you can determine where in the Accom
	  to start reading frames. In other words, you may have a sequence of
	  images recorded in the Accom starting at frame 300 for instance. Once in
	  the Accom setup, you define your starting frame at 300 and whenever 3ds
	  Max requests a frame, the Accom driver offsets the requested frame number
	  by 300. For example, when 3ds Max is rendering its 10th frame and
	  requests a frame from the Accom (for a Map or for a background, etc.),
	  the Accom will see that 3ds Max wants frame 10 and the user had setup the
	  starting frame at 300. After computing the offset the Accom driver
	  returns frame 310. Now, if two or more maps are used in a scene, the
	  cache match mechanism explained above fails as it does not take in
	  consideration the "starting frame" parameter of the Accom driver. For it,
	  the name matches (Accom) and the frame number matches (frame 10 in the
	  example above). If two or more maps start at different positions within
	  the Accom, the first one defined will be used as it satisfies the
	  match.\n\n
	  To handle this condition this new method saves extra information (driver
	  private information) about a frame. Basically, part of the match process
	  is now handled by those drivers that implement this method.\n\n
	  If a driver has some private data (handled by its own setup dialogue)
	  that defines anything in how images are returned (in other words, if
	  different images are returned because of those private settings being
	  different), it must define the <b>BMMIO_EVALMATCH</b> return flag and
	  must also implement this method. The flag is necessary to avoid wasting
	  time setting up instances of the driver just to call the (possibly
	  unimplemented) method.
	  \par Parameters:
	  <b>MCHAR *matchString</b>\n\n
	  The match string. The driver simply builds a string made of its own "per
	  frame" parameters and retuns it so 3ds Max can compare it with another
	  instance of the driver. */
	  BMMExport virtual  void			EvalMatch		            ( MCHAR *matchString ) { matchString[0] = 0; }




};

#pragma warning(pop)
//-----------------------------------------------------------------------------
//-- Bitmap Storage Class
//
//   None of these methods are to be used directly. Use the Bitmap class for
//   any image read/write.
//

//-- Channel Operations (for Get/Put16Channel)

#define BMM_CHANNEL_RED          0    //-- Get/Put only Red
#define BMM_CHANNEL_GREEN        1    //-- Get/Put only Green
#define BMM_CHANNEL_BLUE         3    //-- Get/Put only Blue
#define BMM_CHANNEL_ALPHA        4    //-- Get/Put only Alpha
#define BMM_CHANNEL_Z            5    //-- Get/Put only Z
#define BMM_CHANNEL_LUMINANCE    6    //-- Get (R+G+B)/3

/*! \sa  Class BitmapManager, Class BitmapInfo, Class Bitmap, Class BitmapIO, \ref bitmapErrorCodes.\n\n
\par Description:
When an image is loaded the buffer that will hold it is an instance of this
class. This class provides methods that allow developers to access the image
data in a uniform manner even though the underlying storage might be 1-bit,
8-bit, 16-bit, 32-bit or 64-bit. Standard methods are available for getting /
putting pixels: <b>Get/PutPixels()</b>, <b>Get/Put16Gray()</b>,
<b>Get/Put64Pixels()</b>, <b>Get/PutTruePixels()</b>,
<b>Get/PutIndexPixels()</b>, etc.\n\n
Since a developer accesses the storage through this standard interface, certain
plug-in types may not need to allocate memory for the storage. For example, an
image loader that creates an image from scratch (such as a gradient generator).
This plug-in would simply derive a new type of <b>BitmapStorage</b> and provide
the pixels through the common methods, creating them as requested.\n\n
Note: The "<b>Get/PutPixels()</b>" methods of this class access the image a
single scanline at a time.\n\n
Also note: The following global function may be used by image loader/saver
plug-ins to create an instance of <b>BitmapStorage</b>: */
class BitmapStorage : public BaseInterfaceServer {

   friend class GcsBitmap; // Used in VIZ only

   protected:
   
	  int                      openMode;                   // See above
	  UINT                     usageCount;                 // Number of Bitmaps using this storage
	  BitmapManager           *manager;

	  int                      flags;
	  int                      type;                       // See "Basic bitmap types", below

	  BMM_Color_48             palette[256];               // 256 palette entries max
	  int                      paletteSlots;
	  UWORD                    *gammaTable;               // Gamma correction table
	  
	  RenderInfo               *rendInfo;
	  GBuffer                  *gbuffer;
   public:

	  BMMExport                BitmapStorage               ( );
	  BMMExport virtual        ~BitmapStorage               ( );

	  BitmapInfo               bi;
	   MCHAR*					evalString;

	  // gamma 
	  /*! \remarks Implemented by the System.\n\n
	  Sets the gamma setting to the value passed.
	  \par Parameters:
	  <b>floag gam</b>\n\n
	  The gamma setting to set. */
	  BMMExport   float        SetGamma(float gam);         
	  /*! \remarks Implemented by the System.\n\n
	  Returns nonzero if the gamma table has been allocated; otherwise 0. */
	  inline      int          HasGamma                    ( ) { return (gammaTable!=NULL)          ? 1:0; };
	  /*! \remarks Implemented by the System.\n\n
	  This method allocates or deallocates the gamma table.
	  \par Parameters:
	  <b>BOOL onOff</b>\n\n
	  If TRUE the gamma table is allocated; otherwise it is deleted. */
	  BMMExport   void         SetHasGamma(BOOL onOff);   
	  void               UpdateGammaTable(); 
	  /*! \remarks Implemented by the System.\n\n
	  This methods returns a pointer to a gamma table that can be used for
	  converting pixels using whatever gamma value is appropriate for the
	  storage. It is typically called inside of the <b>GetLinearPixels()</b>
	  method of the particular <b>BitmapStorage</b> subclasses which then use
	  the gamma table to convert pixel values to linear values. Plug-In
	  developers in most cases will not need to call this method directly. */
	  BMMExport   UWORD       *GetInputGammaTable();

	  /*! \remarks Implemented by the System.\n\n
	  Returns the bitmap manager for the storage. */
	  inline  BitmapManager   *Manager                     ( ) { return manager;     }
	  /*! \remarks Implemented by the System.\n\n
	  Returns the mode the storage was opened in. See \ref bitmapOpenModeTypes.
	  */
	  inline  int              OpenMode                    ( ) { return openMode;    }
	  /*! \remarks Implemented by the System.\n\n
	  Returns the width (horizontal dimension) of the storage's BitmapInfo
	  instance (<b>bi.Width()</b>). */
	  inline  int              Width                       ( ) { return bi.Width();  }
	  /*! \remarks Implemented by the System.\n\n
	  Returns the height (vertical dimension) of the storage's BitmapInfo
	  instance (<b>bi.Height()</b>). */
	  inline  int              Height                      ( ) { return bi.Height(); }
	  /*! \remarks Implemented by the System.\n\n
	  Returns the aspect ratio of the storage's BitmapInfo instance
	  (<b>bi.Aspect()</b>). */
	  inline  float            Aspect                      ( ) { return bi.Aspect(); }
	  /*! \remarks Implemented by the System.\n\n
	  Returns the gamma setting of the storage's BitmapInfo instance
	  (<b>bi.Gamma()</b>). */
	  inline  float            Gamma                       ( ) { return bi.Gamma();  }
	  
	  /*! \remarks Implemented by the System.\n\n
	  Determines if the image is paletted. If the image has a palette (indexed
	  color), the number of palette slots used is returned; otherwise zero. */
	  inline  int              Paletted                    ( ) { return (flags & MAP_PALETTED)            ? paletteSlots:0; }
	  /*! \remarks Implemented by the System.\n\n
	  Returns the dithered state of the image. If the image is dithered nonzero
	  is returned; otherwise 0, */
	  inline  int              IsDithered                  ( ) { return (flags & MAP_DITHERED)            ? 1:0; };
	  /*! \remarks Implemented by the System.\n\n
	  Determines if the image has pre-multiplied alpha. If the image has
	  pre-multiplied alpha nonzero is returned; otherwise 0. */
	  inline  int              PreMultipliedAlpha          ( ) { return (flags & MAP_ALPHA_PREMULTIPLIED) ? 1:0; };
	  /*! \remarks Implemented by the System.\n\n
	  Determines if the image has an alpha channel. If the image has an alpha
	  channel nonzero is returned; otherwise 0. */
	  inline  int              HasAlpha                    ( ) { return (flags & MAP_HAS_ALPHA)           ? 1:0; };
	  /*! \remarks	  Implemented by the System.\n\n
	  Set whether colors are scaled (on) or clamped (off) when converting from
	  <b>BMM_Color_fl</b> to <b>BMM_Color_64</b>. */
	  inline  void             UseScaleColors              ( int on ) { flags &= ~MAP_USE_SCALE_COLORS; if (on) flags |= MAP_USE_SCALE_COLORS; };
	  /*! \remarks	  Implemented by the System.\n\n
	  Returns the last value set by <b>UseScaleColors</b>. */
	  inline  int              ScaleColors                 ( ) { return (flags & MAP_USE_SCALE_COLORS)    ? 1:0; };
	  /*! \remarks	  Implemented by the System.\n\n
	  Converts in to out clamping the RGB components to 0 to 65535. The alpha
	  component is not copied.
	  \par Parameters:
	  <b>BMM_Color_64\& out</b>\n\n
	  The result of the conversion.\n\n
	  <b>BMM_Color_fl\& in</b>\n\n
	  The value to convert. */
	  inline  static void      ClampColor                  (BMM_Color_64& out, const BMM_Color_fl& in) { out.r = in.clipColor(in.r); out.g = in.clipColor(in.g); out.b = in.clipColor(in.b); }
	  /*! \remarks	  Implemented by the System.\n\n
	  Converts in to out clamping the RGB components to 0 to 65535.
	  \par Parameters:
	  <b>BMM_Color_64\& out</b>\n\n
	  The result of the conversion.\n\n
	  <b>BMM_Color_fl\& in</b>\n\n
	  The value to convert. */
	  inline  static void      ClampColorA                 (BMM_Color_64& out, const BMM_Color_fl& in) { ClampColor(out, in); out.a = in.clipColor(in.a); }
	  /*! \remarks	  Implemented by the System.\n\n
	  Converts in to out clamping the RGB components to 0 to 65535. The alpha
	  component is not copied.
	  \par Parameters:
	  <b>BMM_Color_64\& out</b>\n\n
	  The result of the conversion.\n\n
	  <b>BMM_Color_fl\& in</b>\n\n
	  The value to convert. */
	  BMMExport static void	   ScaleColor                  (BMM_Color_64& out, BMM_Color_fl in);
	  /*! \remarks	  Implemented by the System.\n\n
	  Converts in to out clamping the RGB components to 0 to 65535.
	  \par Parameters:
	  <b>BMM_Color_64\& out</b>\n\n
	  The result of the conversion.\n\n
	  <b>BMM_Color_fl\& in</b>\n\n
	  The value to convert. */
	  inline static void       ScaleColorA                 (BMM_Color_64& out, const BMM_Color_fl& in) { ScaleColor(out, in); out.a = in.clipColor(in.a); }
	  /*! \remarks	  Implemented by the System.\n\n
	  Converts in to out, using the value of <b>ScaleColors()</b> to determine
	  the clamping or scaling. The alpha component is not copied.
	  \par Parameters:
	  <b>BMM_Color_64\& out</b>\n\n
	  The result of the conversion.\n\n
	  <b>BMM_Color_fl\& in</b>\n\n
	  The value to convert. */
	  inline  void             ClampScaleColor             (BMM_Color_64& out, const BMM_Color_fl& in) { if (ScaleColors()) ScaleColor(out, in); else ClampColor(out, in); }
	  /*! \remarks	  Implemented by the System.\n\n
	  Converts in to out, using the value of <b>ScaleColors()</b> to determine
	  the clamping or scaling.
	  \par Parameters:
	  <b>BMM_Color_64\& out</b>\n\n
	  The result of the conversion.\n\n
	  <b>BMM_Color_fl\& in</b>\n\n
	  The value to convert. */
	  inline  void             ClampScaleColorA            (BMM_Color_64& out, const BMM_Color_fl& in) { if (ScaleColors()) ScaleColorA(out, in); else ClampColorA(out, in); }
	  
	  /*! \remarks Implemented by the System.\n\n
	  Returns the number of times this image is being used in the system. */
	  inline  int              UsageCount                  ( ) { return usageCount; };
	  /*! \remarks Implemented by the System.\n\n
	  Returns the type of bitmap managed by this storage. See \ref bitmapTypes. */
	  inline  int              Type                        ( ) { return type; };
	  /*! \remarks Implemented by the System.\n\n
	  Returns the bitmap flags. See \ref bitmapFlags. */
	  inline  int              Flags                       ( ) { return flags; };
	  /*! \remarks Implemented by the System.\n\n
	  Sets the specified flag bits. See \ref bitmapFlags.
	  \par Parameters:
	  <b>DWORD f</b>\n\n
	  The flags to set. */
	  inline  void             SetFlags                    ( DWORD f ) { flags |=  f; }
	  inline  void             ClearFlags                  ( DWORD f ) { flags &= (~f); } //!< Clears the given flags

	  /*! \remarks This method returns the number of bits per pixel for each
	  color component. For example a 24-bit TARGA has a <b>MaxRGBLevel()</b> of
	  8 (8 red, 8 green, and 8 blue). */
	  BMMExport virtual int             MaxRGBLevel                 ( ) = 0;
	  /*! \remarks Returns the number of bits per pixel in the alpha channel.
	  */
	  BMMExport virtual int             MaxAlphaLevel               ( ) = 0;
	  /*! \remarks	  Returns nonzero if this storage uses high dynamic range data; otherwise
	  zero. See the Advanced Topics secion
	  <a href="ms-its:3dsmaxsdk.chm::/bitmaps_root.html">Working With
	  Bitmaps</a> for details on High Dynamic Range bitmaps. */
	  BMMExport virtual int             IsHighDynamicRange          ( ) = 0;
	  
	  /*! \remarks This method is used to get a pointer to the beginning of the image
	  storage. Not all storage types can return a valid pointer. In those
	  cases, this method will set the passed <b>type</b> to <b>BMM_NO_TYPE</b>
	  and return NULL.
	  \par Parameters:
	  <b>int *type</b>\n\n
	  The type of storage is returned here. See \ref bitmapTypes.
	  \par Default Implementation:
	  <b>{ *type = BMM_NO_TYPE; return (NULL); }</b> */
	  BMMExport virtual void           *GetStoragePtr               ( int *type ) { *type = BMM_NO_TYPE; return (NULL); }
	  /*! \remarks	  This method will attempt to get a pointer to the beginning of the image
	  alpha storage. Not all storage types can return a valid pointer. In those
	  cases, this call will fail and the user should use some other method
	  described below.
	  \par Parameters:
	  <b>int *type</b>\n\n
	  The type of storage is returned here. See \ref bitmapTypes.
	  \par Default Implementation:
	  <b>{ *type = BMM_NO_TYPE; return (NULL); }</b>\n\n
	  Below are the standard methods for accessing image pixels. Important
	  Note: The following "GetPixels()" methods operate on a single scanline of
	  the image at a time. Thus the number of pixels+x must be less than the
	  width of the image. */
	  BMMExport virtual void           *GetAlphaPtr                 ( int *type ) { *type = BMM_NO_TYPE; return (NULL); }

	  //-- Scaling Tools

	  BMMExport virtual void           Scale       	               ( WORD *, int, WORD *, int );
	  /*! \remarks	  Implemented by the System.\n\n
	  This method is used internally. */
	  BMMExport virtual void           Scale       	               ( float *, int, float *, int );
	  BMMExport virtual BOOL           GetSRow     	               ( WORD *, int, WORD *, int );
	  /*! \remarks	  Implemented by the System.\n\n
	  This method is used internally. */
	  BMMExport virtual BOOL           GetSRow     	               ( float *, int, float *, int );
	  BMMExport virtual BOOL           PutSRow     	               ( WORD *, int, WORD *, int );
	  /*! \remarks	  Implemented by the System.\n\n
	  This method is used internally. */
	  BMMExport virtual BOOL           PutSRow     	               ( float *, int, float *, int );
	  BMMExport virtual BOOL           GetSCol     	               ( WORD *, WORD *, int, int );
	  /*! \remarks	  Implemented by the System.\n\n
	  This method is used internally. */
	  BMMExport virtual BOOL           GetSCol     	               ( float *, float *, int, int );
	  BMMExport virtual BOOL           PutSCol     	               ( WORD *, WORD *, int, int );
	  /*! \remarks	  Implemented by the System.\n\n
	  This method is used internally. */
	  BMMExport virtual BOOL           PutSCol     	               ( float *, float *, int, int );
	  BMMExport virtual BOOL           ScaleY      	               ( Bitmap *, BMM_Color_64 *, WORD *, WORD *, HWND, int cw = 0, int ch = 0 );
	  /*! \remarks	  Implemented by the System.\n\n
	  This method is used internally. */
	  BMMExport virtual BOOL           ScaleY      	               ( Bitmap *, BMM_Color_fl *, float *, float *, HWND, int cw = 0, int ch = 0 );
	  BMMExport virtual BOOL           ScaleX      	               ( Bitmap *, BMM_Color_64 *, WORD *, WORD *, HWND, int cw = 0, int ch = 0 );
	  /*! \remarks	  Implemented by the System.\n\n
	  This method is used internally. */
	  BMMExport virtual BOOL           ScaleX      	               ( Bitmap *, BMM_Color_fl *, float *, float *, HWND, int cw = 0, int ch = 0 );
	  BMMExport virtual int            StraightCopy                 ( Bitmap *from ) = 0;

	  //-- These are the standard methods for accessing image pixels
	  
	  /*! \remarks Implemented by the System.\n\n
	  Retrieves the specified 16 bit grayscale pixels from the storage. This
	  method operates on a single scanline of the image at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>WORD *ptr</b>\n\n
	  Pointer to storage for the retrieved pixels.
	  \return  Nonzero if pixels were retrieved; otherwise 0. */
	  BMMExport virtual int             Get16Gray                   ( int x,int y,int pixels,WORD  *ptr) = 0;
	  /*! \remarks Implemented by the System.\n\n
	  Stores the 16 bit grayscale pixels to the specified location in the
	  storage. This method operates on a single scanline of the image at a
	  time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Destination x location.\n\n
	  <b>int y</b>\n\n
	  Destination y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>WORD *ptr</b>\n\n
	  Pointer to storage for the pixels.
	  \return  Nonzero if pixels were stored; otherwise 0. */
	  BMMExport virtual int             Put16Gray                   ( int x,int y,int pixels,WORD  *ptr) = 0;
	  /*! \remarks Implemented by the System.\n\n
	  Retrieves the specified 16 bit grayscale pixels from the storage. This
	  method operates on a single scanline of the image at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>float *ptr</b>\n\n
	  Pointer to storage for the retrieved pixels.
	  \return  Nonzero if pixels were retrieved; otherwise 0. */
	  BMMExport virtual int             Get16Gray                   ( int x,int y,int pixels,float *ptr) = 0;
	  /*! \remarks Implemented by the System.\n\n
	  Stores the 16 bit grayscale pixels to the specified location in the
	  storage. This method operates on a single scanline of the image at a
	  time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Destination x location.\n\n
	  <b>int y</b>\n\n
	  Destination y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>float *ptr</b>\n\n
	  Pointer to storage for the pixels.
	  \return  Nonzero if pixels were stored; otherwise 0. */
	  BMMExport virtual int             Put16Gray                   ( int x,int y,int pixels,float *ptr) = 0;
	  /*! \remarks This method retrieves the specified 64 bit true color pixels from the
	  storage. Pixels returned from this method are NOT gamma corrected. These
	  have linear gamma (1.0). This method operates on a single scanline of the
	  image at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>BMM_Color_64 *ptr</b>\n\n
	  Pointer to storage for the pixels.
	  \return  Nonzero if pixels were retrieved; otherwise 0. */
	  BMMExport virtual int             GetLinearPixels             ( int x,int y,int pixels,BMM_Color_64  *ptr) = 0;
	  /*! \remarks Retrieves the specified 64-bit pixel values from the
	  bitmap. Note: This method provides access to pixel data one scanline at a
	  time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>BMM_Color_64 *ptr</b>\n\n
	  Pointer to storage for the retrieved pixel values. See
	  Structure BMM_Color_64.
	  \return  Returns nonzero if pixels were retrieved; otherwise 0. If
	  storage has not been allocated 0 is returned. */
	  BMMExport virtual int             GetPixels                   ( int x,int y,int pixels,BMM_Color_64  *ptr) = 0;
	  /*! \remarks Stores the specified 64-bit pixel values into the bitmap's
	  own local storage. The pointer you pass to this method may be freed or
	  reused as soon as the function returns. Note: This method provides access
	  to pixel data one scanline at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Destination x location.\n\n
	  <b>int y</b>\n\n
	  Destination y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>BMM_Color_64 *ptr</b>\n\n
	  Pixel values to store. See
	  Structure BMM_Color_64.
	  \return  Returns nonzero if pixels were stored; otherwise 0. If storage
	  has not been allocated 0 is returned. */
	  BMMExport virtual int             PutPixels                   ( int x,int y,int pixels,BMM_Color_64  *ptr) = 0;
	  /*! \remarks This method retrieves the specified 64 bit true color pixels from the
	  storage. Pixels returned from this method are NOT gamma corrected. These
	  have linear gamma (1.0). This method operates on a single scanline of the
	  image at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  Pointer to storage for the pixels.
	  \return  Nonzero if pixels were retrieved; otherwise 0. */
	  BMMExport virtual int             GetLinearPixels             ( int x,int y,int pixels,BMM_Color_fl  *ptr) = 0;
	  /*! \remarks Retrieves the specified 64-bit pixel values from the
	  bitmap. Note: This method provides access to pixel data one scanline at a
	  time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  Pointer to storage for the retrieved pixel values.
	  \return  Returns nonzero if pixels were retrieved; otherwise 0. If
	  storage has not been allocated 0 is returned. */
	  BMMExport virtual int             GetPixels                   ( int x,int y,int pixels,BMM_Color_fl  *ptr) = 0;
	  /*! \remarks Stores the specified 64-bit pixel values into the bitmap's
	  own local storage. The pointer you pass to this method may be freed or
	  reused as soon as the function returns. Note: This method provides access
	  to pixel data one scanline at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Destination x location.\n\n
	  <b>int y</b>\n\n
	  Destination y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  Pixel values to store.
	  \return  Returns nonzero if pixels were stored; otherwise 0. If storage
	  has not been allocated 0 is returned. */
	  BMMExport virtual int             PutPixels                   ( int x,int y,int pixels,BMM_Color_fl  *ptr) = 0;
	  /*! \remarks Implemented by the System.\n\n
	  Retrieves the specified index color pixels from the storage. This is used
	  to retrieve pixels from a paletted image. This method operates on a
	  single scanline of the image at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>unsigned char *ptr</b>\n\n
	  Pointer to storage for the pixels.
	  \return  Nonzero if pixels were retrieved; otherwise 0. */
	  BMMExport virtual int             GetIndexPixels              ( int x,int y,int pixels,unsigned char *ptr) = 0;
	  /*! \remarks Implemented by the System.\n\n
	  Stores the index color pixels to the specified location in the storage.
	  This method operates on a single scanline of the image at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Destination x location.\n\n
	  <b>int y</b>\n\n
	  Destination y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>unsigned char *ptr</b>\n\n
	  Pointer to the pixels to store.
	  \return  Nonzero if pixels were stored; otherwise 0. */
	  BMMExport virtual int             PutIndexPixels              ( int x,int y,int pixels,unsigned char *ptr) = 0;
	  
	  BMMExport virtual int             Fill                        (const BMM_Color_fl& fillColor);
	  BMMExport virtual int             Fill                        (const BMM_Color_64& fillColor);

	  /*! \remarks Adjusts the bitmap size to the specified dimensions. The image is not
	  resized to fit; it is cropped or filled with <b>fillcolor</b> pixels to
	  accommodate the new size.
	  \par Parameters:
	  <b>int width</b>\n\n
	  The new horizontal size for the bitmap.\n\n
	  <b>int height</b>\n\n
	  The new vertical size for the bitmap.\n\n
	  <b>BMM_Color_64 fillcolor</b>\n\n
	  If the bitmap's new size is bigger than its current size, this is the
	  color used to fill the new pixels. See
	  Structure BMM_Color_64.
	  \return  Nonzero if the image was cropped; otherwise 0. */
	  BMMExport virtual int             CropImage                   ( int width,int height,BMM_Color_64 fillcolor) = 0;
	  /*! \remarks	  	  Adjusts the bitmap size to the specified dimensions. The image is not
	  resized to fit; it is cropped or filled with <b>fillcolor</b> pixels to
	  accommodate the new size.
	  \par Parameters:
	  <b>int width</b>\n\n
	  The new horizontal size for the bitmap.\n\n
	  <b>int height</b>\n\n
	  The new vertical size for the bitmap.\n\n
	  <b>BMM_Color_fl fillcolor</b>\n\n
	  If the bitmap's new size is bigger than its current size, this is the
	  color used to fill the new pixels. See
	  Structure BMM_Color_fl.
	  \return  Nonzero if the image was cropped; otherwise 0. */
	  BMMExport virtual int             CropImage                   ( int width,int height,BMM_Color_fl fillcolor) = 0;
	  /*! \remarks Adjusts the bitmap size to the specified dimensions. The image is not
	  resized to fit; it is cropped or filled with <b>fillindex</b> pixels to
	  accommodate the new size.
	  \par Parameters:
	  <b>int width</b>\n\n
	  The new horizontal size for the bitmap.\n\n
	  <b>int height</b>\n\n
	  The new vertical size for the bitmap.\n\n
	  <b>int fillindex</b>\n\n
	  If the bitmap's new size is bigger than its current size, this is the
	  color used to fill the new pixels.
	  \return  Nonzero if the image was cropped; otherwise 0. */
	  BMMExport virtual int             CropImage                   ( int width,int height,int fillindex)  = 0;
	  /*! \remarks This method is no longer used. */
	  BMMExport virtual int             ResizeImage                 ( int width,int height,int newpalette) = 0;

	  /*! \remarks Copies the specified bitmap to this storage. The image is cropped to fit.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The bitmap to copy to this bitmap.\n\n
	  <b>BMM_Color_64 fillcolor</b>\n\n
	  The color to use if the source image is smaller than the destination
	  image. See Structure BMM_Color_64.
	  \return  Nonzero if the copy/crop was performed; otherwise zero. */
	  BMMExport virtual int             CopyCrop                    ( Bitmap *from, BMM_Color_64 fillcolor ) = 0;
	  /*! \remarks	  	  Copies the specified bitmap to this storage. The image is cropped to fit.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The bitmap to copy to this bitmap.\n\n
	  <b>BMM_Color_fl fillcolor</b>\n\n
	  The color to use if the source image is smaller than the destination
	  image. See Structure BMM_Color_fl.
	  \return  Nonzero if the copy/crop was performed; otherwise zero. */
	  BMMExport virtual int             CopyCrop                    ( Bitmap *from, BMM_Color_fl fillcolor ) = 0;
	  /*! \remarks Implemented by the System.\n\n
	  This method copies the specified bitmap to this storage. The source
	  bitmap is scaled to fit using a lower quality but faster algorithm than
	  <b>CopyScaleHigh()</b>.This is an internal function implemented within
	  BMM.DLL for copying bitmaps back and forth. If a developer creates new
	  storage type, they will automatically get these copy functions as these
	  are implemented in the base class.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The source bitmap.
	  \return  Nonzero if the copy/scale was performed; otherwise zero. */
	  BMMExport virtual int             CopyScaleLow                ( Bitmap *from ) = 0;
	  /*! \remarks Implemented by the System.\n\n
	  This method copies the specified bitmap to this storage. The source
	  bitmap is scaled to fit using a higher quality but slower algorithm than
	  <b>CopyScaleLow()</b>. This is an internal function implemented within
	  BMM.DLL for copying bitmaps back and forth. If a developer creates new
	  storage type, they will automatically get these copy functions as these
	  are implemented in the base class. */
	  BMMExport virtual int             CopyScaleHigh               ( Bitmap *from, HWND hWnd, BMM_Color_64 **buf = NULL, int w=0, int h=0 ) = 0;
	  /*! \remarks Implemented by the System.\n\n
	  This method copies the specified bitmap to this storage. The source
	  bitmap is scaled to fit using a higher quality but slower algorithm than
	  <b>CopyScaleLow()</b>. This is an internal function implemented within
	  BMM.DLL for copying bitmaps back and forth. If a developer creates new
	  storage type, they will automatically get these copy functions as these
	  are implemented in the base class. */
	  BMMExport virtual int             CopyScaleHigh               ( Bitmap *from, HWND hWnd, BMM_Color_fl **buf = NULL, int w=0, int h=0 ) = 0;

	  /*! \remarks Copies the specified bitmap to this storage. The image is cropped or
	  resized as specified.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The source bitmap.\n\n
	  <b>int operation</b>\n\n
	  The type of copy to perform:\n\n
	  <b>COPY_IMAGE_CROP</b>\n\n
	  Copy image to current map size using cropping if necessary.\n\n
	  <b>COPY_IMAGE_RESIZE_LO_QUALITY</b>\n\n
	  Resize the source image to the destination map size (draft quality).\n\n
	  <b>COPY_IMAGE_RESIZE_HI_QUALITY</b>\n\n
	  Resize source image to the destination map size (final quality).\n\n
	  <b>COPY_IMAGE_USE_CUSTOM</b>\n\n
	  Resize based on the Image Input Options (BitmapInfo *).\n\n
	  <b>BMM_Color_64 fillcolor</b>\n\n
	  Vacant areas of the bitmap are filled with <b>fillcolor</b> pixels if the
	  operation specified is <b>COPY_IMAGE_CROP</b> and one of the source
	  bitmap dimensions is less than the size of this bitmap. See
	  Structure BMM_Color_64.\n\n
	  <b>BitmapInfo *bi</b>\n\n
	  When using custom options (resize to fit, positioning, etc.) this is how
	  the flags are passed down to the Bitmap Manager. This is an optional
	  argument -- for simple copy operations, <b>*bi</b> can default to NULL.
	  If present, the code checks the option flags and acts accordingly.
	  \return  Nonzero if the copy was performed; otherwise 0. */
	  BMMExport virtual int             CopyImage                   ( Bitmap *from,int operation,BMM_Color_64 fillcolor, BitmapInfo *bi = NULL) = 0;
	  /*! \remarks Copies the specified bitmap to this storage. The image is cropped or
	  resized as specified.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The source bitmap.\n\n
	  <b>int operation</b>\n\n
	  The type of copy to perform:\n\n
	  <b>COPY_IMAGE_CROP</b>\n\n
	  Copy image to current map size using cropping if necessary.\n\n
	  <b>COPY_IMAGE_RESIZE_LO_QUALITY</b>\n\n
	  Resize the source image to the destination map size (draft quality).\n\n
	  <b>COPY_IMAGE_RESIZE_HI_QUALITY</b>\n\n
	  Resize source image to the destination map size (final quality).\n\n
	  <b>COPY_IMAGE_USE_CUSTOM</b>\n\n
	  Resize based on the Image Input Options (BitmapInfo *).\n\n
	  <b>BMM_Color_fl fillcolor</b>\n\n
	  Vacant areas of the bitmap are filled with <b>fillcolor</b> pixels if the
	  operation specified is <b>COPY_IMAGE_CROP</b> and one of the source
	  bitmap dimensions is less than the size of this bitmap. See
	  Structure BMM_Color_fl.\n\n
	  <b>BitmapInfo *bi</b>\n\n
	  When using custom options (resize to fit, positioning, etc.) this is how
	  the flags are passed down to the Bitmap Manager. This is an optional
	  argument -- for simple copy operations, <b>*bi</b> can default to NULL.
	  If present, the code checks the option flags and acts accordingly.
	  \return  Nonzero if the copy was performed; otherwise 0. */
	  BMMExport virtual int             CopyImage                   ( Bitmap *from,int operation,BMM_Color_fl fillcolor, BitmapInfo *bi = NULL) = 0;
	  /*! \remarks Copies the specified bitmap to this storage.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The source bitmap.\n\n
	  <b>int operation</b>\n\n
	  The type of copy to perform.\n\n
	  <b>COPY_IMAGE_CROP</b>\n\n
	  Copy image to current map size using cropping if necessary.\n\n
	  <b>COPY_IMAGE_RESIZE_LO_QUALITY</b>\n\n
	  Resize the source image to the destination map size (draft quality).\n\n
	  <b>COPY_IMAGE_RESIZE_HI_QUALITY</b>\n\n
	  Resize source image to destination map size (final quality).\n\n
	  <b>COPY_IMAGE_USE_CUSTOM</b>\n\n
	  Resize based on the Image Input Options (BitmapInfo *).\n\n
	  <b>int fillindex</b>\n\n
	  Vacant areas of the bitmap are filled with <b>fillindex</b> pixels if the
	  operation specified is <b>COPY_IMAGE_CROP</b> and one of the source
	  bitmap dimensions is less than the size of this bitmap.
	  \return  Nonzero if the copy was performed; otherwise 0. */
	  BMMExport virtual int             CopyImage                   ( Bitmap *from,int operation,int fillindex) = 0;
	  /*! \remarks Retrieves the specified portion of the palette of this storage.
	  \par Parameters:
	  <b>int start</b>\n\n
	  Zero based index of the first palette entry to retrieve.\n\n
	  <b>int count</b>\n\n
	  Number of palette entries to retrieve.\n\n
	  <b>BMM_Color_48 *ptr</b>\n\n
	  Points to storage for the palette values. See
	  Structure BMM_Color_48.
	  \return  Nonzero if the palette was retrieved; otherwise 0. */
	  BMMExport virtual int             GetPalette                  ( int start,int count,BMM_Color_48 *ptr) = 0;
	  /*! \remarks Sets the specified portion of the palette for this storage.
	  \par Parameters:
	  <b>int start</b>\n\n
	  First palette index entry to store.\n\n
	  <b>int count</b>\n\n
	  Number of palette entries to store.\n\n
	  <b>BMM_Color_48 *ptr</b>\n\n
	  Points to storage for the palette values. See
	  Structure BMM_Color_48.
	  \return  Nonzero if the palette was stored; otherwise 0.
	  \par Sample Code:
	  See <b>Load()</b> in <b>/MAXSDK/SAMPLES/IO/BMP/BMP.CPP</b>. */
	  BMMExport virtual int             SetPalette                  ( int start,int count,BMM_Color_48 *ptr) = 0;
	  /*! \remarks This method uses summed area table or pyramidal filtering to compute an
	  averaged color over a specified area.
	  \par Parameters:
	  <b>float u, float v</b>\n\n
	  The location in the bitmap to filter. These values go from 0.0 to 1.0
	  across the size of the bitmap.\n\n
	  <b>float du, float dv</b>\n\n
	  The size of the rectangle to sample. These values go from 0.0 to 1.0
	  across the size of the bitmap.\n\n
	  <b>BMM_Color_64 *ptr</b>\n\n
	  The result is returned here - the average over the specified area. See
	  Structure BMM_Color_64. */
	  BMMExport virtual int             GetFiltered                 ( float u,float v,float du,float dv,BMM_Color_64 *ptr) = 0;
	  /*! \remarks	  	  This method uses summed area table or pyramidal filtering to compute an
	  averaged color over a specified area and outputs to a floating point
	  color structure.
	  \par Parameters:
	  <b>float u, float v</b>\n\n
	  The location in the bitmap to filter. These values go from 0.0 to 1.0
	  across the size of the bitmap.\n\n
	  <b>float du, float dv</b>\n\n
	  The size of the rectangle to sample. These values go from 0.0 to 1.0
	  across the size of the bitmap.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  The result is returned here - the average over the specified area. */
	  BMMExport virtual int             GetFiltered                 ( float u,float v,float du,float dv,BMM_Color_fl *ptr) = 0;
	  
	  //-- User Interface
	  
	  /*! \remarks Implemented by the System.\n\n
	  This method is called to allocate image storage.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  Points to an instance of the BitmapInfo class describing the properties
	  of the bitmap.\n\n
	  <b>BitmapManager *manager</b>\n\n
	  Points to the BitmapManager for the bitmap.\n\n
	  <b>int openMode</b>\n\n
	  See \ref bitmapOpenModeTypes.
	  \return  Nonzero if storage was allocated; otherwise 0. */
	  BMMExport virtual int             Allocate                    ( BitmapInfo *bi,BitmapManager *manager,int openMode)  = 0;
	  BMMExport virtual int             Connect                     ( ) = 0;
	  BMMExport virtual int             Disconnect                  ( ) = 0;
	  BMMExport virtual int             MapReady                    ( ) = 0;
	  BMMExport virtual int             ClosestColor                ( BMM_Color_48 color);
	  BMMExport virtual int             ClosestColor                ( int r,int g,int b);

	  // GBuffer methods ----------------------
	  // get a pointer to specified channel: also determine its type for check
	  /*! \remarks Returns a pointer to specified geometry/graphics buffer channel, and
	  determines its pixel depth.
	  \par Parameters:
	  <b>ULONG channelID</b>\n\n
	  The channel to return a pointer to. See \ref gBufImageChannels.\n\n
	  <b>ULONG\& chanType</b>\n\n
	  The type of the returned channel. One of the following values:\n\n
	  <b>BMM_CHAN_TYPE_UNKNOWN</b>\n\n
	  Channel not of a known type.\n\n
	  <b>BMM_CHAN_TYPE_1</b>\n\n
	  1 bit per pixel\n\n
	  <b>BMM_CHAN_TYPE_8</b>\n\n
	  1 byte per pixel\n\n
	  <b>BMM_CHAN_TYPE_16</b>\n\n
	  1 word per pixel\n\n
	  <b>BMM_CHAN_TYPE_32</b>\n\n
	  2 words per pixel\n\n
	  <b>BMM_CHAN_TYPE_48</b>\n\n
	  3 words per pixel\n\n
	  <b>BMM_CHAN_TYPE_64</b>\n\n
	  4 words per pixel\n\n
	  <b>BMM_CHAN_TYPE_96</b>\n\n
	  6 words per pixel
	  \par Default Implementation:
	  <b>{ return NULL;}</b> */
	  BMMExport virtual void*    GetChannel(ULONG channelID, ULONG& chanType) { return gbuffer?gbuffer->GetChannel(channelID, chanType):NULL;}

	  /*! \remarks Returns a pointer to the G-Buffer associated with this
	  storage. */
	  GBuffer *GetGBuffer()  { return gbuffer; } 

	  // create the specified channels -- return channels present: (creates GBuffer if non-existent); 
	  /*! \remarks Create the specified channels.
	  \par Parameters:
	  <b>ULONG channelIDs</b>\n\n
	  Specifies the channels to create. See \ref gBufImageChannels.
	  \return  The channels that are present.
	  \par Default Implementation:
	  <b>{ return 0;}</b> */
	  BMMExport virtual ULONG    CreateChannels(ULONG channelIDs);
	  
	  // delete all the channels in channelIDs
	  /*! \remarks Delete the specified channels.
	  \par Parameters:
	  <b>ULONG channelIDs</b>\n\n
	  Specifies the channels to delete. See \ref gBufImageChannels. */
	  BMMExport virtual void     DeleteChannels(ULONG channelIDs) { if (gbuffer) gbuffer->DeleteChannels(channelIDs); }  

	  // query which channels are present
	  /*! \remarks Returns the channels that are present. See \ref gBufImageChannels.
	  \par Default Implementation:
	  <b>{ return 0; }</b> */
	  BMMExport virtual ULONG     ChannelsPresent() { return gbuffer?gbuffer->ChannelsPresent():0;  }


	  // For output bitmaps, can get RenderInfo, which is written by the
	  // renderer
	  // AllocRenderInfo will alloc only if RenderInfo doesn't yet exist.
	  /*! \remarks Output bitmaps can get an instance of the class <b>RenderInfo</b>, which
	  is written by the renderer. This method will allocate an instance only if
	  a <b>RenderInfo</b> doesn't yet exist.
	  \return  A pointer to a RenderInfo. See
	  Class RenderInfo. */
	  BMMExport RenderInfo* AllocRenderInfo(); 
	  // GetRenderInfo just hands back RenderInfo pointer
	  /*! \remarks Returns a <b>RenderInfo</b> pointer. See
	  Class RenderInfo. */
	  BMMExport RenderInfo* GetRenderInfo();

};

//-----------------------------------------------------------------------------
//-- Low Dynamic Range Bitmap Storage Class
//
//   None of these methods are to be used directly. Use the Bitmap class for
//   any image read/write.
//

/*! \sa  Class BitmapStorage.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is the base class for the development of plug-in Bitmap Storage plug-ins
that don't use Hight Dynamic Range bitmaps.\n\n
Note that <b>bVirtual</b> is actuall a shortcut macro for <b>BMMExport
virtual</b>.\n\n
All methods of this class are implemented by the System.  */
class BitmapStorageLDR : public BitmapStorage {
   public:

	  using BitmapStorage::Get16Gray;
	  using BitmapStorage::Put16Gray;
	  using BitmapStorage::GetLinearPixels;
	  using BitmapStorage::GetPixels;
	  using BitmapStorage::PutPixels;
	  
	  using BitmapStorage::CropImage;
	  using BitmapStorage::ResizeImage;

	  using BitmapStorage::CopyCrop;
	  using BitmapStorage::CopyScaleLow;

	  using BitmapStorage::CopyImage;
	  using BitmapStorage::GetFiltered;

	  /*! \remarks This method returns 0 if the bitmap is not a high dynamic
	  range bitmap or 1 if it is.
	  \par Default Implementation:
	  <b>{ return(0); }</b>\n\n
		*/
	  BMMExport virtual int             IsHighDynamicRange          ( ) { return 0; }

	  //-- Scaling Tools

	  /*! \remarks Implemented by the System.\n\n
	  This method does a straightforward copy from the specified bitmap.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The bitmap to copy from. */
	  BMMExport virtual int            StraightCopy                 ( Bitmap *from );

	  //-- These are the standard methods for accessing image pixels
	  
	  /*! \remarks Implemented by the System.\n\n
	  Retrieves the specified 16 bit grayscale pixels from the storage. This
	  method operates on a single scanline of the image at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>float *ptr</b>\n\n
	  Pointer to the storage for the retrieved pixels.
	  \return  Nonzero if pixels were retrieved; otherwise 0. */
	  BMMExport virtual int             Get16Gray                   ( int x,int y,int pixels,float *ptr);
	  /*! \remarks Implemented by the System.\n\n
	  Stores the 16 bit grayscale pixels to the specified location in the
	  storage. This method operates on a single scanline of the image at a
	  time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Destination x location.\n\n
	  <b>int y</b>\n\n
	  Destination y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>float *ptr</b>\n\n
	  Pointer to the storage for the pixels.
	  \return  Nonzero if pixels were stored; otherwise 0. */
	  BMMExport virtual int             Put16Gray                   ( int x,int y,int pixels,float *ptr);
	  /*! \remarks This method retrieves the specified 64 bit true color pixels from the
	  storage. Pixels returned from this method are NOT gamma corrected. These
	  have linear gamma (1.0). This method operates on a single scanline of the
	  image at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  Pointer to the storage for the retrieved pixels.
	  \return  Nonzero if pixels were retrieved; otherwise 0. */
	  BMMExport virtual int             GetLinearPixels             ( int x,int y,int pixels,BMM_Color_fl  *ptr);
	  /*! \remarks Retrieves the specified 64-bit pixel values from the
	  bitmap. Note: This method provides access to pixel data one scanline at a
	  time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  Pointer to the storage for the retrieved pixels.
	  \return  Returns nonzero if pixels were retrieved; otherwise 0. If
	  storage has not been allocated 0 is returned. */
	  BMMExport virtual int             GetPixels                   ( int x,int y,int pixels,BMM_Color_fl  *ptr);
	  /*! \remarks Stores the specified 64-bit pixel values into the bitmap's
	  own local storage. The pointer you pass to this method may be freed or
	  reused as soon as the function returns. Note: This method provides access
	  to pixel data one scanline at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Destination x location.\n\n
	  <b>int y</b>\n\n
	  Destination y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  The pixels values to store.
	  \return  Returns nonzero if pixels were stored; otherwise 0. If storage
	  has not been allocated 0 is returned. */
	  BMMExport virtual int             PutPixels                   ( int x,int y,int pixels,BMM_Color_fl  *ptr);
	  /*! \remarks Implemented by the System.\n\n
	  Retrieves the specified index color pixels from the storage. This is used
	  to retrieve pixels from a paletted image. This method operates on a
	  single scanline of the image at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>unsigned char *ptr</b>\n\n
	  Pointer to the storage for the pixels.
	  \return  Nonzero if pixels were retrieved; otherwise 0. */
	  BMMExport virtual int             GetIndexPixels              ( int x,int y,int pixels,unsigned char *ptr) = 0;
	  /*! \remarks Implemented by the System.\n\n
	  Stores the index color pixels to the specified location in the storage.
	  This method operates on a single scanline of the image at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Destination x location.\n\n
	  <b>int y</b>\n\n
	  Destination y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>unsigned char *ptr</b>\n\n
	  The pixels values to store.
	  \return  Nonzero if pixels were stored; otherwise 0. */
	  BMMExport virtual int             PutIndexPixels              ( int x,int y,int pixels,unsigned char *ptr) = 0;
	  
	  /*! \remarks Adjusts the bitmap size to the specified dimensions. The image is not
	  resized to fit; it is cropped or filled with fillcolor pixels to
	  accommodate the new size.
	  \par Parameters:
	  <b>int width</b>\n\n
	  The new horizontal size for the bitmap.\n\n
	  <b>int height</b>\n\n
	  The new vertical size for the bitmap.\n\n
	  <b>BMM_Color_fl fillcolor</b>\n\n
	  If the bitmap's new size is bigger than its current size, this is the
	  color used to fill the new pixels.
	  \return  Nonzero if the image was cropped; otherwise 0. */
	  BMMExport virtual int             CropImage                   ( int width,int height,BMM_Color_fl fillcolor);

	  /*! \remarks Copies the specified bitmap to this storage. The image is cropped to fit.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The bitmap to copy to this bitmap.\n\n
	  <b>BMM_Color_64 fillcolor</b>\n\n
	  The color to use if the source image is smaller than the destination
	  image.
	  \return  Nonzero if the copy/crop was performed; otherwise zero. */
	  BMMExport virtual int             CopyCrop                    ( Bitmap *from, BMM_Color_64 fillcolor );
	  /*! \remarks Copies the specified bitmap to this storage. The image is cropped to fit.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The bitmap to copy to this bitmap.\n\n
	  <b>BMM_Color_fl fillcolor</b>\n\n
	  The color to use if the source image is smaller than the destination
	  image.
	  \return  Nonzero if the copy/crop was performed; otherwise zero. */
	  BMMExport virtual int             CopyCrop                    ( Bitmap *from, BMM_Color_fl fillcolor );
	  /*! \remarks Implemented by the System.\n\n
	  This method copies the specified bitmap to this storage. The source
	  bitmap is scaled to fit using a lower quality but faster algorithm than
	  <b>CopyScaleHigh()</b>.This is an internal function implemented within
	  BMM.DLL for copying bitmaps back and forth. If a developer creates new
	  storage type, they will automatically get these copy functions as these
	  are implemented in the base class.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The bitmap to copy to this bitmap.
	  \return  Nonzero if the copy/scale was performed; otherwise zero. */
	  BMMExport virtual int             CopyScaleLow                ( Bitmap *from );
	  /*! \remarks Implemented by the System.\n\n
	  This method copies the specified bitmap to this storage. The source
	  bitmap is scaled to fit using a higher quality but slower algorithm than
	  <b>CopyScaleLow()</b>. This is an internal function implemented within
	  BMM.DLL for copying bitmaps back and forth. If a developer creates new
	  storage type, they will automatically get these copy functions as these
	  are implemented in the base class. */
	  BMMExport virtual int             CopyScaleHigh               ( Bitmap *from, HWND hWnd, BMM_Color_64 **buf = NULL, int w=0, int h=0 );
	  /*! \remarks Implemented by the System.\n\n
	  This method copies the specified bitmap to this storage. The source
	  bitmap is scaled to fit using a higher quality but slower algorithm than
	  <b>CopyScaleLow()</b>. This is an internal function implemented within
	  BMM.DLL for copying bitmaps back and forth. If a developer creates new
	  storage type, they will automatically get these copy functions as these
	  are implemented in the base class. */
	  BMMExport virtual int             CopyScaleHigh               ( Bitmap *from, HWND hWnd, BMM_Color_fl **buf = NULL, int w=0, int h=0 );

	  /*! \remarks Copies the specified bitmap to this storage. The image is cropped or
	  resized as specified.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The source bitmap.\n\n
	  <b>int operation</b>\n\n
	  The type of copy to perform:\n\n
	  <b>COPY_IMAGE_CROP</b>\n\n
	  Copy image to current map size using cropping if necessary.\n\n
	  <b>COPY_IMAGE_RESIZE_LO_QUALITY</b>\n\n
	  Resize the source image to the destination map size (draft quality).\n\n
	  <b>COPY_IMAGE_RESIZE_HI_QUALITY</b>\n\n
	  Resize source image to the destination map size (final quality).\n\n
	  <b>COPY_IMAGE_USE_CUSTOM</b>\n\n
	  Resize based on the Image Input Options (BitmapInfo *).\n\n
	  <b>BMM_Color_64 fillcolor</b>\n\n
	  Vacant areas of the bitmap are filled with fillcolor pixels if the
	  operation specified is <b>COPY_IMAGE_CROP</b> and one of the source
	  bitmap dimensions is less than the size of this bitmap.\n\n
	  <b>BitmapInfo *bi = NULL</b>\n\n
	  When using custom options (resize to fit, positioning, etc.) this is how
	  the flags are passed down to the Bitmap Manager. This is an optional
	  argument -- for simple copy operations, <b>*bi</b> can default to NULL.
	  If present, the code checks the option flags and acts accordingly.
	  \return  Nonzero if the copy was performed; otherwise 0. */
	  BMMExport virtual int             CopyImage                   ( Bitmap *from,int operation,BMM_Color_64 fillcolor, BitmapInfo *bi = NULL);
	  /*! \remarks Copies the specified bitmap to this storage.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The source bitmap.\n\n
	  <b>int operation</b>\n\n
	  The type of copy to perform:\n\n
	  <b>COPY_IMAGE_CROP</b>\n\n
	  Copy image to current map size using cropping if necessary.\n\n
	  <b>COPY_IMAGE_RESIZE_LO_QUALITY</b>\n\n
	  Resize the source image to the destination map size (draft quality).\n\n
	  <b>COPY_IMAGE_RESIZE_HI_QUALITY</b>\n\n
	  Resize source image to the destination map size (final quality).\n\n
	  <b>COPY_IMAGE_USE_CUSTOM</b>\n\n
	  Resize based on the Image Input Options (BitmapInfo *).\n\n
	  <b>BMM_Color_fl fillcolor</b>\n\n
	  Vacant areas of the bitmap are filled with fillcolor pixels if the
	  operation specified is <b>COPY_IMAGE_CROP</b> and one of the source
	  bitmap dimensions is less than the size of this bitmap.\n\n
	  <b>BitmapInfo *bi = NULL</b>\n\n
	  When using custom options (resize to fit, positioning, etc.) this is how
	  the flags are passed down to the Bitmap Manager. This is an optional
	  argument -- for simple copy operations, <b>*bi</b> can default to NULL.
	  If present, the code checks the option flags and acts accordingly.
	  \return  Nonzero if the copy was performed; otherwise 0.\n\n
		*/
	  BMMExport virtual int             CopyImage                   ( Bitmap *from,int operation,BMM_Color_fl fillcolor, BitmapInfo *bi = NULL);
	  /*! \remarks Copies the specified bitmap to this storage.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The source bitmap.\n\n
	  <b>int operation</b>\n\n
	  The type of copy to perform:\n\n
	  <b>COPY_IMAGE_CROP</b>\n\n
	  Copy image to current map size using cropping if necessary.\n\n
	  <b>COPY_IMAGE_RESIZE_LO_QUALITY</b>\n\n
	  Resize the source image to the destination map size (draft quality).\n\n
	  <b>COPY_IMAGE_RESIZE_HI_QUALITY</b>\n\n
	  Resize source image to the destination map size (final quality).\n\n
	  <b>COPY_IMAGE_USE_CUSTOM</b>\n\n
	  Resize based on the Image Input Options (BitmapInfo *).\n\n
	  <b>int fillindex</b>\n\n
	  Vacant areas of the bitmap are filled with fillcolor pixels if the
	  operation specified is <b>COPY_IMAGE_CROP</b> and one of the source
	  bitmap dimensions is less than the size of this bitmap.
	  \return  Nonzero if the copy was performed; otherwise 0. */
	  BMMExport virtual int             CopyImage                   ( Bitmap *from,int operation,int fillindex);
	  /*! \remarks This method uses summed area table or pyramidal filtering to compute an
	  averaged color over a specified area.
	  \par Parameters:
	  <b>float u, float v</b>\n\n
	  The location in the bitmap to filter. These values go from 0.0 to 1.0
	  across the size of the bitmap.\n\n
	  <b>float du, float dv</b>\n\n
	  The size of the rectangle to sample. These values go from 0.0 to 1.0
	  across the size of the bitmap.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  The result is returned here - the average over the specified area. */
	  BMMExport virtual int             GetFiltered                 ( float u,float v,float du,float dv,BMM_Color_fl *ptr);
	  
};

//-----------------------------------------------------------------------------
//-- High Dynamic Range Bitmap Storage Class
//
//   None of these methods are to be used directly. Use the Bitmap class for
//   any image read/write.
//

/*! \sa  Class BitmapStorage.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is the base class for the development of plug-in Bitmap Storage plug-ins
that use Hight Dynamic Range bitmaps.\n\n
All methods of this class are implemented by the System.  */
class BitmapStorageHDR : public BitmapStorage {
   public:

	  using BitmapStorage::Get16Gray;
	  using BitmapStorage::Put16Gray;
	  using BitmapStorage::GetLinearPixels;
	  using BitmapStorage::GetPixels;
	  using BitmapStorage::PutPixels;
	  
	  using BitmapStorage::CropImage;
	  using BitmapStorage::ResizeImage;

	  using BitmapStorage::CopyCrop;
	  using BitmapStorage::CopyScaleLow;
	  using BitmapStorage::CopyScaleHigh;

	  using BitmapStorage::CopyImage;
	  using BitmapStorage::GetFiltered;

	  /*! \remarks Implemented by the System.\n\n
	  This method returns 0 if the bitmap is not a high dynamic range bitmap or
	  1 if it is.
	  \par Default Implementation:
	  <b>{ return(1); }</b> */
	  BMMExport virtual int             IsHighDynamicRange          ( ) { return 1; }

	  //-- Scaling Tools

	  /*! \remarks Implemented by the System.\n\n
	  This method does a straightforward copy from the specified bitmap.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The bitmap to copy from. */
	  BMMExport virtual int            StraightCopy                 ( Bitmap *from );

	  //-- These are the standard methods for accessing image pixels
	  
	  /*! \remarks Implemented by the System.\n\n
	  Retrieves the specified 16 bit grayscale pixels from the storage. This
	  method operates on a single scanline of the image at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>WORD *ptr</b>\n\n
	  Pointer to the storage for the retrieved pixels.
	  \return  Nonzero if pixels were retrieved; otherwise 0. */
	  BMMExport virtual int             Get16Gray                   ( int x,int y,int pixels,WORD  *ptr);
	  /*! \remarks Implemented by the System.\n\n
	  Stores the 16 bit grayscale pixels to the specified location in the
	  storage. This method operates on a single scanline of the image at a
	  time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Destination x location.\n\n
	  <b>int y</b>\n\n
	  Destination y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>WORD *ptr</b>\n\n
	  Pointer to the storage for the pixels.
	  \return  Nonzero if pixels were stored; otherwise 0. */
	  BMMExport virtual int             Put16Gray                   ( int x,int y,int pixels,WORD  *ptr);
	  /*! \remarks This method retrieves the specified 64 bit true color pixels from the
	  storage. Pixels returned from this method are NOT gamma corrected. These
	  have linear gamma (1.0). This method operates on a single scanline of the
	  image at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>BMM_Color_64 *ptr</b>\n\n
	  Pointer to the storage for the retrieved pixels.
	  \return  Nonzero if pixels were retrieved; otherwise 0. */
	  BMMExport virtual int             GetLinearPixels             ( int x,int y,int pixels,BMM_Color_64  *ptr);
	  /*! \remarks <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  Pointer to the storage for the retrieved pixels.
	  \return  Returns nonzero if pixels were retrieved; otherwise 0. If
	  storage has not been allocated 0 is returned. */
	  BMMExport virtual int             GetPixels                   ( int x,int y,int pixels,BMM_Color_64  *ptr);
	  /*! \remarks Stores the specified 64-bit pixel values into the bitmap's
	  own local storage. The pointer you pass to this method may be freed or
	  reused as soon as the function returns. Note: This method provides access
	  to pixel data one scanline at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Destination x location.\n\n
	  <b>int y</b>\n\n
	  Destination y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  The pixels values to store.
	  \return  Returns nonzero if pixels were stored; otherwise 0. If storage
	  has not been allocated 0 is returned. */
	  BMMExport virtual int             PutPixels                   ( int x,int y,int pixels,BMM_Color_64  *ptr);
	  
	  /*! \remarks Adjusts the bitmap size to the specified dimensions. The image is not
	  resized to fit; it is cropped or filled with fillcolor pixels to
	  accommodate the new size.
	  \par Parameters:
	  <b>int width</b>\n\n
	  The new horizontal size for the bitmap.\n\n
	  <b>int height</b>\n\n
	  The new vertical size for the bitmap.\n\n
	  <b>BMM_Color_64 fillcolor</b>\n\n
	  If the bitmap's new size is bigger than its current size, this is the
	  color used to fill the new pixels.
	  \return  Nonzero if the image was cropped; otherwise 0. */
	  BMMExport virtual int             CropImage                   ( int width,int height,BMM_Color_64 fillcolor);
	  /*! \remarks Adjusts the bitmap size to the specified dimensions. The image is not
	  resized to fit; it is cropped or filled with fillcolor pixels to
	  accommodate the new size.
	  \par Parameters:
	  <b>int width</b>\n\n
	  The new horizontal size for the bitmap.\n\n
	  <b>int height</b>\n\n
	  The new vertical size for the bitmap.\n\n
	  <b>int fillindex</b>\n\n
	  If the bitmap's new size is bigger than its current size, this is the
	  color used to fill the new pixels.
	  \return  Nonzero if the image was cropped; otherwise 0. */
	  BMMExport virtual int             CropImage                   ( int width,int height,int fillindex);

	  /*! \remarks Copies the specified bitmap to this storage. The image is cropped to fit.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The bitmap to copy to this bitmap.\n\n
	  <b>BMM_Color_64 fillcolor</b>\n\n
	  The color to use if the source image is smaller than the destination
	  image.
	  \return  Nonzero if the copy/crop was performed; otherwise zero. */
	  BMMExport virtual int             CopyCrop                    ( Bitmap *from, BMM_Color_64 fillcolor );
	  /*! \remarks Copies the specified bitmap to this storage. The image is cropped to fit.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The bitmap to copy to this bitmap.\n\n
	  <b>BMM_Color_fl fillcolor</b>\n\n
	  The color to use if the source image is smaller than the destination
	  image.
	  \return  Nonzero if the copy/crop was performed; otherwise zero. */
	  BMMExport virtual int             CopyCrop                    ( Bitmap *from, BMM_Color_fl fillcolor );
	  /*! \remarks Implemented by the System.\n\n
	  This method copies the specified bitmap to this storage. The source
	  bitmap is scaled to fit using a lower quality but faster algorithm than
	  <b>CopyScaleHigh()</b>.This is an internal function implemented within
	  BMM.DLL for copying bitmaps back and forth. If a developer creates new
	  storage type, they will automatically get these copy functions as these
	  are implemented in the base class.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The bitmap to copy to this bitmap.
	  \return  Nonzero if the copy/scale was performed; otherwise zero. */
	  BMMExport virtual int             CopyScaleLow                ( Bitmap *from );
	  /*! \remarks Implemented by the System.\n\n
	  This method copies the specified bitmap to this storage. The source
	  bitmap is scaled to fit using a higher quality but slower algorithm than
	  <b>CopyScaleLow()</b>. This is an internal function implemented within
	  BMM.DLL for copying bitmaps back and forth. If a developer creates new
	  storage type, they will automatically get these copy functions as these
	  are implemented in the base class. */
	  BMMExport virtual int             CopyScaleHigh               ( Bitmap *from, HWND hWnd, BMM_Color_64 **buf = NULL, int w=0, int h=0 );
	  /*! \remarks Implemented by the System.\n\n
	  This method copies the specified bitmap to this storage. The source
	  bitmap is scaled to fit using a higher quality but slower algorithm than
	  <b>CopyScaleLow()</b>. This is an internal function implemented within
	  BMM.DLL for copying bitmaps back and forth. If a developer creates new
	  storage type, they will automatically get these copy functions as these
	  are implemented in the base class. */
	  BMMExport virtual int             CopyScaleHigh               ( Bitmap *from, HWND hWnd, BMM_Color_fl **buf = NULL, int w=0, int h=0 );

	  /*! \remarks Copies the specified bitmap to this storage. The image is cropped or
	  resized as specified.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The source bitmap.\n\n
	  <b>int operation</b>\n\n
	  The type of copy to perform:\n\n
	  <b>COPY_IMAGE_CROP</b>\n\n
	  Copy image to current map size using cropping if necessary.\n\n
	  <b>COPY_IMAGE_RESIZE_LO_QUALITY</b>\n\n
	  Resize the source image to the destination map size (draft quality).\n\n
	  <b>COPY_IMAGE_RESIZE_HI_QUALITY</b>\n\n
	  Resize source image to the destination map size (final quality).\n\n
	  <b>COPY_IMAGE_USE_CUSTOM</b>\n\n
	  Resize based on the Image Input Options (BitmapInfo *).\n\n
	  <b>BMM_Color_64 fillcolor</b>\n\n
	  Vacant areas of the bitmap are filled with fillcolor pixels if the
	  operation specified is <b>COPY_IMAGE_CROP</b> and one of the source
	  bitmap dimensions is less than the size of this bitmap.\n\n
	  <b>BitmapInfo *bi = NULL</b>\n\n
	  When using custom options (resize to fit, positioning, etc.) this is how
	  the flags are passed down to the Bitmap Manager. This is an optional
	  argument -- for simple copy operations, <b>*bi</b> can default to NULL.
	  If present, the code checks the option flags and acts accordingly.
	  \return  Nonzero if the copy was performed; otherwise 0. */
	  BMMExport virtual int             CopyImage                   ( Bitmap *from,int operation,BMM_Color_64 fillcolor, BitmapInfo *bi = NULL);
	  /*! \remarks Copies the specified bitmap to this storage.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The source bitmap.\n\n
	  <b>int operation</b>\n\n
	  The type of copy to perform:\n\n
	  <b>COPY_IMAGE_CROP</b>\n\n
	  Copy image to current map size using cropping if necessary.\n\n
	  <b>COPY_IMAGE_RESIZE_LO_QUALITY</b>\n\n
	  Resize the source image to the destination map size (draft quality).\n\n
	  <b>COPY_IMAGE_RESIZE_HI_QUALITY</b>\n\n
	  Resize source image to the destination map size (final quality).\n\n
	  <b>COPY_IMAGE_USE_CUSTOM</b>\n\n
	  Resize based on the Image Input Options (BitmapInfo *).\n\n
	  <b>BMM_Color_fl fillcolor</b>\n\n
	  Vacant areas of the bitmap are filled with fillcolor pixels if the
	  operation specified is <b>COPY_IMAGE_CROP</b> and one of the source
	  bitmap dimensions is less than the size of this bitmap.\n\n
	  <b>BitmapInfo *bi = NULL</b>\n\n
	  When using custom options (resize to fit, positioning, etc.) this is how
	  the flags are passed down to the Bitmap Manager. This is an optional
	  argument -- for simple copy operations, <b>*bi</b> can default to NULL.
	  If present, the code checks the option flags and acts accordingly.
	  \return  Nonzero if the copy was performed; otherwise 0. */
	  BMMExport virtual int             CopyImage                   ( Bitmap *from,int operation,BMM_Color_fl fillcolor, BitmapInfo *bi = NULL);
	  /*! \remarks Copies the specified bitmap to this storage.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The source bitmap.\n\n
	  <b>int operation</b>\n\n
	  The type of copy to perform:\n\n
	  <b>COPY_IMAGE_CROP</b>\n\n
	  Copy image to current map size using cropping if necessary.\n\n
	  <b>COPY_IMAGE_RESIZE_LO_QUALITY</b>\n\n
	  Resize the source image to the destination map size (draft quality).\n\n
	  <b>COPY_IMAGE_RESIZE_HI_QUALITY</b>\n\n
	  Resize source image to the destination map size (final quality).\n\n
	  <b>COPY_IMAGE_USE_CUSTOM</b>\n\n
	  Resize based on the Image Input Options (BitmapInfo *).\n\n
	  <b>int fillindex</b>\n\n
	  Vacant areas of the bitmap are filled with fillcolor pixels if the
	  operation specified is <b>COPY_IMAGE_CROP</b> and one of the source
	  bitmap dimensions is less than the size of this bitmap.
	  \return  Nonzero if the copy was performed; otherwise 0. */
	  BMMExport virtual int             CopyImage                   ( Bitmap *from,int operation,int fillindex);
	  /*! \remarks This method uses summed area table or pyramidal filtering to compute an
	  averaged color over a specified area.
	  \par Parameters:
	  <b>float u, float v</b>\n\n
	  The location in the bitmap to filter. These values go from 0.0 to 1.0
	  across the size of the bitmap.\n\n
	  <b>float du, float dv</b>\n\n
	  The size of the rectangle to sample. These values go from 0.0 to 1.0
	  across the size of the bitmap.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  The result is returned here - the average over the specified area.\n\n
		*/
	  BMMExport virtual int             GetFiltered                 ( float u,float v,float du,float dv,BMM_Color_64 *ptr);
	  
};

//-----------------------------------------------------------------------------
//-- Bitmap Filter Class
//
//   Private class not to be documented
//

class BitmapFilter: public MaxHeapOperators {
   
   protected:
   
	  UINT                     usageCount;                 // Number of Bitmaps using this storage
	  BitmapManager           *manager;                    // Pointer to bitmap manager
	  BitmapStorage           *storage;                    // Pointer to storage itself
	  DWORD                    flags;                      // Filter flags
	  int                      dirty;                      // Needs updating flag
	  UINT                     type;                       // Type index of filter
	  
   public:
   
	  BMMExport                BitmapFilter();
	  BMMExport virtual                ~BitmapFilter();
	  
	  inline    DWORD          Flags                       ( ) { return flags; };
	  inline    void           SetFlag                     ( DWORD flag) { flags |= flag; dirty = 1; };
	  inline    void           ToggleFlag                  ( DWORD flag) { flags ^= flag; dirty = 1; };
	  inline    void           ClearFlag                   ( DWORD flag) { flags &= (~flag); dirty = 1; };
	  inline    UINT           Type                        ( ) { return type; };
	  inline    void           SetType                     ( UINT t) { type = t; };
	  BMMExport int            Initialize                  ( BitmapManager *m,BitmapStorage *s);

	  virtual   int            GetFiltered                 ( float u,float v,float du,float dv,BMM_Color_64 *ptr) = 0;
	  virtual   int            GetFiltered                 ( float u,float v,float du,float dv,BMM_Color_fl *ptr) = 0;
	  virtual   void           Free                        ( ) {};

	  BMMExport int            Connect                     ( );
	  BMMExport int            Disconnect                  ( );
	  BMMExport int            SetStorage                  ( BitmapStorage *storage);
	  inline  BitmapStorage   *GetStorage                  ( ) { return storage; };
	  inline    void           MakeDirty                   ( ) { dirty = 1; };

};

//-----------------------------------------------------------------------------
//-- Bitmap Dither Class
//
//   Private class not to be documented

class BitmapDither: public MaxHeapOperators {
   
   protected:
   
	  BitmapStorage           *storage;                    // Pointer to storage itself
	  int                      type;                       // Type index of filter
	  
   public:
	
	  BMMExport                BitmapDither                ( );
	  BMMExport virtual                ~BitmapDither                ( );
	  inline    UINT           Type                        ( ) { return type; };
	  inline    void           SetType                     ( UINT t) { type = t; };
	  BMMExport int            Initialize                  ( BitmapStorage *s);
	  virtual   int            PutPixels                 ( int x,int y,int pixels,BMM_Color_64 *ptr) = 0;
	  virtual   int            PutPixels                 ( int x,int y,int pixels,BMM_Color_fl *ptr) = 0;
	  virtual   void           Free                        ( ) {};
	  BMMExport int            SetStorage                  ( BitmapStorage *storage);
	
};


//-----------------------------------------------------------------------------
// Callback for notifying bitmaps that their Storage has changed, and 
// any on screen displays need to be refreshed.  Installed via
// Bitmap::SetNotify();
// VFBClosed is 
//

//-- New flag handling. The "flag" parameter was unused in relrease < 5.
// GG: 02/10/02

//-- Notifies the storage has changed (the contents of the bitmap). You should
// update to reflect the changes.
#define BMNOTIFY_FLAG_STORAGE_CHANGE	0
//-- Notifies the file has changed (probably by an external program). You
// should reload the bitmap. Note that by the time this call is made, the API has
// already checked to see if the user has set the global preferences asking for
// these changes to be automatically reloaded.
#define BMNOTIFY_FLAG_FILE_CHANGE		1

/*! \sa  Class Bitmap, Class BitmapStorage.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class is a callback for notifying bitmaps that their storage has changed,
and if any on screen displays need to be refreshed. This is installed as a
callback via the method <b>Bitmap::SetNotify()</b>.\n\n
All methods of this class are implemented by the system.  */
class BitmapNotify: public MaxHeapOperators{
public:
	//-- Call to notify clients the bitmap has changed.
	//-- flags can be one of the BMNOTIFY_FLAG_XXX flags above
	/*! \remarks This method is called when the storage for the Bitmap has
	changed.
	\par Parameters:
	<b>ULONG flags</b>\n\n
	One of the following:\n\n
	<b>BMNOTIFY_FLAG_STORAGE_CHANGE</b>, notifies that the storage (the
	contents of the bitmap) has changed.\n\n
	<b>BMNOTIFY_FLAG_FILE_CHANGE</b>, notifies that that bitmap file has
	changed, probably by an external program. The bitmap should be reloaded.
	Note that by the time this call is made, the API has already checked to see
	if the user has set the global preferences asking for these changes to be
	automatically reloaded. */
	virtual int Changed(ULONG flags)=0;
	/*! \remarks This method is called when Virtual Frame Buffer is closed.
	\par Default Implementation:
	<b>{}</b> */
	virtual void VFBClosed() {}  // called when VFB is closed
	};

//-----------------------------------------------------------------------------
// Callback for interactive adjustment of bitmap "Cropping rectangle", passed
// in as an argument to Bitmap::Display.
/*! \sa  Class Bitmap.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class is a callback for interactive adjustment of bitmap "Cropping
rectangle", passed in as an argument to the <b>Bitmap::Display()</b> method.
See <b>/MAXSDK/SAMPLES/MATERIALS/BMTEX.CPP</b> for sample code.\n\n
All methods of this class are virtual.  */
class CropCallback: public MaxHeapOperators {
	public:
	/*! \remarks Returns the initial U value. */
	virtual float GetInitU()=0;
	/*! \remarks Returns the initial V value. */
	virtual float GetInitV()=0;
	/*! \remarks Returns the initial W value. */
	virtual float GetInitW()=0;
	/*! \remarks Returns the initial H value. */
	virtual float GetInitH()=0;
	/*! \remarks Returns TRUE for place mode; FALSE for crop mode. In crop
	mode the image is clipped at the edges. In place mode, the image is resized
	or moved. */
	virtual BOOL GetInitMode()=0;
	/*! \remarks This method is called to set the values as the user is making
	adjustments. If the parameters may be animated, use
	<b>Interface::GetTime()</b> to retrieve the time they are being set for.
	\par Parameters:
	<b>float u</b>\n\n
	The U parameter to set.\n\n
	<b>float v</b>\n\n
	The V parameter to set.\n\n
	<b>float w</b>\n\n
	The W parameter to set.\n\n
	<b>float h</b>\n\n
	The H parameter to set.\n\n
	<b>BOOL md</b>\n\n
	The mode being set. TRUE is place mode; FALSE is crop. */
	virtual void SetValues(float u, float v, float w, float h, BOOL md)=0;
	/*! \remarks This method is called when the cropping adjustment window is
	closed. */
	virtual void OnClose()=0;
	};
	
//-----------------------------------------------------------------------------
//-- Basic Bitmap Class
//
//
   
#define BMM_SINGLEFRAME -2000000L

/*! \sa  Class BitmapManager, Class BitmapInfo, Class BitmapStorage, Class GBuffer, <a href="ms-its:3dsmaxsdk.chm::/bitmaps_root.html">Working with
Bitmaps</a>.\n\n
\par Description:
The <b>Bitmap</b> class is the bitmap itself. All image access is done through
this class. The <b>Bitmap</b> class has methods to retrieve properties of the
bitmap such as image width, height, whether it is dithered, has an alpha
channel, etc. There are methods to retrieve and store pixels from the image.
Additional methods allow a developer to copy bitmaps. This class also has
methods to open outputs and write multi-frame files. All methods of this class
are implemented by the system.
\par Data Members:
protected:\n\n
<b>float gamma;</b>\n\n
The gamma setting for the bitmap.\n\n
<b>Bitmap *map;</b>\n\n
The bitmap using this output handler.\n\n
<b>BitmapStorage *storage;</b>\n\n
The storage used by this INPUT handler\n\n
<b>int openMode;</b>\n\n
The mode the bitmap is open in.\n\n
<b>BitmapIO *prevIO;</b>\n\n
<b>BitmapIO *nextIO;</b>\n\n
Linked list pointers for multiple output of a single bitmap.
\par Method Groups:
See <a href="class_bitmap_groups.html">Method Groups for Class Bitmap</a>.
*/
class Bitmap : public BaseInterfaceServer
{
   friend class BitmapManagerImp;
	  
   private:
   
	  DWORD                    flags;                      // See above

	  BitmapManager            *manager;                   // Manager of this bitmap
	  BitmapIO                 *output;                    // Head of output handler list
	  BitmapFilter             *filter;                    // Filtered access methods
	  BitmapDither             *dither;                    // Dither methods
	  BitmapStorage            *storage;                   // Actual storage
	  UINT                     filterType;                 // Filtered access type
	  UINT                     ditherType;                 // Dither type
	  DWORD					   modifyID;                   // changes when bitmap changes: used in render effects
	  int                      Free();

	  void 					   *vfbData;	

	  BitmapNotify *bmNotify;		// Called when storage is change so display can update

	  friend LRESULT CALLBACK  InputWndProc                ( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

	  // To delete a Bitmap, call Bitmap::DeleteThis()
	  BMMExport               ~Bitmap                      ( );

	  // To get a new Bitmap, call BitmapManager::NewBitmap()
	  BMMExport                Bitmap                      ( BitmapManager *manager = TheManager );
	 
   public:

	  /*! \remarks Returns a pointer to the bitmap manager being used. */
	  inline  BitmapManager   *Manager                     ( ) { return manager; };

	  //-- Don't use these unless you know what you're doing ----------------
	  
	  BMMExport int            Create                      ( BitmapInfo *bi );
	  BMMExport BOOL           FixDeviceName               ( BitmapInfo *bi );
	  inline    int            MapReady                    ( ) { if (storage) return storage->MapReady(); return 0; };
	  BMMExport void           AddOutput                   ( BitmapIO *out  );
	  BMMExport void           RemoveOutput                ( BitmapIO *out, BOOL deleteOutput=TRUE  );
	  BMMExport BitmapIO *     FindOutput                  ( BitmapInfo *bi );
	  BMMExport PAVIFILE       GetPaviFile                 ( BitmapInfo *bi );


	  inline  void 			   *GetVFBData				   ( )	{ return vfbData; }
	  inline  void 			    SetVFBData				   (void *vfb )	{ vfbData = vfb; }

	  
	  //-- Public Interface -------------------------------------------------
	  
	  /*! \remarks	  This method should be called to free the Bitmap. Note that you should not
	  invoke <b>~Bitmap()</b> directly by calling <b>delete</b> on the Bitmap
	  as was done in 3ds Max1.x. An assert will be raised if you call delete on
	  a Bitmap directly. Instead use this method. */
	  BMMExport void           DeleteThis                  ( ); // Call this , NOT delete, to free a Bitmap.		      
	  /*! \remarks Returns the state of the bitmap flags. These flags describe
	  properties of the bitmap such as if the bitmap is flipped horizontally or
	  inverted vertically, is paletted, is dithered, etc. See \ref bitmapFlags. */
	  inline  DWORD            Flags                       ( ) { return flags; };
	  /*! \remarks Sets the specified flag bit(s).
	  \par Parameters:
	  <b>DWORD flag</b>\n\n
	  The flag(s) to set. See \ref bitmapFlags. */
	  inline  void             SetFlag                     ( DWORD flag ) { flags |= flag; };
	  /*! \remarks Toggles the specified flag bit(s) on/off.
	  \par Parameters:
	  <b>DWORD flag</b>\n\n
	  The flag(s) to toggle. See \ref bitmapFlags. */
	  inline  void             ToggleFlag                  ( DWORD flag ) { flags ^= flag; };
	  /*! \remarks Clears the specified flag bit(s) (sets them to zero).
	  \par Parameters:
	  <b>DWORD flag</b>\n\n
	  The flag(s) to clear. See \ref bitmapFlags. */
	  inline  void             ClearFlag                   ( DWORD flag ) { flags &= (~flag); };
	  
	  /*! \remarks Returns the width of the bitmap (the horizontal dimension).
	  \return  If storage has been allocated the width of the bitmap; otherwise
	  0. */
	  inline  int              Width                       ( ) { if (storage) return storage->Width();              return 0; };
	  /*! \remarks Returns the height (vertical dimension) of the bitmap.
	  \return  If storage has been allocated the height of the bitmap;
	  otherwise 0. */
	  inline  int              Height                      ( ) { if (storage) return storage->Height();             return 0; };
	  /*! \remarks Returns the aspect ratio of the bitmap.
	  \return  If storage has been allocated the aspect ratio of the bitmap;
	  otherwise 0.0f. */
	  inline  float            Aspect                      ( ) { if (storage) return storage->Aspect();             return (float)0.0; };
	  /*! \remarks Returns the gamma value for the bitmap.
	  \return  If storage has been allocated the gamma of the bitmap; otherwise
	  0.0f. */
	  inline  float            Gamma                       ( ) { if (storage) return storage->Gamma();              return (float)0.0; };
	  /*! \remarks Returns whether the bitmap uses a palette (is not true
	  color).
	  \return  If storage has been allocated returns nonzero if the bitmap uses
	  a palette (returns the number of palette slots used); otherwise 0. */
	  inline  int              Paletted                    ( ) { if (storage) return storage->Paletted();           return 0; };
	  /*! \remarks Returns whether the bitmap is dithered or not.
	  \return  If storage has been allocated returns nonzero if the bitmap is
	  dithered; otherwise 0. */
	  inline  int              IsDithered                  ( ) { if (storage) return storage->IsDithered();         return 0; };
	  /*! \remarks Returns whether the bitmap uses pre-multiplied alpha.
	  \return  If storage has been allocated returns nonzero if the bitmap uses
	  pre-multiplied alpha; otherwise 0. */
	  inline  int              PreMultipliedAlpha          ( ) { if (storage) return storage->PreMultipliedAlpha(); return 0; };
	  /*! \remarks Returns whether the bitmap has an alpha channel.
	  \return  If storage has been allocated returns nonzero if the bitmap has
	  an alpha channel; otherwise 0. */
	  inline  int              HasAlpha                    ( ) { if (storage) return storage->HasAlpha();           return 0; };
	  /*! \remarks	  Returns nonzero if this is a bitmap that supports high dynamic range
	  data; zero if it doesn't. */
	  inline  int              IsHighDynamicRange          ( ) { if (storage) return storage->IsHighDynamicRange(); return 0; };
	  /*! \remarks This method returns the number of bits per pixel for each
	  color component. For example a 24-bit TARGA has a <b>MaxRGBLevel()</b> of
	  8. */
	  inline  int              MaxRGBLevel                 ( ) { if (storage) return storage->MaxRGBLevel();        return 0; };
	  /*! \remarks Implemented by the System.\n\n
	  Returns the number of bits per pixel in the alpha channel.
	  \return  If storage has not been allocated returns 0. */
	  inline  int              MaxAlphaLevel               ( ) { if (storage) return storage->MaxAlphaLevel();      return 0; };
	  /*! \remarks	  Implemented by the System.\n\n
	  Set whether colors are scaled (on) or clamped (off) when converting from
	  <b>BMM_Color_fl</b> to <b>BMM_Color_64</b>. If storage is not allocated,
	  does nothing. */
	  inline  void             UseScaleColors              ( int on ) { if (storage) storage->UseScaleColors(on); };
	  /*! \remarks	  Implemented by the System.\n\n
	  Returns the last value set by <b>UseScaleColors</b>. If storage is not
	  allocated, returns 0. */
	  inline  int              ScaleColors                 ( ) { if (storage) return storage->ScaleColors();        return 0; };
	  /*! \remarks	  Implemented by the System.\n\n
	  Converts in to out clamping the RGB components to 0 to 65535. The alpha
	  component is not copied.
	  \par Parameters:
	  <b>BMM_Color_64\& out</b>\n\n
	  The result of the conversion.\n\n
	  <b>BMM_Color_fl\& in</b>\n\n
	  The value to convert. */
	  inline  static void      ClampColor                  (BMM_Color_64& out, const BMM_Color_fl& in) { BitmapStorage::ClampColor(out, in); }
	  /*! \remarks	  Implemented by the System.\n\n
	  Converts in to out clamping the RGB components to 0 to 65535.
	  \par Parameters:
	  <b>BMM_Color_64\& out</b>\n\n
	  The result of the conversion.\n\n
	  <b>BMM_Color_fl\& in</b>\n\n
	  The value to convert. */
	  inline  static void      ClampColorA                 (BMM_Color_64& out, const BMM_Color_fl& in) { BitmapStorage::ClampColorA(out, in); }

	  /*! \remarks	  Implemented by the System.\n\n
	  Converts in to out clamping the RGB components to 0 to 65535. The alpha
	  component is not copied.
	  \par Parameters:
	  <b>BMM_Color_64\& out</b>\n\n
	  The result of the conversion.\n\n
	  <b>BMM_Color_fl\& in</b>\n\n
	  The value to convert. */
	  inline  static void      ScaleColor                  (BMM_Color_64& out, const BMM_Color_fl& in) { BitmapStorage::ScaleColor(out, in);}
	  /*! \remarks	  Implemented by the System.\n\n
	  Converts in to out clamping the RGB components to 0 to 65535.
	  \par Parameters:
	  <b>BMM_Color_64\& out</b>\n\n
	  The result of the conversion.\n\n
	  <b>BMM_Color_fl\& in</b>\n\n
	  The value to convert. */
	  inline  static void      ScaleColorA                 (BMM_Color_64& out, const BMM_Color_fl& in) { BitmapStorage::ScaleColorA(out, in);}
	  /*! \remarks	  Implemented by the System.\n\n
	  Converts in to out, using the value of <b>ScaleColors()</b> to determine
	  the clamping or scaling. The alpha component is not copied. If the
	  storage is not allocated, the clamping is performed.
	  \par Parameters:
	  <b>BMM_Color_64\& out</b>\n\n
	  The result of the conversion.\n\n
	  <b>BMM_Color_fl\& in</b>\n\n
	  The value to convert. */
	  inline  void             ClampScaleColor             (BMM_Color_64& out, const BMM_Color_fl& in) { if (storage) storage->ClampScaleColor(out, in); else ClampColor(out, in); }
	  /*! \remarks	  Implemented by the System.\n\n
	  Converts in to out, using the value of <b>ScaleColors()</b> to determine
	  the clamping or scaling. If the storage is not allocated, the clamping is
	  performed.
	  \par Parameters:
	  <b>BMM_Color_64\& out</b>\n\n
	  The result of the conversion.\n\n
	  <b>BMM_Color_fl\& in</b>\n\n
	  The value to convert. */
	  inline  void             ClampScaleColorA            (BMM_Color_64& out, const BMM_Color_fl& in) { if (storage) storage->ClampScaleColorA(out, in); else ClampColorA(out, in); }

			  int              Put16Gray                   ( int x,int y,int pixels,WORD *ptr )
														   { if (storage) return storage->Put16Gray(x,y,pixels,ptr); return 0; };
			  /*! \remarks Stores the specified 16-bit pixel values into the
			  bitmap. The pixel value pointer you pass to this method may be
			  freed or reused as soon as the function returns. Note: This
			  method provides access to pixel data one scanline at a time.
			  \par Parameters:
			  <b>int x</b>\n\n
			  Destination x location.\n\n
			  <b>int y</b>\n\n
			  Destination y location.\n\n
			  <b>int pixels</b>\n\n
			  Number of pixels to store.\n\n
			  <b>float *ptr</b>\n\n
			  Pixel values to store.
			  \return  Returns nonzero if pixels were stored; otherwise 0. If
			  storage has not been allocated 0 is returned. */
			  int              Put16Gray                   ( int x,int y,int pixels,float *ptr )
														   { if (storage) return storage->Put16Gray(x,y,pixels,ptr); return 0; };
	  inline  void            *GetStoragePtr               ( int *type ) 
														   { if (storage) return storage->GetStoragePtr(type);       return NULL; };
	  inline  void            *GetAlphaPtr                 ( int *type ) 
														   { if (storage) return storage->GetAlphaPtr(type);         return NULL; };
	  inline  int              Get16Gray                   ( int x,int y,int pixels,WORD *ptr )
														   { if (storage) return storage->Get16Gray(x,y,pixels,ptr);         return 0; };
	  /*! \remarks Retrieves the specified 16-bit pixel values from the
	  bitmap. Note: This method provides access to pixel data one scanline at a
	  time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>float *ptr</b>\n\n
	  Pointer to storage for the retrieved pixel values.
	  \return  Returns nonzero if pixels were retrieved; otherwise 0. If
	  storage has not been allocated 0 is returned. */
	  inline  int              Get16Gray                   ( int x,int y,int pixels,float *ptr )
														   { if (storage) return storage->Get16Gray(x,y,pixels,ptr);         return 0; };
	  inline  int              GetPixels                   ( int x,int y,int pixels,BMM_Color_64 *ptr )
														   { if (storage) return storage->GetPixels(x,y,pixels,ptr);         return 0; };
	  /*! \remarks Retrieves the specified 64-bit pixel values from the
	  bitmap. Note: This method provides access to pixel data one scanline at a
	  time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  Pointer to storage for the retrieved pixel values. See
	  <a href="struct_b_m_m___color__fl.html">Structure BMM_Color_fl</a>.
	  \return  Returns nonzero if pixels were retrieved; otherwise 0. If
	  storage has not been allocated 0 is returned. */
	  inline  int              GetPixels                   ( int x,int y,int pixels,BMM_Color_fl *ptr )
														   { if (storage) return storage->GetPixels(x,y,pixels,ptr);         return 0; };
	  BMMExport int            PutPixels                   ( int x,int y,int pixels,BMM_Color_64 *ptr );
	  /*! \remarks Stores the specified 64-bit pixel values into the bitmap's
	  own local storage. The pointer you pass to this method may be freed or
	  reused as soon as the function returns. Note: This method provides access
	  to pixel data one scanline at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Destination x location.\n\n
	  <b>int y</b>\n\n
	  Destination y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  Pixel values to store. See
	  <a href="struct_b_m_m___color__fl.html">Structure BMM_Color_fl</a>.
	  \return  Returns nonzero if pixels were stored; otherwise 0. If storage
	  has not been allocated 0 is returned. */
	  BMMExport int            PutPixels                   ( int x,int y,int pixels,BMM_Color_fl *ptr );
	  /*! \remarks Retrieves the specified 64-bit pixel values from the
	  bitmap. These pixels are NOT gamma corrected (i.e. they have linear gamma
	  - 1.0). Note: This method provides access to pixel data one scanline at a
	  time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>BMM_Color_64 *ptr</b>\n\n
	  Pointer to storage for the retrieved pixel values. See
	  Structure BMM_Color_64.
	  \return  Returns nonzero if pixels were retrieved; otherwise 0. If
	  storage has not been allocated 0 is returned. */
	  inline  int              GetLinearPixels             ( int x,int y,int pixels,BMM_Color_64 *ptr )
														   { if (storage) return storage->GetLinearPixels(x,y,pixels,ptr);     return 0; };
	  inline  int              GetLinearPixels             ( int x,int y,int pixels,BMM_Color_fl *ptr )
														   { if (storage) return storage->GetLinearPixels(x,y,pixels,ptr);     return 0; };
	  /*! \remarks Retrieves the specified pixels from the paletted bitmap.
	  The palette for the image may be accessed using <b>GetPalette()</b>.
	  Note: This method provides access to pixel data one scanline at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Source x location.\n\n
	  <b>int y</b>\n\n
	  Source y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to retrieve.\n\n
	  <b>BYTE *ptr</b>\n\n
	  Pointer to storage for the pixel values.
	  \return  Returns nonzero if pixels were retrieved; otherwise 0. If
	  storage has not been allocated 0 is returned. */
	  inline  int              GetIndexPixels              ( int x,int y,int pixels,BYTE *ptr )
														   { if (storage) return storage->GetIndexPixels(x,y,pixels,ptr);      return 0; };
	  /*! \remarks Stores the pixels into the specified location of the
	  paletted bitmap. The pixel value pointer you pass to this method may be
	  freed or reused as soon as the function returns. Note: This method
	  provides access to pixel data one scanline at a time.
	  \par Parameters:
	  <b>int x</b>\n\n
	  Destination x location.\n\n
	  <b>int y</b>\n\n
	  Destination y location.\n\n
	  <b>int pixels</b>\n\n
	  Number of pixels to store.\n\n
	  <b>BYTE *ptr</b>\n\n
	  Pixels to store.
	  \return  Returns nonzero if pixels were stored; otherwise 0. If storage
	  has not been allocated 0 is returned.\n\n
		*/
	  inline  int              PutIndexPixels              ( int x,int y,int pixels,BYTE *ptr )
														   { if (storage) return storage->PutIndexPixels(x,y,pixels,ptr);      return 0; };
	  inline  int              CropImage                   ( int width,int height,BMM_Color_64 fillcolor)
														   { if (storage) return storage->CropImage(width,height,fillcolor);   return 0; };
	  inline  int              CropImage                   ( int width,int height,BMM_Color_fl fillcolor)
														   { if (storage) return storage->CropImage(width,height,fillcolor);   return 0; };
	  inline  int              CropImage                   ( int width,int height,int fillindex)
														   { if (storage) return storage->CropImage(width,height,fillindex);   return 0; };
	  /*! \remarks This method is not currently implemented. */
	  inline  int              ResizeImage                 ( int width,int height,int newpalette)
														   { if (storage) return storage->ResizeImage(width,height,newpalette);return 0; };
	  inline  int              CopyImage                   ( Bitmap *from,int operation,BMM_Color_64 fillcolor, BitmapInfo *bi = NULL)
														   { if (storage) return storage->CopyImage(from,operation,fillcolor,bi); return 0; };
	  /*! \remarks Copies the specified bitmap to this bitmap.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The source bitmap.\n\n
	  <b>int operation</b>\n\n
	  The type of copy to perform:\n\n
	  <b>COPY_IMAGE_CROP</b>\n\n
	  Copy image to current map size using cropping if necessary.\n\n
	  <b>COPY_IMAGE_RESIZE_LO_QUALITY</b>\n\n
	  Resize the source image to the destination map size (draft quality).\n\n
	  <b>COPY_IMAGE_RESIZE_HI_QUALITY</b>\n\n
	  Resize source image to the destination map size (final quality).\n\n
	  <b>COPY_IMAGE_USE_CUSTOM</b>\n\n
	  Resize based on the Image Input Options (BitmapInfo *).\n\n
	  <b>BMM_Color_fl fillcolor</b>\n\n
	  Vacant areas of the bitmap are filled with <b>fillcolor</b> pixels if the
	  operation specified is <b>COPY_IMAGE_CROP</b> and one of the source
	  bitmap dimensions is less than the size of this bitmap. See
	  Structure BMM_Color_fl.\n\n
	  <b>BitmapInfo *bi = NULL</b>\n\n
	  When using custom options (resize to fit, positioning, etc.) this is how
	  the flags are passed down to the Bitmap Manager. This is an optional
	  argument -- for simple copy operations, <b>*bi</b> can default to NULL.
	  If present, the code checks the option flags and acts accordingly.
	  \return  Nonzero if the copy was performed; otherwise 0. */
	  inline  int              CopyImage                   ( Bitmap *from,int operation,BMM_Color_fl fillcolor, BitmapInfo *bi = NULL)
														   { if (storage) return storage->CopyImage(from,operation,fillcolor,bi); return 0; };
	  /*! \remarks Copies the specified bitmap to this bitmap.
	  \par Parameters:
	  <b>Bitmap *from</b>\n\n
	  The source bitmap.\n\n
	  <b>int operation</b>\n\n
	  The type of copy to perform. See \ref bitmapCopyImageOptions.\n\n
	  <b>int fillindex</b>\n\n
	  Vacant areas of the bitmap are filled with <b>fillindex</b> pixels if the
	  operation specified is <b>COPY_IMAGE_CROP</b> and one of the source
	  bitmap dimensions is less than the size of this bitmap.
	  \return  Nonzero if the copy was performed; otherwise 0. */
	  inline  int              CopyImage                   ( Bitmap *from,int operation,int fillindex)
														   { if (storage) return storage->CopyImage(from,operation,fillindex); return 0; };
	  /*! \remarks Retrieves a portion of the palette from the bitmap.
	  \par Parameters:
	  <b>int start</b>\n\n
	  The index into the palette of where to begin retrieving palette
	  entries.\n\n
	  <b>int count</b>\n\n
	  The number of palette entries to retrieve.\n\n
	  <b>BMM_Color_48 *ptr</b>\n\n
	  Storage for the palette entries. See
	  Structure BMM_Color_48.
	  \return  Nonzero if the palette entries were retrieved; otherwise 0. */
	  inline  int              GetPalette                  ( int start,int count,BMM_Color_48 *ptr)
														   { if (storage) return storage->GetPalette(start,count,ptr);         return 0; };
	  /*! \remarks Sets the specified portion of the palette of this bitmap.
	  \par Parameters:
	  <b>int start</b>\n\n
	  The index into the palette of where to begin storing palette entries.\n\n
	  <b>int count</b>\n\n
	  The number of palette entries to store.\n\n
	  <b>BMM_Color_48 *ptr</b>\n\n
	  The palette entries to store. See
	  Structure BMM_Color_48.
	  \return  Nonzero if the palette entries were stored; otherwise 0. */
	  inline  int              SetPalette                  ( int start,int count,BMM_Color_48 *ptr)
														   { if (storage) return storage->SetPalette(start,count,ptr);         return 0; };

	  //-- GRAINSTART

	  // Effects methods (GG 11/03/98) ----------

	  BMMExport void			FilmGrain				( float grain, BOOL mask, PBITMAP_FX_CALLBACK callback = NULL, void *param = NULL );

	  //-- GRAINEND

	  // GBuffer methods ---------------------

	  /*! \remarks Returns a pointer to the specified channel of the bitmap,
	  and determines its type in terms of bits per pixel.
	  \par Parameters:
	  <b>ULONG channelID</b>\n\n
	  The channel to return a pointer to. See \ref gBufImageChannels.\n\n
	  <b>ULONG\& chanType</b>\n\n
	  The type of the returned channel. One of the following values:\n\n
	  <b>BMM_CHAN_TYPE_UNKNOWN</b>\n\n
	  Channel not of a known type.\n\n
	  <b>BMM_CHAN_TYPE_8</b>\n\n
	  1 byte per pixel\n\n
	  <b>BMM_CHAN_TYPE_16</b>\n\n
	  1 word per pixel\n\n
	  <b>BMM_CHAN_TYPE_32</b>\n\n
	  2 words per pixel\n\n
	  <b>BMM_CHAN_TYPE_48</b>\n\n
	  3 words per pixel\n\n
	  <b>BMM_CHAN_TYPE_64</b>\n\n
	  4 words per pixel\n\n
	  <b>BMM_CHAN_TYPE_96</b>\n\n
	  6 words per pixel */
	  inline void             *GetChannel                ( ULONG channelID, ULONG& chanType ) 
									   { if (storage) return storage->GetChannel(channelID, chanType); return NULL; }   
	  
	  /*! \remarks	  Returns a pointer to the GBuffer for the bitmap (or NULL if none). */
	  inline GBuffer *GetGBuffer()  { return storage? storage->GetGBuffer(): NULL; } 

	  /*! \remarks This method creates the specified channels. After creation,
	  these may be accessed using <b>void *GetChannel()</b>.
	  \par Parameters:
	  <b>ULONG channelIDs</b>\n\n
	  Specifies the channels to create. See \ref gBufImageChannels.
	  \return  The channels that are present. */
	  inline ULONG            CreateChannels             ( ULONG channelIDs ) 
											 { if (storage) return storage->CreateChannels(channelIDs); return 0; }   
	  /*! \remarks Delete the specified channels.
	  \par Parameters:
	  <b>ULONG channelIDs</b>\n\n
	  Specifies the channels to delete. See \ref gBufImageChannels.
	  */
	  inline void             DeleteChannels             ( ULONG channelIDs ) 
											 { if (storage) storage->DeleteChannels(channelIDs); }     
	  /*! \remarks Returns the channels that are present. See \ref gBufImageChannels.
	  */
	  inline ULONG            ChannelsPresent            ( )   
											 { if (storage) return storage->ChannelsPresent();  return 0; }   
	  /*! \remarks This is used internally. It returns a pointer to the
	  RenderInfo associated with the storage if available; otherwise NULL. See
	  Class RenderInfo. */
	  inline RenderInfo*           GetRenderInfo()        { if (storage) return storage->GetRenderInfo(); return NULL; }

	  /*! \remarks This is used internally. It returns a pointer to the
	  RenderInfo instance allocated by the storage. If this could not be
	  allocated NULL is returned. See Class RenderInfo. */
	  inline RenderInfo*           AllocRenderInfo()     { if (storage) return storage->AllocRenderInfo(); return NULL; }


	  //---------------------------------------------------------------------
	  //
	  //   This call will check with the plug-in (file or device) defined in 
	  //   the given BitmapInfo and prepare (create) the proper channels. If 
	  //   a given channel already exists, no new channel will be created. 
	  //
	  //   After creating a bitmap, use this function to define the optional 
	  //   channels that may be required by the given handler. 
	  //

	  /*! \remarks This method is used internally. This method will check with
	  the plug-in (file or device) defined in the given BitmapInfo and prepare
	  (create) the proper channels. If a given channel already exists, no new
	  channel will be created. After creating a bitmap, use this function to
	  define the optional channels that may be required by the given handler.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  Points to an instance of BitmapInfo that defines the properties of the
	  image.
	  \return  TRUE if the channels were created; otherwise FALSE. */
	  BMMExport BOOL          PrepareGChannels           ( BitmapInfo *bi ); 
	  BMMExport BOOL          PrepareGChannels           ( DWORD channels ); 


	  BMMExport int           GetFiltered                ( float u,float v,float du,float dv,BMM_Color_64 *ptr );
	  /*! \remarks This method uses summed area table or pyramidal filtering
	  to compute an averaged color over the specified area. You must have a
	  filter plugged in for this to work. See <b>SetFilter()</b> below.
	  \par Parameters:
	  <b>float u, float v</b>\n\n
	  The location in the bitmap to filter. These values go from 0.0 to 1.0
	  across the size of the bitmap.\n\n
	  <b>float du, float dv</b>\n\n
	  The size of the rectangle to sample. These values go from 0.0 to 1.0
	  across the size of the bitmap.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  The result is returned here -- the average over the specified area. See
	  Structure BMM_Color_fl. */
	  /*! \remarks This method uses summed area table or pyramidal filtering
	  to compute an averaged color over the specified area. You must have a
	  filter plugged in for this to work. See <b>SetFilter()</b> below.
	  \par Parameters:
	  <b>float u, float v</b>\n\n
	  The location in the bitmap to filter. These values go from 0.0 to 1.0
	  across the size of the bitmap.\n\n
	  <b>float du, float dv</b>\n\n
	  The size of the rectangle to sample. These values go from 0.0 to 1.0
	  across the size of the bitmap.\n\n
	  <b>BMM_Color_fl *ptr</b>\n\n
	  The result is returned here -- the average over the specified area. */
	  BMMExport int           GetFiltered                ( float u,float v,float du,float dv,BMM_Color_fl *ptr );
	  /*! \remarks Sets the type of dithering used on the bitmap.
	  \par Parameters:
	  <b>UINT ditherType</b>\n\n
	  The type of dither to perform.\n\n
	  <b>BMM_DITHER_NONE</b>\n\n
	  Specifies no dithering is to be performed.\n\n
	  <b>BMM_DITHER_FLOYD</b>\n\n
	  Specifies the Floyd-Steinberg dithering algorithm.
	  \return  Nonzero if the bitmap dithering was set; otherwise 0. */
	  BMMExport int           SetDither                  ( UINT ditherType );
	  /*! \remarks Establishes a filtering algorithm to be used by the bitmap.
	  \par Parameters:
	  <b>UINT filterType</b>\n\n
	  See \ref bitmapFilterTypes.
	  \return  Nonzero if the bitmap filtering was set; otherwise 0. */
	  BMMExport int           SetFilter                  ( UINT filterType );
	  /*! \remarks Determines if the bitmap has a filter.
	  \return  Returns nonzero if the bitmap has a filter; otherwise 0. */
	  inline    int           HasFilter                  ( ) { return (filter) ? 1:0; };
	  /*! \remarks This method is used internally. It returns a pointer to the
	  bitmap filter used by the bitmap. */
	  inline    BitmapFilter  *Filter                    ( ) { return filter; }; 
	  /*! \remarks Establishes a bitmap storage to manage this bitmap.
	  \par Parameters:
	  <b>BitmapStorage *storage</b>\n\n
	  The storage to manage the bitmap.
	  \return  Nonzero if the storage was assigned; otherwise 0. */
	  BMMExport int           SetStorage                 ( BitmapStorage *storage);
	  /*! \remarks Returns the storage that is managing this bitmap.
	  \return  A pointer to the storage. */
	  inline    BitmapStorage *Storage                   ( ) { return storage; };
	  /*! \remarks Sets the storage pointer to NULL. */
	  inline    void          NullStorage                ( ) { storage = NULL; };
	  
	  //-- Windows DIB Conversion -------------------------------------------
	  //
	  //   Where depth is either 24 (BGR) or 32 (BGR0)
	  //

	  /*! \remarks Creates a new Windows Device Independent Bitmap (DIB) and
	  returns a pointer to it. The DIB bitmap is created from this Bitmap. The
	  DIB is allocated using <b>LocalAlloc()</b>. The pseudo-code below show
	  how one may be created and freed. Note that the DIB is never used or
	  accessed inside 3ds Max (the call to <b>ToDib()</b> is the first and last
	  time that 3ds Max sees this pointer):\n\n
    \code 
	  PBITMAPINFO pDib;
	  pDib = bitmap-\>ToDib();
	  ...
    \endcode      
	  When you are done using the DIB call:\n\n
	  <b>LocalFree(pDib);</b>
	  \par Parameters:
	  <b>int depth = 24</b>\n\n
	  The bitmap depth; either 24 (BGR) or 32 (BGRO). If not specified the
	  default is 24.\n\n
	  <b>UWORD *gam=NULL</b>\n\n
	  Specifies a pointer to an optional gamma table that is used to apply
	  gamma correction to the color components as part of the conversion to a
	  DIB. The table has RCOLN entries.\n\n
	  <b>BOOL dither=FALSE</b>\n\n
	  Specifies if a random dither is applied when reducing the color
	  components from 16 bits per channel to 8 bits per channel (to reduce
	  banding effects).
	  <b>BOOL displayGamma=FALSE</b>\n\n
	  Specifies whether the output should be color corrected for screen display.
	  If so, the gamma table parameter is ignored, and the DIB is color corrected
	  using display gamma or LUT, as necessary.
	  */
	  BMMExport PBITMAPINFO    ToDib                       ( int depth = 24, UWORD *gam=NULL, BOOL dither=FALSE, BOOL displayGamma=FALSE );

	  //-- Do not use this directly. Instead, use BitmapManager::Create(PBITMAPINFO)

	  /*! \remarks Converts the DIB to this bitmap's storage type. This
	  bitmap's storage must already be allocated or the call will fail. The
	  source must be 16, 24 or 32 bit. You cannot use an 8 bit DIB.
	  \par Parameters:
	  <b>PBITMAPINFO pbmi</b>\n\n
	  The source bitmap.
	  \return  TRUE if the conversion was performed; otherwise FALSE. */
	  BMMExport BOOL           FromDib                     ( PBITMAPINFO pbmi );
	  
	  //-- Image output operations ------------------------------------------
	  //
	  //  To write a single image to a file/device:
	  //
	  //  *> Create BitmapInfo class: BitmapInfo bi;
	  //
	  //  *> Define output file/device:
	  //
	  //     Directly:       bi.SetName("file.tga");
	  //   or
	  //     User Interface: BitmapManager::SelectFileOutput( ... &bi ...)
	  //
	  //  *> Define bitmap: 
	  //
	  //                     bi.SetWidth(x)
	  //                     bi.SetHeight(y)
	  //                     etc...
	  //
	  //  *> Create bitmap:  Bitmap *map = BitmapManager::Create(&bi);
	  //                     
	  //
	  //  *> Do something:   map->Fill({0,0,0});
	  //
	  //  *> OpenOutput:     map->OpenOutput(&bi);
	  //
	  //  *> Write:          map->Write(&bi)
	  //
	  //  *> Close:          map->Close(&bi)
	  //
	  //  To write a multiframe file, just keep doing something different to
	  //  the bimap and keep writting. 
	  //
	  //  To write a sequence of images to a file/device:
	  //
	  //  *> Create BitmapInfo class: BitmapInfo bi;
	  //
	  //  *> Define output file/device:
	  //
	  //     Directly:       bi.SetName("file.tga");
	  //   or
	  //     User Interface: BitmapManager::SelectFileOutput( ... &bi ...)
	  //
	  //  *> Define bitmap: 
	  //
	  //                     bi.SetWidth(x)
	  //                     bi.SetHeight(y)
	  //
	  //                     bi.SetFirstFrame(0)
	  //                     bi.SetLastFrame(29)
	  //
	  //                     etc...
	  //
	  //  *> Create bitmap:  Bitmap *map = BitmapManager::Create(&bi);
	  //                     
	  //
	  //  *> OpenOutput:     map->OpenOutput(&bi);
	  //
	  //     for (x = 0 to 29) {
	  //        *> Do something to image...
	  //        *> Write:    map->Write(&bi,x);
	  //     }
	  //
	  //  *> Close:          map->Close(&bi)
	  //
	  //
	  //  Note: You can add any number of  outputs to a bitmap. Just keep
	  //  calling map->OpenInput() with different outputs (Targa file AND
	  //  Frame Buffer for instance). To write or close a specific output,
	  //  use Write()  and Close().  To write  and close them all at once,
	  //  use WriteAll() and CloseAll().
	  //
	  //  It is ok to use WriteAll() and CloseAll() if you have just one
	  //  output defined.
	  //
	  
	  /*! \remarks This method will open the image for output. This allows the
	  image to be written to. Note that you can pass a <b>BitmapInfo</b> to
	  this method where you simply set the name 'by hand'
	  (<b>bi-\>SetName()</b>). This will work and the correct driver wil be
	  selected based on the filename extension. However you won't be able to
	  set any driver specific settings (such as compression settings for
	  JPEGs). The alternative way is to use the <b>BitmapManger</b> methods.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  Contains the name of the image or device to open for output.
	  \return  <b>BMMRES_SUCCESS</b>\n\n
	  Indicates success.\n\n
	  <b>BMMRES_ERRORTAKENCARE</b>\n\n
	  Indicates that 3ds Max could not find a device to handle the image.\n\n
	  <b>BMMRES_INTERNALERROR</b>\n\n
	  Indicates the IO module handling the image could not be opened for
	  writing. */
	  BMMExport BMMRES         OpenOutput                  ( BitmapInfo *bi );                                        // Open output
		/*! \remarks Write the image from the <b>BitmapStorage</b> to disk. Note that
		you must pass the same <b>BitmapInfo</b> used when the file was first "Opened
		for Output". The main reason is that any device specific settings are kept in
		the <b>BitmapInfo</b> object. Also, the custom options such as gamma value,
		optional channels, etc (if any) are kept in this <b>BitmapInfo</b> instance.
		These are assigned when the bitmap is first opened (for either read or write).
		\par Parameters:
		<b>BitmapInfo *bi</b>\n\n
		Contains the name of the file or device to write to.\n\n
		<b>DWORD frame = BMM_SINGLEFRAME</b>\n\n
		Specifies the frame number to write. If this is a single image, allow
		<b>frame</b> to default to single frame. This argument determines if the file
		will have the frame number appended to it. If you want the file to have a
		normal name (no frame number attached to it), you must set the frame argument
		to BMM_SINGLEFRAME. Any other value is considered to be a frame number and it
		will be appended to the given filename.
		\return  <b>BMMRES_SUCCESS</b>\n\n
		Indicates success.\n\n
		<b>BMMRES_ERRORTAKENCARE</b>\n\n
		Indicates that 3ds Max could not find a device to handle the image.\n\n
		<b>BMMRES_INTERNALERROR</b>\n\n
		Indicates the IO module handling the image could not be opened for writing. */
	  BMMExport BMMRES         Write                       ( BitmapInfo *bi, int frame = BMM_SINGLEFRAME );         // Write frame to file
		/*! \remarks Write the image to all the open outputs.
		\par Parameters:
		<b>DWORD frame = BMM_SINGLEFRAME</b>\n\n
		Specifies the frame number to write. If this is a single image, allow
		<b>frame</b> to default to single frame.
		\return  <b>BMMRES_SUCCESS</b>\n\n
		Indicates success.\n\n
		<b>BMMRES_ERRORTAKENCARE</b>\n\n
		Indicates that 3ds Max could not find a device to handle the image.\n\n
		<b>BMMRES_INTERNALERROR</b>\n\n
		Indicates the IO module handling the image could not be opened for writing. */
	  BMMExport BMMRES         WriteAll                    ( int frame = BMM_SINGLEFRAME );                         // Write all open outputs
	  /*! \remarks Close the bitmap. This means the bitmap is no longer open
	  for writing.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  Identifies the bitmap to close.\n\n
	  <b>int flag = BMM_CLOSE_COMPLETE</b>\n\n
	  See \ref bitmapCloseTypes.
	  \return  Nonzero if the image output was closed without error; otherwise
	  0. */
	  BMMExport int            Close                       ( BitmapInfo *bi, int flag = BMM_CLOSE_COMPLETE );         // Close an open output
	  /*! \remarks Closes all the open outputs.
	  \par Parameters:
	  <b>int flag = BMM_CLOSE_COMPLETE</b>\n\n
	  See \ref bitmapCloseTypes.
	  \return  Nonzero if the image outputs were closed without error;
	  otherwise 0. */
	  BMMExport int            CloseAll                    ( int flag = BMM_CLOSE_COMPLETE);                          // Close all open outputs

	  //-- Window gravity

	  #define   BMM_UL  1      //-- Upper Left
	  #define   BMM_LL  2      //-- Lower Left
	  #define   BMM_UR  3      //-- Upper Right
	  #define   BMM_LR  4      //-- Upper Left
	  #define   BMM_CN  5      //-- Center

	  #define   BMM_RND 10     //-- Renderer (Save/Restore)
	  #define   BMM_VPP 11     //-- Video Post Primary (Save/Restore)
	  #define   BMM_VPS 12     //-- Video Post Secondary (Save/Restore)

	  /*! \remarks This method creates a window for the display of this bitmap
	  and displays it.
	  \par Parameters:
	  <b>MCHAR *title = NULL</b>\n\n
	  The title to display in the title bar of the window.\n\n
	  <b>int position = BMM_CN</b>\n\n
	  Specifies how the bitmap should be positioned. One of the following
	  values:\n\n
	  <b>BMM_UL</b> - Upper Left\n\n
	  <b>BMM_LL</b> - Lower Left\n\n
	  <b>BMM_UR</b> - Upper Right\n\n
	  <b>BMM_LR</b> - Lower Right\n\n
	  <b>BMM_CN</b> - Center\n\n
	  <b>BMM_RND</b> - Used internally. Renderer location.\n\n
	  <b>BMM_VPP</b> - Used internally. Video Post Primary location.\n\n
	  <b>BMM_VPS</b> - Used internally. Video Post Secondary location.\n\n
	  <b>BOOL autonomous = FALSE</b>\n\n
	  This is reserved for internal use, always let it default to FALSE.\n\n
	  <b>BOOL savebutton = TRUE</b>\n\n
	  This is reserved for internal use, always let it default to TRUE.\n\n
	  <b>CropCallback *crop=NULL</b>\n\n
	  This parameter is available in release 2.0 and later only.\n\n
	  When non-NULL this will cause the VFB to display, instead of its normal
	  toolbar, a set of sliders for adjusting cropping and also will allow
	  interactive adjustment of the cropping rectangle in the image window. See
	  Class CropCallback.\n\n
	  <b>Bitmap *cloneMyVFB = NULL</b>\n\n
	  This parameter is available in release 4.0 and later only.\n\n
	  A pointer to a bitmap to clone the VFB to.
	  \return  Nonzero if the bitmap was displayed; otherwise 0. */
	  BMMExport int            Display                     ( MCHAR *title = NULL, 	int position = BMM_CN, 
			BOOL autonomous = FALSE, BOOL savebutton = TRUE, CropCallback *crop=NULL, Bitmap *cloneMyVFB = NULL );
	  /*! \remarks Close the display window associated with this bitmap (if
	  any).
	  \return  Always returns nonzero. */
	  BMMExport int            UnDisplay                   ( );
	  /*! \remarks Get the window handle for the displayed bitmap.
	  \return  Returns the window handle, or NULL if it's not displayed in a
	  window. */
	  BMMExport HWND           GetWindow                   ( );
	  /*! \remarks Refreshes the interior of the display window with the
	  bitmap contents. In release 3.0 and later this method respects the
	  <b>Bitmap::ShowProgressLine()</b> setting. See that method for more
	  details.
	  \par Parameters:
	  <b>RECT *rect = NULL</b>\n\n
	  The region of the display window to refresh (specified in image
	  coordinates). If the pointer is NULL the entire window is refreshed. */
	  BMMExport void           RefreshWindow               ( RECT *rect = NULL );
	  /*! \remarks Sets the title displayed in the display window's title bar.
	  \par Parameters:
	  <b>MCHAR *title</b>\n\n
	  The title to display. */
	  BMMExport void           SetWindowTitle              ( MCHAR *title );
	  /*! \remarks	  This method is used when the VFB is being displayed and you want to
	  change the cropping rectangle from your plug-in. An example of this is
	  available in <b>/MAXSDK/SAMPLES/MATERIALS/BMTEX.CPP</b>.
	  \par Parameters:
	  <b>float u</b>\n\n
	  The U value to set.\n\n
	  <b>float v</b>\n\n
	  The U value to set.\n\n
	  <b>float w</b>\n\n
	  The U value to set.\n\n
	  <b>float h</b>\n\n
	  The U value to set.\n\n
	  <b>BOOL placeImage</b>\n\n
	  TRUE for place mode; FALSE for crop. */
	  BMMExport void           SetCroppingValues		   ( float u, float v, float w, float h, BOOL placeImage);

	  //-- Get a Different Frame  -------------------------------------------
	  //
	  //   For  multifrane bitmaps (FLI's, AVI's, DDR devices, etc.),  if you
	  //   simply want to load  another frame replacing a previously "Load"ed
	  //   image.
	  //
	  //   If used with single frame drivers or if the driver doesn't support
	  //   this function,  it returns BMMRES_SINGLEFRAME. If the return value
	  //   is BMMRES_SUCCESS,  a new frame  has  been  loaded  into the given 
	  //   bitmap.
	  //
	  //   To define desired frame, use bi->SetCurrentFrame( frame );
	  //
	  
	  /*! \remarks This method is used with multi-frame bitmaps (FLI's, AVI's,
	  DDR devices, etc.). It is used to load a frame to replace a previously
	  saved image. To define the desired frame, use:\n\n
	  <b>bi-\>SetCurrentFrame(frame);</b>
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  A pointer to the BitmapInfo. The frame number information is passed here.
	  \return  If used with single frame drivers or if the driver doesn't
	  support this function, it returns <b>BMMRES_SINGLEFRAME</b>. If the
	  return value is <b>BMMRES_SUCCESS</b>, a new frame has been loaded into
	  the given bitmap. */
	  BMMExport BMMRES         GoTo                        ( BitmapInfo *bi );

	  //-- Handy built-in functions

	  /*! \remarks Sets every pixel of the bitmap to the specified color and
	  alpha value.
	  \par Parameters:
	  <b>int r</b>\n\n
	  Specifies the red value to fill with.\n\n
	  <b>int g</b>\n\n
	  Specifies the green value to fill with.\n\n
	  <b>int b</b>\n\n
	  Specifies the blue value to fill with.\n\n
	  <b>int alpha</b>\n\n
	  Specifies the alpha value to fill with.
	  \return  Nonzero if the operation succeeded; otherwise FALSE. */
	  BMMExport int            Fill                        ( int r,int g,int b,int alpha);
	  /*! \remarks Used by Maxscript.
	  \par Parameters:
	  <b>fillColor</b>\n\n
	  */
	  BMMExport int            Fill                        (const BMM_Color_fl& fillColor);
	  /*! \remarks Used by Maxscript.
	  \par Parameters:
	  <b>fillColor<br></b> */
	  BMMExport int            Fill                        (const BMM_Color_64& fillColor);

	  // Set a callback so can get notified if storage changed
	  /*! \remarks	  This method is used to set a callback to allow the developer to get
	  notified if the storage for the Bitmap has changed.
	  \par Parameters:
	  <b>BitmapNotify *bmnot=NULL</b>\n\n
	  The pointer to the callback object implemented by the developer used to
	  provide notification when the Bitmap's storage changes. See
	  Class BitmapNotify. */
	  BMMExport void 		  SetNotify( BitmapNotify *bmnot=NULL);
	  /*! \remarks	  Returns a pointer to the callback used to notify a developer when the
	  Bitmap's storage changes.
	  \par Default Implementation:
	  <b>{ return bmNotify; };</b> */
	  BitmapNotify *		  GetNotify() { return bmNotify; }
		
	  /*! \remarks	  Returns TRUE if the virtual frame buffer (VFB) is autoomous; otherwise
	  FALSE. For instance, Video Post has an associated VFB. When Video Post is
	  closed so is its VFB since it belongs to it. In that case the VFB is
	  <b>not</b> autonomous. If the user does a View File command, that VFB is
	  autonomous. */
	  BMMExport BOOL IsAutonomousVFB();

	  // Generic expansion function
	  /*! \remarks	  This is a general purpose function that allows the API to be extended in the
	  future. The 3ds Max development team can assign new <b>cmd</b> numbers and
	  continue to add functionality to this class without having to 'break' the API.
	  \par Parameters:
	  <b>int cmd</b>\n\n
	  The index of the command to execute.\n\n
	  <b>ULONG arg1=0</b>\n\n
	  Optional argument 1. See the documentation where the <b>cmd</b> option is
	  discussed for more details on these parameters.\n\n
	  <b>ULONG arg2=0</b>\n\n
	  Optional argument 2.\n\n
	  <b>ULONG arg3=0</b>\n\n
	  Optional argument 3.
	  \return  An integer return value. See the documentation where the <b>cmd</b>
	  option is discussed for more details on the meaning of this value. */
	  BMMExport INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0); 

	  DWORD GetModifyID() { return modifyID; }
	  void SetModifyID(DWORD m) { modifyID = m; }
	  BMMExport void IncrModifyID();

	  // Print the bitmap (if supported by the host app)
	  /*! \remarks	  This method is only supported in 3D Studio VIZ. */
	  BMMExport void Print(bool silent = false);

	  /*! \remarks	  This method is used for showing a moving scanline in a virtual frame
	  buffer displaying this bitmap. Here's how it works. If you call
	  <b>ShowProgressLine(y)</b>, it clears any previously set white line, and
	  sets an internal counter: you have to call <b>Bitmap::RefreshWindow()</b>
	  to get the new white line to show up. From then that line will be
	  displayed as white. To Clear it call <b>ShowProgressLine(-1)</b>.
	  \par Parameters:
	  <b>int y</b>\n\n
	  The scanline to display as white (the count begins at zero). Use a value
	  of -1 to hide the line. */
	  BMMExport void ShowProgressLine(int y); // y<0 to hide
};

//-- Various Bitmap In-Memory Lists -------------------------------------------

struct BMMStorageList: public MaxHeapOperators {
	BitmapStorage *ptr;
	BMMStorageList *next;
	};

struct BMMFilterList: public MaxHeapOperators {
	BitmapFilter *ptr;
	BMMFilterList *next;
	} ;

struct BMMBitmapList: public MaxHeapOperators {
	Bitmap *ptr;
	BMMBitmapList *next;
	};

struct BMMGammaSettings: public MaxHeapOperators {
	BitmapManager *mgr;
	BitmapInfo    *bi;
	BOOL           out;
	};     

struct BMMVfbPalette: public MaxHeapOperators {
	BYTE  r,g,b;
	};     

class BitmapFileInputDialog: public MaxHeapOperators {
	public:
	virtual BOOL BrowseBitmapFilesInput(BitmapInfo* info, HWND hWnd, MCHAR* title, BOOL view) = 0;
	};

class BitmapFileOutputDialog: public MaxHeapOperators {
	public:
	virtual BOOL BrowseBitmapFilesOutput(BitmapInfo* info, HWND hWnd, MCHAR* title) = 0;
	};

//-----------------------------------------------------------------------------
//-- Main Bitmap Manager Class
//

/*! \sa  Class BitmapInfo, Class Bitmap, Class BitmapIO, Class BitmapStorage, <a href="ms-its:3dsmaxsdk.chm::/bitmaps_root.html">Working with Bitmaps</a>, \ref bitmapErrorCodes.\n\n
\par Description:
This class is used to manage the use of bitmaps within 3ds Max. There is a
global instance of this class provided by 3ds Max that developers may use to
call these methods. It is called <b>TheManager</b>. This class provides methods
for things such as creating and loading bitmaps, and access to the bitmap Map
Path directories. There are also methods for displaying some general dialogs
that let users select input and output files and devices, as well as dialogs
for setting options for the bitmap such as its custom width, height and
positioning.\n\n
Note: In the 3ds Max release 3.0 SDK these methods were made virtual.
\par Data Members:
<b>BMMVfbPalette *pal;</b>\n\n
This is used internally as the virtual framebuffer palette.
\par Defines:
<b>#define BMM_FLUSH_RELATIVE_FILE_RESOLUTION_CACHE 5</b>\n\n
Used to flush the internal cache used to avoid having to re-resolve relative
paths on each LoadInto call\n\n
\par Method Groups:
See <a href="class_bitmap_manager_groups.html">Method Groups for Class BitmapManager</a>.
*/
#pragma warning(push)
#pragma warning(disable:4100)
class BitmapManager : public InterfaceServer{
   
   public:
   
	  BMMVfbPalette            *pal;

	  //-- Construction/Destruction
	  
	  BitmapManager               ( BMMInterface *i) { pal = NULL; }
	  BitmapManager               ( BMMInterface *i,const MCHAR *name) { pal = NULL; }
	  BMMExport virtual           ~BitmapManager               ( );
	  friend void                 DoConstruct      ( BitmapManager *m, BMMInterface *i, const MCHAR *name);

	  //-- These are for internal use only
	  
	  virtual int            DeleteAllMaps               ( )=0;
	  virtual int            AddStorage                  ( BitmapStorage *s)=0;
	  virtual int            AddFilter                   ( BitmapFilter *a)=0;
	  virtual int            AddBitmap                   ( Bitmap *b)=0;
	  virtual int            DelStorage                  ( BitmapStorage *s)=0;
	  virtual int            DelFilter                   ( BitmapFilter *a)=0;
	  virtual int            DelBitmap                   ( Bitmap *b)=0;
	  virtual BitmapFilter  *FindFilter                  ( BitmapStorage *s,UINT type)=0;
	  virtual BitmapStorage *FindStorage                 ( BitmapInfo *bi, int openMode)=0;
	  virtual int            FnametoBitMapInfo           ( BitmapInfo *bi )=0;       
	  virtual void           FixFileExt                  ( OPENFILENAME &ofn, const MCHAR *extension)=0;
	  virtual void           MakeVfbPalette              ( )=0;

	  BMM_IOList               ioList;
	  virtual void           ListIO                      ( )=0;
	  
	  //---------------------------------------------------------------------
	  //-- Public Interface -------------------------------------------------
	  
	  //-- Host Interface

	  /*! \remarks Returns the application instance handle of 3ds Max itself.
	  */
	  virtual HINSTANCE      AppInst                     ( )=0;
	  /*! \remarks Returns the window handle of 3ds Max's main window. */
	  virtual HWND           AppWnd                      ( )=0;
		virtual DllDir        *AppDllDir                   ( )=0;
		/*! \remarks Implemented by the System.\n\n
		Retrieves the specified standard 3ds Max directory name (fonts, scenes, images,
		etc.).
		\par Parameters:
		<b>int i</b>\n\n
		Specifies the directory name to retrieve. See \ref MAXDirIDs
		\return  The name of the specified directory. */
	  virtual const MCHAR   *GetDir                      (int i)=0;
		/*! \remarks For internal use only - This is used to add a MAP PATH to the Map
		path list. */
	  virtual BOOL           AddMapDir                   (const MCHAR *dir,int update)=0;
	  /*! \remarks Returns the number of map paths (used in conjunction with
	  the method below).
	  \return  The number of map paths. */
	  virtual int            GetMapDirCount              ( )=0;
		/*! \remarks Map paths are accessed using a virtual array mechanism. This
		method returns the 'i-th' map path.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies the map path to retrieve.
		\return  The name of the 'i-th' map path. */
		virtual const MCHAR   *GetMapDir                   (int i)=0;
	  /*! \remarks Implemented by the System.\n\n
	  Returns an interface pointer for calling methods provided by 3ds Max. See
	  Class Interface. */
	  virtual Interface     *Max                         ()=0;
	  /*! \remarks	  This method is called to allocate and return a pointer to a new instance
	  of the Bitmap class. The default constructor is used. */
	  virtual Bitmap        *NewBitmap                   ()=0; // returns a new Bitmap 

	  // Set a replacement for the bitmap file dialog. NULL will set the default.
	  virtual void			SetFileInputDialog(BitmapFileInputDialog* dlg = NULL)=0;
	  virtual void			SetFileOutputDialog(BitmapFileOutputDialog* dlg = NULL)=0;

	  //-- These won't stay here. Error handling will be dealt in a couple of 
	  //   different ways. There will be a "Silent" flag that will be set by
	  //   the client and tested here in order to know if an error dialog should
	  //   go up. Normally, if the user is sitting in front of the computer
	  //   this flag will be FALSE. When rendering in the background, or network
	  //   rendering, etc., this flag will be TRUE. There should be some kind of
	  //   "preferences" configuration for this behavior.
	  //
	  //   There also will be a method for logging errors. This method will check
	  //   for a "loggin on/off" flag and either add the entry or ignore it. The
	  //   bitmap manager and its devices will log everything that goes wrong.
	  //   When silent mode is on and logging is also on, this is the method to
	  //   check what went bad. Having each device logging its own error will
	  //   enable a more accurate description of the problem (as opposed to "I/O
	  //   error").
	  //
	  //   Gus
	  //
	  
	  virtual int            Status                      ( int *sCount, int *aCount, int *bCount)=0;
	  virtual int            Error                       ( const MCHAR *string)=0;

	  //-- Error handling ---------------------------------------------------
	  //
	  //

	  //-- Max Interface (means no logging)
	  
	  #define LG_NOLOG         0
	  
	  //-- User Interface
	  
	  #define LG_FATAL         ((DWORD)(1 << 0))
	  #define LG_INFO          ((DWORD)(1 << 1))
	  #define LG_DEBUG         ((DWORD)(1 << 2))
	  #define LG_WARN          ((DWORD)(1 << 3))

	  //-- User Interface 
	  
	  /*! \remarks Determines if silent mode is on. Silent mode specifies if
	  developers should display error messages. If this method returns FALSE,
	  error messages should be displayed. If TRUE, error message dialogs should
	  not be shown.
	  \return  Returns TRUE if silent mode is on; FALSE otherwise. */
	  virtual BOOL           SilentMode                  ( )=0;
	  /*! \remarks This is reserved for future use. */
	  virtual void           SysLog                      ( int type, char *format, ... )=0;

	  //-- Max Interface (used internally)
	  
	  /*! \remarks This method is used internally. */
	  virtual BOOL           SetSilentMode               ( BOOL s )=0;
	  /*! \remarks This method is used internally. */
	  virtual void           SetLogLevel                 ( DWORD level )=0;
	  /*! \remarks This method is used internally. */
	  virtual DWORD          GetLogLevel                 ( )=0;

	  //-- Creating a new bitmap from scracth -------------------------------
	  //
	  //   Make  sure the given  BitmapInfo class has the proper data for the
	  //   creation of the bitmap.  If you used the BitmapManager function to
	  //   define the bitmap (SelectBitmapOutput()), both filename and device
	  //   driver have been defined for you. 
	  //
	  //   Make sure to set the type of bimap using bi.SetType(). This will 
	  //   define the storage type as in (so far):
	  //
	  //   BMM_LINE_ART 
	  //   BMM_PALETTED 
	  //   BMM_GRAY_8
	  //   BMM_GRAY_16
	  //   BMM_TRUE_16  
	  //   BMM_TRUE_32  
	  //   BMM_TRUE_64  
	  //
	  //   Do NOT use BMM_TRUE_24 nor BMM_TRUE_48. These are read only types.
	  //
	  //   Example code is in src/app/vpexecut.cpp
	  //
	  //
	  //   Once a bitmap has been created, use its own methods for adding
	  //   outputs and writing it (i.e. map->OpenOutput(), map->Write() and
	  //   map->Close()).
	  //
	  //   Gus
	  //
	  
	  /*! \remarks This method creates a new bitmap using the properties of
	  the BitmapInfo passed. For more details on creating bitmaps, see the
	  section <a href="ms-its:3dsmaxsdk.chm::/bitmaps_root.html">Working with
	  Bitmaps</a>. Make sure you delete the Bitmap created when you are done
	  using it.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  A pointer to an instance of the class BitmapInfo describing the bitmap to
	  create.
	  \return  A pointer to a newly created instance of class Bitmap. */
	  BMMExport virtual Bitmap        *Create                      ( BitmapInfo *bi   )=0;

	  //-- Creating a new bitmap from an existing Windows DIB ---------------
	  //
	  //   To Convert a Bitmap to a Windows DIB check Bitmap::ToDib()
	  //

	  /*! \remarks This method is used for creating a new bitmap from an
	  existing Windows Device Independent Bitmap. Make sure you delete the
	  Bitmap created when you are done using it.
	  \par Parameters:
	  <b>PBITMAPINFO pbmi</b>\n\n
	  An existing Windows DIB. For more details on creating bitmaps, see the
	  section <a href="ms-its:3dsmaxsdk.chm::/bitmaps_root.html">Working with
	  Bitmaps</a>.
	  \return  Pointer to a new instance of class Bitmap created from the
	  DIB.\n\n
	  \sa  To create a Windows DIB from a Bitmap see <b>Bitmap::ToDib()</b>
	  */
	  BMMExport virtual Bitmap        *Create                      ( PBITMAPINFO pbmi )=0;

	  //-- Loads Bitmap -----------------------------------------------------
	  //
	  //   Loads a bitmap.
	  //
	  //   Like most other  bitmap  functions, you should define the image to
	  //   to load (either setting the name/device directly in BitmapInfo bi,
	  //   or having  SelectFileInput() do it for you).  Once bi has the name
	  //   of the image you want to  load, call Load() to create a new Bitmap
	  //   which contains the image. Additional options may be set by calling
	  //   ImageInputOptions()  before calling Load().  That will as the user 
	  //   for special details such as  positioning of smaller/larger images,
	  //   etc. All this does is to set the proper fields in BitmapInfo. It's
	  //   up to you to use those.
	  //
	  //   BitmapInfo defaults to frame "zero". For multifrane files, such as
	  //   *.avi, *.ifl, *.flc, etc. you should  specify the frame number you
	  //   want. Do it by using bi.SetCurrentFrame(f)  before calling Load().
	  //   
	  //   
	  //   Note: If loading images from a device, make sure bi.Name() is
	  //         empty (bi.SetName(_M(""));). This is automatic if you use
	  //         SelectDeviceInput(). If you just create a BitmapInfo
	  //         instance and set the device name by hand (bi.SetDevice()),
	  //         this is also automatic as both name and device names are
	  //         by default empty. This should only be a concern if you
	  //         reuse a BitmapInfo class previously used for image files.
	  //   
	  //   
	  //   One of the methods in BitmapInfo returns a  window handle to  send
	  //   progress report messages. If you  want to  receive these messages,
	  //   set the window handle  (  bi->SetUpdateWindow(hWnd)  ) and process
	  //   BMM_PROGRESS messages (see above).
	  //   
	  //   Gus
	  //
	  
	  /*! \remarks This method loads a bitmap using the parameters specified
	  by the <b>BitmapInfo</b> pointer. Make sure you delete the Bitmap created
	  when you are done using it.\n\n
	  Note: When several plug-ins call this method to load the same image, they
	  all receive the same pointer to one instance of the <b>BitmapStorage</b>.
	  So if one plug-in manipulates the image, the changes will get reflected
	  everywhere. A developer may use <b>BitmapManager::Create()</b> followed
	  by <b>Bitmap::CopyImage()</b> to create a unique instance of
	  <b>BitmapStorage</b>.\n\n
	  Also Note: One of the methods in <b>BitmapInfo</b> returns a window
	  handle to send progress report messages. If you want to receive these
	  messages (for purposes of putting up a progress bar during the load), set
	  the window handle (<b>bi-\>SetUpdateWindow(hWnd)</b>) and process
	  <b>BMM_PROGRESS</b> and <b>BMM_CHECKABORT</b> messages.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  Specifies the properties of the bitmap to load.\n\n
	  <b>BMMRES *status</b>\n\n
	  The result of the bitmap load operation. See \ref bitmapErrorCodes.
	  \return  A pointer to a new instance of the class Bitmap. */
	  BMMExport virtual Bitmap        *Load                       ( BitmapInfo *bi, BMMRES *status = NULL)=0;

	  //-- Load Image into an existing Bitmap  ----------------

	  /*! \remarks This method loads the bitmap specified by <b>bi</b> into
	  the bitmap pointed to by <b>map</b>. The normal <b>Load()</b> method
	  creates a new bitmap. However, if you already have an existing bitmap and
	  simply want to load in a new frame, this method may be used. Specify
	  which bitmap to use using <b>bi</b> and the map to load into using
	  <b>map</b>. For instance, if you have an AVI file and you want to load a
	  new frame, you can simply update the frame number specified in the
	  <b>BitmapInfo</b> and call this method passing the bitmap associates with
	  the previous frame.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  Specifies the properties of the bitmap to load.\n\n
	  <b>Bitmap **map</b>\n\n
	  A pointer to a pointer to a bitmap. This is the bitmap that will be
	  loaded into.\n\n
	  <b>BOOL forceReload=FALSE</b>\n\n
	  If an existing bitmap that matches <b>bi</b> is already loaded, then
	  calling calling <b>LoadInto()</b> won't load from the disk or device.
	  Rather it will just use the existing in memory version. If you want to
	  force the bitmap to be reloaded from the file or device set this to TRUE.
	  \return  The result of the bitmap load operation. See \ref bitmapErrorCodes. */
	  virtual BMMRES         LoadInto                    ( BitmapInfo *bi, Bitmap **map, BOOL forceReload=FALSE )=0;
	  
	  //-- General User Interface -------------------------------------------
	  
	  /*! \remarks This method will display information about the given bitmap
	  in a dialog. The source of the information is either defined in
	  <b>bi-\>Name()</b>/<b>bi-\>Device()</b> or explicitly in the
	  <b>filename</b> passed). This method is an interface into
	  <b>BitmapIO::GetImageInfoDlg()</b>. It is not normally called by
	  developers.\n\n
	  The default implementation is within the Bitmap Manager. There is a
	  generic Image Info dialog that is used unless the proper BitmapIO class
	  implements it own dialog (and notifies the system through the
	  <b>BitmapIO::Capabilities()</b> method).
	  \par Parameters:
	  <b>HWND hWnd</b>\n\n
	  The parent window handle calling the dialog.\n\n
	  <b>BitmapInfo *bi</b>\n\n
	  Defines the name of the bitmap or device (unless specified below). The
	  image information fields of BitmapInfo *bi are set with the information
	  loaded from the image.\n\n
	  <b>const MCHAR *filename = NULL</b>\n\n
	  Specifies the filename to use explicitly.
	  \return  The result of the operation. See \ref bitmapErrorCodes. */
	  virtual BMMRES         GetImageInfoDlg             ( HWND hWnd, BitmapInfo *bi, const MCHAR *filename = NULL )=0;
	  /*! \remarks This method is used to get information about an image, ie
	  things like image resolution (<b>bi-\>Width()/bi-\>Height()</b>), number
	  of frames, etc. This is an interface into
	  <b>BitmapIO::GetImageInfo()</b>. Given an image definition in
	  <b>bi.Name()</b> / <b>bi.Device()</b> or explicitly in filename (this
	  function will place filename, if not NULL, into <b>bi.Name()</b> before
	  calling <b>BitmapIO::GetImageInfo()</b>), the proper device will fill the
	  data members in <b>BitmapInfo *bi</b> with information about the image.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  Defines the name of the bitmap or device (unless specified below).\n\n
	  <b>const MCHAR *filename = NULL</b>\n\n
	  Specifies the filename to use explicitly.
	  \return  The result of the operation. See \ref bitmapErrorCodes. */
	  virtual BMMRES         GetImageInfo                (            BitmapInfo *bi, const MCHAR *filename = NULL )=0;
	  /*! \remarks This method brings up the standard 3ds Max Image Input
	  Options dialog box. If the users selects OK from the dialog, the
	  appropriate data members of <b>BitmapInfo *bi</b> are filled specifying
	  the user's choices. These are the 'Custom' fields accessed using methods
	  such as <b>GetCustomX()</b>, <b>GetCustomGamma()</b>,
	  <b>GetCustomStep()</b>, etc.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  The instance of BitmapInfo that is updated based on the users dialog
	  selections.\n\n
	  <b>HWND hWnd</b>\n\n
	  The parent window handle for the dialog.
	  \return  Returns TRUE if the users selects OK from the dialog; otherwise
	  FALSE. */
	  virtual BOOL           ImageInputOptions           ( BitmapInfo *bi, HWND hWnd )=0;
	  /*! \remarks Brings up the standard 3ds Max Select Image Input Device
	  dialog box. If the users selects OK from the dialog, then
	  <b>bi-\>Device()</b> is set to the name of the users device choice.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  Points to the instance of BitmapInfo that is updated based on the users
	  dialog selections.\n\n
	  <b>HWND hWnd</b>\n\n
	  The parent window handle for the dialog.
	  \return  TRUE if the user exited the dialog using OK; otherwise FALSE. */
	  virtual BOOL           SelectDeviceInput           ( BitmapInfo *bi, HWND hWnd )=0;
	  /*! \remarks Brings up the standard 3ds Max Select Image Output Device
	  dialog box. If the users selects OK from the dialog, then
	  <b>bi-\>Device()</b> is set to the name of the users device choice.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  Points to the instance of BitmapInfo that is updated based on the users
	  dialog selections.\n\n
	  <b>HWND hWnd</b>\n\n
	  The parent window handle for the dialog.
	  \return  TRUE if the user exited the dialog using OK; otherwise FALSE. */
	  virtual BOOL           SelectDeviceOutput          ( BitmapInfo *bi, HWND hWnd )=0;

// flag passed in to SelectFileOutput
#define BMM_ENABLE_SAVE_REGION  1
// Flag returned by SelectFileOutput
#define BMM_DO_SAVE_REGION		2

	  /*! \remarks Brings up the standard 3ds Max Browse Images for Output
	  dialog box. If the users selects OK from the dialog, then
	  <b>bi-\>Name()</b> is set to the name of the users file choice.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  Points to the instance of BitmapInfo that is updated based on the users
	  dialog selections.\n\n
	  <b>HWND hWnd</b>\n\n
	  The parent window handle for the dialog.\n\n
	  <b>MCHAR *title = NULL</b>\n\n
	  The optional title string to display in the title bar of the dialog.\n\n
	  <b>ULONG *pflags = NULL</b>\n\n
	  This parameter is available in release 4.0 and later only.\n\n
	  One of the following:\n\n
	  <b>BMM_ENABLE_SAVE_REGION</b>\n\n
	  This flag will cause the "SaveRegion" check box to appear in the
	  dialog.\n\n
	  <b>BMM_DO_SAVE_REGION</b>\n\n
	  This flag will return the state of the check box.
	  \return  TRUE if the user exited the dialog using OK; otherwise
	  FALSE.\n\n
	  virtual BOOL SelectFileOutput ( BitmapInfo *bi, HWND hWnd, MCHAR *title =
	  NULL, ULONG *pflags = NULL) */
	  virtual BOOL           SelectFileOutput            ( BitmapInfo *bi, 
															 HWND hWnd, 
															 MCHAR *title = NULL,
															 ULONG *pflags = NULL)=0;

	  /*! \remarks Brings up the standard 3ds Max Browse Images for Input
	  dialog box. If the users selects OK from the dialog, then
	  <b>bi-\>Name()</b> is set to the name of the users file choice.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  The instance of BitmapInfo that is updated based on the users dialog
	  selections.\n\n
	  <b>HWND hWnd</b>\n\n
	  The parent window handle for the dialog.\n\n
	  <b>MCHAR *title = NULL</b>\n\n
	  The optional title string to display in the title bar of the dialog.
	  \return  TRUE if the user exited the dialog using OK; otherwise FALSE. */
	  virtual BOOL           SelectFileInput             ( BitmapInfo *bi, 
															 HWND hWnd, 
															 MCHAR *title = NULL)=0;
	  
	  /*! \remarks This method brings up the standard 3ds Max Browse Images
	  for Input dialog box (the same as <b>SelectFileInput()</b>) but a
	  "Devices" button is present so the user can select both image files and
	  image devices.
	  \par Parameters:
	  <b>BitmapInfo *bi</b>\n\n
	  The instance of BitmapInfo that is updated based on the users dialog
	  selections.\n\n
	  <b>HWND hWnd</b>\n\n
	  The parent window handle for the dialog.\n\n
	  <b>MCHAR *title = NULL</b>\n\n
	  The optional title string to display in the title bar of the dialog.\n\n
	  <b>BOOL viewonly = FALSE</b>\n\n
	  If <b>viewonly</b> is set to TRUE, the View button is hidden in the
	  dialog.
	  \return  TRUE if the user exited the dialog using OK; otherwise FALSE. */
	  virtual BOOL           SelectFileInputEx           ( BitmapInfo *bi, 
															 HWND hWnd, 
															 MCHAR *title  = NULL,
															 BOOL viewonly = FALSE)=0;
	  
	  /*! \remarks	  This method refreshes the interior of all the virtual frame buffer
	  windows with each bitmap's contents. */
	  virtual void RefreshAllVFBs()=0;

	  /*! \remarks	  This method calls <b>Bitmap::DeleteThis()</b> on all the bitmaps whose
	  virtual frame buffers are set to autonomous. */
	  virtual void DeleteAllAutonomousVFBMaps()=0;

// cmd values passed in to Execute
#define BMM_STORE_GEOREF_DATA				0 // Used in VIZ only; arg1: const MCHAR*; arg2: GeoTableItem*; arg3: not used
#define BMM_RETRIEVE_GEOREF_DATA			1 // Used in VIZ only; arg1: const MCHAR*; arg2: GeoTableItem**; arg3: not used
#define BMM_USE_CUSTOM_FILTERLIST			2 // Used to pass a Filter list to the BitmapManager; arg1 MCHAR *, arg2,arg3 not used  
#define BMM_SET_FILE_INPUT_SEQUENCE_FLAG	3 // Used to set the sequence flag  for file input 
#define BMM_GET_FILE_INPUT_SEQUENCE_FLAG	4 // Used to get the sequence flag for file input 
#define BMM_FLUSH_RELATIVE_FILE_RESOLUTION_CACHE 5 // Used to flush the internal cache used to avoid having to re-resolve relative paths on each LoadInto call

	  // Generic expansion function
	  /*! \remarks	  This is a general purpose function that allows the API to be extended in the
	  future. The 3ds Max development team can assign new <b>cmd</b> numbers and
	  continue to add functionality to this class without having to 'break' the API.
	  \par Parameters:
	  <b>int cmd</b>\n\n
	  The index of the command to execute.\n\n
	  <b>ULONG arg1=0</b>\n\n
	  Optional argument 1. See the documentation where the <b>cmd</b> option is
	  discussed for more details on these parameters.\n\n
	  <b>ULONG arg2=0</b>\n\n
	  Optional argument 2.\n\n
	  <b>ULONG arg3=0</b>\n\n
	  Optional argument 3.
	  \return  An integer return value. See the documentation where the <b>cmd</b>
	  option is discussed for more details on the meaning of this value. */
	  virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0)=0; 

	  /*! \remarks	  This method is used for accumulating the names of bitmap files that
	  didn't load. Instead of having the BitmapManager display the missing file
	  dialog, it now just collects the names (which can be retrieved using
	  <b>GetLoadErrorFileList()</b> below). */
	  virtual void BeginSavingLoadErrorFiles()=0;  // --Begin accumulating a list of files that didn't load,
												   // and don't put up load errors in the meantime.
	  /*! \remarks	  This method will return a list of names of bitmap files that were not
	  found as discussed in the method above. See
	  Class NameTab. */
	  virtual NameTab &GetLoadErrorFileList()=0;  // -- List of names of files not found
	  /*! \remarks	  This method ends the accumulation of a list of bitmap files that didn't
	  load, and frees the list. See the two methods above. */
	  virtual void EndSavingLoadErrorFiles()=0;  // --End accumulating a list of files that didn't load, free the list.
	  /*! \remarks	  Returns true if the extension of the specified file name is one of the
	  supported types (i.e. there is a BitmapIO module for it); otherwise
	  false.
	  \par Parameters:
	  <b>const MCHAR* filename</b>\n\n
	  The file name to check. */
	  virtual BMMExport bool CanImport(const MCHAR* filename)=0;

		/*! \remarks Implements the 'strip path' action. */
	  inline void ClearInternalRelativePathResolutionCache()	{ Execute(BMM_FLUSH_RELATIVE_FILE_RESOLUTION_CACHE); }

};

#pragma warning(pop)
//-----------------------------------------------------------------------------
//-- Bitmap Proxy Manager Class

//! \brief Interface ID of the IBitmapProxyManager interface
#define FPBITMAPPROXYMANAGER_INTERFACE_ID Interface_ID(0x24e22528, 0x601f3610)

//! \brief Bitmap Proxy Manager Function-Published Interface
class IBitmapProxyManager : public FPStaticInterface {
	public:
		virtual BOOL		GetGlobalProxyEnable() = 0;						//!< Returns TRUE if the proxy system is enabled, FALSE otherwise
		virtual void		SetGlobalProxyEnable( BOOL enable ) = 0;		//!< Enables or disables the proxy system

		/*! \defgroup proxyRenderMode Bitmap Proxy Render Mode */
		//@{
		enum RenderMode {
			//! Use proxies for rendering.
			kRenderMode_UseProxies,
			//! Do not use proxies for rendering, and keep the full-res image in memory when done rendering.
			//! This option provides high performance as the full-res image does not have to be
			//! re-loaded at every render.
			kRenderMode_UseFullRes_KeepInMemory,
			//! Do not use proxies for rendering, but flush the full-res image from memory when done rendering.
			//! This option reduces memory usage as the full-res image is not maintained in memory
			//! when not needed. The full-res image has to be re-loaded at every render, though.
			kRenderMode_UseFullRes_FlushFromMemory
		};
		//@}
		/*! \return The proxy render mode selected currently in use. */
		virtual RenderMode	GetGlobalProxyRenderMode() = 0;
		/*! Sets the proxy render mode to be used.
			\param[in] renderMode The render mode to be set.
		*/
		virtual void		SetGlobalProxyRenderMode( RenderMode renderMode ) = 0;
   
		/*! \defgroup proxySizeFactor Bitmap Proxy Size Factor values */
		//@{
		enum {
							proxySizeFactor_Full=1,							//!< Proxy is the full size of the original bitmap
							proxySizeFactor_Half=2,							//!< Proxy is half size in each dimension from the original bitmap
							proxySizeFactor_Third=3,						//!< Proxy is one third the size in each dimension from the original bitmap
							proxySizeFactor_Quarter=4,						//!< Proxy is one quarter the size in each dimension from the original bitmap
							proxySizeFactor_Eighth=8,						//!< Proxy is one eighth the size in each dimension from the original bitmap
		};
		//@}
		virtual int			GetGlobalProxySizeFactor() = 0;					//!< Returns the default factor by which proxies are scaled in size (half, quarter, etc.)
		virtual void		SetGlobalProxySizeFactor( int factor ) = 0;		//!< Sets the default factor by which proxies are scaled in size (half, quarter, etc.)
		virtual int			GetGlobalProxySizeMin() = 0;					//!< Returns the default minimum size, below which bitmap are not proxied
		virtual void		SetGlobalProxySizeMin( int min ) = 0;			//!< Sets the default minimum size, below which bitmap are not proxied

		virtual int			GetProxySizeFactor(const MCHAR* filename) = 0;				//!< Returns the proxy size factor for a specific bitmap. (proxySizeFactor_Half, proxySizeFactor_Quarter, etc.)
		virtual void		SetProxySizeFactor(const MCHAR* filename, int factor) = 0;	//!< Sets the proxy size factor for a specific bitmap. (proxySizeFactor_Half, proxySizeFactor_Quarter, etc.)
		virtual BOOL		GetProxyUseGlobal(const MCHAR* filename) = 0;				//!< Returns whether a specific bitmap uses global proxy settings instead of its own settings.
		virtual void		SetProxyUseGlobal(const MCHAR* filename, BOOL global) =0 ;	//!< Sets whether a specific bitmap uses global proxy settings instead of its own settings.
		virtual BOOL		GetProxyReady(const MCHAR* filename) = 0;					//!< Returns whether the desired proxy is ready for use, or is stale (in memory or on disk).

		/*! \defgroup proxyRefresh Bitmap Proxy Refresh Request values
		Used by RefreshProxies() and GenerateProxies() */
		//@{
		enum {
							proxyRequestRefresh_UpdateStale=1,				//!< Refresh proxies and cache files only if stale or uncached
							proxyRequestRefresh_RefreshAll=2,				//!< Refresh all proxies in memory using cached files if available
							proxyRequestRefresh_GenerateAll=4,				//!< Refresh all proxies, generating new cache files for each
		};
		//@}		
		virtual void		RefreshProxies( int requestRefresh ) = 0;		//!< Reloads correct versions of proxies into memory, if proxy settings have changed
		virtual void		GenerateProxies( int requestRefresh ) = 0;		//!< Generates proxy bitmaps needed by the scene and caches them to disk

		//! \brief Displays the UI to configure proxy settings for the given bitmaps, or to configure the global settings if the input is NULL
		virtual void		ShowConfigDialog( Tab<MCHAR*>* bitmapFilenames ) = 0;

		//! \brief Displays the UI to update the proxy cache for the given bitmaps, or to update all out-of-date cache files if the input is NULL
		virtual void		ShowPrecacheDialog( Tab<MCHAR*>* bitmapFilenames ) = 0;

		/*! \defgroup proxyRequestFlags Bitmap Proxy Request flags, used by LoadInto()
		*/
		//@{
		enum {
							proxyRequestFlags_ForceProxyMode=0x40000000,	//!< Flag, forces use of proxies, even if proxy mode is off
		};
		//@}

		/*! \defgroup proxyLoad Bitmap Proxy Load Request values
		Used by LoadInto() */
		//@{
		enum {
							proxyRequestLoad_UseLoadedProxy=1,				//!< Proxy bitmap taken from (possibly stale) in-memory cache if available. Otherwise, same as ReloadProxy
							proxyRequestLoad_ReloadProxy=2,					//!< Proxy file is reloaded from disk if available and if up-to-date. Otherwise, same as UseLoadedSubject

							//! \brief Original subject bitmap taken from (possibly stale) in-memory cache if available. Subject is downsized to proxy, and saved to disk cache.
							//!        Otherwise if subject is not available from in-memory, same as ReloadSubject
							proxyRequestLoad_UseLoadedSubject=3,
							proxyRequestLoad_ReloadSubject=4,				//!< Original subject bitmap is reloaded from disk. Subject is downsized to proxy and saved to disk cache
		};
		//@}
		//! \brief Loads a proxy bitmap storage into the input bitmap, according to the input request info
		//! \see MAP_PROXY
		virtual BMMRES         LoadInto( BitmapInfo* requestInfo, Bitmap** pbitmap, int requestLoad, int requestFlags=0 ) = 0;
};

//! \brief Returns the Bitmap Proxy Manager singleton object
BMMExport IBitmapProxyManager* GetBitmapProxyManager();

//! \brief Useful Guard class for temporarily disabling bitmap proxies.
/*! Bitmap proxies are disabled in the constructor, and disabled in the destructor,
	using NOTIFY_PROXY_TEMPORARY_DISABLE_START and NOTIFY_PROXY_TEMPORARY_DISABLE_END.

	For more details on the way this disables bitmap proxies, see NOTIFY_PROXY_TEMPORARY_DISABLE_START.

	Recursive calls are handled correctly; a count of recursive calls is maintained
	and bitmap proxies are only re-enabled once all instances of this class have been destroyed.
*/
class BitmapProxyTemporaryDisableGuard: public MaxHeapOperators {
public:
	/*! Constructor, disables bitmap proxies.
		\param[in] doDisable If true, bitmap proxies are disabled. 
		If false, nothing is done (the class is essentially disabled).
		This parameter is necessary to conditionally disable proxies.
	*/
	BMMExport explicit BitmapProxyTemporaryDisableGuard(bool doDisable = true);
	//! Destructor, re-enables bitmap proxies.
	BMMExport ~BitmapProxyTemporaryDisableGuard();
private:
	static int m_numInstances;
	bool m_doDisable;
};


//! \brief Bitmap asset declaration flags.
#define BMM_ASSET_GROUP				1 //!< The declared bitmap represents a group. You must call EndGroup on the enum callback after you are finished grouping assets.
#define BMM_ASSET_PROXY				2 //!< The declared bitmap is proxy enabled.

//! \brief Declare a bitmap asset accessor to the specified callback.
/*! Decorates the specified asset accessor as a bitmap enabled asset accessor before declaring
	it to the specified enumeration callback.
	\param[in] enumCallback The EnumAuxFiles callback to declare the asset to.
	\param[in] anAccessor The original asset accessor to decorate with proxy settings.
	\param[in] bitmapInfo The bitmap info of the bitmap. If NULL, the declared bitmap will always
	be treated as a single frame, no pi-data bitmap.
	\param[in] flags Specify flags that define how the specified callback will declare the decorated
	accessor. Also controls how the accessor will be decorated.
	\see IEnumAuxAssetsCallback
	\see IAssetAccessor
*/
BMMExport void DeclareBitmapAsset(IEnumAuxAssetsCallback &enumCallback, IAssetAccessor &anAccessor, BitmapInfo* bitmapInfo, int flags=0);

//-----------------------------------------------------------------------------
//-- Forward References for Bitmap Functions
//
//   Internal Use
//

extern int						ValidBitmapType		( int type );
/*! \remarks This global function will create a new instance of the specified
storage type and return a pointer to it.
\par Parameters:
<b>BitmapManager *manager</b>\n\n
The bitmap manager used for this storage.\n\n
<b>UINT type</b>\n\n
The type of storage to create. One of the following values:\n\n
<b>BMM_LINE_ART</b>\n
<b>BMM_PALETTED</b>\n
<b>BMM_GRAY_8</b>\n
<b>BMM_GRAY_16</b>\n
<b>BMM_TRUE_16</b>\n
<b>BMM_TRUE_32</b>\n
<b>BMM_TRUE_64</b>
\return  The bitmap storage instance created or NULL if the specified type
could not be created.
\par Data Members:
protected:\n\n
<b>int openMode;</b>\n\n
The mode the storage was opened in. See \ref bitmapOpenModeTypes.\n\n
<b>UINT usageCount;</b>\n\n
The number of bitmaps using this storage.\n\n
<b>BitmapManager *manager;</b>\n\n
The bitmap manager associated with this storage.\n\n
<b>int flags;</b>\n\n
See  \ref bitmapFlags.\n\n
<b>int type;</b>\n\n
See \ref bitmapTypes.\n\n
<b>BMM_Color_48 palette[256];</b>\n\n
The palette entries (256 max). See
Structure BMM_Color_48.\n\n
<b>int paletteSlots;</b>\n\n
The number of palette slots used.\n\n
<b>UWORD *gammaTable;</b>\n\n
The gamma correction table.\n\n
<b>RenderInfo *rendInfo;</b>\n\n
A pointer to an instance of RenderInfo. See
Class RenderInfo.\n\n
<b>BitmapInfo bi</b>\n\n
Describes the properties of the bitmap associated with this storage.  */
extern BMMExport BitmapStorage*	BMMCreateStorage	( BitmapManager *manager,UINT type );
extern BMMExport BitmapFilter*	BMMCreateFilter		( BitmapManager *manager,UINT type );
extern BMMExport BitmapDither*	BMMCreateDither		( BitmapManager *manager,UINT type );
extern BMMExport int			BMMCalcPalette		( Bitmap *map,int colors,BMM_Color_48 *palette );
extern BMMExport BYTE			BMMClosestColor		( BMM_Color_64 *color,BMM_Color_48 *palette,int colors );
// Returns a localized string that describes one of the (BMM_TRUE_32, BMM_TRUE_64, etc.) bitmap types.
// The string is loaded into a static buffer and therefore should be used or copied immediately.
// The bitmap flags (from BitmapInfo::Flags()) are also used.
/*! \remarks A global function in bitmap.h\n\n
Returns a localized string that describes one of the (BMM_TRUE_32, BMM_TRUE_64,
etc.) bitmap types. The string is loaded into a static buffer and therefore
should be used or copied immediately. The bitmap flags (from
BitmapInfo::Flags()) are also used.
\par Parameters:
  */
extern BMMExport const MCHAR*	BMMGetBitmapTypeDescription(int bitmapType, DWORD bitmapFlags);

extern BMMExport void			OpenBMM				( BMMInterface *i );
extern BMMExport void			CloseBMM			( );
extern INT_PTR CALLBACK	BMMGammaDlgProc		(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
				// WIN64 Cleanup: Martell
extern           void            ExtractExtension	( MCHAR *string, MCHAR *ext );


//-- Public Utilities ---------------------------------------------------------

/*! \brief Create a filename appended by the specified frame number. This function will pad the filename with zeroes.
	\remarks For example, an input filename of sampleKitchen.bmp and frame 4 results in sampleKitchen0004.bmp
	\param[in] namein: A pointer to the buffer containing the base filename from which to create a numbered filename.
	\param[in] frame: The frame number to append. Can be BMM_SINGLEFRAME for convenience, which results in the orignal filename.
	\param[out] nameout: A pointer to the buffer in which to store the created filename.
This buffer must be large enough to accommodate the original filename, the digits of the framenumber, any zero-padding,
and the file extension.
	\return Zero if an error occurred, non-zero for success.
	\sa BMMFindNumberedFilenames
*/
BMMExport int			BMMCreateNumberedFilename	( const MCHAR *namein,int frame,MCHAR *nameout );

/*! \brief Retrieve a list of existing files in a sequence that matches the base filename and numbering of the specified filename.
	\remarks For example, a filename of sampleKitchen0001.bmp will return all existing files that match sampleKitchenxxxx.bmp, 
such as all files from sampleKitchen0000.bmp to sampleKitchen0060.bmp.
This is similar to the sequence detection used in the Image File List (IFL) image format.
	\param[in] namein: The filename to match.
	\param[out] foundFiles: The list of matching files found. This list is cleared by the function.
	\return The number of matching files found. Zero if none found or an error occurred.
	\sa BMMCreateNumberedFilename
*/
BMMExport int			BMMFindNumberedFilenames ( const MaxSDK::Util::Path &baseFilename, MaxSDK::Array<MaxSDK::Util::Path> &foundFiles);

/*! \defgroup bitmapFilename Bitmap Filename Resolution Functions */
/*!  These functions search the system for a bitmap. The input parameter contains the name of the 
bitmap that is searched for (bi->Name() or namein). If the filename from the input is incorrect, and 
the bitmap is found somewhere else, these functions set the output (bi->Name() or nameout) with 
the correct path. \n\n
	 The order of the search is as follows:
   - The full UNC path/filename indicated in the input object.
   - The path where the current 3ds max file was loaded from.
   - The directory tree under the directory where the current Max files was loaded.
   - The Map path.
	\return TRUE if the file was found; otherwise FALSE. */
//@{
BMMExport int			BMMGetFullFilename			( BitmapInfo *bi ); //<! Resolves the filename in the given BitmapInfo (bi->Name())
BMMExport BOOL			BMMGetFullFilename			( const MCHAR *namein, MCHAR* nameout ); //<! Resolves the given input name (namein) into the given output (nameout)
//@}

BMMExport BOOL			BMMIsFile					( const MCHAR *filename );
BMMExport void			BMMSplitFilename			( const MCHAR *name,MCHAR *p,MCHAR *f,MCHAR *e );
BMMExport LPTSTR		BMMGetLastErrorText			( LPTSTR lpszBuf, DWORD dwSize );
BMMExport Quantizer*	BMMNewQuantizer				();

//! \brief This function appends a slash character to the end of the path passed unless one already exists. 
/// \param[in,out] path: The path name to append. If NULL, no operation.

BMMExport void			BMMAppendSlash				( MCHAR *path );

//! \brief This function removes the slash character from the end of the path passed if present. 
/// \param[in,out] path: The path name to append. If NULL, no operation.

BMMExport void			BMMRemoveSlash				( MCHAR *path );

//! \brief This function appends a slash character to the end of the path passed unless one already exists. 
/// \param[in,out] path: The path name to append.

BMMExport void			BMMAppendSlash				( MSTR &path );

//! \brief This function removes the slash character from the end of the path passed if present. 
/// \param[in,out] path: The path name to append. If NULL, no operation.

BMMExport void			BMMRemoveSlash				( MSTR &path );

//-----------------------------------------------------------------------------
//-- Share Utilities
//
//  BMMGetUniversalName() 
//	---------------------
//  Given a path (E:\path\filename.ext), the function will check and see if 
//	this drive is mapped to a network share. If successful, the full UNC 
//  version will be returned in out_uncname ("\\computer\share\path\file.ext")
//  If the function returns FALSE, out_uncname will be left undefined.
//
//	This function has been enhanced to also return an UNC for a local drive
//  that happens to be shared. For instance, if you pass in something like
//  d:\data\images\maps\background\rottenredmond.tga and it happens that 
//  d:\data is shared as "Image Data", the function will return:
//  \\computername\Image Data\images\rottenredmond.tga.
//
//	Pass "nolocal" as TRUE if you just want to see if this is a network
//	share (don't check if this local drive is shared).
//
//	BMMFindNetworkShare()
//	---------------------
//	Given a path (E:\path\filename.ext) this function will check and see if
//	this [local] path is shared. If successful, it will return both the
//  share name and the path of the share.
//
//	BMMGetLocalShare()
//	------------------
//
//	This is the "second half" of BMMGetUniversalName() above. It is used
//	internally but it is exported here as a convenience. It will check local
//  paths only and return a UNC version if a share exists somewhere up in
//  the path hierarchy.
//
//  GG: 06/28/00

BMMExport BOOL	BMMGetUniversalName	( MCHAR *out_uncname, const MCHAR* in_path);
BMMExport BOOL	BMMFindNetworkShare	( const MCHAR* in_localpath, MCHAR* out_sharename, MCHAR* out_sharepath);
BMMExport BOOL	BMMGetLocalShare	( const MCHAR *local_path, MCHAR *share );

//-----------------------------------------------------------------------------

// Get a color packer.  When done, be sure to call its DeleteThis();
BMMExport ColorPacker *BMMNewColorPacker(
	int w,				// width of bitmap to be packed
	BMM_Color_48 *pal,	// palette to use
	int npal,			// number of entries in the palette
	BYTE* remap=NULL	// optional remap done at last stage.
	);

//-----------------------------------------------------------------------------
//  Rearrange palette "pal" ( which has colors 0..ncols-1 occupied, in 
//  descending order of frequency),  into "newpal" so that the colors 10-245 are 
//  populated first, then 0-9, then 246-255.  Sets optional array "remap" to map
//  the old palette index values to the new ones    
/*! \remarks Rearranges the palette <b>pal</b> (which has colors
0..<b>ncols</b>-1 occupied, in descending order of frequency), into
<b>newpal</b> so that the colors 10-245 are populated first, then 0-9, then
246-255. Sets the optional array <b>remap</b> to map the old palette index
values to the new ones.
\par Parameters:
<b>BMM_Color_48 *pal</b>\n\n
The palette to rearrange.\n\n
<b>BMM_Color_48 *newpal</b>\n\n
The rearranged palette.\n\n
<b>int ncols</b>\n\n
The number of colors in the palette.\n\n
<b>BYTE *remap=NULL</b>\n\n
An array that maps the old palette index values to the new ones.  */
BMMExport void FixPaletteForWindows(BMM_Color_48 *pal, BMM_Color_48 *newpal,int ncols, BYTE *remap=NULL);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-- Layer Utility (IO Support)
//

#define BITMAP_IOLAYER_CLASS	Interface_ID(0x296b79ec,0x73e11944)

class BitmapIOLayer : public BaseInterface {
	public:
		virtual BOOL			Init		(const MCHAR* filename)=0;
		virtual int				LayerCount	()=0;
		virtual const MCHAR*	LayerName	(int index)=0;
		virtual BOOL			SetLayer	(int index, BitmapInfo* bi, BOOL fulframe = TRUE)=0;
};

//-----------------------------------------------------------------------------
//-- Layer Utility (Global)
//

#define LAYER_INTERFACE				Interface_ID(0x1563269c,0x7ec41d89)
#define I_LAYER_INTERFACE			0x000A1001	

class BitmapLayerUtl : public FPStaticInterface  {
	public:
		virtual int				LayerCount	(const MCHAR* filename)=0;
		virtual const MCHAR*	LayerName	(const MCHAR* filename, int index)=0;
		virtual PBBitmap*		LoadLayer	(const MCHAR* filename, int index, BOOL fulframe = TRUE)=0;
};


//-----------------------------------------------------------------------------
//-- Bitmap Pager Class
//

//! \brief Interface ID of the IBitmapPager interface
#define IBITMAPPAGER_INTERFACE_ID Interface_ID(0x48e1041f, 0x34c86c51)

/*!
\sa  Class IBitmapPager
\par Description:
This class exposes parameters to the Bitmap Pager. This pager reduces memory footprint by swapping bitmap data to and from disk,
keeping the memory usage of bitmaps within a certain percentage of total available memory.
Internally, all bitmaps are partitioned into separately allocated tiles, but tiles are only swapped to disk when the pager is enabled.
For performance reasons the tile size varies dynamically for each bitmap, but is typically around 1-4 megabytes.
Bitmaps smaller than one tile are not eligible for paging, and are held in memory at all times.
*/
class IBitmapPager : public FPStaticInterface {
	public:
		DECLARE_DESCRIPTOR( IBitmapPager );

		//----------------------------------------------------------------------
		//-- Bitmap Pager options

		//! \brief Returns whether bitmap paging is enabled.
		/** Internally, all bitmaps are partitioned into separately allocated tiles.
		With paging is enabled, bitmap tiles are swapped to disk, keeping total bitmap usage below the memory limit. */
		virtual BOOL IsPagerEnabled() = 0;
		//! \brief Enables or disables bitmap paging.
		/** Internally, all bitmaps are partitioned into separately allocated tiles.
		Passing TRUE immediately swaps bitmap tiles from memory to disk, reducing total bitmap usage below the memory limit.
		Passing FALSE does not immediately swap bitmap tiles from disk to memory,
		but no further tiles are swapped to disk while paging is disabled. */
		virtual void SetPagerEnabled( BOOL b ) = 0;

		/**
			\defgroup PageFilePath Page File Path
			The folder location for bitmap tile pagefiles.
			The value is read from an ini file when the application is launched, and saved to an ini file when the application is closed.
		*/
		//@{
		//! \brief Returns the page file path.
		virtual const MCHAR* GetPageFilePath() = 0;
		//! \brief Sets the page file path.
		/** Changing the pagefile path does not move existing pagefiles to the new location,
		but any bitmaps created later will page to the new location. */
		virtual void SetPageFilePath(const MCHAR* path) = 0;
		//@}	END OF PageFilePath Page File Path

		/**
			\defgroup MemoryLimitPercentage Memory Limit Percentage
			When paging is enabled, the pager swaps bitmap tiles from memory to disk, keeping the total bitmap usage below this limit.
			This is measured as a percentage of available padded memory. Internally, a physical limit (in megabytes) is calculated
			from the percentage at specific times, during file load, file reset, render begin and render end.
			Whenf Memory Limit Auto Mode is enabled, the Memory Limit Percentage is overwritten on every Auto Mode update. 
			\see GetMemoryAvailablePadded()
		*/
		//@{
		//! \brief Returns the memory limit percentage (as a fraction from 0.0 to 1.0) of available padded memory.
		virtual float GetMemoryLimitPercent() = 0;
		//! \brief Sets the memory limit percentage (as a fraction from 0.0 to 1.0) of available padded memory.
		/** This method forces an immediate recalculation of the physical limit, and swaps bitmap tiles from memory to disk if necessary. */
		virtual void SetMemoryLimitPercent( float percent ) = 0;
		//@}	END OF MemoryLimitPercentage Memory Limit Percentage

		/**
			\defgroup MemoryPadding Memory Padding
			This portion of available memory is set aside as unusable by the pager,
			affecting how the physical memory limit is computed from the memory limit percentage.
		*/
		//@{
		//! \brief Returns the memory padding percent (as a fraction from 0.0 to 1.0) of available memory.
		virtual float GetMemoryPaddingPercent() = 0;
		//! \brief Sets the memory padding percent (as a fraction from 0.0 to 1.0) of available memory.
		/** Setting a higher value may decrease risk of out-of-memory crashes.  Setting a lower value may increase performance. */
		virtual void SetMemoryPaddingPercent( float percent ) = 0;
		//@}	END OF MemoryPadding Memory Padding


		/**
			\defgroup MemoryLimitAutoMode Memory Limit Auto Mode
			When enabled, Auto Mode adjusts the memory limit percentage automatically using a heuristic formula.
			The adjutment happens when the physical limit is calculated, during file load, file reset, render begin and render end.
			Any value set manually for the Memory Limit Percentage is overwritten by Auto Mode updates.
		*/
		//@{
		//! \brief Returns whether Memory Limit Auto Mode is enabled for the pager.
		virtual BOOL GetMemoryLimitAutoMode() = 0;
		//! \brief Sets whether Memory Limit Auto Mode is enabled for the pager.
		virtual void SetMemoryLimitAutoMode( BOOL onOff ) = 0;
		//@}	END OF MemoryLimitAutoMode Memory Limit Auto Mode


		//! \brief Returns the current memory limit physical value, in bytes.
		/** This is the value used internally in deciding whether to swap bitmap tiles to disk. The value is not live,
			but is calculated from the memory limit percentage at specific times, during file load, file reset, render begin and render end. */
		virtual size_t GetMemoryLimit() = 0;
		virtual size_t GetMemoryUsedForPager() = 0;		//!<  Returns the total memory usage of bitmap tiles (excluding tiles on disk), in bytes
		virtual size_t GetMemoryPadding() = 0;			//!<  Returns the current memory padding physical value, in bytes
		virtual size_t GetMemoryAvailablePadded() = 0;	//!<  Returns the amount of available memory for the entire application, after padding, in bytes
		// 64-bit values, may exceed 2GB
		virtual LONGLONG GetMemoryTotalForPager() = 0;	//!<  Returns the total memory and disk usage of bitmap tiles, in bytes.  May exceed 2GB.
};

//! \brief Returns the Bitmap Pager singleton object
inline IBitmapPager* GetIBitmapPager()
{ return (IBitmapPager*)GetCOREInterface(IBITMAPPAGER_INTERFACE_ID); }

