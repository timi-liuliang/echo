/**********************************************************************
 *<
	FILE: pbbitmap.h

	DESCRIPTION: BMM Bitmap parameter wrapper for the ParamBlock2 system

	CREATED BY: John Wainwright

	HISTORY: created 4/27/00

 *>	Copyright (c) Autodesk 2000, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "PB2Export.h"
#include "maxheap.h"
#include <WTypes.h>
#pragma warning(push)
#pragma warning(disable:4201)
#include <vfw.h>
#pragma warning(pop)
#include "bitmap.h"

// a wrapper for bitmap/bitmapinfo pairs for holding bitmaps in a ParamBlock
/*! \sa  Class IParamBlock2, Class Bitmap, Class BitmapInfo, Class BitmapManager.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is a Bitmap/BitmapInfo wrapper class. It is used by ParamBlock2s to store
bitmap information. The class has two public data members that hold the
BitmapInfo and the Bitmap itself.
\par Data Members:
<b>BitmapInfo bi;</b>\n\n
Stores the BitmapInfo for the bitmap.\n\n
<b>Bitmap *bm;</b>\n\n
Points to the Bitmap itself.  */
class PBBitmap: public MaxHeapOperators 
{
public:
	BitmapInfo	bi;
	Bitmap		*bm;
	/*! \remarks Implemented by the System\n\n
	Constructor. The BitmapInfo data member is initialized to the BitmapInfo
	passed. The Bitmap pointer is set to NULL. */
	PB2Export   PBBitmap(BitmapInfo	&bi);
				/*! \remarks Implemented by the System\n\n
				Constructor. The Bitmap pointer is set to NULL. */
				PBBitmap() { bm = NULL; }
	/*! \remarks Implemented by the System\n\n
	Destructor. The Bitmap, if allocated, is deallocated. */
	PB2Export  ~PBBitmap();

	/*! \remarks Implemented by the System\n\n
	The BitmapManager is used to Load the bitmap as specified by the
	BitmapInfo. */
	PB2Export void		Load();
	/*! \remarks Implemented by the System\n\n
	Makes a copy of the bitmap and returns a pointer to it. */
	PB2Export PBBitmap*	Clone();
};



