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
		/// Имена системных директорий проекта (лежат прямо в корне проекта, вне Assets).
		/// </summary>
		public static class SystemDirectories
		{
			public const string ProjectSettings = "Configs/project.toml";
		}

		/// <summary>
		/// Имена директорий ассетов проекта.
		/// </summary>
		public static class AssetDirectories
		{
			public const string Scripts = "Scripts";
			public const string EcsComponents = "Ecs/Components";
			public const string EcsSystems = "Ecs/Systems";
			public const string EcsEntities = "Ecs/Entities";
			public const string Textures = "Textures";
			public const string Geometry = "Geometry";
			public const string Shaders = "Shaders";
			public const string Materials = "Materials";
		}
	}
}
