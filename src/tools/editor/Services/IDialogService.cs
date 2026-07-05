using System.Windows;

namespace editor.Services
{
    public interface IDialogService
    {
        MessageBoxResult ShowMessage(string message, string title, MessageBoxButton buttons, MessageBoxImage icon);

        void ShowAbout();
    }
}
