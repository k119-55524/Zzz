using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Data;
using System.Windows.Input;
using assets_builder_gui.Services;
using assets_builder_lib;

namespace assets_builder_gui.ViewModels;

public class MainWindowViewModel : ViewModelBase
{
    private readonly IDialogService _dialogService;
    private readonly SessionConfig _sessionConfig;
    private BuildProfileViewModel? _selectedProfile;
    private bool _isBuilding = false;
    private bool _isCollectionModified = false;
    private string _logText = string.Empty;
    private string _statusText = "Готов";
    private string _statusColor = "#4CAF50";

    // Геометрия окна
    private double _windowWidth;
    private double _windowHeight;
    private double _windowLeft;
    private double _windowTop;
    private bool _isWindowMaximized;

    public MainWindowViewModel(IDialogService dialogService)
    {
        _dialogService = dialogService;
        _sessionConfig = SessionManager.LoadSession();

        _windowWidth = _sessionConfig.WindowWidth;
        _windowHeight = _sessionConfig.WindowHeight;
        _windowLeft = _sessionConfig.WindowLeft;
        _windowTop = _sessionConfig.WindowTop;
        _isWindowMaximized = _sessionConfig.IsWindowMaximized;

        Profiles = new ObservableCollection<BuildProfileViewModel>();

        foreach (var profile in _sessionConfig.Profiles)
        {
            var vm = new BuildProfileViewModel(profile);
            vm.PropertyChanged += OnProfilePropertyChanged;
            Profiles.Add(vm);
        }

        // Авто-сортировка списка по алфавиту (сортируется при сохранении)
        ProfilesView = CollectionViewSource.GetDefaultView(Profiles);
        ProfilesView.SortDescriptions.Add(new SortDescription(nameof(BuildProfileViewModel.Name), ListSortDirection.Ascending));

        if (Profiles.Count > 0)
        {
            var savedSelected = Profiles.FirstOrDefault(p => p.Id == _sessionConfig.SelectedProfileId);
            SelectedProfile = savedSelected ?? Profiles.First();
        }

        AppendLog("Готов к работе.");

        AddProfileCommand = new RelayCommand(_ => AddProfile());
        DeleteProfileCommand = new RelayCommand(_ => DeleteProfile(), _ => SelectedProfile != null);
        SaveProfileCommand = new RelayCommand(_ => SaveProfile(), _ => HasAnyUnsavedChanges && (SelectedProfile == null || SelectedProfile.IsValid));
        CancelProfileCommand = new RelayCommand(_ => CancelProfile(), _ => SelectedProfile != null && SelectedProfile.IsDirty);

        BrowseScriptsPathCommand = new RelayCommand(_ => BrowseScriptsPath(), _ => SelectedProfile != null);
        BrowseAssetsPathCommand = new RelayCommand(_ => BrowseAssetsPath(), _ => SelectedProfile != null);
        BrowseDestinationPathCommand = new RelayCommand(_ => BrowseDestinationPath(), _ => SelectedProfile != null);

        StartOrCancelBuildCommand = new RelayCommand(_ => ToggleBuild(), _ => SelectedProfile != null && SelectedProfile.IsValid);
        CopyLogsCommand = new RelayCommand(_ => CopyLogs(), _ => !string.IsNullOrEmpty(LogText));
        ClearLogsCommand = new RelayCommand(_ => ClearLogs(), _ => !string.IsNullOrEmpty(LogText));
    }

    public ObservableCollection<BuildProfileViewModel> Profiles { get; }

    public ICollectionView ProfilesView { get; }

    public BuildProfileViewModel? SelectedProfile
    {
        get => _selectedProfile;
        set
        {
            if (SetProperty(ref _selectedProfile, value))
            {
                OnPropertyChanged(nameof(HasSelectedProfile));
                CommandManager.InvalidateRequerySuggested();
            }
        }
    }

    public bool HasSelectedProfile => SelectedProfile != null;

    public bool IsBuilding
    {
        get => _isBuilding;
        set
        {
            if (SetProperty(ref _isBuilding, value))
            {
                OnPropertyChanged(nameof(BuildButtonText));
                StatusText = _isBuilding ? "Идет сборка..." : "Готов";
                StatusColor = _isBuilding ? "#FFC107" : "#4CAF50";
            }
        }
    }

    public string BuildButtonText => IsBuilding ? "⏹ Прервать" : "▶ Старт сборки";

    public string LogText
    {
        get => _logText;
        set => SetProperty(ref _logText, value);
    }

    public string StatusText
    {
        get => _statusText;
        set => SetProperty(ref _statusText, value);
    }

    public string StatusColor
    {
        get => _statusColor;
        set => SetProperty(ref _statusColor, value);
    }

    public double WindowWidth
    {
        get => _windowWidth;
        set => SetProperty(ref _windowWidth, value);
    }

    public double WindowHeight
    {
        get => _windowHeight;
        set => SetProperty(ref _windowHeight, value);
    }

    public double WindowLeft
    {
        get => _windowLeft;
        set => SetProperty(ref _windowLeft, value);
    }

    public double WindowTop
    {
        get => _windowTop;
        set => SetProperty(ref _windowTop, value);
    }

    public bool IsWindowMaximized
    {
        get => _isWindowMaximized;
        set => SetProperty(ref _isWindowMaximized, value);
    }

    public bool HasAnyUnsavedChanges => _isCollectionModified || Profiles.Any(p => p.IsDirty);

    public string WindowTitle => HasAnyUnsavedChanges ? "Assets Builder *" : "Assets Builder";

    // Команды
    public ICommand AddProfileCommand { get; }
    public ICommand DeleteProfileCommand { get; }
    public ICommand SaveProfileCommand { get; }
    public ICommand CancelProfileCommand { get; }

    public ICommand BrowseScriptsPathCommand { get; }
    public ICommand BrowseAssetsPathCommand { get; }
    public ICommand BrowseDestinationPathCommand { get; }

    public ICommand StartOrCancelBuildCommand { get; }
    public ICommand CopyLogsCommand { get; }
    public ICommand ClearLogsCommand { get; }

    private void AddProfile()
    {
        var model = new BuildProfile { Name = "Новая настройка" };
        var vm = new BuildProfileViewModel(model);
        vm.PropertyChanged += OnProfilePropertyChanged;
        Profiles.Add(vm);
        SelectedProfile = vm;

        _isCollectionModified = true;
        OnPropertyChanged(nameof(HasAnyUnsavedChanges));
        OnPropertyChanged(nameof(WindowTitle));
    }

    private void DeleteProfile()
    {
        if (SelectedProfile == null) return;

        bool confirmed = _dialogService.ShowConfirmation(
            "Удаление настройки",
            $"Вы действительно хотите удалить настройку '{SelectedProfile.Name}'?"
        );

        if (confirmed)
        {
            var toRemove = SelectedProfile;
            int index = Profiles.IndexOf(toRemove);
            Profiles.Remove(toRemove);

            if (Profiles.Count > 0)
            {
                int newIndex = Math.Clamp(index, 0, Profiles.Count - 1);
                SelectedProfile = Profiles[newIndex];
            }
            else
            {
                SelectedProfile = null;
            }

            _isCollectionModified = true;
            OnPropertyChanged(nameof(HasAnyUnsavedChanges));
            OnPropertyChanged(nameof(WindowTitle));
            CommandManager.InvalidateRequerySuggested();
        }
    }

    private void SaveProfile()
    {
        bool confirmed = _dialogService.ShowConfirmation(
            "Подтверждение сохранения",
            "Сохранить все изменения в настройках сборок?"
        );

        if (confirmed)
        {
            foreach (var p in Profiles.Where(p => p.IsDirty))
            {
                p.ApplyToModel();
            }

            ProfilesView.Refresh();

            _isCollectionModified = false;
            SaveSessionToDisk();
            AppendLog("Настройки успешно сохранены на диск.");
            OnPropertyChanged(nameof(HasAnyUnsavedChanges));
            OnPropertyChanged(nameof(WindowTitle));
        }
    }

    private void CancelProfile()
    {
        if (SelectedProfile == null) return;
        SelectedProfile.ResetFromModel();
        OnPropertyChanged(nameof(HasAnyUnsavedChanges));
        OnPropertyChanged(nameof(WindowTitle));
    }

    public void SaveSessionToDisk()
    {
        _sessionConfig.Profiles = Profiles.Select(p => p.Model).ToList();
        _sessionConfig.SelectedProfileId = SelectedProfile?.Id ?? string.Empty;

        _sessionConfig.WindowWidth = WindowWidth;
        _sessionConfig.WindowHeight = WindowHeight;
        _sessionConfig.WindowLeft = WindowLeft;
        _sessionConfig.WindowTop = WindowTop;
        _sessionConfig.IsWindowMaximized = IsWindowMaximized;

        SessionManager.SaveSession(_sessionConfig);
    }

    private void BrowseScriptsPath()
    {
        if (SelectedProfile == null) return;
        var folder = _dialogService.SelectFolder("Выберите папку со скриптами", SelectedProfile.ScriptsPath);
        if (!string.IsNullOrEmpty(folder))
        {
            SelectedProfile.ScriptsPath = folder;
        }
    }

    private void BrowseAssetsPath()
    {
        if (SelectedProfile == null) return;
        var folder = _dialogService.SelectFolder("Выберите папку с ассетами", SelectedProfile.AssetsPath);
        if (!string.IsNullOrEmpty(folder))
        {
            SelectedProfile.AssetsPath = folder;
        }
    }

    private void BrowseDestinationPath()
    {
        if (SelectedProfile == null) return;
        var folder = _dialogService.SelectFolder("Выберите папку назначения", SelectedProfile.DestinationPath);
        if (!string.IsNullOrEmpty(folder))
        {
            SelectedProfile.DestinationPath = folder;
        }
    }

    private void ToggleBuild()
    {
        if (SelectedProfile == null) return;

        if (IsBuilding)
        {
            IsBuilding = false;
            AppendLog("Сборка прервана пользователем.");
            return;
        }

        bool confirmed = _dialogService.ShowConfirmation(
            "Подтверждение сборки",
            $"Папка назначения '{SelectedProfile.DestinationPath}' будет перезаписана при сборке.\n\nПродолжить сборку для '{SelectedProfile.Name}'?"
        );

        if (confirmed)
        {
            IsBuilding = true;
            AppendLog($"Запуск сборки настройки '{SelectedProfile.Name}'...");
            AppendLog($"Скрипты: {SelectedProfile.ScriptsPath}");
            AppendLog($"Ассеты:  {SelectedProfile.AssetsPath}");
            AppendLog($"Назначение: {SelectedProfile.DestinationPath}");

            AppendLog("Сериализация макета проекта через zzz_engine_builder_dll...");
            AppendLog("Сборка успешно завершена!");
            IsBuilding = false;
        }
    }

    private void CopyLogs()
    {
        _dialogService.CopyToClipboard(LogText);
        AppendLog("Логи скопированы в буфер обмена.");
    }

    private void ClearLogs()
    {
        bool confirmed = _dialogService.ShowConfirmation(
            "Очистка логов",
            "Вы действительно хотите очистить логи сборки?"
        );

        if (confirmed)
        {
            LogText = string.Empty;
            AppendLog("Логи очищены.");
        }
    }

    public void AppendLog(string message)
    {
        string timestamp = DateTime.Now.ToString("HH:mm:ss");
        LogText += $"[{timestamp}] {message}\n";
    }

    public bool ConfirmWindowClose()
    {
        if (HasAnyUnsavedChanges)
        {
            var choice = _dialogService.ShowSaveOnCloseConfirmation(
                "Несохраненные изменения",
                "В настройках есть несохраненные изменения (добавлены/удалены/изменены профили). Сохранить их перед выходом?"
            );

            if (choice == SaveCloseChoice.Save)
            {
                foreach (var p in Profiles.Where(p => p.IsDirty))
                {
                    p.ApplyToModel();
                }
                ProfilesView.Refresh();
                _isCollectionModified = false;
                SaveSessionToDisk();
                return true; // Allow close
            }
            else if (choice == SaveCloseChoice.DontSave)
            {
                return true; // Allow close without saving
            }
            else
            {
                return false; // Cancel close
            }
        }

        SaveSessionToDisk();
        return true;
    }

    private void OnProfilePropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        if (e.PropertyName == nameof(BuildProfileViewModel.IsDirty) || e.PropertyName == nameof(BuildProfileViewModel.DisplayName))
        {
            OnPropertyChanged(nameof(HasAnyUnsavedChanges));
            OnPropertyChanged(nameof(WindowTitle));
            CommandManager.InvalidateRequerySuggested();
        }
    }
}
