using System;
using System.Collections.Generic;
using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.FileTypes.ProjectSettings
{
	/// <summary>
	/// Парсер настроек проекта (управляет Configs/project_config.toml).
	/// </summary>
	public class ProjectSettingsParser : IFileParser, IEditorConfigParser
	{
		public string FileExtension => ".toml";

		public Type DataType => typeof(ProjectSettingsData);

		public bool CanParse(string relativePath)
		{
			return relativePath == ProjectConstants.SystemDirectories.ProjectSettings;
		}

		public bool Validate(IFileStorage storage, string filePath, out string errorMessage)
		{
			errorMessage = string.Empty;
			try
			{
				if (!storage.FileExists(filePath))
				{
					errorMessage = $"Файл настроек проекта отсутствует: {filePath}";
					return false;
				}

				var content = storage.ReadAllText(filePath);
				if (string.IsNullOrWhiteSpace(content))
				{
					errorMessage = "Файл настроек проекта пуст.";
					return false;
				}

				// Проверяем возможность десериализации
				var data = Deserialize(content);
				if (string.IsNullOrWhiteSpace(data.Version))
				{
					errorMessage = "Файл настроек проекта не содержит версию.";
					return false;
				}

				if (string.IsNullOrWhiteSpace(data.Name))
				{
					errorMessage = "Файл настроек проекта не содержит имя проекта.";
					return false;
				}
			}
			catch (Exception ex)
			{
				errorMessage = $"Ошибка валидации project_config.toml: {ex.Message}";
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
					errorMessage = "Файл настроек проекта отсутствует.";
					return false;
				}

				var content = storage.ReadAllText(filePath);
				var data = Deserialize(content);
				if (Version.TryParse(data.Version, out var parsedVersion) && parsedVersion != null)
				{
					version = parsedVersion;
					return true;
				}
				errorMessage = $"Не удалось прочесть версию из файла: '{data.Version}'";
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
			var data = new ProjectSettingsData
			{
				Version = ProjectConstants.ProjectVersionString,
				Name = "NewProject"
			};
			return Serialize(data);
		}

		public bool Migrate(IFileStorage storage, string filePath, Version targetVersion, out string errorMessage)
		{
			errorMessage = string.Empty;
			try
			{
				if (!TryGetVersion(storage, filePath, out Version currentVersion, out errorMessage))
				{
					return false;
				}

				if (currentVersion >= targetVersion)
				{
					return true; // Миграция не требуется
				}

				var content = storage.ReadAllText(filePath);
				var data = Deserialize(content);

				// Выполняем цепочку миграций по шагам
				// Шаг 1: 0.9.0 -> 1.0.0 (Пример)
				if (currentVersion < new Version(1, 0, 0))
				{
					// Логика конвертации структуры (если бы она изменилась).
					data.Version = "1.0.0";
				}

				// Записываем обновленные данные обратно
				storage.WriteAllText(filePath, Serialize(data));
				return true;
			}
			catch (Exception ex)
			{
				errorMessage = $"Ошибка при миграции файла: {ex.Message}";
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
				string localContent = storage.ReadAllText(localFilePath);
				string remoteContent = storage.ReadAllText(remoteFilePath);

				var localData = Deserialize(localContent);
				var remoteData = Deserialize(remoteContent);

				var mergedData = new ProjectSettingsData
				{
					Name = !string.IsNullOrWhiteSpace(localData.Name) ? localData.Name : remoteData.Name,
					Version = ProjectConstants.ProjectVersionString,
					ShowSystemMode = localData.ShowSystemMode,
					DisabledFilters = localData.DisabledFilters,
					DisabledSystemFilters = localData.DisabledSystemFilters
				};

				storage.WriteAllText(mergedOutputFilePath, Serialize(mergedData));
				return true;
			}
			catch (Exception ex)
			{
				errorMessage = $"Не удалось выполнить слияние настроек проекта: {ex.Message}";
				return false;
			}
		}

		#region TOML Serialization Helper

		public static string Serialize(ProjectSettingsData data)
		{
			var sb = new System.Text.StringBuilder();
			sb.AppendLine($"version = \"{TomlLineParser.Escape(data.Version)}\"");
			sb.AppendLine($"name = \"{TomlLineParser.Escape(data.Name)}\"");
			sb.AppendLine();
			sb.AppendLine("[editor]");
			sb.AppendLine($"show_system_mode = {data.ShowSystemMode.ToString().ToLower()}");
			sb.AppendLine($"disabled_filters = {TomlLineParser.SerializeStringArray(data.DisabledFilters)}");
			sb.AppendLine($"disabled_system_filters = {TomlLineParser.SerializeStringArray(data.DisabledSystemFilters)}");

			return sb.ToString();
		}

		public static ProjectSettingsData Deserialize(string toml)
		{
			var data = new ProjectSettingsData();
			foreach (var (key, valStr) in TomlLineParser.ParseKeyValueLines(toml))
			{
				if (key == "version")
				{
					data.Version = valStr.Trim('"');
				}
				else if (key == "name")
				{
					data.Name = valStr.Trim('"');
				}
				else if (key == "show_system_mode")
				{
					if (bool.TryParse(valStr, out bool showMode))
					{
						data.ShowSystemMode = showMode;
					}
				}
				else if (key == "disabled_filters")
				{
					data.DisabledFilters = TomlLineParser.ParseStringArray(valStr);
				}
				else if (key == "disabled_system_filters")
				{
					data.DisabledSystemFilters = TomlLineParser.ParseStringArray(valStr);
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
			return Serialize((ProjectSettingsData)data);
		}

		#endregion
	}
}
