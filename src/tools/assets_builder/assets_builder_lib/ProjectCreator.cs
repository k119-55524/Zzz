using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using assets_builder_lib.Models;

namespace assets_builder_lib;

public static class ProjectCreator
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
            errorMessage = $"Папка не пуста ({entries.Count} элементов). Создание проекта разрешено только в пустой папке.";
            return false;
        }

        return true;
    }

    public static bool CreateProject(string directoryPath, string companyName, string appName, out string errorMessage)
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
    ""version"": ""1.0.0"",
    ""buildNumber"": 1,
    ""icon"": ""Assets/Icons/win_icon.ico"",
    ""visualStudioToolset"": ""v143""
  },
  ""platform"": {
    ""windowClassName"": ""ZzzEngineWindowClass"",
    ""child_views"": [],
    ""independent_views"": []
  },
  ""add_scripts"": [],
  ""remove_scripts"": [],
  ""add_scenes"": [],
  ""remove_scenes"": [],
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
            File.WriteAllText(Path.Combine(buildSettingsDir, "platforms", "windows", "default_config.json"), winDefault);

            string androidDefault = @"{
  ""name"": ""Android Default"",
  ""build"": {
    ""packageName"": ""com.zzz.game"",
    ""versionName"": ""1.0.0"",
    ""versionCode"": 1,
    ""minSdkVersion"": 24,
    ""targetSdkVersion"": 34
  },
  ""add_scripts"": [],
  ""remove_scripts"": [],
  ""add_scenes"": [],
  ""remove_scenes"": [],
  ""startView"": {
    ""orientation"": ""LandscapeLeft"",
    ""targetFPS"": 60,
    ""cutoutMode"": ""ShortEdges"",
    ""keepScreenOn"": true
  }
}";
            File.WriteAllText(Path.Combine(buildSettingsDir, "platforms", "android", "default_config.json"), androidDefault);

            string iosDefault = @"{
  ""name"": ""iOS Default"",
  ""build"": {
    ""bundleIdentifier"": ""com.zzz.game"",
    ""bundleVersion"": ""1.0.0"",
    ""buildNumber"": 1,
    ""developmentTeam"": ""XYZ123456""
  },
  ""add_scripts"": [],
  ""remove_scripts"": [],
  ""add_scenes"": [],
  ""remove_scenes"": [],
  ""startView"": {
    ""orientation"": ""LandscapeLeft"",
    ""safeAreaMode"": ""ExtendIntoSafeArea"",
    ""homeIndicatorMode"": ""AutoHidden""
  }
}";
            File.WriteAllText(Path.Combine(buildSettingsDir, "platforms", "ios", "default_config.json"), iosDefault);

            string linuxDefault = @"{
  ""name"": ""Linux Default"",
  ""build"": {
    ""executableName"": ""GameZzz"",
    ""version"": ""1.0.0"",
    ""buildNumber"": 1
  },
  ""platform"": {
    ""child_views"": [],
    ""independent_views"": []
  },
  ""add_scripts"": [],
  ""remove_scripts"": [],
  ""add_scenes"": [],
  ""remove_scenes"": [],
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
            File.WriteAllText(Path.Combine(buildSettingsDir, "platforms", "linux", "default_config.json"), linuxDefault);

            string macosDefault = @"{
  ""name"": ""macOS Default"",
  ""build"": {
    ""executableName"": ""GameZzz"",
    ""bundleIdentifier"": ""com.zzz.game"",
    ""bundleVersion"": ""1.0.0"",
    ""buildNumber"": 1,
    ""icon"": ""Assets/Icons/macos_icon.icns""
  },
  ""platform"": {
    ""child_views"": [],
    ""independent_views"": []
  },
  ""add_scripts"": [],
  ""remove_scripts"": [],
  ""add_scenes"": [],
  ""remove_scenes"": [],
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
            File.WriteAllText(Path.Combine(buildSettingsDir, "platforms", "macos", "default_config.json"), macosDefault);

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
                            new BuildPresetTarget { Name = "game_win", Platform = "Windows", ConfigFile = "build_settings/platforms/windows/default_config.json" },
                            new BuildPresetTarget { Name = "game_android", Platform = "Android", ConfigFile = "build_settings/platforms/android/default_config.json" },
                            new BuildPresetTarget { Name = "game_ios", Platform = "iOS", ConfigFile = "build_settings/platforms/ios/default_config.json" },
                            new BuildPresetTarget { Name = "game_linux", Platform = "Linux", ConfigFile = "build_settings/platforms/linux/default_config.json" },
                            new BuildPresetTarget { Name = "game_macos", Platform = "MacOS", ConfigFile = "build_settings/platforms/macos/default_config.json" }
                        }
                    }
                }
            };

            File.WriteAllText(Path.Combine(buildSettingsDir, "presets.json"), JsonSerializer.Serialize(presets, jsonOptions));

            var manifest = new ProjectManifestModel
            {
                CompanyName = string.IsNullOrWhiteSpace(companyName) ? "Zzz" : companyName,
                AppName = string.IsNullOrWhiteSpace(appName) ? "ZzzGame" : appName,
                Description = "Новый проект игровых ресурсов",
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