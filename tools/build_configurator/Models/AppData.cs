namespace BuildConfigurator.Models;

public class AppData
{
    public List<Define> Defines { get; set; } = new();
    public List<BuildConfiguration> Configurations { get; set; } = new();
}
