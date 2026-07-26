using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Validation;

public class ViewAssetValidator : IAssetValidator
{
    public bool CanValidate(string filePath)
    {
        return Path.GetExtension(filePath).Equals(AssetExtensions.View, StringComparison.OrdinalIgnoreCase);
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

            if (root.TryGetProperty("script", out var viewScriptProp))
            {
                string scriptRef = viewScriptProp.GetString() ?? string.Empty;
                if (!string.IsNullOrEmpty(scriptRef))
                {
                    bool isByName = scriptNameToGuidMap.ContainsKey(scriptRef);
                    bool isByGuid = guidToFileMap.ContainsKey(scriptRef);

                    if (!isByName && !isByGuid)
                    {
                        result.AddError(filePath, $"{fileName}: Скрипт вьюшки '{scriptRef}' не найден (ни по имени, ни по GUID)!");
                    }
                    else if (isByGuid)
                    {
                        string targetType = guidToTypeMap.GetValueOrDefault(scriptRef, string.Empty);
                        if (targetType != "script")
                        {
                            result.AddError(filePath, $"{fileName}: Скрипт вьюшки указывает на GUID типа '{targetType}' вместо 'script'!");
                        }
                    }
                }
            }
        }
        catch (Exception ex)
        {
            result.AddError(filePath, $"Ошибка чтения JSON вьюшки {Path.GetFileName(filePath)}: {ex.Message}");
        }

        return result;
    }
}
