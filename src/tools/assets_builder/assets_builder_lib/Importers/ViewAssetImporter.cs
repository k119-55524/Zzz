using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Importers;

public class ViewAssetImporter : IAssetImporter
{
    public bool CanHandle(string filePath)
    {
        string ext = Path.GetExtension(filePath).ToLowerInvariant();
        return ext == AssetExtensions.View;
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
            type = "view"
        };

        return JsonSerializer.Serialize(metaData, new JsonSerializerOptions { WriteIndented = true });
    }
}
