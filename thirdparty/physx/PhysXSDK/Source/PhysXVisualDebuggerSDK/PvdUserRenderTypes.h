/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
#define THERE_IS_NO_INCLUDE_GUARD_HERE_FOR_A_REASON

#ifndef DECLARE_PVD_IMMEDIATE_RENDER_TYPE_NO_COMMA 
#define DECLARE_PVD_IMMEDIATE_RENDER_TYPE_NO_COMMA  DECLARE_PVD_IMMEDIATE_RENDER_TYPE
#endif

DECLARE_PVD_IMMEDIATE_RENDER_TYPE( SetInstanceId )
DECLARE_PVD_IMMEDIATE_RENDER_TYPE( Points )
DECLARE_PVD_IMMEDIATE_RENDER_TYPE( Lines )
DECLARE_PVD_IMMEDIATE_RENDER_TYPE( Triangles )
DECLARE_PVD_IMMEDIATE_RENDER_TYPE( JointFrames )
DECLARE_PVD_IMMEDIATE_RENDER_TYPE( LinearLimit )
DECLARE_PVD_IMMEDIATE_RENDER_TYPE( AngularLimit )
DECLARE_PVD_IMMEDIATE_RENDER_TYPE( LimitCone )
DECLARE_PVD_IMMEDIATE_RENDER_TYPE( DoubleCone ) 
DECLARE_PVD_IMMEDIATE_RENDER_TYPE_NO_COMMA( Text )


#undef DECLARE_PVD_IMMEDIATE_RENDER_TYPE_NO_COMMA 
#undef THERE_IS_NO_INCLUDE_GUARD_HERE_FOR_A_REASON
