/**********************************************************************
 *<
	FILE: gport.h

	DESCRIPTION: Palette management.

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "coreexp.h"
#include "maxheap.h"
#include <WTypes.h>
#include "maxtypes.h"
#include "color.h"
#include "box2.h"


/*! \sa  Advanced Topics on
<a href="ms-its:3dsmaxsdk.chm::/pallets.html">Palettes</a>,
Class Color, Class GammaMgr, <a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF</a>.\n\n
\par Description:
A useful utility class for managing user interface colors. This class has
several purposes:\n\n
* Maintain the default 3ds Max palette for doing 256 color graphics.\n\n
* Provides a mechanism for allocating "animated color slots" in the default
palette for use in the user interface.\n\n
* Provide various functions for doing dithered graphics using the default 3ds
Max palette.\n\n
All methods of this class are implemented by the system.\n\n
The following global function is used to get a pointer that may be used to call
the methods of this class: */
class GPort: public MaxHeapOperators {
	public:
		/*! \remarks Destructor */
		virtual ~GPort() {;}

		// get the palette index associated with the ith slot
		/*! \remarks Retrieves the palette index associated with the 'i-th'
		slot.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies the slot. */
		virtual int AnimPalIndex(int i)=0;
	
		// returns a slot number if available, -1 if not:
		// typically called in WM_INITDIALOG processing for as
		// may slots as you need (total availible is 8)
		/*! \remarks Returns a slot number if available, -1 if not. Typically
		this is called in <b>WM_INITDIALOG</b> processing code for as many
		slots as you need (the total number available is 8).
		\return  A slot number if available; otherwise -1. */
		virtual int GetAnimPalSlot()=0;

		// Release an animated palete slot slot 
		// Typically called in WM_DESTROY for each slot
		// obtained with GetAnimPalSlot
		/*! \remarks Releases the specified animated palette slot. Typically
		this is called in the <b>WM_DESTROY</b> code for each slot obtained
		with <b>GetAnimPalSlot()</b>.
		\par Parameters:
		<b>int i</b>\n\n
		The palette slot to release. */
		virtual void ReleaseAnimPalSlot(int i)=0;

		// set the color associated with the ith animated slot
		/*! \remarks Sets the color associated with the 'i-th' animated slot.
		\par Parameters:
		<b>int i</b>\n\n
		The slot index.\n\n
		<b>COLORREF cr</b>\n\n
		The color to set. */
		virtual void SetAnimPalEntry(int i, COLORREF cr)=0;

		// Stuff the standard MAX palette the palette for the HDC,
		// handing back a handle to the old palette.
		/*! \remarks This method puts the standard 3ds Max palette into the
		palette for the HDC, handing back a handle to the old palette.
		\par Parameters:
		<b>HDC hdc</b>\n\n
		The device context.
		\return  The handle of the old palette. */
		virtual HPALETTE PlugPalette(HDC hdc)=0;

		// Create a brush for drawing with the ith animated palette slot color
		/*! \remarks This method creates a brush for drawing with the
		specified animated palette slot color.
		\par Parameters:
		<b>int slotNum</b>\n\n
		The animated palette slot.\n\n
		<b>COLORREF col</b>\n\n
		The color to use.
		\return  The handle of the brush created. */
		virtual HBRUSH MakeAnimBrush(int slotNum, COLORREF col )=0;

		// Update colors calls the Windows UpdateColors on the hdc.
		// Returns 1 iff it changed screen pixel values .
	 	// Call this when get WM_PALETTECHANGED Msg
		/*! \remarks This method calls the Windows API <b>UpdateColors()</b>
		on the specified device context handle <b>hdc</b>. Call this when you
		get a <b>WM_PALETTECHANGED</b> message.
		\par Parameters:
		<b>HDC hdc</b>\n\n
		The handle of the device context.
		\return  Nonzero if it changed screen pixel values; otherwise zero. */
		virtual int UpdateColors(HDC hdc)=0;

		// After several SetAnimPalEntry calls, call this to affect the
		// HDC's palette
		/*! \remarks After several calls to <b>SetAnimPalEntry()</b>, call
		this to affect the HDC's palette. This puts the palette into the
		<b>HDC</b> so it will take effect.
		\par Parameters:
		<b>HDC hdc</b>\n\n
		The handle of the device context. */
		virtual void AnimPalette(HDC hdc)=0;

		// The companion function to PlugPalette.
		/*! \remarks This method puts the previous 3ds Max palette (returned
		from <b>PlugPalette()</b>) into the palette for the HDC.
		\par Parameters:
		<b>HDC hDC</b>\n\n
		The handle of the device context.\n\n
		<b>HPALETTE hOldPal</b>\n\n
		The palette to restore. */
		virtual void RestorePalette(HDC hDC,HPALETTE hOldPal)=0;

		// Map an single row of pixels 24 bit color to indices into 
		// the current GPort palette, applying a dither pattern.
		// This routine does NOT do gamma correction.
		// inp points to an array of width RGB triples.
		// outp is an array of width bytes.  x and y are necessary to 
		// establish dither pattern alignment.
		/*! \remarks This method maps a single row of pixels in 24 bit color
		to indices into the current GPort palette, applying a dither pattern.
		This routine does NOT do gamma correction.\n\n
		Note that <b>x</b> and <b>y</b> are necessary to establish the dither
		pattern alignment. The dither pattern covers the entire map, and if you
		want to just dither part of it, you need to tell this method where you
		are within the pattern.
		\par Parameters:
		<b>UBYTE* inp</b>\n\n
		Points to an array of width BGR triples. This is a sequence of bytes
		structured as B,G,R,B,G R, etc. The first pixel is B,G,R then the next
		pixel is B,G,R, etc.\n\n
		<b>UBYTE *outp</b>\n\n
		The result - the color indices into the GPort palette. This array is
		<b>width</b> bytes in length.\n\n
		<b>int x</b>\n\n
		The x alignment position.\n\n
		<b>int y</b>\n\n
		The y alignment position.\n\n
		<b>int width</b>\n\n
		The number of items in the arrays above. */
		virtual void MapPixels(UBYTE* inp, UBYTE *outp, int x, int y, int width)=0;

		 
		// Display an array of 24bit colors in the HDC: if the current display is 8 bit
		//  it will display it (with dither) using in the GPort palette, otherwise it 
		//  will just blit to the screen. Does NOT do gamma correction.
		//   "drect" is the destination rectangle in the hdc.
		//   "map" points to an array of RGB triples, with bytesPerRow bytes on each scanline.
		//   "xsrc" and "ysrc" are the position within this source raster of the upper left
		//    corner of the rectangle to be copied..
		/*! \remarks Display an array of 24 bit colors in the HDC. If the
		current display is 8 bit it will display it (with dither) using in the
		GPort palette, otherwise it will just blit to the screen. This method
		does NOT do gamma correction.
		\par Parameters:
		<b>HDC hdc</b>\n\n
		The handle of the device context.\n\n
		<b>Rect\& drect</b>\n\n
		The destination rectangle in the hdc.\n\n
		<b>int xsrc</b>\n\n
		The X position within this source raster of the upper left corner of
		the rectangle to be copied.\n\n
		<b>int ysrc</b>\n\n
		The Y position within this source raster of the upper left corner of
		the rectangle to be copied.\n\n
		<b>UBYTE *map</b>\n\n
		Points to an array of BGR triples.\n\n
		<b>int bytesPerRow</b>\n\n
		The number of bytes per row on each scanline of <b>map</b>. */
		virtual void DisplayMap(HDC hdc, Rect& drect,int xsrc, int ysrc, UBYTE *map, int bytesPerRow)=0;
		
		// This version stretches the image (if src!=dest).
		//  "dest" is the destination rectangle in the hdc;
		//  "src" is the source rectangle in map.
		/*! \remarks This version of <b>DisplayMap()</b> stretches the image
		if the source rectangle is not the same size as the destination
		rectangle. <b>src</b> should be the size of the image.
		\par Parameters:
		<b>HDC hdc</b>\n\n
		The handle of the device context.\n\n
		<b>Rect\& dest</b>\n\n
		The destination rectangle in the <b>hdc</b>.\n\n
		<b>Rect\& src</b>\n\n
		The source rectangle in <b>map</b>.\n\n
		<b>UBYTE *map</b>\n\n
		Points to an array of RGB triples.\n\n
		<b>int bytesPerRow</b>\n\n
		The number of bytes per row on each scanline of <b>map</b>. */
		virtual void DisplayMap(HDC hdc, Rect& dest, Rect& src, UBYTE *map, int bytesPerRow)=0; 

		// DitherColorSwatch first gamma corrects Color c using the current
		// display gamma. In paletted modes, it will fill rectangle "r" with 
		// a dithered pattern  approximating Color c.  In 24 bit modes it just 
		// fills the rectange with c.
		/*! \remarks This method first gamma corrects <b>Color c</b> using the
		current display gamma. In paletted modes, it will fill rectangle
		<b>r</b> with a dithered pattern approximating <b>Color c</b>. In 24
		bit modes it just fills the rectangle with <b>Color c</b>.
		\par Parameters:
		<b>HDC hdc</b>\n\n
		The handle of the device context.\n\n
		<b>Rect\& r</b>\n\n
		The rectangle to fill.\n\n
		<b>Color c</b>\n\n
		The color to approximate (8 bit) or fill with (24 bit). */
		virtual void DitherColorSwatch(HDC hdc, Rect& r, Color c)=0;

		// This attempts to use the animated color slot indicated by "slot"
		// to paint a rectangular color swatch. 
		// If slot is -1, it will uses DitherColorSwatch.  It does gamma correction.
		/*! \remarks This method attempts to use the animated color slot
		indicated by "slot" to paint a rectangular color swatch. If slot is -1,
		it will uses <b>DitherColorSwatch()</b>. This method does handle gamma
		correction.
		\par Parameters:
		<b>HDC hdc</b>\n\n
		The handle of the device context.\n\n
		<b>DWORD col</b>\n\n
		The color to paint. The format is the same as
		<a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF</a>.\n\n
		<b>int slot</b>\n\n
		Specifies the slot to use. If -1 then <b>DitherColorSwatch()</b> is
		used.\n\n
		<b>int left</b>\n\n
		The left coordinate of the rectangular area to paint.\n\n
		<b>int top</b>\n\n
		The top coordinate of the rectangular area to paint.\n\n
		<b>int right</b>\n\n
		The right coordinate of the rectangular area to paint.\n\n
		<b>int bottom</b>\n\n
		The bottom coordinate of the rectangular area to paint. */
		virtual void PaintAnimPalSwatch(HDC hdc, DWORD col, int slot, int left, int top, int right, int bottom)=0;

		// get the current GPort palette.
		/*! \remarks Returns the current GPort palette. */
		virtual HPALETTE GetPalette()=0;
	};

// Normally this is the only one of these, and this gets you a pointer to it.
/*! \remarks There is only a single global instance of this class, and this
method returns a pointer to it. A developer may use this pointer to call the
methods of <b>GPort</b>.
\par Sample Code:
<b>int animColSlot = GetGPort()-\>GetAnimPalSlot();</b>  */
extern CoreExport GPort* GetGPort();


