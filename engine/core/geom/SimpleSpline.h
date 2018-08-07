#pragma once

#include "engine/core/math/Matrix4.h"

namespace Echo
{
	class SimpleSpline
	{
	public:
		SimpleSpline();
		~SimpleSpline();

		void addPoint(const Vector3& p);
		void delPoint(int index);
		const Vector3& getPoint(unsigned short index) const;
		unsigned short getNumPoints(void) const;
		void clear(void);
		void updatePoint(unsigned short index, const Vector3& value);
		Vector3 interpolate(Real t) const;
		Vector3 interpolate(unsigned int fromIndex, Real t) const;
		void setAutoCalculate(bool autoCalc);
		void recalcTangents(void);

	protected:
		bool mAutoCalc;
		vector<Vector3>::type mPoints;
		vector<Vector3>::type mTangents;
		Matrix4 mCoeffs;
	};
}
