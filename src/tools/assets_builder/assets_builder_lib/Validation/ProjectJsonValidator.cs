using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Validation;

public class ProjectJsonValidator : IAssetValidator
{
    public bool CanValidate(string filePath)
    {
        return Path.GetFileName(filePath).Equals(AssetExtensions.ProjectJsonName, StringComparison.OrdinalIgnoreCase);
    }

    public ValidationResult Validate(string filePath)
    {
        var result = new ValidationResult();

        try
        {
            string json = File.ReadAllText(filePath);
            using var doc = JsonDocument.Parse(json);
            var root = doc.RootElement;

            // 1. Проверка наличия поля главного скрипта игры (game_scripts или game_script)
            if (!root.TryGetProperty("game_scripts", out _) && !root.TryGetProperty("game_script", out _))
            {
                result.AddError(filePath, "project.json: Отсутствует обязательное поле 'game_scripts' или 'game_script'.");
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
}
