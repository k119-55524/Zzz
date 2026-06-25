namespace editor.Services.Project
{
    public interface IFileParser
    {
        string FileExtension { get; }
        bool Validate(string filePath, out string errorMessage);
    }

    // Заглушка для парсера настроек проекта (.zzz)
    public class ProjectSettingsParser : IFileParser
    {
        public string FileExtension => ".zzz";

        public bool Validate(string filePath, out string errorMessage)
        {
            // Здесь будет полноценный парсинг JSON/XML. Пока просто базовая проверка:
            errorMessage = string.Empty;
            try
            {
                var content = System.IO.File.ReadAllText(filePath);
                // Простая заглушка валидации (файл не должен быть пустым)
                if (string.IsNullOrWhiteSpace(content))
                {
                    errorMessage = "Файл проекта пуст.";
                    return false;
                }
            }
            catch (System.Exception ex)
            {
                errorMessage = ex.Message;
                return false;
            }
            return true;
        }
    }

    // Заглушка для парсера конфигурации движка (.config)
    public class EngineConfigParser : IFileParser
    {
        public string FileExtension => ".config";

        public bool Validate(string filePath, out string errorMessage)
        {
            errorMessage = string.Empty;
            try
            {
                var content = System.IO.File.ReadAllText(filePath);
                if (string.IsNullOrWhiteSpace(content))
                {
                    errorMessage = "Конфигурационный файл пуст.";
                    return false;
                }
            }
            catch (System.Exception ex)
            {
                errorMessage = ex.Message;
                return false;
            }
            return true;
        }
    }
}
