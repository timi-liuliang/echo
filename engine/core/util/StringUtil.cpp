#include "StringUtil.h"
#include "engine/core/math/Vector2.h"
#include "engine/core/math/Vector3.h"
#include "engine/core/math/Vector4.h"
#include "engine/core/math/Quaternion.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <codecvt>
#include "internal_itoa.h"
#include "internal_dtoa.h"
#ifdef ECHO_PLATFORM_WINDOWS
#	define WIN32_LEAN_AND_MEAN
#	define _CRT_SECURE_NO_WARNINGS
#	include <Windows.h>
#else
#include <iconv.h>
#include <wchar.h>
#include <string.h>
#endif

namespace Echo
{
	static char _HexToChar[] =
	{
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};

	static String MorphNumericString(String str, int n = 3) {
		char* s = (char*)str.c_str();
		char *p = strchr(s, '.');         // Find decimal point, if any.
		if (p)
		{
			int count = n;              // Adjust for more or less decimals.
			while (count >= 0)
			{    // Maximum decimals allowed.
				count--;
				if (*p == '\0')    // If there's less than desired.
				{
					break;
				}
				p++;               // Next character.
			}

			*p-- = '\0';            // Truncate string.
			while (*p == '0')       // Remove trailing zeros.
			{
				*p-- = '\0';
			}

			if (*p == '.')          // If all decimals were zeros, remove ".".
			{
				*p = '\0';
			}
		}
		return String(s);
	}

	String StringUtil::Replace(const String& str, const String& src, const String& dst)
	{    
		if (src == dst)
			return str;

		String out = str;
		size_t pos = str.find(src, 0);
		while (pos != String::npos)
		{
			out.replace(pos, src.size(), dst);
			pos = out.find(src, pos+dst.size());
		}

		return out;
	}

	String StringUtil::Replace(const String& str, char src, char dst)
	{
		String out = str;
		if (out.empty())
		{
			return out;
		}
		char* buf = &(*out.begin());
		while (*buf)
		{
			if (*buf == src)
				*buf = dst;
			buf++;
		}

		return out;
	}

	bool StringUtil::ReplaceRet(String& str, const String& src, const String& dst)
	{    
		if (src == dst)
			return false;

		bool bReplaced = false;
		size_t pos = str.find(src, 0);

		if(pos != String::npos)
		{
			str.replace(pos, src.size(), dst);
			bReplaced = true;
		}

		return bReplaced;
	}

	void StringUtil::Trim(String& str, bool bLeft, bool bRight)
	{
		static const String delims = " \t\r\n";
		if(bRight)
			str.erase(str.find_last_not_of(delims)+1); // trim right
		if(bLeft)
			str.erase(0, str.find_first_not_of(delims)); // trim left
	}

	String StringUtil::Substr(const String& str, const String& delims, bool isClockWise)
	{
		if (isClockWise)
		{
			return str.substr(0, str.find_first_of(delims));
		}
		else
		{
			size_t last_pos = str.find_last_of(delims)+1;
			return str.substr( last_pos, str.size() - last_pos);
		}
	}

	StringArray StringUtil::Split(const String& str, const String& delims, Dword maxSplits)
	{
		StringArray ret;

		if(str.empty())
			return ret;

		// Pre-allocate some space for performance
		ret.reserve(maxSplits ? maxSplits+1 : 10);    // 10 is guessed capacity for most case

		Dword numSplits = 0;

		// Use STL methods
		size_t start, pos;
		start = 0;
		do
		{
			pos = str.find_first_of(delims, start);
			if (pos == start)
			{
				start = pos + 1;
			}
			else if (pos == String::npos || (maxSplits && numSplits == maxSplits))
			{
				// Copy the rest of the string
				ret.emplace_back( str.substr(start) );
				break;
			}
			else
			{
				// Copy up to delimiter
				ret.emplace_back( str.substr(start, pos - start) );
				start = pos + 1;
			}
			// parse up to next real data
			start = str.find_first_not_of(delims, start);
			++numSplits;

		} while (pos != String::npos);

		return ret;
	}

	void StringUtil::SplitFileName(const String& qualifiedName, String& outBasename, String& outPath)
	{
		String path = qualifiedName;
		// Replace \ with / first
		std::replace( path.begin(), path.end(), '\\', '/' );
		// split based on final /
		size_t i = path.find_last_of('/');

		if (i == String::npos)
		{
			outPath.clear();
			outBasename = qualifiedName;
		}
		else
		{
			outBasename = path.substr(i+1, path.size() - i - 1);
			outPath = path.substr(0, i+1);
		}
	}

	void StringUtil::LowerCase(String& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), (int(*)(int)) tolower);
	}

	void StringUtil::UpperCase(String& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), (int(*)(int)) toupper);
	}

	bool StringUtil::IsHaveUpper(const String& str)
	{
		for (char c : str)
		{
			if (isupper(c))
				return true;
		}

		return false;
	}

	bool StringUtil::IsHaveLower(const String& str)
	{
		for (char c : str)
		{
			if (islower(c))
				return true;
		}

		return false;
	}

	String StringUtil::Format(const char* formats, ... )
	{
		const int bufferLength = 8192;
		char szBuffer[bufferLength] = {};
		int numforwrite = 0;

		va_list args;
		va_start(args, formats);
#ifdef WIN32
		numforwrite = _vsnprintf(szBuffer, bufferLength, formats, args);
#else
		numforwrite = vsnprintf( szBuffer, bufferLength, formats, args );
#endif
		va_end(args);

		szBuffer[bufferLength-1] = 0;

		return szBuffer;
	}

	bool StringUtil::StartWith(const String& str, const String& pattern, bool lowCase)
	{
		size_t thisLen = str.length();
		size_t patternLen = pattern.length();
		if (thisLen < patternLen || patternLen == 0)
			return false;

		String startOfThis = str.substr(0, patternLen);
		if(lowCase)
			StringUtil::LowerCase(startOfThis);

		return (startOfThis == pattern);
	}

	bool StringUtil::EndWith(const String& str, const String& pattern, bool bCaseSensitive)
	{
		size_t thisLen = str.length();
		size_t patternLen = pattern.length();
		if (thisLen < patternLen || patternLen == 0)
			return false;

		String endOfThis = str.substr(thisLen - patternLen, patternLen);

		if (bCaseSensitive)
		{
			return (endOfThis == pattern);
		}
		else
		{
			String lstr1 = endOfThis;
			String lstr2 = pattern;
			LowerCase(lstr1);
			LowerCase(lstr2);
			return (lstr1 == lstr2);
		}
	}

	bool StringUtil::Equal(const String& str1, const String& str2, bool bCaseSensitive/*= true*/)
	{
		if (bCaseSensitive)
		{
			return (str1 == str2);
		}
		else
		{
			String lstr1 = str1;
			String lstr2 = str2;
			LowerCase(lstr1);
			LowerCase(lstr2);
			return (lstr1 == lstr2);
		}
	}

	bool StringUtil::Contain(const String& str, const String& pattern, bool bCaseSensitive)
	{
		if (!bCaseSensitive)
		{
			String strLower = str;
			String patternLower = pattern;
			LowerCase(strLower);
			LowerCase(patternLower);

			return strLower.find(patternLower) != std::string::npos;
		}

		return str.find(pattern) != std::string::npos;
	}

	String StringUtil::RemoveLast(const String& str, const String& subStr, bool isCaseSensitive)
	{
		if (EndWith(str, subStr, isCaseSensitive))
		{
			return str.substr(0, str.size() - subStr.size());
		}

		return BLANK;
	}

	String StringUtil::WCS2MBS(const WString &str)
	{
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.to_bytes(str);
	}

	WString StringUtil::MBS2WCS(const String& str)
	{
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
        return conv.from_bytes(str);
	}

	String StringUtil::ToString(float val, Word precision, Word width, char fill)
	{
		char buffer[64] = {}; 
		internal::dtoa_internal(val, buffer, precision); 

		return String(buffer); 
	}

	String StringUtil::ToString(double val, Word precision, Word width, char fill)
	{
		char buffer[64] = {};
		internal::dtoa_internal(val, buffer, precision);

		return String(buffer);
	}

	String StringUtil::ToString(i32 val, Word width, char fill)
	{
		char buffer[64] = {}; 
		internal::i32toa_branchlut(val, buffer); 

		return String(buffer);
	}

	String StringUtil::ToString(i64 val, Word width, char fill)
	{
		char buffer[64] = {};
		internal::i64toa_branchlut(val, buffer);

		return String(buffer);
	}

	String StringUtil::ToString(ui32 val, Word width, char fill)
	{
		char buffer[64] = {};
		internal::u32toa_branchlut(val, buffer);

		return String(buffer);
	}

	String StringUtil::ToString(ui64 val, Word width, char fill)
	{
		char buffer[64] = {};
		internal::u64toa_branchlut(val, buffer);

		return String(buffer);
	}

	String StringUtil::ToString(const vector<ui8>::type& val)
	{
		String result;
		for (ui8 element : val)
			result += Format("%d ", element);

		return result;
	}

	String StringUtil::ToString(bool val, bool bYesNo)
	{
		if (val)
		{
			if (bYesNo)
				return "yes";
			else
				return "true";
		}
		else
		{
			if (bYesNo)
				return "no";
			else
				return "false";
		}
	}

	String StringUtil::ToString(const vector<double>::type& val)
	{
		String result;
		for (double element : val)
			result += Format("%s ", MorphNumericString(ToString(element)).c_str());

		return result;
	}

	String StringUtil::ToString(const Vector2& val)
	{
		// trad convert, only fast than iostream.
		return Format("%s %s", MorphNumericString(ToString(val.x)).c_str(), MorphNumericString(ToString(val.y)).c_str());
	}

	String StringUtil::ToString( const vector<Vector2>::type& val)
	{
		// trad convert, only fast than iostream.
		String result; result.reserve(val.size() * 20); 

		for (const Vector2& element : val)
			result += Format("%s %s;", MorphNumericString(ToString(element.x)).c_str(), MorphNumericString(ToString(element.y)).c_str());

		return result; 
	}

	String StringUtil::ToString(const Vector3& val)
	{
		// trad convert, only fast than iostream.
		return Format("%s %s %s", MorphNumericString(ToString(val.x)).c_str(), MorphNumericString(ToString(val.y)).c_str(), MorphNumericString(ToString(val.z)).c_str());
	}

	String StringUtil::ToString(const vector<Vector3>::type& val)
	{
		// trad convert, only fast than iostream.
		String result; result.reserve(val.size() * 30);

		for (const Vector3& element : val)
			result += Format("%s %s %s ", MorphNumericString(ToString(element.x)).c_str(), MorphNumericString(ToString(element.y)).c_str(), MorphNumericString(ToString(element.z)).c_str());

		return result;
	}

	String StringUtil::ToString(const Vector4& val)
	{
		// trad convert, only fast than iostream.
		return Format("%s %s %s %s", MorphNumericString(ToString(val.x)).c_str(), MorphNumericString(ToString(val.y)).c_str(), MorphNumericString(ToString(val.z)).c_str(), MorphNumericString(ToString(val.w)).c_str());
	}

	String StringUtil::ToString(const Quaternion& val)
	{
		// trad convert, only fast than iostream.
		return Format("%s %s %s %s", MorphNumericString(ToString(val.x)).c_str(), MorphNumericString(ToString(val.y)).c_str(), MorphNumericString(ToString(val.z)).c_str(), MorphNumericString(ToString(val.w)).c_str());;
	}

	String StringUtil::ToString(const StringArray& strs, const char* delims)
	{
		String result; 
		if (strs.size() > 0)
		{
			result.reserve(strs.size() * 16);
			i32 count = i32(strs.size()) - 1;
			for (i32 i=0; i<count; i++)
			{
				result += strs[i];
				result += delims;
			}

			result += strs.back();
		}

		return result;
	}

	float StringUtil::ParseFloat(const String& val, float defVal)
	{
		// trad convert, only fast than iostream.
		float v = static_cast<float>(std::atof(val.c_str()));
		return v ? v : defVal;
	}

	double StringUtil::ParseDouble(const String& val, double defVal)
	{
		// trad convert, only fast than iostream.
		double v = std::atof(val.c_str());
		return v ? v : defVal;
	}

	Real StringUtil::ParseReal(const String& val, Real defVal)
	{
		// trad convert, only fast than iostream.
		Real v = static_cast<Real>(std::atof(val.c_str()));
		return v ? v : defVal;
	}

	vector<Real>::type StringUtil::ParseRealVector(const String& val, const vector<Real>::type& defVal)
	{
		vector<Real>::type result;
		StringArray strArray = Split(val, " ");
		for (const String& element : strArray)
		{
			result.emplace_back(ParseReal(element));
		}

		return result;
	}

	int StringUtil::ParseInt(const String& val, int defVal)
	{
		// trad convert, only fast than iostream.
		int v = std::atoi(val.c_str());
		return v ? v : defVal;
	}

	long StringUtil::ParseLong(const String& val, long defVal)
	{
		// trad convert, only fast than iostream.
		long v = std::atol(val.c_str());
		return v ? v : defVal;
	}

	bool StringUtil::ParseBool(const String& val, bool defVal)
	{
		if (StringUtil::Equal(val, "true", false) ||
			StringUtil::Equal(val, "yes", false) ||
			StringUtil::Equal(val, "1", false))
			return true;
		else if(StringUtil::Equal(val, "false", false) ||
			StringUtil::Equal(val, "no", false) ||
			StringUtil::Equal(val, "0", false))
			return false;
		else
			return defVal;
	}

	Word StringUtil::ParseWord(const String& val, Word defVal /*= 0xffff*/)
	{
		StringStream ss(val.c_str());
		Word ret = defVal;
		ss >> std::hex >> ret;
		return ret;
	}

	Dword StringUtil::ParseDword(const String& val, Dword defVal /*= 0xffffffff*/)
	{
		StringStream ss(val.c_str());
		Dword ret = defVal;
		ss >> std::hex >> ret;
		return ret;
	}

	i8 StringUtil::ParseI8(const String& val, i8 defVal)
	{
		// trad convert, only fast than iostream.
		i8 v = static_cast<i8>(std::atoi(val.c_str()));
		return v ? v : defVal;
	}

	i16 StringUtil::ParseI16(const String& val, i16 defVal)
	{
		// trad convert, only fast than iostream.
		i16 v = static_cast<i16>(std::atoi(val.c_str()));
		return v ? v : defVal;

	}

	i32 StringUtil::ParseI32(const String& val, i32 defVal)
	{
		// trad convert, only fast than iostream.
		i32 v = static_cast<i32>(std::atoi(val.c_str()));
		return v ? v : defVal;
	}

	i64 StringUtil::ParseI64(const String& val, i64 defaultValue)
	{
#ifdef ECHO_PLATFORM_ANDROID
		i64 v = static_cast<i64>(atoll(val.c_str()));
#else
		// trad convert, only fast than iostream.
		i64 v = static_cast<i64>(std::atoll(val.c_str()));
#endif
		return v ? v : defaultValue;
	}

	ui8 StringUtil::ParseUI8(const String& val, ui8 defVal)
	{
		// trad convert, only fast than iostream.
		ui8 v = static_cast<ui8>(std::atoi(val.c_str()));
		return v ? v : defVal;
	}

	ui16 StringUtil::ParseUI16(const String& val, ui16 defVal)
	{
		// trad convert, only fast than iostream.
		ui16 v = static_cast<ui16>(std::atoi(val.c_str()));
		return v ? v : defVal;
	}

	ui32 StringUtil::ParseUI32(const String& val, ui32 defVal)
	{
		// trad convert, only fast than iostream.
		ui32 v = static_cast<ui32>(std::atoi(val.c_str()));
		return v ? v : defVal;
	}

	ui64 StringUtil::ParseUI64(const String& val, ui64 defVal)
	{
#ifdef ECHO_PLATFORM_ANDROID
		ui64 v = static_cast<ui64>(atoll(val.c_str()));
#else
		// trad convert, only fast than iostream.
		ui64 v = static_cast<ui64>(std::atoll(val.c_str()));
#endif
		return v ? v : defVal;
	}

	Vector2 StringUtil::ParseVec2(const String& val)
	{
		// Split on space
		StringArray vec = StringUtil::Split(val);

		if (vec.size() != 2)
		{
			return Vector2::ZERO;
		}
		else
		{
			return Vector2(ParseReal(vec[0]), ParseReal(vec[1]));
		}
	}

	vector<Vector2>::type StringUtil::ParseVec2Array( const String& val)
	{
		vector<Vector2>::type vec2Array;
		StringArray attackAngles = StringUtil::Split( val, ";");
		for( size_t angleIdx=0; angleIdx<attackAngles.size(); angleIdx++)
		{
			vec2Array.emplace_back( StringUtil::ParseVec2( attackAngles[angleIdx]));
		}

		return vec2Array;
	}

	Vector3 StringUtil::ParseVec3(const String& val)
	{
		StringArray vec = StringUtil::Split(val);

		if(vec.size() < 3)
		{
			return Vector3::ZERO;
		}
		else
		{
			return Vector3(ParseReal(vec[0]), ParseReal(vec[1]), ParseReal(vec[2]));
		}
	}

	vector<Vector3>::type StringUtil::ParseVec3Array(const String& val)
	{
		StringArray vec = StringUtil::Split(val); 

		if (vec.size() % 3 != 0)
		{
			return std::vector<Vector3>(); 
		}
		else
		{
			std::vector<Vector3> result; 
			for (size_t i = 0; i < vec.size(); i+= 3)
			{
				result.emplace_back(ParseReal(vec[i]), ParseReal(vec[i + 1]), ParseReal(vec[i + 2]));
			}

			return result; 
		}
	}

	Vector4 StringUtil::ParseVec4(const String& val)
	{
		StringArray vec = StringUtil::Split(val);

		if(vec.size() != 4)
		{
			return Vector4::ZERO;
		}
		else
		{
			return Vector4(ParseReal(vec[0]), ParseReal(vec[1]), ParseReal(vec[2]), ParseReal(vec[3]));
		}
	}

	Quaternion StringUtil::ParseQuaternion(const String& val)
	{
		StringArray vec = StringUtil::Split(val);

		if(vec.size() != 4)
		{
			return Quaternion::IDENTITY;
		}
		else
		{
			return Quaternion(ParseReal(vec[0]), ParseReal(vec[1]), ParseReal(vec[2]), ParseReal(vec[3]));
		}
	}

	bool StringUtil::IsNumber(const String& val)
	{
		StringStream str(val);
		float tst;
		str >> tst;
		return !str.fail() && str.eof();
	}

	String StringUtil::Hex2Char(Dword val)
	{
		String out;
		out.resize(4*2+1);// one byte - two characters

		char* to = (char*)out.c_str();
		Byte *from = (Byte*)(&val) + 3;

		for (int i = 0; i < 4; ++i)
		{
			*(to++) = _HexToChar[(*from) >> 4];		// 4 high bits.
			*(to++) = _HexToChar[(*from) & 0x0f];	// 4 low bits.

			--from;
		}

		return out;
	}

	int sprintf_append(char* s, const int max, const char* const format, ...)
	{
		va_list arg;
		va_start(arg, format);
		int copylen = 0;
		int len = int(strlen(s));
		int freelen = max - len;
		if (freelen > 0)
		{
			copylen = ::vsnprintf(s + len, freelen, format, arg);
			s[max - 1] = 0;
		}
		va_end(arg);
		return copylen;
	}
}
