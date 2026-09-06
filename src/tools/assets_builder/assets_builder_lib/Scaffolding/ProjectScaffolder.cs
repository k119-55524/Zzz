using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using assets_builder_lib.Models;

namespace assets_builder_lib.Scaffolding;

public static class ProjectScaffolder
{
    public static bool CanCreateInDirectory(string directoryPath, out string errorMessage)
    {
        errorMessage = string.Empty;

        if (string.IsNullOrWhiteSpace(directoryPath))
        {
            errorMessage = "Путь к папке не указан.";
            return false;
        }

        if (!Directory.Exists(directoryPath))
        {
            try
            {
                Directory.CreateDirectory(directoryPath);
                return true;
            }
            catch (Exception ex)
            {
                errorMessage = $"Не удалось создать каталог: {ex.Message}";
                return false;
            }
        }

        var entries = Directory.EnumerateFileSystemEntries(directoryPath)
            .Where(e => !Path.GetFileName(e).StartsWith(".git", StringComparison.OrdinalIgnoreCase))
            .ToList();

        if (entries.Count > 0)
        {
            errorMessage = $"Папка не пуста ({entries.Count} элементов). Создание каркаса разрешено только в пустой папке.";
            return false;
        }

        return true;
    }

    public static bool CreateProjectScaffold(string directoryPath, string companyName, string appName, out string errorMessage)
    {
        errorMessage = string.Empty;

        if (!CanCreateInDirectory(directoryPath, out errorMessage))
            return false;

        try
        {
            Directory.CreateDirectory(Path.Combine(directoryPath, "Assets", "Scenes"));
            Directory.CreateDirectory(Path.Combine(directoryPath, "Assets", "Views"));
            Directory.CreateDirectory(Path.Combine(directoryPath, "Assets", "Data"));
            Directory.CreateDirectory(Path.Combine(directoryPath, "Scripts"));

            string buildSettingsDir = Path.Combine(directoryPath, "build_settings");
            Directory.CreateDirectory(Path.Combine(buildSettingsDir, "platforms", "windows"));
            Directory.CreateDirectory(Path.Combine(buildSettingsDir, "platforms", "android"));
            Directory.CreateDirectory(Path.Combine(buildSettingsDir, "platforms", "ios"));
            Directory.CreateDirectory(Path.Combine(buildSettingsDir, "platforms", "linux"));
            Directory.CreateDirectory(Path.Combine(buildSettingsDir, "platforms", "macos"));

            var jsonOptions = new JsonSerializerOptions { WriteIndented = true };

            string winDefault = @"{
  ""name"": ""Windows Default"",
  ""build"": {
    ""executableName"": ""GameZzz.exe"",
    ""icon"": ""Assets/Icons/win_icon.ico"",
    ""visualStudioToolset"": ""v143""
  },
  ""platform"": {
    ""windowClassName"": ""ZzzEngineWindowClass"",
    ""child_views"": [],
    ""independent_views"": []
  },
  ""startView"": {
    ""title"": ""GameZzz"",
    ""defaultSize"": {
      ""width"": 1280,
      ""height"": 720
    },
    ""windowMode"": ""Windowed"",
    ""resizable"": true
  }
}";
            File.WriteAllText(Path.Combine(buildSettingsDir, "platforms", "windows", "default.json"), winDefault);

            string androidDefault = @"{
  ""name"": ""Android Default"",
  ""build"": {
    ""packageName"": ""com.zzz.game"",
    ""minSdkVersion"": 24,
    ""targetSdkVersion"": 34
  },
  ""startView"": {
    ""orientation"": ""LandscapeLeft"",
    ""targetFPS"": 60,
    ""cutoutMode"": ""ShortEdges"",
    ""keepScreenOn"": true
  }
}";
            File.WriteAllText(Path.Combine(buildSettingsDir, "platforms", "android", "default.json"), androidDefault);

            string iosDefault = @"{
  ""name"": ""iOS Default"",
  ""build"": {
    ""bundleIdentifier"": ""com.zzz.game"",
    ""developmentTeam"": ""XYZ123456""
  },
  ""startView"": {
    ""orientation"": ""LandscapeLeft"",
    ""safeAreaMode"": ""ExtendIntoSafeArea"",
    ""homeIndicatorMode"": ""AutoHidden""
  }
}";
            File.WriteAllText(Path.Combine(buildSettingsDir, "platforms", "ios", "default.json"), iosDefault);

            string linuxDefault = @"{
  ""name"": ""Linux Default"",
  ""build"": {
    ""executableName"": ""GameZzz""
  },
  ""platform"": {
    ""child_views"": [],
    ""independent_views"": []
  },
  ""startView"": {
    ""title"": ""GameZzz"",
    ""defaultSize"": {
      ""width"": 1280,
      ""height"": 720
    },
    ""windowMode"": ""Windowed"",
    ""displayServer"": ""Auto"",
    ""resizable"": true
  }
}";
            File.WriteAllText(Path.Combine(buildSettingsDir, "platforms", "linux", "default.json"), linuxDefault);

            string macosDefault = @"{
  ""name"": ""macOS Default"",
  ""build"": {
    ""executableName"": ""GameZzz"",
    ""bundleIdentifier"": ""com.zzz.game"",
    ""icon"": ""Assets/Icons/macos_icon.icns""
  },
  ""platform"": {
    ""child_views"": [],
    ""independent_views"": []
  },
  ""startView"": {
    ""title"": ""GameZzz"",
    ""defaultSize"": {
      ""width"": 1280,
      ""height"": 720
    },
    ""windowMode"": ""Windowed"",
    ""resizable"": true
  }
}";
            File.WriteAllText(Path.Combine(buildSettingsDir, "platforms", "macos", "default.json"), macosDefault);

            var presets = new PresetsContainer
            {
                Presets = new List<BuildPreset>
                {
                    new BuildPreset
                    {
                        Name = "Default",
                        Description = "Базовый набор сборки",
                        Targets = new List<BuildPresetTarget>
                        {
                            new BuildPresetTarget { Name = "game_win", Platform = "Windows", ConfigFile = "build_settings/platforms/windows/default.json", IsEnabled = true },
                            new BuildPresetTarget { Name = "game_android", Platform = "Android", ConfigFile = "build_settings/platforms/android/default.json", IsEnabled = true },
                            new BuildPresetTarget { Name = "game_ios", Platform = "iOS", ConfigFile = "build_settings/platforms/ios/default.json", IsEnabled = true },
                            new BuildPresetTarget { Name = "game_linux", Platform = "Linux", ConfigFile = "build_settings/platforms/linux/default.json", IsEnabled = true },
                            new BuildPresetTarget { Name = "game_macos", Platform = "MacOS", ConfigFile = "build_settings/platforms/macos/default.json", IsEnabled = true }
                        }
                    }
                }
            };

            File.WriteAllText(Path.Combine(buildSettingsDir, "presets.json"), JsonSerializer.Serialize(presets, jsonOptions));

            var manifest = new ProjectManifestModel
            {
                CompanyName = string.IsNullOrWhiteSpace(companyName) ? "Zzz" : companyName,
                AppName = string.IsNullOrWhiteSpace(appName) ? "ZzzGame" : appName,
                AppVersion = "1.0.0",
                Name = string.IsNullOrWhiteSpace(appName) ? "ZzzGame" : appName,
                Version = "1.0.0",
                StartScene = string.Empty,
                BuildSettings = new BuildSettingsInfo
                {
                    PresetsFile = "build_settings/presets.json",
                    ActivePreset = "Default"
                }
            };

            File.WriteAllText(Path.Combine(directoryPath, "project.json"), JsonSerializer.Serialize(manifest, jsonOptions));

            return true;
        }
        catch (Exception ex)
        {
            errorMessage = $"Ошибка при создании каркаса: {ex.Message}";
            return false;
        }
    }
}