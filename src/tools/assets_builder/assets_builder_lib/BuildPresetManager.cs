using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;
using assets_builder_lib.Models;

namespace assets_builder_lib;

public class PlatformConfigOption
{
    public string DisplayName { get; set; } = string.Empty;
    public string RelativePath { get; set; } = string.Empty;

    public override string ToString() => DisplayName;
}

public static class BuildPresetManager
{
    private static readonly JsonSerializerOptions JsonOptions = new() { WriteIndented = true };

    public static PresetsContainer? LoadPresets(string projectPath, string presetsRelativePath)
    {
        string fullPath = Path.Combine(projectPath, presetsRelativePath);
        if (!File.Exists(fullPath)) return null;

        try
        {
            string json = File.ReadAllText(fullPath);
            return JsonSerializer.Deserialize<PresetsContainer>(json);
        }
        catch
        {
            return null;
        }
    }

    public static bool SavePresets(string projectPath, string presetsRelativePath, PresetsContainer container)
    {
        string fullPath = Path.Combine(projectPath, presetsRelativePath);
        try
        {
            string dir = Path.GetDirectoryName(fullPath)!;
            if (!Directory.Exists(dir)) Directory.CreateDirectory(dir);

            string json = JsonSerializer.Serialize(container, JsonOptions);
            File.WriteAllText(fullPath, json);
            return true;
        }
        catch
        {
            return false;
        }
    }

    public static List<PlatformConfigOption> GetAvailablePlatformConfigs(string projectPath, string platformName)
    {
        var options = new List<PlatformConfigOption>
        {
            new PlatformConfigOption { DisplayName = "None", RelativePath = string.Empty }
        };

        if (string.IsNullOrWhiteSpace(projectPath) || !Directory.Exists(projectPath))
            return options;

        string platformDir = Path.Combine(projectPath, "build_settings", "platforms", platformName.ToLowerInvariant());
        if (!Directory.Exists(platformDir))
            return options;

        foreach (var file in Directory.EnumerateFiles(platformDir, "*.json"))
        {
            string relPath = Path.GetRelativePath(projectPath, file).Replace('\\', '/');
            string displayName = Path.GetFileNameWithoutExtension(file);

            try
            {
                string json = File.ReadAllText(file);
                using var doc = JsonDocument.Parse(json);
                if (doc.RootElement.TryGetProperty("name", out var nameProp))
                {
                    string? n = nameProp.GetString();
                    if (!string.IsNullOrWhiteSpace(n))
                        displayName = n;
                }
            }
            catch { }

            options.Add(new PlatformConfigOption
            {
                DisplayName = displayName,
                RelativePath = relPath
            });
        }

        return options;
    }
}
