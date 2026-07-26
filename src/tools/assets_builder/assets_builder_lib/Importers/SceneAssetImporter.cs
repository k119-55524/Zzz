using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Importers;

public class SceneAssetImporter : IAssetImporter
{
    public bool CanHandle(string filePath)
    {
        string ext = Path.GetExtension(filePath).ToLowerInvariant();
        return ext == AssetExtensions.Scene;
    }

    public string GetMetaFilePath(string filePath)
    {
        return filePath + ".meta";
    }

    public string GenerateMetaJson(string filePath, string guid)
    {
        var metaData = new
        {
            guid = guid,
            type = "scene"
        };

        return JsonSerializer.Serialize(metaData, new JsonSerializerOptions { WriteIndented = true });
    }
}
