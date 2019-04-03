#pragma once 

#define _USE_MATH_DEFINES

#include "AppProperties.h"
#include "../Random.h"
#include "NeuronNetwork.h"
#include <math.h>

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

        //[NonSerialized]
		float MoveForceLeft{ 0.0f };
        //[NonSerialized]
		float MoveForceRight{ 0.0f };

        //public bool Alive = true;

        //[NonSerialized]
        int ClonedFrom = -1;

        //[NonSerialized]
        float CurrentEnergy = 0.0f;

        //[NonSerialized]
        Random random;

		std::vector<LightSensor>& GetEye() { return Network->Eye; }

        Cell(Random& r, int maxX, int maxY)
			: random(r.Next())
			, LocationX(static_cast<float>(r.NextDouble()*maxX))
			, LocationY(static_cast<float>(r.NextDouble()*maxY))
			, Rotation((float)(r.NextDouble() * 2.0 * M_PI))
			, Network(std::make_shared<NeuronNetwork>(AppProperties::NetworkSize, r))
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

            MoveForceLeft = Network->OutputVector[AppProperties::NetworkMoveForceLeft];
            MoveForceRight = Network->OutputVector[AppProperties::NetworkMoveForceRight];

            Age++;
        }

        void CloneFrom(const Cell& other, Random& rnd, int maxX, int maxY, bool severeMutations, float severity)
        {
            RandomizeLocation(rnd, maxX, maxY);

            //Alive = true;

            Network->CloneFrom(*other.Network, rnd, severeMutations, severity);

            Age = 0;
        }

        void RandomizeLocation(Random& rnd, int maxX, int maxY)
        {
            LocationX = static_cast<float>(rnd.NextDouble()*maxX);
            LocationY = static_cast<float>(rnd.NextDouble()*maxY);
            Rotation = (float) (rnd.NextDouble() * 2.0 * M_PI);        
        }
	};

    //public sealed class CellUtils
    //{
    //    public static Cell ReadCell(string filename)
    //    {
    //        Cell ret = null;

    //        XmlSerializer serializer = new XmlSerializer(typeof(Cell));

    //        using (FileStream fs = new FileStream(filename, FileMode.Open))
    //        {
    //            ret = (Cell) serializer.Deserialize(fs);
    //        }

    //        return ret;
    //    }

    //    public static List<Cell> ReadCells(string filename)
    //    {
    //        List<Cell> ret = null;

    //        XmlSerializer serializer = new XmlSerializer(typeof(List<Cell>));

    //        using (FileStream fs = new FileStream(filename, FileMode.Open))
    //        {
    //            ret = (List<Cell>)serializer.Deserialize(fs);
    //        }

    //        return ret;
    //    }

    //    public static void SaveCell(string filename, Cell cell)
    //    {
    //        XmlSerializer serializer =new XmlSerializer(typeof(Cell));
    //        using (TextWriter writer = new StreamWriter(filename))
    //        {
    //            serializer.Serialize(writer, cell);
    //        } 
    //    }

    //    public static void SaveCells(string filename, List<Cell> cells)
    //    {
    //        XmlSerializer serializer = new XmlSerializer(typeof(List<Cell>));
    //        using (TextWriter writer = new StreamWriter(filename))
    //        {
    //            serializer.Serialize(writer, cells);
    //        }
    //    }
    //}
}
