#pragma once
#include <vector>
#include "../Random.h"
#include "AppProperties.h"

namespace Neurolution
{

	template <typename T>
	const T& ValueCap(const T& val, const  T& min, const T& max)
	{
		if (val < min)
			return min;
		if (val > max)
			return max;
		return val;
	}


    // specialization types: 
    // Light
    // Current Energy 
    // Smell 

    struct LightSensor
    {
		float Direction{ 0.0f } ; // radians 
		float Width{ 0.0f };
		bool SensetiveToRed{ false };
	};

	enum class NeuronState : int
	{
		Idle,
		Excited0,
		Excited1,
		Recovering0,
		Recovering1
	};

	struct Neuron
    {
		std::vector<float> Weights;
        
        float Charge;

        NeuronState State;

        Neuron(int size, Random& rnd)
			: Charge(0.0f)
			, State(NeuronState::Idle)
			, Weights(size)
        {
            for (int i = 0; i < size; ++i)
                Weights[i] = 0.0f;
        }

		Neuron(const Neuron& other, Random& rnd, bool severe)
		{
			CloneFrom(other, rnd, severe);
		}

        void CloneFrom(const Neuron& other, Random& rnd, bool severe)
        {
			Charge = 0.0f;
			State = NeuronState::Idle;
			
			int size = other.Weights.size();

			if (Weights.size() != size)
			{
				std::vector<float>(size).swap(Weights);
			}

			if (!severe)
			{
				float maxMutation = AppProperties::NetworkMaxRegularMutation;

				for (int i = 0; i < size; ++i)
					Weights[i] = (float)(other.Weights[i] + (2.0 * rnd.NextDouble() - 1.0) * maxMutation);
			}
			else
			{
				float alpha = AppProperties::NetworkSevereMutationAlpha;

				for (int i = 0; i < size; ++i)
					Weights[i] = (float)(other.Weights[i] * alpha + (2.0 * rnd.NextDouble() - 1.0) * (1.0 - alpha));
			}
        }
	};

    struct NeuronNetwork
    {
        std::vector<Neuron> Neurons;

        std::vector<LightSensor> Eye; 

        std::vector<float> InputVector;

        std::vector<float> OutputVector;

		int GetNetworkSize() const { return Neurons.size(); }

		int GetVectorSize() const { return GetNetworkSize() + AppProperties::EyeSize; }

        NeuronNetwork(int networkSize, Random& rnd)
			: Eye(AppProperties::EyeSize)
			, Neurons(networkSize)
			, InputVector(GetVectorSize())
			, OutputVector(GetVectorSize())
        {
			auto eyeIter = std::cbegin(Eye);
            for (int i = 0; i < AppProperties::EyeSize; ++i)
            {
				double iPrime = ((i >> 1) - AppProperties::EyeSize / 2) + 0.5;

				Eye.emplace(eyeIter,
					(float)(AppProperties::EyeCellDirectionStep * iPrime), 
					AppProperties::EyeCellWidth,
					(i & 1) == 0
					);

				++eyeIter;
            }

			auto neuroIter = std::cbegin(Neurons);
            for (int i = 0; i < networkSize; ++i)
            {
				Neurons.emplace(neuroIter, AppProperties::EyeSize + networkSize, rnd);
				++neuroIter;
            }
        }

        void IterateNetwork(Random& rnd, std::vector<float>& inputVector, std::vector<float>& outputVector)
        {
            for (int j = 0; j < Neurons.size(); ++j)
            {
                int neuronPositionInInputVector = j + AppProperties::EyeSize;

                // 1st step - calculate updated charge values
                auto& neuron = Neurons[j];

                float weightedInput = -neuron.Weights[neuronPositionInInputVector] * inputVector[neuronPositionInInputVector];

                for (int i = 0; i < neuron.Weights.size() / 8; i += 8)
                {
                    weightedInput += 
                        neuron.Weights[i+0] * inputVector[i+0] +
                        neuron.Weights[i+1] * inputVector[i+1] +
                        neuron.Weights[i+2] * inputVector[i+2] +
                        neuron.Weights[i+3] * inputVector[i+3] +
                        neuron.Weights[i+4] * inputVector[i+4] +
                        neuron.Weights[i+5] * inputVector[i+5] +
                        neuron.Weights[i+6] * inputVector[i+6] +
                        neuron.Weights[i+7] * inputVector[i+7] ;
                }
                for (int i = 0; i < (neuron.Weights.size() & 7); ++i)
                {
                    weightedInput += 
                        neuron.Weights[i+0] * inputVector[i+0];
                }

                // add some noise 
                weightedInput += (float)((2.0 * rnd.NextDouble() - 1.0) * AppProperties::NetworkNoiseLevel);
                //weightedInput += (float)(0.34* AppProperties.NetworkNoiseLevel);


                // 2nd step - update neuron state according to current state + new input
                // and update state and current output

                switch (neuron.State)
                {
				case NeuronState::Idle:

                        neuron.Charge = ValueCap(
                            neuron.Charge * AppProperties::NeuronChargeDecay + weightedInput,
                            AppProperties::NeuronMinCharge,
                            AppProperties::NeuronMaxCharge
						);

                        if (neuron.Charge > AppProperties::NeuronChargeThreshold)
                        {
                            neuron.State = NeuronState::Excited0;
                            outputVector[j] = 1.0f;
                        }
                        else
                        {
                            outputVector[j] = 0.0f;
                        }
                        break;

				case NeuronState::Excited0:
                        neuron.State = NeuronState::Excited1;
                        outputVector[j] = 1.0f;
                        break;

				case NeuronState::Excited1:
                        neuron.State = NeuronState::Recovering0;
                        outputVector[j] = 0.0f;
                        break;

				case NeuronState::Recovering0:
                        neuron.State = NeuronState::Recovering1;
                        outputVector[j] = 0.0f;
                        break;

				case NeuronState::Recovering1:
                        neuron.State = NeuronState::Idle;
                        neuron.Charge = 0.0f;
                        outputVector[j] = 0.0f;
                        break;
                }
            }
        }


        void PrepareIteration()
        {
			InputVector.swap(OutputVector);
        }

        void IterateNetwork(Random& rnd)
        {
            IterateNetwork(rnd, InputVector, OutputVector);
        }

        void CleanOutputs()
        {
			std::fill(std::cbegin(InputVector), std::cend(InputVector), 0.0f);
			std::fill(std::cbegin(OutputVector), std::cend(OutputVector), 0.0f);
        }

        void CloneFrom(const NeuronNetwork& other, Random& rnd, bool severeMutations = false, float severity = 0.0f)
        {
			int newSize = other.Neurons.size();

			if (Neurons.size() != newSize)
			{
				Neurons.resize(newSize);
			}

			for (int i = 0; i < newSize; ++i)
			{
				bool severe = severeMutations && (rnd.NextDouble() < severity);
				Neurons[i].CloneFrom(other.Neurons[i], rnd, severe);
			}

			std::copy(
				std::cbegin(other.InputVector),
				std::cend(other.InputVector),
				std::cbegin(InputVector));

			std::copy(
				std::cbegin(other.OutputVector),
				std::cend(other.OutputVector),
				std::cbegin(OutputVector));
		}
	};
}
