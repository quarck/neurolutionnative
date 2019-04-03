#pragma once 

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

#include "World.h"
#include "WorldView.h"


namespace Neurolution
{
    class MainController
    {
        std::shared_ptr<World> world;
		std::mutex worldLock;
		std::shared_ptr<WorldView> _worldView;

		std::thread calcThread;

		std::atomic_bool terminate{ false };

	public:

		std::atomic_bool uiNeedsUpdate{ false };


        MainController()
        {
            //string documents = Environment.GetFolderPath(Environment.SpecialFolder.Desktop);
            //string workingFolder = $"{documents}\\Neurolution\\{DateTime.Now:yyyy-MM-dd-HH-mm}";

            world = std::make_shared<World>(
                std::string(""),
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

        //private void HideAllControls()
        //{
        //    ButtonStartNew.Visibility = Visibility.Collapsed;
        //    ButtonLoadSavedWorld.Visibility = Visibility.Collapsed;
        //    ButtonLoadSavedTop.Visibility = Visibility.Collapsed;
        //    MultiThreaded.Visibility = Visibility.Collapsed;
        //}

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

            for (long step = 0; !terminate ; ++step)
            {			
                {
					std::lock_guard<std::mutex> l(worldLock);
                    world->Iterate(step);
                }

                if (step % 4 == 0)
                {
					uiNeedsUpdate = true;
                    //try
                    //{
                    //    var s = step;
                    //    Dispatcher.Invoke(() => UpdateUI(s));
                    //}
                    //catch (TaskCanceledException)
                    //{
                    //    world.Save();
                    //    break;
                    //}
                }

                //if (DateTime.Now > end)
                //    break;
            }
        }

		void DrawWorld()
		{
			std::lock_guard<std::mutex> l(worldLock);
			_worldView->UpdateFrom(world);
		}

        //private void UpdateUI(long step)
        //{
        //    lock (world)
        //    {
        //        _worldView.UpdateFrom(world);
        //        GenerationLabel.Content = $"{step:D8}";
        //    }
        //}
	};
}
