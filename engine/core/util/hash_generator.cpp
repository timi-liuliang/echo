#include "hash_generator.h"

namespace Echo
{
	// BKDR Hash Function
	unsigned int BKDRHash(const char* str)
	{
		unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
		unsigned int hash = 0;

		while (*str)
			hash = hash * seed + (*str++);

		return (hash & 0x7FFFFFFF);
	}
}