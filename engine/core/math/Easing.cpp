#include "Easing.h"
#include <algorithm>
#include <cmath>
#include <cassert>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 (M_PI / 2)
#endif

namespace Echo
{
	Real easeNone(Real progress)
	{
		return progress;
	}

	Real easeInQuad(Real t)
	{
		return t*t;
	}

	Real easeOutQuad(Real t)
	{
		return -t*(t - 2);
	}

	Real easeInOutQuad(Real t)
	{
		t *= 2.0;
		if (t < 1)
		{
			return t*t / Real(2);
		}
		else
		{
			--t;
			return Real(- 0.5) * (t*(t - 2) - 1);
		}
	}

	Real easeOutInQuad(Real t)
	{
		if (t < 0.5) return easeOutQuad(t * 2) / 2;
		return easeInQuad((2 * t) - 1) / 2 + Real(0.5);
	}

	Real easeInCubic(Real t)
	{
		return t*t*t;
	}

	Real easeOutCubic(Real t)
	{
		t -= 1.0;
		return t*t*t + 1;
	}

	Real easeInOutCubic(Real t)
	{
		t *= 2.0;
		if (t < 1)
		{
			return Real(0.5)*t*t*t;
		}
		else
		{
			t -= Real(2.0);
			return Real(0.5)*(t*t*t + 2);
		}
	}

	Real easeOutInCubic(Real t)
	{
		if (t < 0.5) return easeOutCubic(2 * t) / 2;
		return easeInCubic(2 * t - 1) / 2 + Real(0.5);
	}

	Real easeInQuart(Real t)
	{
		return t*t*t*t;
	}

	Real easeOutQuart(Real t)
	{
		t -= Real(1.0);
		return -(t*t*t*t - 1);
	}

	Real easeInOutQuart(Real t)
	{
		t *= 2;
		if (t < 1)
		{
			return Real(0.5)*t*t*t*t;
		}
		else
		{
			t -= 2.0f;
			return -Real(0.5) * (t*t*t*t - 2);
		}
	}

	Real easeOutInQuart(Real t)
	{
		if (t < 0.5) return easeOutQuart(2 * t) / 2;
		return easeInQuart(2 * t - 1) / 2 + Real(0.5);
	}

	Real easeInQuint(Real t)
	{
		return t*t*t*t*t;
	}

	Real easeOutQuint(Real t)
	{
		t -= 1.0;
		return t*t*t*t*t + 1;
	}

	Real easeInOutQuint(Real t)
	{
		t *= 2.0;
		if (t < 1)
		{
			return Real(0.5)*t*t*t*t*t;
		}
		else
		{
			t -= 2.0;
			return Real(0.5)*(t*t*t*t*t + 2);
		}
	}

	Real easeOutInQuint(Real t)
	{
		if (t < 0.5) return easeOutQuint(2 * t) / 2;
		return easeInQuint(2 * t - 1) / 2 + Real(0.5);
	}

	
	Real easeInSine(Real t)
	{
		return (t == 1.0) ? Real(1.0) : -Real(std::cos(t * M_PI_2)) + Real(1.0);
	}

	Real easeOutSine(Real t)
	{
		return Real(std::sin(t* M_PI_2));
	}

	Real easeInOutSine(Real t)
	{
		return -Real(0.5) * Real(std::cos(M_PI*t) - 1);
	}

	Real easeOutInSine(Real t)
	{
		if (t < 0.5) return easeOutSine(2 * t) / 2;
		return easeInSine(2 * t - 1) / 2 + Real(0.5);
	}

	Real easeInExpo(Real t)
	{
		return (t == 0 || t == 1.0) ? t : Real(std::pow(2.0, 10 * (t - 1))) - Real(0.001);
	}

	Real easeOutExpo(Real t)
	{
		return (t == 1.0) ? Real(1.0) : Real(1.001 * (-std::pow(2.0f, -10 * t) + 1));
	}

	
	Real easeInOutExpo(Real t)
	{
		if (t == 0.0) return Real(0.0);
		if (t == 1.0) return Real(1.0);
		t *= 2.0;
		if (t < 1) return Real(0.5 * std::pow(Real(2.0), 10 * (t - 1)) - 0.0005);
		return Real(0.5 * 1.0005 * (-std::pow(Real(2.0), -10 * (t - 1)) + 2));
	}

	Real easeOutInExpo(Real t)
	{
		if (t < 0.5) return easeOutExpo(2 * t) / 2;
		return easeInExpo(2 * t - 1) / 2 + Real(0.5);
	}

	
	Real easeInCirc(Real t)
	{
		return -(std::sqrt(1 - t*t) - 1);
	}

	Real easeOutCirc(Real t)
	{
		t -= Real(1.0);
		return std::sqrt(1 - t*t);
	}

	Real easeInOutCirc(Real t)
	{
		t *= Real(2.0);
		if (t < 1)
		{
			return -Real(0.5 * (std::sqrt(1 - t*t) - 1));
		}
		else
		{
			t -= Real(2.0);
			return Real(0.5 * (std::sqrt(1 - t*t) + 1));
		}
	}

	Real easeOutInCirc(Real t)
	{
		if (t < 0.5) return easeOutCirc(2 * t) / 2;
		return easeInCirc(2 * t - 1) / 2 + Real(0.5);
	}

	static Real easeInElasticHelper(Real t, Real b, Real c, Real d, Real a, Real p)
	{
		if (t == 0) return b;
		Real t_adj = (Real)t / (Real)d;
		if (t_adj == 1) return b + c;

		Real s;
		if (a < std::fabs(c))
		{
			a = c;
			s = p / 4.0f;
		}
		else
		{
			s = Real(p / (2 * M_PI) * std::asin(c / a));
		}

		t_adj -= 1.0f;
		return -Real(a*std::pow(2.0f, 10 * t_adj) * std::sin((t_adj*d - s)*(2 * M_PI) / p)) + b;
	}

	Real easeInElastic(Real t, Real a, Real p)
	{
		return easeInElasticHelper(t, 0, 1, 1, a, p);
	}

	static Real easeOutElasticHelper(Real t, Real /*b*/, Real c, Real /*d*/, Real a, Real p)
	{
		if (t == 0) return 0;
		if (t == 1) return c;

		Real s;
		if (a < c)
		{
			a = c;
			s = p / 4.0f;
		}
		else
		{
			s = Real(p / (2 * M_PI) * std::asin(c / a));
		}

		return Real(a*std::pow(2.0f, -10 * t) * std::sin((t - s)*(2 * M_PI) / p) + c);
	}

	
	Real easeOutElastic(Real t, Real a, Real p)
	{
		return easeOutElasticHelper(t, 0, 1, 1, a, p);
	}

	Real easeInOutElastic(Real t, Real a, Real p)
	{
		if (t == 0) return 0.0;
		t *= 2.0;
		if (t == 2) return 1.0;

		Real s;
		if (a < 1.0)
		{
			a = 1.0;
			s = p / 4.0f;
		}
		else
		{
			s = Real(p / (2 * M_PI) * std::asin(1.0 / a));
		}

		if (t < 1) return -Real(.5*(a*std::pow(2.0f, 10 * (t - 1)) * std::sin((t - 1 - s)*(2 * M_PI) / p)));
		return Real(a*std::pow(2.0f, -10 * (t - 1)) * std::sin((t - 1 - s)*(2 * M_PI) / p)*.5 + 1.0);
	}

	
	Real easeOutInElastic(Real t, Real a, Real p)
	{
		if (t < 0.5) return easeOutElasticHelper(t * 2, 0, 0.5, 1.0, a, p);
		return easeInElasticHelper(Real(2 * t - 1.0), Real(0.5), Real(0.5), Real(1.0), a, p);
	}

	Real easeInBack(Real t, Real s)
	{
		return t*t*((s + 1)*t - s);
	}

	Real easeOutBack(Real t, Real s)
	{
		t -= Real(1.0);
		return t*t*((s + 1)*t + s) + 1;
	}

	Real easeInOutBack(Real t, Real s)
	{
		t *= 2.0;
		if (t < 1)
		{
			s *= 1.525f;
			return Real(0.5)*(t*t*((s + 1)*t - s));
		}
		else
		{
			t -= 2;
			s *= 1.525f;
			return Real(0.5)*(t*t*((s + 1)*t + s) + 2);
		}
	}

	Real easeOutInBack(Real t, Real s)
	{
		if (t < 0.5) return easeOutBack(2 * t, s) / 2;
		return easeInBack(2 * t - 1, s) / 2 + Real(0.5);
	}

	static Real easeOutBounce_helper(Real t, Real c, Real a)
	{
		if (t == 1.0) return c;
		if (t < (4 / 11.0))
		{
			return Real(c*(7.5625*t*t));
		}
		else if (t < (8 / 11.0))
		{
			t -= Real(6 / 11.0);
			return -Real(a * (1. - (7.5625*t*t + .75))) + c;
		}
		else if (t < (10 / 11.0))
		{
			t -= Real(9 / 11.0);
			return -Real(a * (1. - (7.5625*t*t + .9375))) + c;
		}
		else
		{
			t -= Real(21 / 22.0);
			return -Real(a * (1. - (7.5625*t*t + .984375))) + c;
		}
	}

	Real easeOutBounce(Real t, Real a)
	{
		return easeOutBounce_helper(t, 1, a);
	}

	Real easeInBounce(Real t, Real a)
	{
		return Real(1.0 - easeOutBounce_helper(Real(1.0 - t), Real(1.0), a));
	}

	Real easeInOutBounce(Real t, Real a)
	{
		if (t < 0.5) return easeInBounce(2 * t, a) / 2;
		else return (t == 1.0) ? Real(1.0) : easeOutBounce(2 * t - 1, a) / 2 + Real(0.5);
	}

	Real easeOutInBounce(Real t, Real a)
	{
		if (t < 0.5) return easeOutBounce_helper(t * 2, 0.5, a);
		return Real(1.0) - easeOutBounce_helper(Real(2.0 - 2 * t), Real(0.5), a);
	}

	inline static Real sinProgress(Real value)
	{
		return Real(std::sin((value * M_PI) - M_PI_2)) / 2 + Real(0.5);
	}

	inline static Real smoothBeginEndMixFactor(Real value)
	{
		return (std::min)((std::max)(1 - value * 2 + Real(0.3), Real(0.0)), Real(1.0));
	}

	Real easeInCurve(Real t)
	{
		const Real s = sinProgress(t);
		const Real mix = smoothBeginEndMixFactor(t);
		return s * mix + t * (1 - mix);
	}

	Real easeOutCurve(Real t)
	{
		const Real s = sinProgress(t);
		const Real mix = smoothBeginEndMixFactor(1 - t);
		return s * mix + t * (1 - mix);
	}

	Real easeSineCurve(Real t)
	{
		return Real(std::sin(((t * M_PI * 2)) - M_PI_2) + 1) / 2;
	}

	Real easeCosineCurve(Real t)
	{
		return Real(std::cos(((t * M_PI * 2)) - M_PI_2) + 1) / 2;
	}

	Real easeInExpoHelper(Real intensity, Real t, Real min = 2.f, Real max = 6.f)
	{
		t = (std::min)((std::max)(t, 0.f), 1.f);
		intensity = (std::min)((std::max)(intensity + 1, 2.f), 6.f);
		return Real(std::pow(t, intensity));
	}

	Real easeOutExpoHelper(Real intensity, Real t, Real min = 2.f, Real max = 6.f)
	{
		t = (std::min)((std::max)(1 - t, 0.f), 1.f);
		intensity = (std::min)((std::max)(intensity + 1, 2.f), 6.f);
		return Real(1 - std::pow(t, intensity));
	}

	Real easeFunc(int type, Real intensity, Real t)
	{
		switch (type)
		{
		case 0:  // NONE
			return t;
		case 1:  // IN
			return easeInExpoHelper(intensity, t);
		case 2:  // OUT
			return easeOutExpoHelper(intensity, t);
		case 3:  // IN_OUT
			return t < 0.5f ? easeInExpoHelper(intensity, 2 * t) / 2 : easeOutExpoHelper(intensity, 2 * t - 1) / 2 + 0.5f;
		case 4:  // OUT_IN
			return t < 0.5f ? easeOutExpoHelper(intensity, 2 * t) / 2 : easeInExpoHelper(intensity, 2 * t - 1) / 2 + 0.5f;
		default:
			assert(false);
		}
		return t;
	}
}  // namespace Echo
