using CommunityToolkit.Mvvm.ComponentModel;

namespace BuildConfigurator.ViewModels;

// One row in the config editor's defines table (Tab 1)
public partial class DefineEntryViewModel : ViewModelBase
{
    public string Name        { get; init; } = string.Empty;
    public string Description { get; init; } = string.Empty;
    public bool   IsArchived  { get; init; }
    public bool   IsCMake     { get; init; }

    [ObservableProperty] private bool _isActive;

    private bool _originalIsActive;

    public bool IsDirty => IsActive != _originalIsActive;

    public void AcceptChanges() => _originalIsActive = IsActive;

    public void RevertChanges()
    {
        IsActive = _originalIsActive;
    }

    public static DefineEntryViewModel Create(string name, string description, bool isArchived, bool isCMake, bool isActive)
    {
        var vm = new DefineEntryViewModel
        {
            Name        = name,
            Description = description,
            IsArchived  = isArchived,
            IsCMake     = isCMake,
            _originalIsActive = isActive
        };
        vm.IsActive = isActive;
        return vm;
    }
}
