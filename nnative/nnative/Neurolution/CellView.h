#pragma once 
//using System;
//using System.Collections.Generic;
//using System.Linq;
//using System.Text;
//using System.Threading.Tasks;
//using System.Windows;
//using System.Windows.Controls;
//using System.Windows.Media;
//using System.Windows.Shapes;

#include <memory>
#include "Cell.h"

#include <GL/gl.h>			/* OpenGL header file */
#include <GL/glu.h>			/* OpenGL utilities header file */


namespace Neurolution
{
    class CellView
    {
	public:
        std::shared_ptr<Cell> cell;

		float cellColorRed;
		float cellColorGreen;
		float cellColorBlue;

        CellView(std::shared_ptr<Cell>& c, Random& rnd)
			: cell(c)
        {
			cellColorRed = static_cast<float>(rnd.NextDouble() * 0.666 + 0.333);
			cellColorGreen = static_cast<float>(rnd.NextDouble() * 0.666 + 0.333);
			cellColorBlue = static_cast<float>(rnd.NextDouble() * 0.666 + 0.333);
        }

        void Draw()
        {
            //if (Cell.Alive)
            if (cell->CurrentEnergy >= 0.0001f && 
				cell->LocationX >= 0.0 && cell->LocationX < AppProperties::WorldWidth
                && cell->LocationY >= 0.0 && cell->LocationY < AppProperties::WorldHeight)
            {
				glPushMatrix();

				glTranslatef(cell->LocationX, cell->LocationY, 0.0);
				glRotatef(
					static_cast<float>(cell->Rotation / M_PI * 180.0 - 90.0),
					0.0f, 0.0f, 1.0f);
				
				glBegin(GL_TRIANGLES);

				float energy = cell->CurrentEnergy;
				float factor = (energy > 2.0) ? 1.0f : energy / 2.0f;

				glColor3f(1.0f - factor, factor, 0.0f);


				glIndexi(1); glVertex2f(0.0f, 10.0f);
				glIndexi(2); glVertex2f(2.5f, 0.0f);
				glIndexi(3); glVertex2f(-2.5f, 0.0f);

				glIndexi(4); glVertex2f(0.0f, 0.0f);
				glIndexi(5); glVertex2f(2.5f, 0.0f);
				glIndexi(6); glVertex2f(3.0f, -5.0f);

				glIndexi(7); glVertex2f(0.0f, 0.0f);
				glIndexi(8); glVertex2f(-2.5f, 0.0f);
				glIndexi(9); glVertex2f(-3.0f, -5.0f);

				glEnd();

				glPopMatrix();
            }
        }
	};
}
