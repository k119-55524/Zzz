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
        private readonly VirtualFileSystem _vfs;
        private ObservableCollection<VirtualNode> _virtualRootNodes = new();
        private ObservableCollection<FilterItemViewModel> _availableFilters = new();
        private bool _isSystemMode;
        private bool _isUpdatingFilters;

        public AssetsViewModel() : base(WidgetType.Assets)
        {
            _vfs = new VirtualFileSystem(App.ProjectService.Storage);
            
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

        public ObservableCollection<VirtualNode> VirtualRootNodes
        {
            get => _virtualRootNodes;
            set => SetField(ref _virtualRootNodes, value);
        }

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
                    UpdateFiltersList();
                    RefreshTree();
                }
            }
        }

        private void OnProjectChanged(string? projectPath)
        {
            if (string.IsNullOrEmpty(projectPath))
            {
                VirtualRootNodes.Clear();
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

            // Выбираем обязательные папки для текущего режима
            var folders = IsSystemMode ? ProjectStructure.SystemDirectories : ProjectStructure.AssetDirectories;

            foreach (var folder in folders)
            {
                // Имя типа ресурса/папки — это последнее имя в пути (например, Scripts)
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
        /// Перестраивает виртуальное файловое дерево на основе текущих настроек.
        /// </summary>
        public void RefreshTree()
        {
            var mainVm = Application.Current?.MainWindow?.DataContext as MainWindowViewModel;
            string? projectRoot = mainVm?.CurrentProjectPath;

            if (string.IsNullOrEmpty(projectRoot) || App.ProjectService.CurrentSettings == null)
            {
                VirtualRootNodes = new ObservableCollection<VirtualNode>();
                return;
            }

            var settings = App.ProjectService.CurrentSettings;
            var tree = _vfs.BuildTree(
                projectRoot,
                IsSystemMode,
                settings.DisabledFilters);

            VirtualRootNodes = new ObservableCollection<VirtualNode>(tree);
        }

        private void NotifyDirty()
        {
            var mainVm = Application.Current?.MainWindow?.DataContext as MainWindowViewModel;
            if (mainVm != null)
            {
                mainVm.IsDirty = true;
            }
        }

        private void ResetCurrentFilters()
        {
            var settings = App.ProjectService.CurrentSettings;
            if (settings == null) return;

            var folders = IsSystemMode ? ProjectStructure.SystemDirectories : ProjectStructure.AssetDirectories;
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
