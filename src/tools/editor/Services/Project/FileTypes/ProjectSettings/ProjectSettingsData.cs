using System.Collections.Generic;
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

		/// <summary>
		/// Список относительных путей пустых виртуальных папок, сохраненных между сессиями.
		/// </summary>
		[EditorVisibility(EditorVisibility.Hidden)]
		public List<string> EmptyFolders { get; set; } = new();

		/// <summary>
		/// Состояние отображения системного режима во вкладке Проект (по умолчанию false).
		/// </summary>
		[EditorVisibility(EditorVisibility.Hidden)]
		public bool ShowSystemMode { get; set; }

		/// <summary>
		/// Список отключенных фильтров папок (типов ресурсов).
		/// </summary>
		[EditorVisibility(EditorVisibility.Hidden)]
		public List<string> DisabledFilters { get; set; } = new();
	}
}
