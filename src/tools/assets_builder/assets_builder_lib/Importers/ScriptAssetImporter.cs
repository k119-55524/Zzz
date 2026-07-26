using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Importers;

public class ScriptAssetImporter : IAssetImporter
{
    public bool CanHandle(string filePath)
    {
        string ext = Path.GetExtension(filePath).ToLowerInvariant();
        // Подхватываем только заголовки .h и .hpp. Исполняемые .cpp файлы авто-привязываются к заголовку!
        return ext == AssetExtensions.HeaderH || ext == AssetExtensions.HeaderHpp;
    }

    public string GetMetaFilePath(string filePath)
    {
        return filePath + ".meta";
    }

    public string GenerateMetaJson(string filePath, string guid)
    {
        string className = Path.GetFileNameWithoutExtension(filePath);
        var metaData = new
        {
            guid = guid,
            type = "script",
            class_name = className
        };

        return JsonSerializer.Serialize(metaData, new JsonSerializerOptions { WriteIndented = true });
    }
}
