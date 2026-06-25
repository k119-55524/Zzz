using System;
using System.Collections.Generic;
using System.IO;

namespace editor.Services.Project
{
    public class ProjectService
    {
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

                if (Directory.Exists(projectPath) && Directory.GetFileSystemEntries(projectPath).Length > 0)
                {
                    string format = GetLocString("Validation_Folder_Exists");
                    error = string.Format(format, projectPath);
                    return false;
                }

                // Создаем корневую директорию
                Directory.CreateDirectory(projectPath);

                // Создаем обязательные папки
                foreach (var dir in ProjectStructure.RequiredDirectories)
                {
                    Directory.CreateDirectory(Path.Combine(projectPath, dir));
                }

                // Создаем обязательные файлы с контентом по умолчанию
                foreach (var fileSchema in ProjectStructure.RequiredFiles)
                {
                    string fullPath = Path.Combine(projectPath, fileSchema.RelativePath);
                    string dirPath = Path.GetDirectoryName(fullPath) ?? projectPath;
                    if (!Directory.Exists(dirPath))
                    {
                        Directory.CreateDirectory(dirPath);
                    }

                    string content = fileSchema.DefaultContent;
                    if (fileSchema.RelativePath == "project.zzz")
                    {
                        content = content.Replace("\"NewProject\"", $"\"{projectName}\"");
                    }

                    File.WriteAllText(fullPath, content);
                }
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
                if (!Directory.Exists(projectRootPath))
                {
                    error = GetLocString("Validation_Folder_NotExist");
                    return false;
                }

                // 1. Автовосстановление структуры папок
                foreach (var dir in ProjectStructure.RequiredDirectories)
                {
                    string fullPath = Path.Combine(projectRootPath, dir);
                    if (!Directory.Exists(fullPath))
                    {
                        Directory.CreateDirectory(fullPath);
                    }
                }

                // 2. Валидация и автовосстановление файлов проекта
                List<ValidationErrorItem> toRestore = new List<ValidationErrorItem>();
                foreach (var fileSchema in ProjectStructure.RequiredFiles)
                {
                    string fullPath = Path.Combine(projectRootPath, fileSchema.RelativePath);

                    bool needsRestore = false;
                    if (!File.Exists(fullPath))
                    {
                        needsRestore = true;
                    }
                    else if (fileSchema.Parser != null)
                    {
                        if (!fileSchema.Parser.Validate(fullPath, out _))
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
                    var schema = ProjectStructure.RequiredFiles.Find(f => f.RelativePath == err.FilePath);
                    if (schema != null)
                    {
                        string fullPath = Path.Combine(projectRootPath, schema.RelativePath);
                        string dirPath = Path.GetDirectoryName(fullPath) ?? projectRootPath;
                        if (!Directory.Exists(dirPath))
                        {
                            Directory.CreateDirectory(dirPath);
                        }
                        File.WriteAllText(fullPath, schema.DefaultContent);
                    }
                }
                return true;
            }
            catch
            {
                return false;
            }
        }

        // Читает имя проекта из файла настроек проекта (project.zzz) с валидацией
        public string GetProjectName(string projectRootPath)
        {
            try
            {
                var schema = ProjectStructure.RequiredFiles.Find(f => f.RelativePath == "project.zzz");
                if (schema != null)
                {
                    string fullPath = Path.Combine(projectRootPath, schema.RelativePath);
                    if (File.Exists(fullPath))
                    {
                        string json = File.ReadAllText(fullPath);
                        using (var doc = System.Text.Json.JsonDocument.Parse(json))
                        {
                            if (doc.RootElement.TryGetProperty("name", out var nameProp))
                            {
                                string name = nameProp.GetString()?.Trim() ?? string.Empty;
                                if (IsValidProjectName(name, out _))
                                {
                                    return name;
                                }
                            }
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
    }
}
