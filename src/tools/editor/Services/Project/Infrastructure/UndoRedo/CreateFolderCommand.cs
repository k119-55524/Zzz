using System;

namespace editor.Services.Project.Infrastructure.UndoRedo
{
    /// <summary>
    /// Команда создания папки.
    /// </summary>
    public class CreateFolderCommand : ICommand
    {
        private readonly string _folderPath;
        private readonly IFileStorage _storage;
        private bool _created;

        public CreateFolderCommand(string folderPath, IFileStorage storage)
        {
            _folderPath = folderPath;
            _storage = storage;
        }

        public void Execute()
        {
            if (!_storage.DirectoryExists(_folderPath))
            {
                _storage.CreateDirectory(_folderPath);
                _created = true;
            }
        }

        public void Undo()
        {
            // Удаляем папку только если мы её создали и она до сих пор пуста (безопасность)
            if (_created && _storage.DirectoryExists(_folderPath))
            {
                var entries = _storage.GetFileSystemEntries(_folderPath);
                if (entries.Length == 0)
                {
                    _storage.DeleteDirectory(_folderPath, recursive: false);
                }
            }
        }
    }
}
