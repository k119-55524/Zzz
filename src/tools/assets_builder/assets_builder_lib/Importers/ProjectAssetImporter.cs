using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Importers;

public class ProjectAssetImporter : IAssetImporter
{
    public bool CanHandle(string filePath)
    {
        string fileName = Path.GetFileName(filePath).ToLowerInvariant();
        return fileName == AssetExtensions.ProjectJsonName;
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
            type = "project"
        };

        return JsonSerializer.Serialize(metaData, new JsonSerializerOptions { WriteIndented = true });
    }
}
