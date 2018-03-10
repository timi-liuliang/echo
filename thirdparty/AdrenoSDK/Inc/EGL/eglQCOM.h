/*=============================================================================
               Copyright © 2009 QUALCOMM Incorporated.
                        All Rights Reserved.
                     QUALCOMM Proprietary/GTDR
=============================================================================*/
#ifndef __eglQCOM_h__
#define __eglQCOM_h__

// sraut@qti.qualcomm.com: Keeping this file for legacy reasons
// refer Jira ACGSDK-745 for more information: https://jira.qualcomm.com/jira/browse/ACGSDK-745

#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifndef EGL_QUALCOMM_get_color_buffer
#define EGL_QUALCOMM_get_color_buffer
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI void* EGLAPIENTRY eglGetColorBufferQUALCOMM ( void );
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef void* (EGLAPIENTRYP PFNEGLGETCOLORBUFFERQUALCOMMPROC) ( void );
#endif /* EGL_QUALCOMM_get_color_buffer */

#ifndef EGL_QUALCOMM_get_power_level
#define EGL_QUALCOMM_get_power_level
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI int EGLAPIENTRY eglGetPowerLevelQUALCOMM ( void );
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef int (EGLAPIENTRYP PFNEGLGETPOWERLEVELQUALCOMMPROC) ( void );
#endif /* EGL_QUALCOMM_get_power_level */

#ifndef EGL_QUALCOMM_surface_scale
#define EGL_QUALCOMM_surface_scale
typedef EGLint EGLfixed;        // s15.16 fixed point format
typedef struct
{
  EGLint        x;
  EGLint        y;
  EGLint        width;
  EGLint        height;
} EGLSurfaceScaleRect;
typedef struct
{
  EGLfixed      MinXScaleFactor;
  EGLfixed      MaxXScaleFactor;
  EGLfixed      MinYScaleFactor;
  EGLfixed      MaxYScaleFactor;
  EGLint        MinSrcWidth;
  EGLint        MaxSrcWidth;
  EGLint        MinSrcHeight;
  EGLint        MaxSrcHeight;
  EGLint        MinDstWidth;
  EGLint        MaxDstWidth;
  EGLint        MinDstHeight;
  EGLint        MaxDstHeight;
} EGLSurfaceScaleCaps;
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceScaleEnableQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean enable );
EGLAPI EGLBoolean EGLAPIENTRY eglSetSurfaceScaleQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLSurfaceScaleRect *src, EGLSurfaceScaleRect *dst );
EGLAPI EGLBoolean EGLAPIENTRY eglGetSurfaceScaleQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *enabled, EGLSurfaceScaleRect *src, EGLSurfaceScaleRect *dst );
EGLAPI EGLBoolean EGLAPIENTRY eglGetSurfaceScaleCapsQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLSurfaceScaleCaps *param );
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSURFACESCALEENABLEQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean enable );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSETSURFACESCALEQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLSurfaceScaleRect *src, EGLSurfaceScaleRect *dst );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSURFACESCALEQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *enabled, EGLSurfaceScaleRect *src, EGLSurfaceScaleRect *dst );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSURFACESCALECAPSQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLSurfaceScaleCaps *param );
#endif /* EGL_QUALCOMM_surface_scale */

#ifndef EGL_QUALCOMM_surface_rotate
#define EGL_QUALCOMM_surface_rotate
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceRotateEnableQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean enable );
EGLAPI EGLBoolean EGLAPIENTRY eglSetSurfaceRotateQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean rot90, EGLBoolean hmirror, EGLBoolean vmirror );
EGLAPI EGLBoolean EGLAPIENTRY eglGetSurfaceRotateQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *enabled, EGLBoolean *rot90, EGLBoolean *hmirror, EGLBoolean *vmirror );
EGLAPI EGLBoolean EGLAPIENTRY eglGetSurfaceRotateCapsQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *rot90, EGLBoolean *hmirror, EGLBoolean *vmirror );
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSURFACEROTATEENABLEQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean enable );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSETSURFACEROTATEQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean rot90, EGLBoolean hmirror, EGLBoolean vmirror );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSURFACEROTATEQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *enabled, EGLBoolean *rot90, EGLBoolean *hmirror, EGLBoolean *vmirror );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSURFACEROTATECAPSQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *rot90, EGLBoolean *hmirror, EGLBoolean *vmirror );
#endif /* EGL_QUALCOMM_surface_rotate */

#ifndef EGL_QUALCOMM_surface_transparency
#define EGL_QUALCOMM_surface_transparency
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceTransparencyEnableQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean enable );
EGLAPI EGLBoolean EGLAPIENTRY eglSetSurfaceTransparencyQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLint alpha );
EGLAPI EGLBoolean EGLAPIENTRY eglGetSurfaceTransparencyQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *enabled, EGLint *alpha );
EGLAPI EGLBoolean EGLAPIENTRY eglSetSurfaceTransparencyMapQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLSurface alphasurf );
EGLAPI EGLBoolean EGLAPIENTRY eglGetSurfaceTransparencyMapQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *enabled, EGLSurface *alphasurf );
EGLAPI EGLBoolean EGLAPIENTRY eglGetSurfaceTransparencyCapsQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *alphacomb, EGLBoolean *constalpha, EGLBoolean *alphamap );
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSURFACETRANSPARENCYENABLEQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean enable );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSETSURFACETRANSPARENCYQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLint alpha );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSURFACETRANSPARENCYQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *enabled, EGLint *alpha );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSETSURFACETRANSPARENCYMAPQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLSurface alphasurf );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSURFACETRANSPARENCYMAPQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *enabled, EGLSurface *alphasurf );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSURFACETRANSPARENCYCAPSQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *alphacomb, EGLBoolean *constalpha, EGLBoolean *alphamap );
#endif /* EGL_QUALCOMM_surface_transparency */

#ifndef EGL_QUALCOMM_surface_color_key
#define EGL_QUALCOMM_surface_color_key
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceColorKeyEnableQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean enable );
EGLAPI EGLBoolean EGLAPIENTRY eglSetSurfaceColorKeyQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLint red, EGLint green, EGLint blue );
EGLAPI EGLBoolean EGLAPIENTRY eglGetSurfaceColorKeyQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *enabled, EGLint *red, EGLint *green, EGLint *blue );
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSURFACECOLORKEYENABLEQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean enable );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSETSURFACECOLORKEYQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLint red, EGLint green, EGLint blue );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSURFACECOLORKEYQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *enabled, EGLint *red, EGLint *green, EGLint *blue );
#endif /* EGL_QUALCOMM_surface_color_key */

#ifndef EGL_QUALCOMM_surface_overlay
#define EGL_QUALCOMM_surface_overlay
typedef struct
{
  EGLint        max_overlay;
  EGLint        max_underlay;
  EGLint        max_surface_per_layer;
  EGLint        max_total_surfaces;
  EGLBoolean    pbuffer_support;
  EGLBoolean    pixmap_support;
} EGLCompositeSurfaceCaps;
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLSurface EGLAPIENTRY eglCreateCompositeSurfaceQUALCOMM ( EGLDisplay dpy, EGLSurface win, const EGLint *attrib_list );
EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceOverlayEnableQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean enable );
EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceOverlayLayerEnableQUALCOMM ( EGLDisplay dpy, EGLSurface comp_surf, EGLint layer, EGLBoolean enable );
EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceOverlayBindQUALCOMM ( EGLDisplay dpy, EGLSurface comp_surf, EGLSurface surf, EGLint layer, EGLBoolean enable );
EGLAPI EGLBoolean EGLAPIENTRY eglGetSurfaceOverlayBindingQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLSurface *comp_surf, EGLint *layer );
EGLAPI EGLBoolean EGLAPIENTRY eglGetSurfaceOverlayQUALCOMM ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *layer_enable, EGLBoolean *surf_enable );
EGLAPI EGLBoolean EGLAPIENTRY eglGetSurfaceOverlayCapsQUALCOMM ( EGLDisplay dpy, EGLSurface win, EGLCompositeSurfaceCaps *param );
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLSurface (EGLAPIENTRYP PFNEGLCREATECOMPOSITESURFACEQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface win, const EGLint *attrib_list );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSURFACEOVERLAYENABLEQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean enable );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSURFACEOVERLAYLAYERENABLEQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface comp_surf, EGLint layer, EGLBoolean enable );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSURFACEOVERLAYBINDQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface comp_surf, EGLSurface surf, EGLint layer, EGLBoolean enable );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSURFACEOVERLAYBINDINGQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLSurface *comp_surf, EGLint *layer );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSURFACEOVERLAYQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface surf, EGLBoolean *layer_enable, EGLBoolean *surf_enable );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLGETSURFACEOVERLAYCAPSQUALCOMMPROC) ( EGLDisplay dpy, EGLSurface win, EGLCompositeSurfaceCaps *param );
#endif /* EGL_QUALCOMM_surface_overlay */

#ifndef EGL_QUALCOMM_shared_image
#define EGL_QUALCOMM_shared_image
#define EGL_IMAGE_SHARED_OPENGLES_QUALCOMM  0x0001 /* EGL_QUALCOMM_shared_image share_list */
#define EGL_IMAGE_SHARED_OPENGL_QUALCOMM    0x0002 /* EGL_QUALCOMM_shared_image share_list */
#define EGL_IMAGE_SHARED_OPENVG_QUALCOMM    0x0003 /* EGL_QUALCOMM_shared_image share_list */
#define EGL_IMAGE_SHARED_VIDEO_QUALCOMM     0x0004 /* EGL_QUALCOMM_shared_image share_list */
#define EGL_IMAGE_SHARED_CAMERA_QUALCOMM    0x0005 /* EGL_QUALCOMM_shared_image share_list */
#define EGL_IMAGE_SHARED_CPU_QUALCOMM       0x0006 /* EGL_QUALCOMM_shared_image share_list */
#define EGL_COMPONENT_TYPE_QUALCOMM         0x0001 /* EGL_QUALCOMM_shared_image attrib_list */
#define EGL_IMAGE_FORMAT_QUALCOMM           0x0002 /* EGL_QUALCOMM_shared_image attrib_list */
#define EGL_COLOR_QUALCOMM                  0x0001 /* EGL_QUALCOMM_shared_image attrib_list EGL_COMPONENT_TYPE_QUALCOMM */
#define EGL_DEPTH_QUALCOMM                  0x0002 /* EGL_QUALCOMM_shared_image attrib_list EGL_COMPONENT_TYPE_QUALCOMM */
#define EGL_STENCIL_QUALCOMM                0x0004 /* EGL_QUALCOMM_shared_image attrib_list EGL_COMPONENT_TYPE_QUALCOMM */
#define EGL_ALPHAMASK_QUALCOMM              0x0008 /* EGL_QUALCOMM_shared_image attrib_list EGL_COMPONENT_TYPE_QUALCOMM */
#define EGL_RGB_565_QUALCOMM                0x0001 /* EGL_QUALCOMM_shared_image attrib_list EGL_IMAGE_FORMAT_QUALCOMM */
#define EGL_RGB_888_QUALCOMM                0x0002 /* EGL_QUALCOMM_shared_image attrib_list EGL_IMAGE_FORMAT_QUALCOMM */
#define EGL_RGBA_8888_QUALCOMM              0x0003 /* EGL_QUALCOMM_shared_image attrib_list EGL_IMAGE_FORMAT_QUALCOMM */
#define EGL_YUV_YUYV_QUALCOMM               0x0004 /* EGL_QUALCOMM_shared_image attrib_list EGL_IMAGE_FORMAT_QUALCOMM */
#define EGL_YUV_UYVY_QUALCOMM               0x0005 /* EGL_QUALCOMM_shared_image attrib_list EGL_IMAGE_FORMAT_QUALCOMM */
#define EGL_YUV_YV12_QUALCOMM               0x0006 /* EGL_QUALCOMM_shared_image attrib_list EGL_IMAGE_FORMAT_QUALCOMM */
#define EGL_YUV_NV21_QUALCOMM               0x0007 /* EGL_QUALCOMM_shared_image attrib_list EGL_IMAGE_FORMAT_QUALCOMM */
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLImageKHR EGLAPIENTRY eglCreateSharedImageQUALCOMM ( EGLDisplay dpy, EGLContext ctx, const EGLint* share_list, const EGLint* attrib_list );
EGLAPI EGLBoolean EGLAPIENTRY eglQueryImageQUALCOMM ( EGLDisplay dpy, EGLImageKHR image, EGLint attribute, EGLint* value );
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLImageKHR (EGLAPIENTRYP PFNEGLCREATESHAREDIMAGEQUALCOMMPROC) (EGLDisplay dpy, EGLContext ctx, const EGLint* share_list, const EGLint* attrib_list );
typedef EGLBoolean (EGLAPIENTRYP PFNEGLQUERYIMAGEQUALCOMMPROC) ( EGLDisplay dpy, EGLImageKHR image, EGLint attribute, EGLint* value );
#endif /* EGL_QUALCOMM_shared_image */

#ifndef EGL_AMD_create_image
#define EGL_AMD_create_image 1
#define EGL_NEW_IMAGE_AMD                               0x3120
#define EGL_IMAGE_FORMAT_AMD                            0x3121
#define EGL_FORMAT_RGBA_8888_AMD                        0x3122
#define EGL_FORMAT_RGB_565_AMD                          0x3123
#define EGL_FORMAT_YUYV_AMD                             0x3124
#define EGL_FORMAT_UYVY_AMD                             0x3125
#define EGL_FORMAT_YV12_AMD                             0x3126
#define EGL_FORMAT_NV21_AMD                             0x3127
#define EGL_FORMAT_NV12_TILED_AMD                       0x3128
#define EGL_FORMAT_BGRA_8888_AMD                        0x3129
#define EGL_FORMAT_BGRX_8888_AMD                        0x312A
#define EGL_FORMAT_XRGB_8888_AMD                        0x312B
#define EGL_FORMAT_ARGB_4444_AMD                        0x312C
#define EGL_FORMAT_ARGB_1555_AMD                        0x312D
#define EGL_FORMAT_XRGB_1555_AMD                        0x312E

//Added for Powerlift Testing
#define EGL_FORMAT_NV12_QCOM            0x31C2

// 0x312F unused yet.
#endif

#endif // __eglQCOM_h__

