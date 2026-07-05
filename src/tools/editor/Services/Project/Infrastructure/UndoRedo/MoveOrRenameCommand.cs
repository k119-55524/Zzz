using System;
using System.IO;

namespace editor.Services.Project.Infrastructure.UndoRedo
{
    /// <summary>
    /// Команда перемещения или переименования файла/директории.
    /// </summary>
    public class MoveOrRenameCommand : IAssetsTreeCommand
    {
        private readonly string _sourcePath;
        private readonly string _destPath;
        private readonly IFileStorage _storage;
        private readonly bool _isFolder;
        private readonly Action? _onScriptChanged;

        public MoveOrRenameCommand(string sourcePath, string destPath, IFileStorage storage, Action? onScriptChanged = null)
        {
            _sourcePath = sourcePath;
            _destPath = destPath;
            _storage = storage;
            _onScriptChanged = onScriptChanged;

            // Определяем, папка это или файл на момент создания команды
            _isFolder = _storage.DirectoryExists(_sourcePath);
        }

        public void Execute()
        {
            // Убеждаемся, что папка назначения существует
            string parentDir = Path.GetDirectoryName(_destPath);
            if (!string.IsNullOrEmpty(parentDir))
            {
                _storage.CreateDirectory(parentDir);
            }

            if (_isFolder)
            {
                if (_storage.DirectoryExists(_sourcePath))
                {
                    _storage.MoveDirectory(_sourcePath, _destPath);
                }
            }
            else
            {
                if (_storage.FileExists(_sourcePath))
                {
                    _storage.MoveFile(_sourcePath, _destPath);
                }
            }

            if (_onScriptChanged != null && AffectsScripts())
                _onScriptChanged();
        }

        public void Undo()
        {
            // Убеждаемся, что исходная родительская папка существует
            string parentDir = Path.GetDirectoryName(_sourcePath);
            if (!string.IsNullOrEmpty(parentDir))
            {
                _storage.CreateDirectory(parentDir);
            }

            if (_isFolder)
            {
                if (_storage.DirectoryExists(_destPath))
                {
                    _storage.MoveDirectory(_destPath, _sourcePath);
                }
            }
            else
            {
                if (_storage.FileExists(_destPath))
                {
                    _storage.MoveFile(_destPath, _sourcePath);
                }
            }

            if (_onScriptChanged != null && AffectsScripts())
                _onScriptChanged();
        }

        private bool AffectsScripts()
        {
            // Папка может содержать скрипты — всегда триггерим пересборку
            if (_isFolder) return true;
            // Файл: проверяем расширение source или dest
            static bool IsScript(string p)
            {
                string ext = Path.GetExtension(p);
                return ext.Equals(".hpp", StringComparison.OrdinalIgnoreCase) ||
                       ext.Equals(".cpp", StringComparison.OrdinalIgnoreCase);
            }
            return IsScript(_sourcePath) || IsScript(_destPath);
        }
    }
}
