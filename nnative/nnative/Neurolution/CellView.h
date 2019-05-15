#pragma once 

#include <memory>
#include "Cell.h"

#include <GL/gl.h>			/* OpenGL header file */
#include <GL/glu.h>			/* OpenGL utilities header file */


namespace Neurolution
{
    class CellView
    {
        std::shared_ptr<Cell> _cell;
	public:

        CellView(std::shared_ptr<Cell>& c, Random& rnd)
            : _cell(c)
        {
        }

        void Draw()
        {
            if (_cell->CurrentEnergy >= 0.0001f &&
                _cell->LocationX >= 0.0 && _cell->LocationX < AppProperties::WorldWidth
                && _cell->LocationY >= 0.0 && _cell->LocationY < AppProperties::WorldHeight)
            {
                glPushMatrix();

                glTranslatef(_cell->LocationX, _cell->LocationY, 0.0);
                glRotatef(
                    static_cast<float>(_cell->Rotation / M_PI * 180.0 - 90.0),
                    0.0f, 0.0f, 1.0f);

                glBegin(GL_TRIANGLES);

                float energy = _cell->CurrentEnergy;
                float factor = (energy > AppProperties::SedatedAtEnergyLevel) ? 1.0f : energy / AppProperties::SedatedAtEnergyLevel;

                glColor3f(1.0f - factor, factor, 0.0f);


                glIndexi(1); glVertex2f(0.0f, 15.0f);
                glIndexi(2); glVertex2f(4.5f, 0.0f);
                glIndexi(3); glVertex2f(-4.5f, 0.0f);

                glIndexi(4); glVertex2f(0.0f, 0.0f);
                glIndexi(5); glVertex2f(4.5f, 0.0f);
                glIndexi(6); glVertex2f(6.0f, -5.0f);

                glIndexi(7); glVertex2f(0.0f, 0.0f);
                glIndexi(8); glVertex2f(-4.5f, 0.0f);
                glIndexi(9); glVertex2f(-6.0f, -5.0f);

                glEnd();

                glPopMatrix();
            }
        }
    };
}
