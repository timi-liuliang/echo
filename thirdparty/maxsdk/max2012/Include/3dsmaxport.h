/**********************************************************************
 
   FILE:           3dsmaxport.h

   DESCRIPTION:    3ds max portability layer.

   CREATED BY:     Stephane Rouleau, Discreet

   HISTORY:        Created November 12th, 2004

 *>   Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "utilexp.h"
#include "strbasic.h"
#include "BuildWarnings.h"
#include <wtypes.h>
#include <cmath>                    // fabs, abs.
#include <cstdlib>

/*
   Set/GetWindowLong() should never be used to store/retrieve pointers in an HWND, as
   they end up truncating 64 bit pointers in Win64.

   GetWindowLongPtr() has been introduced by MS, but they still involve a lot of 
   casting to properly remove warnings in Win32 and Win64.  The following inline
   functions are typesafe and completely inlined, to they incure no runtime hit.

   Instead of:

   IMyDataType* pData = (IMyDataType*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

   you should instead do:

   IMyDataType* pData = DLGetWindowLongPtr<IMyDataType*>(hWnd);

   or
   
   IMyDataType* pData;
   DLGetWindowLongPtr(hWnd, &pData);

   Similarly, instead of

   SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pData);

   do

   DLSetWindowLongPtr(hWnd, pData);
*/

template<typename DataPtr> DataPtr DLGetWindowLongPtr(HWND hWnd, int n = GWLP_USERDATA, DataPtr = NULL)
{
   return (DataPtr)(static_cast<LONG_PTR>(::GetWindowLongPtr(hWnd, n)));
}

template<typename DataPtr> DataPtr DLGetWindowLongPtr(HWND hWnd, DataPtr* pPtr, int n = GWLP_USERDATA)
{
   *pPtr = DLGetWindowLongPtr<DataPtr>(hWnd, n);

   return *pPtr;
}

template<> inline bool DLGetWindowLongPtr(HWND hWnd, int n, bool)
{
   // Specialize for bool so we can quietly deal 
   // warning C4800: 'LONG_PTR' : forcing value to bool 'true' or 'false' (performance warning)
   return ::GetWindowLongPtr(hWnd, n) != 0;
}

inline WNDPROC DLGetWindowProc(HWND hWnd)
{
   return DLGetWindowLongPtr<WNDPROC>(hWnd, GWLP_WNDPROC);
}

inline HINSTANCE DLGetWindowInstance(HWND hWnd)
{
   return DLGetWindowLongPtr<HINSTANCE>(hWnd, GWLP_HINSTANCE);
}

// Type can be either pointer-sized, or not.  So we'd need either static_cast<> or
// reinterpret_cast<>.  It's simpler here to force using a C cast than going through
// a tag-based solution, such as used by std::distance to pick between random-iterator
// and forward-iterator types.
template<typename Type> inline Type DLSetWindowLongPtr(HWND hWnd, Type ptr, int n = GWLP_USERDATA)
{
#if !defined( _WIN64 )
   // SetWindowLongPtr() maps to SetWindowLong() in 32 bit land; react accordingly to keep
   // the compiler happy, even with /Wp64.
   return (Type)(static_cast<LONG_PTR>(::SetWindowLongPtr(hWnd, n, (LONG)((LONG_PTR)(ptr)))));
#else
   return (Type)(static_cast<LONG_PTR>(::SetWindowLongPtr(hWnd, n, (LONG_PTR)(ptr))));
#endif
}

template<> inline bool DLSetWindowLongPtr(HWND hWnd, bool bo, int n)
{
   // Specialize for bool so we can quietly deal 
   // warning C4800: 'LONG_PTR' : forcing value to bool 'true' or 'false' (performance warning)
   return ::SetWindowLongPtr(hWnd, n, bo) != 0;
}

inline WNDPROC DLSetWindowLongPtr(HWND hWnd, WNDPROC pfn)
{
   return DLSetWindowLongPtr<WNDPROC>(hWnd, pfn, GWLP_WNDPROC);
}

inline WNDPROC DLSetWindowProc(HWND hWnd, WNDPROC pfn)
{
   return DLSetWindowLongPtr<WNDPROC>(hWnd, pfn, GWLP_WNDPROC);
}

#if !defined( _WIN64 )
typedef INT_PTR (CALLBACK* DL_NOTQUITE_WNDPROC)(HWND, UINT, WPARAM, LPARAM);
#endif

/*
   same as above, but for class instead of HWND.
*/

template<typename DataPtr> DataPtr DLGetClassLongPtr(HWND hWnd, int n, DataPtr = NULL)
{
   return (DataPtr)(static_cast<LONG_PTR>(::GetClassLongPtr(hWnd, n)));
}

template<typename DataPtr> DataPtr DLGetClassLongPtr(HWND hWnd, DataPtr* pPtr, int n)
{
   *pPtr = DLGetClassLongPtr<DataPtr>(hWnd, n);

   return *pPtr;
}

template<> inline bool DLGetClassLongPtr(HWND hWnd, int n, bool)
{
   return ::GetClassLongPtr(hWnd, n) != 0;
}

inline WNDPROC DLGetClassWindowProc(HWND hWnd)
{
   return DLGetClassLongPtr<WNDPROC>(hWnd, GCLP_WNDPROC);
}

template<typename Type> inline Type DLSetClassLongPtr(HWND hWnd, Type ptr, int n)
{
#if !defined( _WIN64 )
   // SetClassLongPtr() maps to SetClassLong() in 32 bit land; react accordingly to keep
   // the compiler happy, even with /Wp64.
   return (Type)(static_cast<LONG_PTR>(::SetClassLongPtr(hWnd, n, (LONG)((LONG_PTR)(ptr)))));
#else
   return (Type)(static_cast<LONG_PTR>(::SetClassLongPtr(hWnd, n, (LONG_PTR)(ptr))));
#endif
}

template<> inline bool DLSetClassLongPtr(HWND hWnd, bool bo, int n)
{
   // Specialize for bool so we can quietly deal 
   // warning C4800: 'LONG_PTR' : forcing value to bool 'true' or 'false' (performance warning)
   return ::SetClassLongPtr(hWnd, n, bo) != 0;
}

/*
   RegSet/GetValueEx() does not handle buffers larger than 4G, even in Win64.  Not
   really an issue since who would want to store this much data in the registry
   anyway.

   Unfortunately we often use it to get/set strings, and the length is often obtained
   from _tcslen(), which returns a size_t and would therefore require casting, not
   to mention that the registry functions deal in _bytes_, not _chars_ so this has
   to be taken into account as well.

   These helper functions shield you from the nitty-gritty of casts, and reduce
   the number of parameters you have to use.  Again, no runtime hit because it's all
   inlined.

   This method of having explicit function names instead of one DLRegSetValueEx() 
   overloaded for every data type we awnt was prefered to prevent automatic typecasting
   occuring behind your back.

   Instead of (which, by the way, is off by one byte when compiled for UNICODE, but 
   this was lifted from the codebase and is another reason why using our replacement 
   functions is a Good Thing):

   RegSetValueEx(key2, NULL, 0, REG_SZ, (LPBYTE)buf, _tcslen(buf)*sizeof(MCHAR)+1);

   you should instead do:

   DLRegSetString(key2, buf);
*/

UtilExport LONG DLRegSetString(HKEY key, const MCHAR* lpValue, const MCHAR* lpValueName = NULL);
UtilExport LONG DLRegSetDWord(HKEY key, DWORD dw, const MCHAR* lpValueName = NULL);

template <typename BlobType> inline LONG DLRegSetBlob(HKEY key, const BlobType* lpbValue, DWORD n, 
                         LPCMSTR lpValueName = NULL)
{
   return ::RegSetValueEx(key, lpValueName, 0, REG_BINARY, 
                          reinterpret_cast<LPBYTE>(const_cast<BlobType*>(lpbValue)), 
                          n * sizeof(BlobType));
}

/*
   GetTextExtentPoint32 works with strings up to 2G, even in Win64.  Not a problem,
   but since the length is often obtained from _tcslen(), a lot of casting ensues.
*/
UtilExport BOOL DLGetTextExtent(HDC hDC, const MCHAR* text, LPSIZE lpExtent);
UtilExport BOOL DLGetTextExtent(HDC hDC, const MCHAR* text, size_t len, LPSIZE lpExtent);

/*
   Same with TextOut, TabbedTextOut, DrawText
*/
UtilExport BOOL DLTextOut(HDC hDC, int x, int y, const MCHAR* text);
UtilExport BOOL DLTextOut(HDC hDC, int x, int y, const MCHAR* text, size_t len);
UtilExport LONG DLTabbedTextOut(HDC hDC, int x, int y, const MCHAR* text, int nTabs, 
                            const INT* pTabPositions, int nTabOrigin);

UtilExport int DLDrawText(HDC hDC, MCHAR* lpString, LPRECT lpRect, UINT uFormat);
UtilExport int DLDrawText(HDC hDC, MCHAR* lpString, size_t len, LPRECT lpRect, UINT uFormat);

/*
   Extract hi/low part of pointer-sized chunks, unlike LOWORD/HIWORD which always return
   WORD-sized chunks.

   Notice that the return value changes depending on your platform.
*/

#if defined( _WIN64 )

inline DWORD PointerHiPart(LONG_PTR ptr)
{
   return static_cast<DWORD>((ptr >> 32) & 0xffffffff);
}

inline DWORD PointerLoPart(LONG_PTR ptr)
{
   return static_cast<DWORD>(ptr & 0xffffffff);
}

inline DWORD PointerHiPart(ULONG_PTR ptr)
{
   return static_cast<DWORD>(ptr >> 32);
}

inline DWORD PointerLoPart(ULONG_PTR ptr)
{
   return static_cast<DWORD>(ptr);
}

inline DWORD PointerHiPart(void* ptr)
{
   return PointerHiPart(reinterpret_cast<ULONG_PTR>(ptr));
}

inline DWORD PointerLoPart(void* ptr)
{
   return PointerLoPart(reinterpret_cast<ULONG_PTR>(ptr));
}

#else

inline WORD PointerHiPart(ULONG_PTR ptr)
{
   return static_cast<WORD>(ptr >> 16);
}

inline WORD PointerLoPart(ULONG_PTR ptr)
{
   return static_cast<WORD>(ptr);
}

inline WORD PointerHiPart(LONG_PTR ptr)
{
   return static_cast<WORD>((ptr >> 16) & 0xffff);
}

inline WORD PointerLoPart(LONG_PTR ptr)
{
   return static_cast<WORD>(ptr & 0xffff);
}

inline WORD PointerHiPart(void* ptr)
{
   return PointerHiPart(reinterpret_cast<ULONG_PTR>(ptr));
}

inline WORD PointerLoPart(void* ptr)
{
   return PointerLoPart(reinterpret_cast<ULONG_PTR>(ptr));
}

#endif

/*
   Fowler/Noll/Vo hashing algorithm, taken from http://www.isthe.com/chongo/tech/comp/fnv/.

   We hash to 32 bits values, even for 64 bit pointers, since it would make little sense
   to have such a huge hash table...

 From http://www.isthe.com/chongo/src/fnv/hash_32.c:

 ***
 *
 * Fowler/Noll/Vo hash
 *
 * The basis of this hash algorithm was taken from an idea sent
 * as reviewer comments to the IEEE POSIX P1003.2 committee by:
 *
 *      Phong Vo (http://www.research.att.com/info/kpv/)
 *      Glenn Fowler (http://www.research.att.com/~gsf/)
 *
 * In a subsequent ballot round:
 *
 *      Landon Curt Noll (http://www.isthe.com/chongo/)
 *
 * improved on their algorithm.  Some people tried this hash
 * and found that it worked rather well.  In an EMail message
 * to Landon, they named it the ``Fowler/Noll/Vo'' or FNV hash.
 *
 * FNV hashes are designed to be fast while maintaining a low
 * collision rate. The FNV speed allows one to quickly hash lots
 * of data while maintaining a reasonable collision rate.  See:
 *
 *      http://www.isthe.com/chongo/tech/comp/fnv/index.html
 *
 * for more details as well as other forms of the FNV hash.
 ***
 *
 * NOTE: The FNV-0 historic hash is not recommended.  One should use
 *  the FNV-1 hash instead.
 *
 * To use the 32 bit FNV-0 historic hash, pass FNV0_32_INIT as the
 * Fnv32_t hashval argument to fnv_32_buf() or fnv_32_str().
 *
 * To use the recommended 32 bit FNV-1 hash, pass FNV1_32_INIT as the
 * Fnv32_t hashval argument to fnv_32_buf() or fnv_32_str().
 *
 ***
 *
 * Please do not copyright this code.  This code is in the public domain.
   
*/

enum
{
   FNV1_32_INIT = 0x811c9dc5,
   FNV_32_PRIME = 0x01000193
};


/*
 * fnv_32_buf - perform a 32 bit Fowler/Noll/Vo hash on a buffer
 *
 * input:
 * buf   - start of buffer to hash
 * len   - length of buffer in octets
 * hval  - previous hash value or 0 if first call
 *
 * returns:
 * 32 bit hash as a static hash type
 *
 * NOTE: To use the 32 bit FNV-0 historic hash, use FNV0_32_INIT as the hval
 *  argument on the first call to either fnv_32_buf() or fnv_32_str().
 *
 * NOTE: To use the recommended 32 bit FNV-1 hash, use FNV1_32_INIT as the hval
 *  argument on the first call to either fnv_32_buf() or fnv_32_str().
 */
inline unsigned int fnv_32_buf(const void *buf, int len, unsigned int hval)
{
   const unsigned char *bp = (const unsigned char *)buf; /* start of buffer */
   const unsigned char *be = bp + len;                   /* beyond end of buffer */

   /*
    * FNV-1 hash each octet in the buffer
    */
   while (bp < be) 
   {
      /* multiply by the 32 bit FNV magic prime mod 2^32 */
      hval *= FNV_32_PRIME;

      /* xor the bottom with the current octet */
      hval ^= (unsigned int)*bp++;
   }

   /* return our new hash value */
   return hval;
}


template <typename T> inline unsigned int DLObjectHash(T& t, int HashTableSize = 0)
{
   // Since we're always hashing single objects, we hash using the 'init value';
   // we'd have to carry it over if we were hashing all fields in a structure, for
   // instance.
   // You should never never never never attempt to hash a structure by passing its
   // pointer, since you'd end up hashing the padding bytes as well, which are definitely
   // not going to be preserved.
   unsigned int hash = fnv_32_buf(&t, sizeof(t), (unsigned int)FNV1_32_INIT);

   return HashTableSize ? hash % HashTableSize : hash;
}

inline unsigned int DLPointerHash(const void* pv, int HashTableSize = 0)
{
   return DLObjectHash(pv, HashTableSize);
}

/*
   abs() when you're using _PTR values; in Win64, abs() will complain -- you should be
   using abs64.

   Also, with 2005 fabs(int) generates a warning.
*/

inline int DL_abs(int n) { return std::abs(n); }
inline __int64 DL_abs(__int64 n) { return (n < 0) ? -n : n; }

inline float DL_abs(float f)     { return std::fabsf(f); }
inline double DL_abs(double f)   { return std::fabs(f); }
inline long double DL_abs(long double f) { return std::fabsl(f); }

/*
   hash_set / hash_map changed namespace with VS 2005
*/
#define DL_STDEXT        stdext

#define DL_NON_RTTI_OBJECT  std::__non_rtti_object


// With newer, C++-compliant versions, you need to specify that a type is a typename
// in situations such as: template <typename T> void foo(typename T::SomeType);
#ifndef DL_TYPENAME_OUTSIDE_TEMPLATE
	#define DL_TYPENAME_OUTSIDE_TEMPLATE  typename
#endif

