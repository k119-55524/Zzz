using System.Collections.Generic;
using System.Text.Json.Serialization;

namespace assets_builder_lib.Models;

public class BuildSettingsInfo
{
    [JsonPropertyName("presets_file")]
    public string PresetsFile { get; set; } = "build_settings/presets.json";

    [JsonPropertyName("active_preset")]
    public string ActivePreset { get; set; } = "Default";
}

public class ProjectManifestModel
{
    [JsonPropertyName("company_name")]
    public string CompanyName { get; set; } = string.Empty;

    [JsonPropertyName("app_name")]
    public string AppName { get; set; } = string.Empty;

    [JsonPropertyName("app_version")]
    public string AppVersion { get; set; } = "1.0.0";

    [JsonPropertyName("name")]
    public string Name { get; set; } = string.Empty;

    [JsonPropertyName("version")]
    public string Version { get; set; } = "1.0.0";

    [JsonPropertyName("start_scene")]
    public string StartScene { get; set; } = string.Empty;

    [JsonPropertyName("game_scripts")]
    public List<string> GameScripts { get; set; } = new();

    [JsonPropertyName("scenes")]
    public List<string> Scenes { get; set; } = new();

    [JsonPropertyName("views")]
    public List<string> Views { get; set; } = new();

    [JsonPropertyName("build_settings")]
    public BuildSettingsInfo? BuildSettings { get; set; }
}
