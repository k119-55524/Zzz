using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using editor.Models;
using editor.Services;
using editor.Services.Project;
using editor.Services.Project.Infrastructure;

namespace editor.ViewModels
{
    /// <summary>
    /// Представляет элемент фильтра в выпадающем списке.
    /// </summary>
    public class FilterItemViewModel : ViewModelBase
    {
        private bool _isChecked = true;
        private readonly Action _onCheckedChanged;

        public FilterItemViewModel(string name, string matchKey, bool isChecked, Action onCheckedChanged)
        {
            Name = name;
            MatchKey = matchKey;
            _isChecked = isChecked;
            _onCheckedChanged = onCheckedChanged;
        }

        // Отображаемое (локализованное) имя в чекбоксе.
        public string Name { get; }

        // Ключ для сравнения с DisabledFilters/DisabledSystemFilters и именами папок на диске.
        // Для типов ресурсов ассетов - это условное имя папки (см. AssetResourceTypeRules),
        // для системных директорий - то же самое, что и Name (там локализации нет).
        public string MatchKey { get; }

        public bool IsChecked
        {
            get => _isChecked;
            set
            {
                if (SetField(ref _isChecked, value))
                {
                    _onCheckedChanged?.Invoke();
                }
            }
        }
    }

    /// <summary>
    /// ViewModel для вкладки управления структурой проекта (AssetsWidget).
    /// </summary>
    public class AssetsViewModel : PaneViewModel
    {
        private readonly ProjectFileSystem _vfs;
        private readonly List<IAssetWatchHandler> _assetWatchHandlers = new()
        {
            new ScriptAssetWatchHandler()
        };
        private FileSystemWatcher? _assetsWatcher;
        private System.Windows.Threading.DispatcherTimer? _compileDebounceTimer;
        private ObservableCollection<ProjectNode> _assetRootNodes = new();
        private ObservableCollection<ProjectNode> _systemRootNodes = new();
        private ObservableCollection<FilterItemViewModel> _availableFilters = new();
        private bool _isSystemMode;
        private bool _isUpdatingFilters;
        private string _searchText = string.Empty;

        public AssetsViewModel() : base(WidgetType.Assets)
        {
            _vfs = new ProjectFileSystem(App.ProjectService.Storage);

            // Команда принудительного обновления дерева
            RefreshCommand = new RelayCommand(RefreshTree);
            ResetFiltersCommand = new RelayCommand(ResetCurrentFilters);
        }

        public ICommand ResetFiltersCommand { get; }

        public override void UpdateTitle()
        {
            base.UpdateTitle();
            UpdateFiltersList(); // подписи типов ресурсов локализованы - пересчитываем при смене языка
        }

        public override void OnProjectOpened(string projectPath)
        {
            base.OnProjectOpened(projectPath);

            // Полный скан .meta - только здесь, на открытии/смене проекта (не на каждый RefreshTree)
            EditorLogger.LogInfo("[Meta System] Running full project scan for script meta files...");
            _vfs.SyncScriptMetaFiles(projectPath);

            OnProjectChanged(projectPath);
            StartAssetsWatcher(projectPath);

            // OnProjectChanged уже пересканировал Assets и перегенерировал RegisterAllScripts.cpp,
            // но это не проверяет актуальность самой scripts.dll - без явного вызова здесь она
            // пересобиралась бы только по возврату фокуса окна (MainWindow_Activated) или по
            // следующему созданию/удалению скрипта, а не сразу при открытии проекта.
            // Передаём projectPath явно — CurrentProjectPath в MainWindowViewModel может ещё
            // не быть установлен в момент этого вызова.
            if (Application.Current?.MainWindow is MainWindow mainWindow)
            {
                _ = mainWindow.CheckAndCompileScriptsAsync(forceRebuild: true, projectRoot: projectPath);
            }
        }

        public override void OnProjectClosed()
        {
            StopAssetsWatcher();
            base.OnProjectClosed();
            OnProjectChanged(null);
        }

        private void StartAssetsWatcher(string projectPath)
        {
            StopAssetsWatcher();

            string assetsRoot = Path.Combine(projectPath, "Assets");
            if (!Directory.Exists(assetsRoot))
                return;

            _assetsWatcher = new FileSystemWatcher(assetsRoot)
            {
                IncludeSubdirectories = true,
                // FileName — создание/удаление/переименование файлов
                // LastWrite — правка содержимого (.cpp/.hpp в IDE)
                NotifyFilter = NotifyFilters.FileName | NotifyFilters.LastWrite
            };
            _assetsWatcher.Created += (s, e) => { if (IsScriptFile(e.FullPath)) ScriptRebuildCoordinator.RequestRebuild(projectPath); };
            _assetsWatcher.Deleted += (s, e) => { if (IsScriptFile(e.FullPath)) ScriptRebuildCoordinator.RequestRebuild(projectPath); };
            _assetsWatcher.Changed += (s, e) => { if (IsScriptFile(e.FullPath)) ScriptRebuildCoordinator.RequestRebuild(projectPath); };
            _assetsWatcher.Renamed += (s, e) => { if (IsScriptFile(e.FullPath)) ScriptRebuildCoordinator.RequestRebuild(projectPath); };
            _assetsWatcher.Created += OnWatchedFileCreated;
            _assetsWatcher.Deleted += OnWatchedFileDeleted;
            _assetsWatcher.Renamed += OnWatchedFileRenamed;
            _assetsWatcher.Changed += OnWatchedFileChanged;
            _assetsWatcher.EnableRaisingEvents = true;

            EditorLogger.LogInfo($"[Meta System] Started watching '{assetsRoot}' for external changes.");
        }

        private void StopAssetsWatcher()
        {
            if (_assetsWatcher == null)
                return;

            _assetsWatcher.EnableRaisingEvents = false;
            _assetsWatcher.Created -= OnWatchedFileCreated;
            _assetsWatcher.Deleted -= OnWatchedFileDeleted;
            _assetsWatcher.Renamed -= OnWatchedFileRenamed;
            _assetsWatcher.Changed -= OnWatchedFileChanged;
            _assetsWatcher.Dispose();
            _assetsWatcher = null;

            _compileDebounceTimer?.Stop();
            _compileDebounceTimer = null;
        }

        private void OnWatchedFileCreated(object sender, FileSystemEventArgs e)
        {
            // triggerCompile=true: создание .hpp/.cpp требует пересборки
            DispatchAssetEvent(() => HandleExternalCreate(e.FullPath), triggerCompile: IsScriptFile(e.FullPath));
        }

        private void OnWatchedFileDeleted(object sender, FileSystemEventArgs e)
        {
            DispatchAssetEvent(() => HandleExternalDelete(e.FullPath), triggerCompile: IsScriptFile(e.FullPath));
        }

        private void OnWatchedFileRenamed(object sender, RenamedEventArgs e)
        {
            DispatchAssetEvent(() =>
            {
                HandleExternalDelete(e.OldFullPath);
                HandleExternalCreate(e.FullPath);
            }, triggerCompile: IsScriptFile(e.OldFullPath) || IsScriptFile(e.FullPath));
        }

        // Правка содержимого файла в IDE — только компиляция, дерево не перестраиваем
        // (состав скриптов не менялся, только код внутри .cpp/.hpp).
        // Debounce через DispatcherTimer: IDE может вызвать Changed несколько раз подряд
        // при одном сохранении (write + flush) — ждём 500ms тишины перед запуском cmake.
        private void OnWatchedFileChanged(object sender, FileSystemEventArgs e)
        {
            if (!IsScriptFile(e.FullPath))
                return;

            Application.Current?.Dispatcher.BeginInvoke(() => ScheduleCompileDebounced());
        }

        private void ScheduleCompileDebounced()
        {
            if (_compileDebounceTimer == null)
            {
                _compileDebounceTimer = new System.Windows.Threading.DispatcherTimer
                {
                    Interval = TimeSpan.FromMilliseconds(500)
                };
                _compileDebounceTimer.Tick += (s, e) =>
                {
                    _compileDebounceTimer.Stop();
                    if (Application.Current?.MainWindow is MainWindow mw)
                        _ = mw.CheckAndCompileScriptsAsync(forceRebuild: true);
                };
            }

            // Перезапускаем таймер — каждый новый Changed сдвигает окно на 500ms
            _compileDebounceTimer.Stop();
            _compileDebounceTimer.Start();
        }

        private static bool IsScriptFile(string fullPath)
        {
            string ext = Path.GetExtension(fullPath);
            return ext.Equals(".hpp", StringComparison.OrdinalIgnoreCase) ||
                   ext.Equals(".cpp", StringComparison.OrdinalIgnoreCase);
        }

        // События FileSystemWatcher приходят в фоновом потоке - переносим обработку в UI-поток,
        // т.к. дальше идёт RefreshTree() с обновлением ObservableCollection.
        // triggerCompile=true: ПОСЛЕ RefreshTree (который обновит RegisterAllScripts.cpp)
        // запускаем компиляцию — порядок важен, иначе cmake соберёт устаревший RegisterAllScripts.cpp.
        private void DispatchAssetEvent(Action action, bool triggerCompile = false)
        {
            Application.Current?.Dispatcher.BeginInvoke(() =>
            {
                action();
                RefreshTree();  // ← сначала обновляем RegisterAllScripts.cpp
                if (triggerCompile && Application.Current?.MainWindow is MainWindow mw)
                    _ = mw.CheckAndCompileScriptsAsync(forceRebuild: true); // ← потом cmake
            });
        }

        private void HandleExternalCreate(string fullPath)
        {
            foreach (var handler in _assetWatchHandlers)
            {
                if (handler.CanHandle(fullPath))
                {
                    handler.OnCreated(fullPath, App.ProjectService.Storage);
                }
            }
        }

        private void HandleExternalDelete(string fullPath)
        {
            foreach (var handler in _assetWatchHandlers)
            {
                if (handler.CanHandle(fullPath))
                {
                    handler.OnDeleted(fullPath, App.ProjectService.Storage);
                }
            }
        }

        public ICommand RefreshCommand { get; }

        public ObservableCollection<ProjectNode> AssetRootNodes
        {
            get => _assetRootNodes;
            set
            {
                if (SetField(ref _assetRootNodes, value))
                {
                    OnPropertyChanged(nameof(RootNodes));
                }
            }
        }

        public ObservableCollection<ProjectNode> SystemRootNodes
        {
            get => _systemRootNodes;
            set
            {
                if (SetField(ref _systemRootNodes, value))
                {
                    OnPropertyChanged(nameof(RootNodes));
                }
            }
        }

        public ObservableCollection<ProjectNode> RootNodes => IsSystemMode ? SystemRootNodes : AssetRootNodes;

        // Определяет принадлежность узла дереву системных файлов по факту, а не по тумблеру IsSystemMode.
        // AssetsTree (ассеты) и SystemTree (секция "Project") видны одновременно в UI, поэтому
        // действия над конкретным узлом (создание/удаление/статистика и т.п.) должны определять
        // физическое расположение по тому, в каком дереве узел реально находится, а не по тому,
        // развёрнута ли сейчас секция "Project".
        public bool IsSystemNode(ProjectNode node) => ContainsNode(SystemRootNodes, node);

        private static bool ContainsNode(ObservableCollection<ProjectNode> roots, ProjectNode target)
        {
            foreach (var root in roots)
            {
                if (root == target) return true;
                if (ContainsNode(root.Children, target)) return true;
            }
            return false;
        }

        public ObservableCollection<FilterItemViewModel> AvailableFilters
        {
            get => _availableFilters;
            set => SetField(ref _availableFilters, value);
        }

        // Поиск по имени - действует только на AssetsTree (см. ApplySearchFilter). SystemTree
        // не должен зависеть ни от поиска, ни от фильтра типов ресурсов дерева ассетов.
        public string SearchText
        {
            get => _searchText;
            set
            {
                if (SetField(ref _searchText, value))
                {
                    ApplySearchFilter(AssetRootNodes, _searchText);
                }
            }
        }

        public bool IsSystemMode
        {
            get => _isSystemMode;
            set
            {
                if (SetField(ref _isSystemMode, value))
                {
                    App.ProjectService.CurrentSettings.ShowSystemMode = value;
                    var mainVm = Application.Current?.MainWindow?.DataContext as MainWindowViewModel;
                    if (mainVm?.CurrentProjectPath != null)
                    {
                        App.ProjectService.SaveProject(mainVm.CurrentProjectPath, out _);
                    }
                    OnPropertyChanged(nameof(RootNodes));
                    UpdateFiltersList();
                    RefreshTree();
                }
            }
        }

        private void OnProjectChanged(string? projectPath)
        {
            if (string.IsNullOrEmpty(projectPath))
            {
                AssetRootNodes.Clear();
                SystemRootNodes.Clear();
                AvailableFilters.Clear();
                _searchText = string.Empty;
                OnPropertyChanged(nameof(SearchText));
                return;
            }

            // Загружаем состояние сессии из настроек проекта
            _isSystemMode = App.ProjectService.CurrentSettings.ShowSystemMode;
            OnPropertyChanged(nameof(IsSystemMode));

            UpdateFiltersList();
            RefreshTree();
        }

        private static string Loc(string key, string fallback)
        {
            return Application.Current?.TryFindResource(key) as string ?? fallback;
        }

        /// <summary>
        /// Перестраивает список доступных фильтров. Этот тулбар и его фильтр относятся только к
        /// AssetsTree (System Files не имеет своего UI для фильтрации) - список фильтров всегда
        /// фиксированный: enum AssetResourceType, а не то, что физически есть на диске.
        /// Раньше список ошибочно переключался по IsSystemMode (раскрыт ли экспандер System Files)
        /// и подменялся на системные папки - тот же класс ошибки, что и в операциях с деревом.
        /// </summary>
        private void UpdateFiltersList()
        {
            _isUpdatingFilters = true;
            AvailableFilters.Clear();

            var settings = App.ProjectService.CurrentSettings;
            var activeDisabledFilters = settings.DisabledFilters;
            foreach (AssetResourceType type in System.Enum.GetValues(typeof(AssetResourceType)))
            {
                string matchKey = AssetResourceTypeRules.GetFolderName(type);
                string name = Loc(AssetResourceTypeRules.GetTitleKey(type), matchKey);
                bool isChecked = !activeDisabledFilters.Contains(matchKey);
                AvailableFilters.Add(new FilterItemViewModel(name, matchKey, isChecked, OnFilterCheckedChanged));
            }

            _isUpdatingFilters = false;
        }

        private void OnFilterCheckedChanged()
        {
            if (_isUpdatingFilters) return;

            var settings = App.ProjectService.CurrentSettings;

            var newDisabled = new List<string>();
            foreach (var filter in AvailableFilters)
            {
                if (!filter.IsChecked)
                {
                    newDisabled.Add(filter.MatchKey);
                }
            }

            // Этот фильтр относится только к AssetsTree (см. UpdateFiltersList).
            var activeDisabledFilters = settings.DisabledFilters;

            // Проверяем, изменился ли реальный список отключенных фильтров
            bool changed = false;
            if (newDisabled.Count != activeDisabledFilters.Count)
            {
                changed = true;
            }
            else
            {
                foreach (var item in newDisabled)
                {
                    if (!activeDisabledFilters.Contains(item))
                    {
                        changed = true;
                        break;
                    }
                }
            }

            if (changed)
            {
                activeDisabledFilters.Clear();
                activeDisabledFilters.AddRange(newDisabled);
                var mainVm = Application.Current?.MainWindow?.DataContext as MainWindowViewModel;
                if (mainVm?.CurrentProjectPath != null)
                {
                    App.ProjectService.SaveProject(mainVm.CurrentProjectPath, out _);
                }
                RefreshTree();
            }
        }

        /// <summary>
        /// Перестраивает файловое дерево на основе текущих настроек.
        /// </summary>
        public void RefreshTree()
        {
            var mainVm = Application.Current?.MainWindow?.DataContext as MainWindowViewModel;
            string? projectRoot = mainVm?.CurrentProjectPath;

            if (string.IsNullOrEmpty(projectRoot))
            {
                AssetRootNodes = new ObservableCollection<ProjectNode>();
                SystemRootNodes = new ObservableCollection<ProjectNode>();
                return;
            }

            var settings = App.ProjectService.CurrentSettings;

            // Загружаем ассеты (IsSystemMode = false). Список отключенных фильтров - свой,
            // не пересекается со списком для системного дерева (см. DisabledSystemFilters).
            var assetsTree = _vfs.BuildTree(projectRoot, false, settings.DisabledFilters);
            AssetRootNodes = new ObservableCollection<ProjectNode>(assetsTree);
            ApplySearchFilter(AssetRootNodes, SearchText);

            // Автогенерация RegisterAllScripts.cpp по списку найденных скриптов
            var scriptNodes = new List<ProjectNode>();
            FindScriptNodes(AssetRootNodes, scriptNodes);
            GenerateRegisterAllScripts(projectRoot, scriptNodes);

            // Загружаем системные файлы (IsSystemMode = true). Поиск и фильтр ассетов сюда не применяются.
            var systemTree = _vfs.BuildTree(projectRoot, true, settings.DisabledSystemFilters);
            SystemRootNodes = new ObservableCollection<ProjectNode>(systemTree);
        }


        // Пересчитывает IsSearchVisible по дереву ассетов: узел видим, если его имя содержит
        // искомую подстроку, либо видим хотя бы один из его потомков (чтобы не скрывать
        // папку, внутри которой есть совпадение). Вызывается только для AssetRootNodes -
        // узлы SystemRootNodes этим методом никогда не трогаются.
        private static bool ApplySearchFilter(ObservableCollection<ProjectNode> nodes, string searchText)
        {
            bool hasVisibleChild = false;
            string search = (searchText ?? string.Empty).Trim().ToLowerInvariant();

            foreach (var node in nodes)
            {
                bool childMatches = ApplySearchFilter(node.Children, search);
                bool selfMatches = search.Length == 0 || node.Name.ToLowerInvariant().Contains(search);
                node.IsSearchVisible = selfMatches || childMatches;

                if (node.IsSearchVisible)
                {
                    hasVisibleChild = true;
                }
            }

            return hasVisibleChild;
        }

        private void ResetCurrentFilters()
        {
            var settings = App.ProjectService.CurrentSettings;

            // Этот фильтр относится только к AssetsTree (см. UpdateFiltersList).
            List<string> currentModeMatchKeys = ((AssetResourceType[])System.Enum.GetValues(typeof(AssetResourceType)))
                .Select(AssetResourceTypeRules.GetFolderName)
                .ToList();
            List<string> activeDisabledFilters = settings.DisabledFilters;

            bool changed = false;
            foreach (var key in currentModeMatchKeys)
            {
                if (activeDisabledFilters.Contains(key))
                {
                    activeDisabledFilters.Remove(key);
                    changed = true;
                }
            }

            if (changed)
            {
                _isUpdatingFilters = true;
                foreach (var filter in AvailableFilters)
                {
                    if (currentModeMatchKeys.Contains(filter.MatchKey))
                    {
                        filter.IsChecked = true;
                    }
                }
                _isUpdatingFilters = false;

                var mainVm = Application.Current?.MainWindow?.DataContext as MainWindowViewModel;
                if (mainVm?.CurrentProjectPath != null)
                {
                    App.ProjectService.SaveProject(mainVm.CurrentProjectPath, out _);
                }
                RefreshTree();
            }
        }

        private void FindScriptNodes(System.Collections.ObjectModel.ObservableCollection<ProjectNode> nodes, List<ProjectNode> scriptNodes)
        {
            foreach (var node in nodes)
            {
                if (node.IsScript)
                {
                    scriptNodes.Add(node);
                }
                FindScriptNodes(node.Children, scriptNodes);
            }
        }

        private void GenerateRegisterAllScripts(string projectRoot, List<ProjectNode> scriptNodes)
        {
            // .editor/ — системная папка редактора, не папка ассетов пользователя.
            // RegisterAllScripts.cpp здесь рядом с CMakeLists.txt.
            string editorDir = Path.Combine(projectRoot, ".editor");
            string filePath = Path.Combine(editorDir, "RegisterAllScripts.cpp");

            // Два разных файла с одинаковым именем класса дали бы Register<>(name) с одним и тем же
            // ключом - в реестре движка вторая фабрика молча перетрёт первую (см. ScriptRegistry.h).
            // Такие классы исключаем из кодогена целиком и громко логируем, чтобы баг не маскировался.
            var duplicateGroups = scriptNodes
                .GroupBy(n => n.Name, StringComparer.Ordinal)
                .Where(g => g.Count() > 1)
                .ToList();

            foreach (var group in duplicateGroups)
            {
                string paths = string.Join(", ", group.Select(n => n.HppRelativePath));
                EditorLogger.LogError($"[Meta System] Duplicate script class name '{group.Key}' found in: {paths}. Rename one of them - skipping registration for both until resolved.");
            }

            var duplicateNames = new HashSet<string>(duplicateGroups.Select(g => g.Key), StringComparer.Ordinal);
            var validNodes = scriptNodes.Where(n => !duplicateNames.Contains(n.Name)).ToList();

            var sb = new System.Text.StringBuilder();
            sb.AppendLine("// RegisterAllScripts.cpp — generated automatically by ZzzEngine Editor");
            sb.AppendLine("#include <ScriptRegistry.h>");
            sb.AppendLine();

            // Добавляем инклуды для каждого скрипта
            foreach (var node in validNodes)
            {
                if (node.HasHpp && !string.IsNullOrEmpty(node.HppRelativePath))
                {
                    // Путь инклуда относительно .editor/ — Assets/ на уровень выше
                    string includePath = node.HppRelativePath;
                    if (includePath.StartsWith("Assets/", StringComparison.OrdinalIgnoreCase))
                    {
                        includePath = "../" + includePath;
                    }
                    sb.AppendLine($"#include \"{includePath}\"");
                }
            }

            sb.AppendLine();
            sb.AppendLine("extern \"C\" __declspec(dllexport) void RegisterAllScripts()");
            sb.AppendLine("{");

            foreach (var node in validNodes)
            {
                sb.AppendLine($"    zzz::script::ScriptRegistry::Register<{node.Name}>(\"{node.Name}\");");
            }

            sb.AppendLine("}");

            try
            {
                string content = sb.ToString();

                if (File.Exists(filePath))
                {
                    string oldContent = File.ReadAllText(filePath);
                    if (oldContent == content) return;
                }

                string dir = Path.GetDirectoryName(filePath) ?? "";
                if (!Directory.Exists(dir))
                {
                    Directory.CreateDirectory(dir);
                }

                File.WriteAllText(filePath, content);
                EditorLogger.LogInfo("Successfully generated RegisterAllScripts.cpp.");
            }
            catch (Exception ex)
            {
                EditorLogger.LogError($"Failed to generate RegisterAllScripts.cpp: {ex.Message}");
            }
        }
    }
}
