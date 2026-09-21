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
using editor.Services.Project.Assets;
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
            App.ProjectFileWatcherService.FileChanged += OnProjectFileChanged;
            App.ScriptAssetIndexService.Changed += OnScriptAssetIndexChanged;
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

            App.ScriptAssetIndexService.OpenProject(projectPath);
            App.ProjectFileWatcherService.OpenProject(projectPath);

            OnProjectChanged(projectPath);

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
            App.ProjectFileWatcherService.CloseProject();
            App.ScriptAssetIndexService.CloseProject();
            _compileDebounceTimer?.Stop();
            _compileDebounceTimer = null;
            base.OnProjectClosed();
            OnProjectChanged(null);
        }

        private void OnProjectFileChanged(object? sender, ProjectFileChangedEventArgs e)
        {
            Application.Current?.Dispatcher.BeginInvoke(() => RefreshTree());
        }

        private void OnScriptAssetIndexChanged(object? sender, ScriptAssetIndexChangedEventArgs e)
        {
            Application.Current?.Dispatcher.BeginInvoke(() =>
            {
                RefreshTree();
                if (e.AffectsCompilation)
                {
                    ScheduleCompileDebounced();
                }
            });
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
                    {
                        if (mw.IsActive)
                        {
                            _ = mw.CheckAndCompileScriptsAsync(forceRebuild: true);
                        }
                        else if (mw.DataContext is MainWindowViewModel vm && vm.CurrentProjectPath != null)
                        {
                            ScriptRebuildCoordinator.RequestRebuild(vm.CurrentProjectPath);
                        }
                    }
                };
            }

            // Перезапускаем таймер — каждый новый Changed сдвигает окно на 500ms
            _compileDebounceTimer.Stop();
            _compileDebounceTimer.Start();
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

            // Два разных файла с одинаковым полным именем класса дали бы Register<>(name) с одним и тем же
            // ключом - в реестре движка вторая фабрика молча перетрёт первую (см. ScriptRegistry.h).
            // Такие классы исключаем из кодогена целиком и громко логируем, чтобы баг не маскировался.
            var duplicateGroups = scriptNodes
                .GroupBy(n => n.QualifiedName, StringComparer.Ordinal)
                .Where(g => g.Count() > 1)
                .ToList();

            foreach (var group in duplicateGroups)
            {
                string paths = string.Join(", ", group.Select(n => n.HppRelativePath));
                EditorLogger.LogError($"[Meta System] Дублирующееся полное имя класса скрипта '{group.Key}' найдено в: {paths}. Измените namespace или имя одного из них - регистрация обоих пропущена до разрешения конфликта.");
            }

            var duplicateNames = new HashSet<string>(duplicateGroups.Select(g => g.Key), StringComparer.Ordinal);
            var validNodes = scriptNodes.Where(n => !duplicateNames.Contains(n.QualifiedName)).ToList();

            var sb = new System.Text.StringBuilder();
            sb.AppendLine("// RegisterAllScripts.cpp — генерируется автоматически ZzzEngine Editor");
            sb.AppendLine("#include <core/Core.h>");
            sb.AppendLine("#include <ScriptRegistry.h>");
            sb.AppendLine("#include <core/logger/logger.h>");
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
            sb.AppendLine("extern \"C\" __declspec(dllexport) void InitScriptLogger(void* callback)");
            sb.AppendLine("{");
            sb.AppendLine("    static bool initialized = false;");
            sb.AppendLine("    if (initialized || callback == nullptr)");
            sb.AppendLine("        return;");
            sb.AppendLine();
            sb.AppendLine("    initialized = true;");
            sb.AppendLine("    zzz::logger::g_Logger.AddCallbackBroadcaster((zzz::logger::LogCallback)callback);");
            sb.AppendLine("}");

            // Вызывается редактором перед FreeLibrary() этого модуля. AddCallbackBroadcaster
            // выше запускает фоновый поток рассылки логов внутри самой scripts.dll (свой
            // g_Logger, т.к. logger_lib линкуется статически) - без явной остановки потока
            // здесь деструктор g_Logger попытается его join() уже во время
            // DllMain(DLL_PROCESS_DETACH), а это гарантированный deadlock на loader lock.
            sb.AppendLine();
            sb.AppendLine("extern \"C\" __declspec(dllexport) void ShutdownScriptLogger()");
            sb.AppendLine("{");
            sb.AppendLine("    zzz::logger::g_Logger.StopBroadcastThread();");
            sb.AppendLine("}");

            sb.AppendLine();
            sb.AppendLine("extern \"C\" __declspec(dllexport) void RegisterAllScripts(zzz::core::ScriptRegistry& registry)");
            sb.AppendLine("{");

            foreach (var node in validNodes)
            {
                if (!string.IsNullOrWhiteSpace(node.Guid) && System.Guid.TryParse(node.Guid, out _))
                {
                    sb.AppendLine($"    if (auto g = zzz::core::Guid::Parse(\"{node.Guid}\"))");
                    sb.AppendLine($"        registrar.Register<{node.QualifiedName}>(\"{node.QualifiedName}\", *g);");
                    sb.AppendLine("    else");
                    sb.AppendLine($"        registrar.Register<{node.QualifiedName}>(\"{node.QualifiedName}\");");
                }
                else
                {
                    sb.AppendLine($"    registrar.Register<{node.QualifiedName}>(\"{node.QualifiedName}\");");
                }
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
                EditorLogger.LogInfo("RegisterAllScripts.cpp успешно сгенерирован.");
            }
            catch (Exception ex)
            {
                EditorLogger.LogError($"Не удалось сгенерировать RegisterAllScripts.cpp: {ex.Message}");
            }
        }
    }
}
