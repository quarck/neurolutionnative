#pragma once 

#include <memory>

#include <GL/gl.h>			/* OpenGL header file */
#include <GL/glu.h>			/* OpenGL utilities header file */

#include "../glText.h"

#include "World.h"
#include "CellView.h"

namespace Neurolution
{
	struct WorldViewDetails
	{
		int numActiveThreads;
		long currentIteration;
		int iterationsPerSecond;
		bool showDetailedcontrols;
		bool paused;

		int numPredators;
		int numPreys;

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
		static constexpr uint32_t LABELS_BACKGROUND = 0xff191919;
		static constexpr uint32_t CONTROLS_LABEL_FOREGROUND = 0xff0f0f7f;
		static constexpr uint32_t RUGA_KOLORO = 0xff0f0fdf;
		static constexpr uint32_t VERDA_KOLORO = 0xff108f10u;
		static constexpr uint32_t CFG_CLR_FOREGROUND = 0xff9f004fu;
		
		std::shared_ptr<World> _world;
        std::vector<std::shared_ptr<CellView>> _cellViews;

		glText::Label _controlsLabel{ LABELS_BACKGROUND, CONTROLS_LABEL_FOREGROUND, "<?> - help" };

		glText::Label _controlsLabelDetailed{
			LABELS_BACKGROUND,
			{
				std::pair(RUGA_KOLORO, "<S> - Save,  <L> - Load" /*", <R> - Reset" */),
//				std::pair(RUGA_KOLORO, "<F> - (Un)Freeze _predators, <B> - Brainwash predators"),
				//std::pair(RUGA_KOLORO, "<G> - Recover hamsters"),
				std::pair(RUGA_KOLORO, "<?> - help ON/OFF, <SPACE> - (un)pause, <esc> - quit"),
			}
		};

		glText::Label _iterAndCfgLabel{ LABELS_BACKGROUND, VERDA_KOLORO, "_TMP_" };

		glText::Label _pausedLabel{ LABELS_BACKGROUND, RUGA_KOLORO, "<< PAUSED >>" };

		Color _foodColor{ 192, 64, 64 };
		
    public:

        WorldView(std::shared_ptr<World>& world)
            : _world(world)
            , _cellViews(world->_cells.size() + world->_predators.size())
        {
            Random rnd = Random();

            for (int i = 0; i < _world->_cells.size(); ++i)
            {
                _cellViews[i] = std::make_shared<CellView>(_world->_cells[i], rnd);
            }

			int ofs = static_cast<int>(_world->_cells.size()); // static_please_please_please_cast<int>(..)
		
			for (int i = 0; i < _world->_predators.size(); ++i)
			{
				_cellViews[ofs + i] = std::make_shared<CellView>(_world->_predators[i], rnd);
			}
		}

		void PrintControls(const WorldViewDetails& details) noexcept
		{
			glPushMatrix();

			glPixelZoom(1.f, 1.f);

			((details.showDetailedcontrols || details.paused) ? _controlsLabelDetailed : _controlsLabel)
				.DrawAt(-1.0, -0.99);

			if (details.paused)
				_pausedLabel.DrawAt(-0.2, 0);

			glPopMatrix();
		}

		void PrintStats(const WorldViewDetails& details) noexcept
		{
			glPushMatrix();

			glPixelZoom(1.f, 1.f);

			std::ostringstream iterstr;
			iterstr << "ITER: " << details.currentIteration << ", IPS: " << details.iterationsPerSecond;

			std::ostringstream szstr;
			szstr << "PREDATORS: " << details.numPredators << ", PREY: " << details.numPreys;

			std::ostringstream rcfg;
			rcfg << "#THR: " << details.numActiveThreads;

			_iterAndCfgLabel.Update(
				LABELS_BACKGROUND,
				{ 
					std::pair(VERDA_KOLORO, iterstr.str()),
					std::pair(VERDA_KOLORO, szstr.str()),
					std::pair(CFG_CLR_FOREGROUND, rcfg.str()),
				});
			_iterAndCfgLabel.DrawAt(-1.0, 0.78);

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

            for (auto& cellView : _cellViews)
            {
                cellView->Draw();
            }

            for (auto& food : _world->_foods)
            {
                if (food.Value < 0.01)
                    continue;

                glPushMatrix();
                glTranslatef(food.LocationX, food.LocationY, 0.0);
                //glRotatef(cell->Rotation, 0.0f, 0.0f, 1.0f);

                glBegin(GL_TRIANGLES);

                _foodColor.GlApply();

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

            glPopMatrix();
		}
    };
}
