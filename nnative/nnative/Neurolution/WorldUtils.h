#pragma once

#include <algorithm>
#include <functional>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <sstream>

namespace Neurolution
{
	struct Location
	{
		float LocationX;
		float LocationY;

		void SaveTo(std::ostream& stream)
		{
			stream.write(reinterpret_cast<const char*>(&LocationX), sizeof(LocationX));
			stream.write(reinterpret_cast<const char*>(&LocationY), sizeof(LocationY));
		}

		void LoadFrom(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&LocationX), sizeof(LocationX));
			stream.read(reinterpret_cast<char*>(&LocationY), sizeof(LocationY));
		}
	};

	struct Orientation
	{
		//float OrientationX;
		//float OrientationY;
		float Rotation;

		void SaveTo(std::ostream& stream)
		{
			//stream.write(reinterpret_cast<const char*>(&OrientationX), sizeof(OrientationX));
			//stream.write(reinterpret_cast<const char*>(&OrientationY), sizeof(OrientationY));
			stream.write(reinterpret_cast<const char*>(&Rotation), sizeof(Rotation));
		}

		void LoadFrom(std::istream& stream)
		{
			//stream.read(reinterpret_cast<char*>(&OrientationX), sizeof(OrientationX));
			//stream.read(reinterpret_cast<char*>(&OrientationY), sizeof(OrientationY));
			stream.read(reinterpret_cast<char*>(&Rotation), sizeof(Rotation));
		}

		float ComputeOrientationX() const
		{
			return static_cast<float>(std::cos(Rotation));
		}

		float ComputeOrientationY() const
		{
			return static_cast<float>(std::sin(Rotation));
		}
	};

	struct Velocity
	{
		float VelocityX;
		float VelocityY;

		void SaveTo(std::ostream& stream)
		{
			stream.write(reinterpret_cast<const char*>(&VelocityX), sizeof(VelocityX));
			stream.write(reinterpret_cast<const char*>(&VelocityY), sizeof(VelocityY));
		}

		void LoadFrom(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&VelocityX), sizeof(VelocityX));
			stream.read(reinterpret_cast<char*>(&VelocityY), sizeof(VelocityY));
		}
	};

	struct MaterialPoint : public Orientation, public Location, public Velocity
	{
		float Mass{ 1.0f };

		void Step(int maxX, int maxY, float timeDelta)  noexcept
		{
			LocationX = LoopValue(LocationX + VelocityX * timeDelta, 0.0f, static_cast<float>(maxX));
			LocationY = LoopValue(LocationY + VelocityY * timeDelta, 0.0f, static_cast<float>(maxY));
		}

		void SaveTo(std::ostream & stream)
		{
			this->Orientation::SaveTo(stream);
			this->Location::SaveTo(stream);
			this->Velocity::SaveTo(stream);
			stream.write(reinterpret_cast<const char*>(&Mass), sizeof(Mass));
		}

		void LoadFrom(std::istream & stream)
		{
			this->Orientation::LoadFrom(stream);
			this->Location::LoadFrom(stream);
			this->Velocity::LoadFrom(stream);
			stream.read(reinterpret_cast<char*>(&Mass), sizeof(Mass));
		}
	};

	struct MaterialPointWithEnergyValue : public MaterialPoint
	{
		float EnergyValue{ 0.0f };

		void SaveTo(std::ostream & stream)
		{
			this->MaterialPoint::SaveTo(stream);
			stream.write(reinterpret_cast<const char*>(&EnergyValue), sizeof(EnergyValue));
		}

		void LoadFrom(std::istream & stream)
		{
			this->MaterialPoint::LoadFrom(stream);
			stream.read(reinterpret_cast<char*>(&EnergyValue), sizeof(EnergyValue));
		}
	};



}