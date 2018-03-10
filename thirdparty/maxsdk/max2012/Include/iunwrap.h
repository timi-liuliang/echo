//-------------------------------------------------------------
// Access to UVW Unwrap
//


#pragma once

#include "iFnPub.h"
#include "object.h"

#define UNWRAP_CLASSID	Class_ID(0x02df2e3a,0x72ba4e1f)

// Flags
#define CONTROL_FIT			(1<<0)
#define CONTROL_CENTER		(1<<1)
#define CONTROL_ASPECT		(1<<2)
#define CONTROL_UNIFORM		(1<<3)
#define CONTROL_HOLD		(1<<4)
#define CONTROL_INIT		(1<<5)
#define CONTROL_OP			(CONTROL_FIT|CONTROL_CENTER|CONTROL_ASPECT|CONTROL_UNIFORM)
#define CONTROL_INITPARAMS	(1<<10)

#define IS_MESH		1
#define IS_PATCH	2
#define IS_NURBS	3
#define IS_MNMESH	4




#define FLAG_DEAD		1
#define FLAG_HIDDEN		2
#define FLAG_FROZEN		4
//#define FLAG_QUAD		8
#define FLAG_RIGPOINT		8  //vertex specific flags
#define FLAG_SELECTED	16
#define FLAG_CURVEDMAPPING	32
#define FLAG_INTERIOR	64
#define FLAG_WEIGHTMODIFIED	128
//face/edge specific flags
#define FLAG_HIDDENEDGEA	256
#define FLAG_HIDDENEDGEB	512
#define FLAG_HIDDENEDGEC	1024



#define ID_TOOL_SELECT	0x0001
#define ID_TOOL_MOVEPIVOT	0x0002
#define ID_TOOL_MOVE	0x0100
#define ID_TOOL_ROTATE	0x0110
#define ID_TOOL_SCALE	0x0120
#define ID_TOOL_PAN		0x0130
#define ID_TOOL_ZOOM    0x0140
#define ID_TOOL_PICKMAP	0x0160
#define ID_TOOL_ZOOMREG 0x0170
#define ID_TOOL_UVW		0x0200
#define ID_TOOL_PROP	0x0210 
#define ID_TOOL_SHOWMAP	0x0220
#define ID_TOOL_UPDATE	0x0230
#define ID_TOOL_ZOOMEXT	0x0240
#define ID_TOOL_BREAK	0x0250
#define ID_TOOL_WELD	0x0260
#define ID_TOOL_WELD_SEL 0x0270
#define ID_TOOL_HIDE	 0x0280
#define ID_TOOL_UNHIDE	 0x0290
#define ID_TOOL_FREEZE	 0x0300
#define ID_TOOL_UNFREEZE	 0x0310
#define ID_TOOL_TEXTURE_COMBO 0x0320
#define ID_TOOL_SNAP 0x0330
#define ID_TOOL_LOCKSELECTED 0x0340
#define ID_TOOL_MIRROR 0x0350
#define ID_TOOL_FILTER_SELECTEDFACES 0x0360
#define ID_TOOL_FILTER_MATID 0x0370
#define ID_TOOL_INCSELECTED 0x0380
#define ID_TOOL_FALLOFF 0x0390
#define ID_TOOL_FALLOFF_SPACE 0x0400
#define ID_TOOL_FLIP 0x0410
#define ID_TOOL_DECSELECTED 0x0420




#define FILL_MODE_OFF		1
#define FILL_MODE_SOLID		2
#define FILL_MODE_BDIAGONAL	3
#define FILL_MODE_CROSS		4
#define FILL_MODE_DIAGCROSS	5
#define FILL_MODE_FDIAGONAL	6
#define FILL_MODE_HORIZONAL	7
#define FILL_MODE_VERTICAL	8


#define SKETCH_SELPICK		1
#define SKETCH_SELDRAG		2
#define SKETCH_SELCURRENT	3
#define SKETCH_DRAWMODE		4
#define SKETCH_APPLYMODE	5

#define SKETCH_FREEFORM		1
#define SKETCH_LINE			2
#define SKETCH_BOX			3
#define SKETCH_CIRCLE		4




class IUnwrapMod;

//***************************************************************
//Function Publishing System stuff   
//****************************************************************
#define UNWRAP_CLASSID	Class_ID(0x02df2e3a,0x72ba4e1f)
#define UNWRAP_INTERFACE Interface_ID(0x53b3409b, 0x18ff7ab8)
#define UNWRAP_INTERFACE2 Interface_ID(0x53b3409b, 0x18ff7ab9)
//5.1.05
#define UNWRAP_INTERFACE3 Interface_ID(0x53b3409b, 0x18ff7ac0)

#define UNWRAP_INTERFACE4 Interface_ID(0x53b3409b, 0x18ff7ac1)


#define GetIUnwrapInterface(cd) \
			(IUnwrapMod *)(cd)->GetInterface(UNWRAP_INTERFACE)

enum {  unwrap_planarmap,unwrap_save,unwrap_load, unwrap_reset, unwrap_edit,
		unwrap_setMapChannel,unwrap_getMapChannel,
		unwrap_setProjectionType,unwrap_getProjectionType,
		unwrap_setVC,unwrap_getVC,
		unwrap_move,unwrap_moveh,unwrap_movev,
		unwrap_rotate,
		unwrap_scale,unwrap_scaleh,unwrap_scalev,
		unwrap_mirrorh,unwrap_mirrorv,
		unwrap_expandsel, unwrap_contractsel,
		unwrap_setFalloffType,unwrap_getFalloffType,
		unwrap_setFalloffSpace,unwrap_getFalloffSpace,
		unwrap_setFalloffDist,unwrap_getFalloffDist,
		unwrap_breakselected,
		unwrap_weldselected, unwrap_weld,
		unwrap_updatemap,unwrap_displaymap,unwrap_ismapdisplayed,
		unwrap_setuvspace, unwrap_getuvspace,
		unwrap_options,
		unwrap_lock,
		unwrap_hide, unwrap_unhide,
		unwrap_freeze, unwrap_thaw,
		unwrap_filterselected,
		unwrap_pan,unwrap_zoom, unwrap_zoomregion, unwrap_fit, unwrap_fitselected,
		unwrap_snap,
		unwrap_getcurrentmap,unwrap_setcurrentmap, unwrap_numbermaps,
		unwrap_getlinecolor,unwrap_setlinecolor,
		unwrap_getselectioncolor,unwrap_setselectioncolor,
		unwrap_getrenderwidth,unwrap_setrenderwidth,
		unwrap_getrenderheight,unwrap_setrenderheight,
		unwrap_getusebitmapres,unwrap_setusebitmapres,
		unwrap_getweldtheshold,unwrap_setweldtheshold,

		unwrap_getconstantupdate,unwrap_setconstantupdate,
		unwrap_getshowselectedvertices,unwrap_setshowselectedvertices,
		unwrap_getmidpixelsnap,unwrap_setmidpixelsnap,

		unwrap_getmatid,unwrap_setmatid, unwrap_numbermatids,
		unwrap_getselectedverts, unwrap_selectverts,
		unwrap_isvertexselected,

		unwrap_moveselectedvertices,
		unwrap_rotateselectedverticesc,
		unwrap_rotateselectedvertices,
		unwrap_scaleselectedverticesc,
		unwrap_scaleselectedvertices,
		unwrap_getvertexposition,
		unwrap_numbervertices,

		unwrap_movex, unwrap_movey, unwrap_movez,

		unwrap_getselectedpolygons, unwrap_selectpolygons, unwrap_ispolygonselected,
		unwrap_numberpolygons,

		unwrap_detachedgeverts,
		unwrap_fliph,unwrap_flipv ,

		unwrap_setlockaspect,unwrap_getlockaspect,

		unwrap_setmapscale,unwrap_getmapscale,
		unwrap_getselectionfromface,

		unwrap_forceupdate,
		unwrap_zoomtogizmo,

		unwrap_setvertexposition,
		unwrap_addvertex,
		unwrap_markasdead,

		unwrap_numberpointsinface,
		unwrap_getvertexindexfromface,
		unwrap_gethandleindexfromface,
		unwrap_getinteriorindexfromface,

		unwrap_getvertexgindexfromface,
		unwrap_gethandlegindexfromface,
		unwrap_getinteriorgindexfromface,


		unwrap_addpointtoface,
		unwrap_addpointtohandle,
		unwrap_addpointtointerior,

		unwrap_setfacevertexindex,
		unwrap_setfacehandleindex,
		unwrap_setfaceinteriorindex,

		unwrap_updateview,

		unwrap_getfaceselfromstack,

//UNFOLD STUFF
		unwrap_selectfacesbynormal,
		unwrap_selectclusterbynormal,
		unwrap_selectpolygonsupdate,

		unwrap_normalmap,
		unwrap_normalmapnoparams,
		unwrap_normalmapdialog,

		unwrap_flattenmap,
		unwrap_flattenmapdialog,
		unwrap_flattenmapnoparams,

		unwrap_unfoldmap,
		unwrap_unfoldmapnoparams,
		unwrap_unfoldmapdialog,
		

		unwrap_hideselectedpolygons,
		unwrap_unhideallpolygons,

		unwrap_getnormal,
		unwrap_setseedface,

		unwrap_showvertexconnectionlist,

//COPY PASTE
		unwrap_copy,
		unwrap_paste,
		unwrap_pasteinstance,

		unwrap_setdebuglevel,

//STITCH STUFF
		unwrap_stitchverts,
		unwrap_stitchvertsnoparams,
		unwrap_stitchvertsdialog,
		unwrap_selectelement,

//TILEOPTIONS
		unwrap_gettilemap,
		unwrap_settilemap,
		unwrap_gettilemaplimit,
		unwrap_settilemaplimit,
		unwrap_gettilemapcontrast,
		unwrap_settilemapcontrast,

		unwrap_getshowmap,unwrap_setshowmap,

		unwrap_setlimitsoftsel,
		unwrap_getlimitsoftsel,

		
//SELECTION TOOLS AND OPTIONS
		unwrap_setlimitsoftselrange,
		unwrap_getlimitsoftselrange,

		unwrap_getvertexweight, unwrap_setvertexweight,
		unwrap_isweightmodified,unwrap_modifyweight,

		unwrap_getgeom_elemmode,unwrap_setgeom_elemmode,

		unwrap_getgeom_planarmode,unwrap_setgeom_planarmode,
		unwrap_getgeom_planarmodethreshold,unwrap_setgeom_planarmodethreshold,

		unwrap_getwindowx, unwrap_getwindowy, unwrap_getwindoww, unwrap_getwindowh, 

		unwrap_getbackfacecull,unwrap_setbackfacecull,

		unwrap_getoldselmethod,unwrap_setoldselmethod,
		unwrap_selectbymatid,
		unwrap_selectbysg,

		unwrap_gettvelementmode,unwrap_settvelementmode,

		unwrap_geomexpandsel, unwrap_geomcontractsel,

		unwrap_getalwaysedit,unwrap_setalwaysedit,

		unwrap_getshowvertexconnectionlist,unwrap_setshowvertexconnectionlist,

		unwrap_getfilterselected,unwrap_setfilterselected,

		unwrap_getsnap,	unwrap_setsnap,
		unwrap_getlock,	unwrap_setlock,

		unwrap_pack,
		unwrap_packnoparams,
		unwrap_packdialog,

		unwrap_gettvsubobjectmode,unwrap_settvsubobjectmode,

		unwrap_getselectedfaces, unwrap_selectfaces,
		unwrap_isfaceselected,

		unwrap_getfillmode,unwrap_setfillmode,

		unwrap_moveselected,
		unwrap_rotateselectedc,
		unwrap_rotateselected,
		unwrap_scaleselectedc,
		unwrap_scaleselected,

		unwrap_getselectededges, unwrap_selectedges,
		unwrap_isedgeselected,

		unwrap_getdisplayopenedge,
		unwrap_setdisplayopenedge,
		
		unwrap_getopenedgecolor,
		unwrap_setopenedgecolor,

		unwrap_getuvedgemode,
		unwrap_setuvedgemode,

		unwrap_getopenedgemode,
		unwrap_setopenedgemode,

		unwrap_uvedgeselect,
		unwrap_openedgeselect,

		unwrap_selectverttoedge,
		unwrap_selectverttoface,

		unwrap_selectedgetovert,
		unwrap_selectedgetoface,

		unwrap_selectfacetovert,
		unwrap_selectfacetoedge,

		unwrap_getdisplayhiddenedge,
		unwrap_setdisplayhiddenedge,

		unwrap_gethandlecolor,unwrap_sethandlecolor,

		unwrap_getfreeformmode,unwrap_setfreeformmode,

		unwrap_getfreeformcolor,unwrap_setfreeformcolor,
		unwrap_scaleselectedxy,

		unwrap_snappivot,
		unwrap_getpivotoffset,unwrap_setpivotoffset,
		unwrap_getselcenter,

		unwrap_sketch,
		unwrap_sketchnoparams,
		unwrap_sketchdialog,

		unwrap_sketchreverse,
		
		unwrap_gethitsize,unwrap_sethitsize,

		unwrap_getresetpivotonsel,unwrap_setresetpivotonsel,

		unwrap_getpolymode,unwrap_setpolymode,
		unwrap_polyselect,

		unwrap_getselectioninsidegizmo,unwrap_setselectioninsidegizmo,

		unwrap_setasdefaults,
		unwrap_loaddefaults,

		unwrap_getshowshared,unwrap_setshowshared,
		unwrap_getsharedcolor,unwrap_setsharedcolor,
		
		unwrap_showicon,

		unwrap_getsyncselectionmode,unwrap_setsyncselectionmode,
		unwrap_synctvselection,unwrap_syncgeomselection,

		unwrap_getbackgroundcolor,unwrap_setbackgroundcolor,

		unwrap_updatemenubar,

		unwrap_getbrightcentertile,unwrap_setbrightcentertile,

		unwrap_getblendtiletoback,unwrap_setblendtiletoback,

		unwrap_getblendtoback,unwrap_setblendtoback,

		unwrap_getpaintmode,unwrap_setpaintmode,
		unwrap_getpaintsize,unwrap_setpaintsize,
		unwrap_incpaintsize,unwrap_decpaintsize,

		unwrap_getticksize,unwrap_setticksize,

//NEW
		unwrap_getgridsize,unwrap_setgridsize,
		unwrap_getgridsnap,unwrap_setgridsnap,
		unwrap_getgridvisible,unwrap_setgridvisible,
		unwrap_getgridcolor,unwrap_setgridcolor,
		unwrap_getgridstr,unwrap_setgridstr,

		unwrap_getautomap,unwrap_setautomap,
//flatten defaults
		unwrap_getflattenangle,unwrap_setflattenangle,
		unwrap_getflattenspacing,unwrap_setflattenspacing,
		unwrap_getflattennormalize,unwrap_setflattennormalize,
		unwrap_getflattenrotate,unwrap_setflattenrotate,
		unwrap_getflattenfillholes,unwrap_setflattenfillholes,

		unwrap_getpreventflattening,unwrap_setpreventflattening,

		unwrap_getenablesoftselection,unwrap_setenablesoftselection,
		unwrap_getapplytowholeobject,unwrap_setapplytowholeobject,

		unwrap_setvertexposition2,
		unwrap_relax,
		unwrap_fitrelax,
//5.1.05
		unwrap_getautobackground,unwrap_setautobackground,

//5.1.06
		unwrap_relax2, unwrap_relax2dialog,
		unwrap_setrelaxamount,unwrap_getrelaxamount,
		unwrap_setrelaxiter,unwrap_getrelaxiter,

		unwrap_setrelaxboundary,unwrap_getrelaxboundary,
		unwrap_setrelaxsaddle,unwrap_getrelaxsaddle,

		unwrap_setthickopenedges,unwrap_getthickopenedges,
		unwrap_setviewportopenedges,unwrap_getviewportopenedges,

		unwrap_selectinvertedfaces,
		unwrap_getrelativetypein,unwrap_setrelativetypein,

		unwrap_stitchverts2,

		unwrap_addmap,

		unwrap_flattenmapbymatid,

		unwrap_getarea,
		unwrap_getrotationsrespectaspect,
		unwrap_setrotationsrespectaspect,

		unwrap_setmax5flatten,

		};
//****************************************************************


/*! \sa  Class Modifier, Class FPStaticInterface\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface to the UVW Unwrap Modifier. You can obtain
a pointer to the UVW Unwrap Modifier interface using;
<b>GetIUnwrapInterface(cd)</b>. This macro will return\n\n
<b>(IUnwrapMod *)(cd)-\>GetFPInterace(UNWRAP_INTERFACE).</b> Sample code
supporting this class can be found in
<b>/MAXSDK/SAMPLES/MODIFIERS/UVWUNWRAP</b>\n\n
All methods of this class are Implemented by the System.  */
class IUnwrapMod :  public Modifier, public FPMixinInterface 
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP
			VFN_0(unwrap_planarmap, fnPlanarMap);
			VFN_0(unwrap_save, fnSave);
			VFN_0(unwrap_load, fnLoad);
			VFN_0(unwrap_reset, fnReset);
			VFN_0(unwrap_edit, fnEdit);
			VFN_1(unwrap_setMapChannel, fnSetMapChannel,TYPE_INT);
			FN_0(unwrap_getMapChannel, TYPE_INT, fnGetMapChannel);
			VFN_1(unwrap_setProjectionType, fnSetProjectionType,TYPE_INT);
			FN_0(unwrap_getProjectionType, TYPE_INT, fnGetProjectionType);
			VFN_1(unwrap_setVC, fnSetVC,TYPE_BOOL);
			FN_0(unwrap_getVC, TYPE_BOOL, fnGetVC);

			VFN_0(unwrap_move, fnMove);
			VFN_0(unwrap_moveh, fnMoveH);
			VFN_0(unwrap_movev, fnMoveV);

			VFN_0(unwrap_rotate, fnRotate);

			VFN_0(unwrap_scale, fnScale);
			VFN_0(unwrap_scaleh, fnScaleH);
			VFN_0(unwrap_scalev, fnScaleV);

			VFN_0(unwrap_mirrorh, fnMirrorH);
			VFN_0(unwrap_mirrorv, fnMirrorV);
			VFN_0(unwrap_expandsel, fnExpandSelection);
			VFN_0(unwrap_contractsel, fnContractSelection);
			VFN_1(unwrap_setFalloffType, fnSetFalloffType,TYPE_INT);
			FN_0(unwrap_getFalloffType, TYPE_INT, fnGetFalloffType);
			VFN_1(unwrap_setFalloffSpace, fnSetFalloffSpace,TYPE_INT);
			FN_0(unwrap_getFalloffSpace, TYPE_INT, fnGetFalloffSpace);
			VFN_1(unwrap_setFalloffDist, fnSetFalloffDist,TYPE_FLOAT);
			FN_0(unwrap_getFalloffDist, TYPE_FLOAT, fnGetFalloffDist);
			VFN_0(unwrap_breakselected, fnBreakSelected);
			VFN_0(unwrap_weld, fnWeld);
			VFN_0(unwrap_weldselected, fnWeldSelected);
			VFN_0(unwrap_updatemap, fnUpdatemap);
			VFN_1(unwrap_displaymap, fnDisplaymap, TYPE_BOOL);
			FN_0(unwrap_ismapdisplayed, TYPE_BOOL, fnIsMapDisplayed);
			VFN_1(unwrap_setuvspace, fnSetUVSpace,TYPE_INT);
			FN_0(unwrap_getuvspace, TYPE_INT, fnGetUVSpace);
			VFN_0(unwrap_options, fnOptions);
			VFN_0(unwrap_lock, fnLock);
			VFN_0(unwrap_hide, fnHide);
			VFN_0(unwrap_unhide, fnUnhide);
			VFN_0(unwrap_freeze, fnFreeze);
			VFN_0(unwrap_thaw, fnThaw);
			VFN_0(unwrap_filterselected, fnFilterSelected);

			VFN_0(unwrap_pan, fnPan);
			VFN_0(unwrap_zoom, fnZoom);
			VFN_0(unwrap_zoomregion, fnZoomRegion);
			VFN_0(unwrap_fit, fnFit);
			VFN_0(unwrap_fitselected, fnFitSelected);

			VFN_0(unwrap_snap, fnSnap);

			FN_0(unwrap_getcurrentmap,TYPE_INT, fnGetCurrentMap);
			VFN_1(unwrap_setcurrentmap, fnSetCurrentMap,TYPE_INT);
			FN_0(unwrap_numbermaps,TYPE_INT, fnNumberMaps);

			FN_0(unwrap_getlinecolor,TYPE_POINT3, fnGetLineColor);
			VFN_1(unwrap_setlinecolor, fnSetLineColor,TYPE_POINT3);
			FN_0(unwrap_getselectioncolor,TYPE_POINT3, fnGetSelColor);
			VFN_1(unwrap_setselectioncolor, fnSetSelColor,TYPE_POINT3);

			FN_0(unwrap_getrenderwidth,TYPE_INT, fnGetRenderWidth);
			VFN_1(unwrap_setrenderwidth, fnSetRenderWidth,TYPE_INT);
			FN_0(unwrap_getrenderheight,TYPE_INT, fnGetRenderHeight);
			VFN_1(unwrap_setrenderheight, fnSetRenderHeight,TYPE_INT);

			FN_0(unwrap_getusebitmapres,TYPE_BOOL, fnGetUseBitmapRes);
			VFN_1(unwrap_setusebitmapres, fnSetUseBitmapRes,TYPE_BOOL);

			FN_0(unwrap_getweldtheshold,TYPE_FLOAT, fnGetWeldThresold);
			VFN_1(unwrap_setweldtheshold, fnSetWeldThreshold,TYPE_FLOAT);


			FN_0(unwrap_getconstantupdate,TYPE_BOOL, fnGetConstantUpdate);
			VFN_1(unwrap_setconstantupdate, fnSetConstantUpdate,TYPE_BOOL);

			FN_0(unwrap_getshowselectedvertices,TYPE_BOOL, fnGetShowSelectedVertices);
			VFN_1(unwrap_setshowselectedvertices, fnSetShowSelectedVertices,TYPE_BOOL);

			FN_0(unwrap_getmidpixelsnap,TYPE_BOOL, fnGetMidPixelSnape);
			VFN_1(unwrap_setmidpixelsnap, fnSetMidPixelSnape,TYPE_BOOL);


			FN_0(unwrap_getmatid,TYPE_INT, fnGetMatID);
			VFN_1(unwrap_setmatid, fnSetMatID,TYPE_INT);
			FN_0(unwrap_numbermatids,TYPE_INT, fnNumberMatIDs);

			FN_0(unwrap_getselectedverts,TYPE_BITARRAY, fnGetSelectedVerts);
			VFN_1(unwrap_selectverts, fnSelectVerts,TYPE_BITARRAY);
			FN_1(unwrap_isvertexselected,TYPE_BOOL, fnIsVertexSelected,TYPE_INT);

			VFN_1(unwrap_moveselectedvertices, fnMoveSelectedVertices,TYPE_POINT3);
			VFN_1(unwrap_rotateselectedverticesc, fnRotateSelectedVertices,TYPE_FLOAT);
			VFN_2(unwrap_rotateselectedvertices, fnRotateSelectedVertices,TYPE_FLOAT, TYPE_POINT3);
			VFN_2(unwrap_scaleselectedverticesc, fnScaleSelectedVertices,TYPE_FLOAT, TYPE_INT);
			VFN_3(unwrap_scaleselectedvertices, fnScaleSelectedVertices,TYPE_FLOAT, TYPE_INT,TYPE_POINT3);

			FN_2(unwrap_getvertexposition,TYPE_POINT3, fnGetVertexPosition, TYPE_TIMEVALUE, TYPE_INT);
			FN_0(unwrap_numbervertices,TYPE_INT, fnNumberVertices);

			VFN_1(unwrap_movex, fnMoveX,TYPE_FLOAT);
			VFN_1(unwrap_movey, fnMoveY,TYPE_FLOAT);
			VFN_1(unwrap_movez, fnMoveZ,TYPE_FLOAT);

			FN_0(unwrap_getselectedpolygons,TYPE_BITARRAY, fnGetSelectedPolygons);
			VFN_1(unwrap_selectpolygons, fnSelectPolygons,TYPE_BITARRAY);
			FN_1(unwrap_ispolygonselected,TYPE_BOOL, fnIsPolygonSelected,TYPE_INT);
			FN_0(unwrap_numberpolygons,TYPE_INT, fnNumberPolygons);
			VFN_0(unwrap_detachedgeverts, fnDetachEdgeVerts);
			VFN_0(unwrap_fliph, fnFlipH);
			VFN_0(unwrap_flipv, fnFlipV);
			
			VFN_1(unwrap_setlockaspect, fnSetLockAspect,TYPE_BOOL);
			FN_0(unwrap_getlockaspect,TYPE_BOOL, fnGetLockAspect);

			VFN_1(unwrap_setmapscale, fnSetMapScale,TYPE_FLOAT);
			FN_0(unwrap_getmapscale,TYPE_FLOAT, fnGetMapScale);

			VFN_0(unwrap_getselectionfromface, fnGetSelectionFromFace);

			VFN_1(unwrap_forceupdate, fnForceUpdate,TYPE_BOOL);

			VFN_1(unwrap_zoomtogizmo, fnZoomToGizmo,TYPE_BOOL);

			VFN_3(unwrap_setvertexposition, fnSetVertexPosition,TYPE_TIMEVALUE,TYPE_INT,TYPE_POINT3);
			VFN_1(unwrap_markasdead, fnMarkAsDead,TYPE_INT);

			FN_1(unwrap_numberpointsinface,TYPE_INT,fnNumberPointsInFace,TYPE_INT);
			FN_2(unwrap_getvertexindexfromface,TYPE_INT,fnGetVertexIndexFromFace,TYPE_INT,TYPE_INT);
			FN_2(unwrap_gethandleindexfromface,TYPE_INT,fnGetHandleIndexFromFace,TYPE_INT,TYPE_INT);
			FN_2(unwrap_getinteriorindexfromface,TYPE_INT,fnGetInteriorIndexFromFace,TYPE_INT,TYPE_INT);
			FN_2(unwrap_getvertexgindexfromface,TYPE_INT,fnGetVertexGIndexFromFace,TYPE_INT,TYPE_INT);
			FN_2(unwrap_gethandlegindexfromface,TYPE_INT,fnGetHandleGIndexFromFace,TYPE_INT,TYPE_INT);
			FN_2(unwrap_getinteriorgindexfromface,TYPE_INT,fnGetInteriorGIndexFromFace,TYPE_INT,TYPE_INT);
			
			VFN_4(unwrap_addpointtoface,fnAddPoint,TYPE_POINT3,TYPE_INT,TYPE_INT,TYPE_BOOL);
			VFN_4(unwrap_addpointtohandle,fnAddHandle,TYPE_POINT3,TYPE_INT,TYPE_INT,TYPE_BOOL);
			VFN_4(unwrap_addpointtointerior,fnAddInterior,TYPE_POINT3,TYPE_INT,TYPE_INT,TYPE_BOOL);

			VFN_3(unwrap_setfacevertexindex,fnSetFaceVertexIndex,TYPE_INT,TYPE_INT,TYPE_INT);
			VFN_3(unwrap_setfacehandleindex,fnSetFaceHandleIndex,TYPE_INT,TYPE_INT,TYPE_INT);
			VFN_3(unwrap_setfaceinteriorindex,fnSetFaceInteriorIndex,TYPE_INT,TYPE_INT,TYPE_INT);

			VFN_0(unwrap_updateview,fnUpdateViews);

			VFN_0(unwrap_getfaceselfromstack,fnGetFaceSelFromStack);
			


		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 

		/*! \remarks This method will press the Planar Map button in the
		rollup interface. */
		virtual void	fnPlanarMap()=0;
		/*! \remarks This method will press the Save button in the rollup
		interface. */
		virtual void	fnSave()=0;
		/*! \remarks This method will press the Load button in the rollup
		interface. */
		virtual void	fnLoad()=0;
		/*! \remarks This method will press the Reset button in the rollup
		interface. */
		virtual void	fnReset()=0;
		/*! \remarks This method will press the Edit button in the rollup
		interface. */
		virtual void	fnEdit()=0;

		/*! \remarks This method will set the Map Channel field value in the
		rollup.
		\par Parameters:
		<b>int channel</b>\n\n
		The Map Channel you want to set to. */
		virtual void	fnSetMapChannel(int channel)=0;
		/*! \remarks This method will return the Map Channel field in the
		rollup. */
		virtual int		fnGetMapChannel()=0;

		/*! \remarks This method will set the mapping type.
		\par Parameters:
		<b>int proj</b>\n\n
		The mapping type; 1 for X aligned, 2 for Y aligned, 3 for Z aligned, 4
		for normal aligned.
		\return    */
		virtual void	fnSetProjectionType(int proj)=0;
		/*! \remarks This method will return the mapping type; 1 for X
		aligned, 2 for Y aligned, 3 for Z aligned, 4 for normal aligned. */
		virtual int		fnGetProjectionType()=0;

		/*! \remarks This method will set the Vertex Color Channel radio
		button in the rollup interface.
		\par Parameters:
		<b>BOOL vc</b>\n\n
		TRUE to enable; FALSE to disable.\n\n
		  */
		virtual void	fnSetVC(BOOL vc)=0;
		/*! \remarks This method returns the current state of the Vertex Color
		Channel radio button in the rollup interface. */
		virtual BOOL	fnGetVC()=0;

		/*! \remarks This method will press the Move button in the edit
		floater. */
		virtual void	fnMove()=0;
		/*! \remarks This method will press the Move Horizontal button in the
		edit floater. */
		virtual void	fnMoveH()=0;
		/*! \remarks This method will press the Move Vertical button in the
		edit floater. */
		virtual void	fnMoveV()=0;

		/*! \remarks This method will press the Rotate button in the edit
		floater. */
		virtual void	fnRotate()=0;

		/*! \remarks This method will press the Scale button in the edit
		floater. */
		virtual void	fnScale()=0;
		/*! \remarks This method will press the Scale Horizontal button in the
		edit floater. */
		virtual void	fnScaleH()=0;
		/*! \remarks This method will press the Scale Vertical button in the
		edit floater. */
		virtual void	fnScaleV()=0;

		/*! \remarks This method will press the Mirror Horizontal button in
		the edit floater. */
		virtual void	fnMirrorH()=0;
		/*! \remarks This method will press the Mirror Vertical button in the
		edit floater. */
		virtual void	fnMirrorV()=0;

		/*! \remarks This method will press the Expand Selection button in the
		edit floater. */
		virtual void	fnExpandSelection()=0;
		/*! \remarks This method will press the Contract Selection button in
		the edit floater. */
		virtual void	fnContractSelection()=0;
		

		/*! \remarks This method will set the Falloff type.
		\par Parameters:
		<b>int falloff</b>\n\n
		The falloff type; 1 for linear, 2 for sinual, 3 for fast, and 4 for
		slow. */
		virtual void	fnSetFalloffType(int falloff)=0;
		/*! \remarks This method will return the falloff type; 1 for linear, 2
		for sinual, 3 for fast, and 4 for slow. */
		virtual int		fnGetFalloffType()=0;
		/*! \remarks This method will set the space you want the falloff to be
		computed in.
		\par Parameters:
		<b>int space</b>\n\n
		The falloff space; 1 for XY, the local space of the object, 2 for UV,
		the UVW space of the object. */
		virtual void	fnSetFalloffSpace(int space)=0;
		/*! \remarks This method will return the falloff space; 1 for XY, the
		local space of the object, 2 for UV, the UVW space of the object. */
		virtual int		fnGetFalloffSpace()=0;
		/*! \remarks This method will set the falloff distance in the edit
		floater.
		\par Parameters:
		<b>float dist</b>\n\n
		The falloff distance. */
		virtual void	fnSetFalloffDist(float dist)=0;
		/*! \remarks This method will return the falloff distance. */
		virtual float	fnGetFalloffDist()=0;

		/*! \remarks This method will press the Break Selected button in the
		edit floater. */
		virtual void	fnBreakSelected()=0;
		/*! \remarks This method will press the Target Weld button in the edit
		floater. */
		virtual void	fnWeld()=0;
		/*! \remarks This method will press the Weld Selected button in the
		edit floater. */
		virtual void	fnWeldSelected()=0;

		/*! \remarks This method will press the Update Map button in the edit
		floater. */
		virtual void	fnUpdatemap()=0;
		/*! \remarks This method sets the state of the Display Map button in
		the edit floater
		\par Parameters:
		<b>BOOL update</b>\n\n
		TRUE to toggle the Display Map button on; FALSE to toggle it off. */
		virtual void	fnDisplaymap(BOOL update)=0;
		/*! \remarks This method returns the state of the Display Map button
		in the edit floater. TRUE if it's on; FALSE if it's off. */
		virtual BOOL	fnIsMapDisplayed()=0;

		/*! \remarks This method sets the space that you want to view the
		texture vertices in.
		\par Parameters:
		<b>int space</b>\n\n
		The texture space; 1 for UV, 2 for VW, 3 for UW. */
		virtual void	fnSetUVSpace(int space)=0;
		/*! \remarks This method returns the space that the texture vertices
		are viewed in; 1 for UV, 2 for VW, 3 for UW. */
		virtual int		fnGetUVSpace()=0;
		/*! \remarks This method will press the Options button in the edit
		floater. */
		virtual void	fnOptions()=0;

		/*! \remarks This method will toggle the Lock Selected Vertices button
		in the edit floater. */
		virtual void	fnLock()=0;
		/*! \remarks This method will press the Hide button in the edit
		floater. */
		virtual void	fnHide()=0;
		/*! \remarks This method will press the Unhide button in the edit
		floater. */
		virtual void	fnUnhide()=0;

		/*! \remarks This method will press the Freeze button in the edit
		floater. */
		virtual void	fnFreeze()=0;
		/*! \remarks This method will press the Unfreeze button in the edit
		floater. */
		virtual void	fnThaw()=0;
		/*! \remarks This method will press the Filter Selected Faces button
		in the edit floater. */
		virtual void	fnFilterSelected()=0;

		/*! \remarks This method will press the Pan button in the edit
		floater. */
		virtual void	fnPan()=0;
		/*! \remarks This method will press the Zoom button in the edit
		floater. */
		virtual void	fnZoom()=0;
		/*! \remarks This method will press the Zoom Region button in the edit
		floater. */
		virtual void	fnZoomRegion()=0;
		/*! \remarks This method will press the Fit button in the edit
		floater. */
		virtual void	fnFit()=0;
		/*! \remarks This method will press the Fit Selected button in the
		edit floater. */
		virtual void	fnFitSelected()=0;

		/*! \remarks This method will press the Snap button in the edit
		floater. */
		virtual void	fnSnap()=0;


		/*! \remarks This method returns the index into the map drop down list
		of the current map in the view of the edit floater. */
		virtual int		fnGetCurrentMap()=0;
		/*! \remarks This method sets the currently displayed map to the
		specified map index.
		\par Parameters:
		<b>int map</b>\n\n
		The index of the map in the drop down list to display. */
		virtual void	fnSetCurrentMap(int map)=0;
		/*! \remarks This method returns the number of maps in the map drop
		down list. */
		virtual int		fnNumberMaps()=0;

		/*! \remarks This method returns the color of the lines used to
		connect the texture vertices edges as a Point3 pointer. */
		virtual Point3*	fnGetLineColor()=0;
		/*! \remarks This method sets the line color of the texture vertices.
		\par Parameters:
		<b>Point3 color</b>\n\n
		The color as a Point3. */
		virtual void	fnSetLineColor(Point3 color)=0;

		/*! \remarks This method returns the texture vertices selection color
		as Point3. */
		virtual Point3*	fnGetSelColor()=0;
		/*! \remarks This method sets the color of selected texture vertices.
		\par Parameters:
		<b>Point3 color</b>\n\n
		The color as a Point3. */
		virtual void	fnSetSelColor(Point3 color)=0;



		/*! \remarks This method sets the width of the bitmap used to render
		to for display.
		\par Parameters:
		<b>int dist</b>\n\n
		The width in pixels. */
		virtual void	fnSetRenderWidth(int dist)=0;
		/*! \remarks This method returns the width of the bitmap used to
		render 2d/3d textures to and if the Use Bitmap Resolution bitmaps is
		not set the width used to render bitmap. */
		virtual int		fnGetRenderWidth()=0;
		/*! \remarks This method sets the width of the bitmap used to render
		to for display.
		\par Parameters:
		<b>int dist</b>\n\n
		The height in pixels. */
		virtual void	fnSetRenderHeight(int dist)=0;
		/*! \remarks This method returns the height of the bitmap used to
		render 2d/3d textures to and if the Use Bitmap Resolution bitmaps is
		not set the height used to render bitmap. */
		virtual int		fnGetRenderHeight()=0;
		
		/*! \remarks This method sets the threshold values for welds.
		\par Parameters:
		<b>float dist</b>\n\n
		The welding threshold/
		\return    */
		virtual void	fnSetWeldThreshold(float dist)=0;
		/*! \remarks This method returns the weld threshold. */
		virtual float	fnGetWeldThresold()=0;

		/*! \remarks This method sets the state of the Use Bitmap Resolution
		value. If it is false the bitmaps are rendered using the
		RenderWidth/Height values.
		\par Parameters:
		<b>BOOL useBitmapRes</b>\n\n
		TRUE to toggle on; FALSE to toggle off. */
		virtual void	fnSetUseBitmapRes(BOOL useBitmapRes)=0;
		/*! \remarks This method returns the state of the Use Bitmap
		Resolution, if false the bitmaps are rendered using the
		RenderWidth/Height values. */
		virtual BOOL	fnGetUseBitmapRes()=0;

		
		/*! \remarks This method returns the state of the Constant Update
		value which when set true forces the veiwport to be updated on every
		move, otherwise it is just updated on mouse up. */
		virtual BOOL	fnGetConstantUpdate()=0;
		/*! \remarks This method Sets the state of the Constant Update value
		which when set true forces the viewport to be updated on every move,
		otherwise it is just updated on mouse up.
		\par Parameters:
		<b>BOOL constantUpdates</b>\n\n
		TRUE to toggle on; FALSE to toggle off. */
		virtual void	fnSetConstantUpdate(BOOL constantUpdates)=0;

		/*! \remarks This method returns whether the selected texture vertices
		are also displayed in the view port. */
		virtual BOOL	fnGetShowSelectedVertices()=0;
		/*! \remarks This method sets whether the selected texture vertices
		are also displayed in the view port.
		\par Parameters:
		<b>BOOL show</b>\n\n
		TRUE to toggle on; FALSE to toggle off. */
		virtual void	fnSetShowSelectedVertices(BOOL show)=0;

		/*! \remarks This method returns whether the mid pixels snap is used,
		if it is false the snap is set to the bottom right corner of the pixel,
		else it snaps to the center of the pixel. */
		virtual BOOL	fnGetMidPixelSnape()=0;
		/*! \remarks This method sets whether the mid pixels snap is used, if
		it is false the snap is set to the bottom right corner of the pixel,
		else it snaps to the center of the pixel.
		\par Parameters:
		<b>BOOL midPixel</b>\n\n
		TRUE to toggle on; FALSE to toggle off. */
		virtual void	fnSetMidPixelSnape(BOOL midPixel)=0;

		/*! \remarks This method returns the current material id index filter.
		*/
		virtual int		fnGetMatID()=0;
		/*! \remarks This method sets the material drop list to the index
		supplied.
		\par Parameters:
		<b>int matid</b>\n\n
		The material ID index to set. */
		virtual void	fnSetMatID(int matid)=0;
		/*! \remarks This method returns the number of material ids in the
		material id filter drop down. */
		virtual int		fnNumberMatIDs()=0;

		/*! \remarks This method returns the current selected texture vertices
		in the edit floater as a bit array. */
		virtual BitArray* fnGetSelectedVerts()=0;
		/*! \remarks This method selects texture vertices in the edit floater
		dialog.
		\par Parameters:
		<b>BitArray *sel</b>\n\n
		The selection set as a bit array. */
		virtual void fnSelectVerts(BitArray *sel)=0;
		/*! \remarks This method returns whether a texture vertex is selected.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the vertex to check. */
		virtual BOOL fnIsVertexSelected(int index)=0;

		/*! \remarks This method moves the selected texture vertices by the
		offset.
		\par Parameters:
		<b>Point3 offset</b>\n\n
		The offset by which you want to move the vertices. */
		virtual void fnMoveSelectedVertices(Point3 offset)=0;
		/*! \remarks This method rotates the selected vertices around their
		center point.
		\par Parameters:
		<b>float angle</b>\n\n
		The angle in radians that you want to rotate the selection by. */
		virtual void fnRotateSelectedVertices(float angle)=0;
		/*! \remarks This method rotates the selected vertices around a
		specified point.
		\par Parameters:
		<b>float angle</b>\n\n
		The angle in radians that you want to rotate the selection by.\n\n
		<b>Point3 axis</b>\n\n
		The axis that you want to rotate the selected vertices by. This is in
		the space of the window. */
		virtual void fnRotateSelectedVertices(float angle, Point3 axis)=0;
		/*! \remarks This method scales the selected points around their
		center.
		\par Parameters:
		<b>float scale</b>\n\n
		The amount that you want to scale by\n\n
		<b>int dir</b>\n\n
		The direction; 1 for uniform scaling, 2 for X, and 3 for Y. */
		virtual void fnScaleSelectedVertices(float scale,int dir)=0;
		/*! \remarks This method scales the selected points around a specified
		point.
		\par Parameters:
		<b>float scale</b>\n\n
		The amount that you want to scale by\n\n
		<b>int dir</b>\n\n
		The direction; 1 for uniform scaling, 2 for X, and 3 for Y.\n\n
		<b>Point3 axis</b>\n\n
		The axis that you want to scale the selected vertices by. This is in
		the space of the window. */
		virtual void fnScaleSelectedVertices(float scale,int dir,Point3 axis)=0;
		/*! \remarks This method returns the position of the vertex.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which you want to get the vertex.\n\n
		<b>int index</b>\n\n
		The index of the vertex. */
		virtual Point3* fnGetVertexPosition(TimeValue t,  int index)=0;
		/*! \remarks This method returns the number of texture vertices */
		virtual int fnNumberVertices()=0;

		/*! \remarks This method sets the selected vertices x values in
		absolute coordinates.
		\par Parameters:
		<b>float p</b>\n\n
		The absolute position along the x axis
		\return    */
		virtual void fnMoveX(float p)=0;
		/*! \remarks This method sets the selected vertices y values in
		absolute coordinates.
		\par Parameters:
		<b>float p</b>\n\n
		The absolute position along the y axis */
		virtual void fnMoveY(float p)=0;
		/*! \remarks This method sets the selected vertices z values in
		absolute coordinates.
		\par Parameters:
		<b>float p</b>\n\n
		The absolute position along the s axis */
		virtual void fnMoveZ(float p)=0;

		/*! \remarks This method returns the selected polygons in the view
		port as a bit array. */
		virtual BitArray* fnGetSelectedPolygons()=0;
		/*! \remarks This method selects the polygons in the view ports.
		\par Parameters:
		<b>BitArray *sel</b>\n\n
		The polygons you wish to select. */
		virtual void fnSelectPolygons(BitArray *sel)=0;
		/*! \remarks This method returns whether a polygon is selected or not.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the polygon to check.
		\return    */
		virtual BOOL fnIsPolygonSelected(int index)=0;
		/*! \remarks This method returns the number of polygons in the object.
		*/
		virtual int fnNumberPolygons()=0;

		/*! \remarks This method detaches any vertex that is not completely
		surrounded by selected vertices. This is similar to a polygon selection
		detach except it uses the vertex selection to determine what is
		detached. */
		virtual void fnDetachEdgeVerts()=0;

		/*! \remarks This method will press the Flip Horizontal button in the
		edit floater. */
		virtual void fnFlipH()=0;
		/*! \remarks This method will press the Flip Vertical button in the
		edit floater. */
		virtual void fnFlipV()=0;

		/*! \remarks This method returns whether the edit window aspect ratio
		is locked or not, if the aspect ratio is not locked the image will try
		stretch to fit the aspect ratio of the window.
		\return  TRUE if locked; FALSE if unlocked. */
		virtual BOOL	fnGetLockAspect()=0;
		/*! \remarks This method sets the Lock Aspect Ratio value
		\par Parameters:
		<b>BOOL a</b>\n\n
		TRUE to lock; FALSE to unlock. */
		virtual void	fnSetLockAspect(BOOL a)=0;

		/*! \remarks This method returns the scaling factor when the user
		applies a planar map. The smaller the value the more planar map is
		scaled down. */
		virtual float	fnGetMapScale()=0;
		/*! \remarks This method sets the scaling factor when the user applies
		a planar map. The smaller the value the more planar map is scaled down
		\par Parameters:
		<b>float sc</b>\n\n
		The scaling factor for planar map. */
		virtual void	fnSetMapScale(float sc)=0;

		/*! \remarks This method takes the current polygon selection and uses
		it to select the texture vertices that are associated with it. */
		virtual void	fnGetSelectionFromFace()=0;
		/*! \remarks This method sets a flag to determines how Unwrap will
		behave when a topology change occurs. If update is TRUE the mapping
		gets reset, otherwise unwrap skips mapping the object if it has a
		different topology. It is sometimes useful to turn this off if you have
		MeshSmooth or other topology changing modifiers below Unwrap that have
		different topologies when rendering.
		\par Parameters:
		<b>BOOL update</b>\n\n
		This determines whether the mapping is reset on topology change. TRUE
		to update, otherwise FALSE. */
		virtual void	fnForceUpdate(BOOL update)= 0;

		/*! \remarks This method zooms the selected or all the viewports to
		zoom to the current planar map gizmo.
		\par Parameters:
		<b>BOOL all</b>\n\n
		This determines whether the active or all the viewports get zoomed.
		TRUE to zoom all viewports, FALSE to view the active viewport. */
		virtual void	fnZoomToGizmo(BOOL all)= 0;

		/*! \remarks This method sets the position of a UVW vertex at a
		specific time.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at what you want to set the position.\n\n
		<b>int index</b>\n\n
		The index of the vertex.\n\n
		<b>Point3 pos</b>\n\n
		The position of the vertex in UVW space. */
		virtual void	fnSetVertexPosition(TimeValue t, int index, Point3 pos) = 0;
		/*! \remarks This method marks a vertex that it is dead, and no longer
		in use. Vertices are not actually deleted they are just marked and
		recycled when needed. That means when a vertex is added vertices marked
		as dead will be the first ones checked. If there are no dead vertices,
		the vertex is appended to the end of the list. Using this function
		carefully since marking a vertex as dead that is actually in use will
		cause strange results.
		\par Parameters:
		<b>int vertex</b>\n\n
		The index of the vertex to mark as dead. */
		virtual void	fnMarkAsDead(int index) = 0;

		/*! \remarks This method retrieves the numbers of vertices that a face
		contains. A face can contain 3 to N number of points depending on what
		type of topology Unwrap is working on. For Tri Meshes this is always 3,
		for patches this can be 3 or 4, and for polygons this can be 3 or
		greater. Unwrap abstracts all three object types into one generic
		format.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the face to inspect. */
		virtual int		fnNumberPointsInFace(int index)=0;
		/*! \remarks This method retrieves the index of a vertex, from a face.
		A face contains 0 to N number of vertices. So to retrieve a particular
		vertex index, you give it the face index and the I-th vertex that you
		want to inspect. So if you wanted to look at the 3 vertex on face 1 you
		would call <b>GetVertexIndexFromFace(1,3)</b>.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the face to inspect.\n\n
		<b>int vertexIndex</b>\n\n
		The I-th vertex of that you want to retrieve. This value should range
		from 1 to the number of vertices that the face contains.
		\return    */
		virtual int		fnGetVertexIndexFromFace(int index,int vertexIndex)=0;
		/*! \remarks This method retrieves the index of a handle, from a face.
		A face contains 0 to N number of handles. So to retrieve a particular
		handle index, you give it the face index and the I-th handle that you
		want to inspect. So if you wanted to look at the 3 handle on face 1 you
		would call <b>GetHandleIndexFromFace(1,3)</b>. This only applies for
		patch meshes.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the face to inspect.\n\n
		<b>int vertexIndex</b>\n\n
		The I-th handle of that you want to retrieve. This value should range
		from 1 to the number of vertices*2 that the face contains. */
		virtual int		fnGetHandleIndexFromFace(int index,int vertexIndex)=0;
		/*! \remarks This method retrieves the index of a interior handle,
		from a face. A face contains 0 to N number of interior handles. So to
		retrieve a particular interior handle index, you give it the face index
		and the I-th interior handle that you want to inspect. So if you wanted
		to look at the 3 interior handle on face 1 you would call
		<b>GetInteriorIndexFromFace(1,3)</b>. This only applies for patch
		meshes.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the face to inspect.\n\n
		<b>int vertexIndex</b>\n\n
		The I-th interior handle of that you want to retrieve. This value
		should range from 1 to the number of vertices that the face contains.
		*/
		virtual int		fnGetInteriorIndexFromFace(int index,int vertexIndex)=0;
		/*! \remarks This method retrieves the index of a geometric vertex,
		from a face. This the vertex that is attached to the mesh and not the
		texture faces. A face contains 0 to N number of vertices. So to
		retrieve a particular vertex index, you give it the face index and the
		I-th vertex that you want to inspect. So if you wanted to look at the 3
		vertex on face 1 you would call <b>GetVertexGeomIndexFromFace(1,3)</b>.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the face to inspect.\n\n
		<b>int vertexIndex</b>\n\n
		The I-th vertex of that you want to retrieve. This value should range
		from 1 to the number of vertices that the face contains. */
		virtual int		fnGetVertexGIndexFromFace(int index,int vertexIndex)=0;
		/*! \remarks This method retrieves the index of a geometric handle
		from a patch. This the handle that is attached to the patch and not the
		texture faces. A face contains 0 to N number of handle. So to retrieve
		a particular handle index, you give it the face index and the I-th
		handle that you want to inspect. So if you wanted to look at the 3
		handle on face 1 you would call <b>GetHandleGeomIndexFromFace(1,3)</b>.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the face to inspect.\n\n
		<b>int vertexIndex</b>\n\n
		The I-th handle of that you want to retrieve. This value should range
		from 1 to the number of vertices*2 that the face contains. */
		virtual int		fnGetHandleGIndexFromFace(int index,int vertexIndex)=0;
		/*! \remarks This method retrieves the index of a geometric interior
		handle from a patch. This the interior handle that is attached to the
		patch and not the texture faces. A face contains 0 to N number of
		interior handle. So to retrieve a particular interior handle index, you
		give it the face index and the I-th interior handle that you want to
		inspect. So if you wanted to look at the 3 interior handle on face 1
		you would call <b>GetInteriorGeomIndexFromFace(1,3)</b>.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the face to inspect.\n\n
		<b>int vertexIndex</b>\n\n
		The I-th interior handle of that you want to retrieve. This value
		should range from 1 to the number of vertices that the face contains.
		*/
		virtual int		fnGetInteriorGIndexFromFace(int index,int vertexIndex)=0;

		/*! \remarks This method allows you to manipulate the position of
		vertex attached to a face. Basically it detaches the vertex if multiple
		faces share that vertex and then moves it to the position specified. So
		if you want to move the 3rd vertex of face 1 to .5,.5,.0 you would do
		setFaceVertex [.5 .5 .0] 1 3. If you don't want the vertex broken use
		SetVertexSPosition.
		\par Parameters:
		<b>Point3 pos</b>\n\n
		The position that you want to move a vertex to.\n\n
		<b>int fIndex</b>\n\n
		The index of the face that you wish to work on.\n\n
		<b>int ithV</b>\n\n
		The ith vertex of the face that you want to change\n\n
		<b>BOOL sel</b>\n\n
		Whether or not to select the vertex after it is recreated */
		virtual void	fnAddPoint(Point3 pos, int fIndex,int ithV, BOOL sel)=0;
		/*! \remarks This method is identical to SetFaceVertex except works on
		patch handles.
		\par Parameters:
		<b>Point3 pos</b>\n\n
		The position that you want to move a vertex to.\n\n
		<b>int fIndex</b>\n\n
		The index of the face that you wish to work on.\n\n
		<b>int ithV</b>\n\n
		The ith vertex of the face that you want to change\n\n
		<b>BOOL sel</b>\n\n
		Whether or not to select the vertex after it is recreated */
		virtual void	fnAddHandle(Point3 pos, int fIndex,int ithV, BOOL sel)=0;
		/*! \remarks This method is identical to SetFaceVertex except works on
		patch interior handles.
		\par Parameters:
		<b>Point3 pos</b>\n\n
		The position that you want to move a vertex to.\n\n
		<b>int fIndex</b>\n\n
		The index of the face that you wish to work on.\n\n
		<b>int ithV</b>\n\n
		The ith vertex of the face that you want to change\n\n
		<b>BOOL sel</b>\n\n
		Whether or not to select the vertex after it is recreated */
		virtual void	fnAddInterior(Point3 pos, int fIndex,int ithV, BOOL sel)=0;

		/*! \remarks This method allows you to set the index of the ith vertex
		of a face.
		\par Parameters:
		<b>int fIndex</b>\n\n
		The index of the face that you wish to work on.\n\n
		<b>int ithV</b>\n\n
		The ith vertex of the face that you want to manipulate.\n\n
		<b>int vIndex</b>\n\n
		The index into the vertex list that you want to set to */
		virtual void	fnSetFaceVertexIndex(int fIndex,int ithV, int vIndex)=0;
		/*! \remarks This method is identical to setFaceVertexIndex but works
		on handles for patches.
		\par Parameters:
		<b>int fIndex</b>\n\n
		The index of the face that you wish to work on.\n\n
		<b>int ithV</b>\n\n
		The ith vertex of the face that you want to manipulate.\n\n
		<b>int vIndex</b>\n\n
		The index into the vertex list that you want to set to */
		virtual void	fnSetFaceHandleIndex(int fIndex,int ithV, int vIndex)=0;
		/*! \remarks This method is identical to setFaceVertexIndex but works
		on interior handles for patches.
		\par Parameters:
		<b>int fIndex</b>\n\n
		The index of the face that you wish to work on.\n\n
		<b>int ithV</b>\n\n
		The ith vertex of the face that you want to manipulate.\n\n
		<b>int vIndex</b>\n\n
		The index into the vertex list that you want to set to\n\n
		  */
		virtual void	fnSetFaceInteriorIndex(int fIndex,int ithV, int vIndex)=0;

		/*! \remarks This method forces the viewport and dialog to update. */
		virtual void	fnUpdateViews()=0;

		/*! \remarks This method looks at the current face selection in the
		stack, and copies it to the unwrap face selection. The reason this is
		useful is that if some one creates a new selection modifier Unwrap can
		use it. An example would be if you applied to Unwrap to a whole
		editable mesh , then you went back into the editable mesh, selected
		some faces by smoothing group, then turned off the face subobject
		selection. If you went back to Unwrap you could get this selection by
		calling getFaceSelectionFromStack. */
		virtual void	fnGetFaceSelFromStack()=0;


	};

/*!
\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
The new class allows for Normal, Flatten, and Unfold mapping. You can bring
them up through a dialog or through a script command. All these tools basically
work the same. They are either applied to the current selected faces or the
whole object if no faces are selected.\n\n
As this class has been fully developed with the new Function-Published System
(FPS), all of its methods have a one-to-one correspondence with MaxScript
commands. For the sake of brevity, references to MaxScript commands mean that
there is an attendant C++ method in iunwrap.h that is prepended by lowercase
<b>fn</b>. Examples of this are as follows ( MaxScript and its attendant C++
method):\n\n
copy <b>fpCopy()</b>;\n\n
normalMapNoParams <b>fnNormalMapNoParams().</b>\n\n
There are three distinct modes of unmapping:\n\n
<b>Normal Mapping</b>\n\n
Normal Mapping is mapping based solely are the normals provided. This is
identically to box mapping except you customize what normals you want to
project on. There are three methods to apply a "normalMap" which applies a
normal map based on the parameters passed in (see the methods below) ,
"normalMapNoParams" which applies a normal map using the default setting, and
"normalMapDialog" which brings up a dialog that lets you set the setting. Right
now there are 6 types of default mapping Back/Front, Left/Right, Top/Bottom,
Box Mapping No Top, Box Mapping, and Diamond mapping. Just apply the mapping to
a teapot to see the differences they are pretty obvious once you see the
effect. Parameters in the Normal Mapping dialog are:\n\n
  <ul> <li> Mapping Type which are the 6 mapping types listed above. </li>
<li> Spacing which determines how much space there is between each cluster.
</li> <li> Normalize Cluster will normalize the cluster from 0 to 1 </li> <li>
Rotate Clusters will rotate the clusters so they take the least amount of area.
</li> <li> Align By Width will sort the clusters by there width otherwise it
will use their heights. </li> </ul>  \n\n
Hitting the Save As Default will take the current setting and set them as
defaults for the next time you bring up the dialog and when you use the script
command "<b>normalMapNoParams</b>".\n\n
<b>Flatten Mapping</b>\n\n
Flatten Mapping is similar to normal mapping, except it uses an angle threshold
to define the clusters and the clusters will always be contiguous faces. This
type of mapping will generate mapping that does not overlap so it is useful for
baking textures and lighting. Just like normal mapping this comes in 3 flavors
"flattenMap" which applies a normal map based on the parameters passed in (see
the methods below) , "flattenMapNoParams" which applies a map using the default
setting, and "flattenMapDialog" which brings up a dialog that lets you set the
setting. The parameters for the Flatten Map Dialog are:\n\n
Face Angle Threshold - when building contiguous faces this the angle used to
determine whether a face is part of that cluster. The larger this angle the
larger the cluster will be, but you will get more distortion since the texture
faces area will start deviating from there geometric face area.   <ul>
<li> Spacing which determines how much space there is between each cluster.
</li> <li> Normalize Cluster will normalize the cluster from 0 to 1 </li> <li>
Rotate Clusters will rotate the clusters so they take the least amount of area.
</li> <li> Fill Holes will fill clusters with holes. It places smaller cluster
in the gaps of larger clusters. </li> </ul>  \n\n
<b>Unfold Mapping</b>\n\n
Unfold Mapping where as the Normal and Flatten Mapping basically use a lot of
planar mapping to get their results, this is an actual unfolding algorithm. It
guarantees that all texture faces will have the exact same proportions as their
geometric equivalents, but you may get faces that overlap. This type of mapping
is only good on meshes that are very regular things like cylinders etc. This
has the same type of function calls as the above "unfoldMap" which applies an
unfold map based on the parameters passed in (see the methods below) ,
"unfoldMapNoParams" which applies a map using the default setting, and
"unfoldMapDialog" which brings up a dialog that lets you set the setting. The
parameters for the Unfold Map Dialog are: <ul> <li> Unfold Type which consists
of Walk to closest face and Walk to farthest face. This determines the order of
which face gets unfolded. Under almost all conditions you want to walk to the
closest face. </li> <li> Normalize Cluster will normalize the cluster from 0 to
1 </li> </ul>  \n\n
Copy/Paste allows you to copy a texture face/faces from one part or mesh to
another or to a new mesh. These functions are extremely topology dependant so
if you copy faces onto faces that have a different topology or face order you
will get unpredictable results. There is a "copy" script which takes the
current selected faces and puts them in the copy buffer. There is the "Paste"
and "PasteInstance" commands which do the pasting to current selected faces.
"Paste" take one parameter called rotate which determines if every time you
paste to the same face whether the tvs are reoriented. For instance if you
paste a quad onto a another quad there are actually 4 possible ways you can
paste it onto it. If rotate is on, every time you paste it will go onto the
next variation. "Paste Instance" forces the faces that are being pastes to use
the vertices that the copy buffer use (NOTE you cannot paste instance across
objects). This similar to doing a regular paste and then selecting all the
overlapping vertices and doing a weld. There is no rotate option with this
method. As noted above Pasting faces that have a different topology or face
order will result in unpredictable results.\n\n
Stitching allows you find all the texture vertices that are assigned to the
same geometric vertex and bring them all to the same spot and weld them
together. This allows you take faces that geometrically contiguous, but not
texture face contiguous and line them up. There is a stitchVerts command which
takes parameters, stitchVertsNoParams which uses the current defaults, and a
stitchVertsDialog which brings up a dialog to apply a stitch and set defaults.
The Stitch Tool params are:\n\n
  <ul> <li> Align Cluster which if checked and the edges to be stitched
are on separate clusters it will try to align the clusters and then stitch the
vertices. </li> <li> Bias determines which direction the vertices will move (to
or from the source). At a Bias of 0.0 the vertices will move to the source and
1.0 they will move to the target. </li> </ul>  \n\n
Note stitching will respect the soft selection.\n\n

\par MaxScript Commands / Methods:
Note: the following are MaxScript commands, however, referring to the above
qualifier, their C++ method name is the same as in MaxScript but prepended with
fn.\n\n
The format for input/output parameters is as follows:\n\n
\<output type\> \<name of function\> \<input type params 1...n\>\n\n
<b>copy</b> - this takes the selected faces and places them in the copy
buffer\n\n
<b>paste</b>\n\n
rotate TYPE_BOOL if this on, every time you paste to the same selection it will
try a different variation\n\n
This paste the current copy buffer onto the current face selection.\n\n
<b>pasteInstance</b> this forces the faces that are being pastes to use the
vertices that the copy buffer use (NOTE you cannot paste instance across
objects). This similar to doing a regular paste and then selecting all the
overlapping vertices and doing a weld. There is no rotate option with this
method.\n\n
<b>SetDebugLevel</b>\n\n
level TYPE_INT level of debug info 0 means no debug info, the higher the value
the more spam you will see in your script window\n\n
Debugging tool so I can control the amount of debug info that goes to the
listener and script window.\n\n
<b>TYPE_BOOL getTileMap</b> - returns whether the background is tiled.\n\n
<b>setTileMap</b>\n\n
tile TYPE_BOOL - whether or not to tile the background\n\n
This allows you set the tile state of the background\n\n
<b>TYPE_INT getTileMapLimit</b> - returns the max number of tiles to use in a
direction\n\n
<b>setTileMapLimit</b>\n\n
limit TYPE_INT - the number of tile to limit in a direction\n\n
This allows you to set the tile limit\n\n
<b>TYPE_FLOAT getTileMapBrightness</b> returns the brightness of the tiles.\n\n
<b>setTileMapBrightness</b> brightness\n\n
TYPE_FLOAT - the brightness of the tiled maps\n\n
This allows you to set the brightness of the tiled maps\n\n
New Maxscript funtions\n\n
<b>TYPE_BOOL getShowMap</b> returns the state of the show map button\n\n
<b>TYPE_VOID setShowMap</b>\n\n
showMap TYPE_BOOL the state you want to set the show map button \n\n
Lets you set the state of the show map button \n\n
<b>TYPE_BOOL getLimitSoftSel</b> returns whether the soft selection limit is
on/off\n\n
<b>SetLimitSoftSel</b>\n\n
limit TYPE_BOOL state the to set the soft selection limit\n\n
Allows you to set the soft selection limit state\n\n
<b>TYPE_INT getLimitSoftSelRange</b> returns the edge limit for the for the
soft selection\n\n
<b>setLimitSoftSelRange</b>\n\n
range TYPE_INT this is how far out in edges that soft selection will expand
to\n\n
This lets you set the edge limit range for soft selection\n\n
<b>TYPE_FLOAT getVertexWeight</b>\n\n
 index TYPE_INT the index of the vertex you want to inspect\n\n
 returns the soft selection weight of a particular vertex\n\n
<b>setVertexWeight</b>\n\n
index TYPE_INT the index of the vertex you want to change\n\n
weight TYPE_FLOAT the soft selection weight you want to set the vertex to\n\n
This lets you set the soft selection weight of a particular vertex. Note once
you set the weight of a vertex, it is tagged as being modified and will not
change value unless you unmodifiy or call setVertexWeight on it again.\n\n
<b>TYPE_BOOL isWeightModified</b>\n\n
index TYPE_INT the index of the vertex you want to inspect\n\n
 This returns whether a vertex is modified or not.\n\n
<b>modifyWeight</b>\n\n
 index TYPE_INT the index of the vertex you want to change\n\n
modify TYPE_BOOL the modified state of the vertex\n\n
This lets you set the modified state of vertex. If a vertex is modified it
ignores regular UI soft selection and the vertex soft selection weight is
locked to it current state and can only be changed by the setVertexWeight
method.\n\n
<b>TYPE_BOOL getGeomSelectElementMode</b> returns whether you are in element
mode for face\n\n
selection in the viewport.\n\n
<b>setGeomSelectElementMode</b>\n\n
mode TYPE_BOOL - the state that you want to put viewport selection in\n\n
Lets you set the viewport element mode.\n\n
<b>TYPE_BOOL getGeomPlanarThresholdMode</b> return the whether you are in
planar select mode.\n\n
<b>setGeomPlanarThresholdMode</b>\n\n
mode TYPE_BOOL the state that you want to put planar selection mode\n\n
Lets set the planar selection mode.\n\n
<b>TYPE_FLOAT getGeomPlanarThreshold</b> return the planar selection angle
threshold\n\n
<b>setGeomPlanarThreshold</b>\n\n
angle TYPE_FLOAT the angle threshold\n\n
Lets you set the angle threshold for the planar selection mode.\n\n
<b>TYPE_INT getWindowX</b> returns the current X position of the Unwrap Edit
window\n\n
<b>TYPE_INT getWindowY</b> returns the current Y position of the Unwrap Edit
window\n\n
<b>TYPE_INT getWindowW</b> returns the current width of the Unwrap Edit
window\n\n
<b>TYPE_INT getWindowH</b> returns the current height of the Unwrap Edit
window\n\n
<b>TYPE_BOOL getIgnoreBackFaceCull</b> returns the state of the Ignore Back
Face mode\n\n
<b>setIgnoreBackFaceCull</b>\n\n
 ignoreBackFaceCull TYPE_BOOL state of the Ignore Back Faces mode\n\n
Lets you set the Ignore Back Face mode\n\n
<b>TYPE_BOOL getOldSelMethod</b> returns whether the system is in the old
selection mode. Where drag selection always uses back faces and single pick
mode ignore back faces.\n\n
<b>SetOldSelMethod</b>\n\n
oldSelMethod TYPE_BOOL the state of the old selection method\n\n
This lets you set the system back to the old selection method. Where drag
selection always uses back faces and single pick mode ignore back faces.\n\n
This will override the Ignore Back Faces mode.\n\n
<b>SelectByMatID</b>\n\n
matID TYPE_INT the matID of the face that you want to select\n\n
This lets you select faces by material ids.\n\n
<b>selectBySG</b>\n\n
sg TYPE_INT the smoothing group that you want to select\n\n
This lets you select faces by smoothing group\n\n
<b>TYPE_VOID expandGeomFaceSelection</b> - expands your current viewport face
selection\n\n
<b>TYPE_VOID contractGeomFaceSelection</b>- contracts your current viewport
face selection\n\n
<b>TYPE_BOOL getAlwaysEdit</b> - This returns whether the always edit mode is
on. This mode will always bring up the edit dialog when the Unwrap rollup
window is displayed.\n\n
<b>TYPE_VOID setAlwaysEdit</b>\n\n
always TYPE_BOOL the state that you want to set the always edit mode to.\n\n
This lets you set the always edit mode. This mode will always bring up the edit
dialog when the Unwrap rollup window is displayed.\n\n
<b>TYPE_BOOL getShowVertexConnections</b> this returns whether vertex
connection indices are displayed. Vertex Connections are TV vertices that share
the same geometric vertices\n\n
<b>TYPE_VOID setShowVertexConnections</b>\n\n
show TYPE_BOOL whether to display the vertex connection data.\n\n
This lets you toggle the vertex connection data.\n\n
<b>TYPE_BOOL getFilterSelected</b> this returns the state of the Filter
Selected Faces button\n\n
<b>TYPE_VOID setFilterSelected</b>\n\n
filter TYPE_BOOL the filter state\n\n
This lets you set the Filter Selected Faces button\n\n
<b>TYPE_BOOL getSnap</b> this returns the snap state.\n\n
<b>TYPE_VOID setSnap</b>\n\n
snap TYPE_BOOL the snap state\n\n
This lets you set the snap state.\n\n
<b>TYPE_BOOL getLock</b> this returns the lock selection state\n\n
<b>TYPE_VOID setLock</b>\n\n
lock TYPE_BOOL state of the lock selection.\n\n
This lets you set the state of the lock selection\n\n
<b>TYPE_VOID pack</b>\n\n
method TYPE_INT - 0 is a linear packing algorithm fast but not that efficient,
1 is a recursive algorithm slower but more efficient.\n\n
spacing TYPE_FLOAT - the gap between cluster in percentage of the edge distance
of the square\n\n
normalize TYPE_BOOL - whether the clusters will be fit to 0 to 1 space.\n\n
rotate TYPE_BOOL - whether a cluster will be rotated so it takes up less
space.\n\n
fillholes TYPE_BOOL - whether smaller clusters will be put in the holes of the
larger cluster.\n\n
This lets you pack the texture vertex elements so that they fit within a square
space.\n\n
<b>TYPE_VOID packNoParams</b> - this packs the clusters using the default
parameters.\n\n
<b>TYPE_VOID packDialog</b> - this brings up a dialog that lets the user set
the parameters and then packs the clusters.\n\n
<b>TYPE_INT getTVSubObjectMode</b> sets the current texture subobject mode 1
vertices, 2 edges, 3 faces.\n\n
<b>TYPE_VOID setTVSubObjectMode</b>\n\n
mode TYPE_INT - the subobject mode 1 vertices, 2 edges, 3 faces.\n\n
Lets you set the tv subobject mode.\n\n
<b>TYPE_BITARRAY getSelectedFaces</b> returns the selected face list\n\n
<b>TYPE_VOID selectFaces</b>\n\n
Selection TYPE_BITARRAY selection that you want to make the face selection\n\n
This lets you set the face selection\n\n
<b>TYPE_BOOL IsFaceSelected</b>\n\n
Index TYPE_INT the index of the face you want to check\n\n
This lets you check to see if a face is selected.\n\n
<b>TYPE_INT getFillMode</b> - returns the fill mode type for face selections.
The fill modes are as follows.\n\n
<b>FILL_MODE_OFF   1</b>\n\n
<b>FILL_MODE_SOLID   2</b>\n\n
<b>FILL_MODE_BDIAGONAL  3</b>\n\n
<b>FILL_MODE_CROSS  4</b>\n\n
<b>FILL_MODE_DIAGCROSS  5</b>\n\n
<b>FILL_MODE_FDIAGONAL  6</b>\n\n
<b>FILL_MODE_HORIZONAL  7</b>\n\n
<b>FILL_MODE_VERTICAL  8</b>\n\n
<b>TYPE_VOID setFillMode</b>\n\n
mode TYPE_INT - the fill mode that you want to set\n\n
This lets you set the fill mode for selected faces.\n\n
<b>MoveSelected, RotateSelected, RotateSelectedCenter, ScaleSelectedCenter</b>,
and <b>ScaleSelected</b> are identical to there vertex counter parts but are
applied to the current selection.\n\n
<b>TYPE_VOID MoveSelected</b>\n\n
Offset TYPE_POINT3\n\n
<b>TYPE_VOID RotateSelectedCenter</b>\n\n
Angle TYPE_FLOAT\n\n
<b>TYPE_VOID RotateSelected</b>\n\n
Angle TYPE_FLOAT\n\n
Axis TYPE_POINT3\n\n
<b>TYPE_VOID ScaleSelectedCenter</b>\n\n
Scale TYPE_FLOAT\n\n
Dir TYPE_INT\n\n
<b>TYPE_VOID ScaleSelected</b>\n\n
Scale TYPE_FLOAT\n\n
Dir TYPE_INT\n\n
Axis TYPE_POINT3\n\n
<b>TYPE_BITARRAY getSelectedEdges</b> returns the selected edge list\n\n
<b>TYPE_VOID selectEdges</b>\n\n
Selection TYPE_BITARRAY selection that you want to make the edge selection\n\n
This lets you set the edge selection\n\n
<b>TYPE_BOOL IsEdgeSelected</b>\n\n
Index TYPE_INT the index of the edge you want to check\n\n
This lets you check to see if a face is selected.\n\n
<b>TYPE_BOOL getDisplayOpenEdges</b> returns whethe open edges will display\n\n
<b>TYPE_VOID setDisplayOpenEdges</b>\n\n
displayOpenEdges TYPE_BOOL the state of the open edge display\n\n
This lets you set the open edge display\n\n
<b>TYPE_POINT3 getOpenEdgeColor</b> returns the color used for the open
edges\n\n
<b>TYPE_VOID setOpenEdgeColor</b>\n\n
color TYPE_POINT3 the color to be used for open edges\n\n
This lets you set the open edge color\n\n
<b>TYPE_BOOL getUVEdgeMode</b> returns whether you are in the UV Edge Selection
mode. This mode will try to automatically select all the U or V edges when you
select an edge. Since this is based on edges, the regular tri mesh may produce
incorrect results since the hidden edges are not taken into account.\n\n
<b>TYPE_VOID setUVEdgeMode</b>\n\n
uvEdgeMode TYPE_BOOL the state of the you want to set the UV Edge mode\n\n
This lets you set the UV Edge mode\n\n
<b>TYPE_VOID uvEdgeSelect</b> - this is a command that will take your current
edge selection and try to expand out along the U and V directions. Works best
when you only have one edge selected.\n\n
<b>TYPE_BOOL getOpenEdgeMode</b> returns whether you are in the Open Edge
Selection mode. This mode will try to automatically select all the opens edges
when you select an open edge.\n\n
<b>TYPE_VOID setOpenEdgeMode</b>\n\n
uvOpenMode TYPE_BOOL the state of the you want to set the Open Edge mode\n\n
This lets you set the Open Edge mode\n\n
<b>TYPE_VOID openEdgeSelect</b> - this is a command that will take your current
selection and try to expand all the open edges in it.\n\n
<b>TYPE_VOID vertToEdgeSelect</b> - this command takes your vertex selection
and converts it to the edge selection.\n\n
<b>TYPE_VOID vertToFaceSelect</b> - this command takes your vertex selection
and converts it to the face selection.\n\n
<b>TYPE_VOID edgeToVertSelect</b> - this command takes your edge selection and
converts it to the vertex selection.\n\n
<b>TYPE_VOID edgeToFaceSelect</b> - this command takes your edge selection and
converts it to the face selection.\n\n
<b>TYPE_VOID faceToVertSelect</b> - this command takes your face selection and
converts it to the vertex selection.\n\n
<b>TYPE_VOID faceToEdgeSelect</b> - this command takes your face selection and
converts it to the edge selection.\n\n
<b>TYPE_BOOL getDisplayHiddenEdges</b> return whether hidden edges of a tri
mesh are displayed.\n\n
<b>TYPE_VOID setDisplayHiddenEdges</b>\n\n
displayHiddenEdges TYPE_BOOL - the state that you want to the hidden display to
be.\n\n
This lets you turn on/off whether tri mesh hidden edges are displayed.\n\n
<b>TYPE_POINT3 getHandleColor</b> - returns the color that will be used to
display patch handles.\n\n
<b>TYPE_VOID setHandleColor</b>\n\n
color TYPE_POINT3 - the color that you want to set patch handles to\n\n
This lets you set the color that will be used to display patch handles.\n\n
<b>TYPE_BOOL getFreeFormMode</b> this toggle the free form mode on and off.
This mode is similar to the Photoshops free form mode. You select any where
inside the bounding rectangle to move the selection. You select the corners to
scale the selection, and you you select the edge centers to rotate the
selection. You can also move the center cross which is your rotation pivot
point.\n\n
<b>TYPE_VOID setFreeFormMode</b>\n\n
freeFormMode TYPE_BOOL the state of the free form mode.\n\n
Lets you turn on/off the free from mode.\n\n
<b>TYPE_POINT3 getFreeFormColor</b> returns the color of the free form
gizmo.\n\n
<b>TYPE_VOID setFreeFormColor</b>\n\n
color TYPE_POINT3 the color that you want the gizmo to be\n\n
This lets you set the color of the Free Form Gizmo\n\n
<b>TYPE_VOID ScaleSelectedXY</b>\n\n
ScaleX TYPE_FLOAT the x scale factor\n\n
ScaleY TYPE_FLOAT the y scale factor\n\n
Axis TYPE_POINT3 the axis to scale around\n\n
This lets you nu scale the current selection around an axis\n\n
<b>TYPE_VOID SnapPivot</b>\n\n
Pos TYPE_INT - the pivot position where\n\n
1 is the center\n\n
2 is the lower left of the selection\n\n
3 is the lower center of the selection\n\n
4 is the lower right of the selection\n\n
5 is the right center of the selection\n\n
6 is the upper right of the selection\n\n
7 is the upper center of the selection\n\n
8 is the upper left of the selection\n\n
9 is the left center of the selection\n\n
This lets you quickly snap the free form gizmo pivot the bounding
rectangle.\n\n
<b>TYPE_POINT3 getPivotOffset</b> - returns the pivot offset of the free form
gizmo. This is an offset from the center of the selection.\n\n
<b>TYPE_VOID setPivotOffset</b>\n\n
offset TYPE_POINT3 the offset of the free form gizmo pivot\n\n
This lets you set the offset of the free form gizmo pivot.\n\n
<b>TYPE_POINT3 fnGetSelCenter</b> this returns the selection center so you can
compute the pivot offset from a world uv position.\n\n
<b>TYPE_BOOL getPolygonMode</b>\n\n
This returns whether the polygon mode for sub object face mode is on. Polygon
Mode will just select across triangles across hidden edges of a triangle.\n\n
<b>TYPE_VOID setPolygonMode</b>\n\n
mode TYPE_BOOL the state you want to set the Polygon Mode\n\n
This lets you set the state the of the Polygon Mode\n\n
<b>TYPE_VOID PolygonSelect</b>\n\n
This is command that will take your current selection and expand it to include
all polygons.\n\n
<b>TYPE_VOID sketch</b>\n\n
IndexList TYPE_INT_TAB the indices of the points you want to move.\n\n
PositionList TYPE_POINT3_TAB the list of a points you want to align your
vertices to.\n\n
This lets you align texture vertices to a series of points\n\n
<b>TYPE_VOID sketchNoParams</b>\n\n
This puts you in sketch mode using the default parameters.\n\n
<b>TYPE_VOID sketchDialog</b>\n\n
This brings up the sketch options dialog.\n\n
<b>TYPE_VOID sketchReverse</b>\n\n
This will reverse the order of the select vertices that are being used for
sketch  when you use the Use Current Selection option.\n\n
<b>TYPE_INT getHitSize</b>\n\n
This returns the hit size whne you do a single click in pixels.\n\n
<b>TYPE_VOID SetHitSize</b>\n\n
size TYPE_INT the size you want to set the hit size to\n\n
This lets you set the hit size of a single click.\n\n
<b>TYPE_BOOL getResetPivotOnSelection</b>\n\n
This will return whether the Transform Gizmo will reset when the selection is
changed.\n\n
<b>TYPE_VOID SetResetPivotOnSelection</b>\n\n
reset TYPE_BOOL the state you want to set the reset pivot on selection\n\n
This lets you set the Reset the Pivot On Selection.\n\n
<b>TYPE_BOOL getAllowSelectionInsideGizmo</b>\n\n
This returns whether a user can select sub objects inside the gizmo or not. If
this is FALSE the user is in move mode when inside the gizmo unless they are
over the pivot. If this is TRUE the user is in move mode when they are inside
and over a selected sub object otherwise they are in select mode.\n\n
<b>TYPE_VOID Set AllowSelectionInsideGizmo</b>\n\n
select TYPE_BOOL - the state you want to set the Allow Selection Inside Gizmo
to be\n\n
This lets you set the Allow Selection Inside Gizmo flag\n\n
<b>TYPE_VOID SaveCurrentSettingsAsDefault</b>\n\n
This takes the current state of Unwrap UVW and save it to the plugin cfg
directory into a file called unwrapUVW.ini. The next time the user creates an
Unwrap UVW modifier this ini file will be used to set the defaults.\n\n
<b>TYPE_VOID LoadDefault</b>\n\n
This will load the unwrapUVW.ini defaults into the current Unwrap UVW
modifier.\n\n
<b>TYPE_BOOL getShowShared</b>\n\n
This returns whether shared sub objects are displayed. Shared sub objects are
texture vertices or edges that share the same geometric vertex or edge.\n\n
<b>TYPE_VOID setShowShared</b>\n\n
select TYPE_BOOL whether to display shared sub objects or not.\n\n
This lets you toggle the Show Shared flag.\n\n
<b>TYPE_POINT3 getSharedColor</b>\n\n
This returns the color that will be used to show shared sub objects.\n\n
<b>TYPE_VOID setSharedColor</b>\n\n
color TYPE_POINT3 the color to be used for shared sub objects\n\n
This lets you set the color to be used for shared sub objects.\n\n
<b>TYPE_VOID showIcon</b>\n\n
index TYPE_INT index of the icon to be display/hidden. The icons are as
follows:\n\n
 1 - Move Mode\n\n
 2 - Rotate Mode\n\n
 3 - Scale Mode\n\n
 4 - Transform Mode\n\n
 5 - Mirror Tool\n\n
 6 - Expand Selection\n\n
 7 - Contract Selection\n\n
 8 - Soft Selection Falloff\n\n
 9 - Soft Selection Space\n\n
 10 - Soft Selection Strength\n\n
 11 - Break\n\n
 12 - Target Weld\n\n
 13 - Weld Selected\n\n
 14 - Update Map\n\n
 15 - Show Map\n\n
 16 - UV/VW/UW space\n\n
 17 - Properties Dialog\n\n
 18 - Map Drop List\n\n
 19 - U Spinner\n\n
 20 - V Spinner\n\n
 21 - W Spinner\n\n
 22 - Lock Sub Object\n\n
 23 - Hide/Show\n\n
 24 - Freeze/Thaw\n\n
 25 - Filter Selected\n\n
 26 - Mat Ids\n\n
 27 - Pan Mode\n\n
 28 - Zoom Mode\n\n
 29 - Zoom Region Mode\n\n
 30 - Fit Command\n\n
 31 - Snap\n\n
show TYPE_BOOL whether to show or hide this icon\n\n
This lets you turn on/off icons in the Unwrap UVW Edit dialog\n\n
<b>TYPE_BOOL getSyncSelectionMode</b>\n\n
Returns whether the viewport and the dialog selections are synced \n\n
<b>TYPE_VOID setSyncSelectionMode</b>\n\n
sync TYPE_BOOL \n\n
Lets you set whether the viewport and the dialog selections are
synced \n\n
<b>TYPE_VOID syncTVSelection</b>\n\n
This is a commmand the synces the dialog to the viewport \n\n
<b>TYPE_VOID syncGeomSelection</b>\n\n
This is a commmand the synces the viewport to the dialog \n\n
<b>TYPE_POINT3 getBackgroundColor</b>\n\n
returns the color of the background in the dialog \n\n
<b>TYPE_VOID setBackgroundColor</b>\n\n
color TYPE_POINT3 \n\n
Lets you set the color of the background in the dialog \n\n
<b>TYPE_VOID updateMenuBar</b>\n\n
Forces the menu bar to update \n\n
<b>TYPE_BOOL getBrightnessAffectsCenterTile</b>\n\n
This returns whether the brightness value affects the center tile \n\n
<b>TYPE_VOID setBrightnessAffectsCenterTile</b>\n\n
bright TYPE_BOOL \n\n
This lets you set whether the brightness value affects the center
tile \n\n
<b>TYPE_BOOL getBlendTileToBackground</b>\n\n
This returns whether the tiled images are blended to the background color or
black \n\n
<b>TYPE_VOID setBlendTileToBackground</b>\n\n
blend TYPE_BOOL, \n\n
This lets you set whether the tiled images are blended to the background color
or black \n\n
<b>TYPE_BOOL getPaintSelectMode</b>\n\n
This returns whether you are in paint select mode \n\n
<b>TYPE_VOID setPaintSelectMode</b>\n\n
paint TYPE_BOOL, \n\n
This lets you set whether you are in paint select mode \n\n
<b>TYPE_INT getPaintSelectSize</b>\n\n
Returns the size of the paint select brush this is clamped between 1 and
15. \n\n
<b>TYPE_VOID setPaintSelectSize</b>\n\n
size TYPE_INT \n\n
This lets you set the size of the paint select brush this is clamped between 1
and 15. \n\n
<b>TYPE_VOID PaintSelectIncSize</b>\n\n
This increments the brush size by one \n\n
<b>TYPE_VOID PaintSelectDecSize</b>\n\n
This decrements the brush size by one \n\n
<b>TYPE_INT GetTickSize</b>\n\n
Returns the size of a selected vertex tick\n\n
<b>TYPE_VOID SetTickSize</b>\n\n
size TYPE_INT size of the tick\n\n
This lets you set the size of a vertex tick */
class IUnwrapMod2 : public FPMixinInterface
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP

//UNFOLD STUFF
			VFN_2(unwrap_selectpolygonsupdate, fnSelectPolygonsUpdate,TYPE_BITARRAY, TYPE_BOOL);
			VFN_3(unwrap_selectfacesbynormal,fnSelectFacesByNormal,TYPE_POINT3,TYPE_FLOAT, TYPE_BOOL);
			VFN_4(unwrap_selectclusterbynormal,fnSelectClusterByNormal,TYPE_FLOAT,TYPE_INT, TYPE_BOOL, TYPE_BOOL);

			VFN_7(unwrap_flattenmap,fnFlattenMap,TYPE_FLOAT,TYPE_POINT3_TAB,TYPE_FLOAT,TYPE_BOOL,TYPE_INT,TYPE_BOOL,TYPE_BOOL);

			VFN_6(unwrap_normalmap,fnNormalMap,TYPE_POINT3_TAB,TYPE_FLOAT,TYPE_BOOL,TYPE_INT,TYPE_BOOL,TYPE_BOOL);
			VFN_0(unwrap_normalmapnoparams,fnNormalMapNoParams);
			VFN_0(unwrap_normalmapdialog,fnNormalMapDialog);

			VFN_2(unwrap_unfoldmap,fnUnfoldSelectedPolygons,TYPE_INT,TYPE_BOOL);
			VFN_0(unwrap_unfoldmapdialog,fnUnfoldSelectedPolygonsDialog);
			VFN_0(unwrap_unfoldmapnoparams,fnUnfoldSelectedPolygonsNoParams);
			


			VFN_0(unwrap_hideselectedpolygons,fnHideSelectedPolygons);
			VFN_0(unwrap_unhideallpolygons,fnUnhideAllPolygons);

			FN_1(unwrap_getnormal,TYPE_POINT3,fnGetNormal,TYPE_INT);

			VFN_0(unwrap_setseedface,fnSetSeedFace);

			VFN_0(unwrap_showvertexconnectionlist,fnShowVertexConnectionList);
//COPYPASTE STUFF
			VFN_0(unwrap_copy,fnCopy);
			VFN_1(unwrap_paste,fnPaste,TYPE_BOOL);
			VFN_0(unwrap_pasteinstance,fnPasteInstance);

			VFN_1(unwrap_setdebuglevel,fnSetDebugLevel,TYPE_INT);
			VFN_2(unwrap_stitchverts,fnStitchVerts,TYPE_BOOL,TYPE_FLOAT);
			VFN_0(unwrap_stitchvertsnoparams,fnStitchVertsNoParams);
			VFN_0(unwrap_stitchvertsdialog,fnStitchVertsDialog);

			VFN_0(unwrap_selectelement,fnSelectElement);

			VFN_0(unwrap_flattenmapdialog,fnFlattenMapDialog);
			VFN_0(unwrap_flattenmapnoparams,fnFlattenMapNoParams);
//TILE STUFF
			FN_0(unwrap_gettilemap,TYPE_BOOL, fnGetTile);
			VFN_1(unwrap_settilemap,fnSetTile,TYPE_BOOL);

			FN_0(unwrap_gettilemaplimit,TYPE_INT, fnGetTileLimit);
			VFN_1(unwrap_settilemaplimit,fnSetTileLimit,TYPE_INT);

			FN_0(unwrap_gettilemapcontrast,TYPE_FLOAT, fnGetTileContrast);
			VFN_1(unwrap_settilemapcontrast,fnSetTileContrast,TYPE_FLOAT);


			FN_0(unwrap_getshowmap,TYPE_BOOL, fnGetShowMap);
			VFN_1(unwrap_setshowmap,fnSetShowMap,TYPE_BOOL);


//SELECTION STUFF
			FN_0(unwrap_setlimitsoftsel,TYPE_BOOL, fnGetLimitSoftSel);
			VFN_1(unwrap_getlimitsoftsel,fnSetLimitSoftSel,TYPE_BOOL);

			FN_0(unwrap_setlimitsoftselrange,TYPE_INT, fnGetLimitSoftSelRange);
			VFN_1(unwrap_getlimitsoftselrange,fnSetLimitSoftSelRange,TYPE_INT);

			FN_1(unwrap_getvertexweight,TYPE_FLOAT, fnGetVertexWeight,TYPE_INDEX);
			VFN_2(unwrap_setvertexweight,fnSetVertexWeight,TYPE_INT,TYPE_FLOAT);
		
			
			FN_1(unwrap_isweightmodified,TYPE_BOOL, fnIsWeightModified,TYPE_INT);
			VFN_2(unwrap_modifyweight,fnModifyWeight,TYPE_INT,TYPE_BOOL);

			FN_0(unwrap_getgeom_elemmode,TYPE_BOOL, fnGetGeomElemMode);
			VFN_1(unwrap_setgeom_elemmode,fnSetGeomElemMode,TYPE_BOOL);

			FN_0(unwrap_getgeom_planarmode,TYPE_BOOL, fnGetGeomPlanarMode);
			VFN_1(unwrap_setgeom_planarmode,fnSetGeomPlanarMode,TYPE_BOOL);

			FN_0(unwrap_getgeom_planarmodethreshold,TYPE_FLOAT, fnGetGeomPlanarModeThreshold);
			VFN_1(unwrap_setgeom_planarmodethreshold,fnSetGeomPlanarModeThreshold,TYPE_FLOAT);

			FN_0(unwrap_getwindowx,TYPE_INT, fnGetWindowX);
			FN_0(unwrap_getwindowy,TYPE_INT, fnGetWindowY);
			FN_0(unwrap_getwindoww,TYPE_INT, fnGetWindowW);
			FN_0(unwrap_getwindowh,TYPE_INT, fnGetWindowH);

			FN_0(unwrap_getbackfacecull,TYPE_BOOL, fnGetBackFaceCull);
			VFN_1(unwrap_setbackfacecull,fnSetBackFaceCull,TYPE_BOOL);

			FN_0(unwrap_getoldselmethod,TYPE_BOOL, fnGetOldSelMethod);
			VFN_1(unwrap_setoldselmethod,fnSetOldSelMethod,TYPE_BOOL);

			VFN_1(unwrap_selectbymatid,fnSelectByMatID,TYPE_INT);
			VFN_1(unwrap_selectbysg,fnSelectBySG,TYPE_INT);

			FN_0(unwrap_gettvelementmode,TYPE_BOOL, fnGetTVElementMode);
			VFN_1(unwrap_settvelementmode,fnSetTVElementMode,TYPE_BOOL);

			VFN_0(unwrap_geomexpandsel,fnGeomExpandFaceSel);
			VFN_0(unwrap_geomcontractsel,fnGeomContractFaceSel);

			FN_0(unwrap_getalwaysedit,TYPE_BOOL, fnGetAlwaysEdit);
			VFN_1(unwrap_setalwaysedit,fnSetAlwaysEdit,TYPE_BOOL);

			FN_0(unwrap_getshowvertexconnectionlist,TYPE_BOOL, fnGetShowConnection);
			VFN_1(unwrap_setshowvertexconnectionlist,fnSetShowConnection,TYPE_BOOL);

			FN_0(unwrap_getfilterselected,TYPE_BOOL, fnGetFilteredSelected);
			VFN_1(unwrap_setfilterselected,fnSetFilteredSelected,TYPE_BOOL);

			FN_0(unwrap_getsnap,TYPE_BOOL, fnGetSnap);
			VFN_1(unwrap_setsnap,fnSetSnap,TYPE_BOOL);

			FN_0(unwrap_getlock,TYPE_BOOL, fnGetLock);
			VFN_1(unwrap_setlock,fnSetLock,TYPE_BOOL);

			VFN_5(unwrap_pack, fnPack,TYPE_INT, TYPE_FLOAT, TYPE_BOOL,TYPE_BOOL,TYPE_BOOL);
			VFN_0(unwrap_packnoparams, fnPackNoParams);
			VFN_0(unwrap_packdialog, fnPackDialog);

			FN_0(unwrap_gettvsubobjectmode,TYPE_INT, fnGetTVSubMode);
			VFN_1(unwrap_settvsubobjectmode,fnSetTVSubMode,TYPE_INT);

			FN_0(unwrap_getselectedfaces,TYPE_BITARRAY, fnGetSelectedFaces);
			VFN_1(unwrap_selectfaces, fnSelectFaces,TYPE_BITARRAY);
			FN_1(unwrap_isfaceselected,TYPE_BOOL, fnIsFaceSelected,TYPE_INT);

			FN_0(unwrap_getfillmode,TYPE_INT, fnGetFillMode);
			VFN_1(unwrap_setfillmode,fnSetFillMode,TYPE_INT);


			VFN_1(unwrap_moveselected, fnMoveSelected,TYPE_POINT3);
			VFN_1(unwrap_rotateselectedc, fnRotateSelected,TYPE_FLOAT);
			VFN_2(unwrap_rotateselected, fnRotateSelected,TYPE_FLOAT, TYPE_POINT3);
			VFN_2(unwrap_scaleselectedc, fnScaleSelected,TYPE_FLOAT, TYPE_INT);
			VFN_3(unwrap_scaleselected, fnScaleSelected,TYPE_FLOAT, TYPE_INT,TYPE_POINT3);

			FN_0(unwrap_getselectededges,TYPE_BITARRAY, fnGetSelectedEdges);
			VFN_1(unwrap_selectedges, fnSelectEdges,TYPE_BITARRAY);
			FN_1(unwrap_isedgeselected,TYPE_BOOL, fnIsEdgeSelected,TYPE_INT);


			FN_0(unwrap_getdisplayopenedge,TYPE_BOOL, fnGetDisplayOpenEdges);
			VFN_1(unwrap_setdisplayopenedge,fnSetDisplayOpenEdges,TYPE_BOOL);
		

			FN_0(unwrap_getopenedgecolor,TYPE_POINT3, fnGetOpenEdgeColor);
			VFN_1(unwrap_setopenedgecolor, fnSetOpenEdgeColor,TYPE_POINT3);

			FN_0(unwrap_getuvedgemode,TYPE_BOOL, fnGetUVEdgeMode);
			VFN_1(unwrap_setuvedgemode,fnSetUVEdgeMode,TYPE_BOOL);

			FN_0(unwrap_getopenedgemode,TYPE_BOOL, fnGetOpenEdgeMode);
			VFN_1(unwrap_setopenedgemode,fnSetOpenEdgeMode,TYPE_BOOL);

			VFN_0(unwrap_uvedgeselect,fnUVEdgeSelect);

			VFN_0(unwrap_openedgeselect,fnOpenEdgeSelect);

			VFN_0(unwrap_selectverttoedge,fnVertToEdgeSelect);
			VFN_0(unwrap_selectverttoface,fnVertToFaceSelect);

			VFN_0(unwrap_selectedgetovert,fnEdgeToVertSelect);
			VFN_0(unwrap_selectedgetoface,fnEdgeToFaceSelect);

			VFN_0(unwrap_selectfacetovert,fnFaceToVertSelect);
			VFN_0(unwrap_selectfacetoedge,fnFaceToEdgeSelect);

			FN_0(unwrap_getdisplayhiddenedge,TYPE_BOOL, fnGetDisplayHiddenEdges);
			VFN_1(unwrap_setdisplayhiddenedge,fnSetDisplayHiddenEdges,TYPE_BOOL);


			FN_0(unwrap_gethandlecolor,TYPE_POINT3, fnGetHandleColor);
			VFN_1(unwrap_sethandlecolor, fnSetHandleColor,TYPE_POINT3);

			FN_0(unwrap_getfreeformmode,TYPE_BOOL, fnGetFreeFormMode);
			VFN_1(unwrap_setfreeformmode,fnSetFreeFormMode,TYPE_BOOL);

			FN_0(unwrap_getfreeformcolor,TYPE_POINT3, fnGetFreeFormColor);
			VFN_1(unwrap_setfreeformcolor, fnSetFreeFormColor,TYPE_POINT3);

			VFN_3(unwrap_scaleselectedxy, fnScaleSelectedXY,TYPE_FLOAT, TYPE_FLOAT,TYPE_POINT3);

			VFN_1(unwrap_snappivot, fnSnapPivot,TYPE_INT);
			FN_0(unwrap_getpivotoffset,TYPE_POINT3, fnGetPivotOffset);
			VFN_1(unwrap_setpivotoffset, fnSetPivotOffset,TYPE_POINT3);		
			FN_0(unwrap_getselcenter,TYPE_POINT3, fnGetSelCenter);

			VFN_2(unwrap_sketch, fnSketch,TYPE_INT_TAB,TYPE_POINT3_TAB);
			VFN_0(unwrap_sketchnoparams, fnSketchNoParams);
			VFN_0(unwrap_sketchdialog, fnSketchDialog);
			VFN_0(unwrap_sketchreverse, fnSketchReverse);
 
			FN_0(unwrap_gethitsize,TYPE_INT, fnGetHitSize);
			VFN_1(unwrap_sethitsize, fnSetHitSize,TYPE_INT);

			FN_0(unwrap_getresetpivotonsel,TYPE_BOOL, fnGetResetPivotOnSel);
			VFN_1(unwrap_setresetpivotonsel, fnSetResetPivotOnSel,TYPE_BOOL);

			FN_0(unwrap_getpolymode,TYPE_BOOL, fnGetPolyMode);
			VFN_1(unwrap_setpolymode, fnSetPolyMode,TYPE_BOOL);
			VFN_0(unwrap_polyselect, fnPolySelect);


			FN_0(unwrap_getselectioninsidegizmo,TYPE_BOOL, fnGetAllowSelectionInsideGizmo);
			VFN_1(unwrap_setselectioninsidegizmo, fnSetAllowSelectionInsideGizmo,TYPE_BOOL);

			VFN_0(unwrap_setasdefaults, fnSetAsDefaults);
			VFN_0(unwrap_loaddefaults, fnLoadDefaults);


			
			FN_0(unwrap_getshowshared,TYPE_BOOL, fnGetShowShared);
			VFN_1(unwrap_setshowshared, fnSetShowShared,TYPE_BOOL);
			FN_0(unwrap_getsharedcolor,TYPE_POINT3, fnGetSharedColor);
			VFN_1(unwrap_setsharedcolor, fnSetSharedColor,TYPE_POINT3);

			VFN_2(unwrap_showicon, fnShowIcon,TYPE_INT,TYPE_BOOL);


			FN_0(unwrap_getsyncselectionmode,TYPE_BOOL, fnGetSyncSelectionMode);
			VFN_1(unwrap_setsyncselectionmode, fnSetSyncSelectionMode,TYPE_BOOL);

			VFN_0(unwrap_synctvselection, fnSyncTVSelection);
			VFN_0(unwrap_syncgeomselection, fnSyncGeomSelection);

			FN_0(unwrap_getbackgroundcolor,TYPE_POINT3, fnGetBackgroundColor);
			VFN_1(unwrap_setbackgroundcolor, fnSetBackgroundColor,TYPE_POINT3);

			VFN_0(unwrap_updatemenubar, fnUpdateMenuBar);


			FN_0(unwrap_getbrightcentertile,TYPE_BOOL, fnGetBrightCenterTile);
			VFN_1(unwrap_setbrightcentertile, fnSetBrightCenterTile,TYPE_BOOL);

			FN_0(unwrap_getblendtoback,TYPE_BOOL, fnGetBlendToBack);
			VFN_1(unwrap_setblendtoback, fnSetBlendToBack,TYPE_BOOL);

			FN_0(unwrap_getpaintmode,TYPE_BOOL, fnGetPaintMode);
			VFN_1(unwrap_setpaintmode, fnSetPaintMode,TYPE_BOOL);

			FN_0(unwrap_getpaintsize,TYPE_INT, fnGetPaintSize);
			VFN_1(unwrap_setpaintsize, fnSetPaintSize,TYPE_INT);

			VFN_0(unwrap_incpaintsize, fnIncPaintSize);
			VFN_0(unwrap_decpaintsize, fnDecPaintSize);

			FN_0(unwrap_getticksize,TYPE_INT, fnGetTickSize);
			VFN_1(unwrap_setticksize, fnSetTickSize,TYPE_INT);

//NEW
			FN_0(unwrap_getgridsize,TYPE_FLOAT, fnGetGridSize);
			VFN_1(unwrap_setgridsize, fnSetGridSize,TYPE_FLOAT);

			FN_0(unwrap_getgridsnap,TYPE_BOOL, fnGetGridSnap);
			VFN_1(unwrap_setgridsnap, fnSetGridSnap,TYPE_BOOL);

			FN_0(unwrap_getgridvisible,TYPE_BOOL, fnGetGridVisible);
			VFN_1(unwrap_setgridvisible, fnSetGridVisible,TYPE_BOOL);

			FN_0(unwrap_getgridcolor,TYPE_POINT3, fnGetGridColor);
			VFN_1(unwrap_setgridcolor, fnSetGridColor,TYPE_POINT3);

			FN_0(unwrap_getgridstr,TYPE_FLOAT, fnGetGridStr);
			VFN_1(unwrap_setgridstr, fnSetGridStr,TYPE_FLOAT);


			FN_0(unwrap_getautomap,TYPE_BOOL, fnGetAutoMap);
			VFN_1(unwrap_setautomap, fnSetAutoMap,TYPE_BOOL);

//flatten defaults
			FN_0(unwrap_getflattenangle,TYPE_FLOAT, fnGetFlattenAngle);
			VFN_1(unwrap_setflattenangle, fnSetFlattenAngle,TYPE_ANGLE);

			FN_0(unwrap_getflattenspacing,TYPE_FLOAT, fnGetFlattenSpacing);
			VFN_1(unwrap_setflattenspacing, fnSetFlattenSpacing,TYPE_FLOAT);

			FN_0(unwrap_getflattennormalize,TYPE_BOOL, fnGetFlattenNormalize);
			VFN_1(unwrap_setflattennormalize, fnSetFlattenNormalize,TYPE_BOOL);

			FN_0(unwrap_getflattenrotate,TYPE_BOOL, fnGetFlattenRotate);
			VFN_1(unwrap_setflattenrotate, fnSetFlattenRotate,TYPE_BOOL);

			FN_0(unwrap_getflattenfillholes,TYPE_BOOL, fnGetFlattenFillHoles);
			VFN_1(unwrap_setflattenfillholes, fnSetFlattenFillHoles,TYPE_BOOL);

			FN_0(unwrap_getpreventflattening,TYPE_BOOL, fnGetPreventFlattening);
			VFN_1(unwrap_setpreventflattening, fnSetPreventFlattening,TYPE_BOOL);			

			FN_0(unwrap_getenablesoftselection,TYPE_BOOL, fnGetEnableSoftSelection);
			VFN_1(unwrap_setenablesoftselection, fnSetEnableSoftSelection,TYPE_BOOL);			

			FN_0(unwrap_getapplytowholeobject,TYPE_BOOL, fnGetApplyToWholeObject);
			VFN_1(unwrap_setapplytowholeobject, fnSetApplyToWholeObject,TYPE_BOOL);			

			VFN_5(unwrap_setvertexposition2, fnSetVertexPosition2,TYPE_TIMEVALUE,TYPE_INT,TYPE_POINT3,TYPE_BOOL,TYPE_BOOL);

			VFN_4(unwrap_relax, fnRelax,TYPE_INT,TYPE_FLOAT,TYPE_BOOL,TYPE_BOOL);
			VFN_2(unwrap_fitrelax, fnFit,TYPE_INT,TYPE_FLOAT);


		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 

//UNFOLD STUFF
		virtual void	fnSelectPolygonsUpdate(BitArray *sel, BOOL update)=0;
		virtual void	fnSelectFacesByNormal(Point3 Normal, float angleThreshold, BOOL update)=0;
		virtual void	fnSelectClusterByNormal(float angleThreshold, int seedIndex, BOOL relative, BOOL update)=0;

		virtual void	fnFlattenMap(float angleThreshold, Tab<Point3*> *normaList, float spacing, BOOL normalize, int layoutType, BOOL rotateClusters, BOOL alignWidth)=0;

		virtual void	fnNormalMap(Tab<Point3*> *normaList, float spacing, BOOL normalize, int layoutType, BOOL rotateClusters, BOOL alignWidth)=0;
		virtual void	fnNormalMapNoParams()=0;
		virtual void	fnNormalMapDialog()=0;

		virtual void	fnUnfoldSelectedPolygons(int unfoldMethod,BOOL normalize)=0;
		virtual void	fnUnfoldSelectedPolygonsDialog()=0;
		virtual void	fnUnfoldSelectedPolygonsNoParams()=0;

		virtual void	fnHideSelectedPolygons()=0;
		virtual void	fnUnhideAllPolygons()=0;

		virtual Point3*	fnGetNormal(int index)=0;
		virtual void	fnSetSeedFace()=0;


		virtual void	fnShowVertexConnectionList() = 0;

//COPYPASTE STUFF
		virtual void	fnCopy() = 0;
		virtual void	fnPaste(BOOL rotate) = 0;
		virtual void	fnPasteInstance() = 0;

		virtual void	fnSetDebugLevel(int level) = 0;

		virtual void	fnStitchVerts(BOOL bAlign, float fBias) = 0;
		virtual void	fnStitchVertsNoParams() = 0;
		virtual void	fnStitchVertsDialog() = 0;
		virtual void	fnSelectElement() = 0;

		virtual void	fnFlattenMapDialog() = 0;
		virtual void	fnFlattenMapNoParams() = 0;

//TILE STUFF
		virtual BOOL	fnGetTile() = 0;
		virtual void	fnSetTile(BOOL tile) = 0;

		virtual int		fnGetTileLimit() = 0;
		virtual void	fnSetTileLimit(int lmit) = 0;

		virtual float	fnGetTileContrast() = 0;
		virtual void	fnSetTileContrast(float contrast) = 0;

		virtual BOOL	fnGetShowMap() = 0;
		virtual void	fnSetShowMap(BOOL smap) = 0;


		virtual BOOL	fnGetLimitSoftSel() = 0;
		virtual void	fnSetLimitSoftSel(BOOL limit) = 0;

		virtual int		fnGetLimitSoftSelRange() = 0;
		virtual void	fnSetLimitSoftSelRange(int range) = 0;

		virtual float	fnGetVertexWeight(int index) = 0;
		virtual void	fnSetVertexWeight(int index,float weight) = 0;

		virtual BOOL	fnIsWeightModified(int index) = 0;
		virtual void	fnModifyWeight(int index, BOOL modified) = 0;

		virtual BOOL	fnGetGeomElemMode() = 0;
		virtual void	fnSetGeomElemMode(BOOL elem) = 0;

		virtual BOOL	fnGetGeomPlanarMode() = 0;
		virtual void	fnSetGeomPlanarMode(BOOL planar) = 0;

		virtual float	fnGetGeomPlanarModeThreshold() = 0;
		virtual void	fnSetGeomPlanarModeThreshold(float threshold) = 0;

		virtual int		fnGetWindowX() = 0;
		virtual int		fnGetWindowY() = 0;
		virtual int		fnGetWindowW() = 0;
		virtual int		fnGetWindowH() = 0;


		virtual BOOL	fnGetBackFaceCull() = 0;
		virtual void	fnSetBackFaceCull(BOOL backFaceCull) = 0;

		virtual BOOL	fnGetOldSelMethod() = 0;
		virtual void	fnSetOldSelMethod(BOOL oldSelMethod) = 0;

		virtual void	fnSelectByMatID(int matID) = 0;
		virtual void	fnSelectBySG(int sg) = 0;




		virtual BOOL	fnGetTVElementMode() = 0;
		virtual void	fnSetTVElementMode(BOOL mode) = 0;

		virtual void	fnGeomExpandFaceSel() = 0;
		virtual void	fnGeomContractFaceSel() = 0;

		virtual BOOL	fnGetAlwaysEdit() = 0;
		virtual void	fnSetAlwaysEdit(BOOL always) = 0;

		virtual BOOL	fnGetShowConnection() = 0;
		virtual void	fnSetShowConnection(BOOL show) = 0;


		virtual BOOL	fnGetFilteredSelected() = 0;
		virtual void	fnSetFilteredSelected(BOOL filter) = 0;

		virtual BOOL	fnGetSnap() = 0;
		virtual void	fnSetSnap(BOOL snap) = 0;

		virtual BOOL	fnGetLock() = 0;
		virtual void	fnSetLock(BOOL snap) = 0;

		virtual void	fnPack(int method,  float spacing, BOOL normalize, BOOL rotate, BOOL fillHoles) = 0;
		virtual void	fnPackNoParams() = 0;
		virtual void	fnPackDialog() = 0;

		virtual int		fnGetTVSubMode() = 0;
		virtual void	fnSetTVSubMode(int mode) = 0;

		virtual BitArray* fnGetSelectedFaces()=0;
		virtual void	fnSelectFaces(BitArray *sel)=0;
		virtual BOOL	fnIsFaceSelected(int index)=0;

		virtual int		fnGetFillMode() = 0;
		virtual void	fnSetFillMode(int mode) = 0;

		virtual void fnMoveSelected(Point3 offset)=0;
		virtual void fnRotateSelected(float angle)=0;
		virtual void fnRotateSelected(float angle, Point3 axis)=0;
		virtual void fnScaleSelected(float scale,int dir)=0;
		virtual void fnScaleSelected(float scale,int dir,Point3 axis)=0;


		virtual BitArray* fnGetSelectedEdges()=0;
		virtual void	fnSelectEdges(BitArray *sel)=0;
		virtual BOOL	fnIsEdgeSelected(int index)=0;



		virtual BOOL	fnGetDisplayOpenEdges() = 0;
		virtual void	fnSetDisplayOpenEdges(BOOL openEdgeDisplay) = 0;
		
		virtual Point3*	fnGetOpenEdgeColor()=0;
		virtual void	fnSetOpenEdgeColor(Point3 color)=0;

		virtual BOOL	fnGetUVEdgeMode() = 0;
		virtual void	fnSetUVEdgeMode(BOOL uvmode) = 0;

		virtual BOOL	fnGetOpenEdgeMode() = 0;
		virtual void	fnSetOpenEdgeMode(BOOL uvmode) = 0;

		virtual void	fnUVEdgeSelect() = 0;
		virtual void	fnOpenEdgeSelect() = 0;

		virtual void	fnVertToEdgeSelect() = 0;
		virtual void	fnVertToFaceSelect() = 0;

		virtual void	fnEdgeToVertSelect() = 0;
		virtual void	fnEdgeToFaceSelect() = 0;

		virtual void	fnFaceToVertSelect() = 0;
		virtual void	fnFaceToEdgeSelect() = 0;


		virtual BOOL	fnGetDisplayHiddenEdges() = 0;
		virtual void	fnSetDisplayHiddenEdges(BOOL hiddenEdgeDisplay) = 0;

		virtual Point3*	fnGetHandleColor()=0;
		virtual void	fnSetHandleColor(Point3 color)=0;

		virtual BOOL	fnGetFreeFormMode() = 0;
		virtual void	fnSetFreeFormMode(BOOL freeFormMode) = 0;

		virtual Point3*	fnGetFreeFormColor()=0;
		virtual void	fnSetFreeFormColor(Point3 color)=0;

		virtual void	fnScaleSelectedXY(float scaleX,float scaleY,Point3 axis)=0;

		virtual void	fnSnapPivot(int pos)=0;
		virtual Point3*	fnGetPivotOffset()=0;
		virtual void	fnSetPivotOffset(Point3 color)=0;
		virtual Point3*	fnGetSelCenter()=0;
		
		virtual void	fnSketch(Tab<int> *indexList, Tab<Point3*> *positionList)=0;
		virtual void	fnSketchNoParams()=0;
		virtual void	fnSketchDialog()=0;
		virtual void	fnSketchReverse()=0;

		virtual int		fnGetHitSize()=0;
		virtual void	fnSetHitSize(int size)=0;

		virtual BOOL	fnGetResetPivotOnSel()=0;
		virtual void	fnSetResetPivotOnSel(BOOL reset)=0;

		virtual BOOL	fnGetPolyMode()=0;
		virtual void	fnSetPolyMode(BOOL pmode)=0;
		virtual void	fnPolySelect()=0;


		virtual BOOL	fnGetAllowSelectionInsideGizmo()=0;
		virtual void	fnSetAllowSelectionInsideGizmo(BOOL select)=0;

		virtual void	fnSetAsDefaults()=0;
		virtual void	fnLoadDefaults()=0;


		virtual void	fnSetSharedColor(Point3 color)=0;
		virtual Point3*	fnGetSharedColor()=0;

		virtual BOOL	fnGetShowShared()=0;
		virtual void	fnSetShowShared(BOOL select)=0;

		virtual void	fnShowIcon(int icon, BOOL show)=0;


		virtual BOOL	fnGetSyncSelectionMode()=0;
		virtual void	fnSetSyncSelectionMode(BOOL sync)=0;

		virtual void	fnSyncTVSelection()=0;
		virtual void	fnSyncGeomSelection()=0;


		virtual Point3*	fnGetBackgroundColor()=0;
		virtual void	fnSetBackgroundColor(Point3 color)=0;

		virtual void	fnUpdateMenuBar() = 0;


		virtual BOOL	fnGetBrightCenterTile()=0;
		virtual void	fnSetBrightCenterTile(BOOL bright)=0;

		virtual BOOL	fnGetBlendToBack()=0;
		virtual void	fnSetBlendToBack(BOOL blend)=0;

		virtual BOOL	fnGetPaintMode()=0;
		virtual void	fnSetPaintMode(BOOL paint)=0;

		virtual int		fnGetPaintSize()=0;
		virtual void	fnSetPaintSize(int size)=0;

		virtual void	fnIncPaintSize()=0;
		virtual void	fnDecPaintSize()=0;


		virtual int		fnGetTickSize()=0;
		virtual void	fnSetTickSize(int size)=0;

//new


		virtual float	fnGetGridSize()=0;
		virtual void	fnSetGridSize(float size)=0;

		virtual BOOL	fnGetGridSnap()=0;
		virtual void	fnSetGridSnap(BOOL snap)=0;
		virtual BOOL	fnGetGridVisible()=0;
		virtual void	fnSetGridVisible(BOOL visible)=0;

		virtual Point3*	fnGetGridColor()=0;
		virtual void	fnSetGridColor(Point3 color)=0;

		virtual float	fnGetGridStr()=0;
		virtual void	fnSetGridStr(float size)=0;


		virtual BOOL	fnGetAutoMap()=0;
		virtual void	fnSetAutoMap(BOOL autoMap)=0;

		virtual float	fnGetFlattenAngle()=0;				//Angle is in degrees
		virtual void	fnSetFlattenAngle(float angle)=0;	//Angle is in degrees

		virtual float	fnGetFlattenSpacing()=0;
		virtual void	fnSetFlattenSpacing(float spacing)=0;

		virtual BOOL	fnGetFlattenNormalize()=0;
		virtual void	fnSetFlattenNormalize(BOOL normalize)=0;

		virtual BOOL	fnGetFlattenRotate()=0;
		virtual void	fnSetFlattenRotate(BOOL rotate)=0;

		virtual BOOL	fnGetFlattenFillHoles()=0;
		virtual void	fnSetFlattenFillHoles(BOOL fillHoles)=0;

		virtual BOOL	fnGetPreventFlattening()=0;
		virtual void	fnSetPreventFlattening(BOOL preventFlattening)=0;

		virtual BOOL	fnGetEnableSoftSelection()=0;
		virtual void	fnSetEnableSoftSelection(BOOL enable)=0;

		virtual BOOL	fnGetApplyToWholeObject() = 0;
		virtual void	fnSetApplyToWholeObject(BOOL wholeObject) = 0;

		virtual void	fnSetVertexPosition2(TimeValue t, int index, Point3 pos, BOOL hold, BOOL update) = 0;
		virtual void	fnRelax(int iteration, float str, BOOL lockEdges, BOOL matchArea) = 0;
		virtual void	fnFit(int iteration, float str) = 0;


	};

//5.1.05

/*! This class is only available in Max 6.0 and higher
	\sa Class IUnwrapMod, Class IUnwrapMod2
	\par Description:
	UVW Unwrap has an auto background option which ties the current background to the current Material Filter ID. 
	When the Material Filter ID is changed, it will automatically change the background to a map that uses that material ID. 
	The GetAutoBackground and SetAutoBackground function published methods enable and disable this feature.\n\n
	The remainder of the new methods are related to the Relax UV feature of UVW Unwrap. The relax UV dialog can be
	invoked with the Relax2Dialog method. Or, the parameters within the dialog can be set and applied directly with the
	Relax2 method. Get/SetRelaxBoundary corresponds to the "Keep Boundary Points Fixed" parameter. Get/SetRelaxSaddle corresponds 
	to the "Save Outer Corners" parameter. */
class IUnwrapMod3 : public FPMixinInterface  //interface for R6
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP

//TILE STUFF
			FN_0(unwrap_getautobackground,TYPE_BOOL, fnGetAutoBackground);
			VFN_1(unwrap_setautobackground,fnSetAutoBackground,TYPE_BOOL);

//RELAX
//5.1.06
			FN_0(unwrap_getrelaxamount,TYPE_FLOAT, fnGetRelaxAmount);
			VFN_1(unwrap_setrelaxamount, fnSetRelaxAmount,TYPE_FLOAT);			

			FN_0(unwrap_getrelaxiter,TYPE_INT, fnGetRelaxIter);
			VFN_1(unwrap_setrelaxiter, fnSetRelaxIter,TYPE_INT);			

			FN_0(unwrap_getrelaxboundary,TYPE_BOOL, fnGetRelaxBoundary);
			VFN_1(unwrap_setrelaxboundary, fnSetRelaxBoundary,TYPE_BOOL);			

			FN_0(unwrap_getrelaxsaddle,TYPE_BOOL, fnGetRelaxSaddle);
			VFN_1(unwrap_setrelaxsaddle, fnSetRelaxSaddle,TYPE_BOOL);			

			VFN_0(unwrap_relax2, fnRelax2);			
			VFN_0(unwrap_relax2dialog, fnRelax2Dialog);			

		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 

//auto background
		virtual BOOL	fnGetAutoBackground()=0;
		virtual void	fnSetAutoBackground(BOOL autoBackground)=0;

//5.1.06

		virtual float	fnGetRelaxAmount() = 0;
		virtual void	fnSetRelaxAmount(float amount) = 0;

		virtual int		fnGetRelaxIter() = 0;
		virtual void	fnSetRelaxIter(int iter) = 0;

		virtual BOOL	fnGetRelaxBoundary() = 0;
		virtual void	fnSetRelaxBoundary(BOOL boundary) = 0;

		virtual BOOL	fnGetRelaxSaddle() = 0;
		virtual void	fnSetRelaxSaddle(BOOL saddle) = 0;

		virtual void	fnRelax2()=0;
		virtual void	fnRelax2Dialog()=0;
	};


class IUnwrapMod4 : public FPMixinInterface  //interface for R7
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP

//new display edges in viewport
			FN_0(unwrap_getthickopenedges,TYPE_BOOL, fnGetThickOpenEdges);
			VFN_1(unwrap_setthickopenedges,fnSetThickOpenEdges,TYPE_BOOL);

			FN_0(unwrap_getviewportopenedges,TYPE_BOOL, fnGetViewportOpenEdges);
			VFN_1(unwrap_setviewportopenedges,fnSetViewportOpenEdges,TYPE_BOOL);

			VFN_0(unwrap_selectinvertedfaces,fnSelectInvertedFaces);

			FN_0(unwrap_getrelativetypein,TYPE_BOOL,fnGetRelativeTypeInMode);
			VFN_1(unwrap_setrelativetypein,fnSetRelativeTypeInMode,TYPE_BOOL);

			VFN_3(unwrap_stitchverts2,fnStitchVerts2,TYPE_BOOL,TYPE_FLOAT,TYPE_BOOL);
			
			VFN_1(unwrap_addmap,fnAddMap,TYPE_TEXMAP);


			VFN_6(unwrap_flattenmapbymatid,fnFlattenMapByMatID,TYPE_FLOAT,TYPE_FLOAT,TYPE_BOOL,TYPE_INT,TYPE_BOOL,TYPE_BOOL);


			VFN_7(unwrap_getarea,fnGetArea, TYPE_BITARRAY, 
							TYPE_FLOAT_BR,TYPE_FLOAT_BR,
							TYPE_FLOAT_BR,TYPE_FLOAT_BR,	
							TYPE_FLOAT_BR,TYPE_FLOAT_BR);

				

			FN_0(unwrap_getrotationsrespectaspect,TYPE_BOOL, fnGetRotationsRespectAspect);
			VFN_1(unwrap_setrotationsrespectaspect,fnSetRotationsRespectAspect,TYPE_BOOL);

			VFN_1(unwrap_setmax5flatten,fnSetMax5Flatten,TYPE_BOOL);


		END_FUNCTION_MAP

		FPInterfaceDesc* GetDesc();    // <-- must implement 


		/*! \remarks Returns whether we are in thin and thick viewport open edge display.
		\return true if we are in thick edge display mode */
		virtual BOOL	fnGetThickOpenEdges()=0;
		/*! \remarks let you toggle between thin and thick viewport open edge display.
		\param thick - if true, display thick edges */
		virtual void	fnSetThickOpenEdges(BOOL thick)=0;

		virtual BOOL	fnGetViewportOpenEdges()=0;
		virtual void	fnSetViewportOpenEdges(BOOL thick)=0;
		/*! \remarks Select the inverted faces */
		virtual void	fnSelectInvertedFaces()=0;

		/*! \remarks The getter for the Relative toggle for the typeins
		\return the state of the relateive toggle for the typeins */
		virtual BOOL	fnGetRelativeTypeInMode() = 0;
		/*! \remarks The setter for the Relative toggle for the typeins
		\param absolute - If true, disable the relative toggle for the typeins */
		virtual void	fnSetRelativeTypeInMode(BOOL absolute) = 0;

		/*! \remarks A stitch function which includes the option to match the scale of the clusters */
		virtual void	fnStitchVerts2(BOOL bAlign, float fBias, BOOL bScale) = 0;

		/*! \remarks Gives the user access to add maps to the map drop down through script */
		virtual void	fnAddMap(Texmap *map) = 0;

		/*! \remarks A flatten which separates and flattens an object based on its matID. */
		virtual void	fnFlattenMapByMatID(float angleThreshold, float spacing, BOOL normalize, int layoutType, BOOL rotateClusters, BOOL alignWidth)=0;
		/*! \remarks returns the area of a selection of faces */
		virtual void	fnGetArea(BitArray *faceSelection, 
								  float &x, float &y,
								  float &width, float &height,
								  float &uvArea, float &geomArea) = 0;

		/*! \remarks The getter for the Respect Aspect ratio option.
		This option determines how things get rotated on non-square bitmaps */
		virtual BOOL	fnGetRotationsRespectAspect()=0;
		/*! \remarks The setter for the Respect Aspect ratio option.
		This options determines how things get rotated on non-square bitmaps */
		virtual void	fnSetRotationsRespectAspect(BOOL respect)=0;

		/*! \remarks Forces the flatten algorithm to act like the Max5 version. 
		\param like5 - if true, the flatten algorithm will act like the Max5 version */
		virtual void	fnSetMax5Flatten(BOOL like5)=0;



	};



