using System;
using System.IO;

namespace editor.Services.Project.Infrastructure
{
    /// <summary>
    /// Поддерживает соответствие .hpp/.cpp скрипта и его .meta при изменениях файлов извне
    /// редактора (Проводник, git checkout/merge, другой IDE) - генерирует недостающую .meta
    /// при появлении .hpp, удаляет .meta-сироту при удалении .hpp.
    /// </summary>
    public class ScriptAssetWatchHandler : IAssetWatchHandler
    {
        public bool CanHandle(string filePath)
        {
            string ext = Path.GetExtension(filePath);
            return ext.Equals(".hpp", StringComparison.OrdinalIgnoreCase)
                || ext.Equals(".cpp", StringComparison.OrdinalIgnoreCase);
        }

        public void OnCreated(string filePath, IFileStorage storage)
        {
            // .meta привязана к .hpp (имя класса = базовое имя .hpp); появление одинокого .cpp
            // ничего не меняет в идентичности скрипта.
            if (!Path.GetExtension(filePath).Equals(".hpp", StringComparison.OrdinalIgnoreCase))
                return;

            string baseName = Path.GetFileNameWithoutExtension(filePath);
            string? dir = Path.GetDirectoryName(filePath);
            if (dir == null)
                return;

            string metaPath = Path.Combine(dir, baseName + ".meta");
            if (storage.FileExists(metaPath))
                return;

            var data = ScriptMetaFile.CreateNew(baseName);
            ScriptMetaFile.Save(storage, metaPath, data);
            EditorLogger.LogInfo($"[Meta System] Detected external '{baseName}.hpp' - generated '{baseName}.meta' (GUID: {data.Guid}).");
        }

        public void OnDeleted(string filePath, IFileStorage storage)
        {
            if (!Path.GetExtension(filePath).Equals(".hpp", StringComparison.OrdinalIgnoreCase))
                return;

            string baseName = Path.GetFileNameWithoutExtension(filePath);
            string? dir = Path.GetDirectoryName(filePath);
            if (dir == null)
                return;

            string metaPath = Path.Combine(dir, baseName + ".meta");
            if (!storage.FileExists(metaPath))
                return;

            storage.DeleteFile(metaPath);
            EditorLogger.LogInfo($"[Meta System] Detected external deletion of '{baseName}.hpp' - removed orphan '{baseName}.meta'.");
        }
    }
}
