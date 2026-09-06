using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using assets_builder_gui.ViewModels;

namespace assets_builder_gui;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        Loaded += MainWindow_Loaded;
    }

    private void MainWindow_Loaded(object sender, RoutedEventArgs e)
    {
        if (DataContext is MainWindowViewModel vm)
        {
            if (vm.PresetsPanelHeight >= 170)
            {
                PresetsRow.Height = new GridLength(vm.PresetsPanelHeight, GridUnitType.Pixel);
            }

            if (vm.IsWindowMaximized)
            {
                WindowState = WindowState.Maximized;
                UpdateMaximizeButtons();
            }

            vm.LogItems.CollectionChanged += (s, ev) =>
            {
                Dispatcher.InvokeAsync(() =>
                {
                    LogScrollViewer.ScrollToBottom();
                });
            };
        }
    }

    private void GridSplitter_DragCompleted(object sender, DragCompletedEventArgs e)
    {
        if (DataContext is MainWindowViewModel vm && PresetsRow.ActualHeight >= 170)
        {
            vm.PresetsPanelHeight = PresetsRow.ActualHeight;
            vm.SaveSessionToDisk();
        }
    }

    private void MinimizeWindow_Executed(object sender, ExecutedRoutedEventArgs e)
    {
        SystemCommands.MinimizeWindow(this);
    }

    private void MaximizeWindow_Executed(object sender, ExecutedRoutedEventArgs e)
    {
        SystemCommands.MaximizeWindow(this);
        UpdateMaximizeButtons();
    }

    private void RestoreWindow_Executed(object sender, ExecutedRoutedEventArgs e)
    {
        SystemCommands.RestoreWindow(this);
        UpdateMaximizeButtons();
    }

    private void CloseWindow_Executed(object sender, ExecutedRoutedEventArgs e)
    {
        SystemCommands.CloseWindow(this);
    }

    private void UpdateMaximizeButtons()
    {
        if (WindowState == WindowState.Maximized)
        {
            MaximizeButton.Visibility = Visibility.Collapsed;
            RestoreButton.Visibility = Visibility.Visible;
        }
        else
        {
            MaximizeButton.Visibility = Visibility.Visible;
            RestoreButton.Visibility = Visibility.Collapsed;
        }
    }

    protected override void OnStateChanged(EventArgs e)
    {
        base.OnStateChanged(e);
        UpdateMaximizeButtons();
    }

    protected override void OnClosing(CancelEventArgs e)
    {
        if (DataContext is MainWindowViewModel vm)
        {
            vm.IsWindowMaximized = (WindowState == WindowState.Maximized);
            if (WindowState != WindowState.Maximized)
            {
                vm.WindowWidth = Width;
                vm.WindowHeight = Height;
                vm.WindowLeft = Left;
                vm.WindowTop = Top;
            }

            if (PresetsRow.ActualHeight >= 170)
            {
                vm.PresetsPanelHeight = PresetsRow.ActualHeight;
            }

            if (!vm.ConfirmWindowClose())
            {
                e.Cancel = true;
                return;
            }
        }

        base.OnClosing(e);
    }
}
