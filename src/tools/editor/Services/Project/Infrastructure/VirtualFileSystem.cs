using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace editor.Services.Project.Infrastructure
{
    /// <summary>
    /// Представляет узел в виртуальной файловой системе проекта.
    /// </summary>
    public class VirtualNode : System.ComponentModel.INotifyPropertyChanged
    {
        private string _name = string.Empty;
        private string _relativePath = string.Empty;
        private bool _isFolder;
        private bool _isEmptyVirtual;
        private bool _isEditing;
        private List<VirtualNode> _children = new();

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
        /// Виртуальный относительный путь (например, "Player/Controller.cs").
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
        
        /// <summary>
        /// Флаг, указывающий, является ли папка пустой виртуальной (не существует на диске).
        /// </summary>
        public bool IsEmptyVirtual
        {
            get => _isEmptyVirtual;
            set
            {
                if (_isEmptyVirtual != value)
                {
                    _isEmptyVirtual = value;
                    OnPropertyChanged();
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
        
        public List<VirtualNode> Children
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
    /// Реализует логику слияния папок ресурсов и фильтрации типов файлов для виртуального проводника.
    /// </summary>
    public class VirtualFileSystem
    {
        private readonly IFileStorage _storage;

        public VirtualFileSystem(IFileStorage storage)
        {
            _storage = storage;
        }

        /// <summary>
        /// Сканирует проект и строит виртуальное дерево папок и файлов.
        /// </summary>
        /// <param name="projectRoot">Физический корень проекта.</param>
        /// <param name="showSystemMode">Режим отображения: true - системные файлы/папки (без Assets), false - режим ассетов.</param>
        /// <param name="disabledFilters">Список отключенных фильтров (названия папок типов ресурсов, например "Scripts").</param>
        /// <param name="emptyFolders">Список виртуальных пустых папок для восстановления.</param>
        /// <returns>Список корневых узлов дерева.</returns>
        public List<VirtualNode> BuildTree(
            string projectRoot, 
            bool showSystemMode, 
            List<string> disabledFilters, 
            List<string> emptyFolders)
        {
            var roots = new List<VirtualNode>();

            if (showSystemMode)
            {
                // Системный режим: сканируем корень проекта, исключая папку Assets
                if (!_storage.DirectoryExists(projectRoot))
                    return roots;

                var entries = _storage.GetFileSystemEntries(projectRoot);
                foreach (var entry in entries)
                {
                    string name = Path.GetFileName(entry);
                    
                    // Игнорируем технические директории IDE/VCS и папку ассетов
                    if (name.Equals("Assets", StringComparison.OrdinalIgnoreCase) ||
                        name.Equals(".git", StringComparison.OrdinalIgnoreCase) ||
                        name.Equals(".vs", StringComparison.OrdinalIgnoreCase) ||
                        name.Equals("bin", StringComparison.OrdinalIgnoreCase) ||
                        name.Equals("obj", StringComparison.OrdinalIgnoreCase) ||
                        name.Equals(".idea", StringComparison.OrdinalIgnoreCase))
                    {
                        continue;
                    }

                    bool isDir = _storage.DirectoryExists(entry);
                    if (isDir && disabledFilters.Contains(name))
                    {
                        continue; // Фильтрация системных папок (например, Configs)
                    }

                    var node = new VirtualNode
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
                // Режим ассетов: объединяем подпапки внутри Assets/
                string assetsRoot = Path.Combine(projectRoot, "Assets");
                if (!_storage.DirectoryExists(assetsRoot))
                    return roots;

                // Временный плоский список всех виртуальных папок для слияния
                var virtualFolders = new Dictionary<string, VirtualNode>();

                // 1. Сканируем физические папки типов ресурсов (например, Assets/Scripts)
                var resourceDirs = _storage.GetFileSystemEntries(assetsRoot);
                foreach (var resDir in resourceDirs)
                {
                    if (!_storage.DirectoryExists(resDir))
                        continue;

                    string resTypeName = Path.GetFileName(resDir);
                    if (disabledFilters.Contains(resTypeName))
                        continue; // Фильтр отключил отображение этого типа ресурса

                    // Сканируем содержимое этой папки ресурсов и добавляем в виртуальное дерево
                    ScanAndMergeResourceDir(resDir, resDir, virtualFolders);
                }

                // 2. Добавляем сохраненные пустые виртуальные папки
                foreach (var emptyPath in emptyFolders)
                {
                    string cleanPath = emptyPath.Replace('\\', '/').Trim('/');
                    if (string.IsNullOrEmpty(cleanPath)) continue;

                    EnsureVirtualFolder(cleanPath, virtualFolders, isVirtualOnly: true);
                }

                // 3. Строим древовидную структуру из плоского словаря виртуальных папок
                var folderNodes = virtualFolders.Values.ToList();
                foreach (var folder in folderNodes)
                {
                    string parentPath = GetParentVirtualPath(folder.RelativePath);
                    if (string.IsNullOrEmpty(parentPath))
                    {
                        // Корневая виртуальная папка
                        roots.Add(folder);
                    }
                    else
                    {
                        if (virtualFolders.TryGetValue(parentPath, out var parentNode))
                        {
                            if (!parentNode.Children.Any(c => c.RelativePath == folder.RelativePath))
                            {
                                parentNode.Children.Add(folder);
                            }
                        }
                    }
                }

                // Сортируем: сначала папки, затем файлы
                SortNodeChildren(roots);
            }

            return roots;
        }

        private void ScanDirectoryPhysical(string rootPath, string currentPath, VirtualNode parentNode, List<string> disabledFilters)
        {
            var entries = _storage.GetFileSystemEntries(currentPath);
            foreach (var entry in entries)
            {
                string name = Path.GetFileName(entry);
                bool isDir = _storage.DirectoryExists(entry);
                string relPath = Path.GetRelativePath(rootPath, entry).Replace('\\', '/');

                var node = new VirtualNode
                {
                    Name = name,
                    RelativePath = relPath,
                    IsFolder = isDir
                };

                if (isDir)
                {
                    ScanDirectoryPhysical(rootPath, entry, node, disabledFilters);
                }

                parentNode.Children.Add(node);
            }
        }

        private void ScanAndMergeResourceDir(string resourceRoot, string currentPath, Dictionary<string, VirtualNode> virtualFolders)
        {
            var entries = _storage.GetFileSystemEntries(currentPath);
            foreach (var entry in entries)
            {
                string name = Path.GetFileName(entry);
                bool isDir = _storage.DirectoryExists(entry);
                
                // Получаем относительный путь от корня конкретного ресурса (например, "Character/Player.cs")
                string virtualRelPath = Path.GetRelativePath(resourceRoot, entry).Replace('\\', '/');

                if (isDir)
                {
                    // Гарантируем наличие виртуальной папки
                    var folderNode = EnsureVirtualFolder(virtualRelPath, virtualFolders, isVirtualOnly: false);
                    ScanAndMergeResourceDir(resourceRoot, entry, virtualFolders);
                }
                else
                {
                    // Это файл, добавляем его в родительскую виртуальную папку
                    string parentVirtualPath = GetParentVirtualPath(virtualRelPath);
                    var fileNode = new VirtualNode
                    {
                        Name = name,
                        RelativePath = virtualRelPath,
                        IsFolder = false
                    };

                    if (string.IsNullOrEmpty(parentVirtualPath))
                    {
                        // Файл лежит прямо в виртуальном корне (т.е. лежал в Assets/Scripts/File.cs)
                        // Временно храним файлы корня в специальном списке или привяжем позже
                    }
                    else
                    {
                        var parentFolder = EnsureVirtualFolder(parentVirtualPath, virtualFolders, isVirtualOnly: false);
                        parentFolder.Children.Add(fileNode);
                    }
                }
            }
        }

        private VirtualNode EnsureVirtualFolder(string virtualPath, Dictionary<string, VirtualNode> virtualFolders, bool isVirtualOnly)
        {
            if (virtualFolders.TryGetValue(virtualPath, out var existing))
            {
                if (!isVirtualOnly)
                {
                    existing.IsEmptyVirtual = false; // Папка реально существует на диске хотя бы в одном типе ресурсов
                }
                return existing;
            }

            // Создаем папки вверх по иерархии
            string parentPath = GetParentVirtualPath(virtualPath);
            if (!string.IsNullOrEmpty(parentPath))
            {
                EnsureVirtualFolder(parentPath, virtualFolders, isVirtualOnly);
            }

            var node = new VirtualNode
            {
                Name = Path.GetFileName(virtualPath),
                RelativePath = virtualPath,
                IsFolder = true,
                IsEmptyVirtual = isVirtualOnly
            };

            virtualFolders[virtualPath] = node;
            return node;
        }

        private string GetParentVirtualPath(string virtualPath)
        {
            int idx = virtualPath.LastIndexOf('/');
            if (idx == -1) return string.Empty;
            return virtualPath.Substring(0, idx);
        }

        private void SortNodeChildren(List<VirtualNode> nodes)
        {
            foreach (var node in nodes)
            {
                if (node.Children.Count > 0)
                {
                    SortNodeChildren(node.Children);
                    node.Children = node.Children
                        .OrderByDescending(c => c.IsFolder)
                        .ThenBy(c => c.Name)
                        .ToList();
                }
            }
        }
    }
}
