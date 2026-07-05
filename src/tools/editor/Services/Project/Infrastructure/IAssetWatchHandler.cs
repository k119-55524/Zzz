namespace editor.Services.Project.Infrastructure
{
    /// <summary>
    /// Обрабатывает внешние изменения файлов конкретного типа ассета (обнаруженные через
    /// FileSystemWatcher, минуя действия самого редактора). Каждый тип ассета (скрипты, в будущем -
    /// текстуры, модели и т.д.) подключает свой обработчик без переделки самого watcher'а.
    /// </summary>
    public interface IAssetWatchHandler
    {
        /// <summary>
        /// Может ли обработчик что-то сделать с файлом по этому пути (обычно проверка расширения).
        /// </summary>
        bool CanHandle(string filePath);

        /// <summary>
        /// Файл появился на диске извне (создан или переименован в этот путь).
        /// </summary>
        void OnCreated(string filePath, IFileStorage storage);

        /// <summary>
        /// Файл удалён с диска извне (удалён или переименован из этого пути).
        /// </summary>
        void OnDeleted(string filePath, IFileStorage storage);
    }
}
