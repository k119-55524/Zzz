using System;
using System.Collections.Generic;
using System.IO;

namespace editor.Services.Project.Infrastructure.UndoRedo
{
    /// <summary>
    /// Команда удаления файла или папки с поддержкой перемещения в резервную копию и восстановления.
    /// </summary>
    public class DeleteFileOrFolderCommand : ICommand, IDisposable
    {
        private readonly List<string> _physicalPaths;
        private readonly string _projectRoot;
        private readonly bool _isFolder;
        private readonly IFileStorage _storage;

        // Информация о резервных копиях для каждого физического пути
        private readonly List<(string OriginalPath, string BackupDestPath, string BackupDirPath)> _backups = new();
        private bool _executed;

        public DeleteFileOrFolderCommand(
            List<string> physicalPaths,
            string projectRoot,
            bool isFolder,
            IFileStorage storage)
        {
            _physicalPaths = physicalPaths;
            _projectRoot = projectRoot;
            _isFolder = isFolder;
            _storage = storage;

            // Генерируем уникальные пути резервных копий для каждого физического пути
            string timestamp = DateTime.Now.ToString("yyyyMMdd_HHmmss");
            int counter = 0;
            foreach (var origPath in _physicalPaths)
            {
                string uniqueId = Guid.NewGuid().ToString("N").Substring(0, 8);
                string backupDirPath = Path.Combine(_projectRoot, ".editor", "backup", $"delete_{timestamp}_{uniqueId}_{counter++}");
                string name = Path.GetFileName(origPath);
                string backupDestPath = Path.Combine(backupDirPath, name);
                
                _backups.Add((origPath, backupDestPath, backupDirPath));
            }
        }

        public void Execute()
        {
            if (_executed) return;

            // Создаем бэкапы и удаляем физические файлы/папки
            foreach (var backup in _backups)
            {
                try
                {
                    _storage.CreateDirectory(backup.BackupDirPath);
                    if (_isFolder)
                    {
                        if (_storage.DirectoryExists(backup.OriginalPath))
                        {
                            _storage.MoveDirectory(backup.OriginalPath, backup.BackupDestPath);
                        }
                    }
                    else
                    {
                        if (_storage.FileExists(backup.OriginalPath))
                        {
                            _storage.MoveFile(backup.OriginalPath, backup.BackupDestPath);
                        }
                    }
                }
                catch
                {
                    // Игнорируем или логируем ошибки, если пути уже не существуют
                }
            }

            _executed = true;
        }

        public void Undo()
        {
            if (!_executed) return;

            // Восстанавливаем элементы на диске
            foreach (var backup in _backups)
            {
                try
                {
                    string? parentDir = Path.GetDirectoryName(backup.OriginalPath);
                    if (!string.IsNullOrEmpty(parentDir))
                    {
                        _storage.CreateDirectory(parentDir);
                    }

                    if (_isFolder)
                    {
                        if (_storage.DirectoryExists(backup.BackupDestPath))
                        {
                            _storage.MoveDirectory(backup.BackupDestPath, backup.OriginalPath);
                        }
                    }
                    else
                    {
                        if (_storage.FileExists(backup.BackupDestPath))
                        {
                            _storage.MoveFile(backup.BackupDestPath, backup.OriginalPath);
                        }
                    }
                }
                catch
                {
                    // Игнорируем
                }
            }

            _executed = false;
        }

        public void Dispose()
        {
            try
            {
                foreach (var backup in _backups)
                {
                    if (_storage.DirectoryExists(backup.BackupDirPath))
                    {
                        _storage.DeleteDirectory(backup.BackupDirPath, recursive: true);
                    }
                }
            }
            catch
            {
                // Игнорируем
            }
        }
    }
}
