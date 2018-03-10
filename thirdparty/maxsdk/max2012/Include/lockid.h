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

#pragma once
#include "coreexp.h"
#include "BuildWarnings.h"

/*! \remarks Returns TRUE if the application is running under a trial license, as
opposed to a full, authorized license; otherwise FALSE. */
extern CoreExport bool IsTrialLicense();
//! Returns true if the application is running under a network license.
extern CoreExport bool IsNetworkLicense();


