#pragma once 
//using System;
//using System.Collections.Generic;
//using System.IO;
//using System.Linq;
//using System.Threading;
//using System.Threading.Tasks;

#include "../Random.h"

#include "Cell.h"

#include <algorithm>

namespace Neurolution
{
	struct LocationWithvalue
	{
		float Value;
		float LocationX;
		float LocationY;
	};

	struct DirectionWithDistance
	{
		float DirectionX;
		float DirectionY;
		float Distance;
	};

	float InterlockedCompareExchange(float volatile * _Destination, float _Exchange, float _Comparand)
	{
		static_assert(sizeof(float) == sizeof(long),
			"InterlockedCompareExchange(float*,float,float): expect float to be same size as long");

		auto res = ::_InterlockedCompareExchange(
			reinterpret_cast<volatile long*>(_Destination),
			*reinterpret_cast<long*>(&_Exchange),
			*reinterpret_cast<long*>(&_Comparand)
		);

		return *reinterpret_cast<float*>(&res);
	}

	struct Predator: public LocationWithvalue
    {
		Predator()
		{
		}

        Predator(Random& rnd, int maxX, int maxY)
        {
	        Reset(rnd, maxX, maxY);
        }

        void Reset(Random& rnd, int maxX, int maxY)
        {
            Value = AppProperties::PredatorInitialValue;// * (0.5 + rnd.NextDouble());
            float radius = AppProperties::FoodMinDistanceToBorder;

            LocationX = radius + rnd.Next(maxX - 2 * (int)radius);
            LocationY = radius + rnd.Next(maxY - 2 * (int)radius);
        }

        void Eat(float addValue)
        {
            for(;;)
            {
				static_assert(sizeof(float) == sizeof(long), "expect float to be same size as long");

                float valueCopy = Value;
                float newValue = valueCopy + addValue;

				if (InterlockedCompareExchange(&Value, newValue, valueCopy) == valueCopy)
                {
                    break;
                }
            }
        }
	};

    struct Food : public LocationWithvalue
    {
		float Value;

		Food()
		{
		}

        Food(Random& rnd, int maxX, int maxY)
        {
            Reset(rnd, maxX, maxY);
        }

        float Consume(float delta = 0.071f)
        {
            float ret = 0.0f;

            while (Value > 0.001)
            {
                float valueCopy = Value;
                float newDelta = (float) (valueCopy > AppProperties::InitialCellEnergy * 0.9 ? 0.1 : 0.01);
                float newValue = valueCopy * (1 - newDelta);

                // ReSharper disable once CompareOfFloatsByEqualityOperator
                if (InterlockedCompareExchange(&Value, newValue, valueCopy) == valueCopy)
                {
                    ret = valueCopy - newValue;
                    break;
                }
            }

            return ret;
        }

		bool IsEmpty() const { return Value < 0.00001; }

        void Reset(Random& rnd, int maxX, int maxY)
        {
            Value = AppProperties::FoodInitialValue;// * (0.5 + rnd.NextDouble());
            float radius = AppProperties::FoodMinDistanceToBorder;

            LocationX = radius + rnd.Next(maxX - 2 * (int)radius);
            LocationY = radius + rnd.Next(maxY - 2 * (int)radius);
        }
	};

    struct World
    {
        static constexpr float Sqrt2 = 1.4142135623730950488016887242097f;

//        public const float FoodRadiusSquare = AppProperties.FoodRadius * AppProperties.FoodRadius;
  //      public const float PredatorRadiusSquare = AppProperties.PredatorRadius * AppProperties.PredatorRadius;

        std::vector<std::shared_ptr<Cell>> Cells;
        std::vector<Food> Foods;
        std::vector<Predator> Predators;

		std::vector<DirectionWithDistance> FoodDirections;
		std::vector<DirectionWithDistance> PredatorDirections;

        int _maxX;
        int _maxY;

		Random _random{};

        std::string _workingFolder;
		bool _workingFolderCreated{ false };

		bool MultiThreaded{ false };

        World(const std::string& workingFolder, int size, int foodItems, int predatorItems, int maxX, int maxY)
			: _maxX(maxX)
			, _maxY(maxY)
			, _workingFolder(workingFolder)
			, Cells(size)
			, Foods(foodItems)
			, FoodDirections(foodItems)
			, Predators(predatorItems)
			, PredatorDirections(predatorItems)
        {
            for (int i = 0; i < size; ++i)
                Cells[i] = std::make_shared<Cell>(_random, maxX, maxY);

            for (int i = 0; i < foodItems; ++i)
                Foods[i] = Food(_random, maxX, maxY);

            for(int i = 0; i < predatorItems; ++ i)
                Predators[i] = Predator(_random, maxX, maxY);
        }

        void InitializeFromTopFile(const std::string& filename)
        {
            //Cell masterCell = CellUtils.ReadCell(filename);
            //if (masterCell != null)
            //{
            //    foreach (var cell in Cells)
            //    {
            //        cell.CloneFrom(masterCell, _random, _maxX, _maxY, false, 0.0f);
            //        cell.Random = new Random(_random.Next());
            //        cell.CurrentEnergy = AppProperties.InitialCellEnergy;
            //        cell.LocationX = _random.Next(_maxX);
            //        cell.LocationY = _random.Next(_maxY);
            //    }
            //}
        }

        void InitializeFromWorldFile(const std::string& filename)
        {
            //List<Cell> cells = CellUtils.ReadCells(filename);
            //if (cells != null)
            //{
            //    Cells = cells.ToArray();

            //    foreach (var cell in Cells)
            //    {
            //        cell.Random = new Random(_random.Next());
            //        cell.CurrentEnergy = AppProperties.InitialCellEnergy;
            //        cell.LocationX = _random.Next(_maxX);
            //        cell.LocationY = _random.Next(_maxY);
            //    }
            //}
        }


        void SerializeBest(const std::shared_ptr<Cell>& cell, long step)
        {
            //if (!_workingFolderCreated)
            //{
            //    Directory.CreateDirectory(_workingFolder);
            //    _workingFolderCreated = true;
            //}

            //DateTime now = DateTime.Now;
            //string filename = $"{_workingFolder}/{step:D8}-{now:yyyy-MM-dd-HH-mm-ss}-top.xml";

            //CellUtils.SaveCell(filename, cell);
        }

        void SerializeWorld(const std::vector<std::shared_ptr<Cell>>& world, long step)
        {
            //if (!_workingFolderCreated)
            //{
            //    Directory.CreateDirectory(_workingFolder);
            //    _workingFolderCreated = true;
            //}

            //DateTime now = DateTime.Now;
            //string filename = $"{_workingFolder}/{step:D8}-{now:yyyy-MM-dd-HH-mm-ss}-world.xml";

            //CellUtils.SaveCells(filename, world);
        }

        void Save()
        {
            SerializeWorld(Cells, -1);
        }


		std::vector<int> multipliers { 6, 3, 2, 1 }; 

        void Iterate(long step)
        {
            if (step == 0)
                WorldReset();

            if ((step % AppProperties::StepsPerGeneration) == 0)
                FoodAndPredatorReset();

            if (MultiThreaded)
            {
                //Parallel.ForEach(
                //    Cells,
                //    cell => IterateCell(step, cell)
                //);
            }
            else
            {
				for (auto& cell:  Cells)
                    IterateCell(step, cell);
            }

			if (step != 0 && (step % AppProperties::StepsPerBirthCheck == 0)
				&& (std::any_of(
						std::begin(Cells), 
						std::end(Cells),
						[](std::shared_ptr<Cell>& x){ return x->CurrentEnergy > AppProperties::BirthEnergyConsumption; }
					)
					|| (step % AppProperties::SerializeTopEveryNStep == 0)
					))
            {
                int quant = AppProperties::WorldSize / 16; // == 32 basically

				std::sort(std::begin(Cells), std::end(Cells), 
					[](std::shared_ptr<Cell>& x, std::shared_ptr<Cell>& y) {
					return x->CurrentEnergy > y->CurrentEnergy;
				});

                if (step % AppProperties::SerializeTopEveryNStep == 0)
                {
                    SerializeBest(Cells[0], step);

                    if (step % AppProperties::SerializeWorldEveryNStep == 0)
                        SerializeWorld(Cells, step);
                }

                int srcIdx = 0;
                int dstIdx = static_cast<int>(Cells.size() - 1); //quant * 4;

                for (auto multiplier: multipliers)
                {
                    for (int q = 0; q < quant; ++q)
                    {
                        auto& src = Cells[srcIdx++];

                        for (int j = 0; j < multiplier; ++j)
                        {
                            if (src->CurrentEnergy < AppProperties::BirthEnergyConsumption)
                                break;

                            auto& dst = Cells[dstIdx--];

                            float energy = AppProperties::InitialCellEnergy;
                            src->CurrentEnergy -= AppProperties::BirthEnergyConsumption;

                            MakeBaby(src, dst, energy);
                        }
                    }
                }

				for (auto& cell : Cells) 
				{
					if (cell->Age > AppProperties::OldSince)
						MakeBaby(cell, cell, cell->CurrentEnergy);
				}
            }
        }

        void IterateCell(long step, std::shared_ptr<Cell>& cell)
        {
//            float bodyDirectionX = Math.Cos(cell.Rotation);
//            float bodyDirectionY = Math.Sin(cell.Rotation);

            cell->PrepareIteration();

            // Calculate light sensor values 

			std::transform(
				std::begin(Foods),
				std::end(Foods),
				std::begin(FoodDirections), 
				[&cell](Food& item) {
				return DirectionWithDistance{ 
					item.LocationX - cell->LocationX, 
					item.LocationY - cell->LocationY, 
					(float)(std::sqrt(
								std::pow(item.LocationX - cell->LocationX, 2.0) +
								std::pow(item.LocationY - cell->LocationY, 2.0)
								))
				};
				}
			);

			std::transform(
					std::begin(Predators),
					std::end(Predators),
					std::begin(PredatorDirections),
					[&cell](Predator& item) {
						return DirectionWithDistance{
							item.LocationX - cell->LocationX,
							item.LocationY - cell->LocationY,
							(float)(std::sqrt(
								std::pow(item.LocationX - cell->LocationX, 2.0) +
								std::pow(item.LocationY - cell->LocationY, 2.0)
								))
						};
					}
				);

			auto& eye = cell->GetEye();

            for (unsigned int eyeIdx = 0; eyeIdx < eye.size(); ++ eyeIdx)
            {
                auto& eyeCell = eye[eyeIdx];
                // 
                float viewDirection = cell->Rotation + eyeCell.Direction;

                float viewDirectionX = (float) std::cos(viewDirection);
                float viewDirectionY = (float) std::sin(viewDirection);

                float value = 0.0f;

                if (eyeCell.SensetiveToRed)
                {
                    // This cell can see foods only
					for (unsigned int idx = 0; idx < FoodDirections.size(); ++ idx)
                    {
						auto& food = FoodDirections[idx];
						auto& foodItem = Foods[idx];

                        float modulo = viewDirectionX*food.DirectionX + viewDirectionY*food.DirectionY;

                        if (modulo <= 0.0)
                            continue;

                        float cosine = modulo/food.Distance;

                        float distnaceSquare = (float) std::pow(food.Distance, 2.0);

                        float signalLevel =
                            (float) (foodItem.Value * std::pow(cosine, eyeCell.Width)
                                     / distnaceSquare);

                        value += signalLevel;
                    }
                }
                else
                {
                    // this cell can see predators only
					for (unsigned int idx = 0; idx < PredatorDirections.size(); ++ idx)
                    {
						auto& predator = PredatorDirections[idx];
						auto& predatorItem = Predators[idx];

                        float modulo = viewDirectionX * predator.DirectionX + viewDirectionY * predator.DirectionY;

                        if (modulo <= 0.0)
                            continue;

                        float cosine = modulo / predator.Distance;

                        float distnaceSquare = (float)std::pow(predator.Distance, 2.0);

                        float signalLevel =
                            (float)(predatorItem.Value * std::pow(cosine, eyeCell.Width)
                                     / distnaceSquare);

                        value += signalLevel;
                    }
                }

                cell->Network->InputVector[eyeIdx] = 1000 * value;
            }


            // Iterate network finally
            cell->IterateNetwork(step);

            // Execute action - what is ordered by the neuron network
            float forceLeft = cell->MoveForceLeft;
            float forceRight = cell->MoveForceRight;

			float forwardForce = ((forceLeft + forceRight) / Sqrt2);
            float rotationForce = (forceLeft - forceRight) / Sqrt2;

			float moveEnergyRequired = (std::abs(forceLeft) + std::abs(forceRight)) * AppProperties::MoveEnergyFactor;

            if (moveEnergyRequired <= cell->CurrentEnergy)
            {
                cell->CurrentEnergy -= moveEnergyRequired;

                cell->Rotation += rotationForce;
                if (cell->Rotation > M_PI*2.0)
                    cell->Rotation -= (float)(M_PI*2.0);
                else if (cell->Rotation < 0.0)
                    cell->Rotation += (float)(M_PI*2.0);

                float dX = (float) (forwardForce*std::cos(cell->Rotation));
                float dY = (float) (forwardForce*std::sin(cell->Rotation));

                cell->LocationX += dX;
                cell->LocationY += dY;
            }
            else
            {
                cell->CurrentEnergy = 0.0f; // so it has tried and failed
            }

            if (cell->LocationX < 0.0)
                cell->LocationX += _maxX;
            else if (cell->LocationX >= _maxX)
                cell->LocationX -= _maxX;

            if (cell->LocationY < 0.0)
                cell->LocationY += _maxY;
            else if (cell->LocationY >= _maxY)
                cell->LocationY -= _maxY;

            if (cell->CurrentEnergy < AppProperties::MaxEnergyCapacity)
            {
                // Analyze the outcome - did it get any food? 
                for (auto& food: Foods)
                {
                    if (food.IsEmpty())
                        continue;

                    float dx = std::abs(cell->LocationX - food.LocationX);
                    float dy = std::abs(cell->LocationY - food.LocationY);

                    float dv = (float) (std::sqrt(food.Value) / 2.0 * 5);

                    if (dx <= dv && dy <= dv)
                    {
                        cell->CurrentEnergy += food.Consume();
                    }
                }
            }

            if (cell->CurrentEnergy > AppProperties::SporeEnergyLevel)
            {
                // Analyze the outcome - did it hit any predators? 
                for (auto& predator: Predators)
                {
                    float dx = std::abs(cell->LocationX - predator.LocationX);
                    float dy = std::abs(cell->LocationY - predator.LocationY);

                    float dv = (float)(std::sqrt(predator.Value) / 2.0 * 5);

                    if (dx <= dv && dy <= dv)
                    {
                        predator.Eat(cell->CurrentEnergy);
                        cell->CurrentEnergy = 0.0f;
                    }
                }
            }
        }

        void FoodAndPredatorReset()
        {
            // restore any foods
            for (auto& food: Foods)
                food.Reset(_random, _maxX, _maxY);

            for (auto& predator: Predators)
                predator.Reset(_random, _maxX, _maxY);
        }

        void WorldReset()
        {
            // cleanput outputs & foods 
            for (auto& cell: Cells)
            {
                cell->CurrentEnergy = AppProperties::InitialCellEnergy;
                cell->Network->CleanOutputs();
                //cell.RandomizeLocation(_random, _maxX, _maxY);
            }

            FoodAndPredatorReset();
        }

        void MakeBaby(std::shared_ptr<Cell>& source, std::shared_ptr<Cell>& destination, float initialEnergy)
        {
            double rv = _random.NextDouble();
            bool severeMutations = (rv < AppProperties::SevereMutationFactor);

            float severity = (float) (1.0 - std::pow(rv / AppProperties::SevereMutationFactor,
                                          AppProperties::SevereMutationSlope)); // % of neurons to mutate

            destination->CloneFrom(*source, _random, _maxX, _maxY, severeMutations, severity);
            destination->ClonedFrom = -1;

            destination->CurrentEnergy = initialEnergy;
            destination->Network->CleanOutputs();

            destination->RandomizeLocation(_random, _maxX, _maxY);
        }
	};
}
