/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

 
#ifndef __NX_GAUSS_H__
#define __NX_GAUSS_H__

#include <PxSimpleTypes.h>
#include <float.h>
#include <math.h>
#include <string.h>

/*! \file */

namespace physx
{
namespace apex
{

/**
\class NxGauss
\brief A class which manages a Gaussian or linear random distribution as if it were a single numeric value.

The NxGauss class represents a random distribution expressed as mean, standard deviation, min and max values as
well as whether it should be sampled as a linear or Gaussian distribution.  A linear distribution simply returns a random
value between the standard deviation and clamps the result to min/max.  A Gaussian distribution conforms to the
classic bell curve shape and can produce values approaching either positive or negative infinity, which makes the clamped
min/max values important.

http://en.wikipedia.org/wiki/Gaussian_function
http://en.wikipedia.org/wiki/Normal_distribution
http://www.bearcave.com/misl/misl_tech/wavelets/hurst/random.html

The ASCII representation of a Gaussian distribution is very straight forward.

It is in the following format:

mean:stdev<min:max>

You can use either ':' or ',' as a numeric seperator.
You can use either '< >' or '[ ]' as a seperator for min/max values.

Here are examples of Gaussian numbers in ASCII

"4"			: A mean of 4, a standard deviation of zero
"4:2"		: A mean of 4 with a standard deviation of plus or minus 2, min is FLT_MIN and max is FLT_MAX
"4:2<3>		: A mean of 4 with a standard deviation of two with a min value of 3 and a max value of FLT_MAX
"4:2<1:5>   : A mean of 4 with a standard deviation of two, with a min value of 1 and a max value of 5
"4:2[1:5]   : Brackets are also a valid symbol you can use to seperate the min/max values.
"4,2[1,5]   : a comma is also a valid numeric seperator.

The exclaimation point symbol is used to indicate that you want a linear, rather than a Gaussian distribution.

"4:2!"		: A mean of 4, plus or minus 2, with a linear distribution. Technically no min-max is required if you want the range to be from 2-6.
"4:2<3:6>"	: A mean of 4, plus or minut 2, clamped low to 3 and a max of 6 as a linear distribution.

Note, this implementation of computing Gaussian numbers from a random distribution is based on work published by
Dr. Everett F. Carter (see link above).  This method computes two gaussian numbers each time.

A Gaussian number uses a standar linear congruential generator for random numbers.

http://en.wikipedia.org/wiki/Linear_congruential_generator

*/

#if defined(PX_WINDOWS)
#pragma warning(push)
#pragma warning(disable:4996)
#endif

PX_PUSH_PACK_DEFAULT

/**
	The ascii rep stuff currently uses _fcvt which doesn't work on the PS3.
	Some of the methods here may be duplicated in PxAsciiConversion.h
 */
#define NXGAUSS_USE_ASCII_REPRESENTATIONS 0

class NxGauss
{
public:

	/*
	\brief The default constructor for a Gaussian number.  Mean 0, standard deviation 0
	*/
	PX_INLINE NxGauss(void)
	{
		mSeed = 0;
		mMean = 0;
		mStandardDeviation = 0;
		mMin = -PX_MAX_F32;
		mMax = PX_MAX_F32;
		mFlags = 0;
	}

	PX_INLINE NxGauss(physx::PxF32 mean, physx::PxF32 stdev = 0, physx::PxF32 fmin = -PX_MAX_F32, physx::PxF32 fmax = PX_MAX_F32, bool linear = false, physx::PxU32 seed = 0)
	{
		set(mean, stdev, fmin, fmax, linear, seed);
	}

#if NXGAUSS_USE_ASCII_REPRESENTATIONS
	PX_INLINE NxGauss(const char* str)
	{
		atog(str);
	}
#endif

	PX_INLINE void  set(physx::PxF32 mean, physx::PxF32 stdev = 0, physx::PxF32 fmin = -PX_MAX_F32, physx::PxF32 fmax = PX_MAX_F32, bool linear = false, physx::PxU32 seed = 0);
	PX_INLINE void	setSeed(physx::PxU32 seed)
	{
		mSeed = seed;    // set the random number seed.
	}
	PX_INLINE void  setLinearDistribution(void)
	{
		mFlags |= GF_LINEAR;
	};
	PX_INLINE void  setGaussianDistribution(void)
	{
		mFlags &= ~GF_LINEAR;
	};

#if NXGAUSS_USE_ASCII_REPRESENTATIONS
	PX_INLINE void	atog(const char* str); // ASCII to Gaussian conversion.
	PX_INLINE bool	gtoa(char* buffer, physx::PxU32 bufferSize, physx::PxU32 precision = 9) const; // converts the Gaussian into a string, returns false if it couldn't fit the result into the buffer.

	static PX_INLINE bool  addChar(char * &dest, const char* end, char c); // add a character to a destination buffer without overflow, returns false if it overflowed.
	static PX_INLINE bool  getFloatString(physx::PxF32 v, char* dest, physx::PxU32 maxlen, physx::PxU32 precision = 9); // helper method to convert a floating point number to ASCII of a specified precision.  Special cases FLT_MAX and FLT_MIN
	static PX_INLINE physx::PxF32 atof(const char* n, const char** p); // convert an ASCII representation back into a float value. Special case for FLT_MAX and FLT_MIN
	static PX_INLINE bool  ftoa(physx::PxF32 v, char* n, physx::PxU32 maxlen, physx::PxU32 precision = 9); // Converts a float to ASCII of a specific precision and to a destination buffer.
	static PX_INLINE bool  addString(const char* str, char* buffer, physx::PxU32& bindex, physx::PxU32 maxBuffer);
	// Converts a string into its Gaussian components.
	static PX_INLINE physx::PxU32 strtogmd(const char* spec,
	                                       const char** end,
	                                       physx::PxF32& mean,
	                                       physx::PxF32& deviation,
	                                       physx::PxF32& min,
	                                       physx::PxF32& max,
	                                       bool& linear);
#endif

	PX_INLINE physx::PxF32	getSample(void); // sample the random distribution.  This is not a 'const' because sampling it changes the random number seed.
	PX_INLINE physx::PxF32	getMean(void) const
	{
		return mMean;
	};
	PX_INLINE physx::PxF32	getStandardDeviation(void) const
	{
		return mStandardDeviation;
	};
	PX_INLINE physx::PxF32	getMin(void) const
	{
		return mMin;
	};
	PX_INLINE physx::PxF32	getMax(void) const
	{
		return mMax;
	};
	PX_INLINE void  		setMean(physx::PxF32 mean)
	{
		mMean = mean;
		set(mMean, mStandardDeviation, mMin, mMax, isLinearDistribution(), mSeed);
	};
	PX_INLINE void  		setStandardDeviation(physx::PxF32 st)
	{
		mStandardDeviation = st;
		set(mMean, mStandardDeviation, mMin, mMax, isLinearDistribution(), mSeed);
	};
	PX_INLINE void  		setMin(physx::PxF32 m)
	{
		mMin = m;
		set(mMean, mStandardDeviation, mMin, mMax, isLinearDistribution(), mSeed);
	};
	PX_INLINE void  		setMax(physx::PxF32 m)
	{
		mMax = m;
		set(mMean, mStandardDeviation, mMin, mMax, isLinearDistribution(), mSeed);
	};
	PX_INLINE bool  		isLinearDistribution(void) const
	{
		return ((mFlags & GF_LINEAR) ? true : false);
	};
	PX_INLINE bool  		isGaussianDistribution(void) const
	{
		return ((mFlags & GF_LINEAR) ? false : true);
	};
	PX_INLINE physx::PxF32	ranf(void); // returns a random floating point number between 0-1



private:
	enum GaussFlag
	{
		GF_NONE		= 0,
		GF_STDEV	= (1 << 0),    //  true if it has a non-zero standard deviation.
		GF_MIN_MAX	= (1 << 1),  //  true if it has a clamped min/max range.
		GF_LINEAR	= (1 << 2),  // true if it is a linear distribution.
		GF_SECOND	= (1 << 3),  // used internally to designate which gaussian pair to return.
	};


	PX_INLINE bool  hasGaussFlag(GaussFlag f) const
	{
		return ((mFlags & f) ? true : false);
	};

	physx::PxF32	mMean;					// The mean value of the Gassian distribution.
	physx::PxF32	mStandardDeviation;		// The standard deviation of the Gassian distribution.
	physx::PxF32	mMin;					// The clamped minimum value.
	physx::PxF32	mMax;					// The clamped maximum value.
	physx::PxU32   mSeed;					// The current random number seed value.
	physx::PxU32   mFlags;					// Internal private flags describing the state of the Gaussian
	physx::PxF32   mSecondValue;			// Gaussian numbers are generated in pairs, this is the next one to be retreived.
};

#if NXGAUSS_USE_ASCII_REPRESENTATIONS
PX_INLINE void	NxGauss::atog(const char* str) // ASCII to Gaussian conversion.
{
	mFlags = 0;
	bool linear;
	physx::PxF32 m, s, m1, m2;
	strtogmd(str, 0, m, s, m1, m2, linear);
	set(m, s, m1, m2, linear, 0);
}

PX_INLINE bool  NxGauss::addString(const char* str, char* buffer, physx::PxU32& bindex, physx::PxU32 maxBuffer) // Add a string to a buffer without overflow
{
	bool ret = true;

	physx::PxU32 bend = maxBuffer - 1;

	if (bindex < bend)
	{
		while (*str)
		{
			buffer[bindex++] = *str++;
			if (bindex == bend)
			{
				ret = false;
				break;
			}
		}
		buffer[bindex] = 0;
	}
	else
	{
		ret = false;
	}
	return ret;
}

PX_INLINE bool  NxGauss::getFloatString(physx::PxF32 v, char* dest, physx::PxU32 maxlen, physx::PxU32 precision)
{
	bool ret = true;

	char temp[_CVTBUFSIZE];
	if (v == 0)
	{
		temp[0] = '0';
		temp[1] = 0;
	}
	else if (v == 1)
	{
		temp[0] = '1';
		temp[1] = 0;
	}
	else if (v == -1)
	{
		temp[0] = '-';
		temp[1] = '1';
		temp[2] = 0;
	}
	else if (v == -PX_MAX_F32)
	{
		strncpy(temp, "-FLT_MAX", _CVTBUFSIZE);
	}
	else if (v == PX_MAX_F32)
	{
		strncpy(temp, "FLT_MAX", _CVTBUFSIZE);
	}
	else
	{
		char num[_CVTBUFSIZE];
		ftoa(v, num, _CVTBUFSIZE, precision);
		const char* dot = num;
		while (*dot && *dot != '.')
		{
			dot++;
		}
		if (*dot == '.')
		{
			char* scan = (char*)(dot + 1);
			while (*scan)
			{
				scan++;
			}
			scan--;
			while (*scan != '.' && *scan == '0')
			{
				scan--;
			}
			if (*scan == '.')
			{
				*scan = 0;
			}
			else
			{
				scan[1] = 0;
			}
		}
		strncpy(temp, num, _CVTBUFSIZE);
	}
	char* dscan = dest;
	const char* scan  = temp;
	physx::PxU32 icount = 0;
	while (*scan)
	{
		*dscan++ = *scan++;
		icount++;
		if (icount == (maxlen - 1))
		{
			ret = false;
			break;
		}
	}
	*dscan = 0;
	return ret;
}

PX_INLINE physx::PxF32 NxGauss::atof(const char* n, const char** pos)
{
	physx::PxF32 ret = 0;

	if (strcmp(n, "FLT_MAX") == 0)
	{
		ret = PX_MAX_F32;
		pos[0] = n + 7;
	}
	else if (strcmp(n, "FLT_MIN") == 0)
	{
		ret = -PX_MAX_F32;
		pos[0] = n + 7;
	}
	else
	{
		ret = (physx::PxF32)strtod((char*)n, (char**) pos);
	}
	return ret;
}

PX_INLINE bool	NxGauss::gtoa(char* buffer, physx::PxU32 bufferSize, physx::PxU32 precision) const // converts the gaussian into a string, returns false if it couldn't fit the result into the buffer.
{
	physx::PxU32 index = 0;
	buffer[0] = 0;
	char temp[_CVTBUFSIZE];
	getFloatString(mMean, temp, _CVTBUFSIZE, precision);
	bool ret = addString(temp, buffer, index, bufferSize);
	if (hasGaussFlag(GF_STDEV))
	{
		getFloatString(mStandardDeviation, temp, _CVTBUFSIZE, precision);
		ret = addString(":", buffer, index, bufferSize);
		ret = addString(temp, buffer, index, bufferSize);
	}
	if (hasGaussFlag(GF_MIN_MAX))
	{
		char temp1[_CVTBUFSIZE];
		char temp2[_CVTBUFSIZE];
		getFloatString(mMin, temp1, _CVTBUFSIZE, precision);
		getFloatString(mMax, temp2, _CVTBUFSIZE, precision);
		ret = addString("<", buffer, index, bufferSize);
		ret = addString(temp1, buffer, index, bufferSize);
		ret = addString(":", buffer, index, bufferSize);
		ret = addString(temp2, buffer, index, bufferSize);
		ret = addString(">", buffer, index, bufferSize);
	}
	if (hasGaussFlag(GF_LINEAR))
	{
		ret = addString("!", buffer, index, bufferSize);
	}

	return ret;
}

PX_INLINE bool  NxGauss::addChar(char * &dest, const char* end, char c)
{
	bool ret = true;
	if (dest < end)
	{
		*dest++ = c;
	}
	else
	{
		ret = false;
	}
	return ret;
}

PX_INLINE bool  NxGauss::ftoa(physx::PxF32 v, char* n, physx::PxU32 maxlen, physx::PxU32 precision)
{
	bool ret = true;

	int decimal, sign;
	const char* temp = _fcvt((float)v, precision, &decimal, &sign);
	char* num = n;
	const char* end = n + maxlen - 1;
	if (sign)
	{
		ret = addChar(num, end, '-');
	}
	if (decimal <= 0)
	{
		ret = addChar(num, end, '0');
		ret = addChar(num, end, '.');
		for (int i = decimal; i < 0; i++)
		{
			addChar(num, end, '0');
		}
		decimal = -1;
	}
	for (int i = 0; i < _CVTBUFSIZE; i++)
	{
		if (temp[i] == 0)
		{
			break;
		}
		if (i == decimal)
		{
			ret = addChar(num, end, '.');
		}
		ret = addChar(num, end, temp[i]);
	}

	*num = 0;

	return ret;
}

// convert string to gaussian number.  Return code
// indicates number of arguments found.
PX_INLINE physx::PxU32 NxGauss::strtogmd(const char* _spec,
        const char** end,
        physx::PxF32& mean,
        physx::PxF32& deviation,
        physx::PxF32& vmin,
        physx::PxF32& vmax,
        bool&  linear)
{
	physx::PxU32 ret = 0;
	const char* pos;
	vmin  = -PX_MAX_F32; // default min
	vmax  = PX_MAX_F32; // default max
	linear = false;    // default is gaussian distribution
	deviation = 0;     // default standard deviation is zero
	mean = 0;
	const char* spec = (const char*)_spec;
	physx::PxF32 v = (physx::PxF32) NxGauss::atof(spec, &pos);  // convert the input string to float
	if (pos != spec)   // if we did not encounter a valid number then...
	{
		mean = v;
		ret++;
		if (*pos == ':' || *pos == ',')  // is the next character the standard deviation seperator?
		{
			spec = pos + 1;
			v  = (physx::PxF32) NxGauss::atof(spec, &pos);
			if (pos != spec)
			{
				deviation = v;
				ret++;
				if (*pos == '<' || *pos == '[')
				{
					spec = pos + 1;
					v  = (physx::PxF32) NxGauss::atof(spec, &pos);
					if (pos != spec)
					{
						vmin = v;
						ret++;
						if (*pos == ',' || *pos == ':')
						{
							spec = pos + 1;
							v = (physx::PxF32) NxGauss::atof(spec, &pos);
							if (pos != spec)
							{
								vmax = v;
								if (*pos == ']' || *pos == '>')
								{
									pos++;
								}
							}
						}
					}
				}
			}
		}
	}

	if (*pos == '!')   // see if the last character is an exclamation mark, indicating a linear distribution
	{
		linear = true;
		pos++;
	}

	if (end != 0)
	{
		*end = pos;
	}

	return ret;
}

#endif /* #if NXGAUSS_USE_ASCII_REPRESENTATIONS */

PX_INLINE physx::PxF32	NxGauss::getSample(void) // sample the random distribution.  This is not a 'const' because sampling it changes the random number seed.
{
	physx::PxF32 ret = 0;

	if (hasGaussFlag(GF_SECOND))
	{
		ret = mSecondValue * mStandardDeviation + mMean;
		mFlags &= ~GF_SECOND;
	}
	else
	{
		if (hasGaussFlag(GF_LINEAR))
		{
			ret = ((ranf() * mStandardDeviation * 2) - mStandardDeviation) + mMean;
		}
		else
		{

			physx::PxF32 x1, x2, w;
			do
			{
				x1 = 2.0f * ranf() - 1.0f;
				x2 = 2.0f * ranf() - 1.0f;
				w = x1 * x1 + x2 * x2;
			}
			while (w >= 1.0f);

			w = sqrtf((-2.0f * logf(w)) / w);

			ret = x1 * w;
			mSecondValue = x2 * w;

			ret = ret * mStandardDeviation + mMean;

			mFlags |= GF_SECOND;
		}
	}

	if (hasGaussFlag(GF_MIN_MAX))  // if we are clamping the results, then apply the limits
	{
		if (ret < mMin)
		{
			ret = mMin;
		}
		if (ret > mMax)
		{
			ret = mMax;
		}
	}

	return ret;
}


PX_INLINE void NxGauss::set(physx::PxF32 mean, physx::PxF32 stdev, physx::PxF32 fmin, physx::PxF32 fmax, bool linear, physx::PxU32 seed)
{
	mMean = mean;
	mStandardDeviation = stdev;
	mMin = fmin;
	mMax = fmax;
	mSeed   = seed;
	mFlags  = 0;
	if (linear)
	{
		mFlags |= GF_LINEAR;
	}
	if (mStandardDeviation != 0)
	{
		mFlags |= GF_STDEV;
	}
	if (mMin != -PX_MAX_F32 || mMax != PX_MAX_F32)
	{
		mFlags |= GF_MIN_MAX;
	}
}


PX_INLINE physx::PxF32 NxGauss::ranf(void) // returns a random floating point number between 0-1
{
	mSeed = (mSeed * 214013L + 2531011L) & 0x7fffffff;
	return (physx::PxF32)(mSeed & 0x7FFF) * (1.0f / 32767.0f);
}

#if defined(PX_WINDOWS)
#pragma warning(pop)
#endif


PX_POP_PACK

}
} // end namespace physx::apex

#endif
