namespace assets_builder_lib.Validation;

public interface IAssetValidator
{
    bool CanValidate(string filePath);
    ValidationResult Validate(string filePath);
}
