/**********************************************************************
*<
FILE: HWIndex.h

DESCRIPTION: Hardware Index Interface Implementation

CREATED BY: Peter Watje

HISTORY: Created June 13, 2008

Copyright 2008 Autodesk, Inc.  All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement 
provided at the time of installation or download, or which otherwise accompanies 
this software in either electronic or hard copy form.   

**********************************************************************/


#pragma once
#include "export.h"
#include "maxheap.h" 
#include "tab.h" 

namespace GFX_MESH
{

/*!  \brief These are our current supported primitive mesh types which we uses with the HWIndexBuffer class*/
enum PrimitiveType { kLineList = 2, kTriangleList = 3};

//! \brief This class is used to help with managing 32 bit hardware index buffer.
/*!
This class is used to help with managing 32 bit hardware index buffer it is not designed 
to be used as representation of an entire index buffer but used to map into an existing buffer.  
It can be used to overlay an existing index buffer or as temp buffer to be filled out then 
copied into the final buffer*/
class HWIndex32Bit : public  MaxHeapOperators
{
public:
	//! \brief Constructor 
	/*!
	\param indicesPerElement are the number of indices that make up each element.  For instance	a triangle is 3, a line segment is 2 etc.
	*/
	DllExport HWIndex32Bit(unsigned int indicesPerElement);

	//! \brief Assigns array of DWORD to this hw index
	/*! This lets you assign a buffer to map into DWORD *buffer is the buffer.  This will let the HWIndex
		map into the buffer so you can extract a specific element out of it.
		\param buffer the array of indices to be mapped
	*/
	DllExport void SetBuffer(DWORD *buffer);

	//! \brief Returns the number of indices per element.
	/*! \return the number of indices per element.  For instance a triangle would return 3 a line list 2*/
	DllExport const unsigned int Count();

	//! \brief Array access to each index
	/*! Array access to each index.  Out of bounds access <0 || > Count() will default to returning
	the first index	
	\return The index value.
	*/
	DllExport DWORD& operator[](int i);
	DllExport const DWORD& operator[](int i) const;


private:
	int mCount;				//number of indices per element
	DWORD *mIndex;			//pointer to the actual raw index array
	static DWORD errorIndex;

};

//! \brief This class is used to help with managing 16 bit hardware index buffer.
/*! This class is used to help with managing 16 bit hardware index buffer.
it is not designed to be used as representation of an entire index buffer but
used to map into an existing buffer.  It can be used to overlay an existing index buffer 
or as temp buffer to be filled out then copied into the final buffer*/
class HWIndex16Bit : public  MaxHeapOperators
{
public:
	//! \brief Constructor
	/*!	Constructor 
	\param indicesPerElement are the number of indices that make up each element.  For instance
	a triangle is 3, a line segment is 2 etc.*/
	DllExport HWIndex16Bit(unsigned int indicesPerElement);

	//! \brief Assigns array of WORD to this hw index
	/*! This lets you assign a buffer to map into WORD *buffer is the buffer.  This will let the HWIndex
	map into the buffer so you can extract a specific element out of it.
	\param buffer the array of indices to be mapped
	*/
	DllExport void SetBuffer(WORD *buffer);

	//! \brief Returns the number of indices per element.
	/*! \return This returns the number of indices per element.  For instance a triangle would return 3 a line list 2*/
	DllExport const unsigned int Count();

	//! \brief Array access to each index
	/*! Array access to each index.  Out of bounds access <0 || > Count() will default to returning
	the first index	
	\return The index value
	*/
	DllExport WORD& operator[](int i);
	DllExport const WORD& operator[](int i) const;

private:
	int mCount;		//number of indices per element
	WORD *mIndex;	//pointer to the actual raw index array
};

//! \brief This class is our hw index buffer representation.  
/*! This class is our hw index buffer representation.  It is just basically a list of 
DWORDs or WORDs and a description of the number of indices per element. */
class HWIndexBuffer : public  MaxHeapOperators
{
public:

		//! \brief Constructor
		/*!  HWIndexBuffer(unsigned int count, unsigned int indicesPerPrimitive, bool use16Bit) Constructor		\n\n
			\param count - this is the number of primitives for this buffer to be allocated \n
			\param indicesPerPrimitive - this is the number of indices per primitive \n
			\param use16Bit - this is whether to use 16 bit buffers for the indices \n
		*/
		DllExport HWIndexBuffer(unsigned int count, unsigned int indicesPerPrimitive, bool use16Bit);
		/*! \brief Destructor */
		DllExport ~HWIndexBuffer();

		//! \brief This frees all our index buffers
		/*!  This frees all our index buffers and resets the count to 0*/
		DllExport void FreeBuffer();

		//! \brief returns the number of primitives in the index buffer
		/*! \return the number of primitives in the index buffer */
		DllExport const unsigned int NumberPrimitives();

		//! \brief Returns the number of indices per primitive 
		/*! \return the number of indices per primitive ie a triangle 3, line list 2 etc */
		DllExport const unsigned int IndicesPerPrimitive();

		//! \brief The size of the buffer in bytes 
		//! \return the size of index buffer in bytes */
		DllExport const unsigned int Size();

		//! \brief This lets you reset the number of primitives
		/*! This lets you reset the number of primitives in the list	it will keep the original data.  It will
			returns true if successful.
		\param ct is the new size for the buffer
		\return true is successful otherwise it ran out of memory
		*/
		DllExport bool SetNumberPrimitives(unsigned int ct);

		//! \brief This returns whether the buffers are 16 or 32 bit.
		/*! \return whether the buffers are 16 or 32 bit.  Right now we tend to use 16 bit buffers
			and split up the mesh to fit into those buffers so we can reduce the memory foot print.*/
		DllExport bool Is16Bit();


		//! \brief Returns a particular 32 bit HWIndex
		/*!  Returns a particular 32 bit HWIndex.  This is useful if you dont want to look up the offsets by hand 
			/param int index - the index of that you want
			\return a particular 32 bit HWIndex
		*/
		DllExport  HWIndex32Bit GetHWIndex32Bit(int index);

		//! \brief Returns a particular 16 bit HWIndex
		/*!  Returns a particular 16 bit HWIndex.  This is useful if you dont want to look up the offsets by hand 
			/param int index - the index of that you want
		*/
		DllExport  HWIndex16Bit GetHWIndex16Bit(int index);


		//! \brief Returns the raw pointer to the 32 but buffer
		/*!  \return Access to the raw 32 bit data.   This is only valid if the buffer is not set to 16 bit */
		DllExport  const DWORD *GetRawBufferPointer();
		//! \brief Returns the raw pointer to the 16 bit buffer
		/*!  \return Access to the raw 16 bit data.   This is only valid if the buffer is set to 16 bit */
		DllExport  const WORD *GetRawBufferPointer16Bit();

private:

	PrimitiveType	mPrimitiveType;		//the primitive type

	int mPrimitiveCount;					//Number of elements in the buffers
	int mIndicesPerPrimitive;		//Number of indices per element
	bool mUse16Bit;				//whether to use 16 or 32 bit index buffers
	Tab<DWORD> mIndices32Bit;			// the actual raw pointer to the 32 bit buffer
	Tab<WORD> mIndices16Bit;		// the actual raw pointer to the 16 bit buffer

};
}