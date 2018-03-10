/**********************************************************************
 *<
	FILE: iEPolyMod.h

	DESCRIPTION:   Interface for Edit Poly modifier.

	CREATED BY: Steve Anderson

	HISTORY: created March 2004

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "iFnPub.h"
#include "mnmesh.h"
// forward declarations
class Modifier;
class IParamBlock2;
class IObjParam;

#define EPOLY_MOD_INTERFACE Interface_ID(0x405e217d, 0x49c2abc)
#define GetEPolyModInterface(cd) (EPolyMod *)(cd)->GetInterface(EPOLY_MOD_INTERFACE)

#define EPOLY_MOD13_INTERFACE Interface_ID(0x1a4cc8f2, 0x71682518)
#define GetEPolyMod13Interface(cd) (EPolyMod13 *)(cd)->GetInterface(EPOLY_MOD13_INTERFACE)

// Editable Poly Selection Levels
enum ePolyModSelLevel { EPM_SL_OBJECT, EPM_SL_VERTEX, EPM_SL_EDGE, 
	EPM_SL_BORDER, EPM_SL_FACE, EPM_SL_ELEMENT, EPM_SL_CURRENT };

// Relate EPolyMod selection levels to MNMesh selection levels:
const int meshSelLevel[] = { MNM_SL_OBJECT, MNM_SL_VERTEX, MNM_SL_EDGE, MNM_SL_EDGE,
	MNM_SL_FACE, MNM_SL_FACE, MNM_SL_CURRENT };

// Edit Poly Parameters:
enum epolyModParameters {

	epm_animation_mode,		// int - is the Edit Poly in modeling mode (0) or animation mode (1)
	epm_show_cage,			// BOOL - indicates if the Edit Poly is set to display the cage

	epm_by_vertex,			// BOOL - indicates if the Edit Poly is selecting by vertex.
	epm_ignore_backfacing,	// BOOL - indicates if the Edit Poly is ignoring backfacing when the user selects subobjects.
	epm_stack_selection,	// BOOL - indicates whether or not the Edit Poly is using the subobject selection from the stack below it.

	epm_ss_use,				// BOOL - Is the Edit Poly using soft selection?
	epm_ss_edist_use,		// BOOL - Is the Edit Poly soft selection using edge distances or not?
	epm_ss_edist,			// int - How many edges do we traverse in determining soft selection?  (Ignored if epm_ss_edist_use is set to false.)
	epm_ss_affect_back,		// BOOL - Should soft selection affect backfacing?
	epm_ss_falloff,			// float - soft selection falloff value
	epm_ss_pinch,			// float - soft selection pinch value
	epm_ss_bubble,			// float - soft selection bubble value

	epm_current_operation,	// int - indicates current operation, which should be one of the epolyModButtonOp enums.

	epm_world_to_object_transform, // Matrix3 - Worldspace to common Objectspace transform.  Used in combination with the local mod context tms, this can give us relative WS-to-OS transforms for each modifier instance.  See Edit Poly source for examples of use.

	epm_constrain_type,		// int - Represents the current vertex constraints.  0 for no constraints, 1 for constrain-to-edge, 2 for constrain-to-face, 3 for constrain-to-normal.

	epm_split,				// BOOL - indicates whether Cut and Slice (ep_op_cut, ep_op_slice, ep_op_slice_face) operations should split the mesh, or merely refine it.

	epm_ms_smoothness,		// float - Smoothness parameter for MeshSmooth operation (ep_op_meshsmooth)
	epm_ms_sep_smooth,		// BOOL - Separate-by-smoothing-groups parameter for MeshSmooth operation (ep_op_meshsmooth)
	epm_ms_sep_mat,			// BOOL - Separate-by-materials parameter for MeshSmooth operation (ep_op_meshsmooth)

	epm_tess_type,			// int - Type of Tessellation (ep_op_tessellate) - 0 for 
	epm_tess_tension,		// float - tension parameter for Tessellation operation (ep_op_tessellate)

	epm_bridge_taper,		// float - Taper parameter for Bridge operation (ep_op_bridge_border, ep_op_bridge_polygon)
	epm_bridge_bias,		// float - Bias parameter for Bridge operation (ep_op_bridge_border, ep_op_bridge_polygon)

	epm_align_type,			// int - whether to align to view (0) or construction plane (1) in future uses of ep_op_align
	epm_align_plane_normal,	// Point3 - describes the plane normal currently used for alignment.
	epm_align_plane_offset,	// float - describes the plane offset currently used for alignment.  (The normal and offset together describe a plane.)

	epm_delete_isolated_verts,	// BOOL - Indicates whether the Edit Poly deletes freshly isolated vertices when deleting faces.

	epm_weld_vertex_threshold,	// float - The threshold used when welding selected vertices (ep_op_weld_vertex)

	epm_chamfer_vertex,			// float - the "amount" parameter for the Chamfer Vertex operation (ep_op_chamfer_vertex)

	epm_extrude_vertex_width,	// float - the width of the extrusion for the Extrude Vertex operation (ep_op_extrude_vertex)
	epm_extrude_vertex_height,	// float - the height of the extrusion for the Extrude Vertex operation (ep_op_extrude_vertex)

	epm_connect_edge_segments,	// int - the number of segments to use when connecting edges (ep_op_conect_edge)

	epm_weld_edge_threshold,	// float - The threshold used when welding selected edges (ep_op_weld_edge)

	epm_extrude_edge_height,	// float - the height of the extrusion for the Extrude Edge operation (ep_op_extrude_edge)
	epm_extrude_edge_width,		// float - the width of the extrusion for the Extrude Edge operation (ep_op_extrude_edge)

	epm_chamfer_edge,			// float - the "amount" parameter for the Chamfer Edge operation (ep_op_chamfer_edge)

	epm_extrude_face_type,		// int - indicates the type of face extrusion - 0 for Group Normals, 1 for Local Normals, 2 for By Polygon (ep_op_extrude_face)
	epm_extrude_face_height,	// float - indicates the height of the face extrusion (ep_op_extrude_face)

	epm_bevel_type,				// int - indicates the type of face extrusion used in beveling - 0 for Group Normals, 1 for Local Normals, 2 for By Polygon (ep_op_bevel)
	epm_bevel_height,			// float - height of the face bevel (ep_op_bevel)
	epm_bevel_outline,			// float - "outline" amount (amount to move vertices in or out) in the Bevel operation (ep_op_bevel)

	epm_inset_type,				// int - indicates the type of face extrusion used in insetting - 0 for Group Normals, 1 for Local Normals, 2 for By Polygon (ep_op_inset)
	epm_inset,					// float - the amount of the inset (ep_op_inset)

	epm_outline,				// float - the amount of outlining (ep_op_outline)

	epm_hinge_angle,			// float - the angle to use in Hinge from Edge (ep_op_hinge_from_edge)
	epm_hinge_base,				// Point3 - the base of the hinge (ep_op_hinge_from_edge)
	epm_hinge_dir,				// Point3 - the direction of the hinge - combine with epm_hinge_base to get a complete ray for the hinge (ep_op_hinge_from_edge)
	epm_hinge_segments,			// int - the number of segments to use on the sides of the Hinge from Edge extrusion (ep_op_hinge_from_edge)

	epm_extrude_spline_node,		// INode - the node of the spline to use in Extrude along Spline (ep_op_extrude_along_spline)
	epm_extrude_spline_segments,	// int - the number of segments in Extrude along Spline (ep_op_extrude_along_spline)
	epm_extrude_spline_taper,		// float - the amount of tapering to use in Extrude along Spline (ep_op_extrude_along_spline)
	epm_extrude_spline_taper_curve,	// float - the taper curve for Extrude along Spline (ep_op_extrude_along_spline)
	epm_extrude_spline_twist,		// float - the amount of twist (in radians) for Extrude along Spline (ep_op_extrude_along_spline)
	epm_extrude_spline_rotation,	// float - the rotation (in radians) around the face normal for Extrude along Spline (ep_op_extrude_along_spline)
	epm_extrude_spline_align,		// BOOL - whether or not to try to align the extrusion spline to match the face normal (ep_op_extrude_along_spline)

	epm_autosmooth_threshold,	// float - the threshold for autosmoothing across edges, in radians (ep_op_autosmooth)
	epm_smooth_group_set,		// DWORD cast as an int - the set of smoothing groups to set (ep_op_smooth)
	epm_smooth_group_clear,		// DWORD cast as an int - the set of smoothing groups to clear (ep_op_smooth)

	epm_material,				// int - the material ID to set on selected faces (ep_op_set_material)

	epm_material_selby,			// int - the material ID to select faces by (ep_op_select_by_material)
	epm_material_selby_clear,	// BOOL - whether or not to clear the previous selection before selecting by material (ep_op_select_by_material)

	epm_smoother_selby,			// DWORD cast as an int - the set of smoothing groups to select faces by (ep_op_select_by_smooth)
	epm_smoother_selby_clear,	// BOOL - whether or not to clear the previous selection before selecting faces by smoothing group (ep_op_select_by_smooth)

	epm_clone_to_element,		// BOOL - whether to clone to an element or to a separate object.

	epm_bridge_segments,		// int - the number of segments for bridge operations
	epm_bridge_smooth_thresh,	// float - the smoothing threshold for successive bridge columns (in radians)
	epm_bridge_target_1,		// int - the first edge or face we're bridging between, in 1-based indexing.  (Value of 0 indicates not set yet.)
	epm_bridge_target_2,		// int - the second edge or face we're bridging between, in 1-based indexing.  (Value of 0 indicates not set yet.)
	epm_bridge_twist_1,			// int - the Twist amount for the first end of the bridge.
	epm_bridge_twist_2,			// int - the Twist amount for the second end of the bridge
	epm_bridge_selected,		// BOOL - false indicates we should bridge between the specific targets given in epm_bridge_target_1 and _2, true indicates we should bridge selected borders or polgyons.

	epm_preserve_maps,			// BOOL - Indicates whether map values should be "preserved" in geometric deformations such as subobject transforms.

	epm_relax_amount,			// float - the relax Amount parameter - range of 0 (no relaxing) to 1 (maximum relaxing), default of .5
	epm_relax_iters,			// int - the number of iterations of the relax algorithm to apply.
	epm_relax_hold_boundary,	// BOOL - Indicates whether or not to fix boundary points in place when relaxing
	epm_relax_hold_outer,		// BOOL - Indicates whether or not to fix "outer" points in place when relaxing.  (See the MNMeshUtilities::Relax declaration in MNMesh.h for further explanation.)

	epm_select_by_angle,		// BOOL - Indicates whether Polygon selections should be expanded depending on edge angles.
	epm_select_angle,			// float - Indicates the maximum angle used in expanding polygon selections.  (Ignored if epm_select_by_angle is false.)

	// Paint tools
	epm_ss_lock,
	epm_paintsel_value, epm_paintsel_size, epm_paintsel_strength,
	epm_paintdeform_value, epm_paintdeform_size, epm_paintdeform_strength,
	epm_paintdeform_axis,

	epm_reserved1,					// obsolete -- SEE the #define epm_edge_chamfer_segments epm_reserved1
	                                // beneath this enum!
	epm_reserved2,					// obsolete
	epm_reserved3,					// obsolete
	epm_reserved4,					// obsolete
	epm_reserved5,					// obsolete
	
	epm_connect_edge_pinch,			// float	- distance between new edges created when connecting edges
	epm_connect_edge_slide,			// float	- start position for edge connection
	epm_loop_edge_sel,				// int		- loop spinner value
	epm_ring_edge_sel,				// int		- ring spinner value
	epm_remove_edge_ctrl,			// BOOOL	- is control key pressed when removing edges ? 
	epm_cage_color,					// Point3	- the user supplied cage color 
	epm_selected_cage_color,		// Point3	- the user supplied cage color for selected objects
	epm_cage_color_changed,			// int		- has the cage color been changed 
	epm_selected_cage_color_changed,// int		- has the selected cage color been changed 
	epm_chamfer_vertex_open,		// BOOL		- is a vertex chamfer creating a hole ? 
	epm_chamfer_edge_open,			// BOOL		- is an edge chamfer creating a hole ? 
	epm_bridge_adjacent,			// float	- the angle limit used for bridging adjacent edges 
	epm_bridge_reverse_triangle,	// BOOL		- is a triangle to be reversed when bridging edges ?
	epm_break_vertex_distance,		// float	- the distance used for opening vertices

	epm_paintsel_mode,
	epm_paintdeform_mode,

	epm_select_mode,				// BOOL		-  the select mode that we are in, none, sub object, or multi.

	// new for 9.5
	//epm_edge_chamfer_segments  // int - the number of new edges to create per selected edge (ep_edge_chamfer_segments)
	                             // SEE #define BELOW!
};
// In order to preserve forward file IO compatibility with Makalu (Max 9), the new chamfer parameter
// for Velveeta (Max 9.5) has to take on the value of epm_reserved1. See the implementation of
// PolyOperation::LoadBasics (and PolyOperation::SaveBasics).
#define epm_edge_chamfer_segments epm_reserved1

// Edit Poly Operations:
enum epolyModButtonOp { ep_op_null,
	ep_op_hide_vertex, ep_op_hide_face,
	ep_op_unhide_vertex, ep_op_unhide_face,
	ep_op_hide_unsel_vertex, ep_op_hide_unsel_face,
	ep_op_ns_copy, ep_op_ns_paste, ep_op_cap,
	ep_op_delete_vertex, ep_op_delete_face, ep_op_detach_vertex, ep_op_detach_face,
	ep_op_split, ep_op_break, ep_op_collapse_vertex, ep_op_collapse_edge,
	ep_op_collapse_face, ep_op_reset_plane, ep_op_slice, ep_op_slice_face,
	ep_op_weld_vertex, ep_op_weld_edge, ep_op_create_shape, ep_op_make_planar, ep_op_remove_iso_verts,
	ep_op_meshsmooth, ep_op_tessellate, ep_op_retriangulate,
	ep_op_flip_face, ep_op_flip_element, ep_op_bevel, ep_op_chamfer_vertex,
	ep_op_chamfer_edge, ep_op_cut,
	ep_op_sel_grow, ep_op_sel_shrink, ep_op_inset, ep_op_extrude_vertex,
	ep_op_extrude_edge, ep_op_extrude_face,
	ep_op_extrude_along_spline, ep_op_connect_edge, ep_op_connect_vertex,
	ep_op_hinge_from_edge, ep_op_select_ring, ep_op_select_loop,
	ep_op_remove_iso_map_verts, ep_op_remove_vertex, ep_op_remove_edge, ep_op_outline,
	ep_op_toggle_shaded_faces,
	ep_op_transform, ep_op_delete_edge, ep_op_unused,
	ep_op_create, ep_op_align,
	ep_op_attach, ep_op_target_weld_vertex, ep_op_target_weld_edge,
	ep_op_edit_triangulation, ep_op_create_edge,
	ep_op_clone_vertex, ep_op_clone_edge, ep_op_clone_face,
	ep_op_insert_vertex_edge, ep_op_insert_vertex_face,
	ep_op_autosmooth, ep_op_smooth, ep_op_set_material,
	ep_op_select_by_smooth, ep_op_select_by_material, ep_op_get_stack_selection,
	ep_op_bridge_border, ep_op_bridge_polygon, ep_op_make_planar_x, ep_op_make_planar_y,
	ep_op_make_planar_z, ep_op_turn_edge, ep_op_relax, ep_op_paint_deform,
	ep_op_bridge_edge, ep_op_preserve_uv_settings
};

// Edit Poly Command modes:
enum epolyModCommandMode { ep_mode_create_vertex, ep_mode_create_edge,
	ep_mode_create_face, ep_mode_divide_edge, ep_mode_divide_face,
	ep_mode_extrude_vertex, ep_mode_extrude_edge, ep_mode_extrude_face,
	ep_mode_chamfer_vertex, ep_mode_chamfer_edge, ep_mode_bevel,
	ep_mode_sliceplane, ep_mode_cut, ep_mode_weld, ep_mode_edit_tri,
	ep_mode_inset_face, ep_mode_quickslice, ep_mode_hinge_from_edge,
	ep_mode_pick_hinge, ep_mode_outline, ep_mode_bridge_border,
	ep_mode_bridge_polygon, ep_mode_pick_bridge_1, ep_mode_pick_bridge_2,
	ep_mode_turn_edge,	ep_mode_bridge_edge,
        //! New edit soft selection command mode for 3ds max2009 
        ep_mode_edit_ss
};

// Edit Poly Pick modes:
enum epolyModPickMode { ep_mode_attach, ep_mode_pick_shape };

// Edit Poly methods - for use in function-publishing (for Maxscript access).
enum epolyModMethods {
	epmod_get_sel_level, epmod_get_mn_sel_level, epmod_set_sel_level,
	epmod_get_operation, epmod_set_operation, epmod_local_data_changed,
	epmod_refresh_screen, epmod_commit, epmod_commit_unless_animating,
	epmod_commit_and_repeat, epmod_cancel_operation, epmod_convert_selection, epmod_get_selection,
	epmod_set_selection, epmod_select, epmod_set_primary_node,
	epmod_toggle_command_mode, epmod_enter_command_mode, epmod_enter_pick_mode,
	epmod_get_command_mode, epmod_get_pick_mode,
	epmod_popup_dialog, epmod_button_op, epmod_repeat_last, epmod_in_slice_mode,
	epmod_reset_slice_plane, epmod_get_slice_plane_tm, epmod_get_slice_plane, epmod_set_slice_plane,
	epmod_create_vertex, epmod_create_face, epmod_create_edge, epmod_set_diagonal,
	epmod_cut, epmod_target_weld_vertex, epmod_target_weld_edge, epmod_attach_node,
	epmod_attach_nodes, epmod_detach_to_object, epmod_create_shape,
	epmod_bridge_borders, epmod_bridge_polygons, epmod_set_bridge_node, epmod_get_bridge_node,
	epmod_ready_to_bridge_selected,epmod_get_preserve_map, epmod_set_preserve_map,
	epmod_turn_diagonal,

	// Access to information about mesh:
	epmod_get_num_vertices, epmod_get_vertex, epmod_get_vertex_face_count, epmod_get_vertex_face,
	epmod_get_vertex_edge_count, epmod_get_vertex_edge, epmod_get_num_edges, epmod_get_edge_vertex,
	epmod_get_edge_face, epmod_get_num_faces, epmod_get_face_degree, epmod_get_face_vertex,
	epmod_get_face_edge, epmod_get_face_diagonal, epmod_get_face_material, epmod_get_face_smoothing_group,
	epmod_get_num_map_channels, epmod_get_map_channel_active,
	epmod_get_num_map_vertices, epmod_get_map_vertex, epmod_get_map_face_vertex,

	epmod_get_face_normal, epmod_get_face_center, epmod_get_face_area, epmod_get_open_edges,
	epmod_get_verts_by_flag, epmod_get_edges_by_flag, epmod_get_faces_by_flag,
	epmod_set_vertex_flags, epmod_set_edge_flags, epmod_set_face_flags,
	epmod_get_vertex_flags, epmod_get_edge_flags, epmod_get_face_flags,
	epmod_get_verts_using_edge, epmod_get_edges_using_vert, epmod_get_faces_using_edge,
	epmod_get_elements_using_face, epmod_get_faces_using_vert, epmod_get_verts_using_face,
	epmod_set_vert,

	epmod_move_selection, epmod_move_slicer, epmod_rotate_selection, epmod_rotate_slicer,
	epmod_scale_selection, epmod_scale_slicer,

	epmod_list_operations, epmod_set_hinge_edge, epmod_get_hinge_edge,
	epmod_bridge_edges,

	epmod_in_slice, epmod_show_operation_dialog, epmod_showing_operation_dialog, 
	epmod_close_operation_dialog, epmod_get_primary_node, epmod_get_node_tm, epmod_set_cut_end, 
	epmod_get_last_cut_end, epmod_clear_last_cut_end, epmod_cut_cancel, epmod_divide_edge, 
	epmod_divide_face,epmod_ring_sel, epmod_loop_sel,epmod_convert_selection_to_border,
	epmod_paintdeform_commit, epmod_paintdeform_cancel,

	epmod_matid_floater,epmod_matid_floatervisible,
	epmod_smgrp_floater,epmod_smgrp_floatervisible

};

/// \brief This class is the published interface for Edit Poly modifiers.
/*! You can use it to manipulate Edit Poly modifiers directly.
/// Implementations of all of these methods can be found in the source for the Edit Poly modifier.
/// usages for most of them can be found there as well. 
*/
#pragma warning(push)
#pragma warning(disable:4100)
class EPolyMod : public FPMixinInterface {
public:
	#pragma warning(push)
	#pragma warning(disable:4238)
	BEGIN_FUNCTION_MAP
		FN_0(epmod_get_sel_level, TYPE_ENUM, GetEPolySelLevel);
		FN_0(epmod_get_mn_sel_level, TYPE_ENUM, GetMNSelLevel);
		VFN_1(epmod_set_sel_level, SetEPolySelLevel, TYPE_ENUM);

		FN_3(epmod_convert_selection, TYPE_INT, EpModConvertSelection, TYPE_ENUM, TYPE_ENUM, TYPE_bool);

		FN_0(epmod_get_operation, TYPE_ENUM, GetPolyOperationID);
		VFN_1(epmod_set_operation, EpModSetOperation, TYPE_ENUM);
		VFN_1(epmod_popup_dialog, EpModPopupDialog, TYPE_ENUM);
		VFN_1(epmod_button_op, EpModButtonOp, TYPE_ENUM);
		VFN_0(epmod_repeat_last, EpModRepeatLast);

		VFNT_0(epmod_commit, EpModCommit);
		VFNT_0(epmod_commit_unless_animating, EpModCommitUnlessAnimating);
		VFNT_0(epmod_commit_and_repeat, EpModCommitAndRepeat);
		VFN_0(epmod_cancel_operation, EpModCancel);

		FN_2(epmod_get_selection, TYPE_BITARRAY, EpModGetSelection, TYPE_ENUM, TYPE_INODE);
		FN_3(epmod_set_selection, TYPE_bool, EpModSetSelection, TYPE_ENUM, TYPE_BITARRAY_BR, TYPE_INODE);
		FN_5(epmod_select, TYPE_bool, EpModSelect, TYPE_ENUM, TYPE_BITARRAY_BR, TYPE_bool, TYPE_bool, TYPE_INODE);

		VFN_1(epmod_set_primary_node, EpModSetPrimaryNode, TYPE_INODE);

		VFN_1(epmod_toggle_command_mode, EpModToggleCommandMode, TYPE_ENUM);
		VFN_1(epmod_enter_command_mode, EpModEnterCommandMode, TYPE_ENUM);
		FN_0(epmod_get_command_mode, TYPE_ENUM, EpModGetCommandMode);
		VFN_1(epmod_enter_pick_mode, EpModEnterPickMode, TYPE_ENUM);
		FN_0(epmod_get_pick_mode, TYPE_ENUM, EpModGetPickMode);

		// Transforms:
		VFNT_4(epmod_move_selection, EpModMoveSelection, TYPE_POINT3_BR, TYPE_MATRIX3_BR, TYPE_MATRIX3_BR, TYPE_BOOL);
		VFNT_4(epmod_rotate_selection, EpModRotateSelection, TYPE_QUAT_BR, TYPE_MATRIX3_BR, TYPE_MATRIX3_BR, TYPE_BOOL);
		VFNT_4(epmod_scale_selection, EpModScaleSelection, TYPE_POINT3_BR, TYPE_MATRIX3_BR, TYPE_MATRIX3_BR, TYPE_BOOL);

		VFNT_3(epmod_move_slicer, EpModMoveSlicePlane, TYPE_POINT3_BR, TYPE_MATRIX3_BR, TYPE_MATRIX3_BR);
		VFNT_4(epmod_rotate_slicer, EpModRotateSlicePlane, TYPE_QUAT_BR, TYPE_MATRIX3_BR, TYPE_MATRIX3_BR, TYPE_BOOL);
		VFNT_4(epmod_scale_slicer, EpModScaleSlicePlane, TYPE_POINT3_BR, TYPE_MATRIX3_BR, TYPE_MATRIX3_BR, TYPE_BOOL);

		// Access to slice plane:
		FN_0(epmod_in_slice_mode, TYPE_bool, EpInSliceMode);
		VFN_0(epmod_reset_slice_plane, EpResetSlicePlane);
		FNT_0(epmod_get_slice_plane_tm, TYPE_MATRIX3_BV, EpGetSlicePlaneTM);
		VFNT_2(epmod_get_slice_plane, EpGetSlicePlane, TYPE_POINT3_BR, TYPE_POINT3_BR);
		VFNT_2(epmod_set_slice_plane, EpSetSlicePlane, TYPE_POINT3_BR, TYPE_POINT3_BR);

		FN_2(epmod_create_vertex, TYPE_INDEX, EpModCreateVertex, TYPE_POINT3, TYPE_INODE);
		FN_2(epmod_create_face, TYPE_INDEX, EpModCreateFace, TYPE_INDEX_TAB, TYPE_INODE);
		FN_3(epmod_create_edge, TYPE_INDEX, EpModCreateEdge, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		VFN_3(epmod_set_diagonal, EpModSetDiagonal, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		VFN_5(epmod_cut, EpModCut, TYPE_ENUM, TYPE_INDEX, TYPE_POINT3, TYPE_POINT3, TYPE_INODE);
		VFN_3(epmod_target_weld_vertex, EpModWeldVerts, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		VFN_3(epmod_target_weld_edge, EpModWeldEdges, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);

		VFNT_2(epmod_attach_node, EpModAttach, TYPE_INODE, TYPE_INODE);
		VFNT_2(epmod_attach_nodes, EpModMultiAttach, TYPE_INODE_TAB_BR, TYPE_INODE);

		VFNT_1(epmod_detach_to_object, EpModDetachToObject, TYPE_TSTR_BR);
		VFNT_1(epmod_create_shape, EpModCreateShape, TYPE_TSTR_BR);

		VFN_3(epmod_set_hinge_edge, EpModSetHingeEdge, TYPE_INDEX, TYPE_MATRIX3, TYPE_INODE);
		FN_1 (epmod_get_hinge_edge, TYPE_INDEX, EpModGetHingeEdge, TYPE_INODE);
		VFN_3(epmod_bridge_borders, EpModBridgeBorders, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		VFN_3(epmod_bridge_polygons, EpModBridgePolygons, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_0(epmod_get_bridge_node, TYPE_INODE, EpModGetBridgeNode);
		VFN_1(epmod_set_bridge_node, EpModSetBridgeNode, TYPE_INODE);
		FN_0(epmod_ready_to_bridge_selected, TYPE_bool, EpModReadyToBridgeSelected);

		FN_1(epmod_get_preserve_map, TYPE_bool, EpModGetPreserveMap, TYPE_INT);
		VFN_2(epmod_set_preserve_map, EpModSetPreserveMap, TYPE_INT, TYPE_bool);

		//VFN_2(epmod_turn_edge, EpModTurnEdge, TYPE_INDEX, TYPE_INODE);
		VFN_3(epmod_turn_diagonal, EpModTurnDiagonal, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);

		// Mesh access methods:
		FN_1 (epmod_get_num_vertices, TYPE_INT, EpMeshGetNumVertices, TYPE_INODE);
		FN_2 (epmod_get_vertex, TYPE_POINT3_BV, EpMeshGetVertex, TYPE_INDEX, TYPE_INODE);
		FN_2 (epmod_get_vertex_face_count, TYPE_INT, EpMeshGetVertexFaceCount, TYPE_INDEX, TYPE_INODE);
		FN_3 (epmod_get_vertex_face, TYPE_INDEX, EpMeshGetVertexFace, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_2 (epmod_get_vertex_edge_count, TYPE_INT, EpMeshGetVertexEdgeCount, TYPE_INDEX, TYPE_INODE);
		FN_3 (epmod_get_vertex_edge, TYPE_INDEX, EpMeshGetVertexEdge, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);

		FN_1 (epmod_get_num_edges, TYPE_INT, EpMeshGetNumEdges, TYPE_INODE);
		FN_3 (epmod_get_edge_vertex, TYPE_INDEX, EpMeshGetEdgeVertex, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_3 (epmod_get_edge_face, TYPE_INDEX, EpMeshGetEdgeFace, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);

		FN_1 (epmod_get_num_faces, TYPE_INT, EpMeshGetNumFaces, TYPE_INODE);
		FN_2 (epmod_get_face_degree, TYPE_INT, EpMeshGetFaceDegree, TYPE_INDEX, TYPE_INODE);
		FN_3 (epmod_get_face_vertex, TYPE_INDEX, EpMeshGetFaceVertex, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_3 (epmod_get_face_edge, TYPE_INDEX, EpMeshGetFaceEdge, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_4 (epmod_get_face_diagonal, TYPE_INDEX, EpMeshGetFaceDiagonal, TYPE_INDEX, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_2 (epmod_get_face_material, TYPE_INDEX, EpMeshGetFaceMaterial, TYPE_INDEX, TYPE_INODE);
		FN_2 (epmod_get_face_smoothing_group, TYPE_DWORD, EpMeshGetFaceSmoothingGroup, TYPE_INDEX, TYPE_INODE);

		FN_1 (epmod_get_num_map_channels, TYPE_INT, EpMeshGetNumMapChannels, TYPE_INODE);
		FN_2 (epmod_get_map_channel_active, TYPE_bool, EpMeshGetMapChannelActive, TYPE_INT, TYPE_INODE);
		FN_2 (epmod_get_num_map_vertices, TYPE_INT, EpMeshGetNumMapVertices, TYPE_INT, TYPE_INODE);
		FN_3 (epmod_get_map_vertex, TYPE_POINT3_BV, EpMeshGetMapVertex, TYPE_INT, TYPE_INDEX, TYPE_INODE);
		FN_4 (epmod_get_map_face_vertex, TYPE_INDEX, EpMeshGetMapFaceVertex, TYPE_INT, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);

		VFN_1 (epmod_list_operations, EpModListOperations, TYPE_INODE);

		VFN_1 (epmod_local_data_changed, EpModLocalDataChanged, TYPE_DWORD);
		VFN_0 (epmod_refresh_screen, EpModRefreshScreen);
		FN_0 (epmod_in_slice, TYPE_bool, EpInSlice);
		FN_0 (epmod_show_operation_dialog, TYPE_bool, EpModShowOperationDialog);
		FN_0 (epmod_showing_operation_dialog, TYPE_bool, EpModShowingOperationDialog);
		VFN_0 (epmod_close_operation_dialog, EpModCloseOperationDialog);
		FN_0(epmod_get_primary_node, TYPE_INODE, EpModGetPrimaryNode);
		FNT_1(epmod_get_node_tm, TYPE_MATRIX3_BV, EpModGetNodeTM_FPS, TYPE_INODE);
		VFN_2(epmod_set_cut_end, EpModSetCutEnd, TYPE_POINT3, TYPE_INODE);
		FN_0 (epmod_get_last_cut_end, TYPE_INDEX, EpModGetLastCutEnd);
		VFN_0 (epmod_clear_last_cut_end, EpModClearLastCutEnd);
		VFN_1 (epmod_cut_cancel, EpModCutCancel, TYPE_INODE);
		VFN_3 (epmod_divide_edge, EpModDivideEdge, TYPE_INDEX, TYPE_FLOAT, TYPE_INODE);
		VFN_3 (epmod_divide_face, EpModDivideFace, TYPE_INDEX, TYPE_FLOAT_TAB, TYPE_INODE);

		//Bridge methods introduced in Max 8 - (Moved from EPolyMod8 to here)
		VFN_3(epmod_bridge_edges, EpModBridgeEdges, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		VFN_3(epmod_ring_sel, EpModSetRingShift, TYPE_INT, TYPE_bool, TYPE_bool);
		VFN_3(epmod_loop_sel, EpModSetLoopShift, TYPE_INT, TYPE_bool, TYPE_bool);
		FN_2(epmod_convert_selection_to_border, TYPE_INT, EpModConvertSelectionToBorder, TYPE_ENUM, TYPE_ENUM);
		VFN_0(epmod_paintdeform_commit,EpModPaintDeformCommit);
		VFN_0(epmod_paintdeform_cancel,EpModPaintDeformCancel);

		//Functionality added to match editable poly - (Added in Max 9)
		FN_2 ( epmod_get_face_normal, TYPE_POINT3_BV, EPMeshGetFaceNormal, TYPE_INDEX, TYPE_INODE );
		FN_2 ( epmod_get_face_center, TYPE_POINT3_BV, EPMeshGetFaceCenter, TYPE_INDEX, TYPE_INODE );
		FN_2 ( epmod_get_face_area, TYPE_FLOAT, EPMeshGetFaceArea, TYPE_INDEX, TYPE_INODE );
		FN_1 ( epmod_get_open_edges, TYPE_BITARRAY, EPMeshGetOpenEdges, TYPE_INODE );
		FN_4 ( epmod_get_verts_by_flag, TYPE_bool, EPMeshGetVertsByFlag, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD, TYPE_INODE );
		FN_4 ( epmod_get_edges_by_flag, TYPE_bool, EPMeshGetEdgesByFlag, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD, TYPE_INODE );
		FN_4 ( epmod_get_faces_by_flag, TYPE_bool, EPMeshGetFacesByFlag, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD, TYPE_INODE );
		VFN_5 ( epmod_set_vertex_flags, EPMeshSetVertexFlags, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD, TYPE_bool, TYPE_INODE );
		VFN_5 ( epmod_set_edge_flags, EPMeshSetEdgeFlags, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD, TYPE_bool, TYPE_INODE );
		VFN_5 ( epmod_set_face_flags, EPMeshSetFaceFlags, TYPE_BITARRAY_BR, TYPE_DWORD, TYPE_DWORD, TYPE_bool, TYPE_INODE );
		FN_2 ( epmod_get_vertex_flags, TYPE_INT, EPMeshGetVertexFlags, TYPE_INDEX, TYPE_INODE );
		FN_2 ( epmod_get_edge_flags, TYPE_INT, EPMeshGetEdgeFlags, TYPE_INDEX, TYPE_INODE );
		FN_2 ( epmod_get_face_flags, TYPE_INT, EPMeshGetFaceFlags, TYPE_INDEX, TYPE_INODE );
		VFN_3 ( epmod_get_verts_using_edge, EPMeshGetVertsUsingEdge, TYPE_BITARRAY_BR, TYPE_BITARRAY_BR, TYPE_INODE );
		VFN_3 ( epmod_get_edges_using_vert, EPMeshGetEdgesUsingVert, TYPE_BITARRAY_BR, TYPE_BITARRAY_BR, TYPE_INODE );
		VFN_3 ( epmod_get_faces_using_edge, EPMeshGetFacesUsingEdge, TYPE_BITARRAY_BR, TYPE_BITARRAY_BR, TYPE_INODE );
		VFN_4 ( epmod_get_elements_using_face, EPMeshGetElementsUsingFace, TYPE_BITARRAY_BR, TYPE_BITARRAY_BR, TYPE_BITARRAY_BR, TYPE_INODE );
		VFN_3 ( epmod_get_faces_using_vert, EPMeshGetFacesUsingVert, TYPE_BITARRAY_BR, TYPE_BITARRAY_BR, TYPE_INODE );
		VFN_3 ( epmod_get_verts_using_face, EPMeshGetVertsUsingFace, TYPE_BITARRAY_BR, TYPE_BITARRAY_BR, TYPE_INODE );
		VFN_3 ( epmod_set_vert, EPMeshSetVert, TYPE_BITARRAY_BR, TYPE_POINT3_BR, TYPE_INODE );

		VFN_0(epmod_smgrp_floater, SmGrpFloater);
		FN_0(epmod_smgrp_floatervisible,TYPE_BOOL, SmGrpFloaterVisible);

		VFN_0(epmod_matid_floater, MatIDFloater);
		FN_0(epmod_matid_floatervisible,TYPE_BOOL, MatIDFloaterVisible);

	END_FUNCTION_MAP
	#pragma warning(pop)

	///  \brief 
	/// For system use only - retrieves the function publishing interface description.
	/// 
	FPInterfaceDesc *GetDesc ();

	///  \brief 
	/// Returns the Edit Poly selection level - one of the ePolyModSelLevel enum defined above,
	/// such as EPM_SL_VERTEX.
	/// 
	virtual int GetEPolySelLevel()=0;

	///  \brief 
	/// Returns the MNMesh selection level - one of MNM_SL_OBJECT, MNM_SL_VERTEX, MNM_SL_EDGE, or MNM_SL_FACE.
	/// 
	virtual int GetMNSelLevel()=0;

	///  \brief 
	/// Sets the Edit Poly selection level.
	/// 
	///  \param sl  One of the ePolyModSelLevel enum defined above - such as EPM_SL_VERTEX 
	virtual void SetEPolySelLevel (int sl)=0;

	///  \brief 
	/// Converts selection between two Edit Poly selection levels, by selecting adjacent subobjects, such as
	/// edges using selected vertices, or vertices used by selected faces, etc.
	/// 
	///  \param epSelLevelFrom  The selection level to convert from. 
	///  \param epSelLevelTo  The selection level to convert to - subobjects in this
	/// selection level will have their selection set or cleared based on this algorithm. 
	///  \param requireAll  Indicates if subobjects in the "to" level should be selected
	/// if _any_ subobjects in the "from" level are selected (false) or if _all_ subobjects in the
	/// "from" level are selected (true).
	///  \return  The number of subobjects this method selected in the "To" level.
	///
	///  <br><b>Example: </b>  This could be used to convert the current vertex selection to a selection of all border loops
	/// containing _only_ selected vertices by calling
	/// EpModConvertSelection (EPM_SL_VERTEX, EPM_SL_BORDER, true);
	virtual int EpModConvertSelection (int epSelLevelFrom, int epSelLevelTo, bool requireAll) { return 0; }

	///  \brief 
	/// Returns the "current" Edit Poly operation.  If there is no current operation ep_op_null is returned.
	/// (Values are from the epolyModButtonOp enum above.)
	/// 
	virtual int GetPolyOperationID () { return ep_op_null; }

	///  \brief 
	/// Returns the parameter block for the Edit Poly modifier.  (Clients can use this to control the
	/// individual parameters of the modifier.  Parameters are listed above in the epolyModParameters enum.)
	/// 
	virtual IParamBlock2 *getParamBlock () { return NULL; }

	///  \brief 
	/// Returns the Preserve settings for all map channels.  (Does not reflect the epm_preserve_maps parameter, however.)
	/// 
	virtual MapBitArray GetPreserveMapSettings () const { return MapBitArray(true, false); }

	///  \brief 
	/// Sets the Preserve settings for all map channels.  (Does not affect the epm_preserve_maps parameter, however.)
	/// 
	virtual void SetPreserveMapSettings (const MapBitArray & mapSettings) { }

	///  \brief 
	/// Sets whether a particular map channel will be "preserved" in geometric operations.
	/// (Note: This does not affect the epm_preserve_maps parameter, it just changes the setting
	/// for this particular map channel.)
	/// 
	virtual void EpModSetPreserveMap (int mapChannel, bool preserve) { }

	///  \brief 
	/// Indicates whether a particular map channel is set to be "preserved" in geometric operations.
	/// (Note: This does not indicate whether the epm_preserve_maps parameter is set to true, it only
	/// indicates the setting for this particular map channel.)
	/// 
	virtual bool EpModGetPreserveMap (int mapChannel) { return (mapChannel>0); }

	///  \brief 
	/// Invalidates the computed distances from selected vertices that are used to generate soft selections.
	/// 
	virtual void InvalidateDistanceCache () { }

	///  \brief 
	/// Invalidates the computed soft selections (but not the distances from selected vertices that the
	/// soft selections are based on).
	/// 
	virtual void InvalidateSoftSelectionCache () { }

	///  \brief 
	/// Sets the current operation.
	/// 
	///  \param opcode  Indicates the desired operation, from the epolyModButtonOp enum above 
	virtual void EpModSetOperation (int opcode) { }

	///  \brief 
	/// Sets the operation to the one given, and then pops up the operation dialog, if one exists.
	/// 
	virtual void EpModPopupDialog (int opcode) { EpModSetOperation (opcode); EpModShowOperationDialog(); }

	///  \brief 
	/// Applies the operation given, and commits to it immediately.
	/// 
	virtual void EpModButtonOp (int opcode) { }

	///  \brief 
	/// Indicates to the EditPolyMod that some channels of its mesh have changed.  Forces an update of UI
	/// elements based on the mesh, such as the Number Selected display and the Smoothing Group buttons,
	/// and also notifies the Edit Poly's dependents of the change.
	/// 
	virtual void EpModLocalDataChanged (DWORD parts) { }

	///  \brief 
	/// Triggers a redraw of the viewports at the current time.
	/// 
	virtual void EpModRefreshScreen () { }

	///  \brief 
	/// Commits to the current operation unless we're set to "animation mode" (epm_animation_mode parameter is true)
	/// and the current operation supports animation.
	/// 
	virtual void EpModCommitUnlessAnimating (TimeValue t) { }

	///  \brief 
	/// Commits to the current operation, and clears any animation.
	/// 
	virtual void EpModCommit (TimeValue t) { }

	///  \brief 
	/// Commits to the current operation, but maintains it as the current operation with the same settings
	/// (and any animation in those parameters).
	/// 
	virtual void EpModCommitAndRepeat (TimeValue t) { }

	///  \brief 
	/// Cancels the current operation.
	/// 
	virtual void EpModCancel () { }

	///  \brief 
	/// Toggles a given command mode - if it's not currently in this mode it will enter it,
	/// otherwise it will exit it.
	/// 
	///  \param mode  The command mode, from the epolyModCommandMode enum above. 
	virtual void EpModToggleCommandMode (int mode) { }

	///  \brief 
	/// Exits any previous Edit Poly command modes, and enters the command mode given.
	/// 
	///  \param mode  The command mode, from the epolyModCommandMode enum above. 
	virtual void EpModEnterCommandMode (int mode) { }

	///  \brief 
	/// Enters the pick mode given.
	/// 
	///  \param mode  The pick mode, from the epolyModPickMode enum above. 
	virtual void EpModEnterPickMode (int mode) { }

	///  \brief 
	/// Retrieves the current Edit Poly command mode in use, if any.
	/// 
	///  \return  The current command mode from the epolyModCommandMode above, or -1 if the current
	/// command mode is not any of the Edit Poly command modes. 
	virtual int EpModGetCommandMode () { return -1; }

	///  \brief 
	/// Retrieves the current Edit Poly pick mode in use, if any.
	/// 
	///  \return  The current pick mode (from the epolyModPickMode above), or -1 if the current
	/// pick mode is not any of the Edit Poly pick modes. 
	virtual int EpModGetPickMode () { return -1; }

	///  \brief 
	/// Returns a BitArray containing the subobject selection in the desired selection level.
	/// 
	///  \param meshSelLevel  The MNMesh selection level, such as MNM_SL_VERTEX for vertex or MNM_SL_CURRENT for "the current level". 
	///  \param pNode  If this Edit Poly modifier is instanced across multiple nodes, this parameter specifies which
	/// node we're inquiring about.  
	///  \return  A pointer to an internal BitArray containing the selection.  This BitArray should not be altered!  It is available for read only. 
	virtual BitArray *EpModGetSelection (int meshSelLevel, INode *pNode=NULL) { return NULL; }

	///  \brief 
	/// Sets the subobject selection explicitly.
	/// 
	///  \param meshSelLevel  The MNMesh selection level, such as MNM_SL_VERTEX for vertex or MNM_SL_CURRENT for "the current level". 
	///  \param selection  A BitArray containing the desired selection. 
	///  \param pNode  If this Edit Poly modifier is instanced across multiple nodes, this parameter specifies which
	/// node we want to set selection in. 
	virtual bool EpModSetSelection (int meshSelLevel, BitArray & selection, INode *pNode=NULL) { return false; }

	///  \brief 
	/// Change the selection state of a subset of subobjects - select them, invert their selection, or deselect them.
	/// 
	///  \param meshSelLevel  The MNMesh selection level, such as MNM_SL_VERTEX for vertex or MNM_SL_CURRENT for "the current level". 
	///  \param selection  A BitArray indicating which subobjects we want to change the selection state of. 
	///  \param invert  If true, the selection state of the subobjects should be inverted.  Otherwise, it should be set or cleared depending on "select" 
	///  \param select  If true, the subobjects indicated should be selected; if false, the subobjects indicated should be deselected.  Ignored if "invert" is true. 
	///  \param pNode  If this Edit Poly modifier is instanced across multiple nodes, this parameter specifies which
	/// node we want to set selection in. 
	virtual bool EpModSelect (int meshSelLevel, BitArray & selection, bool invert=false, bool select=true, INode *pNode=NULL) { return false; }

	///  \brief 
	/// Force any hit-testing on the Edit Poly modifier to use the hit level given,
	/// instead of the default hit level associated with the current subobject level.
	/// 
	///  \param hlo  The Hit Level - one of SUBHIT_MNVERTS, SUBHIT_MNEDGES,
	/// SUBHIT_MNFACES, or SUBHIT_MNDIAGONALS, along with SUBHIT_OPENONLY if you want
	/// to restrict hit-testing to "open" vertices or edges (ie those on borders)
	///
	///  <br><b>Example: </b>  Edit Poly's Hinge from Edge interactive command mode uses this to hit-test on faces
	/// even though the user is in Polygon subobject level.  The Cut mode uses it to hit-test on
	/// all three subobject levels, one at a time. 
	virtual void SetHitLevelOverride (DWORD hlo) { }

	///  \brief 
	/// Clear any hit level override, and let hit-testing fall back on the normal hit level for
	/// subobject selection.
	/// 
	virtual void ClearHitLevelOverride () { }

	///  \brief 
	/// Returns the current hit level override, or 0 if there is none.
	/// 
	///  \return  The Hit Level - one of SUBHIT_MNVERTS, SUBHIT_MNEDGES,
	/// SUBHIT_MNFACES, or SUBHIT_MNDIAGONALS, along with SUBHIT_OPENONLY if
	/// hit-testing is restricted to "open" vertices or edges (ie those on borders) 
	virtual DWORD GetHitLevelOverride () { return 0x0; }

	///  \brief 
	/// Indicates the current hit level.  If there's a HitLevelOverride in place, that's returned;
	/// otherwise this method returns the regular hit level, which is typically the hit level for
	/// the current selection level.  If "Select by vertex" is turned on, SUBHIT_MNVERTS will be
	/// returned.
	/// 
	///  \param selByVert  If you specifically want to know whether "Select by vertex" is being
	/// used currently, pass a pointer to an int here.  The int pointed to will be set to true if
	/// select by vertex is on, or false if not.
	///  \return  The Hit Level - one of SUBHIT_MNVERTS, SUBHIT_MNEDGES,
	/// SUBHIT_MNFACES, or SUBHIT_MNDIAGONALS, along with SUBHIT_OPENONLY if
	/// hit-testing is restricted to "open" vertices or edges (ie those on borders) 
	virtual DWORD CurrentHitLevel (int *selByVert=NULL) { return 0x0; }

	///  \brief 
	/// Normally, all hit-testing on Edit Poly meshes is done on the mesh _before_ the "current
	/// operation".  (This makes it possible to change selections underneath a Bevel or Extrude
	/// or Vertex Weld.)  If you want to hit-test on the result of the current operation, you need
	/// to call this method.
	/// 
	///  \param ignoreNew  Controls whether subobjects that are newly created by the current
	/// operation can be hit-tested.  For instance, if the current operation is an Extrude, this controls
	/// whether or not the sides of the extrusion can generate hits.
	///  \remark  This is typically used in Edit Poly command modes, such as Chamfer or Extrude, after
	/// the user has entered the mode, but while the last operation might still be the "current" one,
	/// so that we can hit-test as though the current operation had already been committed.  It's also
	/// used in command modes like Create Face so that we can hit-test on the freshly created vertices. 
	virtual void SetHitTestResult (bool ignoreNew=false) { }

	///  \brief 
	/// Clears any HitTestResult settings, and falls back on hit-testing before the current operation.
	/// (See SetHitTestResult for more details.)
	/// 
	virtual void ClearHitTestResult () { }

	///  \brief 
	/// Sets the display level override, to force display of a particular type of subobject
	/// regardless of the current subobject level.
	/// 
	///  \param dlo  The set of display flags to show.  These are MNDISP flags,
	/// such as MNDISP_VERTTICKS, that are defined in MNMesh.h.
	///
	///  <br><b>Example: </b>  Edit Poly's Create Face command mode uses this to show vertex ticks so that
	/// the user can see what they're clicking on. 
	virtual void SetDisplayLevelOverride (DWORD dlo) { }

	///  \brief 
	/// Clear any display level override, and let hit-testing fall back on the normal display for
	/// the current selection level.
	/// 
	virtual void ClearDisplayLevelOverride () { }

	///  \brief 
	/// Returns any current DisplayLevelOverride.  See SetDisplayLevelOverride for more details.
	/// 
	///  \return  The set of display flags to show, or 0 for no override currently set.  These are MNDISP flags,
	/// such as MNDISP_VERTTICKS, that are defined in MNMesh.h. 
	virtual DWORD GetDisplayLevelOverride () { return 0x0; }

	///  \brief 
	/// This method is used to temporarily force the Edit Poly to ignore backfacing subobjects
	/// when hit-testing.
	/// 
	///  \param force  True to force ignoring backfacing, false to fall back on normal hit-testing. 
	virtual void ForceIgnoreBackfacing (bool force) { }

	///  \brief 
	/// Indicates whether or not the Edit Poly is currently set to force ignoring backfacing subobjects.
	/// See ForceIgnoreBackfacing for details.
	/// 
	virtual bool GetForceIgnoreBackfacing () { return false; }

	///  \brief 
	/// Move the current subobject selection.  This method uses the current selection and soft selection,
	/// and respects the settings for Constraints and for Preserving texture maps.
	/// 
	///  \param val  The Point3 representing the move. 
	///  \param partm  The 'parent' transformation matrix. 
	///  \param tmAxis  The matrix that represents the axis system. This is the space in which the transformation is taking place. 
	///  \param localOrigin  This parameter is ignored 
	///  \param t  The time at which to move the points.  This time is used to generate vertex
	/// animation keys, if appropriate, as well as to obtain the current soft selection parameters. 
	virtual void EpModMoveSelection (Point3 &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t) { }

	///  \brief 
	/// Rotate the current subobject selection.  This method uses the current selection and soft selection,
	/// and respects the settings for Constraints and for Preserving texture maps.
	/// 
	///  \param val  The Quaternion representing the rotation. 
	///  \param partm  The 'parent' transformation matrix. 
	///  \param tmAxis  The matrix that represents the axis system. This is the space in which the transformation is taking place. 
	///  \param localOrigin  This parameter is ignored 
	///  \param t  The time at which to rotate the points.  This time is used to generate vertex
	/// animation keys, if appropriate, as well as to obtain the current soft selection parameters. 
	virtual void EpModRotateSelection (Quat &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t) { }

	///  \brief 
	/// Scale the current subobject selection.  This method uses the current selection and soft selection,
	/// and respects the settings for Constraints and for Preserving texture maps.
	/// 
	///  \param val  A Point3 representing the scale in each dimension. 
	///  \param partm  The 'parent' transformation matrix. 
	///  \param tmAxis  The matrix that represents the axis system. This is the space in which the transformation is taking place. 
	///  \param localOrigin  This parameter is ignored 
	///  \param t  The time at which to scale the points.  This time is used to generate vertex
	/// animation keys, if appropriate, as well as to obtain the current soft selection parameters. 
	virtual void EpModScaleSelection (Point3 &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t) { }

	///  \brief 
	/// Move the Slice Plane that's used in the ep_op_slice and ep_op_slice_face operations.
	/// 
	///  \param val  The Point3 representing the move. 
	///  \param partm  The 'parent' transformation matrix. 
	///  \param tmAxis  The matrix that represents the axis system. This is the space in which the transformation is taking place. 
	///  \param t  The time at which to move the plane.  This time is used to generate animation keys, if appropriate. 
	virtual void EpModMoveSlicePlane (Point3 &val, Matrix3 &partm, Matrix3 &tmAxis, TimeValue t) { }

	///  \brief 
	/// Rotate the Slice Plane that's used in the ep_op_slice and ep_op_slice_face operations.
	/// 
	///  \param val  The Quaternion representing the rotation. 
	///  \param partm  The 'parent' transformation matrix. 
	///  \param tmAxis  The matrix that represents the axis system. This is the space in which the transformation is taking place. 
	///  \param localOrigin  When TRUE the transformation is occurring about the sub-object's local origin; otherwise, the rotation occurs in object space. 
	///  \param t  The time at which to move the plane.  This time is used to generate animation keys, if appropriate. 
	virtual void EpModRotateSlicePlane (Quat &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t) { }

	///  \brief 
	/// Scale the Slice Plane that's used in the ep_op_slice and ep_op_slice_face operations.  Note that scaling
	/// the slice plane with localOrigin == true has no effect on the actual slice effect; it only affects the way the slice plane is displayed.
	/// 
	///  \param val  The Point3 representing the scale in each dimension. 
	///  \param partm  The 'parent' transformation matrix. 
	///  \param tmAxis  The matrix that represents the axis system. This is the space in which the transformation is taking place. 
	///  \param localOrigin  When TRUE the transformation is occurring about the sub-object's local origin; otherwise, the scale occurs in object space. 
	///  \param t  The time at which to move the plane.  This time is used to generate animation keys, if appropriate. 
	virtual void EpModScaleSlicePlane (Point3 &val, Matrix3 &partm, Matrix3 &tmAxis, BOOL localOrigin, TimeValue t) { }

	///  \brief 
	/// Resets the slice plane used in the slice operations so that it lies in the XY plane in object space.
	/// 
	virtual void EpResetSlicePlane () { }

	///  \brief 
	/// Returns the current transform of the slice plane.
	/// 
	virtual Matrix3 EpGetSlicePlaneTM (TimeValue t) { return Matrix3(true); }

	///  \brief 
	/// Retrieves the current slice plane, by its normal and center.
	/// 
	virtual void EpGetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, TimeValue t) { }

	///  \brief 
	/// Sets the slice plane to use the normal and center given.
	/// 
	virtual void EpSetSlicePlane (Point3 & planeNormal, Point3 & planeCenter, TimeValue t) { }

	///  \brief 
	/// Indicates if we're currently in the "Slice Plane" mode, which is not a command mode, but rather
	/// a different way of handling the regular (move, rotate, scale) command modes.
	/// 
	virtual bool EpInSliceMode () { return false; }

	///  \brief 
	/// Indicates if we're currently in a Slice operation (ep_op_slice or ep_op_slice_face).
	/// 
	virtual bool EpInSlice () { return false; }

	///  \brief 
	/// Returns the Edit Poly cast as a modifier.
	/// 
	virtual Modifier* GetModifier ()=0;

	///  \brief 
	/// Returns NULL if the Edit Poly is not currently up in the modifier panel, or the IObjParam interface pointer otherwise.
	/// 
	virtual IObjParam* EpModGetIP() { return NULL; }

	///  \brief 
	/// Returns the handle of the dialog specified
	/// 
	///  \param paramPanelID  This is an int representing the ID of the dialog.
	/// This enum should be moved to this header, but currently it resides in maxsdk/samples/mesh/EditPoly/EditPoly.h.
	/// Here are the IDs:
	/// 0 - ep_animate - The dialog displaying the current operation, and controlling whether we're in model or animate mode
	/// 1 - ep_select - The subobject selection dialog
	/// 2 - ep_softsel - The soft selection dialog
	/// 3 - ep_geom - The dialog with general Edit Geometry operations, such as Create, Collapse, and Attach.
	/// 4 - ep_subobj - The subobject level specific Edit dialog, such as "Edit Vertex" or "Edit Borders",
	/// 5 - ep_surface - The face-level dialog for editing surface material surface features
	/// 6 - ep_settings - The popup dialog with settings for the current operation
	/// 7 - ep_paintdeform - the Paint Deformation dialog.
	/// 8 - ep_face_smooth - The face-level dialog for editing smoothing groups.
	/// If the dialog you're requesting is not currently displayed, NULL will be returned.
	/// 
	///  <br><b>Example: </b>  The Edit Poly command modes use this method to access the buttons for their command mode,
	/// to set them to checked when entering the command mode and unchecked when leaving. 
	virtual HWND GetDlgHandle(int paramPanelID) { return NULL; }

	///  \brief 
	/// Updates the Align normal and offset parameters based on the current viewport and on the
	/// ep_align_type parameter.
	/// If there's no viewport available, method does nothing.
	/// 
	virtual void UpdateAlignParameters (TimeValue t) { }

	///  \brief 
	/// Makes sure that all the LocalModData used by the Edit Poly modifiers has an up to date cache.
	/// This should be called before trying to obtain information about the mesh.
	/// 
	virtual void UpdateCache(TimeValue t) { }

	///  \brief 
	/// Show the popup dialog associated with the current operation.  Does nothing if there is no current
	/// operation, or if the current operation has no associated popup dialog.
	/// 
	///  \return  True if we succeeded in displaying a popup dialog, false if we couldn't find one to display. 
	virtual bool EpModShowOperationDialog () { return false; }

	///  \brief 
	/// Indicates whether or not there is currently a popup dialog being displayed with parameters for the
	/// current operation.
	/// 
	virtual bool EpModShowingOperationDialog () { return false; }

	///  \brief 
	/// Close the popup dialog which shows the current operation's parameters, if it's currently being displayed.
	/// 
	virtual void EpModCloseOperationDialog () { }

	///  \brief 
	/// Repeat the last operation.  If there is a "current" operation, it will be committed and repeated, without
	/// changing any parameters or removing any animations.  If there is not, the last recorded operation, if any, will
	/// be applied and committed.  (In Max 7.0, unfortunately, the identity of this last operation is not exposed in
	/// the SDK.  You can examine it in the Edit Poly source, maxsdk/samples/mesh/EditPoly, the EditPolyMod::mLastOperation
	/// data member.
	/// 
	virtual void EpModRepeatLast () { }

	///  \brief 
	/// Get the current "Primary" node, which is used by default when node-specific commands are given and the
	/// node given is NULL.
	/// 
	virtual INode *EpModGetPrimaryNode () { return NULL; }

	///  \brief 
	/// Set the current "Primary" node, which is used by default when node-specific commands are given and the
	/// node given is NULL.
	/// 
	virtual void EpModSetPrimaryNode (INode *node) { }

	///  \brief 
	/// Retrieve the transform of the Edit Poly's node.
	/// 
	///  \param t  The time at which to retrieve the node's TM. 
	///  \param node  The specific node to use, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  If the node is given, we just return node->GetObjectTM (t).  This method is
	/// more interesting when you just want to know what the TM is of the primary node, without
	/// bothering to retrive it. 
	virtual Matrix3 EpModGetNodeTM (TimeValue t, INode *node=NULL) { return Matrix3(1); }

	//  \brief 
	// Retrieve the transform of the Edit Poly's node.
	// This method is used by the FPS exposure to expose the EpModGetNodeTM method. The FPS 
	// exposure requires that the time parameter be the last argument.
	// 
	Matrix3 EpModGetNodeTM_FPS (INode *node=NULL, TimeValue t=0) { return EpModGetNodeTM(t,node); }

	///  \brief 
	/// Create a new vertex in the mesh.
	/// 
	///  \param p  The location of the vertex in object space 
	///  \param pNode  The specific node to use, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \return  The (0-based) index of the newly created vertex 
	virtual int EpModCreateVertex (Point3 p, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Create a new face in the mesh
	/// 
	///  \param vertex  A table of the vertices to use as the corners of the face 
	///  \param pNode  The specific node to use, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \return  The (0-based) index of the newly created face 
	virtual int EpModCreateFace (Tab<int> *vertex, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Create a new edge in the mesh.  The vertices which we're creating a new edge between must
	/// be on the same polygon, but not have an existing edge between them.  The polygon is split in two
	/// by the new edge.
	/// 
	///  \param v1  The start vertex of the new edge. 
	///  \param v2  The end vertex of the new edge. 
	///  \param pNode  The specific node to use, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \return  The (0-based) index of the newly created edge 
	virtual int EpModCreateEdge (int v1, int v2, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Change the triangulation of a polygon such that there will be a diagonal between the two
	/// vertices given.  (The two vertices must obviously be used by the same polygon.)
	/// 
	///  \param v1  The start vertex of the new diagonal. 
	///  \param v2  The end vertex of the new diagonal. 
	///  \param pNode  The specific node to use, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.
	///
	///  <br><b>Example: </b>  This is used in the Edit Poly "Edit Triangulation" command mode
	virtual void EpModSetDiagonal (int v1, int v2, INode *pNode=NULL) { }

	///  \brief 
	/// Set up a Cut operation, starting at a specific subobject and at a specific point.  (Use EpModSetCutEnd
	/// to finish the cut.)
	/// 
	///  \param startLevel  The starting subobject level, which should be an MNMesh level such as
	/// MNM_SL_VERTEX for vertex or MNM_SL_FACE for face. 
	///  \param startIndex  The index of the starting subobject.  For instance, if startLevel==MNM_SL_EDGE,
	/// and startIndex==4, the Cut should start on edge 4. 
	///  \param startPoint  The location (in object space) of the start of the cut.  The end of the
	/// cut is also initialized to this value, and can be further modified by using EpModSetCutEnd. 
	///  \param normal  The normal direction for the Cut.  In the Cut command mode, this is generally
	/// set to the viewport direction.  Cuts should occur where the plane defined by the start point, the
	/// end point, and this normal intersects the mesh. 
	///  \param pNode  The specific node to use, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual void EpModCut (int startLevel, int startIndex, Point3 startPoint, Point3 normal, INode *pNode=NULL) { }

	///  \brief 
	/// Set the end point of a cut.
	/// 
	///  \param endPoint  The location of the end of the Cut, in object space 
	///  \param pNode  The specific node to use, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual void EpModSetCutEnd (Point3 endPoint, INode *pNode=NULL) { }

	///  \brief 
	/// If the last Cut we made finished successfully, this method will return the index of the vertex
	/// at the end of the cut.  If there was no previous Cut or if it couldn't reach all the way to the
	/// desired end point, this method will return -1.
	/// 
	///  <br><b>Example: </b>  The Cut mode uses this method to generate a series of sequential cuts, where each cut
	/// begins exactly on the vertex at the end of the last cut. 
	virtual int EpModGetLastCutEnd () { return -1; }

	///  \brief 
	/// Resets the "last cut end" data to -1.  (See EpModGetLastCutEnd for more details.)
	/// 
	virtual void EpModClearLastCutEnd () { }

	///  \brief 
	/// Please ignore - this method was never implemented, it does nothing.  At some point in the future,
	/// it may be used to cancel out of a cut, but this probably isn't necessary since EpModCancel does that effectively.
	/// 
	virtual void EpModCutCancel (INode *pNode=NULL) { }

	///  \brief 
	/// Divides edge, inserting a vertex partway along its length.  Used in the Edit Poly "Insert Vertex" mode
	/// in the Edge subobject level.
	/// 
	///  \param edge  The edge to divide 
	///  \param prop  The proportion along the edge for the location of the new vertex.  This should be
	/// in the range 0-1, where 0 is on top of the edge's v1 vertex, 1 is on top of the v2 vertex, and .5 is
	/// exactly in the middle.
	///  \param pNode  The specific node to use, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual void EpModDivideEdge (int edge, float prop, INode *pNode=NULL) { }

	///  \brief 
	/// Divides the face, inserting a vertex in it and splitting the face up into triangles connecting the
	/// new vertex with all the face's original edges.
	/// 
	///  \param face  The face to divide 
	///  \param bary  A set of weights for the corners of the face.  This table should have Count equal
	/// to the face's degree, and its values should add up to 1.  NULL is not permitted.  If you want a vertex
	/// in the center of the polygon, set all values to 1.0f / (face degree).
	///  \param pNode  The specific node to use, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual void EpModDivideFace (int face, Tab<float> *bary, INode *pNode=NULL) { }

	///  \brief 
	/// Weld two vertices.  In order for the operation to succeed, the vertices must either
	/// share an edge, or both be on borders.  Success can be detected by checking vertex v2's
	/// MN_DEAD flag after the operation - if it's set, the weld succeeded.
	/// 
	/// \param v1 index of the first vertex
	/// \param v2 index of the second vertex
	///  \param pNode  The specific node to use, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual void EpModWeldVerts (int v1, int v2, INode *pNode=NULL) { }

	///  \brief 
	/// Weld two edges (by welding the vertex endpoints).
	/// In order for the operation to succeed, both edges must be on borders - their "f2" face
	/// should be empty (-1).  Success can be detected by checking edge e2's MN_DEAD flag after
	/// the operation - if it's set, the weld succeeded.
	/// 
	/// \param e1 index of the first edge
	/// \param e2 index of the second edge
	///  \param pNode  The specific node to use, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual void EpModWeldEdges (int e1, int e2, INode *pNode=NULL) { }

	///  \brief 
	/// Attach another mesh to this one.
	/// 
	///  \param node  The node containing the mesh to attach.  (If the object contained is not a mesh,
	/// it will be converted to a mesh.) 
	///  \param pNode  The specific node to attach to, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL. 
	///  \param t  The time at which to attach it.  (This is the time at which the other node is
	/// evaluated, which may be important if the other node is animated.) 
	///  \remark  Note that this method deletes the original node which is being attached. 
	virtual void EpModAttach (INode *node, INode *pNode=NULL, TimeValue t=0) { }

	///  \brief 
	/// Attach a bunch of other meshes to this one.
	/// 
	///  \param nodeTab  The table of nodes containing the meshes to attach.  (If the object contained is not a mesh,
	/// it will be converted to a mesh.) 
	///  \param pNode  The specific node to attach to, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \param t  The time at which to attach it.  (This is the time at which the other nodes are
	/// evaluated, which may be important if the other nodes are animated.) 
	///  \remark  Note that this method deletes the original nodes which are being attached. 
	virtual void EpModMultiAttach (Tab<INode *> & nodeTab, INode *pNode=NULL, TimeValue t=0) { }

	///  \brief 
	/// Detaches the current selection to a new Editable Poly object.
	/// 
	///  \remark  If this Edit Poly modifier is instanced across multiple nodes,
	/// a new Editable Poly object will be created for each node. 
	virtual void EpModDetachToObject (MSTR & newObjectName, TimeValue t) { }

	///  \brief 
	/// Creates a shape object based on the current set of selected edges.
	/// 
	///  \remark  Note that control over whether the resulting shape is curved or linear is not
	/// currently exposed in the SDK.  It's controlled by the static EditPolyMod variable "mCreateShapeType",
	/// which you can find in the Edit Poly source in maxsdk/samples/mesh/EditPoly.
	virtual void EpModCreateShape (MSTR & shapeObjectName, TimeValue t) { }

	///  \brief 
	/// Sets the Hinge from Edge operation to use the hinge edge given.
	/// 
	///  \param edge  The edge to use as a hinge 
	///  \param modContextTM  The mod context TM for the node containing the edge.  This is important
	/// when the Edit Poly is instanced across multiple nodes, because the coordinates of the 
	/// edge are used to hinge the faces in the other nodes as well.  We need to get these coordinates in
	/// some common reference frame, which is what the mod context tm is used for. 
	///  \param pNode  The specific node which contains the hinge, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual void EpModSetHingeEdge (int edge, Matrix3 modContextTM, INode *pNode=NULL) { }

	///  \brief 
	/// Retrieves the current Hinge Edge, or -1 if no hinge edge is set (or if it's set on a different node).
	/// Note that if the Edit Poly is instanced across multiple nodes, only one node should contain a hinge edge.
	/// 
	///  \param pNode  The specific node which we're referring to, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual int EpModGetHingeEdge (INode *pNode) { return -1; }

	///  \brief 
	/// Create a Bridge between two border loops.
	/// 
	///  \param edge1  An edge on the first border loop 
	///  \param edge2  An edge on the second border loop 
	///  \param pNode  The specific node which we're referring to, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual void EpModBridgeBorders (int edge1, int edge2, INode *pNode=NULL) { }

	///  \brief 
	/// 
	/// 
	/// \param face1 index of the first face to bridge
	/// \param face2 index of the second face to bridge
	///  \param pNode  The specific node which we're referring to, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual void EpModBridgePolygons (int face1, int face2, INode *pNode=NULL) { }

	///  \brief 
	/// 
	/// 
	virtual void EpModSetBridgeNode (INode *pNode) { }

	///  \brief 
	/// 
	/// 
	virtual INode *EpModGetBridgeNode () { return NULL; }

	///  \brief 
	/// 
	/// 
	virtual bool EpModReadyToBridgeSelected () { return false; }

	///  \brief 
	/// 
	/// 
	///  \param pNode  The specific node which we're referring to, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual void EpModTurnDiagonal (int face, int diag, INode *pNode=NULL) { }


	///  \brief 
	/// Lists the entire history of applied operations to the Maxscript listener window.  This is not quite
	/// adequate for recreating the Edit Poly in a script, because some information is omitted; but it'
	/// 
	///  \param pNode  The specific node which we're referring to, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual void EpModListOperations (INode *pNode=NULL) { }


	///  \brief 
	/// Get a pointer to the Edit Poly's cached mesh.  This is the mesh after all the committed operations,
	/// but before the current operation, if any.  The mesh returned from this method should not be modified!
	/// 
	///  \param pNode  The specific node which we want the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual MNMesh *EpModGetMesh (INode *pNode=NULL) { return NULL; }

	///  \brief 
	/// Get a pointer to the Edit Poly's cached output mesh.  This is the mesh after all committed operations
	/// as well as the current operation, if any.  The mesh returned from this method should not be modified!
	/// 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	virtual MNMesh *EpModGetOutputMesh (INode *pNode=NULL) { return NULL; }

	///  \brief 
	/// Returns the number of vertices in the mesh.
	/// 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetNumVertices (INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the position (in object space) of a particular vertex in the mesh.
	/// 
	///  \param vertIndex  The index of the vertex - between 0 and EpMeshGetNumVertices(pNode)-1. 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual Point3 EpMeshGetVertex (int vertIndex, INode *pNode=NULL) { return Point3(0,0,0); }

	///  \brief 
	/// Returns the number of faces using a particular vertex.
	/// 
	///  \param vertIndex  The index of the vertex - between 0 and EpMeshGetNumVertices(pNode)-1. 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetVertexFaceCount (int vertIndex, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns a face that uses the vertex given.
	/// 
	///  \param vertIndex  The index of the vertex - between 0 and EpMeshGetNumVertices(pNode)-1. 
	///  \param whichFace  The index of the face - between 0 and EpMeshGetVertexFaceCount(vertIndex, pNode)-1 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetVertexFace (int vertIndex, int whichFace, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the number of edges using a particualr vertex
	/// 
	///  \param vertIndex  The index of the vertex - between 0 and EpMeshGetNumVertices(pNode)-1. 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetVertexEdgeCount (int vertIndex, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns one of the edges using the vertex given.
	/// 
	///  \param vertIndex  The index of the vertex - between 0 and EpMeshGetNumVertices(pNode)-1. 
	///  \param whichEdge  The index of the edge - between 0 and EpMeshGetVertexEdgeCount(vertIndex, pNode)-1 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetVertexEdge (int vertIndex, int whichEdge, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the number of edges in the mesh.
	/// 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetNumEdges (INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the vertex at one end of the edge given.
	/// 
	///  \param edgeIndex  The index of the edge - between 0 and EpMeshGetNumEdges(pNode)-1. 
	///  \param end  The end of the edge - 0 or 1 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetEdgeVertex (int edgeIndex, int end, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the face on one side of the edge given.
	/// 
	///  \param edgeIndex  The index of the edge - between 0 and EpMeshGetNumEdges(pNode)-1. 
	///  \param side  The side of the edge.  Edges are guaranteed to have a face on side 0, but
	/// "border" edges have no face on side 1.  "No face" is indicated by a return value of -1. 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetEdgeFace (int edgeIndex, int side, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the number of faces in the mesh.
	/// 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetNumFaces(INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the degree of the indicated face.
	/// 
	///  \param faceIndex  The index of the face - between 0 and EpMeshGetNumFaces(pNode)-1. 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetFaceDegree (int faceIndex, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the vertex in a particular corner of a particular face.
	/// 
	///  \param faceIndex  The index of the face - between 0 and EpMeshGetNumFaces(pNode)-1. 
	///  \param corner  The corner of the face - between 0 and EpMeshGetFaceDegree(faceIndex,pNode)-1 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any.  The relationship between sides and corners of the polygon is:
	/// side i goes from corner i to corner (i+1)%degree. 
	virtual int EpMeshGetFaceVertex (int faceIndex, int corner, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the edge on a particular side of a particular face.
	/// 
	///  \param faceIndex  The index of the face - between 0 and EpMeshGetNumFaces(pNode)-1. 
	///  \param side  The side of the face - between 0 and EpMeshGetFaceDegree(faceIndex,pNode)-1 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any.  The relationship between sides and corners of the polygon is:
	/// side i goes from corner i to corner (i+1)%degree. 
	virtual int EpMeshGetFaceEdge (int faceIndex, int side, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the index of the corner used at an end of a face diagonal.
	/// 
	///  \param faceIndex  The index of the face - between 0 and EpMeshGetNumFaces(pNode)-1. 
	///  \param diagonal  The index of the diagonal - between 0 and EpMeshGetFaceDegree(faceIndex, pNode)-4.
	/// (There are always n-3 diagonals in a polygon of degree n.) 
	///  \param end  0 for the start of the diagonal, 1 for the end. 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \return  The index of the corner of the face.  To get the index of the vertex at that corner,
	/// call EpMeshGetFaceVertex on this result. 
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetFaceDiagonal (int faceIndex, int diagonal, int end, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the material ID used by a particular face.
	/// 
	///  \param faceIndex  The index of the face - between 0 and EpMeshGetNumFaces(pNode)-1. 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetFaceMaterial (int faceIndex, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the smoothing groups used by a particular face.
	/// 
	///  \param faceIndex  The index of the face - between 0 and EpMeshGetNumFaces(pNode)-1. 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual DWORD EpMeshGetFaceSmoothingGroup (int faceIndex, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the number of positive map channels in the mesh.  This number depends on the map channels that
	/// are active, or have been in the past.  Default is 2, typically it's in the single digits, but it can
	/// range as high as MAX_MESHMAPS.  (These channels need not all be active - this method just gives an upper
	/// bound on the index of active map channels.)
	/// 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetNumMapChannels (INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Indicates if the specified map channel is "active", that is, if it contains a set of map coordinates
	/// and map faces.
	/// 
	///  \param mapChannel  The index of the map channel, which can range from -NUM_HIDDENMAPS to
	/// EpMeshGetNumMapChannels(pNode)-1. 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual bool EpMeshGetMapChannelActive (int mapChannel, INode *pNode=NULL) { return false; }

	///  \brief 
	/// Returns the number of map vertices in the specified map.  This is typically larger than the number
	/// of regular vertices in the mesh, though it can be the same or lower.  Returns 0 for inactive maps.
	/// 
	///  \param mapChannel  The index of the map channel, which can range from -NUM_HIDDENMAPS to
	/// EpMeshGetNumMapChannels(pNode)-1. 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetNumMapVertices (int mapChannel, INode *pNode=NULL) { return 0; }

	///  \brief 
	/// Returns the UVVert map vertex position for a given map channel and map vertex index.
	/// 
	///  \param mapChannel  The index of the map channel, which can range from -NUM_HIDDENMAPS to
	/// EpMeshGetNumMapChannels(pNode)-1. 
	///  \param vertIndex  The index of the map vertex, in the range of 0 to EpMeshGetNumMapVertices(mapChannel,pNode)-1 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual UVVert EpMeshGetMapVertex (int mapChannel, int vertIndex, INode *pNode=NULL) { return UVVert(0,0,0); }

	///  \brief 
	/// Returns the index of the map vertex used in a particular corner of a particular map face (in a particular map).
	/// 
	///  \param mapChannel  The index of the map channel, which can range from -NUM_HIDDENMAPS to
	/// EpMeshGetNumMapChannels(pNode)-1. 
	///  \param faceIndex  The index of the face - between 0 and EpMeshGetNumFaces(pNode)-1.
	/// (All map channels have the same number of faces as the mesh.) 
	///  \param corner  The corner of the face - between 0 and EpMeshGetFaceDegree(faceIndex,pNode)-1.
	/// (Each map face has the same degree as the corresponding mesh face.) 
	///  \param pNode  The specific node to get the mesh from, in cases where the Edit Poly is instanced
	/// across multiple nodes.  If left at NULL, the Primary node is used.  (See EpModSetPrimaryNode.)
	/// If no primary node has yet been established, one is picked arbitrarily.
	/// If the Edit Poly is only used by one node, you may safely leave this at NULL.  
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int EpMeshGetMapFaceVertex (int mapChannel, int faceIndex, int corner, INode *pNode=NULL) { return 0; }

	//----------------------------------------------------------------------
	//Bridge related methods - removed from EPolyMod8 and joined to EPolyMod
	//----------------------------------------------------------------------

	//! \brief Bridges individual edges, will use current parameters 
	/// \param[in] in_edge1 The source of the bridge operation
	/// \param[in] in_edge2 The target of the bridge operation
	/// \param[in] in_pNode The node to which the bridge is applied

	virtual void EpModBridgeEdges (const int in_edge1,	const int in_edge2,	INode *in_pNode=NULL) { }

	//! \brief Changea the edge selection in the ring  direction
	// \param[in] in_val: Specifies which way the current edge selection is changed. it can take the values : 	
	// \li RING_UP: Moves the selection one edge up in the ring direction
	// \li LOOP_UP: Moves the selection one edge up in the loop direction
	// \li RING_DOWN: Moves the selection one edge down in the ring direction
	// \li LOOP_DOWN: Moves the selection one edge up in the loop direction
	// \li RING_UP_ADD: Adds to the selection one edge in the up ring direction 
	// \li LOOP_UP_ADD: Adds to the selection one edge in the up loop direction 
	// \li RING_DOWN_ADD: Adds to the selection one edge in the down ring direction 
	// \li LOOP_DOWN_ADD: Adds to the selection one edge in the down loop direction 
	// \li RING_UP_SUBTRACT: Subtracts to the selection one edge in the up ring direction 
	// \li LOOP_UP_SUBTRACT: Subtracts to the selection one edge in the up loop direction 
	// \li RING_DOWN_SUBTRACT: Subtracts to the selection one edge in the down ring direction 
	// \li LOOP_DOWN_SUBTRACT: Subtracts to the selection one edge in the down loop direction 
	// \param[in] in_pNode: The node to which the bridge is applied

	virtual void	EpModUpdateRingEdgeSelection ( int in_val,	INode *in_pNode=NULL)	{ }

	//! \brief Changes the edge selection in the loop  direction
	/// \param[in] in_val: Specifies which way the current edge selection is changed. it can take the values : 	
	/// \li RING_UP: Moves the selection one edge up in the ring direction
	/// \li LOOP_UP: Moves the selection one edge up in the loop direction
	/// \li RING_DOWN: Moves the selection one edge down in the ring direction
	/// \li LOOP_DOWN: Moves the selection one edge up in the loop direction
	/// \li RING_UP_ADD: Adds to the selection one edge in the up ring direction 
	/// \li LOOP_UP_ADD: Adds to the selection one edge in the up loop direction 
	/// \li RING_DOWN_ADD: Adds to the selection one edge in the down ring direction 
	/// \li LOOP_DOWN_ADD: Adds to the selection one edge in the down loop direction 
	/// \li RING_UP_SUBTRACT: Subtracts to the selection one edge in the up ring direction 
	/// \li LOOP_UP_SUBTRACT: Subtracts to the selection one edge in the up loop direction 
	/// \li RING_DOWN_SUBTRACT: Subtracts to the selection one edge in the down ring direction 
	/// \li LOOP_DOWN_SUBTRACT: Subtracts to the selection one edge in the down loop direction 
	/// \param[in] in_pNode: The node to which the bridge is applied

	virtual void	EpModUpdateLoopEdgeSelection ( int in_val, INode *in_pNode=NULL) { }

	//! \brief Changes the edge selection , in the ring direction
	/*! This method adds ( move or remove ) edges from the current selection.
	//  \n n the 'positive' ( or negative ) ring direction,using the 3 existing parameters. The ring direction is vertical for a plane. 
	//  \n If in_newPos == -3 , in_moveOnly == false and in_add == true, it adds 3 edges from the current selection,
	//  \n in the negative ring direction. this is equivalent to the MaxScript: $.setRingShift -3 false true
	//  \param[in] in_newPos: If > 0 will change the edge selection the 'positive' ring direction, if < 0 in the 'negative' direction.
	//  \param[in] in_moveOnly: If true, edges are moved, if false, edges are added or removed from selection
	//  \param[in] in_add: If true edges are added, if false, edges are removed from current selection
	*/
	virtual void	EpModSetRingShift( int in_newPos, bool in_moveOnly, bool in_add) {}

	//! \brief Changes the edge selection , in the loop direction
	/*! This method adds ( move or remove ) edges from the current selection.
	//  \n n the 'positive' ( or negative ) loop direction,using the 3 existing parameters. The loop direction is horizontal for a plane. 
	//  \n If in_newPos == -3 , in_moveOnly == false and in_add == true, it adds 3 edges from the current selection,
	//  \n in the negative ring direction. this is equivalent to the MaxScript: $.setLoopShift -3 false true
	//  \param[in] in_newPos: If > 0 will change the edge selection the 'positive' ring direction, if < 0 in the 'negative' direction.
	//  \param[in] in_moveOnly: If true, edges are moved, if false, edges are added or removed from selection
	//  \param[in] in_add: If true edges are added, if false, edges are removed from current selection
	*/

	virtual void	EpModSetLoopShift( int in_newPos, bool in_moveOnly, bool in_add) {}

	//! \brief Converts current sub-object selection to the specified sub-object border.  
	/*! It converts for instance selected faces into their border edges and selects it, 
	//  \n if you call EpModConvertSelectionToBorder( EPM_SL_FACE, EPM_SL_EDGE);
	// \param[in] in_epSelLevelFrom: Can be EPM_SL_EDGE, EPM_SL_FACE or EPM_SL_VERTEX
	// \param[in] in_epSelLevelTo: Can be EPM_SL_EDGE, EPM_SL_FACE or EPM_SL_VERTEX
	*/

	virtual int		EpModConvertSelectionToBorder (int in_epSelLevelFrom, int in_epSelLevelTo) { return 0; }

	//! \brief Commits any Paint Deformation as if pressing the Commit button
	/*! Has no effect when the Commit button is disabled */

	virtual void EpModPaintDeformCommit() {}

	//! \brief Reverts any Paint Deformation as if pressing the Revert button
	/*! Has no effect when the Revert button is disabled */

	virtual void EpModPaintDeformCancel() {}

	//----------------------------------------------------------------------
	//Methods that previously only existed in editable poly - added in Max 9
	//----------------------------------------------------------------------

	///  \brief 
	/// Returns the normal of the face specified
///
	///  \param[in] in_faceIndex  The index of the face - between 0 and EpMeshGetNumFaces(pNode)-1.
	///  \param[in] in_pNode  Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual Point3		EPMeshGetFaceNormal( const int in_faceIndex, INode *in_pNode = NULL ) { return Point3(0,0,0); }
	///  \brief 
	/// Returns the center of the face specified
	/// 
	///  \param[in] in_faceIndex  The index of the face - between 0 and EpMeshGetNumFaces(pNode)-1.
	///  \param[in] in_pNode  Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual Point3		EPMeshGetFaceCenter( const int in_faceIndex, INode *in_pNode = NULL ) { return Point3(0,0,0); }
	///  \brief 
	/// Returns the area of the face specified
	/// 
	///  \param[in] in_faceIndex  The index of the face - between 0 and EpMeshGetNumFaces(pNode)-1.
	///  \param[in] in_pNode  Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual float		EPMeshGetFaceArea( const int in_faceIndex, INode *in_pNode = NULL ) { return 0.0f; }
	///  \brief 
	/// Returns a list of edges that are adjacent to at least one empty face
	/// 
	///  \param[in] in_pNode  Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual BitArray *	EPMeshGetOpenEdges( INode *in_pNode = NULL ) { return NULL; }

	//! \name GetByFlag
	//! \brief These methods retrieve the parts of the object that match the specified flags and mask passed in
	//@{
	///  \brief 
	/// Retrieves the vertices specified the flag and mask, and updates the bitarray passed in
	/// 
	///  \param[out] out_vset  The BitArray passed by reference - the bits will be updated to show the specified vertices
	///  \param[in] in_flags  The flags that are to be checked for
	///  \param[in] in_fmask  The results will use the bitwise-AND of fmask and flags if it is specified
	///  \param in_pNode  Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual bool		EPMeshGetVertsByFlag( BitArray & out_vset, const DWORD in_flags, const DWORD in_fmask=0x0, INode *in_pNode = NULL ) { return false; }
	///  \brief 
	/// Retrieves the edges specified the flag and mask, and updates the bitarray passed in
/// 
	///  \param[out] out_eset  The BitArray passed by reference - the bits will be updated to show the specified edges
	///  \param[in] in_flags  The flags that are to be checked for
	///  \param[in] in_fmask  The results will use the bitwise-AND of fmask and flags if it is specified
	///  \param[in] in_pNode  Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual bool		EPMeshGetEdgesByFlag( BitArray & out_eset, const DWORD in_flags, const DWORD in_fmask=0x0, INode *in_pNode = NULL ) { return false; }
	///  \brief 
	/// Retrieves the faces specified the flag and mask, and updates the bitarray passed in
	/// 
	///  \param[out] out_fset  The BitArray passed by reference - the bits will be updated to show the specified faces
	///  \param[in] in_flags  The flags that are to be checked for
	///  \param[in] in_fmask  The results will use the bitwise-AND of fmask and flags if it is specified
	///  \param[in] in_pNode  Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual bool		EPMeshGetFacesByFlag( BitArray & out_fset, const DWORD in_flags, const DWORD in_fmask=0x0, INode *in_pNode = NULL ) { return false; }
	//@}

	//! \name SetFlags
	//! \brief These methods set the flags on the specified subobjects to match the flags and mask passed in
	//@{
	///  \brief 
	/// Sets the specified vertices flags to the specified value
	/// 
	///  \param[in] in_vset  The list of vertices to have their flag changed
	///  \param[in] in_flags  The flags that are to be set
	///  \param[in] in_fmask  The results will use the bitwise-AND of fmask and flags if it is specified
	///  \param[in] in_undoable  Can the results of this operation be undone through the normal method
	///  \param[in] in_pNode  Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	virtual void		EPMeshSetVertexFlags( BitArray & in_vset, const DWORD in_flags, DWORD in_fmask=0x0, const bool in_undoable = true, INode *in_pNode = NULL ) { return; }
	///  \brief 
	/// Sets the specified vertices flags to the specified value
	/// 
	///  \param[in] in_eset  The list of edges to have their flag changed
	///  \param[in] in_flags  The flags that are to be set
	///  \param[in] in_fmask  The results will use the bitwise-AND of fmask and flags if it is specified
	///  \param[in] in_undoable  Can the results of this operation be undone through the normal method
	///  \param[in] in_pNode  Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	virtual void		EPMeshSetEdgeFlags( BitArray & in_eset, const DWORD in_flags, DWORD in_fmask=0x0, const bool in_undoable = true, INode *in_pNode = NULL ) { return; }
	///  \brief 
	/// Sets the specified faces flags to the specified value
	/// 
	///  \param[in] in_fset  The list of faces to have their flag changed
	///  \param[in] in_flags  The flags that are to be set
	///  \param[in] in_fmask  The results will use the bitwise-AND of fmask and flags if it is specified
	///  \param[in] in_undoable  Can the results of this operation be undone through the normal method
	///  \param[in] in_pNode  Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	virtual void		EPMeshSetFaceFlags( BitArray & in_fset, const DWORD in_flags, DWORD in_fmask=0x0, const bool in_undoable = true, INode *in_pNode = NULL ) { return; }
	//@}

	//! \name GetFlags
	//! \brief These methods get the flags on the specified subobjects
	//@{
	///  \brief 
	/// Retrieves the flags from the specified vertex
		/// 
	///  \param[in] in_vertexIndex  The vertex to retrieve the flags from - between 0 and EpMeshGetNumVertices - 1
	///  \param[in] in_pNode  Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int			EPMeshGetVertexFlags( const int in_vertexIndex, INode *in_pNode = NULL ) { return 0; }
	///  \brief 
	/// Retrieves the flags from the specified edge
		/// 
	///  \param[in] in_edgeIndex  The edge to retrieve the flags from - between 0 and EpMeshGetNumEdges - 1
	///  \param[in] in_pNode  Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int			EPMeshGetEdgeFlags( const int in_edgeIndex, INode *in_pNode = NULL ) { return 0; }
	///  \brief 
	/// Retrieves the flags from the specified face
		///
	///  \param[in] in_faceIndex  The face to retrieve the flags from - between 0 and EpMeshGetNumFaces - 1
	///  \param[in] in_pNode  Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual int			EPMeshGetFaceFlags( const int in_faceIndex, INode *in_pNode = NULL ) { return 0; }
	//@}
		
	//! \name GetUsing
	//! \brief These methods retrieve all subobjects of the specified type that are connected to the second list of subobjects passed in \
	//@{
	///  \brief 
	/// Retrieves list of verts that are used by the edges specified
	/// 
	///  \param[out] out_vset - List of vertices that will be updated with the vertices used by the edges
	///  \param[in] in_eset - List of edges used for finding the vertices
	///  \param[in] in_pNode - Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual void		EPMeshGetVertsUsingEdge( BitArray & out_vset, const BitArray & in_eset, INode *in_pNode = NULL ) { return; }
	///  \brief 
	/// Retrieves list of edges that are used by the vertices specified
	/// 
	///  \param[out] out_eset - List of edges that will be updated with the edges used by the vertices
	///  \param[in] in_vset - List of vertices used for finding the edges
	///  \param[in] in_pNode - Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual void		EPMeshGetEdgesUsingVert( BitArray & out_eset, BitArray & in_vset, INode *in_pNode = NULL ) { return; }
	///  \brief 
	/// Retrieves list of faces that are used by the edges specified
	/// 
	///  \param[out] out_fset - List of faces that will be updated with the faces used by the edges
	///  \param[in] in_eset - List of edges used for finding the faces
	///  \param[in] in_pNode - Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual void		EPMeshGetFacesUsingEdge( BitArray & out_fset, BitArray & in_eset, INode *in_pNode = NULL ) { return; }
	///  \brief 
	/// Retrieves list of elements that are used by the faces specified
	/// 
	///  \param[out] out_eset - List of elements that will be updated with the elements used by the faces
	///  \param[in] in_fset - List of faces used for finding the elements
	///  \param[in] in_fenceSet - List for calculating elements from the face
	///  \param[in] in_pNode - Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual void		EPMeshGetElementsUsingFace( BitArray & out_eset, BitArray & in_fset, BitArray & in_fenceSet, INode *in_pNode = NULL ) { return; }
	///  \brief 
	/// Retrieves list of faces that are used by the vertices specified
	/// 
	///  \param[out] out_fset - List of faces that will be updated with the faces used by the vertices
	///  \param[in] in_vset - List of vertices used for finding the faces
	///  \param[in] in_pNode - Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual void		EPMeshGetFacesUsingVert( BitArray & out_fset, BitArray & in_vset, INode *in_pNode = NULL ) { return; }
	///  \brief 
	/// Retrieves list of verts that are used by the faces specified
	/// 
	///  \param[out] out_vset - List of vertices that will be updated with the vertices used by the faces
	///  \param[in] in_fset - List of faces used for finding the vertices
	///  \param[in] in_pNode - Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual void		EPMeshGetVertsUsingFace( BitArray & out_vset, BitArray & in_fset, INode *in_pNode = NULL ) { return; }
	//@}

	///  \brief 
	/// Moves the specified vertices to the specified point.  Requires a commit for other functions to return the proper data
	/// 
	///  \param[in] in_vset - List of vertices that will be moved
	///  \param[in] in_point - Point in the current coordinate system that the vertices will be moved to
	///  \param[in] in_pNode - Optional node to run the check on.  If the node is not specified, we will use the currently selected noe
	///  \remark  This is based on the mesh after all committed operations, but before the current
	/// operation, if any. 
	virtual void		EPMeshSetVert( const BitArray & in_vset, const Point3 & in_point, INode *in_pNode = NULL ) { return; }

	//!  \brief Brings up the Smooting Group floater dialog.  If it is already up it closes it	
	virtual	void		SmGrpFloater() = 0;
	//!  \brief Brings up the Material ID floater dialog.  If it is already up it closes it	 
	virtual	void		MatIDFloater() = 0;
	//!  \brief Returns whether the Material ID floater dialog is up	
	virtual	BOOL		MatIDFloaterVisible() = 0;
	//!  \brief Returns whether the  Smooting Group floater dialog is up
	virtual	BOOL		SmGrpFloaterVisible() = 0;

	//!  \brief  Closes the Smoothing Group Floater Dialog
	virtual void CloseSmGrpFloater() {}
	//  \brief Closes the Material ID Floater Dialog
	virtual void CloseMatIDFloater() {}

	//!  \brief Returns the Material ID Floater Dialog Handle
	virtual HWND MatIDFloaterHWND() { return NULL; }

	//!  \brief Returns the SmGrp Floater Dialog Handle
	virtual HWND SmGrpFloaterHWND() { return NULL; }
};

// Controls whether or not "inherently" nonanimatable operations are available in Animation mode.
#define EDIT_POLY_DISABLE_IN_ANIMATE

class EPolyManipulatorGrip_Imp;
//! \brief This new class extends EPolyMode, in particular adds a more efficient way for setting several vertex positions, and
//! the ability for EPolyMod to support manipulate grips.
class EPolyMod13 : public EPolyMod
{

	friend class EPolyManipulatorGrip_Imp;

public:
	//! \brief Prepares the Edit Poly modifier for setting several vertex positions in a row.
	//  \param[in] pNode - The node that is used by the modifier.
	virtual void EPMeshStartSetVertices(INode* pNode) { return; }

	//! \brief Sets the posititon of a single vertex. Should only be used inbetween
	//  calls to EPMeshStartSetVertices and EPMeshEndSetVertices.
	//  \param[in] vert - The vert to set.
	//  \param[in] in_point - The position to set, in the objects local coordinate space.
	//  \param[in] pNode - The node that is used by the modifier.
	virtual void EPMeshSetVert( const int vert, const Point3 & in_point, INode* pNode) { return; }

	//! \brief Finishes setting vertex positions and applies undo functionality.
	//  \param[in] pNode - The node that is used by the modifier.
	virtual void EPMeshEndSetVertices(INode* pNode) { return; }


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
		//! Grow Shift Selection Falloff grip.  Only in edge subobject mode.
		eLoopShift,
		//! Right Shift Selection Falloff grip.  Only in edge subobject mode.
		eRingShift

	};
	//@}

	//! \brief Turns the manipulate grip item on or off. When at least one grip item is on and 
	//! select and manpulate mode is active a grip will show up for each item that's on.
	//! \param[in] on - Whether or not the specified item will be turned on or off
	//! \param[in] item - Which grip item that will be turned on or off.
	virtual void SetManipulateGrip(bool on, ManipulateGrips item) = 0;

	//! \brief Get whether or not this manipulator item is on or not. 
	//! \param[in] item - Which grip item we are checking to see if it's on or off.
	//! \return Whether or not the item is on or not.  Note that it doesn't
	//! take into account whether or not the correct subobject mode is also active, it just 
	//! checks to see if it would be on if in the right mode.
	virtual bool GetManipulateGrip(ManipulateGrips item) = 0;
};
#pragma warning(pop)