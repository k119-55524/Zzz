using System.Collections.Generic;
using editor.Services.Project.Infrastructure;
using editor.Services.Project.Infrastructure.UndoRedo;

namespace editor.Services.Project.FileTypes.ProjectSettings
{
	/// <summary>
	/// Данные настроек проекта (представление Configs/project.toml в памяти).
	/// </summary>
	public class ProjectSettingsData : UndoableObject
	{
		private string _name = string.Empty;
		private List<string> _emptyFolders = new();
		private bool _showSystemMode;
		private List<string> _disabledFilters = new();

		/// <summary>
		/// Версия формата проекта.
		/// </summary>
		[EditorVisibility(EditorVisibility.ReadOnly)]
		public string Version { get; set; } = string.Empty;

		/// <summary>
		/// Имя проекта.
		/// </summary>
		[EditorVisibility(EditorVisibility.Editable)]
		public string Name
		{
			get => _name;
			set => SetProperty(ref _name, value, val => _name = val);
		}

		/// <summary>
		/// Список относительных путей пустых виртуальных папок, сохраненных между сессиями.
		/// </summary>
		[EditorVisibility(EditorVisibility.Hidden)]
		public List<string> EmptyFolders
		{
			get => _emptyFolders;
			set => SetProperty(ref _emptyFolders, value, val => _emptyFolders = val);
		}

		/// <summary>
		/// Состояние отображения системного режима во вкладке Проект (по умолчанию false).
		/// </summary>
		[EditorVisibility(EditorVisibility.Hidden)]
		public bool ShowSystemMode
		{
			get => _showSystemMode;
			set => SetProperty(ref _showSystemMode, value, val => _showSystemMode = val);
		}

		/// <summary>
		/// Список отключенных фильтров папок (типов ресурсов).
		/// </summary>
		[EditorVisibility(EditorVisibility.Hidden)]
		public List<string> DisabledFilters
		{
			get => _disabledFilters;
			set => SetProperty(ref _disabledFilters, value, val => _disabledFilters = val);
		}
	}
}
