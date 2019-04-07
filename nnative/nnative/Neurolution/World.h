﻿#pragma once 
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
	
	struct Location
	{
		float LocationX;
		float LocationY;
	};

	struct Direction
	{
		float DirectionX;
		float DirectionY;
	};

	struct LocationAndDirectionWithvalue : public Direction, public Location
	{
		float Value;

		void Step(Random& rnd, int maxX, int maxY)
		{
			LocationX = LoopValue(
				LocationX + DirectionX + (float)(rnd.NextDouble() * 0.25 - 0.125), 0.0f, static_cast<float>(maxX));
			LocationY = LoopValue(
				LocationY + DirectionY + (float)(rnd.NextDouble() * 0.25 - 0.125), 0.0f, static_cast<float>(maxY));
		}
	};

	struct DirectionWithDistanceSquare : public Direction
	{
		float DistanceSquare;

		void Set(float dx, float dy)
		{
			DirectionX = dx;
			DirectionY = dy;
			DistanceSquare = dx * dx + dy * dy;
		}
	};

	//struct Predator: public LocationAndDirectionWithvalue
 //   {
	//	float ViewRotation{ 0.0f };

	//	Predator()
	//	{
	//	}

 //       Predator(Random& rnd, int maxX, int maxY)
 //       {
	//        Reset(rnd, maxX, maxY);
 //       }

 //       void Reset(Random& rnd, int maxX, int maxY, bool valueOnly = false)
 //       {
 //           Value = AppProperties::PredatorInitialValue;// * (0.5 + rnd.NextDouble());

	//		ViewRotation = (float)(rnd.NextDouble() * 90.0);
	//	
	//		if (!valueOnly)
	//		{
	//			LocationX = rnd.Next(maxX);
	//			LocationY = rnd.Next(maxY);
	//			DirectionX = (float)(rnd.NextDouble() - 0.5);
	//			DirectionY = (float)(rnd.NextDouble() - 0.5);
	//		}
	//	}

 //       void Eat(float addValue)
 //       {
 //           for(;;)
 //           {
 //               float valueCopy = Value;
 //               float newValue = valueCopy + addValue;

	//			if (InterlockedCompareExchange(&Value, newValue, valueCopy) == valueCopy)
 //               {
	//				/*DirectionX *= 1.2f;
	//				DirectionY *= 1.2f;*/
 //                   break;
 //               }
 //           }
 //       }
	//};

	struct Food : public LocationAndDirectionWithvalue
	{
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

		bool IsEmpty() const { return Value < 0.00001; }

		void Reset(Random& rnd, int maxX, int maxY, bool valueOnly = false)
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
	};

	struct World
	{
		static constexpr float Sqrt2 = 1.4142135623730950488016887242097f;

		//        public const float FoodRadiusSquare = AppProperties.FoodRadius * AppProperties.FoodRadius;
		  //      public const float PredatorRadiusSquare = AppProperties.PredatorRadius * AppProperties.PredatorRadius;

		std::vector<std::shared_ptr<Cell>> Cells;
		std::vector<std::shared_ptr<Cell>> Predators;
		std::vector<Food> Foods;

		std::vector<DirectionWithDistanceSquare> CellDirections;
		std::vector<DirectionWithDistanceSquare> PredatorDirections;
		std::vector<DirectionWithDistanceSquare> FoodDirections;

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
			, CellDirections(size)
			, Foods(foodItems)
			, FoodDirections(foodItems)
			, Predators(predatorItems)
			, PredatorDirections(predatorItems)
		{
			for (int i = 0; i < size; ++i)
				Cells[i] = std::make_shared<Cell>(_random, maxX, maxY, false);

			for (int i = 0; i < predatorItems; ++i)
				Predators[i] = std::make_shared<Cell>(_random, maxX, maxY, true);

			for (int i = 0; i < foodItems; ++i)
				Foods[i] = Food(_random, maxX, maxY);

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


		std::vector<int> multipliers{ 6, 3, 2, 1 };

		void IterateBabyMaking(long step, std::vector<std::shared_ptr<Cell>>& elements)
		{
			if (step != 0 && (step % AppProperties::StepsPerBirthCheck == 0)
				&& (std::any_of(
					std::begin(elements),
					std::end(elements),
					[](std::shared_ptr<Cell>& x) { return x->CurrentEnergy > AppProperties::BirthEnergyConsumption; }
				)
					|| (step % AppProperties::SerializeTopEveryNStep == 0)
					))
				{
				int quant = static_cast<int>(elements.size() / 16); 

				std::sort(std::begin(elements), std::end(elements),
					[](std::shared_ptr<Cell>& x, std::shared_ptr<Cell>& y) {
					return x->CurrentEnergy > y->CurrentEnergy;
				});

				//if (step % AppProperties::SerializeTopEveryNStep == 0)
				//{
				//	SerializeBest(elements[0], step);

				//	if (step % AppProperties::SerializeWorldEveryNStep == 0)
				//		SerializeWorld(elements, step);
				//}

				int srcIdx = 0;
				int dstIdx = static_cast<int>(elements.size() - 1); //quant * 4;

				for (auto multiplier : multipliers)
				{
					for (int q = 0; q < quant; ++q)
					{
						auto& src = elements[srcIdx++];

						for (int j = 0; j < multiplier; ++j)
						{
							if (src->CurrentEnergy < AppProperties::BirthEnergyConsumption)
								break;

							auto& dst = elements[dstIdx--];

							float energy = src->IsPredator ? AppProperties::PredatorInitialValue : AppProperties::InitialCellEnergy;
							src->CurrentEnergy -= AppProperties::BirthEnergyConsumption;

							MakeBaby(src, dst, energy);
						}
					}
				}

				for (auto& cell : elements)
				{
					if (cell->Age > AppProperties::OldSince)
						MakeBaby(cell, cell, cell->CurrentEnergy);
				}
			}

		}

		void Iterate(long step)
		{
			if (step == 0)
				WorldInitialize();

			for (auto& food : Foods)
				food.Step(_random, _maxX, _maxY);

			//for (auto& predator: Predators)
			//	predator.Step(_random, _maxX, _maxY);

			if ((step % AppProperties::StepsPerGeneration) == 0)
			{
				ResetFoods();
			}

			if (MultiThreaded)
			{
				//Parallel.ForEach(
				//    Cells,
				//    cell => IterateCell(step, cell)
				//);
			}
			else
			{
				for (auto& cell : Cells)
					IterateCell(step, cell);

				for (auto& cell : Predators)
					IterateCell(step, cell);
			}

			IterateBabyMaking(step, Cells);
			IterateBabyMaking(step, Predators);
		}

		void IterateCell(long step, std::shared_ptr<Cell>& cell)
		{
			if (cell->CurrentEnergy < 0.00001f)
				return;

			cell->PrepareIteration();

			// Calculate light sensor values 

			float offsX = _maxX * 1.5f - cell->LocationX;
			float offsY = _maxY * 1.5f - cell->LocationY;
			float halfMaxX = _maxX / 2.0f;
			float halfMaxY = _maxY / 2.0f;


			// Calculate light sensor values 
			for (int idx = 0; idx < Foods.size(); ++idx)
			{
				auto& item = Foods[idx];

				float dx = LoopValue(item.LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
				float dy = LoopValue(item.LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;

				FoodDirections[idx].Set(dx, dy);
			}

			for (int idx = 0; idx < Predators.size(); ++idx)
			{
				auto& item = Predators[idx];

				float dx = LoopValue(item->LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
				float dy = LoopValue(item->LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;

				PredatorDirections[idx].Set(dx, dy);
			}

			for (int idx = 0; idx < Cells.size(); ++idx)
			{
				auto& item = Cells[idx];

				float dx = LoopValue(item->LocationX + offsX, 0.0f, (float)_maxX) - halfMaxX;
				float dy = LoopValue(item->LocationY + offsY, 0.0f, (float)_maxY) - halfMaxY;

				CellDirections[idx].Set(dx, dy);
			}

			auto& eye = cell->GetEye();

			for (unsigned int tripodIdx = 0; 
				tripodIdx < AppProperties::EyeSizeNumTripods; 
				++tripodIdx)
			{

				auto& redCell = eye[3 * tripodIdx];
				auto& greenCell = eye[3 * tripodIdx+1];
				auto& blueCell = eye[3 * tripodIdx+2];
				// 
				float viewDirection = cell->Rotation + redCell.Direction;

				float viewDirectionX = (float)std::cos(viewDirection);
				float viewDirectionY = (float)std::sin(viewDirection);

				// RED 
				{
					float value = 0.0f;

					// This cell can see foods only
					for (unsigned int idx = 0; idx < FoodDirections.size(); ++idx)
					{
						auto& foodItem = Foods[idx];
						if (foodItem.Value < 0.01f)
							continue;

						auto& food = FoodDirections[idx];

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
					cell->Network->InputVector[3 * tripodIdx] = 1000 * value;
				}
				
				// GREEN 
				{
					float value = 0.0f;

					// this cell can see predators only
					for (unsigned int idx = 0; idx < CellDirections.size(); ++idx)
					{
						auto& cellDirection = CellDirections[idx];
						auto& cellItem = Cells[idx];

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
							(float)(cellItem->CurrentEnergy * std::pow(cosine, greenCell.Width)
								* invSqrRoot * invSqrRoot);

						value += signalLevel;
					}

					cell->Network->InputVector[3 * tripodIdx + 1] = 1000 * value;
				}
				
				// BLUE
				{
					float value = 0.0f;

					// this cell can see predators only
					for (unsigned int idx = 0; idx < PredatorDirections.size(); ++idx)
					{
						auto& predator = PredatorDirections[idx];
						auto& predatorItem = Predators[idx];

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
							(float)(predatorItem->CurrentEnergy * std::pow(cosine, blueCell.Width)
								* invSqrRoot * invSqrRoot);

						value += signalLevel;
					}
					cell->Network->InputVector[3 * tripodIdx + 2] = 1000 * value;
				}
			}

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

			float forwardForce = ((forceLeft + forceRight) / Sqrt2);
			float rotationForce = (forceLeft - forceRight) / Sqrt2;

			float moveEnergyRequired = (std::abs(forceLeft) + std::abs(forceRight)) * AppProperties::MoveEnergyFactor;

			if (cell->IsPredator)
			{
				moveEnergyRequired *= 2.25f;
				forwardForce *= 1.5;
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
			else
			{
				cell->CurrentEnergy = 0.0f; // so it has tried and failed
			}

			if (!cell->IsPredator)
			{
				if (cell->CurrentEnergy < AppProperties::MaxEnergyCapacity)
				{
					// Analyze the outcome - did it get any food? 
					for (auto& food : Foods)
					{
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
					for (auto& predator : Predators)
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
			//else
			//{
			//	if (cell->CurrentEnergy < AppProperties::MaxEnergyCapacity)
			//	{
			//		// Analyze the outcome - did it get any prey? 
			//		for (auto& food : Cells)
			//		{
			//			if (food->CurrentEnergy < 0.001f)
			//				continue;

			//			float dx = std::abs(cell->LocationX - food->LocationX);
			//			float dy = std::abs(cell->LocationY - food->LocationY);

			//			float dv = (float)(std::sqrt(food->CurrentEnergy) / 2.0 * 5);
			//			if (dv < 3.0f) dv = 3.0f;

			//			if (dx <= dv && dy <= dv)
			//			{
			//				cell->CurrentEnergy += food.Consume();
			//			}
			//		}
			//	}				
			//}

		}

		void ResetFoods()
		{
			// restore any foods
			for (auto& food : Foods)
				food.Reset(_random, _maxX, _maxY);
		}

		void WorldInitialize()
		{
			// cleanput outputs & foods 
			for (auto& cell : Cells)
			{
				cell->CurrentEnergy = AppProperties::InitialCellEnergy;
				cell->Network->CleanOutputs();
				//cell.RandomizeLocation(_random, _maxX, _maxY);
			}

			for (auto& predator : Predators)
			{
				predator->CurrentEnergy = AppProperties::PredatorInitialValue;
				predator->Network->CleanOutputs();
			}

			ResetFoods();
		}

		void MakeBaby(std::shared_ptr<Cell>& source, std::shared_ptr<Cell>& destination, float initialEnergy)
		{
			double rv = _random.NextDouble();
			bool severeMutations = (rv < AppProperties::SevereMutationFactor);

			float severity = (float)(1.0 - std::pow(rv / AppProperties::SevereMutationFactor,
				AppProperties::SevereMutationSlope)); // % of neurons to mutate

			destination->CloneFrom(*source, _random, _maxX, _maxY, severeMutations, severity);
			destination->ClonedFrom = -1;

			destination->CurrentEnergy = initialEnergy;
			destination->Network->CleanOutputs();

			destination->RandomizeLocation(_random, _maxX, _maxY);
		}

	};
}
