using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using editor.Services;

namespace editor.Services.Project.Infrastructure
{
    public class ScriptMetaData
    {
        public string Guid { get; set; } = string.Empty;

        public string ClassName { get; set; } = string.Empty;

        public string Namespace { get; set; } = string.Empty;

        public string QualifiedName => string.IsNullOrWhiteSpace(Namespace)
            ? ClassName
            : $"{Namespace}::{ClassName}";
    }

    public static class ScriptMetaFile
    {
        public static ScriptMetaData CreateNew(string className, string scriptNamespace = "")
        {
            return new ScriptMetaData
            {
                Guid = System.Guid.NewGuid().ToString(),
                ClassName = className,
                Namespace = scriptNamespace?.Trim() ?? string.Empty
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
                EditorLogger.LogError($"[Meta System] Failed to parse script meta file '{metaPath}': {ex.Message}");
                return null;
            }
        }

        public static void Save(IFileStorage storage, string metaPath, ScriptMetaData data)
        {
            storage.WriteAllText(metaPath, Serialize(data));
        }

        public static string InferNamespaceFromHeader(string hppContent, string className)
        {
            if (string.IsNullOrWhiteSpace(hppContent) || string.IsNullOrWhiteSpace(className))
            {
                return string.Empty;
            }

            var namespaceStack = new List<string>();
            string? pendingNamespace = null;
            var classPattern = new Regex(@"\b(class|struct)\s+" + Regex.Escape(className) + @"\b");

            foreach (string rawLine in hppContent.Replace("\r\n", "\n").Split('\n'))
            {
                string line = rawLine.Trim();
                if (classPattern.IsMatch(line))
                {
                    return string.Join("::", namespaceStack);
                }

                bool openedNamespaceOnLine = false;
                foreach (Match match in Regex.Matches(line, @"\bnamespace\s+([A-Za-z_][A-Za-z0-9_]*)\s*\{"))
                {
                    namespaceStack.Add(match.Groups[1].Value);
                    openedNamespaceOnLine = true;
                }

                if (!openedNamespaceOnLine)
                {
                    var pendingMatch = Regex.Match(line, @"^namespace\s+([A-Za-z_][A-Za-z0-9_]*)$");
                    if (pendingMatch.Success)
                    {
                        pendingNamespace = pendingMatch.Groups[1].Value;
                        continue;
                    }
                }

                if (pendingNamespace != null && line.StartsWith("{", StringComparison.Ordinal))
                {
                    namespaceStack.Add(pendingNamespace);
                    pendingNamespace = null;
                    continue;
                }

                if (pendingNamespace == null && line.StartsWith("}", StringComparison.Ordinal) && namespaceStack.Count > 0)
                {
                    namespaceStack.RemoveAt(namespaceStack.Count - 1);
                }
            }

            return string.Empty;
        }

        private static string Serialize(ScriptMetaData data)
        {
            var sb = new System.Text.StringBuilder();
            sb.AppendLine($"guid = \"{TomlLineParser.Escape(data.Guid)}\"");
            sb.AppendLine($"class_name = \"{TomlLineParser.Escape(data.ClassName)}\"");
            sb.AppendLine($"namespace = \"{TomlLineParser.Escape(data.Namespace)}\"");
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
                else if (key == "namespace")
                {
                    data.Namespace = value.Trim('"');
                }
            }
            return data;
        }
    }
}