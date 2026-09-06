using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Importers;

public class DataAssetImporter : IAssetImporter
{
    public bool CanHandle(string filePath)
    {
        string ext = Path.GetExtension(filePath);
        return AssetExtensions.IsSupportedDataAssetExtension(ext);
    }

    public string GetMetaFilePath(string filePath)
    {
        return filePath + ".meta";
    }

    public string GenerateMetaJson(string filePath, string guid)
    {
        string ext = Path.GetExtension(filePath).TrimStart('.').ToLowerInvariant();
        string assetType = ext switch
        {
            "obj" => "mesh",
            "png" => "texture",
            "zmat" => "material",
            "hlsl" => "shader",
            "zp" => "prefab",
            _ => ext
        };

        var metaData = new
        {
            guid = guid,
            type = assetType
        };

        return JsonSerializer.Serialize(metaData, new JsonSerializerOptions { WriteIndented = true });
    }
}
