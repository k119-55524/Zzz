using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Windows.Input;
using assets_builder_gui.Models;
using assets_builder_gui.Services;
using assets_builder_lib;
using assets_builder_lib.Models;
using assets_builder_lib.Validation;

namespace assets_builder_gui.ViewModels;

public class TargetConfigViewModel : ViewModelBase
{
    public static readonly IReadOnlyList<string> AvailablePlatforms = new[]
    {
        "Windows",
        "Android",
        "iOS",
        "MacOS",
        "Linux"
    };

    private readonly BuildPresetTarget _model;
    private readonly string _assetsProjectPath;
    private readonly Services.IDialogService? _dialogService;

    private string _name;
    private string _projectPath;
    private string _platform;
    private PlatformConfigOption? _selectedConfigOption;

    public TargetConfigViewModel(BuildPresetTarget model, string assetsProjectPath, Services.IDialogService? dialogService)
    {
        _model = model;
        _assetsProjectPath = assetsProjectPath;
        _dialogService = dialogService;

        _name = model.Name;
        _platform = model.Platform;

        if (string.IsNullOrWhiteSpace(model.ProjectPath))
        {
            string defaultPath = Path.Combine(SessionManager.ResolveWorkspaceProjectsPath(), model.Name);
            _projectPath = Directory.Exists(defaultPath) ? defaultPath : string.Empty;
            _model.ProjectPath = _projectPath;
        }
        else
        {
            _projectPath = model.ProjectPath;
        }

        BrowseProjectPathCommand = new RelayCommand(_ =>
        {
            string? folder = _dialogService?.SelectFolder("Выберите папку целевого проекта", _projectPath);
            if (!string.IsNullOrEmpty(folder))
            {
                ProjectPath = folder;
            }
        });

        AvailableConfigOptions = new ObservableCollection<PlatformConfigOption>();
        UpdateAvailableConfigs();
    }

    public BuildPresetTarget Model => _model;

    public ICommand BrowseProjectPathCommand { get; }

    public IReadOnlyList<string> PlatformList => AvailablePlatforms;

    public string Name
    {
        get => _name;
        set => SetProperty(ref _name, value);
    }

    public string ProjectPath
    {
        get => _projectPath;
        set
        {
            if (SetProperty(ref _projectPath, value))
            {
                OnPropertyChanged(nameof(IsDirty));
            }
        }
    }

    public string Platform
    {
        get => AvailablePlatforms.FirstOrDefault(p => p.Equals(_platform, StringComparison.OrdinalIgnoreCase)) ?? _platform;
        set
        {
            if (SetProperty(ref _platform, value))
            {
                UpdateAvailableConfigs();
                OnPropertyChanged(nameof(IsDirty));
            }
        }
    }

    public bool IsEnabled => !IsNone;

    public ObservableCollection<PlatformConfigOption> AvailableConfigOptions { get; }

    public PlatformConfigOption? SelectedConfigOption
    {
        get => _selectedConfigOption;
        set
        {
            if (SetProperty(ref _selectedConfigOption, value))
            {
                OnPropertyChanged(nameof(ConfigFilePath));
                OnPropertyChanged(nameof(IsNone));
                OnPropertyChanged(nameof(IsEnabled));
                OnPropertyChanged(nameof(IsDirty));
            }
        }
    }

    public string ConfigFilePath => SelectedConfigOption?.RelativePath ?? string.Empty;

    public bool IsNone => string.IsNullOrWhiteSpace(ConfigFilePath);

    public bool IsDirty =>
        _projectPath != (_model.ProjectPath ?? string.Empty) ||
        !string.Equals(_platform, _model.Platform, StringComparison.OrdinalIgnoreCase) ||
        !string.Equals(ConfigFilePath, _model.ConfigFile?.Replace('\\', '/'), StringComparison.OrdinalIgnoreCase);

    private void UpdateAvailableConfigs()
    {
        var options = BuildPresetManager.GetAvailablePlatformConfigs(_assetsProjectPath, _platform);
        AvailableConfigOptions.Clear();
        foreach (var opt in options)
        {
            AvailableConfigOptions.Add(opt);
        }

        string targetRel = _selectedConfigOption?.RelativePath ?? _model.ConfigFile?.Replace('\\', '/') ?? string.Empty;
        SelectedConfigOption = AvailableConfigOptions.FirstOrDefault(o =>
            !string.IsNullOrWhiteSpace(o.RelativePath) &&
            o.RelativePath.Equals(targetRel, StringComparison.OrdinalIgnoreCase))
            ?? AvailableConfigOptions.FirstOrDefault(o => string.IsNullOrWhiteSpace(o.RelativePath))
            ?? AvailableConfigOptions.FirstOrDefault();
    }

    public void ApplyToModel()
    {
        _model.Name = _name;
        _model.ProjectPath = _projectPath;
        _model.Platform = _platform;
        _model.ConfigFile = ConfigFilePath;
        OnPropertyChanged(nameof(IsDirty));
    }

    public void ResetFromModel()
    {
        Name = _model.Name;
        _platform = _model.Platform;
        OnPropertyChanged(nameof(Platform));
        _projectPath = _model.ProjectPath ?? string.Empty;
        OnPropertyChanged(nameof(ProjectPath));
        UpdateAvailableConfigs();
        OnPropertyChanged(nameof(IsEnabled));
        OnPropertyChanged(nameof(IsDirty));
    }
}

public class BuildPresetViewModel : ViewModelBase
{
    private readonly BuildPreset _model;
    private readonly string _projectPath;
    private readonly Services.IDialogService? _dialogService;

    public BuildPresetViewModel(BuildPreset model, string projectPath, Services.IDialogService? dialogService = null)
    {
        _model = model;
        _projectPath = projectPath;
        _dialogService = dialogService;
        Targets = new ObservableCollection<TargetConfigViewModel>();

        LoadTargets();
    }

    public BuildPreset Model => _model;

    public string Name => _model.Name;

    public string Description => _model.Description;

    public ObservableCollection<TargetConfigViewModel> Targets { get; }

    public bool IsDirty => Targets.Any(t => t.IsDirty);

    private void LoadTargets()
    {
        Targets.Clear();
        foreach (var target in _model.Targets)
        {
            var vm = new TargetConfigViewModel(target, _projectPath, _dialogService);
            vm.PropertyChanged += (s, e) =>
            {
                if (e.PropertyName == nameof(TargetConfigViewModel.IsDirty))
                {
                    OnPropertyChanged(nameof(IsDirty));
                }
            };
            Targets.Add(vm);
        }
    }

    public void ApplyToModel()
    {
        foreach (var target in Targets)
        {
            target.ApplyToModel();
        }
        OnPropertyChanged(nameof(IsDirty));
    }

    public void ResetFromModel()
    {
        foreach (var target in Targets)
        {
            target.ResetFromModel();
        }
        OnPropertyChanged(nameof(IsDirty));
    }
}

public class BuildProfileViewModel : ViewModelBase
{
    private readonly BuildProfile _model;
    private readonly Services.IDialogService? _dialogService;

    private string _name = string.Empty;
    private string _sourcePath = string.Empty;
    private string _destinationPath = string.Empty;
    private string _activePresetName = "Default";

    private ProjectValidationResult? _validationResult;
    private PresetsContainer? _presetsContainer;
    private BuildPresetViewModel? _selectedPreset;

    public ObservableCollection<BuildPresetViewModel> Presets { get; } = new();

    public BuildProfileViewModel(BuildProfile model, Services.IDialogService? dialogService = null)
    {
        _model = model;
        _dialogService = dialogService;

        ResetFromModel();
    }

    public BuildProfile Model => _model;

    public string Id => _model.Id;

    public string Name
    {
        get => _name;
        set
        {
            if (value.Length > 255)
                value = value.Substring(0, 255);
            if (SetProperty(ref _name, value))
            {
                OnPropertyChanged(nameof(DisplayName));
                OnPropertyChanged(nameof(IsDirty));
                OnPropertyChanged(nameof(IsValid));
            }
        }
    }

    public string SourcePath
    {
        get => _sourcePath;
        set
        {
            if (SetProperty(ref _sourcePath, value))
            {
                if (!string.IsNullOrEmpty(value))
                {
                    DestinationPath = SessionManager.GetDefaultBuildPath();
                }
                ReloadProjectAndPresets();
                OnPropertyChanged(nameof(IsSourcePathValid));
                OnPropertyChanged(nameof(IsProjectValid));
                OnPropertyChanged(nameof(ValidationErrorsSummary));
                OnPropertyChanged(nameof(DisplayName));
                OnPropertyChanged(nameof(IsDirty));
                OnPropertyChanged(nameof(IsValid));
            }
        }
    }

    public string DestinationPath
    {
        get => _destinationPath;
        set
        {
            if (SetProperty(ref _destinationPath, value))
            {
                OnPropertyChanged(nameof(IsDestinationPathValid));
                OnPropertyChanged(nameof(DisplayName));
                OnPropertyChanged(nameof(IsDirty));
                OnPropertyChanged(nameof(IsValid));
            }
        }
    }

    public string ActivePresetName
    {
        get => _activePresetName;
        set
        {
            if (SetProperty(ref _activePresetName, value))
            {
                var match = Presets.FirstOrDefault(p => p.Name.Equals(value, StringComparison.OrdinalIgnoreCase));
                if (match != null && match != SelectedPreset)
                {
                    SelectedPreset = match;
                }
                OnPropertyChanged(nameof(IsDirty));
            }
        }
    }

    public BuildPresetViewModel? SelectedPreset
    {
        get => _selectedPreset;
        set
        {
            if (SetProperty(ref _selectedPreset, value))
            {
                if (value != null)
                {
                    _activePresetName = value.Name;
                    _model.ActivePresetName = value.Name;
                    OnPropertyChanged(nameof(ActivePresetName));
                }
                OnPropertyChanged(nameof(IsDirty));
                OnPropertyChanged(nameof(IsValid));
            }
        }
    }

    public ProjectValidationResult? ValidationResult
    {
        get => _validationResult;
        private set
        {
            if (SetProperty(ref _validationResult, value))
            {
                OnPropertyChanged(nameof(IsProjectValid));
                OnPropertyChanged(nameof(ValidationErrorsSummary));
                OnPropertyChanged(nameof(ProjectDescription));
                OnPropertyChanged(nameof(AppName));
            }
        }
    }

    public string ProjectDescription => _validationResult?.Manifest?.Description ?? string.Empty;

    public string AppName => _validationResult?.Manifest?.AppName ?? string.Empty;

    public bool IsProjectValid => _validationResult != null && _validationResult.IsValid;

    public string ValidationErrorsSummary
    {
        get
        {
            if (_validationResult == null) return string.Empty;
            if (_validationResult.IsValid) return string.Empty;
            return string.Join("\n", _validationResult.Errors);
        }
    }

    // Источниковый путь к папке проекта должен существовать на диске
    public bool IsSourcePathValid => !string.IsNullOrWhiteSpace(_sourcePath) && Directory.Exists(_sourcePath);

    // Валидация пути назначения: синтаксис и имя пути
    public bool IsDestinationPathValid
    {
        get
        {
            if (string.IsNullOrWhiteSpace(_destinationPath))
                return false;

            try
            {
                string fullPath = Path.GetFullPath(_destinationPath);
                char[] invalidChars = Path.GetInvalidPathChars();
                if (_destinationPath.IndexOfAny(invalidChars) >= 0)
                    return false;

                return true;
            }
            catch
            {
                return false;
            }
        }
    }

    public bool HasRunnableTargets
    {
        get
        {
            if (SelectedPreset == null) return false;
            return SelectedPreset.Targets.Any(t => !t.IsNone);
        }
    }

    public bool IsValid => !string.IsNullOrWhiteSpace(_name) &&
                           IsSourcePathValid &&
                           IsDestinationPathValid &&
                           IsProjectValid &&
                           HasRunnableTargets;

    public bool IsDirty => _name != _model.Name ||
                           _sourcePath != _model.SourcePath ||
                           _destinationPath != _model.DestinationPath ||
                           _activePresetName != _model.ActivePresetName ||
                           Presets.Any(p => p.IsDirty);

    public string DisplayName => IsDirty ? $"{_name} *" : _name;

    public void ReloadProjectAndPresets()
    {
        Presets.Clear();
        _selectedPreset = null;

        if (!IsSourcePathValid)
        {
            ValidationResult = null;
            return;
        }

        // Строгая валидация структуры
        ValidationResult = ProjectValidator.Validate(_sourcePath);

        if (!ValidationResult.IsValid)
        {
            return;
        }

        string presetsRelPath = ValidationResult.Manifest?.BuildSettings?.PresetsFile ?? "build_settings/presets.json";
        _presetsContainer = BuildPresetManager.LoadPresets(_sourcePath, presetsRelPath);

        if (_presetsContainer != null)
        {
            foreach (var preset in _presetsContainer.Presets)
            {
                var pvm = new BuildPresetViewModel(preset, _sourcePath, _dialogService);
                pvm.PropertyChanged += (s, e) =>
                {
                    if (e.PropertyName == nameof(BuildPresetViewModel.IsDirty))
                    {
                        OnPropertyChanged(nameof(IsDirty));
                        OnPropertyChanged(nameof(DisplayName));
                        OnPropertyChanged(nameof(IsValid));
                    }
                };
                Presets.Add(pvm);
            }

            var active = Presets.FirstOrDefault(p => p.Name.Equals(_activePresetName, StringComparison.OrdinalIgnoreCase))
                         ?? Presets.FirstOrDefault();
            SelectedPreset = active;
        }
    }

    public void ResetFromModel()
    {
        _name = _model.Name;
        _sourcePath = _model.SourcePath;
        _destinationPath = _model.DestinationPath;
        _activePresetName = string.IsNullOrWhiteSpace(_model.ActivePresetName) ? "Default" : _model.ActivePresetName;

        ReloadProjectAndPresets();

        OnPropertyChanged(nameof(Name));
        OnPropertyChanged(nameof(SourcePath));
        OnPropertyChanged(nameof(DestinationPath));
        OnPropertyChanged(nameof(ActivePresetName));
        OnPropertyChanged(nameof(DisplayName));
        OnPropertyChanged(nameof(IsSourcePathValid));
        OnPropertyChanged(nameof(IsDestinationPathValid));
        OnPropertyChanged(nameof(IsDirty));
        OnPropertyChanged(nameof(IsValid));
    }

    public void ApplyToModel()
    {
        _model.Name = _name;
        _model.SourcePath = _sourcePath;
        _model.DestinationPath = _destinationPath;
        _model.ActivePresetName = _activePresetName;

        // Сохранение изменений в пресетах проекта (в build_settings/presets.json)
        if (IsProjectValid && _presetsContainer != null)
        {
            foreach (var p in Presets)
            {
                p.ApplyToModel();
            }

            string presetsRelPath = ValidationResult?.Manifest?.BuildSettings?.PresetsFile ?? "build_settings/presets.json";
            BuildPresetManager.SavePresets(_sourcePath, presetsRelPath, _presetsContainer);

            // Сохранение активного пресета в project.json
            string projectJsonPath = Path.Combine(_sourcePath, "project.json");
            if (File.Exists(projectJsonPath) && ValidationResult?.Manifest != null)
            {
                try
                {
                    ValidationResult.Manifest.BuildSettings ??= new BuildSettingsInfo();
                    ValidationResult.Manifest.BuildSettings.ActivePreset = _activePresetName;
                    string json = System.Text.Json.JsonSerializer.Serialize(ValidationResult.Manifest, new System.Text.Json.JsonSerializerOptions { WriteIndented = true });
                    File.WriteAllText(projectJsonPath, json);
                }
                catch { }
            }
        }

        OnPropertyChanged(nameof(DisplayName));
        OnPropertyChanged(nameof(IsDirty));
    }
}
