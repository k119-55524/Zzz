using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Windows;
using System.Windows.Input;
using editor.Models;
using editor.Services;
using editor.Services.Project;
using editor.Services.Project.Infrastructure;
using editor.Services.Project.FileTypes.ProjectSettings;

namespace editor.ViewModels
{
    public class CollectionItemViewModel : ViewModelBase
    {
        private readonly Action<CollectionItemViewModel, string> _onChanged;
        private string _value;

        public CollectionItemViewModel(string value, Action<CollectionItemViewModel, string> onChanged)
        {
            _value = value;
            _onChanged = onChanged;
        }

        public string Value
        {
            get => _value;
            set
            {
                if (SetField(ref _value, value))
                {
                    _onChanged(this, value);
                }
            }
        }
    }

    public class TomlPropertyViewModel : ViewModelBase
    {
        private string _value = string.Empty;
        private string _newCollectionItemValue = string.Empty;
        private readonly PropertyInfo _propInfo;
        private readonly object _owner;
        private readonly Action _onChanged;
        private readonly bool _isProjectNameField;
        private readonly Action<string>? _onProjectRenamed;
        private readonly EditorCollectionAttribute? _collectionAttribute;

        public TomlPropertyViewModel(object owner, PropertyInfo propInfo, EditorVisibility visibility, Action onChanged)
        {
            _owner = owner;
            _propInfo = propInfo;
            _onChanged = onChanged;
            _collectionAttribute = propInfo.GetCustomAttribute<EditorCollectionAttribute>();
            Name = propInfo.Name;
            IsReadOnly = visibility == EditorVisibility.ReadOnly;
            IsCollection = _collectionAttribute != null && typeof(IEnumerable).IsAssignableFrom(propInfo.PropertyType) && propInfo.PropertyType != typeof(string);
            IsSortableCollection = _collectionAttribute?.IsSortable == true;

            var options = propInfo.GetCustomAttribute<EditorOptionsAttribute>();
            OptionItems = options != null
                ? new ObservableCollection<string>(ResolveOptions(options))
                : new ObservableCollection<string>();
            HasOptions = OptionItems.Count > 0;
            IsStringValue = !IsCollection && !HasOptions;

            RemoveCollectionItemCommand = new RelayCommand<CollectionItemViewModel>(RemoveCollectionItem);
            MoveCollectionItemUpCommand = new RelayCommand<CollectionItemViewModel>(MoveCollectionItemUp);
            MoveCollectionItemDownCommand = new RelayCommand<CollectionItemViewModel>(MoveCollectionItemDown);
            AddCollectionItemCommand = new RelayCommand(AddCollectionItem);

            RefreshFromOwner();
        }

        // Специальный конструктор для поля "Name" настроек проекта: вместо обычной записи
        // значения свойства, переименовывает физическую корневую папку проекта на диске
        // (см. ProjectService.RenameProject). При неудачной валидации ошибка уходит в лог
        // (EditorLogger), а отображаемое значение откатывается на прежнее имя.
        public TomlPropertyViewModel(object owner, PropertyInfo propInfo, EditorVisibility visibility, Action<string> onProjectRenamed)
            : this(owner, propInfo, visibility, (Action)(() => { }))
        {
            _isProjectNameField = true;
            _onProjectRenamed = onProjectRenamed;
        }

        public string Name { get; }
        public bool IsReadOnly { get; }
        public bool IsCollection { get; }
        public bool IsSortableCollection { get; }
        public bool HasOptions { get; }
        public bool IsStringValue { get; }
        public ObservableCollection<string> OptionItems { get; }
        public ObservableCollection<CollectionItemViewModel> CollectionItems { get; } = new();
        public ICommand RemoveCollectionItemCommand { get; }
        public ICommand MoveCollectionItemUpCommand { get; }
        public ICommand MoveCollectionItemDownCommand { get; }
        public ICommand AddCollectionItemCommand { get; }

        public string NewCollectionItemValue
        {
            get => _newCollectionItemValue;
            set => SetField(ref _newCollectionItemValue, value);
        }

        public string Value
        {
            get => _value;
            set
            {
                if (_value == value) return;

                if (_isProjectNameField)
                {
                    if (App.ProjectService.RenameProject(value, out string error))
                    {
                        _value = value;
                        OnPropertyChanged(nameof(Value));
                        _onProjectRenamed?.Invoke(App.ProjectService.CurrentProjectRootPath ?? value);
                    }
                    else
                    {
                        editor.Services.EditorLogger.LogError(error);
                        OnPropertyChanged(nameof(Value)); // откатывает TextBox на прежнее (валидное) значение
                    }
                    return;
                }

                if (SetField(ref _value, value))
                {
                    ApplyScalarValue(value);
                }
            }
        }

        private void RefreshFromOwner()
        {
            if (IsCollection)
            {
                CollectionItems.Clear();
                foreach (var item in ReadCollectionValues())
                {
                    CollectionItems.Add(new CollectionItemViewModel(item, OnCollectionItemChanged));
                }
                return;
            }

            _value = _propInfo.GetValue(_owner)?.ToString() ?? string.Empty;
            OnPropertyChanged(nameof(Value));
        }

        private void ApplyScalarValue(string value)
        {
            try
            {
                object? typedValue;
                if (_propInfo.PropertyType == typeof(string))
                {
                    typedValue = value;
                }
                else if (_propInfo.PropertyType.IsEnum)
                {
                    typedValue = Enum.Parse(_propInfo.PropertyType, value);
                }
                else
                {
                    typedValue = Convert.ChangeType(value, _propInfo.PropertyType);
                }

                _propInfo.SetValue(_owner, typedValue);
                _onChanged?.Invoke();
            }
            catch (Exception ex)
            {
                EditorLogger.LogError($"Failed to edit '{Name}': {ex.Message}");
            }
        }

        private List<string> ReadCollectionValues()
        {
            if (_propInfo.GetValue(_owner) is IEnumerable values)
            {
                return values.Cast<object?>()
                    .Select(v => v?.ToString() ?? string.Empty)
                    .ToList();
            }

            return new List<string>();
        }

        private void WriteCollectionValues()
        {
            var values = CollectionItems
                .Select(item => item.Value.Trim())
                .Where(item => item.Length > 0)
                .ToList();

            if (_collectionAttribute?.AllowDuplicates != true)
            {
                values = values
                    .Distinct(StringComparer.OrdinalIgnoreCase)
                    .ToList();
            }

            _propInfo.SetValue(_owner, values);
            _onChanged?.Invoke();
        }

        private void OnCollectionItemChanged(CollectionItemViewModel item, string value)
        {
            WriteCollectionValues();
        }

        private void AddCollectionItem()
        {
            var value = NewCollectionItemValue.Trim();
            if (value.Length == 0)
            {
                return;
            }

            if (_collectionAttribute?.AllowDuplicates != true &&
                CollectionItems.Any(item => string.Equals(item.Value, value, StringComparison.OrdinalIgnoreCase)))
            {
                NewCollectionItemValue = string.Empty;
                return;
            }

            CollectionItems.Add(new CollectionItemViewModel(value, OnCollectionItemChanged));
            NewCollectionItemValue = string.Empty;
            WriteCollectionValues();
        }

        private void RemoveCollectionItem(CollectionItemViewModel? item)
        {
            if (item == null) return;
            CollectionItems.Remove(item);
            WriteCollectionValues();
        }

        private void MoveCollectionItemUp(CollectionItemViewModel? item)
        {
            if (item == null || !IsSortableCollection) return;
            int index = CollectionItems.IndexOf(item);
            if (index <= 0) return;
            CollectionItems.Move(index, index - 1);
            WriteCollectionValues();
        }

        private void MoveCollectionItemDown(CollectionItemViewModel? item)
        {
            if (item == null || !IsSortableCollection) return;
            int index = CollectionItems.IndexOf(item);
            if (index < 0 || index >= CollectionItems.Count - 1) return;
            CollectionItems.Move(index, index + 1);
            WriteCollectionValues();
        }

        private static IEnumerable<string> ResolveOptions(EditorOptionsAttribute attribute)
        {
            if (attribute.Source == EditorOptionsSource.LogListeners)
            {
                if (attribute.AllowNone)
                {
                    yield return string.Empty;
                }

                yield return "Console";
                yield return "Network";
                yield return "Callback";
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
        private IEditorConfigParser? _currentParser;
        private object? _currentConfigData;
        private string? _currentConfigFullPath;

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
                             || (p.Value?.Contains(query, StringComparison.OrdinalIgnoreCase) ?? false)
                             || p.CollectionItems.Any(i => i.Value.Contains(query, StringComparison.OrdinalIgnoreCase)))
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
            _currentParser = null;
            _currentConfigData = null;
            _currentConfigFullPath = null;
            OnPropertyChanged(nameof(SearchText));

            if (item is ProjectNode node)
            {
                if (node.IsFolder)
                {
                    var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as MainWindowViewModel;
                    string? projectRoot = mainVm?.CurrentProjectPath;
                    if (mainVm != null && !string.IsNullOrEmpty(projectRoot))
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
                    TryShowConfigFile(node);
                }
            }
        }

        private void OnTomlSettingChanged()
        {
            SaveCurrentConfigFile();
            // Свойство меняется через SetProperty -> HistoryManager, который сам
            // вызывает OnChanged (автосохранение) и отражает dirty-состояние через IsDirty.
        }

        // Вызывается после успешного ProjectService.RenameProject() (см. TomlPropertyViewModel) -
        // корневая папка проекта переехала на newPath, нужно обновить MainWindowViewModel.
        private bool TryShowConfigFile(ProjectNode node)
        {
            var mainVm = Application.Current?.MainWindow?.DataContext as MainWindowViewModel;
            string? projectRoot = mainVm?.CurrentProjectPath;
            if (string.IsNullOrEmpty(projectRoot))
            {
                return false;
            }

            var schema = ProjectStructure.AllFiles.FirstOrDefault(file =>
                string.Equals(file.RelativePath, node.RelativePath, StringComparison.OrdinalIgnoreCase));
            if (schema?.Parser is not IEditorConfigParser editorParser)
            {
                return false;
            }

            string fullPath = Path.Combine(projectRoot, node.RelativePath);
            if (!App.ProjectService.Storage.FileExists(fullPath))
            {
                return false;
            }

            try
            {
                _currentParser = editorParser;
                _currentConfigFullPath = fullPath;
                _currentConfigData = editorParser.DeserializeForEditor(App.ProjectService.Storage.ReadAllText(fullPath));
                BuildTomlProperties(node.RelativePath, _currentConfigData);
                ShowTomlProperties = true;
                return true;
            }
            catch (Exception ex)
            {
                EditorLogger.LogError($"Failed to inspect config '{node.RelativePath}': {ex.Message}");
                return false;
            }
        }

        private void BuildTomlProperties(string relativePath, object settings)
        {
            var props = new List<TomlPropertyViewModel>();
            foreach (var prop in settings.GetType().GetProperties())
            {
                var attr = prop.GetCustomAttribute<EditorVisibilityAttribute>();
                if (attr == null || attr.Visibility == EditorVisibility.Hidden)
                {
                    continue;
                }

                if (relativePath == ProjectConstants.SystemDirectories.ProjectSettings &&
                    prop.Name == nameof(ProjectSettingsData.Name))
                {
                    props.Add(new TomlPropertyViewModel(settings, prop, attr.Visibility, OnProjectRenamed));
                }
                else
                {
                    props.Add(new TomlPropertyViewModel(settings, prop, attr.Visibility, SaveCurrentConfigFile));
                }
            }

            _allTomlProperties = props;
            ApplyFilter();
        }

        private void SaveCurrentConfigFile()
        {
            if (_currentParser == null || _currentConfigData == null || string.IsNullOrEmpty(_currentConfigFullPath))
            {
                return;
            }

            try
            {
                string content = _currentParser.SerializeFromEditor(_currentConfigData);
                App.ProjectService.Storage.WriteAllText(_currentConfigFullPath, content);
            }
            catch (Exception ex)
            {
                EditorLogger.LogError($"Failed to save inspected config: {ex.Message}");
            }
        }

        private void OnProjectRenamed(string newPath)
        {
            var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as MainWindowViewModel;
            mainVm?.UpdateAfterProjectRename(newPath);
            _currentConfigFullPath = Path.Combine(newPath, ProjectConstants.SystemDirectories.ProjectSettings);
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
            _currentParser = null;
            _currentConfigData = null;
            _currentConfigFullPath = null;
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
