using editor.Services.Project.Infrastructure;

namespace editor.Services.Project.FileTypes.ProjectSettings
{
	/// <summary>
	/// Данные настроек проекта (представление Configs/project.toml в памяти).
	/// </summary>
	public class ProjectSettingsData
	{
		/// <summary>
		/// Версия формата проекта.
		/// </summary>
		[EditorVisibility(EditorVisibility.ReadOnly)]
		public string Version { get; set; } = string.Empty;

		/// <summary>
		/// Имя проекта.
		/// </summary>
		[EditorVisibility(EditorVisibility.Editable)]
		public string Name { get; set; } = string.Empty;
	}
}
