#pragma once

#include <vector>
#include <stdexcept>

namespace Neurolution
{
    class PopulationException : public std::exception
    {
    public:
        PopulationException(const char *what) : std::exception(what) {}
    };

    template <typename T>
    class Population : public std::vector<T>
    {
        int idxFirstDead{ 0 }; // all dead by default
    public:
        Population(size_t size)
            : std::vector<T>(size)
        {

        }
        size_t AliveSize() const
        {
            return idxFirstDead;
        }

        size_t DeadSize() const
        {
            return this->std::vector<T>::size() - AliveSize();
        }

        void Kill(int idx)
        {
            if (idx >= idxFirstDead)
                throw PopulationException("Entry is already dead");
            if (idx < 0 || idx >= this->std::vector<T>::size())
                throw PopulationException("Invalid idx");

            std::swap((*this)[idx], (*this)[idxFirstDead - 1]);
            --idxFirstDead;
        }

        template <typename F>
        void KillAll(F&& fn)
        {
            for (int idx = 0; idx < idxFirstDead; ++idx)
            {
                T& current = (*this)[idx];

                if (fn(current))
                {
                    std::swap(current, (*this)[idxFirstDead - 1]);
                    --idxFirstDead;
                }
            }
        }

        T& Reanimate()
        {
            if (DeadSize() == 0)
                throw PopulationException("No dead bodies left");
            ++idxFirstDead;
            return (*this)[idxFirstDead - 1];
        }
    };
}