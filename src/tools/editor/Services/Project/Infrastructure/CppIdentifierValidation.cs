using System;
using System.Linq;
using System.Text.RegularExpressions;

namespace editor.Services.Project.Infrastructure
{
	/// <summary>
	/// Общая проверка корректности C++-идентификаторов и namespace-путей вида "A::B::C",
	/// используемая диалогом создания скрипта (NewScriptDialog) и inline-переименованием
	/// в дереве ассетов (AssetsWidget), чтобы правила не расходились между двумя местами ввода.
	/// </summary>
	public static class CppIdentifierValidation
	{
		private static readonly Regex IdentifierPattern = new(@"^[A-Za-z_][A-Za-z0-9_]*$", RegexOptions.Compiled);

		/// <summary>
		/// Проверяет, что значение является корректным идентификатором C++ (имя класса или
		/// один сегмент namespace).
		/// </summary>
		public static bool IsValidIdentifier(string value)
		{
			return IdentifierPattern.IsMatch(value);
		}

		/// <summary>
		/// Проверяет namespace в формате "A::B::C" - каждый сегмент должен быть корректным
		/// идентификатором C++; пустая строка namespace допустима (скрипт без namespace).
		/// </summary>
		public static bool IsValidNamespace(string value)
		{
			return value
				.Split(new[] { "::" }, StringSplitOptions.None)
				.All(part => !string.IsNullOrWhiteSpace(part) && IsValidIdentifier(part));
		}
	}
}
