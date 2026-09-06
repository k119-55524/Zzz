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

            // 2.1 Проверка стартовой сцены (start_scene), если задана явно — ТРЕБУЕТСЯ СТРОГИЙ GUID
            if (root.TryGetProperty("start_scene", out var startSceneProp) && startSceneProp.ValueKind == JsonValueKind.String)
            {
                string startSceneRef = startSceneProp.GetString() ?? string.Empty;
                if (!string.IsNullOrWhiteSpace(startSceneRef))
                {
                    ValidateStrictGuid(filePath, "project.json", "start_scene", startSceneRef, "scene", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
                }
            }

            // 2.2 Проверка стартовой вьюшки (start_view), если задана явно — ТРЕБУЕТСЯ СТРОГИЙ GUID
            if (root.TryGetProperty("start_view", out var startViewProp) && startViewProp.ValueKind == JsonValueKind.String)
            {
                string startViewRef = startViewProp.GetString() ?? string.Empty;
                if (!string.IsNullOrWhiteSpace(startViewRef))
                {
                    ValidateStrictGuid(filePath, "project.json", "start_view", startViewRef, "view", guidToFileMap, guidToTypeMap, scriptNameToGuidMap, result);
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

            // 4. Проверка глобальных параметров переходов сцен (transition)
            if (root.TryGetProperty("transition", out var transProp) && transProp.ValueKind == JsonValueKind.Object)
            {
                if (transProp.TryGetProperty("type", out var typeProp))
                {
                    string tType = typeProp.GetString() ?? string.Empty;
                    if (tType != "Instant" && tType != "FadeColor" && tType != "CrossFade")
                    {
                        result.AddError(filePath, $"project.json: Недопустимый глобальный тип перехода сцены '{tType}'. Ожидается 'Instant', 'FadeColor' или 'CrossFade'.");
                    }
                }

                if (transProp.TryGetProperty("duration", out var durProp) && durProp.ValueKind == JsonValueKind.Number)
                {
                    if (durProp.GetDouble() < 0.0)
                    {
                        result.AddError(filePath, "project.json: Длительность глобального перехода (duration) не может быть отрицательной.");
                    }
                }
                else if (transProp.TryGetProperty("durationSeconds", out var durSecProp) && durSecProp.ValueKind == JsonValueKind.Number)
                {
                    if (durSecProp.GetDouble() < 0.0)
                    {
                        result.AddError(filePath, "project.json: Длительность глобального перехода (durationSeconds) не может быть отрицательной.");
                    }
                }
            }

            // 5. Проверка имени компании и приложения (company_name/app_name) — обязательны и должны быть
            // валидными именами каталогов (см. Path::IsValidDirectoryName в движке), так как формируют
            // двухуровневый каталог пользовательских данных приложения ( %LOCALAPPDATA%/<company>/<app>/ и т.п.).
            ValidateDirectoryNameField(root, filePath, "company_name", result);
            ValidateDirectoryNameField(root, filePath, "app_name", result);
        }
        catch (Exception ex)
        {
            result.AddError(filePath, $"Ошибка чтения project.json: {ex.Message}");
        }

        return result;
    }

    private void ValidateDirectoryNameField(JsonElement root, string filePath, string fieldName, ValidationResult result)
    {
        if (!root.TryGetProperty(fieldName, out var prop) || prop.ValueKind != JsonValueKind.String)
        {
            result.AddError(filePath, $"project.json: Отсутствует обязательное строковое поле '{fieldName}'.");
            return;
        }

        string value = prop.GetString() ?? string.Empty;
        if (string.IsNullOrWhiteSpace(value))
        {
            result.AddError(filePath, $"project.json: Поле '{fieldName}' не должно быть пустым.");
            return;
        }

        if (!NativeMethods.ValidateDirectoryNameNative(value))
        {
            result.AddError(filePath, $"project.json: Поле '{fieldName}' ('{value}') содержит недопустимое имя каталога " +
                "(запрещены Path Traversal, символы < > : \" / \\ | ? *, управляющие коды, зарезервированные имена " +
                "Windows-устройств (CON, PRN, AUX, NUL, COM1-9, LPT1-9), а также завершающие точки/пробелы).");
        }
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

            bool isViewTypeMatch = expectedType.Equals("view", StringComparison.OrdinalIgnoreCase) &&
                                   actualType.Equals("view", StringComparison.OrdinalIgnoreCase);

            if (!actualType.Equals(expectedType, StringComparison.OrdinalIgnoreCase) && !isScriptTypeMatch && !isViewTypeMatch)
            {
                result.AddError(filePath, $"{fileName}: Поле '{fieldName}' ссылается на GUID '{referenceValue}' типа '{actualType}' вместо ожидаемого типа '{expectedType}'!");
            }
        }
    }
}
