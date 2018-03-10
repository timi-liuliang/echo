//
// Copyright 2009 Autodesk, Inc.  All rights reserved. 
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may
// not be disclosed to, copied or used by any third party without the prior
// written consent of Autodesk, Inc.
#pragma once
#include "baseinterface.h"
#include "strbasic.h" // MCHAR
#include "matrix3.h"
#include "GraphicsConstants.h"
#include "GraphicsTypes.h"
#include "Strip.h" // cannot forward declare this
// forward declarations
class Light;
class Camera;
class Material;
class IPoint3;
class GWFace;
class BitArray;
class HitRegion;
class VertexBuffer;
class LineBuffer;
namespace GFX_MESH
{
	class HWTupleMesh;
	class IHWSubMesh;
};

// abstract graphics window class
/*! \sa  Class InteractiveRenderer for Graphics Window, 
Class GWinSetup, Class HitRegion, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_marker_types.html">List of Marker Types</a>,
<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_rendering_limits.html">List of Rendering Limits</a>,
Class Point3, Class IPoint3, Class Matrix3, Class Interface, Class ViewExp.\n\n
\par Description:
The abstract graphics window class. The methods here provide low-level access
to 3ds Max's graphics system. These methods are available for plug-ins to do
any graphics work not possible using the standard high-level graphics methods
of 3ds Max.\n\n
These methods are for use in the existing 3ds Max viewports. Note that these
APIs are not for casual use, as they are not intended to be a high level
graphics library. For example, many steps are required to display a single lit
polygon. These APIs are optimized for speed, and not at all for plug-in
programmer ease of use.\n\n
These methods are provided, however, so that developers can do things that are
otherwise impossible using the high-level methods.\n\n
<b>Developers should use these methods with an understanding of exactly what
they are doing since it's quite easy to crash 3ds Max when inappropriate
arguments are supplied. The calls are specifically optimized for exactly the
way 3ds Max uses them. In no way should the calls in GraphicsWindow be
considered an "ordinary" 2D/3D API. (That's what OpenGL, D3D, and HEIDI are
for.)</b>\n\n
One final note of warning: most graphics windows methods execute in a separate
thread (or in multiple separate threads) that are owned by the graphics window.
Thus, due to thread scheduling, when a bad argument or incorrect sequencing of
graphics windows calls causes 3ds Max to crash, it is not at all easy to figure
out where the problem is. In particular, the location of the main 3ds Max
thread is not relevant.\n\n
All the methods of this class are implemented by the system.
\par Method Groups:
See <a href="class_graphics_window_groups.html">Method Groups for Class GraphicsWindow</a>.
*/
#pragma warning(push)
#pragma warning(disable:4100)
class GraphicsWindow : public InterfaceServer {
public:
	virtual	~GraphicsWindow() {}
	/*! \remarks	This is called after all four GraphicsWindows used by 3ds Max are created.
	SDK users shouldn't need this call */
	virtual void	postCreate(int ct, GraphicsWindow **gw) = 0;
	/*! \remarks	This is used to tell the driver that it is shutting down. */
	virtual void	shutdown() = 0;
	/*! \remarks	This returns "0x200" to indicate R2.0 */
	virtual int		getVersion() = 0;
	/*! \remarks	This identifies the driver (and includes manufacturer info if available) */
	virtual MCHAR * getDriverString(void) = 0;
	/*! \remarks	This is called to put up the config dialog if the driver supports
	GW_SPT_CAN_CONFIG
	\par Parameters:
	<b>HWND hWnd</b>\n\n
	The parent window handle for the dialog. */
	virtual void	config(HWND hWnd) = 0;
	/*! \remarks Determines if the driver supports the specified feature.
	\par Parameters:
	<b>int what</b>\n\n
	One of the following values:\n\n
	<b>GW_SPT_TXT_CORRECT</b>\n\n
	This is used to enable or gray-out the perspective correction right-click
	viewport menu option.\n\n
	<b>GW_SPT_GEOM_ACCEL</b>\n\n
	This is used to indicate to 3ds Max (and the mesh class in particular) that
	the driver wants to handle all of the 3D data natively. In this case,
	meshes are rendered by passing 3D world space data and letting the driver
	transform, clip, and light the vertices. If this returns FALSE, then the
	mesh class handles all xforms/clip/lighting calculations (using a lazy
	evaluation algorithm) and then calls the hPolygon or hPolyline 2 1/2D calls
	for the driver to rasterize. (Primitives that are actually clipped are
	still sent to the polygon/polyline methods.)\n\n
	Right now, only the OpenGL driver returns TRUE to this query, but other
	drivers have been developed that return TRUE, and the HEIDI and D3D drivers
	may change in the future.\n\n
	<b>GW_SPT_TRI_STRIPS</b>\n\n
	If this returns TRUE, then 3ds Max will try to stripify meshes before
	calling the rendering methods. Right now, the drivers just return the user
	preference that is set in the driver config dialog. It defaults to
	TRUE.\n\n
	<b>GW_SPT_DUAL_PLANES</b>\n\n
	If a driver has dual-planes support it returns TRUE. Our OpenGL display
	driver only returns TRUE for this if the underlying display driver has
	implemented a custom OpenGL extension that allows us to handle this
	efficiently.\n\n
	<b>GW_SPT_SWAP_MODEL</b>\n\n
	This returns TRUE if 3ds Max has to redraw the whole scene any time the
	viewports are exposed.\n\n
	<b>GW_SPT_INCR_UPDATE</b>\n\n
	This returns TRUE if the driver can update a rectangular subset of the
	viewport without trashing the image outside that rectangle. This is TRUE
	for most drivers that blit the viewport region and FALSE for those that do
	page-flipping in the hardware. For OpenGL, this is TRUE if the display
	driver implements the Microsoft glSwapRectHintWIN extension.\n\n
	<b>GW_SPT_1_PASS_DECAL</b>\n\n
	This is TRUE if the driver can handle decalling with only one pass. Right
	now, this is TRUE for OpenGL, but FALSE for HEIDI and D3D. (And as with all
	of these options, it may change in a given driver anytime in the
	future.)\n\n
	<b>GW_SPT_DRIVER_CONFIG</b>\n\n
	This is TRUE if the driver has a configuration dialog box. This is TRUE for
	all three of our standard drivers.\n\n
	<b>GW_SPT_TEXTURED_BKG</b>\n\n
	This is TRUE if the viewport background is implemented as a textured
	rectangle, and FALSE if it is a blitted bitmap.\n\n
	<b>GW_SPT_VIRTUAL_VPTS</b>\n\n
	This is TRUE if the driver allows viewports to be made larger than the
	physical window they are attached to. Right now this is ony TRUE for
	OGL.\n\n
	<b>GW_SPT_PAINT_DOES_BLIT</b>\n\n
	This is TRUE if WM_PAINT messages result in a blit of the backbuffer (as
	opposed to a page-flipping swap). This allows 3ds Max to do quick damage
	region repair, and works together with the GW_SPT_SWAP_MODEL flag.\n\n
	<b>GW_SPT_WIREFRAME_STRIPS</b>\n\n
	This is TRUE if the driver wants 3ds Max to send down wireframe models
	using triangle strips instead of a bundle of 2-pt segments. This is only
	used by the OGL driver, and it is there as a user-choosable
	performance-accuracy tradeoff (since the strips are faster and are
	back-culled, but they displayhidden edges as though they are visible). */
	virtual int		querySupport(int what) = 0;

	/*! \remarks	This returns the "output" window handle. (Input goes to an invisible window
	above the viewport. The invisible window is owned by 3ds Max.) */
	virtual HWND	getHWnd(void) = 0;
	/*! \remarks Sets the size and position of the GraphicsWindow. The
	coordinates are all Windows coordinates in the space of the
	GraphicsWindows' parent window. (The origin is the upper left.)
	\par Parameters:
	<b>int x</b>\n\n
	Window x origin.\n\n
	<b>int y</b>\n\n
	Window y origin.\n\n
	<b>int w</b>\n\n
	Window width.\n\n
	<b>int h</b>\n\n
	Window height. */
	virtual void	setPos(int x, int y, int w, int h) = 0;
	/*! \remarks	The specified value may be sent to the driver to indicate the display state
	of the viewport window controlled by the driver.
	\par Parameters:
	<b>int s</b>\n\n
	The display state to set. One of the following values:\n\n
	<b>GW_DISPLAY_MAXIMIZED</b>\n\n
	<b>GW_DISPLAY_WINDOWED</b>\n\n
	<b>GW_DISPLAY_INVISIBLE</b> */
	virtual void	setDisplayState(int s) = 0;
	/*! \remarks	This method returns the current state. One of the following values:\n\n
	<b>GW_DISPLAY_MAXIMIZED</b>\n\n
	<b>GW_DISPLAY_WINDOWED</b>\n\n
	<b>GW_DISPLAY_INVISIBLE</b> */
	virtual int		getDisplayState() = 0;
	/*! \remarks	This method gets the current window size in X. */
	virtual int		getWinSizeX() = 0;
	/*! \remarks	This method gets the current window size in Y. */
	virtual int		getWinSizeY() = 0;
	/*! \remarks	This method returns the z-buffer depth (in bits) */
	virtual DWORD	getWinDepth(void) = 0;
	/*! \remarks	This method returns the largest device Z value. */
	virtual DWORD	getHitherCoord(void) = 0;
	/*! \remarks	This method returns the smallest device Z value. */
	virtual DWORD	getYonCoord(void) = 0;
	/*! \remarks	This method returns the size (in pixels) that the specified text string
	will occupy
	\par Parameters:
	<b>MCHAR *text</b>\n\n
	The string to check.\n\n
	<b>SIZE *sp</b>\n\n
	The size is returned here. See
	<a href="ms-its:3dsmaxsdk.chm::/start_data_types.html">Data Types</a>. */
	virtual void	getTextExtents(MCHAR *text, SIZE *sp) = 0;
	/*! \remarks	This method returns the largest number of triangles that can be in a strip
	*/
	virtual int		getMaxStripLength() { return GFX_MAX_STRIP; }
	virtual void	setFlags(DWORD f) = 0;
	virtual DWORD	getFlags() = 0;

	/*! \remarks This method resets the update rectangle. The update rectangle
	is the region of the screen that needs to be updated to reflect items that
	have changed. When the system is done rendering items, the goal is to only
	update the region that has actually been altered. This method sets the
	update rectangle (the region that will be blitted to the display) to
	invalid. In this way when <b>enlargeUpdateRect()</b> is later called, the
	RECT passed will be used as the region. */
	virtual void	resetUpdateRect() = 0;
	/*! \remarks This method enlarges the update rectangle to include the RECT
	passed. If <b>rp</b> is NULL, then the whole window will later be updated.
	\par Parameters:
	<b>RECT *rp</b>\n\n
	Pointer to a rectangle (or NULL). */
	virtual void	enlargeUpdateRect(RECT *rp) = 0;
	/*! \remarks This method retrieves the current update rectangle.
	\par Parameters:
	<b>RECT *rp</b>\n\n
	The current update rectangle.
	\return  Zero if the update rectangle is invalid; otherwise nonzero. */
	virtual int		getUpdateRect(RECT *rp) = 0;
	/*! \remarks This method is used internally and should not be called by
	plug-in developers. */
	virtual void	updateScreen() = 0;

	/*! \remarks This method is used internally. Most drivers control two
	image buffers. One is displayed on the screen, and the other is used to
	rasterize geometric primitives. When rasterization of a complete frame is
	done, the off-screen buffer is blitted onto the display screen. This is
	referred to as dual-plane mode. This method will turn dual-plane mode on or
	off. This is used internally by the File/Preferences... Viewport page Use
	Dual Planes toggle.
	\par Parameters:
	<b>int which</b>\n\n
	Specifies which buffer should use dual-planes.\n\n
	<b>BUF_F_BUFFER</b>\n\n
	The image (Framebuffer) buffer.\n\n
	<b>BUF_Z_BUFFER</b>\n\n
	The Z buffer.\n\n
	<b>int b</b>\n\n
	Nonzero to enable access (toggle on); 0 to toggle off.
	\return  TRUE if the graphics window has access to the specified buffer;
	otherwise FALSE. */
	virtual BOOL	setBufAccess(int which, int b) = 0;
	/*! \remarks This method is used internally. It returns a boolean value
	indicating if dual plane mode is on or off for the specified buffer.
	\par Parameters:
	The buffer whose dual-planes setting will be returned.\n\n
	<b>int which</b>\n\n
	The buffer whose dual-planes setting will be returned. One of the following
	values:\n\n
	<b>BUF_F_BUFFER</b>\n\n
	The Framebuffer.\n\n
	<b>BUF_Z_BUFFER</b>\n\n
	The Z buffer.
	\return  TRUE if the dual-plane mode is on; otherwise FALSE. */
	virtual BOOL	getBufAccess(int which) = 0;
	/*! \remarks This method is used internally. It retrieves the size of the
	specified buffer in bytes.
	\par Parameters:
	<b>int which</b>\n\n
	One of the following values:\n\n
	<b>int *size</b>\n\n
	The size of the buffer in bytes.\n\n
	Note the following concerning the HEIDI driver. For HEIDI getBufSize()
	always returns 10 if dual-planes are on (and 0 otherwise). This is because
	HEIDI actually never returns the image - it keeps its own copy stored away.
	Thus the "logical" way to think is that we actually get a copy of the
	buffer by calling getBuf, and that we give it back by calling setBuf. But
	in reality (with the HEIDI driver) getBuf and setBuf only tell HEIDI to do
	some internal buffer manipulation.
	\return  TRUE if the size was returned; otherwise FALSE. */
	virtual BOOL	getBufSize(int which, int *size) = 0;
	/*! \remarks This method is used internally. It retrieves the specified
	buffer.
	\par Parameters:
	<b>int which</b>\n\n
	The buffer to retrieve. One of the following values:\n\n
	<b>BUF_F_BUFFER -</b> The image Framebuffer.\n\n
	<b>BUF_Z_BUFFER -</b> The Z buffer.\n\n
	<b>int size</b>\n\n
	The number of bytes to retrieve. This must be at least the size returned
	from <b>getBufSize()</b>.\n\n
	<b>void *buf</b>\n\n
	Storage for the buffer data.
	\return  TRUE if the buffer was retrieved; otherwise FALSE. */
	virtual BOOL	getBuf(int which, int size, void *buf) = 0;
	/*! \remarks Stores the specified buffer.
	\par Parameters:
	<b>int which</b>\n\n
	The buffer to store. One of the following values:\n\n
	<b>BUF_F_BUFFER -</b> The image Framebuffer.\n\n
	<b>BUF_Z_BUFFER -</b> The Z buffer.\n\n
	<b>int size</b>\n\n
	The number of bytes to store.\n\n
	<b>void *buf</b>\n\n
	The buffer data.\n\n
	<b>RECT *rp</b>\n\n
	This allows only a subset of the saved image rect to be blitted back to the
	screen.
	\return  TRUE if the buffer was stored; otherwise FALSE. */
	virtual BOOL	setBuf(int which, int size, void *buf, RECT *rp) = 0;
	/*! \remarks This method returns the viewport image of this graphics
	window in a packed DIB format. A packed DIB is the standard BMI header
	followed immediately by all the data bytes (pixels) that make up the image.
	This is the standard way in Windows to pass a DIB around. See the sample
	code below for an example of this call in use. Note how it is called twice:
	once to get the size, once to get the DIB.
	\par Parameters:
	<b>BITMAPINFO *bmi</b>\n\n
	The BITMAPINFO structure defines the dimensions and color information for a
	Windows device-independent bitmap (DIB). Note that if this parameter is
	NULL, then only the size value is returned.\n\n
	<b>int *size</b>\n\n
	The size of the image in bytes.
	\return  TRUE if the image was returned; otherwise FALSE.
	\par Sample Code:
	The following sample code saves the current 3ds Max viewport to a user
	specified file.\n\n
	\code	
	void TestGetDIB(IObjParam *ip)
	{
	BITMAPINFO *bmi = NULL;
	BITMAPINFOHEADER *bmih;
	BitmapInfo biFile;
	Bitmap *map;
	int size;
	TheManager->SelectFileOutput(\&biFile,
	ip->GetMAXHWnd(), _M("Testing"));
	if(!biFile.Name()[0])
	return;
	ViewExp *vpt = ip->GetActiveViewport();
	vpt->getGW()->getDIB(NULL, \&size);
	bmi = (BITMAPINFO *)malloc(size);
	bmih = (BITMAPINFOHEADER *)bmi;
	vpt->getGW()->getDIB(bmi, \&size);
	biFile.SetWidth((WORD)bmih->biWidth);
	biFile.SetHeight((WORD)bmih->biHeight);
	biFile.SetType(BMM_TRUE_32);
	map = TheManager->Create(\&biFile);
	map->OpenOutput(\&biFile);
	map->FromDib(bmi);
	map->Write(\&biFile);
	map->Close(\&biFile);
	if(bmi)
	free(bmi);
	ip->ReleaseViewport(vpt);
	} 
	\endcode */
	virtual BOOL	getDIB(BITMAPINFO *bmi, int *size) = 0;
	/*! \remarks	This method is used internally to zoom the viewport. */
	virtual BOOL	setBackgroundDIB(int width, int height, BITMAPINFO *bmi) = 0;
	/*! \remarks	This method is used internally to pan the viewport. */
	virtual void	setBackgroundOffset(int x, int y) = 0;
	virtual int		useClosestTextureSize(int bkg=FALSE) = 0;
	/*! \remarks	This method returns the size of the texture bitmap that the driver wants
	sent in to the <b>getTextureHandle()</b> call (if bkg is FALSE). If bkg is
	TRUE, this returns the size of the texture that 3ds Max shoud send to the
	<b>setBackgroundDIB()</b> call. In general, the return value needs to be a
	power of 2, though that could be driver-specific
	\par Parameters:
	<b>int bkg=FALSE</b>\n\n
	TRUE to get the size for <b>setBackgroundDIB()</b>; FALSE to get the size
	for <b>getTextureHandle()</b>. */
	virtual int		getTextureSize(int bkg=FALSE) = 0;
	/*! \remarks This method returns a handle for the specified texture bitmap.
	This handle is then used with the <b>setTextureByHandle()</b> method (there is
	only one current texture active at any time). The texture dimensions must be a
	power of 2.\n\n
	When a material is on an object, and the material has a texture map, and when
	the button used to display the texture map is pressed, 3ds Max calls this
	method to get the texture handle. This basically loads the texture into the
	hardware RAM (if available). When this mapped object gets displayed the method
	<b>setTextureHanel()</b> is called. When the material is deleted, or the
	dispaly in viewport button is turned off, <b>freeTextureHandle()</b> is called.
	\par Parameters:
	<b>BITMAPINFO *bmi</b>\n\n
	The DIB image to use as a texture.
	\return  The texture handle. */
	virtual DWORD_PTR	getTextureHandle(BITMAPINFO *bmi) = 0;
	/*! \remarks When you are finished with the texture handle, call this method
	to free it.
	\par Parameters:
	<b>DWORD_PTR handle</b>\n\n
	The texture handle to free.
	\return  TRUE if the texture was set; otherwise FALSE. */
	virtual void	freeTextureHandle(DWORD_PTR handle) = 0;
	/*! \remarks This sets the current texture to the image whose handle is passed
	(see <b>getTextureHandle()</b>). The texture dimensions must be a power of 2.
	\par Parameters:
	<b>DWORD_PTR handle</b>\n\n
	The handle of the texture to make current. */
	virtual BOOL	setTextureByHandle(DWORD_PTR handle, int texStage=0) = 0;
	virtual void	setTextureColorOp(int texStage=0, int texOp=GW_TEX_MODULATE, int texAlphaSource=GW_TEX_TEXTURE, int texScale=GW_TEX_SCALE_1X) = 0;
	virtual void	setTextureAlphaOp(int texStage=0, int texOp=GW_TEX_MODULATE, int texAlphaSource=GW_TEX_TEXTURE, int texScale=GW_TEX_SCALE_1X) = 0;
	/*! \remarks Sets the way in which textures are tiled across the surface of
	the object.
	\par Parameters:
	The following parameters may use one of these values:\n\n
	<b>GW_TEX_NO_TILING</b>\n\n
	The texture clamped - Any UVW that is bigger than 1 is interpreted as being
	1.\n\n
	<b>GW_TEX_REPEAT</b>\n\n
	As the UVW numbers keep getting larger than 1 the image is repeated.\n\n
	<b>GW_TEX_MIRROR</b>\n\n
	If UVW goes beyond 1, the numbers are interpreted as going backwards. So if you
	had 0 to 2 it would actually go 0 to 1 then 1 down to 0.\n\n
	<b>int u</b>\n\n
	The type of texturing in the U direction.\n\n
	<b>int v</b>\n\n
	The type of texturing in the V direction.\n\n
	<b>int w=GW_TEX_NO_TILING</b>\n\n
	The type of texturing in the W direction.
	\return  TRUE if the tiling mode was set; otherwise FALSE. */
	virtual BOOL	setTextureTiling(int u, int v, int w=GW_TEX_NO_TILING, int texStage=0) = 0;
	/*! \remarks Returns the type of texture tiling set for the particular
	direction.\n\n
	For example, if <b>setTextureTiling(GW_TEX_NO_TILING, GW_TEX_REPEAT,
	GW_TEX_MIRROR)</b> were called first, then\n\n
	<b>getTextureTiling(0)</b> would yield <b>GW_TEX_NO_TILING</b>, and\n\n
	<b>getTextureTiling(1)</b> would yield <b>GW_TEX_REPEAT</b>.
	\par Parameters:
	<b>int which</b>\n\n
	This value is 0 or 1 and it represents the U or V direction respectively. The
	value 2 is not yet implemented.
	\return  <b>GW_TEX_NO_TILING</b>\n\n
	The texture clamped - Any UVW that is bigger than 1 is interpreted as being
	1.\n\n
	<b>GW_TEX_REPEAT</b>\n\n
	As the UVW numbers keep getting larger than 1 the image is repeated.\n\n
	<b>GW_TEX_MIRROR</b>\n\n
	If UVW goes beyond 1, the numbers are interpreted as going backwards. So if you
	had 0 to 2 it would actually go 0 to 1 then 1 down to 0. */
	virtual int		getTextureTiling(int which, int texStage=0) = 0;

	/*! \remarks If a developer is working with an existing 3ds Max instance
	of GraphicsWindow (one of 3ds Max's viewports) this method should NOT be
	called. */
	virtual void	beginFrame() = 0;
	/*! \remarks As above, if a developer is working with an existing 3ds Max
	instance of GraphicsWindow (one of 3ds Max's viewports) this method should
	NOT be called. */
	virtual void	endFrame() = 0;
	/*! \remarks This method sets the clipping boundaries within a viewport
	within the graphics window. This allows more than one viewport to appear
	within a single graphics window. It has the side-effect of building a 4x4
	viewport matrix. This routine should be called anytime the graphics window
	is resized, or else rendering will still occur to the old window size. (And
	since most drivers do not do range-checking since it is too time-costly,
	this could cause a system crash.)
	\par Parameters:
	Note: all coordinates are in Windows format, with the origin in the upper
	left\n\n
	<b>int x</b>\n\n
	Specifies the left viewport origin.\n\n
	<b>int y</b>\n\n
	Specifies the top viewport origin.\n\n
	<b>int w</b>\n\n
	Specifies the viewport width.\n\n
	<b>int h</b>\n\n
	Specifies the viewport height. */
	virtual void	setViewport(int x, int y, int w, int h) = 0;
	/*! \remarks	This method is used to setup a virtual viewport. Note that this is a no-op
	unless <b>GW_SPT_VIRTUAL_VPTS</b> is TRUE. Plug-in developers should not
	call this method -- it's for internal use only. */
	virtual void	setVirtualViewportParams(float zoom, float xOffset, float yOffset) = 0;
	/*! \remarks	This method is used to set a virtual viewport as active. Note that this is
	a no-op unless <b>GW_SPT_VIRTUAL_VPTS</b> is TRUE. Plug-in developers
	should not call this method -- it's for internal use only.
	\par Parameters:
	<b>int onOff</b>\n\n
	TRUE to set the virtual viewport active; FALSE to make it inactive. */
	virtual void	setUseVirtualViewport(int onOff) = 0;
	/*! \remarks Clears the specified rectangular region of the screen.
	\par Parameters:
	<b>RECT *rp</b>\n\n
	Specifies the rectangular region to clear.\n\n
	<b>int useBkg = FALSE</b>\n\n
	Specifies if the background should be used to fill the cleared area.
	Nonzero indicate the background should be used; 0 indicates the 'clear'
	color should be used (see <b>setColor()</b> above). */
	virtual void	clearScreen(RECT *rp, int useBkg = FALSE) = 0;
	/*! \remarks Sets the current transformation matrix, and updates the
	modeling coordinates to normalized projection coordinates matrix. This
	routine also back-transforms each light and the eye point (so that lighting
	can be done in modeling coordinates).\n\n
	This method may be used to set a matrix that transforms the point passed to
	the drawing methods (like <b>text(), marker(), polyline()</b> or
	<b>polygon()</b>). Normally these methods expect world coordinates. However
	if this matrix is set to an objects transformation matrix you can pass
	objects space coordinates and they will be transformed into world space
	(and then put into screen space when they are drawn). If however this is
	set to the identity matrix you would pass world space coordinates. You can
	set this matrix to the objects matrix using the following code:\n\n
	<b>gw-\>setTransform(inode-\>GetObjectTM(t));</b>\n\n
	Note: For world-to-screen space conversions by the methods <b>text(),
	marker(), polyline()</b>, <b>polygon()</b>, etc, a developer must
	explicitly set this matrix to the identity. This is because the
	<b>GraphicsWindow</b>transform may have a non-identity matrix already in
	place from a previous operation.
	\par Parameters:
	<b>const Matrix3 \&m</b>\n\n
	The new current transformation matrix. */
	virtual void	setTransform(const Matrix3 &m) = 0;
	virtual Matrix3	getTransform(void) = 0;
	virtual void	multiplePass(int pass, BOOL onOff, float scaleFact = 1.005f) = 0;
	/*! \remarks Sets the current transparency flags for the current pass.\n\n

	\par Parameters:
	<b>DWORD settings</b>\n\n
	This can be a combination if GW_TRANSPARENCY and GW_TRANSPARENT_PASS See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_rendering_limits.html">Rendering Limits</a>. You
	also use these settings in the Render limits as well. */
	virtual void	setTransparency(DWORD settings) = 0;
	/*! \remarks This method allows one to put an affine transformation on a
	texture. This allows you to translate, rotate or scale a texture on an object.
	\par Parameters:
	<b>const Matrix3 \&m</b>\n\n
	The texture transformation matrix. */
	virtual void	setTexTransform(const Matrix3 &m, int texStage=0) = 0;
	/*! \remarks This is used internally. It returns if the determinant of the
	current transform is positive or negative. If it's positive 0 is returned;
	if it's negative 1 is returned. */
	virtual BOOL	getFlipped(void)=0;
	/*! \remarks Sets the number of triangles skipped when the viewport is set
	as a 'Fast View Display' viewport. To disable fastview, specify 1. Note
	that the GraphicsWindow class doesn't actually do anything with this number
	other than store it. Since triangles are handed down to GFX one at a time,
	it is up to the code that feeds triangles to the GraphicsWindow to skip the
	specified number of triangles. The mesh rendering in 3ds Max uses the skip
	count in this way.
	\par Parameters:
	<b>int n</b>\n\n
	Specifies that every 'n-th' triangle should be drawn. If set to 2, every
	other triangle should be drawn. */
	virtual void	setSkipCount(int c) = 0;
	/*! \remarks Returns the current skip count setting. */
	virtual int		getSkipCount(void) = 0;
	virtual void	setViewportLimits(DWORD l) = 0;
	virtual DWORD	getViewportLimits(void) = 0;
	/*! \remarks Sets the rendering limits used by primitive calls.\n\n
	Note: Setting the rendering limits is used in communication between the
	various parts of 3ds Max that handle the display of objects. For example,
	setting this limit to <b>GW_POLY_EDGES</b> and then drawing a polygon won't
	result in a polygon drawn with edges. It only sets a flag that indicates
	the edge should be drawn.\n\n
	What happens is as follows. Inside the upper level 3ds Max, part of the
	code knows that polygon edges have been turned on. However this is not
	related through the object oriented architecture to the part of 3ds Max
	that does the actual drawing. When 3ds Max goes to draw objects it will see
	that the polygon edge flag is on. This tells it to do two drawing passed --
	one to do the polygon, then it calls <b>outlinePass()</b> call with TRUE,
	draws a bunch of edges, then calls <b>outline Pass()</b> with FALSE. Thus,
	the drawing routine is responsible for looking at the flags and drawing
	appropriately. This method is only responsible setting the limit which can
	later be checked.
	\par Parameters:
	<b>DWORD l</b>\n\n
	Specifies the rendering limit used by the viewport. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_rendering_limits.html">Rendering Limits</a>. */
	virtual void	setRndLimits(DWORD l) = 0;
	/*! \remarks Retrieves the rendering limits used by primitive calls. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_rendering_limits.html">Rendering Limits</a>. */
	virtual DWORD 	getRndLimits(void) = 0;
	/*! \remarks Returns the current rendering mode used by the viewport. This
	is a subset of the rendering limit, in that any limits imposed by the
	rendering limit are forced onto the current mode. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_rendering_limits.html">Rendering Limits</a>. */
	virtual DWORD 	getRndMode(void) = 0;
	/*! \remarks Returns the maximum number of lights that may be used by the
	interactive renderer. */
	virtual int		getMaxLights(void) = 0;
	/*! \remarks Turns a light on or off, and sets the light parameters. The
	light's position is set through the current transformation matrix at the
	time of this call. A particular light is specified by its light number (-1
	through <b>getMaxLights()-1</b>). Light number -1 is reserved for ambient
	light. If a particular field in the Light class is not needed for a
	particular type of light, that field's value is ignored (for example, only
	the color field is used for ambient light.)
	\par Parameters:
	<b>int num</b>\n\n
	The light number of the light to set. This is a value in the range -1 to
	<b>getMaxLights()-1</b>.\n\n
	<b>const Light *l</b>\n\n
	The light class instance used to set the light parameters. If this is NULL,
	the light is turned off. */
	virtual void	setLight(int num, const Light *l) = 0;
	/*! \remarks This allows a developer to control if a light is used to
	render an object. There is one bit per light (bits 0 through
	<b>getMaxLights()</b>). If the bit is set the light is NOT used to render
	the object. If the bit is off, the light IS used. This method allows you to
	set the exclusion vector controlling the lights.
	\par Parameters:
	<b>DWORD exclVec</b>\n\n
	The exclusion vector controlling the lights. */
	virtual void	setLightExclusion(DWORD exclVec) = 0;
	/*! \remarks This method is no longer used. */
	virtual void	setCamera(const Camera& c) = 0;
	/*! \remarks Sets the properties of the current camera used by the
	GraphicsWindow.
	\par Parameters:
	<b>float mat[4][4]</b>\n\n
	The transformation matrix times the projection matrix.\n\n
	<b>Matrix3 *invTM</b>\n\n
	This is the inverse of the affine part of the camera transformation matrix
	(not the inverse of the projection part).\n\n
	<b>int persp</b>\n\n
	Nonzero indicates this is a perspective view; 0 is orthogonal.\n\n
	<b>float hither</b>\n\n
	Near clip value.\n\n
	<b>float yon</b>\n\n
	Far clip value. */
	virtual void	setCameraMatrix(float mat[4][4], Matrix3 *invTM, int persp, float hither, float yon) = 0;
	/*! \remarks Retrieves the properties of the current camera.
	\par Parameters:
	<b>float mat[4][4]</b>\n\n
	The transformation matrix times the projection matrix.\n\n
	<b>Matrix3 *invTM</b>\n\n
	This is the inverse of the affine part of the camera transformation matrix
	(not the inverse of the projection part).\n\n
	<b>int *persp</b>\n\n
	Nonzero indicates this is a perspective view; 0 is orthogonal.\n\n
	<b>float *hither</b>\n\n
	Near clip value.\n\n
	<b>float *yon</b>\n\n
	Far clip value. */
	virtual void	getCameraMatrix(float mat[4][4], Matrix3 *invTM, int *persp, float *hither, float *yon) = 0;
	/*! \remarks Sets the RGB color used for the specified drawing type (line,
	fill, text, clear).
	\par Parameters:
	<b>ColorType t</b>\n\n
	One of the following values:\n\n
	<b>LINE_COLOR</b>\n\n
	Line drawing color.\n\n
	<b>FILL_COLOR</b>\n\n
	Polygon fill color.\n\n
	<b>TEXT_COLOR</b>\n\n
	Text drawing color.\n\n
	<b>CLEAR_COLOR</b>\n\n
	The color that the viewport is cleared to when you call
	<b>clearScreen()</b>.\n\n
	<b>float r</b>\n\n
	Specifies the red amount 0.0 - 1.0.\n\n
	<b>float g</b>\n\n
	Specifies the green amount 0.0 - 1.0.\n\n
	<b>float b</b>\n\n
	Specifies the blue amount 0.0 - 1.0. */
	virtual void	setColor(ColorType t, float r, float g, float b) = 0;
	void	setColor(ColorType t, Point3 clr) { setColor(t,clr.x,clr.y,clr.z); }
	/*! \remarks Sets the current rendering material, and modifies the
	rendering mode parameter for controlling the rasterizer driver. Note: You
	must have your rendering limit set BEFORE you set the material because the
	material setting may lower the rendering mode based on the material limits.
	\par Parameters:
	<b>const Material \&m</b>\n\n
	The new material to instantiate\n\n
	<b>int index=0</b>\n\n
	Indicates which material index refers to the material which gets set. */
	virtual void	setMaterial(const Material& m, int index = 0) = 0;
	/*! \remarks Returns the current rendering material.\n\n
	*/
	virtual Material *getMaterial(void) = 0;
	virtual int		getMaxTextures(void) = 0;

	/*! \remarks	This method converts coordinates to "<b>h</b>" format device coordinates.
	Note: This method maps points from the GraphicsWindow's current transform
	to device space. If the GraphicsWindow's transform is set to the identity
	matrix then the mapping is done from points specified in world space.
	Otherwise the points given are transformed by the GraphicsWindow transform,
	and are <b>then</b> considered to be in world space. Thus, to get a
	world-space to screen-space conversion, you need to set the transform to
	the identity with <b>gw-\>setTransform(Matrix3(1))</b>.
	\par Parameters:
	<b>const Point3 *in</b>\n\n
	The input point.\n\n
	<b>IPoint3 *out</b>\n\n
	The output point in integer format values in the native device coords for
	the device. For HEIDI and OpenGL the origin at the lower left. For Direct3D
	the origin is at the upper left.
	\return  DWORD containing the clipping flags for the point. If a flag is
	set it indicates the transformed point lies outside the view volume. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_clip_flags.html">List of Clip Flags</a>. */
	virtual DWORD	hTransPoint(const Point3 *in, IPoint3 *out) = 0;
	/*! \remarks	This method is used to convert coordinates to "<b>w</b>" format device
	coordinates. Note: This method maps points from the GraphicsWindow's
	current transform to device space. If the GraphicsWindow's transform is set
	to the identity matrix then the mapping is done from points specified in
	world space. Otherwise the points given are transformed by the
	GraphicsWindow transform, and are <b>then</b> considered to be in world
	space. Thus, to get a world-space to screen-space conversion, you need to
	set the transform to the identity with
	<b>gw-\>setTransform(Matrix3(1))</b>.\n\n

	\par Parameters:
	<b>const Point3 *in</b>\n\n
	The input point.\n\n
	<b>IPoint3 *out</b>\n\n
	The output point in integer format with the origin at the upper left.
	\return  DWORD containing the clipping flags for the point. If a flag is
	set it indicates the transformed point lies outside the view volume. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_clip_flags.html">List of Clip Flags</a>. */
	virtual DWORD	wTransPoint(const Point3 *in, IPoint3 *out) = 0;
	/*! \remarks	This method is used to convert coordinates to "<b>h</b>" <b>floating
	point</b> coordinates. This is just a helper routine to avoid building up
	round-off error. 3ds Max uses it just for IK. Note: This method maps points
	from the GraphicsWindow's current transform to device space. If the
	GraphicsWindow's transform is set to the identity matrix then the mapping
	is done from points specified in world space. Otherwise the points given
	are transformed by the GraphicsWindow transform, and are <b>then</b>
	considered to be in world space. Thus, to get a world-space to screen-space
	conversion, you need to set the transform to the identity with
	<b>gw-\>setTransform(Matrix3(1))</b>.
	\par Parameters:
	<b>const Point3 *in</b>\n\n
	The input point.\n\n
	<b>Point3 *out</b>\n\n
	The output point in floating point format with the origin at the lower
	left.
	\return  DWORD containing the clipping flags for the point. If a flag is
	set it indicates the transformed point lies outside the view volume. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_clip_flags.html">List of Clip Flags</a>. */
	virtual DWORD	transPoint(const Point3 *in, Point3 *out) = 0;
	/*! \remarks Lights a vertex, using all the current lights. The vertex
	appears to be transformed using the current transformation matrix, although
	actually the calculations are done using back-transformed light coordinates
	(for speed). The vertex position and normal are passed in, and a color is
	returned. The rendering uses the current material.
	\par Parameters:
	<b>const Point3 \&pos</b>\n\n
	Vertex position.\n\n
	<b>const Point3 \&nor</b>\n\n
	Vertex normal.\n\n
	<b>Point3 \&rgb</b>\n\n
	Returned color. */
	virtual void	lightVertex(const Point3 &pos, const Point3 &nor, Point3 &rgb) = 0;

	/*! \remarks	Draws 2D fixed font annotation text to the specified location. Note: This
	routine DOES perform clipping of the text if it is off the screen.
	\par Parameters:
	<b>IPoint3 *xyz</b>\n\n
	This is the device coordinate for the text. The origin of the text is at
	the lower left corner.\n\n
	<b>MCHAR *s</b>\n\n
	The text to display. */
	virtual void	hText(IPoint3 *xyz, MCHAR *s) = 0;
	/*! \remarks	Draws a marker at the specified location.
	\par Parameters:
	<b>IPoint3 *xyz</b>\n\n
	This is the device coordinate for the marker (with the origin at the lower
	left).\n\n
	<b>MarkerType type</b>\n\n
	See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_marker_types.html">List of Marker Types</a>. */
	virtual void	hMarker(IPoint3 *xyz, MarkerType type) = 0;
	/*! \remarks	This method draws a multi-segment polyline.
	\par Parameters:
	<b>int ct</b>\n\n
	The number of points in the polyline. The maximum number of points that may
	be used in drawing a polyline is 32.\n\n
	<b>IPoint3 *xyz</b>\n\n
	Array of points. These are device coordinates with the origin at the lower
	left.\n\n
	<b>Point3 *rgb</b>\n\n
	If the shade mode is set to smooth and these colors for the vertices are
	specified the polyline will be drawn Gourand shaded. This is how 3ds Max
	draws lit wireframes for instance. If you simply want ordinary lines (drawn
	using the line color) pass NULL.\n\n
	Note: The use of these colors is not supported under the OpenGL driver. The
	rgb values are ignored. Only the current material is taken into
	consideration. (This is how OpenGL works.)\n\n
	<b>int closed</b>\n\n
	If nonzero the first point is connected to the last point, i.e. the
	polyline is closed.\n\n
	<b>int *es</b>\n\n
	Edge state array. This is an array that Indicates if the 'i-th' edge is one
	of three state:\n\n
	<b>GW_EDGE_SKIP</b>\n\n
	Nonexistent - totally invisible.\n\n
	<b>GW_EDGE_VIS</b>\n\n
	Exists and is solid.\n\n
	<b>GW_EDGE_INVIS</b>\n\n
	Exists and is hidden - shown as a dotted line.\n\n
	You may pass NULL for this array and the method will assume that the edges
	are all solid. */
	virtual void	hPolyline(int ct, IPoint3 *xyz, Point3 *rgb, int closed, int *es) = 0;
	/*! \remarks This method is available in release 2.0 and later
	only.\n\n
	This method draws a multi-segment polyline.
	\par Parameters:
	<b>int ct</b>\n\n
	The number of points in the polyline. The maximum number of points
	that may be used in drawing a polyline is 32.\n\n
	<b>IPoint3 *xyz</b>\n\n
	Array of points. These are device coordinates with the origin at
	the lower left.\n\n
	<b>Point3 *rgb</b>\n\n
	If the shade mode is set to smooth and these colors for the
	vertices are specified the polyline will be drawn Gourand shaded.
	This is how 3ds Max draws lit wireframes for instance. If you
	simply want ordinary lines (drawn using the line color) pass
	NULL.\n\n
	Note: The use of these colors is not supported under the OpenGL
	driver. The rgb values are ignored. Only the current material is
	taken into consideration. (This is how OpenGL works.)\n\n
	<b>Point3 *uvw</b>\n\n
	This is not currently used. Pass NULL.\n\n
	<b>int closed</b>\n\n
	If nonzero the first point is connected to the last point, i.e. the
	polyline is closed.\n\n
	<b>int *es</b>\n\n
	Edge state array. This is an array that Indicates if the 'i-th'
	edge is one of three state:\n\n
	<b>GW_EDGE_SKIP</b>\n\n
	Nonexistent - totally invisible.\n\n
	<b>GW_EDGE_VIS</b>\n\n
	Exists and is solid.\n\n
	<b>GW_EDGE_INVIS</b>\n\n
	Exists and is hidden - shown as a dotted line.\n\n
	You may pass NULL for this array and the method will assume that
	the edges are all solid. */
	void	hPolyline(int ct, IPoint3 *xyz, Point3 *rgb, Point3 *uvw, int closed, int *es)
	{ hPolyline(ct, xyz, rgb, closed, es); }
	/*! \remarks	This method draws a multi-point polygon.
	\par Parameters:
	<b>int ct</b>\n\n
	The number of points in the polygon.\n\n
	<b>IPoint3 *xyz</b>\n\n
	Array of points. These are device coordinates with the origin at the lower
	left.\n\n
	<b>Point3 *rgb</b>\n\n
	The color values at the vertices. The rendering mode must include
	<b>GW_ILLUM</b> for these values to be used.\n\n
	Note: The use of these colors is not supported under the OpenGL driver. The rgb
	values are ignored. Only the current material is taken into consideration.
	(This is how OpenGL works.)\n\n
	<b>Point3 *uvw</b>\n\n
	The UVW coordinates. The rendering mode must include <b>GW_TEXTURE</b> for
	these values to be used. */
	virtual void	hPolygon(int ct, IPoint3 *xyz, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;
	/*! \remarks	This method is used for drawing a series of triangles specified as 'strips'. It
	takes a count of 3 or more, and builds triangles in a strip. This sends a lot
	less data and the underlying graphics library has to set up a lot less data
	since it can use the previous information to start the rasterization. This
	results in a significant speed increase.\n\n
	Note that this routine does no clipping so all the points passed must be within
	view.
	\par Parameters:
	<b>int ct</b>\n\n
	The total number of points. After the first two points, each new point is used
	to create a new triangle.\n\n
	<b>IPoint3 *xyz</b>\n\n
	The point data with the origin at the lower left. For instance, to draw a quad,
	the first three points specify the first triangle and the next one is combined
	with the previous two to complete the square.\n\n
	The order for these points follows the 'standard' conventions for stripping
	used in most graphics libraries (for example Direct3D, OpenGL and Heidi).\n\n
	<b>Point3 *rgb</b>\n\n
	The colors for the vertices.\n\n
	Note: The use of these colors is not supported under the OpenGL driver. The rgb
	values are ignored. Only the current material is taken into consideration.
	(This is how OpenGL works.)\n\n
	<b>Point3 *uvw</b>\n\n
	The UVW texture coordinates for the vertices. */
	virtual void	hTriStrip(int ct, IPoint3 *xyz, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;

	/*! \remarks	Draws 2D fixed font annotation text to the specified location. Note: This
	routine DOES perform clipping of the text if it is off the screen.
	\par Parameters:
	<b>IPoint3 *xyz</b>\n\n
	This is the device coordinate for the text. The origin of the text is at
	the upper left corner.\n\n
	<b>MCHAR *s</b>\n\n
	The text to display. */
	virtual void	wText(IPoint3 *xyz, MCHAR *s) = 0;
	/*! \remarks	Draws a marker at the specified location.
	\par Parameters:
	<b>IPoint3 *xyz</b>\n\n
	This is the device coordinate for the marker.\n\n
	<b>MarkerType type</b>\n\n
	See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_marker_types.html">List of Marker Types</a>. */
	virtual void	wMarker(IPoint3 *xyz, MarkerType type) = 0;
	/*! \remarks	This method draws a multi-segment polyline.
	\par Parameters:
	<b>int ct</b>\n\n
	The number of points in the polyline. The maximum number of points that may
	be used in drawing a polyline is 32.\n\n
	<b>IPoint3 *xyz</b>\n\n
	Array of points. These are device coordinates with the origin at the upper
	left.\n\n
	<b>Point3 *rgb</b>\n\n
	If the shade mode is set to smooth and these colors for the vertices are
	specified the polyline will be drawn Gourand shaded. This is how 3ds Max
	draws lit wireframes for instance. If you simply want ordinary lines (drawn
	using the line color) pass NULL.\n\n
	Note: The use of these colors is not supported under the OpenGL driver. The
	rgb values are ignored. Only the current material is taken into
	consideration. (This is how OpenGL works.)\n\n
	<b>int closed</b>\n\n
	If nonzero the first point is connected to the last point, i.e. the
	polyline is closed.\n\n
	<b>int *es</b>\n\n
	Edge state array. This is an array that Indicates if the 'i-th' edge is one
	of three state:\n\n
	<b>GW_EDGE_SKIP</b>\n\n
	Nonexistent - totally invisible.\n\n
	<b>GW_EDGE_VIS</b>\n\n
	Exists and is solid.\n\n
	<b>GW_EDGE_INVIS</b>\n\n
	Exists and is hidden - shown as a dotted line.\n\n
	You may pass NULL for this array and the method will assume that the edges
	are all solid. */
	virtual void	wPolyline(int ct, IPoint3 *xyz, Point3 *rgb, int closed, int *es) = 0;
	/*! \remarks This method is available in release 2.0 and later
	only.\n\n
	This method draws a multi-segment polyline.
	\par Parameters:
	<b>int ct</b>\n\n
	The number of points in the polyline. The maximum number of points
	that may be used in drawing a polyline is 32.\n\n
	<b>IPoint3 *xyz</b>\n\n
	Array of points. These are device coordinates with the origin at
	the upper left.\n\n
	<b>Point3 *rgb</b>\n\n
	If the shade mode is set to smooth and these colors for the
	vertices are specified the polyline will be drawn Gourand shaded.
	This is how 3ds Max draws lit wireframes for instance. If you
	simply want ordinary lines (drawn using the line color) pass
	NULL.\n\n
	Note: The use of these colors is not supported under the OpenGL
	driver. The rgb values are ignored. Only the current material is
	taken into consideration. (This is how OpenGL works.)\n\n
	<b>Point3 *uvw</b>\n\n
	This is not currently used. Pass NULL.\n\n
	<b>int closed</b>\n\n
	If nonzero the first point is connected to the last point, i.e. the
	polyline is closed.\n\n
	<b>int *es</b>\n\n
	Edge state array. This is an array that Indicates if the 'i-th'
	edge is one of three state:\n\n
	<b>GW_EDGE_SKIP</b>\n\n
	Nonexistent - totally invisible.\n\n
	<b>GW_EDGE_VIS</b>\n\n
	Exists and is solid.\n\n
	<b>GW_EDGE_INVIS</b>\n\n
	Exists and is hidden - shown as a dotted line.\n\n
	You may pass NULL for this array and the method will assume that
	the edges are all solid. */
	void	wPolyline(int ct, IPoint3 *xyz, Point3 *rgb, Point3 *uvw, int closed, int *es)
	{ wPolyline(ct, xyz, rgb, closed, es); }

	/*! \remarks	This method draws a multi-point polygon.
	\par Parameters:
	<b>int ct</b>\n\n
	The number of points in the polygon.\n\n
	<b>IPoint3 *xyz</b>\n\n
	Array of points. These are device coordinates with the origin at the upper
	left.\n\n
	<b>Point3 *rgb</b>\n\n
	The color values at the vertices. The rendering mode must include
	<b>GW_ILLUM</b> for these values to be used.\n\n
	Note: The use of these colors is not supported under the OpenGL driver. The rgb
	values are ignored. Only the current material is taken into consideration.
	(This is how OpenGL works.)\n\n
	<b>Point3 *uvw</b>\n\n
	The UVW coordinates. The rendering mode must include <b>GW_TEXTURE</b> for
	these values to be used. */
	virtual void	wPolygon(int ct, IPoint3 *xyz, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;

	/*! \remarks	This method is used for drawing a series of triangles specified as 'strips'. It
	takes a count of 3 or more, and builds triangles in a strip. This sends a lot
	less data and the underlying graphics library has to set up a lot less data
	since it can use the previous information to start the rasterization. This
	results in a significant speed increase.\n\n
	Note that this routine does no clipping so all the points passed must be within
	view.
	\par Parameters:
	<b>int ct</b>\n\n
	The total number of points. After the first two points, each new point is used
	to create a new triangle.\n\n
	<b>IPoint3 *xyz</b>\n\n
	The point data with the origin at the upper left. For instance, to draw a quad,
	the first three points specify the first triangle and the next one is combined
	with the previous two to complete the square.\n\n
	The order for these points follows the 'standard' conventions for stripping
	used in most graphics libraries (for example Direct3D, OpenGL and Heidi).\n\n
	<b>Point3 *rgb</b>\n\n
	The colors for the vertices.\n\n
	Note: The use of these colors is not supported under the OpenGL driver. The rgb
	values are ignored. Only the current material is taken into consideration.
	(This is how OpenGL works.)\n\n
	<b>Point3 *uvw</b>\n\n
	The UVW texture coordinates for the vertices. */
	virtual void	wTriStrip(int ct, IPoint3 *xyz, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;

	/*! \remarks Draws 2D fixed font annotation text to the specified
	location. This method does perform clipping.\n\n
	Note: This method maps points from the GraphicsWindow's current transform
	to screen space. If the GraphicsWindow's transform is set to the identity
	matrix then the mapping is done from points specified in world space.
	Otherwise the points given are transformed by the GraphicsWindow transform,
	and are <b>then</b> considered to be in world space. Thus, to get a
	world-space to screen-space conversion, you need to set the transform to
	the identity with <b>gw-\>setTransform(Matrix3(1))</b>.
	\par Parameters:
	<b>Point3 *xyz</b>\n\n
	This is the coordinate for the text.\n\n
	<b>MCHAR *s</b>\n\n
	The text to display. Note: This routine DOES perform clipping of the text
	if it is off the screen. */
	virtual void 	text(Point3 *xyz, MCHAR *s) = 0;
	virtual void	startMarkers() = 0;
	/*! \remarks Draws a marker at the specified location in world space. This
	method does perform clipping.\n\n
	Note: This method maps points from the GraphicsWindow's current transform
	to screen space. If the GraphicsWindow's transform is set to the identity
	matrix then the mapping is done from points specified in world space.
	Otherwise the points given are transformed by the GraphicsWindow transform,
	and are <b>then</b> considered to be in world space. Thus, to get a
	world-space to screen-space conversion, you need to set the transform to
	the identity with <b>gw-\>setTransform(Matrix3(1))</b>.
	\par Parameters:
	<b>Point3 *xyz</b>\n\n
	This is the coordinate for the marker.\n\n
	<b>MarkerType type</b>\n\n
	See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_marker_types.html">List of Marker Types</a>. */
	virtual void	marker(Point3 *xyz, MarkerType type) = 0;
	virtual void	endMarkers() = 0;
	virtual void	polyline(int ct, Point3 *xyz, Point3 *rgb, int closed, int *es) = 0;
	/*! \remarks Draws a multi-segment polyline with the coordinates
	specified in world space. This method does perform clipping.\n\n
	Note: The arrays of points and vertex related data all must be at
	least one element larger than the <b>ct</b> parameter that is
	passed in. The 3D clipper will use the "extra" space to clip as
	efficiently as possible. If room for the extra element is not
	provided, 3ds Max may crash.\n\n
	Note: This method maps points from the GraphicsWindow's current
	transform to screen space. If the GraphicsWindow's transform is set
	to the identity matrix then the mapping is done from points
	specified in world space. Otherwise the points given are
	transformed by the GraphicsWindow transform, and are <b>then</b>
	considered to be in world space. Thus, to get a world-space to
	screen-space conversion, you need to set the transform to the
	identity with <b>gw-\>setTransform(Matrix3(1))</b>.
	\par Parameters:
	<b>int ct</b>\n\n
	The number of points in the polyline. The maximum number of points
	that may be used in drawing a polyline is 32.\n\n
	<b>Point3 *xyz</b>\n\n
	Array of points. This array must be at least one element larger
	than the <b>ct</b> parameter that is passed in. The 3D clipper will
	use the "extra" space to clip as efficiently as possible. If room
	for the extra element is not provided, 3ds Max will crash.\n\n
	<b>Point3 *rgb</b>\n\n
	If the shade mode is set to smooth and these colors for the
	vertices are specified the polyline will be drawn Gourand shaded.
	This is how 3ds Max draws lit wireframes for instance. If you
	simply want ordinary lines (drawn using the line color) pass
	NULL.\n\n
	Note: The use of these colors is not supported under the OpenGL
	driver. The rgb values are ignored. Only the current material is
	taken into consideration. (This is how OpenGL works.)\n\n
	<b>Point3 *uvw</b>\n\n
	This is not currently used. Pass NULL.\n\n
	<b>int closed</b>\n\n
	If nonzero the first point is connected to the last point, i.e. the
	polyline is closed.\n\n
	<b>int *es</b>\n\n
	Edge state array. This is an array that Indicates if the 'i-th'
	edge is one of three state:\n\n
	<b>GW_EDGE_SKIP</b>\n\n
	Nonexistent - totally invisible.\n\n
	<b>GW_EDGE_VIS</b>\n\n
	Exists and is solid.\n\n
	<b>GW_EDGE_INVIS</b>\n\n
	Exists and is hidden - shown as a dotted line.\n\n
	You may pass NULL for this array and the method will assume that
	the edges are all solid. */
	void	polyline(int ct, Point3 *xyz, Point3 *rgb, Point3 *uvw, int closed, int *es)
	{ polyline(ct, xyz, rgb, closed, es); }
	/*! \remarks	Draws a multi-segment polyline with the coordinates specified in world
	space. This method takes a polyline with a normal for each vertex. This is
	used for hardware accelerated lit wireframes (when <b>GW_SPT_GEOM_ACCEL</b>
	is TRUE).\n\n
	Note: The arrays of points and vertex related data all must be at least one
	element larger than the <b>ct</b> parameter that is passed in. The 3D
	clipper will use the "extra" space to clip as efficiently as possible. If
	room for the extra element is not provided, 3ds Max may crash.\n\n
	This method does perform clipping.\n\n
	Note: This method maps points from the GraphicsWindow's current transform
	to screen space. If the GraphicsWindow's transform is set to the identity
	matrix then the mapping is done from points specified in world space.
	Otherwise the points given are transformed by the GraphicsWindow transform,
	and are <b>then</b> considered to be in world space. Thus, to get a
	world-space to screen-space conversion, you need to set the transform to
	the identity with <b>gw-\>setTransform(Matrix3(1))</b>.
	\par Parameters:
	<b>int ct</b>\n\n
	The number of points in the polyline. The maximum number of points that may
	be used in drawing a polyline is 32.\n\n
	<b>Point3 *xyz</b>\n\n
	Array of points. This array must be at least one element larger than the
	<b>ct</b> parameter that is passed in. The 3D clipper will use the "extra"
	space to clip as efficiently as possible. If room for the extra element is
	not provided, 3ds Max will crash.\n\n
	<b>Point3 *nor</b>\n\n
	The normal values at the vertices, one for each vertex.\n\n
	<b>int closed</b>\n\n
	If nonzero the first point is connected to the last point, i.e. the
	polyline is closed.\n\n
	<b>int *es</b>\n\n
	Edge state array. This is an array that Indicates if the 'i-th' edge is one
	of three state:\n\n
	<b>GW_EDGE_SKIP</b>\n\n
	Nonexistent - totally invisible.\n\n
	<b>GW_EDGE_VIS</b>\n\n
	Exists and is solid.\n\n
	<b>GW_EDGE_INVIS</b>\n\n
	Exists and is hidden - shown as a dotted line.\n\n
	You may pass NULL for this array and the method will assume that the edges
	are all solid. */
	virtual void	polylineN(int ct, Point3 *xyz, Point3 *nor, int closed, int *es) = 0;
	/*! \remarks	This method is used to begin efficiently sending a lot of 3D line segments.
	First call this method, then call <b>segment()</b> many times (with two
	points), then call <b>endSegments()</b>. */
	virtual void	startSegments() = 0;
	/*! \remarks	This method draws a single 3D line segment between the specified points.
	Call <b>startSegments()</b> once before calling this method.
	\par Parameters:
	<b>Point3 *xyz</b>\n\n
	Points to the two line endpoints in world space.\n\n
	<b>int vis</b>\n\n
	Nonzero for the segment to be visible; zero for invisible. */
	virtual void	segment(Point3 *xyz, int vis) = 0;
	/*! \remarks	Call this method after sending 3D line segments with <b>segment()</b>. */
	virtual void	endSegments() = 0;
	/*! \remarks Draws a multi-point polygon. Note: All arrays (<b>xyz, rgb,
	uvw</b>) must be at least one element larger than the <b>ct</b> parameter that
	is passed in. The 3D clipper will use the "extra" space to clip as efficiently
	as possible. If room for the extra element is not provided, 3ds Max may
	crash.\n\n
	Note: This method maps points from the GraphicsWindow's current transform to
	screen space. If the GraphicsWindow's transform is set to the identity matrix
	then the mapping is done from points specified in world space. Otherwise the
	points given are transformed by the GraphicsWindow transform, and are
	<b>then</b> considered to be in world space. Thus, to get a world-space to
	screen-space conversion, you need to set the transform to the identity with
	<b>gw-\>setTransform(Matrix3(1))</b>.
	\par Parameters:
	<b>int ct</b>\n\n
	The number of points in the polygon.\n\n
	<b>Point3 *xyz</b>\n\n
	Array of points.\n\n
	<b>Point3 *rgb</b>\n\n
	The color values at the vertices. The rendering mode must include
	<b>GW_ILLUM</b> for these values to be used.\n\n
	Note: The use of these colors is not supported under the OpenGL driver. The rgb
	values are ignored. Only the current material is taken into consideration.
	(This is how OpenGL works.)\n\n
	<b>Point3 *uvw</b>\n\n
	The UVW coordinates. The rendering mode must include <b>GW_TEXTURE</b> for
	these values to be used. */
	virtual void 	polygon(int ct, Point3 *xyz, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;
	/*! \remarks	Draws a multi-point polygon. Note: All arrays (<b>xyz, nor, uvw</b>) must be at
	least one element larger than the <b>ct</b> parameter that is passed in. The 3D
	clipper will use the "extra" space to clip as efficiently as possible. If room
	for the extra element is not provided, 3ds Max will crash.\n\n
	This method sends in normal vectors instead of color for 3D accelerated
	rendering (when <b>GW_SPT_GEOM_ACCEL</b> is TRUE)\n\n
	Note: This method maps points from the GraphicsWindow's current transform to
	screen space. If the GraphicsWindow's transform is set to the identity matrix
	then the mapping is done from points specified in world space. Otherwise the
	points given are transformed by the GraphicsWindow transform, and are
	<b>then</b> considered to be in world space. Thus, to get a world-space to
	screen-space conversion, you need to set the transform to the identity with
	<b>gw-\>setTransform(Matrix3(1))</b>.
	\par Parameters:
	<b>int ct</b>\n\n
	The number of points in the polygon.\n\n
	<b>Point3 *xyz</b>\n\n
	Array of points.\n\n
	<b>Point3 *nor</b>\n\n
	The normal values at the vertices, one for each vertex.\n\n
	<b>Point3 *uvw</b>\n\n
	The UVW coordinates. The rendering mode must include <b>GW_TEXTURE</b> for
	these values to be used. */
	virtual void 	polygonN(int ct, Point3 *xyz, Point3 *nor, Point3 *uvw, int texNum=1) = 0;
	/*! \remarks	This method is used for drawing a series of triangles specified as 'strips'. It
	takes a count of 3 or more, and builds triangles in a strip. This sends a lot
	less data and the underlying graphics library has to set up a lot less data
	since it can use the previous information to start the rasterization. This
	results in a significant speed increase.
	\par Parameters:
	<b>int ct</b>\n\n
	The total number of points. After the first two points, each new point is used
	to create a new triangle.\n\n
	<b>IPoint3 *xyz</b>\n\n
	The point data. For instance, to draw a quad, the first three points specify
	the first triangle and the next one is combined with the previous two to
	complete the square.\n\n
	The order for these points follows the 'standard' conventions for stripping
	used in most graphics libraries (for example Direct3D, OpenGL and Heidi).\n\n
	<b>Point3 *rgb</b>\n\n
	The colors for the vertices.\n\n
	Note: The use of these colors is not supported under the OpenGL driver. The rgb
	values are ignored. Only the current material is taken into consideration.
	(This is how OpenGL works.)\n\n
	<b>Point3 *uvw</b>\n\n
	The UVW texture coordinates for the vertices. */
	virtual void	triStrip(int ct, Point3 *xyz, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;
	/*! \remarks	This method is used for drawing a series of triangles specified as 'strips'. It
	takes a count of 3 or more, and builds triangles in a strip. This sends a lot
	less data and the underlying graphics library has to set up a lot less data
	since it can use the previous information to start the rasterization. This
	results in a significant speed increase. This method sends in normal vectors
	instead of color for 3D accelerated rendering (when <b>GW_SPT_GEOM_ACCEL</b> is
	TRUE)
	\par Parameters:
	<b>int ct</b>\n\n
	The total number of points. After the first two points, each new point is used
	to create a new triangle.\n\n
	<b>Point3 *xyz</b>\n\n
	The point data. For instance, to draw a quad, the first three points specify
	the first triangle and the next one is combined with the previous two to
	complete the square.\n\n
	The order for these points follows the 'standard' conventions for stripping
	used in most graphics libraries (for example Direct3D, OpenGL and Heidi).\n\n
	<b>Point3 *nor</b>\n\n
	The normal for each vertex.\n\n
	<b>Point3 *uvw</b>\n\n
	The UVW texture coordinates for the vertices. */
	virtual void	triStripN(int ct, Point3 *xyz, Point3 *nor, Point3 *uvw, int texNum=1) = 0;
	/*! \remarks	This method is called to begin sending a series of non-stripped triangles
	to render. Call this method, then any of the <b>triangle*()</b> methods
	many times, then <b>endTriangles()</b> to finish. */
	virtual void	startTriangles() = 0;
	/*! \remarks	This method sends a single non-stripped triangle to render. Call
	<b>startTriangles()</b> first.\n\n
	Note: This method maps points from the GraphicsWindow's current transform
	to screen space. If the GraphicsWindow's transform is set to the identity
	matrix then the mapping is done from points specified in world space.
	Otherwise the points given are transformed by the GraphicsWindow transform,
	and are <b>then</b> considered to be in world space. Thus, to get a
	world-space to screen-space conversion, you need to set the transform to
	the identity with <b>gw-\>setTransform(Matrix3(1))</b>.
	\par Parameters:
	<b>Point3 *xyz</b>\n\n
	The three points for the triangle.\n\n
	<b>Point3 *rgb</b>\n\n
	The color for each vertex.\n\n
	Note: The use of these colors is not supported under the OpenGL driver. The
	rgb values are ignored. Only the current material is taken into
	consideration. (This is how OpenGL works.) */
	virtual void	triangle(Point3 *xyz, Point3 *rgb) = 0;
	/*! \remarks	This method draws a single triangle by specifying the vertex points in world
	space, a normal, and texture coordinates for each vertex.\n\n
	Note: This method maps points from the GraphicsWindow's current transform to
	screen space. If the GraphicsWindow's transform is set to the identity matrix
	then the mapping is done from points specified in world space. Otherwise the
	points given are transformed by the GraphicsWindow transform, and are
	<b>then</b> considered to be in world space. Thus, to get a world-space to
	screen-space conversion, you need to set the transform to the identity with
	<b>gw-\>setTransform(Matrix3(1))</b>.
	\par Parameters:
	<b>Point3 *xyz</b>\n\n
	The three points for the triangle.\n\n
	<b>Point3 *nor</b>\n\n
	The three normals for the triangle.\n\n
	<b>Point3 *uvw</b>\n\n
	The texture coordinate for each vertex. */
	virtual void	triangleN(Point3 *xyz, Point3 *nor, Point3 *uvw, int texNum=1) = 0;
	/*! \remarks	This method draws a single triangle by specifying the vertex points in
	world space, a normal, and a color for each vertex.\n\n
	Note: This method maps points from the GraphicsWindow's current transform
	to screen space. If the GraphicsWindow's transform is set to the identity
	matrix then the mapping is done from points specified in world space.
	Otherwise the points given are transformed by the GraphicsWindow transform,
	and are <b>then</b> considered to be in world space. Thus, to get a
	world-space to screen-space conversion, you need to set the transform to
	the identity with <b>gw-\>setTransform(Matrix3(1))</b>.
	\par Parameters:
	<b>Point3 *xyz</b>\n\n
	The three points for the triangle.\n\n
	<b>Point3 *nor</b>\n\n
	The normal for each vertex.\n\n
	<b>Point3 *rgb</b>\n\n
	The color for each vertex. */
	virtual void	triangleNC(Point3 *xyz, Point3 *nor, Point3 *rgb) = 0;
	/*! \remarks	This method draws a single triangle by specifying the vertex points in world
	space, a normal, a color, and a texture coordinate for each vertex.\n\n
	Note: This method maps points from the GraphicsWindow's current transform to
	screen space. If the GraphicsWindow's transform is set to the identity matrix
	then the mapping is done from points specified in world space. Otherwise the
	points given are transformed by the GraphicsWindow transform, and are
	<b>then</b> considered to be in world space. Thus, to get a world-space to
	screen-space conversion, you need to set the transform to the identity with
	<b>gw-\>setTransform(Matrix3(1))</b>.
	\par Parameters:
	<b>Point3 *xyz</b>\n\n
	The three points for the triangle.\n\n
	<b>Point3 *nor</b>\n\n
	The normal for each vertex.\n\n
	<b>Point3 *rgb</b>\n\n
	The color for each vertex.\n\n
	<b>Point3 *uvw</b>\n\n
	The texture coordinate for each vertex. */
	virtual void	triangleNCT(Point3 *xyz, Point3 *nor, Point3 *rgb, Point3 *uvw, int texNum=1) = 0;
	virtual void	triangleW(Point3 *xyz, int *es) = 0;
	virtual void	triangleNW(Point3 *xyz, Point3 *nor, int *es) = 0;
	/*! \remarks	Call this method to finish rendering triangles. See <b>startTriangles()</b>
	above. */
	virtual void	endTriangles() = 0;
	virtual void	loadMeshData(DWORD_PTR id, int xyzCt, Point3 *xyz, int norCt, Point3 *nor, int texNum, int uvwCt, Point3 *uvw, int mtlCt, Material *mtl) = 0;
	virtual void	processStrips(DWORD_PTR id, int stripCt, StripTab *s, GFX_ESCAPE_FN fn) = 0;
	virtual void	processWireFaces(int xyzCt, Point3 *xyz, int faceCt, GWFace *face, int dispFlags, BitArray *faceSel, BitArray *edgeSel, int mtlCt, Material *mtl, GFX_ESCAPE_FN fn) = 0;

	/*! \remarks Sets the hit region used for hit testing. See
	Class HitRegion.
	\par Parameters:
	<b>HitRegion *rgn</b>\n\n
	The hit region to use. */
	virtual void	setHitRegion(HitRegion *rgn) = 0;
	/*! \remarks This methods clears the hit code. Call this method before
	performing a hit test. */
	virtual void	clearHitCode(void) = 0;
	/*! \remarks Returns TRUE if the hit code is set indicating a hit was
	made; otherwise FALSE. */
	virtual BOOL	checkHitCode(void) = 0;
	/*! \remarks	This method allows drawing code to manually set the state of the hit code,
	which is returned by the <b>checkHitCode()</b> method. For more information
	see the topic on
	<a href="ms-its:3dsmaxsdk.chm::/vports_hit_testing.html">Hit
	Testing</a>.\n\n
	The new methods <b>setHitDistance()</b> and <b>setHitCode()</b> make it
	possible to work with GraphicsWindow hit-testing in otherwise impossible
	situations. Why are they necessary? An example from is shown below. The
	patch object contains bezier spline-based edges which can consist of up to
	102 vertices. Since the GraphicsWindow::polyline function can only plot
	lines with up to 32 vertices, it is impossible to plot these in a single
	call to the polyline function. Multiple calls to the polyline call do not
	return a proper hitcode when using a "window"-type hit region. By using the
	new <b>setHitCode()</b> method, code can properly handle this situation.
	The code below shows the function in use from the PatchMesh::renderEdge method:
	\code	
	int steps = GetMeshSteps();
	int segNum = steps+2;
	float fsegNum = (float) (segNum-1);
	// If steps are too high for GraphicsWindow's buffer, we must draw it
	manually
	if((steps + 2) > GW_MAX_VERTS) {
	Point3 line[2];
	Point3 prev,current(.0f,.0f,.0f);
	BOOL hitAll = TRUE;
	BOOL hitAny = FALSE;
	DWORD hitDist = 0xffffffff;
	for(int terp = 0; terp \
	{
	prev = current;
	current = work.InterpCurve3D((float)terp / fsegNum);
	if (terp != 0)
	{
	line[0] = prev;
	line[1] = current;
	gw->clearHitCode();
	gw->polyline(2, line, NULL, NULL, 0, NULL);
	if(gw->checkHitCode()) {
	hitAny = TRUE;
	if(gw->getHitDistance() \
	hitDist = gw->getHitDistance();
	}
	else hitAll = FALSE;
	}
	}
	if(hr && !hr->crossing && hr->type != POINT_RGN)
	gw->setHitCode(hitAll);
	else
	gw->setHitCode(hitAny);
	gw->setHitDistance(hitDist);
	} else {
	for(int terp = 0; terp \
	fixedBuf[terp] = work.InterpCurve3D((float)terp /
	fsegNum);
	gw->polyline(steps+2, fixedBuf, NULL, NULL, 0, NULL);
	}
	\endcode 
	Note that the <b>gw-\>polyline</b> call is preceded by a call to
	<b>clearHitCode()</b>, and followed by code which checks the hit code,
	maintaining "hitAny" and "hitAll" flags. When all the segments are drawn,
	the <b>gw-\>setHitCode()</b> call is made, setting the hit code depending
	on the hit region type. When the code which called this function checks the
	GraphicsWindow's hit code, it will contain the proper value. This code also
	keeps track of the closest hit distance and places that into the
	GraphicsWindow when all line segments are drawn.
	\par Parameters:
	<b>BOOL h</b>\n\n
	Set to TRUE if the hit code is set, otherwise FALSE. */
	virtual void	setHitCode(BOOL h) = 0;
	/*! \remarks If <b>checkHitCode()</b> returns TRUE you may call this
	method to return the hit distance. In wireframe mode this is the distance
	to the line. In shaded mode, this is the z distance. This allows you to
	perform 'smart' hit testing by choosing the item with the smallest hit
	distance. This method only returns meaningful values when the hit region is
	a point. */
	virtual DWORD	getHitDistance(void) = 0;
	/*! \remarks	This method allows drawing code to manually set the hit distance, which is
	returned by the <b>getHitDistance()</b> method. For more information see
	the topic on <a href="ms-its:3dsmaxsdk.chm::/vports_hit_testing.html">Hit
	Testing</a>.
	\par Parameters:
	<b>DWORD d</b>\n\n
	In wireframe mode this is the distance to the line. In shaded mode, this is
	the z distance. */
	virtual void	setHitDistance(DWORD d)	= 0;

	/*! \remarks Returns TRUE if the view is in perspective projection;
	otherwise FALSE (orthographic projection). */
	virtual int		isPerspectiveView(void) = 0;
	/*! \remarks This method is used internally. */
	virtual float	interpWorld(Point3 *world1, Point3 *world2, float sParam, Point3 *interpPt) = 0;

	//watje
	virtual void			MarkerBufferSetMarkerType( MarkerType type) {};
	virtual DWORD			MarkerGetDXColor(Point3 p) {return 0;};

	virtual VertexBuffer*	MarkerBufferLock() { return NULL;};
	virtual void			MarkerBufferUnLock() {};
	virtual int				MarkerBufferSize( ) {return 0;};
	virtual int				MarkerBufferStride( ){ return 0;};
	virtual void			MarkerBufferDraw(int numberOfMarkers) {};

	//watje
	virtual DWORD			LineGetDXColor(Point3 p) {return 0;};

	virtual LineBuffer*	LineBufferLock() { return NULL;};
	virtual void			LineBufferUnLock() {};
	virtual int				LineBufferSize( ) {return 0;};
	virtual int				LineBufferStride( ){ return 0;};
	virtual void			LineBufferDraw(int numberOfSegments) {};

	virtual void	escape(GFX_ESCAPE_FN fn, void *data) = 0;
	/*! \remarks	This method calculates the depth value of the screen pixel located at positon x,y.
	This method returns TRUE if the depth value is calculated, FALSE otherwise.
	It is currently supported in Direct3D and OpenGL driver. Thus returning FALSE for unsupported
	driver. This method is used to locate center pivot in SteeringWheel.
	\param[in] x The x coordinate in screen space. \n
	\param[in] y The y coordinate in screen space. \n
	\param[out] z A pointer to the buffer that receives the calculated depth value.
	\return TRUE if calculated correctly, FALSE if the method failed or not supported.
	*/
	virtual BOOL	getDepthValue(float x, float y, float* z) { return FALSE; }
	/*! \remarks This method will clear the depth buffer bits of this GraphicsWindow.
	It is currently supported in Direct3D and OpenGL driver. This method is used
	clear the depth before actually drawing the ViewCube, which cannot be messed up 
	with the scene geometry.
	*/
	virtual void	clearViewportDepth() {};

	/*! \remarks	This take a GFX_MESH::HWTupleMesh and creates a hardware specific mesh buffers
	\param[in]GFX_MESH::HWTupleMesh *hwMesh the mesh used to create the Hardware specific buffers.
	\return the hardware specific IHWDrawMesh.	*/
	virtual  GFX_MESH::IHWSubMesh* CreateHWDrawMesh(GFX_MESH::HWTupleMesh* hwMesh) {return NULL;};

	/*! \remarks	This draws the hwMesh to the display.  If the hardware mesh holds a valid IHWDrawMesh
	it will use that to draw the mesh in retained mode.  Otherwise the buffers in the hwmesh will be used
	to draw the mesh in immediate mode which is much slower.
	\param[in]GFX_MESH::HWTupleMesh *hwMesh the mesh to be drawn. */
	virtual  void DrawHWDrawMesh(GFX_MESH::HWTupleMesh* hwMesh) {};

};
#pragma warning(pop)


