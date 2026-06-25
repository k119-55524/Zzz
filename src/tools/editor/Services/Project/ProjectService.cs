using System;
using System.Collections.Generic;
using System.IO;
using editor.Views;

namespace editor.Services.Project
{
    public class ProjectService
    {
        // Проверяет валидность имени папки проекта
        public bool IsValidProjectName(string name, out string error)
        {
            error = string.Empty;
            if (string.IsNullOrWhiteSpace(name))
            {
                error = "Имя проекта не может быть пустым.";
                return false;
            }

            char[] invalidChars = Path.GetInvalidFileNameChars();
            if (name.IndexOfAny(invalidChars) >= 0)
            {
                error = "Имя проекта содержит недопустимые символы.";
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
                    error = $"Папка '{projectPath}' уже существует и не пуста.";
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
                    File.WriteAllText(fullPath, fileSchema.DefaultContent);
                }
            }
            catch (Exception ex)
            {
                error = $"Не удалось создать проект: {ex.Message}";
                return false;
            }

            return true;
        }

        // Выполняет валидацию проекта при открытии.
        // Автоматически восстанавливает структуру папок (некритичные ошибки).
        // Возвращает список ошибок по файлам.
        public bool OpenProject(string projectRootPath, out string error, out List<ValidationErrorItem> validationErrors)
        {
            error = string.Empty;
            validationErrors = new List<ValidationErrorItem>();

            try
            {
                if (!Directory.Exists(projectRootPath))
                {
                    error = "Указанная папка проекта не существует.";
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

                // 2. Валидация файлов проекта
                foreach (var fileSchema in ProjectStructure.RequiredFiles)
                {
                    string fullPath = Path.Combine(projectRootPath, fileSchema.RelativePath);

                    if (!File.Exists(fullPath))
                    {
                        validationErrors.Add(new ValidationErrorItem
                        {
                            FilePath = fileSchema.RelativePath,
                            ErrorMessage = "Файл отсутствует."
                        });
                    }
                    else if (fileSchema.Parser != null)
                    {
                        if (!fileSchema.Parser.Validate(fullPath, out string parserError))
                        {
                            validationErrors.Add(new ValidationErrorItem
                            {
                                FilePath = fileSchema.RelativePath,
                                ErrorMessage = $"Ошибка валидации: {parserError}"
                            });
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                error = $"Ошибка при сканировании проекта: {ex.Message}";
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
    }
}
