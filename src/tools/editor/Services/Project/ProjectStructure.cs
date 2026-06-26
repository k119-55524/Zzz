using System.Collections.Generic;
using editor.Services.Project.Infrastructure;
using editor.Services.Project.FileTypes.ProjectSettings;

namespace editor.Services.Project
{
	/// <summary>
	/// Представляет элемент ошибки валидации файла проекта.
	/// </summary>
	public class ValidationErrorItem
	{
		/// <summary>
		/// Относительный путь к файлу с ошибкой.
		/// </summary>
		public string FilePath { get; set; } = string.Empty;

		/// <summary>
		/// Сообщение об ошибке.
		/// </summary>
		public string ErrorMessage { get; set; } = string.Empty;
	}

	/// <summary>
	/// Описывает схему файла проекта, его валидатор и дефолтное содержимое.
	/// </summary>
	public class ProjectFileSchema
	{
		/// <summary>
		/// Путь к файлу относительно корня проекта.
		/// </summary>
		public string RelativePath { get; set; } = string.Empty;

		/// <summary>
		/// Флаг, указывающий, является ли файл обязательным для проекта.
		/// </summary>
		public bool IsRequired { get; set; }

		/// <summary>
		/// Парсер/валидатор для проверки корректности формата файла.
		/// </summary>
		public IFileParser? Parser { get; set; }

		/// <summary>
		/// Содержимое файла по умолчанию, используемое при создании проекта или восстановлении файла.
		/// </summary>
		public string DefaultContent { get; set; } = string.Empty;
	}

	/// <summary>
	/// Статический класс, определяющий структуру директорий и файлов нового проекта.
	/// </summary>
	public static class ProjectStructure
	{
		// Обязательные папки проекта
		public static readonly List<string> RequiredDirectories = new()
		{
			ProjectConstants.Directories.Assets,
			ProjectConstants.Directories.Configs
		};

		// Схема файлов проекта
		public static readonly List<ProjectFileSchema> RequiredFiles = new()
		{
			new ProjectFileSchema
			{
				RelativePath = ProjectConstants.Files.ProjectSettings,
				IsRequired = true,
				Parser = new ProjectSettingsParser(),
				DefaultContent = new ProjectSettingsParser().GetDefaultContent()
			}
		};
	}
}
