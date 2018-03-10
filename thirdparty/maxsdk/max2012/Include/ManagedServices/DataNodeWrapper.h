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
// AUTHOR: Nicolas Desjardins
// DATE: 2007-01-08 
//***************************************************************************/

#pragma once
#pragma managed(push, on)

namespace ManagedServices
{

// Note that this documentation does not use "///" for extraction by the
// compiler.  It will fail to be processed correctly.  Here's Microsoft's
// explanation:

// In the current release, code comments are not processed on templates or 
// anything containing a template type (for example, a function taking a 
// parameter as a template). Doing so will result in undefined behavior.
// [Copied from web MSDN on 2007-08-31.]

// <summary>
// Interface formalizing accessors for classes wrapping data nodes.
// </summary>
// <remarks>
// Note that for documentation to work correctly, template classes must be 
// kept internal to their assemblies.
// </remarks>
template<typename HeldType_>
private interface class DataNodeWrapper
{
public:
	// <summary>
	// Type of the wrapped data item.
	// </summary>
	typedef HeldType_ HeldType;

	// <summary>
	// Get the data item held in this wrapper.
	// </summary>
	// <returns>
	// The data item held in this wrapper.
	// </returns>
	virtual HeldType GetDataNode();

	// <summary>
	// Get the Type of the data item held in this wrapper.
	// </summary>
	// <returns>
	// The Type of the data item held in this wrapper.
	// </returns>
	virtual System::Type^ GetDataNodeType();
};

}

#pragma managed(pop)
