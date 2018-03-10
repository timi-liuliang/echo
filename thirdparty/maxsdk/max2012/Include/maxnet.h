//-----------------------------------------------------------------------------
// -------------------
// File ....: maxnet.h
// -------------------
// Author...: Gus J Grubba
// Date ....: February 2000
//
// Descr....: 3D Studio MAX Network Interface
//
// History .: Feb, 07 2000 - Started
//            
//-----------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------
//-- Network Status
//
#include "network\NetworkStatus.h"

//---------------------------------------------------------
//-- Station Configuration Block
//
#include "network\ConfigurationBlock.h"

//---------------------------------------------------------
//-- Manager Info
//
#include "network\ManagerInfo.h"

//---------------------------------------------------------
//-- Server Info
//
#include "network\ServerInfo.h"

//---------------------------------------------------------
//-- Client Info
//
#include "network\ClientInfo.h"

//-------------------------------------------------------------------
//-- Global Server State
//
#include "network\ServerList.h"

//---------------------------------------------------------
//-- Server Statistics
//
#include "network\Statistics.h"

//-------------------------------------------------------------------
//-- Servers in Job Queue 
//
#include "network\JobServer.h"

//-----------------------------------------------------------------------------
//-- Job Frame State
#include "network\JobFrames.h"

//-----------------------------------------------------------------------------
//-- MaxNet Class (Exception Handler)
//
#include "network\MaxNet.h"

#include "maxnet_platform.h"
#include "maxnet_job.h"
#include "maxnet_file.h"
#include "maxnet_archive.h"
#include "maxnet_manager.h"

//-----------------------------------------------------------------------------
//-- Interface
#include "network\Util.h"