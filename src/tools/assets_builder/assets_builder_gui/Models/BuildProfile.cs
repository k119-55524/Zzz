namespace assets_builder_gui.Models;

public class TargetProjectItem
{
    public bool IsEnabled { get; set; } = false;
    public string Name { get; set; } = string.Empty;
    public string ConfigJsonPath { get; set; } = string.Empty;
}

public class BuildProfile
{
    public string Id { get; set; } = Guid.NewGuid().ToString();
    public string Name { get; set; } = "Новая настройка";
    public string Configuration { get; set; } = "Debug"; // Debug, Development, Release
    public string SourcePath { get; set; } = string.Empty;
    public string DestinationPath { get; set; } = string.Empty;
    public List<TargetProjectItem> TargetProjects { get; set; } = new();
}
