using CommunityToolkit.Mvvm.ComponentModel;
using BuildConfigurator.Models;

namespace BuildConfigurator.ViewModels;

public partial class ConfigItemViewModel : ViewModelBase
{
    public BuildConfiguration Configuration { get; }

    [ObservableProperty] private bool _hasUnsavedChanges;

    public string Name        => Configuration.Name;
    public string Description => Configuration.Description;
    public string DisplayName => HasUnsavedChanges ? $"{Name}*" : Name;

    public ConfigItemViewModel(BuildConfiguration cfg) => Configuration = cfg;

    public void RefreshName()
    {
        OnPropertyChanged(nameof(Name));
        OnPropertyChanged(nameof(Description));
        OnPropertyChanged(nameof(DisplayName));
    }

    partial void OnHasUnsavedChangesChanged(bool value) => OnPropertyChanged(nameof(DisplayName));
}
