using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
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
                    
                    // Игнорируем технические директории IDE/VCS, бэкапы редактора и папку ассетов
                    if (name.Equals("Assets", StringComparison.OrdinalIgnoreCase) ||
                        name.Equals(".git", StringComparison.OrdinalIgnoreCase) ||
                        name.Equals(".editor", StringComparison.OrdinalIgnoreCase) ||
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
                // Режим ассетов: просто сканируем физическую папку Assets
                string assetsRoot = Path.Combine(projectRoot, "Assets");
                if (!_storage.DirectoryExists(assetsRoot))
                    return roots;

                var entries = _storage.GetFileSystemEntries(assetsRoot);
                foreach (var entry in entries)
                {
                    string name = Path.GetFileName(entry);
                    bool isDir = _storage.DirectoryExists(entry);
                    string relPath = Path.GetRelativePath(projectRoot, entry).Replace('\\', '/');

                    if (isDir && disabledFilters.Contains(name))
                    {
                        continue; // Фильтрация папок первого уровня в Assets
                    }

                    var node = new ProjectNode
                    {
                        Name = name,
                        RelativePath = relPath,
                        IsFolder = isDir
                    };

                    if (isDir)
                    {
                        ScanDirectoryPhysical(projectRoot, entry, node, disabledFilters);
                    }

                    roots.Add(node);
                }
            }

            SortNodeChildren(roots);
            return roots;
        }

        private void ScanDirectoryPhysical(string rootPath, string currentPath, ProjectNode parentNode, List<string> disabledFilters)
        {
            var entries = _storage.GetFileSystemEntries(currentPath);
            foreach (var entry in entries)
            {
                string name = Path.GetFileName(entry);
                bool isDir = _storage.DirectoryExists(entry);
                string relPath = Path.GetRelativePath(rootPath, entry).Replace('\\', '/');

                var node = new ProjectNode
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
    }
}
