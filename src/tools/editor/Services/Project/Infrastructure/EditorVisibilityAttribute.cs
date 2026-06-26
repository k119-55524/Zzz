using System;

namespace editor.Services.Project.Infrastructure
{
	/// <summary>
	/// Определяет уровень видимости и редактируемости поля в интерфейсе редактора.
	/// </summary>
	public enum EditorVisibility
	{
		/// <summary>
		/// Поле скрыто в интерфейсе редактора (доступно только в коде/файле).
		/// </summary>
		Hidden,

		/// <summary>
		/// Поле отображается в редакторе, но заблокировано для изменения (только для чтения).
		/// </summary>
		ReadOnly,

		/// <summary>
		/// Поле отображается в редакторе и доступно для редактирования.
		/// </summary>
		Editable
	}

	/// <summary>
	/// Атрибут для разметки полей настроек с целью управления их видимостью в редакторе.
	/// </summary>
	[AttributeUsage(AttributeTargets.Property | AttributeTargets.Field, AllowMultiple = false)]
	public class EditorVisibilityAttribute : Attribute
	{
		public EditorVisibility Visibility { get; }

		public EditorVisibilityAttribute(EditorVisibility visibility)
		{
			Visibility = visibility;
		}
	}
}
