using System.Windows;
using assets_builder_gui.Services;
using assets_builder_gui.ViewModels;

namespace assets_builder_gui;

public partial class App : Application
{
    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);

        var dialogService = new WpfDialogService();
        var mainViewModel = new MainWindowViewModel(dialogService);

        if (e.Args.Contains("--build"))
        {
            [System.Runtime.InteropServices.DllImport("kernel32.dll")]
            static extern bool AttachConsole(int dwProcessId);
            AttachConsole(-1);

            try
            {
                mainViewModel.BuildHeadless();
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[CLI Build Error] {ex}");
            }

            Shutdown();
            return;
        }

        var mainWindow = new MainWindow
        {
            DataContext = mainViewModel
        };

        mainWindow.Show();
    }
}
