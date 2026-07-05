namespace BuildConfigurator.Models;

public class BuildConfiguration
{
    public string Name { get; set; } = string.Empty;
    public string Description { get; set; } = string.Empty;
    public List<string> ActiveDefines { get; set; } = new();
}
