using System.Windows.Controls;

namespace BuildConfigurator.Views;

// TODO (2026-07-05): вкладка настроек ещё не реализована и нигде не подключена
// в MainWindow.xaml (нет соответствующего TabItem). Реализовать содержимое вкладки
// либо удалить SettingsTabView/SettingsTabViewModel/CMakeService/ICMakeService.
public partial class SettingsTabView : UserControl
{
    public SettingsTabView() => InitializeComponent();
}
