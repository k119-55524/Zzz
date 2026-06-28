using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
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

                // Очищаем бэкапы редактора и историю для нового проекта
                ClearBackupDirectory(projectPath);
                History.Clear();

                // Связываем с Undo/Redo и включаем автосохранение
                CurrentSettings.SetHistoryManager(History);
                CurrentSettings.OnChanged += () => SaveProject(projectPath, out _);
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

                // Очищаем старые бэкапы и сбрасываем историю
                ClearBackupDirectory(projectRootPath);
                History.Clear();

                // Связываем с Undo/Redo и автосохранением
                CurrentSettings.SetHistoryManager(History);
                CurrentSettings.OnChanged += () => SaveProject(projectRootPath, out _);
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

        // Читает имя проекта из файла настроек проекта (Configs/project.toml) с валидацией
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

        // Сохраняет текущие настройки проекта на диск в Configs/project.toml
        public bool SaveProject(string projectRootPath, out string error)
        {
            error = string.Empty;
            try
            {
                if (CurrentSettings == null)
                    return true;

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
