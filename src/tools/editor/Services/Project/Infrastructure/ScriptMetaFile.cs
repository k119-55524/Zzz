using System;
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
        public string Guid { get; set; } = string.Empty;

        public string ClassName { get; set; } = string.Empty;
    }

    /// <summary>
    /// Чтение/запись .meta файлов скриптов. Единая точка работы с форматом,
    /// чтобы создание, переименование и синхронизация при скане не расходились в деталях.
    /// Формат - TOML, как и все остальные текстовые файлы проекта (см. README в Services/Project),
    /// сериализация - тот же ручной key=value подход, что и у ProjectSettingsParser.
    /// </summary>
    public static class ScriptMetaFile
    {
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
                string toml = storage.ReadAllText(metaPath);
                return Deserialize(toml);
            }
            catch (Exception ex)
            {
                EditorLogger.LogError($"[Meta System] Не удалось разобрать meta-файл '{metaPath}': {ex.Message}");
                return null;
            }
        }

        public static void Save(IFileStorage storage, string metaPath, ScriptMetaData data)
        {
            storage.WriteAllText(metaPath, Serialize(data));
        }

        private static string Serialize(ScriptMetaData data)
        {
            var sb = new System.Text.StringBuilder();
            sb.AppendLine($"guid = \"{data.Guid}\"");
            sb.AppendLine($"class_name = \"{data.ClassName}\"");
            return sb.ToString();
        }

        private static ScriptMetaData Deserialize(string toml)
        {
            var data = new ScriptMetaData();
            foreach (var (key, value) in TomlLineParser.ParseKeyValueLines(toml))
            {
                if (key == "guid")
                {
                    data.Guid = value.Trim('"');
                }
                else if (key == "class_name")
                {
                    data.ClassName = value.Trim('"');
                }
            }
            return data;
        }
    }
}
