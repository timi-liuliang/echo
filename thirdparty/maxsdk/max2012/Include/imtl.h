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
// FILE:        imtl.h
// DESCRIPTION: Renderer materials
// AUTHOR:      Dan Silva 
// HISTORY:      
//**************************************************************************/

#pragma once


#include "maxheap.h"
#include "plugapi.h"
#include "RenderElements.h"

//#include "gport.h" 
#include "custcont.h" 
#include "shape.h"
#include "buildver.h"
#include "autoptr.h"
#include "MtlBaseList.h"
#include "quat.h"

// forward declarations
class TextureInfo;
class MNMesh;
class TimeChangeCallback;
class Renderer;
class ViewParams;
class ShadeContext;
class Bitmap;
class RenderMapsContext; 
class Object;
class UVGen;
class XYZGen;
class Sampler;

#define PROJ_PERSPECTIVE 0   // perspective projection
#define PROJ_PARALLEL 1      // parallel projection

#define AXIS_UV 0
#define AXIS_VW 1
#define AXIS_WU 2

/*! \defgroup textureSymmetryFlags Texture Symmetry Flags
\sa Class StdUVGen, Class TexHandleMaker */
//@{
#define U_WRAP   (1<<0) //!< Indicates the texture map is tiled in the U direction.
#define V_WRAP   (1<<1) //!< Indicates the texture map is tiled in the V direction.
#define U_MIRROR (1<<2) //!< Indicates the texture map is mirrored in the U direction.
#define V_MIRROR (1<<3) //!< Indicates the texture map is mirrored in the V direction.
#define UV_NOISE (1<<4)
#define UV_NOISE_ANI (1<<5)
//@}


#define  X_AXIS 0
#define  Y_AXIS 1
#define  Z_AXIS 2

static inline float Intens(const AColor& c) {   return (c.r+c.g+c.b)/3.0f; }
static inline float Intens(const Color& c) { return (c.r+c.g+c.b)/3.0f; }

// Meta-materials post this message to the MtlEditor when user
// clicks on a sub-material button.
#define WM_SUB_MTL_BUTTON  WM_USER + 0x04001

// Materials or Texture maps post this message to the MtlEditor when user
// clicks on a texture map button.
#define WM_TEXMAP_BUTTON   WM_USER + 0x04002

// Postage Stamp images ----------
// GetImage fills an array RGB triples.
// Width() and Height() return the dimensions of the image in pixels. 
// The width in bytes of the array is given by the following macro, where w is pixel width. 

#define PS_SMALL 0
#define PS_LARGE 1
#define PS_TINY  2

#define PS_TINY_SIZE  24  // size of the tiny postage stamps
#define PS_SMALL_SIZE 32  // size of the small postage stamps
#define PS_LARGE_SIZE 88  // size of the large postage stamps
#define PSDIM(isz) ((isz==0)?PS_SMALL_SIZE:(isz==1)?PS_LARGE_SIZE:PS_TINY_SIZE)

#define ByteWidth(w) (((w*3+3)/4)*4)

#pragma warning(push)
#pragma warning(disable:4100)

/*! \sa  Class AnimProperty.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is used internally by the Material / Map Browser which supports the
display of small and large icon images for material and texture maps. This
class is the postage stamp image object.\n\n
The width in bytes of the image pixel array is given by the following macro,
where w is pixel width.\n\n
<b>#define ByteWidth(w) (((w*3+3)/4)*4)</b>  */
class PStamp: public AnimProperty {
	 public:
	 /*! \remarks Returns the width of the image in pixels. */
	 virtual int Width()=0;
	 /*! \remarks Returns the height of the image in pixels. */
	 virtual int Height()=0;
	 /*! \remarks Sets the image for the postage stamp.
	 \par Parameters:
	 <b>UBYTE *img</b>\n\n
	 This is an array of RGB triplets. */
	 virtual void SetImage(UBYTE *img)=0;
	 /*! \remarks Retrieves the image bytes of the postage stamp.
	 \par Parameters:
	 <b>UBYTE *img</b>\n\n
	 This is an array of RGB triplets. */
	 virtual void GetImage(UBYTE *img)=0;
	 /*! \remarks This method is called to delete this instance of the class. */
	 virtual void DeleteThis()=0;
	 /*! \remarks This method is used to load the postage stamp image. */
	 virtual IOResult Load(ILoad *iload)=0;
	 /*! \remarks This method is used to save the postage stamp image. */
	 virtual IOResult Save(ISave *isave)=0;
	 };

/*! \sa  Class TexHandleMaker.\n\n
\par Description:
This class defines a texture handle. A pointer to an instance of this class is
returned from the methods of <b>TexHandleMaker</b>. Methods of this class allow
the handle to be retrieved and to delete the handle.  */
class TexHandle: public MaxHeapOperators {
	 public:
			/*! \remarks Destructor. */
			virtual ~TexHandle() {;}
		/*! \remarks This method is called to retrieve the texture handle. */
			virtual DWORD_PTR GetHandle() = 0;
			/*! \remarks This method is called to delete the instance of the texture handle. */
			virtual void DeleteThis() = 0;
	 };    

// Values for extraFlags:
#define EX_MULT_ALPHA     1    // if alpha is NOT premultiplied in the Bitmap, set this 
#define EX_RGB_FROM_ALPHA 2    // make map using alpha to define gray level 
#define EX_OPAQUE_ALPHA   4    // make map using opaque alpha 
#define EX_ALPHA_FROM_RGB 8    // make alpha from intensity 

/*! \sa  Class TexHandle, Class Bitmap, Class BitmapInfo.\n\n
\par Description:
This class provides several ways to create a texture handle. The handle may be
created from a 3ds Max bitmap or a Windows Device Independent Bitmap. This
class also provides methods to determine the desired size of the bitmap. <br>
*/
class TexHandleMaker: public MaxHeapOperators{
	 public: 
			/*! \remarks Destructor. */
			virtual ~TexHandleMaker() {;}
			// choice of two ways to create a texture handle.
			// From a 3DSMax Bitmap
			/*! \remarks Implemented by the System\n\n
			This method is called to create a texture handle from a 3ds Max bitmap.
			\par Parameters:
			<b>Bitmap *bm</b>\n\n
			The bitmap to create a handle to.\n\n
			<b>int symflags=0</b>\n\n
			See \ref textureSymmetryFlags.\n\n
			<b>int extraFlags=0;</b>\n\n
			One of the following values:\n\n
			<b>EX_MULT_ALPHA</b>\n\n
			Set this flag if alpha is <b>not</b> premultiplied in the Bitmap.\n\n
			<b>EX_RGB_FROM_ALPHA</b>\n\n
			Set this flag to make the map using the alpha channel of the bitmap to
			define the gray level.\n\n
			<b>EX_OPAQUE_ALPHA</b>\n\n
			Specifies to make the map using opaque alpha.\n\n
			<b>EX_ALPHA_FROM_RGB</b>\n\n
			Specifies to make alpha from the intensity of the map.
			\return  A pointer to the texture handle. */
			virtual TexHandle* CreateHandle(Bitmap *bm, int symflags=0, int extraFlags=0)=0;

			// From a 32 bit DIB
			/*! \remarks Implemented by the System\n\n
			This method is called to create a texture handle from a 32 bit Windows
			Device Independent Bitmap.
			\par Parameters:
			<b>BITMAPINFO *bminf</b>\n\n
			The bitmap to create a handle to.\n\n
			<b>int symflags=0</b>\n\n
			See \ref textureSymmetryFlags.\n\n
			<b>int extraFlags=0;</b>\n\n
			One of the following values:\n\n
			<b>EX_MULT_ALPHA</b>\n\n
			Set this flag if alpha is <b>not</b> premultiplied in the Bitmap.\n\n
			<b>EX_RGB_FROM_ALPHA</b>\n\n
			Set this flag to make the map using the alpha channel of the bitmap to
			define the gray level.\n\n
			<b>EX_OPAQUE_ALPHA</b>\n\n
			Specifies to make the map using opaque alpha.\n\n
			<b>EX_ALPHA_FROM_RGB</b>\n\n
			Specifies to make alpha from the intensity of the map.
			\return  A pointer to the texture handle. */
			virtual TexHandle* CreateHandle(BITMAPINFO *bminf, int symflags=0, int extraFlags=0)=0;

			// Create an 32 bit DIB with symmetry, extraflags already incorporated.
			// iff forceW is non-zero, it is used as the width of the final DIB 
			// iff forceH is non-zero, it is used as the height of the final DIB 
			/*! \remarks			This method creates a 32 bit Windows Device Independent Bitmap with the
			specified symflags and extraflags already incorporated and returns a
			pointer to the associated BitmapInfo.
			\par Parameters:
			<b>Bitmap *bm</b>\n\n
			Points to the bitmap to create the handle to.\n\n
			<b>int symflags</b>\n\n
			See \ref textureSymmetryFlags.\n\n
			<b>int extraFlags</b>\n\n
			One of the following values:\n\n
			<b>EX_MULT_ALPHA</b>\n\n
			Set this flag if alpha is <b>not</b> premultiplied in the Bitmap.\n\n
			<b>EX_RGB_FROM_ALPHA</b>\n\n
			Set this flag to make the map using the alpha channel of the bitmap to
			define the gray level.\n\n
			<b>EX_OPAQUE_ALPHA</b>\n\n
			Specifies to make the map using opaque alpha.\n\n
			<b>EX_ALPHA_FROM_RGB</b>\n\n
			Specifies to make alpha from the intensity of the map.\n\n
			<b>BOOL forceW=0</b>\n\n
			If this parameter is non-zero it is used as the width of the final
			DIB.\n\n
			<b>BOOL forceH=0</b>\n\n
			If this parameter is non-zero it is used as the height of the final DIB
			*/
			virtual BITMAPINFO *BitmapToDIB(Bitmap *bm, int symflags, int extraFlags, BOOL forceW=0, BOOL forceH=0)=0;

			// From a 32 bit DIB that has symflags, extraflags already incorporated.
			/*! \remarks			Returns a texture handle made from a 32 bit DIB that has the symflags,
			and extraflags already incorporated. This takes ownership of the
			BITMAPINFO*.
			\par Parameters:
			<b>BITMAPINFO* bminf</b>\n\n
			Points to the BitmapInfo for the DIB. */
			virtual TexHandle* MakeHandle(BITMAPINFO*bminf)=0;  // this takes ownership of the BITMAPINFO*

			// This tells you the size desired of the bitmap. It ultimately
			// needs a bitmap that is a power of 2 in width and height.  If 
			// UseClosestPowerOf2 returns TRUE, then the bitmap need not be square.
			// If you already have a bitmap around, just pass it in to CreateHandle
			// and it will be converted.  If you are making a bitmap from scratch
			// (i.e.) a procedural texture, then you should make it Size() in 
			// width in height, and save us an extra step.  In either case
			// You own your bitmap, and are responsible for ultimately freeing it.

			/*! \remarks			Returns TRUE if the bitmap does not need to be square and FALSE if it
			does need to be square. */
			virtual BOOL UseClosestPowerOf2()=0;
			/*! \remarks Implemented by the System.\n\n
			This method may be called to determine the desired size for the bitmap.
			The system ultimately needs a square bitmap that is a power of 2 in width
			and height. If you already have a bitmap around, just pass it in to
			<b>CreateHandle()</b> and it will be converted. If you are creating a
			bitmap from scratch (i.e. a procedural texture), then you should make it
			<b>Size()</b> in width in height, and save the system an extra step. In
			either case you own your bitmap, and are responsible for ultimately
			freeing it.
			\return  The size of the desired bitmap. */
			virtual int Size()=0;
	 };


// Callback used with the new multi-texture interface by function SetGfxMtl. 
/*! \sa  Class TexHandleMaker, Class TextureInfo, Class MtlBase, Class Texmap, Class Material.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is the callback used with the new multi-texture interface provided by
method <b>MtlBase::SetupGfxMultiMaps()</b>.  */
class MtlMakerCallback: public TexHandleMaker {
	 public:
	 // This fills all the fields of texinf exept the texhandle and the texture ops.
	 // note TexInfo is defined in mlt.h
	 /*! \remarks This method updates all the fields of the TextureInfo instance
	 exept the texture handle and the texture ops.
	 \par Parameters:
	 <b>TimeValue t</b>\n\n
	 The time at which the texture is evaluated.\n\n
	 <b>TextureInfo\& texinf</b>\n\n
	 The texture info which is updated.\n\n
	 <b>Texmap *txm</b>\n\n
	 Points to the texmap as the source for the texture info update. */
	 virtual void GetGfxTexInfoFromTexmap(TimeValue t, TextureInfo& texinf, Texmap *txm)=0;
	 /*! \remarks This method returns the number of textures that the
	 hardware+driver supports. */
	 virtual BOOL NumberTexturesSupported()=0; // how many textures does the hardware+driver support
	 };


// passed to SetPickMode. This is a callback that gets called as
// the user tries to pick objects in the scene.
/*! \sa  Class IMtlParams, Class INode.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is the callback object passed to <b>IMtlParams::SetPickMode()</b>. It gets
set so the user can pick objects in the scene. Its methods allow for filtering
the picks, changing cursors over valid hits, and allowing multiple picks.\n\n
All methods of this class are virtual.  */
class PickObjectProc: public MaxHeapOperators {
	 public:
			/*! \remarks Destructor. */
			virtual ~PickObjectProc() {;}
			// Called when the user picks something.
			// return TRUE to end the pick mode.
			/*! \remarks This method is called when the user picks something.
			\par Parameters:
			<b>INode *node</b>\n\n
			This is the node that was selected.
			\return  Return TRUE to end the pick mode; FALSE to ignore the pick and
			to continue to allow the user to pick. */
			virtual BOOL Pick(INode *node)=0;

			// Return TRUE if this is an acceptable hit, FALSE otherwise.
			/*! \remarks This method is called to allow the callback to filter hits.
			It should return TRUE if this is an acceptable hit; FALSE otherwise.
			\par Parameters:
			<b>INode *node</b>\n\n
			This is the node that was selected. */
			virtual BOOL Filter(INode *node)=0;

			// These are called as the mode is entered and exited
			/*! \remarks This method is called when the pick mode has been entered.
			The developer may provide any pre-processing here.
			\par Default Implementation:
			<b>{}</b> */
			virtual void EnterMode() {}
			/*! \remarks This method is called when the pick mode has been exited.
			The developer may provide any post-processing here.
			\par Default Implementation:
			<b>{}</b> */
			virtual void ExitMode() {}

			// Provides two cursor, 1 when over a pickable object and 1 when not.
			/*! \remarks This method is called to get the default (non-hit) cursor
			to use.
			\return  The handle of the non-pickable object cursor or NULL if the
			default cursor should be used.
			\par Default Implementation:
			<b>{return NULL;}</b> */
			virtual HCURSOR GetDefCursor() {return NULL;}
			/*! \remarks This method is called to get the pickable object cursor to
			use.
			\return  The handle of the pickable object cursor or NULL if the default
			cursor should be used.
			\par Default Implementation:
			<b>{return NULL;}</b> */
			virtual HCURSOR GetHitCursor() {return NULL;}

			// Return TRUE to allow the user to pick more than one thing.
			// In this case the Pick method may be called more than once.
			/*! \remarks This method determines if multiple objects can be picked.
			Returning TRUE allows the user to pick more than one thing. In this case
			the <b>Pick()</b> method may be called more than once.
			\par Default Implementation:
			<b>{return FALSE;}</b> */
			virtual BOOL AllowMultiSelect() {return FALSE;}
	 };


// Interface that is passed in to the Mtl or Texture Map when it is in the mtl
// editor.
/*! \sa  Class Mtl, <a href="ms-its:3dsmaxsdk.chm::/mtls_materials.html">Working with Materials</a>, Class IRollupWindow.\n\n
\par Description:
This is the interface that is passed in to the material or texture map when it
is in the material editor. All methods of this class are implemented by the
system.  */
class IMtlParams : public InterfaceServer {
	 public:
	 // call after mouse up's in mtls params
	 // It causes the viewports to be redrawn.
	 /*! \remarks This method may be called to causes the viewports to be
	 redrawn. It should be called when any parameter that affects the look of the
	 material in the viewport has been altered. If the material is not on a
	 visible node in a shaded view, nothing will happen. This method should NOT
	 be called as a spinner is being dragged, but only upon release of the mouse
	 button. */
	 virtual void MtlChanged()=0;  

	 // Adds rollup pages to the Material Params. Returns the window
	 // handle of the dialog that makes up the page.
	 /*! \remarks This method may be called to add a rollup page to the material
	 editor dialog. It returns the window handle of the dialog that makes up the
	 rollup page.
	 \par Parameters:
	 <b>HINSTANCE hInst</b>\n\n
	 The DLL instance handle of the plug-in.\n\n
	 <b>MCHAR *dlgTemplate</b>\n\n
	 The dialog template for the rollup page.\n\n
	 <b>DLGPROC dlgProc</b>\n\n
	 The dialog proc to handle the message sent to the rollup page.\n\n
	 <b>MCHAR *title</b>\n\n
	 The title displayed in the title bar.\n\n
	 <b>LPARAM param=0</b>\n\n
	 Any specific data to pass along may be stored here. This may be later
	 retrieved using the GetWindowLong() call from the Windows API.\n\n
	 <b>DWORD flags=0</b>\n\n
	 The following flag is defined:\n\n
	 <b>APPENDROLL_CLOSED</b>\n\n
	 Starts the page in the rolled up state.\n\n
	 <b>int category = ROLLUP_CAT_STANDARD</b>\n\n
	 The category parameter provides flexibility with regard to where a
	 particular rollup should be displayed in the UI. RollupPanels with lower
	 category fields will be displayed before RollupPanels with higher category
	 fields. For RollupPanels with equal category value the one that was added
	 first will be displayed first. Although it is possible to pass any int
	 value as category there exist currently 5 different category defines:
	 <b>ROLLUP_CAT_SYSTEM</b>, <b>ROLLUP_CAT_STANDARD</b>, and
	 <b>ROLLUP_CAT_CUSTATTRIB</b>.\n\n
	 When using <b>ROLLUP_SAVECAT</b>, the rollup page will make the provided
	 category sticky, meaning it will not read the category from the
	 <b>RollupOrder.cfg</b> file, but rather save the category field that was
	 passed as argument in the <b>CatRegistry</b> and in the
	 <b>RollupOrder.cfg</b> file.\n\n
	 The method will take the category of the replaced rollup in case the flags
	 argument contains <b>ROLLUP_USEREPLACEDCAT</b>. This is mainly done, so that
	 this system works with param maps as well.
	 \return  The window handle of the rollup page. */
	 virtual HWND AddRollupPage( HINSTANCE hInst, MCHAR *dlgTemplate, 
			DLGPROC dlgProc, MCHAR *title, LPARAM param=0,DWORD flags=0, int category = ROLLUP_CAT_STANDARD )=0;

	 /*! \remarks	 This method may be called to add a rollup page to the material editor
	 dialog. It returns the window handle of the dialog that makes up the rollup
	 page. This method is currently not used.
	 \par Parameters:
	 <b>HINSTANCE hInst</b>\n\n
	 The DLL instance handle of the plug-in.\n\n
	 <b>DLGTEMPLATE *dlgTemplate</b>\n\n
	 The dialog template for the rollup page.\n\n
	 <b>DLGPROC dlgProc</b>\n\n
	 The dialog proc to handle the message sent to the rollup page.\n\n
	 <b>MCHAR *title</b>\n\n
	 The title displayed in the title bar.\n\n
	 <b>LPARAM param=0</b>\n\n
	 Any specific data to pass along may be stored here. This may be later
	 retrieved using the GetWindowLong() call from the Windows API.\n\n
	 <b>DWORD flags=0</b>\n\n
	 The following flag is defined:\n\n
	 <b>APPENDROLL_CLOSED</b>\n\n
	 Starts the page in the rolled up state.\n\n
	 <b>int category = ROLLUP_CAT_STANDARD</b>\n\n
	 The category parameter provides flexibility with regard to where a
	 particular rollup should be displayed in the UI. RollupPanels with lower
	 category fields will be displayed before RollupPanels with higher category
	 fields. For RollupPanels with equal category value the one that was added
	 first will be displayed first. Although it is possible to pass any int
	 value as category there exist currently 5 different category defines:
	 <b>ROLLUP_CAT_SYSTEM</b>, <b>ROLLUP_CAT_STANDARD</b>, and
	 <b>ROLLUP_CAT_CUSTATTRIB</b>.\n\n
	 When using <b>ROLLUP_SAVECAT</b>, the rollup page will make the provided
	 category sticky, meaning it will not read the category from the
	 <b>RollupOrder.cfg</b> file, but rather save the category field that was
	 passed as argument in the <b>CatRegistry</b> and in the
	 <b>RollupOrder.cfg</b> file.\n\n
	 The method will take the category of the replaced rollup in case the flags
	 argument contains <b>ROLLUP_USEREPLACEDCAT</b>. This is mainly done, so that
	 this system works with param maps as well.
	 \return  The window handle of the rollup page. */
	 virtual HWND AddRollupPage( HINSTANCE hInst, DLGTEMPLATE *dlgTemplate, 
			DLGPROC dlgProc, MCHAR *title, LPARAM param=0,DWORD flags=0, int category = ROLLUP_CAT_STANDARD )=0;


	 // Adds rollup pages to the Material Params. Returns the window
	 // handle of the dialog that makes up the page.
	 /*! \remarks	 This method is used to replace the rollup page whose index is passed.
	 \par Parameters:
	 <b>HWND hOldRollup</b>\n\n
	 The handle to the rollup to replace.\n\n
	 <b>HINSTANCE hInst</b>\n\n
	 The DLL instance handle of the plug-in.\n\n
	 <b>MCHAR *dlgTemplate</b>\n\n
	 The dialog template for the rollup page.\n\n
	 <b>DLGPROC dlgProc</b>\n\n
	 The dialog proc to handle the message sent to the rollup page.\n\n
	 <b>MCHAR *title</b>\n\n
	 The title displayed in the title bar.\n\n
	 <b>LPARAM param=0</b>\n\n
	 Any specific data to pass along may be stored here.\n\n
	 <b>DWORD flags=0</b>\n\n
	 Append rollup page flags:\n\n
	 <b>APPENDROLL_CLOSED</b>\n\n
	 Starts the page in the rolled up state.\n\n
	 <b>int category = ROLLUP_CAT_STANDARD</b>\n\n
	 The category parameter provides flexibility with regard to where a
	 particular rollup should be displayed in the UI. RollupPanels with lower
	 category fields will be displayed before RollupPanels with higher category
	 fields. For RollupPanels with equal category value the one that was added
	 first will be displayed first. Although it is possible to pass any int
	 value as category there exist currently 5 different category defines:
	 <b>ROLLUP_CAT_SYSTEM</b>, <b>ROLLUP_CAT_STANDARD</b>, and
	 <b>ROLLUP_CAT_CUSTATTRIB</b>.\n\n
	 When using <b>ROLLUP_SAVECAT</b>, the rollup page will make the provided
	 category sticky, meaning it will not read the category from the
	 <b>RollupOrder.cfg</b> file, but rather save the category field that was
	 passed as argument in the <b>CatRegistry</b> and in the
	 <b>RollupOrder.cfg</b> file.\n\n
	 The method will take the category of the replaced rollup in case the flags
	 argument contains <b>ROLLUP_USEREPLACEDCAT</b>. This is mainly done, so that
	 this system works with param maps as well.
	 \return  The handle of the replacement page is returned. */
	 virtual HWND ReplaceRollupPage( HWND hOldRollup, HINSTANCE hInst, MCHAR *dlgTemplate, 
			DLGPROC dlgProc, MCHAR *title, LPARAM param=0,DWORD flags=0, int category = ROLLUP_CAT_STANDARD  )=0;

	 /*! \remarks	 This method is used to replace the rollup page whose index is passed. This
	 method is currently not being used.
	 \par Parameters:
	 <b>HWND hOldRollup</b>\n\n
	 The handle to the rollup to replace.\n\n
	 <b>HINSTANCE hInst</b>\n\n
	 The DLL instance handle of the plug-in.\n\n
	 <b>DLGTEMPLATE *dlgTemplate</b>\n\n
	 The dialog template for the rollup page.\n\n
	 <b>DLGPROC dlgProc</b>\n\n
	 The dialog proc to handle the message sent to the rollup page.\n\n
	 <b>MCHAR *title</b>\n\n
	 The title displayed in the title bar.\n\n
	 <b>LPARAM param=0</b>\n\n
	 Any specific data to pass along may be stored here.\n\n
	 <b>DWORD flags=0</b>\n\n
	 Append rollup page flags:\n\n
	 <b>APPENDROLL_CLOSED</b>\n\n
	 Starts the page in the rolled up state.\n\n
	 <b>int category = ROLLUP_CAT_STANDARD</b>\n\n
	 The category parameter provides flexibility with regard to where a
	 particular rollup should be displayed in the UI. RollupPanels with lower
	 category fields will be displayed before RollupPanels with higher category
	 fields. For RollupPanels with equal category value the one that was added
	 first will be displayed first. Although it is possible to pass any int
	 value as category there exist currently 5 different category defines:
	 <b>ROLLUP_CAT_SYSTEM</b>, <b>ROLLUP_CAT_STANDARD</b>, and
	 <b>ROLLUP_CAT_CUSTATTRIB</b>.\n\n
	 When using <b>ROLLUP_SAVECAT</b>, the rollup page will make the provided
	 category sticky, meaning it will not read the category from the
	 <b>RollupOrder.cfg</b> file, but rather save the category field that was
	 passed as argument in the <b>CatRegistry</b> and in the
	 <b>RollupOrder.cfg</b> file.\n\n
	 The method will take the category of the replaced rollup in case the flags
	 argument contains <b>ROLLUP_USEREPLACEDCAT</b>. This is mainly done, so that
	 this system works with param maps as well.
	 \return  The handle of the replacement page is returned. */
	 virtual HWND ReplaceRollupPage( HWND hOldRollup, HINSTANCE hInst, DLGTEMPLATE *dlgTemplate, 
			DLGPROC dlgProc, MCHAR *title, LPARAM param=0,DWORD flags=0, int category = ROLLUP_CAT_STANDARD  )=0;

	 // Removes a rollup page and destroys it.  When a dialog is destroyed
	 // it need not delete all its rollup pages: the Mtl Editor will do
	 // this for it, and it is more efficient.
	 /*! \remarks This method may be called to remove a rollup page and destroy
	 it.
	 \par Parameters:
	 <b>HWND hRollup</b>\n\n
	 The handle of the rollup window. This is the handle returned from
	 <b>AddRollupPage()</b>. */
	 virtual void DeleteRollupPage( HWND hRollup )=0;

	 // When the user mouses down in dead area, the plug-in should pass
	 // mouse messages to this function which will pass them on to the rollup.
	 /*! \remarks This method allows hand cursor scrolling when the user clicks
	 the mouse in an unused area of the dialog. When the user mouses down in dead
	 area of the material editor, the plug-in should pass mouse messages to this
	 function.\n\n
	 Note: In 3ds Max 2.0 and later only use of this method is no longer required
	 -- the functionality happens automatically.
	 \par Parameters:
	 <b>HWND hDlg</b>\n\n
	 The window handle of the dialog.\n\n
	 <b>UINT message</b>\n\n
	 The message sent to the dialog proc.\n\n
	 <b>WPARAM wParam</b>\n\n
	 Passed in to the dialog proc. Pass along to this method.\n\n
	 <b>LPARAM lParam</b>\n\n
	 Passed in to the dialog proc. Pass along to this method.
	 \par Example:
	 <b>case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_MOUSEMOVE:</b>\n\n
	 <b>im-\>iMtlParams-\>RollupMouseMessage(hDlg,message,wParam,lParam);</b> */
	 virtual void RollupMouseMessage( HWND hDlg, UINT message, 
						WPARAM wParam, LPARAM lParam )=0;

	 /*! \remarks Returns nonzero if the rollup page whose handle is passed is
	 open; otherwise zero.
	 \par Parameters:
	 <b>HWND hwnd</b>\n\n
	 The window handle of the rollup page to check. */
	 virtual int IsRollupPanelOpen(HWND hwnd)=0;
	 /*! \remarks Returns the rollup scroll position. This is used, for example,
	 by the Standard material because it saves and restores the rollup page
	 positions with the material. This is just a convenience for the user. */
	 virtual int GetRollupScrollPos()=0;
	 /*! \remarks This method may be called to set the rollup scroll position.
	 If the position was previously saved, this method may be called to restore
	 it. The Standard material uses this method because it saves and restores the
	 rollup positions with the material as a convenience for the user.
	 \par Parameters:
	 <b>int spos</b>\n\n
	 Specifies the rollup scroll position. */
	 virtual void SetRollupScrollPos(int spos)=0;

	 /*! \remarks	 Registers a callback object that will get called every 
	 time the current animation time is changed.
	 \par Parameters:
	 <b>TimeChangeCallback *tc</b>\n\n
	 The callback called when the time changes. See
	 Class TimeChangeCallback. */
	 virtual void RegisterTimeChangeCallback(TimeChangeCallback *tc)=0;
	 /*! \remarks	 This method un-registers the time change callback.
	 \par Parameters:
	 <b>TimeChangeCallback *tc</b>\n\n
	 The callback called when the time changes. See
	 Class TimeChangeCallback. */
	 virtual void UnRegisterTimeChangeCallback(TimeChangeCallback *tc)=0;

	 // Registers a dialog window so IsDlgMesage() gets called for it.
	 // This is done automatically for Rollup Pages.
	 /*! \remarks This is called automatically in <b>AddRollupPage()</b>, so a
	 plug-in doesn't need to do it. It is now obsolete.
	 \par Parameters:
	 <b>HWND hDlg</b>\n\n
	 The handle of the dialog window. */
	 virtual void RegisterDlgWnd( HWND hDlg )=0;
	 /*! \remarks This method is called automatically and is now obsolete.
	 \par Parameters:
	 <b>HWND hDlg</b>\n\n
	 The handle of the dialog window. */
	 virtual int UnRegisterDlgWnd( HWND hDlg )=0;

	 // get the current time.
	 /*! \remarks Returns the current time (the frame slider position).
	 \return  The current time.\n\n
	 Pick an object from the scene */
	 virtual TimeValue GetTime()=0;   

	 // Pick an object from the scene
	 /*! \remarks	 Calling this methods puts the user into a pick mode where they can select
	 items in the scene.
	 \par Parameters:
	 <b>PickObjectProc *proc</b>\n\n
	 This is the callback object for the picking. Its methods allow for filtering
	 the picks, changing cursors over valid hits, and allowing multiple picks.
	 See Class PickObjectProc. */
	 virtual void SetPickMode(PickObjectProc *proc)=0;
	 /*! \remarks	 This method is called to terminate the pick mode. */
	 virtual void EndPickMode()=0;

	 // JBW 10/19/98: get interface to mtl editor rollup
	 /*! \remarks	 This method returns an interface to Materials Editor rollup. */
	 virtual IRollupWindow *GetMtlEditorRollup()=0;

	 // Generic expansion function
	 virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 
	 };

class ParamDlg;
class Texmap;

/*! \sa  <a href="class_i_node.html#A_GM_inode_renderdata">Class INode-Access To Render Data</a>, 
Class LightDesc.\n\n
\par Description:
This class has a single virtual method to delete the class. For example, when
<b>LightDesc</b> objects are deleted this is the method to do so.  */
class RenderData : public InterfaceServer {
	 public:
		/*! \remarks This method is used to delete an instance of the class.
		\par Default Implementation:
		<b>{delete this; }</b> */
		virtual void DeleteThis()=0;
		/*! \remarks		This is a general purpose function that allows the API to be extended in the
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
			virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0;} 
	 };

/*! \sa  Class RenderData, Class LightRayTraversal, Class ObjLightDesc, Class ShadeContext, Class Color,  Class Point3.\n\n
\par Description:
This class has a method <b>Illuminate()</b> used to determine the color and
direction of the light striking the point <b>sc.P()</b> and a method to get the
position of the light. It also has two public data members that determine if
the diffuse and specular colors of objects are affected.
\par Data Members:
<b>BOOL affectDiffuse;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
If TRUE the light affects the diffuse color; otherwise it does not.\n\n
<b>BOOL affectSpecular;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
If TRUE the light affects the specular color; otherwise it does not.\n\n
<b>BOOL ambientOnly;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
If TRUE the light affects the ambient color only; otherwise it does not.\n\n
<b>DWORD extra;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
This is not currently used and is available for use in the future.  */
class LightDesc : public RenderData {
	 public:
			BOOL affectDiffuse;
			BOOL affectSpecular;
			BOOL  ambientOnly;
			DWORD  extra;
			/*! \remarks Constructor. The <b>affectDiffuse</b> and
			<b>affectSpecular</b> data members are set to TRUE. */
			LightDesc() { affectDiffuse = affectSpecular = TRUE; ambientOnly = FALSE; }
			// determine color and direction of illumination: return FALSE if light behind surface.
			// also computes dot product with normal dot_nl, and diffCoef which is to be used in
			// place of dot_n for diffuse light computations.
			 /*! \remarks This method is called to determine the color and direction of
			 illumination from the light that is hitting the point (<b>sc.P()</b>).
			 \par Parameters:
			 <b>ShadeContext\& sc</b>\n\n
			 Describes the properties of the point to shade. The point itself is
			 <b>sc.P()</b>.\n\n
			 <b>Point3\& normal</b>\n\n
			 The normal to the surface in camera space.\n\n
			 <b>Color\& color</b>\n\n
			 The color that is returned. This is the brightness of light striking the
			 point (<b>sc.P()</b>)\n\n
			 <b>Point3 \&dir</b>\n\n
			 The direction that the light is coming from.\n\n
			 <b>float \&dot_nl</b>\n\n
			 This provides a bit of optimization as most lights will have to
			 calculate this quantity. This is the dot product between the light
			 vector and the normal to the surface.\n\n
			 <b>float \&diffuseCoef</b>\n\n
			 This parameter should be set by the Illuminate function. The default
			 value is the same as dot_nl. It will be used by shading functions
			 instead of dot_nl to compute the diffuse illumination of the surface.
			 The built-in lights use the new "Contrast" parameter (which has a range
			 of [0..100]) to compute the diffuseCoef from the dot_nl by the Contrast function:
			\code
			// precomputed:
			float a = contrast/200.0f + 0.5f;				  // so "a" varies from .5 to 1.0
			kA = (2.0f-1.0f/a);
			kB = 1.0f-kA;
			
			// called by Illuminate() to compute diffuseCoef from dot_nl.
			float ContrastFunc(float nl)
			{
			// the "Bias" function described in Graphics Gems IV, pp. 401ff
				return (contrast==0.0f)? nl : nl/(kA*nl+kB);
			}
			\endcode 
			 \return  Returns FALSE if the hitpoint is outside the effective range of
			 the light or if the normal of the surface faces away from the light.
			 This is a hint to the material that the light did not calculate its
			 illumination because it is assumed it wasn't going to be used. If TRUE
			 the point is being illuminated.
			 \par Default Implementation:
			 <b>{ return 0;}</b> */
			 virtual BOOL Illuminate(ShadeContext& sc, Point3& normal, Color& color, Point3 &dir, float &dot_nl, float &diffuseCoef){ return 0;}
			
			/*! \remarks			Returns the position of the light.
			\par Default Implementation:
			<b>{ return Point3(0,0,0); }</b> */
			virtual Point3 LightPosition() { return Point3(0,0,0); } 

			CoreExport virtual void DeleteThis();

	 };

#define SHADELIM_FLAT 1
#define SHADELIM_GOURAUD 2
#define SHADELIM_PHONG 3

// Transform Reference frames: 
enum RefFrame { REF_CAMERA=0, REF_WORLD, REF_OBJECT };

#define N_MAX_RENDER_ELEMENTS 32

/*! \sa  Class ShadeContext, Class Color.\n\n
\par Description:
An instance of this class is a data member of the <b>ShadeContext</b>. This is
used to contain the computed color and transparency of the pixel being shaded
by a material. All methods of this class are implemented by the system.
\par Data Members:
<b>ULONG flags;</b>\n\n
These flags are not currently used.\n\n
<b>Color c;</b>\n\n
Shaded color of the pixel.\n\n
<b>Color t;</b>\n\n
Transparency of the pixel.\n\n
<b>float ior;</b>\n\n
Index of refraction of the pixel.\n\n
<b>int gbufId;</b>\n\n
The G-buffer ID. This allows the <b>MixIn()</b> method to pick the id of the
material which was blended in the highest proportion.  */
class ShadeOutput : public BaseInterfaceServer {
	 public:
			ULONG flags;
			Color c;  // shaded color
			Color t;  // transparency
			float ior;  // index of refraction
			int gbufId;

			// render elements
			int nElements;
			AColor   elementVals[ N_MAX_RENDER_ELEMENTS ];

			/*! \remarks This method is used to blend the output of two texmaps, for
			example, in the Mix texmap. The function is as follows:\n
			\code
			void ShadeOutput::MixIn(ShadeOutput &a, float f)
			{
				if (f<=0.0f) {
					(*this) = a;
					return;
				}
				else if (f>=1.0f) {
					return;
				}
				else {
					float s = 1.0f - f;
					flags |= a.flags;
					c = s*a.c + f*c;
					t = s*a.t + f*t;
					ior = s*a.ior + f*ior;
					if (f<=0.5f) gbufId = a.gbufId;
				}
			}
			\endcode 
			This does a blend of <b>a</b> with <b>(*this)</b>. This blend is applied
			to the color, transparency, and index of refraction. The flags of are
			OR'ed together.
			\par Parameters:
			<b>ShadeOutput\& a</b>\n\n
			The output of the texmap to blend in.\n\n
			<b>float f</b>\n\n
			The amount to blend in, i.e.:\n\n
			<b>a.MixIn(b, 1.0f)</b> results in 100% of <b>a</b> and none of <b>b</b>.
			*/
			CoreExport void MixIn(ShadeOutput& a, float f);  // (*this) =  (1-f)*(*this) + f*a;
			ShadeOutput():nElements(0), flags(0), gbufId(0){};
			ShadeOutput( int nEle ):flags(0), gbufId(0){ nElements = nEle; }
			/*! \remarks Implemented by the System.\n\n
			This method resets the data member such that: <b>c</b> is set to black,
			<b>t</b> is set to black, <b>ior</b> is set to 1.0, <b>gbufId</b> is set to 0,
			and the <b>flags</b> are set to 0.
			\par Parameters:
			<b>int n = -1</b>\n\n
			By supplying a negative value this method will clear elements but leave the
			number of elements unchanged. */
			void Reset( int nEle = -1 )
			{ 
				 flags = 0;
				 gbufId = 0;
				 c.Black(); t.Black(); ior = 1.0f;
				 // clear nEle's worth of render elements
				 if (nEle >= 0)
						nElements = nEle;
				 for( int i=0; i < nElements; ++i )
						elementVals[i].r = elementVals[i].g =
							 elementVals[i].b = elementVals[i].a = 0.0f;
			}
			CoreExport ShadeOutput* Clone();       

			// Render Element interface
			// allocate new val, return index
			int NewElementVal(){
				 DbgAssert((nElements+1)<N_MAX_RENDER_ELEMENTS); 
				 return nElements++; 
			}
			int NElementVals(){ return nElements; }
			// override new val & set nElements directly, for clear mostly
			void SetNElements( int n ){
				 DbgAssert( n>=0 && n<N_MAX_RENDER_ELEMENTS); 
				 nElements = n;
			}
			// return rgba value of element n
			AColor ElementVal( int n ){ 
				 DbgAssert( n>=0 && n<N_MAX_RENDER_ELEMENTS); 
				 return elementVals[n]; 
			}
			// set rgba value of element n
			void SetElementVal( int n, AColor c ){ 
				 DbgAssert( n>=0 && n<N_MAX_RENDER_ELEMENTS); 
				 elementVals[n] = c; 
			}
			// sum rgba value intof element n
			void AccumElementVal( int n, AColor c ){ 
				 DbgAssert( n>=0 && n<N_MAX_RENDER_ELEMENTS); 
				 elementVals[n] += c; 
			}
			// sum all color components of a shade output
			void Sum( ShadeOutput* pSample ){
				 c += pSample->c; t += pSample->t;
				 for( int i = 0; i < nElements; ++i )
						elementVals[i] += pSample->elementVals[i];
			}
			void Scale( float x ){
				 c *= x; t *= x;
				 for( int i = 0; i < nElements; ++i )
						elementVals[i] *= x;
			}
			// sum all color components, w/o alpha of a shade output
			void SumClr( ShadeOutput* pSample ){
				 c += pSample->c; 
				 for( int i = 0; i < nElements; ++i ){
						elementVals[i].r += pSample->elementVals[i].r;
						elementVals[i].g += pSample->elementVals[i].g;
						elementVals[i].b += pSample->elementVals[i].b;
				 }
			}

			ShadeOutput& operator += ( ShadeOutput& a ){
				 c += a.c;  
				 t += a.t; 
				 for( int i = 0; i < nElements; ++i )
						elementVals[i] += a.elementVals[i];
				 return *this;
			}
			ShadeOutput& operator *= ( Color x ){
				 c *= x; // t *= x; 
				 AColor ac( x );
				 ac.a = Intens( x );
				 for( int i = 0; i < nElements; ++i )
						elementVals[i] *= ac;
				 return *this;
			}

			ShadeOutput& operator = ( ShadeOutput& a ){
				 flags = a.flags;
				 c = a.c; t = a.t; 
				 ior = a.ior;
				 gbufId = a.gbufId;
				 nElements = a.nElements;
				 for( int i = 0; i < nElements; ++i )
						elementVals[i] = a.elementVals[i];
				 return *this;
			}


	 };


class RenderInstance;

/*! \sa Class RenderGlobalContext, Class RenderInstance, Class ISectList.
\remarks This structure is available in release 2.0 and later only. \n
This structure is updated by the RenderGlobalContext::IntersectWorld() and 
RenderGlobalContext::IntersectRay() methods. It stores information about 
the intersection of a ray and a single triangle in a mesh.
*/
struct ISect: public MaxHeapOperators {
	 /*! The ray that was intersected has a unit vector specifying a direction.
	 The ray defines an infinite line in the specified direction. If you take
	 this value t and multiply it by the direction itself it gives a point on
	 the ray. This specifies a distance along the vector. For instance if
	 this way 5.0, the point would 5.0 units along the ray vector. This
	 is the point of intersection along the ray.  */
	 float t;   // ray parameter
	 /*! TRUE if the ray is exiting the object; otherwise FALSE.  */
	 BOOL exit;
	 /*! TRUE if the ray hits a back face of the triangle; otherwise FALSE.  */
	 BOOL backFace;
	 /*! Points to the render instance associated with this triangle hit.  */
	 RenderInstance *inst;
	 /*! The face number of the triangle.  */
	 int fnum;  // face number
	 /*! The barycentric coordinates of the intersection.  */
	 Point3 bc; // barycentric coords
	 /*! The intersection point in object coordinates.  */
	 Point3 p;  // intersection point, object coords
	 /*! The intersection point in camera coordinates.  */
	 Point3 pc;  // intersection point, camera coords
	 /*! The material requirements of the intersected face. See \ref materialRequirementsFlags.  */
	 ULONG matreq;
	 /*! The material number for a multi-material.  */
	 int mtlNum;
	 /*! Points to the next ISect structure in the list.  */
	 ISect *next;
	 };

/*! \sa  Structure ISect.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
It provides a list of ray / triangle intersection structures (<b>struct
ISect</b>). Methods are available for initializing the list, adding to the
list, and selectively removing items from the list.\n\n
All methods of this class are implemented by the system.\n\n
Note the following global functions may be used to create and free instances of
this class. */
class ISectList: public MaxHeapOperators {
	 public:
	 ISect *first;
	 /*! \remarks Constructor. The list is set to empty. */
	 ISectList() { first = NULL; }
	 /*! \remarks Destructor. The list is freed. <b>first</b> is set to NULL. */
	 ~ISectList() { Init(); }
	 /*! \remarks Returns TRUE if the list is empty; otherwise FALSE. */
	 BOOL IsEmpty() { return first?0:1; }
	 /*! \remarks Adds the specified <b>ISect</b> to the list with the most
	 distant layers first.
	 \par Parameters:
	 <b>ISect *is</b>\n\n
	 Points to the <b>ISect</b> structure to add to the list. */
	 CoreExport void Add(ISect *is); 
	 /*! \remarks Removes the <b>ISect</b> structures from the list whose
	 <b>t</b> values are less than or equal to the specified value <b>a</b>.
	 \par Parameters:
	 <b>float a</b>\n\n
	 The ray parameter used for comparison. */
	 CoreExport void Prune(float a); 
	 /*! \remarks Deletes any items from the list and sets <b>first</b> to NULL.
	 */
	 CoreExport void Init();
	 };
	 
/*! \remarks This global function is available in release 2.0 and later
only.\n\n
Returns a pointer to a new <b>ISect</b> structure. */
CoreExport ISect *GetNewISect();
/*! \remarks This global function is available in release 2.0 and later
only.\n\n
Deletes the <b>ISect</b> structure whose pointer is passed.
\par Parameters:
<b>ISect *is</b>\n\n
Points to the <b>ISect</b> structure to free.
\par Data Members:
<b>ISect *first;</b>\n\n
A pointer to the first intersection.  */
CoreExport void DiscardISect(ISect *is);

// Value given to blurFrame for non blurred objects  ( Object motion blur)
#define NO_MOTBLUR (-1) 

class FilterKernel;
class IRenderElement;
class ToneOperator;
typedef  Tab<IRenderElement*> RenderElementTab;

/*! \sa  Class ShadeContext, Class Renderer, Class RenderInstance, Class Texmap, Class Atmospheric, Class Matrix3, Class Point2,  Class Point3,  Class AColor, Class Color, Class IRenderElementMgr, Class IRenderElement.\n\n
\par Description:
A pointer to an instance of this class is a data member of the ShadeContext
(<b>RenderGlobalContext *globContext;</b>). This can be used by materials,
texmaps, etc. to retrieve information about the global rendering environment.
This is information such as the renderer in use, the project type for
rendering, the output device width and height, several matrices for
transforming between camera and world coordinates, the environment map, the
atmospheric effects, the current time, field rendering information, and motion
blur information.\n\n
Note that raytracing (and all shading calculations in the default renderer)
take place in camera space.\n\n
When a ray intersects on the face edge it can happen that no intersection is
returned. One way to handle this situation is to perturb the ray minimally so
it will point in a slightly different direction. This presumes that you are
fairly sure that the no intersection is probably not what you're looking for.
<b>IntersectRay()</b> is linear in the number of faces so <b>NUM_ATTEMPTS</b>
should be kept small.\n\n
\code
for (perturb=0; perturb < NUM_ATTEMPTS; perturb++)
{
	Matrix3 ptb;
	float prop = ((float)rand()) / ((float) RAND_MAX);
	// gets random rotation of up to half a degree.
	float ang = PI*prop/360.0f;
	switch (perturb%3) {
		case 0: ptb = RotateXMatrix (ang); break;
		case 1: ptb = RotateYMatrix (ang); break;
		case 2: ptb = RotateZMatrix (ang); break;
	}
	ray.dir = ptb*ray.dir;
	// try IntersectRay() again, see if you get a hit.
}
\endcode 
\par Data Members:
<b>Renderer *renderer;</b>\n\n
A pointer to the active renderer.\n\n
<b>int projType;</b>\n\n
Returns the type of projection used during rendering. One of the following
values:\n\n
<b>PROJ_PERSPECTIVE</b>\n\n
<b>PROJ_PARALLEL</b>\n\n
<b>int devWidth;</b>\n\n
The width in pixels of the output device.\n\n
<b>int devHeight;</b>\n\n
The height in pixels of the output device.\n\n
<b>float xscale;</b>\n\n
The X scale factor for mapping from world space to screen space.\n\n
<b>float yscale;</b>\n\n
The Y scale factor for mapping from world space to screen space.\n\n
<b>float xc;</b>\n\n
The X center point used in mapping from world space to screen space.\n\n
<b>float yc;</b>\n\n
The Y center point used in mapping from world space to screen space.\n\n
<b>BOOL antialias;</b>\n\n
TRUE if antialiasing is enabled; otherwise FALSE.\n\n
<b>Matrix3 camToWorld;</b>\n\n
This matrix may be used to transform coordinates from camera space to world
space.\n\n
<b>Matrix3 worldToCam;</b>\n\n
This matrix may be used to transform coordinates from world space to camera
space.\n\n
<b>float nearRange;</b>\n\n
The near range setting of the camera.\n\n
<b>float farRange;</b>\n\n
The far range setting of the camera.\n\n
<b>float devAspect;</b>\n\n
The <b>pixel</b> aspect ratio of a device pixel. This is the height /
width.\n\n
<b>float frameDur;</b>\n\n
This defines the duration of one frame in floating point units. This is used,
for example, by video post rendering where the user can stretch time. A video
post frame might be 1/2 a frame long for instance.\n\n
<b>Texmap *envMap;</b>\n\n
The environment map (which may be NULL).\n\n
<b>Color globalLightLevel;</b>\n\n
This parameter is available in release 3.0 and later only.\n\n
This is the global light level.\n\n
<b>Atmospheric *atmos;</b>\n\n
The atmosphere effects (which may be NULL).\n\n
<b>ToneOperator* pToneOp;</b>\n\n
This data member is available in release 4.0 and later only.\n\n
The tone operator, may be NULL\n\n
<b>TimeValue time;</b>\n\n
The current time.\n\n
<b>BOOL wireMode;</b>\n\n
This parameter is available in release 3.0 and later only.\n\n
TRUE if rendering in wire frame mode; otherwise FALSE.\n\n
<b>float wire_thick;</b>\n\n
This parameter is available in release 3.0 and later only.\n\n
The global wire thickness.\n\n
<b>BOOL force2Side;</b>\n\n
TRUE if force two-sided rendering enabled; otherwise FALSE.\n\n
<b>BOOL inMtlEdit;</b>\n\n
TRUE if the rendering is being done in the materials editor; otherwise
FALSE.\n\n
<b>BOOL fieldRender;</b>\n\n
TRUE if field rendering is being done; otherwise FALSE.\n\n
<b>BOOL first_field;</b>\n\n
TRUE if this is the first field; FALSE if it's the second.\n\n
<b>BOOL field_order;</b>\n\n
Determines which field is first. 0 if the even first; 1 if odd first.\n\n
<b>BOOL objMotBlur;</b>\n\n
This is used for handling object motion blur in ray-trace maps and materials.
TRUE if object motion blur is enabled; FALSE if it's disabled.\n\n
<b>int nBlurFrames;</b>\n\n
This is used for handling object motion blur in ray-trace maps and materials.
The number of object motion blur time slices. See
Class RenderInstance.  */
class RenderGlobalContext : public BaseInterfaceServer
{
protected:
	 IRenderElementMgr *mpIRenderElementMgr; // current render element manager

public:
	 Renderer* renderer;
	 int projType;  // PROJ_PERSPECTIVE or PROJ_PARALLEL
	 int devWidth, devHeight;
	 float xscale, yscale;
	 float xc,yc;
	 BOOL antialias;
	 Matrix3 camToWorld, worldToCam;
	 float nearRange, farRange;
	 float devAspect;          // PIXEL aspect ratio of device pixel H/W
	 float frameDur;
	 Texmap *envMap;
	 Color globalLightLevel;
	 Atmospheric *atmos;
	 ToneOperator* pToneOp;  // The tone operator, may be NULL
	 TimeValue time;
	 BOOL wireMode;    // wire frame render mode?
	 float wire_thick; // global wire thickness
	 BOOL force2Side;  // is force two-sided rendering enabled
	 BOOL inMtlEdit;      // rendering in mtl editor?
	 BOOL fieldRender; // are we rendering fields
	 BOOL first_field; // is this the first field or the second?
	 BOOL field_order; // which field is first: 0->even first,  1->odd first

	 BOOL objMotBlur;  // is object motion blur enabled
	 int nBlurFrames;  // number of object motion blur time slices

#ifdef SIMPLIFY_AREA_LIGHTS
	 bool simplifyAreaLights; //should area lights degrade to point lights
#endif


	 // Methods
	 RenderGlobalContext() : pToneOp(NULL), mpIRenderElementMgr(NULL) { }

	 // render element methods
	 /*! \remarks	 Returns a pointer to the Render Element Manager. See
	 Class IRenderElementMgr. */
	 IRenderElementMgr *GetRenderElementMgr() { return mpIRenderElementMgr; }
	 /*! \remarks	 Sets the render element manager being used.
	 \par Parameters:
	 <b>IRenderElementMgr *pIRenderElementMgr</b>\n\n
	 Points to the render element manager to set. */
	 void SetRenderElementMgr(IRenderElementMgr *pIRenderElementMgr) { mpIRenderElementMgr = pIRenderElementMgr; }
	 /*! \remarks	 Returns the number of render elements. */
	 int NRenderElements()
	 {
		return mpIRenderElementMgr ? mpIRenderElementMgr->NumRenderElements() : 0;
	 } 
	 /*! \remarks	 Returns a pointer to the specified render element (or NULL if not found).
	 \par Parameters:
	 <b>int n</b>\n\n
	 The zero based index of the render element. */
	 IRenderElement* GetRenderElement(int n)
	 {
		return mpIRenderElementMgr ? mpIRenderElementMgr->GetRenderElement(n) : NULL;
	 }
	 
	 /*! \remarks Computes the screen space coordinates of the point passed in
	 world coordinates. This is implemented as:
	 \par Parameters:
	 <b>Point3 p</b>\n\n
	 The point to map to screen space. */
	 Point2 MapToScreen(Point3 p)
	 {  
			if (projType==1)
			{
				 return Point2(  xc + xscale*p.x , yc + yscale*p.y);
			}
			else
			{
				 if (p.z==0.0f)
						p.z = .000001f;
				 return Point2( xc + xscale*p.x/p.z,  yc + yscale*p.y/p.z);
			}
	 }

	 /*! \remarks Returns a pointer to the current anti-aliasing filter from the
	 renderer. See Class FilterKernel. */
	 virtual FilterKernel* GetAAFilterKernel(){ return NULL; }
	 /*! \remarks Returns the filter size of the current anti-aliasing filter.
	 */
	 virtual float GetAAFilterSize(){ return 0.0f; }

	 // Access RenderInstances
	 /*! \remarks Returns the number of RenderInstances. */
	 virtual int NumRenderInstances() { return 0; }
	 /*! \remarks Returns a pointer to the 'i-th' <b>RenderInstance</b>.
	 \par Parameters:
	 <b>int i</b>\n\n
	 Specifies which <b>RenderInstance</b> to return (<b>0</b> through
	 <b>NumRenderInstances()-1</b>). */
	 virtual RenderInstance* GetRenderInstance( int i ) { return NULL; }

	 // Evaluate the global environment map using ray as point of view,
	 // optionally with atmospheric effect.
	 /*! \remarks This method evaluates the global environment map using the
	 specified ray as a point of view, and returns the resulting color.
	 \par Parameters:
	 <b>ShadeContext \&sc</b>\n\n
	 The shade context.\n\n
	 <b>Ray \&r</b>\n\n
	 Defines the direction of view of the environment. See
	 Class Ray.\n\n
	 <b>BOOL applyAtmos</b>\n\n
	 TRUE if atmospheric effects should be considered; otherwise FALSE.
	 \par Default Implementation:
	 <b>{ return AColor(0.0f,0.0f,0.0f,1.0f); }</b> */
	 virtual AColor EvalGlobalEnvironMap(ShadeContext &sc, Ray &r, BOOL applyAtmos)
	 { 
			return AColor(0.0f,0.0f,0.0f,1.0f); 
	 }

	 /*! \remarks This method takes the specified <b>ray</b> and intersects it
	 with the single RenderInstance <b>inst</b>.
	 \par Parameters:
	 <b>RenderInstance *inst</b>\n\n
	 The render instance to intersect. The Mesh may be retrieved via <b>Mesh\& m
	 = *(inst-\>mesh);</b>\n\n
	 <b>Ray\& ray</b>\n\n
	 Defines the direction to check. This is the point to look from, and a normal
	 vector specifying the direction to look. See Class Ray.\n\n
	 <b>ISect \&isct</b>\n\n
	 The information about the first opaque object hit by the ray is returned
	 here. See Structure ISect.\n\n
	 <b>ISectList \&xpList</b>\n\n
	 The list of transparent objects that are intersected on the way to the
	 opaque one are returned here. See Class ISectList.\n\n
	 <b>BOOL findExit</b>\n\n
	 TRUE to compute the exit point; FALSE to not compute it. Once a ray has been
	 intersected with a transparent object and you want to find out where the
	 refracted ray leaves the object, this parameter may be set to TRUE. This
	 allows the ray to look at the inside faces of the object and compute the
	 intersection point at exit. */
	 virtual void IntersectRay(RenderInstance *inst, Ray& ray, ISect &isct, ISectList &xpList, BOOL findExit) {}
	 /*! \remarks This method takes the specified <b>ray</b> and intersects it
	 with the entire 3ds Max scene.
	 \par Parameters:
	 <b>Ray \&ray</b>\n\n
	 Defines the direction to check. This is the point to look from, and a normal
	 vector specifying the direction to look. See Class Ray.\n\n
	 <b>int skipID</b>\n\n
	 This specifies an ID (from <b>RenderInstance::nodeID</b>) that is skipped in
	 the intersection computations. This is used to prevent self
	 intersection.\n\n
	 <b>ISect \&hit</b>\n\n
	 The information about the first opaque object hit by the ray is returned
	 here. See Structure ISect.\n\n
	 <b>ISectList \&xplist</b>\n\n
	 The list of transparent objects that are intersected on the way to the
	 opaque one are returned here. See Class ISectList.\n\n
	 <b>int blurFrame = NO_MOTBLUR</b>\n\n
	 <b>NO_MOTBLUR</b> is used for non-motion blurred objects. If this is not
	 equal to <b>NO_MOTBLUR</b>, it should be in the range <b>0</b> to
	 <b>nBlurFrames-1</b>. In that case, this method will only consider blur
	 objects corresponding to that blur sub-frame.\n\n
	 When object motion blur is turned on, for each object, several objects are
	 generated. Each of these objects is given a number. This corresponds to the
	 value <b>RenderInstance::objMotBlurFrame</b>.\n\n
	 This method will always intersect objects that aren't motion blurred.
	 However, if this is set to a number other than <b>NO_MOTBLUR</b>, then when
	 it comes to a motion blurred object, it will only look at the sub-object
	 corresponding to the specified slice in time.\n\n
	 This may be used to do a kind of dither where for each of the sub-samples,
	 this number is randomly selected. In this way the different motion blur
	 slices will basically blur together and give a motion blurred ray trace
	 result. */
	 virtual BOOL IntersectWorld(Ray &ray, int skipID, ISect &hit, ISectList &xplist, int blurFrame = NO_MOTBLUR) { return FALSE; }

	 /*! \remarks	 Returns a pointer to a class which describes the properties of a view being
	 rendered.. See Class ViewParams. */
	 virtual ViewParams* GetViewParams() { return NULL; } 

	 /*! \remarks This method is used internally.
	 \par Default Implementation:
	 <b>{ return NULL; };</b> */
	 virtual FILE* DebugFile() { return NULL; }

	 // Generic expansion function
	/*! \remarks	This is a general purpose function that allows the API to be extended in the
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
	option is discussed for more details on the meaning of this value.
	\par Default Implementation:
	<b>{ return 0; }</b> */
	 virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 
};


// > 4/19/02 - 12:25am --MQM-- 
// PLUG-IN DEVELOPER NOTE:
//
//    In this release we are using the xshadeID in the
//    ShadeContext to store information during Light Tracer 
//    and VIZ Radiosity renderings.  The developer should be
//    sure to copy the xshadeID whenever creating new shade
//    contexts based off of an existing ShadeContext instance.
//
//    The renderer performs two passes when a radiosity
//    plugin is on.  The first pass is a very minimal pass,
//    and the second is the normal pass.  Shader plugins
//    can test for the pre-pass flag and possibly do less
//    (or no) work.
//
//    During the regular pass, the Light Tracer will max out
//    the rayLevel to halt raytraced reflections during regathering.  
//    Shaders should also not do specular shading or other things 
//    that will increase variance and/or slow regathering performance 
//    down. 

// we are inside of a regathering ray
#define SHADECONTEXT_REGATHERING_FLAG        0x80000000  
#define SHADECONTEXT_IS_REGATHERING( sc )    ( (sc).xshadeID & SHADECONTEXT_REGATHERING_FLAG )

// we are in a pre-pass in the scanline renderer
#define SHADECONTEXT_PREPASS_FLAG            0x40000000  
#define SHADECONTEXT_IS_PREPASS( sc )        ( (sc).xshadeID & SHADECONTEXT_PREPASS_FLAG )

// in recursive eval, so we don't try to 
// regather something that we're already 
// in the middle of...
#define SHADECONTEXT_RECURSIVE_EVAL_FLAG     0x20000000  
#define SHADECONTEXT_IS_RECURSIVE_EVAL( sc ) ( (sc).xshadeID & SHADECONTEXT_RECURSIVE_EVAL_FLAG )

// matte material needs a way of getting the shadow values out
// of light tracer.  (this is going away when we can change the sdk!!)
#define SHADECONTEXT_GUESS_SHADOWS_FLAG         0x10000000
#define SHADECONTEXT_GUESS_SHADOWS( sc )     ( (sc).xshadeID & SHADECONTEXT_GUESS_SHADOWS_FLAG )

// scanline renderer thread #
#define SHADECONTEXT_THREAD_MASK          0x0000F000  
#define SHADECONTEXT_THREAD_SHIFT            12
#define SHADECONTEXT_GET_THREAD( sc )        ( ( (sc).xshadeID & SHADECONTEXT_THREAD_MASK )  >> SHADECONTEXT_THREAD_SHIFT )
#define SHADECONTEXT_SET_THREAD( sc, thread )   { (sc).xshadeID = ( (sc).xshadeID & (~SHADECONTEXT_THREAD_MASK) )  |  ( ( (thread) << SHADECONTEXT_THREAD_SHIFT ) & SHADECONTEXT_THREAD_MASK ); }

// worker thread #
#define SHADECONTEXT_WORKER_MASK          0x00000F00
#define SHADECONTEXT_WORKER_SHIFT            8
#define SHADECONTEXT_GET_WORKER( sc )        ( ( (sc).xshadeID & SHADECONTEXT_WORKER_MASK )  >> SHADECONTEXT_WORKER_SHIFT )
#define SHADECONTEXT_SET_WORKER( sc, worker )   { (sc).xshadeID = ( (sc).xshadeID & (~SHADECONTEXT_WORKER_MASK) )  |  ( ( (worker) << SHADECONTEXT_WORKER_SHIFT ) & SHADECONTEXT_WORKER_MASK ); }

// bounce # (1..255, 0 not used)
#define SHADECONTEXT_BOUNCE_MASK          0x000000FF
#define SHADECONTEXT_GET_BOUNCE( sc )        ( (sc).xshadeID & SHADECONTEXT_BOUNCE_MASK )
#define SHADECONTEXT_SET_BOUNCE( sc, bounce )   { (sc).xshadeID = ( (sc).xshadeID & (~SHADECONTEXT_BOUNCE_MASK) )  |  ( (bounce) & SHADECONTEXT_BOUNCE_MASK ); }

// general macro to init regathering info in the shade context.
// this wipes out other flags in the xshadeID (prepass, etc)
#define SHADECONTEXT_SET_REGATHERING_INFO( sc, thread, worker, bounce ) \
{ (sc).xshadeID = SHADECONTEXT_REGATHERING_FLAG  |  \
	( ( (thread) << SHADECONTEXT_THREAD_SHIFT ) & SHADECONTEXT_THREAD_MASK )  |  \
	( ( (worker) << SHADECONTEXT_WORKER_SHIFT ) & SHADECONTEXT_WORKER_MASK )  |  \
	( (bounce) & SHADECONTEXT_BOUNCE_MASK ); }


#define SCMODE_NORMAL  0
#define SCMODE_SHADOW  1

//
// Shade Context: passed into Mtls and Texmaps
//
/*! \sa  <a href="ms-its:3dsmaxsdk.chm::/mtls_materials.html">Working with Materials</a>, 
Class LightDesc, Class RenderGlobalContext, Class Color, Class ShadeOutput, Class AColor, Class UVGen,  Class Box3,  Class IPoint2, Class Point2, Class IRenderElement, Class Point3, Class Class_ID.\n\n
\par Description:
This class is passed to materials and texture maps. It contains all the
information necessary for shading the surface at a pixel.\n\n
Normally, the <b>ShadeContext</b> is provided by the 3ds Max renderer. However
developers that need to create their own <b>ShadeContext</b> for use in passing
to the texture and material evaluation functions can do so by deriving a class
from <b>ShadeContext</b> and providing implementations of the virtual methods.
Some sample code is available demonstrating how this is done in
<b>/MAXSDK/SAMPLES/OBJECTS/LIGHT.CPP</b> (see the code for <b>class SCLight :
public ShadeContext</b>). The default implementations of these methods are
shown for developers that need to create their own <b>ShadeContext</b>.\n\n
Note that raytracing (and all shading calculations in the default renderer)
takes place in camera space.\n\n
For additional information on the methods <b>DP()</b>, <b>Curve()</b>,
<b>DUVW()</b> and <b>DPdUVW()</b> see
<a href="class_shade_context.html#A_GM_shadec_add">Additional Notes</a>.\n\n
All methods are implemented by the system unless noted otherwise.
\par Data Members:
<b>ULONG mode;</b>\n\n
One of the following values:\n\n
<b>SCMODE_NORMAL</b>\n\n
In normal mode, the material should do the entire illumination including
transparency, refraction, etc.\n\n
<b>SCMODE_SHADOW</b>\n\n
In shadow mode, you are just trying to find out what color the shadow is that
is falling on an object. In this case, all you care about is transmitted
color.\n\n
<b>BOOL doMaps</b>\n\n
Indicates if texture maps should be applied.\n\n
<b>BOOL filterMaps;</b>\n\n
Indicates if textures should be filtered.\n\n
<b>BOOL shadow</b>\n\n
Indicates if shadows should be applied.\n\n
<b>BOOL backFace;</b>\n\n
Indicates if we are on the back side of a 2-sided face.\n\n
<b>int mtlNum</b>\n\n
The material number of the face being shaded. This is the sub-material number
for multi-materials.\n\n
<b>Color ambientLight</b>\n\n
This is the color of the ambient light.\n\n
<b>int nLights;</b>\n\n
This is the number of lights being used in a render, which is the number of
active lights in the scene, or if there are none, 2 for the default lights. 
For example, this is used in the Standard material in a loop like this:\n\n
\code
LightDesc *l;
for (int i=0; i\<sc.nLights; i++) {
	l = sc.Light(i);
	..etc
}
\endcode
<b>int rayLevel;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
This is used to limit the number of reflections for raytracing. For instance,
if you're rendering a hall of mirrors, and the ray is reflecting back and
forth, you don't want the raytracing to go forever. Every time
<b>Texmap::EvalColor()</b> gets called again on a ray you create a new
<b>ShadeContext</b> and bump up the <b>rayLevel</b> one. This allows you to
test this value and see if it has reached the limit of how deep to go (if it
reaches a maximum level, you can return black for example).\n\n
Note that it is conceivable that more than one raytrace material can be in
effect at a time (from different developers). In such a case, where one surface
might have one raytracer and another surface a different one, and a ray was
bouncing back and forth between them, each needs to be aware of the other. This
is why this value is here -- the two texmaps each modify and check it.\n\n
<b>int xshadeID;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
This is currently not used.\n\n
<b>RenderGlobalContext *globContext;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
Points to an instance of <b>RenderGlobalContext</b>. This class describes the
properties of the global rendering environment. This provides information such
as the renderer in use, the project type for rendering, the output device width
and height, several matrices for transforming between camera and world
coordinates, the environment map, the atmospheric effects, the current time,
field rendering information, and motion blur information.\n\n
<b>LightDesc *atmosSkipLight;</b>\n\n
The light description of lights to prevent self shadowing by volumetric lights.\n\n
<b>RenderGlobalContext *globContext;</b>\n\n
A pointer to the rendering global context.\n\n
<b>ShadeOutput out;</b>\n\n
This is where the material should leave its results.\n\n
The following is a discussion of blending the ShadeContext.out.c and
ShadeContext.out.t together to get the final color:\n\n
The (c,t) returned by shaders is interpreted as follows: t.r is the
(premultiplied) alpha for the r-channel, etc.\n\n
So if you want to composite (c,t) over a background b,\n\n
color = b*t + c ( where the multiplication of b and t multiplies the individual
components ).\n\n
When you want to convert a (c,t) to a simple R,G,B,Alpha, just average together
the components of t to get Alpha. (and use the r,g,b components of c directly).
*/
class ShadeContext : public InterfaceServer {
public:
	ULONG mode;                   // normal, shadow ...
	BOOL doMaps;                  // apply texture maps?
	BOOL filterMaps;              // should texture be filtered                
	BOOL shadow;                  // apply shadows?
	BOOL backFace;                // are we on the back side of a 2-Sided face?
	int mtlNum;                   // sub-mtl number for multi-materials
	Color ambientLight;              // ambient light 
	int nLights;                  // number of lights;
	int rayLevel;
	int xshadeID;                    // different for every shade call in a render.
	LightDesc *atmosSkipLight;
	RenderGlobalContext *globContext;
	ShadeOutput out;                    // where a shading leaves its results

	// Reset output & render elements: 
	// param < 0 clears existing elements, leaving nElements unchanged
	// n >= 0 clears n elements & set nElements to n, default is 0
	/*! \remarks Sets the surface color output and surface transparency output to Black.
	\par Parameters:
	<b>int n = -1</b>\n\n
	By supplying a negative value this method will clear elements but leave the
	number of elements unchanged. */
	void ResetOutput( int n = -1 ) { out.Reset(n); }   

	/*! \remarks	 Returns the Class_ID of this ShadeContext. This is used to distinguish
	different ShadeContexts.
	\par Default Implementation:
	<b>{ return Class_ID(0,0); }</b> */
	virtual Class_ID ClassID() { return Class_ID(0,0); }  // to distinguish different ShadeContexts.
	/*! \remarks	 Returns TRUE if this rendering is for the material editor sample sphere
	(geometry); otherwise FALSE. */
	virtual BOOL InMtlEditor()=0; // is this rendering the mtl editor sample sphere?
	/*! \remarks Returns the state of the antialiasing switch in the renderer
	dialog - TRUE if on; FALSE if off.
	\par Default Implementation:
	<b>{return 0;}</b> */
	virtual int Antialias() {return 0;}
	/*! \remarks This method returns the projection type.
	\return  A value of 0 indicates perspective projection; a value of 1
	indicates parallel projection.
	\par Default Implementation:
	<b>{return 0;}</b> */
	virtual int ProjType() { return 0;} // returns: 0: perspective, 1: parallel
	/*! \remarks This method returns the 'i-th' light. Use data member
	<b>nLights</b> to get the total number of lights.
	\par Parameters:
	<b>int n</b>\n\n
	Specifies the light to return. */
	virtual LightDesc* Light(int n)=0;  // get the nth light. 
	/*! \remarks Returns the current time value (the position of the frame slider).
	\return  The current time. */
	virtual TimeValue CurTime()=0;      // current time value
	/*! \remarks Returns the node ID for the item being rendered or -1 if not
	set. This ID is assigned when the scene is being rendered - each node is
	simply given an ID - 0, 1, 2, 3, etc.
	\par Default Implementation:
	<b>{return -1;}</b> */
	virtual int NodeID() { return -1; }
	/*! \remarks Returns the INode pointer of the node being rendered. This
	pointer allows a developer to access the properties of the node. See
	Class INode.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual INode *Node() { return NULL; }
	/*! \remarks	 Returns the evaluated object for this node. When rendering, usually one
	calls <b>GetRenderMesh()</b> to get the mesh to render. However, at certain
	times you might want to get the object itself from the node. For example,
	you could then call <b>ClassID()</b> on the object and determine its type.
	Then the object could be operated on procedurally (for instance you could
	recognize it as a true sphere, cylinder or torus). Note that this method
	will return NULL if object is motion blurred.\n\n
	For example, here is how you can check if the object is a particle system:\n\n
	\code 
	// . . .
	Object *ob = sc.GetEvalObject();
	if (ob \&\& ob-\>IsParticleSystem()) { // . . . }
	\endcode
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual Object *GetEvalObject() { return NULL; } // Returns the evaluated object for this node. 
	// Will be NULL if object is motion blurred  
	/*! \remarks The coordinates relative to triangular face. The barycentric
	coordinates of a point <b>p</b> relative to a triangle describe that point
	as a weighted sum of the vertices of the triangle. If the barycentric
	coordinates are <b>b0, b1</b>, and <b>b2</b>, then:\n\n
	<b>p = b0*p0 + b1*p1 + b2*p2;</b>\n\n
	where <b>p0, p1</b>, and <b>p2</b> are the vertices of the triangle. The
	Point3 returned by this method has the barycentric coordinates stored in the
	its three coordinates. These coordinates are relative to the current
	triangular face being rendered. These barycentric coordinates can be used to
	interpolate <b>any</b> quantity whose value is known at the vertices of the
	triangle. For example, if a radiosity shader had available the illumination
	values at each of the three vertices, it could determine the illumination at
	the current point using the barycentric coordinates.
	\par Default Implementation:
	<b>{ return Point3(0,0,0);}</b> */
	virtual Point3 BarycentricCoords() { return Point3(0,0,0);}  // coords relative to triangular face 
	/*! \remarks Returns the index of the face being rendered. For the
	scan-line renderer, which renders only triangle meshes, this is the index of
	the face in the Mesh data structure. This is meant for use in plug-in
	utilities such as a radiosity renderer, which stores a table of data,
	indexed on face number, in the Nodes's AppData, for use in a companion
	material. */
	virtual int FaceNumber()=0;         // 
	/*! \remarks Returns the interpolated normal (in camera space). This is the
	value of the face normal facing towards the camera. This is affected by
	<b>SetNormal()</b> below. */
	virtual Point3 Normal()=0;       // interpolated surface normal, in camera coords: affected by SetNormal()
	/*! \remarks This method will set the value of the face normal facing
	towards the camera. This may be used to temporarily perturb the normal. The
	Standard material uses this for example because it implements bump mapping.
	It changes the normal and then calls other lighting functions, etc. These
	other method then see this changed normal value. When it is done it puts
	back the previous value.
	\par Parameters:
	<b>Point3 p</b>\n\n
	The normal to set. */
	virtual void SetNormal(Point3 p) {} // used for perturbing normal
	/*! \remarks	 Returns the original surface normal (not affected by <b>SetNormal()</b>
	above.)
	\par Default Implementation:
	<b>{ return Normal(); }</b> */
	virtual Point3 OrigNormal() { return Normal(); } // original surface normal: not affected by SetNormal();
	/*! \remarks This returns the geometric normal. For triangular mesh objects
	this means the face normal. Normals are unit vectors. */
	virtual Point3 GNormal()=0;      // geometric (face) normal
	/*! \remarks This is an estimate of how fast the normal is varying. For
	example if you are doing environment mapping this value may be used to
	determine how big an area of the environment to sample. If the normal is
	changing very fast a large area must be sampled otherwise you'll get
	aliasing. This is an estimate of <b>dN/dsx, dN/dsy</b> put into a single
	value. */
	virtual float  Curve() { return 0.0f; }          // estimate of dN/dsx, dN/dsy
	/*! \remarks This method returns the unit view vector, from the camera
	towards P, in camera space. */
	virtual Point3 V()=0;            // Unit view vector: from camera towards P 
	/*! \remarks	 Sets the view vector as returned by <b>V()</b>.
	\par Parameters:
	<b>Point3 p</b>\n\n
	The view vector set. */
	virtual void SetView(Point3 p)=0;   // Set the view vector
	/*! \remarks	 This is the original view vector that was not affected by
	<b>ShadeContext::SetView()</b>.
	\par Default Implementation:
	<b>{ return V(); }</b> */
	virtual Point3 OrigView() { return V(); } // Original view vector: not affected by SetView();
	/*! \remarks This takes the current view vector and the current normal
	vector and calculates a vector that would result from reflecting the view
	vector in the surface. This returns the reflection vector. */
	virtual  Point3 ReflectVector()=0;  // reflection vector
	/*! \remarks This is similar to the method above however it calculates the
	view vector being refracted in the surface. This returns the refraction
	vector.
	\par Parameters:
	<b>float ior</b>\n\n
	The relative index of refraction between the air and the material. */
	virtual  Point3 RefractVector(float ior)=0;  // refraction vector
	/*! \remarks	 Set index of refraction.
	\par Parameters:
	<b>float ior</b>\n\n
	The index of refraction to set. This value can be any positive (non-zero)
	value.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetIOR(float ior) {} // Set index of refraction
	/*! \remarks	 Returns the index of refraction.
	\par Default Implementation:
	<b>{ return 1.0f; }</b> */
	virtual float GetIOR() { return 1.0f; } // Get index of refraction
	/*! \remarks Returns the camera position in camera space. For the 3ds Max
	renderer this will always be 0,0,0. */
	virtual Point3 CamPos()=0;       // camera position
	/*! \remarks Returns the point to be shaded in camera space. */
	virtual Point3 P()=0;            // point to be shaded;
	/*! \remarks This returns the derivative of P, relative to the pixel. This
	gives the renderer or shader information about how fast the position is
	changing relative to the screen. */
	virtual Point3 DP()=0;           // deriv of P, relative to pixel, for AA
	/*! \remarks This returns the derivative of P, relative to the pixel - same
	as above. This method just breaks it down into x and y. */
	virtual void DP(Point3& dpdx, Point3& dpdy){};  // deriv of P, relative to pixel
	/*! \remarks Returns the point to be shaded in object coordinates. 
	*/
	virtual Point3 PObj()=0;            // point in obj coords
	/*! \remarks Returns the derivative of <b>PObj()</b>, relative to the pixel. 
	*/
	virtual Point3 DPObj()=0;           // deriv of PObj, rel to pixel, for AA
	/*! \remarks Returns the object extents bounding box in object coordinates.
	*/
	virtual Box3 ObjectBox()=0;      // Object extents box in obj coords
	/*! \remarks Returns the point to be shaded relative to the object box
	where each component is in the range of -1 to +1. */
	virtual Point3 PObjRelBox()=0;      // Point rel to obj box [-1 .. +1 ] 
	/*! \remarks Returns the derivative of <b>PObjRelBox()</b>. This is the
	derivative of the point relative to the object box where each component is
	in the range of -1 to +1. */
	virtual Point3 DPObjRelBox()=0;     // deriv of Point rel to obj box [-1 .. +1 ] 
	/*! \remarks Retrieves the point relative to the screen where the lower
	left corner is 0,0 and the upper right corner is 1,1.
	\par Parameters:
	<b>Point2\& uv</b>\n\n
	The point.\n\n
	<b>Point2 \&duv</b>\n\n
	The derivative of the point. */
	virtual void ScreenUV(Point2& uv, Point2 &duv)=0; // screen relative uv (from lower left)
	/*! \remarks Returns the integer screen coordinate (from the upper left).
	*/
	virtual IPoint2 ScreenCoord()=0; // integer screen coordinate (from upper left)
	/*! \remarks	 Return the surface point at the center of the fragment in floating point
	screen coordinates. See the documentation for <b>Sampler::DoSample()</b> for
	an explanation of the use of this method. See
	Class Sampler.
	\par Default Implementation:
	<b>{ return Point2(0.0,0.0); }</b> */
	virtual Point2 SurfacePtScreen(){ return Point2(0.0,0.0); } // return the surface point in screen coords

	/*! \remarks Returns the UVW coordinates for the point.
	\par Parameters:
	<b>int channel=0;</b>\n\n
	Specifies the channel for the values. One of the following:\n\n
	<b>0</b>: Vertex Color Channel.\n\n
	<b>1</b> through <b>99</b>: Mapping Channels. */
	virtual Point3 UVW(int channel=0)=0;         // return UVW coords for point
	/*! \remarks This method returns the UVW derivatives for the point. This is
	used for filtering texture maps and antialiasing procedurals that are using
	UVW. Note that in standard 3ds Max textures, the <b>UVGen</b> class is used,
	and it calls this method itself. See the methods <b>UVGen::GetBumpDP()</b>
	for more details for using <b>UVGen</b>. If you are not using <b>UVGen</b>
	then you can use this method and <b>UVW()</b>. <b>UVW()</b> gets the UVW
	coordinates of the point and <b>DUVW()</b> gets the change in the UVWs for
	the point. This tells you a maximum change for each of UVW. This tells you
	how much of the area of the map to sample. So when you call the
	<b>Bitmap</b> method <b>GetFiltered(float u, float v, float du, float dv,
	BMM_Color_64 *ptr)</b> this tells you how big the sample should be. This
	lets you filter or average over this area to keep the map from aliasing.
	\par Parameters:
	<b>int channel=0;</b>\n\n
	Specifies the channel for the values. One of the following:\n\n
	<b>0</b>: Vertex Color Channel.\n\n
	<b>1</b> through <b>99</b>: Mapping Channels. */
	virtual Point3 DUVW(int channel=0)=0;        // return UVW derivs for point
	/*! \remarks This returns the bump basis vectors for UVW in camera space.
	Note that if you want to retrieve these bump basis vectors that are altered
	by the UVGen instance use the method <b>UVGen::GetBumpDP()</b>. Also see the
	Advanced Topics section
	<a href="ms-its:3dsmaxsdk.chm::/mtls_materials.html">Working with Materials
	and Textures</a> for more details on bump mapping.
	\par Parameters:
	<b>Point3 dP[3]</b>\n\n
	The bump basic vectors. <b>dP[0]</b> is a vector corresponding to the U
	direction. <b>dp[1]</b> corresponds to V, and <b>dP[2]</b> corresponds to
	W.\n\n
	<b>int channel=0;</b>\n\n
	Specifies the channel for the values. One of the following:\n\n
	<b>0</b>: Vertex Color Channel.\n\n
	<b>1</b> through <b>99</b>: Mapping Channels. */
	virtual void DPdUVW(Point3 dP[3],int channel=0)=0; // Bump vectors for UVW (camera space)

	// BumpBasisVectors: this is going to replace DPdUVW: Now compute bump vectors for 
	//  specific 2D cases only.
	//    axis = AXIS_UV, AXIS_VW, AXIS_WU    
	//    return:     0 => not implemented,  1=> implemented
	// If this function is implemented, it will be used instead of DPdUVW.
	/*! \remarks	 This method should replace DpDUVW over time but is left in place as not to
	break 3rd party plugins. If this method returns 1, that is assumed to mean
	it is implemented, and it will be used instead of DpDUVW.
	\par Parameters:
	<b>Point3 dP[2]</b>\n\n
	The bump basic vectors. <b>dP[0]</b> is a vector corresponding to the U
	direction. <b>dp[1]</b> corresponds to V, and <b>dP[2]</b> corresponds to
	W.\n\n
	<b>int axis</b>\n\n
	Specified the 2D cases for: <b>AXIS_UV</b>, <b>AXIS_VW</b>, or
	<b>AXIS_WU</b>.\n\n
	<b>int channel=0;</b>\n\n
	Specifies the channel for the values. One of the following:\n\n
	<b>0</b>: Vertex Color Channel.\n\n
	<b>1</b> through <b>99</b>: Mapping Channels.
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual int BumpBasisVectors(Point3 dP[2], int axis, int channel=0) { return 0; } 


	virtual BOOL IsSuperSampleOn(){ return FALSE; }
	virtual BOOL IsTextureSuperSampleOn(){ return FALSE; }
	virtual int GetNSuperSample(){ return 0; }
	virtual float GetSampleSizeScale(){ return 1.0f; }

	// UVWNormal: returns a vector in UVW space normal to the face in UVW space. This can 
	// be CrossProd(U[1]-U[0],U[2]-U[1]), where U[i] is the texture coordinate
	// at the ith vertex of the current face.  Used for hiding textures on
	// back side of objects.
	/*! \remarks	 This method returns a vector in UVW space normal to the face in UVW space.
	This can be CrossProd(U[1]-U[0],U[2]-U[1]), where U[i] is the texture
	coordinate at the i-th vertex of the current face. This may be used for
	hiding textures on back side of objects.
	\par Parameters:
	<b>int channel=0;</b>\n\n
	Specifies the channel for the values. One of the following:\n\n
	<b>0</b>: Vertex Color Channel.\n\n
	<b>1</b> through <b>99</b>: Mapping Channels.
	\par Default Implementation:
	<b>{ return Point3(0,0,1); }</b> */
	virtual Point3 UVWNormal(int channel=0) { return Point3(0,0,1); }  

	// diameter of view ray at this point
	/*! \remarks	 Returns the diameter of the ray cone at the pixel point (the point it
	intersects the surface being shaded). This is a dimension in world units. As
	a ray is propagated it is updated for each new surface that is encountered.
	\par Default Implementation:
	<b>{ return Length(DP()); }</b> */
	virtual float RayDiam() { return Length(DP()); } 

	// angle of ray cone hitting this point: gets increased/decreased by curvature 
	// on reflection
	/*! \remarks	 Returns the angle of a ray cone hitting this point. It gets
	increased/decreased by curvature on reflection.\n\n
	Visualize a small pyramid, with the top at the eye point, and its sides
	running through each corner of the pixel to be rendered, then onto the
	scene. Then visualize a small cone fitting inside this pyramid. This method
	returns the angle of that cone. When rendering, if the ray cone goes out and
	hits a flat surface, the angle of reflection will always be constant for
	each pixel. However, if the ray cone hits a curved surface, the angle will
	change between pixels. This change in value give some indication of how fast
	the sample size is getting bigger.
	\par Default Implementation:
	<b>{ return 0.0f; }</b> */
	virtual float RayConeAngle() { return 0.0f; } 

	/*! \remarks This is used by the Standard material to do the reflection
	maps and the refraction maps. Given the map, and a direction from which you
	want to view it, this method changes the view vector to be the specified
	vector and evaluates the function.
	\par Parameters:
	<b>Texmap *map</b>\n\n
	The map to evaluate.\n\n
	<b>Point3 view</b>\n\n
	The view direction.
	\return  The color of the map, in r, g, b, alpha. */
	CoreExport virtual AColor EvalEnvironMap(Texmap *map, Point3 view); //evaluate map with given viewDir
	/*! \remarks Retrieves the background color and the background transparency.
	\par Parameters:
	<b>class Color \&bgCol</b>\n\n
	The returned background color.\n\n
	<b>class Color \&transp</b>\n\n
	The returned transparency.\n\n
	<b>int fogBG</b>\n\n
	Specifies you want the current atmospheric shaders to be applied to the
	background color. If TRUE the shaders are applied; if FALSE they are not. */
	virtual void GetBGColor(Color &bgcol, Color& transp, BOOL fogBG=TRUE)=0;   // returns Background color, bg transparency

	// Camera ranges set by user in camera's UI.
	/*! \remarks Returns the camera near range set by the user in the camera's
	user interface. */
	virtual float CamNearRange() {return 0.0f;}
	/*! \remarks Returns the camera far range set by the user in the camera's
	user interface. */
	virtual float CamFarRange() {return 0.0f;}

	// Transform to and from internal space
	/*! \remarks Transforms the specified point from internal camera space to
	the specified space.
	\par Parameters:
	<b>const Point3\& p</b>\n\n
	The point to transform.\n\n
	<b>RefFrame ito</b>\n\n
	The space to transform the point to. One of the following values:\n\n
	<b>REF_CAMERA</b>\n\n
	<b>REF_WORLD</b>\n\n
	<b>REF_OBJECT</b>
	\return  The transformed point, in the specified space. */
	virtual Point3 PointTo(const Point3& p, RefFrame ito)=0; 
	/*! \remarks Transforms the specified point from the specified coordinate
	system to internal camera space.
	\par Parameters:
	<b>const Point3\& p</b>\n\n
	The point to transform.\n\n
	<b>RefFrame ifrom</b>\n\n
	The space to transform the point from. One of the following values:\n\n
	<b>REF_CAMERA</b>\n\n
	<b>REF_WORLD</b>\n\n
	<b>REF_OBJECT</b>
	\return  The transformed point in camera space. */
	virtual Point3 PointFrom(const Point3& p, RefFrame ifrom)=0; 
	/*! \remarks Transform the vector from internal camera space to the
	specified space.
	\par Parameters:
	<b>const Point3\& p</b>\n\n
	The vector to transform.\n\n
	<b>RefFrame ito</b>\n\n
	The space to transform the vector to. One of the following values:\n\n
	<b>REF_CAMERA</b>\n\n
	<b>REF_WORLD</b>\n\n
	<b>REF_OBJECT</b> */
	virtual Point3 VectorTo(const Point3& p, RefFrame ito)=0; 
	/*! \remarks Transform the vector from the specified space to internal
	camera space.
	\par Parameters:
	<b>const Point3\& p</b>\n\n
	The vector to transform.\n\n
	<b>RefFrame ifrom</b>\n\n
	The space to transform the vector from. One of the following values:\n\n
	<b>REF_CAMERA</b>\n\n
	<b>REF_WORLD</b>\n\n
	<b>REF_OBJECT</b> */
	virtual Point3 VectorFrom(const Point3& p, RefFrame ifrom)=0; 
	/*! \remarks	 Transform the vector from internal camera space to the specified space
	without scaling.
	\par Parameters:
	<b>const Point3\& p</b>\n\n
	The vector to transform.\n\n
	<b>RefFrame ito</b>\n\n
	The space to transform the vector to. One of the following values:\n\n
	<b>REF_CAMERA</b>\n\n
	<b>REF_WORLD</b>\n\n
	<b>REF_OBJECT</b> */
	CoreExport virtual Point3 VectorToNoScale(const Point3& p, RefFrame ito); 
	/*! \remarks	 Transform the vector from the specified space to internal camera space
	without scaling.\n\n
	Note: This method was added to correct a problem that was occurring in 3D
	Textures when the bump perturbation vectors were transformed from object
	space to camera space, so they are oriented correctly as the object rotates.
	If the object has been scaled, this transformation causes the perturbation
	vectors to be scale also, which amplifies the bump effect. This method is
	used to rotate the perturbation vectors so they are correctly oriented in
	space, without scaling them.
	\par Parameters:
	<b>const Point3\& p</b>\n\n
	The vector to transform.\n\n
	<b>RefFrame ifrom</b>\n\n
	<b>RefFrame ifrom</b>\n\n
	The space to transform the vector from. One of the following values:\n\n
	<b>REF_CAMERA</b>\n\n
	<b>REF_WORLD</b>\n\n
	<b>REF_OBJECT</b> */
	CoreExport virtual Point3 VectorFromNoScale(const Point3& p, RefFrame ifrom); 

	// After being evaluated, if a map or material has a non-zero GBufID, it should
	// call this routine to store it into the shade context.
	/*! \remarks When a map or material is evaluated (in <b>Shade()</b>,
	<b>EvalColor()</b> or <b>EvalMono()</b>), if it has a non-zero
	<b>gbufID</b>, it should call this routine to store the <b>gbid</b> into the
	shade context.\n\n
	Note: Normally a texmap calls this method so the index would be set for all
	of the area covered by the texture. There is no reason that this has to be
	done for every pixel however. A texture could just set the ID for particular
	pixels. This could allow post processing routines (for example a glow) to
	only process part of a texture and not the entire thing. For example, at the
	beggining of texmap's <b>EvalColor()</b> one typically has code that
	does:\n\n
	<b> if (gbufid) sc.SetGBufferID(gbufid);</b>\n\n
	This takes the <b>gbufid</b> (which is in <b>MtlBase</b>) and (if it is
	non-zero) stores it into the shade context. The renderer, after evaluating
	the <b>Shade()</b> function for the material at a pixel, looks at the
	gbufferID left in the shade context, and stores it into the gbuffer at that
	pixel. So if the texmap adds another condition like\n\n
	\code
	if (inHotPortion)
	if (gbufid) sc.SetGBufferID(gbufid);
	\endcode  
	It will do it for just the chosen pixels.
	\par Parameters:
	<b>int gbid</b>\n\n
	The ID to store. */
	virtual void SetGBufferID(int gbid) { out.gbufId = gbid; }

	/*! \remarks This method is used internally. 
	*/
	virtual FILE* DebugFile() { return NULL; }

	/*! \remarks	 Returns the color of the global environment map from the given view
	direction.
	\par Parameters:
	<b>Point3 dir</b>\n\n
	Specifies the direction of view.
	\par Default Implementation:
	<b>{ return AColor(0,0,0,0); }</b> */
	virtual AColor EvalGlobalEnvironMap(Point3 dir) { return AColor(0,0,0,0); }

	/*! \remarks	 This method is used with texture maps only. If a map is multiply instanced
	within the same material, say on the diffuse channel and on the shininess
	channel, it will return the same value each time its evaluated. Its a waste
	of processor time to reevaluate the map twice. This method allows you to
	cache the value so it won't need to be computed more than once.\n\n
	Note that the cache is automatically cleared after each ShadeContext call.
	This is used within one evaluation of a material hierarchy.
	\par Parameters:
	<b>Texmap *map</b>\n\n
	Points to the texmap storing the cache (usually the plug-ins <b>this</b>
	pointer).\n\n
	<b>AColor \&c</b>\n\n
	The color to store.
	\return  TRUE if the color was returned; otherwise FALSE.
	\par Default Implementation:
	<b>{ return FALSE; }</b>
	\par Sample Code:
	This code from <b>/MAXSDK/SAMPLES/MATERIALS/NOISE.CPP</b> and shows how the cache is retrieved and stored:\n\n
	\code
	RGBA Noise::EvalColor(ShadeContext& sc) {
	Point3 p,dp;
	if (!sc.doMaps) return black;
	AColor c;
	// If the cache exists, return the color
	if (sc.GetCache(this,c))
	return c;
	// Otherwise compute the color
	. . .
	// At the end of the eval the cache is stored
	sc.PutCache(this,c);
	}
	\endcode
	*/
	virtual BOOL GetCache(Texmap *map, AColor &c){ return FALSE; }
	/*! \remarks	 Retrieves a floating point value from the cache. See the AColor version
	above for details.
	\par Parameters:
	<b>Texmap *map</b>\n\n
	Points to the texmap storing the cache (usually the plug-ins <b>this</b>
	pointer).\n\n
	<b>float \&f</b>\n\n
	The value to store.
	\return  TRUE if the value was returned; otherwise FALSE.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL GetCache(Texmap *map, float &f) { return FALSE; }
	/*! \remarks	 Retrieves a Point3 value from the cache. See the AColor version above for
	details.
	\par Parameters:
	<b>Texmap *map</b>\n\n
	Points to the texmap storing the cache (usually the plug-ins <b>this</b>
	pointer).\n\n
	<b>Point3 \&p</b>\n\n
	The point to store.
	\return  TRUE if the value was returned; otherwise FALSE.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL GetCache(Texmap *map, Point3 &p){ return FALSE; }
	/*! \remarks	 Puts a color to the cache. See the method <b>GetCache(Texmap *map, const
	AColor \&c)</b> above for details.
	\par Parameters:
	<b>Texmap *map</b>\n\n
	Points to the texmap storing the cache (usually the plug-ins <b>this</b>
	pointer).\n\n
	<b>const AColor \&c</b>\n\n
	The color to store.
	\par Default Implementation:
	<b>{}</b> */
	virtual void PutCache(Texmap *map, const AColor &c){}
	/*! \remarks	 Puts a floating point value to the cache. See the method <b>GetCache(Texmap
	*map, const AColor \&c)</b> above for details.
	\par Parameters:
	<b>Texmap *map</b>\n\n
	Points to the texmap storing the cache (usually the plug-ins <b>this</b>
	pointer).\n\n
	<b>const float f</b>\n\n
	The floating point value to store.
	\par Default Implementation:
	<b>{}</b> */
	virtual void PutCache(Texmap *map, const float f) {}
	/*! \remarks	 Puts a floating point value to the cache. See the method <b>GetCache(Texmap
	*map, const AColor \&c)</b> above for details.
	\par Parameters:
	<b>Texmap *map</b>\n\n
	Points to the texmap storing the cache (usually the plug-ins <b>this</b>
	pointer).\n\n
	<b>const Point3 \&p</b>\n\n
	The Point3 value to store.
	\par Default Implementation:
	<b>{}</b> */
	virtual void PutCache(Texmap *map, const Point3 &p){}
	/*! \remarks	 Removes the specified cache.
	\par Parameters:
	<b>Texmap *map</b>\n\n
	Points to the texmap storing the cache (usually the plug-ins <b>this</b>
	pointer).
	\par Default Implementation:
	<b>{}</b> */
	virtual void TossCache(Texmap *map) {}
	// Generic expansion function
	/*! \remarks	This is a general purpose function that allows the API to be extended in the
	future. The 3ds Max development team can assign new <b>cmd</b> numbers and
	continue to add functionality to this class without having to 'break' the API.\n\n
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
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 

	// These are used to prevent self shadowing by volumetric lights
	/*! \remarks	 This method, along with <b>SetAtmosSkipLight()</b> below, are used by the
	lights to avoid self-shadowing when applying atmospheric shadows. This
	method returns a pointer to the <b>LightDesc</b> instance currently calling
	the <b>Atmosphere::Shade()</b> method when computing atmospheric
	shadows.\n\n
	Here's how they are used:\n\n
	(1) When computing the atmospheric shadows:(somewhere in
	<b>::LightDesc::Illuminate()</b>) do the following:\n\n
	<b>sc.SetAtmosSkipLight(this);</b>\n\n
	<b>sc.globContext-\>atmos-\>Shade(sc, lightPos, sc.P(), col, trans);</b>\n\n
	<b>sc.SetAtmosSkipLight(NULL);</b>\n\n
	(2) In <b>LightDesc::TraverseVolume()</b> do the following:\n\n
	<b>if (sc.GetAtmosSkipLight()==this)</b>\n\n
	<b>return;</b>
	\par Default Implementation:
	<b>{ return atmosSkipLight; }</b> */
	/*! \remarks	 This method can be used to determine from within the ShadeContext if a
	volumetric light should be prevented from generating self-shadows.
	\par Default Implementation:
	<b>{ return atmosSkipLight; }</b>\n\n
	*/
	LightDesc *GetAtmosSkipLight() { return atmosSkipLight; }
	/*! \remarks	 This method sets the <b>LightDesc</b> instance currently calling the
	<b>Atmosphere::Shade()</b> method. See <b>GetAtmosSkipLight()</b> above.
	\par Parameters:
	<b>LightDesc *lt</b>\n\n
	Points to the LightDesc to set.
	\par Default Implementation:
	<b>{ atmosSkipLight = lt; }</b> */
	/*! \remarks	 This method allows you to set the volumetric light that should be prevented
	from generating self-shadows.
	\par Parameters:
	<b>LightDesc *lt</b>\n\n
	A pointer to the light to set.
	\par Default Implementation:
	<b>{ atmosSkipLight = lt; }</b>\n\n
	*/
	void SetAtmosSkipLight(LightDesc *lt) { atmosSkipLight = lt; }

	// render element methods
	/*! \remarks	 Returns the number of render elements.
	\par Default Implementation:
	<b>{ return globContext-\>NRenderElements(); }</b> */
	virtual int NRenderElements() { 
		return globContext ? globContext->NRenderElements():0;
	}
	/*! \remarks	 Returns an interface to the 'i-th' render element.
	\par Parameters:
	<b>int n</b>\n\n
	The zero based index of the render element to return.
	\par Default Implementation:
	<b>{ return globContext-\>GetRenderElement(n); }</b> */
	virtual IRenderElement *GetRenderElement(int n) { 
		return globContext ? globContext->GetRenderElement(n) : NULL;
	}

	// diffuse illum utility, computes incoming diffuse illumination, for matte/shadow
	/*! \remarks	 Computes and returns the incoming diffuse illumination color (for
	matte/shadow). */
	CoreExport virtual Color DiffuseIllum();

	// used to preshade in different ways
	// virtual IRenderMode GetIRenderMode(){ return IRM_NORMAL; }
	// IRenderMode GetIRenderMode(){ return iRenderMode; }
	// void SetIRenderMode( IRenderMode mode ){ iRenderMode = mode; }

	// NOTE: All of these methods are inline. The inline definitions not
	// declared here are declared in toneop.h

	// Does this tone operator really map physical values to RGB. This method
	// is provided so shaders can determine whether the shading calculations
	// are in physical or RGB space.
	/*! \remarks	 This method returns TRUE if the operator really maps physical values to RGB,
	otherwise FALSE. This method is provided so shaders can determine whether
	the shading calculations are in physical or RGB space. */
	bool IsPhysicalSpace() const;

	// Map an scaled energy value into RGB. The first version of the
	// method converts a color value and the second converts a monochrome
	// value. The converted color value is stored in <i>energy</i>.
	// The converted monochrome value is returned.
	/*! \remarks	 This method will map a scaled energy value into RGB. This converts a color
	value which will be stored in energy.
	\par Parameters:
	<b>T\& energy</b>\n\n
	The converted color value. */
	template< class T > void ScaledToRGB( T& energy ) const
	{
		if ( globContext != NULL && globContext->pToneOp != NULL )
			globContext->pToneOp->ScaledToRGB( energy );
	}
	/*! \remarks	 This method will map a scaled energy value into RGB. This converts a
	monochrome value which will be returned.
	\par Parameters:
	<b>float energy</b>\n\n
	The scaled energy value. */
	float ScaledToRGB( float energy ) const;

	// Map an energy value int out.c into RGB. The converted value is stored in
	// out.c.
	/*! \remarks	 This method will map an energy value int <b>out.c</b> into RGB. The
	converted value is stored in <b>out.c</b>. */
	void ScaledToRGB( );

	// Scale physical values so they can be used in the renderer. The
	// first version of the method converts a color value and the second
	// converts a monochrome value. The converted color value is stored
	// in <i>energy</i>. The converted monochrome value is returned.
	/*! \remarks	 This method will scale physical values so they can be used in the renderer.
	This converts a color value which will be stored in energy.
	\par Parameters:
	<b>T\& energy</b>\n\n
	The converted color value. */
	template< class T > void ScalePhysical(T& energy) const
	{
		if ( globContext != NULL && globContext->pToneOp != NULL )
			globContext->pToneOp->ScalePhysical( energy );
	}
	/*! \remarks	 This method will scale physical values so they can be used in the renderer.
	This converts a monochrome value which will be returned.
	\par Parameters:
	<b>float energy</b>\n\n
	The energy value. */
	float ScalePhysical(float energy) const;

	// Scale RGB values, just supplied to invert ScalePhysical. The first
	// version of the method converts a color value and the second
	// converts a monochrome value. The converted color value is stored
	// in <i>energy</i>. The converted monochrome value is returned.
	/*! \remarks	 This method will scale RGB values, just supplied to invert ScalePhysical.
	This converts a color value which will be stored in energy.
	\par Parameters:
	<b>T\& energy</b>\n\n
	The converted color value. */
	template< class T > void ScaleRGB(T& energy) const
	{ 
		if ( globContext != NULL && globContext->pToneOp != NULL )
			globContext->pToneOp->ScaleRGB( energy ); 
	}
	/*! \remarks	 This method will scale RGB values, just supplied to invert ScalePhysical.
	This converts a monochrome value which will be returned.
	\par Parameters:
	<b>float energy</b>\n\n
	The energy value. */
	float ScaleRGB(float energy) const;

	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b>mode = SCMODE_NORMAL; nLights = 0; shadow = TRUE; rayLevel = 0;
	globContext = NULL; atmosSkipLight = NULL;</b> */
	ShadeContext() {
		mode = SCMODE_NORMAL; nLights = 0; shadow = TRUE;  rayLevel = 0; 
		globContext = NULL; atmosSkipLight = NULL; 
	}
};

			
/*! \defgroup materialFlags Material Flags
\sa Class MtlBase
*/
//@{
#define MTL_IN_SCENE             (1<<0) //!< The material is being used in the scene.
#define MTL_BEING_EDITED      (1<<1)  //!< The material's parameters are being displayed in the Material Editor.
#define MTL_SUB_BEING_EDITED  (1<<2)  //!< This material OR sub-material texmap is being displayed in the Material Editor. 
#define MTL_TEX_DISPLAY_ENABLED (1<<3)  //!< Viewport display enabled for this material (see MTL_SUB_DISPLAY_ENABLED).
#define MTL_MEDIT_BACKGROUND    (1<<8)  //!< Material has the background shown in Material Editor.
#define MTL_MEDIT_BACKLIGHT      (1<<9)  //!< Material is backlight in the Material Editor.

#define MTL_OBJTYPE_SHIFT     10 //!< 
#define MTL_MEDIT_OBJTYPE     (1<<MTL_OBJTYPE_SHIFT) //!< Object type displayed in Material Editor
#define MTL_MEDIT_OBJTYPE_MASK   ((1<<MTL_OBJTYPE_SHIFT)|(1<<(MTL_OBJTYPE_SHIFT+1))|(1<<(MTL_OBJTYPE_SHIFT+2)))

#define MTL_TILING_SHIFT      13 //!< 
#define MTL_MEDIT_TILING      (1<<MTL_TILING_SHIFT) // Object type displayed in Material Editor.
#define MTL_MEDIT_TILING_MASK ((1<<MTL_TILING_SHIFT)|(1<<(MTL_TILING_SHIFT+1))|(1<<(MTL_TILING_SHIFT+2)))

#ifndef NO_MTLEDITOR_VIDCOLORCHECK  // orb 01-08-2002
#define MTL_MEDIT_VIDCHECK    (1<<16) //!< 
#endif

#define MTL_BROWSE_OPEN1      (1<<18) //!< For internal use.
#define MTL_BROWSE_OPEN2      (1<<19) //!< For internal use.
#define MTL_SUB_DISPLAY_ENABLED (1<<20)  //!< Indicates that texture display is enabled for map or material in this subtree

#define MTL_CLONED            (1<<28)  //!< Indicates material or texture was created by being cloned from another material or texture. 
#define MTL_HW_MAT_PRESENT    (1<<29)  //!< 
#define MTL_HW_MAT_ENABLED    (1<<30)  //!< 

#define MTL_WORK_FLAG         (1<<31)  //!< 

#define MTL_DISPLAY_ENABLE_FLAGS (MTL_TEX_DISPLAY_ENABLED|MTL_SUB_DISPLAY_ENABLED)  //!<  Interactive texture display enabled for THIS mtl base

#define MTL_HW_TEX_ENABLED    (1<<17)  //!<  Indicates that the texture should be part of the realtime shader
//@}

/*! \defgroup materialRequirementsFlags Material Requirements Flags
\sa Class MtlBase, Class RenderInstance */
//@{
#define MTLREQ_2SIDE          (1<<0)  //!< The material is 2-sided.
#define MTLREQ_WIRE           (1<<1)  //!< The material is wire frame material.
#define MTLREQ_WIRE_ABS       (1<<2)  //!< Wire frame material, absolute size
#define MTLREQ_TRANSP         (1<<3)   //!< The material uses transparency.
#define MTLREQ_UV          (1<<4)  //!< The material requires UVW coordinates.
#define MTLREQ_FACEMAP        (1<<5)  //!< The material uses "face map" UV coordinates.
#define MTLREQ_XYZ            (1<<6)  //!< The material requires object XYZ coordinates.
#define MTLREQ_OXYZ        (1<<7)  //!< The material requires object ORIGINAL XYZ coordinates.
#define MTLREQ_BUMPUV         (1<<8)  //!< The Material requires UV bump vectors.
#define MTLREQ_BGCOL       (1<<9)  //!< The material requires background color (e.g. Matte mtl).
#define MTLREQ_PHONG       (1<<10) //!< The material requires interpolated normal.
#define MTLREQ_AUTOREFLECT       (1<<11) //!< The material needs to build auto-reflect map.
#define MTLREQ_AUTOMIRROR     (1<<12) //!< The material needs to build auto-mirror map.
/*! This is used by the Matte material for example. The matte material samples the background
itself and handles the fogging characteristics, so it does not need to have the atmospheric
shader do this again. */
#define MTLREQ_NOATMOS        (1<<13) //!< This suppresses the atmospheric shader. (used by Matte mtl)
/*! Normally, if this is not specified, the background color is attenuated. For instance, for a
material that 30% transparent (70% opaque), you take 30% of the background color and 70% of the
material color. \n\n
Setting this flag causes the computation to be done by multiplying 0.7 times the
material color and then adding this to the background color, which is left un-attenuated. */
#define MTLREQ_ADDITIVE_TRANSP   (1<<14) //!< Specifies that transparent composites are done additively.
#define MTLREQ_VIEW_DEP       (1<<15) //!< Maps or materials which depend on the view should set this bit in their Requirements() method. 
#define MTLREQ_UV2            (1<<16)  //!< The material requires second uv channel values (vertex colors).
#define MTLREQ_BUMPUV2        (1<<17)  //!< The material requires second uv channel bump vectors.
#define MTLREQ_PREPRO         (1<<18)  //!< Pre-processing. MtlBase::BuildMaps is called on every frame.
#define MTLREQ_DONTMERGE_FRAGMENTS  (1<<19) //!< No longer used.
#define MTLREQ_DISPLACEMAP       (1<<20) //!< Material has a Displacement map channel
/*! This tells the scanline renderer that you want super sampling - the Standard material uses
this. This takes multiple samples spread around in the pixel which provides an additional level
of anti-aliasing. */
#define MTLREQ_SUPERSAMPLE       (1<<21) //!< The material requires supersampling
/*! This flag is set by UVGen and XYZGen when world coordinates are involved. The renderer looks
at this and if it is set, takes the validity interval of the Node's transform matrix into account
in computing the validity of a displacement mapped mesh. */
#define MTLREQ_WORLDCOORDS       (1<<22) //!< The material has world coordinates are used in material/map evaluation.
/*! This flag should be returned true for any material that wants to be transparent in the viewport. */
#define MTLREQ_TRANSP_IN_VP   (1<<23) //!< The material is transparent in the viewport ( opacity < 100, or opacity map & ShowMapInVP set).
#define MTLREQ_FACETED        (1<<24) //!< The material should be rendered faceted in the viewports.

/* JH 5/21/03 Adjusting for R6
#define MTLREQ_2SIDE_SHADOW   (1<<25) // USED IN RENDERER.
#define MTLREQ_REND2          (1<<26) // USED IN RENDERER.
#define MTLREQ_REND3          (1<<27) // USED IN RENDERER.
#define MTLREQ_REND4          (1<<28) // USED IN RENDERER.
#define MTLREQ_REND5          (1<<29) // USED IN RENDERER.
#define MTLREQ_REND6          (1<<30) // USED IN RENDERER.
#define MTLREQ_NOEXPOSURE     (1<<31) // don't do the tone-op (ie, for matte/shadow material, etc)
*/
#define MTLREQ_NOEXPOSURE     (1<<25) //!< Do not do the tone-op (ie, for matte/shadow material, etc)
#define MTLREQ_SS_GLOBAL      (1<<26) //!< Material requires supersampling but use global sampler

#define MTLREQ_REND1          (1<<28) //!< USED IN RENDERER.
#define MTLREQ_REND2          (1<<29) //!< USED IN RENDERER.
#define MTLREQ_REND3          (1<<30) //!< USED IN RENDERER.
#define MTLREQ_REND4          (1<<31) //!< USED IN RENDERER.
//@}

/*! \defgroup mapSlotTypes Map Slot Types
\sa Class MtlBase */
//@{
#define MAPSLOT_TEXTURE      0   //!< A slot that holds a texture map.
/*! Generate UVW on-the-fly using view vector, default to spherical */
#define MAPSLOT_ENVIRON      1   //!< A slot which holds an environment map.
#define MAPSLOT_DISPLACEMENT 2   //!< Displacement maps: a type of texture map.
#define MAPSLOT_BACKGROUND   3   //!< Background maps: generate UVW on-the-fly using view vector, default to screen
//@}

// Where to get texture vertices: returned by GetUVWSource();
#define UVWSRC_EXPLICIT    0  // use explicit mesh texture vertices (whichever map channel)
#define UVWSRC_OBJXYZ      1  // generate planar uvw mapping from object xyz on-the-fly
#define UVWSRC_EXPLICIT2   2  // use explicit mesh map color vertices as a tex map
#define UVWSRC_WORLDXYZ    3  // generate planar uvw mapping from world xyz on-the-fly
#ifdef GEOREFSYS_UVW_MAPPING
#define UVWSRC_GEOXYZ      4  // generate planar uvw mapping from geo referenced world xyz on-the-fly
#endif
#define UVWSRC_FACEMAP     5  // use face map
#define UVWSRC_HWGEN    6  // use hardware generated texture coords

/*! \sa  Class MtlBase, Class Texmap, Class ParamDlg, Class ReferenceTarget.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The methods of this class provide access to the sub-textures of a MtlBase.
These properties include the number of sub-maps, the slot type, on / off state,
etc.\n\n
The MtlBase class sub-classes from this class. If a developer is creating a
plug-in derived from MtlBase (for instance a Material or Texture) then
implementations of these methods are required. Developers may call these
methods on an existing MtlBase sub-class.  */
class ISubMap : public MaxHeapOperators 
{
public:
	/*! \remarks Destructor. */
	virtual ~ISubMap() {;}
	// Methods to access sub texture maps of material or texmap
	/*! \remarks Returns the number of sub-textures managed by this material or texture.
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual int NumSubTexmaps() { return 0; }
	/*! \remarks Returns a pointer to the 'i-th' sub-texmap managed by the material or
	texture.\n\n
	Note: For the 3ds Max Standard material, the sub-texmap indexes used with this
	method are listed in \ref Material_TextureMap_IDs.
	\par Parameters:
	<b>int i</b>\n\n
	Specifies the texmap to return.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual Texmap* GetSubTexmap(int i) { return NULL; }
	/*! \remarks In the Coordinates rollup in the user interface for a texture map are two
	options. These options are Texture or Environment. The slot type is one of
	these two options, texture coordinates or environment coordinates. There are
	a variety of texture coordinate types. There are the type assigned to the
	object and the environment type (Spherical, Cylindrical, Shrink-wrap,
	Screen). This method is used to determine the type required by the
	particular sub-texture. This is either texture coordinates
	(<b>MAPSLOT_TEXTURE</b>) or environment coordinates
	(<b>MAPSLOT_ENVIRON</b>).
	\par Parameters:
	<b>int i</b>\n\n
	The index of the sub-texture whose slot type to return.
	\return See \ref mapSlotTypes.
	\par Default Implementation:
	<b>{ return MAPSLOT_TEXTURE; }</b> */
	virtual int MapSlotType(int i)=0;
	/*! \remarks Stores the 'i-th' sub-texmap managed by the material or texture.\n\n
	Note: For the 3ds Max Standard material, the sub-texmap indexes used with this
	method are listed in \ref Material_TextureMap_IDs.
	\par Parameters:
	<b>int i</b>\n\n
	The index of the storage for the texmap.\n\n
	<b>Texmap *m</b>\n\n
	The texmap to store.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetSubTexmap(int i, Texmap *m) { }

	// query ISubMap about the On/Off state of each sub map.
	/*! \remarks Returns nonzero if the specified sub-texmap is on; otherwise zero. Some
	materials may have user interface controls to toggle the sub-maps on and
	off. The Standard material has such controls for example.
	\par Parameters:
	<b>int i</b>\n\n
	The index of the sub-texmap to check.
	\par Default Implementation:
	<b>{ return 1; }</b> */
	virtual int SubTexmapOn(int i) { return 1; } 

	/*! \remarks This method returns the slot name of the 'i-th' sub-texmap. This name
	appears in the materials editor dialog. For instance, if you are in a
	material and then you go down into a map, this is the name that appears just
	below the 'Get Material' icon. For example, in the Standard material when
	you choose one of the maps, the map name appears to let you know which slot
	you are working on. For the Standard material, this may be "Ambient",
	"Diffuse", "Specular", etc.
	\par Parameters:
	<b>int i</b>\n\n
	Specifies the slot whose name is returned.
	\par Default Implementation:
	The default implementation returns an empty ("") string. */
	CoreExport virtual MSTR GetSubTexmapSlotName(int i);
	/*! \remarks Returns the name to appear in Track %View of the 'i-th' sub-texmap.
	\par Parameters:
	<b>int i</b>\n\n
	Specifies the sub-texmap whose name is returned. */
	CoreExport MSTR GetSubTexmapTVName(int i);

	// called by the ParamMap2 AUTO_UI system if the material/map is letting
	// the system build an AutoMParamDlg for it.  SetDlgThing() is called on
	// a material/map coming into an existing set of ParamDlgs, once for each 
	// secondary ParamDlg and it should set the appropriate 'thing' into the
	// given dlg (for example, Texout* or UVGen*).  Return FALSE if dlg is unrecognized.
	// See Gradient::SetDlgThing() for an example.
	/*! \remarks This method is called by the ParamMap2 <b>AUTO_UI</b> system
	if the material/texmap is letting the system build an <b>AutoMParamDlg</b>
	for it. This method is called on a material/texmap coming into an existing
	set of ParamDlgs, once for each secondary <b>ParamDlg</b> and it should set
	the appropriate 'thing' into the given dlg (the 'thing' being, for example,
	a <b>Texout*</b> or <b>UVGen*</b>). Return FALSE if <b>dlg</b> is
	unrecognized.\n\n
	Note: See the discussion above in <b>CreateParamDlg()</b> for additional
	details on this method.
	\par Parameters:
	<b>ParamDlg* dlg</b>\n\n
	Points to the ParamDlg to check. See Class ParamDlg.
	\return  TRUE if the <b>dlg</b> passed is recognized; FALSE if unrecognized.
	\par Default Implementation:
	<b>{ return FALSE; }</b>
	\par Sample Code:
	\code
	BOOL Gradient::SetDlgThing(ParamDlg* dlg)
	{
	if (dlg == uvGenDlg)
	uvGenDlg->SetThing(uvGen);
	else if (dlg == texoutDlg)
	texoutDlg->SetThing(texout);
	else
	return FALSE;
	return TRUE;
	}
	\endcode */
	virtual BOOL SetDlgThing(ParamDlg* dlg) { return FALSE; }

	// use this for drag-and-drop of texmaps
	/*! \remarks Implemented by the System.\n\n
	This method is used to handle the drag-and-drop of texmaps. A developer
	implements the logic to handle the drag and drop. Once they have the
	information about what slot was dropped on what other slot, this method may
	be called to handle the copying. This is used by the 3ds Max Standard
	material.
	\par Parameters:
	<b>HWND hwnd</b>\n\n
	The rollup page window handle.\n\n
	<b>int ifrom</b>\n\n
	The source texmap.\n\n
	<b>int ito</b>\n\n
	The destination texmap. */
	CoreExport void CopySubTexmap(HWND hwnd, int ifrom, int ito);

	/*! \remarks Implemented by the System.\n\n
	The implementation of this method is provided by <b>MtlBase</b>. It returns
	its <b>this</b> pointer. */
	virtual ReferenceTarget *GetRefTarget()=0;
};

// Base class from which materials and textures are subclassed.
/*! \sa  Class ReferenceTarget, Class ISubMap, Class Mtl,  Class Texmap,  Class ShadeContext, Class RenderMapsContext, Class RenderGlobalContext, Class UVGen, Class XYZGen, Class PStamp,  Class Quat.\n\n
\par Description:
This is the base class from which materials and textures are subclassed.
Methods are provided to access the name, flags, and sub-materials/maps. There
is also a method that is called when the material or texture is to be displayed
in the material editor parameters area.\n\n
Note the following about dialog proc processing of sub-map buttons:\n\n
When you post the message:\n\n
<b>PostMessage(hwmedit, WM_TEXMAP_BUTTON, i,(LPARAM)theMtl);</b>\n\n
You are telling the system that the user clicked on the button for the 'i-th'
sub-map of <b>theMtl</b>. The message doesn't propagate up -- it goes directly
to the materials editor. It then uses calls on <b>theMtl-\>SetSubTexmap()</b>
and <b>theMtl-\>GetSubTexmap()</b> to assign the new map. So even if your
material has some complicated internal hierarchical structure of references, to
the system it is still a simple "logical" hierarchy of a material with some
sub-texmaps.
\par Data Members:
<b>Quat meditRotate;</b>\n\n
This data member is available in release 2.0 and later only.\n\n
This describes the rotation of the sample geometry in the materials editor.\n\n
<b>ULONG gbufID;</b>\n\n
This is the G-Buffer ID of the material or texmap. This is a "effects number"
assigned in the materials editor to a map or material, and it will be written
into the effects channel of the G-Buffer when a pixel with that material on it
is rendered. To implement this, each map or material, in the beginning of its
<b>Shade()</b>, <b>EvalColor()</b>, or <b>EvalMono()</b> methods should have
the code:\n\n
<b>if (gbufID) sc.SetGBufferID(gbufID);</b>
\par Method Groups:
See <a href="class_mtl_base_groups.html">Method Groups for Class MtlBase</a>.
*/
class MtlBase: public ReferenceTarget, public ISubMap {
	 friend class Texmap;
	 MSTR name;
	 ULONG mtlFlags;
	 int defaultSlotType;
	 int activeCount;
	 class MtlBaseImplData;
	 const MaxSDK::Util::AutoPtr<MtlBaseImplData> mMtlBaseImplData;
	public:
			Quat meditRotate;
			ULONG gbufID;
			/*! \remarks Constructor. The flags and G-buffer id are initialized. */
			CoreExport MtlBase();
			CoreExport ~MtlBase();
			CoreExport Class_ID ClassID();
			/*! \remarks Implemented by the System.\n\n
			Returns the name of the material or texture. */
			MSTR& GetName() { return name; }
			/*! \remarks Implemented by the System.\n\n
			Stores the name of the material or texture. */
			CoreExport void SetName(MSTR s);
			/*! \remarks Implemented by the System.\n\n
			Alters the flags, either setting or clearing them, using the mask and
			method passed.
			\param mask The flags to alter. 
			See \ref materialFlags.
			\param val If TRUE the mask is ORed into the flags (<b>mtlFlags |= mask</b>);
			otherwise (<b>mtlFlags \&= ~mask</b>). */
			CoreExport void SetMtlFlag(int mask, BOOL val=TRUE);
			/*! \remarks Implemented by the System.\n\n
			Clears the specified flags.
			\param mask The flags to clear. 
			See \ref materialFlags. */
			CoreExport void ClearMtlFlag(int mask);
			/*! \remarks Implemented by the System.\n\n
			Tests the specified flags. Returns nonzero if the flags are set;
			otherwise zero. See \ref materialFlags.
			\param mask The flags to test. */
			CoreExport int TestMtlFlag(int mask); 

			// Used internally by materials editor.
			/*! \remarks This method is used internally. */
			int GetMeditObjType() { return (mtlFlags&MTL_MEDIT_OBJTYPE_MASK)>>MTL_OBJTYPE_SHIFT; } 
			/*! \remarks This method is used internally. */
			void SetMeditObjType(int t) { mtlFlags &= ~MTL_MEDIT_OBJTYPE_MASK; mtlFlags |= t<<MTL_OBJTYPE_SHIFT; }


#ifndef NO_MTLEDITOR_SAMPLEUVTILING // orb 01-08-2002
			/*! \remarks This method is used internally. */
			int GetMeditTiling() { return (mtlFlags&MTL_MEDIT_TILING_MASK)>>MTL_TILING_SHIFT; } 
			/*! \remarks This method is used internally. */
			void SetMeditTiling(int t) { mtlFlags &= ~MTL_MEDIT_TILING_MASK; mtlFlags |= t<<MTL_TILING_SHIFT; }
#endif // NO_MTLEDITOR_SAMPLEUVTILING 

			// recursively determine if there are any multi-materials or texmaps 
			// in tree
			/*! \remarks Implemented by the System.\n\n
			This method may be called to recursively determine if there are any
			multi-materials or texmaps in the tree.
			\return  TRUE if the material or texture map has any mult-materials;
			otherwise FALSE. */
			CoreExport BOOL AnyMulti();

			/*! \remarks This method is used internally.\n\n
				*/
			BOOL TextureDisplayEnabled() { return TestMtlFlag(MTL_TEX_DISPLAY_ENABLED); }

			// Return the "className(instance Name)". 
			// The default implementation should be used in most cases.
			 /*! \remarks Implemented by the System.\n\n
			 This method returns the name that appears in the track view. It returns
			 the "Instance Name(class Name)". For example "Green Glass (Standard)".
			 The default implementation should be used. */
			 CoreExport virtual MSTR GetFullName();

			// Mtls and Texmaps must use this to copy the common portion of 
			// themselves when cloning
			/*! \remarks Implemented by the System.\n\n
			Materials and Texmaps must use this operator to copy the common portion
			of themselves when cloning. */
			CoreExport MtlBase& operator=(const MtlBase& m);

			/*! \remarks This method is called for a plug-in to do any work it needs to do prior
			to rendering. For example this is used by the 3ds Max Mirror and Auto
			Reflect materials to render their bitmaps.
			\param t The current time.
			\param rmc Provides information about the view being rendered and can provide access
			to the global rendering environment information via
			<b>RenderGlobalContext *gc = rmc.GetGlobalContext()</b>. See
			Class RenderMapsContext and Class RenderGlobalContext.
			\return  Nonzero on success; zero on failure. In the case of failure the
			renderer is halted (rendering is cancelled). */
			virtual int BuildMaps(TimeValue t, RenderMapsContext &rmc) { return 1; }

			// This gives the cumulative requirements of the mtl and its
			// tree. The default just OR's together the local requirements
			// of the Mtl with the requirements of all its children.
			// For most mtls, all they need to implement is LocalRequirements,
			// if any.
			/*! \remarks This method returns the cumulative requirements of the material and its
			sub-materials and maps. The default implementation just ORs together the
			local requirements of the material with the requirements of all its
			children. Most materials will only need to implement
			<b>LocalRequirements()</b>.
			\param subMtlNum Specifies the number of the sub-material whose requirements should be
			returned. <b>-1</b> may be used to return a value generated by looping
			over all the sub-materials and ORing together the requirements.
			\return  See \ref materialRequirementsFlags.
			\par Default Implementation:
			The default implementation automatically traverses the sub-material/map
			tree. On any MtlBase that returns TRUE for <b>IsMultiMtl()</b> it will
			only recursively call the sub material (or map) for the <b>subMtlNum</b>
			called. The exception to this is when <b>subMtlNum\<0</b>: in this case
			all sub-mtls and submaps are enumerated. Therefore the
			<b>LocalRequirements()</b> method below only needs to consider the
			material or map itself, not the sub-mtls and sub-maps. */
			CoreExport virtual ULONG Requirements(int subMtlNum); 

			// Specifies various requirements for the material: Should NOT
			// include requirements of its sub-mtls and sub-maps.
			/*! \remarks Specifies various requirements for <b>this</b> material. The value
			returned from this method should <b>not</b> include requirements of its
			sub-materials and sub-maps.
			\param subMtlNum Specifies the number of the sub-material whose requirements should be returned.
			\return  See \ref materialRequirementsFlags.
			\par Default Implementation:
			<b>{ return 0; }</b> */
			virtual ULONG LocalRequirements(int subMtlNum) { return 0; } 

			// This gives the UVW channel requirements of the mtl and its
			// tree. The default implementation just OR's together the local mapping requirements
			// of the Mtl with the requirements of all its children.
			// For most mtls, all they need to implement is LocalMappingsRequired, if any.
			// mapreq and bumpreq will be initialized to empty sets with MAX_MESHMAPS elements
			// set 1's in mapreq for uvw channels required
			// set 1's in bumpreq for bump mapping channels required
			/*! \remarks			This method gives the UVW channel requirements of the material and its
			tree. The default implementation just OR's together the local mapping
			requirements of the material with the requirements of all its children.
			For most materials, all they need to implement is the
			<b>LocalMappingsRequired()</b> method.
			\param subMtlNum Specifies the number of the sub-material whose mapping information is retrieved.
			\param mapreq This array of bits is initialized to an empty set with MAX_MESHMAPS
			elements. Each bit corresponds to a mapping channel. Set a bit to one to
			indicate the material requires the corresponding UVW channel.
			\param bumpreq This array of bits is initialized to an empty set with MAX_MESHMAPS
			elements. Each bit corresponds to a mapping channel. Set a bit to one to
			indicate the material requires the corresponding bump mapping channel. */
			CoreExport virtual void MappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq);

			// Specifies UVW channel requirements for the material: Should NOT
			// include UVW channel requirements of its sub-mtls and sub-maps.
			/*! \remarks			This method specifies UVW channel requirements for the material: This
			method should <b>not</b> include UVW channel requirements of any
			sub-materials and sub-maps.
			\param subMtlNum Specifies the number of the sub-material whose mapping information is retrieved.
			\param mapreq This array of bits is initialized to an empty set with MAX_MESHMAPS
			elements. Each bit corresponds to a mapping channel. Set a bit to one to
			indicate the material requires the corresponding UVW channel.
			\param bumpreq This array of bits is initialized to an empty set with MAX_MESHMAPS
			elements. Each bit corresponds to a mapping channel. Set a bit to one to
			indicate the material requires the corresponding bump mapping channel.
			\par Default Implementation:
			<b>{}</b>
			\par Sample Code:
			All 2D textures that use UVGen or otherwise select mapping channels need
			to implement this method Here's an example:\n\n
			\code	
			void LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq)
			{
				uvGen->MappingsRequired(subMtlNum,mapreq,bumpreq);
			}
			\endcode
			All 3D textures that use the XYZGen to put up a coordinates rollup must
			implement this method. Here's an example:
			\code
			void LocalMappingsRequired(int subMtlNum, BitArray & mapreq,BitArray &bumpreq)
			{
				xyzGen->MappingsRequired(subMtlNum,mapreq,bumpreq);
			}
			\endcode */
			virtual void LocalMappingsRequired(int subMtlNum, BitArray & mapreq, BitArray &bumpreq) {  }

			// This returns true for materials or texmaps that select sub-
			// materials based on mesh faceMtlIndex. Used in 
			// interactive render.
			/*! \remarks This method returns TRUE for materials or textures that select
			sub-materials based on submaterial number (for example a mesh
			<b>faceMtlIndex</b>).\n\n
			The only materials for which this method should return TRUE are those
			that choose to use one of their sub-maps or sub-mtls based on the
			submaterial number. For the majority of materials and maps, they should
			return FALSE, and in that case all of the submaterials and maps are
			enumerated by <b>MtlBase::Requirements()</b>.
			\par Default Implementation:
			<b>{ return FALSE; }</b> */
			virtual  BOOL IsMultiMtl() { return FALSE; }

			// Methods to access sub texture maps of material or texmap
			virtual int MapSlotType(int i) { return defaultSlotType; }

			// This must be called on a sub-Mtl or sub-Map when it is removed,
			// in case it or any of its submaps are active in the viewport.
			/*! \remarks Implemented by the System.\n\n
			This method must be called on a sub-material or sub-map when it is
			removed, in case it or any of its sub-maps are active in the viewport. */
			CoreExport void DeactivateMapsInTree();

			// To make texture & material evaluation more efficient, this function is
			// called whenever time has changed.  It will be called at the
			// beginning of every frame during rendering.
			/*! \remarks A material has a <b>Shade()</b> method, and a texture map has a
			<b>EvalColor()</b> method. These are called by the renderer for every
			pixel. This method is called before rendering begins to allow the plug-in
			to evaluate anything prior to the render so it can store this
			information. In this way this information does not need to be calculated
			over and over for every pixel when <b>Shade()</b> or <b>EvalColor()</b>
			is called. This allows texture and material evaluation to be more
			efficient. It is generally called once at the start of each frame, or
			during interactive playback when the time changes. It is <b>not</b>
			guaranteed to get called on every frame, because if you returned a
			validity interval that covers several frames, your parent material won't
			bother to call you if the current frame is still in that interval.
			\param t The current time.
			\param valid The validity interval to update to reflect the validity interval of the
			material or texture at the time passed. */
			virtual  void Update(TimeValue t, Interval& valid)=0;

			/*! \remarks This method is called to reset the material or texmap back to its default
			values. This should be done by calling the Reset method of the material or 
			texture map plugin's class descriptor (ClassDesc2::Reset). In addition, the 
			plugin can execute any other initialization code it has. 
			Example from maxsdk\\samples\\materials\\dblsided.cpp
			\code
			void DoubleSided::Reset() {
				dblsidedCD.Reset(this, TRUE); // Resets all pb2's
				Init();
			} \endcode
			\note When migrating plugins from ClassDesc to ClassDesc2 and IParamBlock2,
			do not forget to update this method to call the class descriptor's Reset method.
			If the plugin's Reset tries to replace parameter block objects based on
			IParamBlock2 that use the P_AUTO_UI flag, the UI code will continue to hold 
			a pointer to the replaced parameter block, and this will lead to problems.
			*/
			virtual void Reset()=0;

			// call this to determine the validity interval of the mtl or texture
			/*! \remarks This method is called to determine the validity interval of the material
			or texture.
			\param t Specifies the time about which to compute the validity interval.
			\return  The validity interval of the item at the specified time. */
			virtual Interval Validity(TimeValue t)=0;
			
			// this gets called when the mtl or texture is to be displayed in the
			// mtl editor params area.
			/*! \remarks This method gets called when the material or texture is to be displayed
			in the material editor parameters area. The plug-in should allocate a new
			instance of a class derived from <b>ParamDlg</b> to manage the user
			interface.\n\n
			<b>Note:</b> The following is a discussion of this
			<b>CreateParamDlg()</b> method, and the <b>SetDlgThing()</b> method, and
			the way they relate to the ParamMap2 system. A good example for this
			discussion is a texture map which typically has several rollouts, say its
			texmap parameters, a UVW coordinates rollout, and an Output rollout.\n\n
			The normal way for a texmap to implement these other (common) rollouts is
			for the it to create a UVGen instance and a TextureOutput instance as
			'sub-objects' in the map and then ask them to put up their rollouts when
			it is asked to do so by the Materials Editor (in CreateParamDlg()). The
			Materials Editor requires a ParamDlg pointer back from the
			CreateParamDlg() on which it calls methods to control the UI, such as
			time change updates or loading up a new texmap of the same class into the
			UI when the user switches them, so that the whole UI doesn't need to be
			rebuilt.\n\n
			Prior to the ParamMap2 system, a texmap would implement its own ParamDlg
			subclass and would keep track of the UVGen and TextureOutput ParamDialogs
			and pass on any time change or SetThing() calls to them. ParamMap2
			introduced its own ParamDlg subclass
			(Class IAutoMParamDlg) that
			you can ask it to build for you and have manage all interaction with the
			Materials Editor automatically. As before, this still needs to know about
			the other (sub-object) ParamDlgs, and so it has the ability to keep a
			list of 'secondary' ParamDlgs to which it passes on the SetTime()s and
			SetThing()s.\n\n
			When the Materials Editor asks the texmap to CreateParamDlg(), the texmap
			asks its ClassDesc2 to build one of these
			(ClassDesc2::CreateParamDlgs()). If the texmap itself has multiple
			ParamBlock2s, CreateParamDlgs() builds one ParamDlg per pblock/rollout,
			makes the first of them a 'master' and adds the rest as secondary
			ParamDlgs. The texmap then asks the UVGen and TextureOutput objects to
			CreateParamDlg() for their rollouts and adds them to the master
			IAutoMParamDlg also.\n\n
			Now consider the need for the <b>SetDlgThing()</b> method below. It is
			related to the SetThing() method that the Materials Editor calls on the
			'master' ParamDlg to switch into the UI a texmap of the same class as
			that currently in the UI. Normally, the master IAutoParamDlg would
			propagate the SetThing() to its registered secondary ParamDlgs. In the
			case of multiple paramblock2s in the texmap, this would be correct, since
			the 'thing' in this case is the incoming texmap. But this doesn't work
			for the secondary UVGen and TextureOutput ParamDlgs; their 'things' are
			the UVGen and TextureOutput subobjects of the incoming map. So,
			IAutoParamDlg first calls SetDlgThing() on the incoming texmap so that it
			gets a chance to call the correct SetThing()s on the sub-object ParamDlgs
			with the appropriate incoming sub-objects. A clear example of this is in
			<b>Gradient::SetDlgThing()</b> in
			<b>/MAXSDK/SAMPLES/MATERIALS/GRADIENT.CPP</b>. It is called once for each
			secondary ParamDlg. For those ParamDlgs that have special SetThing()
			requirements, it does the appropriate sub-object SetThing() and returns
			TRUE. If it does no special handling for a particular ParamDlg, it
			returns FALSE, signaling to the IAutoMParamDlg that it should do the
			standard SetThing() propagation for that dialog.\n\n
			The Render Effects dialog has a similar arrangement to the Materials Editor
			for controlling Effect UI and so there is an <b>IAutoEParamDlg</b> that
			works exactly the same way as the <b>IAuotMParamDlg</b>.
			\param hwMtlEdit The window handle of the materials editor.
			\param imp The interface pointer for calling methods in 3ds Max.
			\return  A pointer to the created instance of a class derived from
			<b>ParamDlg</b>. */
			virtual ParamDlg* CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp)=0;
			
			// save the common mtlbase stuff.
			// these must be called in a chunk at the beginning of every mtl and
			// texmap.
			/*! \remarks Implemented by the System.\n\n
			This method saves the plug-in's data to disk.. The common MtlBase data
			must be saved as well. The base class method must be called in a chunk at
			the beginning of every Mtl and Texmap.
			\param isave An interface pointer available for saving data. See Class ISave, \ref ioResults.
			\par Sample Code:
			Note in the code below the base class method is called in a chunk before
			the rest of the plug-ins data is saved.\n
			\code
			IOResult Gradient::Save(ISave *isave)
			{
				IOResult res;
				// Save common stuff
				isave->BeginChunk(MTL_HDR_CHUNK);
				res = MtlBase::Save(isave);
				if (res!=IO_OK) return res;
				isave->EndChunk();
			
				for (int i=0; i<NSUBTEX; i++) {
					if (mapOn[i]==0) {
						isave->BeginChunk(MAPOFF_CHUNK+i);
						isave->EndChunk();
					}
				}
				return IO_OK;
			}
			\endcode */
			CoreExport IOResult Save(ISave *isave);
			/*! \remarks Implemented by the System.\n\n
			This method is called to load the material or texture from disk. The
			common MtlBase data must be loaded as well. See the code below.
			\param iload An interface pointer for calling methods to load data. See Class ILoad, \ref ioResults.
			\par Sample Code:
			\code
			IOResult Gradient::Load(ILoad *iload)
			{
				IOResult res;
				int id;
				while (IO_OK==(res=iload->OpenChunk())) {
					switch(id = iload->CurChunkID()) {
						case MTL_HDR_CHUNK:
							res = MtlBase::Load(iload);
							break;
						case MAPOFF_CHUNK+0:
						case MAPOFF_CHUNK+1:
						case MAPOFF_CHUNK+2:
							mapOn[id-MAPOFF_CHUNK] = 0;
							break;
					}
					iload->CloseChunk();
					if (res!=IO_OK)
						return res;
				}
				iload->RegisterPostLoadCallback(new ParamBlockPLCB(versions,NUM_OLDVERSIONS,&curVersion,this,1));
				return IO_OK;
			}
			\endcode */
			CoreExport IOResult Load(ILoad *iload);
			
#ifndef NO_MTLEDITOR_EFFECTSCHANNELS // orb 01-08-2002
			// GBuffer functions
			/*! \remarks Implemented by the System.\n\n
			Returns the G-buffer ID of this material. */
			ULONG GetGBufID() { return gbufID; }
			/*! \remarks Implemented by the System.\n\n
			Sets the G-buffer ID of this material. */
			void SetGBufID(ULONG id) { gbufID = id; }
#endif // NO_MTLEDITOR_EFFECTSCHANNELS
			
			// Default File enumerator.
			/*! \remarks			This is an implementation of the <b>Animatable</b> method. This default
			implementation simply recurses, skipping inactive subTexmaps if
			appropriate. */
			CoreExport void EnumAuxFiles(AssetEnumCallback& nameEnum, DWORD flags);

			// Postage Stamp
			/*! \remarks			Implemented by the System.\n\n
			Returns a pointer to the postage stamp image for the file.
			\param sz One of the following values:\n\n
			<b>PS_SMALL</b> for small (32x32) images.\n
			<b>PS_LARGE</b> for large (88x88) images.\n
			<b>PS_TINY</b> for tiny (24x24) images. */
			CoreExport virtual PStamp* GetPStamp(int sz); // sz = 0: small(32x32), 1: large(88x88), 2: tiny(24x24)      
			/*! \remarks			Implemented by the System.\n\n
			Creates a postage stamp image and returns a pointer to it. If the postage
			stamp image already exists then it is simply returned.\n\n
			Here's an example using this method to display a small material	sample.\n\n
			\code
			void DisplayMB(MtlBase *mb, HDC hdc, int x, int y) {
				mb->CreatePStamp(0,TRUE); // create and render a small pstamp
				PStamp *ps = mb->GetPStamp(0);
				if (ps) {
					int d = PSDIM(0);
					int scanw = ByteWidth(d);
					int nb = scanw*d;
					UBYTE *workImg = new UBYTE[nb];
					if (NULL == workImg)
						return;
					ps->GetImage(workImg);
					Rect rect;
					rect.left = x;
					rect.top = y;
					rect.right = x+d;
					rect.bottom = y+d;
					GetGPort()->DisplayMap(hdc, rect,0,0, workImg, scanw);
					delete [] workImg;
				}
			}
			\endcode
			\param sz One of the following values:\n\n
			<b>PS_SMALL</b> for small (32x32) images.\n
			<b>PS_LARGE</b> for large (88x88) images.\n
			<b>PS_TINY</b> for tiny (24x24) images.
			\param Render If set to true, the postage stamp bitmap will have the <b>MtlBase</b>
			rendered into it automatically. The bitmap can then be retrieved using
			<b>PStamp::GetImage</b>, for drawing in the UI. */
			CoreExport virtual PStamp* CreatePStamp(int sz, BOOL Render = FALSE);         
			/*! \remarks			Implemented by the System.\n\n
			Discards the postage stamp image.
			\param sz One of the following values:\n\n
			<b>PS_SMALL</b> for small (32x32) images.\n
			<b>PS_LARGE</b> for large (88x88) images.\n
			<b>PS_TINY</b> for tiny (24x24) images. */
			CoreExport virtual void DiscardPStamp(int sz);        

			// Schematic View Animatable Overrides...
			CoreExport SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
			CoreExport bool SvHandleDoubleClick(IGraphObjectManager *gom, IGraphNode *gNode);
			CoreExport MSTR SvGetName(IGraphObjectManager *gom, IGraphNode *gNode, bool isBeingEdited);
			CoreExport bool SvCanSetName(IGraphObjectManager *gom, IGraphNode *gNode);
			CoreExport bool SvSetName(IGraphObjectManager *gom, IGraphNode *gNode, MSTR &name);
			CoreExport COLORREF SvHighlightColor(IGraphObjectManager *gom, IGraphNode *gNode);
			CoreExport bool SvIsSelected(IGraphObjectManager *gom, IGraphNode *gNode);
			CoreExport MultiSelectCallback* SvGetMultiSelectCallback(IGraphObjectManager *gom, IGraphNode *gNode);
			CoreExport bool SvCanSelect(IGraphObjectManager *gom, IGraphNode *gNode);

			// Methods for doing interactive texture display
			/*! \remarks Returns TRUE if this texture supports being used in the interactive
			renderer; otherwise FALSE. If the texture does return TRUE it is expected
			to implement the methods <b>ActivateTexDisplay()</b> and
			<b>GetActiveTexHandle()</b>.
			\par Default Implementation:
			<b>{ return FALSE; }</b> */
			virtual BOOL SupportTexDisplay() { return FALSE; }   // supports map display in viewport
			/*! \remarks This method is called to retrieve a texture handle to this texture map.
			\param t The time to return the texture handle.
			\param thmaker This class provides methods for creating a texture handle from a 3ds Max bitmap
			and a Windows DIB. It also has a method to retrieve the required size of the
			texture map. See Class TexHandleMaker.
			\return  The texture handle.
			\par Default Implementation:
			<b>{return 0;}</b> */
			virtual DWORD_PTR GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {return 0;}
			/*! \remarks This method is used internally. */
			CoreExport void IncrActive();
			/*! \remarks This method is used internally. */
			CoreExport void DecrActive(); 
			/*! \remarks This method is used internally. */
			int Active() { return activeCount; }
			/*! \remarks This method is called when the usage of the texture the interactive
			renderer changes. This method must only be implemented if
			<b>SupportTexDisplay()</b> returns TRUE. This method does not cause the
			texture map to be drawn in the viewport but should be called with TRUE as
			the argument before this can occur. For viewport drawing of textures
			refer to <b>Interface::ActivateTexture()</b> and
			<b>Interface::DeActivateTexture()</b> instead.
			\param onoff TRUE if the texture is being used; FALSE if it is no longer being used.
			\par Default Implementation:
			<b>{}</b> */
			virtual void ActivateTexDisplay(BOOL onoff) {}

			//! Get the active MtlBase
			/*! Previous to r9, activeMB was held as a raw pointer. Due to problems where the
			// MtlBase was deleted without clearing this pointer, activeMB is now held indirectly. 
			// Methods GetActiveMB and SetActiveMB are used to get/set the activeMB;
			\return The active MtlBase*/
			CoreExport MtlBase* GetActiveMB();
			//! Set the active MtlBase
			/*! \param[in] activeMB The active MtlBase*/
			CoreExport void SetActiveMB(MtlBase* activeMB);

			// Multiple textures in viewports:
			/*! \remarks						Returns TRUE if this material supports the display of multi-maps in the
			viewports (interactive renderer); FALSE if it doesn't.
			\par Default Implementation:
			<b>{ return FALSE; }</b> */
			virtual BOOL SupportsMultiMapsInViewport() { return FALSE; }
			/*! \remarks						This method is called to initialize the interactive renderer Material
			passed with the properties of this MtlBase.\n\n
			If a <b>MtlBase</b> (material or texmap) wants to display multiple
			textures in the viewports, it implements\n\n
			<b>SupportsMultiMapsInViewport()</b> to return TRUE, and implements
			<b>SetupGfxMultiMaps</b> to store the necessary information in the
			Material passed in, including the <b>TextureInfo</b>'s for each
			texture.\n\n
			The <b>MtlMakerCallback</b> passed in to <b>SetupGfxMultiMaps</b>
			provides functions to help in setting up the "Material" data structure.
			The function <b>NumberTexturesSupported()</b> lets you know the
			capabilities of the current hardware, so you can adapt accordingly. The
			function <b>GetGfxTexInfoFromTexmap</b> fills in the fields of a
			<b>TextureInfo</b> except the texHandle and texture ops.\n\n
			The implementation of <b>SetupGfxMultiMaps</b> must create the
			"texHandle" for each of the textures described in its <b>TextureInfo</b>
			array. It typically does this by calling the submap's
			<b>GetVPDisplayDIB()</b> method, and then creates the texHandle by
			calling the callBack function <b>MakeHandle(bmi)</b>. To avoid doing this
			calculation when not necessary it is best to save the texHandles along
			with their validity intervals. Then when <b>SetupGfxMultiMaps</b> is
			called, if valid texHandles are already in hand they can just be used
			without recomputing.
			\param t The time at which to evaluate the material.
			\param mtl Points to the interactive renderer material to update.
			\param cb This callback object is provided as a helper to fill in the Material
			properties above. See Class MtlMakerCallback.
			\par Default Implementation:
			<b>{}</b> */
			virtual void SetupGfxMultiMaps(TimeValue t, Material *mtl, MtlMakerCallback &cb) {}
			virtual void* GetInterface(ULONG id){ if(id==I_SUBMAP) return (ISubMap*)this; else return ReferenceTarget::GetInterface(id); }
			virtual BaseInterface *GetInterface(Interface_ID id) { return ReferenceTarget::GetInterface(id); }
			virtual ReferenceTarget *GetRefTarget(){return this;}

			// Animatable Properties
			CoreExport virtual int SetProperty(ULONG id, void *data);
			CoreExport virtual void *GetProperty(ULONG id);
			CoreExport virtual void BaseClone(ReferenceTarget *from, ReferenceTarget *to, RemapDir &remap);

			/*! Transparency hint indicates whether the material is potentially transparent for both rendering
			and viewport display, so that the renderers or viewport can decide whether to optimize it or not.
			\param t The time to get the transparency hint at.
			\param valid The validity interval of the returned value.
			\return Nonzero if this material is potentially transparent; Zero if it isn't.
			\par Default Implementation:
			<b>{ return TRUE; }</b> */
			virtual BOOL GetTransparencyHint(TimeValue t, Interval& valid) { return TRUE; }
	 };


// Every MtlBase sub-class defines a ParamDlg to manage its part of
// the material editor.
/*! \sa  Class Mtl, Class Texmap, <a href="ms-its:3dsmaxsdk.chm::/mtls_materials.html">Working with Materials and
Textures</a>.\n\n
\par Description:
Every MtlBase sub-class defines a ParamDlg to manage its part of the material
editor.  */
class ParamDlg : public InterfaceServer {
	 public:
			/*! \remarks Returns the unique Class_ID of the plug-in this dialog manages. See
			Class Class_ID. */
			virtual Class_ID ClassID()=0;
			/*! \remarks This sets the current material or texture being edited to the material or
			texture passed.
			\par Parameters:
			<b>ReferenceTarget *m</b>\n\n
			The material or texture to set as current. */
			virtual void SetThing(ReferenceTarget *m)=0;
			/*! \remarks This method returns the current material or texture being edited. */
			virtual ReferenceTarget* GetThing()=0;
			/*! \remarks This method is called when the current time has changed. This gives the
			developer an opportunity to update any user interface data that may need
			adjusting due to the change in time.
			\par Parameters:
			<b>TimeValue t</b>\n\n
			The new current time. */
			virtual void SetTime(TimeValue t)=0;
			/*! \remarks This method should stuff values into all the parameter dialog's controls,
			edit fields etc. In fact this method is now only called after doing a
			"Reset". It is also useful inside the material/map when a
			<b>NotifyRefChanged()</b> is processed it may need to be called. */
			virtual  void ReloadDialog()=0;
			/*! \remarks This method is called by the system to delete an instance of this class.
			A developer must use the same memory manager to delete the item as they
			did to allocate it (in <b>MtlBase::CreateParamDlg()</b>). For example if
			a developer used the <b>new</b> operator to allocate the memory, this
			method would be implemented as <b>{ delete this; }</b> */
			virtual void DeleteThis()=0;
			/*! \remarks This method is called when the dialog box becomes active or inactive.
			Currently this is used when working with color swatch custom controls.
			Color swatches need to know when the dialog box becomes active or
			inactive because they change their method of drawing themselves. When the
			dialog is active, color swatches are drawn in pure color (this requires a
			color register in the palette). When the dialog becomes inactive the
			color swatches are drawn using dithered painting. This is needed because
			there are only 8 available color registers.\n\n
			A method of the color swatch control is called to indicate if it is in an
			active or inactive dialog. See the sample code below. Also see
			Class IColorSwatch.
			\par Parameters:
			<b>BOOL onOff</b>\n\n
			TRUE if the dialog is active; otherwise FALSE.
			\par Sample Code:
			\code
			void NoiseDlg::ActivateDlg(BOOL onOff)
			{
				for (int i=0; i<NCOLS; i++)
					cs[i]->Activate(onOff);
			}
			\endcode */
			virtual void ActivateDlg(BOOL onOff)=0;
			// These need to be implemented if using a TexDADMgr or a MtlDADMgr
			/*! \remarks			This method needs to be implemented if the plug-in texmap is using a
			<b>TexDADMgr</b>. It should return the index of the sub-texmap
			corresponding to the window whose handle is passed. If the handle is not
			valid return -1.
			\par Parameters:
			<b>HWND hwnd</b>\n\n
			The window handle to check.
			\par Default Implementation:
			<b>{ return -1;}</b>
			\par Sample Code:
			\code
			int CellTexParamDlg::FindSubTexFromHWND(HWND hw)
			{
				if (hw==GetDlgItem(pmap->GetHWnd(),IDC_CELLTEX_CELLCOLOR_MAP))
					return 0;
				if (hw==GetDlgItem(pmap->GetHWnd(),IDC_CELLTEX_DIVCOL1_MAP))
					return 1;
				if (hw==GetDlgItem(pmap->GetHWnd(),IDC_CELLTEX_DIVCOL2_MAP))
					return 2;
				return -1;
			}
			\endcode */
			virtual int FindSubTexFromHWND(HWND hwnd){ return -1;} 
			/*! \remarks			This method needs to be implemented if the plug-in material is using a
			<b>MtlDADMgr</b>. It should return the index of the sub-map corresponding
			to the window whose handle is passed. If the handle is not valid return
			-1.
			\par Parameters:
			<b>HWND hwnd</b>\n\n
			The window handle to check.
			\par Default Implementation:
			<b>{ return -1;}</b> */
			virtual int FindSubMtlFromHWND(HWND hwnd){ return -1;} 
			// Generic expansion function
			virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 

	 };

// Pre-defined categories of texture maps
CoreExport MCHAR TEXMAP_CAT_2D[];  // 2D maps
CoreExport MCHAR TEXMAP_CAT_3D[];  // 3D maps
CoreExport MCHAR TEXMAP_CAT_COMP[]; // Composite
CoreExport MCHAR TEXMAP_CAT_COLMOD[]; // Color modifier
CoreExport MCHAR TEXMAP_CAT_ENV[];  // Environment

/*! \sa  Class Texmap.\n\n
\par Description:
This class provides a method <b>AddMapName()</b> that is called when a Texmap
is attempting to load its map and it cannot be found.  */
class NameAccum: public MaxHeapOperators {
	 public:
			/*! \remarks Destructor. */
			virtual ~NameAccum() {;}
			/*! \remarks Implemented by the System.\n\n
			This method is called to add the name of a map that was not found.
			\par Parameters:
			<b>MCHAR *name</b>\n\n
			The name to add. */
			virtual  void AddMapName(MCHAR *name)=0;
	 };

// this interface is a dummy interface returned by texmap::getinterface
// if the map is a ray tracing type map.
#define IID_RAYTRACE_MAP   0xff8d87a5

class IRayTexmap : public InterfaceServer {
public:
	 bool isRayTraceMap() { return true; }
};

// virtual texture map interface
/*! \sa  Class MtlBase, Class ShadeContext, Class UVGen, Class XYZGen, Class TexHandleMaker, Class NameAccum, Class AColor, Class Matrix3, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_procedural_texture_clamping.html">List of Procedural
Texture Clamping, Noise and Misc Functions</a>.\n\n
\par Description:
This is the base class for the creation of texture map plug-ins. It provides
methods for things such as calculating the color of the map for a given
location, and computing a perturbation to apply to a normal for bump
mapping.\n\n
Note: Developers creating procedural textures should be aware that these
textures may appear less than perfect when a 3ds Max user is using the "Quick
Renderer" in the Materials Editor. This is not the fault of the plug-in
texture, it is simply that the "Quick Renderer" uses an algorithm that is
quicker but less accurate than the standard scanline renderer. Therefore, don't
be concerned if your texture does not show up perfectly when using "Quick
Renderer".
\par Plug-In Information:
Class Defined In IMTL.H\n\n
Super Class ID TEXMAP_CLASS_ID\n\n
Standard File Name Extension DLT\n\n
Extra Include Files Needed IMTL.H (and optionally TEXUTIL.H)
\par Method Groups:
See <a href="class_texmap_groups.html">Method Groups for Class Texmap</a>.
*/
class Texmap: public MtlBase {
	 public:
			int cacheID;    // used by renderer for caching returned values
						
			/*! \remarks Constructor. The number of active uses of this texture is
			set to 0. */
			CoreExport Texmap();

			SClass_ID SuperClassID() { return TEXMAP_CLASS_ID; }
			virtual void GetClassName(MSTR& s) { s= MSTR(_M("Texture")); }  

			// Evaluate the color of map for the context.
			/*! \remarks This method is called to evaluate the color of the texture map for the
			context. This is for channels that have a color such as diffuse,
			specular, ambient, etc. This method is called for every pixel of the
			texture.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			Describes the properties of the pixel to evaluate.
			\return  An <b>AColor</b> object which stores the <b>r, g, b, a</b>
			values. Note: The alpha is premultiplied, and the alpha value goes into
			<b>AColor::a</b>. */
			virtual  AColor EvalColor(ShadeContext& sc)=0;
	 
			// Evaluate the map for a "mono" channel.
			// this just permits a bit of optimization 
			/*! \remarks Evaluate the map for a "mono" channel. Mono channels are those that don't
			have a color, but rather a single value. This is for things like
			shininess, transparency, etc. This just permits a bit of optimization.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			Describes the properties of the pixel to evaluate.
			\return  A floating point value for the mono channel.
			\par Default Implementation:
			<b>{return Intens(EvalColor(sc));}</b> */
			virtual  float  EvalMono(ShadeContext& sc) {
				 return Intens(EvalColor(sc));
				 }
			
			// For Bump mapping, need a perturbation to apply to a normal.
			// Leave it up to the Texmap to determine how to do this.
			/*! \remarks This method is used for bump mapping to retrieve a perturbation to apply
			to a normal.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			Describes the properties of the pixel to evaluate.
			\return  A deflection vector for perturbing the normal. */
			virtual  Point3 EvalNormalPerturb(ShadeContext& sc)=0;

			// This query is made of maps plugged into the Reflection or 
			// Refraction slots:  Normally the view vector is replaced with
			// a reflected or refracted one before calling the map: if the 
			// plugged in map doesn't need this , it should return TRUE.
			/*! \remarks			This query is made of maps plugged into the Reflection or Refraction
			slots: Normally the view vector is replaced with a reflected or refracted
			one before calling the map: if the plugged in map doesn't need this, it
			should return TRUE.
			\par Default Implementation:
			<b>{ return FALSE; }</b> */
			virtual BOOL HandleOwnViewPerturb() { return FALSE; }


			// This gets the viewport display bitmap in DIB format, useful when combining several maps
			// for hardware-supported multiple texture display.  // DS 4/17/00
			// IF mono=TRUE, the map should do a mono evaluation and place the result in RGB.
			// forceW and forceH, if non-zero, override dimensions specified by thmaker.
			/*! \remarks			This gets the viewport display bitmap in DIB format, useful when
			combining several maps for hardware-supported multiple texture display.
			If <b>mono</b> is TRUE, the map should do a mono evaluation and place the
			result in RGB. <b>forceW</b> and <b>forceH</b>, if non-zero, override
			dimensions specified by <b>thmaker</b>.
			\par Parameters:
			<b>TimeValue t</b>\n\n
			The time to get the bitmap at.\n\n
			<b>TexHandleMaker\& thmaker</b>\n\n
			This class provides methods for creating a texture handle from a 3ds Max
			bitmap and a Windows DIB. It also has a method to retrieve the required
			size of the texture map. See Class TexHandleMaker.\n\n
			<b>Interval \&valid</b>\n\n
			The validity interval of the returned bitmap.\n\n
			<b>BOOL mono</b>\n\n
			Indicates whether a map should do mono evaluation.\n\n
			<b>int forceW</b>\n\n
			Overrides the bitmap width usually supplied by <b>thmaker</b>.\n\n
			<b>int forceH</b>\n\n
			Overrides the bitmap height usually supplied by <b>thmaker</b>. */
			CoreExport virtual BITMAPINFO* GetVPDisplayDIB(TimeValue t, TexHandleMaker& thmaker, Interval &valid, BOOL mono=FALSE, int forceW=0, int forceH=0);  

			/*! \remarks This method is called to retrieve the UV transformation matrix for use in
			the viewports. If a developer is using an instance of UVGen, a method of
			that class may be called to retrieve the value:\n\n
			(i.e. <b>{ uvGen-\>GetUVTransform(uvtrans); }</b> ).
			\par Parameters:
			<b>Matrix3 \&uvtrans</b>\n\n
			The transformation matrix is returned here.
			\par Default Implementation:
			<b>{}</b> */
			virtual  void GetUVTransform(Matrix3 &uvtrans) { uvtrans.IdentityMatrix(); }
			/*! \remarks This method is called to get the tiling state of the texture for use in
			the viewports. This is described by a set of symmetry flags that may be
			ORed together. If you are using an instance of UVGen to handle the UV
			user interface you may simply call a method of UVGen to handle this.\n\n
			For example: <b>{ return uvGen-\>GetTextureTiling(); }</b>
			\return  See \ref textureSymmetryFlags.
			\par Default Implementation:
			<b>{ return U_WRAP|V_WRAP; }</b> */
			virtual int GetTextureTiling() { return  U_WRAP|V_WRAP; }
			/*! \remarks This method is called to initialize the slot type. This sets the proper
			button in the coordinate user interface rollup page. If you are using an
			instance of UVGen to handle the UV user interface you may simply call a
			method of UVGen to handle this. For example: <b>{ if (uvGen)
			uvGen-\>InitSlotType(sType); }</b>
			\par Parameters:
			<b>int sType</b>\n\n
			See \ref mapSlotTypes.
			\par Default Implementation:
			<b>{}</b> */
			virtual void InitSlotType(int sType) { defaultSlotType = sType; }          
			virtual int MapSlotType(int i) { return defaultSlotType; }
			/*! \remarks			Returns a value indicating where to get the texture vertices for the
			Texmap.
			\return  One of the following values:\n\n
			<b>UVWSRC_EXPLICIT</b>\n\n
			Use explicit mesh texture vertices from one of the mapping channels (see
			<b>GetMapChannel()</b> below to determine which one). This uses the UVW
			coordinates assigned to the object, either through the Generate Mapping
			Coordinates option in the object's creation parameters, or through
			mapping modifiers, such as UVW Map.\n\n
			<b>UVWSRC_EXPLICIT2</b>\n\n
			Use explicit mesh texture vertices from the Vertex Color Channel.\n\n
			<b>UVWSRC_OBJXYZ</b>\n\n
			Generate planar UVW mapping coordinates from the object local XYZ
			on-the-fly. This corresponds to the "Planar from Object XYZ" option.\n\n
			<b>UVWSRC_WORLDXYZ</b>\n\n
			This value is available in release 3.0 and later only.\n\n
			Generate planar UVW mapping coordinates from the world XYZ on-the-fly.
			This corresponds to the "Planar From World XYZ" option. Note: this value
			used for the UVW is the world XYZ, taken directly, with out normalization
			to the objects bounding box. This differs from "Planar from Object XYZ",
			where the values are normalized to the object's bounding box.
			\par Default Implementation:
			<b>{ return UVWSRC_EXPLICIT; }</b> */
			virtual int GetUVWSource() { return UVWSRC_EXPLICIT; }
			/*! \remarks			Returns the map channel being used by the texmap if <b>GetUVWSource()</b>
			returns <b>UVWSRC_EXPLICIT</b>. The return value should be at least 1. A
			value of 0 is not acceptable.
			\par Default Implementation:
			<b>{ return 1; }</b> */
			virtual int GetMapChannel () { return 1; }   // only relevant if above returns UVWSRC_EXPLICIT

			/*! \remarks			Texture maps that use a UVGen should implement this method to return a
			pointer to it.
			\par Default Implementation:
			<b>{ return NULL; }</b> */
			virtual UVGen *GetTheUVGen() { return NULL; }  // maps with a UVGen should implement this
			/*! \remarks			Texture maps that use a XYZGen should implement this method to return a
			pointer to it.
			\par Default Implementation:
			<b>{ return NULL; }</b> */
			virtual XYZGen *GetTheXYZGen() { return NULL; } // maps with a XYZGen should implement this

			// System function to set slot type for all subtexmaps in a tree.
			/*! \remarks Implemented by the System.\n\n
			This method is used internally to set the slot type for all subtexmaps in
			a tree. */
			CoreExport void RecursInitSlotType(int sType);           
			/*! \remarks Sets the output level at the specified time. It is used to set the output
			level of the embedded Texout object, principally by importing plug-ins.
			It is implemented in all Texmaps.
			\par Parameters:
			<b>TimeValue t</b>\n\n
			The time to set the output level.\n\n
			<b>float v</b>\n\n
			The value to set.
			\par Default Implementation:
			<b>{}</b> */
			virtual void SetOutputLevel(TimeValue t, float v) {}

			// called prior to render: missing map names should be added to NameAccum.
			// return 1: success,   0:failure. 
			/*! \remarks This method is called prior to rendering to allow the plug-in to load any
			bitmap files it requires.\n\n
			Note that <b>LoadMapFiles()</b> is called to load map files only, not to
			list the missing files. The missing files are listed using the
			<b>EnumAuxFiles()</b> method, which allows enumerating them without
			loading them.\n\n
			Also Note: There is currently not an <b>UnloadMapFiles()</b> method.
			There are a couple of ways to do this however. One is to call
			<b>Interface::FreeAllBitmaps()</b>. That method traverses the scene
			reference hierarchy and calls <b>Animatable::FreeAllBitmaps()</b> on each
			item. Another approach is to evaluate the Material / TextureMap hierarchy
			on each material. Then call <b>Animatable::FreeAllBitmaps()</b> yourself
			in the <b>MtlEnum::proc()</b> shown below.\n\n
			\code
			class MtlEnum	
			{
				virtual void proc(MtlBase *m) = 0;
			};
			
			void EnumMtlTree(MtlBase *mb, MtlEnum &tenum)
			{
				tenum.proc(mb);
				for (int i=0; i<mb->NumSubTexmaps(); i++) {
					Texmap *st = mb->GetSubTexmap(i);
					if (st)
						EnumMtlTree(st,tenum);
				}
				if (IsMtl(mb)) {
					Mtl *m = (Mtl *)mb;
					for (i=0; i<m->NumSubMtls(); i++) {
						Mtl *sm = m->GetSubMtl(i);
						if (sm)
							EnumMtlTree(sm,tenum);
					}
				}
			}
			\endcode 
			Now just define a subclass of MtlEnum that does what you want, and call
			EnumMtlTree. In this particular case it is more efficient than
			enumerating the entire reference hierarchy. If you do want to enumerate
			the entire reference hierarchy, here's how:\n\n
			\code
			class RefEnumProc
			{
				virtual void proc(ReferenceMaker *rm)=0;
			};
			
			void EnumRefs(ReferenceMaker *rm, RefEnumProc &proc)
			{
				proc.proc(rm);
				for (int i=0; i<rm->NumRefs(); i++) {
					ReferenceMaker *srm = rm->GetReference(i);
					if (srm) EnumRefs(srm,proc);
				}
			}
			\endcode
				Just define a subclass of RefEnumProc that does what you want, and call
					EnumRefs on the part of the reference hierarchy you want to enumerate.
					For example:\n\n
			\code
			class MyEnum: public RefEnumProc
			{
				void proc(ReferenceMaker *rm) {
					// do something ...
				}
			}
			
			void afunction(Mtl* m)
			{
				MyEnum enumer;
				EnumRefs(m,&enumer);
			}
			\endcode 
			\par Parameters:
			<b>TimeValue t</b>\n\n
			The time the maps are being loaded.
			\return  Always return nonzero from this method.
			\par Default Implementation:
			<b>{ return 1; }</b> */
			virtual int LoadMapFiles(TimeValue t) { return 1; } 

			// render a 2-d bitmap version of map.
			/*! \remarks			This method is used to render a 2D bitmap version of this texmap.
			\par Parameters:
			<b>TimeValue t</b>\n\n
			The time at which to render the texmap to the bitmap.\n\n
			<b>Bitmap *bm</b>\n\n
			A pointer to a bitmap to render to. This bitmap must be created at the
			resolution you wish to render to.\n\n
			<b>float scale3D=1.0f</b>\n\n
			This is a scale factor applied to 3D Texmaps. This is the scale of the
			surface in 3d space that is mapped to UV. This controls how much of the
			texture appears in the bitmap representation.\n\n
			<b>BOOL filter = FALSE</b>\n\n
			If TRUE the bitmap is filtered. It is quite a bit slower to rescale
			bitmaps with filtering on. */
			/*! \remarks			Renders the texmap to the specified bitmap.
			\par Parameters:
			<b>TimeValue t</b>\n\n
			The time at which to render the bitmap.\n\n
			<b>Bitmap *bm</b>\n\n
			The result is stored in this bitmap. The properties of this bitmap define
			the resolution, color depth, etc.\n\n
			<b>float scale3D=1.0f</b>\n\n
			This is the scale of the surface in 3d space that is mapped to UV.\n\n
			<b>BOOL filter = FALSE</b>\n\n
			If TRUE the bitmap is filtered. It is quite a bit slower to rescale
			bitmaps with filtering on.
			\par Default Implementation:
			The default implementation calls <b>Interface::RenderTexmap()</b>. */
			CoreExport virtual void RenderBitmap(TimeValue t, Bitmap *bm, float scale3D=1.0f, BOOL filter = FALSE);

			CoreExport void RefAdded(RefMakerHandle rm);

			// Schematic View Animatable Overrides...
			CoreExport SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);

			// --- Texmap evaluation ---

			// Returns true only if all submaps and itself have a meaningful output.
			// Returns false if at least one sutexmap or itself does not have a 
			// meaningful output,
			/*! \remarks			Implemented by the system.\n\n
			Returns true only if all submaps and itself have a meaningful output.\n\n
			Returns false if at least one sub-texmap or itself does not have a meaningful
			output\n\n
			The output of a texmap is meaningful in a given ShadeContext if it is the same
			as when the scene is rendered. If the map cannot determine whether the output
			value is the same as when rendered, it should not be meaningful. This method
			can be called before <b>EvalColor()</b> or <b>EvalMono()</b> on a texmap in
			order to decide whether to call these methods at all or if their return values
			should be used in further calculations.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			This describes the context of the question. */
			CoreExport virtual bool IsOutputMeaningful( ShadeContext& sc );

			// The default implementation returns false
			/*! \remarks			Returns TRUE if the output of <b>this</b> texmap is meaningful for the given
			context; it should not take into account subtexmaps. This method is called by
			<b>IsOutputMeaningful()</b>.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			This describes the context of the question.
			\par Default Implementation:
			<b>{ return false; }</b> */
			virtual bool IsLocalOutputMeaningful( ShadeContext& sc ) { return false; }

			// Is the texture High Dynamic Range?
			/*! \remarks			Returns nonzero if the texture is returning high dynamic range data;
			otherwise zero.
			\par Default Implementation:
			<b>{ return false; }</b> */
			virtual int IsHighDynamicRange( ) const { return false; }
	 };


/*! \sa  Class ReferenceTarget, Class Texmap.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is simply a class that provides an implementation of
<b>Animatable::SuperClassID()</b> to return <b>TEXMAP_CONTAINER_CLASS_ID</b>.
In 3ds Max 2.0 and later this new super class has been added. This is used by
the Standard material to contain its Texmaps. The track view filter code has
been modified so it now looks for this class and will filter it out when maps
are being filtered out, instead of having special purpose code for the Standard
Material. This will permit plug-in developers to put their Texmaps down in a
sub-directory like the Standard material does. */
class TexmapContainer: public ReferenceTarget {
	 SClass_ID SuperClassID() { return TEXMAP_CONTAINER_CLASS_ID; }
	 CoreExport SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
	 };

// ID's for different DynamicProperties: passed into
// GetDynamicProperty()

#define DYN_BOUNCE 1
#define DYN_STATIC_FRICTION 2
#define DYN_SLIDING_FRICTION 3

//! Class ID used for the Gnormal and VDM classes found in maxsdk\samples\materials\NormalBump\normalrender.h & vdm.h
#define GNORMAL_CLASS_ID	Class_ID(0x243e22c6, 0x63f6a014)
#define VDM_CLASS_ID		Class_ID(0x152c55c7, 0x26160e93)

//! Used for normal and vector displacement map generation. 
//! \Note Moved to SDK since FBX also depends on this
//-- UI and Param Block
enum Normal_Block_IDs
{
	gnormal_params
};
enum Normal_Param_IDs
{ 
	gn_mult_spin,
	gn_bmult_spin,
	gn_map_normal,
	gn_map_bump,
	gn_map1on,
	gn_map2on,
	gn_method,
	gn_flip_red,
	gn_flip_green,
	gn_swap_rg
};

enum VDM_Block_IDs
{
	vdm_params
};

enum VDM_Param_IDs
{
	vdm_mult_spin,				// Scaling value (float)
	vdm_map_vector_enabled,		// Whether the map is enabled or not (bool)
	vdm_map_vector,				// A sub-map defining the actual vector data as a Texmap 
	vdm_is_hdr,					// Whether the sub-map is high-dynamic range and doesn't need re-scaling (i.e. value = pixel * 2.0 - 1.0). (bool)
	vdm_method					// Space the vector map is defined in [0==World, 1==Object, 2==Tangent] (int)
};


// virtual material interface
/*! \sa  Class MtlBase, Class Texmap, Class ShadeContext, Class ShadeOutput, Class Interval, Class Color.\n\n
\par Description:
This is the base class for the creation of material plug-ins. This class
provides methods to do things such as compute the properties of the material
for a given location, return the number of sub-materials and access the
properties of the material for the interactive renderer.
\par Plug-In Information:
Class Defined In IMTL.H\n\n
Super Class ID MATERIAL_CLASS_ID\n\n
Standard File Name Extension DLT\n\n
Extra Include File Needed IMTL.H
\par Method Groups:
See <a href="class_mtl_groups.html">Method Groups for Class Mtl</a>.
*/
class Mtl: public MtlBase {
	 RenderData *renderData;
	 public:

			/*! \remarks Constructor. The active texture map is set to NULL. */
			CoreExport Mtl();
			SClass_ID SuperClassID() { return MATERIAL_CLASS_ID; }
			virtual void GetClassName(MSTR& s) { s= MSTR(_M("Mtl")); }  

			/*! \remarks Implemented by the System.\n\n
			Returns a pointer to the active texture map used in the interactive
			renderer. */
			CoreExport MtlBase* GetActiveTexmap();
			/*! \remarks Implemented by the System.\n\n
			Stores the pointer to the active texture map used in the interactive
			renderer. Note that this method does not do everything required to update
			the viewports with the new texmap. To accomplish that call
			<b>Interface::ActivateTexture()</b>. */
			CoreExport void SetActiveTexmap(MtlBase *txm);

			CoreExport void RefDeleted();
			CoreExport void RefAdded(RefMakerHandle rm);  

			// Must call Update(t) before calling these functions!
			// Their purpose is for setting up the material for the
			// GraphicsWindow renderer.
			/*! \remarks Returns the ambient color of the specified material for use in the
			interactive renderer.
			\param mtlNum This is the material index for mult-materials.
			\param backFace If the surface normal of the face is pointing away from the viewer this
			will be TRUE; otherwise FALSE. */
			virtual Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE)=0;
			/*! \remarks Returns the diffuse color of the specified material for use in the
			interactive renderer.
			\param mtlNum This is the material index for mult-materials.
			\param backFace If the surface normal of the face is pointing away from the viewer this
			will be TRUE; otherwise FALSE. */
			virtual Color GetDiffuse(int mtlNum=0, BOOL backFace=FALSE)=0;     
			/*! \remarks Returns the specular color of the specified material for use in the
			interactive renderer.
			\param mtlNum This is the material index for mult-materials.
			\param backFace If the surface normal of the face is pointing away from the viewer this
			will be TRUE; otherwise FALSE. */
			virtual Color GetSpecular(int mtlNum=0, BOOL backFace=FALSE)=0;
			/*! \remarks Returns the shininess value of the specified material for use in the
			interactive renderer.
			\param mtlNum This is the material index for mult-materials.
			\param backFace If the surface normal of the face is pointing away from the viewer this
			will be TRUE; otherwise FALSE. */
			virtual float GetShininess(int mtlNum=0, BOOL backFace=FALSE)=0;
			/*! \remarks Returns the shininess strength value of the specified material for use in
			the interactive renderer.
			\param mtlNum This is the material index for mult-materials.
			\param backFace If the surface normal of the face is pointing away from the viewer this
			will be TRUE; otherwise FALSE. */
			virtual  float GetShinStr(int mtlNum=0, BOOL backFace=FALSE)=0;      
			/*! \remarks Returns the transparency value of the specified material for use in the
			interactive renderer.
			\param mtlNum This is the material index for mult-materials.
			\param backFace If the surface normal of the face is pointing away from the viewer this
			will be TRUE; otherwise FALSE. */
			virtual float GetXParency(int mtlNum=0, BOOL backFace=FALSE)=0;

			// >>>>> Self-Illum
			/*! \remarks			Returns TRUE if the Self Illumination Color is on (checked) for the
			specified material; otherwise FALSE.
			\param mtlNum This is the material index for mult-materials.
			\param backFace If the surface normal of the face is pointing away from the viewer this
			will be TRUE; otherwise FALSE.
			\par Default Implementation:
			<b>{ return TRUE; }</b> */
			virtual BOOL GetSelfIllumColorOn(int mtlNum=0, BOOL backFace=FALSE) { return TRUE; }
			/*! \remarks Returns the self illumination value of the specified material for use in
			the interactive renderer.
			\param mtlNum This is the material index for mult-materials.
			\param backFace If the surface normal of the face is pointing away from the viewer this
			will be TRUE; otherwise FALSE.
			\par Default Implementation:
			<b>{ return 0.0f; }</b> */
			virtual float GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE) { return 0.0f; }
			/*! \remarks			Returns the Self Illumination Color of the specified material for use in
			the interactive renderer.
			\param mtlNum This is the material index for mult-materials.
			\param backFace If the surface normal of the face is pointing away from the viewer this
			will be TRUE; otherwise FALSE.
			\par Default Implementation:
			<b>{ Color c( .0f,.0f,.0f); return c; }</b> */
			virtual Color GetSelfIllumColor(int mtlNum=0, BOOL backFace=FALSE){ Color c( .0f,.0f,.0f); return c; }

			// sampler
			/*! \remarks Returns a pointer to the Sampler used for the specified sub-material.
			\param mtlNum This is the material index for mult-materials.
			\param backFace If the surface normal of the face is pointing away from the viewer this
			will be TRUE; otherwise FALSE.
			\par Default Implementation:
			<b>{ return NULL; }</b> */
			virtual Sampler * GetPixelSampler(int mtlNum=0, BOOL backFace=FALSE){ return NULL; }

			// used by the scanline renderer
			/*! \remarks Returns the wire size of the material.
			\param mtlNum This is the material index for mult-materials.
			\param backFace If the surface normal of the face is pointing away from the viewer this
			will be TRUE; otherwise FALSE.
			\par Default Implementation:
			<b>{ return 1.0f; }</b> */
			virtual float WireSize(int mtlNum=0, BOOL backFace=FALSE) { return 1.0f; }

			// why incomplete sets?? 
			// used onlt for default material creation....add no more
			/*! \remarks This method saves the specified color at the specified time.
			\param c The color to store.
			\param t The time to set this color. */
			virtual void SetAmbient(Color c, TimeValue t)=0;      
			/*! \remarks This method saves the specified color at the specified time.
			\param c The color to store.
			\param t The time to set this color. */
			virtual void SetDiffuse(Color c, TimeValue t)=0;      
			/*! \remarks This method saves the specified color at the specified time.
			\param c The color to store.
			\param t The time to set this color. */
			virtual void SetSpecular(Color c, TimeValue t)=0;
			/*! \remarks This method saves the specified shininess at the specified time.
			\param v The shininess value to store.
			\param t The time to set this color. */
			virtual void SetShininess(float v, TimeValue t)=0;    
			/*! \remarks			This method returns TRUE if the material supports shaders, otherwise
			FALSE.
			\par Default Implementation:
			<b>{ return FALSE; }</b> */
			virtual BOOL SupportsShaders() { return FALSE; } // moved from class StdMat

			// if supporting render elements
			virtual BOOL SupportsRenderElements() { return FALSE; } 

/*************
// begin - ke/mjm - 03.16.00 - merge reshading code
			// if supporting shaders, plug-in mtl should inherit from StdMat2
			enum ReshadeRequirements{ RR_None = 0, RR_NeedPreshade, RR_NeedReshade }; // mjm - 06.02.00

			virtual BOOL SupportsReShading(ShadeContext& sc) { return FALSE; }
			// GetReshadeValidity is called by the interactive reshader after being notified that the material has changed.
			// the return value indicates if the material is still valid, or needs to be preshaded or reshaded. the value
			// should pertain only to the latest change to the material
			// if a material doesn't support reshading, it doesn't need to override this function -- any change will cause
			// the nodes to which it's attached to be rerendered.
			virtual ReshadeRequirements GetReshadeRequirements() { return RR_NeedPreshade; } // mjm - 06.02.00
			virtual void PreShade(ShadeContext& sc, IReshadeFragment* pFrag ) { }
			virtual void PostShade(ShadeContext& sc, IReshadeFragment* pFrag, int& nextTexIndex) { }
// end - ke/mjm - 03.16.00 - merge reshading code
***********/

			// The main method: called by the renderer to compute color and transparency
			// output returned in sc.out
			/*! \remarks This is the main method of the material. This is called by the renderer
			to compute the color and transparency output returned in <b>sc.out</b>.
			\param sc Describes properties of the pixel to be shaded. The result of this method
			is returned in the <b>ShadeOutput</b> data member of <b>sc</b>. */
			virtual void Shade(ShadeContext& sc)=0;

			// Methods to access sub-materials of meta-materials 
			/*! \remarks Returns the number of sub-materials managed by this material.
			\par Default Implementation:
			<b>{ return 0; }</b> */
			virtual int NumSubMtls() { return 0; }
			/*! \remarks Returns a pointer to the 'i-th' sub-material of this material.
			\param i The index of the material to return.
			\par Default Implementation:
			<b>{ return NULL; }</b> */
			virtual Mtl* GetSubMtl(int i) { return NULL; }
			/*! \remarks Stores the 'i-th' sub-material of this material.
			\param i The index of the material to store.
			\param m The material pointer to store.
			\par Default Implementation:
			<b>{}</b> */
			virtual void SetSubMtl(int i, Mtl *m) { }
			/*! \remarks			This method returns which sub-mtl is to display in the viewport, a return
			value of:-1 indicates not implemented.\n\n
			Note that when a material, such as Blend, has a method of selecting which
			sub-map is to be shown in the viewport, and implements this method, the
			materials editor lets you turn on Show Map In Viewport (SMIV) in all the
			sub maps at once. When the material, such as top-bottom, doesn't have a
			selector, and doesn't implement this method, then the materials editor
			will only let you turn on SMIV for one map/mtl in the entire sub-tree of
			the material.
			\par Default Implementation:
			<b>{ return -1; }</b> */
			virtual int VPDisplaySubMtl() { return -1; }  // which sub-mtl to display in the viewport: -1 indicates not implemented.
			/*! \remarks This method returns the slot name of the 'i-th' sub-material. This name
			appears in the materials editor dialog. For instance, if you are in a
			material and then you go down into a sub-material, this is the name that
			appears just below the 'Get Material' icon. For example, in the
			Multi/Sub-Object material when you choose one of the sub-materials, the
			map name appears to let you know which slot you are working on. For the
			Multi/Sub-Object material, this is the number of the slot, i.e."#1:",
			"#2:", "#3:", etc.
			\param i The index of the sub-materials whose slot name should be returned. */
			CoreExport virtual MSTR GetSubMtlSlotName(int i);
			/*! \remarks Returns the name of the 'i-th' sub-material that should appear in track
			view.
			\param i The index of the sub-materials whose track view name should be returned. */
			CoreExport MSTR GetSubMtlTVName(int i);                 
			CoreExport void CopySubMtl(HWND hwnd, int ifrom, int ito);

			// Dynamics properties
			/*! \remarks			This method returns the specified dynamics property of the material at
			the specified time.
			\param t The time to return the dynamics property.
			\param mtlNum The index of the sub-material or zero if this is a base material.
			\param propID Specifies the type of property. One of the following values:\n\n
			<b>DYN_BOUNCE</b>\n
			The bounce coefficient. Values in the range 0.0 to 1.0.\n\n
			<b>DYN_STATIC_FRICTION</b>\n
			The static friction property. Values in the range 0.0 to 1.0.\n\n
			<b>DYN_SLIDING_FRICTION</b>\n
			The sliding friction property. Values in the range 0.0 to 1.0.
			\par Default Implementation:
			The default implementation for Mtl will handle all multi-materials. All
			root level materials (for instance Standard) need to implement this
			method. */
			CoreExport virtual float GetDynamicsProperty(TimeValue t, int mtlNum, int propID);
			/*! \remarks			This method sets the specified dynamics property of the material at the
			specified time.
			\param t The time at which to set the value.
			\param mtlNum The sub-material number for a multi-material.
			\param propID Specifies the type of property. One of the following values:\n\n
			<b>DYN_BOUNCE</b>\n
			The bounce coefficient. Values in the range 0.0 to 1.0.\n\n
			<b>DYN_STATIC_FRICTION</b>\n
			The static friction property. Values in the range 0.0 to 1.0.\n\n
			<b>DYN_SLIDING_FRICTION</b>\n
			The sliding friction property. Values in the range 0.0 to 1.0.
			\param value The value to set.
			\par Default Implementation:
			The default implementation for Mtl will handle all multi-materials. All
			root level materials (for instance Standard) need to implement this
			method. */
			CoreExport virtual void SetDynamicsProperty(TimeValue t, int mtlNum, int propID, float value);
			
			// Displacement mapping
			/*! \remarks			Returns the amount of displacement along the normal of the surface at the
			point as specified by the <b>ShadeContext</b>.
			\param sc This contains the details of the point being displaced.
			\par Default Implementation:
			<b>{ return 0.0f; }</b> */
			virtual float EvalDisplacement(ShadeContext& sc) { return 0.0f; }
			/*! \remarks			Returns the validity interval of the displacement mapping around the
			specified time.
			\param t The Interval returned reflects the validity around this time.
			\par Default Implementation:
			<b>{ return FOREVER; }</b> */
			virtual Interval DisplacementValidity(TimeValue t) { return FOREVER; }

			// Schematic View Animatable Overrides...
			CoreExport bool SvCanInitiateLink(IGraphObjectManager *gom, IGraphNode *gNode);

			// allows plugins to control the display of the Discard/Keep Old Material dialog when being
			// created in a MtlEditor slot.  Added by JBW for scripted material plugins.
			// return TRUE to prevent the Keep Old Material dialog from pooping.
			/*! \remarks			Return TRUE to prevent the Replace Material (Discard old material? / Keep
			old material as sub-material?) dialog from being presented to the user;
			FALSE to allow it to be presented. This allows a plug-in to control the
			display of this dialog when being created in a Material Editor slot.
			\par Default Implementation:
			<b>{ return FALSE; }</b>\n\n
				*/
			virtual BOOL DontKeepOldMtl() { return FALSE; }

			// allow renderer to attach auxiliary data to each material.
			/*! \remarks			This method allows the renderer to attach auxiliary data to each
			material.
			\param rdata The auxiliary data you wish to attach.
			\par Default Implementation:
			<b>{ renderData = rdata; }</b> */
			virtual void SetRenderData( RenderData *rdata) { renderData = rdata; }
			/*! \remarks			This method returns the auxiliary data attached to the material by the
			renderer.
			\par Default Implementation:
			<b>{ return renderData; }</b> */
			virtual RenderData *GetRenderData() { return renderData; }

			// --- Material evaluation ---

			// Returns true if the evaluated color\value (output) is constant 
			// over all possible inputs. 
			/*! \remarks			This method returns TRUE if the evaluated color/value (output) is constant over
			all possible inputs described by the shade context. If it cannot determine the
			correct answer, it returns FALSE.
			\param sc This describes the context of the question.
			\param stdID The ID of the channel in question. 
			See \ref Material_TextureMap_IDs.
			\par Default Implementation:
			<b>{ return FALSE; }</b> */
			virtual bool IsOutputConst( ShadeContext& sc, int stdID ) { return false; }

			// Evaluate the material on a single standard texmap channel (ID_AM, etc) 
			// at a UVW coordinated and over an area described in the ShadingContext
			/*! \remarks			This method evaluates the material on a single standard texmap channel over an
			area described in the ShadeContext. A return value of FALSE indicates that the
			color could not be evaluated.\n\n
			If there's no texmap defined for a channel or the output of the texmap is not
			"meaningful", the raw color stored by the material or shader is returned. (The
			output of a texmap is meaningful in a given ShadeContext if it is the same as
			when the scene is rendered. If the map cannot determine whether the output
			value is the same as when rendered, it should not be meaningful.)\n\n
			Note that the output color is not clamped. If the method is called with a
			monochrome channel ID, the result value is copied in the R, G and B components
			of the Color structure.\n\n
			As a default implementation, this method sets the output color to black and
			returns FALSE.
			\param sc This describes the context in which the material should be evaluated.
			\param stdID The ID of the channel to perform evaluation on. 
			See \ref Material_TextureMap_IDs.
			\param outClr The result of the evaluation.
			\par Default Implementation:
			<b>{ return FALSE; }</b> */
			virtual bool EvalColorStdChannel( ShadeContext& sc, int stdID, Color& outClr) { return false; }
			CoreExport virtual bool EvalMonoStdChannel( ShadeContext& sc, int stdID, float& outVal) ;
	 };

//  A texture map implements this class and passes it into  EvalUVMap,
//  EvalUVMapMono, and EvalDeriv to evaluate itself with tiling & mirroring
/*! \sa  Class UVGen,
Class ShadeContext.
\par Description:
A texture map implements this class and passes it into the <b>UVGen</b> methods
<b>EvalUVMap()</b>, <b>EvalUVMapMono()</b>, and <b>EvalDeriv()</b> to evaluate
itself with tiling \& mirroring. Each of the methods of this class are used to
sample the map at a single UV coordinate.  */
class MapSampler: public MaxHeapOperators {
	 public:
			/*! \remarks Destructor. */
			virtual ~MapSampler() {;}
			// required:
			/*! \remarks This method is required. It is called to sample the map at a single UV
			coordinate. This method should not try to antialias the map, it should
			just return the value. Take for example a bitmap texture implementing
			this method. If the <b>u</b> values passed was 0.5 and the <b>v</b> value
			passed was 0.5, the plug-in would just return the pixel value at the
			center of the bitmap as an <b>AColor</b>.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			The ShadeContext.\n\n
			<b>float u</b>\n\n
			The U value in the range of 0.0 to 1.0\n\n
			<b>float v</b>\n\n
			The V value in the range of 0.0 to 1.0
			\return  The sampled value as an <b>AColor</b>. */
			virtual  AColor Sample(ShadeContext& sc, float u,float v)=0;
			/*! \remarks This method is required. It is called to return a value from the map that
			is the averaged value over the specified region of width <b>du</b> and
			height <b>dv</b>, centered at <b>u, v</b>. Certain map types may use the
			<b>du</b> and <b>dv</b> values directly to perform antialiasing of the
			map.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			The ShadeContext.\n\n
			<b>float u</b>\n\n
			The U value in the range of 0.0 to 1.0\n\n
			<b>float v</b>\n\n
			The V value in the range of 0.0 to 1.0\n\n
			<b>float du</b>\n\n
			This parameter can be considered the width of a small rectangle center on
			u,v. The u and v value are guaranteed to be inside this rectangle.\n\n
			<b>float dv</b>\n\n
			This parameter can be considered the height of a small rectangle center
			on u,v. The u and v value are guaranteed to be inside this rectangle.
			\return  The sampled value as an <b>AColor</b>. */
			virtual  AColor SampleFilter(ShadeContext& sc, float u,float v, float du, float dv)=0;
			// optional:
			virtual void SetTileNumbers(int iu, int iv) {} // called before SampleFunctions to tell what tile your in
			/*! \remarks This method is optional as a default implementation is provided. It may
			be implemented however if a certain map type can be more efficient in
			evaluating a mono channel. For example a noise function that produces a
			single floating point value as a function of UV. This method is called to
			sample the map at a single UV coordinate. This method should not try to
			antialias the map, it should just return the value.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			The ShadeContext.\n\n
			<b>float u</b>\n\n
			The U value in the range of 0.0 to 1.0\n\n
			<b>float v</b>\n\n
			The V value in the range of 0.0 to 1.0
			\return  The sampled value as an float.
			\par Default Implementation:
			<b>{ return Intens(Sample(sc,u,v)); }</b> */
			virtual  float SampleMono(ShadeContext& sc, float u,float v) { return Intens(Sample(sc,u,v)); }
			/*! \remarks This method is optional. It is called to return a value from the map that
			is the averaged value over the specified region of width <b>du</b> and
			height <b>dv</b>, centered at <b>u, v</b>. Certain map types may use the
			<b>du</b> and <b>dv</b> values directly to perform antialiasing of the
			map.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			The ShadeContext.\n\n
			<b>float u</b>\n\n
			The U value in the range of 0.0 to 1.0\n\n
			<b>float v</b>\n\n
			The V value in the range of 0.0 to 1.0\n\n
			<b>float du</b>\n\n
			This parameter can be considered the width of a small rectangle center on
			<b>u,v</b>. The <b>u</b> and <b>v</b> value are guaranteed to be inside
			this rectangle.\n\n
			<b>float dv</b>\n\n
			This parameter can be considered the height of a small rectangle center
			on <b>u,v</b>. The <b>u</b> and <b>v</b> value are guaranteed to be
			inside this rectangle.
			\return  The sampled value as an float.
			\par Default Implementation:
			<b>{ return Intens(SampleFilter(sc,u,v,du,dv)); }</b> */
			virtual  float SampleMonoFilter(ShadeContext& sc, float u,float v, float du, float dv){
				 return Intens(SampleFilter(sc,u,v,du,dv)); 
				 }
	 };


// This class generates UV coordinates based on the results of a UV 
// Source and user specified transformation.
// A reference to one of these is referenced by all 2D texture maps.
/*! \sa  Class MtlBase, 
Class ShadeContext, Class MapSampler, Class Point2,  Class Point3,  Class Matrix3.\n\n
\par Description:
Most texture maps that use UV coordinates will use an instance of this class.
This class encapsulates much of the user interface functionality for setting
mirroring, tiling and so on. These are the settings found in the 'Coordinate'
and 'Noise' rollup pages. This class generates UV coordinates based on the
results of a UV source provided by the plug-in and the user specified
transformations. An instance of this class is referenced by all the 2D texture
maps. All methods of this class are implemented by the system. <br>  The
following four methods are the only ones a plug-in needs to call typically: */
class UVGen: public MtlBase {
	 public:
			// Get texture coords and derivatives for antialiasing
			/*! \remarks This method is not normally called directly. It retrieves
			the UV coordinates (depending on if they are explicit, shrinked-wrapped
			environment, etc.) and transforms them using the offsets and noise
			parameters entered by the user. It does not do the mirroring or tiling.
			It then returns the texture coordinates and derivatives for antialiasing.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			Describes the properties of the point to evaluate.\n\n
			<b>Point2\& UV</b>\n\n
			The UV texture coordinates for the point.\n\n
			<b>Point2\& dUV</b>\n\n
			The derivatives of UV for the point to shade. This describes how big of a
			range in UV space the current pixel covers. */
			virtual void GetUV( ShadeContext& sc, Point2& UV, Point2& dUV)=0;
			
			// This is how a Texmap evaluates itself
			/*! \remarks This method is called to evaluate the color of the map
			based on the ShadeContext. It takes into account the users selections for
			mirroring, tiling, noise, etc. from the UVGen rollups.
			\par Parameters:
			<b>ShadeContext \&sc</b>\n\n
			Describes the properties of the point to evaluate.\n\n
			<b>MapSampler* samp</b>\n\n
			This is a callback used to sample the map. The plug-in creates an
			instance of this class, and implements its methods. A pointer to the
			instance is passed here. The methods of the class sample the texture for
			a single UV coordinate.\n\n
			<b>BOOL filter=TRUE</b>\n\n
			If TRUE the texture will be filtered.
			\return  The result as an AColor. */
			virtual AColor EvalUVMap(ShadeContext &sc, MapSampler* samp,  BOOL filter=TRUE)=0;
			/*! \remarks This method is called to evaluate the value of the map
			based on the ShadeContext. It takes into account the users selections for
			mirroring, tiling, noise, etc. from the UVGen rollups.
			\par Parameters:
			<b>ShadeContext \&sc</b>\n\n
			Describes the properties of the point to evaluate.\n\n
			<b>MapSampler* samp</b>\n\n
			This is a callback used to sample the map. The plug-in creates an
			instance of this class, and implements its methods. A pointer to the
			instance is passed here. The methods of the class sample the texture for
			a single UV coordinate.\n\n
			<b>BOOL filter=TRUE</b>\n\n
			If TRUE the texture will be filtered.
			\return  The result as a float.\n\n
			The following two methods are called for bump mapping to retrieve a
			normal perturbation. The calling sequence for these methods is shown in
			the sample code below. This code takes the sum of the U derivative times
			the U bump vector, and the V derivative times the V bump vector. The
			result is the normal perturbation returned from EvalNormalPerturb().\n
			\code
			Point3 BMTex::EvalNormalPerturb(ShadeContext& sc)
			{
				Point3 dPdu, dPdv;
				if (thebm==NULL)
					return Point3(0,0,0);
				uvGen->GetBumpDP(sc,dPdu,dPdv);				  // get bump basis vectors
				Point2 dM =(.01f)*uvGen->EvalDeriv(sc,&mysamp,filterType!=FILTER_NADA);
				return texout->Filter(dM.x*dPdu+dM.y*dPdv);
			}
			\endcode 
			Note: In the following code you'll see the 'text book' Blinn's algorithm
			and the 'Lazy' approach that is actually used. During testing it was
			determined that these two methods are visually identical. The 'Lazy'
			algorithm avoids the cross products however.
			\code
			Point3 Gradient::EvalNormalPerturb(ShadeContext\& sc) {
				Point3 dPdu, dPdv;
				Point2 dM = uvGen->EvalDeriv(sc,\&mysamp);
				uvGen->GetBumpDP(sc,dPdu,dPdv);
				// Lazy algorithm
				return texout->Filter(dM.x*dPdu+dM.y*dPdv);
			} 
			\endcode */
			virtual float EvalUVMapMono(ShadeContext &sc, MapSampler* samp, BOOL filter=TRUE)=0;
			/*! \remarks This gets the amount that U and V are changing. It takes
			into account the users selections for mirroring, tiling, noise, etc. from
			the UVGen rollups.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			Describes the properties of the point to evaluate.\n\n
			<b>MapSampler* samp</b>\n\n
			This is a callback used to sample the map. The plug-in creates an
			instance of this class, and implements its methods. A pointer to the
			instance is passed here. The methods of the class sample the texture for
			a single UV coordinate.\n\n
			<b>BOOL filter=TRUE</b>\n\n
			If TRUE the texture will be filtered.
			\return  <b>df/du</b>, <b>df/dv</b> as a Point2. */
			virtual  Point2 EvalDeriv( ShadeContext& sc, MapSampler* samp, BOOL filter=TRUE)=0;

			// Get dPdu and dPdv for bump mapping
			/*! \remarks This method may be called to compute <b>dPdu</b> and
			<b>dPdv</b> bump basis vectors for bump mapping. These are two vector in
			3D space that give you the U and V axis of the map space. This is like
			taking the map space into 3D space so you can determine where the U axis
			is and where the V axis is.\n\n
			Note: In 3ds Max 3.0 and later these vectors are always normalized. This
			change makes it so bump mapping is invariant when a Rescale World Units
			is performed.\n\n
			See the Advanced Topics section on
			<a href="ms-its:3dsmaxsdk.chm::/mtls_materials.html">Working with
			Materials and Textures</a> for more information on bump mapping.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			Describes the properties of the point to evaluate.\n\n
			<b>Point3\& dPdu, Point3\& dPdv</b>\n\n
			The bump basis vectors are returned here. */
			virtual  void GetBumpDP( ShadeContext& sc, Point3& dPdu, Point3& dPdv)=0;

			/*! \remarks This method is used internally to provide information to
			the interactive renderer. */
			virtual void GetUVTransform(Matrix3 &uvtrans)=0;

			/*! \remarks This method is used internally to provide information to
			the interactive renderer. */
			virtual int GetTextureTiling()=0;
			virtual void SetTextureTiling(int t)=0;

			/*! \remarks This method returns the source for the texture vertices.
			\return  One of the following values:\n\n
			<b>UVWSRC_EXPLICIT</b>\n\n
			Use explicit mesh texture vertices from channel 1.\n\n
			<b>UVWSRC_EXPLICIT2</b>\n\n
			Use explicit mesh texture vertices from channel 2.\n\n
			<b>UVWSRC_OBJXYZ</b>\n\n
			Generate planar UVW mapping coordinates from the object XYZ on-the-fly.
			*/
			virtual int GetUVWSource()=0;
			virtual int GetMapChannel () { return 1; }   // only relevant if above returns UVWSRC_EXPLICIT
			/*! \remarks This method sets the source for the texture vertices.
			\par Parameters:
			<b>int s</b>\n\n
			One of the following values:\n\n
			<b>UVWSRC_EXPLICIT</b>\n\n
			Use explicit mesh texture vertices from one of the mapping channels (as
			established by <b>SetMapChannel()</b> below).\n\n
			<b>UVWSRC_EXPLICIT2</b>\n\n
			Use explicit mesh texture vertices from the Vertex Color Channel.\n\n
			<b>UVWSRC_OBJXYZ</b>\n\n
			Generate planar UVW mapping coordinates from the object XYZ
			on-the-fly.\n\n
			<b>UVWSRC_WORLDXYZ</b>\n\n
			This parameter is available in release 3.0 and later only.\n\n
			Generate planar uvw mapping from world xyz on-the-fly. */
			virtual void SetUVWSource(int s)=0;
			/*! \remarks			Sets the mapping channel used when <b>UVWSRC_EXPLICIT</b> is in effect.
			\par Parameters:
			<b>int s</b>\n\n
			The mapping channel value to set.
			\par Default Implementation:
			<b>{}</b> */
			virtual void SetMapChannel (int s) { }

			/*! \remarks This method is used internally. */
			virtual int SymFlags()=0;
			/*! \remarks			This method is used internally. */
			virtual void SetSymFlags(int f)=0;

			virtual int GetSlotType()=0; 
			/*! \remarks Developers typically don't need to call this method. In the
			Coordinates rollup in the user interface for a texture map are two
			options. These options are Texture or Environment. The slot type is one
			of these two options. There are a variety of texture coordinate types.
			There are the type assigned to the object and the environment type
			(Spherical, Cylindrical, Shrink-wrap, Screen). This method is used
			initialize the radio button in the dialog based on the <b>sType</b>
			passed and update the drop down list.
			\par Parameters:
			<b>int sType</b>\n\n
			One of the following values:\n\n
			<b>MAPSLOT_TEXTURE</b>\n\n
			This is either texture coordinates.\n\n
			<b>MAPSLOT_ENVIRON</b>\n\n
			Environment coordinates. */
			virtual void InitSlotType(int sType)=0; 

			/*! \remarks			Return the axis (mapping coordinate system) used.
			\return  One of the following values:\n\n
			<b>AXIS_UV</b>\n\n
			<b>AXIS_VW</b>\n\n
			<b>AXIS_WU</b> */
			virtual int GetAxis()=0;  // returns AXIS_UV, AXIS_VW, AXIS_WU
			/*! \remarks			Sets the axis (mapping coordinate system) used.
			\par Parameters:
			<b>int ax</b>\n\n
			One of the following values:\n\n
			<b>AXIS_UV</b>\n\n
			<b>AXIS_VW</b>\n\n
			<b>AXIS_WU</b> */
			virtual void SetAxis(int ax)=0;  //  AXIS_UV, AXIS_VW, AXIS_WU

			// The clip flag controls whether the U,V values passed to MapSampler by 
			//  EvalUVMap and EvalUVMapMono are clipped to the [0..1] interval or not. 
			// It defaults to ON (i.e., clipped).
			/*! \remarks			This method sets the clip flag. The clip flag controls whether the U,V
			values passed to <b>MapSampler</b> by <b>EvalUVMap()</b> and
			<b>EvalUVMapMono()</b> are clipped to the [0..1] interval or not. It
			defaults to ON (i.e., clipped).\n\n
			If an object is covered by a texture, and that texture repeats (is
			tiled), this methods can be used to tell you which tile you're in. This
			allows one to do something depending on which tile it is. This is done,
			for example, in the Bitmap texture, when 'Jitter Placement' is on, so
			each tile can be moved about slightly. For an example of this see
			<b>/MAXSDK/SAMPLES/MATERIALS/BMTEX.CPP</b>. If you need this information,
			you can just call <b>uvGen-\>SetClipFlag(FALSE)</b> (for instance in your
			<b>Update()</b> method) and then use <b>int(u)</b> and <b>int(v)</b> to
			get the this info. (You'd use <b>frac(u)</b> and <b>frac(v)</b> to get
			clipped versions of u,v. )
			\par Parameters:
			<b>BOOL b</b>\n\n
			TRUE to clip; FALSE to not clip. */
			virtual void SetClipFlag(BOOL b)=0; 
			/*! \remarks			This method returns the clip flag. The clip flag controls whether the U,V
			values passed to <b>MapSampler</b> by <b>EvalUVMap()</b> and
			<b>EvalUVMapMono()</b> are clipped to the [0..1] interval or not. It
			defaults to ON (i.e., clipped). See <b>SetClipFlag()</b> above.
			\return  TRUE for clipped; FALSE if not clipped. */
			virtual BOOL GetClipFlag()=0;
			/*! \remarks			This method asks the question 'Is this class an instance of <b>StdUVGen</b>?'.
			The derived class <b>StdUVGen</b> implements this to return TRUE. Others use
			the default implementation to return FALSE.
			\par Default Implementation:
			<b>{ return FALSE; }</b> */
			virtual BOOL IsStdUVGen() { return FALSE; } // class StdUVGen implements this to return TRUE

			SClass_ID SuperClassID() { return UVGEN_CLASS_ID; }

			/*! \remarks			Sets the UVGen rollup state to open or closed.
			\par Parameters:
			<b>BOOL open</b>\n\n
			TRUE for open; FALSE for closed. */
			virtual void SetRollupOpen(BOOL open)=0;
			/*! \remarks			Returns the open or closed state of the UVGen rollup.
			\return  TRUE is open; FALSE is closed. */
			virtual BOOL GetRollupOpen()=0;
			CoreExport SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
			CoreExport MSTR SvGetName(IGraphObjectManager *gom, IGraphNode *gNode, bool isBeingEdited)
				 {
				 return Animatable::SvGetName(gom, gNode, isBeingEdited);
				 }
	 };

// This class generates Point3 coordinates based on the ShadeContext.
// A reference to one of these is referenced by all 3D texture maps.
/*! \sa  Class MtlBase, Class ShadeContext, Class Point3.\n\n
\par Description:
This class generates <b>Point3</b> coordinates based on the
<b>ShadeContext</b>. A reference to one of these is referenced by all 3D
texture maps. <b>XYZGen</b> does for 3D Texmaps what <b>UVGen</b> does for 2D
Texmaps. It puts up the 3D "Coordinates" rollup, and supplies the 3D Texmap
with transformed 3D coordinates. All methods of this class are implemented by
the system.  */
class XYZGen: public MtlBase {
	 public:
			// Get texture coords and derivatives for antialiasing
			/*! \remarks This method retrieves coordinates and derivatives for
			antialiasing. It returns the coordinates in the "Object Coordinate
			System", scaled and rotated by the parameters given in the Coordinates
			rollup. The default transformation is the identity transform, so you just
			get Object coordinates. For instance, if you have a sphere of radius 100,
			you will get coordinates ranging from -100 to +100.\n\n
			A developer might wonder why 3ds Max don't use some normalized
			coordinates space. This was considered, but it has some real problems.
			Say you make something by assembling several boxes, of varying sizes, and
			then apply a wood texture to the whole thing. You want the wood texture
			to be at the same scale for all the pieces, not smaller on the smaller
			pieces, and thus that doesn't work.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			Describes the properties of the point to be shaded.\n\n
			<b>Point3\& p</b>\n\n
			The 3D point is returned here.\n\n
			<b>Point3\& dp</b>\n\n
			The derivative of the 3D point is returned here. */
			virtual void GetXYZ( ShadeContext& sc, Point3& p, Point3& dp)=0;
			SClass_ID SuperClassID() { return XYZGEN_CLASS_ID; }
			/*! \remarks			Sets the XYZGen rollup state to open or closed.
			\par Parameters:
			<b>BOOL open</b>\n\n
			TRUE for open; FALSE for closed. */
			virtual void SetRollupOpen(BOOL open)=0;
			/*! \remarks			Returns the open or closed state of the XYZGen rollup.
			\return  TRUE is open; FALSE is closed. */
			virtual BOOL GetRollupOpen()=0;
			virtual BOOL IsStdXYZGen() { return FALSE; } // class StdXYZGen implements this to return TRUE
			/*! \remarks			This method returns the transformed bump unit vectors for 3D bump
			mapping. The 3D textures need to use these in computing the gradient.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			Describes the properties of the point to be shaded.\n\n
			<b>Point3* dP</b>\n\n
			The 3 unit vectors for computing differentials are returned here.
			\par Default Implementation:
			<b>{}</b>
			\par Sample Code:
			Here is a typical use of XYZGen-\>GetBumpDP() to compute the bump mapping
			for a 3D texture:\n\n
			\code
			Point3 Marble::EvalNormalPerturb(ShadeContext& sc) {
				float del,d;
				Point3 p,dp;
				if (!sc.doMaps) 
					return Point3(0,0,0);
				if (gbufID) 
					sc.SetGBufferID(gbufID);
				xyzGen->GetXYZ(sc,p,dp);
				if (size==0.0f) 
					size=.0001f;
				p *= FACT/size;
				d = MarbleFunc(p);
				del = 20.0f;
				Point3 np;
				Point3 M[3];
				xyzGen->GetBumpDP(sc,M);
				np.x = (MarbleFunc(p+del*M[0]) - d)/del;
				np.y = (MarbleFunc(p+del*M[1]) - d)/del;
				np.z = (MarbleFunc(p+del*M[2]) - d)/del;
				np *= 100.0f;
				return sc.VectorFrom(np,REF_OBJECT);
			}
			\endcode */
			virtual  void GetBumpDP( ShadeContext& sc, Point3* dP) {}; // returns 3 unit vectors for computing differentials
			CoreExport SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
			CoreExport MSTR SvGetName(IGraphObjectManager *gom, IGraphNode *gNode, bool isBeingEdited)
	    {
			 return Animatable::SvGetName(gom, gNode, isBeingEdited);
		  }
	 };

// This class is used by texture maps to put up the output filter 
// rollup, and perform the output filtering.
/*! \sa  Class MtlBase, Class AColor, Class Point3.\n\n
\par Description:
This class is used by texture maps to put up the 'Output' rollup in the
materials editor, and perform the output filtering. Currently this provides
control over the Output Amount, RGB Level, and RGB Offset. In the future this
may be enhanced to include other things that are often desirable on the output
stage. These are things like tinting, color shifting, etc. All methods of this
class are implemented by the system.\n\n
A plug-in will typically call these methods from the implementations of
<b>EvalColor()</b>, <b>EvalMono()</b>, etc. This is just done as a final
adjustment to scale or offset the value. The code below is from the Gradient
texture map.
\code
AColor Gradient::EvalColor(ShadeContext& sc) {
	return texout->Filter(uvGen->EvalUVMap(sc, &mysamp));
}
float Gradient::EvalMono(ShadeContext& sc) {
	return texout->Filter(uvGen->EvalUVMapMono(sc, &mysamp));
}
\endcode */
class TextureOutput: public MtlBase {
	 public:
			/*! \remarks Filters the specified AColor and returns it.
			\par Parameters:
			<b>AColor c</b>\n\n
			The color to filter.
			\return  The filtered color as an AColor. */
			virtual AColor Filter(AColor c) = 0;
			/*! \remarks Filters the specified float value and returns it.
			\par Parameters:
			<b>float f</b>\n\n
			The value to filter.
			\return  The filtered value. */
			virtual float Filter(float f) = 0;
			/*! \remarks Filters the specified Point3 value and returns it.
			\par Parameters:
			<b>Point3 p</b>\n\n
			The Point3 to filter.
			\return  The filtered Point3. */
			virtual Point3 Filter(Point3 p) = 0;
			/*! \remarks Returns the output level (amount) at the specified time.
			\par Parameters:
			<b>TimeValue t</b>\n\n
			The time at which to retrieve the output level. */
			virtual float GetOutputLevel(TimeValue t) = 0;
			/*! \remarks Sets the output level at the specified time.
			\par Parameters:
			<b>TimeValue t</b>\n\n
			The time to set the output level.\n\n
			<b>float v</b>\n\n
			The value of the output level. */
			virtual void SetOutputLevel(TimeValue t, float v) = 0;
			/*! \remarks Sets the state of the 'Invert' toggle in the Output rollup.
			\par Parameters:
			<b>BOOL onoff</b>\n\n
			TRUE to turn on; FALSE to turn off. */
			virtual void SetInvert(BOOL onoff)=0;
			/*! \remarks Returns the state of the 'Invert' toggle in the Output
			rollup.
			\return  TRUE is on; FALSE is off. */
			virtual BOOL GetInvert()=0;
			/*! \remarks			Sets the 'Output' rollup page to open or closed.
			\par Parameters:
			<b>BOOL open</b>\n\n
			TRUE to open the rollup; FALSE to close it. */
			virtual void SetRollupOpen(BOOL open)=0;
			/*! \remarks			Returns the open or closed state of the 'Output' rollup.
			\return  TRUE for open; FALSE for closed. */
			virtual BOOL GetRollupOpen()=0;
			SClass_ID SuperClassID() { return TEXOUTPUT_CLASS_ID; }
			CoreExport SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
			CoreExport MSTR SvGetName(IGraphObjectManager *gom, IGraphNode *gNode, bool isBeingEdited)
			{
				return Animatable::SvGetName(gom, gNode, isBeingEdited);
			}
	 };

//////////////////////////////////////////////////////////////
//
// post mapping params for shaders & render elements
class Shader;

#ifndef USE_LIMITED_STDMTL
#define STD2_NMAX_TEXMAPS  24
#else
#define STD2_NMAX_TEXMAPS  12
#endif

/*! \sa  Class Shader, Class Point3,  Class Color.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class allows the Standard material to get the parameters from a Shader
plug-in. It also allows the Shader to supply its computed color data back to
the Standard material. The Standard material will handle the texturing but it
needs to know the color before the texturing is done.\n\n
The shader object holds some raw parameters. But these parameters may be
textured. So for each pixel that will be shaded, the 3ds Max shade routine asks
the shader to fill in the raw values. Then it applies the texture over the top
of any of the values which are textured.
\par Data Members:
<b>Shader* pShader;</b>\n\n
This parameter is available in release 4.0 and later only.\n\n
A pointer to the shader.\n\n
<b>Mtl* pMtl;</b>\n\n
This parameter is available in release 4.0 and later only.\n\n
A pointer to the material being shaded or NULL if it's a background.\n\n
<b>Color channels[32];</b>\n\n
A color channel for each of the possible parameters that may be textured. Note
that these channels don't have specific meanings. They are defined by each
Shader. The Shader tells the Standard material what data is in these channels
using its <b>ChannelType()</b> and <b>StdIDToChannel()</b> methods.\n\n
<b>float falloffOpac;</b>\n\n
Shaders using standard opacity can ignore this data member. This is available
for certain Shaders which don't use the standard 3ds Max transparency equation.
This allows these Shaders to simulate the 3ds Max version. This is the
pre-computed textured opacity with standard falloff applied. The value here is
pre-computed to consider all the standard opacity settings of falloff
direction, etc. The standard transparency computation then uses this after
shading. So a shader could modify this value if it wanted to to affect the
transparency.\n\n
Note: The regular opacity can be obtained from the channel data.\n\n
<b>float kR;</b>\n\n
This value is used when there is a reflection and an atmosphere present.
Normally if there is no atmosphere (for instance no Fog in the scene) then the
transparency of the reflection is 100% (it is unaffected). However, if there is
an atmosphere present it will impart some level of opacity. This opacity (alpha
value) is not available via the reflection color. Thus this data member is here
to provide this opacity information to the Shader.\n\n
This value is the alpha which is returned by the reflection query that has the
transparency of the atmosphere contained within it, which is then multiplied by
the Amount spinner. This can then be used either by the Shader or by the
standard handling for reflection maps.\n\n
<b>ULONG hasComponents;</b>\n\n
These are the bits for the active components of bump, reflection, refraction
and opacity mapping. If the bit is set that component is active. This provides
a quick way for a Shader to check if they're used (as opposed to looking
through the <b>channels</b> array searching for these channel types).\n\n
<b>HAS_BUMPS</b> -- If bump mapping is present this is set.\n\n
<b>HAS_REFLECT</b> -- If there is any kind of reflection (raytraced, etc) then
this is set.\n\n
<b>HAS_REFRACT</b> -- If there is any kind of refraction then this is set..\n\n
<b>HAS_OPACITY</b> -- If opacity mapping is used this is set.\n\n
<b>HAS_REFLECT_MAP</b> -- If there is a reflection map only this is set. This
is used by the Strauss shader for example. If it sees a reflection map present
it dims the diffuse channel.\n\n
<b>HAS_REFRACT_MAP</b> -- If there is a refraction map only then this is
set.\n\n
<b>ULONG stdParams;</b>\n\n
The standard parameter bits. See
<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_shader_standard_parameter.html">List of Shader Standard
Parameter Flags</a>. This is filled in by the Standard material.\n\n
<b>Color ambIllumOut;</b>\n\n
This is the ambient output from the <b>Illum()</b> method.\n\n
<b>Color diffIllumOut;</b>\n\n
This is the diffuse output from the <b>Illum()</b> method.\n\n
<b>Color transIllumOut;</b>\n\n
This is the transparency output from the <b>Illum()</b> method.\n\n
<b>Color selfIllumOut;</b>\n\n
This is the self illumination output from the <b>Illum()</b> method.\n\n
<b>Color specIllumOut;</b>\n\n
This is the specular illumination output from the <b>Illum()</b> method.\n\n
<b>Color reflIllumOut;</b>\n\n
This is the reflection output from the <b>Illum()</b> method. Certain shaders
may wish to store the reflection output here (as opposed to providing it in the
<b>channels</b> array). This is the '"raw " color from the direction of
reflection (unattenuated). Some combiner implementations
(<b>Shader::CombineComponents()</b>)can get the reflection data here, others
may get it from the channels.\n\n
<b>float diffIllumIntens;</b>\n\n
Used only by reflection dimming, intensity of diffIllum prior to color
multiply.\n\n
<b>float finalAttenuation;</b>\n\n
The final attenuation for combining components.\n\n
<b>float finalOpac;</b>\n\n
This is the final opacity value used for combining components together in
<b>Shader::CombineComponents()</b>.\n\n
<b>Color finalC;</b>\n\n
This is the final output color that the <b>Shader::CombineComponents()</b>
composites together.\n\n
<b>Color finalT;</b>\n\n
This is the final Shader transparency color output.  */
class IllumParams : public BaseInterfaceServer {
public:
	 // these are the inputs to the shader, mostly textured channels
// ULONG mtlFlags;
	 Shader* pShader;  // for render elements to call the shader & mtl again, may be null
	 Mtl* pMtl;        // max mtl being shaded, null if bgnd
// Point3 N, V;
	 Color channels[ STD2_NMAX_TEXMAPS ];   //inputs, to be textured

	 float finalOpac;     // textured opacity w/ stdfalloff (reg opac in channels)
	 float reflectAmt;          // combined reflection.a * reflectAmt 
	 float refractAmt;          // combined reflection.a * amt 
	 ULONG hasComponents; // bits for active components(e.g.currently has active refl map)
	 ULONG stdParams;
	 int*  stdIDToChannel;   // combined shader & mtl. Note: may be NULL (for example, matte/shadow)

	 // these are the component-wise outputs from the shading process
	 Color ambIllumOut, diffIllumOut, transIllumOut, selfIllumOut; // the diffuse clrs
	 Color specIllumOut, reflIllumOut;   // the specular colors

	 // User Illumination outputs for render elements, name matched
	 int   nUserIllumOut;    // one set of names for all illum params instances
	 MCHAR** userIllumNames;  // we just keep ptr to shared name array, not destroyed
	 Color* userIllumOut; // the user illum color array, new'd & deleted w/ the class

	 float diffIllumIntens; // used only by reflection dimming, intensity of diffIllum prior to color multiply
	 float finalAttenuation; // for combining components

	 // these are the outputs of the combiner
	 Color finalC;  // final clr: combiner composites into this value.
	 Color finalT;  // shader transp clr out

public:
	 // Illum params are allocated by materials during shading process to
	 // communicate with shaders & render elements
	 // So materials need to know how many userIllum slots they will use
	 // most materials know this, but std2 will have to get it from the shader
	 CoreExport IllumParams( int nUserOut = 0, MCHAR** pUserNames = NULL );        

// IllumParams(){ 
//    nUserIllumOut = 0;
//    userIllumOut = NULL;
//    userIllumNames = NULL;
// }


	 CoreExport ~IllumParams();

	 // returns number of user illum channels for this material
	 int nUserIllumChannels(){ return nUserIllumOut; }

	 // returns null if no name array specified
	 MCHAR* GetUserIllumName( int n ) { 
			DbgAssert( n < nUserIllumOut );
			if( userIllumNames )
				 return userIllumNames[n];
			return NULL;
	 }

	 // render elements, mtls & shaders can use this to find the index associated with a name
	 // returns -1 if it can't find the name
	 CoreExport int FindUserIllumName( MCHAR* name );

	 // knowing the index, these set/get the user illum output color
	 void SetUserIllumOutput( int n, Color& out ){
			DbgAssert( n < nUserIllumOut );
			userIllumOut[n] = out;
	 }

	 CoreExport void SetUserIllumOutput( MCHAR* name, Color& out );

	 Color GetUserIllumOutput( int n ){
			DbgAssert( n < nUserIllumOut );
			return userIllumOut[n];
	 }

	 CoreExport Color GetUserIllumOutput( MCHAR* name, int n );

	 /*! \remarks This method is called by the Standard material prior to
	 calling the <b>Illum()</b> method of the Shader. It sets to black all the
	 output colors:\n\n
	 <b>ambIllumOut=diffIllumOut=transIllumOut=selfIllumOut=</b>\n\n
	 <b> specIllumOut=reflIllumOut=Color(0.0f, 0.0f, 0.0f);</b> */
	 void ClearOutputs() { 
			finalC = finalT = ambIllumOut=diffIllumOut=transIllumOut=selfIllumOut=
			specIllumOut=reflIllumOut= Color( 0.0f, 0.0f, 0.0f ); 
			finalAttenuation = diffIllumIntens = 0.0f;
			for( int i=0; i < nUserIllumOut; ++i )
				 userIllumOut[i] = finalC;
	 }

	 void ClearInputs() { 
			/*mtlFlags = */ stdParams = hasComponents = 0;
			pShader = NULL; pMtl = NULL;
			stdIDToChannel = NULL;
			refractAmt = reflectAmt = 0.0f; 
			for( int i=0; i < STD2_NMAX_TEXMAPS; ++i )
				 channels[ i ] = Color( 0, 0, 0 );
	 }
};

///////////////////////////////////////////////////////////////////
//
// Reshading interfaces
//
class Color32;

/*! \sa  Class InterfaceServer, Class IReshading, Class Point2,  Class Point3\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The Reshade Fragment interface is the materials/shaders/textures interface to
reshading fragments. This interface is only concerned with saving and
retrieving values in the fragments variable sized cache. Values are always
saved in multiples of 32 bit words, and they can be a variety of types, colors,
float's, int's, etc. Unit vectors are compressed to a single 32 bit word.
Point2 and Point3 data types save multiple floats.\n\n
Color channels will be compressed to a 32 bit rgba color and saved in the
cache.\n\n
Color32 channels are only used internally. This is the 'raw' form of what's
stored, other forms may be accessed from this class.\n\n
Float and integer channels are stored uncompressed, they are stored directly in
the 32 bits.\n\n
The Point2 \& Point3 channels are shells that store multiple float channels so
these values will be uncompressed. These should be used with great caution, as
this memory is stored per fragment, in a scene with 100,000 fragments of a
given material, a point3 channel will add 1.2 megabytes to the storage for the
reshading buffer.  */
class IReshadeFragment : public InterfaceServer 
{
public:
	 /*! \remarks This method returns the number of textures for the fragment.
	 */
	 virtual int NChannels()=0;
	 /*! \remarks This method returns the index of the first channel. */
	 virtual int NFirstChannel()=0; 
	 /*! \remarks This method returns the number of textures. */
	 virtual int NTextures()=0;

	 /*! \remarks This method returns the specified channel.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The channel you wish to return. */
	 virtual Color32   Channel( int nChan )=0;
	 /*! \remarks This method allows you to set a channel in raw mode to a new
	 value.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The channel you wish to set.\n\n
	 <b>Color32 c</b>\n\n
	 The new value to set. */
	 virtual void SetChannel( int nChan, Color32 c )=0;
	 /*! \remarks This method allows you to add a new color channel in raw mode
	 to the end of the cache.
	 \par Parameters:
	 <b>Color32 c</b>\n\n
	 The channel value you wish to set. */
	 virtual void AddChannel( Color32 c )=0;

	 /*! \remarks This method allows you to add a new color channel to the end
	 of the cache. The color will be compressed to 32bits.
	 \par Parameters:
	 <b>RGBA tex</b>\n\n
	 The color channel value you wish to add.\n\n
		 */
	 virtual void AddColorChannel( RGBA tex )=0;
	 /*! \remarks This method allows you to add a float channel in the fragment
	 cache.
	 \par Parameters:
	 <b>float f</b>\n\n
	 The channel value to add. */
	 virtual void AddFloatChannel( float f )=0;
	 /*! \remarks This method allows you to add an integer channel in the
	 fragment cache.
	 \par Parameters:
	 <b>int i</b>\n\n
	 The integer value to add. */
	 virtual void AddIntChannel( int i )=0;
	 /*! \remarks This method allows you to add a unit vector channel in the
	 fragment cache, compressed to 32-bits.
	 \par Parameters:
	 <b>Point3 v</b>\n\n
	 The unit vector to add. */
	 virtual void AddUnitVecChannel( Point3 v )=0;
	 /*! \remarks This method allows you to add a Point2 channel in the fragment
	 cache, uncompressed.
	 \par Parameters:
	 <b>Point2 p</b>\n\n
	 The Point2 to add. */
	 virtual void AddPoint2Channel( Point2 p )=0;
	 /*! \remarks This method allows you to add a Point3 channel in the fragment
	 cache, uncompressed.
	 \par Parameters:
	 <b>Point3 p</b>\n\n
	 The Point3 to add. */
	 virtual void AddPoint3Channel( Point3 p )=0;

	 /*! \remarks This method sets the existing color channel number
	 <b>nChan</b> to the new value.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The color channel number.\n\n
	 <b>RGBA text</b>\n\n
	 The new color value. */
	 virtual void SetColorChannel( int nChan, RGBA tex )=0;
	 /*! \remarks This method allows you to set a float channel in the fragment
	 cache to a new value.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The float channel number.\n\n
	 <b>float f</b>\n\n
	 The new value to set. */
	 virtual void SetFloatChannel( int nChan, float f )=0;
	 /*! \remarks This method allows you to set an int channel in the fragment
	 cache to a new value.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The int channel number.\n\n
	 <b>int i</b>\n\n
	 The new value to set. */
	 virtual void SetIntChannel( int nChan, int i )=0;
	 /*! \remarks This method allows you to set a unit vector channel in the
	 fragment cache to a new value.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The unit vector channel number.\n\n
	 <b>Point3 v</b>\n\n
	 The new value to set. */
	 virtual void SetUnitVecChannel( int nChan, Point3 v )=0;
	 /*! \remarks This method allows you to set a Point2 channel in the fragment
	 cache to a new value.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The Point2 channel number.\n\n
	 <b>Point2 p</b>\n\n
	 The new value to set. */
	 virtual void SetPoint2Channel( int nChan, Point2 p )=0;
	 /*! \remarks This method allows you to set a Point3 channel in the fragment
	 cache to a new value.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The Point3 channel number.\n\n
	 <b>Point3 p</b>\n\n
	 The new value to set. */
	 virtual void SetPoint3Channel( int nChan, Point3 p )=0;

	 /*! \remarks This method will return an existing color channel, expanded to
	 float RGBA.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The color channel you wish to return. */
	 virtual RGBA GetColorChannel( int nChan )=0;
	 /*! \remarks This method will return an existing float channel.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The float channel you wish to return. */
	 virtual float GetFloatChannel( int nChan )=0;
	 /*! \remarks This method will return an existing int channel.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The int channel you wish to return. */
	 virtual int GetIntChannel( int nChan )=0;
	 /*! \remarks This method will return an existing unit vector channel.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The unit vector channel you wish to return. */
	 virtual Point3 GetUnitVecChannel( int nChan )=0;
	 /*! \remarks This method will return an existing Point2 channel.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The Point2 channel you wish to return. */
	 virtual Point2 GetPoint2Channel( int nChan )=0;
	 /*! \remarks This method will return an existing Point3 channel.
	 \par Parameters:
	 <b>int nChan</b>\n\n
	 The Point3 channel you wish to return. */
	 virtual Point3 GetPoint3Channel( int nChan )=0;
};

// must be greater than I_USERINTERFACE in AnimatableInterfaceIDs.h
#define IID_IReshading  0xabe00001

// Materials support this interface to support preshading/reshading
/*! \sa  Class IReshadeFragment, Class ShadeContext, Class IllumParams\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This interface class is the interface that must be supported by materials and
shaders to support interactive rendering in 3ds Max. If this interface is not
returned when requested through <b>GetInterface()</b> on the mtl or shader,
then it is determined they do not support reshading. Any material that does not
support reshading is evaluated only at preshading time.\n\n
In 3ds Max, interactive rendering is implemented as fragment based caching
scheme. It's very much like a giant multi-layer g-buffer with enough
information stored with each fragment to re-run the material shading process
without re-rendering, all stored in a compressed form.\n\n
The rendering process in divided into 2 parts: preshading and reshading.
Preshading builds the scene and renders fragments to the compressed g-buffer,
called the <b>rsBuffer</b>. To do this, it saves a minimal amount of
information with each fragment, then gives each material and texture the
opportunity to pre-shade itself.\n\n
Basically, the reshader populates a minimal <b>ShadeContext</b>, with the
fragment center position in camera space (where shading is done), the shading
normal, the sub-pixel mask, the coverage, pointers to the original material and
<b>INode</b>, light lists, screen bounding box of object, the renderID, and
screen integer x and y.\n\n
Any other values from the shade context that a material, shader,
texture(future), atmosphere(future) or light(future) needs to save, it does so
at pre-shade time, and retrieves the data at reshading time. So, for example,
the multi-materials save which material is visible, anisotropic shaders save
the Anisotropic reference vector.  */
class IReshading: public MaxHeapOperators
{
public:
			// if supporting shaders, plug-in mtl should inherit from StdMat2
			enum ReshadeRequirements{ RR_None = 0, RR_NeedPreshade, RR_NeedReshade }; // mjm - 06.02.00
			/*! \remarks Destructor. */
			virtual ~IReshading() {;}
			// this is implied by the GetInterface( IID_IReshading ) call on the mtl
//    virtual BOOL SupportsReShading(ShadeContext& sc) { return FALSE; }

			// GetReshadeRequirements is called by the interactive reshader after being notified that the material has changed.
			// the return value indicates if the material is still valid, or needs to be preshaded or reshaded. the value
			// should pertain only to the latest change to the material
			// if a material doesn't support reshading, it doesn't need to override this function -- any change will cause
			// the nodes to which it's attached to be rerendered.
			/*! \remarks This method is called by the interactive reshader after
			being notified that the material has changed. The return value indicates
			if the material is still valid, or needs to be preshaded or reshaded. The
			value should pertain only to the latest change to the material. If a
			material doesn't support reshading, it doesn't need to override this
			function -- any change will cause the nodes to which it's attached to be
			re- preShaded.
			\return  One of the following values;\n\n
			<b>RR_None</b>\n\n
			No actions needs to be taken.\n\n
			<b>RR_NeedPreshade</b>\n\n
			A pre-shade is needed.\n\n
			<b>RR_NeedReshade</b>\n\n
			A re-shade is needed.
			\par Default Implementation:
			<b>{ return RR_NeedPreshade; }</b> */
			virtual ReshadeRequirements GetReshadeRequirements() { return RR_NeedPreshade; } // mjm - 06.02.00
			/*! \remarks This method will pre-shade the object and cache any needed
			values in the fragment.\n\n
			This method is called on the materials/shaders/textures as the reshading
			buffer is being built at the same point in the rendering process where
			the materials shade function would normally be called. Note that at this
			time the shade context is completely valid, as in a normal rendering. Any
			values the material wishes to cache can be attached to the reshading
			fragment passed in, and retrieved later at postShade time.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			A reference to the shade context.\n\n
			<b>IReshadeFragment* pFrag</b>\n\n
			A pointer to the fragment.
			\par Default Implementation:
			<b>{ }</b> */
			virtual void PreShade(ShadeContext& sc, IReshadeFragment* pFrag ) { }
			/*! \remarks This method will retrieve cached values and compute shade
			\& transparency for the fragment.\n\n
			This method is called for the material of each fragment as the reshading
			buffer is being traversed. Materials retrieve whatever they may have
			stored to complete the minimal shade context and compute a shade for the
			fragment. <b>PostShade()</b> calls to shaders contain an additional
			parameter, the <b>IllumParams</b>, filled with the textured/blended but
			unshaded values for each texture. The shade context passed into
			<b>PostShade()</b> is the minimal shade context outlined above.
			\par Parameters:
			<b>ShadeContext\& sc</b>\n\n
			A reference to the shade context.\n\n
			<b>IReshadeFragment* pFrag</b>\n\n
			A pointer to the fragment.\n\n
			<b>int\& nextTexIndex</b>\n\n
			A reference to the next texture index.\n\n
			<b>IllumParams* ip = NULL</b>\n\n
			A pointer to the IllumParams containing textured/blended but unshaded
			values for each texture.
			\par Default Implementation:
			<b>{ }</b> */
			virtual void PostShade(ShadeContext& sc, IReshadeFragment* pFrag, 
									int& nextTexIndex, IllumParams* ip = NULL ) { }

};

// must be greater than I_USERINTERFACE in AnimatableInterfaceIDs.h
#define IID_IValidityToken 0xabe00002

class IValidityToken: public MaxHeapOperators
{
public:
	 virtual ~IValidityToken() {;}
	 virtual BOOL NeedsUpdate( ULONG currentToken )= 0;
	 virtual void SetToken( ULONG token )= 0;
	 virtual ULONG GetToken() = 0;
	 virtual void SetMessage( ULONG message )= 0;
	 virtual ULONG GetMessage() = 0;
};


class IValidityTokenImp : public IValidityToken
{
protected:
	 ULONG mLocalToken;
	 ULONG mMessage;

public:
	 // if the object has been changed the local token will equal the
	 // current token
	 BOOL NeedsUpdate( ULONG currentToken ){
			return mLocalToken >= currentToken;
	 }
	 // mtls/maps set the token to next frame when changed
	 // from core interface GetNextValidityToken()
	 void SetToken( ULONG token ) {
			mLocalToken = token;
	 }
	 // not used, but here for debug & future use
	 ULONG GetToken() {
			return mLocalToken;
	 }

	 // once the render know which mtls maps were changed, this can be used to 
	 // send a secondary message e.g. map/colorVal or map/uv, etc.
	 void SetMessage( ULONG message ) {
			mMessage = message;
	 }
	 ULONG GetMessage() {
			return mMessage;
	 }
};

//------------------------------------

typedef Mtl* MtlHandle;
typedef Texmap* TexmapHandle;


// Simple list of materials
/*! \sa  Template Class Tab, Class Mtl.\n\n
\par Description:
A simple list of materials. All methods of this class are implemented by the
system.\n\n
Note the following typedefs:\n\n
<b>typedef MtlBase* MtlBaseHandle;</b>\n\n
<b>typedef Mtl* MtlHandle;</b>\n\n
<b>typedef Texmap* TexmapHandle;</b>  */
class MtlList: public Tab<MtlHandle> {
	 public:
			/*! \remarks Adds the specified material to this list of materials.
			\par Parameters:
			<b>Mtl *m</b>\n\n
			The material to add.\n\n
			<b>BOOL checkUnique=TRUE</b>\n\n
			If TRUE this method checks to make sure the material is unique, and will
			only add it if so.
			\return  Nonzero if the material was added; otherwise zero. */
			CoreExport int AddMtl(Mtl *m, BOOL checkUnique=TRUE);
			/*! \remarks Finds the specified material in this material list and
			returns its index (0 through the number of materials in the library).
			Returns -1 if not found.
			\par Parameters:
			<b>Mtl *m</b>\n\n
			The material to find. */
			CoreExport int FindMtl(Mtl *m);
			/*! \remarks Finds the material whose name is passed in this material
			list and returns its index (0 through the number of materials in the
			library). Returns -1 if not found.
			\par Parameters:
			<b>MSTR\& name</b>\n\n
			The material to find. */
			CoreExport int FindMtlByName(MSTR& name);
			/*! \remarks Removes the material whose index is passed.
			\par Parameters:
			<b>int n</b>\n\n
			The index of the material to remove. */
			void RemoveEntry(int n) { Delete(n,1); }
			/*! \remarks Empties (clears) the material list. */
			void Empty() { Resize(0); }
	 };

#pragma warning(push)
#pragma warning(disable:4239)

// Materials library
/*! \sa  Class ReferenceTarget, Class MtlList.\n\n
\par Description:
A materials library. All methods of this class are implemented by the system.
 */
class MtlLib: public ReferenceTarget, public MtlList {
	 public:
			CoreExport SClass_ID SuperClassID(); 
			CoreExport Class_ID ClassID();
			/*! \remarks Removes all Mtls from the library and deletes all
			references. */
			CoreExport void DeleteAll();
			CoreExport void GetClassName(MSTR& s);
			/*! \remarks Destructor. All references are deleted. */
			CoreExport ~MtlLib();

			int NumSubs() { 
				 return Count(); 
				 }  
			Animatable* SubAnim(int i) { 
				 return (*this)[i]; 
				 }
			CoreExport MSTR SubAnimName(int i);
			/*! \remarks Removes the specified material from the library.
			\par Parameters:
			<b>Mtl *m</b>\n\n
			The material to remove. */
			CoreExport virtual void Remove(Mtl *m);
			/*! \remarks Adds the specified material from the library.
			\par Parameters:
			<b>Mtl *m</b>\n\n
			The material to add. */
			CoreExport virtual void Add(Mtl *m);

			// From ref
			RefResult AutoDelete() { return REF_SUCCEED; }
			CoreExport void DeleteThis();
			int NumRefs() { return Count();}
			RefTargetHandle GetReference(int i) { return (RefTargetHandle)(*this)[i];}
protected:
			CoreExport virtual void SetReference(int i, RefTargetHandle rtarg);
public:
			CoreExport RefTargetHandle Clone(RemapDir &remap);
			CoreExport RefResult NotifyRefChanged( Interval changeInt, RefTargetHandle hTarget, 
				 PartID& partID, RefMessage message );
			// IO
			CoreExport IOResult Save(ISave *isave);
				CoreExport IOResult Load(ILoad *iload);

	 };


/*! \sa  Class ReferenceTarget, Class MtlBaseList, Class MtlBase, Class Interface.\n\n
\par Description:
This class is a library of <b>MtlBase</b> entries. To get a list of the
currently loaded materials use:\n\n
<b> MtlBaseLib\& Interface::GetMaterialLibrary()</b>.\n\n
When you load a material library, you can enumerate all the materials that it
contains. <b>MtlBaseLib</b> is subclassed off of <b>MtlBaseList</b> which is a
<b>Tab\<MtlHandle\></b>, so for example, you can do the following to work with
each material:\n\n
 <b>for (int i=0; i\<mlib.Count(); i++) { DoSomething(mlib[i]); }</b>\n\n
For additional information on this class see the Advanced Topics section
<a href="ms-its:3dsmaxsdk.chm::/mtls_materials.html">Working with Materials and
Textures</a>. All methods of this class are implemented by the system.  */
class MtlBaseLib : public ReferenceTarget, public MtlBaseList
{
public:
	CoreExport MtlBaseLib();
	/*! \remarks Destructor. All references are deleted. */
	CoreExport virtual ~MtlBaseLib();

	// from Animitable
	CoreExport virtual SClass_ID SuperClassID();
	CoreExport virtual Class_ID ClassID();
	CoreExport virtual void GetClassName(MSTR& s);
	CoreExport virtual int NumSubs();
	CoreExport virtual Animatable* SubAnim(int i);
	CoreExport virtual MSTR SubAnimName(int i);

	// From ReferenceTarget
	CoreExport virtual RefResult AutoDelete();
	CoreExport virtual void DeleteThis();
	CoreExport virtual int NumRefs();
	CoreExport virtual RefTargetHandle GetReference(int i);
protected:
	CoreExport virtual void SetReference(int i, RefTargetHandle rtarg);
public:

	CoreExport virtual RefTargetHandle Clone(RemapDir &remap);
	CoreExport virtual RefResult NotifyRefChanged(
		Interval changeInt, 
		RefTargetHandle hTarget, 
		PartID& partID, 
		RefMessage message );

	// IO
	CoreExport virtual IOResult Save(ISave *isave);
	CoreExport virtual IOResult Load(ILoad *iload);

	/*! \remarks Removes all MtlBases from the library and deletes all
	references. */
	CoreExport virtual void DeleteAll();
	/*! \remarks Removes the specified MtlBase from the library.
	\par Parameters:
	<b>MtlBase *m</b>\n\n
	The MtlBase to remove. */
	CoreExport virtual void Remove(MtlBase *m);
	/*! \remarks Adds the specified MtlBase to the library.
	\par Parameters:
	<b>MtlBase *m</b>\n\n
	The MtlBase to add. */
	CoreExport virtual void Add(MtlBase *m);
	/*! \remarks Removes all duplicate MtlBases from the library. */
	CoreExport virtual void RemoveDuplicates();

protected:
	//! \brief Returns the count of materials under this category.
	int GetMaterialCount() const;
	//! \brief Get a pointer to the material with the given index.
	/*! 
	\pre i must be greater than or equal to 0 and less than the count of materials.
	\param[in] i - The index of the material.
	\return  The pointer to the material. */
	MtlBase* GetMaterial(int i) const;
	//! \brief Find the child material with the given name.
	/*! 
	\pre name must not be NULL and must points to a string which is NULL terminated.
	\param[in] name - The name of the child material to find. 
	\return The pointer to the found child material. */
	MtlBase* FindMaterial(const MCHAR* name) const;
	//! \brief Add a material to the children list of this category.
	/*! 
	\param[in] pMaterial - The material to add to this category. Can be NULL.
	\return  true if the material is succeesfully added to this category, false 
	otherwise. */
	bool AddMaterial(MtlBase* pMaterial);
	//! \brief Removes the specified MtlBase from this category.
	/*! 
	\param[in] pMaterial - The MtlBase to remove. Can be NULL. */
	void RemoveMaterial(MtlBase* pMaterial);

	void SetMaterial(int i, MtlBase* pMaterial);
	void SetMaterialCount(int count);

	void* mpReserved;
};

#pragma warning(pop)
// Simple list of numbers
/*! \sa  Template Class Tab.\n\n
\par Description:
This class maintains a simple list of integer numbers. All methods of this
class are implemented by the system.  */
class NumList: public Tab<int> {
	 public:
			/*! \remarks Adds the number to the list.
			\par Parameters:
			<b>int j</b>\n\n
			The number to add.\n\n
			<b>BOOL checkUnique=TRUE</b>\n\n
			If TRUE the number will be added only if it is not in the table already.
			\return  The index in the table of the added number, or the index where
			it was found already if <b>checkUnique</b> is used. */
			CoreExport int Add(int j, BOOL checkUnique=TRUE);
			/*! \remarks Returns the index in the table of the number passed.
			\par Parameters:
			<b>int j</b>\n\n
			The number to find. */
			CoreExport int Find(int j);
	 };

class MtlRemap: public MaxHeapOperators {
	 public:
			virtual ~MtlRemap() {;}
			virtual Mtl* Map(Mtl *oldAddr)=0;
	 };

//--------------------------------------------------------------------------------
// Use this class to implement Drag-and-Drop for materials sub-Texmaps.
// If this class is used the ParamDlg method FindSubTexFromHWND() must be implemented.
//--------------------------------------------------------------------------------
/*! \sa  Class DADMgr, Class ParamDlg.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
Use this class to provide drag and drop functionality for materials
sub-Texmaps. It provides implementations of the methods of <b>DADMgr</b>. If
this class is used the <b>ParamDlg</b> method <b>FindSubTexFromHWND()</b> must
be implemented. */
class TexDADMgr: public DADMgr {
	 ParamDlg *dlg;
	 public:
			TexDADMgr(ParamDlg *d=NULL) { dlg = d;}
			void Init(ParamDlg *d) { dlg = d; }
			// called on the draggee to see what if anything can be dragged from this x,y
			SClass_ID GetDragType(HWND hwnd, POINT p) { return TEXMAP_CLASS_ID; }
			// called on potential dropee to see if can drop type at this x,y
			CoreExport BOOL OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew);
			int SlotOwner() { return OWNER_MTL_TEX;   }
			CoreExport ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type);
			CoreExport void Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type, DADMgr* srcMgr = NULL, BOOL bSrcClone = FALSE);
			CoreExport BOOL  LetMeHandleLocalDAD(); 
			CoreExport void  LocalDragAndDrop(HWND h1, HWND h2, POINT p1, POINT p2);
			BOOL  AutoTooltip(){ return TRUE; }
	 };


//--------------------------------------------------------------------------------
// Use this class to implement Drag-and-Drop for materials sub-materials.
// If this class is used the ParamDlg method FindSubMtlFromHWND() must be implemented.
//--------------------------------------------------------------------------------
/*! \sa  Class DADMgr.\n\n
\par Description:
All methods of this class are implemented by the system.\n\n
Use this class to implement drag and drop functionality for materials
sub-materials. . It provides implementations of the methods of <b>DADMgr</b>.
If this class is used the <b>ParamDlg</b> method <b>FindSubMtlFromHWND()</b>
must be implemented. */
class MtlDADMgr: public DADMgr {
	 ParamDlg *dlg;
	 public:
			MtlDADMgr(ParamDlg *d=NULL) { dlg = d;}
			void Init(ParamDlg *d) { dlg = d; }
			// called on the draggee to see what if anything can be dragged from this x,y
			SClass_ID GetDragType(HWND hwnd, POINT p) { return MATERIAL_CLASS_ID; }
			// called on potential dropee to see if can drop type at this x,y
			CoreExport BOOL OkToDrop(ReferenceTarget *dropThis, HWND hfrom, HWND hto, POINT p, SClass_ID type, BOOL isNew);
			int SlotOwner() { return OWNER_MTL_TEX;   }
			CoreExport ReferenceTarget *GetInstance(HWND hwnd, POINT p, SClass_ID type);
			CoreExport void Drop(ReferenceTarget *dropThis, HWND hwnd, POINT p, SClass_ID type, DADMgr* srcMgr = NULL, BOOL bSrcClone = FALSE);
			CoreExport BOOL  LetMeHandleLocalDAD(); 
			CoreExport void  LocalDragAndDrop(HWND h1, HWND h2, POINT p1, POINT p2);
			BOOL  AutoTooltip(){ return TRUE; }
	 };


#pragma warning(push)
#pragma warning(disable:4239)

// this class used for Drag/and/dropping Bitmaps
/*! \sa  Class ReferenceTarget.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
It is used to provide drag and drop functionality for Bitmaps. See the
following global function for getting a pointer to the instances of this
class.\n\n
All methods of this class are implemented by the system. */
class DADBitmapCarrier: public ReferenceTarget {
	 MSTR name;
	 public:
	 /*! \remarks Sets the name of the bitmap carrier.
	 \par Parameters:
	 <b>MSTR \&nm</b>\n\n
	 The name to set. */
	 void SetName(const MSTR &nm) { name = nm; }
	 /*! \remarks	 Returns the name of the bitmap carrier. */
	 MSTR& GetName() { return name; }    

	 Class_ID ClassID() { return Class_ID(0,1); }
	 SClass_ID SuperClassID() { return BITMAPDAD_CLASS_ID; }     
	 void DeleteThis() { }
	 RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
						PartID& partID,  RefMessage message) { return REF_DONTCARE; }     

	 RefTargetHandle Clone(RemapDir &remap) { assert(0); return NULL; }

	 };
#pragma warning(pop)

// Get a pointer to the BitmapCarrier (their's only two of these: one for the
// source, and one for the destination:  don't try to delete these; 
/*! \remarks This global function returns a pointer to the
<b>BitmapCarrier</b>. Note that there is only two of these: one for the source,
and one for the destination. Developers should not delete these. Sample code
using this class is available in <b>/MAXSDK/SAMPLES/MATERIALS/BMTEX.CPP</b>.
 */
CoreExport DADBitmapCarrier *GetDADBitmapCarrier();


CoreExport void SetLoadingMtlLib(MtlLib *ml);
CoreExport void SetLoadingMtlBaseLib(MtlBaseLib *ml);

CoreExport ClassDesc* GetMtlLibCD();
CoreExport ClassDesc* GetMtlBaseLibCD();

CoreExport UVGen* GetNewDefaultUVGen();
CoreExport XYZGen* GetNewDefaultXYZGen();
CoreExport TextureOutput* GetNewDefaultTextureOutput();
inline int IsMtl(Animatable *m) { return m->SuperClassID()==MATERIAL_CLASS_ID; }
inline int IsTex(Animatable *m) { return m->SuperClassID()==TEXMAP_CLASS_ID; }
inline int IsMtlBase(Animatable *m) { return IsMtl(m)||IsTex(m); }

// Combines the two materials into a multi-material.
// Either of the two input materials can themselves be multis.
// c1 and c2 will be set to the mat count for mat1 and mat2.
CoreExport Mtl* CombineMaterials(Mtl *mat1, Mtl *mat2, int &mat2Offset);

// Expand multi material size to the largest face ID
CoreExport Mtl* FitMaterialToMeshIDs(Mesh &mesh,Mtl *mat);
CoreExport Mtl* FitMaterialToShapeIDs(BezierShape &shape, Mtl *mat);
CoreExport Mtl* FitMaterialToPatchIDs(PatchMesh &patch, Mtl *mat);
CoreExport Mtl* FitMaterialToPolyMeshIDs (MNMesh & mesh, Mtl *mat);

// Adjust face IDs so that no face ID is greater then them multi size
CoreExport void FitMeshIDsToMaterial(Mesh &mesh, Mtl *mat);
CoreExport void FitShapeIDsToMaterial(BezierShape &shape, Mtl *mat);
CoreExport void FitPatchIDsToMaterial(PatchMesh &patch, Mtl *mat);
CoreExport void FitPolyMeshIDsToMaterial (MNMesh & mesh, Mtl *mat);

// Remove unused mats in a multi and shift face IDs.
CoreExport Mtl *CondenseMatAssignments(Mesh &mesh, Mtl *mat);
CoreExport Mtl *CondenseMatAssignments(BezierShape &shape, Mtl *mat);
CoreExport Mtl *CondenseMatAssignments(PatchMesh &patch, Mtl *mat);
CoreExport Mtl *CondenseMatAssignments (MNMesh &mesh, Mtl *mat);


// Attach materials dialog support -- TH 3/9/99

// Attach Mat values
#define ATTACHMAT_IDTOMAT 0
#define ATTACHMAT_MATTOID 1
#define ATTACHMAT_NEITHER 2

/*! \sa  Class Object, Class Modifier.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class provides a general way for objects to handle the attach materials
dialog presented when users attach objects to each other. To use this class do
the following:\n\n
1) Subclass your modifier or editable object off this class as shown below.
Implement the four functions that give access to the attach/condense
options.\n\n
2) Now your class simply calls the global <b>DoAttachMatOptionDialog()</b>
function, which deals with the parameters uniformly for all users. The
implementation in Edit Spline is like so:\n\n
\code
class EditSplineMod : public Modifier, ..., AttachMatDlgUser
{
	. . .
		static int attachMat;
	static BOOL condenseMat;
	. . .
}
// from AttachMatDlgUser
	int GetAttachMat() { return attachMat; }
	void SetAttachMat(int value) { attachMat = value; }
	BOOL GetCondenseMat() { return condenseMat; }
	void SetCondenseMat(BOOL sw) { condenseMat = sw; }
\endcode  
And the statics are defined as:\n\n
\code
int EditSplineMod::condenseMat = FALSE;
int EditSplineMod::attachMat = ATTACHMAT_NEITHER;
\endcode  */
class AttachMatDlgUser: public MaxHeapOperators {
	 public:
			/*! \remarks Destructor. */
			virtual ~AttachMatDlgUser() {;}
			/*! \remarks Returns the attach material setting. One of the following
			values:\n\n
			<b>ATTACHMAT_IDTOMAT</b>\n\n
			Match material IDs to material.\n\n
			<b>ATTACHMAT_MATTOID</b>\n\n
			Match Material to Material IDs.\n\n
			<b>ATTACHMAT_NEITHER</b>\n\n
			Do not modify Material IDs or Material. */
			virtual int GetAttachMat()=0;
			/*! \remarks Returns the condense material and IDs settings. */
			virtual BOOL GetCondenseMat()=0;
			/*! \remarks Sets the attach material value.
			\par Parameters:
			<b>int value</b>\n\n
			One of the following values:\n\n
			<b>ATTACHMAT_IDTOMAT</b>\n\n
			Match material IDs to material.\n\n
			<b>ATTACHMAT_MATTOID</b>\n\n
			Match Material to Material IDs.\n\n
			<b>ATTACHMAT_NEITHER</b>\n\n
			Do not modify Material IDs or Material. */
			virtual void SetAttachMat(int value)=0;
			/*! \remarks Sets the condense material and IDs setting.
			\par Parameters:
			<b>BOOL sw</b>\n\n
			TRUE for on; FALSE for off. */
			virtual void SetCondenseMat(BOOL sw)=0;
	 };

#pragma warning(pop) // C4100

/*! \remarks This global function is available in release 3.0 and later
only.\n\n
This displays the dialog that you currently get in Edit(able) mesh when you
attach objects to each other. It gives options for how to deal with combining
materials, and whether or not to condense materials which have excess material
slots.
\par Parameters:
<b>IObjParam *ip</b>\n\n
The interface pointer.\n\n
<b>AttachMatDlgUser *user</b>\n\n
Points to the AttachMatDlgUser object.
\return  TRUE if the user OKed the dialog; FALSE if the user Cancelled.  */
CoreExport BOOL DoAttachMatOptionDialog(IObjParam *ip, AttachMatDlgUser *user);

#define IID_INewSubMapNotification Interface_ID(0x3fe013b2, 0x31815874)

// Optional interface which can be implemented by classes derived from MtlBase.
// If this interface is returned from GetInterface, then this method is called
// when a new sub-texmap is added to the material.  It lets the material set
// material-specific defaults on the texture map.  This is only called when
// adding newly created textures to the material.
class INewSubTexmapNotification: public BaseInterface
{
public:
		virtual void NewSubTexmapAdded(int i, Texmap *m) = 0;
	 Interface_ID   GetID() { return IID_INewSubMapNotification; }
};

#define DEFAULT_TEXTURE_MAP_SIZE_INCHES 48.0f
#define DEFAULT_TEXTURE_MAP_SIZE_METERS 1.0f


