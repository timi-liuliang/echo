#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include <ctype.h>

namespace Echo
{
	class Vector2;
	class Vector3;
	class Vector4;
	class Quaternion;
	namespace StringUtil
	{
		static const String		BLANK = String(); 

		String			Replace(const String& str, const String& src, const String& dst);
		String			Replace(const String& str, char src, char dst);
		bool			ReplaceRet(String& str, const String& src, const String& dst);
		void			Trim(String& str, bool bLeft = true, bool bRight = true);
		String			Substr(const String& str, const String& delims=",", bool isClockWise=true);
		StringArray		Split(const String& str, const String& delims = ", ", Dword maxSplits = 0);
		void			SplitFileName(const String& qualifiedName, String& outBasename, String& outPath);
		void			LowerCase(String& str);
		void			UpperCase(String& str);
		bool			IsHaveUpper(const String& str);
		bool			IsHaveLower(const String& str);
		String			Format(const char* formats, ...);
		bool			StartWith(const String& str, const String& pattern, bool lowCase = false);
		bool			EndWith(const String& str, const String& pattern, bool bCaseSensitive = true);
		bool			Equal(const String& str1, const String& str2, bool bCaseSensitive = true);
		bool			Contain(const String& str, const String& pattern, bool bCaseSensitive = true);

		String			RemoveLast(const String& str, const String& subStr, bool isCaseSensitive = true);

		String			WCS2MBS(const WString& str);
		WString			MBS2WCS(const String& str);

		String			ToString(float val, Word precision = 5, Word width = 0, char fill = ' ');
		String			ToString(double val, Word precision = 5, Word width = 0, char fill = ' ');
		String			ToString(i32 val, Word width = 0, char fill = ' ');
		String			ToString(i64 val, Word width = 0, char fill = ' ');
		String			ToString(ui32 val, Word width = 0, char fill = ' ');
		String			ToString(ui64 val, Word width = 0, char fill = ' ');

		String			ToString(bool val, bool bYesNo = false);
		String			ToString(const vector<double>::type& val);
		String			ToString(const Vector2& val);
		String			ToString(const vector<Vector2>::type& val);
		String			ToString(const Vector3& val);
		String			ToString(const vector<Vector3>::type& val);
		String			ToString(const Vector4& val);
		String			ToString(const Quaternion& val);
		String			ToString(const StringArray& array, const char* delims=",");

		float			ParseFloat(const String& val, float defVal = 0.0f);
		double			ParseDouble(const String& val, double defVal = 0.0);
		Real			ParseReal(const String& val, Real defVal = (Real)0.0);
		vector<Real>::type ParseRealVector(const String& val, const vector<Real>::type& defVal = vector<Real>::type());
		int				ParseInt(const String& val, int defVal = 0);
		long			ParseLong(const String& val, long defVal = 0);
		bool			ParseBool(const String& val, bool defVal = false);
		Word			ParseWord(const String& val, Word defVal = 0xffff);
		Dword			ParseDword(const String& val, Dword defVal = 0xffffffff);
		i8				ParseI8(const String& val, i8 defVal = 0);
		i16				ParseI16(const String& val, i16 defVal = 0);
		i32				ParseI32(const String& val, i32 defVal = 0);
		i64				ParseI64(const String& val, i64 defVal = 0);
		ui8				ParseUI8(const String& val, ui8 defVal = 0);
		ui16			ParseUI16(const String& val, ui16 defVal = 0);
		ui32			ParseUI32(const String& val, ui32 defVal = 0);
		ui64			ParseUI64(const String& val, ui64 defVal = 0);
		Vector2			ParseVec2(const String& val);
		vector<Vector2>::type ParseVec2Array( const String& val);
		Vector3			ParseVec3(const String& val);
		vector<Vector3>::type ParseVec3Array( const String& val);
		Vector4			ParseVec4(const String& val);
		Quaternion		ParseQuaternion(const String& val);

		bool			IsNumber(const String& val);
		String			Hex2Char(Dword val);
	};

	int		sprintf_append	(char* s, const int maxByteSize, const char* const format, ...);
}

