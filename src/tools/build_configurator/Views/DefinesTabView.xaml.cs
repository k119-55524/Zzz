using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using BuildConfigurator.ViewModels;
using BuildConfigurator.Views.Dialogs;

namespace BuildConfigurator.Views;

public partial class DefinesTabView : UserControl
{
    private DefinesTabViewModel VM => (DefinesTabViewModel)DataContext;

    public DefinesTabView() => InitializeComponent();

    private void AddDefine_Click(object sender, RoutedEventArgs e)
    {
        var existingNames = VM.Defines.Select(d => d.Name).ToList();
        var dialog = new AddEditDefineDialog("Новый дефайн", existingNames, VM.IsCMakeTab)
        {
            Owner = Window.GetWindow(this)
        };
        if (dialog.ShowDialog() != true) return;
        VM.AddDefineCommand.Execute((dialog.DefineName, dialog.DefineDescription));
    }

    private void DefinesGrid_DoubleClick(object sender, MouseButtonEventArgs e)
    {
        if (VM.SelectedDefine == null) return;
        if (e.OriginalSource is not DependencyObject source) return;
        if (FindVisualParent<DataGridRow>(source) == null) return;
        OpenEditDefineDialog();
    }

    private void OpenEditDefineDialog()
    {
        if (VM.SelectedDefine == null) return;
        var existingNames = VM.Defines.Select(d => d.Name).ToList();
        var dialog = new AddEditDefineDialog(
            "Редактировать дефайн",
            existingNames,
            VM.IsCMakeTab,
            VM.SelectedDefine.Name,
            VM.SelectedDefine.Description)
        {
            Owner = Window.GetWindow(this)
        };
        if (dialog.ShowDialog() != true) return;
        VM.EditDefineCommand.Execute((dialog.DefineName, dialog.DefineDescription));
    }

    private void ArchiveButton_Click(object sender, RoutedEventArgs e)
    {
        var define = (sender as FrameworkElement)?.DataContext as DefineItemViewModel;
        if (define != null)
        {
            define.IsArchived = !define.IsArchived;
            VM?.OnIsArchivedChanged(define);
        }
    }

    private static T? FindVisualParent<T>(DependencyObject child) where T : DependencyObject
    {
        var parent = VisualTreeHelper.GetParent(child);
        while (parent != null)
        {
            if (parent is T found) return found;
            parent = VisualTreeHelper.GetParent(parent);
        }
        return null;
    }
}
