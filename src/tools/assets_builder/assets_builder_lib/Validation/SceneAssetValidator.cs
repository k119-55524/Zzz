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

            // 1. Проверка скрипта самой сцены (по имени ИЛИ по GUID)
            if (root.TryGetProperty("script", out var sceneScriptProp))
            {
                string scriptRef = sceneScriptProp.GetString() ?? string.Empty;
                if (!string.IsNullOrEmpty(scriptRef))
                {
                    ValidateScriptReference(filePath, fileName, "Скрипт сцены", scriptRef, guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                }
            }

            // 2. Проверка скриптов вложенных объектов сцены
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
                            ValidateScriptReference(filePath, fileName, $"Объект '{objName}'", objScriptRef, guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
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

    private void ValidateScriptReference(
        string filePath,
        string fileName,
        string contextName,
        string scriptRef,
        IReadOnlyDictionary<string, string> guidToFileMap,
        IReadOnlyDictionary<string, string> guidToTypeMap,
        IReadOnlyDictionary<string, string> scriptNameToGuidMap,
        ValidationResult result)
    {
        bool isByName = scriptNameToGuidMap.ContainsKey(scriptRef);
        bool isByGuid = guidToFileMap.ContainsKey(scriptRef);

        if (!isByName && !isByGuid)
        {
            result.AddError(filePath, $"{fileName}: {contextName} ссылается на ненайденный скрипт '{scriptRef}' (ни по имени, ни по GUID)!");
        }
        else if (isByGuid)
        {
            string targetType = guidToTypeMap.GetValueOrDefault(scriptRef, string.Empty);
            if (targetType != "script")
            {
                result.AddError(filePath, $"{fileName}: {contextName} ссылается на GUID '{scriptRef}' типа '{targetType}' вместо типа 'script'!");
            }
        }
    }
}
