namespace BuildConfigurator.Services;

public enum ConfirmResult { Yes, No, Cancel }

public interface IDialogService
{
    bool Confirm(string message, string title = "Подтверждение");
    ConfirmResult ConfirmWithCancel(string message, string title = "Подтверждение");
    void ShowError(string message, string title = "Ошибка");
    void ShowInfo(string message, string title = "Информация");
}
