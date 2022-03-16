#include <algorithm>
#include <gtest/gtest.h>
#include <engine/core/math/Vector2.h>
#include <engine/core/math/Math.h>

namespace Echo
{
	// https://www.youtube.com/watch?v=hG0p4XgePSA (Hermit Curves)
	// https://www.desmos.com/calculator?lang=zh-CN (Graph Calculator)
	// http://www.tangrui.net/2006/algorithm-and-implementation-of-hermite-curve.html (Hermit Curve)
	struct HermiteCurve
	{
	public:
		HermiteCurve()
		{
			m_length = std::abs(m_p1.x - m_p0.x);

			findCoefficent();
		}

		// Find Coefficient
		void findCoefficent()
		{
			double p0 = m_p0.y;
			double t0 = m_t0.y / m_t0.x;
			double p1 = m_p1.y;
			double t1 = m_t1.y / m_t1.x;

			m_a = m_p0.y;
			m_b = m_t0.y / m_t0.x;
			m_c = -3 * p0 - 2 * t0 + 3 * p1 - t1;
			m_d = 2 * p0 + t0 - 2 * p1 + t1;
		}

		void testControlPoints()
		{
			double p0 = m_a;
			double t0 = m_b;
			double p1 = m_a + m_b + m_c + m_d;
			double t1 = m_b + 2 * m_c + 3 * m_d;

			EXPECT_FLOAT_EQ(p0, m_p0.y);
			EXPECT_FLOAT_EQ(t0, m_t0.y / m_t0.x);
			EXPECT_FLOAT_EQ(p1, m_p1.y);
			EXPECT_FLOAT_EQ(t1, m_t1.y / m_t1.x);
		}

		// Evaluate by control point
		double evaluateByControlPoint(float t)
		{
			double p0 = m_p0.y;
			double t0 = m_t0.y / m_t0.x;
			double p1 = m_p1.y;
			double t1 = m_t1.y / m_t1.x;

			return (2*p0 + t0 - 2 * p1 + t1) * t * t * t + (-3 * p0 - 2 * t0 + 3 * p1 - t1) * t * t + (t0) * t + p0;
		}

		// Evaluate by polynomial
		double evaluateByPolynomial(float t)
		{
			double a = m_a;
			double b = m_b / m_length;
			double c = m_c / (m_length * m_length);
			double d = m_d / (m_length * m_length * m_length);
			double x = t * m_length;

			return a + b * x + c * x * x + d * x * x * x;
		}

		void testEvaluate(float t)
		{
			double a = evaluateByControlPoint(t);
			double b = evaluateByPolynomial(t);

			EXPECT_FLOAT_EQ(a, b);
		}

		// Test
		void test()
		{
			testControlPoints();

			testEvaluate(0);
			testEvaluate(0.1);
			testEvaluate(0.25);
			testEvaluate(0.5);
			testEvaluate(0.6);
			testEvaluate(0.8);
			testEvaluate(1.0);
		}

	protected:
		double   m_length = 0.f;

		Vector2 m_p0 = Vector2(0.f, 0.f);
		Vector2 m_t0 = Vector2(1.f, 1.f);
		Vector2 m_p1 = Vector2(1.f, 1.f);
		Vector2 m_t1 = Vector2(1.f, 1.f);

		double	m_a = 0.0;
		double	m_b = 0.0;
		double	m_c = 0.0;
		double	m_d = 0.0;
	};
}

TEST(Curve, Hermite)
{
	Echo::HermiteCurve hermite;
	hermite.test();
}
