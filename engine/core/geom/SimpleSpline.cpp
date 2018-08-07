#include "SimpleSpline.h"
#include "engine/core/math/Vector4.h"

namespace Echo
{
	SimpleSpline::SimpleSpline()
	{
		mCoeffs.m00 = 2;
		mCoeffs.m01 = -2;
		mCoeffs.m02 = 1;
		mCoeffs.m03 = 1;
		mCoeffs.m10 = -3;
		mCoeffs.m11 = 3;
		mCoeffs.m12 = -2;
		mCoeffs.m13 = -1;
		mCoeffs.m20 = 0;
		mCoeffs.m21 = 0;
		mCoeffs.m22 = 1;
		mCoeffs.m23 = 0;
		mCoeffs.m30 = 1;
		mCoeffs.m31 = 0;
		mCoeffs.m32 = 0;
		mCoeffs.m33 = 0;

		mAutoCalc = true;
	}

	SimpleSpline::~SimpleSpline()
	{
	}

	void SimpleSpline::addPoint(const Vector3& p)
	{
		mPoints.push_back(p);
		if (mAutoCalc)
		{
			recalcTangents();
		}
	}
	void SimpleSpline::delPoint(int index)
	{
		vector<Vector3>::type::iterator itPoint = mPoints.begin();
		while (index--) ++itPoint;

		mPoints.erase(itPoint);
	}

	Vector3 SimpleSpline::interpolate(Real t) const
	{
		// Work out which segment this is in
		Real fSeg = t * (mPoints.size() - 1);
		unsigned int segIdx = (unsigned int)fSeg;
		// Apportion t 
		t = fSeg - segIdx;

		return interpolate(segIdx, t);

	}

	Vector3 SimpleSpline::interpolate(unsigned int fromIndex, Real t) const
	{
		// Bounds check
		EchoAssert(fromIndex < mPoints.size() && "fromIndex out of bounds");

		if ((fromIndex + 1) == mPoints.size())
		{
			// Duff request, cannot blend to nothing
			// Just return source
			return mPoints[fromIndex];
		}

		// Fast special cases
		if (t == 0.0f)
		{
			return mPoints[fromIndex];
		}
		else if(t == 1.0f)
		{
			return mPoints[fromIndex + 1];
		}

		// Real interpolation
		// Form a vector of powers of t
		Real t2, t3;
		t2 = t * t;
		t3 = t2 * t;
		Vector4 powers(t3, t2, t, 1);


		// Algorithm is ret = powers * mCoeffs * Matrix4(point1, point2, tangent1, tangent2)
		const Vector3& point1 = mPoints[fromIndex];
		const Vector3& point2 = mPoints[fromIndex+1];
		const Vector3& tan1 = mTangents[fromIndex];
		const Vector3& tan2 = mTangents[fromIndex+1];
		Matrix4 pt;

		pt.m00 = point1.x;
		pt.m01 = point1.y;
		pt.m02 = point1.z;
		pt.m03 = 1.0f;
		pt.m10 = point2.x;
		pt.m11 = point2.y;
		pt.m12 = point2.z;
		pt.m13 = 1.0f;
		pt.m20 = tan1.x;
		pt.m21 = tan1.y;
		pt.m22 = tan1.z;
		pt.m23 = 1.0f;
		pt.m30 = tan2.x;
		pt.m31 = tan2.y;
		pt.m32 = tan2.z;
		pt.m33 = 1.0f;

		Vector4 ret = powers * mCoeffs * pt;

		return Vector3(ret.x, ret.y, ret.z);
	}

	void SimpleSpline::recalcTangents(void)
	{
		// Catmull-Rom approach
		// 
		// tangent[i] = 0.5 * (point[i+1] - point[i-1])
		//
		// Assume endpoint tangents are parallel with line with neighbour

		size_t i, numPoints;
		bool isClosed;

		numPoints = mPoints.size();
		if (numPoints < 2)
		{
			// Can't do anything yet
			return;
		}

		// Closed or open?
		if (mPoints[0] == mPoints[numPoints-1])
		{
			isClosed = true;
		}
		else
		{
			isClosed = false;
		}

		mTangents.resize(numPoints);



		for(i = 0; i < numPoints; ++i)
		{
			if (i ==0)
			{
				// Special case start
				if (isClosed)
				{
					// Use numPoints-2 since numPoints-1 is the last point and == [0]
					mTangents[i] = 0.5 * (mPoints[1] - mPoints[numPoints-2]);
				}
				else
				{
					mTangents[i] = 0.5 * (mPoints[1] - mPoints[0]);
				}
			}
			else if (i == numPoints-1)
			{
				// Special case end
				if (isClosed)
				{
					// Use same tangent as already calculated for [0]
					mTangents[i] = mTangents[0];
				}
				else
				{
					mTangents[i] = 0.5 * (mPoints[i] - mPoints[i-1]);
				}
			}
			else
			{
				mTangents[i] = 0.5 * (mPoints[i+1] - mPoints[i-1]);
			}

		}
	}

	const Vector3& SimpleSpline::getPoint(unsigned short index) const
	{
		EchoAssert(index < mPoints.size() && "Point index is out of bounds!!");

		return mPoints[index];
	}

	unsigned short SimpleSpline::getNumPoints(void) const
	{
		return (unsigned short)mPoints.size();
	}

	void SimpleSpline::clear(void)
	{
		mPoints.clear();
		mTangents.clear();
	}

	void SimpleSpline::updatePoint(unsigned short index, const Vector3& value)
	{
		EchoAssert(index < mPoints.size() && "Point index is out of bounds!!");

		mPoints[index] = value;
		if (mAutoCalc)
		{
			recalcTangents();
		}
	}

	void SimpleSpline::setAutoCalculate(bool autoCalc)
	{
		mAutoCalc = autoCalc;
	}
}
