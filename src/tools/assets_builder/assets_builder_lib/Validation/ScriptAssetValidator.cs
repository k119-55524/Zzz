using System.IO;

namespace assets_builder_lib.Validation;

public class ScriptAssetValidator : IAssetValidator
{
    public bool CanValidate(string filePath)
    {
        string ext = Path.GetExtension(filePath).ToLowerInvariant();
        return ext == AssetExtensions.HeaderH || ext == AssetExtensions.HeaderHpp;
    }

    public ValidationResult Validate(string filePath)
    {
        var result = new ValidationResult();
        string dir = Path.GetDirectoryName(filePath) ?? string.Empty;
        string nameWithoutExt = Path.GetFileNameWithoutExtension(filePath);
        string cppFile = Path.Combine(dir, nameWithoutExt + AssetExtensions.SourceCpp);

        if (!File.Exists(cppFile))
        {
            result.AddError(filePath, $"Скрипт {Path.GetFileName(filePath)}: Файл C++ реализации '{nameWithoutExt}.cpp' отсутствует рядом с заголовком!", isCritical: false);
        }

        return result;
    }
}
