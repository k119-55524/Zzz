using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using editor.Services.Project.Infrastructure;

namespace editor.Views
{
	public partial class NewScriptDialog : Window
	{
		private readonly string _projectRoot;
		private readonly string _targetFolderRelativePath;
		private readonly List<string> _recentNamespaces;
		private readonly string _defaultScriptType;
		private bool _isUpdatingNamespaceItems;

		public NewScriptDialog(
			Window? owner,
			string projectRoot,
			string targetFolderRelativePath,
			string defaultScriptType,
			IEnumerable<string> recentNamespaces)
		{
			InitializeComponent();

			Owner = owner;
			_projectRoot = projectRoot;
			_targetFolderRelativePath = targetFolderRelativePath;
			_recentNamespaces = recentNamespaces
				.Where(ns => !string.IsNullOrWhiteSpace(ns))
				.Distinct(StringComparer.Ordinal)
				.ToList();
			_defaultScriptType = defaultScriptType;

			// SelectedItem выставляем только после Loaded: до этого момента визуальное дерево
			// ComboBoxItem'ов ещё не построено, DynamicResource-содержимое (ScriptType_*) ещё не
			// зарезолвлено, и SelectionBoxItem (то, что рисуется в закрытом ComboBox) захватывает
			// пустое значение - выпадающий список визуально выглядит пустым до первого открытия.
			Loaded += NewScriptDialog_Loaded;
		}

		private void NewScriptDialog_Loaded(object sender, RoutedEventArgs e)
		{
			SetScriptType(_defaultScriptType);
			RefreshNamespaceItems();
			ScriptNameTextBox.Focus();
			ValidateInput(this, new RoutedEventArgs());
		}

		public string ScriptType { get; private set; } = "Script";
		public string ScriptNamespace { get; private set; } = string.Empty;
		public string ScriptName { get; private set; } = string.Empty;
		public IReadOnlyList<string> RemovedNamespaces { get; private set; } = Array.Empty<string>();

		private List<string> RemovedNamespaceBuffer { get; } = new();

		private void SetScriptType(string scriptType)
		{
			foreach (ComboBoxItem item in ScriptTypeComboBox.Items)
			{
				if (string.Equals(item.Tag as string, scriptType, StringComparison.Ordinal))
				{
					ScriptTypeComboBox.SelectedItem = item;
					return;
				}
			}

			ScriptTypeComboBox.SelectedIndex = 0;
		}

		private void RefreshNamespaceItems()
		{
			_isUpdatingNamespaceItems = true;
			string currentText = NamespaceComboBox.Text;
			NamespaceComboBox.ItemsSource = null;
			NamespaceComboBox.ItemsSource = _recentNamespaces;
			NamespaceComboBox.Text = currentText;
			_isUpdatingNamespaceItems = false;
		}

		private void ValidateInput(object sender, RoutedEventArgs e)
		{
			// IsSelected="True" на первом ComboBoxItem (см. NewScriptDialog.xaml) может дёрнуть
			// SelectionChanged ещё во время InitializeComponent(), раньше, чем в XAML объявлены
			// NamespaceComboBox/ScriptNameTextBox/StatusTextBlock/AddButton - до Loaded выходим,
			// чтобы не словить NullReferenceException на ещё не подключённых полях.
			if (!IsLoaded || _isUpdatingNamespaceItems)
			{
				return;
			}

			string? error = GetValidationError(out bool namespaceError, out bool nameError);
			SetFieldState(NamespaceComboBox, namespaceError);
			SetFieldState(ScriptNameTextBox, nameError);
			StatusTextBlock.Text = error ?? string.Empty;
			AddButton.IsEnabled = error == null;
		}

		private string? GetValidationError(out bool namespaceError, out bool nameError)
		{
			namespaceError = false;
			nameError = false;

			string scriptNamespace = NamespaceComboBox.Text?.Trim() ?? string.Empty;
			string scriptName = ScriptNameTextBox.Text?.Trim() ?? string.Empty;

			if (!string.IsNullOrEmpty(scriptNamespace) && !CppIdentifierValidation.IsValidNamespace(scriptNamespace))
			{
				namespaceError = true;
				return Loc("Validation_ScriptNamespace_Invalid", "Namespace must use C++ names separated by ::.");
			}

			if (string.IsNullOrEmpty(scriptName))
			{
				nameError = true;
				return Loc("Validation_Name_Empty", "Name cannot be empty.");
			}

			if (scriptName.IndexOfAny(Path.GetInvalidFileNameChars()) >= 0)
			{
				nameError = true;
				return Loc("Validation_Name_InvalidChars", "Name contains invalid characters.");
			}

			if (!CppIdentifierValidation.IsValidIdentifier(scriptName))
			{
				nameError = true;
				return Loc("Validation_ScriptName_InvalidIdentifier", "Script name must be a valid C++ class name.");
			}

			string targetFolder = Path.Combine(_projectRoot, _targetFolderRelativePath.Replace('/', Path.DirectorySeparatorChar));
			if (File.Exists(Path.Combine(targetFolder, scriptName + ".hpp")) ||
				File.Exists(Path.Combine(targetFolder, scriptName + ".cpp")) ||
				File.Exists(Path.Combine(targetFolder, scriptName + ".meta")))
			{
				nameError = true;
				return Loc("Validation_FileName_Exists", "A file with this name already exists.");
			}

			string qualifiedName = string.IsNullOrWhiteSpace(scriptNamespace)
				? scriptName
				: $"{scriptNamespace}::{scriptName}";
			if (App.ScriptAssetIndexService.ByQualifiedName.ContainsKey(qualifiedName))
			{
				nameError = true;
				return Loc("Validation_ScriptQualifiedName_Exists", "A script with this namespace and name already exists in the project.");
			}

			return null;
		}

		private static void SetFieldState(Control control, bool hasError)
		{
			if (hasError)
			{
				control.BorderBrush = Brushes.IndianRed;
			}
			else
			{
				control.ClearValue(Control.BorderBrushProperty);
			}
		}

		private void DeleteNamespace_Click(object sender, RoutedEventArgs e)
		{
			if (sender is not Button button || button.CommandParameter is not string scriptNamespace)
			{
				return;
			}

			_recentNamespaces.Remove(scriptNamespace);
			if (!RemovedNamespaceBuffer.Contains(scriptNamespace, StringComparer.Ordinal))
			{
				RemovedNamespaceBuffer.Add(scriptNamespace);
			}

			if (string.Equals(NamespaceComboBox.Text, scriptNamespace, StringComparison.Ordinal))
			{
				NamespaceComboBox.Text = string.Empty;
			}

			RefreshNamespaceItems();
			ValidateInput(this, new RoutedEventArgs());
			e.Handled = true;
		}

		private void Add_Click(object sender, RoutedEventArgs e)
		{
			string? error = GetValidationError(out _, out _);
			if (error != null)
			{
				ValidateInput(sender, e);
				return;
			}

			ScriptType = (ScriptTypeComboBox.SelectedItem as ComboBoxItem)?.Tag as string ?? "Script";
			ScriptNamespace = NamespaceComboBox.Text?.Trim() ?? string.Empty;
			ScriptName = ScriptNameTextBox.Text?.Trim() ?? string.Empty;
			RemovedNamespaces = RemovedNamespaceBuffer.ToArray();
			DialogResult = true;
		}

		private void Cancel_Click(object sender, RoutedEventArgs e)
		{
			RemovedNamespaces = RemovedNamespaceBuffer.ToArray();
			DialogResult = false;
		}

		private static string Loc(string key, string fallback)
		{
			return Application.Current?.TryFindResource(key) as string ?? fallback;
		}
	}
}
