/*******************************************************************
 *
 *    DESCRIPTION: PALUTIL.H
 *
 *    AUTHOR: D.Silva
 *
 *    HISTORY:    
 *
 *******************************************************************/

#pragma once
#include "maxheap.h"
#include <WTypes.h>

// forward declarations
struct BMM_Color_24;
struct BMM_Color_48;
struct BMM_Color_64;


//-- 256 color dithering-----------------------------------------------------
// For packing colors into 256 color paletted representation.
// Create one with NewColorPacker
/*! \sa  <a href="ms-its:3dsmaxsdk.chm::/pallets.html">Palettes</a>,
Class Quantizer.\n\n
\par Description:
Methods of this class are used for packing colors into a 256 color paletted
representation. Create an instance of this class using
<b>BMMNewColorPacker()</b> described below. All methods of this class are
implemented by the system. */
class ColorPacker: public MaxHeapOperators {
   public:
      /*! \remarks Destructor. */
		 virtual ~ColorPacker() {}
      /*! \remarks This method is used to enable dithering of the packed
      pixels. It defaults to the 3ds Max default.
      \par Parameters:
      <b>BOOL onoff</b>\n\n
      TRUE to enable dithering; FALSE to disable. */
      virtual void EnableDither(BOOL onoff)=0;  // default: MAX default
      /*! \remarks This method controls the propagation of error between
      lines. For static images this is best left to default to on. For animated
      images, it is better to set this to off and not propagate the error
      between lines. This defaults to ON.
      \par Parameters:
      <b>BOOL onoff</b>\n\n
      TRUE to enable error propagation between lines; FALSE to disable. */
      virtual void PropogateErrorBetweenLines(BOOL onoff)=0;  // default ON; 
      /*! \remarks Packs the specified line of pixels into the 256 color
      representation.
      \par Parameters:
      <b>BMM_Color_64* in</b>\n\n
      The line of pixels to pack.\n\n
      <b>BYTE *out</b>\n\n
      The result, the output pixels.\n\n
      <b>int w</b>\n\n
      The number of pixels in the line. */
      virtual void PackLine( BMM_Color_64* in, BYTE *out, int w)=0;
      /*! \remarks Packs the specified line of pixels into the 256 color
      representation.
      \par Parameters:
      <b>BMM_Color_48* in</b>\n\n
      The line of pixels to pack.\n\n
      <b>BYTE *out</b>\n\n
      The result, the output pixels.\n\n
      <b>int w</b>\n\n
      The number of pixels in the line. */
      virtual void PackLine( BMM_Color_48* in, BYTE *out, int w)=0;
      /*! \remarks This method is called to delete the ColorPacker when you
      are done with it. */
      virtual void DeleteThis()=0;
   };

// Get a color packer.  When done, be sure to call its DeleteThis();
/*! \remarks This is called to create an instance of a ColorPacker. When done,
be sure to call <b>ColorPacker::DeleteThis()</b>.
\par Parameters:
<b>int w</b>\n\n
The width of bitmap to be packed.\n\n
<b>BMM_Color_48 *pal</b>\n\n
The palette to use.\n\n
<b>int npal</b>\n\n
The number of entries in the palette.\n\n
<b>BYTE* remap=NULL</b>\n\n
This is a 256 byte table that maps the numbers into another number. This is
used so the palette may be rearranged in Windows order. To make palettes for
Windows, the best thing to do is to put the colors in so that colors that
Windows uses are either left alone, or they are occupied by the colors that are
least important in the image. This is because Windows will come along and alter
these colors. This is the first 10 colors and the last 10 colors.\n\n
The quantizer creates a palette from 0-239, where 0 is the most used color and
239 is the least used. The color packer uses 0-239 as well, and it operates
most efficiently when the colors are sorted as the quantizer orders them.\n\n
What can be done is to use <b>FixPaletteForWindows()</b> to rearrange the
colors for the Windows palette. This creates the <b>remap</b> table passed to
this method. Then as a last step the remap table is used to reorganize the
palette. Below is the documentation for the global function
<b>FixPaletteForWindows()</b>. Sample code that uses these APIs is in
<b>/MAXSDK/SAMPLES/IO/FLIC/FLIC.CPP</b>. */
UtilExport ColorPacker *NewColorPacker(
   int w,            // width of bitmap to be packed
   BMM_Color_48 *pal,   // palette to use
   int npal,         // number of entries in the palette
   BYTE* remap=NULL, // optional remap done at last stage.
   BOOL dither = FALSE
   );

//---------------------------------------------------------------------
// Color quantizer, for doing true-color to paletted conversion
//
/*! \sa  Structure BMM_Color_64, Structure BMM_Color_48, Structure BMM_Color_24.\n\n
\par Description:
Color quantizer, for doing true-color to paletted conversion. All methods of
this class are implemented by the system. Create a Quantizer object by
calling:\n\n
<b>Quantizer *BMMNewQuantizer();</b>\n\n
Be sure to call <b>Quantizer::DeleteThis()</b> when done.  */
class Quantizer: public MaxHeapOperators {
   public:
      /*! \remarks Destructor. */
      virtual ~Quantizer() {}
      /*! \remarks This method allocates the histogram used in doing the
      conversion.
      \return  Nonzero if the histogram was allocated; otherwise zero. */
      virtual int AllocHistogram(void)=0;
      /*! \remarks This method uses the histogram and computes the palette.
      \par Parameters:
      <b>BMM_Color_48 *pal</b>\n\n
      Storage for the palette to compute.\n\n
      <b>int palsize</b>\n\n
      The size of the palette.\n\n
      <b>BMM_Color_64 *forceCol</b>\n\n
      If there is a color that you want to make sure is available in the
      palette, you may pass it here. This is used for the background color of
      an image for example. If this is not NULL the quantizer will make up a
      palette and make sure that this color is in it. For backgrounds, this
      looks much nicer because the background won't be dithered when shown
      using the palette.
      \return  Nonzero if the palette was computed; otherwise zero. */
      virtual int Partition(BMM_Color_48 *pal, int palsize, BMM_Color_64 *forceCol)=0;
      /*! \remarks Adds the specified colors to the histogram so they are
      taken into account in the palette computations.
      \par Parameters:
      <b>BMM_Color_64 *image</b>\n\n
      The pixels to include.\n\n
      <b>int npix</b>\n\n
      The number of pixels above. */
      virtual void AddToHistogram(BMM_Color_64 *image, int npix)=0;  
      /*! \remarks Adds the specified colors to the histogram so they are
      taken into account in the palette computations.
      \par Parameters:
      <b>BMM_Color_48 *image</b>\n\n
      The pixels to include.\n\n
      <b>int npix</b>\n\n
      The number of pixels above. */
      virtual void AddToHistogram(BMM_Color_48 *image, int npix)=0;  
      /*! \remarks Adds the specified colors to the histogram so they are
      taken into account in the palette computations.
      \par Parameters:
      <b>BMM_Color_24 *image</b>\n\n
      The pixels to include.\n\n
      <b>int npix</b>\n\n
      The number of pixels above. */
      virtual void AddToHistogram(BMM_Color_24 *image, int npix)=0;  
      /*! \remarks This method is called to delete the Quantizer when you are
      done with it. */
      virtual void DeleteThis()=0;
   };

UtilExport Quantizer *NewQuantizer();

