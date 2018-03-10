/**********************************************************************
 *<
	FILE: appio.h

	DESCRIPTION:  General chunk-ifying code: useful for writing 
	   hierarchical data structures to a linear stream, such as
	   an AppData block.

	CREATED BY: Dan Silva

	HISTORY: created 3/24/97

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "coreexp.h"
#include "maxheap.h"
#include "strbasic.h"
#include <WTypes.h>
#include "maxtypes.h"

//------------------------------------------------------------------------
// AppSave will write hierarchical chunks into a private buffer, enlarging 
//  it as needed.  When completed, use the methods BufferPtr() and 
// NBytesWritten() to get at this buffer. ( AppSave will delete the buffer in 
// its DeleteThis() method , so you need to copy the buffer to save the data.)

// The chunk hierarchy should always have a single highest level chunk.
// Chunks can be nested to any depth.
// A Chunk can contain either sub-chunks, or data, but not both.

//   For example:
//
//	AppSave *asave = NewAppSave(1000);
//	asave->BeginChunk(MAIN_CHUNK);
//	   asave->BeginChunk(CHUNK1);
//      .. write data 
//	   asave->EndChunk();
//
//	   asave->BeginChunk(CHUNK2);
//      .. write data 
//	   asave->EndChunk();
//
//	   asave->BeginChunk(CHUNK3);
//      .. write data 
//	   asave->EndChunk();
//	asave->EndChunk();  // end MAIN_CHUNK


/*! \sa  Class AppLoad, Class AppDataChunk, Class CharacterStrings.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class is a general chunk-ifying code useful for writing hierarchical data
structures to a linear stream, such as an AppData block.\n\n
All methods of this class are implemented by the system.\n\n
AppSave will write hierarchical chunks into a private buffer, enlarging it as
needed. When completed, use the methods <b>BufferPtr()</b> and
<b>NBytesWritten()</b> to get at this buffer. (<b>AppSave</b> will delete the
buffer in its <b>DeleteThis()</b> method , so you need to copy the buffer to
save the data.)\n\n
The chunk hierarchy should always have a single highest level chunk. Chunks can
be nested to any depth.\n\n
A Chunk can contain either sub-chunks, or data, but not both.
\par Sample Code:
\code
AppSave *asave = NewAppSave(1000);
asave->BeginChunk(MAIN_CHUNK);
asave->BeginChunk(CHUNK1);
// .. write data
asave->EndChunk();
 
asave->BeginChunk(CHUNK2);
// .. write data
asave->EndChunk();
 
asave->BeginChunk(CHUNK3);
// .. write data
asave->EndChunk();
asave->EndChunk(); // end MAIN_CHUNK 
\endcode */
class AppSave: public MaxHeapOperators {
	protected:
		virtual ~AppSave() {}
	public:
		/*! \remarks This method deletes the <b>AppSave</b> instance. */
		virtual void DeleteThis()=0;
		
		// After saving, use this to get pointer to the buffer created.
		/*! \remarks This method is used after saving to get a pointer to the
		buffer created. */
		virtual BYTE* BufferPtr()=0;

		// This tells how many bytes were written in the buffer.
		/*! \remarks This method returns the number of bytes that were written
		in the buffer. */
		virtual int NBytesWritten()=0;
		
		// Begin a chunk.
		/*! \remarks This method is used to begin a chunk. The ID passed need
		only be unique within the plug-ins data itself.
		\par Parameters:
		<b>USHORT id</b>\n\n
		The id for the chunk. */
		virtual void BeginChunk(USHORT id)=0;

		// End a chunk, and back-patch the length.
		/*! \remarks This method is used to end a chunk, and back-patch the
		length. */
		virtual void EndChunk()=0;

		/*! \remarks This method is used internally for checking balanced
		BeginChunk/EndChunk. */
		virtual int CurChunkDepth()=0;  // for checking balanced BeginChunk/EndChunk

		// write a block of bytes to the output stream.
		/*! \remarks This method writes a block of bytes.
		\par Parameters:
		<b>const void *buf</b>\n\n
		The buffer to write.\n\n
		<b>ULONG nbytes</b>\n\n
		The number of bytes to write.\n\n
		<b>ULONG *nwrit</b>\n\n
		The number of bytes actually written.
		\return  <b>IO_OK</b> - The write was acceptable - no errors.\n\n
		<b>IO_ERROR</b> - This is returned if an error occurred. */
		virtual IOResult Write(const void  *buf, ULONG nbytes, ULONG *nwrit)=0;

		// Write character strings
		/*! \remarks This method is used to write wide character strings.
		\par Parameters:
		<b>const char *str</b>\n\n
		The string to write.
		\return  <b>IO_OK</b> - The write was acceptable - no errors.\n\n
		<b>IO_ERROR</b> - This is returned if an error occurred. */
		virtual IOResult WriteWString(const char *str)=0;

		/*! \remarks This method is used to write wide character strings.
		\par Parameters:
		<b>const wchar_t *str</b>\n\n
		The string to write.
		\return  <b>IO_OK</b> - The write was acceptable - no errors.\n\n
		<b>IO_ERROR</b> - This is returned if an error occurred. */
		virtual IOResult WriteWString(const mwchar_t *str)=0;
		/*! \remarks This method is used to write single byte character
		strings.
		\par Parameters:
		<b>const char *str</b>\n\n
		The string to write.
		\return  <b>IO_OK</b> - The write was acceptable - no errors.\n\n
		<b>IO_ERROR</b> - This is returned if an error occurred. */
		virtual IOResult WriteCString(const char *str)=0;

		/*! \remarks This method is used to write single byte character strings.
		\par Parameters:
		<b>const wchar_t *str</b>\n\n
		The string to write.
		\return  <b>IO_OK</b> - The write was acceptable - no errors.\n\n
		<b>IO_ERROR</b> - This is returned if an error occurred. */
		virtual IOResult WriteCString(const mwchar_t *str)=0;

		};

//------------------------------------------------------------------------
// AppLoad takes a chunk-ified data stream, and provides routines for 
// decoding it. 

/*! \sa  Class AppSave, Class AppDataChunk.\n\n
\par Description:
This class takes a chunk-ified data stream (as written by Class AppSave), and
provides routines for decoding it.\n\n
This class is available in release 2.0 and later only.\n\n
All methods of this class are implemented by the system. */
class AppLoad: public MaxHeapOperators {
	protected:
		virtual ~AppLoad() {};
	public:
		/*! \remarks This method deletes the instance of AppLoad. */
		virtual void DeleteThis()=0;
			
		// if OpenChunk returns IO_OK, use following 3 function to get the 
		// info about the chunk. IO_END indicates no more chunks at this level
		/*! \remarks This method is used to open a chunk. If
		<b>OpenChunk()</b> returns <b>IO_OK</b>, use the following 3 functions
		to get the information about the chunk. If it returns <b>IO_END</b>
		this indicates there are no more chunks at this level.
		\return  <b>IO_OK</b> - The result was acceptable - no errors.\n\n
		<b>IO_END</b> - This is returned when the end of the chunks at a
		certain level have been reached. It is used as a signal to terminate
		the processing of chunks at that level.\n\n
		<b>IO_ERROR</b> - This is returned if an error occurred. */
		virtual IOResult OpenChunk()=0;

		// These give info about the most recently opened chunk
		/*! \remarks This method returns the ID of the most recently opened
		chunk. */
		virtual USHORT CurChunkID()=0;
		/*! \remarks This method returns the type of the most recently opened
		chunk. This may be one of the following values:\n\n
		<b>NEW_CHUNK</b>\n\n
		<b>CONTAINER_CHUNK</b>\n\n
		<b>DATA_CHUNK</b> */
		virtual ChunkType CurChunkType()=0;
		/*! \remarks This method returns the chunk length <b>not</b> including
		the header. */
		virtual	ULONG CurChunkLength()=0;  // chunk length NOT including header
		/*! \remarks This method is used internally for checking for balanced
		OpenChunk/CloseChunk pairs. */
		virtual	int CurChunkDepth()=0;  // for checking balanced OpenChunk/CloseChunk

		// close the currently opened chunk, and position at the next chunk
		//  return of IO_ERROR indicates there is no open chunk to close
		/*! \remarks This method is used to close the currently opened chunk,
		and position at the next chunk.
		\return  A return value of <b>IO_ERROR</b> indicates there is no open
		chunk to close; otherwise <b>IO_OK</b>. */
		virtual IOResult CloseChunk()=0;

		// Look at the next chunk ID without opening it.
		// returns 0 if no more chunks
		/*! \remarks This method returns the ID of the next chunk without
		opening it. It returns 0 if there are no more chunks. */
		virtual	USHORT PeekNextChunkID()=0;

		// Read a block of bytes from the output stream.
		/*! \remarks This method is used to read a block of bytes.
		\par Parameters:
		<b>void *buf</b>\n\n
		A pointer to the buffer to read.\n\n
		<b>ULONG nbytes</b>\n\n
		The number of bytes to read.\n\n
		<b>ULONG *nread</b>\n\n
		The number of bytes that were read.
		\return  A return value of <b>IO_ERROR</b> indicates an error occurred,
		otherwise <b>IO_OK</b>. */
		virtual IOResult Read(void  *buf, ULONG nbytes, ULONG *nread )=0;

		// Read a string from a string chunk assumes chunk is already open, 
		// it will NOT close the chunk. Sets buf to point
		// to a char string.  Don't delete buf: ILoad will take care of it.

		//   Read a string that was stored as Wide chars. 
		/*! \remarks This method read a string that was stored as Wide
		characters. Note: This method reads a string from a string chunk. It is
		assumed the chunk is already open, it will NOT close the chunk.
		\par Parameters:
		<b>char** buf</b>\n\n
		A pointer to an array of characters.
		\return  A return value of <b>IO_ERROR</b> indicates an error occurred,
		otherwise <b>IO_OK</b>. */
		virtual IOResult ReadWStringChunk(char** buf)=0;

		/*! \remarks This method read a string that was stored as Wide characters.
		Note: This method reads a string from a string chunk. It is assumed the chunk
		is already open, it will NOT close the chunk.
		\par Parameters:
		<b>mwchar_t** buf</b>\n\n
		A pointer to an array of wide characters.
		\return  A return value of <b>IO_ERROR</b> indicates an error occurred,
		otherwise <b>IO_OK</b>. */
		virtual IOResult ReadWStringChunk(mwchar_t** buf)=0;

		//   Read a string that was stored as single byte chars
		/*! \remarks This method reads a string that was stored as single byte
		characters.
		\par Parameters:
		<b>char** buf</b>\n\n
		A pointer to an array of single byte characters.
		\return  A return value of <b>IO_ERROR</b> indicates an error occurred,
		otherwise <b>IO_OK</b>. */
		virtual IOResult ReadCStringChunk(char** buf)=0;

		/*! \remarks This method read a string that was stored as Wide chars. Note:
		This method reads a string from a string chunk. It is assumed the chunk is
		already open, it will NOT close the chunk.
		\par Parameters:
		<b>mwchar_t** buf</b>\n\n
		A pointer to an array of wide characters.
		\return  A return value of <b>IO_ERROR</b> indicates an error occurred,
		otherwise <b>IO_OK</b>. */
		virtual IOResult ReadCStringChunk(mwchar_t** buf)=0;
	};


// Create a new AppLoad for reading chunks out of buf:
// bufSize specifies the number of bytes that are valid in
// buf.. 
/*! \remarks This global function creates a new AppLoad instance for reading
chunks out of <b>buf</b>:
\par Parameters:
<b>BYTE *buf</b>\n\n
The buffer to read.\n\n
<b>int bufSize</b>\n\n
Specifies the nuymber of bytes that are valid in <b>buf</b>.  */
CoreExport AppLoad* NewAppLoad(BYTE* buf, int bufSize);

// Create a new AppSave for writing chunks
// InitbufSize is the initial size the internal buffer is allocated to.
// It will be enlarged if necessary.
/*! \remarks This global function create a new <b>AppSave</b> instance.
\par Parameters:
<b>int initBufSize</b>\n\n
Specifies the initial size the internal buffer is allocated to. It will be
enlarged if necessary.  */
CoreExport AppSave* NewAppSave(int initBufSize);

