using System.IO;
using assets_builder_gui.Models;

namespace assets_builder_gui.ViewModels;

public class BuildProfileViewModel : ViewModelBase
{
    private readonly BuildProfile _model;

    private string _name = string.Empty;
    private string _sourcePath = string.Empty;
    private string _destinationPath = string.Empty;

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

    public string SourcePath
    {
        get => _sourcePath;
        set
        {
            if (SetProperty(ref _sourcePath, value))
            {
                OnPropertyChanged(nameof(IsSourcePathValid));
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
                OnPropertyChanged(nameof(IsDirty));
                OnPropertyChanged(nameof(IsValid));
            }
        }
    }

    // Источниковый путь к папке проекта должен существовать на диске
    public bool IsSourcePathValid => !string.IsNullOrWhiteSpace(_sourcePath) && Directory.Exists(_sourcePath);

    // Валидация пути назначения: не обязан существовать, но синтаксис пути должен быть правильным
    public bool IsDestinationPathValid
    {
        get
        {
            if (string.IsNullOrWhiteSpace(_destinationPath))
                return false;
            try
            {
                var fullPath = Path.GetFullPath(_destinationPath);
                return true;
            }
            catch
            {
                return false;
            }
        }
    }

    public bool IsValid => !string.IsNullOrWhiteSpace(_name) && IsSourcePathValid && IsDestinationPathValid;

    public bool IsDirty => _name != _model.Name ||
                           _sourcePath != _model.SourcePath ||
                           _destinationPath != _model.DestinationPath;

    public string DisplayName => IsDirty ? $"{_name} *" : _name;

    public void ResetFromModel()
    {
        _name = _model.Name;
        _sourcePath = _model.SourcePath;
        _destinationPath = _model.DestinationPath;

        OnPropertyChanged(nameof(Name));
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
        _model.SourcePath = _sourcePath;
        _model.DestinationPath = _destinationPath;

        OnPropertyChanged(nameof(DisplayName));
        OnPropertyChanged(nameof(IsDirty));
    }
}
