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
	/// Переопределяет подпись, отображаемую для свойства в инспекторе. Без этого
	/// атрибута используется исходное имя свойства.
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
	/// Описывает, как редактору отрисовывать свойство-коллекцию. Реальным значением
	/// по-прежнему владеет модель данных; этот атрибут только выбирает элемент управления
	/// инспектора и подсказки для валидации.
	/// </summary>
	public enum EditorCollectionKind
	{
		/// <summary>
		/// Обычный редактируемый список строк.
		/// </summary>
		StringList,

		/// <summary>
		/// Список GUID-ссылок на ассеты из дерева ассетов проекта.
		/// </summary>
		AssetGuidList
	}

	/// <summary>
	/// Задаёт правило валидации, применяемое к отдельным элементам строковой коллекции.
	/// </summary>
	public enum EditorCollectionItemValidation
	{
		/// <summary>
		/// Валидация значений элементов не выполняется.
		/// </summary>
		None,

		/// <summary>
		/// Элемент должен выглядеть как C/C++ preprocessor define (NAME или NAME=value).
		/// </summary>
		CppDefine
	}

	/// <summary>
	/// Помечает свойство-список/массив как редактируемое в инспекторе и описывает поведение
	/// списка, которое рефлексия вывести не может, например сортировку и правила дублей.
	/// </summary>
	[AttributeUsage(AttributeTargets.Property | AttributeTargets.Field, AllowMultiple = false)]
	public class EditorCollectionAttribute : Attribute
	{
		public EditorCollectionKind Kind { get; }

		/// <summary>
		/// Позволяет инспектору показывать элементы управления перемещения вверх/вниз.
		/// Полезно для упорядоченных рантайм-списков, таких как глобальные скрипты.
		/// </summary>
		public bool IsSortable { get; set; }

		/// <summary>
		/// Разрешает повторяющиеся значения в списке. По умолчанию выключено, т.к.
		/// большинство ссылок в конфигах и дефайнов должны быть уникальными.
		/// </summary>
		public bool AllowDuplicates { get; set; }

		/// <summary>
		/// Позволяет редактору сохранять записи, которые больше не разрешаются в реальные
		/// ассеты. Обычно false, чтобы валидация могла убирать битые GUID-ссылки.
		/// </summary>
		public bool AllowMissingItems { get; set; }

		/// <summary>
		/// Необязательная подсказка типа ассета для списков GUID ассетов. Инспектор
		/// и валидаторы используют её, чтобы выбрать пикер и разрешить отображаемые имена.
		/// </summary>
		public AssetResourceType AssetType { get; set; }

		/// <summary>
		/// Необязательное правило валидации отдельного элемента, применяемое перед принятием правки.
		/// </summary>
		public EditorCollectionItemValidation ItemValidation { get; set; } = EditorCollectionItemValidation.None;

		public EditorCollectionAttribute(EditorCollectionKind kind)
		{
			Kind = kind;
		}
	}

	/// <summary>
	/// Задаёт источник, поставляющий доступные для выбора значения для скалярного свойства.
	/// </summary>
	public enum EditorOptionsSource
	{
		/// <summary>
		/// Доступные бэкенды log listener'ов, плюс опциональный "None".
		/// </summary>
		LogListeners
	}

	/// <summary>
	/// Помечает скалярное свойство как поле с вариантами выбора. Инспектор отрисовывает
	/// его как combo box и запрашивает допустимые значения у указанного источника.
	/// </summary>
	[AttributeUsage(AttributeTargets.Property | AttributeTargets.Field, AllowMultiple = false)]
	public class EditorOptionsAttribute : Attribute
	{
		public EditorOptionsSource Source { get; }

		/// <summary>
		/// Добавляет пустой вариант "None" для необязательных настроек.
		/// </summary>
		public bool AllowNone { get; set; }

		public EditorOptionsAttribute(EditorOptionsSource source)
		{
			Source = source;
		}
	}

	/// <summary>
	/// Помечает строковое свойство как одиночную ссылку на GUID ассета.
	/// Инспектор отрисовывает его с возможностью Drag-and-Drop ассета из дерева.
	/// </summary>
	[AttributeUsage(AttributeTargets.Property | AttributeTargets.Field, AllowMultiple = false)]
	public class EditorAssetGuidAttribute : Attribute
	{
		public AssetResourceType AssetType { get; }

		public EditorAssetGuidAttribute(AssetResourceType assetType)
		{
			AssetType = assetType;
		}
	}
}
