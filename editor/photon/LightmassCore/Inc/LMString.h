/*=============================================================================
	LMString.h: String (C and dynamic strings) functionality
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once


namespace Lightmass
{



/**
 * strcpy wrapper
 *
 * @param Dest - destination string to copy to
 * @param Destcount - size of Dest in characters
 * @param Src - source string
 * @return destination string
 */
inline TCHAR* appStrcpy( TCHAR* Dest, SIZE_T DestCount, const TCHAR* Src )
{
#if USE_SECURE_CRT
	_tcscpy_s( Dest, DestCount, Src );
	return Dest;
#else
	return (TCHAR*)_tcscpy( Dest, Src );
#endif
}

/**
 * strcpy wrapper
 * (templated version to automatically handle static destination array case)
 *
 * @param Dest - destination string to copy to
 * @param Src - source string
 * @return destination string
 */
template<SIZE_T DestCount>
inline TCHAR* appStrcpy( TCHAR (&Dest)[DestCount], const TCHAR* Src ) 
{
	return appStrcpy( Dest, DestCount, Src );
}


/**
 * strcat wrapper
 *
 * @param Dest - destination string to copy to
 * @param Destcount - size of Dest in characters
 * @param Src - source string
 * @return destination string
 */
inline TCHAR* appStrcat( TCHAR* Dest, SIZE_T DestCount, const TCHAR* Src ) 
{ 
#if USE_SECURE_CRT
	_tcscat_s( Dest, DestCount, Src );
	return Dest;
#else
	return (TCHAR*)_tcscat( Dest, Src );
#endif
}

/**
 * strcat wrapper
 * (templated version to automatically handle static destination array case)
 *
 * @param Dest - destination string to copy to
 * @param Src - source string
 * @return destination string
 */
template<SIZE_T DestCount>
inline TCHAR* appStrcat( TCHAR (&Dest)[DestCount], const TCHAR* Src ) 
{ 
	return appStrcat( Dest, DestCount, Src );
}

/**
 * strupr wrapper
 *
 * @param Dest - destination string to convert
 * @param Destcount - size of Dest in characters
 * @return destination string
 */
inline TCHAR* appStrupr( TCHAR* Dest, SIZE_T DestCount ) 
{
#if USE_SECURE_CRT
	_tcsupr_s( Dest, DestCount );
	return Dest;
#else
	return (TCHAR*)_tcsupr( Dest );
#endif
}

/**
 * strupr wrapper
 * (templated version to automatically handle static destination array case)
 *
 * @param Dest - destination string to convert
 * @return destination string
 */
template<SIZE_T DestCount>
inline TCHAR* appStrupr( TCHAR (&Dest)[DestCount] ) 
{
	return appStrupr( Dest, DestCount );
}



inline INT appStrlen( const TCHAR* String ) { return (INT)strlen( String ); }
inline TCHAR* appStrstr( const TCHAR* String, const TCHAR* Find ) { return (TCHAR*)_tcsstr( String, Find ); }
inline TCHAR* appStrchr( const TCHAR* String, WORD c ) { return (TCHAR*)_tcschr( String, c ); }
inline TCHAR* appStrrchr( const TCHAR* String, WORD c ) { return (TCHAR*)_tcsrchr( String, c ); }
inline INT appStrcmp( const TCHAR* String1, const TCHAR* String2 ) { return _tcscmp( String1, String2 ); }
inline INT appStricmp( const TCHAR* String1, const TCHAR* String2 )  { return _tcsicmp( String1, String2 ); }
inline INT appStrncmp( const TCHAR* String1, const TCHAR* String2, PTRINT Count ) { return _tcsncmp( String1, String2, Count ); }
inline INT appAtoi( const TCHAR* String ) { return _tstoi( String ); }
inline QWORD appAtoi64( const TCHAR* String ) { return _tstoi64( String ); }
inline FLOAT appAtof( const TCHAR* String ) { return _tstof( String ); }
inline DOUBLE appAtod( const TCHAR* String ) { return _tcstod( String, NULL ); }
inline INT appStrtoi( const TCHAR* Start, TCHAR** End, INT Base ) { return _tcstoul( Start, End, Base ); }
inline INT appStrnicmp( const TCHAR* A, const TCHAR* B, PTRINT Count ) { return _tcsnicmp( A, B, Count ); }


/** 
* Copy a string with length checking. Behavior differs from strncpy in that last character is zeroed. 
*
* @param Dest - destination buffer to copy to
* @param Src - source buffer to copy from
* @param MaxLen - max length of the buffer (including null-terminator)
* @return pointer to resulting string buffer
*/
TCHAR* appStrncpy( TCHAR* Dest, const TCHAR* Src, INT MaxLen );

/** 
* Concatenate a string with length checking.
*
* @param Dest - destination buffer to append to
* @param Src - source buffer to copy from
* @param MaxLen - max length of the buffer
* @return pointer to resulting string buffer
*/
TCHAR* appStrncat( TCHAR* Dest, const TCHAR* Src, INT MaxLen );

/** 
 * Finds string in string, case insensitive 
 * @param Str The string to look through
 * @param Find The string to find inside Str
 * @return Position in Str if Find was found, otherwise, NULL
 */
const TCHAR* appStristr(const TCHAR* Str, const TCHAR* Find);
TCHAR* appStristr(TCHAR* Str, const TCHAR* Find);


/*-----------------------------------------------------------------------------
	CRC functions. (MOVED FROM UNMISC.CPP FOR MASSIVE WORKAROUND @todo put back!
-----------------------------------------------------------------------------*/

/** 
* Helper class for initializing the global GCRCTable
*/
class FCRCTable
{
public:
	/**
	* Constructor
	*/
	FCRCTable()
	{
		// CRC 32 polynomial.
		const DWORD CRC32_POLY =0x04c11db7;

		// Init CRC table.
		for( DWORD iCRC=0; iCRC<256; iCRC++ )
		{
			for( DWORD c=iCRC<<24, j=8; j!=0; j-- )
			{
				CRC[iCRC] = c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
			}
		}
	}	

	DWORD CRC[256];
};

extern FCRCTable GCRCTable;

/** Case insensitive string hash function. */
inline DWORD appStrihash( const TCHAR* Data )
{
	DWORD Hash=0;
	while( *Data )
	{
		TCHAR Ch = appToUpper(*Data++);
		BYTE  B  = Ch;
		Hash     = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable.CRC[(Hash ^ B) & 0x000000FF];
		B        = Ch>>8;
		Hash     = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable.CRC[(Hash ^ B) & 0x000000FF];
	}
	return Hash;
}


inline DWORD GetTypeHash( const TCHAR* S )
{
	return appStrihash(S);
}





//
// A dynamically sizeable string.
//
class FString : protected TArray<TCHAR>
{
public:
	FString()
	: TArray<TCHAR>()
	{}
	FString( const FString& Other )
	: TArray<TCHAR>( Other.ArrayNum )
	{
		if( ArrayNum )
		{
			appMemcpy( GetData(), Other.GetData(), ArrayNum*sizeof(TCHAR) );
		}
	}
	// Allows slack to be specified
	FString( const FString& Other , INT ExtraSlack)
	{
		TArray<TCHAR>::Empty(Other.ArrayNum + ExtraSlack);
		if( Other.ArrayNum )
		{
			Add(Other.ArrayNum);
			appMemcpy( GetData(), Other.GetData(), ArrayNum*sizeof(TCHAR) );
		}
	}
	FString( const TCHAR* In )
	: TArray<TCHAR>( In && *In ? (appStrlen(In)+1) : 0 )
	{
		if( ArrayNum )
		{
			appMemcpy( GetData(), In, ArrayNum*sizeof(TCHAR) );
		}
	}
	explicit FString( INT InCount, const TCHAR* InSrc )
	:	TArray<TCHAR>( InCount ? InCount+1 : 0 )
	{
		if( ArrayNum )
		{
			appStrncpy( &(*this)(0), InSrc, InCount+1 );
		}
	}
	FString( ENoInit )
	: TArray<TCHAR>( E_NoInit )
	{}
	FString& operator=( const TCHAR* Other )
	{
		if( GetTypedData() != Other )
		{
			ArrayNum = ArrayMax = *Other ? appStrlen(Other)+1 : 0;
			AllocatorInstance.ResizeAllocation(0,ArrayMax,sizeof(TCHAR));
			if( ArrayNum )
			{
				appMemcpy( GetData(), Other, ArrayNum*sizeof(TCHAR) );
			}
		}
		return *this;
	}
	FString& operator=( const FString& Other )
	{
		if( this != &Other )
		{
			ArrayNum = ArrayMax = Other.Num();
			AllocatorInstance.ResizeAllocation(0,ArrayMax,sizeof(TCHAR));
			if( ArrayNum )
			{
				appMemcpy( GetData(), *Other, ArrayNum*sizeof(TCHAR) );
			}
		}
		return *this;
	}
    TCHAR& operator[]( INT i )
	{
		checkSlow(i>=0);
		checkSlow(i<ArrayNum);
		checkSlow(ArrayMax>=ArrayNum);
		return GetTypedData()[i];
	}
	const TCHAR& operator[]( INT i ) const
	{
		checkSlow(i>=0);
		checkSlow(i<ArrayNum);
		checkSlow(ArrayMax>=ArrayNum);
		return GetTypedData()[i];
	}
	// @dawntodo: Should we bother?
	void CheckInvariants() const
	{
		checkSlow(ArrayNum>=0);
		checkSlow(!ArrayNum || !(GetTypedData()[ArrayNum-1]));
		checkSlow(ArrayMax>=ArrayNum);
	}
	void Empty( INT Slack=0 )
	{
		TArray<TCHAR>::Empty(Slack > 0 ? Slack + 1 : Slack);
	}
	void Shrink()
	{
		TArray<TCHAR>::Shrink();
	}
	const TCHAR* operator*() const
	{
		return Num() ? &(*this)(0) : TEXT("");
	}
	TArray<TCHAR>& GetCharArray()
	{
		//warning: Operations on the TArray<CHAR> can be unsafe, such as adding
		// non-terminating 0's or removing the terminating zero.
		return (TArray<TCHAR>&)*this;
	}
	FString& operator+=( const TCHAR* Str )
	{
		checkSlow(Str);
		CheckInvariants();

		if( *Str )
		{
			INT Index = ArrayNum;
			INT NumAdd = appStrlen(Str)+1;
			INT NumCopy = NumAdd;
			if (ArrayNum)
			{
				NumAdd--;
				Index--;
			}
			Add( NumAdd );
			appMemcpy( &(*this)(Index), Str, NumCopy * sizeof(TCHAR) );
		}
		return *this;
	}
#if DAWN_STRING
	FString& operator+=(const TCHAR inChar)
	{
		checkSlow(ArrayMax>=ArrayNum);
		checkSlow(ArrayNum>=0);

		if ( inChar != '\0' )
		{
			// position to insert the character.  
			// At the end of the string if we have existing characters, otherwise at the 0 position
			INT InsertIndex = (ArrayNum > 0) ? ArrayNum-1 : 0;	

			// number of characters to add.  If we don't have any existing characters, 
			// we'll need to append the terminating zero as well.
			INT InsertCount = (ArrayNum > 0) ? 1 : 2;				

			Add(InsertCount);
			(*this)(InsertIndex) = inChar;
			(*this)(InsertIndex+1) = '\0';
		}
		return *this;
	}
#endif
	FString& operator+=( const FString& Str )
	{
		CheckInvariants();
		Str.CheckInvariants();
		if( Str.ArrayNum )
		{
			INT Index = ArrayNum;
			INT NumAdd = Str.ArrayNum;
			if (ArrayNum)
			{
				NumAdd--;
				Index--;
			}
			Add( NumAdd );
			appMemcpy( &(*this)(Index), Str.GetData(), Str.ArrayNum * sizeof(TCHAR) );
		}
		return *this;
	}
#if DAWN_STRING
	FString operator+( const TCHAR inChar ) const
	{
		CheckInvariants();
		return FString(*this, 2) += inChar; // may have an extra character of "slack"
	}
#endif
	FString operator+( const TCHAR* Str ) const
	{
		checkSlow(Str);
		CheckInvariants();

		if( *Str )
		{
			INT Index = ArrayNum;
			INT NumAdd = appStrlen(Str)+1;
			INT NumCopy = NumAdd;
			if (ArrayNum)
			{
				NumAdd--;
				Index--;
			}
			FString Ret( *this, NumAdd ) ; 
			Ret.Add( NumAdd );
			appMemcpy( &(Ret)(Index), Str, NumCopy * sizeof(TCHAR) );
			return Ret;
		}
		return *this;
	}
	FString operator+( const FString& Str ) const
	{
		CheckInvariants();
		Str.CheckInvariants();
		if( Str.ArrayNum )
		{
			INT Index = ArrayNum;
			INT NumAdd = Str.ArrayNum;
			if (ArrayNum)
			{
				NumAdd--;
				Index--;
			}
			FString ret( *this, NumAdd ) ; 
			ret.Add( NumAdd );
			appMemcpy( &(ret)(Index), Str.GetData(), Str.ArrayNum * sizeof(TCHAR) );
			return ret;
		}
		return *this;
	}
#if DAWN_STRING
	FString& operator*=( const TCHAR* Str )
	{
		if( ArrayNum>1 && (*this)(ArrayNum-2)!=PATH_SEPARATOR[0] )
			*this += PATH_SEPARATOR;
		return *this += Str;
	}
	FString& operator*=( const FString& Str )
	{
		return operator*=( *Str );
	}
	FString operator*( const TCHAR* Str ) const
	{
		return FString( *this ) *= Str;
	}
	FString operator*( const FString& Str ) const
	{
		return operator*( *Str );
	}
	UBOOL operator<=( const TCHAR* Other ) const
	{
		return !(appStricmp( **this, Other ) > 0);
	}
	UBOOL operator<( const TCHAR* Other ) const
	{
		return appStricmp( **this, Other ) < 0;
	}
	UBOOL operator<( const FString& Other ) const
	{
		return appStricmp( **this, *Other ) < 0;
	}
	UBOOL operator>=( const TCHAR* Other ) const
	{
		return !(appStricmp( **this, Other ) < 0);
	}
	UBOOL operator>( const TCHAR* Other ) const
	{
		return appStricmp( **this, Other ) > 0;
	}
	UBOOL operator>( const FString& Other ) const
	{
		return appStricmp( **this, *Other ) > 0;
	}
#endif
	UBOOL operator==( const TCHAR* Other ) const
	{
		return appStricmp( **this, Other )==0;
	}
	UBOOL operator==( const FString& Other ) const
	{
		return appStricmp( **this, *Other )==0;
	}
	UBOOL operator!=( const TCHAR* Other ) const
	{
		return appStricmp( **this, Other )!=0;
	}
	UBOOL operator!=( const FString& Other ) const
	{
		return appStricmp( **this, *Other )!=0;
	}
#if DAWN_STRING
	UBOOL operator<=(const FString& Str) const
	{
		return !(appStricmp(**this, *Str) > 0);
	}
	UBOOL operator>=(const FString& Str) const
	{
		return !(appStricmp(**this, *Str) < 0);
	}
#endif
	INT Len() const
	{
		return Num() ? Num()-1 : 0;
	}
	FString Left( INT Count ) const
	{
		return FString( Clamp(Count,0,Len()), **this );
	}
	FString LeftChop( INT Count ) const
	{
		return FString( Clamp(Len()-Count,0,Len()), **this );
	}
	FString Right( INT Count ) const
	{
		return FString( **this + Len()-Clamp(Count,0,Len()) );
	}
	FString Mid( INT Start, INT Count=MAXINT ) const
	{
		DWORD End = Start+Count;
		Start    = Clamp( (DWORD)Start, (DWORD)0,     (DWORD)Len() );
		End      = Clamp( (DWORD)End,   (DWORD)Start, (DWORD)Len() );
		return FString( End-Start, **this + Start );
	}

	//@{
	/**
	 * Searches the string for a substring, and returns index into this string
	 * of the first found instance. Can search from beginning or end, and ignore case or not.
	 *
	 * @param SubStr The string to search for
	 * @param bSearchFromEnd If TRUE, the search will start at the end of the string and go backwards
	 * @param bIgnoreCase If TRUE, the search will be case insensitive
	 */
	INT InStr( const TCHAR* SubStr, UBOOL bSearchFromEnd=FALSE, UBOOL bIgnoreCase=FALSE, INT StartPosition=INDEX_NONE ) const
	{
		if ( SubStr == NULL )
		{
			return INDEX_NONE;
		}
		if( !bSearchFromEnd )
		{
			const TCHAR* Start = **this;
			if ( StartPosition != INDEX_NONE )
			{
				Start += Clamp(StartPosition, 0, Len() - 1);
			}
			const TCHAR* Tmp = bIgnoreCase 
				? appStristr(Start, SubStr)
				: appStrstr(Start, SubStr);

			return Tmp ? (Tmp-**this) : -1;
		}
		else
		{
			// if ignoring, do a onetime ToUpper on both strings, to avoid ToUppering multiple
			// times in the loop below
			if (bIgnoreCase)
			{
				return ToUpper().InStr(FString(SubStr).ToUpper(), TRUE, FALSE, StartPosition);
			}
			else
			{
				const INT SearchStringLength=Max(1, appStrlen(SubStr));
				if ( StartPosition == INDEX_NONE )
				{
					StartPosition = Len();
				}
				for( INT i = StartPosition - SearchStringLength; i >= 0; i-- )
				{
					INT j;
					for( j=0; SubStr[j]; j++ )
					{
						if( (*this)(i+j)!=SubStr[j] )
						{
							break;
						}
					}
					if( !SubStr[j] )
					{
						return i;
					}
				}
				return -1;
			}
		}
	}
	INT InStr( const FString& SubStr, UBOOL bSearchFromEnd=FALSE, UBOOL bIgnoreCase=FALSE, INT StartPosition=INDEX_NONE ) const
	{
		return InStr( *SubStr, bSearchFromEnd, bIgnoreCase, StartPosition );
	}	//@}

	UBOOL Split( const FString& InS, FString* LeftS, FString* RightS, UBOOL InRight=0 ) const
	{
		INT InPos = InStr(InS,InRight);
		if( InPos<0 )
			return 0;
		if( LeftS )
			*LeftS = Left(InPos);
		if( RightS )
			*RightS = Mid(InPos+InS.Len());
		return 1;
	}

	FString ToUpper() const
	{
		FString New( **this );
		for( INT i=0; i< New.ArrayNum; i++ )
			New(i) = appToUpper(New(i));
		return New;
	}
#if DAWN_STRING
	FString ToLower() const
	{
		FString New( **this );
		for( INT i=0; i<New.ArrayNum; i++ )
			New(i) = appToLower(New(i));
		return New;
	}
	FString LeftPad( INT ChCount ) const;
	FString RightPad( INT ChCount ) const;
	
	UBOOL IsNumeric() const;
#endif

	VARARG_DECL( static FString, static FString, return, Printf, VARARG_NONE, const TCHAR*, VARARG_NONE, VARARG_NONE );

#if DAWN_STRING
	static FString Chr( TCHAR Ch );


	/**
	 * Returns TRUE if this string begins with the specified text. (case-insensitive)
	 */
	UBOOL StartsWith(const FString& InPrefix ) const
	{
		return InPrefix.Len() > 0 && !appStrnicmp(**this, *InPrefix, InPrefix.Len());
	}

	/**
	 * Returns TRUE if this string ends with the specified text. (case-insensitive)
	 */
	UBOOL EndsWith(const FString& InSuffix ) const
	{
		return InSuffix.Len() > 0 &&
			   Len() >= InSuffix.Len() &&
			   !appStricmp( &(*this)( Len() - InSuffix.Len() ), *InSuffix );
	}

	/**
	 * Removes whitespace characters from the front of this string.
	 */
	FString Trim()
	{
		INT Pos = 0;
		while(Pos < Len())
		{
			if( appIsWhitespace( (*this)[Pos] ) )
			{
				Pos++;
			}
			else
			{
				break;
			}
		}

		*this = Right( Len()-Pos );

		return *this;
	}

	/**
	 * Removes trailing whitespace characters
	 */
	FString TrimTrailing( void )
	{
		INT Pos = Len() - 1;
		while( Pos >= 0 )
		{
			if( !appIsWhitespace( ( *this )[Pos] ) )
			{
				break;
			}

			Pos--;
		}

		*this = Left( Pos + 1 );

		return( *this );
	}

	/**
	 * Returns a copy of this string with wrapping quotation marks removed.
	 */
	FString TrimQuotes( UBOOL* bQuotesRemoved=NULL ) const
	{
		UBOOL bQuotesWereRemoved=FALSE;
		INT Start = 0, Count = Len();
		if ( Count > 0 )
		{
			if ( (*this)[0] == TCHAR('"') )
			{
				Start++;
				Count--;
				bQuotesWereRemoved=TRUE;
			}

			if ( Len() > 1 && (*this)[Len() - 1] == TCHAR('"') )
			{
				Count--;
				bQuotesWereRemoved=TRUE;
			}
		}

		if ( bQuotesRemoved != NULL )
		{
			*bQuotesRemoved = bQuotesWereRemoved;
		}
		return Mid(Start, Count);
	}

	/**
	 * Breaks up a delimited string into elements of a string array.
	 *
	 * @param	InArray		The array to fill with the string pieces
	 * @param	pchDelim	The string to delimit on
	 * @param	InCullEmpty	If 1, empty strings are not added to the array
	 *
	 * @return	The number of elements in InArray
	 */
	INT ParseIntoArray( TArray<FString>* InArray, const TCHAR* pchDelim, UBOOL InCullEmpty ) const;

	/**
	 * Breaks up a delimited string into elements of a string array, using any whitespace and an 
	 * optional extra delimter, like a ","
	 *
	 * @param	InArray			The array to fill with the string pieces
	 * @param	pchExtraDelim	The string to delimit on
	 *
	 * @return	The number of elements in InArray
	 */
	INT ParseIntoArrayWS( TArray<FString>* InArray, const TCHAR* pchExtraDelim = NULL ) const;

	/**
	 * Takes an array of strings and removes any zero length entries.
	 *
	 * @param	InArray	The array to cull
	 *
	 * @return	The number of elements left in InArray
	 */
	static INT CullArray( TArray<FString>* InArray )
	{
		check(InArray);
		for( INT x=InArray->Num()-1; x>=0; x-- )
		{
			if( (*InArray)(x).Len() == 0 )
			{
				InArray->Remove( x );
			}
		}
		return InArray->Num();
	}
	/**
	 * Returns a copy of this string, with the characters in reverse order
	 */
	FString Reverse() const
	{
		FString New;
		for( int x = Len()-1 ; x > -1 ; x-- )
			New += Mid(x,1);
		return New;
	}

	/**
	 * Reverses the order of characters in this string
	 */
	void ReverseString()
	{
		if ( Len() > 0 )
		{
			TCHAR* StartChar = &(*this)(0);
			TCHAR* EndChar = &(*this)(Len()-1);
			TCHAR TempChar;
			do 
			{
				TempChar = *StartChar;	// store the current value of StartChar
				*StartChar = *EndChar;	// change the value of StartChar to the value of EndChar
				*EndChar = TempChar;	// change the value of EndChar to the character that was previously at StartChar

				StartChar++;
				EndChar--;

			} while( StartChar < EndChar );	// repeat until we've reached the midpoint of the string
		}
	}
#endif
	// Replace all occurrences of a substring
	FString Replace(const TCHAR* From, const TCHAR* To, UBOOL bIgnoreCase=FALSE) const
	{
		if (Len() == 0)
		{
			return *this;
		}

		FString Result;

		// get a pointer into the character data
		TCHAR* Travel = (TCHAR*)GetData();

		// precalc the length of the From string
		INT FromLength = appStrlen(From);

		while (TRUE)
		{
			// look for From in the remaining string
			TCHAR* FromLocation = bIgnoreCase ? appStristr(Travel, From) : appStrstr(Travel, From);
			if (FromLocation)
			{
				// replace the first letter of the From with 0 so we can do a strcpy (FString +=)
				TCHAR C = *FromLocation;
				*FromLocation = 0;
				
				// copy everything up to the From
				Result += Travel;

				// copy over the To
				Result += To;

				// retore the letter, just so we don't have 0's in the string
				*FromLocation = *From;

				Travel = FromLocation + FromLength;
			}
			else
			{
				break;
			}
		}

		// copy anything left over
		Result += Travel;

		return Result;	
	}

#if LM_STRING
	/**
	 * Replace all occurrences of SearchText with ReplacementText in this string.
	 *
	 * @param	SearchText	the text that should be removed from this string
	 * @param	ReplacementText		the text to insert in its place
	 *
	 * @return	the number of occurrences of SearchText that were replaced.
	 */
	INT ReplaceInline( const TCHAR* SearchText, const TCHAR* ReplacementText );

	/**
	 * Returns a copy of this string with all quote marks escaped (unless the quote is already escaped)
	 */
	FString ReplaceQuotesWithEscapedQuotes() const;

	/**
	 * Replaces certain characters with the "escaped" version of that character (i.e. replaces "\n" with "\\n").
	 * The characters supported are: { \n, \r, \t, \', \", \\ }.
	 *
	 * @param	Chars	by default, replaces all supported characters; this parameter allows you to limit the replacement to a subset.
	 *
	 * @return	a string with all control characters replaced by the escaped version.
	 */
	FString ReplaceCharWithEscapedChar( const TArray<TCHAR>* Chars=NULL ) const;
	/**
	 * Removes the escape backslash for all supported characters, replacing the escape and character with the non-escaped version.  (i.e.
	 * replaces "\\n" with "\n".  Counterpart to ReplaceCharWithEscapedChar().
	 */
	FString ReplaceEscapedCharWithChar( const TArray<TCHAR>* Chars=NULL ) const;

	// Takes the number passed in and formats the string in comma format ( 12345 becomes "12,345")
	static FString FormatAsNumber( INT InNumber )
	{
		FString Number = appItoa( InNumber ), Result;

		int dec = 0;
		for( int x = Number.Len()-1 ; x > -1 ; --x )
		{
			Result += Number.Mid(x,1);

			dec++;
			if( dec == 3 && x > 0 )
			{
				Result += TEXT(",");
				dec = 0;
			}
		}

		return Result.Reverse();
	}
#endif
};

/**
 * Returns the file path in front of the filename, including the trailing path separator (e.g. '\').
 */
FString appGetFilePath( FString& FilePath );

/**
 * Returns the filename, including file extension.
 */
FString appGetCleanFilename( FString& FilePath );

/**
 * Returns the filename, not including file extension or the period.
 */
FString appGetBaseFilename( FString& FilePath, UBOOL bRemovePath=TRUE );


inline DWORD GetTypeHash( const class FString& S )
{
	return appStrihash(*S);
}

/**
 * Returns a string with a unique timestamp (useful for creating log filenames)
 */
FString appSystemTimeString();

//
// Convert an integer to a string.
//
// Faster Itoa that also appends to a string
void appItoaAppend( INT InNum,FString &NumberString );

FString appItoa( INT InNum );

/**
 * Copy text to clipboard.
 */
void appClipboardCopy( const TCHAR* Str );

/**
 * Paste text from clipboard into an FString.
 * @return	FString containing the text from the clipboard
 */
FString appClipboardPaste();

/**
 * Returns a pretty-string for a time given in seconds. (I.e. "4:31 min", "2:16:30 hours", etc)
 * @param Seconds	Time in seconds
 * @return			Time in a pretty formatted string
 */
FString appPrettyTime( DOUBLE Seconds );

/**
 * strcpy wrapper (ANSI version)
 *
 * @param Dest - destination string to copy to
 * @param Destcount - size of Dest in characters
 * @param Src - source string
 * @return destination string
 */
inline ANSICHAR* appStrcpyANSI( ANSICHAR* Dest, SIZE_T DestCount, const ANSICHAR* Src ) 
{ 
#if USE_SECURE_CRT
	strcpy_s( Dest, DestCount, Src );
	return Dest;
#else
	return (ANSICHAR*)strcpy( Dest, Src );
#endif
}

/**
* strcpy wrapper (ANSI version)
* (templated version to automatically handle static destination array case)
*
* @param Dest - destination string to copy to
* @param Src - source string
* @return destination string
*/
template<SIZE_T DestCount>
inline ANSICHAR* appStrcpyANSI( ANSICHAR (&Dest)[DestCount], const ANSICHAR* Src ) 
{ 
	return appStrcpyANSI( Dest, DestCount, Src );
}

/**
* strcat wrapper (ANSI version)
*
* @param Dest - destination string to copy to
* @param Destcount - size of Dest in characters
* @param Src - source string
* @return destination string
*/
inline ANSICHAR* appStrcatANSI( ANSICHAR* Dest, SIZE_T DestCount, const ANSICHAR* Src ) 
{ 
#if USE_SECURE_CRT
	strcat_s( Dest, DestCount, Src );
	return Dest;
#else
	return (ANSICHAR*)strcat( Dest, Src );
#endif
}

/**
* strcat wrapper (ANSI version)
*
* @param Dest - destination string to copy to
* @param Destcount - size of Dest in characters
* @param Src - source string
* @return destination string
*/
template<SIZE_T DestCount>
inline ANSICHAR* appStrcatANSI( ANSICHAR (&Dest)[DestCount], const ANSICHAR* Src ) 
{ 
	return appStrcatANSI( Dest, DestCount, Src );
}

}
