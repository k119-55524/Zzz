using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Data;
using System.Windows.Input;
using assets_builder_gui.Models;
using assets_builder_gui.Services;
using assets_builder_lib;
using assets_builder_lib.Validation;

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
        public string ProjectPath { get; init; } = string.Empty;
        public eTargetPlatform TargetPlatform { get; init; }
        public string ConfigFile { get; init; } = string.Empty;
        public string BuildDirectory { get; init; } = string.Empty;
    }

    private readonly IDialogService _dialogService;
    private readonly AssetsBuilderEngine _engine;
    private readonly SessionConfig _sessionConfig;
    private BuildProfileViewModel? _selectedProfile;
    private bool _isBuilding = false;
    private string _logText = string.Empty;
    private string _statusText = "Готов";
    private string _statusColor = "#4CAF50";

    // Геометрия окна
    private double _windowWidth;
    private double _windowHeight;
    private double _windowLeft;
    private double _windowTop;
    private bool _isWindowMaximized;
    private double _presetsPanelHeight = 260;

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
        _presetsPanelHeight = _sessionConfig.PresetsPanelHeight >= 170 ? _sessionConfig.PresetsPanelHeight : 260;

        Profiles = new ObservableCollection<BuildProfileViewModel>();

        foreach (var profile in _sessionConfig.Profiles)
        {
            var vm = new BuildProfileViewModel(profile, _dialogService);
            vm.PropertyChanged += OnProfilePropertyChanged;
            Profiles.Add(vm);
        }

        ProfilesView = CollectionViewSource.GetDefaultView(Profiles);
        ProfilesView.SortDescriptions.Add(new SortDescription(nameof(BuildProfileViewModel.Name), ListSortDirection.Ascending));

        if (Profiles.Count > 0)
        {
            var savedSelected = Profiles.FirstOrDefault(p => p.Id == _sessionConfig.SelectedProfileId);
            SelectedProfile = savedSelected ?? Profiles.First();
        }

        AppendLog("Готов к работе.");

        // Команды профилей / проектов
        SaveProfileCommand = new RelayCommand(_ => SaveProfile(), _ => HasAnyUnsavedChanges && (SelectedProfile == null || SelectedProfile.IsValid));
        CancelProfileCommand = new RelayCommand(_ => CancelProfile(), _ => SelectedProfile != null && SelectedProfile.IsDirty);

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
                if (value != null)
                {
                    _sessionConfig.SelectedProfileId = value.Id;
                    SaveSessionToDisk();
                }
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

    public double PresetsPanelHeight
    {
        get => _presetsPanelHeight;
        set => SetProperty(ref _presetsPanelHeight, value);
    }

    public bool HasAnyUnsavedChanges => Profiles.Any(p => p.IsDirty);

    public string WindowTitle => HasAnyUnsavedChanges ? "Assets Builder *" : "Assets Builder";

    // Команды
    public ICommand SaveProfileCommand { get; }
    public ICommand CancelProfileCommand { get; }
    public ICommand StartOrCancelBuildCommand { get; }
    public ICommand CopyLogsCommand { get; }
    public ICommand ClearLogsCommand { get; }

    private void SaveProfile()
    {
        foreach (var p in Profiles.Where(p => p.IsDirty))
        {
            p.ApplyToModel();
        }

        ProfilesView.Refresh();

        SaveSessionToDisk();
        AppendLog("Настройки успешно сохранены на диск.");
        OnPropertyChanged(nameof(HasAnyUnsavedChanges));
        OnPropertyChanged(nameof(WindowTitle));
        CommandManager.InvalidateRequerySuggested();
    }

    private void CancelProfile()
    {
        if (SelectedProfile == null) return;
        SelectedProfile.ResetFromModel();
        ProfilesView.Refresh();
        OnPropertyChanged(nameof(HasAnyUnsavedChanges));
        OnPropertyChanged(nameof(WindowTitle));
        CommandManager.InvalidateRequerySuggested();
        AppendLog("Изменения отменены.");
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
        _sessionConfig.PresetsPanelHeight = PresetsPanelHeight;

        SessionManager.SaveSession(_sessionConfig);
    }

    private void BrowseSourcePath()
    {
        if (SelectedProfile == null) return;
        var folder = _dialogService.SelectFolder("Выберите папку проекта (содержащую project.json)", SelectedProfile.SourcePath);
        if (!string.IsNullOrEmpty(folder))
        {
            var validation = ProjectValidator.Validate(folder);
            if (!validation.IsValid)
            {
                string errSummary = string.Join("\n", validation.Errors);
                _dialogService.ShowError("Ошибка валидации структуры", $"Папка не прошла строгую валидацию:\n\n{errSummary}\n\nПуть не был применен.");
                return;
            }

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

    private static eTargetPlatform ParseTargetPlatform(string platformName)
    {
        if (Enum.TryParse<eTargetPlatform>(platformName, true, out var result))
        {
            return result;
        }
        return eTargetPlatform.Windows;
    }

    private void StartBuild()
    {
        if (SelectedProfile == null) return;
        if (IsBuilding) return;

        if (SelectedProfile.SelectedPreset == null)
        {
            AppendLog("Ошибка: не выбран пресет для сборки.");
            return;
        }

        // Выбираем таргеты: активные и с выбранным конфигом (не "None" / не пустой)
        var runnableTargets = SelectedProfile.SelectedPreset.Targets
            .Where(t => !t.IsNone)
            .Select(t => new TargetBuildSnapshot
            {
                Name = t.Name,
                ProjectPath = t.ProjectPath,
                TargetPlatform = ParseTargetPlatform(t.Platform),
                ConfigFile = t.ConfigFilePath,
                BuildDirectory = GetTargetBuildDirectory(SelectedProfile.DestinationPath, t.Name, ParseTargetPlatform(t.Platform))
            })
            .ToList();

        if (runnableTargets.Count == 0)
        {
            AppendLog("Ошибка: нет активных таргетов с назначенной конфигурацией для сборки.");
            return;
        }

        string baseDestinationPath = SelectedProfile.DestinationPath;
        string sourcePath = SelectedProfile.SourcePath;
        string presetName = SelectedProfile.SelectedPreset.Name;

        if (SelectedProfile.IsDirty)
        {
            bool saveConfirmed = _dialogService.ShowConfirmation(
                "Несохраненные изменения",
                $"В настройках сборок есть несохраненные изменения. Сохранить их на диск перед запуском сборки набора '{presetName}'?"
            );

            if (!saveConfirmed)
            {
                AppendLog("Сборка отменена: изменения не сохранены на диск.");
                return;
            }

            SelectedProfile.ApplyToModel();
            SaveSessionToDisk();
            ProfilesView.Refresh();
            OnPropertyChanged(nameof(HasAnyUnsavedChanges));
            OnPropertyChanged(nameof(WindowTitle));
            CommandManager.InvalidateRequerySuggested();
            AppendLog("Настройки сохранены перед сборкой.");
        }
        else
        {
            bool confirmed = _dialogService.ShowConfirmation(
                "Подтверждение сборки",
                $"Продолжить сборку набора '{presetName}' для проекта '{SelectedProfile.Name}'?"
            );

            if (!confirmed)
            {
                return;
            }
        }
        {
            IsBuilding = true;

            Task.Run(() =>
            {
                DateTimeOffset buildDateTime = DateTimeOffset.Now;
                ulong buildTimestampMs = (ulong)buildDateTime.ToUnixTimeMilliseconds();
                string buildTime = DateTimeOffset.FromUnixTimeMilliseconds((long)buildTimestampMs).LocalDateTime.ToString("yyyy-MM-dd HH:mm:ss:fff");
                bool success = PrepareBuildRoot(baseDestinationPath, buildTime);
                if (success)
                {
                    // 1. Предварительное сканирование мета-файлов
                    var firstTarget = runnableTargets.First();
                    var commonOptions = new BuildOptions
                    {
                        SourcePath = sourcePath,
                        DestinationPath = baseDestinationPath,
                        TargetProjectName = firstTarget.Name,
                        TargetPlatform = firstTarget.TargetPlatform,
                        PlatformConfigFile = firstTarget.ConfigFile
                    };

                    bool metaValid = _engine.ScanProjectMetaFiles(commonOptions);
                    if (!metaValid)
                    {
                        AppendLog("Ошибка: Сборка отменена из-за ошибок валидации или GUID.");
                        success = false;
                    }
                }

                // 2. Сборка индивидуального пакета для каждого таргета пресета
                if (success)
                {
                    foreach (var target in runnableTargets)
                    {
                        Directory.CreateDirectory(target.BuildDirectory);
                        string targetIncludeDir = Path.Combine(target.BuildDirectory, "include");
                        Directory.CreateDirectory(targetIncludeDir);
                        string targetAssetsDir = Path.Combine(target.BuildDirectory, "assets");
                        Directory.CreateDirectory(targetAssetsDir);

                        _engine.CopyHeaderFiles(sourcePath, targetIncludeDir, target.ConfigFile);
                        _engine.GenerateScriptsCmake(sourcePath, target.BuildDirectory, target.ConfigFile);

                        AppendLog($"Сериализация пакета для '{target.Name}' ({target.TargetPlatform}, конфиг: '{target.ConfigFile}')...");
                        bool packSuccess = PackagePacker.PackProject(sourcePath, target.BuildDirectory, target.TargetPlatform, buildTimestampMs, out _, target.ConfigFile, AppendLog);
                        if (!packSuccess)
                        {
                            AppendLog($"Ошибка упаковки для таргета '{target.Name}'!");
                            success = false;
                            break;
                        }
                    }
                }

                if (success)
                {
                    UpdateTargetProjectsConfig(runnableTargets, baseDestinationPath, buildTime);
                    AppendLog("Сборка всех выбранных целевых проектов завершена успешно!");
                }
                System.Windows.Application.Current?.Dispatcher.Invoke(() => IsBuilding = false);
            });
        }
    }

    public void BuildHeadless()
    {
        _engine.LogReceived += msg => Console.WriteLine(msg);

        if (SelectedProfile == null)
        {
            Console.WriteLine("Ошибка: Профиль не выбран.");
            return;
        }

        if (!SelectedProfile.IsValid)
        {
            Console.WriteLine($"Ошибка: Профиль не валиден. IsSourceValid: {SelectedProfile.IsSourcePathValid}, IsDestValid: {SelectedProfile.IsDestinationPathValid}, IsProjectValid: {SelectedProfile.IsProjectValid}, HasRunnableTargets: {SelectedProfile.HasRunnableTargets}");
            if (!SelectedProfile.IsProjectValid)
            {
                Console.WriteLine($"Ошибки валидации проекта:\n{SelectedProfile.ValidationErrorsSummary}");
            }
            return;
        }

        if (SelectedProfile.SelectedPreset == null)
        {
            Console.WriteLine("Ошибка: Набор сборки (пресет) не выбран.");
            return;
        }

        string sourcePath = SelectedProfile.SourcePath;
        string baseDestinationPath = SelectedProfile.DestinationPath;

        var runnableTargets = SelectedProfile.SelectedPreset.Targets
            .Where(t => !t.IsNone)
            .Select(t => new TargetBuildSnapshot
            {
                Name = t.Name,
                ProjectPath = t.ProjectPath,
                TargetPlatform = ParseTargetPlatform(t.Platform),
                ConfigFile = t.ConfigFilePath,
                BuildDirectory = GetTargetBuildDirectory(baseDestinationPath, t.Name, ParseTargetPlatform(t.Platform))
            })
            .ToList();

        if (runnableTargets.Count == 0)
        {
            Console.WriteLine("Предупреждение: Нет активных целевых проектов для сборки.");
            return;
        }

        DateTimeOffset buildDateTime = DateTimeOffset.Now;
        ulong buildTimestampMs = (ulong)buildDateTime.ToUnixTimeMilliseconds();
        string buildTime = DateTimeOffset.FromUnixTimeMilliseconds((long)buildTimestampMs).LocalDateTime.ToString("yyyy-MM-dd HH:mm:ss:fff");
        bool success = PrepareBuildRoot(baseDestinationPath, buildTime);
        if (success)
        {
            var firstTarget = runnableTargets.First();
            var commonOptions = new BuildOptions
            {
                SourcePath = sourcePath,
                DestinationPath = baseDestinationPath,
                TargetProjectName = firstTarget.Name,
                TargetPlatform = firstTarget.TargetPlatform,
                PlatformConfigFile = firstTarget.ConfigFile
            };

            bool metaValid = _engine.ScanProjectMetaFiles(commonOptions);
            if (!metaValid)
            {
                Console.WriteLine("Ошибка: Сборка отменена из-за ошибок валидации или GUID.");
                success = false;
            }
        }

        if (success)
        {
            foreach (var target in runnableTargets)
            {
                Directory.CreateDirectory(target.BuildDirectory);
                string targetIncludeDir = Path.Combine(target.BuildDirectory, "include");
                Directory.CreateDirectory(targetIncludeDir);
                string targetAssetsDir = Path.Combine(target.BuildDirectory, "assets");
                Directory.CreateDirectory(targetAssetsDir);

                _engine.CopyHeaderFiles(sourcePath, targetIncludeDir, target.ConfigFile);
                _engine.GenerateScriptsCmake(sourcePath, target.BuildDirectory, target.ConfigFile);

                Console.WriteLine($"Сериализация индивидуального пакета для '{target.Name}' ({target.TargetPlatform}, конфиг: '{target.ConfigFile}')...");
                bool packSuccess = PackagePacker.PackProject(sourcePath, target.BuildDirectory, target.TargetPlatform, buildTimestampMs, out _, target.ConfigFile, msg => Console.WriteLine(msg));
                if (!packSuccess)
                {
                    Console.WriteLine($"Ошибка упаковки для таргета '{target.Name}'!");
                    success = false;
                    break;
                }
            }
        }

        if (success)
        {
            UpdateTargetProjectsConfig(runnableTargets, baseDestinationPath, buildTime);
            Console.WriteLine("Headless build completed successfully.");
        }
    }

    private bool PrepareBuildRoot(string destinationPath, string buildTime)
    {
        try
        {
            if (Directory.Exists(destinationPath))
            {
                AppendLog($"Очистка папки назначения: {destinationPath}");
                Directory.Delete(destinationPath, recursive: true);
            }

            Directory.CreateDirectory(destinationPath);
            File.WriteAllText(Path.Combine(destinationPath, "buildtime-data.txt"), buildTime);
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

    private void UpdateTargetProjectsConfig(IEnumerable<TargetBuildSnapshot> targets, string baseDestinationPath, string buildTime)
    {
        // Динамическое определение папки целевых проектов (src/projects) относительно окружения/сессии
        string workspaceProjects = SessionManager.ResolveWorkspaceProjectsPath(_sessionConfig.WorkspaceProjectsPath);

        foreach (var target in targets)
        {
            try
            {
                string targetDir = !string.IsNullOrWhiteSpace(target.ProjectPath) && Directory.Exists(target.ProjectPath)
                    ? target.ProjectPath
                    : Path.Combine(workspaceProjects, target.Name);

                if (!Directory.Exists(targetDir))
                {
                    AppendLog($"Предупреждение: Целевой проект '{target.Name}' не найден по пути '{targetDir}'. Запись assets_config.json пропущена.");
                    continue;
                }

                string jsonPath = Path.Combine(targetDir, "assets_config.json");

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
                                if (!string.IsNullOrEmpty(str) && Directory.Exists(str) && !IsSameOrInsideDirectory(str, baseDestinationPath))
                                {
                                    activeBuilds.Add(str);
                                }
                            }
                        }
                    }
                    catch { }
                }

                activeBuilds.Add(target.BuildDirectory);

                // Единое время сборки, совпадающее с buildtime-data.txt и заголовками package.dat/data.dat
                var jsonObj = new { active_build_directories = activeBuilds, last_build_time = buildTime };
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
        string color = "#CDD9E5";
        string weight = "Normal";

        if (message.Contains("Ошибка", StringComparison.OrdinalIgnoreCase) || message.Contains("error", StringComparison.OrdinalIgnoreCase))
        {
            color = "#FF5252";
            weight = "Bold";
        }
        else if (message.Contains("Предупреждение", StringComparison.OrdinalIgnoreCase) || message.Contains("warning", StringComparison.OrdinalIgnoreCase))
        {
            color = "#FFC107";
            weight = "SemiBold";
        }
        else if (message.Contains("успешно", StringComparison.OrdinalIgnoreCase))
        {
            color = "#4CAF50";
            weight = "SemiBold";
        }

        var app = System.Windows.Application.Current;
        if (app != null && app.Dispatcher != null && !app.Dispatcher.HasShutdownStarted)
        {
            if (app.Dispatcher.CheckAccess())
            {
                LogItems.Add(new LogItem
                {
                    Timestamp = timestamp,
                    Message = message,
                    Color = color,
                    FontWeight = weight
                });
                LogText += $"[{timestamp}] {message}\n";
            }
            else
            {
                app.Dispatcher.BeginInvoke(() =>
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
        }
    }

    public bool ConfirmWindowClose()
    {
        if (HasAnyUnsavedChanges)
        {
            var choice = _dialogService.ShowSaveOnCloseConfirmation(
                "Несохраненные изменения",
                "В настройках сборок есть несохраненные изменения. Сохранить их перед выходом?"
            );

            if (choice == SaveCloseChoice.Save)
            {
                foreach (var p in Profiles.Where(p => p.IsDirty))
                {
                    p.ApplyToModel();
                }
                ProfilesView.Refresh();
                SaveSessionToDisk();
                return true;
            }
            else if (choice == SaveCloseChoice.DontSave)
            {
                return true;
            }
            else
            {
                return false;
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
            ProfilesView.Refresh();
            CommandManager.InvalidateRequerySuggested();
        }
        else if (e.PropertyName == nameof(BuildProfileViewModel.IsValid) ||
                 e.PropertyName == nameof(BuildProfileViewModel.HasRunnableTargets))
        {
            CommandManager.InvalidateRequerySuggested();
        }
    }
}
