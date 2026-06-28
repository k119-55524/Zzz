using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using editor.Models;
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

        public FilterItemViewModel(string name, bool isChecked, Action onCheckedChanged)
        {
            Name = name;
            _isChecked = isChecked;
            _onCheckedChanged = onCheckedChanged;
        }

        public string Name { get; }

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
        private ObservableCollection<ProjectNode> _assetRootNodes = new();
        private ObservableCollection<ProjectNode> _systemRootNodes = new();
        private ObservableCollection<FilterItemViewModel> _availableFilters = new();
        private bool _isSystemMode;
        private bool _isUpdatingFilters;

        public AssetsViewModel() : base(WidgetType.Assets)
        {
            _vfs = new ProjectFileSystem(App.ProjectService.Storage);
            
            // Команда принудительного обновления дерева
            RefreshCommand = new RelayCommand(RefreshTree);
            ResetFiltersCommand = new RelayCommand(ResetCurrentFilters);
        }

        public ICommand ResetFiltersCommand { get; }

        public override void OnProjectOpened(string projectPath)
        {
            base.OnProjectOpened(projectPath);
            OnProjectChanged(projectPath);
        }

        public override void OnProjectClosed()
        {
            base.OnProjectClosed();
            OnProjectChanged(null);
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

        public ObservableCollection<FilterItemViewModel> AvailableFilters
        {
            get => _availableFilters;
            set => SetField(ref _availableFilters, value);
        }

        public bool IsSystemMode
        {
            get => _isSystemMode;
            set
            {
                if (SetField(ref _isSystemMode, value))
                {
                    if (App.ProjectService.CurrentSettings != null)
                    {
                        App.ProjectService.CurrentSettings.ShowSystemMode = value;
                        var mainVm = Application.Current?.MainWindow?.DataContext as MainWindowViewModel;
                        if (mainVm?.CurrentProjectPath != null)
                        {
                            App.ProjectService.SaveProject(mainVm.CurrentProjectPath, out _);
                        }
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
                return;
            }

            // Загружаем состояние сессии из настроек проекта
            var settings = App.ProjectService.CurrentSettings;
            if (settings != null)
            {
                _isSystemMode = settings.ShowSystemMode;
                OnPropertyChanged(nameof(IsSystemMode));
            }

            UpdateFiltersList();
            RefreshTree();
        }

        /// <summary>
        /// Перестраивает список доступных фильтров в зависимости от текущего режима.
        /// </summary>
        private void UpdateFiltersList()
        {
            _isUpdatingFilters = true;
            AvailableFilters.Clear();

            var settings = App.ProjectService.CurrentSettings;
            if (settings == null)
            {
                _isUpdatingFilters = false;
                return;
            }

            var folders = new List<ProjectFolderSchema>();
            if (IsSystemMode)
            {
                folders = ProjectStructure.SystemDirectories;
            }
            else
            {
                var mainVm = Application.Current?.MainWindow?.DataContext as MainWindowViewModel;
                string? projectRoot = mainVm?.CurrentProjectPath;
                if (!string.IsNullOrEmpty(projectRoot))
                {
                    string assetsRoot = Path.Combine(projectRoot, "Assets");
                    if (Directory.Exists(assetsRoot))
                    {
                        foreach (var dir in Directory.GetDirectories(assetsRoot))
                        {
                            string name = Path.GetFileName(dir);
                            folders.Add(new ProjectFolderSchema { RelativePath = $"Assets/{name}" });
                        }
                    }
                }
            }

            foreach (var folder in folders)
            {
                string name = Path.GetFileName(folder.RelativePath);
                bool isChecked = !settings.DisabledFilters.Contains(name);

                AvailableFilters.Add(new FilterItemViewModel(name, isChecked, OnFilterCheckedChanged));
            }

            _isUpdatingFilters = false;
        }

        private void OnFilterCheckedChanged()
        {
            if (_isUpdatingFilters) return;

            var settings = App.ProjectService.CurrentSettings;
            if (settings == null) return;

            var newDisabled = new List<string>();
            foreach (var filter in AvailableFilters)
            {
                if (!filter.IsChecked)
                {
                    newDisabled.Add(filter.Name);
                }
            }

            // Проверяем, изменился ли реальный список отключенных фильтров
            bool changed = false;
            if (newDisabled.Count != settings.DisabledFilters.Count)
            {
                changed = true;
            }
            else
            {
                foreach (var item in newDisabled)
                {
                    if (!settings.DisabledFilters.Contains(item))
                    {
                        changed = true;
                        break;
                    }
                }
            }

            if (changed)
            {
                settings.DisabledFilters.Clear();
                settings.DisabledFilters.AddRange(newDisabled);
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

            if (string.IsNullOrEmpty(projectRoot) || App.ProjectService.CurrentSettings == null)
            {
                AssetRootNodes = new ObservableCollection<ProjectNode>();
                SystemRootNodes = new ObservableCollection<ProjectNode>();
                return;
            }

            var settings = App.ProjectService.CurrentSettings;
            
            // Загружаем ассеты (IsSystemMode = false)
            var assetsTree = _vfs.BuildTree(projectRoot, false, settings.DisabledFilters);
            AssetRootNodes = new ObservableCollection<ProjectNode>(assetsTree);

            // Загружаем системные файлы (IsSystemMode = true)
            var systemTree = _vfs.BuildTree(projectRoot, true, settings.DisabledFilters);
            SystemRootNodes = new ObservableCollection<ProjectNode>(systemTree);
        }


        private void ResetCurrentFilters()
        {
            var settings = App.ProjectService.CurrentSettings;
            if (settings == null) return;

            var folders = new List<ProjectFolderSchema>();
            if (IsSystemMode)
            {
                folders = ProjectStructure.SystemDirectories;
            }
            else
            {
                var mainVm = Application.Current?.MainWindow?.DataContext as MainWindowViewModel;
                string? projectRoot = mainVm?.CurrentProjectPath;
                if (!string.IsNullOrEmpty(projectRoot))
                {
                    string assetsRoot = Path.Combine(projectRoot, "Assets");
                    if (Directory.Exists(assetsRoot))
                    {
                        foreach (var dir in Directory.GetDirectories(assetsRoot))
                        {
                            string name = Path.GetFileName(dir);
                            folders.Add(new ProjectFolderSchema { RelativePath = $"Assets/{name}" });
                        }
                    }
                }
            }

            var currentModeFolderNames = folders
                .Select(d => System.IO.Path.GetFileName(d.RelativePath) ?? string.Empty)
                .Where(n => n != string.Empty)
                .ToList();

            bool changed = false;
            foreach (var name in currentModeFolderNames)
            {
                if (settings.DisabledFilters.Contains(name))
                {
                    settings.DisabledFilters.Remove(name);
                    changed = true;
                }
            }

            if (changed)
            {
                _isUpdatingFilters = true;
                foreach (var filter in AvailableFilters)
                {
                    if (currentModeFolderNames.Contains(filter.Name))
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
    }
}
