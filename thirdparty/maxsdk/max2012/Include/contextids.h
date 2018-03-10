// Copyright 2010 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//
//===============================================================================================
//
// 3ds Max context-sensitive help items.
// Help Contents (help menu IDs are in the menu section).
// These ids can be used with the MaxSDK::IHelpSystem::GetInstance()->ShowProductHelpForTopic function in order to display a specific 3ds Max help topic.
// MaxSDK::IHelpSystem::GetInstance()->ShowProductHelpForTopic( idh_file_save_as ); would display the help related to File/Save as... 
//
// Any change to this file will require an upate of HelpsysContextIDtoString.cpp
//

#pragma once

#define idh_contents				1000	// 3ds Max Online Help (contents page)
#define what_s_new_topic			1001	// Help what's new

// Titlebar items
#define idh_titlebar				2000	// Titlebar

//Menubar items
#define idh_menubar				3000	// Menubar

// File menu items
#define idh_file_menu				3100	// File menu
#define idh_file_new				3110	// New
#define idh_file_reset				3120	// Reset
#define idh_file_open				3130	// Open
#define idh_file_merge				3135	// Merge
#define idh_file_replace			3137	// Replace
#define idh_file_insert_tracks		3138	// Insert Tracks
#define idh_file_save				3140	// Save
#define idh_file_save_as			3150	// Save As
#define idh_file_save_selected			3160	// Save Selected
#define idh_file_import				3180	// Import
#define idh_file_export				3190	// Export
#define idh_file_archive			3200	// Archive
#define idh_file_summary_info			3210	// Summary Info
#define idh_file_view_file			3230	// View File
#define idh_file_configure_paths	3245	// Configure Paths
#define idh_file_preferences			3250	// Preferences
#define idh_file_properties			3255	// Properties
#define idh_file_exit				3260	// Exit
#define idh_file_history			3270	// History
#define idh_file_save_copy_as 3280 // Save a Copy As

// Edit menu items
#define idh_edit_menu				3300	// Edit menu
#define idh_edit_undo_redo			3310	// Undo/Redo
#define idh_edit_hold				3320	// Hold
#define idh_edit_fetch				3330	// Fetch
#define idh_edit_delete				3340	// Delete
#define idh_edit_clone				3350	// Clone
#define idh_edit_select_all			3360	// Select All
#define idh_edit_select_none		3370	// Select None
#define idh_edit_select_invert		3380	// Select Invert
#define idh_edit_select_by			3390	// Select By
#define idh_edit_select_by_color	3392
#define idh_edit_select_by_name		3394
#define idh_edit_select_by_similar	3395 
#define idh_edit_region				3400	// Region
#define idh_edit_select_region_window	3402
#define idh_edit_select_region_crossing	3404
#define idh_edit_remove_named_selections	3410	// Remove Named Selections
#define idh_edit_edit_named_selections	3412
#define idh_edit_transform_type_in		3420	// Transform Type-in
#define idh_edit_mirror				3430	// Mirror
#define idh_edit_array				3435	// Mirror
#define idh_edit_snapshot			3437	// Mirror
#define idh_edit_align				3440	// Align
#define idh_edit_align_normals			3450	// Align Normals
#define idh_edit_place_highlight	3455
#define idh_edit_track_view			3459
#define idh_edit_schematic_view		3460
#define idh_edit_material_editor	3465
#define idh_edit_properties			3470	// Properties
#define idh_spacing_tool			3471

// Tools Menu
#define	idh_tools_menu				3475	// Tools menu
#define idh_tools_display_floater	3477	// display floater
#define idh_tools_selection_floater	3479	// selection floater
#define idh_tools_material_map_browser	3480	// mtl/map browser

// MAXScript menu (added JBW 11/11/98 & 12/15/98)
#define idh_maxscript_listener		3481	// open listener
#define idh_maxscript_open			3482	// open script editor
#define idh_maxscript_new			3483	// new script
#define idh_maxscript_run			3484	// run script file
#define idh_maxscript_macrorec		3485	// toggle macrorecorder enable
#define idh_maxscript_debug_dialog	3486	// open maxscript debugger dialog
#define idh_maxscript_editor		3487	// open script editor dialog

// Grouping menu items
#define idh_grouping_menu			3500	// Grouping menu
#define idh_grouping_group			3510	// Group
#define idh_grouping_open			3520	// Open
#define idh_grouping_close			3530	// Close
#define idh_grouping_ungroup			3540	// Ungroup
#define idh_grouping_explode			3550	// Explode
#define idh_grouping_detach			3560	// Detach
#define idh_grouping_attach			3570	// Attach

// Views menu items
#define idh_views_menu				3600	// Views menu
#define idh_views_undo_redo			3610	// Undo/Redo
#define idh_views_save_active		3620
#define idh_views_restore_active	3621
#define idh_views_units_setup		3625
#define idh_views_grid_and_snap_settings	3630	// Grid and Snap Settings
#define idh_views_grids				3640	// Grids
#define idh_views_background_image		3650	// Background Image
#define idh_views_update_background_image	3652
#define idh_views_reset_background_transform	3654
#define idh_views_show_axis_icon		3660	// Show Axis Icon
#define idh_views_show_ghosting			3662
#define idh_views_show_key_times		3664
#define idh_views_shade_selected		3670
#define idh_views_show_dependencies		3680
#define idh_views_match_camera_to_view	3682
#define idh_views_redraw_all_views		3690	// Redraw All Views
#define idh_views_deactivate_all_maps	3695	// Deactivate all maps in views
#define idh_views_activate_all_maps		3696	// Activate all maps in views
#define idh_views_update_during_spinner_drag	3697
#define idh_views_expert_mode			3698
#define idh_views_viewport_configuration	3710	// Viewport Configuration

// Rendering menu items
#define idh_rendering_menu				3800	// Rendering menu
#define idh_rendering_render			3810	// Render
#define idh_rendering_ireshade			3815	// interactive reshading
#define idh_rendering_video_post		3820	// Video Post
#define idh_rendering_environment		3840	// Environment
#define idh_radiosity				    3841    // Radiosity panel
#define idh_rendering_make_preview		3870	// Make Preview
#define idh_rendering_view_preview		3880	// View Preview
#define idh_rendering_rename_preview 	3890	// Rename Preview
#define idh_rendering_show_last_render	3892	// Show Last Rendering
#define idh_rendering_show_vfb			3893	// Show VFB


#define idh_track_view_menu						3900	// Trackview menu
#define idh_track_view_open_track_view			3902
#define idh_track_view_new_track_view			3904
#define idh_track_view_delete_track_view		3906
#define idh_track_view_stored_track_views_list	3908

// Schematic view menu items...
#define idh_schematic_view_open_schematic_view			3910
#define idh_schematic_view_new_schematic_view			3911
#define idh_schematic_view_delete_schematic_view		3912
#define idh_schematic_view_stored_schematic_views_list	3913

// Help menu items
#define idh_help_menu				4300	// Help menu
#define idh_help_contents			4310	// Help contents
#define idh_help_topics				4320	// topics
#define idh_help_plugin_help		4330	// plug-in help
#define idh_help_about				4340	// About 3D Studio MAX
#define idh_help_learning			4342
#define idh_help_connect_to_support_and_information	4344
#define idh_help_maxscript			4345
#define idh_help_authorize			4346
#define idh_help_connect_to_update_site	4347
#define idh_help_borrowlicense		4348
#define idh_help_returnlicense		4349

// Toolbar items
#define idh_toolbar				5000	// Toolbar
#define idh_tb_undo_redo			5010	// Undo/Redo
#define idh_tb_undo					5011
#define idh_tb_redo					5012
#define idh_tb_link_unlink			5020	// Link/Unlink
#define idh_tb_link					5021
#define idh_tb_unlink				5022
#define idh_tb_bind_to_space_warp		5030	// Bind to Space Warp
#define idh_tb_selection			5040	// Selection
#define idh_tb_selection_region		5041
#define idh_tb_sel_filter			5042
#define idh_tb_hit_by_name			5050	// Hit-by-name
#define idh_tb_transformations			5060	// Transformations
#define idh_tb_move					5061
#define idh_tb_rotate				5062
#define idh_tb_scale				5063
#define idh_tb_coord_sys			5064
#define idh_tb_nuscale				5065
#define idh_tb_pivot				5070	// Pivot button
#define idh_tb_x					5071
#define idh_tb_y					5072
#define idh_tb_z					5073
#define idh_tb_plane				5074
#define idh_tb_constraints			5075	// axis constaint buttons
#define idh_tb_inverse_kinematics_buttons	5080	// Inverse Kinematics Buttons
#define idh_tb_mirror				5085
#define idh_tb_array_button			5090	// Array Button
#define idh_tb_spacing_tool			5092    // Spacing tool
#define idh_tb_align_and_align_normals		5100	// Align and Align Normals
#define idh_tb_track_view_button		5110	// Track View Button
#define idh_tb_schematic_view_button	5111	// Schematic View Button
#define idh_tb_material_editor_button		5115	// Material Editor
#define idh_tb_rendering_controls		5120	// Rendering Controls
#define idh_tb_render					5121
#define idh_tb_render_quick				5122
#define idh_tb_render_last				5123
#define idh_tb_render_region			5124
#define idh_tb_named_selection_sets		5130	// Named Selection Sets

// the following item was added by Vera (we need this button in the toolbar so a  
// user can click on UI items that are not selectable or cannot receive focus)

#define idh_tb_help				5131	// Help button

// track view items
#define idh_track_view				5500	// Track View
#define idh_tv_layout				5510	// Track View Layout
#define idh_tv_hierarchy_list			5520	// Hierarchy list
#define idh_tv_edit_window			5530	// Track View Edit window
#define idh_tv_tool_bar				5540	// Track View Tool bar
#define idh_tv_navigation_icons			5550	// Track View Navigation icons (are there several?, if so, we need one ID per icon)

#define idh_view_ports				5600	// View ports
#define idh_vp_orthographic			5610	// Orthographic Views
#define idh_vp_camera				5620	// Camera Views
#define idh_vp_user				5630	// User Views
#define idh_vp_grid				5640	// Grid Views
#define idh_vp_spotlight			5650	// Spotlight Views

// Status line and prompt area items
#define idh_status_panel			5700
#define idh_sp_time_slider			5710	// Time slider
#define idh_sp_prompt_line			5720	// Prompt line
#define idh_sp_status_line			5730	// Status line
#define idh_sp_selection_lock		5735
#define idh_sp_window_crossing		5740
#define idh_sp_degrade_override		5745
#define idh_sp_snap_mode			5750
#define idh_sp_snap_toggle			5755
#define idh_sp_angle_snap			5760
#define idh_sp_percent_snap			5765
#define idh_sp_spinner_snap			5770
#define idh_sp_oneclick				5775
#define idh_sp_object_snap			5780	//object snapping
#define idh_sp_tti_mode				5790	//transform type-in absolute-relative toggle


// time panel
#define idh_time_panel				5800
#define idh_tp_animate				5810
#define idh_tp_goto_start			5820
#define idh_tp_previous				5830
#define idh_tp_play					5840
#define idh_tp_next					5850
#define idh_tp_goto_end				5860
#define idh_tp_key_mode				5870
#define idh_tp_cur_time				5880
#define idh_tp_time_config			5890
#define idh_tp_dflt_tangent			5900

// View control panel (lower right)
#define idh_view_control			8200
#define idh_vc_zoom					8201
#define idh_vc_zoom_all				8202
#define idh_vc_zoom_extents			8203
#define idh_vc_zoom_ext_sel			8204
#define idh_vc_zoom_ext_all			8205
#define idh_vc_zoom_ext_all_sel		8206
#define idh_vc_zoom_region			8207
#define idh_vc_field_of_view		8208
#define idh_vc_pan					8209
#define idh_vc_arc_rotate			8210
#define idh_vc_arc_rotate_sel		8211
#define idh_vc_min_max_toggle		8212
// camera specific
#define idh_vc_dolly				8213
#define idh_vc_perspective			8214
#define idh_vc_roll					8215
#define idh_vc_truck				8216
#define idh_vc_orbit				8217
#define idh_vc_pan_cam				8218
// spotlight specific
#define idh_vc_hotspot				8219
#define idh_vc_falloff				8220

// Command panel items
#define idh_command_panel			6000	// Command panel

// Create items
#define idh_cp_create				6100	// Create
#define idh_create_entity_names			6110	// Entity Names
#define idh_create_geometry			6120	// Geometry
#define idh_create_space_warps			6130	// Space Warps
#define idh_create_object_colors		6140	// Object Colors
#define idh_create_materials			6150	// Materials
#define idh_create_nonrendering_objects		6160	// Non-rendering Objects
#define idh_create_lights			6170	// Lights
#define idh_create_cameras			6180	// Cameras
#define idh_create_helper_objects		6190	// Helper Objects
#define idh_create_systems			6200	// Systems
#define idh_create_objects_instances_references	6210	// Objects, Instances, and References


// Modify items
#define idh_cp_modify				6300	// Modify
#define idh_modify_panel_layout			6310	// Layout of the Modify Command Panel
#define idh_modify_modifier_stack		6320	// Modifier Stack
#define idh_modify_miscellaneous		6330	// Miscellaneous Additions
#define idh_modify_subobject_selection		6340	// Sub-Object Selection
#define idh_modify_object_modifiers_editors	6350	// Object Modifiers (Editors)
#define idh_modify_object_modifiers_other	6360	// Object Modifiers (Other)
#define idh_modify_lights			6370	// Lights
#define idh_modify_cameras			6380	// Cameras
#define idh_modify_mapping			6390	// Mapping

// Surface items
#define idh_cp_surface				6400	// Surface
#define idh_surface_surfaces			6410	// Surfaces
#define idh_surface_materials			6420	// Materials

// Hierarchy items
#define idh_cp_hierarchy			6500	// Hierarchy
#define idh_hierarchy_adjust_pivot		6510	// Adjust Pivot
#define idh_hierarchy_transform			6520	// Transform
#define idh_hierarchy_link_info			6530	// Link Info
#define idh_hierarchy_ik			6540	// IK
#define idh_hierarchy_links			6550	// Links

// Motion items
#define idh_cp_motion				6600	// Motion
#define idh_motion_assign_transform_controller	6610	// Assign Transform Controller
#define idh_motion_path				6620	// Path
#define idh_motion_key_info			6630	// Key Info

// Display items
#define idh_cp_display				6700	// Display
#define idh_display_color			6710	// Display Color
#define idh_display_category			6720	// Category Displays
#define idh_display_individual			6730	// Individual Displays
#define idh_display_freeze			6740	// Freeze Displays
#define idh_display_optimizations		6750	// Display Optimizations
#define idh_display_links			6760	// Display Links



// Dialog Box: Viewport Configuration 
#define idh_dialog_viewport_configuration	7000	// main box
// render page
#define idh_vpconfig_render_page	7001	// render page
#define idh_vpconfig_rendering_level	7002	// rendering level (all)
#define idh_vpconfig_apply_to		7003	// apply to (for rendering level)
#define idh_vpconfig_rendering_options	7004	// rendering options
#define idh_vpconfig_fast_view		7005
#define idh_vpconfig_persp_user_view	7006
// layout page
#define idh_vpconfig_layout_page	7010
#define idh_vpconfig_layout_types	7011	// all layout buttons at top
#define idh_vpconfig_assign_view	7012	// view assignment region
#define idh_vpconfig_current_layout 7013
// safeframe page
#define idh_vpconfig_safeframe_page	7020
#define idh_vpconfig_safeframe_setup	7021	// setup portion
#define idh_vpconfig_safeframe_app	7022		// application portion
// degradation page
#define idh_vpconfig_degrade_page	7030	// degradation page
#define idh_vpconfig_general_degrade	7031	// general region
#define idh_vpconfig_active_degrade	7032		// active region
#define idh_vpconfig_parameters		7033		// degrade parameters
#define idh_vpconfig_interrupt		7034
// regions page
#define idh_vpconfig_regions_page	7040
#define idh_vpconfig_blowup_region	7041
#define idh_vpconfig_subregion		7042
// statistics page
#define idh_vpconfig_statistics_page	7045
#define idh_vpconfig_statistics_setup	7046	// setup portion
#define idh_vpconfig_statistics_app		7047	// application portion

// Dialog Box: Path Configuration
#define idh_dialog_configure_paths	7050	// main box
// general page
#define idh_paths_general_page		7051	// general page
#define idh_paths_plugin_page		7061	// plugin page
#define idh_paths_bitmaps_page		7062	// bitmaps page

// Dialog Box: Preference Setting 
#define idh_dialog_preference_settings	8000	// main box
// general page
#define idh_pref_general_page		8010
#define idh_pref_preview_options	8011
#define idh_pref_icon_size			8012
#define idh_pref_layout				8013
#define idh_pref_system_units		8014
#define idh_pref_interaction		8015
// rendering page
#define idh_pref_rendering_page		8018
#define idh_pref_video_color		8019
#define idh_pref_dithering			8020
#define idh_pref_field_order		8021
#define idh_pref_super_black		8022
#define idh_pref_hotspot_falloff	8023
#define idh_pref_def_preview		8024
#define idh_pref_renderer			8025
#define idh_pref_render_terminate	8026
// IK page
#define idh_pref_ik_page			8029
#define idh_pref_applied_ik			8030
#define idh_pref_interactive_ik		8031
// animation page
#define idh_pref_animation_page		8034
#define idh_pref_key_brackets		8035
#define idh_pref_animate			8036
#define idh_pref_sound				8037
#define idh_pref_controller_defs	8038
// keyboard page
#define idh_pref_keyboard_page		8042
#define idh_pref_command			8043
#define idh_pref_shortcut			8044
#define idh_pref_sets				8045
// files page
#define idh_pref_files_page			8048
#define idh_pref_file_handling		8049
#define idh_pref_file_menu			8050
#define idh_pref_archive_system		8051
// gamma page
#define idh_pref_gamma_page			8054
#define idh_pref_display			8055
#define idh_pref_device				8056
#define idh_pref_files				8057
// radiosity page
#define idh_pref_radiosity_page		8058
// mental ray page
#define idh_pref_mentalray_page		8059
// viewports page
#define idh_pref_viewports_page		8060
#define idh_pref_viewport_params	8061
#define idh_pref_viewport_bkgnd		8062
#define idh_pref_move_xfm			8063
// colors page
#define idh_pref_colors_page		8065
// maxscript page
#define idh_pref_maxscript_page		8066
// containers page
#define idh_pref_containers_page	8067
// gizmos page
#define idh_pref_tmgizmos_page		8068
// help page
#define idh_pref_help_page			8069

// Dialog Box: Select by Name 
#define idh_dialog_select_by_name	8070
#define idh_sbn_list				8071
#define idh_sbn_list_control		8072	// bottom All/None/Inv and checkboxes
#define idh_sbn_sort				8073
#define idh_sbn_list_display		8074
#define idh_sbn_selection_sets		8075

// Scene Info dialog box
#define idh_dialog_scene_info		8080
#define idh_si_scene_totals			8081
#define idh_si_mesh_totals			8082
#define idh_si_memory				8083
#define idh_si_rendering			8084
#define idh_si_description			8085
#define idh_si_scene_info			8086
#define idh_si_save_to_file			8087
#define idh_si_plugin_info			8088

// Array dialog box
#define idh_dialog_array			8092
#define idh_ar_transformation		8093
#define idh_ar_type_of_entity		8094
#define idh_ar_total				8095
#define idh_ar_reset				8096

// Preview dialog box
#define idh_dialog_preview			8100
#define idh_pv_range				8101
#define idh_pv_frame_rate			8102
#define idh_pv_image_size			8103
#define idh_pv_display				8104
#define idh_pv_rnd_level			8105
#define idh_pv_output				8106


// dialogs with no "control level" granularity
#define idh_dialog_about			9000
#define idh_dialog_select_camera	9010
#define idh_dialog_collapse_warning	9020	// for both collapse all and collapse range
#define idh_dialog_edit_mod_stack	9030
#define idh_dialog_file_new			9040
#define idh_dialog_file_reset		9050
#define idh_dialog_object_prop		9060
//JH 10/22/01 adding id's for prop pages
#define idh_dialog_object_general		9061 //general object properties
#define idh_dialog_object_radiosity		9062 //radiosity object properties
#define idh_dialog_object_mentalray		9063 //mental ray properties
#define idh_dialog_object_bones			9064 //bone properties
#define idh_dialog_object_user			9065 //user defined properties
#define idh_dialog_object_pathtracer	9066 //path tracer properties

#define idh_dialog_grid_snap_settings	9070
#define idh_dialog_group			9080
#define idh_dialog_import_name_conflict	9090
#define idh_dialog_missing_dlls		9100
#define idh_dialog_merge			9110
#define idh_dialog_merge_dup		9111
#define idh_dialog_merge_matlib		9112
#define idh_dialog_merge_matlib_dup	9113
#define idh_dialog_replace			9115
#define idh_dialog_mod_setup		9120
#define idh_dialog_choose_view		9130
#define idh_dialog_mtl_preview		9140
#define idh_dialog_object_color		9150
#define idh_dialog_clone_options	9160
#define idh_dialog_plugin_help		9170
#define idh_dialog_plugin_info		9180	// subdialog of scene info
#define idh_dialog_preview_cancel	9190
#define idh_dialog_rescale_time		9200
#define idh_dialog_time_config		9210
#define idh_dialog_topo_warning		9220	// topology warning
#define idh_dialog_units_setup		9230
#define idh_dialog_viewport_background	9240
#define idh_dialog_customize_ui		9245
#define idh_dialog_createbrushpreset 9246

#define idh_pref_draftrenderer		9250

// UI cutomization tabbed dialog
#define idh_customize_toolbars      9255  // toolbar page
#define idh_customize_menu          9260  // main menus page
#define idh_customize_quadmenus     9265  // quad menus page
#define idh_customize_colors        9270  // colors page
#define idh_customize_tpgroups      9275  // Tool Palette groups page

// FileLink dialogs
#define idh_filelink_mgr_main			9300
#define idh_filelink_mgr_attach_page	9301
#define idh_filelink_mgr_files_page		9302
#define idh_filelink_mgr_presets_page	9303
#define idh_filelink_mgr_rendering_page	9304
#define idh_filelink_dwg_settings		9310
#define idh_filelink_dwg_settings_basic	9311
#define idh_filelink_dwg_settings_adv	9312

//Dans id's
#define idh_dialog_mtl_edit			10001
#define idh_dialog_medit_opt		10002
#define idh_dialog_put_medit		10003
#define idh_dialog_mtlbrows			10004
#define idh_dialog_render			10005
#define idh_dialog_excl_list   		10006
#define idh_dialog_update_mtls 		10007
#define idh_dialog_sme				10008 // schematic material editor

// Gus' IDs

#define idh_net_server_window				10500	//-- Network Render SERVER main UI Window
#define idh_net_manager_window				10501	//-- Network Render MANAGER main UI Window
#define idh_net_logging_prop				10502	//-- Network Render log properties (common to both Manager and Server)
#define idh_net_manager_prop				10503	//-- Network Render Manager Properties Dialogue
#define idh_net_server_prop					10504	//-- Network Render Server Properties Dialogue

#define idh_net_job_assignment				10520	//-- Network Rendering - Job Assignment Dialogue
#define idh_net_job_assignment_srv_prop		10521	//-- Network Rendering - Job Assignment Dialogue - Server Properties
#define idh_net_job_assignment_srv_group	10522	//-- Create Server Group Dialogue in Network Job Assignment
#define idh_net_job_assignment_srv_tabs		10523	//-- Set Server Properties (set the different tabs in the server list)

#define idh_net_qm_main						10530	//-- Queue Manager Main Window
#define idh_net_qm_client_list				10531	//-- Queue Manager - Client List Dialogue
#define idh_net_qm_connect					10532	//-- Queue Manager - Connect Dialogue
#define idh_net_qm_job_report				10533	//-- Queue Manager - Job Report Dialogue
#define idh_net_qm_job_settings				10533	//-- Queue Manager - Job Settings Dialogue
#define idh_net_qm_queue_control			10534	//-- Queue Manager - Someone else requests Queue Control Dialogue
#define idh_net_qm_server_group				10535	//-- Queue Manager - Create Server Group (same as idh_net_job_assignment_srv_group above)
#define idh_net_qm_server_prop				10536	//-- Queue Manager - Server Properties (same as idh_net_job_assignment_srv_prop above)
#define idh_net_qm_server_tabs				10537	//-- Queue Manager - Set Server Properties (set the different tabs in the server list)
#define idh_net_qm_srv_week_schedule		10538	//-- Queue Manager - Server Week Schedule Dialogue

#define idh_notify_main_ui					10600	//-- Notify.exe main UI
#define idh_bmm_vfb							10601	//-- BMM Virtual Frame Buffer Window

#define idh_vp_exec_dialogue				10620	//-- Video Post - Execute Dialogue
#define idh_vp_presets_dialogue				10621	//-- Video Post - Render Presets Dialogue (Predefined Output Resolutions)
#define idh_vp_edit_scene					10622	//-- Video Post - Edit Scene Dialogue
#define idh_vp_edit_filter					10623	//-- Video Post - Edit Filter Dialogue
#define idh_vp_edit_layer					10624	//-- Video Post - Edit Layer Dialogue
#define idh_vp_edit_image_input				10625	//-- Video Post - Edit Image Input Dialogue
#define idh_vp_edit_image_output			10626	//-- Video Post - Edit Image Output Dialogue
#define idh_vp_edit_external				10627	//-- Video Post - Edit External Dialogue
#define idh_vp_edit_loop					10628	//-- Video Post - Edit Loop Dialogue
#define idh_vp_missing_maps					10629	//-- Video Post - Missing Maps Dialogue

// Rolf's IDs
#define idh_dialog_trackview_ort			11000 
#define idh_dialog_trackview_timepaste		11010 
#define idh_dialog_trackview_trackpaste		11020 
#define idh_dialog_trackview_reducekeys		11030 
#define idh_dialog_trackview_filters		11040 
#define idh_dialog_trackview_pick			11050 
#define idh_dialog_trackview_choosecontrol	11060 

#define idh_dialog_timeslide_createkey		11070 

#define idh_dialog_pref_control_defaults	11080 
#define idh_dialog_pref_select_class		11090 

#define idh_dialog_snapshot					11100 
#define idh_dialog_align					11110 
#define idh_dialog_mirror					11120 
#define idh_dialog_normal_align				11130 
#define idh_dialog_view_align				11135

#define idh_dialog_render_params			11140 
#define idh_dialog_render_presetsize		11160 

#define idh_dialog_atmos					11170 
#define idh_dialog_atmos_create				11180 

#define idh_dialog_medit_browser			11190 
#define idh_dialog_medit_puttolib			11200 

#define idh_dialog_buttonsets				11210 

#define idh_dialog_xform_typein				11220
#define idh_dialog_display_floater			11225

#define idh_dialog_effect					11230 
#define idh_dialog_effect_create			11240 

#define idh_dialog_obj_xref					11250	// Object xref dialog
#define idh_dialog_scene_xref				11260   // Scene xref dialog

#define idh_colorclip_floater				11270   // RB 12/07/2000: color clip board utility

#define idh_colorclip_floater				11270   // RB 12/07/2000: color clip board utility


// layer ids
#define idh_tb_layer_current_button			11500
#define idh_tb_layer_prop_button			11510
#define idh_tb_layer_list					11520
#define idh_tb_geom_prop_button				11530

//layer property dialog 
#define idh_dialog_layer_general			11540
#define idh_dialog_layer_radiosity			11560
#define idh_dialog_layer_manager			11570
#define idh_dialog_layer_prop				11580

// Schematic view ids...
#define idh_dialog_schematicview_filters		11600

// CCJ Dialog ids
#define idh_dialog_file_properties			11700
#define idh_ramplayer_window				11710

// DWG/DXF import and export dialog ids
#define idh_dwgexp_options					11750	// DWG/DXF Export options
#define idh_dwgimp_options					11760	// DWG/DXF Import options (tabbed)
#define idh_dwgimp_geometry_page			11761
#define idh_dwgimp_layers_page				11762
#define idh_dwgimp_rendering_page			11763

// Prs dialog ids
#define idh_3dsexp_export					11800	// 3DS Export Dialog

#define idh_3dsimp_import					11820	// 3DS Import Dialog

#define idh_master_unit_dialog				11825	// "system unit setup" dialog
// (idh_pref_system_units is in this dialog but defined above)

#define idh_3dsimp_ls_import           11830 // LS importer
#define idh_3dsimp_lp_import           11831 // LP importer
#define idh_3dsexp_lp_export           11832 // LP exporter

#define idh_vrmlexp_export					11854	// VRML97 Export Dialog

#define idh_vrblout_export					11877	// VRML/VRBL Export Dialog

#define idh_vrmlimp_import					11890	// VRML Import Dialog

#define idh_max2obj_export					11892	// Guruware .obj Export Dialog

#define idh_obj2max_import					11894	// Guruware .obj Import Dialog

#define idh_dynamics_maindlg			11900	// Dynamics main Dialog
#define idh_dynamics_time					11901	// Dynamics time Dialog
#define idh_dynamics_editobj			11902	//  Dynamics edit object Dialog
#define idh_dynamics_editobjlist			11903	// Dynamics edit object listDialog
#define idh_dynamics_collidelist			11904	// Dynamics collision list Dialog
#define idh_dynamics_effectslist			11905	// Dynamics effects list Dialog

#define idh_nurbs_editcrvonsrf				12001	// Edit Curve On Surface Dialog
#define idh_nurbs_selectbyname				12002	// Select Sub-Objects Dialog
#define idh_nurbs_edittexturesurf			12003	// Edit Texture Surface Dialog
#define idh_nurbs_makeloft					12004	// Make Loft Dialog
#define idh_nurbs_selectbyid				12005	// Select by material ID dialog
#define idh_nurbs_convertsurface			12006	// Convert Surface dialog
#define idh_nurbs_convertcurve				12007	// Convert Curve dialog
#define idh_nurbs_detach					12008	// Detach dialog
#define idh_nurbs_reparameterize			12009	// reparameterize dialog
#define idh_nurbs_makepointsurf				12010	// make point surface dialog
#define idh_nurbs_rebuildcvsurf				12011	// rebuild CV Surface dialog
#define idh_nurbs_rebuildtexturesurf		12012	// rebuild texture surface dialog
#define idh_nurbs_toolbox					12013	// toolbox dialog
#define idh_nurbs_advancedsurfapprox		12014	// advanced surface approximation dialog
#define idh_nurbs_rebuildcvcurv				12015	// rebuild CV Curv dialog 

#define idh_parameter_wiring_dialog			12500	// paramteer wiring dialog

#define idh_dialog_assembly_create			12520 // Create assembly dialog. No control level granularity

#define idh_radiosity_lighting_analysis     12540   // Radiosity Lighting Analysis dialog

#define idh_render_presets_control			12550	// Render presets control

// Help ids for restrictions that apply for actively linked objects
#ifdef LINKEDOBJ_NO_LAYER_ASSIGNMENT
#define idh_linkedobj_no_layer_assignment 12560
#endif
#ifdef LINKEDOBJ_NO_DELETE
#define idh_linkedobj_no_delete 12561
#endif
#ifdef LINKEDOBJ_NO_CLONE
#define idh_linkedobj_no_clone 12562
#endif
#ifdef LINKEDOBJ_NO_UNLINK_NODE
#define idh_linkedobj_no_unlink_mode 12563
#endif
#ifdef LINKEDOBJ_NO_COLLAPSE
#define idh_linkedobj_no_collapse 12564
#endif
#ifdef LINKEDOBJ_OK_TO_APPLY_TOPODEPENDENT_MODIFIER
#define idh_linkedobj_ok_to_apply_topodependent_modifier 12565
#endif
// Help ids for restrictions on substituting objects
#ifdef SUBSTOBJ_NO_DELETE
#define idh_substobj_no_delete 12566
#endif
#ifdef SUBSTOBJ_NO_CLONE
#define idh_substobj_no_clone 12567
#endif
#ifdef SUBSTOBJ_NO_LINK_UNLINK_NODE
#define idh_substobj_no_link_unlink_mode 12568
#endif

#define idh_tool_palette_set_general									12569
#define idh_tool_palette_set_view_options								12570
#define idh_tool_palette_properties										12571

#define idh_mtl_replace                                                 12572
#define idh_mtl_convert                                                 12573

// CA - 5/15/03 - Help id for fix ambient utility
#define idh_fix_ambient									12600

// aszabo|May.18.04|Help id of the Clone & Align (Batch Insert) tool
#define idh_clone_align_tool							12605

// aszabo|Feb.09.04|Help id of the "Stale RefID Problem" help topic
//#define idh_stale_refid_problem					12605

// Scott Morrison: Help ID for the Athens communications center.
#define idh_communication_center							12606

// CCJ: Help ID for the DWF Exporter.
#define idh_dwf_export										12610


// Peter Watje Unwrap Dialog IDs
#define idh_unwrap_edit										13000
#define idh_unwrap_options									13010
#define idh_unwrap_splinemap								13020
#define idh_unwrap_peltmap									13030
#define idh_unwrap_stitch									13040
#define idh_unwrap_sketch									13050
#define idh_unwrap_relax									13060
#define idh_unwrap_renderuvw								13070
#define idh_unwrap_flattenmap								13080
#define idh_unwrap_normalmap								13090
#define idh_unwrap_unfoldmap								13100

// Susan Amkraut: Lighting Analysis Assistant Dialog IDs
#define idh_dialog_lighting_analysis		14000	// main box
#define idh_lightanal_lighting_page			14001	// lighting page
#define idh_lightanal_materials_page		14001	// materials page
#define idh_lightanal_analysis_page			14001	// analysis page
#define idh_lightanal_general_page			14001	// general page

#define idh_viewcube_help					15000
#define idh_steerwheel_help					15001

#define id_massfx_tools_world				15010
#define id_massfx_tools_tools				15011
#define id_massfx_tools_edit				15012
#define id_massfx_tools_display				15013
#define id_massfx_modrb_prop				15014
#define id_massfx_modrb_mat					15015
#define id_massfx_modrb_meshes				15016
#define id_massfx_modrb_advanced			15017
#define id_massfx_modrb_meshparam_sphere	15018
#define id_massfx_modrb_meshparam_box		15019
#define id_massfx_modrb_meshparam_capsule	15020
#define id_massfx_modrb_meshparam_convex	15021
#define id_massfx_modrb_meshparam_custom	15022
#define id_massfx_modrb_meshparam_composite	15023
#define id_massfx_skeleton_general			15024
#define id_massfx_skeleton_associated_skin	15025
#define id_massfx_uconstraints_connection	15026
#define id_massfx_uconstraints_transunit	15027
#define id_massfx_uconstraints_limits		15028
#define id_massfx_uconstraints_spring		15029
#define id_massfx_uconstraints_advanced		15030

//
// Any chnage to this file will require an upate of HelpsysContextIDtoString.cpp
//
