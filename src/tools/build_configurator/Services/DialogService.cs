using System.Windows;

namespace BuildConfigurator.Services;

public class DialogService : IDialogService
{
    public bool Confirm(string message, string title = "Подтверждение")
    {
        var r = MessageBox.Show(message, title, MessageBoxButton.YesNo, MessageBoxImage.Question);
        return r == MessageBoxResult.Yes;
    }

    public ConfirmResult ConfirmWithCancel(string message, string title = "Подтверждение")
    {
        var r = MessageBox.Show(message, title, MessageBoxButton.YesNoCancel, MessageBoxImage.Question);
        return r switch
        {
            MessageBoxResult.Yes    => ConfirmResult.Yes,
            MessageBoxResult.No     => ConfirmResult.No,
            _                       => ConfirmResult.Cancel
        };
    }

    public void ShowError(string message, string title = "Ошибка")
        => MessageBox.Show(message, title, MessageBoxButton.OK, MessageBoxImage.Error);

    public void ShowInfo(string message, string title = "Информация")
        => MessageBox.Show(message, title, MessageBoxButton.OK, MessageBoxImage.Information);
}
