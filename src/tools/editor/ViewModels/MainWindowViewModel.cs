
using System.IO;
using editor.Models;
using System.Windows;
using editor.Services;
using IAssetsTreeCommand = editor.Services.Project.Infrastructure.UndoRedo.IAssetsTreeCommand;
using System.Windows.Input;
using System.Collections.ObjectModel;

namespace editor.ViewModels
{
	public class MainWindowViewModel : ViewModelBase
	{
		private readonly IDialogService _dialogService;
		private GlobalSessionState _globalState = new();

		private string? _currentProjectPath;

		public MainWindowViewModel(IDialogService dialogService)
		{
			_dialogService = dialogService;

			Panes = new ObservableCollection<PaneViewModel>(
				Enum.GetValues(typeof(WidgetType))
					.Cast<WidgetType>()
					.Select<WidgetType, PaneViewModel>(type => type switch
					{
						WidgetType.World => new WorldPaneViewModel(),
						WidgetType.Game => new GamePaneViewModel(),
						WidgetType.Console => new ConsoleViewModel(),
						WidgetType.Assets => new AssetsViewModel(),
						WidgetType.Inspector => new InspectorViewModel(),
						_ => new PaneViewModel(type)
					}));

			WorldPane = (WorldPaneViewModel)Panes.First(p => p.Type == WidgetType.World);
			GamePane = (GamePaneViewModel)Panes.First(p => p.Type == WidgetType.Game);
			RecentProjects = new ObservableCollection<string>();

			foreach (var pane in Panes)
			{
				pane.OnProjectClosed();
			}



			NewProjectCommand = new RelayCommand(NewProject);
			OpenProjectCommand = new RelayCommand(OpenProject);
			OpenRecentProjectCommand = new RelayCommand<string>(OpenRecentProject);
			CloseProjectCommand = new RelayCommand(CloseProject, () => IsProjectOpen);
			ExitCommand = new RelayCommand(() => CloseRequested?.Invoke(this, EventArgs.Empty));
			AboutCommand = new RelayCommand(_dialogService.ShowAbout);
			UndoCommand = new RelayCommand(Undo, () => App.ProjectService.History.CanUndo);
			RedoCommand = new RelayCommand(Redo, () => App.ProjectService.History.CanRedo);
			PlayCommand = new RelayCommand(() =>
			{
				IsRunning = true;
				IsPaused = false;

				var guids = App.ProjectService.CurrentGameConfig.GlobalScriptGuids;
				var classNames = new System.Collections.Generic.List<string>();

				if (guids != null)
				{
					foreach (var guid in guids)
					{
						if (App.ScriptAssetIndexService.TryGetByGuid(guid, out var info) && !string.IsNullOrEmpty(info.ClassName))
						{
							classNames.Add(info.ClassName);
						}
					}
				}

				EngineRuntime.Play(classNames.ToArray());
			}, () => IsProjectOpen && !IsRunning);
			PauseCommand = new RelayCommand(() =>
			{
				IsPaused = !IsPaused;
				EngineRuntime.Pause(IsPaused);
			}, () => IsProjectOpen && IsRunning);
			StopCommand = new RelayCommand(() =>
			{
				IsRunning = false;
				IsPaused = false;
				EngineRuntime.Stop();
			}, () => IsProjectOpen && IsRunning);
			ShowWidgetCommand = new RelayCommand<PaneViewModel>(pane =>
			{
				if (pane != null)
				{
					ShowWidgetRequested?.Invoke(this, pane);
				}
			});
			ResetLayoutCommand = new RelayCommand(ResetLayout);
			SwitchLanguageCommand = new RelayCommand<LanguageItem>((lang) =>
			{
				if (lang != null)
				{
					SelectedLanguage = lang;
				}
			});
		}

		public ObservableCollection<PaneViewModel> Panes { get; }

		public WorldPaneViewModel WorldPane { get; }
		public GamePaneViewModel GamePane { get; }

		public ObservableCollection<string> RecentProjects { get; }

		public bool HasRecentProjects => RecentProjects.Count > 0;

		public string AppName => EditorConstants.ApplicationName;

		public class LanguageItem
		{
			public string Name { get; set; } = string.Empty;
			public string Code { get; set; } = string.Empty;
		}

		public System.Collections.Generic.List<LanguageItem> AvailableLanguages { get; } = new()
		{
			new LanguageItem { Name = "Русский", Code = "ru-RU" },
			new LanguageItem { Name = "English", Code = "en-US" }
		};

		public LanguageItem SelectedLanguage
		{
			get => AvailableLanguages.Find(l => l.Code == LocalizationManager.CurrentCulture) ?? AvailableLanguages[1];
			set
			{
				if (value != null && value.Code != LocalizationManager.CurrentCulture)
				{
					LocalizationManager.SetLanguage(value.Code);
					_globalState.Language = value.Code;
					SaveSession();

					OnPropertyChanged(nameof(SelectedLanguage));
					OnPropertyChanged(nameof(ProjectDisplayName));
					OnPropertyChanged(nameof(WindowTitle));

					foreach (var pane in Panes)
					{
						pane.UpdateTitle();
					}
				}
			}
		}

		private string GetLocString(string key)
		{
			return Application.Current?.TryFindResource(key) as string ?? string.Empty;
		}

		public string ProjectDisplayName
		{
			get
			{
				string noProjectText = GetLocString("Menu_File_NoProject");
				return string.IsNullOrEmpty(_currentProjectPath)
					? noProjectText
					: App.ProjectService.GetProjectName(_currentProjectPath);
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

					if (IsRunning && StopCommand.CanExecute(null))
					{
						StopCommand.Execute(null);
					}

					IsRunning = false;
					IsPaused = false;
					CommandManager.InvalidateRequerySuggested();

					WorldPane.IsToolbarEnabled = IsProjectOpen;
					GamePane.IsToolbarEnabled = IsProjectOpen;
				}
			}
		}

		public bool IsProjectOpen
		{
			get => !string.IsNullOrEmpty(_currentProjectPath);
		}

		private bool _isRunning;
		public bool IsRunning
		{
			get => _isRunning;
			set
			{
				if (SetField(ref _isRunning, value))
					CommandManager.InvalidateRequerySuggested();
			}
		}

		private bool _isPaused;
		public bool IsPaused
		{
			get => _isPaused;
			set
			{
				if (SetField(ref _isPaused, value))
					CommandManager.InvalidateRequerySuggested();
			}
		}

		public string WindowTitle => $"{AppName} - [{ProjectDisplayName}]";

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
		public ICommand SwitchLanguageCommand { get; }

		public event EventHandler? CloseRequested;
		public event EventHandler? ResetLayoutRequested;
		public event EventHandler<PaneViewModel>? ShowWidgetRequested;

		public void LoadSession()
		{
			_globalState = EditorSessionManager.LoadGlobalSession();
			RefreshRecentProjects();

			if (!string.IsNullOrEmpty(_globalState.LastOpenProjectPath))
			{
				if (Directory.Exists(_globalState.LastOpenProjectPath))
				{
					Application.Current.Dispatcher.BeginInvoke(new Action(() =>
					{
						OpenProjectInternal(_globalState.LastOpenProjectPath);
					}), System.Windows.Threading.DispatcherPriority.ApplicationIdle);
				}
				else
				{
					string missingPath = _globalState.LastOpenProjectPath;
					_globalState.LastOpenProjectPath = string.Empty;
					SaveSession();

					string format = GetLocString("Msg_Project_NotFound_Load");
					string title = GetLocString("Msg_Project_NotFound_Title");
					_dialogService.ShowMessage(string.Format(format, missingPath), title, MessageBoxButton.OK, MessageBoxImage.Warning);
					RemoveRecentProject(missingPath);
				}
			}
		}

		public void SaveSession()
		{
			EditorSessionManager.SaveGlobalSession(_globalState);
		}

		// Возвращает false, если закрытие нужно отменить
		public bool RequestClose()
		{
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

			var parentWindow = Application.Current.MainWindow;
			var dialog = new editor.Views.NewProjectDialog(parentWindow, _globalState.LastCreatedProjectParentDir);

			if (dialog.ShowDialog() == true)
			{
				string projectDir = dialog.TargetProjectDirectory;

				if (App.ProjectService.CreateProject(dialog.ParentDirectory, dialog.ProjectName, out string error))
				{
					_globalState.LastOpenProjectPath = projectDir;
					_globalState.LastCreatedProjectParentDir = dialog.ParentDirectory;
					SaveSession();
					CurrentProjectPath = projectDir;
					AddRecentProject(projectDir);

					App.EngineService.OnProjectOpened(projectDir);
					foreach (var pane in Panes)
					{
						pane.OnProjectOpened(projectDir);
					}

					App.ProjectService.History.Clear();
				}
				else
				{
					string errorTitle = GetLocString("Msg_Error_Title");
					_dialogService.ShowMessage(error, errorTitle, MessageBoxButton.OK, MessageBoxImage.Error);
				}
			}
		}

		private void OpenProject()
		{
			if (!RequestClose())
			{
				return;
			}

			var dialog = new Microsoft.Win32.OpenFolderDialog
			{
				Title = GetLocString("Dialog_OpenProject_Title")
			};

			if (dialog.ShowDialog() == true)
			{
				OpenProjectInternal(dialog.FolderName);
			}
		}

		private void OpenRecentProject(string? path)
		{
			if (string.IsNullOrEmpty(path))
			{
				return;
			}

			if (!Directory.Exists(path))
			{
				string format = GetLocString("Msg_Project_NotFound_Recent");
				string title = GetLocString("Msg_Project_NotFound_Title");
				_dialogService.ShowMessage(string.Format(format, path), title, MessageBoxButton.OK, MessageBoxImage.Warning);
				RemoveRecentProject(path);
				return;
			}

			if (!RequestClose())
			{
				return;
			}

			OpenProjectInternal(path);
		}

		private void OpenProjectInternal(string projectDir)
		{
			if (CurrentProjectPath == projectDir)
			{
				return; // Защита от открытия самого себя
			}

			// Очистка временных файлов сборки при открытии проекта
			try
			{
				string editorDir = System.IO.Path.Combine(projectDir, ".editor");
				string buildDir = System.IO.Path.Combine(editorDir, "build");
				string binDir = System.IO.Path.Combine(editorDir, "bin");

				if (System.IO.Directory.Exists(buildDir))
				{
					System.IO.Directory.Delete(buildDir, true);
				}

				if (System.IO.Directory.Exists(binDir))
				{
					var pdbs = System.IO.Directory.GetFiles(binDir, "*.pdb");
					foreach (var pdb in pdbs)
					{
						try { System.IO.File.Delete(pdb); } catch { }
					}
					var tempDlls = System.IO.Directory.GetFiles(binDir, "scripts_temp_*.dll");
					foreach (var tempDll in tempDlls)
					{
						try { System.IO.File.Delete(tempDll); } catch { }
					}
				}
			}
			catch (Exception ex)
			{
				EditorLogger.LogWarning($"[Project] Ошибка при очистке временных файлов сборки: {ex.Message}");
			}

			if (App.ProjectService.OpenProject(projectDir, out string error))
			{
				_globalState.LastOpenProjectPath = projectDir;
				SaveSession();
				CurrentProjectPath = projectDir;
				AddRecentProject(projectDir);

				App.EngineService.OnProjectOpened(projectDir);
				foreach (var pane in Panes)
				{
					pane.OnProjectOpened(projectDir);
				}

				App.ProjectService.History.Clear();
			}
			else
			{
				string errorTitle = GetLocString("Msg_Error_Title");
				_dialogService.ShowMessage(error, errorTitle, MessageBoxButton.OK, MessageBoxImage.Error);
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
			App.ProjectService.History.Clear();
			foreach (var pane in Panes)
			{
				pane.OnProjectClosed();
			}
		}

		// Вызывается после успешного App.ProjectService.RenameProject(): корневая папка проекта
		// физически переехала на newPath - нужно обновить текущий путь, сессию и список
		// последних проектов (иначе они продолжат указывать на несуществующую старую папку).
		public void UpdateAfterProjectRename(string newPath)
		{
			string oldPath = _currentProjectPath ?? string.Empty;

			CurrentProjectPath = newPath;
			_globalState.LastOpenProjectPath = newPath;
			SaveSession();

			var list = new List<string>(_globalState.RecentProjects ?? Array.Empty<string>());
			int idx = list.FindIndex(p => string.Equals(p, oldPath, StringComparison.OrdinalIgnoreCase));
			if (idx >= 0)
			{
				list[idx] = newPath;
			}
			else
			{
				list.Insert(0, newPath);
			}

			_globalState.RecentProjects = list.ToArray();
			RefreshRecentProjects();
		}

		private void RemoveRecentProject(string path)
		{
			var list = new List<string>(_globalState.RecentProjects ?? Array.Empty<string>());
			list.Remove(path);
			_globalState.RecentProjects = list.ToArray();
			SaveSession();
			RefreshRecentProjects();
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


		private void Undo()
		{
			if (App.ProjectService.History.CanUndo)
			{
				var command = App.ProjectService.History.Undo();
				OnPropertyChanged(nameof(WindowTitle));
				CommandManager.InvalidateRequerySuggested();

				// Дерево ассетов перестраиваем только для команд, которые реально создают/удаляют/
				// переименовывают файлы - иначе, например, Undo правки поля конфига без нужды
				// пересобирает дерево, роняя текущее выделение в AssetsWidget (см. IAssetsTreeCommand).
				if (command is IAssetsTreeCommand)
				{
					RefreshAssetsTree();
				}
			}
		}

		private void Redo()
		{
			if (App.ProjectService.History.CanRedo)
			{
				var command = App.ProjectService.History.Redo();
				OnPropertyChanged(nameof(WindowTitle));
				CommandManager.InvalidateRequerySuggested();

				if (command is IAssetsTreeCommand)
				{
					RefreshAssetsTree();
				}
			}
		}

		private void RefreshAssetsTree()
		{
			var assetsVm = Panes.OfType<AssetsViewModel>().FirstOrDefault();
			assetsVm?.RefreshTree();
		}

		public void RefreshDirtyState()
		{
			OnPropertyChanged(nameof(WindowTitle));
			CommandManager.InvalidateRequerySuggested();
		}
	}
}
