using System.IO;
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
        var json = JsonSerializer.Serialize(data, JsonOptions);
        File.WriteAllText(DataFilePath, json);
    }
}
