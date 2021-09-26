#include <gtest/gtest.h>
#include <engine/core/camera/Camera.h>

TEST(OrthoCamera, depthIsLinear)
{
	//Echo::Vector3 vCam2DPos(0, 0, 0);
	//Echo::Vector3 vCam2DDir = -Echo::Vector3::UNIT_Z;

	//Echo::Camera orthoCamera(Echo::Camera::PM_ORTHO);
	//orthoCamera.setPosition(vCam2DPos);
	//orthoCamera.setDirection(vCam2DDir);
	//orthoCamera.setNearClip(-256.f);
	//orthoCamera.setFarClip(256.0f);
	//orthoCamera.update();

	//for (int i = 0; i < 1000; i++)
	//{
	//	float lenght = i / 1000.f * 512.f;
	//	Echo::Vector3 pos = orthoCamera.getPosition() + orthoCamera.getDirection() * lenght;

	//	orthoCamera.getViewProjMatrix();
	//	Echo::Vector4 outPos;
	//	Echo::Matrix4::TransformVec4(outPos, Echo::Vector4(pos.x, pos.y, pos.z, 1.f), orthoCamera.getViewProjMatrix());

	//	outPos = outPos / outPos.w;

	//	float u = outPos.x;
	//	float v = outPos.y;
	//	float depth = outPos.z;

	//	float normailzedLength = i / 1000.f;

	//	// find the relation between "normalizedLength" with "depth"
	//	int a = 10;
	//}
}

TEST(FishEyeCamera, polynomialSolve)
{
#define FISHEYE_EQUIDISTANCE_PROJECTION

#ifdef FISHEYE_EQUISOLID_ANGLE_PROJECTION
	// Equisolid angle projection (r=2fsin(¦È/2)) https://www.researchgate.net/publication/6899685_A_Generic_Camera_Model_and_Calibration_Method_for_Conventional_Wide-Angle_and_Fish-Eye_Lenses
	// General Polynoimal https://image.hanspub.org/Html/3-2840246_31449.htm
	double k1 = 1.0;
	double k2 = -1.0 / 24.0;
	double k3 = 1.0 / 1920.0;
	double k4 = -2.0 / 645120.0;
	double k5 = 0.0;
#elif defined(FISHEYE_ORTHOGONAL_PROJECTION)
	// orthogonal projection
	double k1 = 1.0;
	double k2 = -1.0 / 6.0;
	double k3 = 1.0 / 120.0;
	double k4 = -1.0 / 5040.0;
	double k5 = 0.0;
#elif defined(FISHEYE_EQUIDISTANCE_PROJECTION)
	double k1 = 1.0;
	double k2 = 0.0;
	double k3 = 0.0;
	double k4 = 0.0;
	double k5 = 0.0;
#endif

	auto polynomialEquisolidAngleProjectionFun = [&](double r, double theta)
	{
		// k1 * ¦È + k2 * ¦È ^ 3 + k3 * ¦È ^ 5 + k4 * ¦È ^ 7 + k5 * ¦È ^ 9 - r = 0.0
		return
			k1 * theta +
			k2 * pow(theta, 3) +
			k3 * pow(theta, 5) +
			k4 * pow(theta, 7) +
			k5 * pow(theta, 9) -
			r;
	};

	auto polynomialEquisolidAngleProjectionDerivativeFun = [&](double theta)
	{
		// k1 + 3 * k2 * ¦È ^ 2 + 5 * k3 * ¦È ^ 4 + 7 * k4 * ¦È ^ 6 + 9 * k5 * ¦È ^ 8
		return
			k1 +
			3 * k2 * pow(theta, 2) +
			5 * k3 * pow(theta, 4) +
			7 * k4 * pow(theta, 6) +
			9 * k5 * pow(theta, 8);
	};

	// https://en.wikipedia.org/wiki/Newton%27s_method
	auto methodOfNewtonFun = [&](double r)
	{
		double theta = Echo::Math::PI_DIV4;
		double rNow = polynomialEquisolidAngleProjectionFun(r, theta);
		int iteratorCount = 0;

		while (std::abs(rNow) > std::abs(1e-6))
		{
			theta = theta - rNow / polynomialEquisolidAngleProjectionDerivativeFun(theta);
			rNow = polynomialEquisolidAngleProjectionFun(r, theta);

			iteratorCount++;
		}

		EXPECT_LE(iteratorCount, 32);

		return theta;
	};

	double theta0 = methodOfNewtonFun(0.0) * Echo::Math::RAD2DEG;
	double theta1 = methodOfNewtonFun(3.0) * Echo::Math::RAD2DEG;
	double theta2 = methodOfNewtonFun(0.99999) * Echo::Math::RAD2DEG;
	double theta3 = methodOfNewtonFun(0.5) * Echo::Math::RAD2DEG;
	double theta4 = methodOfNewtonFun(0.70711) * Echo::Math::RAD2DEG;
	double theta5 = methodOfNewtonFun(0.866) * Echo::Math::RAD2DEG;
	double theta6 = methodOfNewtonFun(Echo::Math::PI_DIV2) * Echo::Math::RAD2DEG;
	double theta7 = methodOfNewtonFun(Echo::Math::PI_DIV4) * Echo::Math::RAD2DEG;

	int breakPoint;
}