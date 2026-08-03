using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Windows.Data;
using System.Windows.Input;
using assets_builder_gui.Models;
using assets_builder_gui.Services;
using assets_builder_lib;

namespace assets_builder_gui.ViewModels;

public class LogItem
{
    public string Timestamp { get; set; } = string.Empty;
    public string Message { get; set; } = string.Empty;
    public string Color { get; set; } = "#CDD9E5";
    public string FontWeight { get; set; } = "Normal";
    public string FullText => $"[{Timestamp}] {Message}";
}

public class MainWindowViewModel : ViewModelBase
{
    private sealed class TargetBuildSnapshot
    {
        public string Name { get; init; } = string.Empty;
        public eTargetPlatform TargetPlatform { get; init; }
        public string ConfigJsonPath { get; init; } = string.Empty;
        public string BuildDirectory { get; init; } = string.Empty;
    }

    private readonly IDialogService _dialogService;
    private readonly AssetsBuilderEngine _engine;
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
        _engine = new AssetsBuilderEngine();

        // Подписка на стрим логов от ядра сборщика
        _engine.LogReceived += msg =>
        {
            AppendLog(msg);
            if (IsBuilding && !string.IsNullOrWhiteSpace(msg))
            {
                string cleanMsg = msg.Trim();
                if (cleanMsg.StartsWith("Старт сборки") || cleanMsg.StartsWith("Сканирование") || 
                    cleanMsg.StartsWith("Упаковка") || cleanMsg.StartsWith("Очистка") || 
                    cleanMsg.StartsWith("Сгенерирован") || cleanMsg.StartsWith("Копирование"))
                {
                    StatusText = cleanMsg;
                }
            }
        };

        _sessionConfig = SessionManager.LoadSession();

        _windowWidth = _sessionConfig.WindowWidth;
        _windowHeight = _sessionConfig.WindowHeight;
        _windowLeft = _sessionConfig.WindowLeft;
        _windowTop = _sessionConfig.WindowTop;
        _isWindowMaximized = _sessionConfig.IsWindowMaximized;

        Profiles = new ObservableCollection<BuildProfileViewModel>();

        foreach (var profile in _sessionConfig.Profiles)
        {
            var vm = new BuildProfileViewModel(profile, _dialogService);
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

        BrowseSourcePathCommand = new RelayCommand(_ => BrowseSourcePath(), _ => SelectedProfile != null);
        BrowseDestinationPathCommand = new RelayCommand(_ => BrowseDestinationPath(), _ => SelectedProfile != null);

        StartOrCancelBuildCommand = new RelayCommand(_ => StartBuild(), _ => SelectedProfile != null && SelectedProfile.IsValid && !IsBuilding);
        CopyLogsCommand = new RelayCommand(_ => CopyLogs(), _ => LogItems.Count > 0);
        ClearLogsCommand = new RelayCommand(_ => ClearLogs(), _ => LogItems.Count > 0);
    }

    public ObservableCollection<BuildProfileViewModel> Profiles { get; }
    public ObservableCollection<LogItem> LogItems { get; } = new();

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
                StatusText = _isBuilding ? "Идет сборка..." : "Готов";
                StatusColor = _isBuilding ? "#FFC107" : "#4CAF50";
                CommandManager.InvalidateRequerySuggested();
            }
        }
    }

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

    public ICommand BrowseSourcePathCommand { get; }
    public ICommand BrowseDestinationPathCommand { get; }

    public ICommand StartOrCancelBuildCommand { get; }
    public ICommand CopyLogsCommand { get; }
    public ICommand ClearLogsCommand { get; }

    private void AddProfile()
    {
        var model = new BuildProfile 
        { 
            Name = "Новая настройка",
            TargetProjects = new List<TargetProjectItem>()
        };
        var vm = new BuildProfileViewModel(model, _dialogService);
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

    private void BrowseSourcePath()
    {
        if (SelectedProfile == null) return;
        var folder = _dialogService.SelectFolder("Выберите папку проекта (содержащую project.json)", SelectedProfile.SourcePath);
        if (!string.IsNullOrEmpty(folder))
        {
            SelectedProfile.SourcePath = folder;
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

    private void StartBuild()
    {
        if (SelectedProfile == null) return;
        if (IsBuilding) return;

        var enabledTargets = SelectedProfile.TargetProjects
            .Where(t => t.IsEnabled)
            .Select(t => new TargetBuildSnapshot
            {
                Name = t.Name,
                TargetPlatform = t.TargetPlatform,
                ConfigJsonPath = t.ConfigJsonPath,
                BuildDirectory = GetTargetBuildDirectory(SelectedProfile.DestinationPath, t.Name, t.TargetPlatform)
            })
            .ToList();

        if (enabledTargets.Count == 0)
        {
            AppendLog("Ошибка: нет включенных целевых проектов для сборки.");
            return;
        }

        string baseDestinationPath = SelectedProfile.DestinationPath;
        string sourcePath = SelectedProfile.SourcePath;

        bool confirmed = _dialogService.ShowConfirmation(
            "Подтверждение сборки",
            $"Папка назначения '{SelectedProfile.DestinationPath}' будет вычищена и перезаписана.\n\nПродолжить сборку ассетов для '{SelectedProfile.Name}'?"
        );

        if (confirmed)
        {
            IsBuilding = true;

            Task.Run(() =>
            {
                bool success = PrepareBuildRoot(baseDestinationPath);
                foreach (var target in enabledTargets)
                {
                    if (!success)
                    {
                        break;
                    }

                    var options = new BuildOptions
                    {
                        SourcePath = sourcePath,
                        DestinationPath = target.BuildDirectory,
                        TargetProjectName = target.Name,
                        TargetPlatform = target.TargetPlatform
                    };

                    success = _engine.BuildPackage(options);
                }

                if (success)
                {
                    UpdateTargetProjectsConfig(enabledTargets, baseDestinationPath);
                }
                System.Windows.Application.Current?.Dispatcher.Invoke(() => IsBuilding = false);
            });
        }
    }

    private bool PrepareBuildRoot(string destinationPath)
    {
        try
        {
            if (Directory.Exists(destinationPath))
            {
                AppendLog($"Очистка папки назначения: {destinationPath}");
                Directory.Delete(destinationPath, recursive: true);
            }

            Directory.CreateDirectory(destinationPath);
            return true;
        }
        catch (Exception ex)
        {
            AppendLog($"Ошибка очистки папки назначения: {ex.Message}");
            return false;
        }
    }

    private static string GetTargetBuildDirectory(string destinationPath, string targetName, eTargetPlatform targetPlatform)
    {
        string safeName = SanitizeDirectoryName(targetName);
        if (string.IsNullOrWhiteSpace(safeName))
        {
            safeName = "target";
        }

        return Path.Combine(destinationPath, $"{safeName}_{targetPlatform}");
    }

    private static string SanitizeDirectoryName(string value)
    {
        char[] invalidChars = Path.GetInvalidFileNameChars();
        string trimmed = value.Trim();
        return new string(trimmed.Select(ch => invalidChars.Contains(ch) ? '_' : ch).ToArray());
    }

    private static bool IsSameOrInsideDirectory(string path, string directory)
    {
        try
        {
            string fullPath = Path.GetFullPath(path).TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
            string fullDirectory = Path.GetFullPath(directory).TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);
            return fullPath.Equals(fullDirectory, StringComparison.OrdinalIgnoreCase) ||
                   fullPath.StartsWith(fullDirectory + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase) ||
                   fullPath.StartsWith(fullDirectory + Path.AltDirectorySeparatorChar, StringComparison.OrdinalIgnoreCase);
        }
        catch
        {
            return false;
        }
    }

    private void UpdateTargetProjectsConfig(IEnumerable<TargetBuildSnapshot> targets, string baseDestinationPath)
    {
        foreach (var target in targets.Where(t => !string.IsNullOrWhiteSpace(t.ConfigJsonPath)))
        {
            try
            {
                string jsonPath = target.ConfigJsonPath;
                string dir = Path.GetDirectoryName(jsonPath)!;
                if (!Directory.Exists(dir))
                {
                    Directory.CreateDirectory(dir);
                }

                List<string> activeBuilds = new();
                if (File.Exists(jsonPath))
                {
                    try
                    {
                        string existingJson = File.ReadAllText(jsonPath);
                        using var doc = System.Text.Json.JsonDocument.Parse(existingJson);
                        if (doc.RootElement.TryGetProperty("active_build_directories", out var arr))
                        {
                            foreach (var elem in arr.EnumerateArray())
                            {
                                string str = elem.GetString() ?? "";
                                if (!string.IsNullOrEmpty(str) && !IsSameOrInsideDirectory(str, baseDestinationPath))
                                {
                                    activeBuilds.Add(str);
                                }
                            }
                        }
                    }
                    catch { }
                }

                activeBuilds.Add(target.BuildDirectory);

                var jsonObj = new { active_build_directories = activeBuilds };
                string outputJson = System.Text.Json.JsonSerializer.Serialize(jsonObj, new System.Text.Json.JsonSerializerOptions { WriteIndented = true });
                File.WriteAllText(jsonPath, outputJson);
                AppendLog($"Обновлен конфиг целевого проекта ({target.Name}): {jsonPath}");
            }
            catch (Exception ex)
            {
                AppendLog($"Ошибка записи конфига целевого проекта ({target.Name}): {ex.Message}");
            }
        }
    }

    private void CopyLogs()
    {
        string fullLog = string.Join("\n", LogItems.Select(item => item.FullText));
        _dialogService.CopyToClipboard(fullLog);
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
            LogItems.Clear();
            LogText = string.Empty;
            AppendLog("Логи очищены.");
        }
    }

    public void AppendLog(string message)
    {
        string timestamp = DateTime.Now.ToString("HH:mm:ss");
        string color = "#CDD9E5"; // Стандартный нейтральный светлый цвет
        string weight = "Normal";

        if (message.Contains("Ошибка", StringComparison.OrdinalIgnoreCase) || message.Contains("error", StringComparison.OrdinalIgnoreCase))
        {
            color = "#FF5252"; // Красный цвет ошибок
            weight = "Bold";
        }
        else if (message.Contains("Предупреждение", StringComparison.OrdinalIgnoreCase) || message.Contains("warning", StringComparison.OrdinalIgnoreCase))
        {
            color = "#FFC107"; // Яркий желтый цвет предупреждений
            weight = "SemiBold";
        }
        else if (message.Contains("успешно", StringComparison.OrdinalIgnoreCase))
        {
            color = "#4CAF50"; // Зеленый цвет успеха
            weight = "SemiBold";
        }

        System.Windows.Application.Current?.Dispatcher.Invoke(() =>
        {
            LogItems.Add(new LogItem
            {
                Timestamp = timestamp,
                Message = message,
                Color = color,
                FontWeight = weight
            });
            LogText += $"[{timestamp}] {message}\n";
        });
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
