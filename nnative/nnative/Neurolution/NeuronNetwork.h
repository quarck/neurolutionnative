#pragma once

#include <ostream>
#include <istream>
#include <vector>

#include "../Random.h"
#include "../Utils.h"

namespace Neurolution
{
    enum class LightSensorColor
    {
        Unset = 0,
        Red = 1,	// Plants 
        Green = 2,	// Plant eaters
        Blue = 3	// Meat eaters 
    };

    struct LightSensor
    {
        float Direction{ 0.0f }; // radians 
        float Width{ 0.0f };
        LightSensorColor Color{ LightSensorColor::Unset };

        LightSensor()
        {

        }

        LightSensor(float dir, float width, LightSensorColor& color)
            : Direction(dir), Width(width), Color(color)
        {

        }
	
		void SaveTo(std::ostream& stream)
		{
			stream.write(reinterpret_cast<const char*>(&Direction), sizeof(Direction));
			stream.write(reinterpret_cast<const char*>(&Width), sizeof(Width));
			stream.write(reinterpret_cast<const char*>(&Color), sizeof(Color));
		}

		void LoadFrom(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&Direction), sizeof(Direction));
			stream.read(reinterpret_cast<char*>(&Width), sizeof(Width));
			stream.read(reinterpret_cast<char*>(&Color), sizeof(Color));
		}
	};

    enum class NeuronState : int
    {
        Idle,
        Excited0,
        Excited1,
        Recovering0,
        Recovering1
    };


	template <typename WorldProp, typename TNumericType>
    struct Neuron
    {
        std::vector<TNumericType> Weights;

		TNumericType Charge;

        NeuronState State;

        Neuron()
			: Neuron(0)
        {
        }

        Neuron(int size)
            : Charge(0)
            , State(NeuronState::Idle)
            , Weights(size)
        {
            for (int i = 0; i < size; ++i)
                Weights[i] = 0;
        }

        Neuron(const Neuron& other, Random& rnd, bool severe)
        {
            CloneFrom(other, rnd, severe);
        }

        void CloneFrom(const Neuron& other, Random& rnd, bool severe) noexcept
        {
            Charge = 0;
            State = NeuronState::Idle;

            size_t size = other.Weights.size();

            if (Weights.size() != size)
            {
                std::vector<TNumericType>(size).swap(Weights);
            }

            if (!severe)
            {
                float maxMutation = WorldProp::NetworkMaxRegularMutation;

                for (int i = 0; i < size; ++i)
                    Weights[i] = TNumericType(static_cast<float>(other.Weights[i]) + (2.0 * rnd.NextDouble() - 1.0) * maxMutation);
            }
            else
            {
                float alpha = WorldProp::NetworkSevereMutationAlpha;

                for (int i = 0; i < size; ++i)
                    Weights[i] = TNumericType((float)(other.Weights[i]) * alpha + (2.0 * rnd.NextDouble() - 1.0) * (1.0 - alpha));
            }
        }

		void SaveTo(std::ostream& stream)
		{
			stream.write(reinterpret_cast<const char*>(&Charge), sizeof(Charge));
			stream.write(reinterpret_cast<const char*>(&State), sizeof(State));
			int size = static_cast<int>(Weights.size());
			stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
			stream.write(reinterpret_cast<const char*>(&Weights[0]), sizeof(Weights[0]) * size);
		}

		void LoadFrom(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&Charge), sizeof(Charge));
			stream.read(reinterpret_cast<char*>(&State), sizeof(State));
			int size = 0;
			stream.read(reinterpret_cast<char*>(&size), sizeof(size));
			Weights.resize(size);
			stream.read(reinterpret_cast<char*>(&Weights[0]), sizeof(Weights[0]) * size);
		}

    };

	template <typename WorldProp, typename TNumericType>
    struct NeuronNetwork
    {
		using TNeuron = Neuron<WorldProp, TNumericType>;
		using ThisType = NeuronNetwork<WorldProp, TNumericType>;

        std::vector<TNeuron> Neurons;

        std::vector<LightSensor> Eye;

        std::vector<TNumericType> InputVector;

        std::vector<TNumericType> OutputVector;

        size_t GetNetworkSize() const noexcept { return Neurons.size(); }

        size_t GetVectorSize() const noexcept { return GetNetworkSize() + WorldProp::EyeSize; }

		NeuronNetwork()
		{
		}

        NeuronNetwork(int networkSize)
            : Eye(WorldProp::EyeSize)
            , Neurons(networkSize)
            , InputVector(GetVectorSize())
            , OutputVector(GetVectorSize())
        {
            for (int i = 0; i < WorldProp::EyeSize; ++i)
            {
                int tripodIdx = i / 3;
                LightSensorColor color = (LightSensorColor)((i % 3) + 1);

                Eye[i] = LightSensor(
                    (float)(WorldProp::EyeCellDirectionStep * tripodIdx),
					WorldProp::EyeCellWidth,
                    color
                );
            }

            for (int i = 0; i < networkSize; ++i)
            {
                Neurons[i] = TNeuron(WorldProp::EyeSize + networkSize);
            }
        }

        void IterateNetwork(
			Random& rnd, 
			std::vector<TNumericType>& inputVector,
			std::vector<TNumericType>& outputVector) noexcept
        {
            for (unsigned int j = 0; j < Neurons.size(); ++j)
            {
                int neuronPositionInInputVector = j + WorldProp::EyeSize;

                // 1st step - calculate updated charge values
                auto& neuron = Neurons[j];

				TNumericType weightedInput = -neuron.Weights[neuronPositionInInputVector] * inputVector[neuronPositionInInputVector];

				if constexpr (WorldProp::ManualLoopUnroll)
				{
					for (unsigned int i = 0; i < neuron.Weights.size() / 8; i += 8)
					{
						weightedInput +=
							neuron.Weights[i + 0] * inputVector[i + 0] +
							neuron.Weights[i + 1] * inputVector[i + 1] +
							neuron.Weights[i + 2] * inputVector[i + 2] +
							neuron.Weights[i + 3] * inputVector[i + 3] +
							neuron.Weights[i + 4] * inputVector[i + 4] +
							neuron.Weights[i + 5] * inputVector[i + 5] +
							neuron.Weights[i + 6] * inputVector[i + 6] +
							neuron.Weights[i + 7] * inputVector[i + 7];
					}
					for (unsigned int i = 0; i < (neuron.Weights.size() & 7); ++i)
					{
						weightedInput +=
							neuron.Weights[i + 0] * inputVector[i + 0];
					}
				}
				else
				{
					int sz = neuron.Weights.size();
					for (unsigned int i = 0; i < sz; ++i)
					{
						weightedInput += neuron.Weights[i] * inputVector[i];
					}
				}

				if constexpr (WorldProp::ApplyNetworkNoise)
				{
					// add some noise 
					weightedInput += rnd.Next<TNumericType>(-WorldProp::NetworkNoiseLevel, WorldProp::NetworkNoiseLevel);
				}


                // 2nd step - update neuron state according to current state + new input
                // and update state and current output

                switch (neuron.State)
                {
                case NeuronState::Idle:

                    neuron.Charge = ValueCap(
                        neuron.Charge * TNumericType(WorldProp::NeuronChargeDecay) + weightedInput,
						TNumericType(WorldProp::NeuronMinCharge),
						TNumericType(WorldProp::NeuronMaxCharge)
                    );

                    if (neuron.Charge > TNumericType(WorldProp::NeuronChargeThreshold))
                    {
                        neuron.State = NeuronState::Excited0;

						if constexpr (WorldProp::ApplyNetworkSpikeNoise)
						{
							outputVector[j] = 1.0f + rnd.Next<TNumericType>(-WorldProp::NetworkSpikeNoiseLevel, WorldProp::NetworkSpikeNoiseLevel);
						}
						else
						{
							outputVector[j] = 1.0f;
						}
                    }
                    else
                    {
                        outputVector[j] = 0.0f;
                    }
                    break;

                case NeuronState::Excited0:
                    neuron.State = NeuronState::Excited1;

					if constexpr (WorldProp::ApplyNetworkSpikeNoise)
					{
						outputVector[j] = 1.0f + rnd.Next<TNumericType>(-WorldProp::NetworkSpikeNoiseLevel, WorldProp::NetworkSpikeNoiseLevel);
					}
					else
					{
						outputVector[j] = 1.0f;
					}
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

        void PrepareIteration() noexcept
        {
            InputVector.swap(OutputVector);
        }

        void IterateNetwork(Random& rnd) noexcept
        {
            IterateNetwork(rnd, InputVector, OutputVector);
        }

        void CleanOutputs() noexcept
        {
            std::fill(std::begin(InputVector), std::end(InputVector), 0.0f);
            std::fill(std::begin(OutputVector), std::end(OutputVector), 0.0f);
        }

        void CloneFrom(const ThisType& other, Random& rnd, bool severeMutations = false,
			float severity = 0.0f) noexcept
        {
            size_t newSize = other.Neurons.size();

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
                std::begin(InputVector));

            std::copy(
                std::cbegin(other.OutputVector),
                std::cend(other.OutputVector),
                std::begin(OutputVector));
        }

		void SaveTo(std::ostream& stream)
		{
			int numNeurons = static_cast<int>(Neurons.size());
			int numEyeCells = static_cast<int>(Eye.size());
			int vectorSize = static_cast<int>(InputVector.size());

			if (OutputVector.size() != vectorSize)
				throw std::runtime_error("Internal erorr: InputVector.size() must match OutputVector.size()");
			
			stream.write(reinterpret_cast<const char*>(&numNeurons), sizeof(numNeurons));
			stream.write(reinterpret_cast<const char*>(&numEyeCells), sizeof(numEyeCells));
			stream.write(reinterpret_cast<const char*>(&vectorSize), sizeof(vectorSize));

			for (int nidx = 0; nidx < Neurons.size(); ++nidx)
			{
				Neurons[nidx].SaveTo(stream);
			}

			for (int eidx = 0; eidx < Eye.size(); ++eidx)
			{
				Eye[eidx].SaveTo(stream);
			}

			stream.write(reinterpret_cast<const char*>(&InputVector[0]), sizeof(InputVector[0]) * vectorSize);
			stream.write(reinterpret_cast<const char*>(&OutputVector[0]), sizeof(OutputVector[0]) * vectorSize);
		}

		void LoadFrom(std::istream& stream)
		{
			int numNeurons;
			int numEyeCells;
			int vectorSize;

			stream.read(reinterpret_cast<char*>(&numNeurons), sizeof(numNeurons));
			stream.read(reinterpret_cast<char*>(&numEyeCells), sizeof(numEyeCells));
			stream.read(reinterpret_cast<char*>(&vectorSize), sizeof(vectorSize));

			if (OutputVector.size() != vectorSize)
				throw std::runtime_error("Internal erorr: InputVector.size() must match OutputVector.size()");

			Neurons.resize(numNeurons);

			for (int nidx = 0; nidx < Neurons.size(); ++nidx)
			{
				Neurons[nidx].LoadFrom(stream);
			}

			Eye.resize(numEyeCells);

			for (int eidx = 0; eidx < Eye.size(); ++eidx)
			{
				Eye[eidx].LoadFrom(stream);
			}

			InputVector.resize(vectorSize);
			OutputVector.resize(vectorSize);

			stream.read(reinterpret_cast<char*>(&InputVector[0]), sizeof(InputVector[0]) * vectorSize);
			stream.read(reinterpret_cast<char*>(&OutputVector[0]), sizeof(OutputVector[0]) * vectorSize);
		}
	};
}
