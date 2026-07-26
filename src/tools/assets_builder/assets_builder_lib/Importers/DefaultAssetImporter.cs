using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Importers;

public class DefaultAssetImporter : IAssetImporter
{
    public bool CanHandle(string filePath)
    {
        return true; // Fallback for all other files
    }

    public string GetMetaFilePath(string filePath)
    {
        return filePath + ".meta";
    }

    public string GenerateMetaJson(string filePath, string guid)
    {
        string ext = Path.GetExtension(filePath).TrimStart('.').ToLowerInvariant();
        var metaData = new
        {
            guid = guid,
            type = string.IsNullOrEmpty(ext) ? "binary" : ext
        };

        return JsonSerializer.Serialize(metaData, new JsonSerializerOptions { WriteIndented = true });
    }
}
