using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using editor.Services;
using editor.Services.Project;

namespace editor.Services.Project.Infrastructure
{
    /// <summary>
    /// Представляет узел в файловой системе проекта.
    /// </summary>
    public class ProjectNode : System.ComponentModel.INotifyPropertyChanged
    {
        private string _name = string.Empty;
        private string _relativePath = string.Empty;
        private bool _isFolder;
        private bool _isEditing;
        private bool _isSearchVisible = true;
        private System.Collections.ObjectModel.ObservableCollection<ProjectNode> _children = new();

        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged();
                    OnPropertyChanged(nameof(DisplayName));
                }
            }
        }
        
        /// <summary>
        /// Относительный путь (например, "Assets/Player/Controller.cs").
        /// </summary>
        public string RelativePath
        {
            get => _relativePath;
            set
            {
                if (_relativePath != value)
                {
                    _relativePath = value;
                    OnPropertyChanged();
                }
            }
        }
        
        public bool IsFolder
        {
            get => _isFolder;
            set
            {
                if (_isFolder != value)
                {
                    _isFolder = value;
                    OnPropertyChanged();
                    OnPropertyChanged(nameof(DisplayName));
                }
            }
        }
        
        public bool IsEditing
        {
            get => _isEditing;
            set
            {
                if (_isEditing != value)
                {
                    _isEditing = value;
                    OnPropertyChanged();
                }
            }
        }
        
        /// <summary>
        /// Видимость узла в дереве при активном текстовом поиске (см. AssetsViewModel.SearchText).
        /// Используется только для AssetsTree - на узлы SystemTree не влияет и никогда не пересчитывается.
        /// </summary>
        public bool IsSearchVisible
        {
            get => _isSearchVisible;
            set
            {
                if (_isSearchVisible != value)
                {
                    _isSearchVisible = value;
                    OnPropertyChanged();
                }
            }
        }

        private bool _isScript;
        private bool _hasHpp;
        private bool _hasCpp;
        private bool _hasMeta;
        private string _hppRelativePath = string.Empty;
        private string _cppRelativePath = string.Empty;
        private string _metaRelativePath = string.Empty;

        public bool IsScript
        {
            get => _isScript;
            set
            {
                if (_isScript != value)
                {
                    _isScript = value;
                    OnPropertyChanged();
                }
            }
        }

        public bool HasHpp
        {
            get => _hasHpp;
            set
            {
                if (_hasHpp != value)
                {
                    _hasHpp = value;
                    OnPropertyChanged();
                }
            }
        }

        public bool HasCpp
        {
            get => _hasCpp;
            set
            {
                if (_hasCpp != value)
                {
                    _hasCpp = value;
                    OnPropertyChanged();
                }
            }
        }

        public string HppRelativePath
        {
            get => _hppRelativePath;
            set
            {
                if (_hppRelativePath != value)
                {
                    _hppRelativePath = value;
                    OnPropertyChanged();
                }
            }
        }

        public string CppRelativePath
        {
            get => _cppRelativePath;
            set
            {
                if (_cppRelativePath != value)
                {
                    _cppRelativePath = value;
                    OnPropertyChanged();
                }
            }
        }

        public bool HasMeta
        {
            get => _hasMeta;
            set
            {
                if (_hasMeta != value)
                {
                    _hasMeta = value;
                    OnPropertyChanged();
                }
            }
        }

        public string MetaRelativePath
        {
            get => _metaRelativePath;
            set
            {
                if (_metaRelativePath != value)
                {
                    _metaRelativePath = value;
                    OnPropertyChanged();
                }
            }
        }

        public System.Collections.ObjectModel.ObservableCollection<ProjectNode> Children
        {
            get => _children;
            set
            {
                if (_children != value)
                {
                    _children = value;
                    OnPropertyChanged();
                }
            }
        }

        public string DisplayName => IsFolder ? Name : System.IO.Path.GetFileNameWithoutExtension(Name);

        public event System.ComponentModel.PropertyChangedEventHandler? PropertyChanged;

        protected void OnPropertyChanged([System.Runtime.CompilerServices.CallerMemberName] string? propertyName = null)
        {
            PropertyChanged?.Invoke(this, new System.ComponentModel.PropertyChangedEventArgs(propertyName));
        }
    }

    /// <summary>
    /// Реализует логику сканирования физического каталога проекта.
    /// </summary>
    public class ProjectFileSystem
    {
        private readonly IFileStorage _storage;

        public ProjectFileSystem(IFileStorage storage)
        {
            _storage = storage;
        }

        /// <summary>
        /// Сканирует проект и строит дерево папок и файлов.
        /// </summary>
        /// <param name="projectRoot">Физический корень проекта.</param>
        /// <param name="showSystemMode">Режим отображения: true - системные файлы/папки (без Assets), false - режим ассетов.</param>
        /// <param name="disabledFilters">Список отключенных фильтров (названия папок).</param>
        /// <returns>Список корневых узлов дерева.</returns>
        public List<ProjectNode> BuildTree(
            string projectRoot,
            bool showSystemMode,
            List<string> disabledFilters)
        {
            var roots = new List<ProjectNode>();

            if (showSystemMode)
            {
                // Системный режим: сканируем корень проекта, исключая папку Assets
                if (!_storage.DirectoryExists(projectRoot))
                    return roots;

                var entries = _storage.GetFileSystemEntries(projectRoot);
                foreach (var entry in entries)
                {
                    string name = Path.GetFileName(entry);
                    
                    // Игнорируем скрытые файлы/папки (начинающиеся с точки), папку ассетов и технические папки сборки
                    if (name.StartsWith(".") ||
                        name.Equals("Assets", StringComparison.OrdinalIgnoreCase) ||
                        name.Equals("bin", StringComparison.OrdinalIgnoreCase) ||
                        name.Equals("obj", StringComparison.OrdinalIgnoreCase))
                    {
                        continue;
                    }

                    bool isDir = _storage.DirectoryExists(entry);
                    if (isDir && disabledFilters.Contains(name))
                    {
                        continue; // Фильтрация системных папок
                    }

                    var node = new ProjectNode
                    {
                        Name = name,
                        RelativePath = name,
                        IsFolder = isDir
                    };

                    if (isDir)
                    {
                        ScanDirectoryPhysical(projectRoot, entry, node, disabledFilters);
                    }

                    roots.Add(node);
                }
            }
            else
            {
                // Режим ассетов: сканируем физическую папку Assets с группировкой скриптов
                string assetsRoot = Path.Combine(projectRoot, "Assets");
                roots = ProcessDirectoryEntries(projectRoot, assetsRoot, disabledFilters);
            }

            SortNodeChildren(roots);
            return roots;
        }

        private List<ProjectNode> ProcessDirectoryEntries(string rootPath, string currentPath, List<string> disabledFilters)
        {
            var nodes = new List<ProjectNode>();
            if (!_storage.DirectoryExists(currentPath))
                return nodes;

            var entries = _storage.GetFileSystemEntries(currentPath);
            
            // Разделяем папки и файлы
            var dirPaths = new List<string>();
            var filePaths = new List<string>();
            foreach (var entry in entries)
            {
                if (_storage.DirectoryExists(entry))
                    dirPaths.Add(entry);
                else
                    filePaths.Add(entry);
            }

            // 1. Обрабатываем папки
            foreach (var dirPath in dirPaths)
            {
                string name = Path.GetFileName(dirPath);
                if (disabledFilters != null && disabledFilters.Contains(name))
                    continue;

                string relPath = Path.GetRelativePath(rootPath, dirPath).Replace('\\', '/');
                var node = new ProjectNode
                {
                    Name = name,
                    RelativePath = relPath,
                    IsFolder = true
                };

                // Рекурсивно сканируем
                var children = ProcessDirectoryEntries(rootPath, dirPath, disabledFilters);
                foreach (var child in children)
                {
                    node.Children.Add(child);
                }

                nodes.Add(node);
            }

            // 2. Обрабатываем файлы и группируем скрипты
            var processedFiles = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            foreach (var filePath in filePaths)
            {
                if (processedFiles.Contains(filePath))
                    continue;

                string ext = Path.GetExtension(filePath).ToLower();
                string name = Path.GetFileName(filePath);
                string relPath = Path.GetRelativePath(rootPath, filePath).Replace('\\', '/');


                if (ext == ".hpp")
                {
                    // Проверяем наличие .cpp и .meta в той же папке
                    string baseName = Path.GetFileNameWithoutExtension(filePath);
                    string dir = Path.GetDirectoryName(filePath) ?? "";
                    string cppPath = Path.Combine(dir, baseName + ".cpp");
                    string metaPath = Path.Combine(dir, baseName + ".meta");

                    bool hasCpp = filePaths.Contains(cppPath, StringComparer.OrdinalIgnoreCase);
                    bool hasMeta = filePaths.Contains(metaPath, StringComparer.OrdinalIgnoreCase);

                    var scriptNode = new ProjectNode
                    {
                        Name = baseName,
                        RelativePath = relPath,
                        IsFolder = false,
                        IsScript = true,
                        HasHpp = true,
                        HppRelativePath = relPath,
                        HasCpp = hasCpp,
                        CppRelativePath = hasCpp ? Path.GetRelativePath(rootPath, cppPath).Replace('\\', '/') : string.Empty,
                        HasMeta = hasMeta,
                        MetaRelativePath = hasMeta ? Path.GetRelativePath(rootPath, metaPath).Replace('\\', '/') : string.Empty
                    };

                    processedFiles.Add(filePath);
                    if (hasCpp)
                    {
                        processedFiles.Add(cppPath);
                    }
                    if (hasMeta)
                    {
                        processedFiles.Add(metaPath);
                    }

                    nodes.Add(scriptNode);
                }
            }

            // 3. Обрабатываем все остальные файлы (.meta всегда скрыты - это служебные данные редактора,
            // не ассет; включая "осиротевшие" .meta без пары .hpp - их подчищает SyncScriptMetaFiles)
            foreach (var filePath in filePaths)
            {
                if (processedFiles.Contains(filePath))
                    continue;

                string ext = Path.GetExtension(filePath).ToLower();
                if (ext == ".meta")
                    continue;

                string name = Path.GetFileName(filePath);
                string relPath = Path.GetRelativePath(rootPath, filePath).Replace('\\', '/');

                var node = new ProjectNode
                {
                    Name = name,
                    RelativePath = relPath,
                    IsFolder = false
                };

                nodes.Add(node);
            }

            return nodes;
        }

        private void ScanDirectoryPhysical(string rootPath, string currentPath, ProjectNode parentNode, List<string> disabledFilters)
        {
            var children = ProcessDirectoryEntries(rootPath, currentPath, disabledFilters);
            foreach (var child in children)
            {
                parentNode.Children.Add(child);
            }
        }

        private void SortNodeChildren(System.Collections.Generic.IList<ProjectNode> nodes)
        {
            for (int i = 0; i < nodes.Count; i++)
            {
                var node = nodes[i];
                if (node.Children.Count > 0)
                {
                    SortNodeChildren(node.Children);
                    var sorted = node.Children
                        .OrderByDescending(c => c.IsFolder)
                        .ThenBy(c => c.Name)
                        .ToList();
                    node.Children = new System.Collections.ObjectModel.ObservableCollection<ProjectNode>(sorted);
                }
            }
        }

        /// <summary>
        /// Полная синхронизация .meta файлов скриптов с диском: генерирует .meta для .hpp без пары
        /// и удаляет "осиротевшие" .meta без .hpp. Вызывается только при открытии/смене проекта
        /// (не на каждый RefreshTree) - это единственное место, где скан мутирует диск.
        /// </summary>
        public void SyncScriptMetaFiles(string projectRoot)
        {
            string assetsRoot = Path.Combine(projectRoot, "Assets");
            if (!_storage.DirectoryExists(assetsRoot))
                return;

            var guidToPaths = new Dictionary<string, List<string>>(StringComparer.OrdinalIgnoreCase);
            SyncScriptMetaFilesRecursive(assetsRoot, guidToPaths);

            // Сканер коллизий GUID сознательно пока не обрабатывает найденные дубли (см. обсуждение
            // архитектуры) - точка интеграции уже на месте, чтобы не искать её, когда дойдут руки.
            GuidCollisionScanner.Scan(guidToPaths);
        }

        private void SyncScriptMetaFilesRecursive(string currentPath, Dictionary<string, List<string>> guidToPaths)
        {
            if (!_storage.DirectoryExists(currentPath))
                return;

            var entries = _storage.GetFileSystemEntries(currentPath);
            var filePaths = entries.Where(e => !_storage.DirectoryExists(e)).ToList();

            foreach (var hppPath in filePaths.Where(f => Path.GetExtension(f).Equals(".hpp", StringComparison.OrdinalIgnoreCase)))
            {
                string baseName = Path.GetFileNameWithoutExtension(hppPath);
                string dir = Path.GetDirectoryName(hppPath) ?? currentPath;
                string metaPath = Path.Combine(dir, baseName + ".meta");

                if (!_storage.FileExists(metaPath))
                {
                    var data = ScriptMetaFile.CreateNew(baseName);
                    ScriptMetaFile.Save(_storage, metaPath, data);
                    EditorLogger.LogInfo($"[Meta System] Generated missing meta file '{baseName}.meta' for '{baseName}.hpp' (GUID: {data.Guid}).");
                    AddGuid(guidToPaths, data.Guid, metaPath);
                }
                else
                {
                    var data = ScriptMetaFile.Load(_storage, metaPath);
                    if (data != null)
                    {
                        AddGuid(guidToPaths, data.Guid, metaPath);
                    }
                }
            }

            foreach (var metaPath in filePaths.Where(f => Path.GetExtension(f).Equals(".meta", StringComparison.OrdinalIgnoreCase)))
            {
                string baseName = Path.GetFileNameWithoutExtension(metaPath);
                string dir = Path.GetDirectoryName(metaPath) ?? currentPath;
                string hppPath = Path.Combine(dir, baseName + ".hpp");

                if (!_storage.FileExists(hppPath))
                {
                    _storage.DeleteFile(metaPath);
                    EditorLogger.LogInfo($"[Meta System] Removed orphan meta file '{baseName}.meta' (no matching '{baseName}.hpp').");
                }
            }

            foreach (var dirPath in entries.Where(e => _storage.DirectoryExists(e)))
            {
                SyncScriptMetaFilesRecursive(dirPath, guidToPaths);
            }
        }

        private static void AddGuid(Dictionary<string, List<string>> guidToPaths, string guid, string path)
        {
            if (string.IsNullOrEmpty(guid))
                return;

            if (!guidToPaths.TryGetValue(guid, out var paths))
            {
                paths = new List<string>();
                guidToPaths[guid] = paths;
            }
            paths.Add(path);
        }
    }
}
