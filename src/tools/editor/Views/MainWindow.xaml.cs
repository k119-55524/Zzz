
using System;
using System.Linq;
using System.Windows;
using editor.Services;
using editor.ViewModels;
using AvalonDock.Layout;
using editor.Views.Widgets;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;
using System.Windows.Controls;
using System.Windows.Threading;

namespace editor
{
	public partial class MainWindow : Window
	{
		private readonly MainWindowViewModel _viewModel;
		private string _defaultLayoutXml = "";

		static MainWindow()
		{
			EventManager.RegisterClassHandler(
				typeof(AvalonDock.Controls.LayoutAnchorablePaneControl),
				LoadedEvent,
				new RoutedEventHandler(OnAnchorablePaneControlLoaded));

			EventManager.RegisterClassHandler(
				typeof(AvalonDock.Controls.LayoutAnchorableFloatingWindowControl),
				LoadedEvent,
				new RoutedEventHandler(OnFloatingWindowLoaded));
		}

		private static void OnFloatingWindowLoaded(object sender, RoutedEventArgs e)
		{
			if (sender is Window window)
			{
				MainWindow? mainWin = null;
				foreach (Window w in Application.Current.Windows)
				{
					if (w is MainWindow mw)
					{
						mainWin = mw;
						break;
					}
				}

				if (mainWin != null)
				{
					var style = mainWin.DockManager.FindResource(typeof(AvalonDock.Controls.AnchorablePaneTitle));
					if (style != null)
					{
						window.Resources[typeof(AvalonDock.Controls.AnchorablePaneTitle)] = style;
					}
					var paneStyle = mainWin.DockManager.FindResource("CompactAnchorablePaneStyle");
					if (paneStyle != null)
					{
						window.Resources["CompactAnchorablePaneStyle"] = paneStyle;
					}
				}

				// Жёлтая окантовка в 1 пиксель вокруг отсоединенного (плавающего) окна
				window.BorderBrush = new SolidColorBrush((Color)ColorConverter.ConvertFromString("#E2C08D"));
				window.BorderThickness = new Thickness(1);
				window.Padding = new Thickness(2);

				Application.Current.Dispatcher.BeginInvoke(new System.Action(() =>
				{
					var button = FindVisualChild<FrameworkElement>(window, "SinglePaneContextMenu");
					if (button != null)
					{
						button.Visibility = Visibility.Collapsed;
						button.Width = 0;
						button.Height = 0;
					}
				}), System.Windows.Threading.DispatcherPriority.Background);
			}
		}

		private static T? FindVisualChild<T>(DependencyObject obj, string name) where T : DependencyObject
		{
			for (int i = 0; i < VisualTreeHelper.GetChildrenCount(obj); i++)
			{
				var child = VisualTreeHelper.GetChild(obj, i);
				if (child is T t && (child is FrameworkElement fe && fe.Name == name))
				{
					return t;
				}
				var descendant = FindVisualChild<T>(child, name);
				if (descendant != null)
				{
					return descendant;
				}
			}
			return null;
		}

		private static void OnAnchorablePaneControlLoaded(object sender, RoutedEventArgs e)
		{
			var tabControl = (TabControl)sender;
			tabControl.Style = (Style)tabControl.FindResource("CompactAnchorablePaneStyle");
			var scheduler = new OutlineUpdateScheduler(tabControl);
			tabControl.SelectionChanged += (s, _) => scheduler.Schedule();
			tabControl.LayoutUpdated += (s, _) => scheduler.Schedule();
			tabControl.Loaded += (s, _) => scheduler.Schedule();
			scheduler.Schedule();
		}

		private sealed class OutlineUpdateScheduler
		{
			private const int MaxConsecutiveRetries = 5;
			private readonly TabControl _tabControl;
			private bool _isPending;
			private int _retryCount;

			public OutlineUpdateScheduler(TabControl tabControl)
			{
				_tabControl = tabControl;
			}

			public void Schedule()
			{
				_retryCount = 0;
				ScheduleCore();
			}

			public void ScheduleRetry()
			{
				if (_retryCount >= MaxConsecutiveRetries)
				{
					return;
				}

				_retryCount++;
				ScheduleCore();
			}

			private void ScheduleCore()
			{
				if (_isPending)
				{
					return;
				}

				_isPending = true;
				_tabControl.Dispatcher.BeginInvoke(DispatcherPriority.Loaded, new Action(() =>
				{
					_isPending = false;
					UpdateTabSelectionOutline(_tabControl, ScheduleRetry);
				}));
			}
		}

		private static void UpdateTabSelectionOutline(TabControl tabControl, Action retryScheduler)
		{
			if (tabControl.Template == null)
			{
				return;
			}

			var rootGrid = tabControl.Template.FindName("RootGrid", tabControl) as FrameworkElement;
			var contentPanel = tabControl.Template.FindName("ContentPanel", tabControl) as FrameworkElement;
			var outline = tabControl.Template.FindName("SelectionOutline", tabControl) as Path;

			if (rootGrid == null || contentPanel == null || outline == null)
			{
				return;
			}

			var selectedTab = tabControl.ItemContainerGenerator.ContainerFromItem(tabControl.SelectedItem) as TabItem;
			if (selectedTab == null || selectedTab.ActualWidth <= 0 || contentPanel.ActualWidth <= 0)
			{
				outline.Data = null;
				retryScheduler();
				return;
			}

			const double radius = 5;

			Rect tabBounds = new Rect(selectedTab.TranslatePoint(new Point(0, 0), rootGrid), new Size(selectedTab.ActualWidth, selectedTab.ActualHeight));
			Rect contentBounds = new Rect(contentPanel.TranslatePoint(new Point(0, 0), rootGrid), new Size(contentPanel.ActualWidth, contentPanel.ActualHeight));
			if (outline.Tag is (Rect cachedTabBounds, Rect cachedContentBounds)
				&& cachedTabBounds == tabBounds && cachedContentBounds == contentBounds)
			{
				return;
			}

			outline.Tag = (tabBounds, contentBounds);

			double tabTop = tabBounds.Top + 0.5;
			double tabLeft = tabBounds.Left + 0.5;
			double tabRight = tabBounds.Right - 0.5;
			double seamY = contentBounds.Top + 0.5;
			double contentLeft = contentBounds.Left + 0.5;
			double contentRight = contentBounds.Right - 0.5;
			double contentBottom = contentBounds.Bottom - 0.5;

			var figure = new PathFigure { StartPoint = new Point(tabLeft, seamY), IsClosed = true, IsFilled = false };

			// вверх по левому краю закладки, с закруглением в верхнем левом углу
			figure.Segments.Add(new LineSegment(new Point(tabLeft, tabTop + radius), true));
			figure.Segments.Add(new ArcSegment(new Point(tabLeft + radius, tabTop), new Size(radius, radius), 0, false, SweepDirection.Clockwise, true));

			// верх закладки
			figure.Segments.Add(new LineSegment(new Point(tabRight - radius, tabTop), true));
			figure.Segments.Add(new ArcSegment(new Point(tabRight, tabTop + radius), new Size(radius, radius), 0, false, SweepDirection.Clockwise, true));

			// вниз по правому краю закладки до стыка с областью контента, затем "ступенька" вправо
			// (если закладка уже, чем область контента — у соседних закладок справа)
			figure.Segments.Add(new LineSegment(new Point(tabRight, seamY), true));
			figure.Segments.Add(new LineSegment(new Point(contentRight, seamY), true));

			// вниз по правому краю контента, с закруглением в нижнем правом углу
			figure.Segments.Add(new LineSegment(new Point(contentRight, contentBottom - radius), true));
			figure.Segments.Add(new ArcSegment(new Point(contentRight - radius, contentBottom), new Size(radius, radius), 0, false, SweepDirection.Clockwise, true));

			// низ контента
			figure.Segments.Add(new LineSegment(new Point(contentLeft + radius, contentBottom), true));
			figure.Segments.Add(new ArcSegment(new Point(contentLeft, contentBottom - radius), new Size(radius, radius), 0, false, SweepDirection.Clockwise, true));

			// вверх по левому краю контента до стыка, затем "ступенька" обратно к левому краю закладки
			figure.Segments.Add(new LineSegment(new Point(contentLeft, seamY), true));
			figure.Segments.Add(new LineSegment(new Point(tabLeft, seamY), true));

			var geometry = new PathGeometry();
			geometry.Figures.Add(figure);
			outline.Data = geometry;
		}

		public MainWindow()
		{
			InitializeComponent();

			_viewModel = new MainWindowViewModel(new DialogService());
			DataContext = _viewModel;

			_viewModel.CloseRequested += (s, e) => Close();
			_viewModel.ResetLayoutRequested += (s, e) =>
			{
				RestoreLayout(_defaultLayoutXml);
			};
			_viewModel.ShowWidgetRequested += (s, pane) => ShowWidget(pane);

			Loaded += MainWindow_Loaded;
			Closing += MainWindow_Closing;
			Closed += MainWindow_Closed;
			Activated += MainWindow_Activated;
		}

		private void MainWindow_Loaded(object sender, RoutedEventArgs e)
		{
			_defaultLayoutXml = AvalonDockLayoutPersistence.Serialize(DockManager);
			_viewModel.LoadSession();
			var layoutState = EditorSessionManager.LoadLayoutSession();
			if (layoutState.Width > 100 && layoutState.Height > 100)
			{
				Width = layoutState.Width;
				Height = layoutState.Height;
				Left = layoutState.Left;
				Top = layoutState.Top;
				if (layoutState.IsMaximized)
				{
					WindowState = WindowState.Maximized;
				}
			}

			RestoreLayout(string.IsNullOrEmpty(layoutState.LayoutXml) ? _defaultLayoutXml : layoutState.LayoutXml);
			DockManager.UpdateLayout();
		}

		private void RestoreLayout(string layoutXml)
		{
			AvalonDockLayoutPersistence.TryDeserialize(DockManager, layoutXml, _viewModel.Panes);
		}

		private void ShowWidget(PaneViewModel pane)
		{
			var anchorable = DockManager.Layout.Descendents()
				.OfType<LayoutAnchorable>()
				.FirstOrDefault(a => ReferenceEquals(a.Content, pane));

			if (anchorable == null)
			{
				return;
			}

			if (anchorable.IsHidden)
			{
				anchorable.Show();
			}

			anchorable.IsActive = true;
		}

		private void DockManager_AnchorableClosing(object sender, AvalonDock.AnchorableClosingEventArgs e)
		{
			e.Cancel = true;
			e.Anchorable.Hide();
		}



		private void MainWindow_Closing(object? sender, System.ComponentModel.CancelEventArgs e)
		{
			if (!_viewModel.RequestClose())
			{
				e.Cancel = true;
				return;
			}

			double saveWidth = Width;
			double saveHeight = Height;
			double saveLeft = Left;
			double saveTop = Top;

			if (WindowState != WindowState.Normal)
			{
				var restoreBounds = RestoreBounds;
				saveWidth = restoreBounds.Width;
				saveHeight = restoreBounds.Height;
				saveLeft = restoreBounds.Left;
				saveTop = restoreBounds.Top;
			}

			var layoutState = new LayoutSessionState
			{
				Width = saveWidth,
				Height = saveHeight,
				Left = saveLeft,
				Top = saveTop,
				IsMaximized = WindowState == WindowState.Maximized,
				IsDirty = false,
				LayoutXml = AvalonDockLayoutPersistence.Serialize(DockManager)
			};
			EditorSessionManager.SaveLayoutSession(layoutState);

			_viewModel.SaveSession();

			App.EngineService.ShutdownEngine();
		}

		private void MainWindow_Closed(object? sender, EventArgs e)
		{
		}

		private void MinimizeWindow_Executed(object sender, ExecutedRoutedEventArgs e) => SystemCommands.MinimizeWindow(this);
		private void MaximizeWindow_Executed(object sender, ExecutedRoutedEventArgs e) => SystemCommands.MaximizeWindow(this);
		private void RestoreWindow_Executed(object sender, ExecutedRoutedEventArgs e) => SystemCommands.RestoreWindow(this);
		private void CloseWindow_Executed(object sender, ExecutedRoutedEventArgs e) => SystemCommands.CloseWindow(this);

		private static T? FindVisualChild<T>(DependencyObject parent) where T : DependencyObject
		{
			for (int i = 0; i < VisualTreeHelper.GetChildrenCount(parent); i++)
			{
				var child = VisualTreeHelper.GetChild(parent, i);
				if (child is T typedChild)
				{
					return typedChild;
				}
				var result = FindVisualChild<T>(child);
				if (result != null)
				{
					return result;
				}
			}
			return null;
		}

		private bool _isCompiling = false;

		private async void MainWindow_Activated(object? sender, EventArgs e)
		{
			string? projectRoot = _viewModel.CurrentProjectPath;
			if (string.IsNullOrEmpty(projectRoot) || !System.IO.Directory.Exists(projectRoot))
				return;

			if (_isCompiling) return;

			// 1. Проверяем изменения в файлах
			string assetsDir = System.IO.Path.Combine(projectRoot, "Assets");
			if (!System.IO.Directory.Exists(assetsDir)) return;

			var scriptFiles = System.IO.Directory.GetFiles(assetsDir, "*.*", System.IO.SearchOption.AllDirectories)
				.Where(f => f.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase) || 
                            f.EndsWith(".cpp", StringComparison.OrdinalIgnoreCase));

			DateTime maxWriteTime = DateTime.MinValue;
			int fileCount = 0;
			foreach (var file in scriptFiles)
			{
				var writeTime = System.IO.File.GetLastWriteTime(file);
				if (writeTime > maxWriteTime)
				{
					maxWriteTime = writeTime;
				}
				fileCount++;
			}

			if (fileCount == 0) return; // нет скриптов для сборки

			string dllPath = System.IO.Path.Combine(projectRoot, "bin", "scripts.dll");
			bool dllExists = System.IO.File.Exists(dllPath);
			DateTime dllWriteTime = dllExists ? System.IO.File.GetLastWriteTime(dllPath) : DateTime.MinValue;

			if (!dllExists || maxWriteTime > dllWriteTime)
			{
				// Требуется пересборка!
				await CompileScriptsAsync(projectRoot, dllPath);
			}
		}

		private async System.Threading.Tasks.Task CompileScriptsAsync(string projectRoot, string dllPath)
		{
			_isCompiling = true;
			EditorLogger.LogInfo("Scripts modification detected. Starting background compilation...");

			try
			{
				// 1. Создаем папку .editor если не существует
				string editorDir = System.IO.Path.Combine(projectRoot, ".editor");
				System.IO.Directory.CreateDirectory(editorDir);

				// 2. Генерируем CMakeLists.txt
				string cmakePath = System.IO.Path.Combine(editorDir, "CMakeLists.txt");
				string engineSourceDir = System.IO.Path.GetFullPath(System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "..", "..", "..")).Replace('\\', '/');
				
				// Определяем конфигурацию сборки
				string config = "Debug";
				#if DEBUG
				config = "Debug";
				#else
				config = "Release";
				#endif

				string editorDllLib = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "editor_dll.lib").Replace('\\', '/');

				string cmakeContent = $@"cmake_minimum_required(VERSION 3.28)
project(project_scripts LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ""${{PROJECT_SOURCE_DIR}}/../bin"")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ""${{PROJECT_SOURCE_DIR}}/../bin"")

add_library(scripts SHARED)

target_include_directories(scripts PRIVATE
    ""{engineSourceDir}/src""
    ""{engineSourceDir}/src/engine""
    ""{engineSourceDir}/src/engine/private/core/scene""
    ""{engineSourceDir}/src/engine/private/core/scene/scripts""
    ""{engineSourceDir}/src/engine/private/core/scene/scripts/base_script""
    ""{engineSourceDir}/src/engine/private/core""
    ""{engineSourceDir}/src/common""
    ""{engineSourceDir}/src/logger""
)

file(GLOB_RECURSIVE SCRIPT_SOURCES ""${{PROJECT_SOURCE_DIR}}/../Assets/*.cpp"")
target_sources(scripts PRIVATE ${{SCRIPT_SOURCES}})

target_compile_definitions(scripts PRIVATE Z_EDITOR=1)

target_link_libraries(scripts PRIVATE ""{editorDllLib}"")
";

				System.IO.File.WriteAllText(cmakePath, cmakeContent);

				// 3. Вызываем CMake конфигурирование и сборку в фоновом потоке
				string buildDir = System.IO.Path.Combine(editorDir, "build");
				System.IO.Directory.CreateDirectory(buildDir);

				await System.Threading.Tasks.Task.Run(() =>
				{
					// Запуск конфигурации
					var startInfoConfig = new System.Diagnostics.ProcessStartInfo
					{
						FileName = "cmake",
						Arguments = $"-B \"{buildDir}\" -S \"{editorDir}\"",
						CreateNoWindow = true,
						UseShellExecute = false,
						RedirectStandardError = true,
						RedirectStandardOutput = true
					};
					using (var proc = System.Diagnostics.Process.Start(startInfoConfig))
					{
						proc?.WaitForExit();
					}

					// Запуск сборки
					var startInfoBuild = new System.Diagnostics.ProcessStartInfo
					{
						FileName = "cmake",
						Arguments = $"--build \"{buildDir}\" --config {config}",
						CreateNoWindow = true,
						UseShellExecute = false,
						RedirectStandardError = true,
						RedirectStandardOutput = true
					};
					using (var proc = System.Diagnostics.Process.Start(startInfoBuild))
					{
						proc?.WaitForExit();
						if (proc?.ExitCode != 0)
						{
							throw new Exception($"CMake build failed with exit code {proc?.ExitCode}.");
						}
					}
				});

				EditorLogger.LogInfo("Compilation finished successfully. Reloading DLL...");

				// 4. Оповещаем движок о перезагрузке DLL
				EngineRuntime.SetProjectPath(projectRoot);
				EngineRuntime.ReloadScripts();
			}
			catch (Exception ex)
			{
				EditorLogger.LogError($"Script compilation failed: {ex.Message}");
			}
			finally
			{
				_isCompiling = false;
			}
		}
	}
}
