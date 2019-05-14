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

#include <GL/gl.h>			/* OpenGL header file */
#include <GL/glu.h>			/* OpenGL utilities header file */

#include "../glText.h"

#include "World.h"
#include "CellView.h"

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

	struct WorldViewDetails
	{
		int numActiveThreads;
		long currentIteration;
		int iterationsPerSecond;
		bool showDetailedcontrols;
		bool paused;

		WorldViewDetails(int nThr, bool p) 
			: numActiveThreads{ nThr }
			, currentIteration { 0 }
			, iterationsPerSecond { 0 }
			, showDetailedcontrols { false }
			, paused { p }
		{

		}
	};

    class WorldView
    {
        std::shared_ptr<World> _world;
        std::vector<std::shared_ptr<CellView>> CellViews;

		static constexpr uint32_t labelBgs = 0xff191919;
		static constexpr uint32_t controlsLabelFg = 0xff0f0f7f;
		static constexpr uint32_t rugxaKoloro = 0xff0f0fdf;

		glText::Label controlsLabel{ labelBgs, "<?> - help", controlsLabelFg };

		glText::Label controlsLabelDetailed{
			labelBgs, 
			{
				std::pair("<S> - Save,  <L> - Load, <R> - Reset", rugxaKoloro),
				std::pair("<F> - (Un)Freeze Predators, <B> - Brainwash predators", rugxaKoloro),
				std::pair("<G> - Recover hamsters", rugxaKoloro),
				std::pair("<?> - help ON/OFF, <SPACE> - (un)pause, <esc> - quit", rugxaKoloro),
			}
		};

		glText::Label pausedLabel{ labelBgs, "<< PAUSED >>", rugxaKoloro };

    public:

        //public Line[] FoodLocations;

        //public Line[] PredatorLocations;

        Color foodColor{ 192, 64, 64 };
        Color predatorColor{ 64, 64, 255 };

        WorldView(std::shared_ptr<World>& world)
            : _world(world)
            , CellViews(world->Cells.size())
        {
            Random rnd = Random();

            for (int i = 0; i < _world->Cells.size(); ++i)
            {
                CellViews[i] = std::make_shared<CellView>(_world->Cells[i], rnd);
            }
        }

		void PrintControls(const WorldViewDetails& details) noexcept
		{
			glPushMatrix();

			glPixelZoom(1.f, 1.f);

			((details.showDetailedcontrols || details.paused) ? controlsLabelDetailed : controlsLabel)
				.DrawAt(-1.0, -0.99);

			if (details.paused)
				pausedLabel.DrawAt(-0.2, 0);

			glPopMatrix();
		}

		void PrintStats(const WorldViewDetails& details) noexcept
		{
			glPushMatrix();

			glPixelZoom(1.f, 1.f);

			std::ostringstream ostr;
			ostr << "ITER: " << details.currentIteration << ", IPS: " << details.iterationsPerSecond;

			std::ostringstream rcfg;
			rcfg << "#THR: " << details.numActiveThreads;

			glText::Label lbl(0xff191919, 
				{ 
					std::pair(ostr.str(), 0xff006f00u), 
					std::pair(rcfg.str(), 0xff9f004fu),
				});
			lbl.DrawAt(-1.0, 0.88);

			glPopMatrix();
		}

        void UpdateFrom(std::shared_ptr<World>& world, 
			const WorldViewDetails& details
		)  noexcept
        {
            glPushMatrix();

            // BG BEGIN
            glBegin(GL_TRIANGLES);

            glColor3f(0.1f, 0.1f, 0.1f);

            glIndexi(1); glVertex2f(1.0f, 1.0f);
            glIndexi(2); glVertex2f(-1.0f, 1.0f);
            glIndexi(3); glVertex2f(-1.0f, -1.0f);

            glIndexi(4); glVertex2f(1.0f, 1.0f);
            glIndexi(5); glVertex2f(1.0f, -1.0f);
            glIndexi(6); glVertex2f(-1.0f, -1.0f);

            glEnd();
            // BG END
			PrintControls(details);
			PrintStats(details);

            glScalef(
                static_cast<GLfloat>(2.0 / AppProperties::WorldWidth),
                static_cast<GLfloat>(2.0 / AppProperties::WorldHeight),
                1.0f);

            glTranslatef(-AppProperties::WorldWidth / 2.0f, -AppProperties::WorldHeight / 2.0f, 0.0);

            for (auto& cellView : CellViews)
            {
                cellView->Draw();
            }

            for (auto& food : _world->Foods)
            {
                if (food.Value < 0.01)
                    continue;

                glPushMatrix();
                glTranslatef(food.LocationX, food.LocationY, 0.0);
                //glRotatef(cell->Rotation, 0.0f, 0.0f, 1.0f);

                glBegin(GL_TRIANGLES);

                foodColor.GlApply();

                float halfdiameter = static_cast<float>(std::sqrt(food.Value) * 5.0 / 1.5);

                int idx = 0;
                glIndexi(++idx); glVertex3f(0.0f, halfdiameter, 0.0f);
                glIndexi(++idx); glVertex3f(halfdiameter / 1.5f, 0.0f, 0.0f);
                glIndexi(++idx); glVertex3f(-halfdiameter / 1.5f, 0.0f, 0.0f);

                glIndexi(++idx); glVertex3f(0.0f, -halfdiameter, 0.0f);
                glIndexi(++idx); glVertex3f(halfdiameter / 1.5f, 0.0f, 0.0f);
                glIndexi(++idx); glVertex3f(-halfdiameter / 1.5f, 0.0f, 0.0f);


                glIndexi(++idx); glVertex3f(halfdiameter, 0.0f, 0.0f);
                glIndexi(++idx); glVertex3f(0.0f, halfdiameter / 1.5f, 0.0f);
                glIndexi(++idx); glVertex3f(0.0f, -halfdiameter / 1.5f, 0.0f);

                glIndexi(++idx); glVertex3f(-halfdiameter, 0.0f, 0.0f);
                glIndexi(++idx); glVertex3f(0.0f, halfdiameter / 1.5f, 0.0f);
                glIndexi(++idx); glVertex3f(0.0f, -halfdiameter / 1.5f, 0.0f);

                glEnd();

                glPopMatrix();
            }

            for (auto& predator : _world->Predators)
            {
                if (predator->CurrentEnergy < 0.001f)
                    continue; // dead one

                glPushMatrix();

                glTranslatef(predator->LocationX, predator->LocationY, 0.0);
                glRotatef(
                    static_cast<float>(predator->Rotation / M_PI * 180.0 - 90.0),
                    0.0f, 0.0f, 1.0f);

                predatorColor.GlApply();

                float halfdiameter = 10.0f;

                glBegin(GL_TRIANGLES);

                int idx = 0;
                glIndexi(++idx); glVertex3f(0.0f, 2.5f*halfdiameter, 0.0f);
                glIndexi(++idx); glVertex3f(halfdiameter / 4.0f, 0.0f, 0.0f);
                glIndexi(++idx); glVertex3f(-halfdiameter / 4.0f, 0.0f, 0.0f);

                glIndexi(++idx); glVertex3f(0.0f, -halfdiameter, 0.0f);
                glIndexi(++idx); glVertex3f(halfdiameter / 4.0f, 0.0f, 0.0f);
                glIndexi(++idx); glVertex3f(-halfdiameter / 4.0f, 0.0f, 0.0f);


                glIndexi(++idx); glVertex3f(halfdiameter, 0.0f, 0.0f);
                glIndexi(++idx); glVertex3f(0.0f, halfdiameter / 4.0f, 0.0f);
                glIndexi(++idx); glVertex3f(0.0f, -halfdiameter / 4.0f, 0.0f);

                glIndexi(++idx); glVertex3f(-halfdiameter, 0.0f, 0.0f);
                glIndexi(++idx); glVertex3f(0.0f, halfdiameter / 4.0f, 0.0f);
                glIndexi(++idx); glVertex3f(0.0f, -halfdiameter / 4.0f, 0.0f);

                glEnd();

                glPopMatrix();
            }

            glPopMatrix();

		}
    };
}
