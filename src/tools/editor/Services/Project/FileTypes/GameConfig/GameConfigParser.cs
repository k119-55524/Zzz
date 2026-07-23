
using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.FileTypes.GameConfig
{
	/// <summary>
	/// Парсер Configs/game_config.toml.
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
					errorMessage = $"Файл конфигурации игры отсутствует: {filePath}";
					return false;
				}

				var data = Deserialize(storage.ReadAllText(filePath));
				if (string.IsNullOrWhiteSpace(data.Version))
				{
					errorMessage = "Файл конфигурации игры не содержит версию.";
					return false;
				}
			}
			catch (Exception ex)
			{
				errorMessage = $"Ошибка валидации game_config.toml: {ex.Message}";
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
					errorMessage = "Файл конфигурации игры отсутствует.";
					return false;
				}

				var data = Deserialize(storage.ReadAllText(filePath));
				if (Version.TryParse(data.Version, out var parsedVersion) && parsedVersion != null)
				{
					version = parsedVersion;
					return true;
				}

				errorMessage = $"Не удалось прочесть версию конфигурации игры: '{data.Version}'";
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
				errorMessage = $"Ошибка при миграции файла конфигурации игры: {ex.Message}";
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
				errorMessage = $"Не удалось выполнить слияние конфигурации игры: {ex.Message}";
				return false;
			}
		}

		public static string Serialize(GameConfigData data)
		{
			var sb = new System.Text.StringBuilder();
			sb.AppendLine($"version = \"{TomlLineParser.Escape(data.Version)}\"");
			sb.AppendLine();
			sb.AppendLine($"defines = {TomlLineParser.SerializeStringArray(data.Defines)}");
			sb.AppendLine($"log_listener = \"{TomlLineParser.Escape(data.LogListener)}\"");
			sb.AppendLine($"view_guids = {TomlLineParser.SerializeStringArray(data.ViewGuids)}");
			sb.AppendLine($"global_script_guids = {TomlLineParser.SerializeStringArray(data.GlobalScriptGuids)}");
			return sb.ToString();
		}

		public static GameConfigData Deserialize(string toml)
		{
			var data = new GameConfigData();
			foreach (var (key, value) in TomlLineParser.ParseKeyValueLines(toml))
			{
				if (key == "version")
				{
					data.Version = value.Trim('"');
				}
				else if (key == "defines")
				{
					data.Defines = TomlLineParser.ParseStringArray(value);
				}
				else if (key == "log_listener")
				{
					data.LogListener = value.Trim('"');
				}
				else if (key == "view_guids")
				{
					data.ViewGuids = TomlLineParser.ParseStringArray(value);
				}
				else if (key == "global_script_guids")
				{
					data.GlobalScriptGuids = TomlLineParser.ParseStringArray(value);
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
	}
}
