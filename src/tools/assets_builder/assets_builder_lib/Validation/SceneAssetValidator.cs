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

            // 1. Проверка скрипта самой сцены (ТРЕБУЕТСЯ СТРОГИЙ GUID)
            if (root.TryGetProperty("script", out var sceneScriptProp))
            {
                string scriptRef = sceneScriptProp.GetString() ?? string.Empty;
                if (!string.IsNullOrEmpty(scriptRef))
                {
                    ValidateStrictGuid(filePath, fileName, "Скрипт сцены", scriptRef, "script", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                }
            }

            // 2. Проверка скриптов вложенных объектов сцены (ТРЕБУЕТСЯ СТРОГИЙ GUID)
            if (root.TryGetProperty("objects", out var objectsProp) && objectsProp.ValueKind == JsonValueKind.Array)
            {
                foreach (var objElem in objectsProp.EnumerateArray())
                {
                    string objName = objElem.TryGetProperty("name", out var nameProp) ? nameProp.GetString() ?? "Object" : "Object";
                    if (objElem.TryGetProperty("script", out var objScriptProp))
                    {
                        string objScriptRef = objScriptProp.GetString() ?? string.Empty;
                        if (!string.IsNullOrEmpty(objScriptRef))
                        {
                            ValidateStrictGuid(filePath, fileName, $"Объект '{objName}'", objScriptRef, "script", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
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
