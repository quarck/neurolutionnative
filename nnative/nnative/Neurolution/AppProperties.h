#pragma once

#define _USE_MATH_DEFINES

#include <string>
#include <math.h>

namespace Neurolution
{
    class AppProperties0
    {
    public:
        static constexpr int EyeSizeNumTripods = 24; // each covering ~15 deg
        static constexpr int EyeSize = EyeSizeNumTripods * 3; // total number of light-sensing cells 

        static constexpr float EyeCellWidth = 0.1f;
        static constexpr float EyeCellDirectionStep = (float)(M_PI / EyeSize);

        static constexpr int StepsPerGeneration = 1024;
        static constexpr int StepsPerBirthCheck = 1024;

        static constexpr int NetworkSize = 512;

        static constexpr int WorldSize = 128;
        static constexpr int FoodCountPerIteration = 24;
        static constexpr int PredatorCountPerIteration = 16;

        static constexpr int NetworkMoveForceGentleLeft = EyeSize + 0;
        static constexpr int NetworkMoveForceGentleRight = EyeSize + 1;
        static constexpr int NetworkMoveForceNormalLeft = EyeSize + 2;
        static constexpr int NetworkMoveForceNormalRight = EyeSize + 3;
        static constexpr int NetworkMoveForceStrongLeft = EyeSize + 4;
        static constexpr int NetworkMoveForceStrongRight = EyeSize + 5;


        static constexpr float NetworkMaxRegularMutation = 0.03f;
        static constexpr float NetworkSevereMutationAlpha = 0.4f;
        static constexpr float NetworkNoiseLevel = 0.00001f;

        static constexpr float FoodInitialValue = 14;

        static constexpr float PredatorInitialValue = 5.5f;

        static constexpr float CellTailLength = 4.0f;
        static constexpr float CellEyeBase = 3.0f;

        static constexpr int WorldWidth = 1500;
        static constexpr int WorldHeight = 1500;

        static constexpr float SevereMutationFactor = 0.15f;
        static constexpr float SevereMutationSlope = 0.33f;

        static constexpr float FoodMinDistanceToBorder = 5;

        static constexpr float MoveEnergyFactor = 0.0001f;
        static constexpr float InitialCellEnergy = 1.0f;

        static constexpr float MaxEnergyCapacity = 144.0f;
        static constexpr float SedatedAtEnergyLevel = 2.0f;

        static constexpr float BirthEnergyConsumption = 2.0f;
        static constexpr float PredatorBirthEnergyConsumption = 8.0f;

        static constexpr float SporeEnergyLevel = 0.01f;

        static constexpr float NeuronChargeDecay = 0.30f;

        static constexpr float NeuronMinCharge = -2.0f;
        static constexpr float NeuronMaxCharge = 2.0f;

        static constexpr float NeuronChargeThreshold = 1.0f;

        static constexpr long OldSince = 4096;
    };
}
