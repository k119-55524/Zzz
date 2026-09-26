using System.IO;
using System.Windows;
using Microsoft.Win32;

namespace assets_builder_gui.Services;

public class WpfDialogService : IDialogService
{
    public bool ShowConfirmation(string title, string message)
    {
        var result = MessageBox.Show(message, title, MessageBoxButton.YesNo, MessageBoxImage.Warning);
        return result == MessageBoxResult.Yes;
    }

    public SaveCloseChoice ShowSaveOnCloseConfirmation(string title, string message)
    {
        var result = MessageBox.Show(message, title, MessageBoxButton.YesNoCancel, MessageBoxImage.Question);
        return result switch
        {
            MessageBoxResult.Yes => SaveCloseChoice.Save,
            MessageBoxResult.No => SaveCloseChoice.DontSave,
            _ => SaveCloseChoice.Cancel
        };
    }


    public void ShowError(string title, string message)
    {
        MessageBox.Show(message, title, MessageBoxButton.OK, MessageBoxImage.Error);
    }

    public string? SelectFolder(string title, string initialPath)
    {
        var dialog = new OpenFolderDialog
        {
            Title = title,
            InitialDirectory = Directory.Exists(initialPath) ? initialPath : string.Empty
        };

        return dialog.ShowDialog() == true ? dialog.FolderName : null;
    }

    public void CopyToClipboard(string text)
    {
        try
        {
            Clipboard.SetText(text);
        }
        catch
        {
            // Ignore clipboard errors
        }
    }
}
