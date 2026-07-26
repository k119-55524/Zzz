using System.IO;
using assets_builder_lib;

namespace assets_builder_gui.ViewModels;

public class BuildProfileViewModel : ViewModelBase
{
    private readonly BuildProfile _model;

    private string _name = string.Empty;
    private string _scriptsPath = string.Empty;
    private string _assetsPath = string.Empty;
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

    public string ScriptsPath
    {
        get => _scriptsPath;
        set
        {
            if (SetProperty(ref _scriptsPath, value))
            {
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
            if (SetProperty(ref _assetsPath, value))
            {
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
            if (SetProperty(ref _destinationPath, value))
            {
                OnPropertyChanged(nameof(IsDestinationPathValid));
                OnPropertyChanged(nameof(IsDirty));
                OnPropertyChanged(nameof(IsValid));
            }
        }
    }

    public bool IsScriptsPathValid => !string.IsNullOrWhiteSpace(_scriptsPath) && Directory.Exists(_scriptsPath);
    public bool IsAssetsPathValid => !string.IsNullOrWhiteSpace(_assetsPath) && Directory.Exists(_assetsPath);

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
}
