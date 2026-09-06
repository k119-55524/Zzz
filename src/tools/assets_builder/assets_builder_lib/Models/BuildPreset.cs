using System.Collections.Generic;
using System.Text.Json.Serialization;

namespace assets_builder_lib.Models;

public class BuildPresetTarget
{
    [JsonPropertyName("name")]
    public string Name { get; set; } = string.Empty;

    [JsonPropertyName("project_path")]
    public string ProjectPath { get; set; } = string.Empty;

    [JsonPropertyName("platform")]
    public string Platform { get; set; } = string.Empty;

    [JsonPropertyName("config_file")]
    public string ConfigFile { get; set; } = string.Empty;

    [JsonIgnore]
    public bool IsEnabled => !string.IsNullOrWhiteSpace(ConfigFile);
}

public class BuildPreset
{
    [JsonPropertyName("name")]
    public string Name { get; set; } = string.Empty;

    [JsonPropertyName("description")]
    public string Description { get; set; } = string.Empty;

    [JsonPropertyName("targets")]
    public List<BuildPresetTarget> Targets { get; set; } = new();
}

public class PresetsContainer
{
    [JsonPropertyName("presets")]
    public List<BuildPreset> Presets { get; set; } = new();
}
