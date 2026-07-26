using System.Collections.Generic;
using System.IO;
using assets_builder_gui.Models;

namespace assets_builder_gui.ViewModels;

public class BuildProfileViewModel : ViewModelBase
{
    private readonly BuildProfile _model;

    private string _name = string.Empty;
    private string _configuration = "Debug";
    private string _sourcePath = string.Empty;
    private string _destinationPath = string.Empty;

    public static List<string> AvailableConfigurations { get; } = new() { "Debug", "Development", "Release" };

    public BuildProfileViewModel(BuildProfile model)
    {
        _model = model;
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

    public string Configuration
    {
        get => _configuration;
        set
        {
            if (SetProperty(ref _configuration, value))
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

    public bool IsValid => !string.IsNullOrWhiteSpace(_name) && !string.IsNullOrWhiteSpace(_configuration) && IsSourcePathValid && IsDestinationPathValid;

    public bool IsDirty => _name != _model.Name ||
                           _configuration != _model.Configuration ||
                           _sourcePath != _model.SourcePath ||
                           _destinationPath != _model.DestinationPath;

    public string DisplayName => IsDirty ? $"{_name} *" : _name;

    public void ResetFromModel()
    {
        _name = _model.Name;
        _configuration = string.IsNullOrWhiteSpace(_model.Configuration) ? "Debug" : _model.Configuration;
        _sourcePath = _model.SourcePath;
        _destinationPath = _model.DestinationPath;

        OnPropertyChanged(nameof(Name));
        OnPropertyChanged(nameof(Configuration));
        OnPropertyChanged(nameof(SourcePath));
        OnPropertyChanged(nameof(DestinationPath));
        OnPropertyChanged(nameof(DisplayName));
        OnPropertyChanged(nameof(IsSourcePathValid));
        OnPropertyChanged(nameof(IsDestinationPathValid));
        OnPropertyChanged(nameof(IsDirty));
        OnPropertyChanged(nameof(IsValid));
    }

    public void ApplyToModel()
    {
        _model.Name = _name;
        _model.Configuration = _configuration;
        _model.SourcePath = _sourcePath;
        _model.DestinationPath = _destinationPath;

        OnPropertyChanged(nameof(DisplayName));
        OnPropertyChanged(nameof(IsDirty));
    }
}
