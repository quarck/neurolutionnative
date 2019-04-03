#pragma once

#include <stdexcept>

class Random
{
public:
	Random(int seed = 0)
	{
		::srand(seed); // BAD BA DBAD 
	}

	double NextDouble() 
	{
		return ((double)(rand() % 0x7fff)) / 0x7fff; // also bAD bad
	}

	int Next(int maxI = 0) 
	{
		int r = rand();
		int r2 = rand();
		int r3 = rand();
		int val = (r | (r2 << 14) | (r3 << 17)) & 0x7fffffff;
		if (maxI)
			return val % maxI; // very bad code -- re-write when have a time
		return val;
	}

};