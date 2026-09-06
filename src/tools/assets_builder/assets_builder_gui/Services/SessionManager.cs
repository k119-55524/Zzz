using System.IO;
using System.Text.Json;
using assets_builder_gui.Models;

namespace assets_builder_gui.Services;

public class SessionConfig
{
    public List<BuildProfile> Profiles { get; set; } = new();
    public string SelectedProfileId { get; set; } = string.Empty;

    // Путь к папке целевых проектов (src/projects). Если не задан, определяется динамически.
    public string WorkspaceProjectsPath { get; set; } = string.Empty;

    // Геометрия окна
    public double WindowWidth { get; set; } = 860;
    public double WindowHeight { get; set; } = 720;
    public double WindowLeft { get; set; } = 100;
    public double WindowTop { get; set; } = 100;
    public bool IsWindowMaximized { get; set; } = false;
    public double PresetsPanelHeight { get; set; } = 260;
}

public static class SessionManager
{
    public static readonly string AppDataFolder = Path.Combine(
        Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
        "Zzz",
        "AssetsBuilder"
    );

    /// <summary>
    /// Общая папка сборки для всех проектов/профилей - лежит рядом с настройками сборщика,
    /// а не внутри исходной папки проекта.
    /// </summary>
    public static string GetDefaultBuildPath()
    {
        return Path.Combine(AppDataFolder, ".build");
    }

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
                {
                    foreach (var p in config.Profiles)
                    {
                        bool isOldStyleDefault = !string.IsNullOrEmpty(p.SourcePath) &&
                            string.Equals(p.DestinationPath, Path.Combine(p.SourcePath, ".build"), StringComparison.OrdinalIgnoreCase);

                        if (string.IsNullOrEmpty(p.DestinationPath) || p.DestinationPath.EndsWith("_build") || p.DestinationPath.Contains("builds") || isOldStyleDefault)
                        {
                            p.DestinationPath = GetDefaultBuildPath();
                        }
                    }
                    return config;
                }
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

    public static string ResolveWorkspaceProjectsPath(string? customPath = null)
    {
        if (!string.IsNullOrWhiteSpace(customPath) && Directory.Exists(customPath))
        {
            return customPath;
        }

        // Поиск папки src/projects относительно текущего приложения (dist/Debug, bin, и т.д.)
        string? current = AppDomain.CurrentDomain.BaseDirectory;
        while (!string.IsNullOrEmpty(current))
        {
            string candidate = Path.Combine(current, "src", "projects");
            if (Directory.Exists(candidate))
            {
                return candidate;
            }

            string rootMarker = Path.Combine(current, "CMakeLists.txt");
            if (File.Exists(rootMarker))
            {
                string p = Path.Combine(current, "src", "projects");
                if (Directory.Exists(p)) return p;
            }

            var parent = Directory.GetParent(current);
            if (parent == null) break;
            current = parent.FullName;
        }

        // Fallback на стандартный путь репозитория
        return @"C:\Workspaces\ZzzTest\src\projects";
    }

    public static SessionConfig GetDefaultConfig()
    {
        string workspaceProjects = ResolveWorkspaceProjectsPath();
        string defaultSource = Path.Combine(workspaceProjects, "assets_projects", "zzz_assets_test_000");
        var defaultProfile = new BuildProfile
        {
            Id = Guid.NewGuid().ToString(),
            Name = "zzz_assets_test_000",
            SourcePath = defaultSource,
            DestinationPath = GetDefaultBuildPath(),
            ActivePresetName = "Default"
        };

        return new SessionConfig
        {
            Profiles = new List<BuildProfile> { defaultProfile },
            SelectedProfileId = defaultProfile.Id,
            WorkspaceProjectsPath = workspaceProjects,
            WindowWidth = 860,
            WindowHeight = 720,
            WindowLeft = 100,
            WindowTop = 100,
            IsWindowMaximized = false,
            PresetsPanelHeight = 260
        };
    }
}
