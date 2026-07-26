namespace assets_builder_gui.Services;

public enum SaveCloseChoice
{
    Save,
    DontSave,
    Cancel
}

public interface IDialogService
{
    bool ShowConfirmation(string title, string message);
    SaveCloseChoice ShowSaveOnCloseConfirmation(string title, string message);
    string? SelectFolder(string title, string initialPath);
    void CopyToClipboard(string text);
}
