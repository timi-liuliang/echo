#include <algorithm>
#include <gtest/gtest.h>

// RANDOM 1-5
int rand_1_5()
{
	int result = rand() % 5 + 1;
	return result;
}

int rand_1_7()
{
	int vals[5][5] = {
	{1,2,3,4,5},
	{6,7,1,2,3},
	{4,5,6,7,1},
	{2,3,4,5,6},
	{7,0,0,0,0}
	};
	int result = 0;
	while (result == 0)
	{
		int i = rand_1_5();
		int j = rand_1_5();
		result = vals[i - 1][j - 1];
	}
	return result;
}

TEST(RANDOM, random_1_5)
{
	int randCount_5[] = { 0, 0, 0, 0, 0};

	const int testCount = 5000000;
	for (int i = 0; i < testCount; i++)
	{
		int rand = rand_1_5();
		assert(rand >= 1 && rand <= 5);
		randCount_5[rand-1]++;
	}
}

TEST(RANDOM, random_1_7)
{
	int randCount_7[] = { 0, 0, 0, 0, 0, 0, 0 };

	const int testCount = 7000000;
	for (int i = 0; i < testCount; i++)
	{
		int rand = rand_1_7();
		assert(rand >= 1 && rand <= 7);
		randCount_7[rand-1]++;
	}
}
