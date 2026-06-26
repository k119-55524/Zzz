
using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.FileTypes.ProjectSettings
{
	/// <summary>
	/// Парсер настроек проекта (управляет Configs/project.toml).
	/// </summary>
	public class ProjectSettingsParser : IFileParser
	{
		public string FileExtension => ".toml";

		public bool CanParse(string relativePath)
		{
			return relativePath == ProjectConstants.Files.ProjectSettings;
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
				errorMessage = $"Ошибка валидации project.toml: {ex.Message}";
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
					// На данный момент просто обновляем версию в объекте.
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
				// Заглушка для VCS Merge:
				// Для настроек проекта при конфликте мы можем попробовать объединить имя проекта и версию.
				// В простейшем случае: берем локальные изменения, если они валидны, иначе remote.
				// В будущем здесь будет полноценное структурное трехстороннее слияние TOML-документов.

				string localContent = storage.ReadAllText(localFilePath);
				string remoteContent = storage.ReadAllText(remoteFilePath);

				var localData = Deserialize(localContent);
				var remoteData = Deserialize(remoteContent);

				// Логика слияния:
				// Используем локальное имя, если оно задано, и берем максимальную версию.
				var mergedData = new ProjectSettingsData
				{
					Name = !string.IsNullOrWhiteSpace(localData.Name) ? localData.Name : remoteData.Name,
					Version = ProjectConstants.ProjectVersionString
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
			return $"version = \"{data.Version}\"\r\nname = \"{data.Name}\"\r\n";
		}

		public static ProjectSettingsData Deserialize(string toml)
		{
			var data = new ProjectSettingsData();
			var lines = toml.Split(new[] { "\r\n", "\r", "\n" }, StringSplitOptions.RemoveEmptyEntries);
			foreach (var line in lines)
			{
				var trimmed = line.Trim();
				if (trimmed.StartsWith("#") || string.IsNullOrWhiteSpace(trimmed))
					continue;

				var parts = trimmed.Split(new[] { '=' }, 2);
				if (parts.Length == 2)
				{
					var key = parts[0].Trim();
					var value = parts[1].Trim().Trim('"');

					if (key.Equals("version", StringComparison.OrdinalIgnoreCase))
					{
						data.Version = value;
					}
					else if (key.Equals("name", StringComparison.OrdinalIgnoreCase))
					{
						data.Name = value;
					}
				}
			}
			return data;
		}

		#endregion
	}
}
