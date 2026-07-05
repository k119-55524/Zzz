using System;
using editor.Models;

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

	/// <summary>
	/// Overrides the label shown for a property in the inspector. Without this
	/// attribute the raw property name is used.
	/// </summary>
	[AttributeUsage(AttributeTargets.Property | AttributeTargets.Field, AllowMultiple = false)]
	public class EditorDisplayNameAttribute : Attribute
	{
		public string DisplayName { get; }

		public EditorDisplayNameAttribute(string displayName)
		{
			DisplayName = displayName;
		}
	}

	/// <summary>
	/// Describes how the editor should render a collection property. The data model
	/// still owns the real value; this attribute only selects the inspector control
	/// and validation hints.
	/// </summary>
	public enum EditorCollectionKind
	{
		/// <summary>
		/// A plain editable list of strings.
		/// </summary>
		StringList,

		/// <summary>
		/// A list of GUID references to assets from the project asset tree.
		/// </summary>
		AssetGuidList
	}

	/// <summary>
	/// Names a validation rule applied to individual items of a string collection.
	/// </summary>
	public enum EditorCollectionItemValidation
	{
		/// <summary>
		/// No validation is performed on item values.
		/// </summary>
		None,

		/// <summary>
		/// The item must look like a C/C++ preprocessor define (NAME or NAME=value).
		/// </summary>
		CppDefine
	}

	/// <summary>
	/// Marks a list/array property as editable in the inspector and describes list
	/// behaviour that reflection cannot infer, such as sorting and duplicate rules.
	/// </summary>
	[AttributeUsage(AttributeTargets.Property | AttributeTargets.Field, AllowMultiple = false)]
	public class EditorCollectionAttribute : Attribute
	{
		public EditorCollectionKind Kind { get; }

		/// <summary>
		/// Allows the inspector to expose move up/down controls. Useful for ordered
		/// runtime lists such as global scripts.
		/// </summary>
		public bool IsSortable { get; set; }

		/// <summary>
		/// Allows repeated values in the list. Disabled by default because most config
		/// references and defines should be unique.
		/// </summary>
		public bool AllowDuplicates { get; set; }

		/// <summary>
		/// Lets the editor keep entries that no longer resolve to live assets. This is
		/// normally false so validation can remove broken GUID references.
		/// </summary>
		public bool AllowMissingItems { get; set; }

		/// <summary>
		/// Optional asset type hint for asset GUID lists. The inspector and validators
		/// use it to choose a picker and to resolve display names.
		/// </summary>
		public AssetResourceType AssetType { get; set; }

		/// <summary>
		/// Optional per-item validation rule applied before an edit is accepted.
		/// </summary>
		public EditorCollectionItemValidation ItemValidation { get; set; } = EditorCollectionItemValidation.None;

		public EditorCollectionAttribute(EditorCollectionKind kind)
		{
			Kind = kind;
		}
	}

	/// <summary>
	/// Names a provider that supplies selectable values for a scalar property.
	/// </summary>
	public enum EditorOptionsSource
	{
		/// <summary>
		/// Available log listener backends, plus optional "None".
		/// </summary>
		LogListeners
	}

	/// <summary>
	/// Marks a scalar property as an option field. The inspector renders it as a
	/// combo box and asks the named source for valid values.
	/// </summary>
	[AttributeUsage(AttributeTargets.Property | AttributeTargets.Field, AllowMultiple = false)]
	public class EditorOptionsAttribute : Attribute
	{
		public EditorOptionsSource Source { get; }

		/// <summary>
		/// Adds an empty "None" option for optional settings.
		/// </summary>
		public bool AllowNone { get; set; }

		public EditorOptionsAttribute(EditorOptionsSource source)
		{
			Source = source;
		}
	}
}
