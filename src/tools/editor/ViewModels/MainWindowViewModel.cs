using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Input;
using editor.Models;
using editor.Services;

namespace editor.ViewModels
{
    public class MainWindowViewModel : ViewModelBase
    {
        private readonly IDialogService _dialogService;
        private GlobalSessionState _globalState = new();

        private bool _isDirty;
        private string? _currentProjectPath;

        public MainWindowViewModel(IDialogService dialogService)
        {
            _dialogService = dialogService;

            Panes = new ObservableCollection<PaneViewModel>(
                Enum.GetValues(typeof(WidgetType))
                    .Cast<WidgetType>()
                    .Select(type => new PaneViewModel(type)));

            RenderPane = Panes.First(p => p.Type == WidgetType.Render);
            RecentProjects = new ObservableCollection<string>();

            NewProjectCommand = new RelayCommand(NewProject);
            OpenProjectCommand = new RelayCommand(OpenProject);
            OpenRecentProjectCommand = new RelayCommand<string>(OpenRecentProject);
            ExitCommand = new RelayCommand(() => CloseRequested?.Invoke(this, EventArgs.Empty));
            AboutCommand = new RelayCommand(_dialogService.ShowAbout);
            UndoCommand = new RelayCommand(() => { /* Логика отмены действия (Undo) будет реализована позже */ }, () => IsDirty);
            RedoCommand = new RelayCommand(() => { /* Логика повтора действия (Redo) будет реализована позже */ }, () => IsDirty);
            PlayCommand = new RelayCommand(() => { /* Запуск симуляции */ });
            PauseCommand = new RelayCommand(() => { /* Пауза симуляции */ });
            StopCommand = new RelayCommand(() => { /* Остановка симуляции */ });
            ShowWidgetCommand = new RelayCommand<PaneViewModel>(pane =>
            {
                if (pane != null)
                {
                    ShowWidgetRequested?.Invoke(this, pane);
                }
            });
            ResetLayoutCommand = new RelayCommand(ResetLayout);
        }

        public ObservableCollection<PaneViewModel> Panes { get; }

        public PaneViewModel RenderPane { get; }

        public ObservableCollection<string> RecentProjects { get; }

        public bool HasRecentProjects => RecentProjects.Count > 0;

        public string AppName => EditorConstants.ApplicationName;

        public string ProjectDisplayName
        {
            get
            {
                string noProjectText = Application.Current?.TryFindResource("Menu_File_NoProject") as string ?? "Нет проекта";
                return string.IsNullOrEmpty(_currentProjectPath)
                    ? noProjectText
                    : Path.GetFileNameWithoutExtension(_currentProjectPath);
            }
        }

        public bool IsDirty
        {
            get => _isDirty;
            set
            {
                if (SetField(ref _isDirty, value))
                {
                    OnPropertyChanged(nameof(WindowTitle));
                    CommandManager.InvalidateRequerySuggested();
                }
            }
        }

        public string WindowTitle => $"{AppName} - [{ProjectDisplayName}]{(IsDirty ? "*" : "")}";

        public ICommand NewProjectCommand { get; }
        public ICommand OpenProjectCommand { get; }
        public ICommand OpenRecentProjectCommand { get; }
        public ICommand ExitCommand { get; }
        public ICommand AboutCommand { get; }
        public ICommand UndoCommand { get; }
        public ICommand RedoCommand { get; }
        public ICommand PlayCommand { get; }
        public ICommand PauseCommand { get; }
        public ICommand StopCommand { get; }
        public ICommand ShowWidgetCommand { get; }
        public ICommand ResetLayoutCommand { get; }

        public event EventHandler? CloseRequested;
        public event EventHandler? ResetLayoutRequested;
        public event EventHandler<PaneViewModel>? ShowWidgetRequested;

        public void LoadSession()
        {
            _globalState = EditorSessionManager.LoadGlobalSession();
            RefreshRecentProjects();
        }

        public void SaveSession()
        {
            EditorSessionManager.SaveGlobalSession(_globalState);
        }

        // Возвращает false, если закрытие нужно отменить (пользователь нажал "Отмена" в диалоге сохранения)
        public bool RequestClose()
        {
            if (!IsDirty)
            {
                return true;
            }

            string title = Application.Current?.TryFindResource("Dialog_Close_Title") as string ?? "Выход";
            string message = Application.Current?.TryFindResource("Dialog_Close_Unsaved") as string ?? "Сохранить проект перед выходом?";

            var result = _dialogService.ShowMessage(message, title, MessageBoxButton.YesNoCancel, MessageBoxImage.Warning);
            if (result == MessageBoxResult.Cancel)
            {
                return false;
            }

            if (result == MessageBoxResult.Yes)
            {
                IsDirty = false;
            }

            return true;
        }

        private void ResetLayout()
        {
            string layoutPath = EditorSessionManager.GetLayoutFilePath();
            try
            {
                if (File.Exists(layoutPath))
                {
                    File.Delete(layoutPath);
                }
            }
            catch
            {
                // Пропускаем сбой удаления файла раскладки
            }

            ResetLayoutRequested?.Invoke(this, EventArgs.Empty);
        }

        private void NewProject()
        {
            _dialogService.ShowMessage("Создание нового проекта (заглушка)", "Проект", MessageBoxButton.OK, MessageBoxImage.Information);
            _currentProjectPath = @"C:\Projects\NewProject.zzz";
            RaiseProjectChanged();
            IsDirty = true;
        }

        private void OpenProject()
        {
            _dialogService.ShowMessage("Открытие существующего проекта (заглушка)", "Проект", MessageBoxButton.OK, MessageBoxImage.Information);
            string projectPath = @"C:\Projects\ZzzGame_" + Random.Shared.Next(100) + ".zzz";
            _currentProjectPath = projectPath;
            RaiseProjectChanged();
            IsDirty = false;

            AddRecentProject(projectPath);
        }

        private void OpenRecentProject(string? path)
        {
            if (string.IsNullOrEmpty(path))
            {
                return;
            }

            _dialogService.ShowMessage($"Открываем последний проект: {path}", "Проект", MessageBoxButton.OK, MessageBoxImage.Information);
            _currentProjectPath = path;
            RaiseProjectChanged();
            IsDirty = false;
        }

        private void AddRecentProject(string path)
        {
            var list = new List<string>(_globalState.RecentProjects ?? Array.Empty<string>());
            list.Remove(path);
            list.Insert(0, path);
            if (list.Count > 10)
            {
                list.RemoveAt(10);
            }

            _globalState.RecentProjects = list.ToArray();
            RefreshRecentProjects();
        }

        private void RaiseProjectChanged()
        {
            OnPropertyChanged(nameof(ProjectDisplayName));
            OnPropertyChanged(nameof(WindowTitle));
        }

        private void RefreshRecentProjects()
        {
            RecentProjects.Clear();
            foreach (var path in _globalState.RecentProjects ?? Array.Empty<string>())
            {
                RecentProjects.Add(path);
            }

            OnPropertyChanged(nameof(HasRecentProjects));
        }
    }
}
