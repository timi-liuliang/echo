/*	
 *		primitives.h - primitive function definitions for MAXScript
 *
 *		A Primitive is an object that is global, and doesn't necassarily
 *		belong it a maxscript type. 
 *		In this file, there are (with a few exceptions) declarations and 
 *		instantiations of two types of Primitives:
 *
 *		1. Visible Primitives - Usually global functions like 'pickObject'. These
 *			are created using the def_visible_primitive macro. 
 *		2. Visible Structs - These are maxscript global structs, like 'dotnet', 
 *			or 'callbacks'. These are different from and predate function published interfaces.
 *			These are created using the def_struct_primitive macro.
 *		
 *			Copyright © John Wainwright 1996
 *
 */

#include "..\..\buildver.h"
#pragma warning(push)
#pragma warning(disable:4100)

/* scripter-visible primitives */

	/* user input */

	def_visible_primitive (					pick_object,				"pickObject");  
	def_visible_primitive (					pick_point,					"pickPoint"); 
	def_visible_primitive (					get_kb_value,				"getKBValue"); 
	def_visible_primitive (					get_kb_point,				"getKBPoint"); 
	def_visible_primitive (					get_kb_line,				"getKBLine"); 
	def_visible_primitive (					get_kb_char,				"getKBChar"); 
	def_visible_primitive (					redraw_views,				"redrawViews"); 
	def_visible_primitive (					pick_offset_distance,		"pickOffsetDistance"); 
	def_visible_primitive (					clear_listener,				"clearListener"); 
	def_visible_primitive (					get_listener_sel,			"getListenerSel");
	def_visible_primitive (					set_listener_sel,			"setListenerSel");
	def_visible_primitive (					get_listener_sel_text,		"getListenerSelText");
	def_visible_primitive (					set_listener_sel_text,		"setListenerSelText");

	/* command-line debugger */

	def_visible_primitive_debug_ok (		dbg_break,					"break");
	def_visible_primitive_debug_ok (		show_stack,					"stack");
	def_visible_primitive_debug_ok (		show_locals,				"locals");
	def_visible_primitive_debug_ok (		dbg_go,						"go");
	def_visible_primitive_debug_ok (		show_threads,				"threads");
	def_visible_primitive_debug_ok (		heapchk,					"heapCheck");
	def_visible_primitive_debug_ok (		GetCurrentThreadId,			"GetCurrentThreadId");
	def_visible_primitive          (		maxscript_assert,			"assert");

	/* max file I/O */
	
	def_visible_primitive (					load_max_file,				"loadMaxFile");
	def_visible_primitive (					merge_max_file,				"mergeMaxFile");
	def_visible_primitive (					get_max_file_object_names,	"getMAXFileObjectNames");
	def_visible_primitive (					save_max_file,				"saveMaxFile");
	def_visible_primitive (					save_nodes,					"saveNodes");
	def_visible_primitive (					reset_max_file,				"resetMaxFile");
	def_visible_primitive (					hold_max_file,				"holdMaxFile");
	def_visible_primitive (					fetch_max_file,				"fetchMaxFile");
	def_visible_primitive (					import_file,				"importFile");
	def_visible_primitive (					export_file,				"exportFile");

	def_visible_primitive (					load_material_library,		"loadMaterialLibrary");
#ifndef NO_MATLIB_SAVING // orb 01-09-2002
#ifndef USE_CUSTOM_MATNAV // orb 08-23-2001 removing mtl lib saving from maxscript
	def_visible_primitive (					save_material_library,		"saveMaterialLibrary");
#endif
#endif

	def_visible_primitive (					load_temp_material_library,		"loadTempMaterialLibrary");
#ifndef NO_MATLIB_SAVING // orb 01-09-2002
#ifndef USE_CUSTOM_MATNAV // orb 08-23-2001 removing mtl lib saving from maxscript
	def_visible_primitive (					save_temp_material_library,		"saveTempMaterialLibrary");
#endif
#endif

	def_visible_primitive_debug_ok (		get_medit_material,			"getMeditMaterial");
	def_visible_primitive (					set_medit_material,			"setMeditMaterial");

	def_visible_primitive (					quit_max,					"quitMax");

	/* max animation */
	
	def_visible_primitive (					play_animation,				"playAnimation");
	def_visible_primitive (					stop_animation,				"stopAnimation");

	/* text file I/O */

	def_visible_primitive (					open_file,					"openfile");
	def_visible_primitive (					create_file,				"createfile");
	def_visible_primitive (					open_encrypted,				"openEncryptedFile");
	def_visible_primitive (					file_in,					"filein");
	def_visible_primitive_debug_ok (		format,						"format");
	def_visible_primitive_debug_ok (		open_log,					"openlog");
	def_visible_primitive_debug_ok (		close_log,					"closelog");
	def_visible_primitive_debug_ok (		flush_log,					"flushlog");

	def_visible_primitive (					get_open_filename,			"getOpenFileName");
	def_visible_primitive (					get_save_filename,			"getSaveFileName");
	def_visible_primitive (					get_save_path,				"getSavePath");
	def_visible_primitive (					filename_from_path,			"filenameFromPath");

	def_visible_primitive_debug_ok (		get_files,					"getFiles");
	def_visible_primitive_debug_ok (		get_directories,			"getDirectories");
	def_visible_primitive_debug_ok (		delete_file,				"deleteFile");
	def_visible_primitive_debug_ok (		rename_file,				"renameFile");
	def_visible_primitive_debug_ok (		copy_file,					"copyFile");
	def_visible_primitive_debug_ok (		make_dir,					"makeDir");
	def_visible_primitive_debug_ok (		get_filename_path,			"getFilenamePath");
	def_visible_primitive_debug_ok (		get_filename_file,			"getFilenameFile");
	def_visible_primitive_debug_ok (		get_filename_type,			"getFilenameType");
	def_visible_primitive_debug_ok (		get_file_mod_date,			"getFileModDate");
	def_visible_primitive_debug_ok (		get_file_create_date,		"getFileCreateDate");
	def_visible_primitive_debug_ok (		get_file_attribute,			"getFileAttribute");
	def_visible_primitive_debug_ok (		set_file_attribute,			"setFileAttribute");

	def_visible_primitive (					edit_script,				"edit");
	def_visible_primitive (					new_script,					"newScript");

	/* rendering */

	def_visible_primitive (					render,						"render");

	/* noise functions */

	def_visible_primitive_debug_ok (		noise3,						"noise3");
	def_visible_primitive_debug_ok (		noise4,						"noise4");
	def_visible_primitive_debug_ok (		turbulence,					"turbulence");
	def_visible_primitive_debug_ok (		fractalNoise,				"fractalNoise");

#ifndef NO_ATMOSPHERICS		// russom - 04/11/02
	/* atmospherics */

	def_visible_primitive (					addAtmospheric,				"addAtmospheric");
	def_visible_primitive (					setAtmospheric,				"setAtmospheric");
	def_visible_primitive (					getAtmospheric,				"getAtmospheric");
	def_visible_primitive (					deleteAtmospheric,			"deleteAtmospheric");
	def_visible_primitive (					editAtmospheric,			"editAtmospheric"); // RK: Added this
#endif // NO_ATMOSPHERICS

#ifndef NO_RENDEREFFECTS	// russom - 03/26/02
	/* effects */ // RK: Added this

	def_visible_primitive (					addEffect,					"addEffect");
	def_visible_primitive (					setEffect,					"setEffect");
	def_visible_primitive (					getEffect,					"getEffect");
	def_visible_primitive (					deleteEffect,				"deleteEffect");
	def_visible_primitive (					editEffect,					"editEffect");
#endif // NO_RENDEREFFECTS

	/* trackview nodes */

	def_visible_primitive (					newTrackViewNode,			"newTrackViewNode");
	def_visible_primitive (					deleteTrackViewNode,		"deleteTrackViewNode");
	def_visible_primitive (					addTrackViewController,		"addTrackViewController");
	def_visible_primitive (					deleteTrackViewController,	"deleteTrackViewController");

	/* CUI */
#ifndef NO_CUI	// russom - 02/12/01
	def_struct_primitive (					cui_setConfigFile,			cui,			"setConfigFile");
	def_struct_primitive (					cui_saveConfig,				cui,			"saveConfig");
	def_struct_primitive (					cui_saveConfigAs,			cui,			"saveConfigAs");
	def_struct_primitive (					cui_loadConfig,				cui,			"loadConfig");
	def_struct_primitive (					cui_getConfigFile,			cui,			"getConfigFile");
#endif // NO_CUI
	def_struct_primitive_debug_ok (			cui_getDir,					cui,			"getDir");
	def_struct_primitive (					cui_expertModeOn,			cui,			"expertModeOn");
	def_struct_primitive (					cui_expertModeOff,			cui,			"expertModeOff");
	def_struct_primitive (					cui_getExpertMode,			cui,			"getExpertMode");

	/* macro scripts */
	
	def_struct_primitive (					run_macro,					macros,			"run");
	def_struct_primitive (					edit_macro,					macros,			"edit");
	def_struct_primitive (					new_macro,					macros,			"new");
	def_struct_primitive (					load_macros,				macros,			"load");
	def_struct_primitive (					list_macros,				macros,			"list");

	/* track view window access */
	
	def_struct_primitive (					tvw_open,					trackView,		"open");
	def_struct_primitive (					tvw_zoomSelected,			trackView,		"zoomSelected");
	def_struct_primitive (					tvw_close,					trackView,		"close");
	def_struct_primitive (					tvw_numTrackViews,			trackView,		"numTrackViews");
	def_struct_primitive (					tvw_getTrackViewName,		trackView,		"getTrackViewName");
	def_struct_primitive (					tvw_setFilter,				trackView,		"setFilter");
	def_struct_primitive (					tvw_clearFilter,			trackView,		"clearFilter");
// LAM 4/1/00 - defined in MXSAgni
//	def_struct_primitive (					tvw_pickTrackDlg,			trackView,	"pickTrackDlg");

	/* schematic view window access */
#ifndef NO_SCHEMATICVIEW
	def_struct_primitive (					svw_open,					schematicView,	"open");
	def_struct_primitive (					svw_zoomSelected,			schematicView,	"zoomSelected");
	def_struct_primitive (					svw_close,					schematicView,	"close");
	def_struct_primitive (					svw_numSchematicViews,		schematicView,	"numSchematicViews");
	def_struct_primitive (					svw_getSchematicViewName,	schematicView,	"getSchematicViewName");
#endif // NO_SCHEMATICVIEW

	/* asset manager access (					should be filled out and moved into asset manager eventually) */
	def_struct_primitive (					ab_open,					assetBrowser,	"open");
	def_struct_primitive (					ab_gotoURL,					assetBrowser,	"gotoURL");

	/* modifier panel control */

	def_struct_primitive (					modp_setCurrentObject,		modPanel,		"setCurrentObject");
	def_struct_primitive (					modp_getCurrentObject,		modPanel,		"getCurrentObject");
	def_struct_primitive (					modp_getModifierIndex,		modPanel,		"getModifierIndex");
	def_struct_primitive (					modp_addModToSelection,		modPanel,		"addModToSelection");
	def_struct_primitive (					modp_validModifier,			modPanel,		"validModifier");

	def_struct_primitive (					modp_setPinStack,			modPanel,		"setPinStack");
	def_struct_primitive (					modp_getPinStack,			modPanel,		"getPinStack");
	def_struct_primitive (					modp_isPinStackEnabled,		modPanel,		"isPinStackEnabled");

	/* MAX map & xref path config */

	def_struct_primitive (					pathConfig_getDir,					pathConfig,		"getDir");
	def_struct_primitive (					pathConfig_setDir,					pathConfig,		"setDir");

	def_struct_primitive (	pathConfig_getCurrentProjectFolder,					pathConfig,		"getCurrentProjectFolder");
	def_struct_primitive (	pathConfig_setCurrentProjectFolder,					pathConfig,		"setCurrentProjectFolder");

	def_struct_primitive (	pathConfig_getProjectSubDirectoryCount,				pathConfig,		"getProjectSubDirectoryCount");
	def_struct_primitive (	pathConfig_getProjectSubDirectory,					pathConfig,		"getProjectSubDirectory");
	def_struct_primitive (	pathConfig_addProjectDirectoryCreateFilter,			pathConfig,		"addProjectDirectoryCreateFilter");
	def_struct_primitive (	pathConfig_removeAllProjectDirectoryCreateFilters,	pathConfig,		"removeAllProjectDirectoryCreateFilters");
	def_struct_primitive (	pathConfig_doProjectSetupSteps,						pathConfig,		"doProjectSetupSteps");
	def_struct_primitive (	pathConfig_doProjectSetupStepsUsingDirectory,		pathConfig,		"doProjectSetupStepsUsingDirectory");

	def_struct_primitive (	pathConfig_isProjectFolder,							pathConfig,		"isProjectFolder");
	def_struct_primitive (	pathConfig_getProjectFolderPath,					pathConfig,		"getProjectFolderPath");
	def_struct_primitive (	pathConfig_getCurrentProjectFolderPath,				pathConfig,		"getCurrentProjectFolderPath");

	// -- functions for working with paths & relative paths
	def_struct_primitive (	pathConfig_isAbsolutePath,				pathConfig, "isAbsolutePath");
	def_struct_primitive (	pathConfig_isLegalPath,					pathConfig, "isLegalPath");
	def_struct_primitive (	pathConfig_isRootPath,					pathConfig, "isRootPath");
	def_struct_primitive (	pathConfig_isUncPath,					pathConfig, "isUncPath");
	def_struct_primitive (	pathConfig_isUncSharePath,				pathConfig, "isUncSharePath");
	def_struct_primitive (	pathConfig_isPathRootedAtBackslash,		pathConfig, "isPathRootedAtBackslash");
	def_struct_primitive (	pathConfig_isPathRootedAtDriveLetter,	pathConfig, "isPathRootedAtDriveLetter");
	def_struct_primitive (	pathConfig_pathsResolveEquivalent,		pathConfig, "pathsResolveEquivalent");
	def_struct_primitive (	pathConfig_appendPath,					pathConfig, "appendPath");
	def_struct_primitive (	pathConfig_stripPathToTopParent,		pathConfig, "stripPathToTopParent");
	def_struct_primitive (	pathConfig_removePathTopParent,			pathConfig, "removePathTopParent");
	def_struct_primitive (	pathConfig_stripPathToLeaf,				pathConfig, "stripPathToLeaf");
	def_struct_primitive (	pathConfig_removePathLeaf,				pathConfig, "removePathLeaf");
	def_struct_primitive (	pathConfig_resolvePathSymbols,			pathConfig, "resolvePathSymbols");
	def_struct_primitive (	pathConfig_normalizePath,				pathConfig, "normalizePath");
	def_struct_primitive (	pathConfig_convertPathToLowerCase,		pathConfig, "convertPathToLowerCase");
	def_struct_primitive (	pathConfig_convertPathToRelativeTo,		pathConfig, "convertPathToRelativeTo");
	def_struct_primitive (	pathConfig_convertPathToUnc, 			pathConfig, "convertPathToUnc");
	def_struct_primitive (	pathConfig_convertPathToAbsolute,		pathConfig, "convertPathToAbsolute");
	
	def_struct_primitive (					pathConfig_isUsingProfileDirectories,	pathConfig,		"isUsingProfileDirectories");
	def_struct_primitive (					pathConfig_isUsingRoamingProfiles,		pathConfig,		"isUsingRoamingProfiles");

	def_struct_primitive (					pathConfig_LoadFromPathConfigFile,	pathConfig,			"Load");
	def_struct_primitive (					pathConfig_MergeFromPathConfigFile,	pathConfig,			"Merge");
	def_struct_primitive (					pathConfig_SaveToPathConfigFile,	pathConfig,			"SaveTo");

	def_struct_primitive (					mapPaths_add,				pathConfig.mapPaths,		"add");
	def_struct_primitive (					mapPaths_count,				pathConfig.mapPaths,		"count");
	def_struct_primitive (					mapPaths_get,				pathConfig.mapPaths,		"get");
	def_struct_primitive (					mapPaths_delete,			pathConfig.mapPaths,		"delete");
	def_struct_primitive (					mapPaths_getFullFilePath,	pathConfig.mapPaths,		"getFullFilePath");
	def_struct_primitive (					xrefPaths_add,				pathConfig.xrefPaths,		"add");
	def_struct_primitive (					xrefPaths_count,			pathConfig.xrefPaths,		"count");
	def_struct_primitive (					xrefPaths_get,				pathConfig.xrefPaths,		"get");
	def_struct_primitive (					xrefPaths_delete,			pathConfig.xrefPaths,		"delete");
	def_struct_primitive (					xrefPaths_getFullFilePath,	pathConfig.xrefPaths,		"getFullFilePath");
	def_struct_primitive (					sessionPaths_add,			pathConfig.sessionPaths,	"add");
	def_struct_primitive (					sessionPaths_count,			pathConfig.sessionPaths,	"count");
	def_struct_primitive (					sessionPaths_get,			pathConfig.sessionPaths,	"get");
	def_struct_primitive (					sessionPaths_delete,		pathConfig.sessionPaths,	"delete");

	def_struct_primitive (					pluginPaths_count,			pathConfig.pluginPaths,		"count");
	def_struct_primitive (					pluginPaths_get,			pathConfig.pluginPaths,		"get");

	def_struct_primitive (					mapPaths_add2,				mapPaths,		"add");
	def_struct_primitive (					mapPaths_count2,			mapPaths,		"count");
	def_struct_primitive (					mapPaths_get2,				mapPaths,		"get");
	def_struct_primitive (					mapPaths_delete2,			mapPaths,		"delete");
	def_struct_primitive (					mapPaths_getFullFilePath2,	mapPaths,		"getFullFilePath");
	def_struct_primitive (					xrefPaths_add2,				xrefPaths,		"add");
	def_struct_primitive (					xrefPaths_count2,			xrefPaths,		"count");
	def_struct_primitive (					xrefPaths_get2,				xrefPaths,		"get");
	def_struct_primitive (					xrefPaths_delete2,			xrefPaths,		"delete");
	def_struct_primitive (					xrefPaths_getFullFilePath2,	xrefPaths,		"getFullFilePath");
	def_struct_primitive (					sessionPaths_add2,			sessionPaths,	"add");
	def_struct_primitive (					sessionPaths_count2,		sessionPaths,	"count");
	def_struct_primitive (					sessionPaths_get2,			sessionPaths,	"get");
	def_struct_primitive (					sessionPaths_delete2,		sessionPaths,	"delete");

	def_struct_primitive (					pluginPaths_count2,			pluginPaths,	"count");
	def_struct_primitive (					pluginPaths_get2,			pluginPaths,	"get");

	/* tool modes */

	def_struct_primitive (					toolMode_uniformScale,		toolMode,		"uniformScale");
	def_struct_primitive (					toolMode_nonUniformScale,	toolMode,		"nonUniformScale");
	def_struct_primitive (					toolMode_squashScale,		toolMode,		"squashScale");
	def_struct_primitive (					toolMode_coordsys,			toolMode,		"coordsys");
	def_struct_primitive (					toolMode_pivotCenter,		toolMode,		"pivotCenter");
	def_struct_primitive (					toolMode_selectionCenter,	toolMode,		"selectionCenter");
	def_struct_primitive (					toolMode_transformCenter,	toolMode,		"transformCenter");

	/* utils */

	def_visible_generic_debug_ok (			show_interfaces,			"showInterfaces"); // LAM: 06/29/00
	def_visible_generic_debug_ok (			show_interface,				"showInterface");  // LAM: 08/29/00
	def_visible_generic_debug_ok (			get_interfaces,				"getInterfaces");
	def_visible_generic_debug_ok (			get_interface,				"getInterface");
	def_visible_primitive_debug_ok (		show_class,					"showClass");
	def_visible_generic_debug_ok (			show_props,					"showProperties");
	def_visible_generic_debug_ok (			show_methods,				"showMethods");
	def_visible_generic_debug_ok (			show_events,				"showEvents");
	def_visible_generic_debug_ok (			get_props,					"getPropNames");
	def_visible_primitive_debug_ok (		has_prop,					"hasProperty");
	def_visible_primitive_debug_ok (		is_prop,					"isProperty");
	def_visible_generic (					create_instance,			"createInstance");
	def_visible_primitive_debug_ok (		getproperty,				"getProperty");
	def_visible_primitive (					setproperty,				"setProperty");
	def_visible_primitive_debug_ok (		apropos,					"apropos");
	def_visible_generic_debug_ok (			exprForMAXObject,			"exprForMAXObject");
	def_visible_generic_debug_ok (			getSubAnim,					"getSubAnim");
	def_visible_generic_debug_ok (			getSubAnimName,				"getSubAnimName");
	def_visible_generic_debug_ok (			getSubAnimNames,			"getSubAnimNames");
	def_visible_primitive (					show_source,				"showSource");
	def_visible_generic_debug_ok (			getAppData,					"getAppData");
	def_visible_generic_debug_ok (			setAppData,					"setAppData");
	def_visible_generic_debug_ok (			deleteAppData,				"deleteAppData");
	def_visible_generic_debug_ok (			clearAllAppData,			"clearAllAppData");
	def_visible_generic (					addPluginRollouts,			"addPluginRollouts");
	def_visible_primitive (					startObjectCreation,		"startObjectCreation");
	def_visible_primitive_debug_ok (		isCreatingObject,			"isCreatingObject");
//	def_visible_primitive (					waitForToolStop,			"waitForToolStop");
	def_visible_generic_debug_ok (			iscompatible,				"isCompatible");

	def_visible_primitive (					progressStart,				"progressStart");
	def_visible_primitive (					progressUpdate,				"progressUpdate");
	def_visible_primitive (					progressEnd,				"progressEnd");
	def_visible_primitive (					getProgressCancel,			"getProgressCancel");
	def_visible_primitive (					setProgressCancel,			"setProgressCancel");

	def_visible_primitive_debug_ok (		dos_command,				"DOSCommand");
	def_visible_primitive_debug_ok (		sleep,						"sleep");
	def_visible_primitive_debug_ok (		timeStamp,					"timeStamp");

	def_visible_primitive (					open_utility,				"openUtility");
	def_visible_primitive (					close_utility,				"closeUtility");
	def_visible_primitive (					add_rollout,				"addRollout");
	def_visible_primitive (					remove_rollout,				"removeRollout");
	def_visible_primitive (					new_rollout_floater,		"newRolloutFloater");
	def_visible_primitive (					close_rollout_floater,		"closeRolloutFloater");

	def_visible_primitive_debug_ok (		message_box,				"messageBox");
	def_visible_primitive (					query_box,					"queryBox");
	def_visible_primitive (					yes_no_cancel_box,			"yesNoCancelBox");

	def_visible_primitive (					set_open_scene_script,		"setOpenSceneScript");
	def_visible_primitive (					set_save_scene_script,		"setSaveSceneScript");

	def_struct_primitive_debug_ok (			callbacks_addScript,			callbacks,		"addScript");
	def_struct_primitive_debug_ok (			callbacks_removeScript,			callbacks,		"removeScripts");
	def_struct_primitive (					callbacks_broadcastCallback,	callbacks,		"broadcastCallback");
	def_struct_primitive_debug_ok (			callbacks_show,					callbacks,		"show");
	def_struct_primitive_debug_ok (			callbacks_notificationParam,	callbacks,		"notificationParam");

	def_visible_primitive_debug_ok (		gc,							"gc");
	def_visible_primitive_debug_ok (		freeSceneBitmaps,			"freeSceneBitmaps");
	def_visible_primitive_debug_ok (		enumerateFiles,				"enumerateFiles");
	def_visible_primitive_debug_ok (		setSaveRequired,			"setSaveRequired");
	def_visible_primitive_debug_ok (		getSaveRequired,			"getSaveRequired");
	def_visible_primitive_debug_ok (		clearUndoBuffer,			"clearUndoBuffer");

	def_visible_primitive_debug_ok (		getCurrentSelection,		"getCurrentSelection");
	def_visible_primitive (					selectByName,				"selectByName");
	def_primitive_debug_ok (				getDollarSel,				"getDollarSel");
	def_primitive_debug_ok (				getCurrentScriptCtrl,		"getCurrentScriptCtrl");

	def_visible_primitive (					animateVertex,				"animateVertex");

	def_visible_primitive (					showTextureMap,				"showTextureMap");
//To produce a filename with frame number, implemented by BMMCreateNumberedFilename()
	def_visible_primitive (					createNumberedFilename,		"createNumberedFilename");
//Chengqing 03/July/07 : add functions for handling texture map in hardware mode
	def_visible_primitive (					showHWTextureMap,			"showHWTextureMap");
	def_visible_primitive (					enableHardwareMaterial,		"enableHardwareMaterial");

	def_visible_primitive (					setWaitCursor,				"setWaitCursor");
	def_visible_primitive (					setArrowCursor,				"setArrowCursor");

	def_visible_primitive_debug_ok (		registerTimeCallback,		"registerTimeCallback");
	def_visible_primitive_debug_ok (		unregisterTimeCallback,		"unregisterTimeCallback");
	def_struct_primitive (					mtlBrowser_browseFrom,		mtlBrowser,			"browseFrom");


	def_visible_primitive_debug_ok (		get_source_file_name,		"getSourceFileName");
	def_visible_primitive_debug_ok (		get_source_file_name_avgdlx,"getThisScriptFilename");
	def_visible_primitive_debug_ok (		get_source_file_offset,		"getSourceFileOffSet");

	/* active viewport utils */

	def_visible_primitive_debug_ok (		getActiveCamera,			"getActiveCamera");
	def_visible_primitive_debug_ok (		getScreenScaleFactor,		"getScreenScaleFactor");
	def_visible_primitive_debug_ok (		mapScreenToWorldRay,		"mapScreenToWorldRay");
	def_visible_primitive_debug_ok (		mapScreenToView,			"mapScreenToView");
	def_visible_primitive_debug_ok (		mapScreenToCP,				"mapScreenToCP");
	def_visible_primitive_debug_ok (		getCPTM,					"getCPTM");
	def_visible_primitive_debug_ok (		getViewTM,					"getViewTM");
	def_visible_primitive_debug_ok (		getViewSize,				"getViewSize");
	def_visible_primitive_debug_ok (		getViewFOV,					"getViewFOV");

	def_visible_primitive (					encryptScript,				"encryptScript");
	def_visible_primitive (					encryptFile,				"encryptFile");

	def_visible_primitive_debug_ok (		deleteChangeHandler,		"deleteChangeHandler");
	def_visible_primitive_debug_ok (		deleteAllChangeHandlers,	"deleteAllChangeHandlers");

	/* morph objects & keys */

	def_visible_primitive_debug_ok (		getMKTime,					"getMKTime");
	def_visible_primitive (					setMKTime,					"setMKTime");
	def_visible_primitive_debug_ok (		getMKWeight,				"getMKWeight");
	def_visible_primitive (					setMKWeight,				"setMKWeight");
	def_visible_primitive_debug_ok (		getMKKey,					"getMKKey");
	def_visible_primitive_debug_ok (		getMKKeyIndex,				"getMKKeyIndex");
	def_visible_primitive_debug_ok (		getMKTargetNames,			"getMKTargetNames");
	def_visible_primitive_debug_ok (		getMKTargetWeights,			"getMKTargetWeights");
#ifndef NO_OBJECT_MORPH	// russom - 10/13/01
	def_visible_primitive (					createMorphObject,			"createMorphObject");
	def_visible_primitive (					addMorphTarget,				"addMorphTarget");
	def_visible_primitive (					setMorphTarget,				"setMorphTarget");
	def_visible_primitive (					deleteMorphTarget,			"deleteMorphTarget");
	def_visible_primitive (					setMorphTargetName,			"setMorphTargetName");
#endif

	/* OLE automation */

	def_visible_primitive (					register_OLE_interface,		"registerOLEInterface");
	def_visible_primitive (					create_OLE_object,			"createOLEObject");
	def_visible_primitive_debug_ok (		release_OLE_object,			"releaseOLEObject");
	def_visible_primitive_debug_ok (		release_all_OLE_objects,	"releaseAllOLEObjects");

#ifdef DDE_ENABLED			// enable this for R4!!
	/* DDE access */

	def_struct_primitive (					dde_connect,				dde,		"connect");
	def_struct_primitive (					dde_disconnect,				dde,		"disconnect");
	def_struct_primitive (					dde_request,				dde,		"request");
	def_struct_primitive (					dde_execute,				dde,		"execute");
	def_struct_primitive (					dde_poke,					dde,		"poke");
	def_struct_primitive (					dde_adviseStart,			dde,		"adviseStart");
	def_struct_primitive (					dde_adviseStop,				dde,		"adviseStop");
	def_struct_primitive (					dde_createServer,			dde,		"createServer");
	def_struct_primitive (					dde_deleteServer,			dde,		"deleteServer");
	def_struct_primitive (					dde_createTopic,			dde,		"createTopic");
#endif

	// Set Key buffer control on an animatable

	def_struct_primitive_debug_ok (			setKey_subAnimBufferPresent,	setKey,		"subAnimBufferPresent");
	def_struct_primitive_debug_ok (			setKey_bufferPresent,			setKey,		"bufferPresent");
	def_struct_primitive (					setKey_subAnimCommitBuffer,		setKey,		"subAnimCommitBuffer");
	def_struct_primitive (					setKey_commitBuffer,			setKey,		"commitBuffer");
	def_struct_primitive (					setKey_subAnimRevertBuffer,		setKey,		"subAnimRevertBuffer");
	def_struct_primitive (					setKey_revertBuffer,			setKey,		"revertBuffer");

	/* evaluator primitives - emitted by parser */
	
	def_primitive_debug_ok (				progn,						"progn");
	def_primitive (							max_command,				"max");
	def_primitive_debug_ok (				return,						"return");
	def_primitive_debug_ok (				exit,						"exit");
	def_primitive_debug_ok (				continue,					"continue");

	def_lazy_primitive_debug_ok (			quote,						"quote");
	def_lazy_primitive_debug_ok (			if,							"if");
	def_lazy_primitive_debug_ok (			case,						"case");
	def_lazy_primitive_debug_ok (			while,						"while");
	def_lazy_primitive_debug_ok (			do,							"do");
	def_lazy_primitive_debug_ok (			for,						"for");
	def_lazy_primitive_debug_ok (			and,						"and");
	def_lazy_primitive_debug_ok (			or,							"or");

	def_lazy_primitive (					animate_context,			"animate_context");
	def_lazy_primitive (					redraw_context,				"redraw_context");
	def_lazy_primitive (					pivot_context,				"pivot_context");
	def_lazy_primitive (					level_context,				"level_context");
	def_lazy_primitive (					time_context,				"time_context");
	def_lazy_primitive (					coordsys_context,			"coordsys_context");
	def_lazy_primitive (					center_context,				"center_context");
	def_lazy_primitive (					undo_context,				"undo_context");
	def_lazy_primitive_debug_ok (			printallelements_context,	"printallelements_context");
	def_lazy_primitive (					defaultactions_context,		"defaultactions_context");
	def_lazy_primitive (					quiet_context,				"quiet_context");

	def_lazy_primitive_debug_ok (			assign,						"=");
	def_lazy_primitive_debug_ok (			op_assign,					"+=");
	def_lazy_primitive_debug_ok (			make_persistent,			"make_persistent");

	def_mapped_generic_debug_ok (			assign,						"=");
	def_mapped_generic_debug_ok (			op_assign,					"+=");
	def_generic_debug_ok (					not,						"not");

	def_lazy_primitive_debug_ok (			try,						"try");
	def_primitive_debug_ok (				throw,						"throw");
	def_visible_primitive_debug_ok (		getCurrentException,		"getCurrentException");

	def_lazy_primitive (					change_handler,				"change_handler");

	def_visible_lazy_primitive_debug_ok (	swap,						"swap");

	def_visible_primitive_debug_ok (		dependsOn,					"dependsOn");

// LAM 4/1/00 - moved from MXSAgni

	def_visible_primitive (					registerRightClickMenu,				"registerRightClickMenu");
	def_visible_primitive (					unregisterRightClickMenu,			"unregisterRightClickMenu");
	def_visible_primitive (					unregisterAllRightClickMenus,		"unregisterAllRightClickMenus");
	def_visible_primitive (					registerRedrawViewsCallback,		"registerRedrawViewsCallback");
	def_visible_primitive (					unregisterRedrawViewsCallback,		"unregisterRedrawViewsCallback");
//watje 7-9-00 exposes the new filter callback
	def_visible_primitive (					registerSelectFilterCallback,		"registerSelectFilterCallback");
	def_visible_primitive (					unregisterSelectFilterCallback,		"unregisterSelectFilterCallback");

//watje 7-11-00 exposes the new display filter callback
	def_visible_primitive (					registerDisplayFilterCallback,		"registerDisplayFilterCallback");
	def_visible_primitive (					unregisterDisplayFilterCallback,	"unregisterDisplayFilterCallback");

// LAM 7/23/02 - moved from gScript
	def_visible_primitive (					AddSubRollout,				"AddSubRollout");
	def_visible_primitive (					RemoveSubRollout,			"RemoveSubRollout");

	def_struct_primitive (					registerDialogBar,			cui,"registerDialogBar");
	def_struct_primitive (					unRegisterDialogBar,		cui,"unRegisterDialogBar");
	def_struct_primitive (					dockDialogBar,				cui,"dockDialogBar");
	def_struct_primitive (					floatDialogBar,				cui,"floatDialogBar");
	def_struct_primitive (					getDockState,				cui,"getDockState");
	def_struct_primitive (					setAppTitle,				cui,"setAppTitle");

	def_visible_primitive (					CreateDialog,				"CreateDialog");
	def_visible_primitive (					DestroyDialog,				"DestroyDialog");
	def_visible_primitive (					SetDialogPos,				"SetDialogPos");
	def_visible_primitive (					GetDialogPos,				"GetDialogPos");
	def_visible_primitive (					GetDialogSize,				"GetDialogSize");

	def_visible_primitive (					SetDialogBitmap,			"SetDialogBitmap");
	def_visible_primitive (					GetDialogBitmap,			"GetDialogBitmap");

	def_visible_primitive (					PopupMenu,					"PopupMenu");
//	def_visible_primitive (					PopupMenuBar,				"PopupMenuBar");

	def_visible_primitive_debug_ok (		getNodeByName,				"getNodeByName");

	def_visible_primitive_debug_ok	(		setModContextBBox,			"setModContextBBox");
	def_visible_primitive_debug_ok	(		setModContextTM,			"setModContextTM");

	// --------------- CustAttrib primitives

	def_struct_primitive (					ca_add,				custAttributes,			"add");
	def_struct_primitive (					ca_delete,			custAttributes,			"delete");
	def_struct_primitive_debug_ok (			ca_count,			custAttributes,			"count");
	def_struct_primitive_debug_ok (			ca_get,				custAttributes,			"get");
	def_struct_primitive (					ca_makeUnique,		custAttributes,			"makeUnique");
	def_struct_primitive_debug_ok (			ca_getDef,			custAttributes,			"getDef");
	def_struct_primitive_debug_ok (			ca_getDefSource,	custAttributes,			"getDefSource");
	def_struct_primitive_debug_ok (			ca_getDefData,		custAttributes,			"getDefData");
	def_struct_primitive (					ca_setDefData,		custAttributes,			"setDefData");
	def_struct_primitive_debug_ok (			ca_getPBlockDefs,	custAttributes,			"getPBlockDefs");
	def_struct_primitive_debug_ok (			ca_getSceneDefs,	custAttributes,			"getSceneDefs");
	def_struct_primitive (					ca_deleteDef,		custAttributes,			"deleteDef");
	def_struct_primitive_debug_ok (			ca_getDefs,			custAttributes,			"getDefs");
	def_struct_primitive (					ca_redefine,		custAttributes,			"redefine");
	def_struct_primitive_debug_ok (			ca_getOwner,		custAttributes,			"getOwner");
	def_struct_primitive_debug_ok (			ca_getDefClass,		custAttributes,			"getDefClass");
	def_struct_primitive_debug_ok (			ca_getDefInstances,	custAttributes,			"getDefInstances");

	//      hold manager exposure

	def_struct_primitive_debug_ok (			theHold_getMaxLevels,		theHold,		"getMaxUndoLevels");
	def_struct_primitive (					theHold_setMaxLevels,		theHold,		"setMaxUndoLevels");
	def_struct_primitive_debug_ok (			theHold_getCurLevels,		theHold,		"getCurrentUndoLevels");
	def_visible_primitive_debug_ok (		IsUndoDisabled,				"isUndoDisabled");


	// --------------- symbolic pathname primitives

	def_struct_primitive_debug_ok (			symbolicPaths_num_paths,			symbolicPaths,			"numPaths");
	def_struct_primitive_debug_ok (			symbolicPaths_get_path_name,		symbolicPaths,			"getPathName");
	def_struct_primitive_debug_ok (			symbolicPaths_is_path_name,			symbolicPaths,			"isPathName");
	def_struct_primitive_debug_ok (			symbolicPaths_get_path_value,		symbolicPaths,			"getPathValue");
	def_struct_primitive_debug_ok (			symbolicPaths_num_user_paths,		symbolicPaths,			"numUserPaths");
	def_struct_primitive_debug_ok (			symbolicPaths_get_user_path_name,	symbolicPaths,			"getUserPathName");
	def_struct_primitive_debug_ok (			symbolicPaths_is_user_path_name,	symbolicPaths,			"isUserPathName");
	def_struct_primitive_debug_ok (			symbolicPaths_get_user_path_value,	symbolicPaths,			"getUserPathValue");
	def_struct_primitive_debug_ok (			symbolicPaths_set_user_path_value,	symbolicPaths,			"setUserPathValue");
	def_struct_primitive_debug_ok (			symbolicPaths_add_user_path,		symbolicPaths,			"addUserPath");
	def_struct_primitive_debug_ok (			symbolicPaths_remove_user_path,		symbolicPaths,			"removeUserPath");
	def_struct_primitive_debug_ok (			symbolicPaths_expandFileName,		symbolicPaths,			"expandFileName");

#pragma warning(pop)
