using System.Text.Json.Serialization;

namespace assets_builder_lib.Models;

public class PlatformConfigFile
{
    [JsonPropertyName("name")]
    public string Name { get; set; } = string.Empty;
}
