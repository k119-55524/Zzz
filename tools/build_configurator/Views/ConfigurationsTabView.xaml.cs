using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using BuildConfigurator.ViewModels;
using BuildConfigurator.Views.Dialogs;

namespace BuildConfigurator.Views;

public partial class ConfigurationsTabView : UserControl
{
    private ConfigurationsTabViewModel VM => (ConfigurationsTabViewModel)DataContext;

    public ConfigurationsTabView() => InitializeComponent();

    private void AddConfig_Click(object sender, RoutedEventArgs e)
    {
        var existingNames = VM.ConfigItems.Select(c => c.Name).ToList();
        var dialog = new AddConfigurationDialog(existingNames) { Owner = Window.GetWindow(this) };
        if (dialog.ShowDialog() != true) return;
        VM.AddConfigurationCommand.Execute((dialog.ConfigName, dialog.ConfigDescription));
    }

    private void ConfigListBox_DoubleClick(object sender, MouseButtonEventArgs e)
    {
        if (VM.SelectedConfigItem == null) return;
        if (e.OriginalSource is not DependencyObject source) return;
        if (FindVisualParent<ListBoxItem>(source) == null) return;
        OpenEditConfigDialog();
    }

    private void OpenEditConfigDialog()
    {
        var item = VM.SelectedConfigItem;
        if (item == null) return;
        var existingNames = VM.ConfigItems.Select(c => c.Name).ToList();
        var dialog = new EditConfigurationDialog(item.Name, item.Description, existingNames)
        {
            Owner = Window.GetWindow(this)
        };
        if (dialog.ShowDialog() != true) return;
        VM.EditConfigurationCommand.Execute((dialog.ConfigName, dialog.ConfigDescription));
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
