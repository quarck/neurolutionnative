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
			cellColorRed = static_cast<float>(rnd.NextDouble() / 2.0 + 0.5);
			cellColorGreen = static_cast<float>(rnd.NextDouble() / 2.0 + 0.5);
			cellColorBlue = static_cast<float>(rnd.NextDouble() / 2.0 + 0.5);
        }

        void Draw()
        {
            //if (Cell.Alive)
            if (cell->LocationX >= 0.0 && cell->LocationX < AppProperties::WorldWidth
                && cell->LocationY >= 0.0 && cell->LocationY < AppProperties::WorldHeight)
            {
                //double adjRotation = cell->Rotation - M_PI/2.0;

                //double dxBody = cell->EyeBase * std::cos(adjRotation) / 2.0;
                //double dyBody = cell->EyeBase * std::sin(adjRotation) / 2.0;

                //double dxTail = cell->TailLength * std::cos(cell->Rotation);
                //double dyTail = cell->TailLength * std::sin(cell->Rotation);

				glPushMatrix();

				glScalef(
					static_cast<GLfloat>(1.0 / AppProperties::WorldWidth), 
					static_cast<GLfloat>(1.0 / AppProperties::WorldHeight),
					1.0f);

				glTranslatef(cell->LocationX, cell->LocationY, 0.0);
				glRotatef(cell->Rotation, 0.0f, 0.0f, 1.0f);
				
				glBegin(GL_TRIANGLES);

				glColor3f(cellColorRed, cellColorGreen, cellColorBlue);
				glIndexi(1); glVertex3f(0.0f, 15.0f, 0.0f);
				glIndexi(2); glVertex3f(-5.0f, -15.0f, 0.0f);
				glIndexi(3); glVertex3f(5.0f, -15.0f, 0.0f);
				glEnd();
				glPopMatrix();
            }
        }
	};
}
