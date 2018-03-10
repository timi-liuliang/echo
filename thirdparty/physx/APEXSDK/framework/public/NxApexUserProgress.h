/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_APEX_USER_PROGRESS_H
#define NX_APEX_USER_PROGRESS_H

/*!
\file
\brief class IProgressListener
*/

#include "NxApexUsingNamespace.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

//! \brief User-provided class for following the progress of a task
class IProgressListener
{
public:
	//! \brief APEX calls this method to update the progress of a task
	virtual void	setProgress(int progress, const char* taskName = NULL) = 0;
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_APEX_USER_PROGRESS_H
