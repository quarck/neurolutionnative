// TODO: ideas to try: 
// * caves 
// * R-G-B games
// * gravity 

#pragma once 

#include <algorithm>
#include <functional>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <sstream>

#include "../Random.h"
#include "../ThreadGrid.h"

#include "Cell.h"
#include "Population.h"

namespace Neurolution
{
    struct Location
    {
        float LocationX;
        float LocationY;

		void SaveTo(std::ostream& stream)
		{
			stream.write(reinterpret_cast<const char*>(&LocationX), sizeof(LocationX));
			stream.write(reinterpret_cast<const char*>(&LocationY), sizeof(LocationY));
		}

		void LoadFrom(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&LocationX), sizeof(LocationX));
			stream.read(reinterpret_cast<char*>(&LocationY), sizeof(LocationY));
		}
    };

    struct Direction
    {
        float DirectionX;
        float DirectionY;

		void SaveTo(std::ostream& stream)
		{
			stream.write(reinterpret_cast<const char*>(&DirectionX), sizeof(DirectionX));
			stream.write(reinterpret_cast<const char*>(&DirectionY), sizeof(DirectionY));
		}

		void LoadFrom(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&DirectionX), sizeof(DirectionX));
			stream.read(reinterpret_cast<char*>(&DirectionY), sizeof(DirectionY));
		}
	};

    struct LocationAndDirectionWithvalue : public Direction, public Location
    {
        float Value;

        void Step(Random& rnd, int maxX, int maxY)  noexcept
        {
            LocationX = LoopValue(
                LocationX + DirectionX + (float)(rnd.NextDouble() * 0.25 - 0.125), 0.0f, static_cast<float>(maxX));
            LocationY = LoopValue(
                LocationY + DirectionY + (float)(rnd.NextDouble() * 0.25 - 0.125), 0.0f, static_cast<float>(maxY));
        }

		void SaveTo(std::ostream& stream) 
		{
			stream.write(reinterpret_cast<const char*>(&Value), sizeof(Value));
			this->Direction::SaveTo(stream);
			this->Location::SaveTo(stream);
		}

		void LoadFrom(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&Value), sizeof(Value));
			this->Direction::LoadFrom(stream);
			this->Location::LoadFrom(stream);
		}
	};

    struct DirectionWithDistanceSquare : public Direction
    {
        float DistanceSquare;

        void Set(float dx, float dy)  noexcept
        {
            DirectionX = dx;
            DirectionY = dy;
            DistanceSquare = dx * dx + dy * dy;
        }

		void SaveTo(std::ostream& stream)
		{
			this->Direction::SaveTo(stream);
		}

		void LoadFrom(std::istream& stream)
		{
			this->Direction::LoadFrom(stream);
			DistanceSquare = DirectionX * DirectionX + DirectionY * DirectionY;
		}
    };

    struct Food : public LocationAndDirectionWithvalue
    {
        Food()
        {
        }

        Food(Random& rnd, int maxX, int maxY)
        {
            Reset(rnd, maxX, maxY);
        }

        float Consume(float delta = 0.071f)  noexcept
        {
            float ret = 0.0f;

            while (Value > 0.001)
            {
                float valueCopy = InterlockedCompareExchange(&Value, 0.0f, 0.0f);
                float newDelta = valueCopy < 0.5f ? valueCopy : 0.5f;
                float newValue = valueCopy - newDelta;

                // ReSharper disable once CompareOfFloatsByEqualityOperator
                if (InterlockedCompareExchange(&Value, newValue, valueCopy) == valueCopy)
                {
                    ret = valueCopy - newValue;
                    break;
                }
            }

            return ret;
        }

        bool IsEmpty() const  noexcept { return Value < 0.00001; }

        void Reset(Random& rnd, int maxX, int maxY, bool valueOnly = false)  noexcept
        {
            Value = AppProperties::FoodInitialValue;// * (0.5 + rnd.NextDouble());

            if (!valueOnly)
            {
                LocationX = (float)rnd.Next(maxX);
                LocationY = (float)rnd.Next(maxY);

                DirectionX = (float)(rnd.NextDouble() * 0.5 - 0.25);
                DirectionY = (float)(rnd.NextDouble() * 0.5 - 0.25);
            }
        }

		void SaveTo(std::ostream& stream)
		{
			this->LocationAndDirectionWithvalue::SaveTo(stream);
		}

		void LoadFrom(std::istream& stream)
		{
			this->LocationAndDirectionWithvalue::LoadFrom(stream);
		}
    };

    class World
    {
        static constexpr float SQRT_2 = 1.4142135623730950488016887242097f; // unfortunately std::sqrt is not a constexpr function

        int _numWorkerThreads;

		Population<std::shared_ptr<Cell>> _cells;
		Population<std::shared_ptr<Cell>> _predators;
        Population<Food> _foods;

        std::vector<std::vector<DirectionWithDistanceSquare>> _cellDirections;
        std::vector<std::vector<DirectionWithDistanceSquare>> _predatorDirections;
        std::vector<std::vector<DirectionWithDistanceSquare>> _foodDirections;

		std::mutex _cellIdofariLock;
		std::mutex _predatorsIdofariLock;

        int _maxX;
        int _maxY;

        int _foodsPerCycle;
        int _nextFoodIdx{ 0 };

        Random _random{};

        ThreadGrid _grid;

	public:
        World(
			int maxX,
			int maxY,
            int nWorkerThreads,
			int maxPreys,
			int maxPredators,
			int maxFoods,
			int initialPreys,
			int initialPredators
		)
            : _grid(nWorkerThreads)
            , _maxX(maxX)
            , _maxY(maxY)
            , _numWorkerThreads(nWorkerThreads)
            , _cells(maxPreys, initialPreys)
            , _foods(maxFoods)
            , _foodsPerCycle(maxFoods)
            , _predators(maxPredators, initialPredators)
            , _cellDirections(nWorkerThreads)
            , _foodDirections(nWorkerThreads)
            , _predatorDirections(nWorkerThreads)
        {
			for (int i = 0; i < maxPreys; ++i)
			{
				_cells[i] = std::make_shared<Cell>(_random, maxX, maxY, false);
				if (i >= initialPreys)
					_cells[i]->Alive = false;
			}

			for (int i = 0; i < maxPredators; ++i)
			{
				_predators[i] = std::make_shared<Cell>(_random, maxX, maxY, true);
				if (i >= initialPredators)
					_predators[i]->Alive = false;
			}

            for (int i = 0; i < _foodsPerCycle; ++i)
            {
                _foods.Reanimate() = Food(_random, maxX, maxY);
            }

            for (int i = 0; i < _numWorkerThreads; ++i)
            {
                // Some in-efficiency here, yes
                _cellDirections[i] = std::vector<DirectionWithDistanceSquare>(maxPreys);
                _foodDirections[i] = std::vector<DirectionWithDistanceSquare>(maxFoods);
                _predatorDirections[i] = std::vector<DirectionWithDistanceSquare>(maxPredators);
            }
        }

		void SaveTo(std::ostream& stream)
		{
			stream.write(reinterpret_cast<const char*>(&_maxX), sizeof(_maxX));
			stream.write(reinterpret_cast<const char*>(&_maxY), sizeof(_maxY));
			stream.write(reinterpret_cast<const char*>(&_foodsPerCycle), sizeof(_foodsPerCycle));
			stream.write(reinterpret_cast<const char*>(&_nextFoodIdx), sizeof(_nextFoodIdx));

			_cells.SaveTo(stream, [&](std::shared_ptr<Cell> & item, std::ostream & s) {item->SaveTo(s); });
			_predators.SaveTo(stream, [&](std::shared_ptr<Cell> & item, std::ostream & s) {item->SaveTo(s); });
			_foods.SaveTo(stream, [&](Food& item, std::ostream & s) {item.SaveTo(s); });
		}

		void LoadFrom(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&_maxX), sizeof(_maxX));
			stream.read(reinterpret_cast<char*>(&_maxY), sizeof(_maxY));
			stream.read(reinterpret_cast<char*>(&_foodsPerCycle), sizeof(_foodsPerCycle));
			stream.read(reinterpret_cast<char*>(&_nextFoodIdx), sizeof(_nextFoodIdx));

			_cells.LoadFrom(stream, [&](std::shared_ptr<Cell> & item, std::istream & s) {item->LoadFrom(s); });
			_predators.LoadFrom(stream, [&](std::shared_ptr<Cell> & item, std::istream & s) {item->LoadFrom(s); });
			_foods.LoadFrom(stream, [&](Food & item, std::istream & s) {item.LoadFrom(s); });
		}

	private:

        std::vector<int> multipliers{ 6, 3, 2, 1 };

	public:
        void Iterate(long step)  noexcept
        {
            if (step == 0)
                WorldInitialize();

            for (int idx = 0; idx < _foods.AliveSize(); ++idx)
                _foods[idx].Step(_random, _maxX, _maxY);


            if ((step % (AppProperties::StepsPerGeneration / _foodsPerCycle)) == 0)
            {
                GiveOneFood();
            }

            _grid.GridRun(
                [&](int idx, int n)
				{
					for (int cellIdx = idx; cellIdx < _cells.AliveSize(); cellIdx += n)
					{
						IterateCellEye(idx, step, _cells[cellIdx]);
					}
					for (int pIdx = idx; pIdx < _predators.AliveSize(); pIdx += n)
					{
						IterateCellEye(idx, step, _predators[pIdx]);
					}
				});

            _grid.GridRun(
                [&](int idx, int n)
				{
					for (int cellIdx = idx; cellIdx < _cells.AliveSize(); cellIdx += n)
					{
						IterateCellThinkingAndMoving(idx, step, _cells[cellIdx]);
					}
					for (int pIdx = idx; pIdx < _predators.AliveSize(); pIdx += n)
					{
						IterateCellThinkingAndMoving(idx, step, _predators[pIdx]);
					}
				});

            _grid.GridRun(
                [&](int idx, int n)
				{
					for (int cellIdx = idx; cellIdx < _cells.AliveSize(); cellIdx += n)
					{
						IterateCellCollisions(idx, step, _cells[cellIdx]);
					}
				});

            // Kill any empty foods 
            _foods.MortigxumuCxiun([](Food& f) { return f.Value < 0.001f; });

		    // TODO: Clean.Dead.Cells() && Predatos();
			// Clean dead-bodies.. 
			for (int cellIdx = 0; cellIdx < _cells.AliveSize(); )
			{
				auto& cell = _cells[cellIdx];
				if (cell->CurrentEnergy < 0.0001f)
				{
					cell->Alive = false;
					_cells.Mortigxumu(cellIdx);
					continue;
				}
				++cellIdx;
			}

			for (int pIdx = 0; pIdx < _predators.AliveSize(); )
			{
				auto& predator = _predators[pIdx];
				if (predator->CurrentEnergy < 0.0001f)
				{
					predator->Alive = false;
					_predators.Mortigxumu(pIdx);
					continue;
				}
				++pIdx;
			}

			if (_cells.AliveSize() == 0)
			{
				// Some failback in case of full extinction 
				for (int i = 0; i < 5; ++i)
				{
					auto& c0 = _cells.Reanimate();
					c0->Alive = true; c0->CurrentEnergy = AppProperties::InitialCellEnergy;
				}
			}

			int cellsAliveSize = _cells.AliveSize();
			int predatorsAliveSize = _predators.AliveSize();

			_grid.GridRun(
				[&](int idx, int n)
				{
					for (int cellIdx = idx; cellIdx < cellsAliveSize; cellIdx += n)
					{
						auto& cell = _cells[cellIdx];
						if (cell->CurrentEnergy > 2*AppProperties::BirthEnergyConsumption)
						{
							std::lock_guard<std::mutex> l(_cellIdofariLock);

							if (_cells.DeadSize() > 0)
							{
								auto& tgt = _cells.Reanimate();

								FaruBebon(cell, tgt, AppProperties::BirthEnergyConsumption, AppProperties::InitialCellEnergy);
							}
						}
					}
					for (int pIdx = idx; pIdx < predatorsAliveSize; pIdx += n)
					{
						auto& predator = _predators[pIdx];
						if (predator->CurrentEnergy > 2*AppProperties::PredatorBirthEnergyConsumption)
						{
							std::lock_guard<std::mutex> l(_predatorsIdofariLock);

							if (_predators.DeadSize() > 0)
							{
								auto& tgt = _predators.Reanimate();
								FaruBebon(predator, tgt, AppProperties::PredatorBirthEnergyConsumption, AppProperties::PredatorInitialValue);
							}
						}
					}
				});

			// Mix-in some mutation for elderly units 
			_grid.GridRun(
				[&](int idx, int n)
				{
					for (int cellIdx = idx; cellIdx < _cells.AliveSize(); cellIdx += n)
					{
						auto& cell = _cells[cellIdx];
						if (cell->Age > AppProperties::OldSince)
							FaruBebon(cell, cell, 0, cell->CurrentEnergy);
					}
					for (int pIdx = idx; pIdx < _predators.AliveSize(); pIdx += n)
					{
						auto& predator = _predators[pIdx];
						if (predator->Age > AppProperties::OldSince)
							FaruBebon(predator, predator, 0, predator->CurrentEnergy);
					}
				});
        }

		int GetNumPredators() const 
		{
			return _predators.AliveSize();
		}

		int GetNumPreys() const
		{
			return _cells.AliveSize();
		}
	private:

        void IterateCellEye(int threadIdx, long step, std::shared_ptr<Cell>& cell)  noexcept
        {
            if (cell->CurrentEnergy < 0.00001f)
                return;

            auto& cellDirections = _cellDirections[threadIdx];
            auto& foodDirections = _foodDirections[threadIdx];
            auto& predatorDirections = _predatorDirections[threadIdx];

            cell->PrepareIteration();

            // Calculate light sensor values 

            float offsX = _maxX * 1.5f - cell->LocationX;
            float offsY = _maxY * 1.5f - cell->LocationY;
            float halfMaxX = _maxX / 2.0f;
            float halfMaxY = _maxY / 2.0f;


            // Calculate light sensor values 
            for (int idx = 0; idx < _foods.AliveSize(); ++idx)
            {
                auto& item = _foods[idx];

                float dx = LoopValue(item.LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
                float dy = LoopValue(item.LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;

                foodDirections[idx].Set(dx, dy);
            }

            for (int idx = 0; idx < _predators.AliveSize(); ++idx)
            {
                auto& item = _predators[idx];

                float dx = LoopValue(item->LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
                float dy = LoopValue(item->LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;

                predatorDirections[idx].Set(dx, dy);
            }

            for (int idx = 0; idx < _cells.AliveSize(); ++idx)
            {
                auto& item = _cells[idx];

                float dx = LoopValue(item->LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
                float dy = LoopValue(item->LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;

                cellDirections[idx].Set(dx, dy);
            }

            auto& eye = cell->GetEye();

            for (unsigned int tripodIdx = 0;
                tripodIdx < AppProperties::EyeSizeNumTripods;
                ++tripodIdx)
            {

                auto& redCell = eye[3 * tripodIdx];
                auto& greenCell = eye[3 * tripodIdx + 1];
                auto& blueCell = eye[3 * tripodIdx + 2];
                // 
                float viewDirection = cell->Rotation + redCell.Direction;

                float viewDirectionX = (float)std::cos(viewDirection);
                float viewDirectionY = (float)std::sin(viewDirection);

                // RED 
                {
                    float value = 0.0f;

                    // This cell can see foods only
                    for (unsigned int idx = 0; idx < _foods.AliveSize(); ++idx)
                    {
                        auto& foodItem = _foods[idx];
                        if (foodItem.Value < 0.01f)
                            continue;

                        auto& food = foodDirections[idx];

                        float modulo = viewDirectionX * food.DirectionX + viewDirectionY * food.DirectionY;

                        if (modulo <= 0.0)
                            continue;

                        float invSqrRoot = Q_rsqrt(food.DistanceSquare);

                        float cosine = modulo * invSqrRoot;

                        //float distnaceSquare = (float) std::pow(food.Distance, 2.0);

                        float signalLevel =
                            (float)(foodItem.Value * std::pow(cosine, redCell.Width)
                                * invSqrRoot * invSqrRoot);

                        value += signalLevel;
                    }
                    cell->Network->InputVector[3 * tripodIdx] = 20000 * value;
                }

                // GREEN 
                {
                    float value = 0.0f;

                    // this cell can see predators only
                    for (unsigned int idx = 0; idx < _cells.AliveSize(); ++idx)
                    {
                        auto& cellDirection = cellDirections[idx];
                        auto& cellItem = _cells[idx];

                        if (cellItem->CurrentEnergy < 0.01f)
                            continue;

                        if (cellItem == cell)
                            continue;

                        float modulo = viewDirectionX * cellDirection.DirectionX + viewDirectionY * cellDirection.DirectionY;

                        if (modulo <= 0.0)
                            continue;

                        float invSqrRoot = Q_rsqrt(cellDirection.DistanceSquare);

                        float cosine = modulo * invSqrRoot;

                        // float distnaceSquare = (float)std::pow(predator.Distance, 2.0);

                        float signalLevel =
                            (float)(/*cellItem->CurrentEnergy*/AppProperties::InitialCellEnergy * std::pow(cosine, greenCell.Width)
                                * invSqrRoot * invSqrRoot);

                        value += signalLevel;
                    }

                    cell->Network->InputVector[3 * tripodIdx + 1] = 20000 * value;
                }

                // BLUE
                {
                    float value = 0.0f;

                    // this cell can see predators only
                    for (unsigned int idx = 0; idx < _predators.AliveSize(); ++idx)
                    {
                        auto& predator = predatorDirections[idx];
                        auto& predatorItem = _predators[idx];

                        if (predatorItem->CurrentEnergy < 0.01f)
                            continue;

                        if (predatorItem == cell)
                            continue;

                        float modulo = viewDirectionX * predator.DirectionX + viewDirectionY * predator.DirectionY;

                        if (modulo <= 0.0)
                            continue;

                        float invSqrRoot = Q_rsqrt(predator.DistanceSquare);

                        float cosine = modulo * invSqrRoot;

                        // float distnaceSquare = (float)std::pow(predator.Distance, 2.0);

                        float signalLevel =
                            (float)(/*predatorItem->CurrentEnergy*/AppProperties::PredatorInitialValue * std::pow(cosine, blueCell.Width)
                                * invSqrRoot * invSqrRoot);

                        value += signalLevel;
                    }
                    cell->Network->InputVector[3 * tripodIdx + 2] = 20000 * value;
                }
            }
        }

        void IterateCellThinkingAndMoving(int threadIdx, long step, std::shared_ptr<Cell>& cell) noexcept
        {
            float forceLeft = 0.0f;
            float forceRight = 0.0f;

            for (int nIter = 0; nIter < 4; ++nIter)
            {
                // Iterate network finally
                cell->IterateNetwork(step * 4 + nIter);

                // Execute action - what is ordered by the neuron network
                forceLeft += cell->MoveForceLeft / 4.0f;
                forceRight += cell->MoveForceRight / 4.0f;
            }

            float forwardForce = ((forceLeft + forceRight) / SQRT_2);
            float rotationForce = (forceLeft - forceRight) / SQRT_2;

            float moveEnergyRequired = (std::abs(forceLeft) + std::abs(forceRight)) * AppProperties::MoveEnergyFactor;

            if (cell->IsPredator)
            {
                forwardForce *= 0.94f; // a bit slow and heavy
            }
            else if (!cell->IsPredator && cell->CurrentEnergy < AppProperties::SedatedAtEnergyLevel)
            {
                float slowdownFactor = cell->CurrentEnergy / AppProperties::SedatedAtEnergyLevel;
                forwardForce *= slowdownFactor;
                rotationForce *= slowdownFactor;
                moveEnergyRequired *= slowdownFactor;
            }

            if (moveEnergyRequired <= cell->CurrentEnergy)
            {
                cell->CurrentEnergy -= moveEnergyRequired;

                cell->Rotation = LoopValue(cell->Rotation + rotationForce, 0.0f, (float)(M_PI * 2.0f));

                float dX = (float)(forwardForce*std::cos(cell->Rotation));
                float dY = (float)(forwardForce*std::sin(cell->Rotation));

                cell->LocationX = LoopValue(cell->LocationX + dX, 0.0f, (float)_maxX);
                cell->LocationY = LoopValue(cell->LocationY + dY, 0.0f, (float)_maxY);
            }
            else if (!cell->IsPredator)
            {
                cell->CurrentEnergy = 0.0f; // so it has tried and failed
            }

        }

        void IterateCellCollisions(int threadIdx, long step, std::shared_ptr<Cell>& cell)  noexcept
        {
            if (!cell->IsPredator)
            {
                if (cell->CurrentEnergy < AppProperties::MaxEnergyCapacity)
                {
                    // Analyze the outcome - did it get any food? 
                    for (int foodIdx = 0; foodIdx < _foods.AliveSize(); ++foodIdx)
                    {
                        auto& food = _foods[foodIdx];
                        if (food.IsEmpty())
                            continue;

                        float pdx = std::powf(cell->LocationX - food.LocationX, 2.0f);
                        float pdy = std::powf(cell->LocationY - food.LocationY, 2.0f);

                        if (pdx + pdy <= 100.0f)
                        {
                            cell->CurrentEnergy += food.Consume();
                        }
                    }
                }

                if (cell->CurrentEnergy > AppProperties::SporeEnergyLevel)
                {
                    // Analyze the outcome - did it hit any predators? 
                    for (auto& predator : _predators)
                    {
                        if (predator->CurrentEnergy < 0.0001f)
                            continue; // skip deads 

                        float pdx = std::powf(cell->LocationX - predator->LocationX, 2.0f);
                        float pdy = std::powf(cell->LocationY - predator->LocationY, 2.0f);

                        if (pdx + pdy <= 100.0f)
                        {
                            float energy = InterlockedCompareExchange(&(cell->CurrentEnergy), 0.0f, cell->CurrentEnergy);

                            if (!predator->PredatoryEat(energy))
                            {
                                // predator has failed 
                                InterlockedCompareExchange(&(cell->CurrentEnergy), energy, 0.0f);
                            }
                        }
                    }
                }
            }
        }

        void GiveOneFood()  noexcept
        {
            if (_foods.DeadSize() > 0)
            {
                _foods.Reanimate().Reset(_random, _maxX, _maxY);
            }
            else
            {
                if (_foods[_nextFoodIdx].Value < AppProperties::FoodInitialValue / 2.0f)
                {
                    _foods[_nextFoodIdx].Reset(_random, _maxX, _maxY);
                    _nextFoodIdx = (_nextFoodIdx + 1) % _foods.size();
                }
            }
        }

        void WorldInitialize()  noexcept
        {
            // cleanput outputs & foods 
            for (auto& cell : _cells)
            {
                cell->CurrentEnergy = AppProperties::InitialCellEnergy;
                cell->Network->CleanOutputs();
                //cell.RandomizeLocation(_random, _maxX, _maxY);
            }

            for (auto& predator : _predators)
            {
                predator->CurrentEnergy = AppProperties::PredatorInitialValue;
                predator->Network->CleanOutputs();
            }

            GiveOneFood();
        }

        void FaruBebon(std::shared_ptr<Cell>& source, std::shared_ptr<Cell>& destination, float energyConsumption, float initialEnergy)  noexcept
        {
			source->CurrentEnergy -= energyConsumption;

            double rv = _random.NextDouble();
            bool severeMutations = (rv < AppProperties::SevereMutationFactor);

            float severity = (float)(1.0 - std::pow(rv / AppProperties::SevereMutationFactor,
                AppProperties::SevereMutationSlope)); // % of neurons to mutate

            destination->CloneFrom(*source, _random, _maxX, _maxY, severeMutations, severity);
            destination->ClonedFrom = -1;

            destination->CurrentEnergy = initialEnergy;
            destination->Network->CleanOutputs();

            destination->RandomizeLocation(_random, _maxX, _maxY);
			destination->Alive = true;

			destination->Age = 0; 
        }


		friend class WorldView;
    };
}
