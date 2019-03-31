#pragma once 

#include "AppProperties.h"


namespace Neurolution
{
    struct Cell
    {
	public:

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
        //public Random Random;

        [XmlIgnore]
        public LightSensor[] Eye => Network.Eye;

        public Cell()
        {
            Random = new Random();
        }

        public Cell(Random r, int maxX, int maxY)
        {
            LocationX = r.Next(maxX);
            LocationY = r.Next(maxY);
            Rotation = (float) (r.NextDouble() * 2.0 * Math.PI);
            Network = new NeuronNetwork(AppProperties.NetworkSize, r);

            Random = new Random(r.Next());
        }

        public void PrepareIteration()
        {
            Network.PrepareIteration();
        }

        // set sensors 
        // call Iterate
        // digest Motor* params 
        public void IterateNetwork(long step)
        {
            Network.IterateNetwork(Random);

            MoveForceLeft = Network.OutputVector[AppProperties.NetworkMoveForceLeft];
            MoveForceRight = Network.OutputVector[AppProperties.NetworkMoveForceRight];

            Age++;
        }

        public void CloneFrom(Cell other, Random rnd, int maxX, int maxY, bool severeMutations, float severity)
        {
            RandomizeLocation(rnd, maxX, maxY);

            //Alive = true;

            Network.CloneFrom(other.Network, rnd, severeMutations, severity);

            Age = 0;
        }

        public void RandomizeLocation(Random rnd, int maxX, int maxY)
        {
            LocationX = rnd.Next(maxX);
            LocationY = rnd.Next(maxY);
            Rotation = (float) (rnd.NextDouble() * 2.0 * Math.PI);        
        }
    }

    public sealed class CellUtils
    {
        public static Cell ReadCell(string filename)
        {
            Cell ret = null;

            XmlSerializer serializer = new XmlSerializer(typeof(Cell));

            using (FileStream fs = new FileStream(filename, FileMode.Open))
            {
                ret = (Cell) serializer.Deserialize(fs);
            }

            return ret;
        }

        public static List<Cell> ReadCells(string filename)
        {
            List<Cell> ret = null;

            XmlSerializer serializer = new XmlSerializer(typeof(List<Cell>));

            using (FileStream fs = new FileStream(filename, FileMode.Open))
            {
                ret = (List<Cell>)serializer.Deserialize(fs);
            }

            return ret;
        }

        public static void SaveCell(string filename, Cell cell)
        {
            XmlSerializer serializer =new XmlSerializer(typeof(Cell));
            using (TextWriter writer = new StreamWriter(filename))
            {
                serializer.Serialize(writer, cell);
            } 
        }

        public static void SaveCells(string filename, List<Cell> cells)
        {
            XmlSerializer serializer = new XmlSerializer(typeof(List<Cell>));
            using (TextWriter writer = new StreamWriter(filename))
            {
                serializer.Serialize(writer, cells);
            }
        }
    }
}
