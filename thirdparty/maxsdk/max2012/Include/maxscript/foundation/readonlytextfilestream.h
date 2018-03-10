//
// Copyright [2010] Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//

#pragma once

#include "streams.h"

invisible_class (ReadonlyTextFileStream)

/**
 * ReadonlyTextFileStream is only used for opening text file in read-only 
 * mode. It's equivalent to use FileStream when opening files in "rt" mode. 
 * But FileStream is very slow when reading files. So we optimize the original
 * codes in FileStream and create this new class which is thousand times faster
 * for reading text files than FileStream.
 * \note Invoke any non-read-only methods in ReadonlyTextFileStream will result
 *		 in a runtime exception. Such methods include putch(),puts(),printf().
 */
class ReadonlyTextFileStream : public CharStream
{
public:
	/**
	* The default constructor.
	*/
	ScripterExport ReadonlyTextFileStream();
	/**
	* The default destructor.
	*/
	ScripterExport ~ReadonlyTextFileStream();

	/**
	* Open a new text file.
	* \param[in]  ifile_name The file name.
	* \return The ReadonlyTextFileStream instance itself if max is able to 
				open the file or undefined if it fails.
	*/
	ScripterExport ReadonlyTextFileStream* open(const MCHAR* ifile_name);

	/* internal char stream protocol */
	MCHAR	get_char();
	void	unget_char(MCHAR c);
	MCHAR	peek_char();
	int		at_eos();
	unsigned int	pos();
	void	seek(int pos);
	void	rewind();
	void	flush_to_eol();
	MCHAR	putch(MCHAR c);
	const MCHAR*	puts(const MCHAR* str);
	int		printf(const MCHAR *format, ...);

	void	collect();
	void	close();

	void	gc_trace();
	void	log_to(CharStream* log);
	void	close_log();
	CharStream*	get_log();
private:
	void VerifyFileOpen() const;
	MCHAR CharAtPos(size_t pos);
	int mUngetcharCount;
	MCHAR mUngetcharBuf[8];
	char* mFileBuf;
	size_t mCharCount;
	size_t mCharPos;
	CharStream*	mLog;
};