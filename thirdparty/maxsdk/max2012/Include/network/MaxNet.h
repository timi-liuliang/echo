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
#include "MaxNetExport.h"
#include "..\maxnet_types.h"
#include "..\maxheap.h"
#include "..\strbasic.h"

/*! \sa  Class MaxNetManager, <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_maxnet_errors.html">List of MaxNet Errors</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The MaxNet class serves as the base class for MaxNetManager and should be
treated as the exception handler when using the Network Rendering API.  */
class MAXNETEXPORT MaxNet : public MaxHeapOperators
{
	protected:
		int	gerror;
		maxnet_error_t	error;
		maxnet_error_t	TranslateError	(int err);
	public:
						/*! \remarks Constructor */
						MaxNet			();
		/*! \remarks This method returns the MaxNet error. See the list of
		MaxNet error codes for details. */
		maxnet_error_t	GetError		();
		/*! \remarks This method returns the MaxNet error. See the list of
		MaxNet error codes for details. */
		int				GetGError		();
		/*! \remarks This method returns the MaxNet error description string.
		*/
		const MCHAR*	GetErrorText	();
};
