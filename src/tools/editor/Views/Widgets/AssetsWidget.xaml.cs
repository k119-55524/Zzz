
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
		public AssetsWidget()
		{
			InitializeComponent();
		}

		private void TreeView_SelectedItemChanged(object sender, System.Windows.RoutedPropertyChangedEventArgs<object> e)
		{
			App.SelectionService.SelectedItem = e.NewValue;
		}

		private VirtualNode? GetSelectedNode(MenuItem menuItem)
		{
			return menuItem.DataContext as VirtualNode;
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
						VirtualNode parentNode;
						string parentVirtualPath;

						if (node.IsFolder)
						{
							parentNode = node;
							parentVirtualPath = node.RelativePath;
						}
						else
						{
							parentNode = FindParentNode(mainAssetsVm.VirtualRootNodes, node, out parentVirtualPath);
						}

						string baseName = "Новая папка";
						string folderName = baseName;
						int index = 1;
						var siblings = parentNode != null ? parentNode.Children : mainAssetsVm.VirtualRootNodes.ToList();
						while (siblings.Any(c => c.Name.Equals(folderName, StringComparison.OrdinalIgnoreCase)))
						{
							folderName = $"{baseName} {index++}";
						}

						string newRelPath = string.IsNullOrEmpty(parentVirtualPath) ? folderName : $"{parentVirtualPath}/{folderName}";

						var newFolderNode = new VirtualNode
						{
							Name = folderName,
							RelativePath = newRelPath,
							IsFolder = true,
							IsEditing = true,
							IsEmptyVirtual = true
						};

						if (parentNode != null)
						{
							parentNode.Children.Add(newFolderNode);
						}
						else
						{
							mainAssetsVm.VirtualRootNodes.Add(newFolderNode);
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
				var siblings = mainAssetsVm.VirtualRootNodes.ToList();
				while (siblings.Any(c => c.Name.Equals(folderName, StringComparison.OrdinalIgnoreCase)))
				{
					folderName = $"{baseName} {index++}";
				}

				var newFolderNode = new VirtualNode
				{
					Name = folderName,
					RelativePath = folderName,
					IsFolder = true,
					IsEditing = true,
					IsEmptyVirtual = true
				};

				mainAssetsVm.VirtualRootNodes.Add(newFolderNode);
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
			if (sender is TextBox textBox && textBox.DataContext is VirtualNode node)
			{
				if (node.IsEditing)
				{
					CommitRename(node, textBox.Text);
				}
			}
		}

		private void NodeEditTextBox_KeyDown(object sender, KeyEventArgs e)
		{
			if (sender is TextBox textBox && textBox.DataContext is VirtualNode node)
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

		private void CommitRename(VirtualNode node, string newName)
		{
			node.IsEditing = false;
			newName = newName?.Trim();
			if (string.IsNullOrEmpty(newName))
			{
				RefreshTree();
				return;
			}

			char[] invalidChars = System.IO.Path.GetInvalidFileNameChars();
			if (newName.IndexOfAny(invalidChars) >= 0)
			{
				System.Windows.MessageBox.Show("Имя содержит недопустимые символы.", "Ошибка переименования", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
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

			if (actualNewName.Equals(node.Name, StringComparison.OrdinalIgnoreCase) && node.IsEmptyVirtual == false)
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

			if (node.IsEmptyVirtual && !App.ProjectService.CurrentSettings.EmptyFolders.Contains(node.RelativePath))
			{
				if (!isSystemMode)
				{
					App.ProjectService.CurrentSettings.EmptyFolders.Add(newRelPath);
					App.ProjectService.SaveProject(projectRoot, out _);
				}
				else
				{
					string physPath = System.IO.Path.Combine(projectRoot, newRelPath);
					try
					{
						System.IO.Directory.CreateDirectory(physPath);
					}
					catch (Exception ex)
					{
						System.Windows.MessageBox.Show($"Не удалось создать папку: {ex.Message}", "Ошибка", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
					}
				}
				RefreshTree();
				return;
			}

			if (!isSystemMode && App.ProjectService.CurrentSettings.EmptyFolders.Count > 0)
			{
				var emptyFolders = App.ProjectService.CurrentSettings.EmptyFolders;
				for (int i = 0; i < emptyFolders.Count; i++)
				{
					string path = emptyFolders[i];
					if (path.Equals(node.RelativePath, StringComparison.OrdinalIgnoreCase))
					{
						emptyFolders[i] = newRelPath;
					}
					else if (path.StartsWith(node.RelativePath + "/", StringComparison.OrdinalIgnoreCase))
					{
						emptyFolders[i] = newRelPath + path.Substring(node.RelativePath.Length);
					}
				}
				App.ProjectService.SaveProject(projectRoot, out _);
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

		private VirtualNode FindParentNode(System.Collections.ObjectModel.ObservableCollection<VirtualNode> roots, VirtualNode target, out string parentPath)
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

		private VirtualNode FindParentNodeInternal(VirtualNode current, VirtualNode target, out string parentPath)
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

		private bool IsSystemNode(VirtualNode node, bool isSystemMode)
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

		private List<string> GetPhysicalPaths(VirtualNode node, string projectRoot, bool isSystemMode, List<string> disabledFilters)
		{
			var paths = new List<string>();
			string relPath = node.RelativePath;
			if (string.IsNullOrEmpty(relPath)) return paths;

			if (isSystemMode)
			{
				paths.Add(System.IO.Path.Combine(projectRoot, relPath));
			}
			else
			{
				string assetsRoot = System.IO.Path.Combine(projectRoot, "Assets");
				if (System.IO.Directory.Exists(assetsRoot))
				{
					var resourceDirs = System.IO.Directory.GetDirectories(assetsRoot);
					foreach (var resDir in resourceDirs)
					{
						string resTypeName = System.IO.Path.GetFileName(resDir);
						if (disabledFilters.Contains(resTypeName))
							continue;

						string path = System.IO.Path.Combine(resDir, relPath);
						if (node.IsFolder)
						{
							if (System.IO.Directory.Exists(path))
								paths.Add(path);
						}
						else
						{
							if (System.IO.File.Exists(path))
								paths.Add(path);
						}
					}
				}
			}
			return paths;
		}

		private void TreeView_ContextMenuOpening(object sender, ContextMenuEventArgs e)
		{
			var mainVm = System.Windows.Application.Current?.MainWindow?.DataContext as ViewModels.MainWindowViewModel;
			var mainAssetsVm = mainVm?.Panes.OfType<ViewModels.AssetsViewModel>().FirstOrDefault();
			if (mainAssetsVm != null)
			{
				bool isSystemMode = mainAssetsVm.IsSystemMode;

				if (e.OriginalSource is System.Windows.DependencyObject depObj)
				{
					var item = FindVisualParent<TreeViewItem>(depObj);
					if (item != null)
					{
						if (item.Header is VirtualNode node)
						{
							var contextMenu = item.ContextMenu;
							if (contextMenu != null)
							{
								bool isSystem = IsSystemNode(node, isSystemMode);
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
											menuItem.Visibility = node.IsEmptyVirtual ? System.Windows.Visibility.Collapsed : System.Windows.Visibility.Visible;
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
					if (mainAssetsVm.IsSystemMode)
					{
						e.Handled = true; // Block ContextMenu on empty space in SystemMode
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
							if (!isSystemMode && settings.EmptyFolders.Count > 0)
							{
								var emptyFolders = settings.EmptyFolders;
								for (int i = emptyFolders.Count - 1; i >= 0; i--)
								{
									string path = emptyFolders[i];
									if (path.Equals(node.RelativePath, StringComparison.OrdinalIgnoreCase) ||
										path.StartsWith(node.RelativePath + "/", StringComparison.OrdinalIgnoreCase))
									{
										emptyFolders.RemoveAt(i);
									}
								}
								App.ProjectService.SaveProject(projectRoot, out _);
							}

							var physicalPaths = GetPhysicalPaths(node, projectRoot, isSystemMode, activeFilters);
							foreach (var physPath in physicalPaths)
							{
								try
								{
									if (node.IsFolder)
									{
										if (System.IO.Directory.Exists(physPath))
										{
											System.IO.Directory.Delete(physPath, true);
										}
									}
									else
									{
										if (System.IO.File.Exists(physPath))
										{
											System.IO.File.Delete(physPath);
										}
									}
								}
								catch (Exception ex)
								{
									System.Windows.MessageBox.Show($"Не удалось удалить: {ex.Message}", "Ошибка удаления", System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
								}
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
