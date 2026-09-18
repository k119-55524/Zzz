using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using editor.Services.Project.FileTypes.GameConfig;
using editor.Services.Project.Infrastructure;
using editor.Services.Project.Infrastructure.UndoRedo;
using editor.Services.Project.FileTypes.ProjectSettings;

namespace editor.Services.Project
{
	public class ProjectService(IFileStorage storage)
	{
		private readonly IFileStorage _storage = storage;

		public ProjectService() : this(new PhysicalFileStorage())
		{
		}

		public IFileStorage Storage => _storage;

		public HistoryManager History { get; } = new();

		public ProjectSettingsData CurrentSettings { get; private set; } = new();

		public GameConfigData CurrentGameConfig { get; private set; } = new();

		// Физический корень текущего открытого проекта. Хранится здесь (а не захватывается
		// через замыкание над параметром метода), чтобы после RenameProject автосохранение
		// (CurrentSettings.OnChanged) писало файл по актуальному, а не по старому пути.
		public string? CurrentProjectRootPath { get; private set; }

		private string GetLocString(string key)
		{
			return System.Windows.Application.Current?.TryFindResource(key) as string ?? string.Empty;
		}

		// Проверяет валидность имени папки проекта
		public bool IsValidProjectName(string name, out string error)
		{
			error = string.Empty;
			if (string.IsNullOrWhiteSpace(name))
			{
				error = GetLocString("Validation_ProjectName_Empty");
				return false;
			}

			char[] invalidChars = Path.GetInvalidFileNameChars();
			if (name.IndexOfAny(invalidChars) >= 0)
			{
				error = GetLocString("Validation_ProjectName_InvalidChars");
				return false;
			}

			return true;
		}

		// Переименовывает текущий открытый проект: физически переименовывает корневую папку
		// проекта на диске и обновляет CurrentSettings.Name (с автосохранением project_config.toml
		// по новому пути). Не делает ничего и возвращает true, если имя не изменилось.
		public bool RenameProject(string newName, out string error)
		{
			error = string.Empty;
			newName = newName?.Trim() ?? string.Empty;

			if (CurrentProjectRootPath == null)
			{
				error = GetLocString("Validation_Folder_NotExist");
				return false;
			}

			if (!IsValidProjectName(newName, out error))
			{
				return false;
			}

			if (newName == CurrentSettings.Name)
			{
				return true;
			}

			string? parentDir = Path.GetDirectoryName(CurrentProjectRootPath);
			if (string.IsNullOrEmpty(parentDir))
			{
				error = GetLocString("Validation_Folder_NotExist");
				return false;
			}

			string newPath = Path.Combine(parentDir, newName);

			bool isCaseOnlyRename = string.Equals(newPath, CurrentProjectRootPath, StringComparison.OrdinalIgnoreCase);
			if (!isCaseOnlyRename && _storage.DirectoryExists(newPath))
			{
				string format = GetLocString("Validation_Folder_Exists");
				error = string.Format(format, newPath);
				return false;
			}

			try
			{
				_storage.MoveDirectory(CurrentProjectRootPath, newPath);
			}
			catch (IOException ex)
			{
				// Самая частая причина: папка проекта открыта в Проводнике/терминале/другой
				// программе (ОС держит хендл на саму директорию) - это не ошибка валидации.
				string format = GetLocString("Error_RenameProject_FolderInUse");
				error = string.Format(format, ex.Message);
				return false;
			}
			catch (Exception ex)
			{
				error = ex.Message;
				return false;
			}

			CurrentProjectRootPath = newPath;
			CurrentSettings.Name = newName;
			return true;
		}

		// Создает проект (новую папку, структуру каталогов и дефолтные файлы)
		public bool CreateProject(string parentDir, string projectName, out string error)
		{
			error = string.Empty;
			try
			{
				string projectPath = Path.Combine(parentDir, projectName);

				if (_storage.DirectoryExists(projectPath) && _storage.GetFileSystemEntries(projectPath).Length > 0)
				{
					string format = GetLocString("Validation_Folder_Exists");
					error = string.Format(format, projectPath);
					return false;
				}

				// Создаем корневую директорию
				_storage.CreateDirectory(projectPath);

				// Создаем обязательные папки
				foreach (var folder in ProjectStructure.AllDirectories)
				{
					_storage.CreateDirectory(Path.Combine(projectPath, folder.RelativePath));
				}

				// Создаем обязательные файлы с контентом по умолчанию
				foreach (var fileSchema in ProjectStructure.AllFiles)
				{
					string fullPath = Path.Combine(projectPath, fileSchema.RelativePath);
					string dirPath = Path.GetDirectoryName(fullPath) ?? projectPath;
					if (!_storage.DirectoryExists(dirPath))
					{
						_storage.CreateDirectory(dirPath);
					}

					string content = fileSchema.DefaultContent;
					if (fileSchema.RelativePath == ProjectConstants.SystemDirectories.ProjectSettings)
					{
						content = content.Replace("NewProject", projectName);
					}

					_storage.WriteAllText(fullPath, content);
				}
				CurrentSettings = new ProjectSettingsData
				{
					Version = ProjectConstants.ProjectVersionString,
					Name = projectName
				};
				CurrentGameConfig = new GameConfigData
				{
					Version = ProjectConstants.ProjectVersionString
				};

				// Создаем Main.zscene
				string mainScenePath = Path.Combine(projectPath, "Assets", "Scenes", "Main.zscene");
				string scenesDirPath = Path.GetDirectoryName(mainScenePath)!;
				if (!_storage.DirectoryExists(scenesDirPath)) _storage.CreateDirectory(scenesDirPath);
				var mainSceneData = new editor.Services.Project.FileTypes.Assets.SceneAssetData { Version = "1.0" };
				_storage.WriteAllText(mainScenePath, editor.Services.Project.FileTypes.Assets.SceneAssetParser.Serialize(mainSceneData));

				var mainSceneMeta = AssetMetaFile.CreateNew("Main");
				AssetMetaFile.Save(_storage, mainScenePath + ".meta", mainSceneMeta);

				// Создаем Main.zview
				string mainViewPath = Path.Combine(projectPath, "Assets", "Views", "Main.zview");
				string viewsDirPath = Path.GetDirectoryName(mainViewPath)!;
				if (!_storage.DirectoryExists(viewsDirPath)) _storage.CreateDirectory(viewsDirPath);
				var mainViewData = new editor.Services.Project.FileTypes.Assets.ViewAssetData { Version = "1.0", SceneGuid = mainSceneMeta.Guid };
				_storage.WriteAllText(mainViewPath, editor.Services.Project.FileTypes.Assets.ViewAssetParser.Serialize(mainViewData));

				var mainViewMeta = AssetMetaFile.CreateNew("Main");
				AssetMetaFile.Save(_storage, mainViewPath + ".meta", mainViewMeta);

				// Добавляем Main.zview в GameConfig
				CurrentGameConfig.ViewGuids.Add(mainViewMeta.Guid);

				// Очищаем бэкапы редактора и историю для нового проекта
				ClearBackupDirectory(projectPath);
				History.Clear();

				CurrentProjectRootPath = projectPath;

				// Связываем с Undo/Redo и включаем автосохранение
				CurrentSettings.SetHistoryManager(History);
				CurrentSettings.OnChanged += () =>
				{
					if (CurrentProjectRootPath != null) SaveProject(CurrentProjectRootPath, out _);
				};
			}
			catch (Exception ex)
			{
				string format = GetLocString("Validation_Create_Failed");
				error = string.Format(format, ex.Message);
				return false;
			}

			return true;
		}

		// Выполняет валидацию проекта при открытии.
		// Автоматически восстанавливает структуру папок (некритичные ошибки).
		// Возвращает список ошибок по файлам.
		public bool OpenProject(string projectRootPath, out string error)
		{
			error = string.Empty;

			try
			{
				if (!_storage.DirectoryExists(projectRootPath))
				{
					error = GetLocString("Validation_Folder_NotExist");
					return false;
				}

				// 1. Автовосстановление структуры папок
				foreach (var folder in ProjectStructure.AllDirectories)
				{
					string fullPath = Path.Combine(projectRootPath, folder.RelativePath);
					if (!_storage.DirectoryExists(fullPath))
					{
						_storage.CreateDirectory(fullPath);
					}
				}

				// 2. Валидация и автовосстановление файлов проекта
				List<ValidationErrorItem> toRestore = new List<ValidationErrorItem>();
				foreach (var fileSchema in ProjectStructure.AllFiles)
				{
					string fullPath = Path.Combine(projectRootPath, fileSchema.RelativePath);

					bool needsRestore = false;
					if (!_storage.FileExists(fullPath))
					{
						needsRestore = true;
					}
					else if (fileSchema.Parser != null)
					{
						if (!fileSchema.Parser.Validate(_storage, fullPath, out _))
						{
							needsRestore = true;
						}
					}

					if (needsRestore)
					{
						toRestore.Add(new ValidationErrorItem { FilePath = fileSchema.RelativePath });
					}
				}

				if (toRestore.Count > 0)
				{
					RestoreDefaultFiles(projectRootPath, toRestore);
				}
				string fullSettingsPath = Path.Combine(projectRootPath, ProjectConstants.SystemDirectories.ProjectSettings);
				if (_storage.FileExists(fullSettingsPath))
				{
					string toml = _storage.ReadAllText(fullSettingsPath);
					CurrentSettings = ProjectSettingsParser.Deserialize(toml);
				}
				else
				{
					CurrentSettings = new ProjectSettingsData
					{
						Version = ProjectConstants.ProjectVersionString,
						Name = Path.GetFileName(projectRootPath.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar))
					};
				}

				string fullGameConfigPath = Path.Combine(projectRootPath, ProjectConstants.SystemDirectories.GameConfigs);
				if (_storage.FileExists(fullGameConfigPath))
				{
					string toml = _storage.ReadAllText(fullGameConfigPath);
					CurrentGameConfig = GameConfigParser.Deserialize(toml);
				}
				else
				{
					CurrentGameConfig = new GameConfigData
					{
						Version = ProjectConstants.ProjectVersionString
					};
				}

				// Очищаем старые бэкапы и сбрасываем историю
				ClearBackupDirectory(projectRootPath);
				History.Clear();

				CurrentProjectRootPath = projectRootPath;

				// Связываем с Undo/Redo и автосохранением
				CurrentSettings.SetHistoryManager(History);
				CurrentSettings.OnChanged += () =>
				{
					if (CurrentProjectRootPath != null) SaveProject(CurrentProjectRootPath, out _);
				};
			}
			catch (Exception ex)
			{
				string format = GetLocString("Validation_Load_Failed");
				error = string.Format(format, ex.Message);
				return false;
			}

			return true;
		}

		// Перезаписывает указанные проблемные файлы их версиями по умолчанию
		public bool RestoreDefaultFiles(string projectRootPath, List<ValidationErrorItem> errors)
		{
			try
			{
				foreach (var err in errors)
				{
					var schema = ProjectStructure.AllFiles.FirstOrDefault(f => f.RelativePath == err.FilePath);
					if (schema != null)
					{
						string fullPath = Path.Combine(projectRootPath, schema.RelativePath);
						string dirPath = Path.GetDirectoryName(fullPath) ?? projectRootPath;
						if (!_storage.DirectoryExists(dirPath))
						{
							_storage.CreateDirectory(dirPath);
						}
						string content = schema.DefaultContent;
						if (schema.RelativePath == ProjectConstants.SystemDirectories.ProjectSettings)
						{
							string projectName = Path.GetFileName(projectRootPath.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar));
							content = content.Replace("NewProject", projectName);
						}
						_storage.WriteAllText(fullPath, content);
					}
				}
				return true;
			}
			catch
			{
				return false;
			}
		}

		// Читает имя проекта из файла настроек проекта (Configs/project_config.toml) с валидацией
		public string GetProjectName(string projectRootPath)
		{
			try
			{
				var schema = ProjectStructure.AllFiles.FirstOrDefault(f => f.RelativePath == ProjectConstants.SystemDirectories.ProjectSettings);
				if (schema != null)
				{
					string fullPath = Path.Combine(projectRootPath, schema.RelativePath);
					if (_storage.FileExists(fullPath))
					{
						string toml = _storage.ReadAllText(fullPath);
						var data = ProjectSettingsParser.Deserialize(toml);
						string name = data.Name?.Trim() ?? string.Empty;
						if (IsValidProjectName(name, out _))
						{
							return name;
						}
					}
				}
			}
			catch
			{
				// Игнорируем и возвращаем имя папки
			}
			return Path.GetFileName(projectRootPath);
		}

		// Сохраняет текущие настройки проекта на диск в Configs/project_config.toml
		public bool SaveProject(string projectRootPath, out string error)
		{
			error = string.Empty;
			try
			{
				string fullPath = Path.Combine(projectRootPath, ProjectConstants.SystemDirectories.ProjectSettings);
				string toml = ProjectSettingsParser.Serialize(CurrentSettings);
				_storage.WriteAllText(fullPath, toml);
				return true;
			}
			catch (Exception ex)
			{
				error = ex.Message;
				return false;
			}
		}

		// Сохраняет текущую конфигурацию игры на диск в Configs/game_config.toml. Единственная точка
		// записи этого файла - CurrentGameConfig теперь единственный источник правды (инспектор
		// редактирует его напрямую, см. InspectorViewModel.TryShowConfigFile), поэтому без параметра
		// пути (в отличие от SaveProject): вызывающая точка - обычно onChanged-колбэк History-команды -
		// может сработать позже, когда UI уже переключился на другой узел/проект.
		public bool SaveGameConfig(out string error)
		{
			error = string.Empty;
			if (CurrentProjectRootPath == null)
			{
				error = GetLocString("Validation_Folder_NotExist");
				return false;
			}

			try
			{
				string fullPath = Path.Combine(CurrentProjectRootPath, ProjectConstants.SystemDirectories.GameConfigs);
				string toml = GameConfigParser.Serialize(CurrentGameConfig);
				_storage.WriteAllText(fullPath, toml);
				return true;
			}
			catch (Exception ex)
			{
				error = ex.Message;
				return false;
			}
		}

		private void ClearBackupDirectory(string projectRootPath)
		{
			try
			{
				string backupPath = Path.Combine(projectRootPath, ".editor", "backup");
				if (_storage.DirectoryExists(backupPath))
				{
					_storage.DeleteDirectory(backupPath, recursive: true);
				}
			}
			catch
			{
				// Игнорируем ошибки при очистке бэкапов
			}
		}
	}
}
