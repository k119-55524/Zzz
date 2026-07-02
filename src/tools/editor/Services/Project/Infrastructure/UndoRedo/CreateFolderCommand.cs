using System;
using System.Collections.Generic;
using System.IO;

namespace editor.Services.Project.Infrastructure.UndoRedo
{
    /// <summary>
    /// Команда создания физической папки на диске с поддержкой Undo/Redo.
    /// </summary>
    public class CreateFolderCommand : IAssetsTreeCommand
    {
        private readonly string _relativePath;
        private readonly string _projectRoot;
        private readonly bool _isSystemMode;
        private readonly IFileStorage _storage;
        private readonly List<string> _createdPaths = new();

        public CreateFolderCommand(string relativePath, string projectRoot, bool isSystemMode, IFileStorage storage)
        {
            _relativePath = relativePath;
            _projectRoot = projectRoot;
            _isSystemMode = isSystemMode;
            _storage = storage;
        }

        public void Execute()
        {
            _createdPaths.Clear();
            string path = Path.Combine(_projectRoot, _relativePath);
            if (_storage.DirectoryExists(path) || File.Exists(path))
            {
                throw new IOException("Каталог или файл с таким именем уже существует.");
            }
            _storage.CreateDirectory(path);
            _createdPaths.Add(path);
        }

        public void Undo()
        {
            for (int i = _createdPaths.Count - 1; i >= 0; i--)
            {
                string path = _createdPaths[i];
                if (_storage.DirectoryExists(path))
                {
                    var entries = _storage.GetFileSystemEntries(path);
                    if (entries.Length == 0)
                    {
                        _storage.DeleteDirectory(path, recursive: false);
                    }
                }
            }
            _createdPaths.Clear();
        }
    }
}
