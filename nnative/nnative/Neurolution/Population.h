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
		int idxFirstDead;
    public:
        Population(size_t size, int firstDead = 0)
            : std::vector<T>{ size }
			, idxFirstDead{ firstDead }
        {
        }
        size_t AliveSize() const noexcept
        {
            return idxFirstDead;
        }

        size_t DeadSize() const noexcept
        {
            return this->std::vector<T>::size() - AliveSize();
        }

        void Mortigxumu(int idx) noexcept
        {
			if (idx >= idxFirstDead)
			{
				std::cout << "Population: entry is already dead" << std::endl;
				std::terminate();
			}
			if (idx < 0 || idx >= this->std::vector<T>::size())
			{
				std::cout << "Invalid idx" << std::endl;
				std::terminate();
			}

            std::swap((*this)[idx], (*this)[idxFirstDead - 1]);
            --idxFirstDead;
        }

        template <typename F>
        void MortigxumuCxiun(F&& fn) noexcept
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

        T& Reanimate() noexcept
        {
			if (DeadSize() == 0)
			{
				std::cout << "No dead bodies left" << std::endl;
				std::terminate();
			}
            ++idxFirstDead;
            return (*this)[idxFirstDead - 1];
        }

		template <typename TSerializeFn>
		void SaveTo(std::ostream& stream, TSerializeFn fn)
		{
			int sz = static_cast<int>(this->size());
			stream.write(reinterpret_cast<const char*>(&sz), sizeof(sz));
			stream.write(reinterpret_cast<const char*>(&idxFirstDead), sizeof(idxFirstDead));

			for (int idx = 0; idx < idxFirstDead; ++idx)
			{
				fn((*this)[idx], stream);
			}
		}

		template <typename TSerializeFn>
		void LoadFrom(std::istream& stream, TSerializeFn fn)
		{
			int sz{ 0 };
			stream.read(reinterpret_cast<char*>(&sz), sizeof(sz));
			this->resize(sz);

			stream.read(reinterpret_cast<char*>(&idxFirstDead), sizeof(idxFirstDead));

			for (int idx = 0; idx < idxFirstDead; ++idx)
			{
				fn((*this)[idx], stream);
			}
		}
    };
}