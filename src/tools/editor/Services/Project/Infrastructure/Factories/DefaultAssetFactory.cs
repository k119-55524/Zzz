using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.Infrastructure.Factories
{
    public class DefaultAssetFactory : IAssetFactory
    {
        public string Extension => "";

        public void CreateAsset(string fullPath, string baseName, string guid, IFileStorage storage)
        {
            storage.WriteAllText(fullPath, "");

            var meta = AssetMetaFile.CreateNew(baseName);
            meta.Guid = guid;
            AssetMetaFile.Save(storage, fullPath + ".meta", meta);
        }
    }
}
