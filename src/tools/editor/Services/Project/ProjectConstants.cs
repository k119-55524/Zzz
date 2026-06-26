using System;

namespace editor.Services.Project
{
	/// <summary>
	/// Содержит все константы структуры проекта, включая версии, пути к файлам и папки.
	/// </summary>
	public static class ProjectConstants
	{
		/// <summary>
		/// Единая версия формата проекта.
		/// </summary>
		public static readonly Version ProjectVersion = new(1, 0, 0);

		/// <summary>
		/// Строковое представление версии проекта (формат Major.Minor.Build).
		/// </summary>
		public static string ProjectVersionString => ProjectVersion.ToString(3);

		/// <summary>
		/// Относительные пути к обязательным файлам проекта.
		/// </summary>
		public static class Files
		{
			public const string ProjectSettings = "Configs/project.toml";
		}

		/// <summary>
		/// Имена обязательных директорий проекта.
		/// </summary>
		public static class Directories
		{
			public const string Assets = "Assets/Scripts";
			public const string Configs = "Configs";
		}
	}
}
