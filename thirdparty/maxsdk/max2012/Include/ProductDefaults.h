/*****************************************************************************
* FILE:					ProductDefaults.h
* DESCRIPTION:	File defining per product built in defaults 
* CREATED BY:		Attila Szabo
* HISTORY:	
* - 2003.Oct.14 - created 
* - 2003.Oct.21 - AS - Added MAX_RELEASE_EXTERNAL 
******************************************************************************
* Copyright (c) 2003 Autodesk, All Rights Reserved.
******************************************************************************/

#pragma once

#include "buildver.h"

#ifdef METRIC_UNITS_FIXED_VALUES
  #define DFLT_XFORM_MOVE_SPIN_INCREMENT        0.01f
  #define DFLT_EDIT_MESH_NORMAL_SCALE           0.5f
  #define DFLT_EDIT_MESH_MOD_NORMAL_SCALE       0.5f
  #define DFLT_EDIT_MESH_WELD_TRESHOLD          0.005f
  #define DFLT_EDIT_MESH_MOD_WELD_TRESHOLD      0.005f
  #define DFLT_MAPSCALER_OSM_MOD_DFLT_SCALE     1.0f
  #define DFLT_MAPSCALER_WSM_MOD_DFLT_SCALE     1.0f
  #define DFLT_EDIT_SPLINE_WELD_TRESHOLD        0.005f
  #define DFLT_EDIT_SPLINE_MOD_WELD_TRESHOLD    0.005f
  #define DFLT_EDIT_SPLINE_THICKNESS            0.05f
#endif // METRIC_UNITS_FIXED_VALUES

