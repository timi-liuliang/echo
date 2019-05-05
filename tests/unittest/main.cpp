#include <gtest/gtest.h>

namespace Echo
{
	// implement by application or dll
	void registerModules()
	{

	}
}

// main function
int main(int argc, char* argv[])
{
	testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();

	system("PAUSE");
}