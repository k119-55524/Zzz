
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
			// AssetsTree и SystemTree видны одновременно и независимо друг от друга хранят
			// собственный SelectedItem. Если не сбрасывать выделение в другом дереве, то повторный
			// клик по узлу, оставшемуся "выделенным" там с прошлого раза, не поднимет
			// SelectedItemChanged (для WPF это не изменение значения) - и SelectionService/Inspector
			// не узнают о клике, хотя визуально этот узел даже не выглядел выделенным (фокус был
			// в другом дереве).
			var otherTree = sender == AssetsTree ? SystemTree : AssetsTree;
			if (otherTree.SelectedItem != null)
			{
				otherTree.SelectedItemChanged -= TreeView_SelectedItemChanged;
				ClearTreeSelection(otherTree);
				otherTree.SelectedItemChanged += TreeView_SelectedItemChanged;
			}

			App.SelectionService.SelectedItem = e.NewValue;
		}

		private static void ClearTreeSelection(ItemsControl container)
		{
			foreach (var item in container.Items)
			{
				if (container.ItemContainerGenerator.ContainerFromItem(item) is not TreeViewItem treeViewItem)
				{
					continue;
				}

				if (treeViewItem.IsSelected)
				{
					treeViewItem.IsSelected = false;
					return;
				}

				ClearTreeSelection(treeViewItem);
			}
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
						// Надёжный запасной вариант на случай блокировки буфера обмена
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
									break; // Открываем только одно окно проводника
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
			ShowNewScriptDialog("Assets", scriptType);
		}

		private void AddSceneToRoot_Click(object sender, System.Windows.RoutedEventArgs e) => AddAssetToRoot(".zs", "New Scene");
		private void AddViewToRoot_Click(object sender, System.Windows.RoutedEventArgs e) => AddAssetToRoot(".zv", "New View");

		private void AddAssetToRoot(string extension, string baseName)
		{
			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
			if (mainVm != null && mainAssetsVm != null)
			{
				string fileName = baseName + extension;
				int index = 1;
				var siblings = mainAssetsVm.AssetRootNodes.ToList();
				while (siblings.Any(c => c.Name.Equals(fileName, StringComparison.OrdinalIgnoreCase)))
				{
					fileName = $"{baseName} {index++}{extension}";
				}

				string newRelPath = $"Assets/{fileName}";

				var newNode = new ProjectNode
				{
					Name = fileName,
					RelativePath = newRelPath,
					IsFolder = false,
					IsEditing = true,
					PendingAssetExtension = extension
				};
				_pendingNewNodes.Add(newNode);

				mainAssetsVm.AssetRootNodes.Add(newNode);
			}
		}

		private void NodeEditTextBox_Loaded(object sender, System.Windows.RoutedEventArgs e)
		{
			if (sender is TextBox textBox && textBox.DataContext is ProjectNode node)
			{
				textBox.Focus();
				textBox.SelectAll();
				UpdateEditValidation(textBox, node);
			}
		}

		private void NodeEditTextBox_TextChanged(object sender, TextChangedEventArgs e)
		{
			if (sender is TextBox textBox && textBox.DataContext is ProjectNode node)
			{
				UpdateEditValidation(textBox, node);
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
					if (GetEditValidationError(node, textBox.Text) != null)
					{
						UpdateEditValidation(textBox, node);
						return;
					}
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
				RefreshTree();
				return;
			}

			// Невалидные символы и (для скриптов) некорректный C++-идентификатор проверяет
			// GetEditValidationError ниже - здесь их не дублируем.
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

			string? validationError = GetEditValidationError(node, newName);
			if (validationError != null)
			{
				EditorLogger.LogError(validationError);
				if (isPendingNew) _pendingNewNodes.Remove(node);
				RefreshTree();
				return;
			}

			// Новый ещё не созданный на диске скрипт тоже имеет IsScript=true (см. AddScript_Click/
			// AddScriptToRoot_Click), но у него нет HasHpp/HasCpp/HasMeta - значит это не переименование,
			// а первое сохранение имени, и его нужно обработать в ветке isPendingNew ниже.
			if (node.IsScript && !isPendingNew)
			{
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
				_pendingNewNodes.Remove(node);

				editor.Services.Project.Infrastructure.UndoRedo.IAssetsTreeCommand cmd;
				if (!string.IsNullOrEmpty(node.PendingAssetExtension))
				{
					cmd = new editor.Services.Project.Infrastructure.UndoRedo.CreateAssetCommand(
						newRelPath,
						projectRoot,
						node.PendingAssetExtension,
						App.ProjectService.Storage
					);
				}
				else
				{
					cmd = new editor.Services.Project.Infrastructure.UndoRedo.CreateFolderCommand(
						newRelPath,
						projectRoot,
						isSystemMode,
						App.ProjectService.Storage
					);
				}

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
										if (menuItem.Name == "OpenInVsMenuItem")
										{
											menuItem.Visibility = (!node.IsFolder && node.IsScript) ? System.Windows.Visibility.Visible : System.Windows.Visibility.Collapsed;
											menuItem.Click -= OpenInVs_Click;
											menuItem.Click += OpenInVs_Click;
										}
										else if (menuItem.Name == "DeleteMenuItem")
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
													// Пункт скрипта помечен Tag (см. XAML: AddScriptMenuItem) - базовый тип
													// (Script/Game/Scene) выбирается внутри NewScriptDialog, пункт папки - без Tag.
													if (subMenuItem.Tag is string)
													{
														subMenuItem.Click -= AddScript_Click;
														subMenuItem.Click += AddScript_Click;
													}
													else if (subMenuItem.Name == "AddSceneMenuItem")
													{
														subMenuItem.Click -= AddScene_Click;
														subMenuItem.Click += AddScene_Click;
													}
													else if (subMenuItem.Name == "AddViewMenuItem")
													{
														subMenuItem.Click -= AddView_Click;
														subMenuItem.Click += AddView_Click;
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

					// Клик по пустому месту
					var treeControl = FindVisualParent<TreeView>(depObj);
					if (treeControl == SystemTree)
					{
						e.Handled = true; // Блокируем ContextMenu на пустом месте в SystemTree
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

		private void OpenInVs_Click(object sender, System.Windows.RoutedEventArgs e)
		{
			if (sender is MenuItem menuItem)
			{
				var node = GetSelectedNode(menuItem);
				if (node != null)
				{
					OpenFileInVS(node);
				}
			}
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

							var deleteCmd = new editor.Services.Project.Infrastructure.UndoRedo.DeleteFileOrFolderCommand(
								physicalPaths,
								projectRoot,
								node.IsFolder,
								App.ProjectService.Storage,
								onScriptChanged: TriggerScriptRebuild,
								containsScripts: containsScripts
							);

							// Удаляемый скрипт (или скрипты внутри удаляемой папки) может быть отмечен как
							// глобальный в game_config.toml - без этого шага GlobalScriptGuids оставался бы
							// с "мёртвым" GUID (см. docs/resources.md "Game config references"). Список читаем
							// заново с диска, а не из App.ProjectService.CurrentGameConfig: этот кэш не
							// обновляется, когда правки вносятся через инспектор (InspectorViewModel держит
							// свою отдельную десериализованную копию game_config.toml и пишет сразу на диск,
							// минуя CurrentGameConfig) - иначе удаление не находило бы только что добавленный
							// туда GUID.
							string gameConfigPath = System.IO.Path.Combine(projectRoot, editor.Services.Project.ProjectConstants.SystemDirectories.GameConfigs);
							var currentGameConfig = App.ProjectService.Storage.FileExists(gameConfigPath)
								? editor.Services.Project.FileTypes.GameConfig.GameConfigParser.Deserialize(App.ProjectService.Storage.ReadAllText(gameConfigPath))
								: App.ProjectService.CurrentGameConfig;
							var oldGuids = new List<string>(currentGameConfig.GlobalScriptGuids);
							var affectedGuids = CollectScriptGuidsUnderNode(node)
								.Where(guid => oldGuids.Contains(guid))
								.ToList();

							editor.Services.Project.Infrastructure.UndoRedo.ICommand cmd = deleteCmd;
							if (affectedGuids.Count > 0)
							{
								var newGuids = oldGuids.Where(guid => !affectedGuids.Contains(guid)).ToList();
								var guidCmd = new editor.Services.Project.Infrastructure.UndoRedo.PropertyChangeCommand<List<string>>(
									setValueDirectly: v => App.ProjectService.CurrentGameConfig.GlobalScriptGuids = v,
									oldValue: oldGuids,
									newValue: newGuids,
									onChanged: SaveGameConfig);

								cmd = new editor.Services.Project.Infrastructure.UndoRedo.CompositeCommand(
									new List<editor.Services.Project.Infrastructure.UndoRedo.ICommand> { deleteCmd, guidCmd });
							}

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

		// Собирает GUID скриптов, лежащих под удаляемым узлом (сам узел - для одиночного скрипта,
		// либо все скрипты внутри - для папки), по уже построенному ScriptAssetIndexService -
		// единому источнику GUID в редакторе (см. docs/resources.md "ScriptAssetIndexService"),
		// а не через повторное чтение .meta с диска.
		private static List<string> CollectScriptGuidsUnderNode(ProjectNode node)
		{
			string nodePath = node.RelativePath;
			var guids = new List<string>();
			foreach (var info in App.ScriptAssetIndexService.ByGuid.Values)
			{
				bool matches = node.IsFolder
					? info.HppPath.StartsWith(nodePath + "/", StringComparison.OrdinalIgnoreCase)
					: string.Equals(info.HppPath, nodePath, StringComparison.OrdinalIgnoreCase);
				if (matches)
				{
					guids.Add(info.Guid);
				}
			}
			return guids;
		}

		private void SaveGameConfig()
		{
			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			string? projectRoot = mainVm?.CurrentProjectPath;
			if (string.IsNullOrEmpty(projectRoot))
			{
				return;
			}

			string configPath = System.IO.Path.Combine(projectRoot, editor.Services.Project.ProjectConstants.SystemDirectories.GameConfigs);
			string content = editor.Services.Project.FileTypes.GameConfig.GameConfigParser.Serialize(App.ProjectService.CurrentGameConfig);
			App.ProjectService.Storage.WriteAllText(configPath, content);
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
			App.ScriptAssetIndexService.Rebuild(false);
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
			
			// Игнорируем клики по стрелочке разворачивания (ToggleButton)
			if (e.OriginalSource is System.Windows.DependencyObject source && FindVisualParent<System.Windows.Controls.Primitives.ToggleButton>(source) != null)
			{
				_dragCandidateNode = null;
				return;
			}

			var node = item?.DataContext as ProjectNode;
			_dragCandidateNode = (node != null && !node.IsEditing) ? node : null;
			_dragStartPoint = e.GetPosition(null);

			if (item != null && !item.IsSelected)
			{
				// Откладываем выделение до MouseUp, чтобы при перетаскивании (Drag&Drop) 
				// не переключался Inspector на перетаскиваемый узел сразу.
				e.Handled = true;
			}
		}

		private void AssetsTree_PreviewMouseLeftButtonUp(object sender, System.Windows.Input.MouseButtonEventArgs e)
		{
			if (_dragCandidateNode == null)
				return;

			var item = FindVisualParent<TreeViewItem>(e.OriginalSource as System.Windows.DependencyObject);
			if (item != null && item.DataContext == _dragCandidateNode)
			{
				if (!item.IsSelected)
				{
					item.IsSelected = true;
				}
				item.Focus();
			}

			_dragCandidateNode = null;
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
				var tb = FindVisualChildren<TextBlock>(_dropHighlightItem).FirstOrDefault();
				if (tb != null)
				{
					tb.ClearValue(System.Windows.Controls.TextBlock.ForegroundProperty);
				}
				else
				{
					_dropHighlightItem.ClearValue(System.Windows.Controls.Control.BackgroundProperty);
				}
			}

			_dropHighlightItem = item;

			if (_dropHighlightItem != null)
			{
				var tb = FindVisualChildren<TextBlock>(_dropHighlightItem).FirstOrDefault();
				if (tb != null)
				{
					tb.Foreground = (System.Windows.Media.Brush)System.Windows.Application.Current.FindResource("Brush_Accent");
				}
				else
				{
					_dropHighlightItem.Background = (System.Windows.Media.Brush)System.Windows.Application.Current.FindResource("Brush_Accent");
				}
			}
		}

		// Физически переносит узел на диске в папку targetFolderPath, с фиксацией в Undo/Redo
		// через тот же MoveOrRenameCommand, что используется при переименовании.
		private void MoveNode(ProjectNode draggedNode, string targetFolderPath)
		{
			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
			string? projectRoot = mainVm?.CurrentProjectPath;
			if (mainVm == null || mainAssetsVm == null || string.IsNullOrEmpty(projectRoot)) return;

			bool isSystemMode = mainAssetsVm.IsSystemNode(draggedNode);
			var activeFilters = App.ProjectService.CurrentSettings.DisabledFilters;

			var physicalPaths = GetPhysicalPaths(draggedNode, projectRoot, isSystemMode, activeFilters);
			var commands = new List<editor.Services.Project.Infrastructure.UndoRedo.ICommand>();

			foreach (string oldPhysPath in physicalPaths)
			{
				string fileName = System.IO.Path.GetFileName(oldPhysPath);
				string newRelPath = $"{targetFolderPath}/{fileName}";
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

				commands.Add(new editor.Services.Project.Infrastructure.UndoRedo.MoveOrRenameCommand(
					oldPhysPath, 
					newPhysPath, 
					App.ProjectService.Storage, 
					onScriptChanged: TriggerScriptRebuild));
			}

			if (commands.Count == 0) return;

			var compositeCmd = new editor.Services.Project.Infrastructure.UndoRedo.CompositeCommand(commands);

			try
			{
				App.ProjectService.History.Execute(compositeCmd);
				mainVm.RefreshDirtyState();
			}
			catch (Exception ex)
			{
				EditorLogger.LogError(string.Format(Loc("Error_Rename_Failed", "Failed to move: {0}"), ex.Message));
			}

			RefreshTree();
		}

		private void NodeText_PreviewMouseLeftButtonDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
		{
			if (sender is FrameworkElement element && element.DataContext is ProjectNode node)
			{
				if (e.ClickCount == 2)
				{
					if (node.RelativePath.EndsWith(".zv", StringComparison.OrdinalIgnoreCase))
					{
						var info = App.ScriptAssetIndexService.ByGuid.Values.FirstOrDefault(i => string.Equals(i.AssetPath, node.RelativePath, StringComparison.OrdinalIgnoreCase));
						if (info != null)
						{
							App.ProjectService.CurrentSettings.ActiveViewGuid = info.Guid;
							EditorLogger.LogInfo($"[Editor] Активный вид изменен: {System.IO.Path.GetFileName(node.RelativePath)}");
						}
						e.Handled = true;
						return;
					}
					else if (node.RelativePath.EndsWith(".zs", StringComparison.OrdinalIgnoreCase))
					{
						var sceneInfo = App.ScriptAssetIndexService.ByGuid.Values.FirstOrDefault(i => string.Equals(i.AssetPath, node.RelativePath, StringComparison.OrdinalIgnoreCase));
						if (sceneInfo != null)
						{
							string viewGuid = App.ProjectService.CurrentSettings.ActiveViewGuid;
							if (string.IsNullOrEmpty(viewGuid))
							{
								viewGuid = App.ProjectService.CurrentGameConfig.ViewGuids.FirstOrDefault() ?? string.Empty;
							}

							if (!string.IsNullOrEmpty(viewGuid) && App.ScriptAssetIndexService.TryGetByGuid(viewGuid, out var viewInfo))
							{
								var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
								string? projectRoot = mainVm?.CurrentProjectPath;
								if (!string.IsNullOrEmpty(projectRoot))
								{
									string viewPath = System.IO.Path.Combine(projectRoot, viewInfo.AssetPath);
									if (App.ProjectService.Storage.FileExists(viewPath))
									{
										var toml = App.ProjectService.Storage.ReadAllText(viewPath);
										var viewData = editor.Services.Project.FileTypes.Assets.ViewAssetParser.Deserialize(toml);
										viewData.SceneGuid = sceneInfo.Guid;
										App.ProjectService.Storage.WriteAllText(viewPath, editor.Services.Project.FileTypes.Assets.ViewAssetParser.Serialize(viewData));
										EditorLogger.LogInfo($"[Editor] Сцена '{System.IO.Path.GetFileName(node.RelativePath)}' привязана к виду '{System.IO.Path.GetFileName(viewInfo.AssetPath)}'");
									}
								}
							}
							else
							{
								EditorLogger.LogWarning("[Editor] Нет активного вида для привязки сцены.");
							}
						}
						e.Handled = true;
						return;
					}

					OpenFileInVS(node);
					e.Handled = true;
					return;
				}

				if (sender is TextBlock && App.SelectionService.SelectedItem == node && !node.IsScript)
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
				var paths = new List<string>();

				if (node.HasHpp && !string.IsNullOrEmpty(node.HppRelativePath))
				{
					string path = System.IO.Path.Combine(projectRoot, node.HppRelativePath);
					if (System.IO.File.Exists(path)) paths.Add(path);
				}
				if (node.HasCpp && !string.IsNullOrEmpty(node.CppRelativePath))
				{
					string path = System.IO.Path.Combine(projectRoot, node.CppRelativePath);
					if (System.IO.File.Exists(path)) paths.Add(path);
				}

				// Открываем hpp и cpp одним вызовом devenv — два отдельных Process.Start подряд
				// запускали два разных экземпляра Visual Studio (второй стартует раньше, чем
				// первый успевает зарегистрироваться как "запущенный" для этого решения).
				if (paths.Count > 0)
				{
					LaunchFiles(paths.ToArray());
				}
			}
			else
			{
				string path = System.IO.Path.Combine(projectRoot, node.RelativePath);
				if (System.IO.File.Exists(path))
				{
					LaunchFiles(path);
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
					LaunchFiles(path);
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
					LaunchFiles(path);
				}
			}
		}

		private static string? _cachedDevenvPath;
		private static bool _devenvPathResolved;

		// devenv.exe не зарегистрирован в PATH, поэтому ищем его через vswhere —
		// стандартный инструмент, который ставится вместе с любой версией Visual Studio.
		private static string? ResolveDevenvPath()
		{
			if (_devenvPathResolved) return _cachedDevenvPath;
			_devenvPathResolved = true;

			try
			{
				string vswherePath = System.IO.Path.Combine(
					Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86),
					"Microsoft Visual Studio", "Installer", "vswhere.exe");



				if (!System.IO.File.Exists(vswherePath))
				{

					return null;
				}

				var psi = new System.Diagnostics.ProcessStartInfo
				{
					FileName = vswherePath,
					Arguments = "-latest -prerelease -property productPath",
					RedirectStandardOutput = true,
					RedirectStandardError = true,
					UseShellExecute = false,
					CreateNoWindow = true
				};

				using var process = System.Diagnostics.Process.Start(psi);
				string output = process!.StandardOutput.ReadToEnd().Trim();
				string error = process.StandardError.ReadToEnd().Trim();
				process.WaitForExit(5000);



				if (!string.IsNullOrEmpty(output) && System.IO.File.Exists(output))
				{
					_cachedDevenvPath = output;
				}
				else
				{

				}
			}
			catch (Exception ex)
			{
				// молча игнорируем
			}

			return _cachedDevenvPath;
		}

		private void AddScene_Click(object sender, System.Windows.RoutedEventArgs e) => AddAsset_Click(sender, ".zs", "New Scene");
		private void AddView_Click(object sender, System.Windows.RoutedEventArgs e) => AddAsset_Click(sender, ".zv", "New View");

		private void AddAsset_Click(object sender, string extension, string baseName)
		{
			if (sender is not MenuItem menuItem) return;
			var clickedNode = GetSelectedNode(menuItem);
			if (clickedNode == null) return;

			CommitActiveEditIfAny();

			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
			if (mainVm == null || mainAssetsVm == null) return;

			string targetRelativePath = clickedNode.RelativePath;
			var parentNode = FindNodeByPath(mainAssetsVm.AssetRootNodes, targetRelativePath);
			var ownerRoots = mainAssetsVm.AssetRootNodes;

			if (parentNode == null)
			{
				parentNode = FindNodeByPath(mainAssetsVm.SystemRootNodes, targetRelativePath);
				ownerRoots = mainAssetsVm.SystemRootNodes;
			}
			if (parentNode == null) return;

			if (!parentNode.IsFolder)
			{
				string parentRelPath = GetParentRelativePath(parentNode.RelativePath);
				parentNode = string.IsNullOrEmpty(parentRelPath) ? null : FindNodeByPath(ownerRoots, parentRelPath);
			}

			var childrenCollection = parentNode != null ? parentNode.Children : ownerRoots;
			string parentPrefix = parentNode != null ? parentNode.RelativePath + "/" : "Assets/";

			string fileName = baseName + extension;
			int index = 1;
			while (childrenCollection.Any(c => c.Name.Equals(fileName, StringComparison.OrdinalIgnoreCase)))
			{
				fileName = $"{baseName} {index++}{extension}";
			}

			string newRelPath = parentPrefix + fileName;

			var newNode = new ProjectNode
			{
				Name = fileName,
				RelativePath = newRelPath,
				IsFolder = false,
				IsEditing = true,
				PendingAssetExtension = extension
			};
			_pendingNewNodes.Add(newNode);

			childrenCollection.Add(newNode);
		}

		public static string GetExactPathName(string pathName)
		{
			if (!System.IO.File.Exists(pathName) && !System.IO.Directory.Exists(pathName))
				return pathName;

			var di = new System.IO.DirectoryInfo(pathName);
			if (di.Parent != null)
			{
				return System.IO.Path.Combine(
					GetExactPathName(di.Parent.FullName), 
					di.Parent.GetFileSystemInfos(di.Name)[0].Name);
			}
			else
			{
				return di.Name.ToUpper();
			}
		}

		private void LaunchFiles(params string[] paths)
		{
			// Нормализуем пути (слэши и регистр букв), чтобы VS не дублировала вкладки
			paths = paths.Select(p => GetExactPathName(System.IO.Path.GetFullPath(p))).ToArray();

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
						string devenvPath = ResolveDevenvPath() ?? "devenv";

						// Пытаемся открыть файлы через COM-интерфейс, если VS с этим решением уже запущена.
						// Это гарантирует, что вкладки не будут дублироваться и не откроется новая копия VS.
						bool openedViaCom = editor.Services.VisualStudioInterop.OpenFileInSolution(slnPath, paths);
						
						if (!openedViaCom)
						{
							var psi = new System.Diagnostics.ProcessStartInfo
							{
								FileName = devenvPath,
								Arguments = $"\"{slnPath}\"",
								UseShellExecute = true
							};
							System.Diagnostics.Process.Start(psi);

							// Запускаем фоновую задачу ожидания запуска студии
							System.Threading.Tasks.Task.Run(async () =>
							{
								for (int i = 0; i < 30; i++) // Ждем до 15 секунд
								{
									await System.Threading.Tasks.Task.Delay(500);
									if (editor.Services.VisualStudioInterop.OpenFileInSolution(slnPath, paths))
									{
										break;
									}
								}
							});
						}

						return;
					}
				}


				foreach (string path in paths)
				{
					var psiFallback = new System.Diagnostics.ProcessStartInfo
					{
						FileName = path,
						UseShellExecute = true
					};
					System.Diagnostics.Process.Start(psiFallback);
				}
			}
			catch (Exception ex)
			{
				// молча игнорируем
			}
		}

		private void AddScript_Click(object sender, System.Windows.RoutedEventArgs e)
		{
			if (sender is not MenuItem menuItem) return;

			string scriptType = menuItem.Tag as string ?? "Script";
			var clickedNode = GetSelectedNode(menuItem);
			if (clickedNode == null) return;

			string targetRelativePath = clickedNode.RelativePath;
			CommitActiveEditIfAny();

			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
			if (mainVm == null || mainAssetsVm == null) return;

			var node = FindNodeByPath(mainAssetsVm.AssetRootNodes, targetRelativePath);
			var ownerRoots = mainAssetsVm.AssetRootNodes;
			if (node == null)
			{
				node = FindNodeByPath(mainAssetsVm.SystemRootNodes, targetRelativePath);
				ownerRoots = mainAssetsVm.SystemRootNodes;
			}
			if (node == null) return;

			string parentRelativePath;
			if (node.IsFolder)
			{
				parentRelativePath = node.RelativePath;
			}
			else
			{
				FindParentNode(ownerRoots, node, out parentRelativePath);
			}

			ShowNewScriptDialog(parentRelativePath, scriptType);
		}

		private void ShowNewScriptDialog(string parentRelativePath, string scriptType)
		{
			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			string? projectRoot = mainVm?.CurrentProjectPath;
			if (mainVm == null || string.IsNullOrEmpty(projectRoot)) return;

			var dialog = new editor.Views.NewScriptDialog(
				System.Windows.Window.GetWindow(this),
				projectRoot,
				parentRelativePath,
				scriptType,
				mainVm.RecentScriptNamespaces);

			bool? result = dialog.ShowDialog();
			foreach (string removedNamespace in dialog.RemovedNamespaces)
			{
				mainVm.RemoveRecentScriptNamespace(removedNamespace);
			}

			if (result != true)
			{
				return;
			}

			string newRelPath = string.IsNullOrEmpty(parentRelativePath)
				? dialog.ScriptName
				: $"{parentRelativePath}/{dialog.ScriptName}";

			var scriptCmd = new editor.Services.Project.Infrastructure.UndoRedo.CreateScriptCommand(
				newRelPath,
				dialog.ScriptName,
				dialog.ScriptType,
				dialog.ScriptNamespace,
				GetTemplatesDirectory(),
				projectRoot,
				App.ProjectService.Storage,
				onScriptChanged: TriggerScriptRebuild
			);

			try
			{
				App.ProjectService.History.Execute(scriptCmd);
				if (!string.IsNullOrWhiteSpace(dialog.ScriptNamespace))
				{
					mainVm.AddRecentScriptNamespace(dialog.ScriptNamespace);
				}
				EditorLogger.LogInfo(string.Format(Loc("ScriptCreate_Success", "Successfully created script '{0}' from '{1}' template."), dialog.ScriptName, dialog.ScriptType));
				mainVm.RefreshDirtyState();
			}
			catch (Exception ex)
			{
				EditorLogger.LogError(string.Format(Loc("Error_CreateScript_Failed", "Failed to create script: {0}"), ex.Message));
			}

			RefreshTree();
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

			return localPath; // запасной вариант
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

		private void UpdateEditValidation(TextBox textBox, ProjectNode node)
		{
			string? error = GetEditValidationError(node, textBox.Text);
			if (error == null)
			{
				textBox.ClearValue(Control.BorderBrushProperty);
				textBox.ToolTip = null;
				return;
			}

			textBox.BorderBrush = System.Windows.Media.Brushes.IndianRed;
			textBox.ToolTip = error;
		}

		private string? GetEditValidationError(ProjectNode node, string? rawName)
		{
			string newName = rawName?.Trim() ?? string.Empty;
			if (string.IsNullOrEmpty(newName))
			{
				return Loc("Validation_Name_Empty", "Name cannot be empty.");
			}

			if (newName.IndexOfAny(System.IO.Path.GetInvalidFileNameChars()) >= 0)
			{
				return Loc("Validation_Name_InvalidChars", "Name contains invalid characters.");
			}

			if (node.IsScript && !CppIdentifierValidation.IsValidIdentifier(newName))
			{
				return Loc("Validation_ScriptName_InvalidIdentifier", "Script name must be a valid C++ class name.");
			}

			bool isPendingNew = _pendingNewNodes.Contains(node);
			if (node.IsScript && !isPendingNew && !newName.Equals(node.Name, StringComparison.OrdinalIgnoreCase))
			{
				return Loc("Validation_ScriptRename_Disabled", "Script names can only be set when the script is created.");
			}

			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			string? projectRoot = mainVm?.CurrentProjectPath;
			var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
			if (string.IsNullOrEmpty(projectRoot) || mainAssetsVm == null)
			{
				return null;
			}

			bool isSystemMode = mainAssetsVm.IsSystemNode(node);
			string parentPath = GetParentRelativePath(node.RelativePath);
			string targetRelPath = string.IsNullOrEmpty(parentPath) ? newName : $"{parentPath}/{newName}";
			string targetBasePath = System.IO.Path.Combine(projectRoot, targetRelPath.Replace('/', System.IO.Path.DirectorySeparatorChar));

			if (node.IsScript)
			{
				if (ScriptFileExists(targetBasePath, ".hpp", node.HppRelativePath, projectRoot) ||
					ScriptFileExists(targetBasePath, ".cpp", node.CppRelativePath, projectRoot) ||
					ScriptFileExists(targetBasePath, ".meta", node.MetaRelativePath, projectRoot))
				{
					return Loc("Validation_FileName_Exists", "A file with this name already exists.");
				}
				return null;
			}

			string actualNewName = node.IsFolder ? newName : newName + System.IO.Path.GetExtension(node.Name);
			string actualTargetRelPath = string.IsNullOrEmpty(parentPath) ? actualNewName : $"{parentPath}/{actualNewName}";
			string targetPath = System.IO.Path.Combine(projectRoot, actualTargetRelPath.Replace('/', System.IO.Path.DirectorySeparatorChar));

			if (node.IsFolder)
			{
				string currentPath = System.IO.Path.Combine(projectRoot, node.RelativePath.Replace('/', System.IO.Path.DirectorySeparatorChar));
				if (!PathsEqual(targetPath, currentPath) && System.IO.Directory.Exists(targetPath))
				{
					return Loc("Validation_FolderName_Exists", "A folder with this name already exists.");
				}
			}
			else
			{
				var currentPaths = GetPhysicalPaths(node, projectRoot, isSystemMode, App.ProjectService.CurrentSettings.DisabledFilters);
				if (!currentPaths.Any(path => PathsEqual(path, targetPath)) && System.IO.File.Exists(targetPath))
				{
					return Loc("Validation_FileName_Exists", "A file with this name already exists.");
				}
			}

			return null;
		}

		private static bool ScriptFileExists(string targetBasePath, string extension, string currentRelativePath, string projectRoot)
		{
			string targetPath = targetBasePath + extension;
			if (!System.IO.File.Exists(targetPath))
			{
				return false;
			}

			if (string.IsNullOrEmpty(currentRelativePath))
			{
				return true;
			}

			string currentPath = System.IO.Path.Combine(projectRoot, currentRelativePath.Replace('/', System.IO.Path.DirectorySeparatorChar));
			return !PathsEqual(targetPath, currentPath);
		}

		private static bool PathsEqual(string left, string right)
		{
			return string.Equals(
				System.IO.Path.GetFullPath(left).TrimEnd(System.IO.Path.DirectorySeparatorChar, System.IO.Path.AltDirectorySeparatorChar),
				System.IO.Path.GetFullPath(right).TrimEnd(System.IO.Path.DirectorySeparatorChar, System.IO.Path.AltDirectorySeparatorChar),
				StringComparison.OrdinalIgnoreCase);
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
