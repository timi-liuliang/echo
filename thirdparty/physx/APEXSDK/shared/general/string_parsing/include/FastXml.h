/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef FAST_XML_H
#define FAST_XML_H

#include "PsShare.h"
#include "foundation/PxSimpleTypes.h"	 // defines basic data types; modify for your platform as needed.
#include "PxFileBuf.h"		// defines the basic file stream interface.

namespace FAST_XML
{

class FastXml
{
public:
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
		virtual bool processClose(const char *element,physx::PxU32 depth,bool &isError) = 0;	  // process the 'close' indicator for a previously encountered element

		// return true to continue processing the XML document, false to skip.
		virtual bool processElement(
			const char *elementName,   // name of the element
			physx::PxI32 argc,         // number of attributes pairs
			const char **argv,         // list of attributes.
			const char  *elementData,  // element data, null if none
			physx::PxI32 lineno) = 0;  // line number in the source XML file

		// process the XML declaration header
		virtual bool processXmlDeclaration(
			physx::PxI32 /*argc*/,
			const char ** /*argv*/,
			const char  * /*elementData*/,
			physx::PxI32 /*lineno*/)
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

		virtual void *  fastxml_malloc(physx::PxU32 size) = 0;
		virtual void	fastxml_free(void *mem) = 0;

	};

	virtual bool processXml(physx::PxFileBuf &buff,bool streamFromMemory=false) = 0;

	virtual const char *getError(physx::PxI32 &lineno) = 0; // report the reason for a parsing error, and the line number where it occurred.

	virtual void release(void) = 0;

    virtual ~FastXml() {}
};

const char *getAttribute(const char *attr, physx::PxI32 argc, const char **argv);

FastXml * createFastXml(FastXml::Callback *iface);

}; // end of namespace FAST_XML

#endif // FAST_XML_H
