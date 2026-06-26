using System.IO;

namespace editor.Services.Project.Infrastructure
{
    /// <summary>
    /// Физическая реализация IFileStorage, использующая стандартный ввод-вывод System.IO.
    /// </summary>
    public class PhysicalFileStorage : IFileStorage
    {
        public bool FileExists(string path)
        {
            return File.Exists(path);
        }

        public bool DirectoryExists(string path)
        {
            return Directory.Exists(path);
        }

        public string ReadAllText(string path)
        {
            return File.ReadAllText(path);
        }

        public void WriteAllText(string path, string content)
        {
            File.WriteAllText(path, content);
        }

        public void CreateDirectory(string path)
        {
            Directory.CreateDirectory(path);
        }

        public string[] GetFileSystemEntries(string path)
        {
            return Directory.GetFileSystemEntries(path);
        }
    }
}
