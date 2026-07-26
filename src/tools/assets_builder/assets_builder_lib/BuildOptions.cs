namespace assets_builder_lib;

public class BuildOptions
{
    public string SourcePath { get; set; } = string.Empty;
    public string DestinationPath { get; set; } = string.Empty;

    public string ProjectJsonPath => System.IO.Path.Combine(SourcePath, AssetExtensions.ProjectJsonName);
}
