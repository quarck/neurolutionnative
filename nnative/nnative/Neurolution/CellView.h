#pragma once 

#include <memory>
#include "Cell.h"

#include <GL/gl.h>			/* OpenGL header file */
#include <GL/glu.h>			/* OpenGL utilities header file */


namespace Neurolution
{
	struct Color
	{
		unsigned short R, G, B;
		Color(int r, int g, int b) : R(r), G(g), B(b) {}
		Color() : Color(255, 255, 255) {}

		void GlApply() noexcept
		{
			glColor3f(R / 255.0f, G / 255.0f, B / 255.0f);
		}
	};

	template <typename WorldProp, typename TCell>
    class CellView
    {
        std::shared_ptr<TCell> _cell;

		Color _predatorColor{ 64, 64, 255 };
	public:

        CellView(std::shared_ptr<TCell>& c, Random& rnd)
            : _cell(c)
        {
        }

        void DrawNonPredator()
        {
            if (_cell->CurrentEnergy >= 0.0001f &&
                _cell->LocationX >= 0.0 && _cell->LocationX < WorldProp::WorldWidth
                && _cell->LocationY >= 0.0 && _cell->LocationY < WorldProp::WorldHeight)
            {
                glPushMatrix();

                glTranslatef(_cell->LocationX, _cell->LocationY, 0.0);
                glRotatef(
                    static_cast<float>(_cell->Rotation / M_PI * 180.0 - 90.0),
                    0.0f, 0.0f, 1.0f);

                glBegin(GL_TRIANGLES);

                float energy = _cell->CurrentEnergy;
                float factor = (energy > WorldProp::SedatedAtEnergyLevel) ? 1.0f : energy / WorldProp::SedatedAtEnergyLevel;

                glColor3f(1.0f - factor, factor, 0.0f);

				constexpr float scale = 1;

                glIndexi(1); glVertex2f(0.0f, 15.0f * scale);
                glIndexi(2); glVertex2f(4.5f * scale, 0.0f);
                glIndexi(3); glVertex2f(-4.5f * scale, 0.0f);

                glIndexi(4); glVertex2f(0.0f, 0.0f);
                glIndexi(5); glVertex2f(4.5f * scale, 0.0f);
                glIndexi(6); glVertex2f(6.0f * scale, -5.0f * scale);

                glIndexi(7); glVertex2f(0.0f, 0.0f);
                glIndexi(8); glVertex2f(-4.5f * scale, 0.0f);
                glIndexi(9); glVertex2f(-6.0f * scale, -5.0f * scale);

				glColor3f(1.0f, 0.5f, 0.5f);

				glIndexi(10); glVertex2f(-5.0f * scale, -4.0f * scale);
				glIndexi(11); glVertex2f(-7 * scale, -(5.0f + 5.0f * _cell->TotalMoveForceLeft) * scale);
				glIndexi(12); glVertex2f(-5 * scale, -(5.0f + 5.0f * _cell->TotalMoveForceLeft) * scale);

				glIndexi(13); glVertex2f(5.0f * scale, -4.0f * scale);
				glIndexi(14); glVertex2f(7 * scale, -(5.0f + 5.0f * _cell->TotalMoveForceRight) * scale);
				glIndexi(15); glVertex2f(5 * scale, -(5.0f + 5.0f * _cell->TotalMoveForceRight) * scale);

				_cell->TotalMoveForceLeft = 0.0f;
				_cell->TotalMoveForceRight = 0.0f;

                glEnd();

                glPopMatrix();
            }
        }

		void DrawPredator()
		{
			if (_cell->CurrentEnergy < 0.001f)
				return;

			glPushMatrix();

			glTranslatef(_cell->LocationX, _cell->LocationY, 0.0);
			glRotatef(
				static_cast<float>(_cell->Rotation / M_PI * 180.0 - 90.0),
				0.0f, 0.0f, 1.0f);

			_predatorColor.GlApply();

			float halfdiameter = 10.0f;

			glBegin(GL_TRIANGLES);

			constexpr float scale = 1;

			int idx = 0;
			glIndexi(++idx); glVertex3f(0.0f, 2.5f * halfdiameter * scale, 0.0f);
			glIndexi(++idx); glVertex3f(halfdiameter / 4.0f * scale, 0.0f, 0.0f);
			glIndexi(++idx); glVertex3f(-halfdiameter / 4.0f * scale, 0.0f, 0.0f);

			glIndexi(++idx); glVertex3f(0.0f, -halfdiameter * scale, 0.0f);
			glIndexi(++idx); glVertex3f(halfdiameter / 4.0f * scale, 0.0f, 0.0f);
			glIndexi(++idx); glVertex3f(-halfdiameter / 4.0f * scale, 0.0f, 0.0f);


			glIndexi(++idx); glVertex3f(halfdiameter * scale, 0.0f, 0.0f);
			glIndexi(++idx); glVertex3f(0.0f, halfdiameter / 4.0f * scale, 0.0f);
			glIndexi(++idx); glVertex3f(0.0f, -halfdiameter / 4.0f * scale, 0.0f);

			glIndexi(++idx); glVertex3f(-halfdiameter * scale, 0.0f, 0.0f);
			glIndexi(++idx); glVertex3f(0.0f, halfdiameter / 4.0f * scale, 0.0f);
			glIndexi(++idx); glVertex3f(0.0f, -halfdiameter / 4.0f * scale, 0.0f);

			glColor3f(1.0f, 0.5f, 0.5f);

			glIndexi(++idx); glVertex2f(-6.0f * scale, -4.0f * scale);
			glIndexi(++idx); glVertex2f(-8 * scale, -(5.0f + 5.0f * _cell->TotalMoveForceLeft) * scale);
			glIndexi(++idx); glVertex2f(-6 * scale, -(5.0f + 5.0f * _cell->TotalMoveForceLeft) * scale);

			glIndexi(++idx); glVertex2f(6.0f * scale, -4.0f * scale);
			glIndexi(++idx); glVertex2f(8 * scale, -(5.0f + 5.0f * _cell->TotalMoveForceRight) * scale);
			glIndexi(++idx); glVertex2f(6 * scale, -(5.0f + 5.0f * _cell->TotalMoveForceRight) * scale);

			_cell->TotalMoveForceLeft = 0.0f;
			_cell->TotalMoveForceRight = 0.0f;

			glEnd();

			glPopMatrix();
		}

		void Draw()
		{
			if (_cell->IsPredator)
				DrawPredator();
			else
				DrawNonPredator();
		}
    };
}
