using System;
using System.IO;
using System.Text.Json;
using NetworkLogListener.Models;

namespace NetworkLogListener;

public static class SettingsManager
{
    private static readonly string AppDataFolder = Path.Combine(
        Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), 
        "NetworkLogListener");
        
    private static readonly string SettingsFilePath = Path.Combine(AppDataFolder, "appsettings.json");

    public static AppSettings Load()
    {
        AppSettings settings = new AppSettings();
        try
        {
            if (File.Exists(SettingsFilePath))
            {
                var json = File.ReadAllText(SettingsFilePath);
                var loaded = JsonSerializer.Deserialize<AppSettings>(json);
                if (loaded != null)
                {
                    settings = loaded;
                }
            }
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Failed to load settings: {ex.Message}");
        }

        if (settings.SavedAddresses == null)
            settings.SavedAddresses = new System.Collections.Generic.List<ListenAddress>();
            
        bool hasDefault = false;
        foreach (var addr in settings.SavedAddresses)
        {
            if (addr.IpAddress == Constants.LocalhostIp && addr.Port == Constants.DefaultPort && addr.Name == "Localhost")
            {
                addr.IsBuiltIn = true;
                hasDefault = true;
                break;
            }
        }
        
        if (!hasDefault)
        {
            settings.SavedAddresses.Insert(0, new ListenAddress { Name = "Localhost", IpAddress = Constants.LocalhostIp, Port = Constants.DefaultPort, IsBuiltIn = true });
        }

        return settings;
    }

    public static void Save(AppSettings settings)
    {
        try
        {
            if (!Directory.Exists(AppDataFolder))
            {
                Directory.CreateDirectory(AppDataFolder);
            }

            var options = new JsonSerializerOptions { WriteIndented = true };
            var json = JsonSerializer.Serialize(settings, options);
            File.WriteAllText(SettingsFilePath, json);
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Failed to save settings: {ex.Message}");
        }
    }
}
