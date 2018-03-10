#pragma once
#include "engine/core/Base/TypeDef.h"

namespace Echo
{
	/**
	* Easing equation function for a simple linear tweening, with no easing.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeNone(Real progress);

	/**
	* Easing equation function for a quadratic (t^2) easing in: accelerating from zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInQuad(Real t);

	/**
	* Easing equation function for a quadratic (t^2) easing out: decelerating to zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutQuad(Real t);

	/**
	* Easing equation function for a quadratic (t^2) easing in/out: acceleration until halfway, then deceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInOutQuad(Real t);

	/**
	* Easing equation function for a quadratic (t^2) easing out/in: deceleration until halfway, then acceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutInQuad(Real t);

	/**
	* Easing equation function for a cubic (t^3) easing in: accelerating from zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInCubic(Real t);

	/**
	* Easing equation function for a cubic (t^3) easing out: decelerating to zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutCubic(Real t);

	/**
	* Easing equation function for a cubic (t^3) easing in/out: acceleration until halfway, then deceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInOutCubic(Real t);

	/**
	* Easing equation function for a cubic (t^3) easing out/in: deceleration until halfway, then acceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutInCubic(Real t);

	/**
	* Easing equation function for a quartic (t^4) easing in: accelerating from zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInQuart(Real t);

	/**
	* Easing equation function for a quartic (t^4) easing out: decelerating to zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutQuart(Real t);

	/**
	* Easing equation function for a quartic (t^4) easing in/out: acceleration until halfway, then deceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInOutQuart(Real t);

	/**
	* Easing equation function for a quartic (t^4) easing out/in: deceleration until halfway, then acceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutInQuart(Real t);

	/**
	* Easing equation function for a quintic (t^5) easing in: accelerating from zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInQuint(Real t);

	/**
	* Easing equation function for a quintic (t^5) easing out: decelerating to zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutQuint(Real t);

	/**
	* Easing equation function for a quintic (t^5) easing in/out: acceleration until halfway, then deceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInOutQuint(Real t);

	/**
	* Easing equation function for a quintic (t^5) easing out/in: deceleration until halfway, then acceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutInQuint(Real t);

	/**
	* Easing equation function for a sinusoidal (sin(t)) easing in: accelerating from zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInSine(Real t);

	/**
	* Easing equation function for a sinusoidal (sin(t)) easing out: decelerating to zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutSine(Real t);

	/**
	* Easing equation function for a sinusoidal (sin(t)) easing in/out: acceleration until halfway, then deceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInOutSine(Real t);

	/**
	* Easing equation function for a sinusoidal (sin(t)) easing out/in: deceleration until halfway, then acceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutInSine(Real t);

	/**
	* Easing equation function for an exponential (2^t) easing in: accelerating from zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInExpo(Real t);

	/**
	* Easing equation function for an exponential (2^t) easing out: decelerating to zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutExpo(Real t);

	/**
	* Easing equation function for an exponential (2^t) easing in/out: acceleration until halfway, then deceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInOutExpo(Real t);

	/**
	* Easing equation function for an exponential (2^t) easing out/in: deceleration until halfway, then acceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutInExpo(Real t);

	/**
	* Easing equation function for a circular (sqrt(1-t^2)) easing in: accelerating from zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInCirc(Real t);

	/**
	* Easing equation function for a circular (sqrt(1-t^2)) easing out: decelerating to zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutCirc(Real t);

	/**
	* Easing equation function for a circular (sqrt(1-t^2)) easing in/out: acceleration until halfway, then deceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeInOutCirc(Real t);

	/**
	* Easing equation function for a circular (sqrt(1-t^2)) easing out/in: deceleration until halfway, then acceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @return		The correct value.
	*/
	Real easeOutInCirc(Real t);

	/**
	* Easing equation function for an elastic (exponentially decaying sine wave) easing in: accelerating from zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @param a		Amplitude.
	* @param p		Period.
	* @return		The correct value.
	*/
	Real easeInElastic(Real t, Real a, Real p);

	/**
	* Easing equation function for an elastic (exponentially decaying sine wave) easing out: decelerating to zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @param a		Amplitude.
	* @param p		Period.
	* @return		The correct value.
	*/
	Real easeOutElastic(Real t, Real a, Real p);

	/**
	* Easing equation function for an elastic (exponentially decaying sine wave) easing in/out: acceleration until halfway, then deceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @param a		Amplitude.
	* @param p		Period.
	* @return		The correct value.
	*/
	Real easeInOutElastic(Real t, Real a, Real p);


	/**
	* Easing equation function for an elastic (exponentially decaying sine wave) easing out/in: deceleration until halfway, then acceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @param a		Amplitude.
	* @param p		Period.
	* @return		The correct value.
	*/
	Real easeOutInElastic(Real t, Real a, Real p);

	/**
	* Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing in: accelerating from zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @param s		Overshoot ammount: higher s means greater overshoot (0 produces cubic easing with no overshoot, and the default value of 1.70158 produces an overshoot of 10 percent).
	* @return		The correct value.
	*/
	Real easeInBack(Real t, Real s);

	/**
	* Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing out: decelerating to zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @param s		Overshoot ammount: higher s means greater overshoot (0 produces cubic easing with no overshoot, and the default value of 1.70158 produces an overshoot of 10 percent).
	* @return		The correct value.
	*/
	Real easeOutBack(Real t, Real s);

	/**
	* Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing in/out: acceleration until halfway, then deceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @param s		Overshoot ammount: higher s means greater overshoot (0 produces cubic easing with no overshoot, and the default value of 1.70158 produces an overshoot of 10 percent).
	* @return		The correct value.
	*/
	Real easeInOutBack(Real t, Real s);

	/**
	* Easing equation function for a back (overshooting cubic easing: (s+1)*t^3 - s*t^2) easing out/in: deceleration until halfway, then acceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @param s		Overshoot ammount: higher s means greater overshoot (0 produces cubic easing with no overshoot, and the default value of 1.70158 produces an overshoot of 10 percent).
	* @return		The correct value.
	*/
	Real easeOutInBack(Real t, Real s);

	/**
	* Easing equation function for a bounce (exponentially decaying parabolic bounce) easing out: decelerating to zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @param a		Amplitude.
	* @return		The correct value.
	*/
	Real easeOutBounce(Real t, Real a);

	/**
	* Easing equation function for a bounce (exponentially decaying parabolic bounce) easing in: accelerating from zero velocity.
	*
	* @param t		Current time (in frames or seconds).
	* @param a		Amplitude.
	* @return		The correct value.
	*/
	Real easeInBounce(Real t, Real a);

	/**
	* Easing equation function for a bounce (exponentially decaying parabolic bounce) easing in/out: acceleration until halfway, then deceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @param a		Amplitude.
	* @return		The correct value.
	*/
	Real easeInOutBounce(Real t, Real a);

	/**
	* Easing equation function for a bounce (exponentially decaying parabolic bounce) easing out/in: deceleration until halfway, then acceleration.
	*
	* @param t		Current time (in frames or seconds).
	* @param a		Amplitude.
	* @return		The correct value.
	*/
	Real easeOutInBounce(Real t, Real a);

	// SmoothBegin blends Smooth and Linear Interpolation.
	// Progress 0 - 0.3      : Smooth only
	// Progress 0.3 - ~ 0.5  : Mix of Smooth and Linear
	// Progress ~ 0.5  - 1   : Linear only

	/**
	* Easing function that starts growing slowly, then increases in speed. At the end of the curve the speed will be constant.
	*/
	Real easeInCurve(Real t);

	/**
	* Easing function that starts growing steadily, then ends slowly. The speed will be constant at the beginning of the curve.
	*/
	Real easeOutCurve(Real t);

	/**
	* Easing function where the value grows sinusoidally. Note that the calculated  end value will be 0 rather than 1.
	*/
	Real easeSineCurve(Real t);

	/**
	* Easing function where the value grows cosinusoidally. Note that the calculated start value will be 0.5 and the end value will be 0.5
	* contrary to the usual 0 to 1 easing curve.
	*/
	Real easeCosineCurve(Real t);

	Real easeFunc(int type, Real intensity, Real t);

}  // namespace Echo
