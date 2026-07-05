using System.Windows;

namespace BuildConfigurator.Views.Dialogs;

public partial class EditConfigurationDialog : Window
{
    public string ConfigName        { get; private set; }
    public string ConfigDescription { get; private set; }

    private readonly IReadOnlyCollection<string> _existingNames;
    private readonly string _originalName;

    public EditConfigurationDialog(string currentName, string currentDescription, IReadOnlyCollection<string> existingNames)
    {
        _existingNames = existingNames;
        _originalName  = currentName;
        ConfigName        = currentName;
        ConfigDescription = currentDescription;
        InitializeComponent();
        NameBox.Text     = currentName;
        DescBox.Text     = currentDescription;
        NameCounter.Text = $"{currentName.Length} / 25";
        DescCounter.Text = $"{currentDescription.Length} / 100";
        NameBox.Focus();
        NameBox.SelectAll();
    }

    private void NameBox_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
    {
        NameCounter.Text = $"{NameBox.Text.Length} / 25";
        ValidateName();
    }

    private void DescBox_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
        => DescCounter.Text = $"{DescBox.Text.Length} / 100";

    private void ValidateName()
    {
        var name = NameBox.Text.Trim();

        if (string.IsNullOrEmpty(name))
        {
            ShowNameError(string.Empty);
            OkButton.IsEnabled = false;
            return;
        }

        var isDuplicate = _existingNames.Any(n =>
            !n.Equals(_originalName, StringComparison.OrdinalIgnoreCase) &&
             n.Equals(name, StringComparison.OrdinalIgnoreCase));

        if (isDuplicate)
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
