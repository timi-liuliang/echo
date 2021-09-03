#include <algorithm>
#include <gtest/gtest.h>

static double random_double() 
{
	// Returns a random real in [0,1).
	return rand() / (RAND_MAX + 1.0);
}

static double random_double(double min, double max) 
{
	// Returns a random real in [min,max).
	return min + (max - min) * random_double();
}

static int random_int(int min, int max) 
{
	// Returns a random integer in [min,max].
	return static_cast<int>(random_double(min, max + 1));
}

// https://raytracing.github.io/books/RayTracingTheRestOfYourLife.html
TEST(MonteCarlo, Pi)
{
	int randomSamplingCount = 10000000;
	int insideCircle = 0;

	for (int i = 0; i < randomSamplingCount; i++) 
	{
		auto x = random_double(-1, 1);
		auto y = random_double(-1, 1);
		if (x * x + y * y < 1)
			insideCircle++;
	}

	double pi = 4 * double(insideCircle) / randomSamplingCount;

	EXPECT_LE(pi, 3.142);
	EXPECT_GE(pi, 3.14);
}
