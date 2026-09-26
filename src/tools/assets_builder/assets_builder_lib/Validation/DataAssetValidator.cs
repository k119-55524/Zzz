using System.IO;

namespace assets_builder_lib.Validation;

public class DataAssetValidator : IAssetValidator
{
    public bool CanValidate(string filePath)
    {
        string ext = Path.GetExtension(filePath);
        return AssetExtensions.IsSupportedDataAssetExtension(ext);
    }

    public ValidationResult Validate(string filePath)
    {
        var result = new ValidationResult();
        string fileName = Path.GetFileName(filePath);

        if (!File.Exists(filePath))
        {
            result.AddError(filePath, $"Файл ресурса '{fileName}' не найден на диске!");
            return result;
        }

        var fileInfo = new FileInfo(filePath);
        if (fileInfo.Length == 0)
        {
            result.AddError(filePath, $"Файл ресурса '{fileName}' имеет нулевой размер (0 байт)!");
        }

        return result;
    }
}
