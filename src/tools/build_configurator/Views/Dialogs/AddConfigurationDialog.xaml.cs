using System.Windows;

namespace BuildConfigurator.Views.Dialogs;

public partial class AddConfigurationDialog : Window
{
    public string ConfigName        { get; private set; } = string.Empty;
    public string ConfigDescription { get; private set; } = string.Empty;

    // Существующие имена передаются для проверки на уникальность
    private readonly IReadOnlyCollection<string> _existingNames;

    public AddConfigurationDialog(IReadOnlyCollection<string> existingNames)
    {
        _existingNames = existingNames;
        InitializeComponent();
        NameBox.Focus();
    }

    private void NameBox_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
    {
        NameCounter.Text = $"{NameBox.Text.Length} / 25";
        ValidateName();
    }

    private void DescBox_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
    {
        DescCounter.Text = $"{DescBox.Text.Length} / 100";
    }

    private void ValidateName()
    {
        var name = NameBox.Text.Trim();

        if (string.IsNullOrEmpty(name))
        {
            ShowNameError(string.Empty);
            OkButton.IsEnabled = false;
            return;
        }

        if (_existingNames.Any(n => n.Equals(name, StringComparison.OrdinalIgnoreCase)))
        {
            ShowNameError($"Конфигурация «{name}» уже существует.");
            OkButton.IsEnabled = false;
            return;
        }

        HideNameError();
        OkButton.IsEnabled = true;
    }

    private void ShowNameError(string msg)
    {
        NameError.Text       = msg;
        NameError.Visibility = string.IsNullOrEmpty(msg) ? Visibility.Collapsed : Visibility.Visible;
    }

    private void HideNameError() => NameError.Visibility = Visibility.Collapsed;

    private void Ok_Click(object sender, RoutedEventArgs e)
    {
        ConfigName        = NameBox.Text.Trim();
        ConfigDescription = DescBox.Text.Trim();
        DialogResult = true;
    }

    private void Cancel_Click(object sender, RoutedEventArgs e) => DialogResult = false;
}
