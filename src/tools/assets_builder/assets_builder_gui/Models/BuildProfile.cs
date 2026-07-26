namespace assets_builder_gui.Models;

public class BuildProfile
{
    public string Id { get; set; } = Guid.NewGuid().ToString();
    public string Name { get; set; } = "Новая настройка";
    public string Configuration { get; set; } = "Debug"; // Debug, Development, Release
    public string SourcePath { get; set; } = string.Empty;
    public string DestinationPath { get; set; } = string.Empty;
}
