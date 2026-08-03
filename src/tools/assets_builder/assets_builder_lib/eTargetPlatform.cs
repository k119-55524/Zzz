using System.Text.Json.Serialization;

namespace assets_builder_lib;

[JsonConverter(typeof(JsonStringEnumConverter))]
public enum eTargetPlatform
{
    Windows,
    Linux,
    Android,
    MacOS,
    iOS
}
