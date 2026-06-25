
using System.IO;
using editor.Models;
using System.Windows;
using editor.Services;
using System.Windows.Input;
using System.Collections.ObjectModel;

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

			WorldPane = Panes.First(p => p.Type == WidgetType.World);
			GamePane = Panes.First(p => p.Type == WidgetType.Game);
			RecentProjects = new ObservableCollection<string>();

			SaveCommand = new RelayCommand(Save, () => IsDirty);

			NewProjectCommand = new RelayCommand(NewProject);
			OpenProjectCommand = new RelayCommand(OpenProject);
			OpenRecentProjectCommand = new RelayCommand<string>(OpenRecentProject);
			CloseProjectCommand = new RelayCommand(CloseProject, () => IsProjectOpen);
			ExitCommand = new RelayCommand(() => CloseRequested?.Invoke(this, EventArgs.Empty));
			AboutCommand = new RelayCommand(_dialogService.ShowAbout);
			UndoCommand = new RelayCommand(() => { /* Логика отмены действия (Undo) будет реализована позже */ }, () => IsDirty);
			RedoCommand = new RelayCommand(() => { /* Логика повтора действия (Redo) будет реализована позже */ }, () => IsDirty);
			PlayCommand = new RelayCommand(() => { /* Запуск симуляции */ }, () => IsProjectOpen);
			PauseCommand = new RelayCommand(() => { /* Пауза симуляции */ }, () => IsProjectOpen);
			StopCommand = new RelayCommand(() => { /* Остановка симуляции */ }, () => IsProjectOpen);
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

		public PaneViewModel WorldPane { get; }
		public PaneViewModel GamePane { get; }

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

		public string? CurrentProjectPath
		{
			get => _currentProjectPath;
			set
			{
				if (SetField(ref _currentProjectPath, value))
				{
					OnPropertyChanged(nameof(ProjectDisplayName));
					OnPropertyChanged(nameof(WindowTitle));
					OnPropertyChanged(nameof(IsProjectOpen));
					CommandManager.InvalidateRequerySuggested();
				}
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

		public bool IsProjectOpen
		{
			get => !string.IsNullOrEmpty(_currentProjectPath);
		}

		public string WindowTitle => $"{AppName} - [{ProjectDisplayName}]{(IsDirty ? "*" : "")}";

		public ICommand SaveCommand { get; }

		public ICommand NewProjectCommand { get; }
		public ICommand OpenProjectCommand { get; }
		public ICommand OpenRecentProjectCommand { get; }
		public ICommand CloseProjectCommand { get; }
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

			if (!string.IsNullOrEmpty(_globalState.LastOpenProjectPath) && File.Exists(_globalState.LastOpenProjectPath))
			{
				Application.Current.Dispatcher.BeginInvoke(new Action(() =>
				{
					OpenProjectInternal(_globalState.LastOpenProjectPath);
				}), System.Windows.Threading.DispatcherPriority.ApplicationIdle);
			}
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
			if (!RequestClose())
			{
				return;
			}

			var dialog = new Microsoft.Win32.SaveFileDialog
			{
				Filter = "Project File (*.zzz)|*.zzz",
				Title = "Создать новый проект"
			};

			if (dialog.ShowDialog() == true)
			{
				string projectFilePath = dialog.FileName;
				string projectDir = Path.GetDirectoryName(projectFilePath)!;
				string projectName = Path.GetFileNameWithoutExtension(projectFilePath);

				if (App.ProjectService.CreateProject(Path.GetDirectoryName(projectDir)!, projectName, out string error))
				{
					_globalState.LastOpenProjectPath = projectFilePath;
					SaveSession();
					CurrentProjectPath = projectFilePath;
					IsDirty = true;
					AddRecentProject(projectFilePath);

					App.EngineService.OnProjectOpened(projectDir);
				}
				else
				{
					_dialogService.ShowMessage(error, "Ошибка", MessageBoxButton.OK, MessageBoxImage.Error);
				}
			}
		}

		private void OpenProject()
		{
			if (!RequestClose())
			{
				return;
			}

			var dialog = new Microsoft.Win32.OpenFileDialog
			{
				Filter = "Project File (*.zzz)|*.zzz",
				Title = "Открыть проект"
			};

			if (dialog.ShowDialog() == true)
			{
				OpenProjectInternal(dialog.FileName);
			}
		}

		private void OpenRecentProject(string? path)
		{
			if (string.IsNullOrEmpty(path))
			{
				return;
			}

			if (!RequestClose())
			{
				return;
			}

			OpenProjectInternal(path);
		}

		private void OpenProjectInternal(string projectFilePath)
		{
			if (CurrentProjectPath == projectFilePath)
			{
				return; // Защита от открытия самого себя
			}

			string projectDir = Path.GetDirectoryName(projectFilePath)!;

			if (App.ProjectService.OpenProject(projectDir, out string scanError, out var validationErrors))
			{
				if (validationErrors.Count > 0)
				{
					var parentWindow = Application.Current.MainWindow;
					var errorDialog = new editor.Views.ProjectValidationErrorDialog(parentWindow, validationErrors);
					if (errorDialog.ShowDialog() == true)
					{
						if (errorDialog.Resolution == editor.Views.ProjectLoadResolution.RestoreDefaults)
						{
							App.ProjectService.RestoreDefaultFiles(projectDir, validationErrors);
						}
						// Если Ignore или RestoreDefaults - загружаем
					}
					else
					{
						// Отмена загрузки
						return;
					}
				}

				_globalState.LastOpenProjectPath = projectFilePath;
				SaveSession();
				CurrentProjectPath = projectFilePath;
				IsDirty = false;
				AddRecentProject(projectFilePath);

				App.EngineService.OnProjectOpened(projectDir);
			}
			else
			{
				_dialogService.ShowMessage(scanError, "Ошибка", MessageBoxButton.OK, MessageBoxImage.Error);
			}
		}

		private void CloseProject()
		{
			if (!RequestClose())
			{
				return;
			}

			App.EngineService.OnProjectClosed();
			_globalState.LastOpenProjectPath = string.Empty;
			SaveSession();
			CurrentProjectPath = null;
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

		private void RefreshRecentProjects()
		{
			RecentProjects.Clear();
			foreach (var path in _globalState.RecentProjects ?? Array.Empty<string>())
			{
				RecentProjects.Add(path);
			}

			OnPropertyChanged(nameof(HasRecentProjects));
		}

		void Save()
		{
		}
	}
}
