#include <gtest/gtest.h>

// main function
int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();

	system("PAUSE");
}