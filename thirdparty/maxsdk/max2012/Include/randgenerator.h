/**********************************************************************
 *<
	FILE: RandGenerator.h

	DESCRIPTION: Generator of random numbers

	CREATED BY: Oleg Bayborodin

	HISTORY: Nov.17 2000 - created

	GOALS: To solve the need in satisfactory generator of random numbers.

	DESCRIPTION: The class has interfaces for srand/rand methods of VC++ 
		and other functions for random number generation.

		Srand/rand methods from stdlib.h have two main problems:
		a) It's not satisfactory random. Since rand() function returns a 
			pseudorandom integer in the range 0 to 0x7fff=32767, if we
			need a lot of random numbers (i.e. for generating 100,000 
			particles), we are running out of continuity of random num-
			bers. Generated random numbers became too discrete.
		b) rand() method is global function, not class object. Hence it's
			shared between all modules of your plug-in. Changes in one
			module may change randomness pattern in other independent 
			module. To solve this contradiction, rand methods have to be
			implemented as a class object.

		RandGenerator does exactly that. It has much more random numbers:
		RAND_MAX = 0xFFFFFFFF = 4,294,967,295. Also, using instance of 
		the class, it's much easier to create separate thread of random 
		numbers for specific module.

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#pragma once
#include <WTypes.h>
#include "coreexp.h"
#include "maxheap.h"


// to fix conflict between RandGenerator and stdlib.h do the following:
#undef RAND_MAX
const unsigned int RAND_MAX = 0x7fff; // as defined in stdlib.h

/*! class RandGenerator
\par Description:
This class is available in release 4.0 and later only.\n\n
This class has interfaces for <b>srand()</b> and <b>rand()</b> methods of VC++
and other functions for random number generation. The <b>srand()</b> and
<b>rand()</b> methods from <b>stdlib.h</b> have two main problems:\n\n
a) It's not satisfactorily random. The <b>rand()</b> function returns a
pseudorandom integer in the range 0 to 0x7fff=32767. If we need a lot of random
numbers using <b>rand()</b> (i.e. for generating 100,000 particles), we run out
of continuity of random numbers. Generated random numbers becomes too
discrete.\n\n
b) The <b>rand()</b> method is global function, not class object. Hence it is
shared between all modules of your plug-in. Changes in one module may change
randomness pattern in other independent module. To solve this contradiction,
rand methods have to be implemented as a class object.\n\n
The <b>RandGenerator</b> does exactly that. It has much more random numbers:
RAND_MAX = 0xFFFFFFFF = 4,294,967,295. Also, using instances of the class, it's
much easier to create separate threads of random numbers for a specific
module.\n\n

\par Data Members:
<b>static const DWORD32 RAND_MAX</b>\n\n
This definition is used to override the VC++ rand methods.  */
class RandGenerator: public MaxHeapOperators 
{
	public:
		/*! \remarks Constructor. */
		CoreExport RandGenerator(); // constructor
	
	// override for VC++ rand methods
		CoreExport static const DWORD32 RAND_MAX;

		// The srand function sets the starting point for generating a 
		// series of pseudorandom integers. To reinitialize the generator, 
		// use 1 as the seed argument. Any other value for seed sets the 
		// generator to a random starting point. rand() retrieves the 
		// pseudorandom numbers that are generated. Calling rand() before 
		// any call to srand() generates the same sequence as calling srand() 
		// with seed passed as 1.
		/*! \remarks This method sets the starting point for generating a
		series of pseudorandom integers. To reinitialize the generator, use 1
		as the seed argument. Any other value for seed sets the generator to a
		random starting point. <b>rand()</b> retrieves the pseudorandom numbers
		that are generated. Calling <b>rand()</b> before any call to this
		method generates the same sequence as calling it with seed passed as 1.
		\par Parameters:
		<b>DWORD32 seed</b>\n\n
		The starting seed. */
		CoreExport void	srand(DWORD32 seed);
		// The rand function returns a pseudorandom integer in the range 0 to RAND_MAX
		/*! \remarks This method returns a pseudorandom integer in the range 0 to RAND_MAX */
		CoreExport DWORD32 rand( void );

	// other useful functions:
		/*! \remarks This method returns the random number sign, either -1 or 1. */
		CoreExport int		RandSign( void );
		/*! \remarks This method return a random number between 0.0f and 1.0f.	*/
		CoreExport float	Rand01( void );
		/*! \remarks This method return a random number between -1.0f and 1.0f. */
		CoreExport float	Rand11( void );
		/*! \remarks This method return a random number between -0.5f and 0.5f. */
		CoreExport float	Rand55( void );
		/*! \remarks This method return a random number between 0 and maxnum. */
		CoreExport int		Rand0X(int maxnum);

		/*! \remarks This method returns TRUE if the random number generator has been
		explicitly initialized by the <b>srand()</b> method. */
		const bool	Valid( void ) const { 
			return m_explicitelyInitialized; 
		}

	private:
		static const DWORD32 kMagicNumber1, kMagicNumber2;
		static const DWORD32 HALF_RAND_MAX;
		static const double kIntMax, kIntMax1, kHalfIntMax;
		DWORD32 m_randar[256];
		DWORD32 m_randX, m_randY;
		bool m_explicitelyInitialized;
};



