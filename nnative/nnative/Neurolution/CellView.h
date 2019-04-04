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
            if (cell->LocationX >= 0.0 && cell->LocationX < AppProperties::WorldWidth
                && cell->LocationY >= 0.0 && cell->LocationY < AppProperties::WorldHeight)
            {
				glPushMatrix();

				glTranslatef(cell->LocationX, cell->LocationY, 0.0);
				glRotatef(
					static_cast<float>(cell->Rotation / M_PI * 180.0 - 90.0),
					0.0f, 0.0f, 1.0f);
				
				glBegin(GL_TRIANGLES);

				glColor3f(cellColorRed, cellColorGreen, cellColorBlue);

				//glIndexi(1); glVertex3f(0.0f, 7.0f, 0.0f);
				//glIndexi(2); glVertex3f(-3.0f, -7.0f, 0.0f);
				//glIndexi(3); glVertex3f(3.0f, -7.0f, 0.0f);

				int idx = 0;

				glIndexi(++idx); glVertex2f(0.0f, 0.0f);
				glIndexi(++idx); glVertex2f(0.0f, 10.0f);
				glIndexi(++idx); glVertex2f(1.0f, 4.0f);

				glIndexi(++idx); glVertex2f(0.0f, 0.0f);
				glIndexi(++idx); glVertex2f(1.0f, 4.0f);
				glIndexi(++idx); glVertex2f(3.0f, 2.0f);

				glIndexi(++idx); glVertex2f(0.0f, 0.0f);
				glIndexi(++idx); glVertex2f(3.0f, 2.0f);
				glIndexi(++idx); glVertex2f(2.0f, 0.0f);

				glIndexi(++idx); glVertex2f(3.0f, 2.0f);
				glIndexi(++idx); glVertex2f(4.0f, -4.0f);
				glIndexi(++idx); glVertex2f(2.0f, 0.0f);

				glIndexi(++idx); glVertex2f(0.0f, 0.0f);
				glIndexi(++idx); glVertex2f(1.0f, 0.0f);
				glIndexi(++idx); glVertex2f(0.0f, -2.0f);

				glIndexi(++idx); glVertex2f(1.0f, 0.0f);
				glIndexi(++idx); glVertex2f(1.0f, -6.0f);
				glIndexi(++idx); glVertex2f(0.0f, -2.0f);

				glIndexi(++idx); glVertex2f(1.0f, 0.0f);
				glIndexi(++idx); glVertex2f(2.0f, -4.0f);
				glIndexi(++idx); glVertex2f(1.0f, -6.0f);



				glIndexi(++idx); glVertex2f(-0.0f, 0.0f);
				glIndexi(++idx); glVertex2f(-0.0f, 10.0f);
				glIndexi(++idx); glVertex2f(-1.0f, 4.0f);
											
				glIndexi(++idx); glVertex2f(-0.0f, 0.0f);
				glIndexi(++idx); glVertex2f(-1.0f, 4.0f);
				glIndexi(++idx); glVertex2f(-3.0f, 2.0f);
											
				glIndexi(++idx); glVertex2f(-0.0f, 0.0f);
				glIndexi(++idx); glVertex2f(-3.0f, 2.0f);
				glIndexi(++idx); glVertex2f(-2.0f, 0.0f);
											
				glIndexi(++idx); glVertex2f(-3.0f, 2.0f);
				glIndexi(++idx); glVertex2f(-4.0f, -4.0f);
				glIndexi(++idx); glVertex2f(-2.0f, 0.0f);
											
				glIndexi(++idx); glVertex2f(-0.0f, 0.0f);
				glIndexi(++idx); glVertex2f(-1.0f, 0.0f);
				glIndexi(++idx); glVertex2f(-0.0f, -2.0f);
											
				glIndexi(++idx); glVertex2f(-1.0f, 0.0f);
				glIndexi(++idx); glVertex2f(-1.0f, -6.0f);
				glIndexi(++idx); glVertex2f(-0.0f, -2.0f);
											
				glIndexi(++idx); glVertex2f(-1.0f, 0.0f);
				glIndexi(++idx); glVertex2f(-2.0f, -4.0f);
				glIndexi(++idx); glVertex2f(-1.0f, -6.0f);





				glEnd();

				glPopMatrix();
            }
        }
	};
}
