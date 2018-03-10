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
#ifndef PX_XML_WRITER_H
#define PX_XML_WRITER_H

#include "PxSimpleTypes.h"

namespace physx {

	struct PxRepXObject;

	/** 
	 *	Writer used by extensions to write elements to a file or database
	 */
	class XmlWriter
	{
	protected:
		virtual ~XmlWriter(){}
	public:
		/** Write a key-value pair into the current item */
		virtual void write( const char* inName, const char* inData ) = 0;
		/** Write an object id into the current item */
		virtual void write( const char* inName, const PxRepXObject& inLiveObject ) = 0;
		/** Add a child that then becomes the current context */
		virtual void addAndGotoChild( const char* inName ) = 0;
		/** Leave the current child */
		virtual void leaveChild() = 0;
	};
} 

#endif
