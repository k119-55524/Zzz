using editor.Services.Project.FileTypes.Assets;
using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.Infrastructure.Factories
{
    public class SceneAssetFactory : IAssetFactory
    {
        public string Extension => ".zscene";

        public void CreateAsset(string fullPath, string baseName, string guid, IFileStorage storage)
        {
            var data = new SceneAssetData { Version = "1.0" };
            storage.WriteAllText(fullPath, SceneAssetParser.Serialize(data));

            var meta = AssetMetaFile.CreateNew(baseName, "", "Scene");
            meta.Guid = guid;
            AssetMetaFile.Save(storage, fullPath + ".meta", meta);
        }
    }
}
