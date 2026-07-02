namespace editor.Services.Project.Infrastructure.UndoRedo
{
    /// <summary>
    /// Обновляет поле "class_name" в .meta файле скрипта (GUID не трогает).
    /// Применяется после физического переименования .meta на новый путь.
    /// </summary>
    public class UpdateScriptMetaClassNameCommand : IAssetsTreeCommand
    {
        private readonly string _metaPath;
        private readonly string _newClassName;
        private readonly IFileStorage _storage;
        private string? _previousContent;

        public UpdateScriptMetaClassNameCommand(string metaPath, string newClassName, IFileStorage storage)
        {
            _metaPath = metaPath;
            _newClassName = newClassName;
            _storage = storage;
        }

        public void Execute()
        {
            if (!_storage.FileExists(_metaPath))
                return;

            _previousContent = _storage.ReadAllText(_metaPath);

            var data = ScriptMetaFile.Load(_storage, _metaPath);
            if (data == null)
                return;

            data.ClassName = _newClassName;
            ScriptMetaFile.Save(_storage, _metaPath, data);
        }

        public void Undo()
        {
            if (_previousContent != null && _storage.FileExists(_metaPath))
            {
                _storage.WriteAllText(_metaPath, _previousContent);
            }
        }
    }
}
