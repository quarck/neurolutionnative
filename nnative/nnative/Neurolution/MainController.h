#pragma once 

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

#include <Commdlg.h>
#include <Windows.h> // file dialogs 

#include "World.h"
#include "WorldView.h"
#include "RuntimeConfig.h"

namespace Neurolution
{
    class MainController
    {
        RuntimeConfig& config;

        std::shared_ptr<World> world;
        std::mutex worldLock;
        std::shared_ptr<WorldView> _worldView;


		WorldViewDetails viewDetails;

		//int iterationPerSeconds{ 0 };
		//long currentStep{ 0 };

        std::thread calcThread;

        std::atomic_bool terminate{ false };

        std::atomic_bool uiNeedsUpdate{ false };
        std::atomic_bool appPaused{ true };

        HDC hDC;				/* device context */
        HPALETTE hPalette{ 0 };			/* custom palette (if needed) */

        HWND hWND;
    public:

        MainController(RuntimeConfig& cfg)
            : config(cfg)
			, viewDetails { cfg.GetNumWorkerThreads(), true }
        {
            //string documents = Environment.GetFolderPath(Environment.SpecialFolder.Desktop);
            //string workingFolder = $"{documents}\\Neurolution\\{DateTime.Now:yyyy-MM-dd-HH-mm}";

            world = std::make_shared<World>(
                std::string(""),
                config.GetNumWorkerThreads(),
                AppProperties::WorldSize,
                AppProperties::FoodCountPerIteration,
                AppProperties::PredatorCountPerIteration,
                AppProperties::WorldWidth,
                AppProperties::WorldHeight);

            _worldView = std::make_shared<WorldView>(world);
        }

        ~MainController()
        {
            terminate = true;
            if (calcThread.joinable())
                calcThread.join();
        }

        void Start()
        {
            calcThread = std::thread(&MainController::CalcThread, this);
        }

        void Stop()
        {
            terminate = true;
        }

        void CalcThread()
        {
            auto lastUIUpdate = std::chrono::high_resolution_clock::now();
			long lastUpdateAt = 0;

            for (viewDetails.currentIteration = 0; !terminate; ++viewDetails.currentIteration)
            {
				while (appPaused && !terminate)
				{
					::Sleep(100);
					uiNeedsUpdate = true;
					::SendMessage(hWND, WM_USER, 0, 0);
					while (uiNeedsUpdate)
					{
						// Keep yeild-ing the thread while UI thread is doing the painting job, 
						// this is to avoid the white lock situation
						std::this_thread::yield();
					}
				}

                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> sinceLastUpdate = std::chrono::duration_cast<std::chrono::duration<double>>(now - lastUIUpdate);

                if (viewDetails.currentIteration % 4 == 0 && sinceLastUpdate.count() > 1.0 / 30.0)
                {
					viewDetails.iterationsPerSecond = static_cast<long>((viewDetails.currentIteration - lastUpdateAt) / sinceLastUpdate.count());

                    lastUIUpdate = now;
					lastUpdateAt = viewDetails.currentIteration;

                    uiNeedsUpdate = true;
                    ::SendMessage(hWND, WM_USER, 0, 0);
                    while (uiNeedsUpdate)
                    {
                        // Keep yeild-ing the thread while UI thread is doing the painting job, 
                        // this is to avoid the white lock situation
                        std::this_thread::yield();
                    }
                }

                std::lock_guard<std::mutex> l(worldLock);
                world->Iterate(viewDetails.currentIteration);
            }
        }

		void OnKeyboard(WPARAM wParam)
		{
			switch (wParam)
			{
			case 27:			/* ESC key */
				Stop();
				PostQuitMessage(0);
				break;
			case ' ':
				appPaused = !appPaused;
				break;

			case '?':
				viewDetails.showDetailedcontrols = !viewDetails.showDetailedcontrols;
				__faststorefence();
				break;

			case 'S': case 's': 
				onSave();
				break;

			case 'L': case 'l': 
				onLoad();
				break;

			case 'r': case 'R': 
				onResetWorld();
				break;

			case 'f': case 'F': 
				onToggleFreezePredators();
				break;

			case 'b': case 'B': 
				onBrainwashPredators();
				break;

			case 'g': case 'G': 
				onRecoverHamsters();
				break;
			}
		}

        void DrawWorld()
        {
            std::lock_guard<std::mutex> l(worldLock);
			viewDetails.paused = appPaused;
            _worldView->UpdateFrom(world, viewDetails);
            uiNeedsUpdate = false;
        }

        bool IsUINeedsUpdate() const { return uiNeedsUpdate; }
        void ClearUINeedsUpdate() { uiNeedsUpdate = false; }

        bool IsAppPaused() const { return appPaused; }
        void SetAppIsPaused(bool val) { appPaused = val; }

        bool IsTerminating() const { return terminate; }


        const HWND& GetHWND() const { return hWND; }
        void SetHWND(HWND hwnd) 
        { 
            hWND = hwnd; 
            SetHDC(GetDC(hWND));
        }

        const HDC& GetHDC() const { return hDC; }
        void SetHDC(HDC hdc) { hDC = hdc; }

        const HPALETTE& GetHPalette() const { return hPalette; }
        void SetHPalette(HPALETTE hp) { hPalette = hp; }

	private:

		void onSave()
		{
			TCHAR file[MAX_PATH] = _T("");

			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);

			ofn.hwndOwner = hWND;
			ofn.lpstrFilter = _T("N-Network (*.nn)\0*.nn\0");
			ofn.lpstrFile = &file[0];
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.lpstrDefExt = _T("nn");

			if (::GetSaveFileName(&ofn))
			{
				int i = 0;
				// proceed 
			}
		}

		void onLoad()
		{
			TCHAR file[MAX_PATH] = _T("");

			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);

			ofn.hwndOwner = hWND;
			ofn.lpstrFilter = _T("N-Network (*.nn)\0*.nn\0");
			ofn.lpstrFile = &file[0];
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			ofn.lpstrDefExt = _T("nn");

			if (::GetOpenFileName(&ofn))
			{
				int i = 0;
				// proceed 
			}
		}

		void onResetWorld()
		{

		}

		void onToggleFreezePredators()
		{

		}

		void onBrainwashPredators()
		{

		}

		void onRecoverHamsters()
		{

		}
    };
}
