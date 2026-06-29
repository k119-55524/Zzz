using System.Collections.Generic;
using System.Linq;
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
	/// Описывает обязательную папку проекта. Папка может дополнительно нести
	/// в себе схему файла, который должен быть создан внутри неё.
	/// </summary>
	public class ProjectFolderSchema
	{
		/// <summary>
		/// Путь к папке относительно корня проекта.
		/// </summary>
		public string RelativePath { get; set; } = string.Empty;

		/// <summary>
		/// Схема файла, создаваемого внутри этой папки. Null, если папка не несёт файла.
		/// </summary>
		public ProjectFileSchema? File { get; set; }
	}

	/// <summary>
	/// Статический класс, определяющий структуру директорий и файлов нового проекта.
	/// Делит обязательные папки на системные и папки ассетов.
	/// </summary>
	public static class ProjectStructure
	{
		// Системные папки проекта (вне Assets)
		public static readonly List<ProjectFolderSchema> SystemDirectories = new()
		{
			CreateFolderWithFile(ProjectConstants.SystemDirectories.ProjectSettings, new ProjectSettingsParser())
		};

		// Корневая папка ассетов
		public const string AssetsRoot = "Assets";

		// .gitignore в корне проекта: без Parser (нет версии/формата для валидации/миграции) -
		// см. логику восстановления в ProjectService.OpenProject, она перезаписывает файл из
		// DefaultContent только если он отсутствует физически, не трогая существующий (даже
		// отредактированный руками) .gitignore.
		private static readonly ProjectFolderSchema RootGitignore = new()
		{
			RelativePath = string.Empty,
			File = new ProjectFileSchema
			{
				RelativePath = ".gitignore",
				IsRequired = true,
				Parser = null,
				DefaultContent =
					"# Генерируется и собирается редактором по требованию: CMakeLists.txt содержит\n" +
					"# абсолютные пути текущей машины, .editor/bin/ - скомпилированные scripts.dll/.lib\n" +
					"# для Hot-Reload. Не предназначена для коммита.\n" +
					".editor/\n"
			}
		};

		// Все обязательные папки проекта (системные + ассеты)
		public static IEnumerable<ProjectFolderSchema> AllDirectories =>
			SystemDirectories.Concat(new[] { RootGitignore, new ProjectFolderSchema { RelativePath = AssetsRoot } });

		// Схемы всех файлов, привязанных к обязательным папкам
		public static IEnumerable<ProjectFileSchema> AllFiles =>
			AllDirectories.Where(d => d.File != null).Select(d => d.File!);

		// Строит схему папки по пути её обязательного файла: сама папка — это директория файла.
		private static ProjectFolderSchema CreateFolderWithFile(string filePath, IFileParser parser)
		{
			int slashIndex = filePath.LastIndexOf('/');
			string folderPath = slashIndex >= 0 ? filePath.Substring(0, slashIndex) : string.Empty;

			return new ProjectFolderSchema
			{
				RelativePath = folderPath,
				File = new ProjectFileSchema
				{
					RelativePath = filePath,
					IsRequired = true,
					Parser = parser,
					DefaultContent = parser.GetDefaultContent()
				}
			};
		}
	}
}
