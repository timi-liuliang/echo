/**********************************************************************
 *<
	FILE: iEPoly.h

	DESCRIPTION:   Editable Polygon Mesh Object SDK Interface

	CREATED BY: Steve Anderson

	HISTORY: created March 2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "iFnPub.h"
#include "point3.h"
#include "mesh.h"
#include "mnmesh.h"
#include "color.h"
// forward declarations
class MNMesh;
class INodeTab;
class IParamBlock2;


// Editable Poly Selection Levels
enum ePolySelLevel { EP_SL_OBJECT, EP_SL_VERTEX, EP_SL_EDGE, 
	EP_SL_BORDER, EP_SL_FACE, EP_SL_ELEMENT, EP_SL_CURRENT };

// Editable Poly Parameters:
enum epolyParameters { ep_by_vertex, ep_ignore_backfacing, ep_show_normals,
	ep_normal_size, ep_ss_use, ep_ss_retro, ep_ss_edist_use, ep_ss_edist,
	ep_ss_affect_back, ep_ss_falloff, ep_ss_pinch, ep_ss_bubble,
	ep_extrusion_type, ep_split, ep_refine_ends, ep_weld_threshold,
	ep_weld_pixels, ep_ms_smoothness, ep_ms_smooth, ep_ms_sep_smooth,
	ep_ms_sep_mat, ep_tess_type, ep_tess_tension, ep_surf_subdivide,
	ep_surf_ig_sel, ep_surf_iter, ep_surf_thresh, ep_surf_riter,
	ep_surf_rthresh, ep_surf_use_riter, ep_surf_use_rthresh,
	ep_surf_sep_smooth, ep_surf_sep_mat, ep_surf_update,
	ep_vert_sel_color, ep_vert_selc_r, ep_vert_selc_g, ep_vert_selc_b,
	ep_face_smooth_thresh, ep_vert_color_selby,
	ep_sd_use, ep_sd_split_mesh, ep_sd_method, ep_sd_tess_steps,
	ep_sd_tess_edge, ep_sd_tess_distance, ep_sd_tess_angle, ep_sd_view_dependent,
	ep_asd_style, ep_asd_min_iters, ep_asd_max_iters, ep_asd_max_tris,
	ep_surf_subdiv_smooth, ep_face_extrude_height, ep_bevel_outline,
	ep_vertex_chamfer, ep_cut_start_level, ep_cut_start_index,
	ep_cut_start_coords, ep_cut_end_coords, ep_cut_normal,
	ep_interactive_full, ep_vertex_extrude_width, ep_lift_angle, ep_lift_edge, 
	ep_lift_segments, ep_connect_edge_segments, ep_slide_edge_direction,
	ep_slide_edge_start, ep_extrude_spline_node, ep_extrude_spline_segments,
	ep_extrude_spline_taper, ep_extrude_spline_taper_curve, ep_extrude_spline_twist,
	ep_extrude_spline_rotation, ep_inset, ep_inset_type, ep_constrain_type,
	ep_extrude_spline_align, ep_show_cage, ep_bevel_height, ep_vertex_extrude_height,
	ep_edge_extrude_height, ep_edge_extrude_width, ep_outline, ep_edge_chamfer, 
	ep_bevel_type, ep_edge_weld_threshold, ep_surf_isoline_display,
	ep_delete_isolated_verts, ep_preserve_maps,

	// Paint tools
	ep_ss_lock,
	ep_paintsel_value, ep_paintsel_size, ep_paintsel_strength,
	ep_paintdeform_value, ep_paintdeform_size, ep_paintdeform_strength,
	ep_paintdeform_axis,

	// New parameters in 7.0:
	ep_bridge_taper, ep_bridge_bias, ep_bridge_segments, ep_bridge_smooth_thresh,
	// Target 1 and target 2 contain either edge or face indices, in 1-based indexing.
	// Value of 0 indicates not yet set.
	ep_bridge_target_1, ep_bridge_target_2, ep_bridge_twist_1, ep_bridge_twist_2,
	ep_bridge_selected,
	ep_relax_amount, ep_relax_iters, ep_relax_hold_boundary, ep_relax_hold_outer,
	ep_select_by_angle, ep_select_angle,

	//new params for 8.0
	ep_bridge_adjacent,
	ep_vertex_chamfer_open,
	ep_edge_chamfer_open,
	ep_bridge_reverse_triangle,
	ep_connect_edge_pinch,
	ep_connect_edge_slide,
	ep_ring_edge_sel,
	ep_loop_edge_sel,
	ep_vertex_break,
	ep_cage_color,
	ep_selected_cage_color,
	ep_cage_color_changed,
	ep_selected_cage_color_changed,
	ep_paintsel_mode,
	ep_paintdeform_mode,

	//new params for 9.5
	ep_edge_chamfer_segments,

	//new for 10.0
	ep_select_mode				// BOOL		-  the select mode that we are in, none, sub object, or multi.
};

// Editable Poly Operations:
enum epolyButtonOp { epop_hide, epop_unhide, epop_ns_copy, epop_ns_paste,
	epop_cap, epop_delete, epop_detach, epop_attach_list, epop_split,
	epop_break, epop_collapse, epop_reset_plane, epop_slice, epop_weld_sel,
	epop_create_shape, epop_make_planar, epop_align_grid, epop_align_view,
	epop_remove_iso_verts,
	epop_meshsmooth, epop_tessellate, epop_update, epop_selby_vc,
	epop_retriangulate, epop_flip_normals, epop_selby_matid, epop_selby_smg,
	epop_autosmooth, epop_clear_smg, epop_bevel, epop_chamfer, epop_cut,
	epop_null, epop_sel_grow, epop_sel_shrink, epop_inset, epop_extrude,
	epop_extrude_along_spline, epop_connect_edges, epop_connect_vertices,
	epop_lift_from_edge, epop_select_ring, epop_select_loop,
	epop_remove_iso_map_verts, epop_remove, epop_outline,
	epop_toggle_shaded_faces, epop_hide_unsel,

	// New operations in 7.0:
	epop_bridge_border, epop_bridge_polygon, epop_make_planar_x, epop_make_planar_y,
	epop_make_planar_z, epop_relax,

	// New operation in 8.0
	epop_bridge_edge,
	//11.0
	epop_selby_matidfloater,	// select id coming from the floater
	//12.0
	epop_preserve_uv_settings	// Launch Preserve UV Settings Dialog
};

// Editable Poly Command modes:
enum epolyCommandMode { epmode_create_vertex, epmode_create_edge,
	epmode_create_face, epmode_divide_edge, epmode_divide_face,
	epmode_extrude_vertex, epmode_extrude_edge,
	epmode_extrude_face, epmode_chamfer_vertex, epmode_chamfer_edge,
	epmode_bevel, epmode_sliceplane, epmode_cut_vertex, epmode_cut_edge,
	epmode_cut_face, epmode_weld, epmode_edit_tri, epmode_inset_face,
	epmode_quickslice, epmode_lift_from_edge, epmode_pick_lift_edge,
	epmode_outline,

	// New command modes in 7.0:
	epmode_bridge_border, epmode_bridge_polygon,
	epmode_pick_bridge_1, epmode_pick_bridge_2,
	epmode_turn_edge,

	// new command modes in 8.0
	epmode_bridge_edge,

	//! New edit soft selection command mode for 3ds max2009
	epmode_edit_ss
};

// Editable Poly Pick modes:
enum epolyPickMode { epmode_attach, epmode_pick_shape };

// ---- from here on, everything is used in Editable Poly, but not in Edit Poly ----

// Enum our enums, for use in the function publishing interface description
enum epolyEnumList { epolyEnumButtonOps, epolyEnumCommandModes, epolyEnumPickModes,
	ePolySelLevel, PMeshSelLevel, axisEnum, mapChannelEnum };

#define EPOLY_INTERFACE Interface_ID( 0x092779, 0x634020)

#define GetEPolyInterface(cd) (EPoly *)(cd)->GetInterface(EPOLY_INTERFACE)

// "Mixin" Interface methods:
enum ePolyMethods { epfn_hide, epfn_unhide_all, epfn_named_selection_copy,
	epfn_named_selection_paste, epfn_create_vertex, epfn_create_edge,
	epfn_create_face, epfn_cap_holes, epfn_delete, epfn_attach, epfn_multi_attach,
	epfn_detach_to_element, epfn_detach_to_object, epfn_split_edges,
	epfn_break_verts, epfn_divide_face, epfn_divide_edge,
	epfn_collapse, epfn_extrude_faces, epfn_bevel_faces,
	epfn_chamfer_vertices, epfn_chamfer_edges, epfn_slice, epfn_cut_vertex,
	epfn_cut_edge, epfn_cut_face, epfn_weld_verts, epfn_weld_edges,
	epfn_weld_flagged_verts, epfn_weld_flagged_edges, epfn_create_shape,
	epfn_make_planar, epfn_move_to_plane, epfn_align_to_grid, epfn_align_to_view,
	epfn_delete_iso_verts, epfn_meshsmooth,
	epfn_tessellate, epfn_set_diagonal, epfn_retriangulate,
	epfn_flip_normals, epfn_select_by_mat, epfn_select_by_smooth_group,
	epfn_autosmooth, epfn_in_slice_plane_mode, epfn_force_subdivision,
	epfn_button_op, epfn_toggle_command_mode, epfn_enter_pick_mode,
	epfn_exit_command_modes,
	// Flag Accessors:
	epfn_get_vertices_by_flag, epfn_get_edges_by_flag, epfn_get_faces_by_flag,
	epfn_set_vertex_flags, epfn_set_edge_flags, epfn_set_face_flags,
	// Data accessors:
	epfn_reset_slice_plane, epfn_set_slice_plane, epfn_get_slice_plane,
	epfn_get_vertex_color, epfn_set_vertex_color, epfn_get_face_color,
	epfn_set_face_color, epfn_get_vertex_data, epfn_get_edge_data,
	epfn_set_vertex_data, epfn_set_edge_data, epfn_reset_vertex_data,
	epfn_reset_edge_data, epfn_begin_modify_perdata, 
	epfn_in_modify_perdata, epfn_end_modify_perdata,
	epfn_begin_modify_vertex_color, epfn_in_modify_vertex_color,
	epfn_end_modify_vertex_color,
	epfn_get_mat_index, epfn_set_mat_index,
	epfn_get_smoothing_groups, epfn_set_smoothing_groups,
	// other things...
	epfn_collapse_dead_structs, /*epfn_apply_delta,*/ epfn_propagate_component_flags,

	// New for Luna:
	// Preview mode access:
	epfn_preview_clear, epfn_preview_begin, epfn_preview_cancel, epfn_preview_accept,
	epfn_preview_invalidate, epfn_preview_on, epfn_preview_set_dragging, epfn_preview_get_dragging,

	// Popup dialog access:
	epfn_popup_dialog, epfn_close_popup_dialog,

	// Other new stuff:
	epfn_repeat_last, epfn_grow_selection, epfn_shrink_selection, epfn_convert_selection,
	epfn_select_border, epfn_select_element, epfn_select_edge_loop, epfn_select_edge_ring,
	epfn_remove, epfn_delete_iso_map_verts, epfn_outline, epfn_connect_edges,
	epfn_connect_vertices, epfn_extrude_along_spline, epfn_lift_from_edge,
	epfn_toggle_shaded_faces,

	// Access to selections:
	epfn_get_epoly_sel_level, epfn_get_mn_sel_level, epfn_get_selection, epfn_set_selection,

	// Access to information about mesh:
	epfn_get_num_vertices, epfn_get_vertex, epfn_get_vertex_face_count, epfn_get_vertex_face,
	epfn_get_vertex_edge_count, epfn_get_vertex_edge, epfn_get_num_edges, epfn_get_edge_vertex,
	epfn_get_edge_face, epfn_get_num_faces, epfn_get_face_degree, epfn_get_face_vertex,
	epfn_get_face_edge, epfn_get_face_material, epfn_get_face_smoothing_group,
	epfn_get_num_map_channels, epfn_get_map_channel_active,
	epfn_get_num_map_vertices, epfn_get_map_vertex, epfn_get_map_face_vertex,

	// All right all ready
	epfn_collapse_dead_structs_spelled_right, epfn_get_map_face_vert,

	// New published functions in 7.0:
	epfn_get_preserve_map, epfn_set_preserve_map,
	epfn_bridge, epfn_ready_to_bridge_selected, epfn_turn_diagonal,
	epfn_relax, epfn_make_planar_in,

	// new function in 8.0 
	epfn_chamfer_vertices_open,
	epfn_chamfer_edges_open,
	epfn_set_ring_shift,
	epfn_set_loop_shift,
	epfn_convert_selection_to_border,
	epfn_paintdeform_commit,
	epfn_paintdeform_cancel,

	epfn_set_cache_systemon,epfn_get_cache_systemoff,
	epfn_set_cache_suspend_dxcache,epfn_get_cache_suspend_dxcache,
	epfn_set_cache_cutoff_count,epfn_get_cache_cutoff_count,

	epfn_get_command_mode,
	epfn_get_pick_mode,

	epfn_matid_floater,epfn_matid_floatervisible,
	epfn_smgrp_floater,epfn_smgrp_floatervisible
};

/*! \sa  Class FPMixinInterface,  Class MNMesh, Class IParamBlock2, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_edge_data_index_options.html">List of Edge Data Channels</a>, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_vertex_data_index_options.html">List of Vertex Data Channels</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The EPoly class is the main interface to the Editable Poly Object. This class
is a virtual class with no data members. More details can be found in the SDK
samples under <b>/MAXSDK/SAMPLES/MESH/EDITABLEPOLY</b>.
\par Method Groups:
See <a href="class_e_poly_groups.html">Method Groups for Class EPoly</a>.
*/
#pragma warning(push)
#pragma warning(disable:4100)
class EPoly : public FPMixinInterface {
public:
	#pragma warning(push)
	#pragma warning(disable:4238)
	BEGIN_FUNCTION_MAP
		FN_2(epfn_hide, TYPE_bool, EpfnHide, TYPE_INT, TYPE_DWORD);
		FN_1(epfn_unhide_all, TYPE_bool, EpfnUnhideAll, TYPE_INT);
		VFN_1(epfn_named_selection_copy, EpfnNamedSelectionCopy, TYPE_STRING);
		VFN_1(epfn_named_selection_paste, EpfnNamedSelectionPaste, TYPE_bool);
		FN_3(epfn_create_vertex, TYPE_INDEX, EpfnCreateVertex, TYPE_POINT3, TYPE_bool, TYPE_bool);
		FN_3(epfn_create_edge, TYPE_INDEX, EpfnCreateEdge, TYPE_INDEX, TYPE_INDEX, TYPE_bool);
		FN_2(epfn_create_face, TYPE_INDEX, EpfnCreateFace2, TYPE_INDEX_TAB, TYPE_bool);	// switching in createFace2, because CreateFace wasn't working here.
		FN_2(epfn_cap_holes, TYPE_bool, EpfnCapHoles, TYPE_INT, TYPE_DWORD);
		FN_3(epfn_delete, TYPE_bool, EpfnDelete, TYPE_INT, TYPE_DWORD, TYPE_bool);
		VFNT_2(epfn_attach, EpfnAttach, TYPE_INODE, TYPE_INODE);
		//VFNT_2(epfn_multi_attach, EpfnMultiAttach, TYPE_INODE_TAB, TYPE_INODE);
		FN_3(epfn_detach_to_element, TYPE_bool, EpfnDetachToElement, TYPE_INT, TYPE_DWORD, TYPE_bool);
		//FNT_5(epfn_detach_to_object, TYPE_bool, EpfnDetachToObject, TYPE_STRING, TYPE_INT, TYPE_DWORD, TYPE_bool, TYPE_INODE_BP);
		FN_1(epfn_split_edges, TYPE_bool, EpfnSplitEdges, TYPE_DWORD);
		FN_1(epfn_break_verts, TYPE_bool, EpfnBreakVerts, TYPE_DWORD);
		FN_3(epfn_divide_face, TYPE_INDEX, EpfnDivideFace, TYPE_INDEX, TYPE_FLOAT_TAB_BR, TYPE_bool);
		FN_3(epfn_divide_edge, TYPE_INDEX, EpfnDivideEdge, TYPE_INDEX, TYPE_FLOAT, TYPE_bool);
		FN_2(epfn_collapse, TYPE_bool, EpfnCollapse, TYPE_INT, TYPE_DWORD);
		VFNT_2(epfn_extrude_faces, EpfnExtrudeFaces, TYPE_FLOAT, TYPE_DWORD);
		VFNT_3(epfn_bevel_faces, EpfnBevelFaces, TYPE_FLOAT, TYPE_FLOAT, TYPE_DWORD);
		VFNT_1(epfn_chamfer_vertices, EpfnChamferVertices, TYPE_FLOAT);
		VFNT_1(epfn_chamfer_edges, EpfnChamferEdges, TYPE_FLOAT);
		FN_4(epfn_slice, TYPE_bool, EpfnSlice, TYPE_POINT3, TYPE_POINT3, TYPE_bool, TYPE_DWORD);
		FN_0(epfn_in_slice_plane_mode, TYPE_bool, EpfnInSlicePlaneMode);
		FN_3(epfn_cut_vertex, TYPE_INDEX, EpfnCutVertex, TYPE_INDEX, TYPE_POINT3, TYPE_POINT3);
		FN_5(epfn_cut_edge, TYPE_INDEX, EpfnCutEdge, TYPE_INDEX, TYPE_FLOAT, TYPE_INDEX, TYPE_FLOAT, TYPE_POINT3);
		FN_4(epfn_cut_face, TYPE_INDEX, EpfnCutFace, TYPE_INDEX, TYPE_POINT3, TYPE_POINT3, TYPE_POINT3);
		FN_3(epfn_weld_verts, TYPE_bool, EpfnWeldVerts, TYPE_INDEX, TYPE_INDEX, TYPE_POINT3);
		FN_2(epfn_weld_edges, TYPE_bool, EpfnWeldEdges, TYPE_INDEX, TYPE_INDEX);
		FN_1(epfn_weld_flagged_verts, TYPE_bool, EpfnWeldFlaggedVerts, TYPE_DWORD);
		FN_1(epfn_weld_flagged_edges, TYPE_bool, EpfnWeldFlaggedEdges, TYPE_DWORD);
		FN_4(epfn_create_shape, TYPE_bool, EpfnCreateShape, TYPE_STRING, TYPE_bool, TYPE_INODE, TYPE_DWORD);
		FNT_2(epfn_make_planar, TYPE_bool, EpfnMakePlanar, TYPE_INT, TYPE_DWORD);
		FNT_4(epfn_move_to_plane, TYPE_bool, EpfnMoveToPlane, TYPE_POINT3, TYPE_FLOAT, TYPE_INT, TYPE_DWORD);
		FN_2(epfn_align_to_grid, TYPE_bool, EpfnAlignToGrid, TYPE_INT, TYPE_DWORD);
		FN_2(epfn_align_to_view, TYPE_bool, EpfnAlignToView, TYPE_INT, TYPE_DWORD);
		FN_0(epfn_delete_iso_verts, TYPE_bool, EpfnDeleteIsoVerts);
		FN_2(epfn_meshsmooth, TYPE_bool, EpfnMeshSmooth, TYPE_INT, TYPE_DWORD );
		FN_2(epfn_tessellate, TYPE_bool, EpfnTessellate, TYPE_INT, TYPE_DWORD );
		VFN_0(epfn_force_subdivision, EpfnForceSubdivision);
		VFN_3(epfn_set_diagonal, EpfnSetDiagonal, TYPE_INDEX, TYPE_INDEX, TYPE_INDEX);
		FN_1(epfn_retriangulate, TYPE_bool, EpfnRetriangulate, TYPE_DWORD);
		FN_1(epfn_flip_normals, TYPE_bool, EpfnFlipNormals, TYPE_DWORD);
		VFNT_2(epfn_select_by_mat, EpfnSelectByMat, TYPE_INDEX, TYPE_bool);
		VFNT_2(epfn_select_by_smooth_group, EpfnSelectBySmoothGroup, TYPE_DWORD, TYPE_bool);
		VFNT_0(epfn_autosmooth, EpfnAutoSmooth);

		VFN_1(epfn_button_op, EpActionButtonOp, TYPE_ENUM);
		VFN_1(epfn_toggle_command_mode, EpActionToggleCommandMode, TYPE_ENUM);
		FN_0(epfn_get_command_mode, TYPE_ENUM, EpActionGetCommandMode);
		VFN_1(epfn_enter_pick_mode, EpActionEnterPickMode, TYPE_ENUM);
		FN_0(epfn_get_pick_mode, TYPE_ENUM, EpActionGetPickMode);
		VFN_0(epfn_exit_command_modes, EpActionExitCommandModes);

		// Flag accessors:
		FN_3(epfn_get_vertices_by_flag, TYPE_bool, EpGetVerticesByFlag, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD);
		FN_3(epfn_get_edges_by_flag, TYPE_bool, EpGetEdgesByFlag, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD);
		FN_3(epfn_get_faces_by_flag, TYPE_bool, EpGetFacesByFlag, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD);

		VFN_4(epfn_set_vertex_flags, EpSetVertexFlags, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD, TYPE_bool);
		VFN_4(epfn_set_edge_flags, EpSetEdgeFlags, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD, TYPE_bool);
		VFN_4(epfn_set_face_flags, EpSetFaceFlags, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD, TYPE_bool);

		// Data accessors:
		VFN_0(epfn_reset_slice_plane, EpResetSlicePlane);
		VFN_3(epfn_get_slice_plane, EpGetSlicePlane, TYPE_POINT3_BR, TYPE_POINT3_BR, TYPE_FLOAT_BP);
		VFN_3(epfn_set_slice_plane, EpSetSlicePlane, TYPE_POINT3_BR, TYPE_POINT3_BR, TYPE_FLOAT);
		FNT_4(epfn_get_vertex_data, TYPE_FLOAT, GetVertexDataValue, TYPE_INT, TYPE_INT_BP, TYPE_bool_BP, TYPE_DWORD);
		FNT_4(epfn_get_edge_data, TYPE_FLOAT, GetEdgeDataValue, TYPE_INT, TYPE_INT_BP, TYPE_bool_BP, TYPE_DWORD);
		VFNT_3(epfn_set_vertex_data, SetVertexDataValue, TYPE_INT, TYPE_FLOAT, TYPE_DWORD);
		VFNT_3(epfn_set_edge_data, SetEdgeDataValue, TYPE_INT, TYPE_FLOAT, TYPE_DWORD);
		VFN_1(epfn_reset_vertex_data, ResetVertexData, TYPE_INT);
		VFN_1(epfn_reset_edge_data, ResetEdgeData, TYPE_INT);
		VFN_2(epfn_begin_modify_perdata, BeginPerDataModify, TYPE_INT, TYPE_INT);
		FN_0(epfn_in_modify_perdata, TYPE_bool, InPerDataModify);
		VFN_1(epfn_end_modify_perdata, EndPerDataModify, TYPE_bool);
		FN_1(epfn_get_vertex_color, TYPE_COLOR_BV, EpFnGetVertexColor, TYPE_ENUM);
		VFN_2(epfn_set_vertex_color, EpFnSetVertexColor, TYPE_COLOR, TYPE_ENUM); //color map channel
		FN_1(epfn_get_face_color, TYPE_COLOR_BV, EpFnGetFaceColor, TYPE_ENUM);
		VFN_2(epfn_set_face_color, EpFnSetFaceColor, TYPE_COLOR, TYPE_ENUM); //color map channel
		//VFN_1(epfn_begin_modify_vertex_color, BeginVertexColorModify, TYPE_INT);
		//FN_0(epfn_in_modify_vertex_color, TYPE_bool, InVertexColorModify);
		//VFN_1(epfn_end_modify_vertex_color, EndVertexColorModify, TYPE_bool);
		FN_1(epfn_get_mat_index, TYPE_INDEX, GetMatIndex, TYPE_bool_BP);
		VFN_2(epfn_set_mat_index, SetMatIndex, TYPE_INDEX, TYPE_DWORD);
		VFN_3(epfn_get_smoothing_groups, GetSmoothingGroups, TYPE_DWORD, TYPE_DWORD_BP, TYPE_DWORD_BP);
		VFN_3(epfn_set_smoothing_groups, SetSmoothBits, TYPE_DWORD, TYPE_DWORD, TYPE_DWORD);

		VFN_0(epfn_collapse_dead_structs, CollapseDeadStructs);
		VFN_0(epfn_collapse_dead_structs_spelled_right, CollapseDeadStructs);
//		VFNT_2(epfn_apply_delta, ApplyDelta, TYPE_POINT3_TAB_BR, TYPE_IOBJECT); // no type corresponding to Tab<Point3>
		FN_7(epfn_propagate_component_flags, TYPE_INT, EpfnPropagateComponentFlags, TYPE_INT, TYPE_DWORD, TYPE_INT, TYPE_DWORD, TYPE_bool, TYPE_bool, TYPE_bool);

		VFN_1 (epfn_preview_begin, EpPreviewBegin, TYPE_ENUM);
		VFN_0 (epfn_preview_cancel, EpPreviewCancel);
		VFN_0 (epfn_preview_accept, EpPreviewAccept);
		VFN_0 (epfn_preview_invalidate, EpPreviewInvalidate);
		FN_0 (epfn_preview_on, TYPE_bool, EpPreviewOn);
		VFN_1 (epfn_preview_set_dragging, EpPreviewSetDragging, TYPE_bool);
		FN_0 (epfn_preview_get_dragging, TYPE_bool, EpPreviewGetDragging);

		FN_1 (epfn_popup_dialog, TYPE_bool, EpfnPopupDialog, TYPE_ENUM);
		VFN_0 (epfn_close_popup_dialog, EpfnClosePopupDialog);

		VFN_0 (epfn_repeat_last, EpfnRepeatLastOperation);
		VFN_1 (epfn_grow_selection, EpfnGrowSelection, TYPE_ENUM);
		VFN_1 (epfn_shrink_selection, EpfnShrinkSelection, TYPE_ENUM);
		FN_3 (epfn_convert_selection, TYPE_INT, EpfnConvertSelection, TYPE_ENUM, TYPE_ENUM, TYPE_bool);
		VFN_0 (epfn_select_border, EpfnSelectBorder);
		VFN_0 (epfn_select_element, EpfnSelectElement);
		VFN_0 (epfn_select_edge_loop, EpfnSelectEdgeLoop);
		VFN_0 (epfn_select_edge_ring, EpfnSelectEdgeRing);
		FN_2 (epfn_remove, TYPE_bool, EpfnRemove, TYPE_ENUM, TYPE_DWORD);
		FN_0 (epfn_delete_iso_map_verts, TYPE_bool, EpfnDeleteIsoMapVerts);
		FN_1 (epfn_outline, TYPE_bool, EpfnOutline, TYPE_DWORD);
		FN_1 (epfn_connect_edges, TYPE_bool, EpfnConnectEdges, TYPE_DWORD);
		FN_1 (epfn_connect_vertices, TYPE_bool, EpfnConnectVertices, TYPE_DWORD);
		FN_1 (epfn_extrude_along_spline, TYPE_bool, EpfnExtrudeAlongSpline, TYPE_DWORD);
		FN_1 (epfn_lift_from_edge, TYPE_bool, EpfnLiftFromEdge, TYPE_DWORD);
		VFN_0 (epfn_toggle_shaded_faces, EpfnToggleShadedFaces);

		FN_0 (epfn_get_epoly_sel_level, TYPE_ENUM, GetEPolySelLevel);
		FN_0 (epfn_get_mn_sel_level, TYPE_ENUM, GetMNSelLevel);

		FN_1 (epfn_get_selection, TYPE_BITARRAY, EpfnGetSelection, TYPE_ENUM);
		VFN_2 (epfn_set_selection, EpfnSetSelection, TYPE_ENUM, TYPE_BITARRAY);

		FN_0 (epfn_get_num_vertices, TYPE_INT, EpfnGetNumVertices);
		FN_1 (epfn_get_vertex, TYPE_POINT3_BV, EpfnGetVertex, TYPE_INDEX);
		FN_1 (epfn_get_vertex_face_count, TYPE_INT, EpfnGetVertexFaceCount, TYPE_INDEX);
		FN_2 (epfn_get_vertex_face, TYPE_INDEX, EpfnGetVertexFace, TYPE_INDEX, TYPE_INDEX);
		FN_1 (epfn_get_vertex_edge_count, TYPE_INT, EpfnGetVertexEdgeCount, TYPE_INDEX);
		FN_2 (epfn_get_vertex_edge, TYPE_INDEX, EpfnGetVertexEdge, TYPE_INDEX, TYPE_INDEX);

		FN_0 (epfn_get_num_edges, TYPE_INT, EpfnGetNumEdges);
		FN_2 (epfn_get_edge_vertex, TYPE_INDEX, EpfnGetEdgeVertex, TYPE_INDEX, TYPE_INDEX);
		FN_2 (epfn_get_edge_face, TYPE_INDEX, EpfnGetEdgeFace, TYPE_INDEX, TYPE_INDEX);

		FN_0 (epfn_get_num_faces, TYPE_INT, EpfnGetNumFaces);
		FN_1 (epfn_get_face_degree, TYPE_INT, EpfnGetFaceDegree, TYPE_INDEX);
		FN_2 (epfn_get_face_vertex, TYPE_INDEX, EpfnGetFaceVertex, TYPE_INDEX, TYPE_INDEX);
		FN_2 (epfn_get_face_edge, TYPE_INDEX, EpfnGetFaceEdge, TYPE_INDEX, TYPE_INDEX);
		FN_1 (epfn_get_face_material, TYPE_INDEX, EpfnGetFaceMaterial, TYPE_INDEX);
		FN_1 (epfn_get_face_smoothing_group, TYPE_DWORD, EpfnGetFaceSmoothingGroup, TYPE_INDEX);

		FN_0 (epfn_get_num_map_channels, TYPE_INT, EpfnGetNumMapChannels);
		FN_1 (epfn_get_map_channel_active, TYPE_bool, EpfnGetMapChannelActive, TYPE_INT);
		FN_1 (epfn_get_num_map_vertices, TYPE_INT, EpfnGetNumMapVertices, TYPE_INT);
		FN_2 (epfn_get_map_vertex, TYPE_POINT3_BV, EpfnGetMapVertex, TYPE_INT, TYPE_INDEX);
		FN_3 (epfn_get_map_face_vertex, TYPE_INT, EpfnGetMapFaceVertex, TYPE_INT, TYPE_INDEX, TYPE_INDEX);
		FN_3 (epfn_get_map_face_vert, TYPE_INDEX, EpfnGetMapFaceVertex, TYPE_INT, TYPE_INDEX, TYPE_INDEX);

		// New functionality in 7.0:
		FN_2 (epfn_bridge, TYPE_bool, EpfnBridge, TYPE_ENUM, TYPE_DWORD);
		FN_2 (epfn_ready_to_bridge_selected, TYPE_bool, EpfnReadyToBridgeFlagged, TYPE_ENUM, TYPE_DWORD);
		FN_2 (epfn_turn_diagonal, TYPE_bool, EpfnTurnDiagonal, TYPE_INDEX, TYPE_INDEX);
		FNT_2 (epfn_relax, TYPE_bool, EpfnRelax, TYPE_ENUM, TYPE_DWORD);
		FNT_3 (epfn_make_planar_in, TYPE_bool, EpfnMakePlanarIn, TYPE_ENUM, TYPE_ENUM, TYPE_DWORD);

		FN_1(epfn_get_preserve_map, TYPE_bool, EpfnGetPreserveMap, TYPE_INT);
		VFN_2(epfn_set_preserve_map, EpfnSetPreserveMap, TYPE_INT, TYPE_bool);

		//Added in Max 8 - Removed from EPoly8
		VFNT_2(epfn_chamfer_vertices_open, EpfnChamferVerticesOpen, TYPE_FLOAT,TYPE_bool);
		VFNT_2(epfn_chamfer_edges_open, EpfnChamferEdgesOpen, TYPE_FLOAT,TYPE_bool);
		VFN_3(epfn_set_ring_shift, EpfnSetRingShift, TYPE_INT, TYPE_bool, TYPE_bool);
		VFN_3(epfn_set_loop_shift, EpfnSetLoopShift, TYPE_INT, TYPE_bool, TYPE_bool);
		FN_2 (epfn_convert_selection_to_border, TYPE_INT, EpfnConvertSelectionToBorder, TYPE_ENUM, TYPE_ENUM);
		VFN_0(epfn_paintdeform_commit,EpfnPaintDeformCommit);
		VFN_0(epfn_paintdeform_cancel,EpfnPaintDeformCancel);

		VFN_0(epfn_smgrp_floater, SmGrpFloater);
		FN_0(epfn_smgrp_floatervisible,TYPE_BOOL, SmGrpFloaterVisible);
		
		VFN_0(epfn_matid_floater, MatIDFloater);
		FN_0(epfn_matid_floatervisible,TYPE_BOOL, MatIDFloaterVisible);
		
	END_FUNCTION_MAP
	#pragma warning(pop)

	/*! \remarks This method returns a description of the published function
	interface (for scripter access to EPoly functions). */
	FPInterfaceDesc *GetDesc ();

	// General interface tools for accessing polymesh, updating object.
	/*! \remarks This method returns a pointer to the Editable Poly's
	<b>MNMesh</b>. See Class MNMesh for fun things to do with an MNMesh.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual MNMesh* GetMeshPtr () { return NULL; }
	/*! \remarks Gets a pointer to the Editable Poly's parameter block, which
	can be used to get or set Editable Poly UI parameters. (See class
	IParamBlock2 for more information on access methods.) Here is a list of the
	available parameters, as defined in the epolyParameters enum near the top
	of <b>iEPoly.h</b>. (Please see the Editable Poly documentation in the
	normal 3ds Max User Reference file for more information on these
	parameters.)\n\n
	<b>Selection dialog parameters:</b>\n\n
	<b>ep_by_vertex</b>: select by vertex (checkbox)\n\n
	<b>ep_ignore_backfacing</b>: Ignore backfacing (checkbox)\n\n
	<b>ep_show_normals</b>: Currently unused.\n\n
	<b>ep_normal_size</b>: Currently unused.\n\n
	<b>Soft Selection dialog parameters:</b>\n\n
	<b>ep_ss_use</b>: Use soft selection (checkbox)\n\n
	<b>ep_ss_retro</b>: Currently unused\n\n
	<b>ep_ss_edist_use</b>: Use edge-based distances to compute soft selection
	(checkbox)\n\n
	<b>ep_ss_edist</b>: Maximum number of edges to traverse in computing
	edge-based distances (int spinner - range: 1-999999)\n\n
	<b>ep_ss_affect_back</b>: Affect backfacing in soft selection
	(checkbox)\n\n
	<b>ep_ss_falloff</b>: Falloff value for soft selection (float spinner)\n\n
	<b>ep_ss_pinch</b>: Pinch value for soft selection (float spinner)\n\n
	<b>ep_ss_bubble</b>: Bubble value for soft selection (float spinner)\n\n
	<b>Edit Geometry dialog parameters:</b>\n\n
	<b>ep_extrusion_type</b>: Type of face extrusion. (Radio.) Values:\n\n
	<b>0</b>: Extrude by group (cluster)\n\n
	<b>1</b>: Extrude by local normals\n\n
	<b>2</b>: Extrude by polygon - each polygon extrudes separately.\n\n
	<b>ep_split</b>: Controls whether the Cut algorithm splits the mesh open.
	(Checkbox)\n\n
	<b>ep_refine_ends</b>: Currently unused\n\n
	<b>ep_weld_threshold</b>: Threshold for welding selected vertices. (float
	spinner)\n\n
	<b>ep_weld_pixels</b>: Pixel threshold for Target welding (int spinner)\n\n
	<b>Subdivide dialog parameters:</b>\n\n
	<b>ep_ms_smoothness</b>: Smoothness value for MeshSmooth type subdivision
	(float spinner, range 0-1)\n\n
	<b>ep_ms_smooth</b>: Currently unused.\n\n
	<b>ep_ms_sep_smooth</b>: Separate by smoothing groups for MeshSmooth type
	subdivision (checkbox)\n\n
	<b>ep_ms_sep_mat</b>: Separate by material IDs for MeshSmooth type
	subdivision (checkbox)\n\n
	<b>ep_tess_type</b>: Tessellation type (Radio) Values:\n\n
	<b>0</b>: by edge\n\n
	<b>1</b>: by face.\n\n
	<b>ep_tess_tension</b>: Tessellation tension (float spinner)\n\n
	<b>Surface Properties dialog parameters (object level):</b>\n\n
	<b>ep_surf_subdivide</b>: Apply NURMS-style MeshSmooth subdivision to
	polymesh. (checkbox)\n\n
	<b>ep_surf_subdiv_smooth</b>: Apply MeshSmooth smoothing group algorithm
	after subdividing (checkbox)\n\n
	<b>ep_surf_ig_sel</b>: Currently unused.\n\n
	<b>ep_surf_iter</b>: Number of iterations of subdivision (int spinner)\n\n
	<b>ep_surf_thresh</b>: "Smoothness" threshold for adaptive subdivision
	(float spinner, range 0-1)\n\n
	<b>ep_surf_riter</b>: Render value of iterations (int spinner)\n\n
	<b>ep_surf_rthresh</b>: Render value of smoothness threshold (float
	spinner, range 0-1)\n\n
	<b>ep_surf_use_riter</b>: Use Render iterations value when rendering
	(checkbox)\n\n
	<b>ep_surf_use_rthresh</b>: Use Render smoothness threshold when rendering
	(checkbox)\n\n
	<b>ep_surf_sep_smooth</b>: Separate by smoothing groups in subdivision
	(checkbox)\n\n
	<b>ep_surf_sep_mat</b>: Separate by materials in subdivision (checkbox)\n\n
	<b>ep_surf_update</b>: Update type for subdivision (radio). Values:\n\n
	<b>0</b>: Update always\n\n
	<b>1</b>: Update when rendering\n\n
	<b>2</b>: Update manually\n\n
	<b>Surface Properties dialog parameters (vertex level):</b>\n\n
	<b>ep_vert_sel_color</b>: Target color for select-by-color (color
	swatch)\n\n
	<b>ep_vert_selc_r</b>: Tolerance of red values in select-by-color. (int
	spinner, range 0-255)\n\n
	<b>ep_vert_selc_g</b>: Tolerance of green values in select-by-color. (int
	spinner, range 0-255)\n\n
	<b>ep_vert_selc_b</b>: Tolerance of blue values in select-by-color. (int
	spinner, range 0-255)\n\n
	<b>ep_vert_color_selby</b>: Which kind of color to select by in
	select-by-color. Radio values:\n\n
	<b>0</b>: Select by regular vertex color\n\n
	<b>1</b>: Select by vertex illumination\n\n
	<b>Surface Properties dialog parameters (face level):</b>\n\n
	<b>ep_face_smooth_thresh</b>: Autosmooth threshold (float spinner, angle
	units).\n\n
	<b>Subdivision Displacement parameters (object level):</b>\n\n
	<b>ep_sd_use</b>: Apply subdivision displacement (checkbox)\n\n
	<b>ep_sd_split_mesh</b>: Split the mesh (checkbox)\n\n
	<b>ep_sd_method</b>: Subdivision displacement method (radio). Values:\n\n
	<b>0</b>: Regular method\n\n
	<b>1</b>: Spatial method\n\n
	<b>2</b>: Curvature method\n\n
	<b>3</b>: Use both spatial \& curvature methods.\n\n
	<b>ep_sd_tess_steps</b>: Tessellation steps (for regular method) (int
	spinner)\n\n
	<b>ep_sd_tess_edge</b>: Edge size (for spatial method) (float spinner)\n\n
	<b>ep_sd_tess_distance</b>: Distance (for curvature method) (float
	spinner)\n\n
	<b>ep_sd_tess_angle</b>: Angle value (for curvature method) (float
	spinner)\n\n
	<b>ep_sd_view_dependent</b>: View dependency (checkbox)\n\n
	<b>Advanced Subdivision Displacement parameters:</b>\n\n
	<b>ep_asd_style</b>: Subdivision style. Radio values:\n\n
	<b>0</b>: Grid-based\n\n
	<b>1</b>: Tree-based\n\n
	<b>2</b>: Delauney algorithm.\n\n
	<b>ep_asd_min_iters</b>: Minimum number of iterations (grid or tree style)
	(int spinner)\n\n
	<b>ep_asd_max_iters</b>: Maximum number of iterations (grid or tree style)
	(int spinner)\n\n
	<b>ep_asd_max_tris</b>: Maximum number of triangles (Delauney style) (int
	spinner)
	\par Default Implementation:
	<b>{ return NULL; }</b>\n\n
	  */
	virtual IParamBlock2	*getParamBlock () { return NULL; }
	/*! \remarks This method is used to indicate to the EPoly that some parts
	of its mesh have changed. This is automatically handled by most EPoly
	methods; you only need to use it if you're directly manipulating the mesh
	yourself.
	\par Parameters:
	<b>DWORD parts</b>\n\n
	Parts of the mesh that have been changed, such as <b>PART_GEOM</b>,
	<b>PART_TOPO</b>, etc.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void			LocalDataChanged (DWORD parts) { }

	// Invalidates soft selection values:
	/*! \remarks This method invalidates soft selection values in the mesh and
	in any cached data.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void InvalidateSoftSelectionCache () { }

	// Invalidates precomputed distances on which soft selection values are based
	// (as well as soft selection values) - note this is automatically done when
	// you call LocalDataChanged (PART_GEOM).  Should also be done if the soft
	// selection parameters are changed.
	/*! \remarks This method invalidates pre-computed distances on which soft
	selection values are based (as well as soft selection values) - note this
	is automatically done when you call <b>LocalDataChanged (PART_GEOM)</b>.
	This should also be done if the soft selection parameters are changed.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void InvalidateDistanceCache () { }

	// Simple call to ip->RedrawViewports.
	/*! \remarks This is a handy method that does a simple call to
	<b>ip-\>RedrawViewports</b>. Also updates the named selection dropdown
	list. Also, if the EPoly project has been compiled as a debug build, it
	will verify that the MNMesh is free of errors using the
	<b>MNMesh::CheckAllData</b> method.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void RefreshScreen () { }

	// returns TRUE if between BeginEditParams, EndEditParams
	/*! \remarks This method indicates if the Editable Poly object is
	currently being edited in the modifier panel (and has its UI present).
	\par Default Implementation:
	<b>{ return FALSE; }</b>\n\n
	  */
	virtual bool Editing () { return FALSE; }

	// Selection level accessors:
	/*! \remarks This method returns the EPoly selection level, as defined by
	the <b>ePolySelLevel enum</b>: One of the following values;
	<b>EP_SL_OBJECT, EP_SL_VERTEX, EP_SL_EDGE, EP_SL_BORDER, EP_SL_FACE,
	EP_SL_ELEMENT</b>.
	\par Default Implementation:
	<b>{ return EP_SL_OBJECT; }</b> */
	virtual int  GetEPolySelLevel() { return EP_SL_OBJECT; }
	/*! \remarks This method returns the MNMesh's selection level, as defined
	by the <b>PMeshSelLevel enum</b> in <b>MNMesh.h</b>: one of the following;
	<b>MNM_SL_OBJECT, MNM_SL_VERTEX, MNM_SL_EDGE, MNM_SL_FACE</b>. (Note that
	the Editable Poly selection levels <b>EP_SL_BORDER</b> and
	<b>EP_SL_ELEMENT</b> are considered varieties of <b>MNM_SL_EDGE</b> and
	<b>MNM_SL_FACE</b> selection types, respectively.)
	\par Default Implementation:
	<b>{ return MNM_SL_OBJECT; }</b> */
	virtual int  GetMNSelLevel () { return MNM_SL_OBJECT; }
	/*! \remarks This method sets the Editable Poly selection level.
	\par Parameters:
	<b>int level</b>\n\n
	Possible values are defined by the <b>ePolySelLevel enum</b>: one of
	<b>EP_SL_OBJECT, EP_SL_VERTEX, EP_SL_EDGE, EP_SL_BORDER, EP_SL_FACE,
	EP_SL_ELEMENT</b>.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void SetEPolySelLevel(int level) { }

	// Component flag accessors:
	/*! \remarks This method fills in a <b>BitArray</b> depending on whether
	or not each <b>MNVert</b> in the <b>MNMesh</b> has a particular flag or set
	of flags set or cleared.\n\n
	Example: for instance to set the <b>BitArray</b> according to selected
	vertices, you'd just call <b>EpGetVerticesByFlag (vset, MN_SEL)</b>. But to
	find vertices which do not have the <b>MN_DEAD</b> flag set, but which do
	have the <b>MN_WHATEVER</b> flag set, you'd call <b>EpGetVerticesByFlag
	(vset, MN_WHATEVER, MN_WHATEVER|MN_DEAD)</b>.
	\par Parameters:
	<b>BitArray \& vset</b>\n\n
	The array for output to be stored in. The <b>vset</b> will be set to size
	of the number of verts in the mesh.\n\n
	<b>DWORD flags</b>\n\n
	The flags we're looking for in the vertices\n\n
	<b>DWORD fmask=0x0</b>\n\n
	The mask of flags we're checking. This is automatically or'd with "flags".
	\return  TRUE if successful, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpGetVerticesByFlag (BitArray & vset, DWORD flags, DWORD fmask=0x0) { return false; }
	/*! \remarks This method fills in a <b>BitArray</b> depending on whether
	or not each <b>MNEdge</b> in the <b>MNMesh</b> has a particular flag or set
	of flags set or cleared.\n\n
	Example: for instance to set the <b>BitArray</b> according to selected
	edges, you'd just call <b>EpGetEdgesByFlag (eset, MN_SEL)</b>. But to find
	edges which do not have the <b>MN_DEAD</b> flag set, but which do have the
	<b>MN_WHATEVER</b> flag set, you'd call <b>EpGetEdgesByFlag (eset,
	MN_WHATEVER, MN_WHATEVER|MN_DEAD)</b>.
	\par Parameters:
	<b>BitArray \& vset</b>\n\n
	The array for output to be stored in. eset will be set to size of the
	number of edges in the mesh.\n\n
	<b>DWORD flags</b>\n\n
	The flags we're looking for in the edges\n\n
	<b>DWORD fmask=0x0</b>\n\n
	The mask of flags we're checking. This is automatically or'd with "flags".
	\return  TRUE if successful, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpGetEdgesByFlag (BitArray & eset, DWORD flags, DWORD fmask=0x0) { return false; }
	/*! \remarks This method fills in a <b>BitArray</b> depending on whether
	or not each <b>MNFace</b> in the <b>MNMesh</b> has a particular flag or set
	of flags set or cleared.\n\n
	Example: for instance to set the <b>BitArray</b> according to selected
	faces, you'd just call <b>EpGetFacesByFlag (fset, MN_SEL)</b>. But to find
	faces which do not have the <b>MN_DEAD</b> flag set, but which do have the
	<b>MN_WHATEVER</b> flag set, you'd call <b>EpGetFacesByFlag (fset,
	MN_WHATEVER, MN_WHATEVER|MN_DEAD)</b>.
	\par Parameters:
	<b>BitArray \& vset</b>\n\n
	The array for output to be stored in. fset will be set to size of the
	number of faces in the mesh.\n\n
	<b>DWORD flags</b>\n\n
	The flags we're looking for in the faces\n\n
	<b>DWORD fmask=0x0</b>\n\n
	The mask of flags we're checking. This is automatically or'd with "flags".
	\return  TRUE if successful, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpGetFacesByFlag (BitArray & fset, DWORD flags, DWORD fmask=0x0) { return false; }
	/*! \remarks This method sets <b>MNVert</b> flags based on a
	<b>BitArray</b>.\n\n
	Example: to hide vertices specified by the <b>BitArray</b>, you'd just call
	<b>EpSetVertexFlags (vset, MN_HIDE)</b>. To unhide the vertices, you'd use
	<b>EpSetVertexFlags (vset, 0, MN_HIDE)</b>.
	\par Parameters:
	<b>BitArray \& vset</b>\n\n
	This bitarray indicates which vertices should have their flags
	modified.\n\n
	<b>DWORD flags</b>\n\n
	The flags to set.\n\n
	<b>DWORD fmask=0x0</b>\n\n
	The flag mask - if it includes bits not in "flags", those bits are cleared
	in the specified vertices.\n\n
	<b>bool undoable=true</b>\n\n
	If <b>(undoable \&\& theHold.Holding())</b>, a restore object for this flag
	change will be added to the current undo stack.
	\par Default Implementation:
	<b>{ return; }</b> */
	virtual void EpSetVertexFlags (BitArray &vset, DWORD flags, DWORD fmask=0x0, bool undoable=true) { return; }
	/*! \remarks This method sets <b>MNEdge</b> flags based on a
	<b>BitArray</b>.\n\n
	Example: to select edges specified by the <b>BitArray</b>, you'd just call
	<b>EpSetEdgeFlags (eset, MN_SEL)</b>. To clear selection on the edges,
	you'd use <b>EpSetEdgeFlags (eset, 0, MN_SEL)</b>.
	\par Parameters:
	<b>BitArray \& vset</b>\n\n
	This bitarray indicates which edges should have their flags modified.\n\n
	<b>DWORD flags</b>\n\n
	The flags to set.\n\n
	<b>DWORD fmask=0x0</b>\n\n
	The flag mask - if it includes bits not in "flags", those bits are cleared
	in the specified edges.\n\n
	<b>bool undoable=true</b>\n\n
	If <b>(undoable \&\& theHold.Holding())</b>, a restore object for this flag
	change will be added to the current undo stack.
	\par Default Implementation:
	<b>{ return; }</b> */
	virtual void EpSetEdgeFlags (BitArray &eset, DWORD flags, DWORD fmask = 0x0, bool undoable=true) { return; }
	/*! \remarks This method sets <b>MNFace</b> flags based on a
	<b>BitArray</b>.\n\n
	Example: to hide faces specified by the <b>BitArray</b>, you'd just call
	<b>EpSetFaceFlags (fset, MN_HIDE)</b>. To unhide the faes, you'd use
	<b>EpSetFaceFlags (fset, 0, MN_HIDE)</b>.
	\par Parameters:
	<b>BitArray \& vset</b>\n\n
	This bitarray indicates which faces should have their flags modified.\n\n
	<b>DWORD flags</b>\n\n
	The flags to set.\n\n
	<b>DWORD fmask=0x0</b>\n\n
	The flag mask - if it includes bits not in "flags", those bits are cleared
	in the specified faces.\n\n
	<b>bool undoable=true</b>\n\n
	If <b>(undoable \&\& theHold.Holding())</b>, a restore object for this flag
	change will be added to the current undo stack.
	\par Default Implementation:
	<b>{ return; }</b>\n\n
	  */
	virtual void EpSetFaceFlags (BitArray &fset, DWORD flags, DWORD fmask = 0x0, bool undoable=true) { return; }

	// Action interface stuff:
	/*! \remarks Completes the action corresponding to the specified UI
	button.
	\par Parameters:
	<b>int opcode</b>\n\n
	The list of "button operations" is defined by the <b>epolyButtonOp
	enum</b>.\n\n
	Select dialog button operations:\n\n
	<b>epop_hide</b>: Hide current selection\n\n
	<b>epop_unhide</b>: Unhide current selection\n\n
	<b>epop_ns_copy</b>: Copy named selection (brings up UI)\n\n
	<b>epop_ns_paste</b>: Paste named selection (may bring up UI)\n\n
	Edit Geometry dialog button operations:\n\n
	<b>epop_cap</b>: Cap currently selected borders\n\n
	<b>epop_delete</b>: Delete current selection.\n\n
	<b>epop_detach</b>: Detach current selection.\n\n
	<b>epop_attach_list</b>: Attach any number of nodes using the attach by
	name dialog.\n\n
	<b>epop_split</b>: Split currently selected edges.\n\n
	<b>epop_break</b>: Break currently selected vertices\n\n
	<b>epop_collapse</b>: Collapse current selection\n\n
	<b>epop_reset_plane</b>: Reset the slice plane\n\n
	<b>epop_slice</b>: Slice\n\n
	<b>epop_weld_sel</b>: Weld current vertex or edge selection\n\n
	<b>epop_create_shape</b>: Create a shape from current edge selection
	(brings up UI)\n\n
	<b>epop_make_planar</b>: Make current selection planar\n\n
	<b>epop_align_grid</b>: Align current selection to construction grid\n\n
	<b>epop_align_view</b>: Align current selection at right angles to
	view.\n\n
	<b>epop_remove_iso_verts</b>: Remove isolated vertices\n\n
	Subdivide dialog button operations:\n\n
	<b>epop_meshsmooth</b>: Subdivide by MeshSmooth (NURMS Style)\n\n
	<b>epop_tessellate</b>: Subdivide by Tessellation\n\n
	Surface Properties dialog button operations:\n\n
	Object Level:\n\n
	<b>epop_update</b>: Update MeshSmooth subdivision\n\n
	Vertex Level:\n\n
	<b>epop_selby_vc</b>: Select by vertex color\n\n
	Face Level:\n\n
	<b>epop_retriangulate</b>: Retriangulate currently selected faces\n\n
	<b>epop_flip_normals</b>: Flip normals on currently selected elements\n\n
	<b>epop_selby_matid</b>: Select faces by Material ID (brings up UI)\n\n
	<b>epop_selby_smg</b>: Select faces by smoothing groups (brings up UI)\n\n
	<b>epop_autosmooth</b>: Autosmooth currently selected faces\n\n
	<b>epop_clear_smg</b>: Clear all smoothing groups on currently selected
	faces.\n\n

	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpActionButtonOp (int opcode) { }
	/*! \remarks If the user is currently in the specified command mode, this
	method causes them to exit it. If the user is not, this method will enter
	it.
	\par Parameters:
	<b>int mode</b>\n\n
	The list of command modes is defined by the <b>epolyCommandMode enum</b>
	and is given here for reference. (Their names are self-explanatory.)\n\n
	<b>epmode_create_vertex</b>\n\n
	<b>epmode_create_edge</b>\n\n
	<b>epmode_create_face</b>\n\n
	<b>epmode_divide_edge</b>\n\n
	<b>epmode_divide_face</b>\n\n
	<b>epmode_extrude_vertex</b> (note: currently inactive)\n\n
	<b>epmode_extrude_edge</b> (note: currently inactive)\n\n
	<b>epmode_extrude_face</b>\n\n
	<b>epmode_chamfer_vertex</b>\n\n
	<b>epmode_chamfer_edge</b>\n\n
	<b>epmode_bevel</b>\n\n
	<b>epmode_sliceplane</b>\n\n
	<b>epmode_cut_vertex</b>\n\n
	<b>epmode_cut_edge</b>\n\n
	<b>epmode_cut_face</b>\n\n
	<b>epmode_weld</b>\n\n
	<b>epmode_edit_tri</b>\n\n

	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpActionToggleCommandMode (int mode) { }
	/*! \remarks This method enters the specified pick mode, which is like a
	command mode but relates to picking nodes.
	\par Parameters:
	<b>int mode</b>\n\n
	Currently there is only one pick mode supported by EPoly, which is defined
	in the <b>epolyPickMode enum: epmode_attach</b>, which allows the user to
	pick a node to attach to this Editable Poly object.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpActionEnterPickMode (int mode) { }
	/*! \remarks This method exits from any command mode the system currently
	may be in.
	\par Default Implementation:
	<b>{ }</b>\n\n
	  */
	virtual void EpActionExitCommandModes () { }

	///  \brief 
	/// Retrieves the current EPoly command mode in use, if any.
	/// 
	///  \return  The current command mode from the EpActionToggleCommandMode above, or -1 if the current
	/// command mode is not any of the EPoly command modes. 
	virtual int EpActionGetCommandMode () { return -1; }

	///  \brief 
	/// Retrieves the current EPoly pick mode in use, if any.
	/// 
	///  \return  The current pick mode (from the EpActionEnterPickMode above), or -1 if the current
	/// pick mode is not any of the EPoly pick modes. 
	virtual int EpActionGetPickMode () { return -1; }

	// Transform commands:
	/*! \remarks This method moves the current selection (including any soft
	selection) by the specified amount in the specified coordinate system.
	\par Parameters:
	<b>int level</b>\n\n
	The <b>enum ePolySelLevel</b>, being one of the following values;
	<b>EP_SL_OBJECT, EP_SL_VERTEX, EP_SL_EDGE, EP_SL_BORDER, EP_SL_FACE,
	EP_SL_ELEMENT</b>.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to apply the move operation.\n\n
	<b>Matrix3\& partm</b>\n\n
	The parent transformation matrix.\n\n
	<b>Matrix3\& tmAxis</b>\n\n
	The transformation axis.\n\n
	<b>Point3\& val</b>\n\n
	The vector describing the translation.\n\n
	<b>BOOL localOrigin</b>\n\n
	TRUE to move based on the local origin, otherwise FALSE.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void MoveSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin) { }
	/*! \remarks This method rotates the current selection (including any soft
	selection) by the specified amount in the specified coordinate system.
	\par Parameters:
	<b>int level</b>\n\n
	The <b>enum ePolySelLevel</b>, being one of the following values;
	<b>EP_SL_OBJECT, EP_SL_VERTEX, EP_SL_EDGE, EP_SL_BORDER, EP_SL_FACE,
	EP_SL_ELEMENT</b>.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to apply the rotate operation.\n\n
	<b>Matrix3\& partm</b>\n\n
	The parent transformation matrix.\n\n
	<b>Matrix3\& tmAxis</b>\n\n
	The transformation axis.\n\n
	<b>Quat\& val</b>\n\n
	The rotation quaternion.\n\n
	<b>BOOL localOrigin</b>\n\n
	TRUE to rotate based on the local origin, otherwise FALSE.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void RotateSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin) { }
	/*! \remarks This method scales the current selection (including any soft
	selection) by the specified amount in the specified coordinate system.
	\par Parameters:
	<b>int level</b>\n\n
	The <b>enum ePolySelLevel</b>, being one of the following values;
	<b>EP_SL_OBJECT, EP_SL_VERTEX, EP_SL_EDGE, EP_SL_BORDER, EP_SL_FACE,
	EP_SL_ELEMENT</b>.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to apply the rotate operation.\n\n
	<b>Matrix3\& partm</b>\n\n
	The parent transformation matrix.\n\n
	<b>Matrix3\& tmAxis</b>\n\n
	The transformation axis.\n\n
	<b>Point3\& val</b>\n\n
	The scaling value.\n\n
	<b>BOOL localOrigin</b>\n\n
	TRUE to scale based on the local origin, otherwise FALSE.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void ScaleSelection(int level, TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin) { }
	/*! \remarks This method applies a geometric "delta" vector to the current
	mesh at the specified time. (Note: if t!=0 and the system's animate feature
	is on, this will set keys.)
	\par Parameters:
	<b>Tab\<Point3\> \& delta</b>\n\n
	The table of geometry delta vectors.\n\n
	<b>EPoly *epol</b>\n\n
	A pointer to the editable poly object to apply to.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to apply the geometric delta.
	\par Default Implementation:
	<b>{ }</b>\n\n
	  */
	virtual void ApplyDelta (Tab<Point3> & delta, EPoly *epol, TimeValue t) { }

	// Slice plane accessors:
	/*! \remarks This method resets the slice plane.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpResetSlicePlane () { }
	/*! \remarks This method returns the slice plane, as defined by its
	normal, center, and size. (Size is irrelevant for slicing, but defines the
	size of the slice gizmo the user sees.)
	\par Parameters:
	<b>Point3 \& planeNormal</b>\n\n
	The plane normal vector.\n\n
	<b>Point3 \& planeCenter</b>\n\n
	The plane center.\n\n
	<b>float *planeSize=NULL</b>\n\n
	The size of the plane.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpGetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, float *planeSize=NULL) { }
	/*! \remarks This method sets the slice plane to have the specified
	normal, center, and size. (Unlike in <b>EpGetSlicePlane()</b>, size is not
	an optional argument here.)
	\par Parameters:
	<b>Point3 \& planeNormal</b>\n\n
	The plane normal vector.\n\n
	<b>Point3 \& planeCenter</b>\n\n
	The plane center.\n\n
	<b>float planeSize</b>\n\n
	The size of the plane.
	\par Default Implementation:
	<b>{ }</b>\n\n
	  */
	virtual void EpSetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, float planeSize) { }

	// Component data access methods:
	/*! \remarks This method obtains the vertex color for the flagged vertices
	in the indicated map channel.
	\par Parameters:
	<b>bool *uniform=NULL</b>\n\n
	If non-NULL, the bool this pointer points to is set to true if all flagged
	vertices have the identical color, and false otherwise. (It's set to true
	if there are 0 vertices.)\n\n
	<b>int *num=NULL</b>\n\n
	If non-NULL, the number of vertices currently flagged is computed and
	stored here.\n\n
	<b>int mp=0</b>\n\n
	The map channel we're using. Most vertex color applications use the
	standard vertex color channel, 0. However, you can also use this method
	with the Illumination channel (<b>MAP_SHADING</b> = -1) or the alpha
	channel (<b>MAP_ALPHA</b> = -2) - or even with a regular UVW map channel
	(1-99).\n\n
	<b>DWORD flag=MN_SEL</b>\n\n
	This indicates the vertices we look at. If left at the default, selected
	vertices' colors are analyzed. If flag were to equal <b>MN_WHATEVER</b>,
	then vertices with the <b>MN_WHATEVER</b> flag would have their colors
	analyzed.\n\n
	<b>TimeValue t=0</b>\n\n
	This is not currently used.
	\return  The color of the flagged vertices, or black (0,0,0) if the
	vertices' colors are not the same.
	\par Default Implementation:
	<b>{ return Color(1,1,1); }</b> */
	virtual Color GetVertexColor (bool* uniform = NULL, int* num = NULL, int mp = 0, DWORD flag = MN_SEL, TimeValue t = 0) { return Color(1,1,1); }
	/*! \remarks This method sets the vertex color for the flagged vertices in
	the indicated map channel.
	\par Parameters:
	<b>Color clr</b>\n\n
	The color to set the vertices to.\n\n
	<b>int mp=0</b>\n\n
	The map channel we're using. Most vertex color applications use the
	standard vertex color channel, 0. However, you can also use this method
	with the Illumination channel (<b>MAP_SHADING</b> = -1) or the alpha
	channel (<b>MAP_ALPHA</b> = -2) - or even with a regular UVW map channel
	(1-99).\n\n
	<b>DWORD flag=MN_SEL</b>\n\n
	This indicates which vertices we set. If left at the default, selected
	vertices' colors are set. If flag were to equal <b>MN_WHATEVER</b>, then
	vertices with the <b>MN_WHATEVER</b> flag would have their colors set.\n\n
	<b>TimeValue t=0</b>\n\n
	This is not currently used.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void  SetVertexColor (Color clr, int mp=0, DWORD flag=MN_SEL, TimeValue t=0) { }

	/*! \remarks This method obtains the face color for the flagged faces in
	the indicated map channel.
	\par Parameters:
	<b>bool *uniform=NULL</b>\n\n
	If non-NULL, the bool this pointer points to is set to true if all flagged
	faces have the identical color, and false otherwise. (It's set to true if
	there are 0 flagged faces.)\n\n
	<b>int *num=NULL</b>\n\n
	If non-NULL, the number of faces currently flagged is computed and stored
	here.\n\n
	<b>int mp=0</b>\n\n
	The map channel we're using. Most vertex color applications use the
	standard vertex color channel, 0. However, you can also use this method
	with the Illumination channel (<b>MAP_SHADING</b> = -1) or the alpha
	channel (<b>MAP_ALPHA</b> = -2) - or even with a regular UVW map channel
	(1-99).\n\n
	<b>DWORD flag=MN_SEL</b>\n\n
	This indicates which vertices we set. If left at the default, selected
	vertices' colors are set. If flag were to equal <b>MN_WHATEVER</b>, then
	vertices with the <b>MN_WHATEVER</b> flag would have their colors set.\n\n
	<b>TimeValue t=0</b>\n\n
	This is not currently used.
	\return  The color of the flagged faces, or black (0,0,0) if the faces'
	colors are not the same.
	\par Default Implementation:
	<b>{ return Color(1,1,1); }</b> */
	virtual Color GetFaceColor (bool *uniform=NULL, int *num=NULL, int mp=0, DWORD flag=MN_SEL, TimeValue t=0) { return Color(1,1,1); }
	/*! \remarks This method sets the vertex color for the flagged faces in
	the indicated map channel.
	\par Parameters:
	<b>Color clr</b>\n\n
	The color to set the faces to.\n\n
	<b>int mp=0</b>\n\n
	The map channel we're using. Most vertex color applications use the
	standard vertex color channel, 0. However, you can also use this method
	with the Illumination channel (<b>MAP_SHADING</b> = -1) or the alpha
	channel (<b>MAP_ALPHA</b> = -2) - or even with a regular UVW map channel
	(1-99).\n\n
	<b>DWORD flag=MN_SEL</b>\n\n
	This indicates which vertices we set. If left at the default, selected
	vertices' colors are set. If flag were to equal <b>MN_WHATEVER</b>, then
	vertices with the <b>MN_WHATEVER</b> flag would have their colors set.\n\n
	<b>TimeValue t=0</b>\n\n
	This is not currently used.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void  SetFaceColor (Color clr, int mp=0, DWORD flag=MN_SEL, TimeValue t=0) { }
	/*! \remarks This method obtains floating-point vertex data from the
	flagged vertices in the specified vertex data channel.
	\par Parameters:
	<b>int channel</b>\n\n
	The vertex data channel we're querying. See the
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_vertex_data_index_options.html">List of Vertex Data
	Channels</a> (which are defined in <b>mesh.h</b>).\n\n
	<b>int *numSel</b>\n\n
	If non-NULL, this is filled in with the current number of flagged
	vertices.\n\n
	<b>bool *uniform</b>\n\n
	If non-NULL, this is set to indicate whether the currently flagged vertices
	have uniform values or not.\n\n
	<b>DWORD vertexFlags</b>\n\n
	Indicates which vertices to evaluate. (Use <b>MN_SEL</b> to get vertex data
	from selected vertices.)\n\n
	<b>TimeValue t</b>\n\n
	This is not currently used.
	\return  The vertex data value for the flagged vertices. If the vertices'
	values vary, the first value found is returned.
	\par Default Implementation:
	<b>{ return 1.0f; }</b> */
	virtual float GetVertexDataValue (int channel, int *numSel, bool *uniform, DWORD vertexFlags, TimeValue t) { return 1.0f; }
	/*! \remarks This method obtains floating-point edge data from the flagged
	edges in the specified edge data channel.
	\par Parameters:
	<b>int channel</b>\n\n
	The edge data channel we're querying. See the
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_edge_data_index_options.html">List of Edge Data
	Channels</a> (which are defined in <b>mnmesh.h</b>).\n\n
	<b>int *numSel</b>\n\n
	If non-NULL, this is filled in with the current number of flagged
	edges.\n\n
	<b>bool *uniform</b>\n\n
	If non-NULL, this is set to indicate whether the currently flagged edges
	have uniform values or not.\n\n
	<b>DWORD edgeFlags</b>\n\n
	Indicates which edges to evaluate. (Use <b>MN_SEL</b> to get vertex data
	from selected vertices.)\n\n
	<b>TimeValue t</b>\n\n
	This is not currently used.
	\return  The edge data value for the flagged edges. If the edges' values
	vary, the first value found is returned.
	\par Default Implementation:
	<b>{ return 1.0f; }</b> */
	virtual float GetEdgeDataValue (int channel, int *numSel, bool *uniform, DWORD edgeFlags, TimeValue t) { return 1.0f; }
	/*! \remarks This method sets floating-point vertex data for the flagged
	vertices in the specified vertex data channel.
	\par Parameters:
	<b>int channel</b>\n\n
	The vertex data channel we're modifying. See the
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_vertex_data_index_options.html">List of Vertex Data
	Channels</a> (which are defined in <b>mesh.h</b>).\n\n
	<b>float w</b>\n\n
	The value to set the flagged vertices to.\n\n
	<b>DWORD vertexFlags</b>\n\n
	Indicates which vertices to modify. (Use <b>MN_SEL</b> to set vertex data
	in selected vertices.)\n\n
	<b>TimeValue t</b>\n\n
	This is not currently used.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void  SetVertexDataValue (int channel, float w, DWORD vertexFlags, TimeValue t) { }
	/*! \remarks This method sets floating-point edge data for the flagged
	edges in the specified edge data channel.
	\par Parameters:
	<b>int channel</b>\n\n
	The edge data channel we're modifying. See the
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_edge_data_index_options.html">List of Edge Data
	Channels</a>(which are defined in <b>mnmesh.h</b>).\n\n
	<b>float w</b>\n\n
	The value to set the flagged edges to.\n\n
	<b>DWORD edgeFlags</b>\n\n
	Indicates which edges to modify. (Use <b>MN_SEL</b> to set edge data in
	selected edges.)\n\n
	<b>TimeValue t</b>\n\n
	This is not currently used.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void  SetEdgeDataValue (int channel, float w, DWORD edgeFlags, TimeValue t) { }
	/*! \remarks This method resets all vertex data in the specified channel.
	For instance, <b>ResetEdgeData (VDATA_WEIGHT)</b> would reset all vertex
	weights to 1.
	\par Parameters:
	<b>int channel</b>\n\n
	The vertex data. See the
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_vertex_data_index_options.html">List of Vertex Data
	Channels</a>
	\par Default Implementation:
	<b>{ }</b> */
	virtual void  ResetVertexData (int channel) { }
	/*! \remarks This method resets all edge data in the specified channel.
	For instance, <b>ResetEdgeData (EDATA_CREASE)</b> would reset all edge
	crease values to 0.
	\par Parameters:
	<b>int channel</b>\n\n
	The edge data channel. See the
	<a href="ms-its:listsandfunctions.chm::/idx_R_list_of_edge_data_index_options.html">List of Edge Data
	Channels</a>
	\return   
	\par Default Implementation:
	<b>{ }</b> */
	virtual void  ResetEdgeData (int channel) { }
	/*! \remarks This method is used in combination with
	<b>EndPerDataModify</b> to store undo information for any vertex or edge
	data modification.
	\par Parameters:
	<b>int mnSelLevel</b>\n\n
	Set to one of <b>MNM_SL_VERTEX</b> or <b>MNM_SL_EDGE</b> for vertex or edge
	data respectively.\n\n
	<b>int channel</b>\n\n
	Indicates the channel of vertex or edge data we're modifying. For instance,
	<b>BeginPerDataModify (MNM_SL_EDGE, EDATA_CREASE)</b> would be used before
	modifying edge crease information.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void  BeginPerDataModify (int mnSelLevel, int channel) { }
	/*! \remarks This method returns true if we're between
	<b>BeginPerDataModify</b> and <b>EndPerDataModify</b> calls.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool  InPerDataModify () { return false; }
	/*! \remarks This method completes the undo object corresponding to the
	vertex or edge data modifications made since the related
	<b>BeginPerDataModify</b> call.
	\par Parameters:
	<b>bool success</b>\n\n
	If FALSE, the system restores the original vertex colors and throws away
	the undo object.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void  EndPerDataModify (bool success) { }
	/*! \remarks This method is used in combination with
	<b>EndVertexColorModify</b> to store undo information for any vertex or
	edge data modification.
	\par Parameters:
	<b>int mp=0</b>\n\n
	The map channel we're using. Most vertex color applications use the
	standard vertex color channel, 0. However, you can also use this method
	with the Illumination channel (<b>MAP_SHADING</b> = -1) or the alpha
	channel (<b>MAP_ALPHA</b> = -2) - or even with a regular UVW map channel
	(1-99).
	\par Default Implementation:
	<b>{ }</b> */
	virtual void  BeginVertexColorModify (int mp=0) { }
	/*! \remarks This method returns true if we're between
	<b>BeginVertexColorModify</b> and <b>EndVertexColorModify</b> calls.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool  InVertexColorModify () { return false; }
	/*! \remarks This method completes the undo object corresponding to the
	vertex color modifications made since the related
	<b>BeginVertexColorModify</b> call.
	\par Parameters:
	<b>bool success</b>\n\n
	If FALSE, the system restores the original vertex colors and throws away
	the undo object.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void  EndVertexColorModify (bool success) { }

	// Face level characteristics:
	/*! \remarks This method obtains the material index for the selected faces.
	\par Parameters:
	<b>bool *determined</b>\n\n
	The bool this points to (which should not be NULL) is filled with:\n\n
	<b>FALSE</b>if there are no selected faces or if selected faces have different
	material indices\n\n
	<b>TRUE</b> if at least one face is selected and all selected faces have the
	same material ID.\n\n
	\par Default Implementation:
	<b>{ determined=false; return 0; }</b> */
	virtual int GetMatIndex (bool *determined) { determined=false; return 0; }
	/*! \remarks This method sets the material index for the flagged faces.
	\par Parameters:
	<b>int index</b>\n\n
	The material index to set flagged faces to.\n\n
	<b>DWORD flag=MN_SEL</b>\n\n
	Indicates which faces should have their material IDs set.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void SetMatIndex (int index, DWORD flag=MN_SEL) { }
	/*! \remarks This method obtains smoothing group information for the
	specified faces.
	\par Parameters:
	<b>DWORD faceFlag</b>\n\n
	Indicates which faces to read smoothing group information from. If this
	value is 0, all faces are read.\n\n
	<b>DWORD *anyFaces</b>\n\n
	<b>DWORD *allFaces=NULL</b>\n\n
	These two parameters are where the output is stored. "<b>anyFaces</b>" has
	bits set that are present in any of the faces' smoothing groups.
	"<b>allFaces</b>", if non-NULL, has bits set that are present in all of the
	faces' smoothing groups. In other words, <b>anyFaces</b> or's together the
	faces' groups, while <b>allFaces</b> and's them together.
	\par Default Implementation:
	<b>{ if (anyFaces) *anyFaces = 0; if (allFaces) *allFaces = 0; }</b> */
	virtual void GetSmoothingGroups (DWORD faceFlag, DWORD *anyFaces, DWORD *allFaces=NULL) { if (anyFaces) *anyFaces = 0; if (allFaces) *allFaces = 0; }
	/*! \remarks This method sets (or clears) smoothing group bits in the
	specified faces.
	\par Parameters:
	<b>DWORD bits</b>\n\n
	The smoothing group bits to set in flagged faces.\n\n
	<b>DWORD bitmask</b>\n\n
	The smoothing group bits to clear in flagged faces\n\n
	<b>DWORD flag</b>\n\n
	Indicates which faces to set smoothing group information in. If this value
	is 0, all faces are modified.
	\par Default Implementation:
	<b>{ }</b>\n\n
	  */
	virtual void SetSmoothBits (DWORD bits, DWORD bitmask, DWORD flag) { }

	// Displacement approximation methods:
	// Copy displacement parameters from pblock to polyobject:
	/*! \remarks This method copies displacement parameters from pblock to
	polyobject.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void SetDisplacementParams () { }

	// Copy displacement parameters from polyobject to pblock:
	/*! \remarks This method copies displacement parameters from polyobject to
	pblock.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void UpdateDisplacementParams () { }

	// Engage a displacement approximation preset:
	/*! \remarks This method engages a displacement approximation preset.
	\par Parameters:
	<b>int presetNumber</b>\n\n
	The presetNumber values are either 0 (low), 1 (medium), or 2 (high).
	\par Default Implementation:
	<b>{ }</b>\n\n
	  */
	virtual void UseDisplacementPreset (int presetNumber) { }

	// Drag operations:
	/*! \remarks This method is called at the beginning of an interactive
	extrusion operation. Performs the topological extrusion.
	\par Parameters:
	<b>int msl</b>\n\n
	Indicates the MNMesh-based selection level we're extruding. (Currently,
	this must be <b>MNM_SL_FACE</b>.)\n\n
	<b>DWORD flag</b>\n\n
	Indicates the faces we're extruding.\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnBeginExtrude (int msl, DWORD flag, TimeValue t) { }
	/*! \remarks This method is called at the end of an interactive extrusion
	operation. Completes the RestoreObjects and finalizes the geometric edit.
	\par Parameters:
	<b>bool accept</b>\n\n
	If TRUE, end extrude normally. If FALSE, cancel the extrusion completely
	(undoing the original topological extrusion).\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnEndExtrude (bool accept, TimeValue t) { }
	/*! \remarks This method is used to drag the current extrusion to the
	amount specified. May be called multiple times in one session between
	<b>EpfnBeginExtrude</b> and <b>EpfnEndExtrude</b>.
	\par Parameters:
	<b>float amount</b>\n\n
	The (absolute) amount of the extrusion\n\n
	<b>TimeValue t</b>\n\n
	The current time
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnDragExtrude (float amount, TimeValue t) { }
	/*! \remarks This method is called at the beginning of an interactive
	bevel or outline operation. Performs the topological extrusion if
	necessary, and prepares certain cached data.
	\par Parameters:
	<b>int msl</b>\n\n
	Indicates the MNMesh-based selection level we're beveling. (Currently, this
	must be <b>MNM_SL_FACE</b>.)\n\n
	<b>DWORD flag</b>\n\n
	Indicates the faces we're extruding.\n\n
	<b>bool doExtrude</b>\n\n
	Indicates whether or not a topological extrusion should be done for this
	bevel. (For instance, leaving this at false you to do "outlining", or to
	adjust a previous bevel.)\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnBeginBevel (int msl, DWORD flag, bool doExtrude, TimeValue t) { }
	/*! \remarks This method is called at the end of an interactive bevel.
	Completes the <b>RestoreObjects</b> and finalizes the geometric edit.
	\par Parameters:
	<b>bool accept</b>\n\n
	If TRUE, end bevel normally. If FALSE, cancel the bevel completely (undoing
	any earlier topological extrusion).\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnEndBevel (bool accept, TimeValue t) { }
	/*! \remarks This method is used to drag the current bevel to the outline
	and height specified. May be called multiple times in one session between
	<b>EpfnBeginBevel</b> and <b>EpfnEndBevel</b>.
	\par Parameters:
	<b>float outline</b>\n\n
	The (positive or negative) outline amount for the bevel.\n\n
	<b>float height</b>\n\n
	The (positive or negative) height of the bevel.\n\n
	<b>TimeValue t</b>\n\n
	The current time
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnDragBevel (float outline, float height, TimeValue t) { }
	/*! \remarks This method is called at the beginning of an interactive
	chamfer operation. Performs the topological changes and prepares certain
	cached data.
	\par Parameters:
	<b>int msl</b>\n\n
	Indicates the MNMesh-based selection level we're chamfering. (Either
	<b>MNM_SL_VERTEX</b> or <b>MNM_SL_EDGE</b>.)\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnBeginChamfer (int msl, TimeValue t) { }
	/*! \remarks This method is called at the end of an interactive chamfer.
	Completes the <b>RestoreObjects</b> and finalizes the geometric edits.
	\par Parameters:
	<b>bool accept</b>\n\n
	If TRUE, end chamfer normally. If FALSE, cancel the chamfer completely
	(undoing the earlier topological changes).\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnEndChamfer (bool accept, TimeValue t) { }
	/*! \remarks This method is used to drag the current chamfer to the amount
	specified. May be called multiple times in one session between
	<b>EpfnBeginChamfer</b> and <b>EpfnEndChamfer</b>.
	\par Parameters:
	<b>float amount</b>\n\n
	The amount of the chamfer.\n\n
	<b>TimeValue t</b>\n\n
	The current time
	\par Default Implementation:
	<b>{ }</b>\n\n
	  */
	virtual void EpfnDragChamfer (float amount, TimeValue t) { }

	// Operations:
	// Select panel ops:
	/*! \remarks This method Hides flagged components.
	\par Parameters:
	<b>int msl</b>\n\n
	MNMesh selection level - should be either <b>MNM_SL_VERTEX</b> or
	<b>MNM_SL_FACE</b>.\n\n
	<b>DWORD flags</b>\n\n
	Indicates which components to hide. For instance, <b>MN_SEL</b> would cause
	it to hide selected vertices or faces.
	\return  TRUE if components were hidden, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnHide (int msl, DWORD flags) { return false; }
	/*! \remarks This method unhides all components at the specified level.
	\par Parameters:
	<b>int msl</b>\n\n
	MNMesh selection level - should be either <b>MNM_SL_VERTEX</b> or
	<b>MNM_SL_FACE</b>.
	\return  TRUE if components were unhidden, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnUnhideAll (int msl) { return false; }
	/*! \remarks This method copies the named selection specified to the named
	selection copy/paste buffer.
	\par Parameters:
	<b>MSTR setName</b>\n\n
	The name of the selection set.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnNamedSelectionCopy (MSTR setName) {}
	/*! \remarks This method pastes selection from named selection copy/paste
	buffer.
	\par Parameters:
	<b>bool useDlgToRename</b>\n\n
	Only matters if there is a name conflict with an existing named selection.
	If true, the system should throw up an interactive dialog for the user to
	rename the selection. If false, 3ds Max should use string techniques to
	rename the selection (by adding or increasing a number, etc.)
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnNamedSelectionPaste (bool useDlgToRename) {}

	// Topological & Geometric ops from the Edit Geometry panel:
	// Create button:
	/*! \remarks This method creates a new vertex in the mesh.
	\par Parameters:
	<b>Point3 pt</b>\n\n
	The location of the new vertex in object or world space.\n\n
	<b>bool pt_local=false</b>\n\n
	If true, the point passed is assumed to be in object space. If false, the
	point is assumed to represent world space, and the object space location
	must be computed by the method.\n\n
	<b>bool select=true</b>\n\n
	Indicates if the new vertex should be selected (have its <b>MN_SEL</b> flag
	set).
	\return  The index of the new vertex, or -1 to indicate failure to create
	the vertex.
	\par Default Implementation:
	<b>{ return -1; }</b> */
	virtual int  EpfnCreateVertex(Point3 pt, bool pt_local=false, bool select=true) { return -1; }
	/*! \remarks This method creates a new edge, dividing a polygon into two
	smaller polygons.
	\par Parameters:
	<b>int v1, v2</b>\n\n
	The endpoint vertices for this edge. These vertices must have at least one
	face in common, or the creation will fail.\n\n
	<b>bool select=true</b>\n\n
	Indicates if the new edge should be selected (have its <b>MN_SEL</b> flag
	set).
	\return  The index of the edge created.
	\par Default Implementation:
	<b>{ return -1; }</b> */
	virtual int  EpfnCreateEdge (int v1, int v2, bool select=true) { return -1; }
	/*! \remarks This method creates a new face on a set of vertices.
	\par Parameters:
	<b>int *v</b>\n\n
	An array of vertices for this new face. Note that each vertex must be an
	"open" vertex - it either must be on no edges or faces, or it must be part
	of a border (i.e. it's on more edges than faces).\n\n
	<b>int deg</b>\n\n
	The degree of the new face - and the size of the "<b>v</b>" array.\n\n
	<b>bool select=true</b>\n\n
	Indicates if the new face should be selected (have its <b>MN_SEL</b> flag
	set).
	\return  The index of the face created.
	\par Default Implementation:
	<b>{ return -1; }</b> */
	virtual int  EpfnCreateFace(int *v, int deg, bool select=true) { return -1; }
	/*! \remarks This method caps the indicated holes.
	\par Parameters:
	<b>int msl=MNM_SL_EDGE</b>\n\n
	MNMesh-based selection level, one of <b>MNM_SL_VERTEX, MNM_SL_EDGE</b>, or
	<b>MNM_SL_FACE</b>. The holes are border loops which can be identified by
	containing selected edges, using selected vertices, or being touched by
	selected faces.\n\n
	<b>DWORD targetFlags=MN_SEL</b>\n\n
	The flags we're looking for (in the vertex, edge, or face levels, according
	to <b>msl</b>) to identify the holes we should cap.
	\return  TRUE if any hole was successfully capped, otherwise FALSE
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnCapHoles (int msl=MNM_SL_EDGE, DWORD targetFlags=MN_SEL) { return false; }

	// Delete button:
	/*! \remarks This method deletes the specified components (and any other
	components dependent on them).
	\par Parameters:
	<b>int msl</b>\n\n
	MNMesh-based selection level for deletion to occur on; one of
	<b>MNM_SL_VERTEX, MNM_SL_EDGE</b>, or <b>MNM_SL_FACE</b>. Note that edge
	deletion joins, rather than removes, the neighboring faces. Border edges
	cannot be deleted. Deleting an edge between two quads makes a hexagon.
	Etc.\n\n
	<b>DWORD delFlag=MN_SEL</b>\n\n
	The flag indicating components to delete.\n\n
	<b>bool delIsoVerts=false</b>\n\n
	If deleting faces, this indicates whether vertices that are left isolated
	by the face deletion should also be deleted. (Note that in the reverse
	situation, faces dependent on deleted vertices are always deleted.)
	\return  TRUE if components were deleted, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnDelete (int msl, DWORD delFlag=MN_SEL, bool delIsoVerts=false) { return false; }

	// Attach button:
	virtual void EpfnAttach (INode *node, bool & canUndo, INode *myNode, TimeValue t) {}

	// Detach button:
	/*! \remarks This method attaches a bunch of nodes to this mesh. The
	objects in the nodes are converted to polymeshes if needed, then attached
	as elements in this Editable Poly. (Then the originals are deleted.)
	\par Parameters:
	<b>INodeTab \&nodeTab</b>\n\n
	A table of nodes we want to attach.\n\n
	<b>INode *myNode</b>\n\n
	A pointer to this Editable Poly's node (used to match the attached objects
	to our object space).\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{}</b> */
	virtual void EpfnMultiAttach (INodeTab& nodeTab, INode* myNode, TimeValue t) {}
	/*! \remarks This method detaches part of PolyMesh to a separate element.
	\par Parameters:
	<b>int msl</b>\n\n
	Indicates the MNMesh-based selection level, one of <b>MNM_SL_VERTEX,
	MNM_SL_EDGE</b>, or <b>MNM_SL_FACE</b>.\n\n
	<b>DWORD flag</b>\n\n
	Flag indicates which components should be detached. For instance,
	<b>MNM_SL_VERTEX</b> and <b>MN_SEL</b> means selected vertices.\n\n
	<b>bool keepOriginal</b>\n\n
	If TRUE, the original components are left intact and a new element is
	cloned instead.
	\return  TRUE if elements are detached, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnDetachToElement (int msl, DWORD flag, bool keepOriginal) { return false; }
	/*! \remarks This method detaches part of PolyMesh to a separate object.
	\par Parameters:
	<b>MSTR name</b>\n\n
	The desired name for the new node.\n\n
	<b>int msl</b>\n\n
	Indicates the MNMesh-based selection level, one of <b>MNM_SL_VERTEX,
	MNM_SL_EDGE</b>, or <b>MNM_SL_FACE</b>.\n\n
	<b>DWORD flag</b>\n\n
	Flag indicates which components should be detached. For instance,
	<b>MNM_SL_VERTEX</b> and <b>MN_SEL</b> means selected vertices.\n\n
	<b>bool keepOriginal</b>\n\n
	If TRUE, the original components are left intact and the new object is
	cloned from them.\n\n
	<b>INode *myNode</b>\n\n
	A pointer to this EPoly's node (for transform and other node property
	access).\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\return  TRUE if parts were detached, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnDetachToObject (MSTR name, int msl, DWORD flag, bool keepOriginal, INode *myNode, TimeValue t) { return false; }
	/*! \remarks This method "splits" edges by breaking vertices on two or
	more flagged edges into as many copies as needed. In this way, any path of
	flagged edges becomes two open seams.
	\par Parameters:
	<b>DWORD flag=MN_SEL</b>\n\n
	Indicates which edges should be split. (Left at the default, selected edges
	are split.)
	\return  TRUE if any topological changes happened, FALSE if nothing
	happened.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnSplitEdges (DWORD flag=MN_SEL) { return false; }

	// Break/divide button:
	/*! \remarks This method breaks vertices into separate copies for each
	face using them. For example, breaking one corner of a box polymesh turns
	it into 3 vertices, one for each side that met at that vertex.
	\par Parameters:
	<b>DWORD flag=MN_SEL</b>\n\n
	Indicates which vertices should be broken. (Left at the default, selected
	vertices are broken.)
	\return  TRUE if any topological changes happened, FALSE if nothing
	happened.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnBreakVerts (DWORD flag=MN_SEL) { return false; }
	/*! \remarks This method divides the face into triangles meeting at a
	point described by generalized barycentric coordinates on the face. An
	n-sided polygon will become n triangles using this technique.
	\par Parameters:
	<b>int face</b>\n\n
	The face to divide.\n\n
	<b>Tab\<float\> \&bary</b>\n\n
	A table of floats, of the same size as the face's degree, indicating the
	contribution of each of the face's vertices to the division point. They
	should all sum to 1, indicating that the division point is a linear
	combination of the vertices. If they are all equal (1/n), the center of the
	face will be used.\n\n
	<b>bool select=true</b>\n\n
	Indicates whether the new triangles should have the MN_SEL flag set or not.
	\return  The index of the new vertex, or -1 for failure.
	\par Default Implementation:
	<b>{ return -1; }</b> */
	virtual int  EpfnDivideFace (int face, Tab<float> &bary, bool select=true) { return -1; }
	/*! \remarks This method divides an edge in two, creating a new vertex.
	\par Parameters:
	<b>int edge</b>\n\n
	The edge to divide.\n\n
	<b>float prop</b>\n\n
	The proportion along the edge for the new vertex, going from 0 at the v1
	end to 1 at the v2 end. For instance, a prop of .35 means that the new
	point will be located at;\n\n
	<b>.65*(v[e[edge].v1].p) + .35*(v[e[edge].v2].p)</b>\n\n
	<b>bool select=true</b>\n\n
	Indicates if the new vertex should be selected. (The new edge picks up its
	selection flag from the old edge.)
	\return  The index of the new vertex, or -1 for failure.
	\par Default Implementation:
	<b>{ return -1; }</b> */
	virtual int  EpfnDivideEdge (int edge, float prop, bool select=true) { return -1; }

	// Collapse button:
	/*! \remarks This method will collapse the current selection, turning each
	cluster (in edge or face level) or all selected points into a single point.
	Some restrictions inherent in a 3ds Max polygon-based mesh may prevent a
	complete collapse, if the result would have an illegal geometry.
	\par Parameters:
	<b>int msl</b>\n\n
	MNMesh-based selection level, one of <b>MNM_SL_VERTEX, MNM_SL_EDGE</b>, or
	<b>MNM_SL_FACE</b>.\n\n
	<b>DWORD flag</b>\n\n
	The flag on the components we wish to collapse.
	\return  TRUE if any changes occurred, FALSE if nothing happened.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnCollapse (int msl, DWORD flag) { return false; }

	// Extrude/Bevel/Chamfer section:
	/*! \remarks This method extrudes the flagged faces by the specified
	amount. Note that this method uses the parameter block value for
	<b>ep_extrusion_type</b>. Values can be:\n\n
	<b>0</b>: Extrude by group (cluster)\n\n
	<b>1</b>: Extrude by local normals\n\n
	<b>2</b>: Extrude by polygon - each polygon extrudes separately.
	\par Parameters:
	<b>float amount</b>\n\n
	The height of the extrusion. Can be positive or negative.\n\n
	<b>DWORD flag</b>\n\n
	Indicates which faces should be beveled.\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnExtrudeFaces (float amount, DWORD flag, TimeValue t) {}
	/*! \remarks This method bevels the flagged faces by the specified height
	and outline. Note that this method uses the parameter block value for
	<b>ep_extrusion_type</b> in making the extrusion component. Values can
	be:\n\n
	<b>0</b>: Extrude by group (cluster)\n\n
	<b>1</b>: Extrude by local normals\n\n
	<b>2</b>: Extrude by polygon - each polygon extrudes separately.
	\par Parameters:
	<b>float height</b>\n\n
	The height of the desired bevel. Can be positive or negative.\n\n
	<b>float outline</b>\n\n
	The amount of the outlining in the bevel. Positive amounts make the
	selected region larger; negative amounts make it smaller.\n\n
	<b>DWORD flag</b>\n\n
	Indicates which faces should be beveled.\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnBevelFaces (float height, float outline, DWORD flag, TimeValue t) {}
	/*! \remarks This method chamfers the flagged vertices by the specified
	amount.
	\par Parameters:
	<b>float amount</b>\n\n
	The amount of the chamfer.\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnChamferVertices (float amount, TimeValue t) {}
	/*! \remarks This method chamfers the flagged edges by the specified
	amount.
	\par Parameters:
	<b>float amount</b>\n\n
	The amount of the chamfer.\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnChamferEdges (float amount, TimeValue t) {}

	// Cut 'n' Slice
	/*! \remarks This method slices the mesh with the specified plane.
	\par Parameters:
	<b>Point3 planeNormal</b>\n\n
	<b>Point3 planeCenter</b>\n\n
	The definition of the plane, by the normal and the "center" (which can be
	any point in the plane).\n\n
	<b>bool flaggedFacesOnly=false</b>\n\n
	If set, only the flagged faces should be sliced. If false, all faces should
	be sliced.\n\n
	<b>DWORD faceFlags=MN_SEL</b>\n\n
	Indicates which faces should be sliced, if <b>flaggedFacesOnly</b> is TRUE.
	\return  TRUE if something has been sliced, or FALSE if nothing happened.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnSlice (Point3 planeNormal, Point3 planeCenter, bool flaggedFacesOnly=false, DWORD faceFlags=MN_SEL) { return false; }
	/*! \remarks This method indicates whether the Editable Poly is currently
	in Slice Plane mode.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnInSlicePlaneMode () { return false; }
	/*! \remarks This method cuts from one vertex to another. Note that this
	algorithm uses the parameter block value for <b>ep_split</b>, which
	controls whether the cut algorithm splits the mesh open.
	\par Parameters:
	<b>int startv</b>\n\n
	The starting vertex for the cut\n\n
	<b>Point3 destination</b>\n\n
	The location of the ending vertex for the cut.\n\n
	<b>Point3 projDir</b>\n\n
	The direction of the "view". (projDir and the vector between the two
	vertices define the plane that the cut occurs in.)
	\return  The destination vertex, or -1 if the cut was unable to be
	completed for some reason.
	\par Default Implementation:
	<b>{ return -1; }</b> */
	virtual int  EpfnCutVertex (int startv, Point3 destination, Point3 projDir) { return -1; }
	/*! \remarks This method cuts from one edge to another. Note that this
	algorithm uses the parameter block value for <b>ep_split</b>, which
	controls whether the cut algorithm splits the mesh open.
	\par Parameters:
	<b>int el</b>\n\n
	The edge index along that edge for the start of the cut.\n\n
	<b>float propl</b>\n\n
	The edge proportion along that edge for the start of the cut.\n\n
	<b>int e2</b>\n\n
	The edge index along that edge for the end of the cut.\n\n
	<b>float prop2</b>\n\n
	The edge proportion along that edge for the end of the cut.\n\n
	<b>Point3 projDir</b>\n\n
	The direction of the "view". (projDir and the vector between the two
	endpoints define the plane that the cut occurs in.)
	\return  The vertex created at the end of the cut, or -1 if the cut was
	unable to be completed for some reason.
	\par Default Implementation:
	<b>{ return -1; }</b> */
	virtual int  EpfnCutEdge (int e1, float prop1, int e2, float prop2, Point3 projDir) { return -1; }
	/*! \remarks This method cuts from one face to another, subdividing the
	start and end faces as needed for precisely matching the given start and
	end points. Note that this algorithm uses the parameter block value for
	<b>ep_split</b>, which controls whether the cut algorithm splits the mesh
	open.
	\par Parameters:
	<b>int f1</b>\n\n
	The face we should start on.\n\n
	<b>Point3 p1</b>\n\n
	The point (on face f1) for the start of the cut.\n\n
	<b>Point3 p2</b>\n\n
	The point at the end of the cut.\n\n
	<b>Point3 projDir</b>\n\n
	The direction of the "view". (projDir and the vector between the two
	endpoints define the plane that the cut occurs in.)
	\return  The vertex created at the end of the cut, or -1 if the cut was
	unable to be completed for some reason.
	\par Default Implementation:
	<b>{ return -1; }</b> */
	virtual int  EpfnCutFace (int f1, Point3 p1, Point3 p2, Point3 projDir) { return -1; }

	// Weld section:
	/*! \remarks This method welds the specified vertices together (if
	possible), and puts the result at the location specified.
	\par Parameters:
	<b>int vert1, vert2</b>\n\n
	The two vertices we wish to weld. Note that these vertices must be "border"
	vertices, that is, they must be used by some open (one-sided) edges.\n\n
	<b>Point3 destination</b>\n\n
	The desired location for the result. Usually this is <b>v[vert1].p</b>,
	<b>v[vert2].p</b>, or the average of the two.
	\return  Indicates if any welding successfully occurred. If FALSE, nothing
	happened.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnWeldVerts (int vert1, int vert2, Point3 destination) { return false; }
	/*! \remarks This method welds the first edge to the second edge, leaving
	the result at the location of the second edge.
	\par Parameters:
	<b>int edge1, edge2</b>\n\n
	The two edges we wish to weld. Note that these must be open (one-sided)
	edges.
	\return  Indicates if any welding successfully occurred. (If false, nothing
	happened.)
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnWeldEdges (int edge1, int edge2) { return false; }
	/*! \remarks This method welds all flagged vertices together to their
	average location.
	\par Parameters:
	<b>DWORD flag</b>\n\n
	Indicates which vertices should be welded. Note that these vertices must be
	"border" vertices, that is, they must be used by some open (one-sided)
	edges.
	\return  Indicates if any welding successfully occurred. If FALSE, nothing
	happened.)
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnWeldFlaggedVerts (DWORD flag) { return false; }
	/*! \remarks This method welds flagged edges together.
	\par Parameters:
	<b>DWORD flag</b>\n\n
	Indicates which edges should be welded. Note that these edges must be open
	(one-sided).
	\return  Indicates if any welding successfully occurred. (If false, nothing
	happened.)
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnWeldFlaggedEdges (DWORD flag) { return false; }

	// Rest of Geom Panel
	/*! \remarks This method creates a new shape object from flagged edges in
	the polymesh.
	\par Parameters:
	<b>MSTR name</b>\n\n
	The desired name for the new node containing the shape.\n\n
	<b>bool smooth</b>\n\n
	If TRUE, the new shape should be a smooth curve. If FALSE, it should be a
	linear shape which exactly follows the edges.\n\n
	<b>INode *myNode</b>\n\n
	A pointer to the node of the PolyMesh (used to obtain transform and other
	node level information for the new shape node).\n\n
	<b>DWORD edgeFlag=MN_SEL</b>\n\n
	Indicates which faces should be used in creating this shape.
	\return  TRUE if successful, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnCreateShape (MSTR name, bool smooth, INode *myNode, DWORD edgeFlag=MN_SEL) { return false; }
	/*! \remarks This method moves the flagged area into its "average plane".
	(Plane computed using average vertex positions and normals.)
	\par Parameters:
	<b>int msl</b>\n\n
	MNMesh-based selection level, one of <b>MNM_SL_VERTEX</b>,
	<b>MNM_SL_EDGE</b>, or <b>MNM_SL_FACE</b>.\n\n
	<b>DWORD flag=MN_SEL</b>\n\n
	The flag that indicates which components to align. Ignored if <b>msl</b> is
	set to <b>MNM_SL_OBJECT</b>.\n\n
	<b>TimeValue t=0</b>\n\n
	The current time. (This action can be used to set a key for animation.)
	\return  Indicates whether anything was moved.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnMakePlanar (int msl, DWORD flag=MN_SEL, TimeValue t=0) { return false; }
	/*! \remarks This method moved flagged region to the plane given (by
	orthogonal projection into the plane).
	\par Parameters:
	<b>Point3 planeNormal</b>\n\n
	<b>float planeOffset</b>\n\n
	The definition of the plane. The plane is that region of points X for which
	DotProd (planeNormal, X) = planeOffset.\n\n
	<b>int msl</b>\n\n
	MNMesh-based selection level, one of <b>MNM_SL_VERTEX</b>,
	<b>MNM_SL_EDGE</b>, or <b>MNM_SL_FACE</b>.\n\n
	<b>DWORD flag=MN_SEL</b>\n\n
	The flag that indicates which components to align. Ignored if <b>msl</b> is
	set to <b>MNM_SL_OBJECT</b>.\n\n
	<b>TimeValue t=0</b>\n\n
	The current time. (This action can be used to set a key for animation.)
	\return  Indicates whether anything was moved.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnMoveToPlane (Point3 planeNormal, float planeOffset, int msl, DWORD flag=MN_SEL, TimeValue t=0) { return false; }
	/*! \remarks This method aligns flagged parts to be on the current
	construction plane (in the current viewport).
	\par Parameters:
	<b>int msl</b>\n\n
	MNMesh-based selection level, one of <b>MNM_SL_OBJECT</b>,
	<b>MNM_SL_VERTEX</b>, <b>MNM_SL_EDGE</b>, or <b>MNM_SL_FACE</b>.\n\n
	<b>DWORD flag=MN_SEL</b>\n\n
	The flag that indicates which components to align. Ignored if <b>msl</b> is
	set to <b>MNM_SL_OBJECT</b>.
	\return  Indicates whether anything was aligned.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnAlignToGrid (int msl, DWORD flag=MN_SEL) { return false; }
	/*! \remarks This method aligns flagged parts to be at right angles to the
	current view (through the active viewport).
	\par Parameters:
	<b>int msl</b>\n\n
	MNMesh-based selection level, one of <b>MNM_SL_OBJECT</b>,
	<b>MNM_SL_VERTEX</b>, <b>MNM_SL_EDGE</b>, or <b>MNM_SL_FACE</b>.\n\n
	<b>DWORD flag=MN_SEL</b>\n\n
	The flag that indicates which components to align. Ignored if <b>msl</b> is
	set to <b>MNM_SL_OBJECT</b>.
	\return  Indicates whether anything was aligned.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnAlignToView (int msl, DWORD flag=MN_SEL) { return false; }
	/*! \remarks This method deletes isolated vertices, those that aren't used
	by any faces.
	\return  Indicates whether any vertices were deleted.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnDeleteIsoVerts () { return false; }

	// Subdivision panel
	/*! \remarks This method applies the NURMS style MeshSmooth algorithm to
	the flagged area of the mesh. Uses parameter block parameters:\n\n
	<b>ep_ms_smoothness</b>: Smoothness value for MeshSmooth type subdivision
	(float spinner, range 0-1)\n\n
	<b>ep_ms_smooth</b>: Currently unused.\n\n
	<b>ep_ms_sep_smooth</b>: Separate by smoothing groups for MeshSmooth type
	subdivision (checkbox)\n\n
	<b>ep_ms_sep_mat</b>: Separate by material IDs for MeshSmooth type
	subdivision (checkbox)
	\par Parameters:
	<b>int msl</b>\n\n
	MNMesh-based selection level, one of <b>MNM_SL_OBJECT</b>,
	<b>MNM_SL_VERTEX</b>, <b>MNM_SL_EDGE</b>, or <b>MNM_SL_FACE</b>.\n\n
	<b>DWORD flag=MN_SEL</b>\n\n
	The flag that indicates which components to meshsmooth. Ignored if
	<b>msl</b> is set to <b>MNM_SL_OBJECT</b>.
	\return  TRUE if applied, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnMeshSmooth (int msl, DWORD flag=MN_SEL)  { return false; }
	/*! \remarks This method tessellates flagged area of the mesh. Uses
	<b>ep_tess_type</b> and <b>ep_tess_tension</b> from the param block.
	\par Parameters:
	<b>int msl</b>\n\n
	MNMesh-based selection level, one of <b>MNM_SL_OBJECT, MNM_SL_VERTEX,
	MNM_SL_EDGE</b>, or <b>MNM_SL_FACE</b>.\n\n
	<b>DWORD flag=MN_SEL</b>\n\n
	The flag that indicates which components to tessellate. Ignored if
	<b>msl</b> is set to <b>MNM_SL_OBJECT</b>.
	\return  TRUE is tessellated, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnTessellate (int msl, DWORD flag=MN_SEL) { return false; }

	// Surface panels
	/*! \remarks This method will cue an update of the subdivision surface
	based on this Editable Poly. (See the Editable Poly documentation on the
	Object-level Surface Properties dialog for information on this surface.)
	Equivalent to the user pressing the "Update" button.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnForceSubdivision () { }
	/*! \remarks This method selects (or deselects) vertices based on their
	color. Note that the color to compare to is part of the parameter block -
	see <b>ep_vert_sel_color</b> and related parameters for details.
	\par Parameters:
	<b>BOOL add</b>\n\n
	If TRUE, this selection should be in addition to the current selection. If
	FALSE, only the vertices within the color range should be selected.\n\n
	<b>BOOL sub</b>\n\n
	If TRUE, then instead of the options under "add" above, the vertices within
	the specified color range are deselected (while other selections are
	unmodified).\n\n
	<b>int mp=0</b>\n\n
	The map channel used for vertex colors. Use 0 for the traditional vertex
	color channel, 1-99 for a texture map channel, <b>MAP_SHADING</b> for the
	vertex illumination channel, or <b>MAP_ALPHA</b> for the vertex alpha
	channel.\n\n
	<b>TimeValue t=0</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnSelectVertByColor (BOOL add, BOOL sub, int mp=0, TimeValue t=0) { }
	/*! \remarks This method modifies a polygon's triangulation so that the
	specified diagonal is part of it.
	\par Parameters:
	<b>int face</b>\n\n
	The face whose triangulation we want to modify\n\n
	<b>int corner1, corner2</b>\n\n
	The endpoints of the new diagonal. They should be indices into the vertex
	list. So for instance corner values of (0,3) would refer to vertices
	<b>f[face].vtx[0]</b> and <b>f[face].vtx[3]</b>. Note that if
	<b>(corner1+1)%deg == corner2</b> (or vice versa), or if
	<b>corner1==corner2</b>, nothing will happen.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnSetDiagonal (int face, int corner1, int corner2) { }
	/*! \remarks This method will automatically re-triangulate flagged faces,
	using the standard polygon triangulation algorithm. Note that this
	algorithm is designed to work well on a single face, but not necessarily to
	produce the best results on groups of faces.
	\par Parameters:
	<b>DWORD flag=MN_SEL</b>\n\n
	Indicates which faces should be affected.
	\return  Indicates whether anything happened. (If nothing was in fact
	flagged, it returns false.)
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnRetriangulate (DWORD flag=MN_SEL) { return false; }
	/*! \remarks This method flips the normals in selected elements. (Note
	that because of topological rules for the PolyMesh, there is no way to flip
	a single face - whole elements must be flipped at once.)
	\par Parameters:
	<b>DWORD flag=MN_SEL</b>\n\n
	Indicates which elements should be flipped. An element is considered
	flagged if any of its faces have that flag set.
	\return  Indicates which elements should be flipped. An element is
	considered flagged if any of its faces have that flag set.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool EpfnFlipNormals (DWORD flag=MN_SEL) { return false; }
	/*! \remarks This method selects or deselects faces by material ID.
	\par Parameters:
	<b>int index</b>\n\n
	The material ID that indicates a face should be affected.\n\n
	<b>bool clear</b>\n\n
	If TRUE, these faces should have their selection cleared. (If FALSE, their
	selection is set.)\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnSelectByMat (int index, bool clear, TimeValue t) { }
	/*! \remarks This method selects or deselects faces by smoothing group.
	\par Parameters:
	<b>DWORD bits</b>\n\n
	The smoothing group bits that indicate a face should be affected.\n\n
	<b>BOOL clear</b>\n\n
	If TRUE, these faces should have their selection cleared. (If FALSE, the
	selection is set.)\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnSelectBySmoothGroup (DWORD bits, BOOL clear, TimeValue t) { }
	/*! \remarks This method autosmooths the current face selection, using the
	autosmooth threshold set in the parameter block.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void EpfnAutoSmooth (TimeValue t) { }

	// other things...
	/*! \remarks This method deletes all the components in all the levels that
	have the MN_DEAD flag set. Note that this causes a renumbering of affected
	component levels. If you're deleting faces in the middle of a complex
	operation, it's usually best to just set their MN_DEAD flags, then call
	<b>CollapseDeadStructs</b> at the end, so that you don't have to worry
	about other faces' indices changing before you're done.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void CollapseDeadStructs () { }
	/*! \remarks This method is used for setting flags in the MNMesh
	components based on flags of other components that they touch. This is very
	versatile. For instance, to hide all faces that use selected vertices, you
	would call <b>EpfnPropegateComponentFlags (MNM_SL_FACE, MN_HIDE,
	MNM_SL_VERTEX, MN_SEL, false, true, true)</b>. (Hiding faces should be
	undoable.) To set the MN_WHATEVER flag on all vertices that are used only
	by selected edges, you'd call <b>EpfnPropegateComponentFlags
	(MNM_SL_VERTEX, MN_WHATEVER, MNM_SL_EDGE, MN_SEL, true);</b>
	\par Parameters:
	<b>int slTo</b>\n\n
	The selection level we wish to modify. (One of <b>MNM_SL_OBJECT,
	MNM_SL_VERTEX, MNM_SL_EDGE</b>, or <b>MNM_SL_FACE</b>.)\n\n
	<b>DWORD flTo</b>\n\n
	The flags we wish to change in that selection level.\n\n
	<b>int slFromt</b>\n\n
	The selection level we wish to base the changes on.\n\n
	<b>DWORD flFrom</b>\n\n
	The flags that indicate a change should happen.\n\n
	<b>bool ampersand=FALSE</b>\n\n
	When <b>slFrom</b> and <b>slTo</b> are different, this indicates whether
	the flags of the nearby components should be "or'd" or "and'd". If it's
	false, then any flagged components in the "from" level will cause the
	component in the "to" level to be affected. If true, then all the
	components in the "from" level that touch a component in the "to" level
	must be flagged in order for the "to" level component to be affected.
	(i.e., if from is faces and to is vertices, a vertex would only be modified
	if all faces that use it have the <b>flFrom</b> flag set.)\n\n
	<b>bool set=TRUE</b>\n\n
	If TRUE, this parameter indicates that the <b>flTo</b> flags should be set
	on targeted components. If false, it indicates that the flags should be
	cleared. For instance, to clear MN_HIDE flags on vertices that are used by
	selected edges, you'd call <b>EpfnPropegateComponentFlags (MNM_SL_VERTEX,
	MN_HIDE, MNM_SL_EDGE, MN_SEL, false, false, true);</b>\n\n
	<b>bool undoable=FALSE</b>\n\n
	Indicates if this action should create an entry for the undo system.
	Changes to <b>MN_SEL, MN_HIDE</b>, and <b>MN_DEAD</b> flags should
	generally be undoable, but changes to more minor flags like
	<b>MN_WHATEVER</b> that are used to set up for other operations generally
	don't have to be.
	\return  The number of components in the <b>slTo</b> level that were
	affected by the call. (If 0, nothing happened.)
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual int	 EpfnPropagateComponentFlags (int slTo, DWORD flTo, int slFrom, DWORD flFrom, bool ampersand=FALSE, bool set=TRUE, bool undoable=FALSE) { return 0; }

	//----------------------------------------
	//Methods from Max 8 - removed from EPoly8
	//----------------------------------------

	//! \brief Chamfer vertices, creating a whole between the chamfered vertices. 
	/// \param[in] in_amount: Chamfer distance between new vertices, must be positive  
	/// \param[in] in_open: If true, newly created faces are removed 
	/// \param[in] in_time: The time at which to chamfer the vertices 

	virtual void EpfnChamferVerticesOpen (float in_amount, bool in_open , TimeValue in_time) {}

	//! \brief Chamfer edges, creating a whole between the chamfered edges. 
	/// \param[in] in_amount: Chamfer distance between new edges, must be positive
	/// \param[in] in_open: If true, newly created faces are removed 
	/// \param[in] in_time: The time at which to chamfer the edges  

	virtual void EpfnChamferEdgesOpen (float in_amount,bool in_open,TimeValue in_time) {}						


	//! \brief Changes the edge selection , in the ring direction
	/*! This method adds ( move or remove ) edges from the current selection.
	//  \n n the 'positive' ( or negative ) ring direction,using the 3 existing parameters. The ring direction is vertical for a plane. 
	//  \n If in_newPos == -3 , in_moveOnly == false and in_add == true, it adds 3 edges from the current selection,
	//  \n in the negative ring direction. this is equivalent to the MaxScript: $.setRingShift -3 false true
	//  \param[in] in_newPos: If > 0 will change the edge selection the 'positive' ring direction, if < 0 in the 'negative' direction.
	//  \param[in] in_moveOnly: If true, edges are moved, if false, edges are added or removed from selection
	//  \param[in] in_add: If true edges are added, if false, edges are removed from current selection
	*/

	virtual void EpfnSetRingShift(int in_newPos,bool in_moveOnly, bool in_add) {}

	//! \brief Changes the edge selection , in the loop direction
	/*! This method adds ( move or remove ) edges from the current selection.
	//  \n n the 'positive' ( or negative ) loop direction,using the 3 existing parameters. The loop direction is horizontal for a plane. 
	//  \n If in_newPos == -3 , in_moveOnly == false and in_add == true, it adds 3 edges from the current selection,
	//  \n in the negative ring direction. this is equivalent to the MaxScript: $.setLoopShift -3 false true
	//  \param[in] in_newPos: If > 0 will change the edge selection the 'positive' ring direction, if < 0 in the 'negative' direction.
	//  \param[in] in_moveOnly: If true, edges are moved, if false, edges are added or removed from selection
	//  \param[in] in_add: If true edges are added, if false, edges are removed from current selection
	*/

	virtual void EpfnSetLoopShift(int in_newPos,bool in_moveOnly,bool in_add) {}		

	//! \brief Converts current sub-object selection to the specified sub-object border. 
	/*! It converts for instance selected faces into their border edges and selects it 
	//  \n if you call EpfnConvertSelectionToBorder( EP_SL_FACE, EP_SL_EDGE); 
	//  \param[in] in_epSelLevelFrom: Can be EP_SL_EDGE, EP_SL_FACE or EP_SL_VERTEX
	//  \param[in] in_epSelLevelTo: Can be EP_SL_EDGE, EP_SL_FACE or EP_SL_VERTEX
	*/

	virtual int EpfnConvertSelectionToBorder (int in_epSelLevelFrom,int in_epSelLevelTo) { return false; }	

	//! \brief Commits any Paint Deformation as if pressing the Commit button
	/*! Has no effect when the Commit button is disabled */

	virtual void EpfnPaintDeformCommit() {}

	//! \brief Cancels any Paint Deformation as if pressing the Cancel button
	/*! Has no effect when the Cancel button is disabled */

	virtual void EpfnPaintDeformCancel() {}



	//! \brief Closes the Smooth Group Floater Floater dialog
	virtual void CloseSmGrpFloater() {}
	
	//! \brief Closes the Material ID Floater dialog
	virtual void CloseMatIDFloater() {}

	//! \brief Returns the Material ID Floater dialog handle
	virtual HWND MatIDFloaterHWND() { return NULL; }
	//! \brief Returns the Smooth Group Floater dialog handle
	virtual HWND SmGrpFloaterHWND() { return NULL; }


	//!  \brief Brings up the Smooting Group floater dialog.  If it is already up it closes it
	virtual	void		SmGrpFloater() = 0;
	//!  \brief  Brings up the Material ID floater dialog.  If it is already up it closes it
	virtual	void		MatIDFloater() = 0;
	//!  \brief  Returns whether the Material ID floater dialog is up
	virtual	BOOL		MatIDFloaterVisible() = 0;
	//!  \brief  Returns whether the  Smooting Group floater dialog is up
	virtual	BOOL		SmGrpFloaterVisible() = 0;


private:
	// Access to new Preview mode:
	virtual void 		EpPreviewBegin (int previewOperation) { }
	virtual void 		EpPreviewCancel () { }
	virtual void 		EpPreviewAccept () { }
	virtual void 		EpPreviewInvalidate () { }
	virtual bool 		EpPreviewOn () { return false; }
	virtual void 		EpPreviewSetDragging (bool drag) { }
	virtual bool 		EpPreviewGetDragging () { return false; }

	virtual bool 		EpfnPopupDialog (int popupOperation) { return false; }
	virtual void 		EpfnClosePopupDialog () { }

	virtual void 		EpfnRepeatLastOperation () { }

	virtual void 		EpfnGrowSelection (int meshSelLevel) { }
	virtual void 		EpfnShrinkSelection (int meshSelLevel) { }
	virtual int  		EpfnConvertSelection (int epSelLevelFrom, int epSelLevelTo, bool requireAll=false) { return false; }
	virtual void 		EpfnSelectBorder () { }
	virtual void 		EpfnSelectElement () { }
	virtual void 		EpfnSelectEdgeLoop () { }
	virtual void 		EpfnSelectEdgeRing () { }
	virtual bool 		EpfnRemove (int msl, DWORD delFlag=MN_SEL) { return false; }
	virtual bool 		EpfnDeleteIsoMapVerts () { return false; }
	virtual bool 		EpfnOutline (DWORD flag) { return false; }
	virtual bool 		EpfnConnectEdges (DWORD edgeFlag) { return false; }
	virtual bool 		EpfnConnectVertices (DWORD vertexFlag) { return false; }
	virtual bool 		EpfnExtrudeAlongSpline (DWORD faceFlag) { return false; }
	virtual bool 		EpfnLiftFromEdge (DWORD faceFlag) { return false; }
	virtual void 		EpfnToggleShadedFaces () { }
	virtual int  		EpfnCreateFace2 (Tab<int> *vertices, bool select=true) { return -1; }
	/*! \remarks This method attaches the specified object to this mesh. The
	object given is first converted to a polymesh (if needed), then attached as
	an element in the Editable Poly, then the original is deleted.
	\par Parameters:
	<b>INode *node</b>\n\n
	A pointer to the node we want to attach.\n\n
	<b>INode *myNode</b>\n\n
	A pointer to this Editable Poly's node (used to match the attached object
	to our object space).\n\n
	<b>TimeValue t</b>\n\n
	The current time.
	\par Default Implementation:
	<b>{ }</b> */
	virtual void 		EpfnAttach (INode *node, INode *myNode, TimeValue t) { }

	virtual BitArray	*EpfnGetSelection (int msl) { return NULL; }
	virtual void		EpfnSetSelection (int msl, BitArray *newSel) { }

	virtual int			EpfnGetNumVertices () { return 0; }
	virtual Point3		EpfnGetVertex (int vertIndex) { return Point3(0,0,0); }
	virtual int			EpfnGetVertexFaceCount (int vertIndex) { return 0; }
	virtual int 		EpfnGetVertexFace (int vertIndex, int whichFace) { return 0; }
	virtual int 		EpfnGetVertexEdgeCount (int vertIndex) { return 0; }
	virtual int 		EpfnGetVertexEdge (int vertIndex, int whichEdge) { return 0; }

	virtual int 		EpfnGetNumEdges () { return 0; }
	virtual int 		EpfnGetEdgeVertex (int edgeIndex, int end) { return 0; }
	virtual int 		EpfnGetEdgeFace (int edgeIndex, int side) { return 0; }

	virtual int 		EpfnGetNumFaces() { return 0; }
	virtual int 		EpfnGetFaceDegree (int faceIndex) { return 0; }
	virtual int 		EpfnGetFaceVertex (int faceIndex, int corner) { return 0; }
	virtual int 		EpfnGetFaceEdge (int faceIndex, int side) { return 0; }
	virtual int			EpfnGetFaceMaterial (int faceIndex) { return 0; }
	virtual DWORD		EpfnGetFaceSmoothingGroup (int faceIndex) { return 0; }

	virtual int			EpfnGetNumMapChannels () { return 0; }
	virtual bool		EpfnGetMapChannelActive (int mapChannel) { return false; }
	virtual int			EpfnGetNumMapVertices (int mapChannel) { return 0; }
	virtual UVVert		EpfnGetMapVertex (int mapChannel, int vertIndex) { return UVVert(0,0,0); }
	virtual int			EpfnGetMapFaceVertex (int mapChannel, int faceIndex, int corner) { return 0; }

	// New in 7.0:
	virtual MapBitArray GetPreserveMapSettings () const { return MapBitArray(true, false); }
	virtual void		SetPreserveMapSettings (const MapBitArray & mapSettings) { }
	virtual void 		EpfnSetPreserveMap (int mapChannel, bool preserve) { }
	virtual bool 		EpfnGetPreserveMap (int mapChannel) { return (mapChannel>0); }

	virtual bool 		EpfnBridge (int epolySelLevel, DWORD flag) { return false; }
	virtual bool 		EpfnReadyToBridgeFlagged (int epolySelLevel, DWORD flag) { return false; }
	virtual bool 		EpfnTurnDiagonal (int face, int diag) { return false; }
	virtual bool 		EpfnRelax (int msl, DWORD flag, TimeValue t) { return false; }
	virtual bool 		EpfnMakePlanarIn (int dimension, int msl, DWORD flag, TimeValue t) { return false; }

	virtual void		EpFnSetVertexColor (Color *clr, int mp) {}
	virtual Color		EpFnGetVertexColor (int mp) { return Color(0,0,0); }
	virtual void		EpFnSetFaceColor (Color *clr, int mp) {}
	virtual Color		EpFnGetFaceColor (int mp) { return Color(0,0,0); }

};


class EPolyManipulatorGrip_Imp;

//! This new class extends EPoly with new functionality, in particular the ability for epoly to support manipulate grips.
class EPoly13: public EPoly
{
	friend class EPolyManipulatorGrip_Imp;
protected:
	/*! \defgroup ManipulateGrips  Manipulate Grip Items
		The different select and manipulate mode grip items that can be toggled on or off.	
	*/
	//@{
	enum ManipulateGrips
	{
		//! Soft Selection Falloff grip.  May be on for all subobject modes.
		eSoftSelFalloff = 0,
		//! Soft Selection Pinch grip.  May be on for all subobject modes.
		eSoftSelPinch,
		//! Soft Selection Bubble grip.  May be on for all subobject modes.
		eSoftSelBubble,
		//! Set Flow grip.  Only on in edge subobject mode.
		eSetFlow,
		//! Loop Shift Selection Falloff grip.  Only in edge subobject mode.
		eLoopShift,
		//! Ring Shift Selection Falloff grip.  Only in edge subobject mode.
		eRingShift,
		//! Edge Crease grip.  Only in edge subobject mode.
		eEdgeCrease,
		//! Edge Weight grip.  Only in edge subobject mode.
		eEdgeWeight,
		//! Vertex Weight grip.  Only in vertex subobject mode.
		eVertexWeight
	};
	//}@

	//! \brief Turns the manipulate grip item on or off. When at least one grip item is on and 
	//! select and manipulate mode is on a grip will show up for each item that's on.
	//! \param[in] on - Whether or not the specified item will be turned on or off
	//! \param[in] item - Which grip item that will be turned on or off.
	virtual void SetManipulateGrip(bool on, ManipulateGrips item) = 0;

	//! \brief Get whether or not this manipulator item is on or not. 
	//! \param[in] item - Which grip item we are checking to see if it's on or off.
	//! \return Whether or not the item is on or not.  Note that it doesn't
	//! take into account whether or not the correct subobject mode is also active, it just 
	//! checks to see if it would be on if we were in the right mode.
	virtual bool GetManipulateGrip(ManipulateGrips item) = 0;

};


#pragma warning(pop)

