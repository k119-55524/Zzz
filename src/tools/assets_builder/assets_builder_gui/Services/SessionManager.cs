using System.IO;
using System.Text.Json;
using assets_builder_gui.Models;

namespace assets_builder_gui.Services;

public class SessionConfig
{
    public List<BuildProfile> Profiles { get; set; } = new();
    public string SelectedProfileId { get; set; } = string.Empty;

    // Геометрия окна
    public double WindowWidth { get; set; } = 860;
    public double WindowHeight { get; set; } = 720;
    public double WindowLeft { get; set; } = 100;
    public double WindowTop { get; set; } = 100;
    public bool IsWindowMaximized { get; set; } = false;
}

public static class SessionManager
{
    private static readonly string AppDataFolder = Path.Combine(
        Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
        "Zzz",
        "AssetsBuilder"
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
                if (config != null && config.Profiles != null && config.Profiles.Count > 0)
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
        string workspaceProjects = @"C:\Workspaces\ZzzTest\src\projects";
        var defaultProfile = new BuildProfile
        {
            Id = Guid.NewGuid().ToString(),
            Name = "zzz_assets_test_000",
            SourcePath = Path.Combine(workspaceProjects, "assets_projects", "zzz_assets_test_000"),
            DestinationPath = Path.Combine(workspaceProjects, "assets_projects", "zzz_assets_test_000_build"),
            TargetProjects = new List<TargetProjectItem>
            {
                new TargetProjectItem { IsEnabled = true, Name = "game_win", ConfigJsonPath = Path.Combine(workspaceProjects, "game_win", "assets_config.json") },
                new TargetProjectItem { IsEnabled = true, Name = "game_linux", ConfigJsonPath = Path.Combine(workspaceProjects, "game_linux", "assets_config.json") },
                new TargetProjectItem { IsEnabled = true, Name = "game_android", ConfigJsonPath = Path.Combine(workspaceProjects, "game_android", "assets_config.json") },
                new TargetProjectItem { IsEnabled = true, Name = "game_ios", ConfigJsonPath = Path.Combine(workspaceProjects, "game_ios", "assets_config.json") },
                new TargetProjectItem { IsEnabled = true, Name = "game_macos", ConfigJsonPath = Path.Combine(workspaceProjects, "game_macos", "assets_config.json") }
            }
        };

        return new SessionConfig
        {
            Profiles = new List<BuildProfile> { defaultProfile },
            SelectedProfileId = defaultProfile.Id,
            WindowWidth = 860,
            WindowHeight = 720,
            WindowLeft = 100,
            WindowTop = 100,
            IsWindowMaximized = false
        };
    }
}
