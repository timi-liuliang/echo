#ifndef __LORD_COLOR_H__
#define __LORD_COLOR_H__

#include "LordMath.h"

namespace Lightmass
{
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

	public:
		Color() 
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
	};
}

#endif
