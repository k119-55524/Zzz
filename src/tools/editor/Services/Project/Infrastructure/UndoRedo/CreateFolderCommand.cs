using System;
using System.IO;

namespace editor.Services.Project.Infrastructure.UndoRedo
{
    /// <summary>
    /// Команда создания физической папки на диске с поддержкой Undo/Redo.
    /// </summary>
    public class CreateFolderCommand : ICommand
    {
        private readonly string _relativePath;
        private readonly string _projectRoot;
        private readonly bool _isSystemMode;
        private readonly IFileStorage _storage;
        private bool _created;
        private string? _physicalPath;

        public CreateFolderCommand(string relativePath, string projectRoot, bool isSystemMode, IFileStorage storage)
        {
            _relativePath = relativePath;
            _projectRoot = projectRoot;
            _isSystemMode = isSystemMode;
            _storage = storage;
        }

        public void Execute()
        {
            if (!_isSystemMode) return;

            _physicalPath = Path.Combine(_projectRoot, _relativePath);
            if (!_storage.DirectoryExists(_physicalPath))
            {
                _storage.CreateDirectory(_physicalPath);
                _created = true;
            }
        }

        public void Undo()
        {
            if (!_isSystemMode) return;

            if (_created && _physicalPath != null && _storage.DirectoryExists(_physicalPath))
            {
                var entries = _storage.GetFileSystemEntries(_physicalPath);
                if (entries.Length == 0)
                {
                    _storage.DeleteDirectory(_physicalPath, recursive: false);
                    _created = false;
                }
            }
        }
    }
}
