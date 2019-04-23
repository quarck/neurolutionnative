﻿/*
*/
#pragma once 

#define _USE_MATH_DEFINES

#include <memory>
#include <math.h>

#include "../Random.h"
#include "../Utils.h"

#include "AppProperties.h"
#include "NeuronNetwork.h"


namespace Neurolution
{
    struct Cell
    {
        std::shared_ptr<NeuronNetwork> Network;

        static constexpr float TailLength = AppProperties::CellTailLength;
        static constexpr float EyeBase = AppProperties::CellEyeBase;

        float LocationX{ 0.0f };
        float LocationY{ 0.0f };
        float Rotation{ 0.0f };

        long Age{ 0 };

        float MoveForceLeft{ 0.0f };
        float MoveForceRight{ 0.0f };

        int ClonedFrom = -1;

        float CurrentEnergy = 0.0f;

        Random random;

        std::vector<LightSensor>& GetEye() { return Network->Eye; }

        bool IsPredator{ false };

        Cell(Random& r, int maxX, int maxY, bool isPredator = false)
            : random(r)
            , LocationX(static_cast<float>(r.NextDouble()*maxX))
            , LocationY(static_cast<float>(r.NextDouble()*maxY))
            , Rotation((float)(r.NextDouble() * 2.0 * M_PI))
            , Network(std::make_shared<NeuronNetwork>(AppProperties::NetworkSize, r))
            , IsPredator(isPredator)
        {
        }

        void PrepareIteration()
        {
            Network->PrepareIteration();
        }

        // set sensors 
        // call Iterate
        // digest Motor* params 
        void IterateNetwork(long step)
        {
            Network->IterateNetwork(random);

            MoveForceLeft =
                0.3f * Network->OutputVector[AppProperties::NetworkMoveForceGentleLeft] +
                1.0f * Network->OutputVector[AppProperties::NetworkMoveForceNormalLeft] +
                1.7f * Network->OutputVector[AppProperties::NetworkMoveForceStrongLeft];

			MoveForceRight =
                0.3f * Network->OutputVector[AppProperties::NetworkMoveForceGentleRight] +
                1.0f * Network->OutputVector[AppProperties::NetworkMoveForceNormalRight] +
                1.7f * Network->OutputVector[AppProperties::NetworkMoveForceStrongRight];

            Age++;
        }

        void CloneFrom(const Cell& other, Random& rnd, int maxX, int maxY, bool severeMutations, float severity)
        {
            //RandomizeLocation(rnd, maxX, maxY);

            //Alive = true;

            Network->CloneFrom(*other.Network, rnd, severeMutations, severity);

            Age = 0;
        }

        void RandomizeLocation(Random& rnd, int maxX, int maxY)
        {
            LocationX = static_cast<float>(rnd.NextDouble()*maxX);
            LocationY = static_cast<float>(rnd.NextDouble()*maxY);
            Rotation = (float)(rnd.NextDouble() * 2.0 * M_PI);
        }

        bool PredatoryEat(float& preyEnergy)
        {
            for (;;)
            {
                float valueCopy = InterlockedCompareExchange(&CurrentEnergy, 0.0f, 0.0f); // means just read
                float newValue = valueCopy + preyEnergy;

                if (InterlockedCompareExchange(&CurrentEnergy, newValue, valueCopy) == valueCopy)
                {
                    break;
                }
            }

            return true;
        }

		void SaveTo(std::ostream& stream)
		{
			stream.write(reinterpret_cast<const char*>(&LocationX), sizeof(LocationX));
			stream.write(reinterpret_cast<const char*>(&LocationY), sizeof(LocationY));
			stream.write(reinterpret_cast<const char*>(&Rotation), sizeof(Rotation));

			stream.write(reinterpret_cast<const char*>(&CurrentEnergy), sizeof(CurrentEnergy));
			stream.write(reinterpret_cast<const char*>(&IsPredator), sizeof(IsPredator));

			stream.write(reinterpret_cast<const char*>(&Age), sizeof(Age));
			stream.write(reinterpret_cast<const char*>(&ClonedFrom), sizeof(ClonedFrom));

			Network->SaveTo(stream);
		}

		void LoadFrom(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&LocationX), sizeof(LocationX));
			stream.read(reinterpret_cast<char*>(&LocationY), sizeof(LocationY));
			stream.read(reinterpret_cast<char*>(&Rotation), sizeof(Rotation));

			stream.read(reinterpret_cast<char*>(&CurrentEnergy), sizeof(CurrentEnergy));
			stream.read(reinterpret_cast<char*>(&IsPredator), sizeof(IsPredator));

			stream.read(reinterpret_cast<char*>(&Age), sizeof(Age));
			stream.read(reinterpret_cast<char*>(&ClonedFrom), sizeof(ClonedFrom));

			Network->LoadFrom(stream);
		}
    };

}
