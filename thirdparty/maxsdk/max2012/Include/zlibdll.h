/* zlibdll.h -- interface of the 'zlib' general purpose compression library
  version 1.0.4, Jul 24th, 1996.

  Copyright (C) 1995-1996 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  gzip@prep.ai.mit.edu    madler@alumni.caltech.edu


  The data format used by the zlib library is described by RFCs (Request for
  Comments) 1950 to 1952 in the files ftp://ds.internic.net/rfc/rfc1950.txt
  (zlib format), rfc1951.txt (deflate format) and rfc1952.txt (gzip format).
*/

/* Modifications:
 *     conversion to C++; to handle IStream objects; to be a DLL
 * Copyright (C) 1998 Autodesk, Inc.
 *
 * Written by Pete Samson, Kinetix
 */

#pragma once

#define ZLIB_VERSION "1.0.4"

#include "maxheap.h"
#include "strbasic.h"

#define USE_ISTREAMS

#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4
/* Allowed flush values; see deflate() below for details */

#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)
/* Return codes for the compression/decompression functions. Negative
 * values are errors, positive values are used for special but normal events.
 */


#define Z_NULL  0  /* for initializing zalloc, zfree, opaque */

#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)
/* compression levels */

#define Z_FILTERED            1
#define Z_HUFFMAN_ONLY        2
#define Z_DEFAULT_STRATEGY    0
/* compression strategy; see deflateInit2() below for details */


// IMPORTANT: Must follow the definitions from zconf.h!
typedef unsigned char  Byte;  /* 8 bits */
typedef unsigned int   uInt;  /* 16 bits or more */

// SR NOTE64: Support for files bigger than 4G; the code is reasonably clean and
// can handle it without any problems.
#if defined(_WIN64)
#  include <basetsd.h>        // ULONG_PTR

typedef ULONG_PTR uLong;      /* 32 bits or more */	

#else

typedef unsigned long  uLong; /* 32 bits or more */

#endif

typedef unsigned long uLong32;   /* 32 bits always */

typedef void     *voidpf;
typedef void     *voidp;
typedef voidpf (*alloc_func) (voidpf opaque, uInt items, uInt size);
typedef void   (*free_func)  (voidpf opaque, voidpf address);
typedef Byte  Bytef;
typedef char   charf;
typedef int    intf;
typedef uInt   uIntf;
typedef uLong  uLongf;
typedef uLong32 uLong32f;

typedef struct z_stream_s: public MaxHeapOperators {
    Bytef    *next_in;  /* next input byte */
    uInt     avail_in;  /* number of bytes available at next_in */
    uLong    total_in;  /* total nb of input bytes read so far */

    Bytef    *next_out; /* next output byte should be put there */
    uInt     avail_out; /* remaining free space at next_out */
    uLong    total_out; /* total nb of bytes output so far */

    char     *msg;      /* last error message, NULL if no error */
    struct internal_state *state; /* not visible by applications */

    alloc_func zalloc;  /* used to allocate the internal state */
    free_func  zfree;   /* used to free the internal state */
    voidpf     opaque;  /* private data object passed to zalloc and zfree */

    int     data_type;  /* best guess about the data type: ascii or binary */
    uLong   adler;      /* adler32 value of the uncompressed data */
    uLong   reserved;   /* reserved for future use */
} z_stream;

typedef z_stream *z_streamp;

typedef mwchar_t WCHAR;


                        /* basic functions */

extern const char *  zlibVersion(void);
/* The application can compare zlibVersion and ZLIB_VERSION for consistency.
   If the first character differs, the library code actually used is
   not compatible with the zlib.h header file used by the application.
   This check is automatically made by deflateInit and inflateInit.
 */

/* 
extern int  deflateInit(z_streamp strm, int level);

     Initializes the internal stream state for compression. The fields
   zalloc, zfree and opaque must be initialized before by the caller.
   If zalloc and zfree are set to Z_NULL, deflateInit updates them to
   use default allocation functions.

     The compression level must be Z_DEFAULT_COMPRESSION, or between 0 and 9:
   1 gives best speed, 9 gives best compression, 0 gives no compression at
   all (the input data is simply copied a block at a time).
   Z_DEFAULT_COMPRESSION requests a default compromise between speed and
   compression (currently equivalent to level 6).

     deflateInit returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_STREAM_ERROR if level is not a valid compression level,
   Z_VERSION_ERROR if the zlib library version (zlib_version) is incompatible
   with the version assumed by the caller (ZLIB_VERSION).
   msg is set to null if there is no error message.  deflateInit does not
   perform any compression: this will be done by deflate().
*/


extern int  deflate(z_streamp strm, int flush);
/*
  Performs one or both of the following actions:

  - Compress more input starting at next_in and update next_in and avail_in
    accordingly. If not all input can be processed (because there is not
    enough room in the output buffer), next_in and avail_in are updated and
    processing will resume at this point for the next call of deflate().

  - Provide more output starting at next_out and update next_out and avail_out
    accordingly. This action is forced if the parameter flush is non zero.
    Forcing flush frequently degrades the compression ratio, so this parameter
    should be set only when necessary (in interactive applications).
    Some output may be provided even if flush is not set.

  Before the call of deflate(), the application should ensure that at least
  one of the actions is possible, by providing more input and/or consuming
  more output, and updating avail_in or avail_out accordingly; avail_out
  should never be zero before the call. The application can consume the
  compressed output when it wants, for example when the output buffer is full
  (avail_out == 0), or after each call of deflate(). If deflate returns Z_OK
  and with zero avail_out, it must be called again after making room in the
  output buffer because there might be more output pending.

    If the parameter flush is set to Z_PARTIAL_FLUSH, the current compression
  block is terminated and flushed to the output buffer so that the
  decompressor can get all input data available so far. For method 9, a future
  variant on method 8, the current block will be flushed but not terminated.
  Z_SYNC_FLUSH has the same effect as partial flush except that the compressed
  output is byte aligned (the compressor can clear its internal bit buffer)
  and the current block is always terminated; this can be useful if the
  compressor has to be restarted from scratch after an interruption (in which
  case the internal state of the compressor may be lost).
    If flush is set to Z_FULL_FLUSH, the compression block is terminated, a
  special marker is output and the compression dictionary is discarded; this
  is useful to allow the decompressor to synchronize if one compressed block
  has been damaged (see inflateSync below).  Flushing degrades compression and
  so should be used only when necessary.  Using Z_FULL_FLUSH too often can
  seriously degrade the compression. If deflate returns with avail_out == 0,
  this function must be called again with the same value of the flush
  parameter and more output space (updated avail_out), until the flush is
  complete (deflate returns with non-zero avail_out).

    If the parameter flush is set to Z_FINISH, pending input is processed,
  pending output is flushed and deflate returns with Z_STREAM_END if there
  was enough output space; if deflate returns with Z_OK, this function must be
  called again with Z_FINISH and more output space (updated avail_out) but no
  more input data, until it returns with Z_STREAM_END or an error. After
  deflate has returned Z_STREAM_END, the only possible operations on the
  stream are deflateReset or deflateEnd.
  
    Z_FINISH can be used immediately after deflateInit if all the compression
  is to be done in a single step. In this case, avail_out must be at least
  0.1% larger than avail_in plus 12 bytes.  If deflate does not return
  Z_STREAM_END, then it must be called again as described above.

    deflate() may update data_type if it can make a good guess about
  the input data type (Z_ASCII or Z_BINARY). In doubt, the data is considered
  binary. This field is only for information purposes and does not affect
  the compression algorithm in any manner.

    deflate() returns Z_OK if some progress has been made (more input
  processed or more output produced), Z_STREAM_END if all input has been
  consumed and all output has been produced (only when flush is set to
  Z_FINISH), Z_STREAM_ERROR if the stream state was inconsistent (for example
  if next_in or next_out was NULL), Z_BUF_ERROR if no progress is possible.
*/


extern int  deflateEnd(z_streamp strm);
/*
     All dynamically allocated data structures for this stream are freed.
   This function discards any unprocessed input and does not flush any
   pending output.

     deflateEnd returns Z_OK if success, Z_STREAM_ERROR if the
   stream state was inconsistent, Z_DATA_ERROR if the stream was freed
   prematurely (some input or output was discarded). In the error case,
   msg may be set but then points to a static string (which must not be
   deallocated).
*/


 
extern int  inflateInit(z_streamp strm);

/*
     Initializes the internal stream state for decompression. The fields
   zalloc, zfree and opaque must be initialized before by the caller.  If
   zalloc and zfree are set to Z_NULL, inflateInit updates them to use default
   allocation functions.

     inflateInit returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_VERSION_ERROR if the zlib library version is incompatible
   with the version assumed by the caller.  msg is set to null if there is no
   error message. inflateInit does not perform any decompression: this will be
   done by inflate().
*/


extern int  inflate(z_streamp strm, int flush);
/*
  Performs one or both of the following actions:

  - Decompress more input starting at next_in and update next_in and avail_in
    accordingly. If not all input can be processed (because there is not
    enough room in the output buffer), next_in is updated and processing
    will resume at this point for the next call of inflate().

  - Provide more output starting at next_out and update next_out and avail_out
    accordingly.  inflate() provides as much output as possible, until there
    is no more input data or no more space in the output buffer (see below
    about the flush parameter).

  Before the call of inflate(), the application should ensure that at least
  one of the actions is possible, by providing more input and/or consuming
  more output, and updating the next_* and avail_* values accordingly.
  The application can consume the uncompressed output when it wants, for
  example when the output buffer is full (avail_out == 0), or after each
  call of inflate(). If inflate returns Z_OK and with zero avail_out, it
  must be called again after making room in the output buffer because there
  might be more output pending.

    If the parameter flush is set to Z_PARTIAL_FLUSH, inflate flushes as much
  output as possible to the output buffer. The flushing behavior of inflate is
  not specified for values of the flush parameter other than Z_PARTIAL_FLUSH
  and Z_FINISH, but the current implementation actually flushes as much output
  as possible anyway.

    inflate() should normally be called until it returns Z_STREAM_END or an
  error. However if all decompression is to be performed in a single step
  (a single call of inflate), the parameter flush should be set to
  Z_FINISH. In this case all pending input is processed and all pending
  output is flushed; avail_out must be large enough to hold all the
  uncompressed data. (The size of the uncompressed data may have been saved
  by the compressor for this purpose.) The next operation on this stream must
  be inflateEnd to deallocate the decompression state. The use of Z_FINISH
  is never required, but can be used to inform inflate that a faster routine
  may be used for the single inflate() call.

    inflate() returns Z_OK if some progress has been made (more input
  processed or more output produced), Z_STREAM_END if the end of the
  compressed data has been reached and all uncompressed output has been
  produced, Z_NEED_DICT if a preset dictionary is needed at this point (see
  inflateSetDictionary below), Z_DATA_ERROR if the input data was corrupted,
  Z_STREAM_ERROR if the stream structure was inconsistent (for example if
  next_in or next_out was NULL), Z_MEM_ERROR if there was not enough memory,
  Z_BUF_ERROR if no progress is possible or if there was not enough room in
  the output buffer when Z_FINISH is used. In the Z_DATA_ERROR case, the
  application may then call inflateSync to look for a good compression block.
  In the Z_NEED_DICT case, strm->adler is set to the Adler32 value of the
  dictionary chosen by the compressor.
*/


extern int  inflateEnd(z_streamp strm);
/*
     All dynamically allocated data structures for this stream are freed.
   This function discards any unprocessed input and does not flush any
   pending output.

     inflateEnd returns Z_OK if success, Z_STREAM_ERROR if the stream state
   was inconsistent. In the error case, msg may be set but then points to a
   static string (which must not be deallocated).
*/

                        /* Advanced functions */

/*
    The following functions are needed only in some special applications.
*/

extern int  deflateInit2(z_streamp strm,
                                   int  level,
                                   int  method,
                                   int  windowBits,
                                   int  memLevel,
                                   int  strategy);

/*
     This is another version of deflateInit with more compression options. The
   fields next_in, zalloc, zfree and opaque must be initialized before by
   the caller.

     The method parameter is the compression method. It must be Z_DEFLATED in
   this version of the library. (Method 9 will allow a 64K history buffer and
   partial block flushes.)

     The windowBits parameter is the base two logarithm of the window size
   (the size of the history buffer).  It should be in the range 8..15 for this
   version of the library (the value 16 will be allowed for method 9). Larger
   values of this parameter result in better compression at the expense of
   memory usage. The default value is 15 if deflateInit is used instead.

     The memLevel parameter specifies how much memory should be allocated
   for the internal compression state. memLevel=1 uses minimum memory but
   is slow and reduces compression ratio; memLevel=9 uses maximum memory
   for optimal speed. The default value is 8. See zconf.h for total memory
   usage as a function of windowBits and memLevel.

     The strategy parameter is used to tune the compression algorithm. Use the
   value Z_DEFAULT_STRATEGY for normal data, Z_FILTERED for data produced by a
   filter (or predictor), or Z_HUFFMAN_ONLY to force Huffman encoding only (no
   string match).  Filtered data consists mostly of small values with a
   somewhat random distribution. In this case, the compression algorithm is
   tuned to compress them better. The effect of Z_FILTERED is to force more
   Huffman coding and less string matching; it is somewhat intermediate
   between Z_DEFAULT and Z_HUFFMAN_ONLY. The strategy parameter only affects
   the compression ratio but not the correctness of the compressed output even
   if it is not set appropriately.

     If next_in is not null, the library will use this buffer to hold also
   some history information; the buffer must either hold the entire input
   data, or have at least 1<<(windowBits+1) bytes and be writable. If next_in
   is null, the library will allocate its own history buffer (and leave next_in
   null). next_out need not be provided here but must be provided by the
   application for the next call of deflate().

     If the history buffer is provided by the application, next_in must
   must never be changed by the application since the compressor maintains
   information inside this buffer from call to call; the application
   must provide more input only by increasing avail_in. next_in is always
   reset by the library in this case.

      deflateInit2 returns Z_OK if success, Z_MEM_ERROR if there was
   not enough memory, Z_STREAM_ERROR if a parameter is invalid (such as
   an invalid method). msg is set to null if there is no error message.
   deflateInit2 does not perform any compression: this will be done by
   deflate(). 

                            
extern int  deflateSetDictionary(z_streamp strm,
                                           const Bytef *dictionary,
				           uInt  dictLength);
/*
     Initializes the compression dictionary (history buffer) from the given
   byte sequence without producing any compressed output. This function must
   be called immediately after deflateInit or deflateInit2, before any call
   of deflate. The compressor and decompressor must use exactly the same
   dictionary (see inflateSetDictionary).
     The dictionary should consist of strings (byte sequences) that are likely
   to be encountered later in the data to be compressed, with the most commonly
   used strings preferably put towards the end of the dictionary. Using a
   dictionary is most useful when the data to be compressed is short and
   can be predicted with good accuracy; the data can then be compressed better
   than with the default empty dictionary. In this version of the library,
   only the last 32K bytes of the dictionary are used.
     Upon return of this function, strm->adler is set to the Adler32 value
   of the dictionary; the decompressor may later use this value to determine
   which dictionary has been used by the compressor. (The Adler32 value
   applies to the whole dictionary even if only a subset of the dictionary is
   actually used by the compressor.)

     deflateSetDictionary returns Z_OK if success, or Z_STREAM_ERROR if a
   parameter is invalid (such as NULL dictionary) or the stream state
   is inconsistent (for example if deflate has already been called for this
   stream). deflateSetDictionary does not perform any compression: this will
   be done by deflate(). 
*/


extern int  deflateReset(z_streamp strm);
/*
     This function is equivalent to deflateEnd followed by deflateInit,
   but does not free and reallocate all the internal compression state.
   The stream will keep the same compression level and any other attributes
   that may have been set by deflateInit2.

      deflateReset returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent (such as zalloc or state being NULL).
*/


/*   
extern int  inflateInit2(z_streamp strm,
                                   int  windowBits);

     This is another version of inflateInit with more compression options. The
   fields next_out, zalloc, zfree and opaque must be initialized before by
   the caller.

     The windowBits parameter is the base two logarithm of the maximum window
   size (the size of the history buffer).  It should be in the range 8..15 for
   this version of the library (the value 16 will be allowed soon). The
   default value is 15 if inflateInit is used instead. If a compressed stream
   with a larger window size is given as input, inflate() will return with
   the error code Z_DATA_ERROR instead of trying to allocate a larger window.

     If next_out is not null, the library will use this buffer for the history
   buffer; the buffer must either be large enough to hold the entire output
   data, or have at least 1<<windowBits bytes.  If next_out is null, the
   library will allocate its own buffer (and leave next_out null). next_in
   need not be provided here but must be provided by the application for the
   next call of inflate().

     If the history buffer is provided by the application, next_out must
   never be changed by the application since the decompressor maintains
   history information inside this buffer from call to call; the application
   can only reset next_out to the beginning of the history buffer when
   avail_out is zero and all output has been consumed.

      inflateInit2 returns Z_OK if success, Z_MEM_ERROR if there was
   not enough memory, Z_STREAM_ERROR if a parameter is invalid (such as
   windowBits < 8). msg is set to null if there is no error message.
   inflateInit2 does not perform any decompression: this will be done by
   inflate().
*/

extern int  inflateSetDictionary(z_streamp strm,
				           const Bytef *dictionary,
					   uInt  dictLength);
/*
     Initializes the decompression dictionary (history buffer) from the given
   uncompressed byte sequence. This function must be called immediately after
   a call of inflate if this call returned Z_NEED_DICT. The dictionary chosen
   by the compressor can be determined from the Adler32 value returned by this
   call of inflate. The compressor and decompressor must use exactly the same
   dictionary (see deflateSetDictionary).

     inflateSetDictionary returns Z_OK if success, Z_STREAM_ERROR if a
   parameter is invalid (such as NULL dictionary) or the stream state is
   inconsistent, Z_DATA_ERROR if the given dictionary doesn't match the
   expected one (incorrect Adler32 value). inflateSetDictionary does not
   perform any decompression: this will be done by subsequent calls of
   inflate().
*/

extern int  inflateSync(z_streamp strm);
/* 
    Skips invalid compressed data until the special marker (see deflate()
  above) can be found, or until all available input is skipped. No output
  is provided.

    inflateSync returns Z_OK if the special marker has been found, Z_BUF_ERROR
  if no more input was provided, Z_DATA_ERROR if no marker has been found,
  or Z_STREAM_ERROR if the stream structure was inconsistent. In the success
  case, the application may save the current current value of total_in which
  indicates where valid compressed data was found. In the error case, the
  application may repeatedly call inflateSync, providing more input each time,
  until success or end of the input data.
*/


extern int  inflateReset(z_streamp strm);
/*
     This function is equivalent to inflateEnd followed by inflateInit,
   but does not free and reallocate all the internal decompression state.
   The stream will keep attributes that may have been set by inflateInit2.

      inflateReset returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent (such as zalloc or state being NULL).
*/



                        /* utility functions */

/*
     The following utility functions are implemented on top of the
   basic stream-oriented functions. To simplify the interface, some
   default options are assumed (compression level, window size,
   standard memory allocation functions). The source code of these
   utility functions can easily be modified if you need special options.
*/

extern int  compress(Bytef *dest,   uLongf *destLen,
			       const Bytef *source, uLong sourceLen);
/*
     Compresses the source buffer into the destination buffer.  sourceLen is
   the byte length of the source buffer. Upon entry, destLen is the total
   size of the destination buffer, which must be at least 0.1% larger than
   sourceLen plus 12 bytes. Upon exit, destLen is the actual size of the
   compressed buffer.
     This function can be used to compress a whole file at once if the
   input file is mmap'ed.
     compress returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_BUF_ERROR if there was not enough room in the output
   buffer.
*/

extern int  uncompress(Bytef *dest,   uLongf *destLen,
				 const Bytef *source, uLong sourceLen);
/*
     Decompresses the source buffer into the destination buffer.  sourceLen is
   the byte length of the source buffer. Upon entry, destLen is the total
   size of the destination buffer, which must be large enough to hold the
   entire uncompressed data. (The size of the uncompressed data must have
   been saved previously by the compressor and transmitted to the decompressor
   by some mechanism outside the scope of this compression library.)
   Upon exit, destLen is the actual size of the compressed buffer.
     This function can be used to decompress a whole file at once if the
   input file is mmap'ed.

     uncompress returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_BUF_ERROR if there was not enough room in the output
   buffer, or Z_DATA_ERROR if the input data was corrupted.
*/

typedef void     *voidp;
typedef voidp gzFile;

struct IStorage;
struct IStream;

#ifdef USE_ISTREAMS
extern gzFile  gzopen(IStream *, const WCHAR *ipath, const char *path, const char *mode);
#else
extern gzFile  gzopen(const char *path, const char *mode);
#endif
/*
     Opens a gzip (.gz) file for reading or writing. The mode parameter
   is as in fopen ("rb" or "wb") but can also include a compression level
   ("wb9").  gzopen can be used to read a file which is not in gzip format;
   in this case gzread will directly read from the file without decompression.
     gzopen returns NULL if the file could not be opened or if there was
   insufficient memory to allocate the (de)compression state; errno
   can be checked to distinguish the two cases (if errno is zero, the
   zlib error is Z_MEM_ERROR).
*/


extern int     gzread(gzFile file, voidp buf, unsigned len);
/*
     Reads the given number of uncompressed bytes from the compressed file.
   If the input file was not in gzip format, gzread copies the given number
   of bytes into the buffer.
     gzread returns the number of uncompressed bytes actually read (0 for
   end of file, -1 for error). */

extern int     gzwrite(gzFile file, const voidp buf, unsigned len);
/*
     Writes the given number of uncompressed bytes into the compressed file.
   gzwrite returns the number of uncompressed bytes actually written
   (0 in case of error).
*/

extern int     gzflush(gzFile file, int flush);
/*
     Flushes all pending output into the compressed file. The parameter
   flush is as in the deflate() function. The return value is the zlib
   error number (see function gzerror below). gzflush returns Z_OK if
   the flush parameter is Z_FINISH and all output could be flushed.
     gzflush should be called only when strictly necessary because it can
   degrade compression.
*/

extern int     gzclose(gzFile file);
/*
     Flushes all pending output if necessary, closes the compressed file
   and deallocates all the (de)compression state. The return value is the zlib
   error number (see function gzerror below).
*/

extern const char *  gzerror(gzFile file, int *errnum);
/*
     Returns the error message for the last error which occurred on the
   given compressed file. errnum is set to zlib error number. If an
   error occurred in the file system and not in the compression library,
   errnum is set to Z_ERRNO and the application may consult errno
   to get the exact error code.
*/

                        /* checksum functions */
extern uLong32 crc32   (uLong32 crc, const Bytef *buf, uInt len);
/*
     Update a running crc with the bytes buf[0..len-1] and return the updated
   crc. If buf is NULL, this function returns the required initial value
   for the crc. Pre- and post-conditioning (one's complement) is performed
   within this function so it shouldn't be done by the application.
   Usage example:

     uLong crc = crc32(0L, Z_NULL, 0);

     while (read_buffer(buffer, length) != EOF) {
       crc = crc32(crc, buffer, length);
     }
     if (crc != original_crc) error();
*/


                        /* various hacks, don't look :) */

/* deflateInit and inflateInit are macros to allow checking the zlib version
 * and the compiler's view of z_stream:
 */
extern int  deflateInit_(z_streamp strm, int level,
			           const char *version, int stream_size);
extern int  inflateInit_(z_streamp strm,
				   const char *version, int stream_size);
extern int  deflateInit2_(z_streamp strm, int  level, int  method,
				    int windowBits, int memLevel, int strategy,
				    const char *version, int stream_size);
extern int  inflateInit2_(z_streamp strm, int  windowBits,
				    const char *version, int stream_size);
#define deflateInit(strm, level) \
        deflateInit_((strm), (level),       ZLIB_VERSION, sizeof(z_stream))
#define inflateInit(strm) \
        inflateInit_((strm),                ZLIB_VERSION, sizeof(z_stream))
#define deflateInit2(strm, level, method, windowBits, memLevel, strategy) \
        deflateInit2_((strm),(level),(method),(windowBits),(memLevel),\
		      (strategy),           ZLIB_VERSION, sizeof(z_stream))
#define inflateInit2(strm, windowBits) \
        inflateInit2_((strm), (windowBits), ZLIB_VERSION, sizeof(z_stream))


