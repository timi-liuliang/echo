//**************************************************************************/
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
#pragma once

#include <WTypes.h>

/*!	The permanent ID of a Parameter.
	see class ParamDef */
typedef short ParamID;
/*!	The permanent ID of the parameter block.
	see class ParamBlockDesc2 */
typedef short BlockID;
/*!	Parameter Map ID.
	see class class ParamBlockDesc2 and class IParamMap2 */
typedef short MapID;
/*!	Resource String ID, that points to the fixed internal name for a function published definition.
	see class ParamBlockDesc2, FPFunctionDef, FPActionDef, FPPropDef, FPParamDef, etc.. */
typedef INT_PTR StringResID;
/*!	Resource ID.
	see class ParamBlockDesc2 */
typedef int ResID;
