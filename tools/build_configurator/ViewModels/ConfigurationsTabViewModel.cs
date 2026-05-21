using System.Collections.ObjectModel;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using BuildConfigurator.Models;
using BuildConfigurator.Services;

namespace BuildConfigurator.ViewModels;

public partial class ConfigurationsTabViewModel : ViewModelBase
{
    private readonly IFileService   _fileService;
    private readonly IDialogService _dialogService;
    private AppData _data;

    // Left panel list
    public ObservableCollection<ConfigItemViewModel> ConfigItems { get; } = new();

    [ObservableProperty] private ConfigItemViewModel? _selectedConfigItem;

    // Editor fields
    [ObservableProperty] private string _editingName        = string.Empty;
    [ObservableProperty] private string _editingDescription = string.Empty;
    private bool _isLoading;
    [ObservableProperty] private string _definesSearchText  = string.Empty;
    [ObservableProperty] private bool   _hasUnsavedChanges;

    public ObservableCollection<DefineEntryViewModel> DefineEntries { get; } = new();

    public IEnumerable<DefineEntryViewModel> FilteredEntries => string.IsNullOrWhiteSpace(DefinesSearchText)
        ? DefineEntries
        : DefineEntries.Where(e =>
              e.Name.Contains(DefinesSearchText, StringComparison.OrdinalIgnoreCase) ||
              e.Description.Contains(DefinesSearchText, StringComparison.OrdinalIgnoreCase));

    public string NameCountText        => $"{EditingName.Length} / 25";
    public string DescriptionCountText => $"{EditingDescription.Length} / 100";
    public string ActiveDefinesCountText => $"Активные дефайны ({DefineEntries.Count(e => e.IsActive && !e.IsArchived)})";
    public string TotalConfigsText     => $"Всего конфигураций: {ConfigItems.Count}";

    public bool HasSelectedConfig => SelectedConfigItem != null;

    public event Action? DataChanged;

    public ConfigurationsTabViewModel(AppData data, IFileService fileService, IDialogService dialogService)
    {
        _data          = data;
        _fileService   = fileService;
        _dialogService = dialogService;
        RebuildList(null);
    }

    public void UpdateData(AppData data)
    {
        _data = data;
        RebuildList(SelectedConfigItem?.Configuration.Name);
    }

    public void RefreshDefineEntries()
    {
        // Called when defines list changes (define added/archived/restored/deleted)
        if (SelectedConfigItem == null) return;
        LoadEditorFromConfig(SelectedConfigItem.Configuration);
    }

    private void RebuildList(string? selectName)
    {
        ConfigItems.Clear();
        foreach (var cfg in _data.Configurations)
            ConfigItems.Add(new ConfigItemViewModel(cfg));

        OnPropertyChanged(nameof(TotalConfigsText));

        var toSelect = ConfigItems.FirstOrDefault(c => c.Name == selectName) ?? ConfigItems.FirstOrDefault();
        SelectedConfigItem = toSelect;
    }

    partial void OnSelectedConfigItemChanged(ConfigItemViewModel? value)
    {
        HasUnsavedChanges = false;
        OnPropertyChanged(nameof(HasSelectedConfig));
        DeleteConfigurationCommand.NotifyCanExecuteChanged();
        if (value != null)
            LoadEditorFromConfig(value.Configuration);
        else
            ClearEditor();
    }

    private void LoadEditorFromConfig(BuildConfiguration cfg)
    {
        _isLoading = true;
        EditingName        = cfg.Name;
        EditingDescription = cfg.Description;
        _isLoading = false;

        UnsubscribeDefineEntries();
        DefineEntries.Clear();
        foreach (var d in _data.Defines)
        {
            var entry = DefineEntryViewModel.Create(
                d.Name, d.Description, d.IsArchived,
                cfg.ActiveDefines.Contains(d.Name));
            entry.PropertyChanged += OnDefineEntryPropertyChanged;
            DefineEntries.Add(entry);
        }

        OnPropertyChanged(nameof(FilteredEntries));
        OnPropertyChanged(nameof(ActiveDefinesCountText));
        HasUnsavedChanges = false;
        if (SelectedConfigItem != null)
            SelectedConfigItem.HasUnsavedChanges = false;
    }

    private void ClearEditor()
    {
        _isLoading = true;
        EditingName        = string.Empty;
        EditingDescription = string.Empty;
        _isLoading = false;
        UnsubscribeDefineEntries();
        DefineEntries.Clear();
        OnPropertyChanged(nameof(FilteredEntries));
    }

    private void UnsubscribeDefineEntries()
    {
        foreach (var entry in DefineEntries)
            entry.PropertyChanged -= OnDefineEntryPropertyChanged;
    }

    private void OnDefineEntryPropertyChanged(object? sender, System.ComponentModel.PropertyChangedEventArgs e)
    {
        if (e.PropertyName == nameof(DefineEntryViewModel.IsActive))
            OnDefineEntryChanged();
    }

    // ── Dirty tracking ───────────────────────────────────────────────────────

    partial void OnEditingNameChanged(string value)
    {
        OnPropertyChanged(nameof(NameCountText));
        if (!_isLoading) MarkDirty();
    }

    partial void OnEditingDescriptionChanged(string value)
    {
        OnPropertyChanged(nameof(DescriptionCountText));
        if (!_isLoading) MarkDirty();
    }

    partial void OnDefinesSearchTextChanged(string value)
        => OnPropertyChanged(nameof(FilteredEntries));

    private void OnDefineEntryChanged()
    {
        MarkDirty();
        OnPropertyChanged(nameof(ActiveDefinesCountText));
    }

    private void MarkDirty()
    {
        HasUnsavedChanges = true;
        if (SelectedConfigItem != null)
            SelectedConfigItem.HasUnsavedChanges = true;
    }

    // ── Commands ─────────────────────────────────────────────────────────────

    [RelayCommand]
    private void AddConfiguration((string name, string description) args)
    {
        var name = args.name.Trim();
        var desc = args.description.Trim();

        if (string.IsNullOrEmpty(name))
        {
            _dialogService.ShowError("Имя конфигурации не может быть пустым.", "Ошибка");
            return;
        }

        if (name.Length > 25)
        {
            _dialogService.ShowError("Имя не должно превышать 25 символов.", "Ошибка");
            return;
        }

        if (_data.Configurations.Any(c => c.Name.Equals(name, StringComparison.OrdinalIgnoreCase)))
        {
            _dialogService.ShowError($"Конфигурация «{name}» уже существует.", "Дубликат");
            return;
        }

        var cfg = new BuildConfiguration { Name = name, Description = desc };
        _data.Configurations.Add(cfg);
        _fileService.SaveData(_data);

        var item = new ConfigItemViewModel(cfg);
        ConfigItems.Add(item);
        SelectedConfigItem = item;
        OnPropertyChanged(nameof(TotalConfigsText));
        DataChanged?.Invoke();
    }

    [RelayCommand(CanExecute = nameof(CanDeleteConfig))]
    private void DeleteConfiguration()
    {
        if (SelectedConfigItem == null) return;

        if (!_dialogService.Confirm(
                $"Удалить конфигурацию «{SelectedConfigItem.Name}»?",
                "Удаление"))
            return;

        var cfg = SelectedConfigItem.Configuration;
        _data.Configurations.Remove(cfg);
        _fileService.SaveData(_data);

        ConfigItems.Remove(SelectedConfigItem);
        SelectedConfigItem = ConfigItems.FirstOrDefault();
        OnPropertyChanged(nameof(TotalConfigsText));
        DataChanged?.Invoke();
    }

    private bool CanDeleteConfig() => SelectedConfigItem != null;

    [RelayCommand(CanExecute = nameof(HasUnsavedChanges))]
    private void SaveConfiguration()
    {
        if (SelectedConfigItem == null) return;

        var name = EditingName.Trim();
        if (string.IsNullOrEmpty(name))
        {
            _dialogService.ShowError("Имя конфигурации не может быть пустым.", "Ошибка");
            return;
        }

        if (name.Length > 25)
        {
            _dialogService.ShowError("Имя не должно превышать 25 символов.", "Ошибка");
            return;
        }

        var duplicate = _data.Configurations.FirstOrDefault(c =>
            c != SelectedConfigItem.Configuration &&
            c.Name.Equals(name, StringComparison.OrdinalIgnoreCase));

        if (duplicate != null)
        {
            _dialogService.ShowError($"Конфигурация «{name}» уже существует.", "Дубликат");
            return;
        }

        if (!_dialogService.Confirm("Сохранить изменения конфигурации?", "Сохранение"))
            return;

        var cfg = SelectedConfigItem.Configuration;
        cfg.Name        = name;
        cfg.Description = EditingDescription.Trim();
        cfg.ActiveDefines = DefineEntries
            .Where(e => e.IsActive && !e.IsArchived)
            .Select(e => e.Name)
            .ToList();

        // Also persist archived defines that were active (remembered but excluded from cmake)
        var archivedActive = DefineEntries
            .Where(e => e.IsActive && e.IsArchived)
            .Select(e => e.Name);
        cfg.ActiveDefines.AddRange(archivedActive);

        _fileService.SaveData(_data);

        foreach (var e in DefineEntries) e.AcceptChanges();
        HasUnsavedChanges = false;
        SelectedConfigItem.HasUnsavedChanges = false;
        SelectedConfigItem.RefreshName();
        DataChanged?.Invoke();
    }

    [RelayCommand]
    private void CancelChanges()
    {
        if (SelectedConfigItem == null) return;
        LoadEditorFromConfig(SelectedConfigItem.Configuration);
    }

    partial void OnHasUnsavedChangesChanged(bool value)
    {
        SaveConfigurationCommand.NotifyCanExecuteChanged();
    }

    // Apply staged changes to the model without confirmation (used by global Save)
    public bool ApplyChanges()
    {
        if (SelectedConfigItem == null || !HasUnsavedChanges) return true;

        var name = EditingName.Trim();
        if (string.IsNullOrEmpty(name))
        {
            _dialogService.ShowError("Имя конфигурации не может быть пустым.", "Ошибка");
            return false;
        }
        if (name.Length > 25)
        {
            _dialogService.ShowError("Имя не должно превышать 25 символов.", "Ошибка");
            return false;
        }
        var duplicate = _data.Configurations.FirstOrDefault(c =>
            c != SelectedConfigItem.Configuration &&
            c.Name.Equals(name, StringComparison.OrdinalIgnoreCase));
        if (duplicate != null)
        {
            _dialogService.ShowError($"Конфигурация «{name}» уже существует.", "Дубликат");
            return false;
        }

        var cfg = SelectedConfigItem.Configuration;
        cfg.Name        = name;
        cfg.Description = EditingDescription.Trim();
        cfg.ActiveDefines = DefineEntries
            .Where(e => e.IsActive && !e.IsArchived)
            .Select(e => e.Name)
            .ToList();
        cfg.ActiveDefines.AddRange(DefineEntries
            .Where(e => e.IsActive && e.IsArchived)
            .Select(e => e.Name));

        foreach (var e in DefineEntries) e.AcceptChanges();
        HasUnsavedChanges = false;
        SelectedConfigItem.HasUnsavedChanges = false;
        SelectedConfigItem.RefreshName();
        return true;
    }

    public void DiscardChanges()
    {
        if (SelectedConfigItem != null)
            LoadEditorFromConfig(SelectedConfigItem.Configuration);
    }

    // Allow MainVM to programmatically save before switching configs
    public bool TrySave()
    {
        SaveConfiguration();
        return !HasUnsavedChanges;
    }
}
