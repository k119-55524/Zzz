namespace editor.Services.Project.Infrastructure
{
    /// <summary>
    /// Интерфейс хранилища файлов для абстрагирования дисковых операций (I/O).
    /// </summary>
    public interface IFileStorage
    {
        /// <summary>
        /// Проверяет существование файла.
        /// </summary>
        bool FileExists(string path);

        /// <summary>
        /// Проверяет существование директории.
        /// </summary>
        bool DirectoryExists(string path);

        /// <summary>
        /// Считывает весь текст из файла.
        /// </summary>
        string ReadAllText(string path);

        /// <summary>
        /// Записывает текст в файл.
        /// </summary>
        void WriteAllText(string path, string content);

        /// <summary>
        /// Создает директорию и все вложенные поддиректории.
        /// </summary>
        void CreateDirectory(string path);

        /// <summary>
        /// Возвращает все элементы (файлы и папки) внутри указанной директории.
        /// </summary>
        string[] GetFileSystemEntries(string path);
    }
}
