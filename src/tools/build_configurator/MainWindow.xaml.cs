using System.ComponentModel;
using System.Windows;
using BuildConfigurator.Services;
using BuildConfigurator.ViewModels;

namespace BuildConfigurator;

public partial class MainWindow : Window
{
    private MainWindowViewModel _vm = null!;

    public MainWindow()
    {
        InitializeComponent();
    }

    private void Window_Loaded(object sender, RoutedEventArgs e)
    {
        var fileService   = new FileService();
        var dialogService = new DialogService();

        _vm = new MainWindowViewModel(fileService, dialogService);

        if (!_vm.Initialize())
        {
            Close();
            return;
        }

        DataContext = _vm;
    }

    private void Window_Closing(object sender, CancelEventArgs e)
    {
        if (_vm == null) return;
        if (!_vm.CanClose())
            e.Cancel = true;
    }
}
