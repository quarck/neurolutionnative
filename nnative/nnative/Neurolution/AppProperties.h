#pragma once

#include <string>
#include <math.h>

namespace Neurolution
{
    class AppProperties
    {
	public: 
		//static const std::string SerializeTo { "c:\\users\\spars\\Desktop\\cell.xml" };

		static constexpr double MATH_PI = 3.1415926;

        static constexpr int RedEyeSize = 24;
        static constexpr int BlueEyeSize = 24;

        static constexpr int EyeSize = RedEyeSize + BlueEyeSize;

        static constexpr float EyeCellWidth = 0.1f;
        static constexpr float EyeCellDirectionStep = (float) (MATH_PI / 48.0);

        static constexpr int StepsPerGeneration = 512;
        static constexpr int StepsPerBirthCheck = 512;

        static constexpr int SerializeTopEveryNStep = 8192 * 8;
        static constexpr int SerializeWorldEveryNStep = 8192 * 64;

        static constexpr int NetworkSize = 256;

		static constexpr int WorldSize = 128;
        static constexpr int FoodCountPerIteration = 32;
        static constexpr int PredatorCountPerIteration = 16;

        static constexpr int NetworkMoveForceGentleLeft = RedEyeSize + BlueEyeSize + 0;
        static constexpr int NetworkMoveForceGentleRight = RedEyeSize + BlueEyeSize + 1;
		static constexpr int NetworkMoveForceNormalLeft = RedEyeSize + BlueEyeSize + 2;
		static constexpr int NetworkMoveForceNormalRight = RedEyeSize + BlueEyeSize + 3;
		static constexpr int NetworkMoveForceStrongLeft = RedEyeSize + BlueEyeSize + 4;
		static constexpr int NetworkMoveForceStrongRight = RedEyeSize + BlueEyeSize + 5;


        static constexpr float NetworkMaxRegularMutation = 0.03f;
        static constexpr float NetworkSevereMutationAlpha = 0.4f;
        static constexpr float NetworkNoiseLevel = 0.00001f;

        static constexpr float FoodInitialValue = 7;

        static constexpr float PredatorInitialValue = 7;

        static constexpr float CellTailLength = 4.0f;
        static constexpr float CellEyeBase = 3.0f;

        static constexpr int WorldWidth = 800;
        static constexpr int WorldHeight = 600;

        static constexpr float SevereMutationFactor = 0.15f;
        static constexpr float SevereMutationSlope = 0.33f;

        static constexpr float FoodMinDistanceToBorder = 5;

        static constexpr float MoveEnergyFactor = 0.001f;
        static constexpr float InitialCellEnergy = 1.0f;

        static constexpr float MaxEnergyCapacity = 14.0f;

        static constexpr float BirthEnergyConsumption = 2.0f;

        static constexpr float SporeEnergyLevel = 0.01f;

        static constexpr float NeuronChargeDecay = 0.30f;

        static constexpr float NeuronMinCharge = -2.0f;
        static constexpr float NeuronMaxCharge = 2.0f;

        static constexpr float NeuronChargeThreshold = 1.0f;

        static constexpr long OldSince = 1024;
	};
}
