#pragma once

#include <stdexcept>
#include <random>
#include <chrono>

class Random
{
    std::default_random_engine generator;

    std::uniform_real_distribution<double> realDistribution{ 0.0, 1.0 };
    std::uniform_int_distribution<int> intDistribution{ 0, INT_MAX };

public:
    Random()
    {
        unsigned seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
        generator.seed(seed);
    }

    double NextDouble()
    {
        return realDistribution(generator);
    }

    int Next()
    {
        return intDistribution(generator);
    }

    int Next(int max)
    {
        return Next() % max;
    }
};