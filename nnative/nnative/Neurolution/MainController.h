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


        //private void buttonStartNew_Click(object sender, RoutedEventArgs e)
        //{
        //    HideAllControls();
        //    _worldView = new WorldView(grid, world);
        //    world.MultiThreaded = MultiThreaded.IsChecked ?? false;
        //    new Thread(this.CalcThread).Start();
        //}

        //private void buttonLoadSavedTop_Click(object sender, RoutedEventArgs e)
        //{
        //    OpenFileDialog dialog = new OpenFileDialog();

        //    dialog.Filter = "XML Files (.xml)|*.xml|All Files (*.*)|*.*";
        //    dialog.FilterIndex = 1;

        //    dialog.Multiselect = false;

        //    bool? userClickedOK = dialog.ShowDialog();

        //    if (userClickedOK == true)
        //    {
        //        // Open the selected file to read.
        //        string filename = dialog.FileName;

        //        world.InitializeFromTopFile(filename);

        //        _worldView = new WorldView(grid, world);
        //        HideAllControls();

        //        world.MultiThreaded = MultiThreaded.IsChecked ?? false;

        //        new Thread(this.CalcThread).Start();
        //    }
        //}

        //private void buttonLoadSavedWorld_Click(object sender, RoutedEventArgs e)
        //{
        //    OpenFileDialog dialog = new OpenFileDialog();

        //    dialog.Filter = "XML Files (.xml)|*.xml|All Files (*.*)|*.*";
        //    dialog.FilterIndex = 1;

        //    dialog.Multiselect = false;

        //    bool? userClickedOK = dialog.ShowDialog();

        //    if (userClickedOK == true)
        //    {
        //        // Open the selected file to read.
        //        string filename = dialog.FileName;

        //        world.InitializeFromWorldFile(filename);

        //        _worldView = new WorldView(grid, world);
        //        HideAllControls();

        //        world.MultiThreaded = MultiThreaded.IsChecked ?? false;

        //        new Thread(this.CalcThread).Start();
        //    }
        //}

        void CalcThread()
        {
            //var start = DateTime.Now;
            //var end = start + TimeSpan.FromSeconds(20);

            auto lastUIUpdate = std::chrono::high_resolution_clock::now();

            for (long step = 0; !terminate; ++step)
            {
                while (appPaused && !terminate)
                    ::Sleep(100);

                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> sinceLastUpdate = std::chrono::duration_cast<std::chrono::duration<double>>(now - lastUIUpdate);

                if (step % 4 == 0 && sinceLastUpdate.count() > 1.0 / 30.0)
                {
                    lastUIUpdate = now;

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
                world->Iterate(step);
            }
        }

        void DrawWorld()
        {
            std::lock_guard<std::mutex> l(worldLock);
            _worldView->UpdateFrom(world);
            uiNeedsUpdate = false;
        }

        //private void UpdateUI(long step)
        //{
        //    lock (world)
        //    {
        //        _worldView.UpdateFrom(world);
        //        GenerationLabel.Content = $"{step:D8}";
        //    }
        //}


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
