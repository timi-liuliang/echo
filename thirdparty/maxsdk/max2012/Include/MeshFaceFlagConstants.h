//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//

#pragma once

/** \defgroup Edge_s_visibility_constants Edge's Visibility Constants
 * \see Face::setEdgeVis() */
//@{
#define EDGE_VIS			1	//!< Edge is visible
#define EDGE_INVIS			0	//!< Edge is invisible
//@}

/// \defgroup first_edge_visibility_bit_field First Edge-Visibility Bit Field
//@{
#define VIS_BIT			0x0001
#define VIS_MASK			0x0007
//@}

/** \defgroup Edge_visibility_flags Edge Visibility Flags
 * If the bit is 1, the edge is visible. */
//@{
#define EDGE_A		(1<<0)
#define EDGE_B		(1<<1)
#define EDGE_C		(1<<2)
#define EDGE_ALL	(EDGE_A|EDGE_B|EDGE_C)
//@}

#define FACE_HIDDEN	(1<<3)	//!< Face visibility bit. If the bit is 1, the face is hidden.
#define HAS_TVERTS	(1<<4)	//!< \deprecated Replaced by mapSupport(). Texture vertices bit: If the bit is 1, texture vertices are present. 
#define FACE_WORK	(1<<5)	
#define FACE_STRIP	(1<<6)

/** \defgroup Normal_Face_Usage_Flags Normal Face Usage Flags
 * Flags to indicate that face normal is used because no smoothing group normal is found. */
//@{
#define FACE_NORM_A	(1<<8)
#define FACE_NORM_B	(1<<9)
#define FACE_NORM_C	(1<<10)
#define FACE_NORM_MASK	0x0700
//@}

#define FACE_INFOREGROUND	(1<<11)  //!< Used to track faces that are in the foreground.
#define FACE_BACKFACING		(1<<12)  //!<Used to track which faces are back facing. A temporary flag.


/** \defgroup Material_ID_Masks Material ID Masks
 * \see Face::getMatID(),  */
//@{
#define FACE_MATID_SHIFT	16		//!< The number of bits to shift the flags to access the material.
#define FACE_MATID_MASK		0xFFFF	//!< A mask used to access the material ID.
//@}


/** \defgroup Render_Options_Flags Render Options Flags
* \see Mesh::render()*/
//@{
#define COMP_TRANSFORM	0x0001	//!< If set, it forces recalculate of model->screen transform. If reset, will attempt to use cache
#define COMP_IGN_RECT	0x0002	//!< If set, it forces all polys to be rendered. If reset, only those intersecting the box will be rendered.
#define COMP_LIGHTING	0x0004	//!< If set, it forces re-lighting of all verts (as when a light moves). If reset, only the moved verts will be re-lighted.
#define COMP_ALL		0x00ff  //!< Combination of COMP_TRANSFORM, COMP_IGN_RECT and COMP_LIGHTING
/** If set, the node being displayed by this mesh is selected.
 * Certain display flags only activate when this bit is set.*/
#define COMP_OBJSELECTED	(1<<8)
/** If set, the node being displayed by this mesh is frozen. */
#define COMP_OBJFROZEN		(1<<9)
//@}

/** \defgroup RVertex_flags RVertex Flags
 * RVertex flags: contain clip flags, number of normals at the vertex
 * and the number of normals that have already been rendered.*/
//@{
#define NORCT_MASK			0x000000ffUL
#define SPECIFIED_NORMAL	0x00004000UL
#define OUT_LEFT			0x00010000UL
#define OUT_RIGHT			0x00020000UL
#define OUT_TOP				0x00040000UL
#define OUT_BOTTOM			0x00080000UL
#define RECT_MASK			0x000f0000UL
#define RND_MASK			0xfff00000UL
#define RND_NOR0			0x00100000UL
#define RND_NOR(n)  		(RND_NOR0 << (n))
//@}