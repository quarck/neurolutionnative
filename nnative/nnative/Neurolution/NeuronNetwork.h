#pragma once

#include <ostream>
#include <istream>
#include <vector>

#include <immintrin.h>

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


	template <typename WorldProp>
    struct Neuron
    {
        std::vector<float> Weights;

		float Charge;

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
                std::vector<float>(size).swap(Weights);
            }

            if (!severe)
            {
                float maxMutation = WorldProp::NetworkMaxRegularMutation;

                for (int i = 0; i < size; ++i)
                    Weights[i] = other.Weights[i] + (2.0f * rnd.NextFloat() - 1.0f) * maxMutation;
            }
            else
            {
                float alpha = WorldProp::NetworkSevereMutationAlpha;

                for (int i = 0; i < size; ++i)
                    Weights[i] = other.Weights[i] * alpha + (2.0f * rnd.NextFloat() - 1.0f) * (1.0f - alpha);
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

	template <typename WorldProp>
    struct NeuronNetwork
    {
		using TNeuron = Neuron<WorldProp>;
		using ThisType = NeuronNetwork<WorldProp>;

        std::vector<TNeuron> Neurons;

        std::vector<LightSensor> Eye;

        std::vector<float> InputVector;

        std::vector<float> OutputVector;

        size_t GetNetworkSize() const noexcept 
        {
            return Neurons.size();
        }

        size_t GetVectorSize() const noexcept 
        { 
            size_t ret = GetNetworkSize() + WorldProp::SensorPackSize; 
            if (ret % 16 != 0)
                throw std::exception("Internal error: vector size is not multiple of 16");
            return ret;
        }

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
                Neurons[i] = TNeuron(WorldProp::SensorPackSize + networkSize);
            }
        }

        void IterateNetwork(
			Random& rnd, 
			std::vector<float>& inputVector,
			std::vector<float>& outputVector) noexcept
        {
            for (unsigned int j = 0; j < Neurons.size(); ++j)
            {
                int neuronPositionInInputVector = j + WorldProp::SensorPackSize;

                // 1st step - calculate updated charge values
                auto& neuron = Neurons[j];

				float weightedInput = -neuron.Weights[neuronPositionInInputVector] * inputVector[neuronPositionInInputVector];

				if constexpr (WorldProp::ManualLoopUnroll)
				{
                    __m256 acc1 = _mm256_setzero_ps();
                    __m256 acc2 = _mm256_setzero_ps();

                    const float* nwptr = &neuron.Weights[0];
                    const float* iwptr = &inputVector[0];
                    unsigned int num_iters = neuron.Weights.size() / 16;

					for (unsigned int i = 0; i < num_iters; ++ i)
					{
                        acc1 = _mm256_fmadd_ps(
                            _mm256_load_ps(nwptr), 
                            _mm256_load_ps(iwptr), 
                            acc1
                        );
                        acc2 = _mm256_fmadd_ps(
                            _mm256_load_ps(nwptr + 8),
                            _mm256_load_ps(iwptr + 8),
                            acc2
                        );
                        nwptr += 16;
                        iwptr += 16;
                    }

                    /*
                    a = _mm256_hadd_ps(a, b)
                    a'0 := a1 + a0
                    a'1 := a3 + a2
                    a'2 := b1 + b0
                    a'3 := b2 + b3
                    a'4 := a5 + a4
                    a'5 := a7 + a6
                    a'6 := b5 + b4
                    a'7 := b7 + b6

                    2nd: 
                    a = _mm256_hadd_ps(a, 0)
                    a''0 := a3 + a2 + a1 + a0
                    a''1 := b2 + b3 + b1 + b0
                    a''2 := 0
                    a''3 := 0
                    a''4 := a7 + a6 + a5 + a4
                    a''5 := b7 + b6 + b5 + b4
                    a''6 := 0
                    a''7 := 0

                    3rd: 
                    a = _mm256_hadd_ps(a, 0)
                    a'''0 := b2 + b3 + b1 + b0 + a3 + a2 + a1 + a0
                    a'''1 := 0
                    a'''2 := 0
                    a'''3 := 0
                    a'''4 := b7 + b6 + b5 + b4 + a7 + a6 + a5 + a4
                    a'''5 := 0
                    a'''6 := 0
                    a'''7 := 0
                    */

                    acc1 = _mm256_hadd_ps(acc1, acc2);
                    acc1 = _mm256_hadd_ps(acc1, _mm256_setzero_ps());
                    acc1 = _mm256_hadd_ps(acc1, _mm256_setzero_ps());                    
                    weightedInput += acc1.m256_f32[0] + acc1.m256_f32[4];

     //               unsigned int offset = neuron.Weights.size() & (~15);
					//for (unsigned int i = 0; i < (neuron.Weights.size() & 15); ++i)
					//{
					//	weightedInput += neuron.Weights[i + offset] * inputVector[i + offset];
					//}
				}
				else
				{
					int sz = neuron.Weights.size();
					for (unsigned int i = 0; i < sz; ++i)
					{
                        _mm_prefetch((char*)&neuron.Weights[i + 1], 1);
                        _mm_prefetch((char*)&inputVector[i + 1], 1);

                        weightedInput += neuron.Weights[i] * inputVector[i];
					}
				}

				if constexpr (WorldProp::ApplyNetworkNoise)
				{
					// add some noise 
					weightedInput += rnd.Next<float>(-WorldProp::NetworkNoiseLevel, WorldProp::NetworkNoiseLevel);
				}


                // 2nd step - update neuron state according to current state + new input
                // and update state and current output

                switch (neuron.State)
                {
                case NeuronState::Idle:

                    neuron.Charge = ValueCap(
                        neuron.Charge * WorldProp::NeuronChargeDecay + weightedInput,
						WorldProp::NeuronMinCharge,
						WorldProp::NeuronMaxCharge
                    );

                    if (neuron.Charge > WorldProp::NeuronChargeThreshold)
                    {
                        neuron.State = NeuronState::Excited0;

						if constexpr (WorldProp::ApplyNetworkSpikeNoise)
						{
							outputVector[j] = 1.0f + rnd.Next<float>(-WorldProp::NetworkSpikeNoiseLevel, WorldProp::NetworkSpikeNoiseLevel);
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
						outputVector[j] = 1.0f + rnd.Next<float>(-WorldProp::NetworkSpikeNoiseLevel, WorldProp::NetworkSpikeNoiseLevel);
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
