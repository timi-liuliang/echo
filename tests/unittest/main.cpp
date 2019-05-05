#include <gtest/gtest.h>
#include <engine/core/log/Log.h>

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
	// init log system
	Echo::LogDefault logDefault("unittest");
	Echo::Log::instance()->addOutput(&logDefault);

	// google test
	testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();

	system("PAUSE");
}