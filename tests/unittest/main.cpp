#include <gtest/gtest.h>
#include <cmath>

double cubic(double d)
{
	return pow(d, 3);
}


TEST(testMath, myCubeTest)
{
	EXPECT_EQ(1000, cubic(10));
}

// main function
int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}