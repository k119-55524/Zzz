
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using editor.Services;
using editor.Services.Project.Infrastructure;

namespace editor.Views.Widgets
{
	public partial class AssetsWidget : UserControl
	{
		// Узлы, добавленные через "Добавить -> Папку" и еще не подтвержденные вводом имени.
		private readonly HashSet<ProjectNode> _pendingNewNodes = new();
		private readonly Dictionary<ProjectNode, string> _pendingNewScripts = new();
		private System.Windows.Window? _hostWindow;

		public AssetsWidget()
		{
			InitializeComponent();
		}

		private static string Loc(string key, string fallback)
		{
			return System.Windows.Application.Current?.TryFindResource(key) as string ?? fallback;
		}

		// Кликаем за пределами виджета (другая панель, тулбар и т.п.) тоже должны завершать
		// редактирование - иначе поле ввода остаётся открытым, т.к. PreviewMouseDown на самом
		// UserControl ловит только клики внутри его границ.
		private void AssetsWidget_Loaded(object sender, System.Windows.RoutedEventArgs e)
		{
			_hostWindow = System.Windows.Window.GetWindow(this);
			_hostWindow?.AddHandler(System.Windows.UIElement.PreviewMouseDownEvent, new MouseButtonEventHandler(Window_PreviewMouseDown), true);
		}

		private void AssetsWidget_Unloaded(object sender, System.Windows.RoutedEventArgs e)
		{
			_hostWindow?.RemoveHandler(System.Windows.UIElement.PreviewMouseDownEvent, new MouseButtonEventHandler(Window_PreviewMouseDown));
			_hostWindow = null;
		}

		private void Window_PreviewMouseDown(object sender, MouseButtonEventArgs e)
		{
			CommitEditIfClickOutside(e.OriginalSource as System.Windows.DependencyObject);
		}

		private void TreeView_SelectedItemChanged(object sender, System.Windows.RoutedPropertyChangedEventArgs<object> e)
		{
			App.SelectionService.SelectedItem = e.NewValue;
		}

		private ProjectNode? GetSelectedNode(MenuItem menuItem)
		{
			return menuItem.DataContext as ProjectNode;
		}

		private void CopyName_Click(object sender, System.Windows.RoutedEventArgs e)
		{
			if (sender is MenuItem menuItem)
			{
				var node = GetSelectedNode(menuItem);
				if (node != null)
				{
					try
					{
						System.Windows.Clipboard.SetText(node.Name);
					}
					catch (Exception)
					{
						// Robust fallback for clipboard locking
						for (int i = 0; i < 10; i++)
						{
							try
							{
								System.Windows.Clipboard.SetDataObject(node.Name, true);
								break;
							}
							catch { System.Threading.Thread.Sleep(10); }
						}
					}
				}
			}
		}

		private void OpenInExplorer_Click(object sender, System.Windows.RoutedEventArgs e)
		{
			if (sender is MenuItem menuItem)
			{
				var node = GetSelectedNode(menuItem);
				if (node != null)
				{
					var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
					string? projectRoot = mainVm?.CurrentProjectPath;
					var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
					if (mainVm != null && !string.IsNullOrEmpty(projectRoot) && mainAssetsVm != null)
					{
						bool isSystemMode = mainAssetsVm.IsSystemNode(node);
						var activeFilters = App.ProjectService.CurrentSettings.DisabledFilters;
						var paths = GetPhysicalPaths(node, projectRoot, isSystemMode, activeFilters);
						foreach (var path in paths)
						{
							if (System.IO.File.Exists(path) || System.IO.Directory.Exists(path))
							{
								try
								{
									var psi = new System.Diagnostics.ProcessStartInfo
									{
										FileName = "explorer.exe",
										Arguments = $"/select,\"{path.Replace('/', '\\')}\"",
										UseShellExecute = true
									};
									System.Diagnostics.Process.Start(psi);
									break; // Only open one explorer window
								}
								catch { }
							}
						}
					}
				}
			}
		}



		private void AddFolder_Click(object sender, System.Windows.RoutedEventArgs e)
		{
			if (sender is MenuItem menuItem)
			{
				var clickedNode = GetSelectedNode(menuItem);
				if (clickedNode != null)
				{
					// Запоминаем узел по пути, а не по ссылке: CommitActiveEditIfAny() ниже может
					// закоммитить чужое редактирование и вызвать RefreshTree(), которая пересобирает
					// все ProjectNode как новые объекты - старая ссылка clickedNode после этого "осиротеет".
					string targetRelativePath = clickedNode.RelativePath;

					CommitActiveEditIfAny();

					var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
					var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
					if (mainVm != null && mainAssetsVm != null)
					{
						// Ищем узел в обоих деревьях по отдельности, а не через mainAssetsVm.RootNodes -
						// это свойство зависит от тумблера IsSystemMode и может указывать не на то дерево,
						// где реально находится узел (AssetsTree и SystemTree видны одновременно).
						var node = FindNodeByPath(mainAssetsVm.AssetRootNodes, targetRelativePath);
						var ownerRoots = mainAssetsVm.AssetRootNodes;
						if (node == null)
						{
							node = FindNodeByPath(mainAssetsVm.SystemRootNodes, targetRelativePath);
							ownerRoots = mainAssetsVm.SystemRootNodes;
						}
						if (node == null) return;

						ProjectNode parentNode;
						string parentRelativePath;

						if (node.IsFolder)
						{
							parentNode = node;
							parentRelativePath = node.RelativePath;
						}
						else
						{
							parentNode = FindParentNode(ownerRoots, node, out parentRelativePath);
						}

						string baseName = Loc("Tree_NewFolder_Name", "New Folder");
						string folderName = baseName;
						int index = 1;
						var siblings = parentNode != null ? parentNode.Children.ToList() : ownerRoots.ToList();
						while (siblings.Any(c => c.Name.Equals(folderName, StringComparison.OrdinalIgnoreCase)))
						{
							folderName = $"{baseName} {index++}";
						}

						string newRelPath = string.IsNullOrEmpty(parentRelativePath) ? folderName : $"{parentRelativePath}/{folderName}";

						var newFolderNode = new ProjectNode
						{
							Name = folderName,
							RelativePath = newRelPath,
							IsFolder = true,
							IsEditing = true
						};
						_pendingNewNodes.Add(newFolderNode);

						if (parentNode != null)
						{
							parentNode.Children.Add(newFolderNode);
						}
						else
						{
							ownerRoots.Add(newFolderNode);
						}
					}
				}
			}
		}


		private void AddFolderToRoot_Click(object sender, System.Windows.RoutedEventArgs e)
		{
			CommitActiveEditIfAny();
			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
			if (mainVm != null && mainAssetsVm != null)
			{
				// Этот пункт меню навешан только на пустую область AssetsTree (см. XAML),
				// поэтому он всегда добавляет папку в ассеты, независимо от того, развёрнута
				// ли сейчас секция "Project" (SystemTree) - тумблер IsSystemMode тут не при чём.
				string baseName = Loc("Tree_NewFolder_Name", "New Folder");
				string folderName = baseName;
				int index = 1;
				var siblings = mainAssetsVm.AssetRootNodes.ToList();
				while (siblings.Any(c => c.Name.Equals(folderName, StringComparison.OrdinalIgnoreCase)))
				{
					folderName = $"{baseName} {index++}";
				}

				string newRelPath = $"Assets/{folderName}";

				var newFolderNode = new ProjectNode
				{
					Name = folderName,
					RelativePath = newRelPath,
					IsFolder = true,
					IsEditing = true
				};
				_pendingNewNodes.Add(newFolderNode);

				mainAssetsVm.AssetRootNodes.Add(newFolderNode);
			}
		}

		private void AddScriptToRoot_Click(object sender, System.Windows.RoutedEventArgs e)
		{
			if (sender is not MenuItem menuItem) return;
			string scriptType = menuItem.Tag as string ?? "Script";

			CommitActiveEditIfAny();
			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
			if (mainVm == null || mainAssetsVm == null) return;

			// Этот пункт меню навешан только на пустую область AssetsTree (см. XAML),
			// поэтому он всегда добавляет скрипт в ассеты, как и AddFolderToRoot_Click.
			string baseName = "NewScript";
			if (scriptType == "Game") baseName = "NewGameScript";
			else if (scriptType == "Scene") baseName = "NewSceneScript";

			string scriptName = baseName;
			int index = 1;
			var siblings = mainAssetsVm.AssetRootNodes.ToList();
			while (siblings.Any(c => c.Name.Equals(scriptName, StringComparison.OrdinalIgnoreCase)))
			{
				scriptName = $"{baseName}_{index++}";
			}

			string newRelPath = $"Assets/{scriptName}";

			var newScriptNode = new ProjectNode
			{
				Name = scriptName,
				RelativePath = newRelPath,
				IsFolder = false,
				IsScript = true,
				IsEditing = true
			};

			_pendingNewNodes.Add(newScriptNode);
			_pendingNewScripts[newScriptNode] = scriptType;

			mainAssetsVm.AssetRootNodes.Add(newScriptNode);
		}

		private void NodeEditTextBox_Loaded(object sender, System.Windows.RoutedEventArgs e)
		{
			if (sender is TextBox textBox)
			{
				textBox.Focus();
				textBox.SelectAll();
			}
		}

		private void NodeEditTextBox_LostFocus(object sender, System.Windows.RoutedEventArgs e)
		{
			if (sender is TextBox textBox && textBox.DataContext is ProjectNode node)
			{
				if (node.IsEditing)
				{
					CommitRename(node, textBox.Text);
				}
			}
		}

		private void NodeEditTextBox_KeyDown(object sender, KeyEventArgs e)
		{
			if (sender is TextBox textBox && textBox.DataContext is ProjectNode node)
			{
				if (e.Key == Key.Enter)
				{
					e.Handled = true;
					CommitRename(node, textBox.Text);
				}
				else if (e.Key == Key.Escape)
				{
					e.Handled = true;
					node.IsEditing = false;
					RefreshTree();
				}
			}
		}

		private void CommitRename(ProjectNode node, string newName)
		{
			node.IsEditing = false;
			bool isPendingNew = _pendingNewNodes.Contains(node);

			newName = newName?.Trim();
			if (string.IsNullOrEmpty(newName))
			{
				if (isPendingNew) _pendingNewNodes.Remove(node);
				if (_pendingNewScripts.ContainsKey(node)) _pendingNewScripts.Remove(node);
				RefreshTree();
				return;
			}

			char[] invalidChars = System.IO.Path.GetInvalidFileNameChars();
			if (newName.IndexOfAny(invalidChars) >= 0)
			{
				EditorLogger.LogError(Loc("Validation_Name_InvalidChars", "Name contains invalid characters."));
				if (isPendingNew) _pendingNewNodes.Remove(node);
				if (_pendingNewScripts.ContainsKey(node)) _pendingNewScripts.Remove(node);
				RefreshTree();
				return;
			}

			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			string? projectRoot = mainVm?.CurrentProjectPath;
			var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
			if (mainVm == null || string.IsNullOrEmpty(projectRoot) || mainAssetsVm == null)
			{
				RefreshTree();
				return;
			}

			bool isSystemMode = mainAssetsVm.IsSystemNode(node);
			var activeFilters = App.ProjectService.CurrentSettings.DisabledFilters;

			// Новый ещё не созданный на диске скрипт тоже имеет IsScript=true (см. AddScript_Click/
			// AddScriptToRoot_Click), но у него нет HasHpp/HasCpp/HasMeta - значит это не переименование,
			// а первое сохранение имени, и его нужно обработать в ветке isPendingNew ниже.
			if (node.IsScript && !isPendingNew)
			{
				if (newName.Equals(node.Name, StringComparison.OrdinalIgnoreCase))
				{
					RefreshTree();
					return;
				}

				// Переименование hpp/cpp/meta выполняется одной атомарной командой (Undo/Redo одним
				// шагом), поэтому сначала проверяем ВСЕ целевые пути и только потом что-либо исполняем.
				var renameTargets = new List<(string OldPath, string NewPath)>();
				string? newMetaPath = null;

				if (node.HasHpp && !string.IsNullOrEmpty(node.HppRelativePath))
				{
					string oldPhysPath = System.IO.Path.Combine(projectRoot, node.HppRelativePath);
					string? oldDir = System.IO.Path.GetDirectoryName(oldPhysPath);
					if (oldDir != null)
					{
						string newPhysPath = System.IO.Path.Combine(oldDir, newName + ".hpp");
						if (System.IO.File.Exists(newPhysPath))
						{
							EditorLogger.LogError(Loc("Validation_FileName_Exists", "A file with this name already exists."));
							RefreshTree();
							return;
						}
						renameTargets.Add((oldPhysPath, newPhysPath));
					}
				}
				if (node.HasCpp && !string.IsNullOrEmpty(node.CppRelativePath))
				{
					string oldPhysPath = System.IO.Path.Combine(projectRoot, node.CppRelativePath);
					string? oldDir = System.IO.Path.GetDirectoryName(oldPhysPath);
					if (oldDir != null)
					{
						string newPhysPath = System.IO.Path.Combine(oldDir, newName + ".cpp");
						if (System.IO.File.Exists(newPhysPath))
						{
							EditorLogger.LogError(Loc("Validation_FileName_Exists", "A file with this name already exists."));
							RefreshTree();
							return;
						}
						renameTargets.Add((oldPhysPath, newPhysPath));
					}
				}
				if (node.HasMeta && !string.IsNullOrEmpty(node.MetaRelativePath))
				{
					string oldPhysPath = System.IO.Path.Combine(projectRoot, node.MetaRelativePath);
					string? oldDir = System.IO.Path.GetDirectoryName(oldPhysPath);
					if (oldDir != null)
					{
						string newPhysPath = System.IO.Path.Combine(oldDir, newName + ".meta");
						if (System.IO.File.Exists(newPhysPath))
						{
							EditorLogger.LogError(Loc("Validation_FileName_Exists", "A file with this name already exists."));
							RefreshTree();
							return;
						}
						renameTargets.Add((oldPhysPath, newPhysPath));
						newMetaPath = newPhysPath;
					}
				}

				var commands = new List<editor.Services.Project.Infrastructure.UndoRedo.ICommand>();
				foreach (var (oldPath, newPath) in renameTargets)
				{
					commands.Add(new editor.Services.Project.Infrastructure.UndoRedo.MoveOrRenameCommand(oldPath, newPath, App.ProjectService.Storage, onScriptChanged: TriggerScriptRebuild));
				}
				if (newMetaPath != null)
				{
					commands.Add(new editor.Services.Project.Infrastructure.UndoRedo.UpdateScriptMetaClassNameCommand(newMetaPath, newName, App.ProjectService.Storage));
				}

				if (commands.Count > 0)
				{
					var composite = new editor.Services.Project.Infrastructure.UndoRedo.CompositeCommand(commands);
					App.ProjectService.History.Execute(composite);
					EditorLogger.LogInfo($"[Meta System] Renamed script '{node.Name}' to '{newName}' (hpp/cpp/meta updated atomically).");
				}

				if (mainVm != null) mainVm.RefreshDirtyState();
				RefreshTree();
				return;
			}

			string actualNewName = newName;
			if (!node.IsFolder)
			{
				string ext = System.IO.Path.GetExtension(node.Name);
				actualNewName = newName + ext;
			}
			if (actualNewName.Equals(node.Name, StringComparison.OrdinalIgnoreCase) && !isPendingNew)
			{
				RefreshTree();
				return;
			}

			string parentPath = "";
			int lastSlash = node.RelativePath.LastIndexOf('/');
			if (lastSlash != -1)
			{
				parentPath = node.RelativePath.Substring(0, lastSlash);
			}

			string newRelPath = string.IsNullOrEmpty(parentPath) ? actualNewName : $"{parentPath}/{actualNewName}";

			if (isPendingNew)
			{
				if (_pendingNewScripts.ContainsKey(node))
				{
					string scriptType = _pendingNewScripts[node];
					_pendingNewScripts.Remove(node);
					_pendingNewNodes.Remove(node);

					var scriptCmd = new editor.Services.Project.Infrastructure.UndoRedo.CreateScriptCommand(
						newRelPath,
						newName,
						scriptType,
						GetTemplatesDirectory(),
						projectRoot,
						App.ProjectService.Storage,
						onScriptChanged: TriggerScriptRebuild
					);

					try
					{
						App.ProjectService.History.Execute(scriptCmd);
						EditorLogger.LogInfo($"Successfully created script '{newName}' from '{scriptType}' template.");
						if (mainVm != null) mainVm.RefreshDirtyState();
					}
					catch (Exception ex)
					{
						EditorLogger.LogError($"Failed to create script: {ex.Message}");
					}

					RefreshTree();
					return;
				}

				_pendingNewNodes.Remove(node);

				var cmd = new editor.Services.Project.Infrastructure.UndoRedo.CreateFolderCommand(
					newRelPath,
					projectRoot,
					isSystemMode,
					App.ProjectService.Storage
				);

				try
				{
					App.ProjectService.History.Execute(cmd);
					if (mainVm != null) mainVm.RefreshDirtyState();
				}
				catch (Exception ex)
				{
					EditorLogger.LogError(string.Format(Loc("Error_CreateFolder_Failed", "Failed to create folder: {0}"), ex.Message));
				}

				RefreshTree();
				return;
			}

			var physicalPaths = GetPhysicalPaths(node, projectRoot, isSystemMode, activeFilters);
			foreach (var oldPhysPath in physicalPaths)
			{
				string? oldDir = System.IO.Path.GetDirectoryName(oldPhysPath);
				if (oldDir != null)
				{
					string newPhysPath = System.IO.Path.Combine(oldDir, actualNewName);
					
					// Валидация: если целевой путь уже существует
					if (node.IsFolder && System.IO.Directory.Exists(newPhysPath))
					{
						EditorLogger.LogError(Loc("Validation_FolderName_Exists", "A folder with this name already exists."));
						RefreshTree();
						return;
					}
					else if (!node.IsFolder && System.IO.File.Exists(newPhysPath))
					{
						EditorLogger.LogError(Loc("Validation_FileName_Exists", "A file with this name already exists."));
						RefreshTree();
						return;
					}

					var cmd = new editor.Services.Project.Infrastructure.UndoRedo.MoveOrRenameCommand(
						oldPhysPath,
						newPhysPath,
						App.ProjectService.Storage,
						onScriptChanged: TriggerScriptRebuild
					);

					try
					{
						App.ProjectService.History.Execute(cmd);
						if (mainVm != null) mainVm.RefreshDirtyState();
					}
					catch (Exception ex)
					{
						EditorLogger.LogError(string.Format(Loc("Error_Rename_Failed", "Failed to rename: {0}"), ex.Message));
					}
				}
			}

			RefreshTree();
		}

		private ProjectNode FindNodeByPath(System.Collections.ObjectModel.ObservableCollection<ProjectNode> nodes, string relativePath)
		{
			foreach (var node in nodes)
			{
				if (node.RelativePath == relativePath) return node;
				var found = FindNodeByPath(node.Children, relativePath);
				if (found != null) return found;
			}
			return null;
		}

		private ProjectNode FindParentNode(System.Collections.ObjectModel.ObservableCollection<ProjectNode> roots, ProjectNode target, out string parentPath)
		{
			parentPath = "";
			foreach (var root in roots)
			{
				if (root.Children.Contains(target))
				{
					parentPath = root.RelativePath;
					return root;
				}
				var found = FindParentNode(root.Children, target, out parentPath);
				if (found != null) return found;
			}
			return null;
		}

		private List<string> GetPhysicalPaths(ProjectNode node, string projectRoot, bool isSystemMode, List<string> disabledFilters)
		{
			var paths = new List<string>();
			if (node.IsScript)
			{
				if (node.HasHpp && !string.IsNullOrEmpty(node.HppRelativePath))
					paths.Add(System.IO.Path.Combine(projectRoot, node.HppRelativePath));
				if (node.HasCpp && !string.IsNullOrEmpty(node.CppRelativePath))
					paths.Add(System.IO.Path.Combine(projectRoot, node.CppRelativePath));
				if (node.HasMeta && !string.IsNullOrEmpty(node.MetaRelativePath))
					paths.Add(System.IO.Path.Combine(projectRoot, node.MetaRelativePath));
				return paths;
			}
			string relPath = node.RelativePath;
			if (string.IsNullOrEmpty(relPath)) return paths;

			paths.Add(System.IO.Path.Combine(projectRoot, relPath));
			return paths;
		}

		private void TreeView_ContextMenuOpening(object sender, ContextMenuEventArgs e)
		{
			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
			if (mainAssetsVm != null)
			{
				if (e.OriginalSource is System.Windows.DependencyObject depObj)
				{
					var item = FindVisualParent<TreeViewItem>(depObj);
					if (item != null)
					{
						if (item.Header is ProjectNode node)
						{
							var contextMenu = item.ContextMenu;
							if (contextMenu != null)
							{
								var tree = FindVisualParent<TreeView>(depObj);
								bool isSystem = (tree == SystemTree);
								foreach (var menuObj in contextMenu.Items)
								{
									if (menuObj is MenuItem menuItem)
									{
										if (menuItem.Name == "DeleteMenuItem")
										{
											menuItem.Visibility = isSystem ? System.Windows.Visibility.Collapsed : System.Windows.Visibility.Visible;
											menuItem.Click -= Delete_Click;
											menuItem.Click += Delete_Click;
										}
										else if (menuItem.Name == "AddMenuItem")
										{
											menuItem.Visibility = (isSystem || !node.IsFolder) ? System.Windows.Visibility.Collapsed : System.Windows.Visibility.Visible;
											foreach (var subObj in menuItem.Items)
											{
												if (subObj is MenuItem subMenuItem)
												{
													// Пункты скриптов помечены Tag (см. XAML: AddScriptMenuItem/AddGameScriptMenuItem/
													// AddSceneScriptMenuItem), пункт папки - без Tag.
													if (subMenuItem.Tag is string)
													{
														subMenuItem.Click -= AddScript_Click;
														subMenuItem.Click += AddScript_Click;
													}
													else
													{
														subMenuItem.Click -= AddFolder_Click;
														subMenuItem.Click += AddFolder_Click;
													}
												}
											}
										}
										else if (menuItem.Name == "OpenInExplorerMenuItem")
										{
											menuItem.Visibility = _pendingNewNodes.Contains(node) ? System.Windows.Visibility.Collapsed : System.Windows.Visibility.Visible;
											menuItem.Click -= OpenInExplorer_Click;
											menuItem.Click += OpenInExplorer_Click;
										}
										else if (menuItem.Name == "CopyNameMenuItem")
										{
											menuItem.Click -= CopyName_Click;
											menuItem.Click += CopyName_Click;
										}
									}
								}
							}
						}
						return;
					}

					// Clicking empty space
					var treeControl = FindVisualParent<TreeView>(depObj);
					if (treeControl == SystemTree)
					{
						e.Handled = true; // Block ContextMenu on empty space in SystemTree
					}
				}
			}
		}

		private T? FindVisualParent<T>(System.Windows.DependencyObject child) where T : System.Windows.DependencyObject
		{
			// VisualTreeHelper.GetParent падает на элементах, которые не Visual/Visual3D
			// (например, System.Windows.Documents.Run внутри TextBlock в заголовке MenuItem) -
			// для них поднимаемся по логическому дереву, пока не дойдём до Visual.
			System.Windows.DependencyObject? parentObject =
				child is System.Windows.Media.Visual || child is System.Windows.Media.Media3D.Visual3D
					? System.Windows.Media.VisualTreeHelper.GetParent(child)
					: System.Windows.LogicalTreeHelper.GetParent(child);

			if (parentObject == null) return null;
			if (parentObject is T parent) return parent;
			return FindVisualParent<T>(parentObject);
		}

		private void Delete_Click(object sender, System.Windows.RoutedEventArgs e)
		{
			if (sender is MenuItem menuItem)
			{
				var node = GetSelectedNode(menuItem);
				if (node != null)
				{
					var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
					var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
					var settings = App.ProjectService.CurrentSettings;
					if (mainVm != null && mainAssetsVm != null)
					{
						string? projectRoot = mainVm.CurrentProjectPath;
						if (string.IsNullOrEmpty(projectRoot)) return;

						bool isSystemMode = mainAssetsVm.IsSystemNode(node);
						var activeFilters = settings.DisabledFilters;

						string title = System.Windows.Application.Current?.TryFindResource("Dialog_Delete_Title") as string ?? "Delete Item";
						string confirmFormat = System.Windows.Application.Current?.TryFindResource("Dialog_Delete_Confirm") as string ?? "Are you sure you want to delete '{0}'?";
						string confirmMsg = string.Format(confirmFormat, node.Name);

						var result = System.Windows.MessageBox.Show(confirmMsg, title, System.Windows.MessageBoxButton.YesNo, System.Windows.MessageBoxImage.Warning);
						if (result == System.Windows.MessageBoxResult.Yes)
						{
							var physicalPaths = GetPhysicalPaths(node, projectRoot, isSystemMode, activeFilters);

							// Для папок: проверяем наличие .hpp/.cpp до перемещения в backup
							Func<bool>? containsScripts = node.IsFolder
								? () => physicalPaths.Any(p =>
									{
										// После Execute файлы уже перемещены в backup;
										// проверяем backup-папку или оригинал
										string checkDir = System.IO.Directory.Exists(p) ? p
											: System.IO.Path.Combine(projectRoot, ".editor", "backup");
										return System.IO.Directory.Exists(checkDir) &&
											(System.IO.Directory.GetFiles(checkDir, "*.hpp", System.IO.SearchOption.AllDirectories).Length > 0 ||
											System.IO.Directory.GetFiles(checkDir, "*.cpp", System.IO.SearchOption.AllDirectories).Length > 0);
									})
								: (Func<bool>?)null;

							var cmd = new editor.Services.Project.Infrastructure.UndoRedo.DeleteFileOrFolderCommand(
								physicalPaths,
								projectRoot,
								node.IsFolder,
								App.ProjectService.Storage,
								onScriptChanged: TriggerScriptRebuild,
								containsScripts: containsScripts
							);

							try
							{
								App.ProjectService.History.Execute(cmd);
								if (mainVm != null) mainVm.RefreshDirtyState();
							}
							catch (Exception ex)
							{
								EditorLogger.LogError(string.Format(Loc("Error_Delete_Failed", "Failed to delete: {0}"), ex.Message));
							}

							RefreshTree();
						}
					}
				}
			}
		}

		private void RefreshTree()
		{
			if (DataContext is ViewModels.AssetsViewModel vm)
			{
				vm.RefreshTree();
			}
		}

		// Колбэк для команд Undo/Redo: RefreshTree обновляет RegisterAllScripts.cpp,
		// затем запускается forceRebuild-компиляция. Порядок важен!
		private void TriggerScriptRebuild()
		{
			RefreshTree();
			if (_hostWindow is MainWindow mw)
				_ = mw.CheckAndCompileScriptsAsync(forceRebuild: true);
		}

		// === Drag & Drop: перетаскивание узлов AssetsTree для изменения структуры проекта ===
		// Работает для папок и файлов одинаково (MoveOrRenameCommand не различает их), но запускается
		// только из AssetsTree - SystemTree остаётся read-only, как и для создания/удаления/переименования.

		private ProjectNode? _dragCandidateNode;
		private System.Windows.Point _dragStartPoint;
		private TreeViewItem? _dropHighlightItem;

		private void AssetsTree_PreviewMouseLeftButtonDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
		{
			var item = FindVisualParent<TreeViewItem>(e.OriginalSource as System.Windows.DependencyObject);
			var node = item?.DataContext as ProjectNode;
			_dragCandidateNode = (node != null && !node.IsEditing) ? node : null;
			_dragStartPoint = e.GetPosition(null);
		}

		private void AssetsTree_PreviewMouseMove(object sender, System.Windows.Input.MouseEventArgs e)
		{
			if (_dragCandidateNode == null || e.LeftButton != System.Windows.Input.MouseButtonState.Pressed)
			{
				return;
			}

			var pos = e.GetPosition(null);
			if (Math.Abs(pos.X - _dragStartPoint.X) < System.Windows.SystemParameters.MinimumHorizontalDragDistance &&
				Math.Abs(pos.Y - _dragStartPoint.Y) < System.Windows.SystemParameters.MinimumVerticalDragDistance)
			{
				return;
			}

			var draggedNode = _dragCandidateNode;
			_dragCandidateNode = null; // одна попытка перетаскивания на жест мыши

			var data = new System.Windows.DataObject(typeof(ProjectNode), draggedNode);
			System.Windows.DragDrop.DoDragDrop(AssetsTree, data, System.Windows.DragDropEffects.Move);
		}

		private void AssetsTree_DragOver(object sender, System.Windows.DragEventArgs e)
		{
			e.Handled = true;
			var targetItem = FindVisualParent<TreeViewItem>(e.OriginalSource as System.Windows.DependencyObject);
			bool isValid = TryResolveDropTarget(e, targetItem, out _, out _);
			e.Effects = isValid ? System.Windows.DragDropEffects.Move : System.Windows.DragDropEffects.None;
			SetDropHighlight(isValid ? targetItem : null);
		}

		private void AssetsTree_DragLeave(object sender, System.Windows.DragEventArgs e)
		{
			SetDropHighlight(null);
		}

		private void AssetsTree_Drop(object sender, System.Windows.DragEventArgs e)
		{
			e.Handled = true;
			SetDropHighlight(null);

			var targetItem = FindVisualParent<TreeViewItem>(e.OriginalSource as System.Windows.DependencyObject);
			if (!TryResolveDropTarget(e, targetItem, out var draggedNode, out var targetFolderPath))
			{
				return;
			}

			// Коммитим чужое незакрытое редактирование до переноса - тот же порядок, что и в AddFolder_Click.
			CommitActiveEditIfAny();
			MoveNode(draggedNode, targetFolderPath);
		}

		// Определяет, можно ли перетащить узел из e.Data на targetItem, и вычисляет относительный путь
		// папки назначения. Общая проверка для подсветки при DragOver и для самого Drop.
		private bool TryResolveDropTarget(System.Windows.DragEventArgs e, TreeViewItem? targetItem, out ProjectNode draggedNode, out string targetFolderPath)
		{
			draggedNode = null;
			targetFolderPath = null;

			if (!e.Data.GetDataPresent(typeof(ProjectNode))) return false;
			draggedNode = (ProjectNode)e.Data.GetData(typeof(ProjectNode));
			if (draggedNode == null) return false;

			var vm = DataContext as ViewModels.AssetsViewModel;
			if (vm == null) return false;

			var targetNode = targetItem?.DataContext as ProjectNode;
			ProjectNode targetFolder = targetNode == null
				? null
				: (targetNode.IsFolder ? targetNode : FindParentNode(vm.AssetRootNodes, targetNode, out _));

			targetFolderPath = targetFolder?.RelativePath ?? "Assets";

			if (targetFolder == draggedNode) return false;

			string currentParentPath = GetParentRelativePath(draggedNode.RelativePath);
			if (targetFolderPath.Equals(currentParentPath, StringComparison.OrdinalIgnoreCase)) return false;

			if (draggedNode.IsFolder && IsSameOrDescendantPath(targetFolderPath, draggedNode.RelativePath)) return false;

			return true;
		}

		private static string GetParentRelativePath(string relativePath)
		{
			int lastSlash = relativePath.LastIndexOf('/');
			return lastSlash == -1 ? "" : relativePath.Substring(0, lastSlash);
		}

		private static bool IsSameOrDescendantPath(string candidatePath, string basePath)
		{
			return candidatePath.Equals(basePath, StringComparison.OrdinalIgnoreCase) ||
				   candidatePath.StartsWith(basePath + "/", StringComparison.OrdinalIgnoreCase);
		}

		private void SetDropHighlight(TreeViewItem? item)
		{
			if (_dropHighlightItem == item) return;

			if (_dropHighlightItem != null)
			{
				_dropHighlightItem.ClearValue(System.Windows.Controls.Control.BackgroundProperty);
			}

			_dropHighlightItem = item;

			if (_dropHighlightItem != null)
			{
				_dropHighlightItem.Background = (System.Windows.Media.Brush)System.Windows.Application.Current.FindResource("Brush_Hover");
			}
		}

		// Физически переносит узел на диске в папку targetFolderPath, с фиксацией в Undo/Redo
		// через тот же MoveOrRenameCommand, что используется при переименовании.
		private void MoveNode(ProjectNode draggedNode, string targetFolderPath)
		{
			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			string? projectRoot = mainVm?.CurrentProjectPath;
			if (mainVm == null || string.IsNullOrEmpty(projectRoot)) return;

			string newRelPath = $"{targetFolderPath}/{draggedNode.Name}";
			string oldPhysPath = System.IO.Path.Combine(projectRoot, draggedNode.RelativePath);
			string newPhysPath = System.IO.Path.Combine(projectRoot, newRelPath.Replace('/', System.IO.Path.DirectorySeparatorChar));

			if (draggedNode.IsFolder && System.IO.Directory.Exists(newPhysPath))
			{
				EditorLogger.LogError(Loc("Validation_FolderName_Exists", "A folder with this name already exists."));
				return;
			}
			if (!draggedNode.IsFolder && System.IO.File.Exists(newPhysPath))
			{
				EditorLogger.LogError(Loc("Validation_FileName_Exists", "A file with this name already exists."));
				return;
			}

			var cmd = new editor.Services.Project.Infrastructure.UndoRedo.MoveOrRenameCommand(oldPhysPath, newPhysPath, App.ProjectService.Storage, onScriptChanged: TriggerScriptRebuild);
			try
			{
				App.ProjectService.History.Execute(cmd);
				mainVm.RefreshDirtyState();
			}
			catch (Exception ex)
			{
				EditorLogger.LogError(string.Format(Loc("Error_Rename_Failed", "Failed to rename: {0}"), ex.Message));
			}

			RefreshTree();
		}

		private void NodeText_PreviewMouseLeftButtonDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
		{
			if (sender is FrameworkElement element && element.DataContext is ProjectNode node)
			{
				if (e.ClickCount == 2)
				{
					OpenFileInVS(node);
					e.Handled = true;
					return;
				}

				if (sender is TextBlock && App.SelectionService.SelectedItem == node)
				{
					CommitActiveEditIfAny();
					node.IsEditing = true;
					e.Handled = true;
				}
			}
		}

		private void BtnH_PreviewMouseLeftButtonDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
		{
			if (e.ClickCount == 2 && sender is FrameworkElement el && el.DataContext is ProjectNode node)
			{
				OpenHppOnly(node);
				e.Handled = true;
			}
		}

		private void BtnCpp_PreviewMouseLeftButtonDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
		{
			if (e.ClickCount == 2 && sender is FrameworkElement el && el.DataContext is ProjectNode node)
			{
				OpenCppOnly(node);
				e.Handled = true;
			}
		}

		private void OpenFileInVS(ProjectNode node)
		{
			if (node.IsFolder) return;

			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			string? projectRoot = mainVm?.CurrentProjectPath;
			if (string.IsNullOrEmpty(projectRoot)) return;

			if (node.IsScript)
			{
				if (node.HasHpp && !string.IsNullOrEmpty(node.HppRelativePath))
				{
					string path = System.IO.Path.Combine(projectRoot, node.HppRelativePath);
					if (System.IO.File.Exists(path))
					{
						LaunchFile(path);
					}
				}
				if (node.HasCpp && !string.IsNullOrEmpty(node.CppRelativePath))
				{
					string path = System.IO.Path.Combine(projectRoot, node.CppRelativePath);
					if (System.IO.File.Exists(path))
					{
						LaunchFile(path);
					}
				}
			}
			else
			{
				string path = System.IO.Path.Combine(projectRoot, node.RelativePath);
				if (System.IO.File.Exists(path))
				{
					LaunchFile(path);
				}
			}
		}

		private void OpenHppOnly(ProjectNode node)
		{
			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			string? projectRoot = mainVm?.CurrentProjectPath;
			if (string.IsNullOrEmpty(projectRoot)) return;

			if (node.IsScript && node.HasHpp && !string.IsNullOrEmpty(node.HppRelativePath))
			{
				string path = System.IO.Path.Combine(projectRoot, node.HppRelativePath);
				if (System.IO.File.Exists(path))
				{
					LaunchFile(path);
				}
			}
		}

		private void OpenCppOnly(ProjectNode node)
		{
			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			string? projectRoot = mainVm?.CurrentProjectPath;
			if (string.IsNullOrEmpty(projectRoot)) return;

			if (node.IsScript && node.HasCpp && !string.IsNullOrEmpty(node.CppRelativePath))
			{
				string path = System.IO.Path.Combine(projectRoot, node.CppRelativePath);
				if (System.IO.File.Exists(path))
				{
					LaunchFile(path);
				}
			}
		}

		private void LaunchFile(string path)
		{
			try
			{
				var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
				string? projectRoot = mainVm?.CurrentProjectPath;
				if (!string.IsNullOrEmpty(projectRoot))
				{
					string buildDir = System.IO.Path.Combine(projectRoot, ".editor", "build");
					
					string? slnPath = null;
					if (System.IO.Directory.Exists(buildDir))
					{
						var slnFiles = System.IO.Directory.GetFiles(buildDir, "*.sln");
						if (slnFiles.Length > 0) slnPath = slnFiles[0];
						else
						{
							var slnxFiles = System.IO.Directory.GetFiles(buildDir, "*.slnx");
							if (slnxFiles.Length > 0) slnPath = slnxFiles[0];
						}
					}

					if (!string.IsNullOrEmpty(slnPath))
					{
						var psi = new System.Diagnostics.ProcessStartInfo
						{
							FileName = "devenv",
							Arguments = $"\"{slnPath}\" /edit \"{path}\"",
							UseShellExecute = true
						};
						System.Diagnostics.Process.Start(psi);
						return;
					}
				}

				var psiFallback = new System.Diagnostics.ProcessStartInfo
				{
					FileName = path,
					UseShellExecute = true
				};
				System.Diagnostics.Process.Start(psiFallback);
			}
			catch (Exception ex)
			{
				EditorLogger.LogError($"Failed to open file: {ex.Message}");
			}
		}

		private void AddScript_Click(object sender, System.Windows.RoutedEventArgs e)
		{
			if (sender is MenuItem menuItem)
			{
				string scriptType = menuItem.Tag as string ?? "Script";
				var clickedNode = GetSelectedNode(menuItem);
				if (clickedNode != null)
				{
					string targetRelativePath = clickedNode.RelativePath;
					CommitActiveEditIfAny();

					var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
					var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
					if (mainVm != null && mainAssetsVm != null)
					{
						var node = FindNodeByPath(mainAssetsVm.AssetRootNodes, targetRelativePath);
						var ownerRoots = mainAssetsVm.AssetRootNodes;
						if (node == null)
						{
							node = FindNodeByPath(mainAssetsVm.SystemRootNodes, targetRelativePath);
							ownerRoots = mainAssetsVm.SystemRootNodes;
						}
						if (node == null) return;

						ProjectNode parentNode;
						string parentRelativePath;

						if (node.IsFolder)
						{
							parentNode = node;
							parentRelativePath = node.RelativePath;
						}
						else
						{
							parentNode = FindParentNode(ownerRoots, node, out parentRelativePath);
						}

						string baseName = "NewScript";
						if (scriptType == "Game") baseName = "NewGameScript";
						else if (scriptType == "Scene") baseName = "NewSceneScript";

						string scriptName = baseName;
						int index = 1;
						var siblings = parentNode != null ? parentNode.Children.ToList() : ownerRoots.ToList();
						while (siblings.Any(c => c.Name.Equals(scriptName, StringComparison.OrdinalIgnoreCase)))
						{
							scriptName = $"{baseName}_{index++}";
						}

						string newRelPath = string.IsNullOrEmpty(parentRelativePath) ? scriptName : $"{parentRelativePath}/{scriptName}";

						var newScriptNode = new ProjectNode
						{
							Name = scriptName,
							RelativePath = newRelPath,
							IsFolder = false,
							IsScript = true,
							IsEditing = true
						};

						_pendingNewNodes.Add(newScriptNode);
						_pendingNewScripts[newScriptNode] = scriptType;

						if (parentNode != null)
						{
							parentNode.Children.Add(newScriptNode);
						}
						else
						{
							ownerRoots.Add(newScriptNode);
						}
					}
				}
			}
		}

		private string GetTemplatesDirectory()
		{
			string baseDir = AppDomain.CurrentDomain.BaseDirectory;
			
			// 1. Проверяем локальный путь (если развернуто с билдом)
			string localPath = System.IO.Path.Combine(baseDir, "templates", "scripts");
			if (System.IO.Directory.Exists(localPath)) return localPath;

			// 2. Проверяем путь в репозитории разработчика
			string devPath = System.IO.Path.Combine(baseDir, "..", "..", "src", "editor_dll", "templates", "scripts");
			devPath = System.IO.Path.GetFullPath(devPath);
			if (System.IO.Directory.Exists(devPath)) return devPath;

			return localPath; // fallback
		}

		private void UserControl_PreviewMouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
		{
			CommitEditIfClickOutside(e.OriginalSource as System.Windows.DependencyObject);
		}

		// Завершает текущее редактирование, если клик пришёлся не на поле ввода имени.
		// Используется и для кликов внутри виджета, и для кликов в остальном окне (см. Window_PreviewMouseDown).
		private void CommitEditIfClickOutside(System.Windows.DependencyObject? originalSource)
		{
			var textBox = originalSource != null ? FindVisualParent<TextBox>(originalSource) : null;
			if (textBox != null && textBox.Name == "NodeEditTextBox")
			{
				return;
			}

			CommitActiveEditIfAny();
		}

		// Гарантирует, что в дереве одновременно редактируется не более одного узла:
		// коммитит уже открытое поле перед тем, как разрешить открыть новое.
		// AssetsTree и SystemTree - это два независимых, одновременно видимых дерева
		// (вкладка "Project" - это SystemTree, а не альтернативный режим того же дерева),
		// поэтому проверять нужно оба, а не только то, что соответствует текущему IsSystemMode.
		private void CommitActiveEditIfAny()
		{
			if (DataContext is ViewModels.AssetsViewModel vm)
			{
				var editingNode = FindEditingNode(vm.AssetRootNodes) ?? FindEditingNode(vm.SystemRootNodes);
				if (editingNode != null)
				{
					var activeTextBox = FindVisualChildren<TextBox>(AssetsTree)
						.Concat(FindVisualChildren<TextBox>(SystemTree))
						.FirstOrDefault(tb => tb.Name == "NodeEditTextBox" && tb.DataContext == editingNode);

					if (activeTextBox != null)
					{
						CommitRename(editingNode, activeTextBox.Text);
					}
					else
					{
						editingNode.IsEditing = false;
					}
				}
			}
		}

		private ProjectNode FindEditingNode(System.Collections.ObjectModel.ObservableCollection<ProjectNode> nodes)
		{
			foreach (var node in nodes)
			{
				if (node.IsEditing) return node;
				var child = FindEditingNode(node.Children);
				if (child != null) return child;
			}
			return null;
		}
		private System.Collections.Generic.IEnumerable<T> FindVisualChildren<T>(System.Windows.DependencyObject depObj) where T : System.Windows.DependencyObject
		{
			if (depObj != null)
			{
				for (int i = 0; i < System.Windows.Media.VisualTreeHelper.GetChildrenCount(depObj); i++)
				{
					System.Windows.DependencyObject child = System.Windows.Media.VisualTreeHelper.GetChild(depObj, i);
					if (child != null && child is T t)
					{
						yield return t;
					}

					foreach (T childOfChild in FindVisualChildren<T>(child))
					{
						yield return childOfChild;
					}
				}
			}
		}
	}
}
