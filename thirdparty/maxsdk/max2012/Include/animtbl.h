//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// FILE:        animtbl.h
// DESCRIPTION: Defines Animatable Classes
// AUTHOR:      Rolf Berteig & Dan Silva
// HISTORY:     created 9 September 1994
//**************************************************************************/
#pragma once

#include "WindowsDefines.h"
#include "GetCOREInterface.h"


#include "AnimatableFlags.h"

#include "TimeMap.h"

#include "TrackHitRecord.h"

#include "TrackFlags.h"

#include "TrackScreenUtils.h"

#include "TrackClipObject.h"

#include "ParamDimension.h"

#include "AnimatableInterfaceIDs.h"

#include "AnimProperty.h"

#include "AnimPropertyList.h"

// Property IDs
#include "AnimPropertyID.h"

// BeginEditParams flags values
#include "EditParamFlags.h"

#include "AssetEnumCallback.h"

class NoteTrack;

#include "SysNodeContext.h"

#include "Animatable.h"

//
// Callback for EnumAnimTree:
//
#include "AnimEnum.h"

#include "SubObjAxisCallback.h"

// --- AppData ---------------------------------------------

#include "AppDataChunk.h"

// This list is maintained by the systems. Plug-ins need not concern themselves with it.
#include "AnimAppData.h"

#include "SchematicViewProperty.h"
 
#include "CoreFunctions.h"
