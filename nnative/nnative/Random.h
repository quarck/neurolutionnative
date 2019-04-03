#pragma once

#include <stdexcept>

class Random
{
public:
	Random(int seed = 0)
	{
		throw std::exception("Not implemented");
	}

	double NextDouble() 
	{
		throw std::exception("Not implemented");
	}

	int Next(int maxI = 0) 
	{
		throw std::exception("Not implemented");
	}

};