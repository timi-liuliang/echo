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

#ifndef PX_FOUNDATION_PSFASTXML_H
#define PX_FOUNDATION_PSFASTXML_H

#include "foundation/PxSimpleTypes.h"	// defines basic data types; modify for your platform as needed.
#include "foundation/PxIO.h"
#include "foundation/PxAssert.h"
#include "PsAllocator.h"

namespace physx
{
namespace shdfnd
{

class FastXml
{
	PX_NOCOPY(FastXml)

public:	
	
	class AttributePairs
	{
		int argc;
		const char** argv;		

	public:
		AttributePairs() : argc(0), argv(NULL) {}
		AttributePairs(int c, const char** v) : argc(c), argv(v) {}

		PX_INLINE int getNbAttr() const { return argc/2; } 

		const char* getKey(PxU32 index) const 
		{ 
			PX_ASSERT( (index*2) < (PxU32)argc);
			return argv[index*2];			
		}

		const char* getValue(PxU32 index) const 
		{ 
			PX_ASSERT( (index*2+1) < (PxU32)argc);
			return argv[index*2+1];			
		}

		const char* get(const char* attr) const
		{
			PxI32 count = argc/2;
			for(PxI32 i = 0; i < count; ++i)
			{
				const char *key = argv[i*2], *value = argv[i*2+1];
				if( strcmp(key, attr) == 0 )
					return value;
			}

			return NULL;
		}
	};


	/***
	* Callbacks to the user with the contents of the XML file properly digested.
	*/
	class Callback
	{
	public:

        virtual ~Callback() {}
		virtual bool processComment(const char *comment) = 0; // encountered a comment in the XML

		// 'element' is the name of the element that is being closed.
		// depth is the recursion depth of this element.
		// Return true to continue processing the XML file.
		// Return false to stop processing the XML file; leaves the read pointer of the stream right after this close tag.
		// The bool 'isError' indicates whether processing was stopped due to an error, or intentionally canceled early.
		virtual bool processClose(const char *element,PxU32 depth,bool &isError) = 0;	  // process the 'close' indicator for a previously encountered element

		// return true to continue processing the XML document, false to skip.
		virtual bool processElement(
			const char *elementName,   // name of the element                      
			const char *elementData,   // element data, null if none
			const AttributePairs& attr,// attributes
			PxI32 lineno) = 0;         // line number in the source XML file
		
		// process the XML declaration header
		virtual bool processXmlDeclaration(
			const AttributePairs& ,  // attributes
			const char  * /*elementData*/,
			PxI32 /*lineno*/)
		{
			return true;
		}

		virtual bool processDoctype(
			const char * /*rootElement*/, //Root element tag
			const char * /*type*/,        //SYSTEM or PUBLIC
			const char * /*fpi*/,         //Formal Public Identifier
			const char * /*uri*/)         //Path to schema file
		{
			return true;
		}

		virtual void* allocate(PxU32 size) 
		{
			return getAllocator().allocate(size, "FastXml", __FILE__, __LINE__);
		}

		virtual void deallocate(void* ptr)
		{
			getAllocator().deallocate(ptr);
		}

	};

	virtual bool processXml(PxInputData &buff,bool streamFromMemory=false) = 0;

	virtual const char *getError(PxI32 &lineno) = 0; // report the reason for a parsing error, and the line number where it occurred.

	FastXml(){}

	virtual void release(void) = 0;

protected:
	virtual ~FastXml() {}
};

PX_FOUNDATION_API FastXml * createFastXml(FastXml::Callback *iface);

}  //shdfnd
} //physx

#endif // PX_FOUNDATION_PSFASTXML_H
