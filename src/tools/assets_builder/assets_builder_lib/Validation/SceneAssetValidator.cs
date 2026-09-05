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

            // 2. Проверка скриптов самой сцены в массиве scripts (если задано массивом)
            if (root.TryGetProperty("scripts", out var sceneScriptsProp) && sceneScriptsProp.ValueKind == JsonValueKind.Array)
            {
                int sIndex = 0;
                foreach (var sElem in sceneScriptsProp.EnumerateArray())
                {
                    sIndex++;
                    string scriptRef = sElem.GetString() ?? string.Empty;
                    if (!string.IsNullOrEmpty(scriptRef))
                    {
                        ValidateStrictGuid(filePath, fileName, $"Скрипт сцены #{sIndex}", scriptRef, "script", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                    }
                }
            }

            // 3. Проверка параметров переходов сцены (transition)
            if (root.TryGetProperty("transition", out var transProp) && transProp.ValueKind == JsonValueKind.Object)
            {
                if (transProp.TryGetProperty("type", out var typeProp))
                {
                    string tType = typeProp.GetString() ?? string.Empty;
                    if (tType != "Instant" && tType != "FadeColor" && tType != "CrossFade")
                    {
                        result.AddError(filePath, $"{fileName}: Недопустимый тип перехода сцены '{tType}'. Ожидается 'Instant', 'FadeColor' или 'CrossFade'.");
                    }
                }

                if (transProp.TryGetProperty("duration", out var durProp) && durProp.ValueKind == JsonValueKind.Number)
                {
                    if (durProp.GetDouble() < 0.0)
                    {
                        result.AddError(filePath, $"{fileName}: Длительность перехода (duration) не может быть отрицательной.");
                    }
                }
                else if (transProp.TryGetProperty("durationSeconds", out var durSecProp) && durSecProp.ValueKind == JsonValueKind.Number)
                {
                    if (durSecProp.GetDouble() < 0.0)
                    {
                        result.AddError(filePath, $"{fileName}: Длительность перехода (durationSeconds) не может быть отрицательной.");
                    }
                }
            }

            if (root.TryGetProperty("transitionSource", out var sourceProp))
            {
                string srcStr = sourceProp.GetString() ?? string.Empty;
                if (srcStr != "UseGlobal" && srcStr != "Custom")
                {
                    result.AddError(filePath, $"{fileName}: Недопустимый источник перехода '{srcStr}'. Ожидается 'UseGlobal' или 'Custom'.");
                }
            }

            // 4. Проверка объектов сцены (как в корневом массиве objects, так и внутри layers[].objects)
            if (root.TryGetProperty("objects", out var objectsProp) && objectsProp.ValueKind == JsonValueKind.Array)
            {
                foreach (var objElem in objectsProp.EnumerateArray())
                {
                    ValidateObjectElement(objElem, "Объект", filePath, fileName, guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                }
            }

            if (root.TryGetProperty("layers", out var layersProp) && layersProp.ValueKind == JsonValueKind.Array)
            {
                foreach (var layerElem in layersProp.EnumerateArray())
                {
                    string layerName = layerElem.TryGetProperty("name", out var lNameProp) ? lNameProp.GetString() ?? "Layer" : "Layer";
                    if (layerElem.TryGetProperty("objects", out var layerObjsProp) && layerObjsProp.ValueKind == JsonValueKind.Array)
                    {
                        foreach (var objElem in layerObjsProp.EnumerateArray())
                        {
                            ValidateObjectElement(objElem, $"Слой '{layerName}' -> Объект", filePath, fileName, guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
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

    private void ValidateObjectElement(
        JsonElement objElem,
        string contextPrefix,
        string filePath,
        string fileName,
        IReadOnlyDictionary<string, string> guidToFileMap,
        IReadOnlyDictionary<string, string> guidToTypeMap,
        IReadOnlyDictionary<string, string> scriptNameToGuidMap,
        ValidationResult result)
    {
        string objName = objElem.TryGetProperty("name", out var nameProp) ? nameProp.GetString() ?? "Object" : "Object";

        // Валидация скрипта объекта
        if (objElem.TryGetProperty("script", out var objScriptProp))
        {
            string objScriptRef = objScriptProp.GetString() ?? string.Empty;
            if (!string.IsNullOrEmpty(objScriptRef))
            {
                ValidateStrictGuid(filePath, fileName, $"{contextPrefix} '{objName}'", objScriptRef, "script", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
            }
        }

        if (objElem.TryGetProperty("scripts", out var objScriptsProp) && objScriptsProp.ValueKind == JsonValueKind.Array)
        {
            int idx = 0;
            foreach (var sElem in objScriptsProp.EnumerateArray())
            {
                idx++;
                string sRef = sElem.GetString() ?? string.Empty;
                if (!string.IsNullOrEmpty(sRef))
                {
                    ValidateStrictGuid(filePath, fileName, $"{contextPrefix} '{objName}' (скрипт #{idx})", sRef, "script", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                }
            }
        }

        // Валидация меша объекта (render.mesh)
        if (objElem.TryGetProperty("render", out var renderProp) && renderProp.ValueKind == JsonValueKind.Object)
        {
            if (renderProp.TryGetProperty("mesh", out var meshProp))
            {
                string meshGuid = meshProp.GetString() ?? string.Empty;
                if (!string.IsNullOrEmpty(meshGuid))
                {
                    ValidateStrictGuid(filePath, fileName, $"{contextPrefix} '{objName}' (меш)", meshGuid, "mesh", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                }
            }
        }
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
            bool isScriptTypeMatch = expectedType.Equals("script", StringComparison.OrdinalIgnoreCase) &&
                                     (actualType.Equals("script", StringComparison.OrdinalIgnoreCase) ||
                                      actualType.Equals("h", StringComparison.OrdinalIgnoreCase) ||
                                      actualType.Equals("hpp", StringComparison.OrdinalIgnoreCase));

            bool isMeshTypeMatch = expectedType.Equals("mesh", StringComparison.OrdinalIgnoreCase) &&
                                   (actualType.Equals("mesh", StringComparison.OrdinalIgnoreCase) ||
                                    actualType.Equals("obj", StringComparison.OrdinalIgnoreCase));

            if (!actualType.Equals(expectedType, StringComparison.OrdinalIgnoreCase) && !isScriptTypeMatch && !isMeshTypeMatch)
            {
                result.AddError(filePath, $"{fileName}: {contextName} ссылается на GUID '{referenceValue}' типа '{actualType}' вместо ожидаемого типа '{expectedType}'!");
            }
        }
    }
}
