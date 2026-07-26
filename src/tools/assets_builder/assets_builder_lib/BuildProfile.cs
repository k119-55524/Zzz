namespace assets_builder_lib;

public class BuildProfile
{
    public string Id { get; set; } = Guid.NewGuid().ToString();
    public string Name { get; set; } = "Новая настройка";
    public string ScriptsPath { get; set; } = string.Empty;
    public string AssetsPath { get; set; } = string.Empty;
    public string DestinationPath { get; set; } = string.Empty;
}
