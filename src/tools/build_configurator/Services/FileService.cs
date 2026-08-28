using System.IO;
using System.Linq;
using System.Text.Encodings.Web;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Text.Unicode;
using BuildConfigurator.Models;

namespace BuildConfigurator.Services;

public class FileService : IFileService
{
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        WriteIndented = true,
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
        DefaultIgnoreCondition = JsonIgnoreCondition.Never,
        Encoder = JavaScriptEncoder.Create(UnicodeRanges.All)
    };

    private static string BaseDir =>
        Path.GetDirectoryName(Environment.ProcessPath) ?? AppDomain.CurrentDomain.BaseDirectory;
    private string DataFilePath => Path.Combine(BaseDir, "profiles.json");

    public string GetDataFilePath() => DataFilePath;

    public AppData LoadData()
    {
        if (!File.Exists(DataFilePath))
            return new AppData();

        var json = File.ReadAllText(DataFilePath);
        return JsonSerializer.Deserialize<AppData>(json, JsonOptions) ?? new AppData();
    }

    public void SaveData(AppData data)
    {
        // Сериализуем отсортированную копию, не трогая порядок элементов в исходных коллекциях
        // (они привязаны к UI через ObservableCollection/InsertSorted и должны остаться как есть).
        var sorted = new AppData
        {
            Defines = data.Defines
                .OrderBy(d => d.Name, StringComparer.OrdinalIgnoreCase)
                .ToList(),
            Configurations = data.Configurations
                .OrderBy(c => c.Name, StringComparer.OrdinalIgnoreCase)
                .Select(c => new BuildConfiguration
                {
                    Name = c.Name,
                    Description = c.Description,
                    ActiveDefines = c.ActiveDefines
                        .OrderBy(n => n, StringComparer.OrdinalIgnoreCase)
                        .ToList()
                })
                .ToList()
        };

        var json = JsonSerializer.Serialize(sorted, JsonOptions);
        File.WriteAllText(DataFilePath, json);
    }
}
