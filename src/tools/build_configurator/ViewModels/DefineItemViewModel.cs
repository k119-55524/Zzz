using CommunityToolkit.Mvvm.ComponentModel;
using BuildConfigurator.Models;

namespace BuildConfigurator.ViewModels;

// Used in Tab 2 (Defines list)
public partial class DefineItemViewModel : ViewModelBase
{
    public Define Model { get; }

    public string Name        => Model.Name;
    public string Description => Model.Description;
    public string DisplayName => HasUnsavedChanges ? $"{Name}*" : Name;

    public bool IsArchived
    {
        get => Model.IsArchived;
        set
        {
            if (Model.IsArchived == value) return;
            Model.IsArchived = value;
            OnPropertyChanged();
        }
    }

    [ObservableProperty] private bool _isSelected;
    [ObservableProperty] private bool _hasUnsavedChanges;

    public DefineItemViewModel(Define model) => Model = model;

    public void MarkDirty() => HasUnsavedChanges = true;

    public void Refresh()
    {
        OnPropertyChanged(nameof(Name));
        OnPropertyChanged(nameof(Description));
        OnPropertyChanged(nameof(IsArchived));
        OnPropertyChanged(nameof(DisplayName));
    }

    partial void OnHasUnsavedChangesChanged(bool value)
        => OnPropertyChanged(nameof(DisplayName));
}
