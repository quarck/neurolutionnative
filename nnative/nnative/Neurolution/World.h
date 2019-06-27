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

    struct Orientation
    {
        float OrientationX;
        float OrientationY;

		void SaveTo(std::ostream& stream)
		{
			stream.write(reinterpret_cast<const char*>(&OrientationX), sizeof(OrientationX));
			stream.write(reinterpret_cast<const char*>(&OrientationY), sizeof(OrientationY));
		}

		void LoadFrom(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&OrientationX), sizeof(OrientationX));
			stream.read(reinterpret_cast<char*>(&OrientationY), sizeof(OrientationY));
		}
	};

	struct Velocity
	{
		float VelocityX;
		float VelocityY;

		void SaveTo(std::ostream& stream)
		{
			stream.write(reinterpret_cast<const char*>(&VelocityX), sizeof(VelocityX));
			stream.write(reinterpret_cast<const char*>(&VelocityY), sizeof(VelocityY));
		}

		void LoadFrom(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&VelocityX), sizeof(VelocityX));
			stream.read(reinterpret_cast<char*>(&VelocityY), sizeof(VelocityY));
		}
	};

	template <typename WorldProp>
	struct LocationAndDirectionWithValue : public Orientation, public Location, public Velocity
    {
        float Value;

        void Step(Random& rnd, int maxX, int maxY)  noexcept
        {
            LocationX = LoopValue(
                LocationX + OrientationX + (float)(rnd.NextDouble() * 0.25 - 0.125), 0.0f, static_cast<float>(maxX));
            LocationY = LoopValue(
                LocationY + OrientationY + (float)(rnd.NextDouble() * 0.25 - 0.125), 0.0f, static_cast<float>(maxY));
        }

		void SaveTo(std::ostream& stream) 
		{
			stream.write(reinterpret_cast<const char*>(&Value), sizeof(Value));
			this->Orientation::SaveTo(stream);
			this->Location::SaveTo(stream);
			this->Velocity::SaveTo(stream);
		}

		void LoadFrom(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&Value), sizeof(Value));
			this->Orientation::LoadFrom(stream);
			this->Location::LoadFrom(stream);
			this->Velocity::LoadFrom(stream);
		}
	};

	template <typename WorldProp>
    struct DirectionWithDistanceSquare : public Orientation
    {
        float DistanceSquare;

        void Set(float dx, float dy)  noexcept
        {
            OrientationX = dx;
            OrientationY = dy;
            DistanceSquare = dx * dx + dy * dy;
        }

		void SaveTo(std::ostream& stream)
		{
			this->Orientation::SaveTo(stream);
		}

		void LoadFrom(std::istream& stream)
		{
			this->Orientation::LoadFrom(stream);
			DistanceSquare = OrientationX * OrientationX + OrientationY * OrientationY;
		}
    };

	template <typename WorldProp>
    struct Food : public LocationAndDirectionWithValue<WorldProp>
    {
		using BaseType = LocationAndDirectionWithValue<WorldProp>;

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

            while (BaseType::Value > 0.001)
            {
				float* basedAddr = &(this->BaseType::Value);
                float valueCopy = InterlockedCompareExchange(basedAddr, 0.0f, 0.0f);
                float newDelta = valueCopy < 0.5f ? valueCopy : 0.5f;
                float newValue = valueCopy - newDelta;

                // ReSharper disable once CompareOfFloatsByEqualityOperator
                if (InterlockedCompareExchange(basedAddr, newValue, valueCopy) == valueCopy)
                {
                    ret = valueCopy - newValue;
                    break;
                }
            }

            return ret;
        }

        bool IsEmpty() const  noexcept { return BaseType::Value < 0.00001; }

        void Reset(Random& rnd, int maxX, int maxY, bool valueOnly = false)  noexcept
        {
			BaseType::Value = WorldProp::FoodInitialValue;// * (0.5 + rnd.NextDouble());

            if (!valueOnly)
            {
                BaseType::LocationX = (float)rnd.Next(maxX);
                BaseType::LocationY = (float)rnd.Next(maxY);

				BaseType::OrientationX = (float)(rnd.NextDouble() * 0.5 - 0.25);
				BaseType::OrientationY = (float)(rnd.NextDouble() * 0.5 - 0.25);
            }
        }

		void SaveTo(std::ostream& stream)
		{
			this->BaseType::SaveTo(stream);
		}

		void LoadFrom(std::istream& stream)
		{
			this->BaseType::LoadFrom(stream);
		}
    };

	template <typename WorldProp, typename TNetworkNumericType>
    class World
    {
	public:
		using TProp = WorldProp;
		using TCell = Cell<WorldProp, TNetworkNumericType>;
	private:
        static constexpr float SQRT_2 = 1.4142135623730950488016887242097f; // unfortunately std::sqrt is not a constexpr function

        int _numWorkerThreads;


		Population<std::shared_ptr<TCell>> _cells;
		Population<std::shared_ptr<TCell>> _predators;
        Population<Food<WorldProp>> _foods;
		std::vector<std::pair<int, int>> _interGenerationCloneMapCells;
		std::vector<std::pair<int, int>> _interGenerationCloneMapPredators;

        std::vector<std::vector<DirectionWithDistanceSquare<WorldProp>>> _cellDirections;
        std::vector<std::vector<DirectionWithDistanceSquare<WorldProp>>> _predatorDirections;
        std::vector<std::vector<DirectionWithDistanceSquare<WorldProp>>> _foodDirections;

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
                _cellDirections[i] = std::vector<DirectionWithDistanceSquare<WorldProp>>(numPreys);
                _foodDirections[i] = std::vector<DirectionWithDistanceSquare<WorldProp>>(maxFoods);
                _predatorDirections[i] = std::vector<DirectionWithDistanceSquare<WorldProp>>(numPredators);
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

            if (std::any_of(
                    std::begin(elements),
                    std::end(elements),
                    [=](std::shared_ptr<TCell>& x) { return x->CurrentEnergy > birthEnergyConsumption; }
                ))
            {
                int quant = static_cast<int>(elements.size() / 32);

                std::sort(std::begin(elements), std::end(elements),
                    [](std::shared_ptr<TCell>& x, std::shared_ptr<TCell>& y) {
                    return x->CurrentEnergy > y->CurrentEnergy;
                });

                int srcIdx = 0;
                int dstIdx = static_cast<int>(elements.size() - 1); //quant * 4;

				while (srcIdx < dstIdx)
				{
					auto& src = elements[srcIdx];

					if (src->CurrentEnergy < birthEnergyConsumption)
					{
						++srcIdx;
						continue;
					}

					//auto& dst = elements[dstIdx];
					src->CurrentEnergy -= birthEnergyConsumption;
					auto& cm = cloneMap[nextCloneMapIdx++];
					cm.first = srcIdx;
					cm.second = dstIdx--;
					//CreateChild(src, dst, initialEnergy);
				}
        
                //for (auto& cell : elements)
                //{
                //    if (cell->Age > WorldProp::OldSince)
                //        CreateChild(cell, cell, cell->CurrentEnergy);
                //}
            }

			return nextCloneMapIdx;
        }

	public:
        void Iterate(long step)  noexcept
        {
            if (step == 0)
                WorldInitialize();

            for (int idx = 0; idx < _foods.AliveSize(); ++idx)
                _foods[idx].Step(_random, _maxX, _maxY);



            if ((step % (WorldProp::StepsPerGeneration / _foodsPerCycle)) == 0)
            {
                GiveOneFood();
            }


            _grid.GridRun(
                [&](int idx, int n)
            {
                for (int cellIdx = idx; cellIdx < _cells.size(); cellIdx += n)
                {
                    IterateCellEye(idx, step, _cells[cellIdx]);
                }
                for (int pIdx = idx; pIdx < _predators.size(); pIdx += n)
                {
                    IterateCellEye(idx, step, _predators[pIdx]);
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
            _foods.KillAll([](Food<WorldProp>& f) { return f.Value < 0.001f; });

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
								CreateChild(cell, cell, cell->CurrentEnergy);
						}
						for (int pIdx = idx; pIdx < _predators.size(); pIdx += n)
						{
							auto& cell = _predators[pIdx];
							if (cell->Age > WorldProp::OldSince)
								CreateChild(cell, cell, cell->CurrentEnergy);
						}
					});
			}
        }

	private:

        void IterateCellEye(int threadIdx, long step, std::shared_ptr<TCell>& cell)  noexcept
        {
            if (cell->CurrentEnergy < 0.00001f)
                return;

            auto& cellDirections = _cellDirections[threadIdx];
            auto& foodDirections = _foodDirections[threadIdx];
            auto& predatorDirections = _predatorDirections[threadIdx];

            cell->PrepareIteration();

            // Calculate light sensor values 

            /*float offsX = _maxX * 1.5f - cell->LocationX;
            float offsY = _maxY * 1.5f - cell->LocationY;
            float halfMaxX = _maxX / 2.0f;
            float halfMaxY = _maxY / 2.0f;*/


            // Calculate light sensor values 
            for (int idx = 0; idx < _foods.AliveSize(); ++idx)
            {
                auto& item = _foods[idx];

				float dx = item.LocationX - cell->LocationX;
				float dy = item.LocationY - cell->LocationY;

                //float dx = LoopValue(item.LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
                //float dy = LoopValue(item.LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;

                foodDirections[idx].Set(dx, dy);
            }

            for (int idx = 0; idx < _predators.size(); idx += 4)
            {
				auto& item0 = _predators[idx + 0];
				auto& item1 = _predators[idx + 1];
				auto& item2 = _predators[idx + 2];
				auto& item3 = _predators[idx + 3];

				float dx0 = item0->LocationX - cell->LocationX;
				float dy0 = item0->LocationY - cell->LocationY;
				float dx1 = item1->LocationX - cell->LocationX;
				float dy1 = item1->LocationY - cell->LocationY;
				float dx2 = item2->LocationX - cell->LocationX;
				float dy2 = item2->LocationY - cell->LocationY;
				float dx3 = item3->LocationX - cell->LocationX;
				float dy3 = item3->LocationY - cell->LocationY;

    //            float dx0 = LoopValue(item0->LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
    //            float dy0 = LoopValue(item0->LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;
    //            float dx1 = LoopValue(item1->LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
    //            float dy1 = LoopValue(item1->LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;
				//float dx2 = LoopValue(item2->LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
				//float dy2 = LoopValue(item2->LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;
				//float dx3 = LoopValue(item3->LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
				//float dy3 = LoopValue(item3->LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;

                predatorDirections[idx + 0].Set(dx0, dy0);
				predatorDirections[idx + 1].Set(dx1, dy1);
				predatorDirections[idx + 2].Set(dx2, dy2);
				predatorDirections[idx + 3].Set(dx3, dy3);
            }

            for (int idx = 0; idx < _cells.size(); idx += 4)
            {
                auto& item0 = _cells[idx + 0];
				auto& item1 = _cells[idx + 1];
				auto& item2 = _cells[idx + 2];
				auto& item3 = _cells[idx + 3];

				float dx0 = item0->LocationX - cell->LocationX;
				float dy0 = item0->LocationY - cell->LocationY;
				float dx1 = item1->LocationX - cell->LocationX;
				float dy1 = item1->LocationY - cell->LocationY;
				float dx2 = item2->LocationX - cell->LocationX;
				float dy2 = item2->LocationY - cell->LocationY;
				float dx3 = item3->LocationX - cell->LocationX;
				float dy3 = item3->LocationY - cell->LocationY;

				//float dx0 = LoopValue(item0->LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
				//float dy0 = LoopValue(item0->LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;
				//float dx1 = LoopValue(item1->LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
				//float dy1 = LoopValue(item1->LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;
				//float dx2 = LoopValue(item2->LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
				//float dy2 = LoopValue(item2->LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;
				//float dx3 = LoopValue(item3->LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
				//float dy3 = LoopValue(item3->LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;

                cellDirections[idx + 0].Set(dx0, dy0);
				cellDirections[idx + 1].Set(dx1, dy1);
				cellDirections[idx + 2].Set(dx2, dy2);
				cellDirections[idx + 3].Set(dx3, dy3);
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
                {
                    float value = 0.0f;

                    // This cell can see foods only
                    for (unsigned int idx = 0; idx < _foods.AliveSize(); ++idx)
                    {
                        auto& foodItem = _foods[idx];
                        if (foodItem.Value < 0.01f)
                            continue;

                        auto& food = foodDirections[idx];

						if (food.DistanceSquare > WorldProp::MaxDistanceSquareVisibility)
							continue;

                        float modulo = viewDirectionX * food.OrientationX + viewDirectionY * food.OrientationY;

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
                    for (unsigned int idx = 0; idx < cellDirections.size(); ++idx)
                    {
                        auto& cellDirection = cellDirections[idx];
                        auto& cellItem = _cells[idx];

                        if (cellItem->CurrentEnergy < 0.01f)
                            continue;

						if (cellDirection.DistanceSquare > WorldProp::MaxDistanceSquareVisibility)
							continue;

                        if (cellItem == cell)
                            continue;

                        float modulo = viewDirectionX * cellDirection.OrientationX + viewDirectionY * cellDirection.OrientationY;

                        if (modulo <= 0.0)
                            continue;

                        float invSqrRoot = Q_rsqrt(cellDirection.DistanceSquare);

                        float cosine = modulo * invSqrRoot;

                        // float distnaceSquare = (float)std::pow(predator.Distance, 2.0);

                        float signalLevel =
                            (float)(/*cellItem->CurrentEnergy*/WorldProp::InitialCellEnergy * std::pow(cosine, greenCell.Width)
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

                        if (predatorItem->CurrentEnergy < 0.01f)
                            continue;

                        if (predatorItem == cell)
                            continue;

                        float modulo = viewDirectionX * predator.OrientationX + viewDirectionY * predator.OrientationY;

                        if (modulo <= 0.0)
                            continue;

                        float invSqrRoot = Q_rsqrt(predator.DistanceSquare);

                        float cosine = modulo * invSqrRoot;

                        // float distnaceSquare = (float)std::pow(predator.Distance, 2.0);

                        float signalLevel =
                            (float)(/*predatorItem->CurrentEnergy*/WorldProp::PredatorInitialValue * std::pow(cosine, blueCell.Width)
                                * invSqrRoot * invSqrRoot);

                        value += signalLevel;
                    }
                    cell->Network->InputVector[3 * tripodIdx + 2] = 20000 * value;
                }
            }

			cell->Network->InputVector[WorldProp::CurrentEnergyLevelSensor] = cell->CurrentEnergy;
			cell->Network->InputVector[WorldProp::OrientationXSensor] = TNetworkNumericType(std::cos(cell->Rotation));
			cell->Network->InputVector[WorldProp::OrientationYSensor] = TNetworkNumericType(std::sin(cell->Rotation));
			cell->Network->InputVector[WorldProp::AbsoluteVelocitySensor] = 0.0; // Currently not calculating velocities 
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
            }
            else if (!cell->IsPredator && cell->CurrentEnergy < WorldProp::SedatedAtEnergyLevel)
            {
                float slowdownFactor = cell->CurrentEnergy / WorldProp::SedatedAtEnergyLevel;
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

        void IterateCellCollisions(int threadIdx, long step, std::shared_ptr<TCell>& cell)  noexcept
        {
            if (!cell->IsPredator)
            {
                if (cell->CurrentEnergy < WorldProp::MaxEnergyCapacity)
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

                if (cell->CurrentEnergy > WorldProp::SporeEnergyLevel)
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
                if (_foods[_nextFoodIdx].Value < WorldProp::FoodInitialValue / 2.0f)
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
                cell->CurrentEnergy = WorldProp::InitialCellEnergy;
                cell->Network->CleanOutputs();
                //cell.RandomizeLocation(_random, _maxX, _maxY);
            }

            for (auto& predator : _predators)
            {
                predator->CurrentEnergy = WorldProp::PredatorInitialValue;
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

            destination->CurrentEnergy = initialEnergy;
            destination->Network->CleanOutputs();

            destination->RandomizeLocation(_random, _maxX, _maxY);

            source->Age = 0; // kind of hack
        }


		//friend class WorldView<WorldProp>;
    };
}
