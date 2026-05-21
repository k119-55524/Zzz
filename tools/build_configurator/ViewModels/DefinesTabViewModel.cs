using System.Collections.ObjectModel;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using BuildConfigurator.Models;
using BuildConfigurator.Services;

namespace BuildConfigurator.ViewModels;

public partial class DefinesTabViewModel : ViewModelBase
{
    private readonly IFileService   _fileService;
    private readonly IDialogService _dialogService;
    private AppData _data;

    public ObservableCollection<DefineItemViewModel> Defines { get; } = new();

    [ObservableProperty] private DefineItemViewModel? _selectedDefine;
    [ObservableProperty] private string _searchText = string.Empty;
    [ObservableProperty] private bool   _showArchivedOnly;

    public string StatusText => BuildStatusText();

    public event Action? DataChanged;

    public DefinesTabViewModel(AppData data, IFileService fileService, IDialogService dialogService)
    {
        _data          = data;
        _fileService   = fileService;
        _dialogService = dialogService;
        RebuildList();
    }

    public void UpdateData(AppData data)
    {
        _data = data;
        RebuildList();
    }

    private void RebuildList()
    {
        Defines.Clear();
        foreach (var d in _data.Defines)
            Defines.Add(new DefineItemViewModel(d));
        OnPropertyChanged(nameof(FilteredDefines));
        RefreshStatus();
    }

    private void RefreshStatus() => OnPropertyChanged(nameof(StatusText));

    private string BuildStatusText()
    {
        var total    = _data.Defines.Count;
        var active   = _data.Defines.Count(d => !d.IsArchived);
        var archived = _data.Defines.Count(d => d.IsArchived);
        return $"Всего дефайнов: {total} (активных: {active}, в архиве: {archived})";
    }

    public IEnumerable<DefineItemViewModel> FilteredDefines
    {
        get
        {
            if (string.IsNullOrWhiteSpace(SearchText) && !ShowArchivedOnly)
                return Defines;

            var q = Defines.AsEnumerable();
            if (!string.IsNullOrWhiteSpace(SearchText))
                q = q.Where(d => d.Name.Contains(SearchText, StringComparison.OrdinalIgnoreCase)
                               || d.Description.Contains(SearchText, StringComparison.OrdinalIgnoreCase));
            if (ShowArchivedOnly)
                q = q.Where(d => d.IsArchived);
            return q.ToList();
        }
    }

    partial void OnSearchTextChanged(string value)      => OnPropertyChanged(nameof(FilteredDefines));
    partial void OnShowArchivedOnlyChanged(bool value)  => OnPropertyChanged(nameof(FilteredDefines));

    // Called from view when IsArchived checkbox is toggled
    public void OnIsArchivedChanged()
    {
        _fileService.SaveData(_data);
        RefreshStatus();
        DataChanged?.Invoke();
    }

    // ── Commands ─────────────────────────────────────────────────────────────

    [RelayCommand]
    private void AddDefine((string name, string description) args)
    {
        var name = args.name.Trim().ToUpperInvariant();
        var desc = args.description.Trim();

        if (!IsValidDefineName(name))
        {
            _dialogService.ShowError(
                "Имя дефайна должно содержать только буквы, цифры и '_', и не начинаться с цифры.",
                "Неверное имя");
            return;
        }

        if (_data.Defines.Any(d => d.Name.Equals(name, StringComparison.OrdinalIgnoreCase)))
        {
            _dialogService.ShowError($"Дефайн «{name}» уже существует.", "Дубликат");
            return;
        }

        var define = new Define { Name = name, Description = desc };
        _data.Defines.Add(define);
        _fileService.SaveData(_data);

        var vm = new DefineItemViewModel(define);
        Defines.Add(vm);
        SelectedDefine = vm;
        OnPropertyChanged(nameof(FilteredDefines));
        RefreshStatus();
        DataChanged?.Invoke();
    }

    [RelayCommand]
    private void EditDefine((string name, string description) args)
    {
        if (SelectedDefine == null) return;

        var newName = args.name.Trim().ToUpperInvariant();
        var newDesc = args.description.Trim();

        if (!IsValidDefineName(newName))
        {
            _dialogService.ShowError(
                "Имя дефайна должно содержать только буквы, цифры и '_', и не начинаться с цифры.",
                "Неверное имя");
            return;
        }

        var oldName = SelectedDefine.Model.Name;
        var isDuplicateName = _data.Defines
            .Any(d => d != SelectedDefine.Model &&
                      d.Name.Equals(newName, StringComparison.OrdinalIgnoreCase));

        if (isDuplicateName)
        {
            _dialogService.ShowError($"Дефайн «{newName}» уже существует.", "Дубликат");
            return;
        }

        SelectedDefine.Model.Name        = newName;
        SelectedDefine.Model.Description = newDesc;

        if (!oldName.Equals(newName, StringComparison.Ordinal))
        {
            foreach (var cfg in _data.Configurations)
            {
                var idx = cfg.ActiveDefines.IndexOf(oldName);
                if (idx >= 0) cfg.ActiveDefines[idx] = newName;
            }
        }

        _fileService.SaveData(_data);
        SelectedDefine.Refresh();
        OnPropertyChanged(nameof(FilteredDefines));
        DataChanged?.Invoke();
    }

    [RelayCommand(CanExecute = nameof(CanDelete))]
    private void DeleteDefine()
    {
        if (SelectedDefine == null) return;

        var usedIn = _data.Configurations
            .Where(c => c.ActiveDefines.Contains(SelectedDefine.Name))
            .Select(c => $"  • {c.Name}")
            .ToList();

        var warning = usedIn.Count > 0
            ? $"\n\nОн используется в конфигурациях:\n{string.Join('\n', usedIn)}\n\nБудет удалён отовсюду безвозвратно."
            : "\n\nЭто действие необратимо.";

        if (!_dialogService.Confirm(
                $"Удалить дефайн «{SelectedDefine.Name}»?{warning}",
                "Удаление"))
            return;

        foreach (var cfg in _data.Configurations)
            cfg.ActiveDefines.Remove(SelectedDefine.Name);

        _data.Defines.Remove(SelectedDefine.Model);
        _fileService.SaveData(_data);

        Defines.Remove(SelectedDefine);
        SelectedDefine = null;
        OnPropertyChanged(nameof(FilteredDefines));
        RefreshStatus();
        DataChanged?.Invoke();
    }

    private bool CanDelete() => SelectedDefine != null;

    partial void OnSelectedDefineChanged(DefineItemViewModel? value)
    {
        DeleteDefineCommand.NotifyCanExecuteChanged();
    }

    private static bool IsValidDefineName(string name)
    {
        if (string.IsNullOrEmpty(name)) return false;
        if (char.IsDigit(name[0])) return false;
        return name.All(c => char.IsLetterOrDigit(c) || c == '_');
    }
}
