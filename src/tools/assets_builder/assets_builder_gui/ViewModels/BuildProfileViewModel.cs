using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using assets_builder_gui.Models;
using assets_builder_lib;

namespace assets_builder_gui.ViewModels;

public class TargetProjectViewModel : ViewModelBase
{
    private readonly Services.IDialogService? _dialogService;
    private bool _isEnabled;
    private string _name;
    private eTargetPlatform _targetPlatform;
    private string _configJsonPath;

    public TargetProjectViewModel(TargetProjectItem model, Services.IDialogService? dialogService = null)
    {
        Model = model;
        _dialogService = dialogService;
        _isEnabled = model.IsEnabled;
        _name = model.Name;
        _targetPlatform = model.TargetPlatform;
        _configJsonPath = model.ConfigJsonPath;

        BrowseConfigJsonPathCommand = new RelayCommand(_ => BrowseConfigJsonPath());
    }

    public TargetProjectItem Model { get; }

    public static List<eTargetPlatform> AvailableTargetPlatforms { get; } = new()
    {
        eTargetPlatform.Windows,
        eTargetPlatform.Linux,
        eTargetPlatform.Android,
        eTargetPlatform.MacOS,
        eTargetPlatform.iOS
    };

    public System.Windows.Input.ICommand BrowseConfigJsonPathCommand { get; }

    private void BrowseConfigJsonPath()
    {
        string currentFolder = string.IsNullOrWhiteSpace(_configJsonPath) ? string.Empty : Path.GetDirectoryName(_configJsonPath) ?? string.Empty;
        string selectedFolder = _dialogService?.SelectFolder("Выберите папку целевого проекта", currentFolder) ?? string.Empty;
        if (!string.IsNullOrEmpty(selectedFolder))
        {
            if (string.IsNullOrWhiteSpace(_name))
            {
                Name = Path.GetFileName(selectedFolder.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar));
            }
            ConfigJsonPath = Path.Combine(selectedFolder, "assets_config.json");
        }
    }

    public bool IsConfigJsonPathValid
    {
        get
        {
            if (string.IsNullOrWhiteSpace(_configJsonPath))
                return false;

            try
            {
                string fullPath = Path.GetFullPath(_configJsonPath);
                char[] invalidChars = Path.GetInvalidPathChars();
                if (_configJsonPath.IndexOfAny(invalidChars) >= 0)
                    return false;
                return true;
            }
            catch
            {
                return false;
            }
        }
    }

    public bool IsEnabled
    {
        get => _isEnabled;
        set
        {
            if (SetProperty(ref _isEnabled, value))
            {
                OnPropertyChanged(nameof(IsDirty));
            }
        }
    }

    public string Name
    {
        get => _name;
        set
        {
            if (SetProperty(ref _name, value))
            {
                OnPropertyChanged(nameof(IsDirty));
            }
        }
    }

    public eTargetPlatform TargetPlatform
    {
        get => _targetPlatform;
        set
        {
            if (SetProperty(ref _targetPlatform, value))
            {
                OnPropertyChanged(nameof(IsDirty));
            }
        }
    }

    public string ConfigJsonPath
    {
        get => _configJsonPath;
        set
        {
            if (SetProperty(ref _configJsonPath, value))
            {
                OnPropertyChanged(nameof(IsConfigJsonPathValid));
                OnPropertyChanged(nameof(IsDirty));
            }
        }
    }

    public bool IsDirty => _isEnabled != Model.IsEnabled ||
                           _name != Model.Name ||
                           _targetPlatform != Model.TargetPlatform ||
                           _configJsonPath != Model.ConfigJsonPath;

    public void ApplyToModel()
    {
        Model.IsEnabled = _isEnabled;
        Model.Name = _name;
        Model.TargetPlatform = _targetPlatform;
        Model.ConfigJsonPath = _configJsonPath;
    }

    public void ResetFromModel()
    {
        IsEnabled = Model.IsEnabled;
        Name = Model.Name;
        TargetPlatform = Model.TargetPlatform;
        ConfigJsonPath = Model.ConfigJsonPath;
        OnPropertyChanged(nameof(IsConfigJsonPathValid));
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
    private bool _isTargetProjectsModified = false;

    public ObservableCollection<TargetProjectViewModel> TargetProjects { get; } = new();

    public BuildProfileViewModel(BuildProfile model, Services.IDialogService? dialogService = null)
    {
        _model = model;
        _dialogService = dialogService;

        AddTargetProjectCommand = new RelayCommand(_ => AddTargetProject());
        RemoveTargetProjectCommand = new RelayCommand(param => RemoveTargetProject(param as TargetProjectViewModel));

        ResetFromModel();
    }

    public System.Windows.Input.ICommand AddTargetProjectCommand { get; }
    public System.Windows.Input.ICommand RemoveTargetProjectCommand { get; }

    private void AddTargetProject()
    {
        string path = _dialogService?.SelectFolder("Выберите папку целевого проекта", string.Empty) ?? string.Empty;
        if (!string.IsNullOrEmpty(path))
        {
            string projName = Path.GetFileName(path.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar));
            string jsonPath = Path.Combine(path, "assets_config.json");

            var item = new TargetProjectItem
            {
                IsEnabled = true,
                Name = projName,
                TargetPlatform = InferTargetPlatform(projName),
                ConfigJsonPath = jsonPath
            };

            var vm = new TargetProjectViewModel(item, _dialogService);
            vm.PropertyChanged += OnTargetProjectPropertyChanged;
            TargetProjects.Add(vm);

            _isTargetProjectsModified = true;
            OnPropertyChanged(nameof(IsDirty));
            OnPropertyChanged(nameof(DisplayName));
            OnPropertyChanged(nameof(IsValid));
        }
    }

    private void RemoveTargetProject(TargetProjectViewModel? item)
    {
        if (item != null && TargetProjects.Contains(item))
        {
            TargetProjects.Remove(item);
            _isTargetProjectsModified = true;
            OnPropertyChanged(nameof(IsDirty));
            OnPropertyChanged(nameof(DisplayName));
            OnPropertyChanged(nameof(IsValid));
        }
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

    private static eTargetPlatform InferTargetPlatform(string name)
    {
        if (name.Contains("linux", StringComparison.OrdinalIgnoreCase))
            return eTargetPlatform.Linux;
        if (name.Contains("android", StringComparison.OrdinalIgnoreCase))
            return eTargetPlatform.Android;
        if (name.Contains("ios", StringComparison.OrdinalIgnoreCase))
            return eTargetPlatform.iOS;
        if (name.Contains("macos", StringComparison.OrdinalIgnoreCase) || name.Contains("mac", StringComparison.OrdinalIgnoreCase))
            return eTargetPlatform.MacOS;
        return eTargetPlatform.Windows;
    }

    public string SourcePath
    {
        get => _sourcePath;
        set
        {
            if (SetProperty(ref _sourcePath, value))
            {
                OnPropertyChanged(nameof(IsSourcePathValid));
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

    // Источниковый путь к папке проекта должен существовать на диске
    public bool IsSourcePathValid => !string.IsNullOrWhiteSpace(_sourcePath) && Directory.Exists(_sourcePath);

    // Валидация пути назначения: проверяем ТОЛЬКО корректность синтаксиса и имён пути, наличие папки на диске НЕ требуется!
    public bool IsDestinationPathValid
    {
        get
        {
            if (string.IsNullOrWhiteSpace(_destinationPath))
                return false;

            try
            {
                // Проверка синтаксиса полного пути
                string fullPath = Path.GetFullPath(_destinationPath);
                
                // Проверка недопустимых символов в пути Windows
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

    public bool IsValid => !string.IsNullOrWhiteSpace(_name) && IsSourcePathValid && IsDestinationPathValid && TargetProjects.Any(tp => tp.IsEnabled);

    public bool IsDirty => _name != _model.Name ||
                           _sourcePath != _model.SourcePath ||
                           _destinationPath != _model.DestinationPath ||
                           _isTargetProjectsModified ||
                           TargetProjects.Any(tp => tp.IsDirty);

    public string DisplayName => IsDirty ? $"{_name} *" : _name;

    public void ResetFromModel()
    {
        _name = _model.Name;
        _sourcePath = _model.SourcePath;
        _destinationPath = _model.DestinationPath;
        _isTargetProjectsModified = false;

        TargetProjects.Clear();
        if (_model.TargetProjects != null)
        {
            foreach (var item in _model.TargetProjects)
            {
                var vm = new TargetProjectViewModel(item, _dialogService);
                vm.PropertyChanged += OnTargetProjectPropertyChanged;
                TargetProjects.Add(vm);
            }
        }

        OnPropertyChanged(nameof(Name));
        OnPropertyChanged(nameof(SourcePath));
        OnPropertyChanged(nameof(DestinationPath));
        OnPropertyChanged(nameof(DisplayName));
        OnPropertyChanged(nameof(IsSourcePathValid));
        OnPropertyChanged(nameof(IsDestinationPathValid));
        OnPropertyChanged(nameof(IsDirty));
        OnPropertyChanged(nameof(IsValid));
    }

    private void OnTargetProjectPropertyChanged(object? sender, System.ComponentModel.PropertyChangedEventArgs e)
    {
        if (e.PropertyName == nameof(TargetProjectViewModel.IsDirty))
        {
            OnPropertyChanged(nameof(IsDirty));
            OnPropertyChanged(nameof(DisplayName));
            OnPropertyChanged(nameof(IsValid));
        }
    }

    public void ApplyToModel()
    {
        _model.Name = _name;
        _model.SourcePath = _sourcePath;
        _model.DestinationPath = _destinationPath;

        _model.TargetProjects = TargetProjects.Select(tp => {
            tp.ApplyToModel();
            return tp.Model;
        }).ToList();

        _isTargetProjectsModified = false;

        OnPropertyChanged(nameof(DisplayName));
        OnPropertyChanged(nameof(IsDirty));
    }
}
