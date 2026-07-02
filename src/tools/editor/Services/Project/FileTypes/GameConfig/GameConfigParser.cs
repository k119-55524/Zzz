using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.FileTypes.GameConfig
{
	/// <summary>
	/// Parser for Configs/game_config.toml.
	/// </summary>
	public class GameConfigParser : IFileParser, IEditorConfigParser
	{
		public string FileExtension => ".toml";

		public Type DataType => typeof(GameConfigData);

		public bool CanParse(string relativePath)
		{
			return relativePath == ProjectConstants.SystemDirectories.GameConfigs;
		}

		public bool Validate(IFileStorage storage, string filePath, out string errorMessage)
		{
			errorMessage = string.Empty;
			try
			{
				if (!storage.FileExists(filePath))
				{
					errorMessage = $"Game config file is missing: {filePath}";
					return false;
				}

				var data = Deserialize(storage.ReadAllText(filePath));
				if (string.IsNullOrWhiteSpace(data.Version))
				{
					errorMessage = "Game config file does not contain a version.";
					return false;
				}
			}
			catch (Exception ex)
			{
				errorMessage = $"Game config validation failed: {ex.Message}";
				return false;
			}

			return true;
		}

		public bool TryGetVersion(IFileStorage storage, string filePath, out Version version, out string errorMessage)
		{
			version = new Version(0, 0, 0);
			errorMessage = string.Empty;
			try
			{
				if (!storage.FileExists(filePath))
				{
					errorMessage = "Game config file is missing.";
					return false;
				}

				var data = Deserialize(storage.ReadAllText(filePath));
				if (Version.TryParse(data.Version, out var parsedVersion) && parsedVersion != null)
				{
					version = parsedVersion;
					return true;
				}

				errorMessage = $"Could not read game config version: '{data.Version}'";
				return false;
			}
			catch (Exception ex)
			{
				errorMessage = ex.Message;
				return false;
			}
		}

		public string GetDefaultContent()
		{
			return Serialize(new GameConfigData
			{
				Version = ProjectConstants.ProjectVersionString
			});
		}

		public bool Migrate(IFileStorage storage, string filePath, Version targetVersion, out string errorMessage)
		{
			errorMessage = string.Empty;
			try
			{
				var data = storage.FileExists(filePath)
					? Deserialize(storage.ReadAllText(filePath))
					: new GameConfigData();

				data.Version = targetVersion.ToString(3);
				storage.WriteAllText(filePath, Serialize(data));
				return true;
			}
			catch (Exception ex)
			{
				errorMessage = $"Game config migration failed: {ex.Message}";
				return false;
			}
		}

		public bool Merge(
			IFileStorage storage,
			string baseFilePath,
			string localFilePath,
			string remoteFilePath,
			string mergedOutputFilePath,
			out string errorMessage)
		{
			errorMessage = string.Empty;
			try
			{
				var local = storage.FileExists(localFilePath)
					? Deserialize(storage.ReadAllText(localFilePath))
					: new GameConfigData();

				if (string.IsNullOrWhiteSpace(local.Version))
				{
					local.Version = ProjectConstants.ProjectVersionString;
				}

				storage.WriteAllText(mergedOutputFilePath, Serialize(local));
				return true;
			}
			catch (Exception ex)
			{
				errorMessage = $"Game config merge failed: {ex.Message}";
				return false;
			}
		}

		public static string Serialize(GameConfigData data)
		{
			var sb = new System.Text.StringBuilder();
			sb.AppendLine($"version = \"{Escape(data.Version)}\"");
			sb.AppendLine();
			sb.AppendLine($"defines = {SerializeStringArray(data.Defines)}");
			sb.AppendLine($"log_listener = \"{Escape(data.LogListener)}\"");
			sb.AppendLine($"global_script_guids = {SerializeStringArray(data.GlobalScriptGuids)}");
			return sb.ToString();
		}

		public static GameConfigData Deserialize(string toml)
		{
			var data = new GameConfigData();
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

				var key = parts[0].Trim().ToLowerInvariant();
				var value = parts[1].Trim();
				if (key == "version")
				{
					data.Version = value.Trim('"');
				}
				else if (key == "defines")
				{
					data.Defines = ParseStringArray(value);
				}
				else if (key == "log_listener")
				{
					data.LogListener = value.Trim('"');
				}
				else if (key == "global_script_guids")
				{
					data.GlobalScriptGuids = ParseStringArray(value);
				}
			}

			return data;
		}

		public object DeserializeForEditor(string content)
		{
			return Deserialize(content);
		}

		public string SerializeFromEditor(object data)
		{
			return Serialize((GameConfigData)data);
		}

		private static string SerializeStringArray(List<string>? items)
		{
			if (items == null || items.Count == 0)
			{
				return "[]";
			}

			return "[" + string.Join(", ", items.ConvertAll(item => $"\"{Escape(item)}\"")) + "]";
		}

		private static List<string> ParseStringArray(string value)
		{
			var list = new List<string>();
			var matches = Regex.Matches(value, "\"([^\"]*)\"");
			foreach (Match match in matches)
			{
				list.Add(match.Groups[1].Value);
			}
			return list;
		}

		private static string Escape(string? value)
		{
			return (value ?? string.Empty).Replace("\\", "\\\\").Replace("\"", "\\\"");
		}
	}
}
