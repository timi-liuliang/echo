/**********************************************************************
 *<
	FILE: gbuf.h : GBuffer manager.

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "coreexp.h"
#include "maxheap.h"
#include "strbasic.h"
#include "maxtypes.h"
#include "point2.h"
#include "color.h"
#include "baseinterface.h"
// forward declarations
class NameTab;

#define NUMGBCHAN 14

/*! \defgroup gBufChannelIndices G-Buffer Channel Indices
The number of bytes is shown in parenthesis for each. */
//@{
#define GB_Z       			0  	//!< (4)  Z-Buffer depth, float
#define GB_MTL_ID  			1  	//!< (1)  ID assigned to mtl via mtl editor
#define GB_NODE_ID 			2  	//!< (2)  ID assigned to node via properties
#define GB_UV       		3 	//!< (8)  UV coordinates - Point2 
#define GB_NORMAL   		4 	//!< (4)  Normal vector in view space, compressed 
#define GB_REALPIX  		5 	//!< (4)  Non-clamped colors in "RealPixel" format 
#define GB_COVERAGE 		6 	//!< (1)  Pixel coverage of the front surface
#define GB_BG 	     		7 	//!< (3)  RGB color of what's behind the front object 
#define GB_NODE_RENDER_ID 	8 	//!< (2)  System node number, valid during a render
#define GB_COLOR		 	9 	//!< (3)  Color (RGB)
#define GB_TRANSP		 	10 	//!< (3)  Transparency (RGB)
#define GB_VELOC		 	11 	//!< (8)  Velocity (Point2)
#define GB_WEIGHT		 	12 	//!< (3)  Weight of layers contribution to pixel color
#define GB_MASK			 	13 	//!< (2)  Sub pixel coverage mask
//@}

/*! \remarks Returns the number of bytes per pixel for the specified channel.
\par Parameters:
<b>int i</b>\n\n
The index of the channel. See \ref gBufChannelIndices. */
CoreExport int GBDataSize(int i);

/*! \remarks Returns the name of the specified channel.
\par Parameters:
<b>int i</b>\n\n
The index of the channel. See \ref gBufChannelIndices.
\return  The name returned for the specified index:\n\n
GB_Z (0): <b>"Z"</b>\n\n
GB_MTL_ID (1): <b>"Material Effects"</b>\n\n
GB_NODE_ID (2): <b>"Object"</b>\n\n
GB_UV (3): <b>"UV Coordinates"</b>\n\n
GB_NORMAL (4): <b>"Normal"</b>\n\n
GB_REALPIX (5): <b>"Non-Clamped Color"</b>\n\n
GB_COVERAGE (6): <b>"Coverage"</b>\n\n
GB_BG (7): <b>"Coverage Background"</b>\n\n
GB_NODE_RENDER_ID (8): <b>"Node Render ID"</b>\n\n
GB_COLOR (9): <b>"Color"</b>\n\n
GB_TRANSP10): <b>"Transparency"</b>\n\n
GB_VELOC (11): <b>"Velocity"</b>\n\n
GB_WEIGHT (12): <b>"Sub-Pixel Weight"</b>\n\n
GB_MASK (2): <b>"Sub-Pixel Coverage Mask"</b>  */
CoreExport MCHAR* GBChannelName(int i);

// Recognized channel bits 

/*! \defgroup gBufImageChannels Image (G-Buffer) Channels
These descriptions indicate the number of bits per pixel occupied by each channel.
\note The term 'fragment' indicates the portion of a triangle of a mesh that's seen by a
particular pixel being rendered. It's as if the pixel was a cookie-cutter and chopped
a visible section of the triangle off for rendering -- that cut piece is called a 
fragment.
\sa Class Gbuffer, Class ImageFilter, Class Bitmap, Class MtlBase, Class Interface,
Class INode, Structure RealPixel, Structure Color24.
*/
//@{
#define BMM_CHAN_NONE     0
/*! The size is 32 bits per pixel. This is the channel that would be used by a depth of 
field blur routine for instance. The Z value is at the center of the fragment that is
foremost in the sorted list of a-buffer fragments. The Z buffer is an array of float values
giving the Z-coordinate in camera space of the point where a ray from the camera through
the pixel center first intersects a surface. All Z values are negative, with more negative
numbers representing points that are farther from the camera. The Z buffer is initialized
with the value -1.0E30. \n\n
Note that for non-camera viewports (such as Front, User, Grid, Shape, etc.) the values may
be both positive and negative. In such cases the developer could add a large value onto all
the values to make them all positive. This is because positive versus negative doesn't
really mean anything. It is just the distance between values that matters. \n\n
As noted above, the Z values in the A buffer are in camera space. The projection for a
point in camera space to a point in screen space is:
\code
Point2 RenderInfo::MapCamToScreen(Point3 p) {
  return (projType==ProjPerspective)?
  Point2(xc + kx*p.x/p.z, yc + ky*p.y/p.z):
  Point2(xc + kx*p.x, yc + ky*p.y);
}
\endcode
This function is supplied by the RenderInfo data structure which can be obtained from the bitmap output by the renderer using the function Bitmap::GetRenderInfo(). Note that this outputs a Point2. There is no projection for Z. As noted before, the Z buffer just uses the camera space Z.
\code
float *zbuffer = (float *)GetChannel(BMM_CHAN_Z,type);
\endcode
*/
#define BMM_CHAN_Z        (1<<GB_Z) 		//!<  Z-buffer depth, float
/*! The size is 8 bits per pixel.
This channel is currently settable to a value between 0 and 8 by the 'Material Effects
Channel' flyoff in the Material Editor. A plug-in material can generated up to 255
different material ID's (since this is an 8-bit quantity). This channel would be used to
apply an effect (i.e., a glow) to a specific material.
\code
BYTE *bbuffer = (BYTE *)GetChannel(BMM_CHAN_MTL_ID,type);
\endcode
*/
#define BMM_CHAN_MTL_ID   (1<<GB_MTL_ID) 	//!<  ID assigned to the material via the Material Editor. 
/*! The size is 16 bits per pixel. This channel would be used to perform an effect (for
example a flare) on a specific node.
\code
WORD *wbuffer = (WORD *)GetChannel(BMM_CHAN_NODE_ID,type);
\endcode
*/
#define BMM_CHAN_NODE_ID  (1<<GB_NODE_ID) 	//!<  ID assigned to node using the Object Properties / G-buffer ID spinner.
/*! The size is 64 bits per pixel. If you have UV Coordinates on your object this channel
provides access to them. This channel could be used by 3D paint programs or image
processing routines to affect objects based on their UVs. The UV coordinate is stored as a
Point2, using Point2::x for u and Point2::y for v. The UV coordinates are values prior to
applying the offset, tiling, and rotation associated with specific texture maps.
\code
Point2 *pbuffer = (Point2 *)GetChannel(BMM_CHAN_UV,type);
\endcode
*/
#define BMM_CHAN_UV       (1<<GB_UV) 		//!<  UV coordinates, stored as Point2.
/*! The size is 32 bits per pixel. Object normals are available for image processing
routines that take advantage of the normal vectors to do effects based on curvature (for
example), as well as for 3D paint programs. The normal value is at the center of the
fragment that is foremost in the sorted list of a-buffer fragments.
\code
DWORD *dbuffer = (DWORD *)GetChannel(BMM_CHAN_NORMAL,type);
\endcode
\note The following function is available to decompress this value to a standard
normalized Point3 value (DWORD and ULONG are both 32 bit quantities):
\code
Point3 DeCompressNormal(ULONG n);
\endcode
The decompressed vector has absolute error < 0.001 in each component.
*/
#define BMM_CHAN_NORMAL   (1<<GB_NORMAL) 	//!<  Normal vector in view space, compressed
/*! The size is 32 bits per pixel. See Structure RealPixel. These are 'real' colors that
are available for physically-correct image processing routines to provide optical effects
that duplicate the way the retina works.
\code
RealPixel *rbuffer = (RealPixel *)GetChannel(BMM_CHAN_REALPIX,type);
\endcode
*/
#define BMM_CHAN_REALPIX  (1<<GB_REALPIX) 	//!<  Non clamped colors in "RealPixel" format
/*! This provides an 8-bit value (0..255) that gives the coverage of the surface fragment
from which the other G-buffer values are obtained. This channel is being written and read
with RLA files, and shows up in the Virtual Frame Buffer. This may be used to make the
antialiasing in 2.5D plug-ins such as Depth Of Field filters much better.
\code
UBYTE *gbufCov = (UBYTE*)GetChannel(BMM_CHAN_COVERAGE,type);
\endcode
*/
#define BMM_CHAN_COVERAGE (1<<GB_COVERAGE) 	//!<  Pixel coverage of front surface
/*! The size is 24 bits per pixel. If you have the image color at a pixel, and the Z
coverage at the pixel, then when the Z coverage is < 255, this channel tells you the color
of the object that was partially obscured by the foreground object. For example, this info
will let you determine what the "real" color of the foreground object was before it was
blended (antialiased) into the background.
\code
Color24 *bgbuffer = (Color24 *)GetChannel(BMM_CHAN_BG,type);
\endcode
*/
#define BMM_CHAN_BG 	  (1<<GB_BG) 		//!<  RGB color of what's behind front object
/*! The size is 16 bits per pixel. The renderer will set the RenderID of all rendered
nodes, and will set all non-rendered nodes to 0xffff. Video Post plug-ins can use the
Interface::GetINodeFromRenderID() method to get a node pointer from an ID in this channel.
Note that this channel is NOT saved with RLA files, because the IDs would not be meaningful
unless the scene was the one rendered.
\code
UWORD *renderID = (UWORD *)GetChannel(BMM_CHAN_NODE_RENDER_ID,type);
INode *node = ip->GetINodeFromRenderID(*renderID);
\endcode
*/
#define BMM_CHAN_NODE_RENDER_ID (1<<GB_NODE_RENDER_ID) //!<  System node number - valid during a render.
/*! It is a 24 bit RGB color (3 bytes per pixel).
\code
Color24 *c1 = (Color24 *)GetChannel(BMM_CHAN_COLOR,type);
\endcode
*/
#define BMM_CHAN_COLOR    (1<<GB_COLOR) 	//!<  Color returned by the material shader for the fragment.
/*! It is a 24 bit RGB color (3 bytes per pixel).
\code
Color24 *transp = (Color24 *)GetChannel(BMM_CHAN_TRANSP,type);
\endcode
*/
#define BMM_CHAN_TRANSP   (1<<GB_TRANSP) 	//!<  Transparency returned by the material shader for the fragment.
/*! It is a Point 2 (8 bytes per pixel).
\code
Point2 *src = (Point2 *)GetChannel(BMM_CHAN_VELOC,type);
\endcode
*/
#define BMM_CHAN_VELOC    (1<<GB_VELOC) 	//!<  Gives the velocity vector of the fragment relative to the screen, in screen coordinates.
/*! It is a 24 bit RGB color (3 bytes per pixel). It is the fraction of the total pixel
color contributed by the fragment. The sum of (color *weight) for all the fragments should
give the final pixel color. The weight ( which is an RGB triple) for a given fragment
takes into account the coverage of the fragment and the transparency of any fragments
which are in front of the given fragment. \n\n
If c1, c2, c3.. etc are the fragment colors, and w2, w2, w3... etc are the fragment
weights, then:
\code
pixel color = c1*w1 + c2*w2 +c3*w3 + ... + cN*wN;
\endcode
The purpose of the sub-pixel weight is to allow post processes to weight the conribution
of a post-effect from a particular fragment. It may also be necessary to multiply by the
fragment's own transparency, which is not included in its weight. Note that for fragments
that have no transparent fragments in front of them, the weight will be equal to the
coverage.
\code
Color24 *w1 = (Color24 *)GetChannel(BMM_CHAN_WEIGHT,type);
\endcode
*/
#define BMM_CHAN_WEIGHT   (1<<GB_WEIGHT) 	//!<  The sub-pixel weight of a fragment.
/*! The 4x4 (16 bits = 1 word) pixel coverage mask. */
#define BMM_CHAN_MASK	  (1<<GB_MASK)   	//!<  Subpixel mask 
//@}

/*! \defgroup gBufChannelTypes G-Buffer Channel Types
These are the recognized types of G-Buffer channels. The types are defined by the number
of bits per pixel for the channel.
\sa \ref gBufImageChannels, \ref gBufChannelIndices */
//@{
#define BMM_CHAN_TYPE_UNKNOWN 0 
#define BMM_CHAN_TYPE_8   2 //!< 1 byte per pixel
#define BMM_CHAN_TYPE_16  3 //!< 1 word per pixel
#define BMM_CHAN_TYPE_24  8 //!< 3 bytes per pixel
#define BMM_CHAN_TYPE_32  4 //!< 2 words per pixel
#define BMM_CHAN_TYPE_48  5 //!< 3 words per pixel
#define BMM_CHAN_TYPE_64  6 //!< 4 words per pixel
#define BMM_CHAN_TYPE_96  7 //!< 6 words per pixel
//@}

/*! \sa Class GBufReader, Class GBufWriter, Class GBuffer, Structure RealPixel, Structure Color24, Class Point2, \ref gBufImageChannels, \ref gBufChannelTypes.
\remarks This structure is used by the GBufReader and GBufWriter code to hold the G-Buffer data.
*/
struct GBufData: public MaxHeapOperators {
	/*! The floating point depth value at the center of the fragment that is foremost in the sorted list of a-buffer fragments.  */
	float z;
	/*! The ID assigned to the material via the Material Editor.  */
	UBYTE mtl_id;
	/*! This is the ID assigned to node via the Object Properties / G-buffer ID spinner.  */
	UWORD node_id;
	/*! UV coordinates, stored as a Point2.  */
	Point2 uv;
	/*! Normal vector in view space, compressed.  */
	DWORD normal;
	/*! Non clamped colors in "RealPixel" format.  */
	RealPixel realpix;
	/*! Pixel coverage of the front surface.  */
	UBYTE coverage;
	/*! The renderer will set this ID for all rendered nodes, and will set all non-rendered nodes to 0xffff.  */
	UWORD rend_id;
	/*! This is color returned by the material shader for the fragment.  */
	Color24 color;
	/*! This is transparency returned by the material shader for the fragment.  */
	Color24 transp;
	/*! This is the sub-pixel weight of a fragment.  */
	Color24 weight;
	/*! This gives the velocity vector of the fragment relative to the screen, in screen coordinates.  */
	Point2 veloc;
	UWORD mask;
	};
	

//------------------------------------gbuf.h ----------------------------------------------

/*! \sa  Class GBuffer, Class GBufWriter, Structure GBufData, \ref gBufChannelIndices.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the object returned by <b>GBuffer::CreateReader()</b>. Methods of this
class allow the G-Buffer data to be read.\n\n
Here is an example of reading multiple layer data from the G-Buffer using
methods of this class.\n\n
\code
void ReadExample(GBuffer *gb)
{
	float zr, UBYTE midr;
	GBufReader *rdr = gb->CreateReader();
	for (int y=0; y<10; y++) {
		rdr->StartLine(y);
		for (int x=5; x<100; x+=4) {
			int res = rdr->StartPixel(x);
			rdr->ReadChannelData(GB_Z,(void *)&zr);
			rdr->ReadChannelData(GB_MTL_ID,(void *)&midr);
			while (rdr->StartNextLayer()) {
				rdr->ReadChannelData(GB_Z,(void *)&zr);
				rdr->ReadChannelData(GB_MTL_ID,(void *)&midr);
			}
		}
	}
	gb->DestroyReader(rdr);
}
\endcode
All methods of this class are implemented by the System.  */
class GBufReader  : public InterfaceServer {
	public:
	/*! \remarks Call this method to start a new scan line. Call this method
	before the first scan line.
	\par Parameters:
	<b>int y</b>\n\n
	The zero based index of the scan line to start.
	\return  Returns -1 if there was no data for line, or the x value of first
	non-empty pixel. */
	virtual int  StartLine(int y)=0;      	// -1 = no data for line, or x of first non-empty pixel
	/*! \remarks Call this method to start a new pixel. This method
	automatically starts the first layer.
	\par Parameters:
	<b>int x</b>\n\n
	The zero based index of the pixel to start.
	\return  Returns TRUE. */
	virtual BOOL StartPixel(int x)=0;     	// -1 = eol,; 0 = empty; 1= has data  ( Automatically starts first layer)
	/*! \remarks This method is called to start a new line and pixel. This
	method is equivalent to:\n\n
	<b> StartLine(y);</b>\n\n
	<b> return StartPixel(x);</b> */
	virtual BOOL StartPixel(int x,int y)=0; // -1 = eol,; 0 = empty; 1= has data ( includes StartLine)
	/*! \remarks This method is called to begin reading data from a new layer.
	Do <b>not</b> call this method before reading the first layer.
	\return  TRUE if more data to read; otherwise FALSE. */
	virtual BOOL StartNextLayer()=0;		// 0 = no more layers ( Do not call before first layer )
	/*! \remarks Call this method to prepare for reading the next pixel.
	\return  Returns TRUE. */
	virtual	int NextPixel()=0;       		// -1 = eol,; 0 = empty; 1 = has data
	// Read a data element: chan is one of { GB_Z, GB_MTL_ID, GB_NODE_ID, ... etc }
	/*! \remarks Reads a data element from the specified channel of the
	G-Buffer from the current scan line and pixel.
	\par Parameters:
	The channel to read. One of the items from: \ref gBufChannelIndices.\n\n
	<b>void *data</b>\n\n
	Points to storage for the data.
	\return  TRUE if data was available; otherwise FALSE. */
	virtual BOOL ReadChannelData(int chan, void *data)=0; // 1= has data;  0= didnt have data
	/*! \remarks Reads all the data from the G-Buffer into the <b>GBufData</b>
	structure passed from the current scan line and pixel.
	\par Parameters:
	<b>GBufData *data</b>\n\n
	Points to storage for the data. See
	Structure GBufData.
	\return  TRUE if data was available; otherwise FALSE. */
	virtual BOOL ReadAllData(GBufData *data)=0; // 1= has data;  0= didnt have data
	/*! \remarks	This method allows values in the specified layer to be written. Note that
	it may seem strange, writing data from the reader, but developers asked for
	the capability of writing to the already created gbuffer, and it is much
	simpler to add this capability to the GBufReader than to GBufWriter, which
	is designed to construct gbuffers from scratch, not modify existing ones.
	\par Parameters:
	<b>int chan</b>\n\n
	Specifies the channel to write to.\n\n
	<b>void *data</b>\n\n
	Points to the data to write.
	\return  TRUE indicates success; FALSE indicates failure. */
	virtual BOOL ModifyChannelData(int chan, void *data)=0;  // 1=success   0=fail
	/*! \remarks	This method allows values in the current layer to be written. Note that it
	may seem strange, writing data from the reader, but developers asked for
	the capability of writing to the already created gbuffer, and it is much
	simpler to add this capability to the GBufReader than to GBufWriter, which
	is designed to construct gbuffers from scratch, not modify existing ones.
	\par Parameters:
	<b>GBufData *data</b>\n\n
	Points to the data to write.
	\return  TRUE indicates success; FALSE indicates failure. */
	virtual BOOL ModifyAllData(GBufData *data)=0; // 1= success;  0= fail
	/*! \remarks Deletes this reader object. Call this method when finished.
	*/
	virtual void DeleteThis()=0;
	// Generic expansion function
	/*! \remarks	This is a general purpose function that allows the API to be extended in the
	future. The 3ds Max development team can assign new <b>cmd</b> numbers and
	continue to add functionality to this class without having to 'break' the
	API.\n\n
	This is reserved for future use.
	\par Parameters:
	<b>int cmd</b>\n\n
	The command to execute.\n\n
	<b>ULONG arg1=0</b>\n\n
	Optional argument 1 (defined uniquely for each <b>cmd</b>).\n\n
	<b>ULONG arg2=0</b>\n\n
	Optional argument 2.\n\n
	<b>ULONG arg3=0</b>\n\n
	Optional argument 3.
	\return  An integer return value (defined uniquely for each <b>cmd</b>).
	\par Default Implementation:
	<b>{ return 0; }</b> */
	#pragma warning(push)
	#pragma warning(disable:4100)
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 
	#pragma warning(pop)
	};	


// This assumes pixels are created in increasing order of x.
/*! \sa  Class GBuffer, Class GBufReader, Structure GBufData, \ref gBufChannelIndices.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the writer object returned from <b>GBuffer::CreateWriter()</b>. This
class assumes pixels are created in increasing order of x.\n\n
Here is an example of writing multiple layer data to the G-Buffer using methods
of this class.\n\n
\code
GBuffer *gb = NewDefaultGBuffer();
gb->SetRasterSize(100,10);
gb->CreateChannels((1<<GB_Z)|(1<<GB_MTL_ID)|(1<<GB_BG));
gb->InitBuffer();

GBufWriter *wrt = gb->CreateWriter();
for (int y=0; y<10; y++)
{
	wrt->StartLine(y);
	BOOL bb = 0;
	for (int x=5; x<100; x+=4) {
		wrt->StartPixel(x);
		wrt->StartNextLayer();
		float z = 5.0f*float(x)*float(y);
		wrt->WriteChannelData(GB_Z,(void *)&z);
		UBYTE mid = 36+x;
		wrt->WriteChannelData(GB_MTL_ID,(void *)&mid);
		Color24 c;
		c.r = 10+x; c.g = 20+x; c.b = 30+x;
		wrt->WriteChannelData(GB_BG,(void *)&c);

		wrt->StartNextLayer();
		z = 15.0f*float(x)*float(y);
		wrt->WriteChannelData(GB_Z,(void *)&z);
		mid = 26+x;
		wrt->WriteChannelData(GB_MTL_ID,(void *)&mid);
		c.r =30+x; c.g = 20+x; c.b = 10+x;
		wrt->WriteChannelData(GB_BG,(void *)&c);

		if (bb) {
			wrt->StartNextLayer();
			z = 17.0f*float(x)*float(y);
			wrt->WriteChannelData(GB_Z,(void *)&z);
			mid = 64+x;
			wrt->WriteChannelData(GB_MTL_ID,(void
				*)&mid);
			c.r = 130+x; c.g = 120+x; c.b = 110+x;
			wrt->WriteChannelData(GB_BG,(void *)&c);
		}
		bb = !bb;
	}
	wrt->EndLine();
}
gb->DestroyWriter(wrt);
\endcode 
All methods of this class are implemented by the System.  */
class GBufWriter : public InterfaceServer {
	public:
	/*! \remarks This method should be called before writing every line.
	\par Parameters:
	<b>int y</b>\n\n
	The zero based index of the scan line to start. */
	virtual void StartLine(int y)=0;		 // 1 = success  0 = fail ( Must call before every line)
	/*! \remarks This method must be called before writing each pixel and must
	be called with increasing x values on a line.
	\par Parameters:
	<b>int x</b>\n\n
	The zero based index of the pixel to start. */
	virtual void StartPixel(int x)=0;        // 1 = success  0 = fail  ( Must call to start each pixel )
	/*! \remarks This method must be called before writing the first layer. */
	virtual void StartNextLayer()=0;		 // 1 = success  0 = fail  ( Must call before first layer)
	// Write a data element: chan is one of { GB_Z, GB_MTL_ID, GB_NODE_ID, ... etc }
	/*! \remarks Call this method to write a data element to the specified
	channel of the G-Buffer to the current scan line and pixel.
	\par Parameters:
	<b>int chan</b>\n\n
	See \ref gBufChannelIndices.\n\n
	<b>void *data</b>\n\n
	Points to the G-Buffer data to write.
	\return  TRUE on success; FALSE on failure. */
	virtual BOOL WriteChannelData(int chan, void *data)=0;  // 1=success   0=fail
	/*! \remarks This method writes all the channel data from the GBufData
	structure passed to the current scan line and pixel.
	\par Parameters:
	<b>GBufData *data</b>\n\n
	Points to the G-Buffer data to write. See
	Structure GBufData.
	\return  TRUE on success; FALSE on failure. */
	virtual BOOL WriteAllData(GBufData *data)=0; // 1= success;  0= fail
	/*! \remarks This method should be called after writing each line.
	\return  TRUE on success; FALSE on failure. */
	virtual BOOL EndLine()=0;                 // 1=success   0=fail ( Just call after every line)
	/*! \remarks Deletes this writer object. Call this method when finished
	using it. */
	virtual void DeleteThis()=0;
	// Generic expansion function
	#pragma warning(push)
	#pragma warning(disable:4100)
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 
	#pragma warning(pop)
	};	

/*! \sa  Class GBufReader, Class GBufWriter, \ref gBufImageChannels,
\ref gBufChannelTypes, 
Class Bitmap.\n\n
class GBuffer : public InterfaceServer\n\n
<a href="#max8Methods">New Methods in Max8</a>\n\n
<a href="#globalMethods">Global Methods</a>\n\n
<a href="#memberMethods">Member Functions</a>
\par Description:
This class is available in release 3.0 and later only.\n\n
This class is used by Bitmaps to implement an enhanced G-Buffer (providing more
functionality than the 2.x G-Buffer). The new G-Buffer stores multiple layers
at each pixel. The Virtual Frame Buffer includes a new spinner to the right of
the "channels" drop down which lets you look at the different layers. (It only
appears when there are G-Buffer channels present).\n\n
The multiple layers of the G-Buffer allow rendering effects to do better
antialiasing and handling of transparency. The frontmost layer is still stored
in the G-Buffer as full, screen-sized arrays, one for each channel. The
subsequent layers are stored in a variable number of "layer records" all of
which are the same size (which depends on which channels have been requested
via the <b>GBuffer::CreateChannels()</b> function). Even scenes with no
transparency can have more than one layer at a given pixel. This can occur
along the edges of an object, where the object partially covers some pixels,
and another object (or objects) is visible in the remaining part of the pixel.
Each visible face in the pixel will be represented by a fragment. A given
object will not always appear on the same layer. If it is the only object in a
pixel it will be in the frontmost layer, but where it is partially occluded it
will lie in a layer behind the occluding object.\n\n
Another way multiple layers can occur (aside from transparency) is when objects
are given the "Render Occluded Objects" property in the object properties
dialog. When an object has this property, it acts as if it were transparent,
and objects behind it, though invisible, are rendered and stored in the
G-Buffer.\n\n
Another G-Buffer layer is added to contain the background color in its color
channel for any pixel which is not empty and on which the background gets
evaluated. This means the background layer will be present behind transparent
objects or objects with "render occluded" turned on. It will also be present
along the edges of objects where the background is partially visible in the
pixel. Note: This does not depend at all on having the <b>GB_BG</b> flag set.
It is necessary to have the <b>GB_COLOR</b> channel, however.\n\n
All methods of this class are implemented by the System.\n\n
<b>Note on RPF files</b>: The following information relates to the layer
storage scheme used in the 3ds Max RPF files.\n\n
The layer information for each scan line is stored as a series of run-encoded
channels, each containing the same number of data elements, one per layer
record. The first channel is a "psuedo-channel" that contains the x value for
each layer record. If there are multiple layers for a given x, this will be
reflected as several identical x values in a row.\n\n
For instance, if the x channel contained\n\n
5,6,7,7,8,8,8,9,10\n\n
And the Z channel contained\n\n
100, 100, 100,200, 100,200,300, 100, 100\n\n
This would mean at pixel 7 there are 2 layers, and pixel 8 there are 3
layers.\n\n
Pixel depths\n\n
5 100\n\n
6 100\n\n
7 100, 200\n\n
8 100, 200, 300\n\n
9 100\n\n
10 100   \n\n
As an extra note regarding RPF files, a new data member was added to
class RenderInfo, which is saved with the
RPF (and RLA) files. In order to avoid getting out of sync with previous file
format implementations a "version" word preceding the RenderInfo record has
been added into the format. The version is set to a value that will distinguish
it from the first word of the RenderInfo data, enabling it to determine the
version. If we make further additions to RenderInfo in the future, the version
will allow us to keep it straight. To load both older and newer RPF and RLA
files correctly, the code that loads the RenderInfo been modified, and
therefore any plugins and 3rd party code that read these files need to be
changed. Here's my the code:\n\n
<b><b>#define RENDINFO_VERS1 1000</b></b>\n\n
<b><b>int RLAReader::ReadRendInfo(RenderInfo *ri) {</b></b>\n\n
<b><b>short vers;</b></b>\n\n
<b><b>if (fread(\&vers,sizeof(short),1,fd)!=1) return 0;</b></b>\n\n
<b><b>int size = sizeof(RenderInfo);</b></b>\n\n
<b><b>if (vers != RENDINFO_VERS1) {</b></b>\n\n
<b><b>// the old version didn't start with a version word, but</b></b>\n\n
<b><b>// with projType which is 0 or 1.</b></b>\n\n
<b><b>size -= sizeof(Rect); // The old record didn't have the region
Rect.</b></b>\n\n
<b><b>fseek(fd,-sizeof(short),SEEK_CUR); // Undo the version read</b></b>\n\n
<b><b>}</b></b>\n\n
<b><b>if (ri) {</b></b>\n\n
<b><b>if (fread(ri,size,1,fd)!=1) return 0;</b></b>\n\n
<b><b>}</b></b>\n\n
<b><b>else</b></b>\n\n
<b><b>fseek(fd, size, SEEK_CUR);</b></b>\n\n
<b><b>return 1;</b></b>\n\n
<b><b>}</b></b> */
class GBuffer : public InterfaceServer {
	public:
	/*! \remarks This method is used internally to set the size of the
	G-Buffer.
	\par Parameters:
	<b>int ww</b>\n\n
	The width to set in pixels.\n\n
	<b>int hh</b>\n\n
	The height to set in pixels. */
	virtual void SetRasterSize(int ww, int hh)=0;  	  // when create bitmap or size changes.
	/*! \remarks Returns the width of the GBuffer. */
	virtual int Width()=0;
	/*! \remarks Returns the height of the GBuffer. */
	virtual int Height()=0;
	/*! \remarks Initializes the GBuffer. Call this method before writing to
	the buffer. If present, the GB_Z channels is set to -BIGFLOAT and the
	GB_NODE_RENDER_ID channel is set to 0xffff. The Render ID Name Table is
	cleared.
	\return  Nonzero on success; otherwise zero.
	\par Sample Code:
	\code
	GBuffer *gbuf = tobm->GetGBuffer();
	if (gbuf) gbuf->InitBuffer();
	\endcode   */
	virtual int InitBuffer()=0;    					   // call before writing buffer.
	/*! \remarks This method creates the specified channels in this G-Buffer.
	\par Parameters:
	<b>ULONG channelMask</b>\n\n
	Specifies the channels to create. See \ref gBufImageChannels.
	\return  The channels that are currently present. */
	virtual ULONG CreateChannels(ULONG channelMask)=0; // create specified channels
	/*! \remarks This method delete specified channels.
	\par Parameters:
	<b>ULONG channelMask</b>\n\n
	Specifies the channels to create. See \ref gBufImageChannels.
	\return  The channels that are currently present. */
	virtual void  DeleteChannels(ULONG channelMask)=0; // delete specified channels 
	/*! \remarks Returns the channels that are currently present. See \ref gBufImageChannels. */
	virtual ULONG ChannelsPresent()=0;
	/*! \remarks Returns a pointer to the specified channel of the G-Buffer,
	and determines its type in terms of bits per pixel. NULL is returned if the
	channel can't be found.
	\par Parameters:
	<b>ULONG channelID</b>\n\n
	The channel to get. See \ref gBufImageChannels.\n\n
	<b>ULONG\& chanType</b>\n\n
	The channel type. See \ref gBufChannelTypes. */
	virtual void  *GetChannel( ULONG channelID, ULONG& chanType)=0; 
	/*! \remarks Creates and returns a pointer to a <b>GBufReader</b> object.
	*/
	virtual GBufReader *CreateReader()=0;
	/*! \remarks Deletes the specified <b>GBufReader</b> object.
	\par Parameters:
	<b>GBufReader *pRdr</b>\n\n
	Points to the reader to delete. */
	virtual void DestroyReader(GBufReader *pRdr)=0;
	/*! \remarks Creates and returns a pointer to a <b>GBufWriter</b> object.
	*/
	virtual GBufWriter *CreateWriter()=0;
	/*! \remarks Deletes the specified <b>GBufWriter</b> object.
	\par Parameters:
	<b>GBufWriter *pRdr</b>\n\n
	Points to the writer to delete. */
	virtual void DestroyWriter(GBufWriter *pRdr)=0;
	/*! \remarks Returns TRUE if this is a default G-Buffer; otherwise FALSE.
	A 'default' G-Buffer is one that 3ds Max itself creates. Since GBuffer is a
	virtual class a plug-in developer could create their own G-Buffers (for
	some very special purpose). In that case it would not be a 'default'
	G-Buffer and this method would return FALSE. */
	virtual BOOL IsDefaultGBuffer() { return FALSE; }
	/*! \remarks Deletes this G-Buffer. */
	virtual void DeleteThis()=0;
	/*! \remarks Copies the data from the specified G-Buffer to this one. This
	method maintains the multi-layer data correctly. The channels present in
	each G-Buffer must match.
	\par Parameters:
	<b>GBuffer *gbfrom</b>\n\n
	Points to the source of the copy. */
	virtual void Copy(GBuffer *gbfrom)=0;
	/*! \remarks Copies the data from the specified G-Buffer and optionally
	scales its size while maintaining the multi-layer data correctly. This is
	used in Video Post whenever there is an input bitmap that is a different
	size than the current Video Post image dimensions.
	\par Parameters:
	<b>GBuffer *gbfrom</b>\n\n
	Points to the source of the copy.\n\n
	<b>int cw=-1</b>\n\n
	The width for the copy. If -1 is specified the copy is the same size as the
	original.\n\n
	<b>int ch=-1</b>\n\n
	The height for the copy. If -1 is specified the copy is the same size as
	the original. */
	virtual	void CopyScale(GBuffer *gbfrom, int cw=-1, int ch=-1)=0;
	/*! \remarks This method is used internally and is something that normally
	a developer shouldn't call directly. It is used inside the Bitmap routines
	for manipulating bitmaps. It takes a sub rectangle of the G-Buffer which
	has an upper left corner at (srcx,srcy), and dimensions of (trgw,trgh), and
	moves it to be positioned at (trgx,trgy). Portions of the G-Buffer outside
	of this rectangle are discarded.
	\par Parameters:
	<b>int srcy</b>\n\n
	The source y location.\n\n
	<b>int trgy</b>\n\n
	The target y location.\n\n
	<b>int srcx</b>\n\n
	The source x location.\n\n
	<b>int trgx</b>\n\n
	The target x location.\n\n
	<b>int trgw</b>\n\n
	The target width.\n\n
	<b>int trgh</b>\n\n
	The target height. */
	virtual void Position(int srcy, int trgy, int srcx, int trgx, int trgw, int trgh )=0;

	// for file writing
	/*! \remarks Returns the total number of layer records for the specified
	scan line. On each scan line there is a single array of layer records. Each
	layer record contains an x value which tells at what pixel it is located,
	and then all of the G-Buffer data that is being stored in this particular
	G-Buffer. The layer records for a given pixel all have the same x value,
	and are ordered front-to-back in scene depth.\n\n
	This method is used by the RLA and RPF writer code. See
	<b>/MAXSDK/SAMPLES/IO/RLA/RLA.CPP</b> for an example.
	\par Parameters:
	<b>int y</b>\n\n
	The zero based index of the scan line. */
	virtual int  NumberLayerRecords(int y)=0;  
	/*! \remarks This method goes through all the layer records and pulls out
	the values for a particular channel into an array, which is handy for
	run-encoding when saving to a file.
	\par Parameters:
	<b>int y</b>\n\n
	The zero based index of the scan line.\n\n
	<b>int ichan</b>\n\n
	The channel to check. See \ref gBufChannelIndices. When <b>ichan=-1</b>, it gives an array of the x
	values from each of the layer records.\n\n
	<b>char *data</b>\n\n
	Points to storage for the layer data.
	\return  TRUE on success; otherwise FALSE. */
	virtual int  GetLayerChannel(int y, int ichan, char *data)=0; // ichan = -1 creates array of x values

	// for file reading
	/*! \remarks This method creates the specifed number of layer records for
	the scan line passed.
	\par Parameters:
	<b>int y</b>\n\n
	The zero based index of the scan line.\n\n
	<b>int num</b>\n\n
	The number of records to create.
	\return  TRUE on success; otherwise FALSE. */
	virtual int CreateLayerRecords(int y, int num)=0;  
	/*! \remarks Sets the layer record data for the specified channel for the
	scan line passed.
	\par Parameters:
	<b>int y</b>\n\n
	The zero based index of the scan line.\n\n
	<b>int ichan</b>\n\n
	The channel to set. See \ref gBufChannelIndices. When <b>ichan=-1</b>, it sets an array of the x values
	from each of the layer records.\n\n
	<b>char *data</b>\n\n
	Points to storage for the layer data.
	\return  TRUE on success; otherwise FALSE. */
	virtual int SetLayerChannel(int y, int ichan, char *data)=0; // ichan = -1 gets array of x values
	
	// This scans the entire image and updates the minimum and maximum values
	/*! \remarks This method searches the G-Buffer Z, UV and VELOC channels
	and stores the minimum and maximum values. This enables the
	<b>GetChannelMin()</b> and <b>GetChannelMax()</b> methods to work. This is
	called internally whey 3ds Max generates a G-Buffer. When developers get
	one from the renderer this method will already have been called. */
	virtual void UpdateChannelMinMax()=0;

	// Get Channel limits  ( currently only valid for z,uv,veloc)
	/*! \remarks Retrieves the minimum value in the specified channel.
	\par Parameters:
	<b>int chan</b>\n\n
	The channel to check. One of the following (the other channels are not
	supported):\n\n
	<b>GB_Z</b>\n\n
	Z-Buffer depth - <b>float</b>\n\n
	<b>GB_UV</b>\n\n
	UV coordinates - <b>Point2</b>\n\n
	<b>GB_VELOC</b>\n\n
	Velocity - <b>Point2</b>\n\n
	<b>void *data</b>\n\n
	Points to storage for the value to get.
	\return  TRUE if <b>chan</b> was valid and the value could be stored in
	<b>data</b>; otherwise FALSE. */
	virtual BOOL GetChannelMin(int chan, void *data)=0;
	/*! \remarks Retrieves the maximum value in the specified channel.
	\par Parameters:
	<b>int chan</b>\n\n
	The channel to check. One of the following (the other channels are not
	supported):\n\n
	<b>GB_Z</b>\n\n
	Z-Buffer depth - <b>float</b>\n\n
	<b>GB_UV</b>\n\n
	UV coordinates - <b>Point2</b>\n\n
	<b>GB_VELOC</b>\n\n
	Velocity - <b>Point2</b>\n\n
	<b>void *data</b>\n\n
	Points to storage for the value to get.
	\return  TRUE if <b>chan</b> was valid and the value could be stored in
	<b>data</b>; otherwise FALSE. */
	virtual BOOL GetChannelMax(int chan, void *data)=0;
	
	// names indexed by NodeRenderId
	/*! \remarks This method returns a reference to the table of node names
	indexed by their <b>NodeRenderId</b>. See
	Class NameTab. */
	virtual NameTab& NodeRenderIDNameTab()=0;

	// Array of Image blur multipliers indexed by NodeRenderID
	/*! \remarks	This method returns a reference to a table of floats. These are image blur
	multipliers indexed by NodeRenderID. */
	virtual Tab<float>& ImageBlurMultiplierTab()=0;

	// Generic expansion function
	// JH 11/17/00 Now deriving from InterfaceServer. Extesibility via GetInterface()
	//virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 


	// Gives the color corresponding to the id in input
	// This function represent the color-id mapping that is done for :
	// The Material Id render element
	// The Object Id render element
	// The Material Effects and Object channels of the rpf format
	// id - the material or object id for which the color is wanted
	// cOut - output color of BMM_Color_24 type
	/*! \remarks Gives the color corresponding to the id in the input. This
	function represents the color-id mapping that is done for <ul> <li>The
	Material Id render element</li> <li>The Object Id render element</li>
	<li>The Material Effects and Object channels of the RPF format</li> </ul>
	\par Parameters:
	<b>id</b>\n\n
	The material or object id for which the color is wanted\n\n
	<b>cOut</b>\n\n
	The output color BMM_Color_24 type. */
	CoreExport static void ColorById( int id , BMM_Color_24& cOut );

	};

/*! \remarks Sets the maximum GBuffer layer depth.
\par Parameters:
<b>int m</b>\n\n
The number to set. */
CoreExport void SetMaximumGBufferLayerDepth(int m);
/*! \remarks Returns the maximum GBuffer layer depth. */
CoreExport int GetMaximumGBufferLayerDepth();

/*! \remarks Creates and returns a pointer to a new default G-Buffer. The
buffer is essentially created empty.\n\n
A 'default' G-Buffer is one that 3ds Max itself creates. See
<b>GBuffer::IsDefaultGBuffer()</b> below. */
CoreExport GBuffer *NewDefaultGBuffer();

