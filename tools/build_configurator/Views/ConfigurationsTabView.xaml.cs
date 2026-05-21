using System.Windows;
using System.Windows.Controls;
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

}
