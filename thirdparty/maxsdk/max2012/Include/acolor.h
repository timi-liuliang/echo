/**********************************************************************
 *<
	FILE: acolor.h

	DESCRIPTION:  floating point color + alpha

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include "maxtypes.h"
#include "point3.h"
#include "point4.h"
#include "color.h"

/*! \sa  Class Color, Structure BMM_Color_24, Structure BMM_Color_32, Structure BMM_Color_48, Structure BMM_Color_64, Structure BMM_Color_fl, Structure RealPixel.\n\n
\par Description:
This class represents color as four floating point values: <b>r, g, b</b>, and
an alpha channel <b>a</b>.
Also note the following typedef:
<b>typedef AColor RGBA;</b> */
class AColor : public MaxHeapOperators
{
public:
	//! These values are in the range 0.0 to 1.0.
	float r,g,b,a;

	// Constructors
	/*! \remarks Constructor. The resulting object should be initialized with
	one of the initialization methods. */
	AColor()  {}
	/*! \remarks Constructor. Initializes the AColor to the RGBA color values
	passed. */
	AColor(float R, float G, float B, float A=1.0f)  { r = R; g = G; b = B; a = A; }
	/*! \remarks Constructor. Initializes the AColor to the RGBA color values
	passed (cast to float). */
	AColor(double R, double G, double B, double A=1.0) {
		 r = (float)R; g = (float)G; b = (float)B; a = (float)A; }
	/*! \remarks Constructor. Initializes the AColor to the RGBA color values
	passed (cast to float). */
	AColor(int R, int G, int B, int A=0) { 
		r = (float)R; g = (float)G; b = (float)B; a = (float)A; }
	/*! \remarks Constructor. Initializes the AColor to the AColor passed. */
	AColor(const AColor& c) { r = c.r; g = c.g; b = c.b; a = c.a; } 
	/*! \remarks Constructor. Initializes the AColor to the Color passed,
	optionally specifying an alpha value. */
	AColor(const Color& c, float alph=1.0f) { r = c.r; g = c.g; b = c.b; a = alph; } 
	/*! \remarks Constructor. Initializes the color to the Windows RGB value,
	optionally specifying an alpha value. */
	GEOMEXPORT explicit AColor(DWORD rgb, float alph=1.0f);  // from Windows RGB value
	AColor(Point4 p) { r = p.x; g = p.y; b = p.z; a = p.w; }
	/*! \remarks Constructor. Initializes the color to the value passed.
	\par Parameters:
	<b>float af[3]</b>\n\n
	Specifies the color. r=af[0], g=af[1], b=af[2], a=af[3]. */
	AColor(float af[4]) { r = af[0]; g = af[1]; b = af[2];a = af[3]; }
	/*! \remarks	Constructor. Initializes this AColor from the 24 bit color value passed.
	\par Parameters:
	<b>const BMM_Color_24\& c</b>\n\n
	The 24 bit color to initialize from. */
	AColor(const BMM_Color_24& c) { 
		r = float(c.r)/255.0f; g = float(c.g)/255.0f; b = float(c.b)/255.0f; a = 1.0f; 
		}
	/*! \remarks	Constructor. Initializes this AColor from the 32 bit color value passed.
	\par Parameters:
	<b>const BMM_Color_32\& c</b>\n\n
	The 32 bit color to initialize from. */
	AColor(const BMM_Color_32& c) { 
		r = float(c.r)/255.0f; g = float(c.g)/255.0f; b = float(c.b)/255.0f; a = float(c.a)/255.0f; 
		}
	/*! \remarks	Constructor. Initializes this AColor from the 48 bit color value passed.
	\par Parameters:
	<b>const BMM_Color_48\& c</b>\n\n
	The 48 bit color to initialize from. */
	AColor(const BMM_Color_48& c) { 
		r = float(c.r)/65535.0f; g = float(c.g)/65535.0f; b = float(c.b)/65535.0f; a = 1.0f; 
		}
	/*! \remarks	Constructor. Initializes this AColor from the 64 bit color value passed.
	\par Parameters:
	<b>const BMM_Color_64\& c</b>\n\n
	The 64 bit color to initialize from. */
	AColor(const BMM_Color_64& c) { 
		r = float(c.r)/65535.0f; g = float(c.g)/65535.0f; b = float(c.b)/65535.0f; a = float(c.a)/65535.0f; 
		}
	/*! \remarks	Constructor. Initializes this AColor from the floating point color passed.
	\par Parameters:
	<b>const BMM_Color_fl\& c</b>\n\n
	The floating point color to initialize from. No conversion or scaling is
	done. */
	AColor(const BMM_Color_fl& c) { 
		r = c.r; g = c.g; b = c.b; a = c.a; 
		}
	
	/*! \remarks Sets this AColor to black. r = g = b = 0.0f; a= 1.0f */
	void Black() { r = g = b = 0.0f; a = 1.0f; }
	/*! \remarks Sets the AColor to white. r = g = b = a = 1.0f */
	void White() { r = g = b = 1.0f; a = 1.0f; }

	/*! \remarks Makes all the components of the AColor \<= 1.0 */
	GEOMEXPORT void ClampMax();  // makes components >= 0.0
	/*! \remarks Makes all the components of the AColor \>= 0.0 */
	GEOMEXPORT void ClampMin();  // makes components <= 1.0
    /*! \remarks Makes all the components of the AColor fall in the range 0.0
    to 1.0. */
    GEOMEXPORT void ClampMinMax();  // makes components in [0,1]

	 /*! \remarks Access operators.
	 \par Parameters:
	 <b>int i</b>\n\n
	 The index of the component to return.
	 \return  0=r, 1=g, 2=b, 3=a. */
	float& operator[](int i) { return (&r)[i]; }     

	/*! \remarks Access operators.
	\par Parameters:
	<b>int i</b>\n\n
	The index of the component to return.
	\return  0=r, 1=g, 2=b, 3=a. */
	const float& operator[](int i) const { return (&r)[i]; }  

	// Conversion functions
	/*! \remarks Conversion function.\n\n
	Returns a pointer to the AColor. */
	operator float*() { return(&r); }
	operator Color() { return Color(r,g,b); }

	// Convert to Bitmap Manager types
	/*! \remarks	Converts this AColor to the BMM_Color_24 format.\n\n
	  */
	operator BMM_Color_24() { 
		BMM_Color_24 c; 
		c.r = (BYTE)int(r*255.0f); c.g = (BYTE)int(g*255.0f); c.b = (BYTE)int(b*255.0f);
		return c;
		}
	/*! \remarks	Converts this AColor to the BMM_Color_32 format. */
	operator BMM_Color_32() { 
		BMM_Color_32 c; 
		c.r = (BYTE)int(r*255.0f); c.g = (BYTE)int(g*255.0f); c.b = (BYTE)int(b*255.0f); c.a = (BYTE)int(a*255.0f);
		return c;
		}
	/*! \remarks	Converts this AColor to the BMM_Color_48 format. */
	operator BMM_Color_48() { 
		BMM_Color_48 c; 
		c.r = (WORD)int(r*65535.0f); c.g = (WORD)int(g*65535.0f); c.b = (WORD)int(b*65535.0f); 
		return c;
		}
	/*! \remarks	Converts this AColor to the BMM_Color_64 format. */
	operator BMM_Color_64() { 
		BMM_Color_64 c; 
		c.r = (WORD)int(r*65535.0f); c.g = (WORD)int(g*65535.0f); c.b = (WORD)int(b*65535.0f); c.a = (WORD)int(a*65535.0f);
		return c;
		}
	/*! \remarks	Converts this AColor to the BMM_Color_fl format. */
	operator BMM_Color_fl() { 
		BMM_Color_fl c; 
		c.r = r;	c.g = g;	c.b = b;	c.a = a;
		return c;
		}

	// Convert to Windows RGB
//	operator DWORD() { return RGB(FLto255(r),FLto255(g), FLto255(b)); }
	/*! \remarks Convert the AColor to a Windows RGB color. See
	<a href="ms-its:listsandfunctions.chm::/idx_R_colorref.html">COLORREF</a>.
	\return  A Windows RGB color. */
	DWORD toRGB() { return RGB(FLto255(r),FLto255(g), FLto255(b)); };

	// Convert to Point3, 4
	/*! \remarks Convert the AColor to a Point3.
	\return  A Point3. x=r, y=g, z=b. */
	operator Point3() { return Point3(r,g,b); }
	operator Point4() { return Point4(r,g,b,a); }

	// Unary operators
	/*! \remarks Unary - operator.
	\return  The Color with the components negated, i.e.\n\n
	<b>{ return(AColor(-r,-g,-b, -a)); }</b> */
	AColor operator-() const { return (AColor(-r,-g,-b, -a)); } 
	/*! \remarks Unary + operator.
	\return  The AColor itself. */
	AColor operator+() const { return *this; } 

	// Assignment operators
	/*! \remarks Subtracts an AColor from this AColor.
	\return  A Color that is the difference between two Colors. */
	inline AColor& operator-=(const AColor&);
    /*! \remarks Adds an AColor to this AColor.
    \return  An AColor that is the sum of two AColors. */
    inline AColor& operator+=(const AColor&);
	/*! \remarks Multiplies the components of this AColor by a float.
	\return  An AColor multiplied by a float. */
	inline AColor& operator*=(float); 
	/*! \remarks Divides the components of this AColor by a float.
	\return  An AColor divided by a float. */
	inline AColor& operator/=(float);
	/*! \remarks Performs element-by-element multiplying between two AColors.
	\return  This AColor element-by-element multiplied by another AColor. */
	inline AColor& operator*=(const AColor&);	// element-by-element multiplg.

	// Test for equality
	/*! \remarks Test for equality between two AColors.
	\return  Nonzero if the AColors are equal; otherwise 0. */
	int operator==(const AColor& p) const { return ((p.r==r)&&(p.g==g)&&(p.b==b)&&(p.a==a)); }
	/*! \remarks Tests for inequality between two AColors.
	\return  Nonzero if the AColors are not equal; otherwise 0. */
	int operator!=(const AColor& p) const { return ((p.r!=r)||(p.g!=g)||(p.b!=b)||(p.a!=a)); }

	// Binary operators
	/*! \remarks Subtracts an AColor from an AColor.
	\return  An AColor that is the difference between two AColors. */
	inline AColor operator-(const AColor&) const;
	/*! \remarks Adds an AColor to an AColor.
	\return  An AColor that is the difference between two AColors. */
	inline AColor operator+(const AColor&) const;
	/*! \remarks Divides an AColor by an AColor.
	\return  An AColor divided by an AColor. r/r, g/g, b/b, a/a. */
	inline AColor operator/(const AColor&) const;
    /*! \remarks Multiplies an AColor by an AColor.
    \return  An AColor multiplied by an AColor. r*r, g*g, b*b, a*a. */
    inline AColor operator*(const AColor&) const;   
	/*! \remarks Cross product of two AColors.
		The alpha component is not affected by the cross product.
	\return  An AColor that is the cross product of two AColors. */
	inline AColor operator^(const AColor&) const;   // CROSS PRODUCT
	};

/*! \remarks Returns the index of the component with the maximum absolute
value.
\par Parameters:
<b>const AColor\&</b>\n\n
The color to check.
\return  The index of the component with the maximum absolute value. r=0, g=1,
b=2, a=3. */
GEOMEXPORT int MaxComponent(const AColor&);  // the component with the maximum abs value


/*! \remarks Returns the index of the component with the minimum absolute
value.
\par Parameters:
<b>const AColor\&</b>\n\n
The color to check.
\return  The index of the component with the minimum absolute value. r=0, g=1,
b=2, a=3. */
GEOMEXPORT int MinComponent(const AColor&);  // the component with the minimum abs value

// Inlines:

inline AColor& AColor::operator-=(const AColor& c) {	
	r -= c.r;	g -= c.g;	b -= c.b;  a -= c.a;
	return *this;
	}

inline AColor& AColor::operator+=(const AColor& c) {
	r += c.r;	g += c.g;	b += c.b; a += c.a;
	return *this;
	}

inline AColor& AColor::operator*=(float f) {
	r *= f;   g *= f;	b *= f;  a *= f;
	return *this;
	}

inline AColor& AColor::operator/=(float f) { 
	r /= f;	g /= f;	b /= f;	a /= f;
	return *this; 
	}

inline AColor& AColor::operator*=(const AColor& c) { 
	r *= c.r;	g *= c.g;	b *= c.b;	a *= c.a;
	return *this; 
	}

inline AColor AColor::operator-(const AColor& c) const {
	return(AColor(r-c.r,g-c.g,b-c.b,a-c.a));
	}

inline AColor AColor::operator+(const AColor& c) const {
	return(AColor(r+c.r,g+c.g,b+c.b,a+c.a));
	}

inline AColor AColor::operator/(const AColor& c) const {
	return AColor(r/c.r,g/c.g,b/c.b,a/c.a);
	}

inline AColor AColor::operator*(const AColor& c) const {  
	return AColor(r*c.r, g*c.g, b*c.b, a*c.a);	
	}

inline AColor AColor::operator^(const AColor& c) const {
	return AColor(g * c.b - b * c.g, b * c.r - r * c.b, r * c.g - g * c.r, c.a);
	}

/*! \remarks Multiplies each component of an AColor by a float.
\return  An AColor with each component multiplied by a float. */
inline AColor operator*(float f, const AColor& a) {
	return(AColor(a.r*f, a.g*f, a.b*f, a.a*f));
	}

/*! \remarks Multiplies each component of an AColor by a float.
\return  An AColor with each component multiplied by a float. */
inline AColor operator*(const AColor& a, float f) {
	return(AColor(a.r*f, a.g*f, a.b*f, a.a*f));
	}

// Composite  fg over bg, assuming associated alpha,
// i.e. pre-multiplied alpha for both fg and bg
/*! \remarks Composite <b>fg</b> over <b>bg</b>, assuming associated alpha,
i.e. pre-multiplied alpha for both <b>fg</b> and <b>bg</b>\n\n
This is: <b>fg + (1.0f-fg.a)*bg</b>
\par Parameters:
<b>const AColor \&fg</b>\n\n
Specifies the foreground color to composite.\n\n
<b>const AColor\& bg</b>\n\n
Specifies the background color to composite over.
\return  The resulting AColor. */
inline AColor CompOver(const AColor &fg, const AColor& bg) {
	return  fg + (1.0f-fg.a)*bg;
	}

typedef AColor RGBA;


