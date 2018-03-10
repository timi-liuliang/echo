/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#define THERE_IS_NO_INCLUDE_GUARD_FOR_A_REASON

#ifndef DECLARE_PVD_COMM_STREAM_EVENT_NO_COMMA 
#define DECLARE_PVD_COMM_STREAM_EVENT_NO_COMMA DECLARE_PVD_COMM_STREAM_EVENT
#endif


//meta data
DECLARE_PVD_COMM_STREAM_EVENT( StringHandleEvent )
DECLARE_PVD_COMM_STREAM_EVENT( CreateClass )
DECLARE_PVD_COMM_STREAM_EVENT( DeriveClass )
DECLARE_PVD_COMM_STREAM_EVENT( CreateProperty )
DECLARE_PVD_COMM_STREAM_EVENT( CreatePropertyMessage )


//instance editing
DECLARE_PVD_COMM_STREAM_EVENT( CreateInstance )
DECLARE_PVD_COMM_STREAM_EVENT( SetPropertyValue )

DECLARE_PVD_COMM_STREAM_EVENT( BeginSetPropertyValue )
DECLARE_PVD_COMM_STREAM_EVENT( AppendPropertyValueData )
DECLARE_PVD_COMM_STREAM_EVENT( EndSetPropertyValue )

DECLARE_PVD_COMM_STREAM_EVENT( SetPropertyMessage )

DECLARE_PVD_COMM_STREAM_EVENT( BeginPropertyMessageGroup )
DECLARE_PVD_COMM_STREAM_EVENT( SendPropertyMessageFromGroup )
DECLARE_PVD_COMM_STREAM_EVENT( EndPropertyMessageGroup )

DECLARE_PVD_COMM_STREAM_EVENT( DestroyInstance )

DECLARE_PVD_COMM_STREAM_EVENT( PushBackObjectRef )
DECLARE_PVD_COMM_STREAM_EVENT( RemoveObjectRef )

//section system
DECLARE_PVD_COMM_STREAM_EVENT( BeginSection )
DECLARE_PVD_COMM_STREAM_EVENT( EndSection )

//Instance meta data
DECLARE_PVD_COMM_STREAM_EVENT( SetPickable )
DECLARE_PVD_COMM_STREAM_EVENT( SetColor )
DECLARE_PVD_COMM_STREAM_EVENT( SetIsTopLevel )

//Cameras
DECLARE_PVD_COMM_STREAM_EVENT( SetCamera )

DECLARE_PVD_COMM_STREAM_EVENT( AddProfileZone )
DECLARE_PVD_COMM_STREAM_EVENT( AddProfileZoneEvent )

//System
DECLARE_PVD_COMM_STREAM_EVENT( StreamEndEvent )

//ErrorStream
DECLARE_PVD_COMM_STREAM_EVENT( ErrorMessage )
//Origin Shifting
DECLARE_PVD_COMM_STREAM_EVENT_NO_COMMA( OriginShift )

#undef DECLARE_PVD_COMM_STREAM_EVENT_NO_COMMA
#undef THERE_IS_NO_INCLUDE_GUARD_FOR_A_REASON
