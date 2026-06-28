using System;
using System.IO;
using System.Linq;
using System.Collections.ObjectModel;
using System.Collections.Generic;
using editor.Models;
using editor.Services.Project.Infrastructure;

namespace editor.ViewModels
{
    public class TomlPropertyViewModel : ViewModelBase
    {
        private string _value = string.Empty;
        private readonly System.Reflection.PropertyInfo _propInfo;
        private readonly object _owner;
        private readonly Action _onChanged;

        public TomlPropertyViewModel(object owner, System.Reflection.PropertyInfo propInfo, EditorVisibility visibility, Action onChanged)
        {
            _owner = owner;
            _propInfo = propInfo;
            _onChanged = onChanged;
            Name = propInfo.Name;
            IsReadOnly = visibility == EditorVisibility.ReadOnly;
            _value = propInfo.GetValue(owner)?.ToString() ?? string.Empty;
        }

        public string Name { get; }
        public bool IsReadOnly { get; }

        public string Value
        {
            get => _value;
            set
            {
                if (SetField(ref _value, value))
                {
                    try
                    {
                        var typedValue = Convert.ChangeType(value, _propInfo.PropertyType);
                        _propInfo.SetValue(_owner, typedValue);
                        _onChanged?.Invoke();
                    }
                    catch
                    {
                        // Ignore parse exceptions
                    }
                }
            }
        }
    }

    public class InspectorViewModel : PaneViewModel
    {
        private object? _selectedItem;
        private FolderStatisticsViewModel? _folderStats;
        private bool _showFolderStats;
        private ObservableCollection<TomlPropertyViewModel>? _tomlProperties;
        private bool _showTomlProperties;
        private List<TomlPropertyViewModel> _allTomlProperties = new();
        private string _searchText = string.Empty;

        public InspectorViewModel() : base(WidgetType.Inspector)
        {
            App.SelectionService.SelectedItemChanged += OnSelectedItemChanged;
        }

        public string SearchText
        {
            get => _searchText;
            set
            {
                if (SetField(ref _searchText, value))
                {
                    ApplyFilter();
                }
            }
        }

        private void ApplyFilter()
        {
            if (string.IsNullOrWhiteSpace(_searchText))
            {
                TomlProperties = new ObservableCollection<TomlPropertyViewModel>(_allTomlProperties);
            }
            else
            {
                var query = _searchText.Trim();
                var filtered = _allTomlProperties
                    .Where(p => p.Name.Contains(query, StringComparison.OrdinalIgnoreCase) 
                             || (p.Value?.Contains(query, StringComparison.OrdinalIgnoreCase) ?? false))
                    .ToList();
                TomlProperties = new ObservableCollection<TomlPropertyViewModel>(filtered);
            }
        }

        private void OnSelectedItemChanged(object? item)
        {
            SelectedItem = item;
            ShowFolderStats = false;
            ShowTomlProperties = false;
            FolderStats = null;
            _allTomlProperties.Clear();
            TomlProperties = null;
            _searchText = string.Empty;
            OnPropertyChanged(nameof(SearchText));

            if (item is ProjectNode node)
            {
                if (node.IsFolder)
                {
                    var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as MainWindowViewModel;
                    string? projectRoot = mainVm?.CurrentProjectPath;
                    if (mainVm != null && !string.IsNullOrEmpty(projectRoot) && App.ProjectService.CurrentSettings != null)
                    {
                        var activeFilters = App.ProjectService.CurrentSettings.DisabledFilters;
                        var mainAssetsVm = mainVm.Panes.OfType<AssetsViewModel>().FirstOrDefault();
                        bool isSystemMode = mainAssetsVm?.IsSystemNode(node) ?? false;
                        var roots = isSystemMode
                            ? (mainAssetsVm?.SystemRootNodes ?? new ObservableCollection<ProjectNode>())
                            : (mainAssetsVm?.AssetRootNodes ?? new ObservableCollection<ProjectNode>());

                        var stats = FolderStatisticsCalculator.Calculate(node, roots, projectRoot);
                        FolderStats = new FolderStatisticsViewModel(node.Name, node.RelativePath, stats);
                        ShowFolderStats = true;
                    }
                }
                else if (node.Name.EndsWith(".toml", StringComparison.OrdinalIgnoreCase))
                {
                    var settings = App.ProjectService.CurrentSettings;
                    if (settings != null)
                    {
                        var props = new List<TomlPropertyViewModel>();
                        var properties = settings.GetType().GetProperties();
                        foreach (var prop in properties)
                        {
                            var attr = prop.GetCustomAttributes(typeof(EditorVisibilityAttribute), true)
                                           .FirstOrDefault() as EditorVisibilityAttribute;
                            if (attr != null && attr.Visibility != EditorVisibility.Hidden)
                            {
                                props.Add(new TomlPropertyViewModel(settings, prop, attr.Visibility, OnTomlSettingChanged));
                            }
                        }
                        _allTomlProperties = props;
                        ApplyFilter();
                        ShowTomlProperties = true;
                    }
                }
            }
        }

        private void OnTomlSettingChanged()
        {
            // Свойство меняется через SetProperty -> HistoryManager, который сам
            // вызывает OnChanged (автосохранение) и отражает dirty-состояние через IsDirty.
        }

        public object? SelectedItem
        {
            get => _selectedItem;
            set => SetField(ref _selectedItem, value);
        }

        public FolderStatisticsViewModel? FolderStats
        {
            get => _folderStats;
            set => SetField(ref _folderStats, value);
        }

        public bool ShowFolderStats
        {
            get => _showFolderStats;
            set => SetField(ref _showFolderStats, value);
        }

        public ObservableCollection<TomlPropertyViewModel>? TomlProperties
        {
            get => _tomlProperties;
            set => SetField(ref _tomlProperties, value);
        }

        public bool ShowTomlProperties
        {
            get => _showTomlProperties;
            set => SetField(ref _showTomlProperties, value);
        }

        public override void OnProjectClosed()
        {
            base.OnProjectClosed();
            SelectedItem = null;
            FolderStats = null;
            ShowFolderStats = false;
            _allTomlProperties.Clear();
            TomlProperties = null;
            ShowTomlProperties = false;
            _searchText = string.Empty;
            OnPropertyChanged(nameof(SearchText));
        }
    }

    public class FolderStatisticsViewModel : ViewModelBase
    {
        public string Name { get; }
        public string Path { get; }
        public int SubfoldersCount { get; }
        public int FilesCount { get; }
        public string TotalSizeFormatted { get; }

        public FolderStatisticsViewModel(string name, string path, FolderStatistics stats)
        {
            Name = name;
            Path = path;
            SubfoldersCount = stats.SubfoldersCount;
            FilesCount = stats.FilesCount;
            TotalSizeFormatted = FormatSize(stats.TotalSize);
        }

        private string FormatSize(long bytes)
        {
            string[] suffix = { "B", "KB", "MB", "GB", "TB" };
            double dblSized = bytes;
            int i = 0;
            while (dblSized >= 1024 && i < suffix.Length - 1)
            {
                i++;
                dblSized /= 1024;
            }
            return $"{dblSized:N2} {suffix[i]}";
        }
    }
}
