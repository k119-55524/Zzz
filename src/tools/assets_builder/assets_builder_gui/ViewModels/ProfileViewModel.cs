using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;
using assets_builder_lib;

namespace assets_builder_gui.ViewModels;

public class ProfileViewModel : INotifyPropertyChanged
{
    private readonly BuildProfile _model;

    private string _name = string.Empty;
    private string _scriptsPath = string.Empty;
    private string _assetsPath = string.Empty;
    private string _destinationPath = string.Empty;

    public ProfileViewModel(BuildProfile model)
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
            if (_name != value)
            {
                _name = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(DisplayName));
                OnPropertyChanged(nameof(IsDirty));
                OnPropertyChanged(nameof(IsValid));
            }
        }
    }

    public string ScriptsPath
    {
        get => _scriptsPath;
        set
        {
            if (_scriptsPath != value)
            {
                _scriptsPath = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(IsScriptsPathValid));
                OnPropertyChanged(nameof(IsDirty));
                OnPropertyChanged(nameof(IsValid));
            }
        }
    }

    public string AssetsPath
    {
        get => _assetsPath;
        set
        {
            if (_assetsPath != value)
            {
                _assetsPath = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(IsAssetsPathValid));
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
            if (_destinationPath != value)
            {
                _destinationPath = value;
                OnPropertyChanged();
                OnPropertyChanged(nameof(IsDestinationPathValid));
                OnPropertyChanged(nameof(IsDirty));
                OnPropertyChanged(nameof(IsValid));
            }
        }
    }

    // Валидация источниковых путей: должны реально существовать на диске
    public bool IsScriptsPathValid => !string.IsNullOrWhiteSpace(_scriptsPath) && Directory.Exists(_scriptsPath);
    public bool IsAssetsPathValid => !string.IsNullOrWhiteSpace(_assetsPath) && Directory.Exists(_assetsPath);

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

    public bool IsValid => !string.IsNullOrWhiteSpace(_name) && IsScriptsPathValid && IsAssetsPathValid && IsDestinationPathValid;

    public bool IsDirty => _name != _model.Name ||
                           _scriptsPath != _model.ScriptsPath ||
                           _assetsPath != _model.AssetsPath ||
                           _destinationPath != _model.DestinationPath;

    public string DisplayName => IsDirty ? $"{_name} *" : _name;

    public void ResetFromModel()
    {
        _name = _model.Name;
        _scriptsPath = _model.ScriptsPath;
        _assetsPath = _model.AssetsPath;
        _destinationPath = _model.DestinationPath;

        OnPropertyChanged(nameof(Name));
        OnPropertyChanged(nameof(ScriptsPath));
        OnPropertyChanged(nameof(AssetsPath));
        OnPropertyChanged(nameof(DestinationPath));
        OnPropertyChanged(nameof(DisplayName));
        OnPropertyChanged(nameof(IsScriptsPathValid));
        OnPropertyChanged(nameof(IsAssetsPathValid));
        OnPropertyChanged(nameof(IsDestinationPathValid));
        OnPropertyChanged(nameof(IsDirty));
        OnPropertyChanged(nameof(IsValid));
    }

    public void ApplyToModel()
    {
        _model.Name = _name;
        _model.ScriptsPath = _scriptsPath;
        _model.AssetsPath = _assetsPath;
        _model.DestinationPath = _destinationPath;

        OnPropertyChanged(nameof(DisplayName));
        OnPropertyChanged(nameof(IsDirty));
    }

    public event PropertyChangedEventHandler? PropertyChanged;

    protected void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}
