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

    public ObservableCollection<ConfigItemViewModel> ConfigItems { get; } = new();

    [ObservableProperty] private ConfigItemViewModel? _selectedConfigItem;
    [ObservableProperty] private string _definesSearchText = string.Empty;

    // Computed from ConfigItems — set via RefreshHasUnsavedChanges()
    private bool _hasUnsavedChanges;
    public bool HasUnsavedChanges
    {
        get => _hasUnsavedChanges;
        private set
        {
            if (SetProperty(ref _hasUnsavedChanges, value))
                SaveConfigurationCommand.NotifyCanExecuteChanged();
        }
    }

    public ObservableCollection<DefineEntryViewModel> DefineEntries { get; } = new();

    public IEnumerable<DefineEntryViewModel> FilteredEntries => string.IsNullOrWhiteSpace(DefinesSearchText)
        ? DefineEntries
        : DefineEntries.Where(e =>
              e.Name.Contains(DefinesSearchText, StringComparison.OrdinalIgnoreCase) ||
              e.Description.Contains(DefinesSearchText, StringComparison.OrdinalIgnoreCase));

    public string ActiveDefinesCountText => $"Активные дефайны ({DefineEntries.Count(e => e.IsActive && !e.IsArchived)})";
    public string TotalConfigsText       => $"Всего конфигураций: {ConfigItems.Count}";
    public bool   HasSelectedConfig      => SelectedConfigItem != null;

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
        if (SelectedConfigItem == null) return;
        // Preserve pending checkbox changes by flushing them to the model first
        if (SelectedConfigItem.HasUnsavedChanges)
            CommitDefineEntriesToModel(SelectedConfigItem.Configuration);
        LoadDefineEntries(SelectedConfigItem.Configuration);
    }

    private ConfigItemViewModel CreateConfigItem(BuildConfiguration cfg)
    {
        var item = new ConfigItemViewModel(cfg);
        item.PropertyChanged += (_, e) =>
        {
            if (e.PropertyName == nameof(ConfigItemViewModel.HasUnsavedChanges))
                RefreshHasUnsavedChanges();
        };
        return item;
    }

    private void RefreshHasUnsavedChanges()
        => HasUnsavedChanges = ConfigItems.Any(c => c.HasUnsavedChanges);

    private void RebuildList(string? selectName)
    {
        ConfigItems.Clear();
        foreach (var cfg in _data.Configurations)
            ConfigItems.Add(CreateConfigItem(cfg));

        OnPropertyChanged(nameof(TotalConfigsText));

        var toSelect = ConfigItems.FirstOrDefault(c => c.Name == selectName) ?? ConfigItems.FirstOrDefault();
        SelectedConfigItem = toSelect;
    }

    // Auto-commit DefineEntries to model BEFORE switching away from a config
    partial void OnSelectedConfigItemChanging(ConfigItemViewModel? oldValue, ConfigItemViewModel? newValue)
    {
        if (oldValue != null && DefineEntries.Any())
            CommitDefineEntriesToModel(oldValue.Configuration);
    }

    partial void OnSelectedConfigItemChanged(ConfigItemViewModel? value)
    {
        OnPropertyChanged(nameof(HasSelectedConfig));
        DeleteConfigurationCommand.NotifyCanExecuteChanged();
        if (value != null)
            LoadDefineEntries(value.Configuration);
        else
            ClearDefineEntries();
    }

    private void LoadDefineEntries(BuildConfiguration cfg)
    {
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
        // Do NOT reset HasUnsavedChanges here — dirty state persists until global Save
    }

    private void ClearDefineEntries()
    {
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
        {
            MarkDirty();
            OnPropertyChanged(nameof(ActiveDefinesCountText));
        }
    }

    private void MarkDirty()
    {
        if (SelectedConfigItem != null)
            SelectedConfigItem.HasUnsavedChanges = true;
        // HasUnsavedChanges computed via subscription
    }

    partial void OnDefinesSearchTextChanged(string value)
        => OnPropertyChanged(nameof(FilteredEntries));

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

        var item = CreateConfigItem(cfg);
        ConfigItems.Add(item);
        SelectedConfigItem = item;
        OnPropertyChanged(nameof(TotalConfigsText));
        DataChanged?.Invoke();
    }

    [RelayCommand]
    private void EditConfiguration((string name, string description) args)
    {
        if (SelectedConfigItem == null) return;
        var cfg = SelectedConfigItem.Configuration;
        cfg.Name        = args.name;
        cfg.Description = args.description;
        _fileService.SaveData(_data);
        SelectedConfigItem.RefreshName();
        MarkDirty();
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
        CommitActiveDefines();
        _fileService.SaveData(_data);
        DataChanged?.Invoke();
    }

    [RelayCommand]
    private void CancelChanges()
    {
        if (SelectedConfigItem == null) return;
        // Reload from model (discards uncommitted checkbox changes)
        SelectedConfigItem.HasUnsavedChanges = false;
        LoadDefineEntries(SelectedConfigItem.Configuration);
    }

    // ── Commit helpers ───────────────────────────────────────────────────────

    // Writes current DefineEntries to model without touching dirty flags
    private void CommitDefineEntriesToModel(BuildConfiguration cfg)
    {
        cfg.ActiveDefines = DefineEntries
            .Where(e => e.IsActive && !e.IsArchived)
            .Select(e => e.Name)
            .ToList();
        cfg.ActiveDefines.AddRange(DefineEntries
            .Where(e => e.IsActive && e.IsArchived)
            .Select(e => e.Name));
        foreach (var e in DefineEntries) e.AcceptChanges();
    }

    // Commits current config and clears its dirty flag
    private void CommitActiveDefines()
    {
        if (SelectedConfigItem == null) return;
        CommitDefineEntriesToModel(SelectedConfigItem.Configuration);
        SelectedConfigItem.HasUnsavedChanges = false;
    }

    // ── Public API for MainWindowViewModel ──────────────────────────────────

    // Commit current config before global file save
    public bool ApplyChanges()
    {
        if (!HasUnsavedChanges) return true;
        CommitActiveDefines();
        // All other configs were auto-committed in OnSelectedConfigItemChanging
        return true;
    }

    // Clear all dirty flags after global file save
    public void ClearAllDirty()
    {
        foreach (var item in ConfigItems)
            item.HasUnsavedChanges = false;
    }

    public void DiscardChanges()
    {
        ClearAllDirty();
        if (SelectedConfigItem != null)
            LoadDefineEntries(SelectedConfigItem.Configuration);
    }

    public bool TrySave()
    {
        SaveConfiguration();
        return !HasUnsavedChanges;
    }
}
