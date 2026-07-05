using System.Text.RegularExpressions;
using System.Windows;

namespace BuildConfigurator.Views.Dialogs;

public partial class AddEditDefineDialog : Window
{
    public string DefineName        { get; private set; } = string.Empty;
    public string DefineDescription { get; private set; } = string.Empty;

    private readonly IReadOnlyCollection<string> _existingNames;
    private readonly string _originalName;
    private readonly bool _isCMake;
    private static readonly Regex ValidName = new(@"^[A-Z_][A-Z0-9_]*$", RegexOptions.Compiled);

    public AddEditDefineDialog(
        string title,
        IReadOnlyCollection<string> existingNames,
        bool isCMake,
        string initialName = "",
        string initialDescription = "")
    {
        _existingNames = existingNames;
        _originalName  = initialName;
        _isCMake       = isCMake;
        InitializeComponent();
        Title          = title;
        OkButton.Content = string.IsNullOrEmpty(initialName) ? "Добавить" : "Сохранить";
        NameBox.Text   = initialName;
        DescBox.Text   = initialDescription;
        NameBox.Focus();
        NameBox.SelectAll();
    }

    private void NameBox_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
        => ValidateName();

    private void DescBox_TextChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
        => DescCounter.Text = $"{DescBox.Text.Length} / 256";

    private void ValidateName()
    {
        var raw  = NameBox.Text;
        var name = raw.Trim().ToUpperInvariant();

        if (string.IsNullOrEmpty(name))
        {
            ShowError(string.Empty);
            OkButton.IsEnabled = false;
            return;
        }

        if (!ValidName.IsMatch(name))
        {
            ShowError("Недопустимые символы в имени.");
            OkButton.IsEnabled = false;
            return;
        }

        var isDuplicate = _existingNames.Any(n =>
            !n.Equals(_originalName, StringComparison.OrdinalIgnoreCase) &&
             n.Equals(name, StringComparison.OrdinalIgnoreCase));

        if (isDuplicate)
        {
            ShowError($"Дефайн «{name}» уже существует.");
            OkButton.IsEnabled = false;
            return;
        }

        HideError();
        OkButton.IsEnabled = true;
    }

    private void ShowError(string msg)
    {
        NameError.Text       = msg;
        NameError.Visibility = string.IsNullOrEmpty(msg) ? Visibility.Collapsed : Visibility.Visible;
    }

    private void HideError() => NameError.Visibility = Visibility.Collapsed;

    private void Ok_Click(object sender, RoutedEventArgs e)
    {
        DefineName        = NameBox.Text.Trim().ToUpperInvariant();
        DefineDescription = DescBox.Text.Trim();
        DialogResult = true;
    }

    private void Cancel_Click(object sender, RoutedEventArgs e) => DialogResult = false;
}
