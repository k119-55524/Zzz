
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
            // If a script change happened while window was not active, process it now.
            await ScriptRebuildCoordinator.ProcessPendingAsync(this);
        }

		// Тот же чек запускается из AssetsViewModel при создании/удалении/переименовании .hpp/.cpp
		// (см. AssetsViewModel.TriggerScriptCompileCheckIfRelevant) - иначе пользователь, который не
		// переключал фокус окна после добавления скрипта, никогда не увидел бы автокомпиляцию.
		//
		// forceRebuild=true пропускает эвристику по времени изменения файлов и пересобирает сразу.
		// Эвристика (maxWriteTime > dllWriteTime) видит только ПРАВКУ существующих .hpp/.cpp - при
		// УДАЛЕНИИ скрипта время записи оставшихся файлов не меняется, поэтому "тише" удалённого
		// скрипта эвристика никогда не сочтёт DLL устаревшей. Вызывающая сторона (watcher) точно
		// знает, что .hpp/.cpp изменился, и эвристика здесь не нужна.
		public async System.Threading.Tasks.Task CheckAndCompileScriptsAsync(bool forceRebuild = false, string? projectRoot = null)
		{
			projectRoot ??= _viewModel.CurrentProjectPath;
			if (string.IsNullOrEmpty(projectRoot) || !System.IO.Directory.Exists(projectRoot))
			{
				EditorLogger.LogInfo("[Scripts] CheckAndCompile: skip — project path is null or missing.");
				return;
			}

			if (_isCompiling)
			{
				EditorLogger.LogInfo("[Scripts] CheckAndCompile: skip — already compiling.");
				return;
			}

			string assetsDir = System.IO.Path.Combine(projectRoot, "Assets");
			if (!System.IO.Directory.Exists(assetsDir))
			{
				EditorLogger.LogInfo("[Scripts] CheckAndCompile: skip — Assets dir not found.");
				return;
			}

			string dllPath = System.IO.Path.Combine(projectRoot, ".editor", "bin", "scripts.dll");

			if (!forceRebuild)
			{
				// Считаем только пользовательские .hpp — именно они говорят о наличии скриптов.
				// RegisterAllScripts.cpp исключаем: он генерируется редактором автоматически
				// и его write-time обновляется при любом изменении дерева — включать его в
				// проверку означало бы пересобирать DLL на каждый чих.
				var userHppFiles = System.IO.Directory.GetFiles(assetsDir, "*.hpp", System.IO.SearchOption.AllDirectories);

				if (userHppFiles.Length == 0)
				{
					EditorLogger.LogInfo("[Scripts] CheckAndCompile: skip — no .hpp scripts found in Assets.");
					return;
				}

				// Ориентируемся на .hpp: именно изменения в заголовках требуют пересборки.
				// .cpp тоже учитываем, но отдельно от RegisterAllScripts.cpp.
				DateTime maxWriteTime = DateTime.MinValue;
				var allUserSources = System.IO.Directory.GetFiles(assetsDir, "*.*", System.IO.SearchOption.AllDirectories)
					.Where(f =>
						(f.EndsWith(".hpp", StringComparison.OrdinalIgnoreCase) ||
						 f.EndsWith(".cpp", StringComparison.OrdinalIgnoreCase)) &&
						!System.IO.Path.GetFileName(f).Equals("RegisterAllScripts.cpp", StringComparison.OrdinalIgnoreCase));

				foreach (var file in allUserSources)
				{
					var writeTime = System.IO.File.GetLastWriteTime(file);
					if (writeTime > maxWriteTime)
						maxWriteTime = writeTime;
				}

				bool dllExists = System.IO.File.Exists(dllPath);
				DateTime dllWriteTime = dllExists ? System.IO.File.GetLastWriteTime(dllPath) : DateTime.MinValue;

				if (dllExists && maxWriteTime <= dllWriteTime)
				{
					// Дополнительная проверка: если RegisterAllScripts.cpp новее DLL — состав скриптов
					// изменился (добавили/удалили скрипт пока редактор был закрыт), но write-time .hpp
					// не поменялся. RefreshTree уже обновил RegisterAllScripts.cpp — форсируем сборку.
					string registerAllPath = System.IO.Path.Combine(projectRoot, ".editor", "RegisterAllScripts.cpp");
					if (System.IO.File.Exists(registerAllPath) &&
					    System.IO.File.GetLastWriteTime(registerAllPath) <= dllWriteTime)
					{
						EditorLogger.LogInfo($"[Scripts] CheckAndCompile: skip — DLL up to date (dll: {dllWriteTime:HH:mm:ss}, maxSrc: {maxWriteTime:HH:mm:ss}, register: {System.IO.File.GetLastWriteTime(registerAllPath):HH:mm:ss}).");
						return;
					}
					// RegisterAllScripts.cpp не существует или новее DLL → пересобираем
					EditorLogger.LogInfo("[Scripts] CheckAndCompile: RegisterAllScripts.cpp changed or missing — forcing rebuild.");
				}
				else if (!dllExists)
				{
					EditorLogger.LogInfo("[Scripts] CheckAndCompile: DLL doesn't exist — building.");
				}
				else
				{
					EditorLogger.LogInfo($"[Scripts] CheckAndCompile: sources newer than DLL (dll: {dllWriteTime:HH:mm:ss}, maxSrc: {maxWriteTime:HH:mm:ss}) — rebuilding.");
				}
			}

			await CompileScriptsAsync(projectRoot, dllPath);
		}

		private async System.Threading.Tasks.Task CompileScriptsAsync(string projectRoot, string dllPath)
		{
			            _isCompiling = true;
            // Ensure any previously loaded script DLL is unloaded before starting a new compilation to avoid file lock issues.
            EngineRuntime.ClearEngine();
            EditorLogger.LogInfo("Scripts modification detected. Starting background compilation...");

			try
			{
				// 1. Создаем папку .editor если не существует
				string editorDir = System.IO.Path.Combine(projectRoot, ".editor");
				System.IO.Directory.CreateDirectory(editorDir);

				// 2. Генерируем CMakeLists.txt
				string cmakePath = System.IO.Path.Combine(editorDir, "CMakeLists.txt");
				// OutputPath editor.csproj = "..\..\..\bin\$(Configuration)\" относительно src/tools/editor,
				// AppendTargetFrameworkToOutputPath=false - до корня репозитория от BaseDirectory всего
				// два уровня вверх (см. аналогичный фикс в AssetsWidget.GetTemplatesDirectory).
				string engineSourceDir = System.IO.Path.GetFullPath(System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "..", "..")).Replace('\\', '/');
				string zlibsIncludeDir = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "libs", "zlibs", "include").Replace('\\', '/');
				
				// Определяем конфигурацию сборки
				string config = "Debug";
				#if DEBUG
				config = "Debug";
				#else
				config = "Release";
				#endif

				string editorDllLib = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "editor_dll.lib").Replace('\\', '/');

				// Имя CMake-проекта = имя папки игрового проекта, иначе .slnx у всех проектов
				// называется одинаково ("project_scripts") и неотличимо в списке Recent Projects VS.
				string gameProjectName = System.IO.Path.GetFileName(projectRoot.TrimEnd('\\', '/'));
				string sanitizedProjectName = System.Text.RegularExpressions.Regex.Replace(gameProjectName, @"[^A-Za-z0-9_]", "_");
				if (string.IsNullOrEmpty(sanitizedProjectName) || char.IsDigit(sanitizedProjectName[0]))
					sanitizedProjectName = "_" + sanitizedProjectName;
				string cmakeProjectName = $"{sanitizedProjectName}_scripts";

				string cmakeContent = $@"cmake_minimum_required(VERSION 3.28)
project({cmakeProjectName} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ""${{PROJECT_SOURCE_DIR}}/bin"")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ""${{PROJECT_SOURCE_DIR}}/bin"")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ""${{PROJECT_SOURCE_DIR}}/bin"")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ""${{PROJECT_SOURCE_DIR}}/bin"")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ""${{PROJECT_SOURCE_DIR}}/bin"")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ""${{PROJECT_SOURCE_DIR}}/bin"")

add_library(scripts SHARED)
target_include_directories(scripts PRIVATE
    ""{zlibsIncludeDir}""
    ""{zlibsIncludeDir}/engine""
    ""{zlibsIncludeDir}/engine/public/core/scene""
    ""{zlibsIncludeDir}/engine/public/core/scene/scripts""
    ""{zlibsIncludeDir}/engine/public/core/scene/scripts/base_script""
    ""{zlibsIncludeDir}/common""
    ""{zlibsIncludeDir}/logger""
)

file(GLOB_RECURSE SCRIPT_SOURCES ""${{PROJECT_SOURCE_DIR}}/../Assets/*.cpp"")
target_sources(scripts PRIVATE
    ""${{PROJECT_SOURCE_DIR}}/RegisterAllScripts.cpp""
    ${{SCRIPT_SOURCES}}
)

target_compile_definitions(scripts PRIVATE Z_EDITOR=1)

target_link_libraries(scripts PRIVATE ""{editorDllLib}"")
";

				bool needWriteCmake = !System.IO.File.Exists(cmakePath) ||
				                      System.IO.File.ReadAllText(cmakePath) != cmakeContent;
				if (needWriteCmake)
					System.IO.File.WriteAllText(cmakePath, cmakeContent);

				string buildDir = System.IO.Path.Combine(editorDir, "build");
				System.IO.Directory.CreateDirectory(buildDir);
				
				// We need to configure if CMakeLists changed, OR if RegisterAllScripts.cpp was just written
				// (which implies scripts were added/removed, so the GLOB needs to be re-evaluated).
				bool needConfigure = needWriteCmake || !System.IO.File.Exists(System.IO.Path.Combine(buildDir, "CMakeCache.txt"));
				
				// Quick check if RegisterAllScripts.cpp was recently modified (within last 2 seconds)
				string registerPath = System.IO.Path.Combine(editorDir, "RegisterAllScripts.cpp");
				if (System.IO.File.Exists(registerPath))
				{
					if ((DateTime.Now - System.IO.File.GetLastWriteTime(registerPath)).TotalSeconds < 2)
					{
						needConfigure = true;
					}
				}

				await System.Threading.Tasks.Task.Run(() =>
				{
					if (needConfigure)
					{
						var startInfoConfig = new System.Diagnostics.ProcessStartInfo
						{
							FileName = "cmake",
							Arguments = $"-B \"{buildDir}\" -S \"{editorDir}\"",
							CreateNoWindow = true,
							UseShellExecute = false,
							RedirectStandardError = true,
							RedirectStandardOutput = true,
							StandardOutputEncoding = System.Text.Encoding.UTF8,
							StandardErrorEncoding = System.Text.Encoding.UTF8
						};
						using (var proc = System.Diagnostics.Process.Start(startInfoConfig))
						{
							string stdout = proc?.StandardOutput.ReadToEnd() ?? string.Empty;
							string stderr = proc?.StandardError.ReadToEnd() ?? string.Empty;
							proc?.WaitForExit();

							if (!string.IsNullOrWhiteSpace(stdout))
								EditorLogger.LogInfo($"[CMake Configure] {stdout.Trim()}");
							if (!string.IsNullOrWhiteSpace(stderr))
								EditorLogger.LogInfo($"[CMake Configure] {stderr.Trim()}");

							if (proc?.ExitCode != 0)
								throw new Exception($"CMake configure failed with exit code {proc?.ExitCode}.\n{stdout}\n{stderr}");
						}
					}

					// Before building, try to rename existing DLL and PDB to avoid MSVC linker lock issues (LNK1201).
					// If the engine has the PDB locked via DbgHelp, we can't overwrite it, but we CAN rename it!
					string binDir = System.IO.Path.Combine(editorDir, "bin");
					string dllPath = System.IO.Path.Combine(binDir, "scripts.dll");
					string pdbPath = System.IO.Path.Combine(binDir, "scripts.pdb");
					string tick = DateTime.Now.Ticks.ToString();

					if (System.IO.File.Exists(dllPath))
					{
						try { System.IO.File.Move(dllPath, System.IO.Path.Combine(binDir, $"scripts_old_{tick}.dll")); } catch { }
					}
					if (System.IO.File.Exists(pdbPath))
					{
						try { System.IO.File.Move(pdbPath, System.IO.Path.Combine(binDir, $"scripts_old_{tick}.pdb")); } catch { }
					}

					// Cleanup old files in a background thread so we don't block
					System.Threading.Tasks.Task.Run(() =>
					{
						try
						{
							foreach (var file in System.IO.Directory.GetFiles(binDir, "scripts_old_*.dll"))
								try { System.IO.File.Delete(file); } catch { }
							foreach (var file in System.IO.Directory.GetFiles(binDir, "scripts_old_*.pdb"))
								try { System.IO.File.Delete(file); } catch { }
						}
						catch { }
					});

					var startInfoBuild = new System.Diagnostics.ProcessStartInfo
					{
						FileName = "cmake",
						Arguments = $"--build \"{buildDir}\" --config {config}",
						CreateNoWindow = true,
						UseShellExecute = false,
						RedirectStandardError = true,
						RedirectStandardOutput = true,
						StandardOutputEncoding = System.Text.Encoding.UTF8,
						StandardErrorEncoding = System.Text.Encoding.UTF8
					};
					using (var proc = System.Diagnostics.Process.Start(startInfoBuild))
					{
						string stdout = proc?.StandardOutput.ReadToEnd() ?? string.Empty;
						string stderr = proc?.StandardError.ReadToEnd() ?? string.Empty;
						proc?.WaitForExit();

						// MSBuild (генератор Visual Studio) пишет ошибки компиляции в stdout, а не
						// в stderr - в отличие от самого CMake. Без этого реальный текст ошибки
						// (например, какая строка C++ не скомпилировалась) терялся, и пользователь
						// видел только "exit code 1" без объяснения причины.
						if (!string.IsNullOrWhiteSpace(stdout))
							EditorLogger.LogInfo($"[CMake Build] {stdout.Trim()}");
						if (!string.IsNullOrWhiteSpace(stderr))
							EditorLogger.LogInfo($"[CMake Build] {stderr.Trim()}");

						if (proc?.ExitCode != 0)
							throw new Exception($"CMake build failed with exit code {proc?.ExitCode}.\n{stdout}\n{stderr}");
					}
				});

				EditorLogger.LogInfo("Compilation finished successfully. Reloading DLL...");

				// 4. Оповещаем движок о перезагрузке DLL
				EngineRuntime.SetProjectPath(projectRoot);
				EngineRuntime.ClearEngine();
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

