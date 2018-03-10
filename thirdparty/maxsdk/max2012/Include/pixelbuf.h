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
// FILE:        pixelbuf.h
// DESCRIPTION: Pixel Buffer Classes
//              These templated classes let you set up a buffer for pixels
//              that will automatically clean itself up when it goes out of 
//              scope.
// AUTHOR:      Tom Hudson
// HISTORY:     Dec. 09 1995 - Started file
//**************************************************************************/

#pragma once

#include "maxheap.h"
#include "maxheapdirect.h"
#include "maxtypes.h"

/*!
\par Description:
These templated classes allow you to set up a buffer for pixels that will
automatically deallocate the buffer when they goes out of scope.  All methods
of this class are implemented by the system.\n\n
Note the following typedefs set up for the standard pixel storage formats.\n\n
<b>typedef PixelBufT\<UBYTE\> PixelBuf8;</b>\n\n
<b>typedef PixelBufT\<USHORT\> PixelBuf16;</b>\n\n
<b>typedef PixelBufT\<BMM_Color_24\> PixelBuf24;</b>\n\n
<b>typedef PixelBufT\<BMM_Color_32\> PixelBuf32;</b>\n\n
<b>typedef PixelBufT\<BMM_Color_48\> PixelBuf48;</b>\n\n
<b>typedef PixelBufT\<BMM_Color_64\> PixelBuf64;</b>  */
template <class T> class PixelBufT: public MaxHeapOperators
{
private:
     T *buf;
     int width;
public:
     /*! \remarks Constructor.  This allocates the pixel buffer using the
     specified width.
     \par Parameters:
     <b>int width</b>\n\n
     The number of pixels to allocate for the buffer. */
     inline               PixelBufT(int width) { buf = (T *)MAX_calloc(width,sizeof(T)); this->width=width; };
     /*!   \remarks Destructor.  The pixel buffer is deallocated. */
     inline               ~PixelBufT() { if(buf) MAX_free(buf); };
     /*! \remarks Returns the address of the pixel buffer. */
     inline   T*          Ptr() { return buf; };
	 /*! \remarks Array operator.  This allows access to the pixel buffer
	 using the [ ] operator.
	 \par Parameters:
	 <b>int i</b>\n\n
	 The index to access. */
	 inline   T&          operator[](int i) { return buf[i]; }
           /*! \remarks Fills the specified portion of the pixel buffer with
           the specified color.
           \par Parameters:
           <b>int start</b>\n\n
           The start location for the fill.\n\n
           <b>int count</b>\n\n
           The number of pixels to fill.\n\n
           <b>T color</b>\n\n
           The color to use as the fill.
           \return  Nonzero if filled; otherwise 0.
           \par Operators:
           */
           int            Fill(int start, int count, T color) {
                          int ix,jx=start+count;
                          if(jx > width) // MAB - 07/15/03 - changed from >=
                             return 0;
                          for(ix=start; ix<jx; buf[ix++]=color);
                          return 1;
                          };
     };

typedef PixelBufT<UBYTE> PixelBuf8;
typedef PixelBufT<USHORT> PixelBuf16;
typedef PixelBufT<BMM_Color_24> PixelBuf24;
typedef PixelBufT<BMM_Color_32> PixelBuf32;
typedef PixelBufT<BMM_Color_48> PixelBuf48;
typedef PixelBufT<BMM_Color_64> PixelBuf64;
typedef PixelBufT<BMM_Color_fl> PixelBufFloat;

typedef PixelBufT<BMM_Color_64> PixelBuf;

