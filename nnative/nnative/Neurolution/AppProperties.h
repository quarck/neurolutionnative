#pragma once

#define _USE_MATH_DEFINES

#include <string>
#include <math.h>

namespace Neurolution
{
    class AppProperties0
    {
    public:

		static constexpr float StepTimeDelta = 1.0f; // each step is 1.0 unit of time (whatever that means :)

		static constexpr float RealPhysics = true;

		static constexpr float AirDragFactorLinear = 0.03f;
		static constexpr float AirDragFactorQuadratic = 0.003f;
		static constexpr float AirDragFactorCube = 0.0003f;

		static constexpr bool ManualLoopUnroll = true;

        static constexpr int EyeSizeNumTripods = 24; // each covering ~15 deg
        static constexpr int EyeSize = EyeSizeNumTripods * 3; // total number of light-sensing cells 

		static constexpr float EyeCellWidth = 0.1f;
		static constexpr float EyeCellDirectionStep = (float)(M_PI / EyeSize);

		static constexpr int CurrentEnergyLevelSensor = EyeSize + 0;
		static constexpr int OrientationXSensor = EyeSize + 1;
		static constexpr int OrientationYSensor = EyeSize + 2;
		static constexpr int AbsoluteVelocitySensor = EyeSize + 3;

		static constexpr int SensorPackSize = AbsoluteVelocitySensor + 1;


		static constexpr int StepsPerGeneration = 1024;
        static constexpr int StepsPerBirthCheck = 1024;

        static constexpr int NetworkSize = 512;

        static constexpr int WorldSize = 128;
        static constexpr int FoodCountPerIteration = 48;
        static constexpr int PredatorCountPerIteration = 24;

        static constexpr int NetworkMoveForceGentleLeft = SensorPackSize + 0;
        static constexpr int NetworkMoveForceGentleRight = SensorPackSize + 1;
        static constexpr int NetworkMoveForceNormalLeft = SensorPackSize + 2;
        static constexpr int NetworkMoveForceNormalRight = SensorPackSize + 3;
        static constexpr int NetworkMoveForceStrongLeft = SensorPackSize + 4;
        static constexpr int NetworkMoveForceStrongRight = SensorPackSize + 5;


        static constexpr float NetworkMaxRegularMutation = 0.03f;
        static constexpr float NetworkSevereMutationAlpha = 0.4f;
        static constexpr float NetworkNoiseLevel = 0.00001f;
		static constexpr bool ApplyNetworkNoise = false;

		static constexpr float NetworkSpikeNoiseLevel = 0.01f;
		static constexpr bool ApplyNetworkSpikeNoise = true;

        static constexpr float FoodInitialValue = 14;

        static constexpr float PredatorInitialValue = 5.5f;

        static constexpr float CellTailLength = 4.0f;
        static constexpr float CellEyeBase = 3.0f;

        static constexpr int WorldWidth = 2048;
        static constexpr int WorldHeight = 1536;

		static constexpr int MaxDistanceSquareVisibility = (WorldWidth / 3) * (WorldWidth / 3) + (WorldHeight / 3) * (WorldHeight / 3);

        static constexpr float SevereMutationFactor = 0.15f;
        static constexpr float SevereMutationSlope = 0.33f;

        static constexpr float FoodMinDistanceToBorder = 5;

        static constexpr float MoveEnergyFactor = 0.0001f;
        static constexpr float InitialCellEnergy = 1.0f;

        static constexpr float MaxEnergyCapacity = 144.0f;
        static constexpr float SedatedAtEnergyLevel = 2.0f;

		static constexpr float PredatorsOvereatEnergy = 100.0f;
		static constexpr float PredatorsOveratSlowdownFactor = 0.33f;

        static constexpr float BirthEnergyConsumption = 2.0f;
        static constexpr float PredatorBirthEnergyConsumption = 8.0f;

        static constexpr float SporeEnergyLevel = 0.01f;

        static constexpr float NeuronChargeDecay = 0.30f;

        static constexpr float NeuronMinCharge = -2.0f;
        static constexpr float NeuronMaxCharge = 2.0f;

        static constexpr float NeuronChargeThreshold = 1.0f;

        static constexpr long OldSince = 4096;

		static constexpr float CellFoodCaptureDistance = 10.0f;
		static constexpr float PredatorCaptureDistance = 10.0f;

		static float FoodConsumptionProbability(float relativeVelocityX, float relativeVelocityY)
		{
			float sq = relativeVelocityX * relativeVelocityX + relativeVelocityY * relativeVelocityY;
			return 1.0f / (1.0f + sq * 0.4f);
		}

		static float PreyCatchingProbability(float relativeVelocityX, float relativeVelocityY)
		{
			float sq = relativeVelocityX * relativeVelocityX + relativeVelocityY * relativeVelocityY;
			return 1.0f / (1.0f + sq * 0.4f);
		}
    };
}
