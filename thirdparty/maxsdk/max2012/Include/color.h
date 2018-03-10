	/**********************************************************************
 *<
	FILE: color.h

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "GeomExport.h"
#include "maxheap.h"
#include <WTypes.h>
#include "point3.h"
#include "maxtypes.h"

#define FLto255(x) ((int)((x)*255.0f+.5))	

class Color;

/*! \sa Class Color.
\remarks This structure describes color in terms of r, g, b, e. \n\n
This is taken from GraphicsGems II, "Real Pixels" by Greg Ward of Lawrence 
Berkeley Laboratory. What it means is this: "e" is the base 2 exponent of the 
maximum RGB component, and r,g,b are the mantissas of R,G,and B, relative to 
this exponent. It essentially compresses the essential data of a floating 
point color into 32 bits. \n\n
Quoting from Graphics Gems II: \n
"It appears that this format favors the largest primary value at the expense 
of accuracy in the other two primaries. This is true, but it also is true 
that the largest value dominates the displayed pixel color so that the 
other primaries become less noticeable". \n\n
One GBuffer option is to write out the image in RealPixel format, storing 
NON CLAMPED colors. This could be used by a Video Post process to detect 
those areas of the image where the intensity goes beyond 1 and apply halo 
and flare effects much more realistically. \n\n
There are functions for converting between floating point and RealPixel format:\n
<code>RealPixel MakeRealPixel(float r, float g, float b);</code> \n
and  \n
<code>ExpandRealPixel(RealPixel &rp, float& r, float &g, float& b);</code> \n
as well as methods in RealPixel and Color.
*/
struct RealPixel: public MaxHeapOperators {
	/*! The mantissas of R,G,and B, relative to this exponent. */
	unsigned char r,g,b;   // mantissas
	/*! The base 2 exponent of the maximum RGB component. */
	unsigned char e;	   // exponent
	/*! \remarks Converts the RealPixel format to the Color format. */
	GEOMEXPORT operator Color();
	};

GEOMEXPORT RealPixel MakeRealPixel(float r, float g, float b);
GEOMEXPORT void ExpandRealPixel(const RealPixel &rp, float &r, float &g, float &b);

/*! \sa  Class AColor, Structure BMM_Color_24, Structure BMM_Color_32, Structure BMM_Color_48, Structure BMM_Color_64, Structure BMM_Color_fl, Structure RealPixel.\n\n
\par Description:
This class represents color as three floating point values: r, g, and b. All
methods of this class are implemented by the system.
\par Data Members:
<b>float r,g,b;</b>\n\n
These values are in the range 0.0 to 1.0.  */
class Color : public MaxHeapOperators {
public:
	//! These values are in the range 0.0 to 1.0.
	float r;
	//! These values are in the range 0.0 to 1.0.
	float g;
	//! These values are in the range 0.0 to 1.0.
	float b;

	// Constructors
	/*! \remarks Constructor. The resulting object should be initialized with
	one of the initialization methods. */
	Color()  {}
	/*! \remarks Constructor. Initializes the Color to the RGB color values
	passed.
	\par Parameters:
	<b>float R</b>\n\n
	Specifies the red component of the color.\n\n
	<b>float G</b>\n\n
	Specifies the green component of the color.\n\n
	<b>float B</b>\n\n
	Specifies the blue component of the color. */
	Color(float R, float G, float B)  { r = R; g = G; b = B;  }
	/*! \remarks Constructor. Initializes the Color to the RGB color values
	passed.
	\par Parameters:
	<b>double R</b>\n\n
	Specifies the red component of the color.\n\n
	<b>double G</b>\n\n
	Specifies the green component of the color.\n\n
	<b>double B</b>\n\n
	Specifies the blue component of the color. */
	Color(double R, double G, double B) { r = (float)R; g = (float)G; b = (float)B; }
	/*! \remarks Constructor. Initializes the Color to the RGB color values
	passed.
	\par Parameters:
	<b>int R</b>\n\n
	Specifies the red component of the color.\n\n
	<b>int G</b>\n\n
	Specifies the green component of the color.\n\n
	<b>int B</b>\n\n
	Specifies the blue component of the color. */
	Color(int R, int G, int B) { r = (float)R; g = (float)G; b = (float)B; }
	/*! \remarks Constructor. Initializes the Color to the Color passed.
	\par Parameters:
	<b>Color\& a</b>\n\n
	Specifies the initial color. */
	Color(const Color& a) { r = a.r; g = a.g; b = a.b; } 
	/*! \remarks Constructor. Initializes the color to the Windows RGB value.
	\par Parameters:
	<b>DWORD rgb</b>\n\n
	Specifies the initial color via a Windows RGB value. */
	GEOMEXPORT explicit Color(DWORD rgb);  // from Windows RGB value
	/*! \remarks Constructor. Initializes the Color to the value of the Point3
	passed.
	\par Parameters:
	<b>Point3 p</b>\n\n
	Specifies the color. r=x, g=y, b=z. */
	Color(Point3 p) { r = p.x; g = p.y; b = p.z; }
	/*! \remarks Constructor. Initializes the color to the value passed.
	\par Parameters:
	<b>float af[3]</b>\n\n
	Specifies the color. r=af[0], g=af[1], b=af[2]. */
	Color(float af[3]) { r = af[0]; g = af[1]; b = af[2]; }
	/*! \remarks Constructor. Initializes the color to the RealPixel structure
	passed.
	\par Parameters:
	<b>RealPixel rp</b>\n\n
	Specifies the RealPixel format to convert. */
	Color(RealPixel rp) { ExpandRealPixel(rp,r,g,b); } 
	/*! \remarks	Constructor. Initializes this Color from the 24 bit color value passed.
	\par Parameters:
	<b>const BMM_Color_24\& c</b>\n\n
	The 24 bit color to initialize from. */
	Color(const BMM_Color_24& c) { 
		r = float(c.r)/255.0f; g = float(c.g)/255.0f; b = float(c.b)/255.0f;  
		}
	/*! \remarks	Constructor. Initializes this Color from the 32 bit color value passed.
	\par Parameters:
	<b>const BMM_Color_32\& c</b>\n\n
	The 32 bit color to initialize from. */
	Color(const BMM_Color_32& c) { 
		r = float(c.r)/255.0f; g = float(c.g)/255.0f; b = float(c.b)/255.0f;  
		}
	/*! \remarks	Constructor. Initializes this Color from the 48 bit color value passed.
	\par Parameters:
	<b>const BMM_Color_48\& c</b>\n\n
	The 48 bit color to initialize from. */
	Color(const BMM_Color_48& c) { 
		r = float(c.r)/65535.0f; g = float(c.g)/65535.0f; b = float(c.b)/65535.0f;  
		}
	/*! \remarks	Constructor. Initializes this Color from the 64 bit color value passed.
	\par Parameters:
	<b>const BMM_Color_64\& c</b>\n\n
	The 64 bit color to initialize from. */
	Color(const BMM_Color_64& c) { 
		r = float(c.r)/65535.0f; g = float(c.g)/65535.0f; b = float(c.b)/65535.0f;  
		}
	/*! \remarks	Constructor. Initializes this Color from the floating point color passed.
	\par Parameters:
	<b>const BMM_Color_fl\& c</b>\n\n
	The floating point color to initialize from. No conversion or scaling is
	done. */
	Color(const BMM_Color_fl& c) { 
		r = c.r; g = c.g; b = c.b;  
		}
	
		
	/*! \remarks Sets the Color to black. r = g = b = 0.0f */
	void Black() { r = g = b = 0.0f; }
	/*! \remarks Sets the Color to white. r = g = b = 1.0f */
	void White() { r = g = b = 1.0f; }

	/*! \remarks Makes all the components of the Color \<= 1.0 */
	GEOMEXPORT void ClampMax();  // makes components >= 0.0
	/*! \remarks Makes all the components of the Color \>= 0.0 */
	GEOMEXPORT void ClampMin();  // makes components <= 1.0
    /*! \remarks Makes all the components fall in the range [0,1] */
    GEOMEXPORT void ClampMinMax();  // makes components in [0,1]

	// Access operators
	/*! \remarks Access operator.
	\par Parameters:
	<b>int i</b>\n\n
	The index of the component to return.
	\return  0=r, 1=g, 2=b. */
	float& operator[](int i) { return (&r)[i]; }     
	/*! \remarks Access operator.
	\par Parameters:
	<b>int i</b>\n\n
	The index of the component to return.
	\return  0=r, 1=g, 2=b. */
	const float& operator[](int i) const { return (&r)[i]; }  

	// Conversion function
	/*! \remarks Returns a pointer to the red component of the color. This may
	be used to treat the Color as an array of three floats. */
	operator float*() { return(&r); }
	operator const float*() const { return(&r); }

	// Convert to Windows RGB
//	operator DWORD() { return RGB(FLto255(r),FLto255(g), FLto255(b)); }
	DWORD toRGB() { return RGB(FLto255(r),FLto255(g), FLto255(b)); };

	// Convert to Point3
	/*! \remarks Convert the Color to a Point3. x=r, y=g, z=b. */
	operator Point3() { return Point3(r,g,b); }

	// Convert to RealPixel
	/*! \remarks Convert the Color to the RealPixel format. */
	GEOMEXPORT operator RealPixel();

	// Convert to Bitmap Manager types
	/*! \remarks	Converts this Color to the BMM_Color_24 format.  */
	operator BMM_Color_24() { 
		BMM_Color_24 c; 
		c.r = (BYTE) int(r*255.0f); c.g = (BYTE)int(g*255.0f); c.b = (BYTE) int(b*255.0f);
		return c;
		}
	/*! \remarks	Converts this Color to the BMM_Color_32 format. */
	operator BMM_Color_32() { 
		BMM_Color_32 c; 
		c.r = (BYTE)int(r*255.0f); c.g = (BYTE)int(g*255.0f); c.b = (BYTE)int(b*255.0f);
		return c;
		}
	/*! \remarks	Converts this Color to the BMM_Color_48 format. */
	operator BMM_Color_48() { 
		BMM_Color_48 c; 
		c.r = (WORD)int(r*65535.0f); c.g = (WORD)int(g*65535.0f); c.b = (WORD)int(b*65535.0f);
		return c;
		}
	/*! \remarks	Converts this Color to the BMM_Color_64 format. */
	operator BMM_Color_64() { 
		BMM_Color_64 c; 
		c.r = (WORD)int(r*65535.0f); c.g = (WORD)int(g*65535.0f); c.b = (WORD)int(b*65535.0f);
		return c;
		}
	/*! \remarks	Converts this Color to the BMM_Color_fl format. */
	operator BMM_Color_fl() { 
		BMM_Color_fl c; 
		c.r = r; c.g = g; c.b = b;
		return c;
		}

	// Unary operators
	/*! \remarks Unary - operator.
	\return  The Color with the components negated, i.e.\n\n
	<b>{ return(Color(-r,-g,-b)); }</b> */
	Color operator-() const { return(Color(-r,-g,-b)); } 
	/*! \remarks Unary + operator.
	\return  The Color itself. */
	Color operator+() const { return *this; } 

	// Assignment operators
	/*! \remarks Subtracts a Color from this Color.
	\return  A Color that is the difference between two Colors. */
	inline Color& operator-=(const Color&);
    /*! \remarks Adds a Color to this Color.
    \return  A Color that is the sum of two Colors. */
    inline Color& operator+=(const Color&);
	/*! \remarks Multiplies the components of this Color by a float.
	\return  A Color multiplied by a float. */
	inline Color& operator*=(float); 
	/*! \remarks Divides the components of a Color by a float.
	\return  A Color divided by a float. */
	inline Color& operator/=(float);
	/*! \remarks Performs element-by-element multiplying between two Colors.
	\return  A Color element-by-element multiplied by another Color. */
	inline Color& operator*=(const Color&);	// element-by-element multiplg.

	// Test for equality
	/*! \remarks Test for equality between two Colors.
	\return  Nonzero if the Colors are equal; otherwise 0. */
	int operator==(const Color& p) const { return ((p.r==r)&&(p.g==g)&&(p.b==b)); }
	/*! \remarks Tests for inequality between two Colors.
	\return  Nonzero if the Colors are not equal; otherwise 0. */
	int operator!=(const Color& p) const { return ((p.r!=r)||(p.g!=g)||(p.b!=b)); }

	// Binary operators
	/*! \remarks Subtracts a Color from a Color.
	\return  A Color that is the difference between two Colors. */
	inline Color operator-(const Color&) const;
	/*! \remarks Adds a Color to a Color.
	\return  A Color that is the difference between two Colors. */
	inline Color operator+(const Color&) const;
	/*! \remarks Divides a Color by a Color.
	\return  A Color divided by a Color. r/r, g/g, b/b. */
	inline Color operator/(const Color&) const;
	/*! \remarks Multiplies a Color by a Color.
	\return  A Color multiplied by a Color. r*r, g*g, b*b. */
	inline Color operator*(const Color&) const;   
	inline Color operator^(const Color&) const;   // CROSS PRODUCT
};

/*! \remarks Returns the index of the component with the maximum absolute
value.
\par Parameters:
<b>const Color\&</b>\n\n
The color to check.
\return  The index of the component with the maximum absolute value. r=0, g=1,
b=2. */
GEOMEXPORT int MaxComponent(const Color&);  // index of the component with the maximum abs value
/*! \remarks Returns the index of the component with the minimum absolute
value
\par Parameters:
<b>const Color\&</b>\n\n
The color to check.
\return  The index of the component with the minimum absolute value. r=0, g=1,
b=2. */
GEOMEXPORT int  MinComponent(const Color&);  // index of the component with the minimum abs value

/*! \remarks Returns the value of the component with the maximum absolute
value.
\par Parameters:
<b>const Color\&</b>\n\n
The color to check.
\return  The value of the component with the maximum absolute value. */
GEOMEXPORT float  MaxVal(const Color&);  // value of the component with the maximum abs value
/*! \remarks The value of the component with the minimum absolute value.
\par Parameters:
<b>const Color\&</b>\n\n
The color to check.
\return */
GEOMEXPORT float MinVal(const Color&);  // value of the component with the minimum abs value

// Inlines:

/*! \remarks Returns the 'length' of the color, i.e.\n\n
<b> return (float)sqrt(v.r*v.r+v.g*v.g+v.b*v.b);</b>
\par Parameters:
<b>const Color\& v</b>\n\n
The color to return the length of.
\return  The length of the color.
\par Operators:
*/
inline float Length(const Color& v) {	
	return (float)sqrt(v.r*v.r+v.g*v.g+v.b*v.b);
	}

inline Color& Color::operator-=(const Color& a) {	
	r -= a.r;	g -= a.g;	b -= a.b;
	return *this;
	}

inline Color& Color::operator+=(const Color& a) {
	r += a.r;	g += a.g;	b += a.b;
	return *this;
	}

inline Color& Color::operator*=(float f) {
	r *= f;   g *= f;	b *= f;
	return *this;
	}

inline Color& Color::operator/=(float f) { 
	r /= f;	g /= f;	b /= f;	
	return *this; 
	}

inline Color& Color::operator*=(const Color& a) { 
	r *= a.r;	g *= a.g;	b *= a.b;	
	return *this; 
	}

inline Color Color::operator-(const Color& c) const {
	return(Color(r-c.r,g-c.g,b-c.b));
	}

inline Color Color::operator+(const Color& c) const {
	return(Color(r+c.r,g+c.g,b+c.b));
	}

inline Color Color::operator/(const Color& c) const {
	return Color(r/c.r,g/c.g,b/c.b);
	}

inline Color Color::operator*(const Color& c) const {  
	return Color(r*c.r, g*c.g, b*c.b);
	}

inline Color Color::operator^(const Color& c) const {
	return Color(g * c.b - b * c.g, b * c.r - r * c.b, r * c.g - g * c.r);
	}

inline Color operator*(float f, const Color& a) {
	return(Color(a.r*f, a.g*f, a.b*f));
	}

inline Color operator*(const Color& a, float f) {
	return(Color(a.r*f, a.g*f, a.b*f));
	}

inline Color operator/(const Color& a, float f) {
	return(Color(a.r/f, a.g/f, a.b/f));
	}

inline Color operator+(const Color& a, float f) {
	return(Color(a.r+f, a.g+f, a.b+f));
	}

inline Color operator+(float f, const Color& a) {
	return(Color(a.r+f, a.g+f, a.b+f));
	}

inline Color operator-(const Color& a, float f) {
	return(Color(a.r-f, a.g-f, a.b-f));
	}

inline Color operator-(float f, const Color& a) {
	return(Color(f-a.r, f-a.g, f-a.b));
	}


/*! \sa  Class Color.
\remarks This structure is available in release 4.0 and later only. \n\n
This structure is a 32 bit pixel format that stores 1 bit for sign, 
15 bits for the log of the luminance, and 16 bits of chroma. \n\n
This class stores colors in XYZ space. XYZ color space is the space 
define by the CIE by the red (X), green (Y) and blue (Z) response curves 
of the eye. So to calculate a color in XYZ space, you take the incoming light, 
multiply it by each response curvey and integrate the result over the visible 
spectrum. There are several RGB spaces, all depending on what XYZ coordinates 
get assigned to the red, green and blue primaries of the space. The transformations 
between XYZ and RGB space are all linear and can be represented as 3 by 3 matrices. \n\n
The mapping used by XYZtoRGB and RGBtoXYZ is for CCIR-709 primaries and was 
taken from the code in the tiff reader for the LogLUV32 format. Both XYZ and 
RGB methods are supplied, so developers can supply different XYZ to RGB 
transforms, if desired. \n\n
This transform is important, because in the LogLUV32 format the log is taken 
of the Y coordinate in XYZ space. So, it is important that Y not be 0. 
The transform helps guarantee this. In fact, Y is zero only when r, g 
and b are all 0.
*/
struct LogLUV32Pixel: public MaxHeapOperators {
	/*! Storage for the pixel value. */
	DWORD32		value;

	/*! \remarks This method will return the pixel format as a Color.
	*/
	operator Color() const { Color c; GetRGB(c); return c; }
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const float c[3]</b>\n
	The array of color values to assign in RGB order.
	*/
	LogLUV32Pixel& operator=(const float c[3]) { SetRGB(c); return *this; }

	/*! \remarks Retrieves the RGB space values.
	\par Parameters:
	<b>float rgb[3] </b>\n
	The results are stored here.
	*/
	GEOMEXPORT void GetRGB(float rgb[3]) const;
	/*! \remarks Sets the RGB space values.
	\par Parameters:
	<b>const float rgb[3]</b>\n
	The values to set.
	*/
	GEOMEXPORT void SetRGB(const float rgb[3]);

	/*! \remarks Retrieves the XYZ space values.
	\par Parameters:
	<b>const float xyz[3]</b>\n
	The results are stored here.
	*/
	GEOMEXPORT void GetXYZ(float xyz[3]) const;
	/*! \remarks Sets the XYZ space values.
	\par Parameters:
	<b>const float xyz[3]</b>\n
	The values to set.
	*/
	GEOMEXPORT void SetXYZ(const float xyz[3]);

	/*! \remarks This method converts from XYZ space to RGB space.
	\par Parameters:
	<b>const float xyz[3]</b>\n
	The input values to convert.\n\n
	<b>float rgb[3]</b>\n
	The output values are stored here.
	*/
	GEOMEXPORT static void XYZtoRGB(const float xyz[3], float rgb[3]);
	/*! \remarks This method converts from RGB space to XYZ space.
	\par Parameters:
	<b>const float rgb[3]</b>\n
	The input values to convert.\n\n
	<b>float xyz[3]</b>\n
	The output values are stored here.
	*/
	GEOMEXPORT static void RGBtoXYZ(const float rgb[3], float xyz[3]);
};

/*! \sa Class Color, Structure LogUV32Pixel
\remarks This class is available in release 4.0 and later only. \n
This structure is a 24 bit pixel format that stores 10 bits for log of luminance and 14 bits of chroma.
*/
struct LogLUV24Pixel: public MaxHeapOperators {
	/*! Storage for the pixel value. */
	unsigned char	value[3];

	/*! \remarks This method will return the pixel format as a Color.
	*/
	operator Color() const { Color c; GetRGB(c); return c; }
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const float c[3]</b>\n
	The array of color values to assign in RGB order.
	*/
	LogLUV24Pixel& operator=(const float c[3]) { SetRGB(c); return *this; }

	/*! \remarks This method will return the RGB space values.
	\par Parameters:
	<b>float rgb[3]</b>\n
	The results are stored in this array.
	*/
	GEOMEXPORT void GetRGB(float rgb[3]) const;
	/*! \remarks This method allows you to set the RGB space values.
	\par Parameters:
	<b>const float rgb[3]</b>\n
	The values to set.
	*/
	GEOMEXPORT void SetRGB(const float rgb[3]);

	/*! \remarks This method will return the XYZ space values.
	\par Parameters:
	<b>float xyz[3]</b>\n
	The values are stored in this array.
	*/
	GEOMEXPORT void GetXYZ(float xyz[3]) const;
	/*! \remarks This method allows you to set the XYZ space values.
	\par Parameters:
	<b>const float xyz[3]</b>\n
	The values to set.
	*/
	GEOMEXPORT void SetXYZ(const float xyz[3]);

	/*! \remarks This method will convert from XYZ space to RGB space.
	\par Parameters:
	<b>const float xyz[3]</b>\n
	The input values to convert.\n\n
	<b>float rgb[3]</b>\n
	The output values are stored in this array.
	*/
	GEOMEXPORT static void XYZtoRGB(const float xyz[3], float rgb[3]);
	/*! \remarks This method will convert from RGB space to XYZ space.
	\par Parameters:
	<b>const float rgb[3] </b>\n
	The input values to convert.\n\n
	<b>float xyz[3]</b>\n
	The output values are stored in this array.
	*/
	GEOMEXPORT static void RGBtoXYZ(const float rgb[3], float xyz[3]);
};



