/*
*/
#pragma once 

#define _USE_MATH_DEFINES

#include <memory>
#include <math.h>

#include "../Random.h"
#include "../Utils.h"

#include "NeuronNetwork.h"

#include "WorldUtils.h"

namespace Neurolution
{
	template <typename WorldProp>
    struct Cell : public MaterialPointWithEnergyValue
    {
		using TNetwork = NeuronNetwork<WorldProp>;
        std::shared_ptr<TNetwork> Network;

        static constexpr float TailLength = WorldProp::CellTailLength;
        static constexpr float EyeBase = WorldProp::CellEyeBase;

        long Age{ 0 };

        float MoveForceLeft{ 0.0f };
        float MoveForceRight{ 0.0f };

		// a big of an ugly hack - used by the view to display the total movement since the last scene 
		// update
		float TotalMoveForceLeft{ 0.0f };
		float TotalMoveForceRight{ 0.0f };

		int ClonedFrom = -1;

        //float CurrentEnergy = 0.0f;

        Random random;

        std::vector<LightSensor>& GetEye() noexcept { return Network->Eye; }

        bool IsPredator{ false };

        Cell(Random& r, int maxX, int maxY, bool isPredator = false)
            : random(r)
            , Network(std::make_shared<TNetwork>(WorldProp::NetworkSize))
            , IsPredator(isPredator)
        {
			LocationX = static_cast<float>(r.NextDouble() * maxX);
			LocationY = static_cast<float>(r.NextDouble() * maxY);
			Rotation = static_cast<float>(r.NextDouble() * 2.0 * M_PI);
		}

        void PrepareIteration() noexcept
        {
            Network->PrepareIteration();
        }

        // set sensors 
        // call Iterate
        // digest Motor* params 
        void IterateNetwork(long step) noexcept
        {
            MoveForceLeft = 0;
            MoveForceRight = 0;

            for (int i = 0; i < WorldProp::NetworkStepsPerIteration; ++i)
            {
                Network->IterateNetwork(random);

                MoveForceLeft +=
                    0.3f * Network->OutputVector[WorldProp::NetworkMoveForceGentleLeft] +
                    1.0f * Network->OutputVector[WorldProp::NetworkMoveForceNormalLeft] +
                    1.7f * Network->OutputVector[WorldProp::NetworkMoveForceStrongLeft];

                MoveForceRight +=
                    0.3f * Network->OutputVector[WorldProp::NetworkMoveForceGentleRight] +
                    1.0f * Network->OutputVector[WorldProp::NetworkMoveForceNormalRight] +
                    1.7f * Network->OutputVector[WorldProp::NetworkMoveForceStrongRight];
            }

            MoveForceLeft /= WorldProp::NetworkStepsPerIteration;
            MoveForceRight /= WorldProp::NetworkStepsPerIteration;

			TotalMoveForceLeft += MoveForceLeft;
			TotalMoveForceRight += MoveForceRight;

            Age++;
        }

        void CloneFrom(const Cell& other, Random& rnd, int maxX, int maxY, bool severeMutations, float severity) noexcept
        {
            //RandomizeLocation(rnd, maxX, maxY);

            //Alive = true;

            Network->CloneFrom(*other.Network, rnd, severeMutations, severity);

            Age = 0;
        }

        void RandomizeLocation(Random& rnd, int parentX, int parentY, int maxX, int maxY) noexcept
        {
            LocationX = parentX + static_cast<float>(rnd.NextDouble()*maxX/10 - maxX/20);
            LocationY = parentY + static_cast<float>(rnd.NextDouble()*maxY/10 - maxY/20);
            if (LocationX < 0)
                LocationX = 0;
            else if (LocationX > maxX)
                LocationX = maxX;
            if (LocationY < 0)
                LocationY = 0;
            else if (LocationY > maxY)
                LocationY = maxY;
            Rotation = static_cast<float>(rnd.NextDouble() * 2.0 * M_PI);
        }

        bool PredatoryEat(float& preyEnergy) noexcept
        {
            for (;;)
            {
                float valueCopy = InterlockedCompareExchange(&EnergyValue, 0.0f, 0.0f); // means just read
                float newValue = valueCopy + preyEnergy;

                if (InterlockedCompareExchange(&EnergyValue, newValue, valueCopy) == valueCopy)
                {
                    break;
                }
            }

            return true;
        }

		void SaveTo(std::ostream& stream)
		{
			this->MaterialPointWithEnergyValue::SaveTo(stream);
			
			//stream.write(reinterpret_cast<const char*>(&LocationX), sizeof(LocationX));
			//stream.write(reinterpret_cast<const char*>(&LocationY), sizeof(LocationY));
			//stream.write(reinterpret_cast<const char*>(&Rotation), sizeof(Rotation));

			//stream.write(reinterpret_cast<const char*>(&CurrentEnergy), sizeof(CurrentEnergy));
			stream.write(reinterpret_cast<const char*>(&IsPredator), sizeof(IsPredator));

			stream.write(reinterpret_cast<const char*>(&Age), sizeof(Age));
			stream.write(reinterpret_cast<const char*>(&ClonedFrom), sizeof(ClonedFrom));

			Network->SaveTo(stream);
		}

		void LoadFrom(std::istream& stream)
		{
			this->MaterialPointWithEnergyValue::LoadFrom(stream);

			//stream.read(reinterpret_cast<char*>(&LocationX), sizeof(LocationX));
			//stream.read(reinterpret_cast<char*>(&LocationY), sizeof(LocationY));
			//stream.read(reinterpret_cast<char*>(&Rotation), sizeof(Rotation));

			//stream.read(reinterpret_cast<char*>(&CurrentEnergy), sizeof(CurrentEnergy));
			stream.read(reinterpret_cast<char*>(&IsPredator), sizeof(IsPredator));

			stream.read(reinterpret_cast<char*>(&Age), sizeof(Age));
			stream.read(reinterpret_cast<char*>(&ClonedFrom), sizeof(ClonedFrom));

			Network->LoadFrom(stream);
		}
    };

}
