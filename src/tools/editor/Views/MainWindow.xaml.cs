
using System;
using System.Linq;
using System.Windows;
using editor.Services;
using editor.Services.Project;
using editor.Services.Project.FileTypes.GameConfig;
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
		private bool _compileRequestedAgain = false;
		private bool _queuedForceRebuild = false;
		private string? _queuedCompileProjectRoot;

        private async void MainWindow_Activated(object? sender, EventArgs e)
        {
            // Если изменение скрипта произошло, пока окно было неактивно, обрабатываем его сейчас.
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
				EditorLogger.LogInfo("[Scripts] CheckAndCompile: пропуск — путь проекта пуст или не существует.");
				return;
			}

			if (_isCompiling)
			{
				_compileRequestedAgain = true;
				_queuedForceRebuild |= forceRebuild;
				_queuedCompileProjectRoot = projectRoot;
				EditorLogger.LogInfo("[Scripts] CheckAndCompile: поставлено в очередь — уже идёт компиляция.");
				return;
			}

			string assetsDir = System.IO.Path.Combine(projectRoot, "Assets");
			if (!System.IO.Directory.Exists(assetsDir))
			{
				EditorLogger.LogInfo("[Scripts] CheckAndCompile: пропуск — папка Assets не найдена.");
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
					EditorLogger.LogInfo("[Scripts] CheckAndCompile: пропуск — в Assets не найдено .hpp скриптов.");
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
						EditorLogger.LogInfo($"[Scripts] CheckAndCompile: пропуск — DLL актуальна (dll: {dllWriteTime:HH:mm:ss}, maxSrc: {maxWriteTime:HH:mm:ss}, register: {System.IO.File.GetLastWriteTime(registerAllPath):HH:mm:ss}).");
						return;
					}
					// RegisterAllScripts.cpp не существует или новее DLL → пересобираем
					EditorLogger.LogInfo("[Scripts] CheckAndCompile: RegisterAllScripts.cpp изменён или отсутствует — форсируем пересборку.");
				}
				else if (!dllExists)
				{
					EditorLogger.LogInfo("[Scripts] CheckAndCompile: DLL не существует — собираем.");
				}
				else
				{
					EditorLogger.LogInfo($"[Scripts] CheckAndCompile: исходники новее DLL (dll: {dllWriteTime:HH:mm:ss}, maxSrc: {maxWriteTime:HH:mm:ss}) — пересобираем.");
				}
			}

			await CompileScriptsAsync(projectRoot, dllPath);
		}

		private static string ToCMakePath(string path)
		{
			return path.Replace('\\', '/').Replace("\"", "\\\"");
		}

		private static string BuildCMakeSourceList(string assetsDir)
		{
			var scriptSources = System.IO.Directory
				.GetFiles(assetsDir, "*.cpp", System.IO.SearchOption.AllDirectories)
				.OrderBy(path => path, StringComparer.OrdinalIgnoreCase)
				.Select(path => $"    \"{ToCMakePath(path)}\"");

			return string.Join(Environment.NewLine, scriptSources);
		}

		private async System.Threading.Tasks.Task CompileScriptsAsync(string projectRoot, string dllPath)
		{
			if (_viewModel.IsRunning && _viewModel.StopCommand.CanExecute(null))
			{
				EditorLogger.LogInfo("Остановка Play Mode перед рекомпиляцией скриптов...");
				_viewModel.StopCommand.Execute(null);
			}

			_isCompiling = true;
			Application.Current.Dispatcher.Invoke(() => { 
				StatusText.Text = "Компиляция скриптов..."; 
				CompilationProgressBar.Visibility = Visibility.Visible;
				CompilationOverlay.Visibility = Visibility.Visible; 
				if (MainMenu != null) MainMenu.IsEnabled = false;
			});
			EditorLogger.LogInfo("Обнаружено изменение скриптов. Запускаем фоновую компиляцию...");

			try
			{
				// 1. Создаем папку .editor если не существует
				string editorDir = System.IO.Path.Combine(projectRoot, ".editor");
				string assetsDir = System.IO.Path.Combine(projectRoot, "Assets");
				string scriptSources = BuildCMakeSourceList(assetsDir);
				System.IO.Directory.CreateDirectory(editorDir);

				// 2. Генерируем CMakeLists.txt
				string cmakePath = System.IO.Path.Combine(editorDir, "CMakeLists.txt");
				// OutputPath editor.csproj = "..\..\..\bin\$(Configuration)\" относительно src/tools/editor,
				// AppendTargetFrameworkToOutputPath=false - до корня репозитория от BaseDirectory всего
				// два уровня вверх (см. аналогичный фикс в AssetsWidget.GetTemplatesDirectory).
				string engineSourceDir = System.IO.Path.GetFullPath(System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "..", "..")).Replace('\\', '/');
				string zlibsIncludeDir = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "libs", "zlibs", "include").Replace('\\', '/');
				
				// Всегда собираем скрипты в режиме Debug при работе в редакторе (чтобы работали точки останова)
				// TODO: Добавить в настройки проекта/редактора возможность выбора режима сборки скриптов (Debug/Release/RelWithDebInfo)
				string config = "Debug";

				string editorDllLib = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "editor_dll.lib").Replace('\\', '/');
				string loggerLib = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "logger_lib.lib").Replace('\\', '/');

				// Имя CMake-проекта = имя папки игрового проекта, иначе .slnx у всех проектов
				// называется одинаково ("project_scripts") и неотличимо в списке Recent Projects VS.
				string gameProjectName = System.IO.Path.GetFileName(projectRoot.TrimEnd('\\', '/'));
				string sanitizedProjectName = System.Text.RegularExpressions.Regex.Replace(gameProjectName, @"[^A-Za-z0-9_]", "_");
				if (string.IsNullOrEmpty(sanitizedProjectName) || char.IsDigit(sanitizedProjectName[0]))
					sanitizedProjectName = "_" + sanitizedProjectName;
				string cmakeProjectName = $"{sanitizedProjectName}_scripts";

				// Defines из Configs/game_config.toml (см. GameConfigData.Defines) должны попадать
				// в реальную сборку scripts.dll - иначе поле в инспекторе ничего не значит.
				string gameConfigPath = System.IO.Path.Combine(projectRoot, ProjectConstants.SystemDirectories.GameConfigs);
				var userDefines = new System.Collections.Generic.List<string>();
				if (System.IO.File.Exists(gameConfigPath))
				{
					try
					{
						userDefines = GameConfigParser.Deserialize(System.IO.File.ReadAllText(gameConfigPath)).Defines;
					}
					catch (Exception ex)
					{
						EditorLogger.LogError($"[Scripts] Не удалось прочитать дефайны из game_config.toml: {ex.Message}");
					}
				}

				string userDefinesLines = userDefines.Count > 0
					? Environment.NewLine + string.Join(Environment.NewLine, userDefines.Select(d => $"    \"{d}\""))
					: string.Empty;

				string cmakeContent = $@"cmake_minimum_required(VERSION 3.28)
project({cmakeProjectName} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ""${{PROJECT_SOURCE_DIR}}/bin_build"")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ""${{PROJECT_SOURCE_DIR}}/bin_build"")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ""${{PROJECT_SOURCE_DIR}}/bin_build"")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ""${{PROJECT_SOURCE_DIR}}/bin_build"")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ""${{PROJECT_SOURCE_DIR}}/bin_build"")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ""${{PROJECT_SOURCE_DIR}}/bin_build"")

add_library(scripts SHARED)
target_include_directories(scripts PRIVATE
    ""{zlibsIncludeDir}""
    ""{zlibsIncludeDir}/engine""
    ""{zlibsIncludeDir}/engine/scene""
    ""{zlibsIncludeDir}/core/userscripts""
    ""{zlibsIncludeDir}/core/userscripts/base_script""
    ""{zlibsIncludeDir}/logger""
)

target_sources(scripts PRIVATE
    ""${{PROJECT_SOURCE_DIR}}/RegisterAllScripts.cpp""
{scriptSources}
)

target_compile_definitions(scripts PRIVATE
    Z_EDITOR=1
    Z_ADD_LOGGER=1
    Z_DEVELOPMENT_BUILD=1{userDefinesLines}
)

if(MSVC)
    target_compile_options(scripts PRIVATE /Zi /Od)
    target_link_options(scripts PRIVATE /DEBUG:FULL /INCREMENTAL:NO ""/PDB:${{PROJECT_SOURCE_DIR}}/bin_build/scripts.pdb"")
endif()

target_link_libraries(scripts PRIVATE ""{editorDllLib}"" ""{loggerLib}"" ws2_32.lib)
";

				bool needWriteCmake = !System.IO.File.Exists(cmakePath) ||
				                      System.IO.File.ReadAllText(cmakePath) != cmakeContent;
				if (needWriteCmake)
					System.IO.File.WriteAllText(cmakePath, cmakeContent);

				string buildDir = System.IO.Path.Combine(editorDir, "build");
				System.IO.Directory.CreateDirectory(buildDir);
				bool needConfigure = needWriteCmake || !System.IO.File.Exists(System.IO.Path.Combine(buildDir, "CMakeCache.txt"));

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
						startInfoConfig.EnvironmentVariables["VSLANG"] = "1033"; // Force MSVC to output in English
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

					string stagingDir = System.IO.Path.Combine(editorDir, "bin_build");
					System.IO.Directory.CreateDirectory(stagingDir);
					string stagedDllPath = System.IO.Path.Combine(stagingDir, "scripts.dll");
					string stagedPdbPath = System.IO.Path.Combine(stagingDir, "scripts.pdb");

					try { if (System.IO.File.Exists(stagedDllPath)) System.IO.File.Delete(stagedDllPath); } catch { }
					try { if (System.IO.File.Exists(stagedPdbPath)) System.IO.File.Delete(stagedPdbPath); } catch { }

					System.Threading.Tasks.Task.Run(() =>
					{
						try
						{
							string binDir = System.IO.Path.Combine(editorDir, "bin");
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
					startInfoBuild.EnvironmentVariables["VSLANG"] = "1033"; // Force MSVC to output in English
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

				EditorLogger.LogInfo("Компиляция успешно завершена. Перезагружаем DLL...");

				// 4. Оповещаем движок о перезагрузке DLL
				EngineRuntime.SetProjectPath(projectRoot);
				string finalBinDir = System.IO.Path.GetDirectoryName(dllPath) ?? System.IO.Path.Combine(projectRoot, ".editor", "bin");
				string finalPdbPath = System.IO.Path.ChangeExtension(dllPath, ".pdb");
				string acceptStagingDir = System.IO.Path.Combine(projectRoot, ".editor", "bin_build");
				string acceptStagedDllPath = System.IO.Path.Combine(acceptStagingDir, "scripts.dll");
				string acceptStagedPdbPath = System.IO.Path.Combine(acceptStagingDir, "scripts.pdb");

				if (!System.IO.File.Exists(acceptStagedDllPath))
					throw new Exception($"CMake build did not produce expected DLL: {acceptStagedDllPath}");

				EngineRuntime.ClearEngine();
				System.IO.Directory.CreateDirectory(finalBinDir);
				string tick = DateTime.Now.Ticks.ToString();
				string? oldDllPath = null;
				string? oldPdbPath = null;

				try
				{
					if (System.IO.File.Exists(dllPath))
					{
						oldDllPath = System.IO.Path.Combine(finalBinDir, $"scripts_old_{tick}.dll");
						System.IO.File.Move(dllPath, oldDllPath);
					}

					if (System.IO.File.Exists(finalPdbPath))
					{
						oldPdbPath = System.IO.Path.Combine(finalBinDir, $"scripts_old_{tick}.pdb");
						System.IO.File.Move(finalPdbPath, oldPdbPath);
					}

					System.IO.File.Move(acceptStagedDllPath, dllPath);
					if (System.IO.File.Exists(acceptStagedPdbPath))
					{
						System.IO.File.Copy(acceptStagedPdbPath, finalPdbPath, overwrite: true);
						EditorLogger.LogInfo($"[Scripts] PDB kept at linker path '{acceptStagedPdbPath}' and copied to '{finalPdbPath}'.");
					}
				}
				catch
				{
					try
					{
						if (!System.IO.File.Exists(dllPath) && oldDllPath != null && System.IO.File.Exists(oldDllPath))
							System.IO.File.Move(oldDllPath, dllPath);
						if (!System.IO.File.Exists(finalPdbPath) && oldPdbPath != null && System.IO.File.Exists(oldPdbPath))
							System.IO.File.Move(oldPdbPath, finalPdbPath);
						if (System.IO.File.Exists(dllPath))
							EngineRuntime.ReloadScripts();
					}
					catch { }

					throw;
				}

				EngineRuntime.ReloadScripts();
			}
			catch (Exception ex)
			{
				EditorLogger.LogError($"Компиляция скриптов не удалась: {ex.Message}");
			}
			finally
			{
				_isCompiling = false;
				Application.Current.Dispatcher.Invoke(() => { 
					StatusText.Text = "Готово"; 
					CompilationProgressBar.Visibility = Visibility.Collapsed;
					CompilationOverlay.Visibility = Visibility.Collapsed; 
					if (MainMenu != null) MainMenu.IsEnabled = true;
				});
			}

			if (_compileRequestedAgain)
			{
				bool forceQueuedRebuild = _queuedForceRebuild;
				string? queuedProjectRoot = _queuedCompileProjectRoot;
				_compileRequestedAgain = false;
				_queuedForceRebuild = false;
				_queuedCompileProjectRoot = null;

				await CheckAndCompileScriptsAsync(forceQueuedRebuild, queuedProjectRoot);
			}
		}
	}
}

