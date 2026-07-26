using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Input;
using assets_builder_lib;

namespace assets_builder_gui.ViewModels;

public class MainViewModel : INotifyPropertyChanged
{
    private readonly SessionConfig _sessionConfig;
    private ProfileViewModel? _selectedProfile;
    private bool _isBuilding = false;
    private string _logText = "[System] Готов к работе.\n";
    private string _statusText = "Готов";
    private string _statusColor = "#4CAF50";

    public MainViewModel()
    {
        _sessionConfig = SessionManager.LoadSession();
        Profiles = new ObservableCollection<ProfileViewModel>();

        foreach (var profile in _sessionConfig.Profiles)
        {
            var vm = new ProfileViewModel(profile);
            vm.PropertyChanged += OnProfilePropertyChanged;
            Profiles.Add(vm);
        }

        if (Profiles.Count > 0)
        {
            var savedSelected = Profiles.FirstOrDefault(p => p.Id == _sessionConfig.SelectedProfileId);
            SelectedProfile = savedSelected ?? Profiles.First();
        }

        // Инициализация команд
        AddProfileCommand = new RelayCommand(_ => AddProfile());
        DeleteProfileCommand = new RelayCommand(_ => DeleteProfile(), _ => SelectedProfile != null);
        SaveProfileCommand = new RelayCommand(_ => SaveProfile(), _ => SelectedProfile != null && SelectedProfile.IsDirty && SelectedProfile.IsValid);
        CancelProfileCommand = new RelayCommand(_ => CancelProfile(), _ => SelectedProfile != null && SelectedProfile.IsDirty);
        
        BrowseScriptsPathCommand = new RelayCommand(_ => BrowseScriptsPath(), _ => SelectedProfile != null);
        BrowseAssetsPathCommand = new RelayCommand(_ => BrowseAssetsPath(), _ => SelectedProfile != null);
        BrowseDestinationPathCommand = new RelayCommand(_ => BrowseDestinationPath(), _ => SelectedProfile != null);

        StartOrCancelBuildCommand = new RelayCommand(_ => ToggleBuild(), _ => SelectedProfile != null && SelectedProfile.IsValid);
        CopyLogsCommand = new RelayCommand(_ => CopyLogs(), _ => !string.IsNullOrEmpty(LogText));
    }

    public ObservableCollection<ProfileViewModel> Profiles { get; }

    public ProfileViewModel? SelectedProfile
    {
        get => _selectedProfile;
        set
        {
            if (_selectedProfile != value)
            {
                _selectedProfile = value;
                OnPropertyChanged();
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
            if (_isBuilding != value)
            {
                _isBuilding = value;
                OnPropertyChanged();
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
        set
        {
            if (_logText != value)
            {
                _logText = value;
                OnPropertyChanged();
            }
        }
    }

    public string StatusText
    {
        get => _statusText;
        set
        {
            if (_statusText != value)
            {
                _statusText = value;
                OnPropertyChanged();
            }
        }
    }

    public string StatusColor
    {
        get => _statusColor;
        set
        {
            if (_statusColor != value)
            {
                _statusColor = value;
                OnPropertyChanged();
            }
        }
    }

    public bool HasAnyUnsavedChanges => Profiles.Any(p => p.IsDirty);

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

    private void AddProfile()
    {
        var model = new BuildProfile
        {
            Name = "Новая настройка"
        };

        var vm = new ProfileViewModel(model);
        vm.PropertyChanged += OnProfilePropertyChanged;
        Profiles.Add(vm);
        SelectedProfile = vm;
        OnPropertyChanged(nameof(WindowTitle));
    }

    private void DeleteProfile()
    {
        if (SelectedProfile == null) return;

        var result = MessageBox.Show(
            $"Вы действительно хотите удалить настройку '{SelectedProfile.Name}'?",
            "Удаление настройки",
            MessageBoxButton.YesNo,
            MessageBoxImage.Warning
        );

        if (result == MessageBoxResult.Yes)
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

            SaveSessionToDisk();
            OnPropertyChanged(nameof(WindowTitle));
        }
    }

    private void SaveProfile()
    {
        if (SelectedProfile == null) return;

        var result = MessageBox.Show(
            $"Сохранить изменения для настройки '{SelectedProfile.Name}'?",
            "Подтверждение сохранения",
            MessageBoxButton.YesNo,
            MessageBoxImage.Question
        );

        if (result == MessageBoxResult.Yes)
        {
            SelectedProfile.ApplyToModel();
            SaveSessionToDisk();
            AppendLog($"[System] Настройка '{SelectedProfile.Name}' успешно сохранена.");
            OnPropertyChanged(nameof(WindowTitle));
        }
    }

    private void CancelProfile()
    {
        if (SelectedProfile == null) return;
        SelectedProfile.ResetFromModel();
        OnPropertyChanged(nameof(WindowTitle));
    }

    private void SaveSessionToDisk()
    {
        _sessionConfig.Profiles = Profiles.Select(p => p.Model).ToList();
        _sessionConfig.SelectedProfileId = SelectedProfile?.Id ?? string.Empty;
        SessionManager.SaveSession(_sessionConfig);
    }

    private void BrowseScriptsPath()
    {
        if (SelectedProfile == null) return;
        var folder = PromptSelectFolder("Выберите папку со скриптами", SelectedProfile.ScriptsPath);
        if (!string.IsNullOrEmpty(folder))
        {
            SelectedProfile.ScriptsPath = folder;
        }
    }

    private void BrowseAssetsPath()
    {
        if (SelectedProfile == null) return;
        var folder = PromptSelectFolder("Выберите папку с ассетами", SelectedProfile.AssetsPath);
        if (!string.IsNullOrEmpty(folder))
        {
            SelectedProfile.AssetsPath = folder;
        }
    }

    private void BrowseDestinationPath()
    {
        if (SelectedProfile == null) return;
        var folder = PromptSelectFolder("Выберите папку назначения", SelectedProfile.DestinationPath);
        if (!string.IsNullOrEmpty(folder))
        {
            SelectedProfile.DestinationPath = folder;
        }
    }

    private string? PromptSelectFolder(string title, string initialPath)
    {
        var dialog = new Microsoft.Win32.OpenFolderDialog
        {
            Title = title,
            InitialDirectory = System.IO.Directory.Exists(initialPath) ? initialPath : string.Empty
        };

        return dialog.ShowDialog() == true ? dialog.FolderName : null;
    }

    private void ToggleBuild()
    {
        if (SelectedProfile == null) return;

        if (IsBuilding)
        {
            // Прервать сборку
            IsBuilding = false;
            AppendLog("[Build] Сборка прервана пользователем.");
            return;
        }

        // Подтверждение перед стартом сборки
        var result = MessageBox.Show(
            $"Папка назначения '{SelectedProfile.DestinationPath}' будет перезаписана при сборке.\n\nПродолжить сборку для '{SelectedProfile.Name}'?",
            "Подтверждение сборки",
            MessageBoxButton.YesNo,
            MessageBoxImage.Warning
        );

        if (result == MessageBoxResult.Yes)
        {
            IsBuilding = true;
            AppendLog($"[Build] Запуск сборки настройки '{SelectedProfile.Name}'...");
            AppendLog($"[Build] Скрипты: {SelectedProfile.ScriptsPath}");
            AppendLog($"[Build] Ассеты:  {SelectedProfile.AssetsPath}");
            AppendLog($"[Build] Назначение: {SelectedProfile.DestinationPath}");
            
            // Тестовая имитация процесса
            AppendLog("[Build] Сериализация макета проекта через zzz_engine_builder_dll...");
            AppendLog("[Build] Сборка успешно завершена!");
            IsBuilding = false;
        }
    }

    private void CopyLogs()
    {
        try
        {
            Clipboard.SetText(LogText);
            AppendLog("[System] Логи скопированы в буфер обмена.");
        }
        catch
        {
            // Игнорируем ошибки буфера
        }
    }

    private void AppendLog(string message)
    {
        string timestamp = DateTime.Now.ToString("HH:mm:ss");
        LogText += $"[{timestamp}] {message}\n";
    }

    private void OnProfilePropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        if (e.PropertyName == nameof(ProfileViewModel.IsDirty) || e.PropertyName == nameof(ProfileViewModel.DisplayName))
        {
            OnPropertyChanged(nameof(WindowTitle));
            CommandManager.InvalidateRequerySuggested();
        }
    }

    public event PropertyChangedEventHandler? PropertyChanged;

    protected void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}
