using System;
using System.IO;

namespace editor.Services.Project.Infrastructure.UndoRedo
{
    /// <summary>
    /// Команда удаления файла или папки с поддержкой перемещения в резервную копию и восстановления.
    /// </summary>
    public class DeleteFileOrFolderCommand : ICommand, IDisposable
    {
        private readonly string _originalPath;
        private readonly string _backupDirPath;
        private readonly string _backupDestPath;
        private readonly IFileStorage _storage;
        private readonly bool _isFolder;
        private bool _executed;

        public DeleteFileOrFolderCommand(string originalPath, string projectRoot, IFileStorage storage)
        {
            _originalPath = originalPath;
            _storage = storage;

            _isFolder = _storage.DirectoryExists(originalPath);

            // Генерируем уникальный путь резервной копии внутри .editor/backup/
            string timestamp = DateTime.Now.ToString("yyyyMMdd_HHmmss");
            string uniqueId = Guid.NewGuid().ToString("N").Substring(0, 8);
            
            // Вся папка операции
            _backupDirPath = Path.Combine(projectRoot, ".editor", "backup", $"delete_{timestamp}_{uniqueId}");

            // Сохраняем структуру: имя файла/папки будет в корне этой уникальной папки
            string name = Path.GetFileName(_originalPath);
            _backupDestPath = Path.Combine(_backupDirPath, name);
        }

        public void Execute()
        {
            if (_executed) return;

            // Создаем уникальную директорию бэкапа
            _storage.CreateDirectory(_backupDirPath);

            if (_isFolder)
            {
                if (_storage.DirectoryExists(_originalPath))
                {
                    _storage.MoveDirectory(_originalPath, _backupDestPath);
                    _executed = true;
                }
            }
            else
            {
                if (_storage.FileExists(_originalPath))
                {
                    _storage.MoveFile(_originalPath, _backupDestPath);
                    _executed = true;
                }
            }
        }

        public void Undo()
        {
            if (!_executed) return;

            // Убеждаемся, что родительская папка назначения существует
            string parentDir = Path.GetDirectoryName(_originalPath);
            if (!string.IsNullOrEmpty(parentDir))
            {
                _storage.CreateDirectory(parentDir);
            }

            if (_isFolder)
            {
                if (_storage.DirectoryExists(_backupDestPath))
                {
                    _storage.MoveDirectory(_backupDestPath, _originalPath);
                    _executed = false;
                }
            }
            else
            {
                if (_storage.FileExists(_backupDestPath))
                {
                    _storage.MoveFile(_backupDestPath, _originalPath);
                    _executed = false;
                }
            }
        }

        /// <summary>
        /// Вызывается при удалении команды из истории (когда очищаются ресурсы бэкапа).
        /// </summary>
        public void Dispose()
        {
            try
            {
                if (_storage.DirectoryExists(_backupDirPath))
                {
                    _storage.DeleteDirectory(_backupDirPath, recursive: true);
                }
            }
            catch
            {
                // Игнорируем ошибки удаления бэкапа при очистке
            }
        }
    }
}
