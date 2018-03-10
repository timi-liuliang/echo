/**********************************************************************
 *<
        FILE: random.h

        DESCRIPTION: Pseudo-random number generator

        CREATED BY: Jeff Kowalski

        HISTORY: Created 11 February 1999

 *>     Copyright (c) 1999, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"

/*! \sa  Class Interface.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class defines a Pseudo-random number generator that precisely matches the
behavior of the MSVCRT 6.0 random routines. That is to say, for equivalent
calls to ::srand() and Random::srand(), both ::rand() and Random::rand() will
produce the same results.\n\n
The benefit, however, in having this class is that each instantiation is
independent, permitting several uncoupled random number generators to be
present in the system at once. Moreover, each instantiation is automatically
"pre-seeded", making calls to Random::srand unnecessary in most uses. Even
arrays of Random items will operate independently.\n\n
In addition to providing the analogues to the "stdlib" functions, this class
also provides two useful member functions which can be used to get a random
number bounded in either a float or int interval.\n\n
Note: To use this class be sure to link to <b>MAXUTIL.LIB</b>.
\par Sample Code:
\code
#include "random.h"
...
Random r;
r.srand(1); // generally unnecessary, seeds generator like stdlib.h's srand()
r.rand(); // returns a random number a la stdlib.h's rand()
r.get(); // ditto
r.get(16); // returns 1 of 16 possible random numbers from 0 to 15 inclusive
r.get(5,2); // returns 1 of 3 possible random numbers from 2 to 4 inclusive
r.get(1.0f); // returns 1 of "Random::s_rand_max+1" floats, 0 \<= value \<1
r.get(1.0f,0.5f); // as above, but limits the result to 0.5 \<= value \<1
Random::s_rand_max; // similar to stdlib.h's RAND_MAX
...
\endcode
Note in all "get" cases that contain limits they are specified (max, min). Also
be aware that the min value can be attained, but the max cannot. That is to say
min \<= value \< max.
\par Data Members:
<b>static const int s_rand_max;</b>\n\n
This is akin to the Windows API global <b>RAND_MAX</b>. The constant
<b>RAND_MAX</b> is the maximum value that can be returned by the <b>rand</b>
function. <b>RAND_MAX</b> is defined as the value 0x7fff.  */
class Random: public MaxHeapOperators {
	private:
		long   m_seed;

	public:
		// The constructor will automatically initialize the seed
		/*! \remarks The constructor will automatically initialize the seed.	*/
		UtilExport Random ();

		// Analogues of the random rountines from MSVCRT:
		/*! \remarks This method is akin to the global <b>srand()</b>
		function. From the Windows API documentation:\n\n
		The <b>srand</b> function sets the starting point for generating a
		series of pseudorandom integers.
		\par Parameters:
		<b>unsigned int seed = 1</b>\n\n
		To reinitialize the generator, use 1 as the seed argument. Any other
		value for seed sets the generator to a random starting point.
		<b>rand</b> retrieves the pseudorandom numbers that are generated.
		Calling <b>rand</b> before any call to <b>srand</b> generates the same
		sequence as calling <b>srand</b> with seed passed as 1. */
		UtilExport void    srand (unsigned int seed = 1); // akin to global ::srand

		/*! \remarks This method is akin to the global <b>rand()</b> function.
		From the Windows API documentation:\n\n
		The <b>rand</b> function returns a pseudorandom integer in the range 0
		to <b>RAND_MAX</b>. Use the srand function to seed the
		pseudorandom-number generator before calling <b>rand</b>. */
		UtilExport int     rand  ();                      // akin to global ::rand
		UtilExport static const int s_rand_max;           // akin to global RAND_MAX

		/*! \remarks Returns a random number in the half-open interval [min, max) such
		that r=get(max, min) := min \<= r \< max. Note that max is the first arg, and
		min is the second, permitting one to select, for example, an int in [0,5) =
		[0,4] with "get(5)". With no arguments, <b>Random::get()</b> is equivalent to
		<b>Random::rand()</b>.
		\par Parameters:
		<b>int max_exclusive = s_rand_max+1</b>\n\n
		The maximum value.\n\n
		<b>int min_inclusive = 0</b>\n\n
		The minimum value. */
		inline int get(int max_exclusive = s_rand_max+1, int min_inclusive = 0) {
			return (this->rand() % (max_exclusive - min_inclusive) + min_inclusive);
		}

		/*! \remarks Returns a random number in the half-open interval [min,
		max) such that r=get(max, min) := min \<= r \< max. Note that max is
		the first arg, and min is the second, permitting one to select, for
		example, a float in [0.0, 5.0) with "getf(5)". With no arguments,
		<b>Random::getf()</b> returns a float in [0.0, 1.0).
		\par Parameters:
		<b>float max_exclusive = 1.0f</b>\n\n
		The maximum value.\n\n
		<b>float min_inclusive = 0.0f</b>\n\n
		The minimum value. */
		inline float getf(float max_exclusive = 1.0f, float min_inclusive = 0.0f) {
			return (this->rand() / (s_rand_max+1.0f) * (max_exclusive - min_inclusive) + min_inclusive);
		}
};

