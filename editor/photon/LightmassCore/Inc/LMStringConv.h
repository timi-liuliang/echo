/*=============================================================================
	LMStringConv.h: Used for converting strings between standards (ANSI, UNICODE, etc.)
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{


/** Class that handles the TCHAR to ANSI conversion */
class FTCHARToANSI_Convert
{
	/**
	 * The code page to use during conversion
	 */
	DWORD CodePage;

public:
	/**
	 * Defaults the code page to CP_ACP
	 */
	FORCEINLINE FTCHARToANSI_Convert(DWORD InCodePage = CP_ACP) :
		CodePage(InCodePage)
	{
	}

	/**
	 * Converts the string to the desired format. Allocates memory if the
	 * specified destination buffer isn't large enough
	 *
	 * @param Source The source string to convert
	 * @param Dest the destination buffer that holds the converted data
	 * @param Size the size of the dest buffer in bytes
	 */
	FORCEINLINE ANSICHAR* Convert(const TCHAR* Source,ANSICHAR* Dest,DWORD Size)
	{
		// Determine whether we need to allocate memory or not
#if _MSC_VER
		DWORD LengthW = (DWORD)lstrlenA(Source) + 1;
#else
		DWORD LengthW = (DWORD)wcslen(Source) + 1;
#endif
		// Needs to be 2x the wide in case each converted char is multibyte
		DWORD LengthA = LengthW * 2;
		if (LengthA > Size)
		{
			// Need to allocate memory because the string is too big
			Dest = new char[LengthA * sizeof(ANSICHAR)];
		}
		// Now do the conversion
#if _MSC_VER
		//WideCharToMultiByte(CodePage,0,Source,LengthW,Dest,LengthA,NULL,NULL);
#else
		for (INT C = 0; C < LengthW; C++)
		{
			Dest[C] = Source[C] & 0xFF;
		}
#endif
		return Dest;
	}
};

/** Class that converts TCHAR to OEM */
class FTCHARToOEM_Convert :
	public FTCHARToANSI_Convert
{
public:
	/**
	 * Sets the code page to OEM
	 */
	FORCEINLINE FTCHARToOEM_Convert() :
		FTCHARToANSI_Convert(CP_OEMCP)
	{
	}
};

/** Class that handles the ANSI to TCHAR conversion */
class FANSIToTCHAR_Convert
{
public:
	/**
	 * Converts the string to the desired format. Allocates memory if the
	 * specified destination buffer isn't large enough
	 *
	 * @param Source The source string to convert
	 * @param Dest the destination buffer that holds the converted data
	 * @param Size the size of the dest buffer in bytes
	 */
	FORCEINLINE TCHAR* Convert(const ANSICHAR* Source,TCHAR* Dest,DWORD Size)
	{
		// Determine whether we need to allocate memory or not
#if _MSC_VER
		DWORD Length = (DWORD)lstrlenA(Source) + 1;
#else
		DWORD Length = (DWORD)strlen(Source) + 1;
#endif
		if (Length > Size)
		{
			// Need to allocate memory because the string is too big
			Dest = new TCHAR[Length * sizeof(TCHAR)];
		}
		// Now do the conversion
#if _MSC_VER
		//MultiByteToWideChar(CP_ACP,0,Source,Length,Dest,Length);
#else
		for (INT C = 0; C < Length; C++)
		{
			Dest[C] = (BYTE)Source[C];
		}
#endif

		return Dest;
	}
};

class FTCHARToUTF8_Convert
{
public:
	FORCEINLINE FTCHARToUTF8_Convert()
	{
	}

    enum { UNICODE_BOGUS_CHAR_CODEPOINT='?' };

    // I wrote this function for originally for PhysicsFS. --ryan.
    // !!! FIXME: Maybe this shouldn't be inline...
    FORCEINLINE static void utf8fromcodepoint(DWORD cp, ANSICHAR **_dst, DWORD *_len)
    {
        char *dst = *_dst;
        INT len = *_len;

        if (len == 0)
            return;

        if (cp > 0x10FFFF)   // No Unicode codepoints above 10FFFFh, (for now!)
            cp = UNICODE_BOGUS_CHAR_CODEPOINT;
        else if ((cp == 0xFFFE) || (cp == 0xFFFF))  // illegal values.
            cp = UNICODE_BOGUS_CHAR_CODEPOINT;
        else
        {
            // There are seven "UTF-16 surrogates" that are illegal in UTF-8.
            switch (cp)
            {
                case 0xD800:
                case 0xDB7F:
                case 0xDB80:
                case 0xDBFF:
                case 0xDC00:
                case 0xDF80:
                case 0xDFFF:
                    cp = UNICODE_BOGUS_CHAR_CODEPOINT;
            }
        }

        // Do the encoding...
        if (cp < 0x80)
        {
            *(dst++) = (char) cp;
            len--;
        }

        else if (cp < 0x800)
        {
            if (len < 2)
                len = 0;
            else
            {
                *(dst++) = (char) ((cp >> 6) | 128 | 64);
                *(dst++) = (char) (cp & 0x3F) | 128;
                len -= 2;
            }
        }

        else if (cp < 0x10000)
        {
            if (len < 3)
                len = 0;
            else
            {
                *(dst++) = (char) ((cp >> 12) | 128 | 64 | 32);
                *(dst++) = (char) ((cp >> 6) & 0x3F) | 128;
                *(dst++) = (char) (cp & 0x3F) | 128;
                len -= 3;
            }
        }

        else
        {
            if (len < 4)
                len = 0;
            else
            {
                *(dst++) = (char) ((cp >> 18) | 128 | 64 | 32 | 16);
                *(dst++) = (char) ((cp >> 12) & 0x3F) | 128;
                *(dst++) = (char) ((cp >> 6) & 0x3F) | 128;
                *(dst++) = (char) (cp & 0x3F) | 128;
                len -= 4;
            }
        }

        *_dst = dst;
        *_len = len;
    }


	/**
	 * Converts the string to the desired format. Allocates memory if the
	 * specified destination buffer isn't large enough
	 *
	 * @param Source The source string to convert
	 * @param Dest the destination buffer that holds the converted data
	 * @param Size the size of the dest buffer in bytes
	 */
	FORCEINLINE ANSICHAR* Convert(const TCHAR* Source,ANSICHAR* Dest,DWORD Size)
	{
		// Determine whether we need to allocate memory or not
#if _MSC_VER
		DWORD LengthW = (DWORD)lstrlenA(Source) + 1;
#else
		DWORD LengthW = (DWORD)wcslen(Source) + 1;
#endif

		// Needs to be 4x the wide in case each converted char is multibyte
		DWORD LengthA = LengthW * 4;

		if (LengthA > Size)
		{
			// Need to allocate memory because the string is too big
			Dest = new char[LengthA * sizeof(ANSICHAR)];
		}

        ANSICHAR *retval = Dest;

		// Now do the conversion
        // You have to do this even if !UNICODE, since high-ASCII chars
        //  become multibyte. If you aren't using UNICODE and aren't using
        //  a Latin1 charset, you are just screwed, since we don't handle
        //  codepages, etc.
        while (--LengthW)
        {
            const DWORD cp = (DWORD) *(Source++);
            if (cp == 0) break;
            utf8fromcodepoint(cp, &Dest, &LengthA);
        }
        *Dest = 0;

		return retval;
	}
};


class FUTF8ToTCHAR_Convert
{
public:
	FORCEINLINE FUTF8ToTCHAR_Convert()
	{
	}

    enum { UNICODE_BOGUS_CHAR_CODEPOINT='?' };

    // I wrote this function for originally for PhysicsFS. --ryan.
    // !!! FIXME: Maybe this shouldn't be inline...
    FORCEINLINE static DWORD utf8codepoint(const ANSICHAR **_str)
    {
        const char *str = *_str;
        DWORD retval = 0;
        DWORD octet = (DWORD) ((BYTE) *str);
        DWORD octet2, octet3, octet4;

        if (octet == 0)  // null terminator, end of string.
            return 0;

        else if (octet < 128)  // one octet char: 0 to 127
        {
            (*_str)++;  // skip to next possible start of codepoint.
            return(octet);
        }

        else if ((octet > 127) && (octet < 192))  // bad (starts with 10xxxxxx).
        {
            // Apparently each of these is supposed to be flagged as a bogus
            //  char, instead of just resyncing to the next valid codepoint.
            (*_str)++;  // skip to next possible start of codepoint.
            return UNICODE_BOGUS_CHAR_CODEPOINT;
        }

        else if (octet < 224)  // two octets
        {
            octet -= (128+64);
            octet2 = (DWORD) ((BYTE) *(++str));
            if ((octet2 & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            *_str += 2;  // skip to next possible start of codepoint.
            retval = ((octet << 6) | (octet2 - 128));
            if ((retval >= 0x80) && (retval <= 0x7FF))
                return retval;
        }

        else if (octet < 240)  // three octets
        {
            octet -= (128+64+32);
            octet2 = (DWORD) ((BYTE) *(++str));
            if ((octet2 & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            octet3 = (DWORD) ((BYTE) *(++str));
            if ((octet3 & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            *_str += 3;  // skip to next possible start of codepoint.
            retval = ( ((octet << 12)) | ((octet2-128) << 6) | ((octet3-128)) );

            // There are seven "UTF-16 surrogates" that are illegal in UTF-8.
            switch (retval)
            {
                case 0xD800:
                case 0xDB7F:
                case 0xDB80:
                case 0xDBFF:
                case 0xDC00:
                case 0xDF80:
                case 0xDFFF:
                    return UNICODE_BOGUS_CHAR_CODEPOINT;
            }

            // 0xFFFE and 0xFFFF are illegal, too, so we check them at the edge.
            if ((retval >= 0x800) && (retval <= 0xFFFD))
                return retval;
        }

        else if (octet < 248)  // four octets
        {
            octet -= (128+64+32+16);
            octet2 = (DWORD) ((BYTE) *(++str));
            if ((octet2 & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            octet3 = (DWORD) ((BYTE) *(++str));
            if ((octet3 & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            octet4 = (DWORD) ((BYTE) *(++str));
            if ((octet4 & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            *_str += 4;  // skip to next possible start of codepoint.
            retval = ( ((octet << 18)) | ((octet2 - 128) << 12) |
                       ((octet3 - 128) << 6) | ((octet4 - 128)) );
            if ((retval >= 0x10000) && (retval <= 0x10FFFF))
                return retval;
        }

        // Five and six octet sequences became illegal in rfc3629.
        //  We throw the codepoint away, but parse them to make sure we move
        //  ahead the right number of bytes and don't overflow the buffer.

        else if (octet < 252)  // five octets
        {
            octet = (DWORD) ((BYTE) *(++str));
            if ((octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            octet = (DWORD) ((BYTE) *(++str));
            if ((octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            octet = (DWORD) ((BYTE) *(++str));
            if ((octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            octet = (DWORD) ((BYTE) *(++str));
            if ((octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            *_str += 5;  // skip to next possible start of codepoint.
            return UNICODE_BOGUS_CHAR_CODEPOINT;
        }

        else  // six octets
        {
            octet = (DWORD) ((BYTE) *(++str));
            if ((octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            octet = (DWORD) ((BYTE) *(++str));
            if ((octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            octet = (DWORD) ((BYTE) *(++str));
            if ((octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            octet = (DWORD) ((BYTE) *(++str));
            if ((octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            octet = (DWORD) ((BYTE) *(++str));
            if ((octet & (128+64)) != 128)  // Format isn't 10xxxxxx?
                return UNICODE_BOGUS_CHAR_CODEPOINT;

            *_str += 6;  // skip to next possible start of codepoint.
            return UNICODE_BOGUS_CHAR_CODEPOINT;
        }

        return UNICODE_BOGUS_CHAR_CODEPOINT;  // catch everything else.
    }


	/**
	 * Converts the string to the desired format. Allocates memory if the
	 * specified destination buffer isn't large enough
	 *
	 * @param Source The source string to convert
	 * @param Dest the destination buffer that holds the converted data
	 * @param Size the size of the dest buffer in bytes
	 */
	FORCEINLINE TCHAR* Convert(const ANSICHAR* Source,TCHAR* Dest,DWORD Size)
	{
		// Determine whether we need to allocate memory or not
#if _MSC_VER
		DWORD Length = (DWORD)lstrlenA(Source) + 1;
#else
		DWORD Length = (DWORD)strlen(Source) + 1;
#endif
		if (Length > Size)
		{
			// Need to allocate memory because the string is too big
			Dest = new TCHAR[Length * sizeof(TCHAR)];
		}

        TCHAR *retval = Dest;

		// Now do the conversion
        // You have to do this even if !UNICODE, since high-ASCII chars
        //  become multibyte. If you aren't using UNICODE and aren't using
        //  a Latin1 charset, you are just screwed, since we don't handle
        //  codepages, etc.
        while (--Length)
        {
            DWORD cp = utf8codepoint(&Source);
            if (cp == 0)
			{
                break;  // end of string, we're done.
			}

            // Please note that we're truncating this to a UCS-2 Windows TCHAR.
            //  A UCS-4 Unix wchar_t can hold this, and we're ignoring UTF-16 for now.
            else if (cp > 0xFFFF)
			{
                cp = UNICODE_BOGUS_CHAR_CODEPOINT;
			}

            *(Dest++) = cp;
        }
        *Dest = 0;

        return retval;
	}
};



/**
 * Class takes one type of string and converts it to another. The class includes a
 * chunk of presized memory of the destination type. If the presized array is
 * too small, it mallocs the memory needed and frees on the class going out of
 * scope. Uses the overloaded cast operator to return the converted data.
 */
template<typename CONVERT_TO,typename CONVERT_FROM,typename BASE_CONVERTER,
	DWORD DefaultConversionSize = 128>
class TStringConversion :
	public BASE_CONVERTER
{
	/**
	 * Holds the converted data if the size is large enough
	 */
	CONVERT_TO Buffer[DefaultConversionSize];
	/**
	 * Points to the converted data. If this pointer doesn't match Buffer, then
	 * memory was allocated and needs to be freed.
	 */
	CONVERT_TO* ConvertedString;

	/** Hide the default ctor */
	TStringConversion();

public:
	/**
	 * Converts the data by using the Convert() method on the base class
	 */
	explicit inline TStringConversion(const CONVERT_FROM* Source)
	{
		if (Source != NULL)
		{
			// Use base class' convert method
			ConvertedString = BASE_CONVERTER::Convert(Source,Buffer,DefaultConversionSize);
		}
		else
		{
			ConvertedString = NULL;
		}
	}

	/**
	 * If memory was allocated, then it is freed below
	 */
	inline ~TStringConversion()
	{
		if (ConvertedString != Buffer && ConvertedString != NULL)
		{
			delete [] ConvertedString;
		}
	}

	/** Operator to get access to the converted string */
	inline operator CONVERT_TO*(void) const
	{
		return ConvertedString;
	}
};


// Conversion typedefs
typedef TStringConversion<TCHAR,ANSICHAR,FANSIToTCHAR_Convert> FANSIToTCHAR;
typedef TStringConversion<ANSICHAR,TCHAR,FTCHARToANSI_Convert> FTCHARToANSI;
typedef TStringConversion<ANSICHAR,TCHAR,FTCHARToOEM_Convert> FTCHARToOEM;
typedef TStringConversion<ANSICHAR,TCHAR,FTCHARToUTF8_Convert> FTCHARToUTF8;
typedef TStringConversion<TCHAR,ANSICHAR,FUTF8ToTCHAR_Convert> FUTF8ToTCHAR;


}	// namespace Lightmass
