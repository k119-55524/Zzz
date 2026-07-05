using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Input;
using editor.Models;
using editor.Services;
using editor.Services.Project;
using editor.Services.Project.Assets;
using editor.Services.Project.FileTypes.GameConfig;
using editor.Services.Project.Infrastructure;
using editor.Services.Project.FileTypes.ProjectSettings;

namespace editor.ViewModels
{
    public class CollectionItemViewModel : ViewModelBase
    {
        private readonly Func<CollectionItemViewModel, string, bool> _onChanging;
        private string _value;

        public CollectionItemViewModel(string value, Func<CollectionItemViewModel, string, bool> onChanging)
        {
            _value = value;
            _onChanging = onChanging;
        }

        public string Value
        {
            get => _value;
            set
            {
                if (_value == value)
                {
                    return;
                }

                string oldValue = _value;
                _value = value;
                if (_onChanging(this, value))
                {
                    OnPropertyChanged(nameof(Value));
                    OnPropertyChanged(nameof(DisplayValue));
                }
                else
                {
                    _value = oldValue;
                    OnPropertyChanged(nameof(Value));
                    OnPropertyChanged(nameof(DisplayValue));
                }
            }
        }

        public string DisplayValue
        {
            get;
            set;
        } = string.Empty;
    }

    public class TomlPropertyViewModel : ViewModelBase
    {
        private string _value = string.Empty;
        private CollectionItemViewModel? _selectedCollectionItem;
        private readonly PropertyInfo _propInfo;
        private readonly object _owner;
        private readonly Action _onChanged;
        private readonly bool _isProjectNameField;
        private readonly Action<string>? _onProjectRenamed;
        private readonly EditorCollectionAttribute? _collectionAttribute;
        private List<string> _appliedCollectionValues = new();

        public TomlPropertyViewModel(object owner, PropertyInfo propInfo, EditorVisibility visibility, Action onChanged)
        {
            _owner = owner;
            _propInfo = propInfo;
            _onChanged = onChanged;
            _collectionAttribute = propInfo.GetCustomAttribute<EditorCollectionAttribute>();
            Name = propInfo.GetCustomAttribute<EditorDisplayNameAttribute>()?.DisplayName ?? propInfo.Name;
            IsReadOnly = visibility == EditorVisibility.ReadOnly;
            IsCollection = _collectionAttribute != null && typeof(IEnumerable).IsAssignableFrom(propInfo.PropertyType) && propInfo.PropertyType != typeof(string);
            IsSortableCollection = _collectionAttribute?.IsSortable == true;
            IsAssetGuidCollection = _collectionAttribute?.Kind == EditorCollectionKind.AssetGuidList;
            IsStringCollection = _collectionAttribute?.Kind == EditorCollectionKind.StringList;

            var options = propInfo.GetCustomAttribute<EditorOptionsAttribute>();
            OptionItems = options != null
                ? new ObservableCollection<string>(ResolveOptions(options))
                : new ObservableCollection<string>();
            HasOptions = OptionItems.Count > 0;
            IsStringValue = !IsCollection && !HasOptions;

            AddCollectionItemCommand = new RelayCommand(AddCollectionItem);
            DeleteCollectionItemCommand = new RelayCommand<CollectionItemViewModel>(DeleteCollectionItem);

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
        public bool IsAssetGuidCollection { get; }
        public bool IsStringCollection { get; }
        public bool HasOptions { get; }
        public bool IsStringValue { get; }
        public ObservableCollection<string> OptionItems { get; }
        public ObservableCollection<CollectionItemViewModel> CollectionItems { get; } = new();
        public ICommand AddCollectionItemCommand { get; }
        public ICommand DeleteCollectionItemCommand { get; }

        // Нет пользовательского Apply/Reset на само поле - незавершённые правки коллекции
        // (например, Defines) фиксируются парой Apply/Reset под всем конфигом
        // (InspectorViewModel.ApplyPendingChangesCommand/ResetPendingChangesCommand) либо диалогом
        // при уходе с этого файла (см. InspectorViewModel.CommitOrDiscardPendingTomlChanges).

        public CollectionItemViewModel? SelectedCollectionItem
        {
            get => _selectedCollectionItem;
            set => SetField(ref _selectedCollectionItem, value);
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
                // RefreshFromOwner запускается часто (после каждого удаления, Reset/Apply) - если
                // каждый раз выделять что-то по умолчанию, жёлтая рамка выделения ListBoxItem
                // (см. InspectorCollectionListBoxItemStyle) подсвечивает элемент, который
                // пользователь не выбирал (например, первый дефайн сразу при открытии файла).
                // Поэтому только сохраняем позицию (индекс) прежнего выделения, если оно было -
                // если выделения не было (первая загрузка, Reset без предварительного клика по
                // элементу), после обновления тоже ничего не выделяем.
                int previousIndex = SelectedCollectionItem != null ? CollectionItems.IndexOf(SelectedCollectionItem) : -1;

                CollectionItems.Clear();
                foreach (var item in ReadCollectionValues())
                {
                    CollectionItems.Add(CreateCollectionItem(item));
                }
                _appliedCollectionValues = ReadCollectionValues();
                SelectedCollectionItem = previousIndex >= 0 && previousIndex < CollectionItems.Count
                    ? CollectionItems[previousIndex]
                    : null;
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
            var values = BuildCollectionValues();
            if (values.SequenceEqual(_appliedCollectionValues, StringComparer.Ordinal))
            {
                return;
            }

            var oldValues = new List<string>(_appliedCollectionValues);
            var command = new editor.Services.Project.Infrastructure.UndoRedo.PropertyChangeCommand<List<string>>(
                setValueDirectly: v => _propInfo.SetValue(_owner, v),
                oldValue: oldValues,
                newValue: values,
                onChanged: SaveAndRefreshCollection);

            App.ProjectService.History.Execute(command);
        }

        // Есть ли в коллекции черновая правка (добавление/удаление/переименование элемента),
        // ещё не записанная во владельца. Используется как для CanExecute кнопок Apply/Reset
        // (InspectorViewModel.ApplyPendingChangesCommand/ResetPendingChangesCommand), так и для
        // диалога при уходе с файла (InspectorViewModel.CommitOrDiscardPendingTomlChanges).
        public bool HasPendingChanges => IsCollection && !BuildCollectionValues().SequenceEqual(_appliedCollectionValues, StringComparer.Ordinal);

        // Фиксирует черновую правку коллекции (кнопка "Применить" или "Да" в диалоге при уходе
        // с файла).
        public void ApplyPendingChanges()
        {
            if (HasPendingChanges)
            {
                WriteCollectionValues();
            }
        }

        // Отбрасывает черновую правку и возвращает список к значению, сохранённому на диске
        // (кнопка "Сбросить" или "Нет" в диалоге при уходе с файла).
        public void DiscardPendingChanges()
        {
            if (HasPendingChanges)
            {
                RefreshFromOwner();
            }
        }

        // Пишет файл на диск и, для дефайнов, перезапускает сборку scripts.dll - без пересборки
        // правка в инспекторе ни на что не влияет (см. MainWindow.CompileScriptsAsync).
        private void SaveAndRefreshCollection()
        {
            _onChanged?.Invoke();
            RefreshFromOwner();

            if (_collectionAttribute?.ItemValidation == EditorCollectionItemValidation.CppDefine)
            {
                RequestScriptsRebuild();
            }
        }

        private List<string> BuildCollectionValues()
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

            return values;
        }

        private bool OnCollectionItemChanging(CollectionItemViewModel item, string value)
        {
            if (_collectionAttribute?.ItemValidation == EditorCollectionItemValidation.CppDefine &&
                !IsValidCppDefine(value, out string error))
            {
                EditorLogger.LogError(error);
                return false;
            }

            return true;
        }

        private void AddCollectionItem()
        {
            var item = CreateCollectionItem(string.Empty);
            CollectionItems.Add(item);
            SelectedCollectionItem = item;
        }

        public void DeleteCollectionItem(CollectionItemViewModel? item)
        {
            if (item == null) return;
            int index = CollectionItems.IndexOf(item);
            CollectionItems.Remove(item);
            if (CollectionItems.Count == 0)
            {
                SelectedCollectionItem = null;
            }
            else
            {
                SelectedCollectionItem = CollectionItems[Math.Min(index, CollectionItems.Count - 1)];
            }

            WriteCollectionValues();
        }

        public void MoveCollectionItem(CollectionItemViewModel? draggedItem, CollectionItemViewModel? targetItem)
        {
            if (draggedItem == null || targetItem == null || draggedItem == targetItem || !IsSortableCollection)
            {
                return;
            }

            int oldIndex = CollectionItems.IndexOf(draggedItem);
            int newIndex = CollectionItems.IndexOf(targetItem);
            if (oldIndex < 0 || newIndex < 0 || oldIndex == newIndex)
            {
                return;
            }

            CollectionItems.Move(oldIndex, newIndex);
            SelectedCollectionItem = draggedItem;
            if (IsAssetGuidCollection)
            {
                WriteCollectionValues();
            }
        }

        public void TryAddProjectNode(ProjectNode? node)
        {
            if (!IsAssetGuidCollection)
            {
                return;
            }

            if (node == null || !node.IsScript)
            {
                EditorLogger.LogError("[Game Config] Only script assets can be added to Global scripts.");
                return;
            }

            var script = App.ScriptAssetIndexService.ByGuid.Values.FirstOrDefault(info =>
                string.Equals(info.HppPath, node.HppRelativePath, StringComparison.OrdinalIgnoreCase) ||
                string.Equals(info.MetaPath, node.MetaRelativePath, StringComparison.OrdinalIgnoreCase));
            if (script == null)
            {
                EditorLogger.LogError($"[Game Config] Script '{node.DisplayName}' has no valid GUID in the script index.");
                return;
            }

            if (_collectionAttribute?.AllowDuplicates != true &&
                CollectionItems.Any(item => string.Equals(item.Value, script.Guid, StringComparison.OrdinalIgnoreCase)))
            {
                EditorLogger.LogError($"[Game Config] Script '{script.ClassName}' is already in Global scripts.");
                return;
            }

            CollectionItems.Add(CreateCollectionItem(script.Guid));
            SelectedCollectionItem = CollectionItems.LastOrDefault();
            WriteCollectionValues();
        }

        // Дефайны попадают в CMakeLists.txt scripts.dll (см. MainWindow.CompileScriptsAsync) - без
        // пересборки правка в инспекторе ни на что не влияет. Явно логируем запуск - иначе для
        // пользователя пересборка выглядит так, будто ничего не произошло.
        private static void RequestScriptsRebuild()
        {
            if (Application.Current?.MainWindow is not MainWindow mainWindow)
            {
                return;
            }

            EditorLogger.LogInfo("[Game Config] Defines changed - rebuilding scripts.dll...");

            if (mainWindow.IsActive)
            {
                _ = mainWindow.CheckAndCompileScriptsAsync(forceRebuild: true);
            }
            else if (mainWindow.DataContext is MainWindowViewModel vm && vm.CurrentProjectPath != null)
            {
                ScriptRebuildCoordinator.RequestRebuild(vm.CurrentProjectPath);
            }
        }

        private CollectionItemViewModel CreateCollectionItem(string value)
        {
            var item = new CollectionItemViewModel(value, OnCollectionItemChanging)
            {
                DisplayValue = ResolveCollectionDisplayValue(value)
            };
            return item;
        }

        private string ResolveCollectionDisplayValue(string value)
        {
            if (IsAssetGuidCollection)
            {
                if (App.ScriptAssetIndexService.TryGetByGuid(value, out ScriptAssetInfo info))
                {
                    return info.ClassName;
                }

                return string.IsNullOrWhiteSpace(value) ? "<missing script>" : $"<missing> {value}";
            }

            return value;
        }

        private static bool IsValidCppDefine(string value, out string error)
        {
            error = string.Empty;
            string trimmed = value.Trim();
            if (trimmed.Length == 0)
            {
                return true;
            }

            var match = Regex.Match(trimmed, @"^([A-Za-z_][A-Za-z0-9_]*)(=.*)?$");
            if (!match.Success)
            {
                error = $"[Game Config] Invalid C++ define '{value}'. Use NAME or NAME=value; NAME must be a valid C/C++ identifier.";
                return false;
            }

            return true;
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
            ApplyPendingChangesCommand = new RelayCommand(ApplyAllPendingTomlChanges, HasAnyPendingTomlChanges);
            ResetPendingChangesCommand = new RelayCommand(DiscardAllPendingTomlChanges, HasAnyPendingTomlChanges);
        }

        // Reset/Apply на весь открытый конфиг (см. InspectorWidget.xaml) - фиксируют или
        // отбрасывают черновые правки любых коллекций текущего файла (например, Defines).
        public ICommand ApplyPendingChangesCommand { get; }
        public ICommand ResetPendingChangesCommand { get; }

        private bool HasAnyPendingTomlChanges()
        {
            return _allTomlProperties.Any(p => p.HasPendingChanges);
        }

        private void ApplyAllPendingTomlChanges()
        {
            foreach (var prop in _allTomlProperties)
            {
                prop.ApplyPendingChanges();
            }
        }

        private void DiscardAllPendingTomlChanges()
        {
            foreach (var prop in _allTomlProperties)
            {
                prop.DiscardPendingChanges();
            }
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
            // Гарантируем, что незавершённая правка коллекции (Apply ещё не нажат) не потеряется
            // молча при переключении на другой узел дерева - иначе _allTomlProperties сейчас
            // очистится вместе с несохранённым состоянием (см. CommitOrDiscardPendingTomlChanges).
            CommitOrDiscardPendingTomlChanges();

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

        // Спрашивает один раз за все накопленные черновые правки текущего файла (обычно это
        // только Defines) - предлагает применить их или отбросить и вернуть значение с диска.
        // Вызывается при выборе другого узла дерева и при закрытии проекта.
        private void CommitOrDiscardPendingTomlChanges()
        {
            var pending = _allTomlProperties.Where(p => p.HasPendingChanges).ToList();
            if (pending.Count == 0)
            {
                return;
            }

            string title = Application.Current?.TryFindResource("Inspector_ApplyChanges_Confirm_Title") as string
                ?? "Unsaved changes";
            string messageFormat = Application.Current?.TryFindResource("Inspector_ApplyChanges_Confirm_Message") as string
                ?? "There are unsaved changes in '{0}'. Apply them now? Choose \"No\" to discard them and revert to the value saved on disk.";
            string names = string.Join(", ", pending.Select(p => p.Name));

            var result = MessageBox.Show(string.Format(messageFormat, names), title, MessageBoxButton.YesNo, MessageBoxImage.Question);
            foreach (var prop in pending)
            {
                if (result == MessageBoxResult.Yes)
                {
                    prop.ApplyPendingChanges();
                }
                else
                {
                    prop.DiscardPendingChanges();
                }
            }
        }

        public override void OnProjectClosed()
        {
            CommitOrDiscardPendingTomlChanges();
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
