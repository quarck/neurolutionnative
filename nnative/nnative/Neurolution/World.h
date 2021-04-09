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
#include "../Utils.h"

#include "WorldUtils.h"

namespace Neurolution
{
	//template <typename WorldProp>
    struct DirectionWithDistanceSquare //: public Orientation
    {
		float DirectionX;
		float DirectionY;
        float DistanceSquare;

        void Set(float dx, float dy)  noexcept
        {
            DirectionX = dx;
            DirectionY = dy;
            DistanceSquare = dx * dx + dy * dy;
        }
    };

	template <typename WorldProp>
    struct Food : public MaterialPointWithEnergyValue
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

            while (EnergyValue > 0.001)
            {
                float valueCopy = InterlockedCompareExchange(&EnergyValue, 0.0f, 0.0f);
                float newDelta = valueCopy < 0.5f ? valueCopy : 0.5f;
                float newValue = valueCopy - newDelta;

                // ReSharper disable once CompareOfFloatsByEqualityOperator
                if (InterlockedCompareExchange(&EnergyValue, newValue, valueCopy) == valueCopy)
                {
                    ret = valueCopy - newValue;
                    break;
                }
            }

            return ret;
        }

        bool IsEmpty() const  noexcept { return EnergyValue < 0.00001; }

        void Reset(Random& rnd, int maxX, int maxY, bool valueOnly = false)  noexcept
        {
			EnergyValue = WorldProp::FoodInitialValue;// * (0.5 + rnd.NextDouble());

            if (!valueOnly)
            {
                LocationX = (float)rnd.Next(maxX);
                LocationY = (float)rnd.Next(maxY);

				VelocityX = (float)(rnd.NextDouble() * 2.5 - 1.25);
				VelocityY = (float)(rnd.NextDouble() * 2.5 - 1.25);
            }
        }

		void SaveTo(std::ostream& stream)
		{
			this->MaterialPointWithEnergyValue::SaveTo(stream);
		}

		void LoadFrom(std::istream& stream)
		{
			this->MaterialPointWithEnergyValue::LoadFrom(stream);
		}
    };

	template <typename WorldProp>
    class World
    {
	public:
		using TProp = WorldProp;
		using TCell = Cell<WorldProp>;
	private:
        static constexpr float SQRT_2 = 1.4142135623730950488016887242097f; // unfortunately std::sqrt is not a constexpr function

        int _numWorkerThreads;


		Population<std::shared_ptr<TCell>> _cells;
		Population<std::shared_ptr<TCell>> _predators;
        Population<Food<WorldProp>> _foods;
		std::vector<std::pair<int, int>> _interGenerationCloneMapCells;
		std::vector<std::pair<int, int>> _interGenerationCloneMapPredators;

        std::vector<std::vector<DirectionWithDistanceSquare>> _cellDirections;
        std::vector<std::vector<DirectionWithDistanceSquare>> _predatorDirections;
        std::vector<std::vector<DirectionWithDistanceSquare>> _foodDirections;

        int _maxX;
        int _maxY;

        int _foodsPerCycle;
        int _nextFoodIdx{ 0 };

        Random _random{};

        std::string _workingFolder;
        bool _workingFolderCreated{ false };

        ThreadGrid _grid;

	public:
        World(const std::string& workingFolder,
            int nWorkerThreads,
            int numPreys, int maxFoods, int numPredators, int maxX, int maxY)
            : _grid(nWorkerThreads)
            , _maxX(maxX)
            , _maxY(maxY)
            , _workingFolder(workingFolder)
            , _numWorkerThreads(nWorkerThreads)
            , _cells(numPreys, true)
			, _interGenerationCloneMapCells(numPreys)
			, _foods(maxFoods)
            , _foodsPerCycle(maxFoods)
            , _predators(numPredators, true)
			, _interGenerationCloneMapPredators(numPredators)
			, _cellDirections(nWorkerThreads)
            , _foodDirections(nWorkerThreads)
            , _predatorDirections(nWorkerThreads)
        {
            for (int i = 0; i < numPreys; ++i)
                _cells[i] = std::make_shared<TCell>(_random, maxX, maxY, false);

            for (int i = 0; i < numPredators; ++i)
                _predators[i] = std::make_shared<TCell>(_random, maxX, maxY, true);

            for (int i = 0; i < _foodsPerCycle; ++i)
            {
                _foods.Reanimate() = Food<WorldProp>(_random, maxX, maxY);
            }

            for (int i = 0; i < _numWorkerThreads; ++i)
            {
                // Some in-efficiency here, yes
                _cellDirections[i] = std::vector<DirectionWithDistanceSquare>(numPreys);
                _foodDirections[i] = std::vector<DirectionWithDistanceSquare>(maxFoods);
                _predatorDirections[i] = std::vector<DirectionWithDistanceSquare>(numPredators);
            }
        }


		Population<Food<WorldProp>>& GetFoods() noexcept
		{
			return _foods;
		}

		Population<std::shared_ptr<TCell>> GetCells() noexcept
		{
			return _cells;	
		}

		Population<std::shared_ptr<TCell>> GetPredators() noexcept
		{
			return _predators; 
		}

		void SaveTo(std::ostream& stream)
		{
			stream.write(reinterpret_cast<const char*>(&_maxX), sizeof(_maxX));
			stream.write(reinterpret_cast<const char*>(&_maxY), sizeof(_maxY));
			stream.write(reinterpret_cast<const char*>(&_foodsPerCycle), sizeof(_foodsPerCycle));
			stream.write(reinterpret_cast<const char*>(&_nextFoodIdx), sizeof(_nextFoodIdx));

			_cells.SaveTo(stream, [&](std::shared_ptr<TCell> & item, std::ostream & s) {item->SaveTo(s); });
			_predators.SaveTo(stream, [&](std::shared_ptr<TCell> & item, std::ostream & s) {item->SaveTo(s); });
			_foods.SaveTo(stream, [&](Food<WorldProp> & item, std::ostream & s) {item.SaveTo(s); });
		}

		void LoadFrom(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&_maxX), sizeof(_maxX));
			stream.read(reinterpret_cast<char*>(&_maxY), sizeof(_maxY));
			stream.read(reinterpret_cast<char*>(&_foodsPerCycle), sizeof(_foodsPerCycle));
			stream.read(reinterpret_cast<char*>(&_nextFoodIdx), sizeof(_nextFoodIdx));

			_cells.LoadFrom(stream, [&](std::shared_ptr<TCell> & item, std::istream & s) {item->LoadFrom(s); });
			_predators.LoadFrom(stream, [&](std::shared_ptr<TCell> & item, std::istream & s) {item->LoadFrom(s); });
			_foods.LoadFrom(stream, [&](Food<WorldProp> & item, std::istream & s) {item.LoadFrom(s); });
		}

	private:

        int IterateBabyMaking(
			long step, 
			std::vector<std::shared_ptr<TCell>>& elements, 
			std::vector<std::pair<int, int>>& cloneMap,
			float birthEnergyConsumption, 
			float initialEnergy
		)  noexcept
        {
			int nextCloneMapIdx = 0;

			if (step == 0 || step % WorldProp::StepsPerBirthCheck != 0)
				return  0;

            int quant = static_cast<int>(elements.size() / 32);

            std::sort(std::begin(elements), std::end(elements),
                [](std::shared_ptr<TCell>& x, std::shared_ptr<TCell>& y) {
                return x->EnergyValue > y->EnergyValue;
            });

            int srcIdx = 0;
            int dstIdx = static_cast<int>(elements.size() - 1); //quant * 4;

			int numChild = 0; 
			while (srcIdx < dstIdx)
			{
				auto& src = elements[srcIdx];

				if (src->EnergyValue < 0.01f)
				{
					break;
				}

				if (src->EnergyValue < birthEnergyConsumption)
				{
					++srcIdx;
					continue;
				}

				//auto& dst = elements[dstIdx];
				src->EnergyValue -= birthEnergyConsumption;
				auto& cm = cloneMap[nextCloneMapIdx++];
				cm.first = srcIdx;
				cm.second = dstIdx--;
				++numChild;
				//CreateChild(src, dst, initialEnergy);
			}

			if (numChild == 0)
			{
				// a very special case: everyone is dead. 
				// Re-initialize energy levels with default values, and hope for the best 
				for (auto& el : elements)
				{
					el->EnergyValue = initialEnergy;
				}
			}
			else
			{
				// fill dead bodies with clones of the top ones 
				for (int idx = srcIdx + 1; idx < dstIdx; ++ idx)
				{
					auto& src = elements[0];
					auto& dst = elements[idx];
					
					if (dst->EnergyValue > 0.01f)
						continue;

					auto& cm = cloneMap[nextCloneMapIdx++];
					cm.first = 0;
					cm.second = idx;
				}
			}        

			return nextCloneMapIdx;
        }

	public:
        void Iterate(long step)  noexcept
        {
            if (step == 0)
                WorldInitialize();

            for (int idx = 0; idx < _foods.AliveSize(); ++idx)
                _foods[idx].Step(_maxX, _maxY, WorldProp::StepTimeDelta);

            if ((step % (WorldProp::StepsPerGeneration / _foodsPerCycle)) == 0)
            {
                GiveOneFood();
            }


            _grid.GridRun(
                [&](int idx, int n)
            {
                for (int cellIdx = idx; cellIdx < _cells.size(); cellIdx += n)
                {
					IterateEyeAndSensors(idx, step, _cells[cellIdx]);
                }
                for (int pIdx = idx; pIdx < _predators.size(); pIdx += n)
                {
					IterateEyeAndSensors(idx, step, _predators[pIdx]);
                }
            });

            _grid.GridRun(
                [&](int idx, int n)
            {
                for (int cellIdx = idx; cellIdx < _cells.size(); cellIdx += n)
                {
                    IterateCellThinkingAndMoving(idx, step, _cells[cellIdx]);
                }
                for (int pIdx = idx; pIdx < _predators.size(); pIdx += n)
                {
                    IterateCellThinkingAndMoving(idx, step, _predators[pIdx]);
                }
            });

            _grid.GridRun(
                [&](int idx, int n)
            {
                for (int cellIdx = idx; cellIdx < _cells.size(); cellIdx += n)
                {
                    IterateCellCollisions(idx, step, _cells[cellIdx]);
                }
            });

            // Kill any empty foods 
            _foods.KillAll([](Food<WorldProp>& f) { return f.EnergyValue < 0.001f; });

			if (step != 0 && step % WorldProp::StepsPerBirthCheck == 0)
			{
				int nmCells = IterateBabyMaking(
					step, _cells, 
					_interGenerationCloneMapCells, 
					WorldProp::BirthEnergyConsumption, WorldProp::InitialCellEnergy);
				int nmPredators = IterateBabyMaking(
					step, _predators, 
					_interGenerationCloneMapPredators,
					WorldProp::PredatorBirthEnergyConsumption,
					WorldProp::PredatorInitialValue);

				_grid.GridRun(
					[&](int idx, int n)
					{
						for (int i = idx; i < nmCells; i += n)
						{
							auto& p = _interGenerationCloneMapCells[i];
							CreateChild(_cells[p.first], _cells[p.second], WorldProp::InitialCellEnergy);
						}

						for (int i = idx; i < nmPredators; i += n)
						{
							auto& p = _interGenerationCloneMapPredators[i];
							CreateChild(_predators[p.first], _predators[p.second], WorldProp::PredatorInitialValue);
						}
					});

				_grid.GridRun(
					[&](int idx, int n)
					{
						for (int cellIdx = idx; cellIdx < _cells.size(); cellIdx += n)
						{
							auto& cell = _cells[cellIdx];
							if (cell->Age > WorldProp::OldSince)
								CreateChild(cell, cell, cell->EnergyValue);
						}
						for (int pIdx = idx; pIdx < _predators.size(); pIdx += n)
						{
							auto& cell = _predators[pIdx];
							if (cell->Age > WorldProp::OldSince)
								CreateChild(cell, cell, cell->EnergyValue);
						}
					});
			}
        }

	private:

        void IterateEyeAndSensors(int threadIdx, long step, std::shared_ptr<TCell>& cell)  noexcept
        {
            if (cell->EnergyValue < 0.00001f)
                return;

            auto& cellDirections = _cellDirections[threadIdx];
            auto& foodDirections = _foodDirections[threadIdx];
            auto& predatorDirections = _predatorDirections[threadIdx];

            cell->PrepareIteration();

            // Calculate light sensor values 

			// a bit of evolution force -- don't let predators see prey's food, so they don't 
			// cheat by waiting at the food points 
			if (!cell->IsPredator)
			{
				for (int idx = 0; idx < _foods.AliveSize(); ++idx)
				{
					auto& item = _foods[idx];

					float dx = item.LocationX - cell->LocationX;
					float dy = item.LocationY - cell->LocationY;
					foodDirections[idx].Set(dx, dy);
				}
			}

            for (int idx = 0; idx < _predators.size(); ++idx)
            {
				auto& item0 = _predators[idx + 0];
				float dx0 = item0->LocationX - cell->LocationX;
				float dy0 = item0->LocationY - cell->LocationY;
				predatorDirections[idx + 0].Set(dx0, dy0);				
            }

            for (int idx = 0; idx < _cells.size(); ++idx)
            {
                auto& item0 = _cells[idx + 0];
				float dx0 = item0->LocationX - cell->LocationX;
				float dy0 = item0->LocationY - cell->LocationY;
                cellDirections[idx + 0].Set(dx0, dy0);
            }

            auto& eye = cell->GetEye();

            for (unsigned int tripodIdx = 0;
                tripodIdx < WorldProp::EyeSizeNumTripods;
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
                if (!cell->IsPredator)  // same as above about predators seeing prey's foods 
				{
                    float value = 0.0f;

                    // This cell can see foods only
                    for (unsigned int idx = 0; idx < _foods.AliveSize(); ++idx)
                    {
                        auto& foodItem = _foods[idx];
                        if (foodItem.EnergyValue < 0.01f)
                            continue;

                        auto& food = foodDirections[idx];

						if (food.DistanceSquare > WorldProp::MaxDistanceSquareVisibility)
							continue;

                        float modulo = viewDirectionX * food.DirectionX + viewDirectionY * food.DirectionY;

                        if (modulo <= 0.0)
                            continue;

                        float invSqrRoot = Q_rsqrt(food.DistanceSquare);

                        float cosine = modulo * invSqrRoot;

                        //float distnaceSquare = (float) std::pow(food.Distance, 2.0);

                        float signalLevel =
                            (float)(foodItem.EnergyValue * std::pow(cosine, redCell.Width)
                                * invSqrRoot * invSqrRoot);

                        value += signalLevel;
                    }
                    cell->Network->InputVector[3 * tripodIdx] = 20000 * value;
                }

                // GREEN 
                {
                    float value = 0.0f;

                    // this cell can see predators only
                    for (unsigned int idx = 0; idx < cellDirections.size(); ++idx)
                    {
                        auto& cellDirection = cellDirections[idx];
                        auto& cellItem = _cells[idx];

                        if (cellItem->EnergyValue < 0.01f)
                            continue;

						if (cellDirection.DistanceSquare > WorldProp::MaxDistanceSquareVisibility)
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
                            (float)(WorldProp::InitialCellEnergy * std::pow(cosine, greenCell.Width)
                                * invSqrRoot * invSqrRoot);

                        value += signalLevel;
                    }

                    cell->Network->InputVector[3 * tripodIdx + 1] = 20000 * value;
                }

                // BLUE
                {
                    float value = 0.0f;

                    // this cell can see predators only
                    for (unsigned int idx = 0; idx < predatorDirections.size(); ++idx)
                    {
                        auto& predator = predatorDirections[idx];
                        auto& predatorItem = _predators[idx];

						if (predator.DistanceSquare > WorldProp::MaxDistanceSquareVisibility)
							continue;

                        if (predatorItem->EnergyValue < 0.01f)
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
                            (float)(WorldProp::PredatorInitialValue * std::pow(cosine, blueCell.Width)
                                * invSqrRoot * invSqrRoot);

                        value += signalLevel;
                    }
                    cell->Network->InputVector[3 * tripodIdx + 2] = 20000 * value;
                }
            }

			cell->Network->InputVector[WorldProp::CurrentEnergyLevelSensor] = cell->EnergyValue;
			cell->Network->InputVector[WorldProp::OrientationXSensor] = std::cosf(cell->Rotation);
			cell->Network->InputVector[WorldProp::OrientationYSensor] = std::sinf(cell->Rotation);
			cell->Network->InputVector[WorldProp::AbsoluteVelocitySensor] =
				std::sqrtf(cell->VelocityX * cell->VelocityX + cell->VelocityY * cell->VelocityY);
		}

        void IterateCellThinkingAndMoving(int threadIdx, long step, std::shared_ptr<TCell>& cell) noexcept
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

            float moveEnergyRequired = (std::abs(forceLeft) + std::abs(forceRight)) * WorldProp::MoveEnergyFactor;

            if (cell->IsPredator)
            {
                forwardForce *= 0.94f; // a bit slow and heavy

				if (cell->EnergyValue > WorldProp::PredatorsOvereatEnergy)
				{
					forwardForce *= WorldProp::PredatorsOveratSlowdownFactor;
				}
            }
            else if (!cell->IsPredator && cell->EnergyValue < WorldProp::SedatedAtEnergyLevel)
            {
                float slowdownFactor = cell->EnergyValue / WorldProp::SedatedAtEnergyLevel;
                forwardForce *= slowdownFactor;
                rotationForce *= slowdownFactor;
                moveEnergyRequired *= slowdownFactor;
            }

			//if ((step % 1024) > 512)
			//{
			//	moveEnergyRequired = 0.000001f;
			//	forwardForce = 0.0f;
			//	rotationForce = 0.0f;
			//}

            if (moveEnergyRequired <= cell->EnergyValue)
            {
                cell->EnergyValue -= moveEnergyRequired;

                cell->Rotation = LoopValue(cell->Rotation + rotationForce, 0.0f, (float)(M_PI * 2.0f));

				if constexpr (WorldProp::RealPhysics)
				{
					constexpr float timeDelta = WorldProp::StepTimeDelta;

					// Forward force is applied in the direction of engine thrust
					// Drag applied in the direction of movement
					// Do everything in two steps: 
					// 1. Apply drag fully 
					// 2. Apply forward force

					// AIR DRAG
					float velocitySquare = cell->VelocityX * cell->VelocityX + cell->VelocityY * cell->VelocityY;
					if (velocitySquare > 0.0000001f)
					{
						float velocity = std::sqrtf(velocitySquare);
						float velocityCube = velocity * velocitySquare;

						float airDrag =
							WorldProp::AirDragFactorCube * velocityCube +
							WorldProp::AirDragFactorQuadratic * velocitySquare +
							WorldProp::AirDragFactorLinear * velocity;


						float airDragDeltaV = airDrag / cell->Mass * timeDelta;

						// Air resistance can't decelerate us more than we have 
						airDragDeltaV = (airDragDeltaV > velocity) ? velocity : airDragDeltaV;


						//                                      This part is cos / sin of 
						//                                      the velocity direction vector
						//                                               |             
						//                                  /------------^-----------\ 
						cell->VelocityX -= airDragDeltaV * (cell->VelocityX / velocity) * timeDelta;
						cell->VelocityY -= airDragDeltaV * (cell->VelocityY / velocity) * timeDelta;
					}

					// THRUST 

					float forwardForceDeltaV = forwardForce / cell->Mass * timeDelta;

					cell->VelocityX += forwardForceDeltaV * static_cast<float>(std::cos(cell->Rotation)) * timeDelta;
					cell->VelocityY += forwardForceDeltaV * static_cast<float>(std::sin(cell->Rotation)) * timeDelta;


					// Finally - location update 
					cell->LocationX += cell->VelocityX * timeDelta; 
					cell->LocationY += cell->VelocityY * timeDelta; 

					cell->LocationX = LoopValue(cell->LocationX, 0.0f, (float)_maxX);
					cell->LocationY = LoopValue(cell->LocationY, 0.0f, (float)_maxY);
				}
				else
				{
					float dX = (float)(forwardForce * std::cos(cell->Rotation));
					float dY = (float)(forwardForce * std::sin(cell->Rotation));

					cell->LocationX = LoopValue(cell->LocationX + dX, 0.0f, (float)_maxX);
					cell->LocationY = LoopValue(cell->LocationY + dY, 0.0f, (float)_maxY);
				}
            }
            else if (!cell->IsPredator)
            {
                cell->EnergyValue = 0.0f; // so it has tried and failed
            }

        }

        void IterateCellCollisions(int threadIdx, long step, std::shared_ptr<TCell>& cell)  noexcept
        {
            if (!cell->IsPredator)
            {
                if (cell->EnergyValue < WorldProp::MaxEnergyCapacity)
                {
                    // Analyze the outcome - did it get any food? 
                    for (int foodIdx = 0; foodIdx < _foods.AliveSize(); ++foodIdx)
                    {
                        auto& food = _foods[foodIdx];
                        if (food.IsEmpty())
                            continue;

                        float pdx = std::powf(cell->LocationX - food.LocationX, 2.0f);
                        float pdy = std::powf(cell->LocationY - food.LocationY, 2.0f);

						constexpr float foodCaptureDistanceSquare = WorldProp::CellFoodCaptureDistance * WorldProp::CellFoodCaptureDistance;
                        if (pdx + pdy <= foodCaptureDistanceSquare)
                        {
							if (_random.NextDouble() <
								WorldProp::FoodConsumptionProbability(cell->VelocityX - food.VelocityX, cell->VelocityY - food.VelocityY))
							{
								cell->EnergyValue += food.Consume();
							}
                        }
                    }
                }

                if (cell->EnergyValue > WorldProp::SporeEnergyLevel)
                {
                    // Analyze the outcome - did it hit any predators? 
                    for (auto& predator : _predators)
                    {
                        if (predator->EnergyValue < 0.0001f)
                            continue; // skip deads 

                        float pdx = std::powf(cell->LocationX - predator->LocationX, 2.0f);
                        float pdy = std::powf(cell->LocationY - predator->LocationY, 2.0f);

						constexpr float captureDistanceSquare = WorldProp::PredatorCaptureDistance * WorldProp::PredatorCaptureDistance;

                        if (pdx + pdy <= captureDistanceSquare)
                        {
							if (_random.NextDouble() <
								WorldProp::PreyCatchingProbability(predator->VelocityX - cell->VelocityX, predator->VelocityY - cell->VelocityY))
							{
								float energy = InterlockedCompareExchange(&(cell->EnergyValue), 0.0f, cell->EnergyValue);

								if (!predator->PredatoryEat(energy))
								{
									// predator has failed 
									InterlockedCompareExchange(&(cell->EnergyValue), energy, 0.0f);
								}
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
                if (_foods[_nextFoodIdx].EnergyValue < WorldProp::FoodInitialValue / 2.0f)
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
                cell->EnergyValue = WorldProp::InitialCellEnergy;
                cell->Network->CleanOutputs();
                //cell.RandomizeLocation(_random, _maxX, _maxY);
            }

            for (auto& predator : _predators)
            {
                predator->EnergyValue = WorldProp::PredatorInitialValue;
                predator->Network->CleanOutputs();
            }

            GiveOneFood();
        }

        void CreateChild(std::shared_ptr<TCell>& source, std::shared_ptr<TCell>& destination, float initialEnergy)  noexcept
        {
            double rv = _random.NextDouble();
            bool severeMutations = (rv < WorldProp::SevereMutationFactor);

            float severity = (float)(1.0 - std::pow(rv / WorldProp::SevereMutationFactor,
                WorldProp::SevereMutationSlope)); // % of neurons to mutate

            destination->CloneFrom(*source, _random, _maxX, _maxY, severeMutations, severity);
            destination->ClonedFrom = -1;

            destination->EnergyValue = initialEnergy;
            destination->Network->CleanOutputs();

            destination->RandomizeLocation(_random, source->LocationX, source->LocationY, _maxX, _maxY);

            source->Age = 0; // kind of hack
        }


		//friend class WorldView<WorldProp>;
    };
}
