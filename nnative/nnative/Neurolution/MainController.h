#pragma once 

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

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

		int iterationPerSeconds{ 0 };
		long currentStep{ 0 };

        std::thread calcThread;

        std::atomic_bool terminate{ false };

        std::atomic_bool uiNeedsUpdate{ false };
        std::atomic_bool appPaused{ false };

        HDC hDC;				/* device context */
        HPALETTE hPalette{ 0 };			/* custom palette (if needed) */

        HWND hWND;
    public:

        MainController(RuntimeConfig& cfg)
            : config(cfg)
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

            for (currentStep = 0; !terminate; ++currentStep)
            {
                while (appPaused && !terminate)
                    ::Sleep(100);

                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> sinceLastUpdate = std::chrono::duration_cast<std::chrono::duration<double>>(now - lastUIUpdate);

                if (currentStep % 4 == 0 && sinceLastUpdate.count() > 1.0 / 30.0)
                {
					iterationPerSeconds = static_cast<long>((currentStep - lastUpdateAt) / sinceLastUpdate.count());

                    lastUIUpdate = now;
					lastUpdateAt = currentStep;

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
                world->Iterate(currentStep);
            }
        }

        void DrawWorld()
        {
            std::lock_guard<std::mutex> l(worldLock);
            _worldView->UpdateFrom(world, currentStep, iterationPerSeconds, config.GetNumWorkerThreads());
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
    };
}
