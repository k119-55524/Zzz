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

        var mainWindow = new MainWindow
        {
            DataContext = mainViewModel
        };

        mainWindow.Show();
    }
}
