/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef PX_FOUNDATION_PSPRINTSTRING_H
#define PX_FOUNDATION_PSPRINTSTRING_H

#include "Ps.h"

namespace physx
{
namespace shdfnd
{
	/**
	\brief The maximum supported formatted output string length 
	(number of characters after replacement).

	@see printFormatted()
	*/
	static const size_t PS_MAX_PRINTFORMATTED_LENGTH = 1024;
	
	/** 
	\brief Prints the formatted data.
	
	@see PS_MAX_PRINTFORMATTED_LENGTH
	*/
	PX_FOUNDATION_API void	printFormatted(const char*, ...);
	
	/** 
	\brief Prints the string literally (does not consume % specifier).
	*/
	PX_FOUNDATION_API void	printString(const char*);

} // namespace shdfnd
} // namespace physx


#endif
