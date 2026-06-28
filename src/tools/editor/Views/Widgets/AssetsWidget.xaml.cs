
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Controls;
using System.Windows.Input;
using editor.Services.Project.Infrastructure;

namespace editor.Views.Widgets
{
	public partial class AssetsWidget : UserControl
	{
		// Узлы, добавленные через "Добавить -> Папку" и еще не подтвержденные вводом имени.
		private readonly HashSet<ProjectNode> _pendingNewNodes = new();

		public AssetsWidget()
		{
			InitializeComponent();
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
						bool isSystemMode = mainAssetsVm.IsSystemMode;
						var activeFilters = App.ProjectService.CurrentSettings?.DisabledFilters ?? new List<string>();
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
								}
								catch { }
							}
						}
					}
				}
			}
		}

		private void Rename_Click(object sender, System.Windows.RoutedEventArgs e)
		{
			if (sender is MenuItem menuItem)
			{
				var node = GetSelectedNode(menuItem);
				if (node != null)
				{
					node.IsEditing = true;
				}
			}
		}

		private void AddFolder_Click(object sender, System.Windows.RoutedEventArgs e)
		{
			if (sender is MenuItem menuItem)
			{
				var node = GetSelectedNode(menuItem);
				if (node != null)
				{
					var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
					var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
					if (mainVm != null && mainAssetsVm != null)
					{
						ProjectNode parentNode;
						string parentRelativePath;

						if (node.IsFolder)
						{
							parentNode = node;
							parentRelativePath = node.RelativePath;
						}
						else
						{
							parentNode = FindParentNode(mainAssetsVm.RootNodes, node, out parentRelativePath);
						}

						string baseName = "Новая папка";
						string folderName = baseName;
						int index = 1;
						var siblings = parentNode != null ? parentNode.Children.ToList() : mainAssetsVm.RootNodes.ToList();
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
							mainAssetsVm.RootNodes.Add(newFolderNode);
						}
					}
				}
			}
		}


		private void AddFolderToRoot_Click(object sender, System.Windows.RoutedEventArgs e)
		{
			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
			if (mainVm != null && mainAssetsVm != null)
			{
				string baseName = "Новая папка";
				string folderName = baseName;
				int index = 1;
				var siblings = mainAssetsVm.RootNodes.ToList();
				while (siblings.Any(c => c.Name.Equals(folderName, StringComparison.OrdinalIgnoreCase)))
				{
					folderName = $"{baseName} {index++}";
				}

				bool isSystemMode = mainAssetsVm.IsSystemMode;
				string newRelPath = isSystemMode ? folderName : $"Assets/{folderName}";

				var newFolderNode = new ProjectNode
				{
					Name = folderName,
					RelativePath = newRelPath,
					IsFolder = true,
					IsEditing = true
				};
				_pendingNewNodes.Add(newFolderNode);

				mainAssetsVm.RootNodes.Add(newFolderNode);
			}
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
				RefreshTree();
				return;
			}

			char[] invalidChars = System.IO.Path.GetInvalidFileNameChars();
			if (newName.IndexOfAny(invalidChars) >= 0)
			{
				System.Windows.MessageBox.Show("Имя содержит недопустимые символы.", "Ошибка переименования", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
				if (isPendingNew) _pendingNewNodes.Remove(node);
				RefreshTree();
				return;
			}

			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			string? projectRoot = mainVm?.CurrentProjectPath;
			var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
			if (mainVm == null || string.IsNullOrEmpty(projectRoot) || mainAssetsVm == null || App.ProjectService.CurrentSettings == null)
			{
				RefreshTree();
				return;
			}

			bool isSystemMode = mainAssetsVm.IsSystemMode;
			var activeFilters = App.ProjectService.CurrentSettings.DisabledFilters;

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
					System.Windows.MessageBox.Show($"Не удалось создать папку: {ex.Message}", "Ошибка", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
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
					try
					{
						if (node.IsFolder)
						{
							if (System.IO.Directory.Exists(oldPhysPath))
							{
								System.IO.Directory.Move(oldPhysPath, newPhysPath);
							}
						}
						else
						{
							if (System.IO.File.Exists(oldPhysPath))
							{
								System.IO.File.Move(oldPhysPath, newPhysPath);
							}
						}
					}
					catch (Exception ex)
					{
						System.Windows.MessageBox.Show($"Не удалось переименовать: {ex.Message}", "Ошибка переименования", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
					}
				}
			}

			RefreshTree();
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
				var found = FindParentNodeInternal(root, target, out parentPath);
				if (found != null)
				{
					return found;
				}
			}
			return null;
		}

		private ProjectNode FindParentNodeInternal(ProjectNode current, ProjectNode target, out string parentPath)
		{
			parentPath = "";
			foreach (var child in current.Children)
			{
				if (child.Children.Contains(target))
				{
					parentPath = child.RelativePath;
					return child;
				}
				var found = FindParentNodeInternal(child, target, out parentPath);
				if (found != null)
				{
					return found;
				}
			}
			return null;
		}

		private bool IsSystemNode(ProjectNode node, bool isSystemMode)
		{
			if (isSystemMode)
			{
				string relPath = node.RelativePath.Replace('\\', '/').Trim('/');
				if (relPath.Equals("Configs", StringComparison.OrdinalIgnoreCase) ||
					relPath.StartsWith("Configs/", StringComparison.OrdinalIgnoreCase))
				{
					return true;
				}
			}
			return false;
		}

		private List<string> GetPhysicalPaths(ProjectNode node, string projectRoot, bool isSystemMode, List<string> disabledFilters)
		{
			var paths = new List<string>();
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
										if (menuItem.Name == "RenameMenuItem")
										{
											menuItem.Visibility = isSystem ? System.Windows.Visibility.Collapsed : System.Windows.Visibility.Visible;
											menuItem.Click -= Rename_Click;
											menuItem.Click += Rename_Click;
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
													subMenuItem.Click -= AddFolder_Click;
													subMenuItem.Click += AddFolder_Click;
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
			System.Windows.DependencyObject parentObject = System.Windows.Media.VisualTreeHelper.GetParent(child);
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
					if (mainVm != null && mainAssetsVm != null && settings != null)
					{
						string? projectRoot = mainVm.CurrentProjectPath;
						if (string.IsNullOrEmpty(projectRoot)) return;

						bool isSystemMode = mainAssetsVm.IsSystemMode;
						var activeFilters = settings.DisabledFilters;

						string title = System.Windows.Application.Current?.TryFindResource("Dialog_Delete_Title") as string ?? "Delete Item";
						string confirmFormat = System.Windows.Application.Current?.TryFindResource("Dialog_Delete_Confirm") as string ?? "Are you sure you want to delete '{0}'?";
						string confirmMsg = string.Format(confirmFormat, node.Name);

						var result = System.Windows.MessageBox.Show(confirmMsg, title, System.Windows.MessageBoxButton.YesNo, System.Windows.MessageBoxImage.Warning);
						if (result == System.Windows.MessageBoxResult.Yes)
						{
							var physicalPaths = GetPhysicalPaths(node, projectRoot, isSystemMode, activeFilters);
							var cmd = new editor.Services.Project.Infrastructure.UndoRedo.DeleteFileOrFolderCommand(
								physicalPaths,
								projectRoot,
								node.IsFolder,
								App.ProjectService.Storage
							);

							try
							{
								App.ProjectService.History.Execute(cmd);
								if (mainVm != null) mainVm.RefreshDirtyState();
							}
							catch (Exception ex)
							{
								System.Windows.MessageBox.Show($"Не удалось выполнить удаление: {ex.Message}", "Ошибка удаления", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
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
	}
}
