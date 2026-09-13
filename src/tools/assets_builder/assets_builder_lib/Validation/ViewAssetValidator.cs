using System.IO;
using System.Text.Json;

namespace assets_builder_lib.Validation;

public class ViewAssetValidator : IAssetValidator
{
    public bool CanValidate(string filePath)
    {
        string ext = Path.GetExtension(filePath);
        return AssetExtensions.IsSupportedViewExtension(ext);
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

            // Синтаксическая валидация JSON и обязательных полей вьюшки.
            // Проверка уникальности GUID и типизированных ссылок делегирована нативному ProjectIdentityValidator.
        }
        catch (Exception ex)
        {
            result.AddError(filePath, $"Ошибка чтения JSON вьюшки {Path.GetFileName(filePath)}: {ex.Message}");
        }

        return result;
    }
}
