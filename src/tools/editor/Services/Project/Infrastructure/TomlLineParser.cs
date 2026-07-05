using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace editor.Services.Project.Infrastructure
{
    /// <summary>
    /// Общий ручной построчный парсер простых TOML-файлов проекта (без секций и вложенных таблиц),
    /// используемый GameConfigParser, ProjectSettingsParser и ScriptMetaFile, чтобы разбор
    /// "key = value" и строковых массивов не расходился в деталях между ними.
    /// </summary>
    public static class TomlLineParser
    {
        /// <summary>
        /// Построчно разбирает "key = value", пропуская комментарии (#), заголовки секций ([...])
        /// и пустые строки. Ключ приводится к нижнему регистру и обрезается, значение только
        /// обрезается - кавычки/тип значения (строка, bool, массив) остаются на усмотрение вызывающего.
        /// </summary>
        public static IEnumerable<(string Key, string Value)> ParseKeyValueLines(string toml)
        {
            var lines = toml.Split(new[] { "\r\n", "\r", "\n" }, StringSplitOptions.RemoveEmptyEntries);
            foreach (var line in lines)
            {
                var trimmed = line.Trim();
                if (trimmed.StartsWith("#") || trimmed.StartsWith("[") || string.IsNullOrWhiteSpace(trimmed))
                {
                    continue;
                }

                var parts = trimmed.Split(new[] { '=' }, 2);
                if (parts.Length != 2)
                {
                    continue;
                }

                yield return (parts[0].Trim().ToLowerInvariant(), parts[1].Trim());
            }
        }

        /// <summary>
        /// Разбирает TOML-массив строк вида ["a", "b"] в список значений без кавычек.
        /// </summary>
        public static List<string> ParseStringArray(string value)
        {
            var list = new List<string>();
            var matches = Regex.Matches(value, "\"([^\"]*)\"");
            foreach (Match match in matches)
            {
                list.Add(match.Groups[1].Value);
            }
            return list;
        }

        /// <summary>
        /// Сериализует список строк в TOML-массив ["a", "b"], экранируя спецсимволы в элементах.
        /// </summary>
        public static string SerializeStringArray(IEnumerable<string>? items)
        {
            var list = new List<string>(items ?? Array.Empty<string>());
            if (list.Count == 0)
            {
                return "[]";
            }

            return "[" + string.Join(", ", list.ConvertAll(item => $"\"{Escape(item)}\"")) + "]";
        }

        /// <summary>
        /// Экранирует обратный слэш и двойную кавычку для безопасной записи строкового значения в TOML.
        /// </summary>
        public static string Escape(string? value)
        {
            return (value ?? string.Empty).Replace("\\", "\\\\").Replace("\"", "\\\"");
        }
    }
}
