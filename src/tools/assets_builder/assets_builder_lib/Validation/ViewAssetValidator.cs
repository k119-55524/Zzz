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

            // 1. Одиночное поле "script" (строка) — ТРЕБУЕТСЯ СТРОГИЙ GUID
            if (root.TryGetProperty("script", out var singleScriptProp) && singleScriptProp.ValueKind == JsonValueKind.String)
            {
                string scriptRef = singleScriptProp.GetString() ?? string.Empty;
                if (!string.IsNullOrEmpty(scriptRef))
                {
                    ValidateStrictGuid(filePath, fileName, "Скрипт вьюшки", scriptRef, "script", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                }
            }

            // 2. Множественные скрипты "scripts" (массив строк) — ТРЕБУЕТСЯ СТРОГИЙ GUID
            if (root.TryGetProperty("scripts", out var scriptsArrayProp) && scriptsArrayProp.ValueKind == JsonValueKind.Array)
            {
                int index = 0;
                foreach (var scriptElem in scriptsArrayProp.EnumerateArray())
                {
                    index++;
                    string scriptRef = scriptElem.GetString() ?? string.Empty;
                    if (!string.IsNullOrEmpty(scriptRef))
                    {
                        ValidateStrictGuid(filePath, fileName, $"Скрипт вьюшки #{index}", scriptRef, "script", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                    }
                }
            }

            // 3. Скрипты во вложенных элементах вьюшки ("elements") — ТРЕБУЕТСЯ СТРОГИЙ GUID
            if (root.TryGetProperty("elements", out var elementsProp) && elementsProp.ValueKind == JsonValueKind.Array)
            {
                foreach (var elem in elementsProp.EnumerateArray())
                {
                    string elemName = elem.TryGetProperty("name", out var nameProp) ? nameProp.GetString() ?? "Element" : "Element";

                    if (elem.TryGetProperty("script", out var elemScriptProp) && elemScriptProp.ValueKind == JsonValueKind.String)
                    {
                        string scriptRef = elemScriptProp.GetString() ?? string.Empty;
                        if (!string.IsNullOrEmpty(scriptRef))
                        {
                            ValidateStrictGuid(filePath, fileName, $"Элемент вьюшки '{elemName}'", scriptRef, "script", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                        }
                    }

                    if (elem.TryGetProperty("scripts", out var elemScriptsProp) && elemScriptsProp.ValueKind == JsonValueKind.Array)
                    {
                        foreach (var scriptElem in elemScriptsProp.EnumerateArray())
                        {
                            string scriptRef = scriptElem.GetString() ?? string.Empty;
                            if (!string.IsNullOrEmpty(scriptRef))
                            {
                                ValidateStrictGuid(filePath, fileName, $"Элемент вьюшки '{elemName}'", scriptRef, "script", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                            }
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

    private void ValidateStrictGuid(
        string filePath,
        string fileName,
        string contextName,
        string referenceValue,
        string expectedType,
        IReadOnlyDictionary<string, string> guidToFileMap,
        IReadOnlyDictionary<string, string> guidToTypeMap,
        IReadOnlyDictionary<string, string> scriptNameToGuidMap,
        ValidationResult result)
    {
        bool isGuidPresent = guidToFileMap.ContainsKey(referenceValue);

        if (!isGuidPresent)
        {
            if (scriptNameToGuidMap.ContainsKey(referenceValue))
            {
                result.AddError(filePath, $"{fileName}: {contextName} использует имя скрипта '{referenceValue}' вместо обязательного GUID!");
            }
            else
            {
                result.AddError(filePath, $"{fileName}: {contextName} ссылается на неизвестный GUID или имя '{referenceValue}'!");
            }
        }
        else
        {
            string actualType = guidToTypeMap.GetValueOrDefault(referenceValue, string.Empty);
            if (!actualType.Equals(expectedType, StringComparison.OrdinalIgnoreCase))
            {
                result.AddError(filePath, $"{fileName}: {contextName} ссылается на GUID '{referenceValue}' типа '{actualType}' вместо ожидаемого типа '{expectedType}'!");
            }
        }
    }
}
