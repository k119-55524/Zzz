using System;

namespace editor.Services.Project.Infrastructure
{
    /// <summary>
    /// Интерфейс для парсинга, валидации, миграции и VCS-слияния файлов проекта определенного типа.
    /// </summary>
    public interface IFileParser
    {
        /// <summary>
        /// Возвращает расширение файла, которое обрабатывает этот парсер (например, ".toml").
        /// </summary>
        string FileExtension { get; }

        /// <summary>
        /// Проверяет, может ли данный парсер обрабатывать файл по его относительному пути.
        /// </summary>
        bool CanParse(string relativePath);

        /// <summary>
        /// Проверяет формат и структуру файла на валидность.
        /// </summary>
        bool Validate(IFileStorage storage, string filePath, out string errorMessage);

        /// <summary>
        /// Извлекает версию формата из указанного файла.
        /// </summary>
        bool TryGetVersion(IFileStorage storage, string filePath, out Version version, out string errorMessage);

        /// <summary>
        /// Генерирует дефолтное текстовое содержимое файла в соответствии с типом ресурса.
        /// </summary>
        string GetDefaultContent();

        /// <summary>
        /// Конвертирует файл к целевой версии.
        /// Применяется цепочка миграций для перевода старой версии в актуальную.
        /// </summary>
        bool Migrate(IFileStorage storage, string filePath, Version targetVersion, out string errorMessage);

        /// <summary>
        /// Выполняет трехстороннее (three-way) слияние файлов при конфликтах в VCS.
        /// </summary>
        /// <param name="storage">Хранилище файлов.</param>
        /// <param name="baseFilePath">Путь к файлу общего предка (ancestor).</param>
        /// <param name="localFilePath">Путь к локальной версии файла (наши изменения).</param>
        /// <param name="remoteFilePath">Путь к удаленной версии файла (их изменения).</param>
        /// <param name="mergedOutputFilePath">Путь, куда записать результат успешного слияния.</param>
        /// <param name="errorMessage">Сообщение об ошибке, если слияние невозможно выполнить автоматически.</param>
        /// <returns>True, если автослияние прошло успешно без конфликтов.</returns>
        bool Merge(
            IFileStorage storage, 
            string baseFilePath, 
            string localFilePath, 
            string remoteFilePath, 
            string mergedOutputFilePath, 
            out string errorMessage);
    }

    /// <summary>
    /// Optional editor bridge for project files that can be shown as reflected
    /// property models in the inspector.
    /// </summary>
    public interface IEditorConfigParser
    {
        /// <summary>
        /// CLR type returned by <see cref="DeserializeForEditor"/>.
        /// </summary>
        Type DataType { get; }

        /// <summary>
        /// Parses text content into a data object for reflection-based editing.
        /// </summary>
        object DeserializeForEditor(string content);

        /// <summary>
        /// Serializes a reflected data object back to file text.
        /// </summary>
        string SerializeFromEditor(object data);
    }
}
