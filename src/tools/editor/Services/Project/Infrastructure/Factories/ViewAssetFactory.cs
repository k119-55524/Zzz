using editor.Services.Project.FileTypes.Assets;
using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.Infrastructure.Factories
{
    public class ViewAssetFactory : IAssetFactory
    {
        public string Extension => ".zv";

        public void CreateAsset(string fullPath, string baseName, string guid, IFileStorage storage)
        {
            var data = new ViewAssetData { Version = "1.0" };
            storage.WriteAllText(fullPath, ViewAssetParser.Serialize(data));

            var meta = AssetMetaFile.CreateNew(baseName, "", "View");
            meta.Guid = guid;
            AssetMetaFile.Save(storage, fullPath + ".meta", meta);
        }
    }
}
