using System.IO;

namespace assets_builder_lib.Validation;

public class DefaultAssetValidator : IAssetValidator
{
    public bool CanValidate(string filePath)
    {
        return true; // Fallback for all other files
    }

    public ValidationResult Validate(
        string filePath,
        IReadOnlyDictionary<string, string> guidToFileMap,
        IReadOnlyDictionary<string, string> guidToTypeMap,
        IReadOnlyDictionary<string, string> scriptNameToGuidMap)
    {
        var result = new ValidationResult();
        if (!File.Exists(filePath))
        {
            result.AddError(filePath, $"Файл ресурса '{Path.GetFileName(filePath)}' не найден на диске!");
        }

        return result;
    }
}
