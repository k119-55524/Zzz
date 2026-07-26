namespace assets_builder_lib.Validation;

public class ValidationError
{
    public string FilePath { get; set; } = string.Empty;
    public string Message { get; set; } = string.Empty;
    public bool IsCritical { get; set; } = true;
}

public class ValidationResult
{
    public bool IsValid => Errors.Count == 0;
    public List<ValidationError> Errors { get; } = new();

    public void AddError(string filePath, string message, bool isCritical = true)
    {
        Errors.Add(new ValidationError
        {
            FilePath = filePath,
            Message = message,
            IsCritical = isCritical
        });
    }
}
