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

	// Return gtest's result so the process exit code reflects pass/fail.
	// Agents and CI decide success from this code, not from parsing stdout.
	// NOTE: do NOT add an interactive pause here - it blocks any non-interactive
	// (CI / headless / redirected) run forever.
	return RUN_ALL_TESTS();
}