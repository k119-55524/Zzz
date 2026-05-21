using System.Windows;
using System.Windows.Controls;
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
        var dialog = new AddEditDefineDialog("Новый дефайн", existingNames)
        {
            Owner = Window.GetWindow(this)
        };
        if (dialog.ShowDialog() != true) return;
        VM.AddDefineCommand.Execute((dialog.DefineName, dialog.DefineDescription));
    }

    private void EditDefine_Click(object sender, RoutedEventArgs e)
    {
        if (VM.SelectedDefine == null) return;

        var existingNames = VM.Defines.Select(d => d.Name).ToList();
        var dialog = new AddEditDefineDialog(
            "Редактировать дефайн",
            existingNames,
            VM.SelectedDefine.Name,
            VM.SelectedDefine.Description)
        {
            Owner = Window.GetWindow(this)
        };
        if (dialog.ShowDialog() != true) return;
        VM.EditDefineCommand.Execute((dialog.DefineName, dialog.DefineDescription));
    }

    private void ArchiveCheckBox_Changed(object sender, RoutedEventArgs e)
        => VM?.OnIsArchivedChanged();
}
