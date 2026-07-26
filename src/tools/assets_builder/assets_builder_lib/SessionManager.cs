using System.IO;
using System.Text.Json;

namespace assets_builder_lib;

public class SessionConfig
{
    public string SourceProjectPath { get; set; } = string.Empty;
    public string DestinationPath { get; set; } = string.Empty;
    public List<string> RecentSourcePaths { get; set; } = new();
    public List<string> RecentDestinationPaths { get; set; } = new();
}

public static class SessionManager
{
    private static readonly string AppDataFolder = Path.Combine(
        Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
        "Zzz",
        "AssetsBuilder",
        "1.0.0"
    );

    private static readonly string ConfigFilePath = Path.Combine(AppDataFolder, "session_config.json");

    public static SessionConfig LoadSession()
    {
        try
        {
            if (File.Exists(ConfigFilePath))
            {
                string json = File.ReadAllText(ConfigFilePath);
                var config = JsonSerializer.Deserialize<SessionConfig>(json);
                if (config != null)
                    return config;
            }
        }
        catch
        {
            // Fallback to default
        }

        return GetDefaultConfig();
    }

    public static void SaveSession(SessionConfig config)
    {
        try
        {
            if (!Directory.Exists(AppDataFolder))
            {
                Directory.CreateDirectory(AppDataFolder);
            }

            string json = JsonSerializer.Serialize(config, new JsonSerializerOptions { WriteIndented = true });
            File.WriteAllText(ConfigFilePath, json);
        }
        catch
        {
            // Ignore write errors
        }
    }

    public static SessionConfig GetDefaultConfig()
    {
        string defaultSource = @"C:\Workspaces\ZzzTest\src\projects\assets_projects\zzz_assets_test_000";
        string defaultDest = @"C:\Workspaces\ZzzTest\bin\packages\zzz_assets_test_000";

        return new SessionConfig
        {
            SourceProjectPath = defaultSource,
            DestinationPath = defaultDest,
            RecentSourcePaths = new List<string> { defaultSource },
            RecentDestinationPaths = new List<string> { defaultDest }
        };
    }
}
