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
// FILE:        maxtypes.h
// DESCRIPTION: Typedefs for general jaguar types.
// AUTHOR:      Rolf Berteig
// HISTORY:     created 19 November 1994
//**************************************************************************/

#pragma once

#include "maxheap.h"
#include <wtypes.h>
#include <basetsd.h>

typedef unsigned long ulong;
typedef unsigned char uchar;
typedef uchar UBYTE;
typedef unsigned short USHORT;
typedef unsigned short UWORD;
typedef __int64        INT64;
typedef unsigned short MtlID;				

/*! \defgroup bitmapStorageTypes Bitmap Storage Types */
//@{
//! \brief 8 bits for each of the Red, Green and Blue components.
/*! Structure Color48, Structure Color64. */
struct Color24: public MaxHeapOperators {
	Color24(uchar vr = 0, uchar vg = 0, uchar vb = 0): r(vr), g(vg), b(vb) {}
	uchar r,g,b;
};

//! \brief 16 bits for each of the Red, Green and Blue components.
/*! \sa Structure Color24, Structure Color64. */
struct Color48: public MaxHeapOperators {
	Color48(UWORD vr = 0, UWORD vg = 0, UWORD vb = 0): r(vr), g(vg), b(vb) {}
	UWORD r,g,b;
};

//! \brief 16 bits for each of the Red, Green, Blue, and Alpha components.
/*! \sa Structure Color24, Structure Color48. */
struct Color64: public MaxHeapOperators { 
	Color64(UWORD vr = 0, UWORD vg = 0, UWORD vb = 0, UWORD va = 0): r(vr), g(vg), b(vb), a(va) {}
	UWORD r,g,b,a;
};

//-- Pixel storage classes used by BitmapManager ----------------------------------------------------

//! \brief 24 bit color: 8 bits each for Red, Green, and Blue.
/*! \sa Class Bitmap, Class BitmapStorage, Class BitmapManager. */
struct BMM_Color_24: public MaxHeapOperators {
	BMM_Color_24(BYTE vr = 0, BYTE vg = 0, BYTE vb = 0): r(vr), g(vg), b(vb) {}
	BYTE r,g,b;
};

//! \brief 32 bit color: 8 bits each for Red, Green, Blue, and Alpha.
/*! \sa Class Bitmap, Class BitmapStorage, Class BitmapManager. */
struct BMM_Color_32: public MaxHeapOperators {
	BMM_Color_32(BYTE vr = 0, BYTE vg = 0, BYTE vb = 0, BYTE va = 0): r(vr), g(vg), b(vb), a(va) {}
	BYTE r,g,b,a;
};

//! \brief 48 bit color: 16 bits each for Red, Green, and Blue.
/*! \sa Class Bitmap, Class BitmapStorage, Class BitmapManager. */
struct BMM_Color_48: public MaxHeapOperators {
	BMM_Color_48(WORD vr = 0, WORD vg = 0, WORD vb = 0): r(vr), g(vg), b(vb) {}
	WORD r,g,b;
};

//! \brief 64 bit color: 16 bits each for Red, Green, Blue, and Alpha.
/*! \sa Class Bitmap, Class BitmapStorage, Class BitmapManager. */
struct BMM_Color_64: public MaxHeapOperators {
	BMM_Color_64(WORD vr = 0, WORD vg = 0, WORD vb = 0, WORD va = 0): r(vr), g(vg), b(vb), a(va) {}
	WORD r,g,b,a;
};

//! \brief High Dynamic Range bitmaps make use of this class to store color information using floating point values.
/*! \sa Class Bitmap, Class BitmapStorage, Class BitmapManager, <a href="ms-its:3dsmaxsdk.chm::/bitmaps_root.html">Working with Bitmaps</a>. */
struct BMM_Color_fl : public MaxHeapOperators
{
	BMM_Color_fl(float vr = 0.0f, float vg = 0.0f, float vb = 0.0f, float va = 0.0f): r(vr), g(vg), b(vb), a(va) {}

	/*! Storage for the floating point color information. */
	float r,g,b,a;

	/*! \remarks Returns the address of the floating point values. */
	operator float*() { return &r; }
	/*! \remarks Returns the address of the floating point values. */
	operator const float*() const { return &r; }

	/*! \remarks Returns the specified color c clipped (limited to) the range 0 to 65535. */
	static WORD clipColor(float c)
	{
		return c <= 0.0f ? 0 : c >= 1.0f ? 65535 : (WORD)(c * 65535.0);
	}
};
//@}

/* Time:
*/
typedef int TimeValue;

#define TIME_TICKSPERSEC	4800

#define TicksToSec( ticks ) ((float)(ticks)/(float)TIME_TICKSPERSEC)
#define SecToTicks( secs ) ((TimeValue)(secs*TIME_TICKSPERSEC))
#define TicksSecToTime( ticks, secs ) ( (TimeValue)(ticks)+SecToTicks(secs) )
#define TimeToTicksSec( time, ticks, secs ) { (ticks) = (time)%TIME_TICKSPERSEC; (secs) = (time)/TIME_TICKSPERSEC ; } 

#define TIME_PosInfinity	TimeValue(0x7fffffff)
#define TIME_NegInfinity 	TimeValue(0x80000000)


//-----------------------------------------------------
// Class_ID
//-----------------------------------------------------
/*! \sa  <a href="ms-its:3dsmaxsdk.chm::/start_dll_functions.html">Dll
Functions and Class Descriptors</a>,
<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_class_ids.html">List of Class_IDs</a>\n\n
\par Description:
This class represents the unique class ID for a 3ds Max plug-in. A plug-ins
Class_ID must be <b>unique</b>. A program is provided with the SDK to generate
these ClassIDs. It is VERY important you use this program to create the
ClassIDs for your plug-ins. To generate a random Class_ID and optionally copy
it to the clipboard, run the <b>gencid.exe</b> program. A Class_ID consists of
two unsigned 32-bit quantities. The constructor assigns a value to each of
these, for example <b>Class_ID(0xCAD834E2, 0x27E47C5A)</b>.\n\n
All the methods of this class are implemented by the system.   */
class Class_ID: public MaxHeapOperators {
	ULONG a,b;
public:
	/*! \remarks Constructor.\n\n
	Assigns a value of <b>0xFFFFFFFF</b> to each 32-bit quantity. */
	Class_ID() { a = b = 0xffffffff; }
	/*! \remarks Constructor.\n\n
	Creates a new class ID with the same values as the argument.
	\par Parameters:
	<b>const Class_ID\& cid</b>\n\n
	A reference to the Class ID to copy. */
	Class_ID(const Class_ID& cid) { a = cid.a; b = cid.b;	}
	/*! \remarks Constructor.\n\n
	This is the standard constructor to be used by 3ds Max plug-ins. Each
	of the 32-bit quantities may be assigned separately.
	\par Parameters:
	<b>ulong aa</b>\n\n
	Assigned to the first 32-bit quantity.\n\n
	<b>ulong bb</b>\n\n
	Assigned to the second 32-bit quantity. */
	Class_ID(ulong aa, ulong bb) { a = aa; b = bb; }
	/*! \remarks Returns the first unsigned 32-bit quantity. */
	ULONG PartA() const { return a; }
	/*! \remarks Returns the second unsigned 32-bit quantity.
	\par Operators:
	*/
	ULONG PartB() const { return b; }
	void SetPartA( ulong aa ) { a = aa; } //-- Added 11/21/96 GG
	void SetPartB( ulong bb ) { b = bb; }
	/*! \remarks Checks for equality between two Class IDs. */
	int operator==(const Class_ID& cid) const { return (a==cid.a&&b==cid.b); }
	/*! \remarks Check for Inequality between two Class IDs. */
	int operator!=(const Class_ID& cid) const { return (a!=cid.a||b!=cid.b); }
	/*! \remarks Assignment operator. Note: In release 3.0 and later this
	method checks for self-assignment. */
	Class_ID& operator=(const Class_ID& cid)  { a=cid.a; b = cid.b; return (*this); }
	// less operator - allows for ordering Class_IDs (used by stl maps for example) 
	/*! \remarks This operator is available in release 4.0 and later
	only.\n\n
	Less than operator. This returns true if the specified Class_ID's two
	parts are numerically less than this Class_ID's; false if not. */
	bool operator<(const Class_ID& rhs) const
	{
		if ( a < rhs.a || ( a == rhs.a && b < rhs.b ) )
			return true;

		return false;
	}
};

// SuperClass ID
typedef ULONG SClass_ID;  	// SR NOTE64: was ULONG_PTR, which was WRONG.

/*! \sa  Class FPInterface, <a href="ms-its:3dsmaxsdk.chm::/function_root.html">Function Publishing System</a>.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class is the interface ID for the Function Publishing System of 3ds Max.
This class is structurally very similar to a Class_ID, containing two
randomly-chosen longwords to provide a unique global ID. The various
constructors assign a value to each of these. There are also methods to assign
and retrieve the individual parts and operators to check for equality or
inequality.\n\n
All the methods of this class are implemented by the system.  */
class Interface_ID: public MaxHeapOperators {
	ULONG a,b;
public:
	/*! \remarks Constructor. The two parts of the ID are initialized to
	<b>0xffffffff</b>. */
	Interface_ID() { a = b = 0xffffffff; }
	/*! \remarks Constructor. The two parts of the ID are initialized from
	the corresponding parts of the Interface_ID passed.
	\par Parameters:
	<b>const Interface_ID\& iid</b>\n\n
	The ID whose parts are used to initialize this ID. */
	Interface_ID(const Interface_ID& iid) { a = iid.a; b = iid.b;	}
	/*! \remarks Constructor. The two parts of the ID are initialized from
	the from the parts passed.\n\n

	\par Parameters:
	<b>ulong aa</b>\n\n
	Passed to initialize the first part of the ID.\n\n
	<b>ulong bb</b>\n\n
	Passed to initialize the second part of the ID. */
	Interface_ID(ulong aa, ulong bb) { a = aa; b = bb; }
	/*! \remarks Returns the first part of the ID. */
	ULONG PartA() { return a; }
	/*! \remarks Returns the second part of the ID. */
	ULONG PartB() { return b; }
	/*! \remarks Sets the first part of the ID.
	\par Parameters:
	<b>ulong aa</b>\n\n
	Passed to set the first part. */
	void SetPartA( ulong aa ) { a = aa; }
	/*! \remarks Sets the second part of the ID.
	\par Parameters:
	<b>ulong bb</b>\n\n
	Passed to set the second part. */
	void SetPartB( ulong bb ) { b = bb; }
	/*! \remarks Equality operator. Returns nonzero if the two parts of
	the ID are equal to the ID passed; otherwise zero.
	\par Parameters:
	<b>const Interface_ID\& iid</b>\n\n
	The ID to check. */
	int operator==(const Interface_ID& iid) const { return (a==iid.a&&b==iid.b); }
	/*! \remarks Inequality operator. Returns nonzero if either of the
	parts of the ID are NOT equal to the ID passed; otherwise zero.
	\par Parameters:
	<b>const Interface_ID\& iid</b>\n\n
	The ID to check. */
	int operator!=(const Interface_ID& iid) const { return (a!=iid.a||b!=iid.b); }
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const Interface_ID\& iid</b>\n\n
	The ID to assign from. */
	Interface_ID& operator=(const Interface_ID& iid)  { a=iid.a; b = iid.b; return (*this); }
	// less operator - allows for ordering Class_IDs (used by stl maps for example) 
	bool operator<(const Interface_ID& rhs) const
	{
		if ( a < rhs.a || ( a == rhs.a && b < rhs.b ) )
			return true;

		return false;
	}
};


// Types used by ISave, ILoad, AppSave, AppLoad

/*! \defgroup ioResults I/O Results */
//@{
enum IOResult
{
	//! The result was acceptable - no errors.
	IO_OK = 0,	
	/*!	Indicates the end of the chunks at a certain level have been reached. It 
	is used as a signal to terminates the processing of chunks at that level. 
	Returned from ILoad::OpenChunk()*/
	IO_END = 1,
	/*!	Returned when an error occurred. Note that the plug-in should not put up a
	message box if a read error occurred. It should simply return the error 
	status. This prevents a excess of messages from appearing. */
	IO_ERROR = 2
};
//@}

enum ChunkType
{
	NEW_CHUNK       = 0,
	CONTAINER_CHUNK = 1,
	DATA_CHUNK      = 2
};

enum FileIOType
{
	/*!	\brief File IO concerns a scene file (.max, .viz). 
	See \ref NOTIFY_FILE_PRE_OPEN, \ref NOTIFY_FILE_POST_OPEN */
	IOTYPE_MAX    = 0, 
	/*!	\brief File IO concerns a material library file (.mat). 
	See \ref NOTIFY_FILE_PRE_OPEN, \ref NOTIFY_FILE_POST_OPEN */
	IOTYPE_MATLIB = 1, 
	/*!	\brief File IO concerns a render preset file (.rps). 
	See \ref NOTIFY_FILE_PRE_OPEN, \ref NOTIFY_FILE_POST_OPEN */
	IOTYPE_RENDER_PRESETS = 2
}; 

//! Product Licensing Type
enum ProductVersionType
{
	PRODUCT_VERSION_DEVEL,
	PRODUCT_VERSION_TRIAL,
	PRODUCT_VERSION_ORDINARY,
	PRODUCT_VERSION_EDU, //Educational - Institution
	PRODUCT_VERSION_NFR,
	PRODUCT_VERSION_EMR //Educational - Student
};

//! Product license behavior
enum LockBehaviorType
{
	LICENSE_BEHAVIOR_PERMANENT,
	LICENSE_BEHAVIOR_EXTENDABLE,
	LICENSE_BEHAVIOR_NONEXTENDABLE,
	LICENSE_BEHAVIOR_BORROWED
};
