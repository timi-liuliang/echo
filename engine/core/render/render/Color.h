#pragma once

#include <engine/core/Math/Math.h>
#include "RenderDef.h"

namespace Echo
{
	/**
	 * ÑÕÉ«¶¨Òå
	 */
	class Color
	{
	public:	
		Real r, g, b, a;

		static const Color INVALID;
		static const Color RED;
		static const Color GREEN;
		static const Color BLUE;
		static const Color BLACK;
		static const Color WHITE;
		static const Color YELLOW;
		static const Color CYAN;
		static const Color PURPLE;
		static const Color GRAY;

	public:
		Color()
			: r(0.f)
			, g(0.f)
			, b(0.f)
			, a(1.f)
		{

		}

		inline Color(Dword dw)
		{
			setARGB(dw);
		}

		inline Color(const Real* pf)
		{
			r = pf[0];
			g = pf[1];
			b = pf[2];
			a = pf[3];
		}

		inline Color(const Color& rhs)
		{
			r = rhs.r;
			g = rhs.g;
			b = rhs.b;
			a = rhs.a;
		}

		inline Color(Real fr, Real fg, Real fb, Real fa = 1.0f)
			: r(fr)
			, g(fg)
			, b(fb)
			, a(fa)
		{

		}

	public:
		// casting
		inline operator Dword () const
		{
			return getARGB();
		}

		inline operator Real* ()
		{
			return (Real*)&r;
		}

		inline operator const Real* () const
		{
			return (const Real*)&r;
		}

		inline operator const Vector4() const 
		{
			return Vector4(r, g, b, a);
		}

		inline Color& operator = (const Vector4& c)
		{
			r = c.x;
			g = c.y;
			b = c.z;
			a = c.w;

			return *this;
		}

		// assignment operators
		inline  Color& operator += (const Color& c)
		{
			r += c.r;
			g += c.g;
			b += c.b;
			a += c.a;
			return *this;
		}

		inline Color& operator -= (const Color& c)
		{
			r -= c.r;
			g -= c.g;
			b -= c.b;
			a -= c.a;
			return *this;
		}

		inline Color& operator *= (const Color& c)
		{
			r *= c.r;
			g *= c.g;
			b *= c.b;
			a *= c.a;

			return *this;
		}

		inline Color& operator *= (Real f)
		{
			r *= f;
			g *= f;
			b *= f;
			a *= f;
			return *this;
		}

		inline Color& operator /= (Real f)
		{
			Real fInv = 1.0f / f;
			r *= fInv;
			g *= fInv;
			b *= fInv;
			a *= fInv;
			return *this;
		}

		// unary operators
		inline Color operator + () const
		{
			return *this;
		}

		inline Color operator - () const
		{
			return Color(-r, -g, -b, -a);
		}

		// binary operators
		inline Color operator + (const Color& c) const
		{
			return Color(r + c.r, g + c.g, b + c.b, a + c.a);
		}

		inline Color operator - (const Color& c) const
		{
			return Color(r - c.r, g - c.g, b - c.b, a - c.a);
		}

		inline Color operator * (const Color& c) const
		{
			return Color(r * c.r, g * c.g, b * c.b, a * c.a);
		}

		inline Color operator * (Real f) const
		{
			return Color(r * f, g * f, b * f, a * f);
		}

		inline Color operator / (Real f) const
		{
			Real fInv = 1.0f / f;
			return Color(r * fInv, g * fInv, b * fInv, a * fInv);
		}


		inline friend Color operator * (Real f, const Color& c)
		{
			return Color(f * c.r, f * c.g, f * c.b, f * c.a);
		}

		inline bool operator == (const Color& c) const
		{
			return r == c.r && g == c.g && b == c.b && a == c.a;
		}

		inline bool operator != (const Color& c) const
		{
			return r != c.r || g != c.g || b != c.b || a != c.a;
		}

		inline void set(Real fr, Real fg, Real fb, Real fa)
		{
			r = fr;
			g = fg;
			b = fb;
			a = fa;
		}

		void setRGBA(Byte ir, Byte ig, Byte ib, Byte ia)
		{
			const Real f = 1.0f / 255.0f;
			r = f * (Real)ir;
			g = f * (Real)ig;
			b = f * (Real)ib;
			a = f * (Real)ia;
		}

		inline void setARGB(Dword argb)
		{
			const Real f = 1.0f / 255.0f;
			r = f * (Real) (Byte) (argb >> 16);
			g = f * (Real) (Byte) (argb >>  8);
			b = f * (Real) (Byte) (argb >>  0);
			a = f * (Real) (Byte) (argb >> 24);
		}

		inline void setABGR(Dword abgr)
		{
			const Real f = 1.0f / 255.0f;
			r = f * (Real) (Byte) (abgr >> 0);
			g = f * (Real) (Byte) (abgr >>  8);
			b = f * (Real) (Byte) (abgr >>  16);
			a = f * (Real) (Byte) (abgr >> 24);
		}

		inline Dword getARGB() const 
		{
			Dword dwR = r >= 1.0f ? 0xff : r <= 0.0f ? 0x00 : (Dword) (r * 255.0f + 0.5f);
			Dword dwG = g >= 1.0f ? 0xff : g <= 0.0f ? 0x00 : (Dword) (g * 255.0f + 0.5f);
			Dword dwB = b >= 1.0f ? 0xff : b <= 0.0f ? 0x00 : (Dword) (b * 255.0f + 0.5f);
			Dword dwA = a >= 1.0f ? 0xff : a <= 0.0f ? 0x00 : (Dword) (a * 255.0f + 0.5f);

			return (dwA << 24) | (dwR << 16) | (dwG << 8) | dwB;
		}

		inline Dword getABGR() const
		{
			Dword dwA = a >= 1.0 ? 0xff : a <= 0.0 ? 0x00 : (Dword) (a * 255.0 + 0.5);
			Dword dwB = b >= 1.0 ? 0xff : b <= 0.0 ? 0x00 : (Dword) (b * 255.0 + 0.5);
			Dword dwR = r >= 1.0 ? 0xff : r <= 0.0 ? 0x00 : (Dword) (r * 255.0 + 0.5);
			Dword dwG = g >= 1.0 ? 0xff : g <= 0.0 ? 0x00 : (Dword) (g * 255.0 + 0.5);

			return (dwA << 24) | (dwB << 16) | (dwG << 8) | dwR;
		}

		inline Dword getRGBA() const
		{
			Dword dwR = r >= 1.0 ? 0xff : r <= 0.0 ? 0x00 : (Dword) (r * 255.0 + 0.5);
			Dword dwG = g >= 1.0 ? 0xff : g <= 0.0 ? 0x00 : (Dword) (g * 255.0 + 0.5);
			Dword dwB = b >= 1.0 ? 0xff : b <= 0.0 ? 0x00 : (Dword) (b * 255.0 + 0.5);
			Dword dwA = a >= 1.0 ? 0xff : a <= 0.0 ? 0x00 : (Dword) (a * 255.0 + 0.5);

			return (dwR << 24) | (dwG << 16) | (dwB << 8) | dwA;
		}

		inline Dword getBGRA() const
		{
			Dword dwB = b >= 1.0 ? 0xff : b <= 0.0 ? 0x00 : (Dword) (b * 255.0 + 0.5);
			Dword dwG = g >= 1.0 ? 0xff : g <= 0.0 ? 0x00 : (Dword) (g * 255.0 + 0.5);
			Dword dwR = r >= 1.0 ? 0xff : r <= 0.0 ? 0x00 : (Dword) (r * 255.0 + 0.5);
			Dword dwA = a >= 1.0 ? 0xff : a <= 0.0 ? 0x00 : (Dword) (a * 255.0 + 0.5);

			return (dwB << 24) | (dwG << 16) | (dwR << 8) | dwA;
		}

		inline Color& saturate()
		{
			if ( r > 1.0 ) r = 1.0;
			if ( g > 1.0 ) g = 1.0;
			if ( b > 1.0 ) b = 1.0;
			if ( a > 1.0 ) a = 1.0;

			if ( r < 0.0 ) r = 0.0;
			if ( g < 0.0 ) g = 0.0;
			if ( b < 0.0 ) b = 0.0;
			if ( a < 0.0 ) a = 0.0;

			return *this;
		}

		inline Color& pow(Real exponent)
		{
			r = Math::Pow(r, exponent);
			g = Math::Pow(g, exponent);
			b = Math::Pow(b, exponent);
			a = Math::Pow(a, exponent);

			return *this;
		}

		inline static bool HSV_to_RGB(Vector3& HSV)
		{
			// H and S and V are given on [0, 1].
			// RGB are each returned on [0, 1].
			real32 h = HSV.x * 6.f, s = HSV.y, v = HSV.z, m, n, f;
			i32 i;

			if (h == 0) h = .01f;

			i = static_cast<i32>(Math::Floor(h));
			f = h - i;
			if (!(i & 1)) f = 1 - f; // if i is even
			m = v * (1 - s);
			n = v * (1 - s * f);
			switch (i)
			{
				case 6:
				case 0: HSV = Vector3(v, n, m); break;
				case 1: HSV = Vector3(n, v, m); break;
				case 2: HSV = Vector3(m, v, n); break;
				case 3: HSV = Vector3(m, n, v); break;
				case 4: HSV = Vector3(n, m, v); break;
				case 5: HSV = Vector3(v, m, n); break;

				return true;
			}

			return false;
		}

	};

	namespace StringUtil
	{
		Color ParseColor(const String& val);
		String ToString(const Color& val);
		Color fromString(const String& val);
	}
}
