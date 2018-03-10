/*! \file PreciseTimeValue.h
    \brief Class definitions for PreciseTimeValue.
				Though time is stored internall in 3ds max as an integer
				number of ticks, the precision is not sufficient particularly
				for collision detection and particle systems tests.
				The class is used where an additional degree of time pre-
				cision is required.
				The "fraction" member stays in range [-0.5f, 0.5f] if
				there is no direct manipulation with the member.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY:	created 10-17-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "..\maxheap.h"
#include "PFExport.h"

class PFExport PreciseTimeValue: public MaxHeapOperators {
public:
	int tick;
	float fraction;

	// Constructors
	PreciseTimeValue() {}
	PreciseTimeValue(int t, float f) { tick = t; fraction = f; }
	PreciseTimeValue(int t) { tick = t; fraction = 0.0f; }
	PreciseTimeValue(float f);
	PreciseTimeValue(const PreciseTimeValue& v);

    // Data members
    static const PreciseTimeValue Origin;

	// Conversion function
	operator int() { return tick; }
	operator float() { return tick+fraction; }

	// Unary operators
	PreciseTimeValue operator-() const;
	PreciseTimeValue operator+() const { return *this; }

    // Property functions
	int TimeValue() const { return tick; }
	float PreciseTime() const { return tick+fraction; }

	// Assignment operators
	PreciseTimeValue& operator-=(const PreciseTimeValue&);
	PreciseTimeValue& operator-=(const int v) { tick -= v; return *this; }
	PreciseTimeValue& operator-=(const float);
	PreciseTimeValue& operator+=(const PreciseTimeValue&);
	PreciseTimeValue& operator+=(const int v) { tick += v; return *this; }
	PreciseTimeValue& operator+=(const float);
	PreciseTimeValue& operator*=(const int);
	PreciseTimeValue& operator*=(const float);
	PreciseTimeValue& operator/=(const float);

	PreciseTimeValue& Set(int t, float f) { tick=t; fraction=f; return *this; }

	// Test for equality
	int operator==(const PreciseTimeValue& v) const
			{ return ((v.tick==tick)&&(v.fraction==fraction)); }
	int operator!=(const PreciseTimeValue& v) const
			{ return ((v.tick!=tick)||(v.fraction!=fraction)); }
	int Equals(const PreciseTimeValue& v, float epsilon = 1E-6f) const;
	int Equals(int t, float epsilon = 1E-6f) const;
	int Equals(float t, float epsilon = 1E-6f) const;
	int operator>(const PreciseTimeValue& v) const;
	int operator<(const PreciseTimeValue& v) const;
	int operator>=(const PreciseTimeValue& v) const;
	int operator<=(const PreciseTimeValue& v) const;

	// Binary operators
	PreciseTimeValue operator-(const PreciseTimeValue&) const;
	PreciseTimeValue operator-(const int) const;
	PreciseTimeValue operator-(const float) const;
	PreciseTimeValue operator+(const PreciseTimeValue&) const;
	PreciseTimeValue operator+(const int) const;
	PreciseTimeValue operator+(const float) const;
	PreciseTimeValue operator*(const int) const;
	PreciseTimeValue operator*(const float) const;
	PreciseTimeValue operator/(const float) const;

	// internal use
	void AdjustFraction();
};


