using System.Collections.Generic;
using editor.Services.Project.Infrastructure;
using editor.Services.Project.Infrastructure.UndoRedo;

namespace editor.Services.Project.FileTypes.ProjectSettings
{
	/// <summary>
	/// Данные настроек проекта (представление Configs/project_config.toml в памяти).
	/// </summary>
	public class ProjectSettingsData : UndoableObject
	{
		private string _name = string.Empty;
		private bool _showSystemMode;
		private List<string> _disabledFilters = new();
		private List<string> _disabledSystemFilters = new();

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
		/// Состояние отображения системного режима во вкладке Проект (по умолчанию false).
		/// </summary>
		[EditorVisibility(EditorVisibility.Hidden)]
		public bool ShowSystemMode
		{
			get => _showSystemMode;
			set
			{
				if (_showSystemMode != value)
				{
					_showSystemMode = value;
					RaiseOnChanged();
				}
			}
		}

		/// <summary>
		/// Список отключенных фильтров папок (типов ресурсов) для дерева ассетов (Assets/).
		/// </summary>
		[EditorVisibility(EditorVisibility.Hidden)]
		public List<string> DisabledFilters
		{
			get => _disabledFilters;
			set => SetProperty(ref _disabledFilters, value, val => _disabledFilters = val);
		}

		/// <summary>
		/// Список отключенных фильтров папок для дерева системных файлов (SystemTree).
		/// Хранится отдельно от <see cref="DisabledFilters"/>, чтобы фильтрация ассетов
		/// никогда не затрагивала системное дерево, и наоборот.
		/// </summary>
		[EditorVisibility(EditorVisibility.Hidden)]
		public List<string> DisabledSystemFilters
		{
			get => _disabledSystemFilters;
			set => SetProperty(ref _disabledSystemFilters, value, val => _disabledSystemFilters = val);
		}
	}
}
