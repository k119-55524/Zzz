using System.Text.Json;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using BuildConfigurator.Models;
using BuildConfigurator.Services;

namespace BuildConfigurator.ViewModels;

public partial class MainWindowViewModel : ViewModelBase
{
    private readonly IFileService   _fileService;
    private readonly IDialogService _dialogService;
    private AppData _data = new();

    [ObservableProperty] private string _windowTitle = "Build Configurator";

    public ConfigurationsTabViewModel ConfigurationsTab { get; private set; } = null!;
    public DefinesTabViewModel        ProjectDefinesTab { get; private set; } = null!;
    public DefinesTabViewModel        CMakeDefinesTab   { get; private set; } = null!;

    public bool HasUnsavedChanges => ConfigurationsTab?.HasUnsavedChanges == true
                                  || ProjectDefinesTab?.HasUnsavedChanges == true
                                  || CMakeDefinesTab?.HasUnsavedChanges == true;

    public MainWindowViewModel(IFileService fileService, IDialogService dialogService)
    {
        _fileService   = fileService;
        _dialogService = dialogService;
    }

    public bool Initialize()
    {
        try
        {
            _data = _fileService.LoadData();
        }
        catch (JsonException ex)
        {
            _dialogService.ShowError(
                $"Не удалось прочитать profiles.json:\n{ex.Message}\n\n" +
                "Исправьте файл вручную или удалите его, чтобы начать заново.\n" +
                $"Путь: {_fileService.GetDataFilePath()}",
                "Ошибка чтения данных");
            return false;
        }

        ConfigurationsTab = new ConfigurationsTabViewModel(_data, _fileService, _dialogService);
        ProjectDefinesTab = new DefinesTabViewModel(_data, _fileService, _dialogService, false);
        CMakeDefinesTab   = new DefinesTabViewModel(_data, _fileService, _dialogService, true);

        ConfigurationsTab.DataChanged += OnDataChanged;
        ProjectDefinesTab.DataChanged += OnDefinesDataChanged;
        CMakeDefinesTab.DataChanged   += OnDefinesDataChanged;
        ConfigurationsTab.PropertyChanged += (_, e) =>
        {
            if (e.PropertyName == nameof(ConfigurationsTab.HasUnsavedChanges))
            {
                OnPropertyChanged(nameof(HasUnsavedChanges));
                UpdateWindowTitle();
            }
        };
        ProjectDefinesTab.PropertyChanged += (_, e) =>
        {
            if (e.PropertyName == nameof(ProjectDefinesTab.HasUnsavedChanges))
            {
                OnPropertyChanged(nameof(HasUnsavedChanges));
                UpdateWindowTitle();
            }
        };
        CMakeDefinesTab.PropertyChanged += (_, e) =>
        {
            if (e.PropertyName == nameof(CMakeDefinesTab.HasUnsavedChanges))
            {
                OnPropertyChanged(nameof(HasUnsavedChanges));
                UpdateWindowTitle();
            }
        };

        OnPropertyChanged(nameof(ConfigurationsTab));
        OnPropertyChanged(nameof(ProjectDefinesTab));
        OnPropertyChanged(nameof(CMakeDefinesTab));
        UpdateWindowTitle();
        return true;
    }

    private void UpdateWindowTitle()
    {
        WindowTitle = HasUnsavedChanges ? "Build Configurator*" : "Build Configurator";
    }

    private void OnDataChanged() => UpdateWindowTitle();

    private void OnDefinesDataChanged()
    {
        ConfigurationsTab.RefreshDefineEntries();
        UpdateWindowTitle();
    }

    // ── Общее сохранение / перезагрузка ─────────────────────────────────────

    [RelayCommand]
    private void SaveAll()
    {
        if (!_dialogService.Confirm("Сохранить все изменения?", "Сохранение"))
            return;
        ExecuteSave();
    }

    // Вызывается без подтверждения из CanClose
    private void ExecuteSave()
    {
        if (!ConfigurationsTab.ApplyChanges()) return;
        _fileService.SaveData(_data);
        ConfigurationsTab.ClearAllDirty();
        ProjectDefinesTab.MarkSaved();
        CMakeDefinesTab.MarkSaved();
        OnPropertyChanged(nameof(HasUnsavedChanges));
        UpdateWindowTitle();
    }

    [RelayCommand]
    private void ReloadAll()
    {
        var message = HasUnsavedChanges
            ? "Есть несохранённые изменения.\nПерезагрузить данные с диска? Изменения будут потеряны."
            : "Перезагрузить данные с диска?";
        if (!_dialogService.Confirm(message, "Перезагрузка"))
            return;

        try { _data = _fileService.LoadData(); }
        catch (JsonException ex)
        {
            _dialogService.ShowError($"Не удалось прочитать profiles.json:\n{ex.Message}", "Ошибка");
            return;
        }

        ConfigurationsTab.UpdateData(_data);
        ProjectDefinesTab.UpdateData(_data);
        CMakeDefinesTab.UpdateData(_data);
        ConfigurationsTab.ClearAllDirty();
        ProjectDefinesTab.MarkSaved();
        CMakeDefinesTab.MarkSaved();
        OnPropertyChanged(nameof(HasUnsavedChanges));
        UpdateWindowTitle();
    }

    // ── Закрытие окна ────────────────────────────────────────────────────────

    public bool CanClose()
    {
        if (!HasUnsavedChanges) return true;

        var result = _dialogService.ConfirmWithCancel(
            "Есть несохранённые изменения.\nСохранить перед выходом?",
            "Выход");

        switch (result)
        {
            case ConfirmResult.Yes:
                ExecuteSave(); // без повторного подтверждения
                return true;
            case ConfirmResult.No:
                return true;
            default:
                return false;
        }
    }
}
