using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Validation;

public class SceneAssetValidator : IAssetValidator
{
    public bool CanValidate(string filePath)
    {
        return Path.GetExtension(filePath).Equals(AssetExtensions.Scene, StringComparison.OrdinalIgnoreCase);
    }

    public ValidationResult Validate(
        string filePath,
        IReadOnlyDictionary<string, string> guidToFileMap,
        IReadOnlyDictionary<string, string> guidToTypeMap,
        IReadOnlyDictionary<string, string> scriptNameToGuidMap)
    {
        var result = new ValidationResult();

        try
        {
            string json = File.ReadAllText(filePath);
            using var doc = JsonDocument.Parse(json);
            var root = doc.RootElement;

            string fileName = Path.GetFileName(filePath);

            // 4. Корневой objects запрещен (удален в этапе 17)
            if (root.TryGetProperty("objects", out _))
            {
                result.AddError(filePath, $"{fileName}: Корневой массив 'objects' больше не поддерживается. Объекты должны объявляться внутри 'layers[].objects'.");
            }

            // 5. Проверка структуры слоев и объектов
            if (root.TryGetProperty("layers", out var layersProp) && layersProp.ValueKind == JsonValueKind.Array)
            {
                foreach (var layerElem in layersProp.EnumerateArray())
                {
                    string layerName = layerElem.TryGetProperty("name", out var lNameProp) ? lNameProp.GetString() ?? "Layer" : "Layer";
                    if (layerElem.TryGetProperty("objects", out var layerObjsProp) && layerObjsProp.ValueKind == JsonValueKind.Array)
                    {
                        foreach (var objElem in layerObjsProp.EnumerateArray())
                        {
                            ValidateObjectStructure(objElem, $"Слой '{layerName}' -> Объект", filePath, fileName, result);
                        }
                    }
                }
            }
        }
        catch (Exception ex)
        {
            result.AddError(filePath, $"Ошибка чтения JSON сцены {Path.GetFileName(filePath)}: {ex.Message}");
        }

        return result;
    }

    private void ValidateObjectStructure(
        JsonElement objElem,
        string contextPrefix,
        string filePath,
        string fileName,
        ValidationResult result)
    {
        string objName = objElem.TryGetProperty("name", out var nameProp) ? nameProp.GetString() ?? "Object" : "Object";

        // Валидация признака ECS-сущности (isEntity)
        if (objElem.TryGetProperty("isEntity", out var isEntityProp))
        {
            if (isEntityProp.ValueKind != JsonValueKind.True && isEntityProp.ValueKind != JsonValueKind.False)
            {
                result.AddError(filePath, $"{fileName}: {contextPrefix} '{objName}': поле 'isEntity' должно быть булевым (true или false).");
            }
        }

        // Рекурсивный обход children
        if (objElem.TryGetProperty("children", out var childrenProp) && childrenProp.ValueKind == JsonValueKind.Array)
        {
            foreach (var childElem in childrenProp.EnumerateArray())
            {
                ValidateObjectStructure(childElem, $"{contextPrefix} '{objName}' -> Потомок", filePath, fileName, result);
            }
        }
    }
}
