using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Validation;

public class ProjectJsonValidator : IAssetValidator
{
    public bool CanValidate(string filePath)
    {
        return Path.GetFileName(filePath).Equals(AssetExtensions.ProjectJsonName, StringComparison.OrdinalIgnoreCase);
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

            // 1. Проверка главных скриптов игры (game_scripts или game_script) — ТРЕБУЕТСЯ СТРОГИЙ GUID
            if (root.TryGetProperty("game_scripts", out var gameScriptsProp) && gameScriptsProp.ValueKind == JsonValueKind.Array)
            {
                int index = 0;
                foreach (var scriptElem in gameScriptsProp.EnumerateArray())
                {
                    index++;
                    string scriptRef = scriptElem.GetString() ?? string.Empty;
                    if (!string.IsNullOrEmpty(scriptRef))
                    {
                        ValidateStrictGuid(filePath, "project.json", $"game_scripts[{index}]", scriptRef, "script", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                    }
                }
            }
            else if (root.TryGetProperty("game_script", out var gameScriptProp))
            {
                string scriptRef = gameScriptProp.GetString() ?? string.Empty;
                if (!string.IsNullOrEmpty(scriptRef))
                {
                    ValidateStrictGuid(filePath, "project.json", "game_script", scriptRef, "script", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                }
            }
            else
            {
                result.AddError(filePath, "project.json: Отсутствует обязательное поле 'game_scripts' или 'game_script'.");
            }

            // 2. Проверка начальных сцен (scenes) — ТРЕБУЕТСЯ СТРОГИЙ GUID
            if (root.TryGetProperty("scenes", out var scenesProp) && scenesProp.ValueKind == JsonValueKind.Array)
            {
                int index = 0;
                foreach (var sceneElem in scenesProp.EnumerateArray())
                {
                    index++;
                    string sceneRef = sceneElem.GetString() ?? string.Empty;
                    if (!string.IsNullOrEmpty(sceneRef))
                    {
                        ValidateStrictGuid(filePath, "project.json", $"scenes[{index}]", sceneRef, "scene", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                    }
                }
            }

            // 3. Проверка стартовых вьюшек (views) — ТРЕБУЕТСЯ СТРОГИЙ GUID
            if (root.TryGetProperty("views", out var viewsProp) && viewsProp.ValueKind == JsonValueKind.Array)
            {
                int index = 0;
                foreach (var viewElem in viewsProp.EnumerateArray())
                {
                    index++;
                    string viewRef = viewElem.GetString() ?? string.Empty;
                    if (!string.IsNullOrEmpty(viewRef))
                    {
                        ValidateStrictGuid(filePath, "project.json", $"views[{index}]", viewRef, "view", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                    }
                }
            }
        }
        catch (Exception ex)
        {
            result.AddError(filePath, $"Ошибка чтения project.json: {ex.Message}");
        }

        return result;
    }

    private void ValidateStrictGuid(
        string filePath,
        string fileName,
        string fieldName,
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
                result.AddError(filePath, $"{fileName}: Поле '{fieldName}' использует имя скрипта '{referenceValue}' вместо обязательного GUID!");
            }
            else
            {
                result.AddError(filePath, $"{fileName}: Поле '{fieldName}' ссылается на неизвестный GUID или имя '{referenceValue}'!");
            }
        }
        else
        {
            string actualType = guidToTypeMap.GetValueOrDefault(referenceValue, string.Empty);
            bool isScriptTypeMatch = expectedType.Equals("script", StringComparison.OrdinalIgnoreCase) &&
                                     (actualType.Equals("script", StringComparison.OrdinalIgnoreCase) ||
                                      actualType.Equals("h", StringComparison.OrdinalIgnoreCase) ||
                                      actualType.Equals("hpp", StringComparison.OrdinalIgnoreCase));

            if (!actualType.Equals(expectedType, StringComparison.OrdinalIgnoreCase) && !isScriptTypeMatch)
            {
                result.AddError(filePath, $"{fileName}: Поле '{fieldName}' ссылается на GUID '{referenceValue}' типа '{actualType}' вместо ожидаемого типа '{expectedType}'!");
            }
        }
    }
}
