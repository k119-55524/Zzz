using System;
using System.Text.Json;
using System.Text.Json.Serialization;
using editor.Services;

namespace editor.Services.Project.Infrastructure
{
    /// <summary>
    /// Содержимое файла метаданных скрипта (Player.meta), привязанного к паре Player.hpp/Player.cpp.
    /// GUID используется только редактором/сборщиком (для устойчивости ссылок при переименовании);
    /// в скомпилированный движок не попадает.
    /// </summary>
    public class ScriptMetaData
    {
        [JsonPropertyName("guid")]
        public string Guid { get; set; } = string.Empty;

        [JsonPropertyName("class_name")]
        public string ClassName { get; set; } = string.Empty;
    }

    /// <summary>
    /// Чтение/запись .meta файлов скриптов. Единая точка работы с форматом,
    /// чтобы создание, переименование и синхронизация при скане не расходились в деталях.
    /// </summary>
    public static class ScriptMetaFile
    {
        private static readonly JsonSerializerOptions s_Options = new() { WriteIndented = true };

        public static ScriptMetaData CreateNew(string className)
        {
            return new ScriptMetaData
            {
                Guid = System.Guid.NewGuid().ToString(),
                ClassName = className
            };
        }

        public static ScriptMetaData? Load(IFileStorage storage, string metaPath)
        {
            if (!storage.FileExists(metaPath))
                return null;

            try
            {
                string json = storage.ReadAllText(metaPath);
                return JsonSerializer.Deserialize<ScriptMetaData>(json);
            }
            catch (Exception ex)
            {
                EditorLogger.LogError($"[Meta System] Failed to parse meta file '{metaPath}': {ex.Message}");
                return null;
            }
        }

        public static void Save(IFileStorage storage, string metaPath, ScriptMetaData data)
        {
            string json = JsonSerializer.Serialize(data, s_Options);
            storage.WriteAllText(metaPath, json);
        }
    }
}
