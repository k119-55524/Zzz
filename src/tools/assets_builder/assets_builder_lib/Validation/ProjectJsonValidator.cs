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

            // 1. Проверка главного скрипта игры (game_script) — разрешено имя скрипта ИЛИ GUID скрипта
            if (root.TryGetProperty("game_script", out var gameScriptProp))
            {
                string scriptRef = gameScriptProp.GetString() ?? string.Empty;
                if (!string.IsNullOrEmpty(scriptRef))
                {
                    bool isByName = scriptNameToGuidMap.ContainsKey(scriptRef);
                    bool isByGuid = guidToFileMap.ContainsKey(scriptRef);

                    if (!isByName && !isByGuid)
                    {
                        result.AddError(filePath, $"project.json: Указанный главный скрипт 'game_script': '{scriptRef}' не найден в проекте ни по имени, ни по GUID!");
                    }
                    else if (isByGuid)
                    {
                        // Проверка типа ресурса по GUID
                        string targetType = guidToTypeMap.GetValueOrDefault(scriptRef, string.Empty);
                        if (targetType != "script")
                        {
                            result.AddError(filePath, $"project.json: Поле 'game_script' указывает на GUID типа '{targetType}' вместо 'script'!");
                        }
                    }
                }
            }

            // 2. Проверка начальных сцен (scenes) — разрешен относительный путь ИЛИ GUID сцены
            if (root.TryGetProperty("scenes", out var scenesProp) && scenesProp.ValueKind == JsonValueKind.Array)
            {
                string projectDir = Path.GetDirectoryName(filePath) ?? string.Empty;
                foreach (var sceneElem in scenesProp.EnumerateArray())
                {
                    string sceneRef = sceneElem.GetString() ?? string.Empty;
                    bool isByGuid = guidToFileMap.ContainsKey(sceneRef);

                    if (isByGuid)
                    {
                        string targetType = guidToTypeMap.GetValueOrDefault(sceneRef, string.Empty);
                        if (targetType != "scene")
                        {
                            result.AddError(filePath, $"project.json: Элемент в 'scenes' ссылается на GUID типа '{targetType}' вместо 'scene'!");
                        }
                    }
                    else
                    {
                        string fullScenePath = Path.Combine(projectDir, sceneRef);
                        if (!File.Exists(fullScenePath))
                        {
                            result.AddError(filePath, $"project.json: Указанный файл сцены '{sceneRef}' не существует на диске!");
                        }
                    }
                }
            }

            // 3. Проверка стартовых вьюшек (views) — разрешен относительный путь ИЛИ GUID вьюшки
            if (root.TryGetProperty("views", out var viewsProp) && viewsProp.ValueKind == JsonValueKind.Array)
            {
                string projectDir = Path.GetDirectoryName(filePath) ?? string.Empty;
                foreach (var viewElem in viewsProp.EnumerateArray())
                {
                    string viewRef = viewElem.GetString() ?? string.Empty;
                    bool isByGuid = guidToFileMap.ContainsKey(viewRef);

                    if (isByGuid)
                    {
                        string targetType = guidToTypeMap.GetValueOrDefault(viewRef, string.Empty);
                        if (targetType != "view")
                        {
                            result.AddError(filePath, $"project.json: Элемент в 'views' ссылается на GUID типа '{targetType}' вместо 'view'!");
                        }
                    }
                    else
                    {
                        string fullViewPath = Path.Combine(projectDir, viewRef);
                        if (!File.Exists(fullViewPath))
                        {
                            result.AddError(filePath, $"project.json: Указанный файл вьюшки '{viewRef}' не существует на диске!");
                        }
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
}
