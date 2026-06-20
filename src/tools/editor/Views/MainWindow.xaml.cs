using System;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Markup;
using System.Runtime.InteropServices;
using System.Xml;
using AvalonDock;
using AvalonDock.Layout;
using AvalonDock.Layout.Serialization;
using editor.Services;
using editor.Views.Widgets;
using editor.Models;

namespace editor
{
	public partial class MainWindow : Window
	{
		[DllImport("editorDLL.dll", CallingConvention = CallingConvention.Cdecl)]
		private static extern bool Initialize(IntPtr hwnd);

		[DllImport("editorDLL.dll", CallingConvention = CallingConvention.Cdecl)]
		private static extern void Deinitialize();

		[DllImport("editorDLL.dll", CallingConvention = CallingConvention.Cdecl)]
		private static extern void Tick();

		private RenderWidget? _renderWidget;
		private GlobalSessionState _globalState = new();
		private LayoutSessionState _layoutState = new();
		private bool _isEngineInitialized = false;
		private string _defaultLayoutXml = "";

		// Заглушки для Undo/Redo команд хоткеев
		public static RoutedCommand UndoCommand = new RoutedCommand();
		public static RoutedCommand RedoCommand = new RoutedCommand();

		public MainWindow()
		{
			InitializeComponent();
			
			// Настройка биндингов команд
			CommandBindings.Add(new CommandBinding(UndoCommand, ExecuteUndo, CanExecuteUndo));
			CommandBindings.Add(new CommandBinding(RedoCommand, ExecuteRedo, CanExecuteRedo));

			Loaded += MainWindow_Loaded;
			Closing += MainWindow_Closing;
			Closed += MainWindow_Closed;

			// Инициализируем контент вкладок
			InitializeWidgets();
		}

		private void InitializeWidgets()
		{
			// Создаем инстансы виджетов-шаблонов
			_renderWidget = new RenderWidget();
			PaneRender.Content = _renderWidget;

			PaneInspector.Content = new InspectorWidget();
			PaneSceneTree.Content = new SceneTreeWidget();
			PaneAssets.Content = new AssetsWidget();
			PaneConsole.Content = new ConsoleWidget();
			PaneSettings.Content = new SettingsWidget();
			PaneBuild.Content = new BuildWidget();

			// Локализуем заголовки
			LocalizeTitle(PaneRender, "Widget_Render_Title");
			LocalizeTitle(PaneInspector, "Widget_Inspector_Title");
			LocalizeTitle(PaneSceneTree, "Widget_SceneTree_Title");
			LocalizeTitle(PaneAssets, "Widget_Assets_Title");
			LocalizeTitle(PaneConsole, "Widget_Console_Title");
			LocalizeTitle(PaneSettings, "Widget_Settings_Title");
			LocalizeTitle(PaneBuild, "Widget_Build_Title");

			// Построение меню "Окно"
			BuildWindowMenu();
		}

		private void LocalizeTitle(object pane, string key)
		{
			string localized = TryFindResource(key) as string ?? key;
			if (pane is AvalonDock.Layout.LayoutContent layoutContent)
			{
				layoutContent.Title = localized;
			}
		}

		private void BuildWindowMenu()
		{
			MenuWindow.Items.Clear();

			// Первое поле: Сбросить макет
			string resetTitle = TryFindResource("Menu_Window_Reset") as string ?? "Сбросить макет";
			var resetItem = new MenuItem { Header = resetTitle };
			resetItem.Click += (s, e) => ResetLayout();
			MenuWindow.Items.Add(resetItem);

			MenuWindow.Items.Add(new Separator());

			foreach (WidgetType type in Enum.GetValues(typeof(WidgetType)))
			{
				var meta = WidgetRules.GetMetadata(type);
				string title = TryFindResource(meta.TitleKey) as string ?? meta.TitleKey;

				var item = new MenuItem { Header = title, Tag = type };
				item.Click += WindowMenuItem_Click;
				MenuWindow.Items.Add(item);
			}
		}

		private void ResetLayout()
		{
			// Удаляем локальный файл макета сессии
			string layoutPath = EditorSessionManager.GetLayoutFilePath();
			try
			{
				if (File.Exists(layoutPath))
				{
					File.Delete(layoutPath);
				}
			}
			catch { }

			// Немедленно восстанавливаем раскладку по умолчанию
			if (!string.IsNullOrEmpty(_defaultLayoutXml))
			{
				try
				{
					var serializer = new XmlLayoutSerializer(DockManager);
					SetupSerializationCallback(serializer);
					using (var reader = new StringReader(_defaultLayoutXml))
					{
						serializer.Deserialize(reader);
					}
				}
				catch { }
			}
		}

		private void WindowMenuItem_Click(object sender, RoutedEventArgs e)
		{
			if (sender is MenuItem item && item.Tag is WidgetType type)
			{
				ShowWidget(type);
			}
		}

		private void ShowWidget(WidgetType type)
		{
			var meta = WidgetRules.GetMetadata(type);
			
			// Сначала ищем в активном дереве разметки
			AvalonDock.Layout.LayoutContent? layoutContent = FindLayoutContent(meta.SystemName);
			
			// Если не нашли (например, полностью откреплен и потерялся), берем по прямой ссылке
			if (layoutContent == null)
			{
				layoutContent = meta.SystemName switch
				{
					"SceneTreeWidget" => PaneSceneTree,
					"InspectorWidget" => PaneInspector,
					"AssetsWidget" => PaneAssets,
					"ConsoleWidget" => PaneConsole,
					"SettingsWidget" => PaneSettings,
					"BuildWidget" => PaneBuild,
					"RenderWidget" => PaneRender,
					_ => null
				};
			}

			if (layoutContent != null)
			{
				if (layoutContent is AvalonDock.Layout.LayoutAnchorable anchorable)
				{
					if (anchorable.IsHidden || !anchorable.IsVisible)
					{
						anchorable.IsVisible = true;
						anchorable.Show();
						
						// Откладываем выполнение Float до завершения прохода разметки WPF
						Dispatcher.BeginInvoke(new Action(() =>
						{
							try
							{
								anchorable.Float();
							}
							catch { }
						}), System.Windows.Threading.DispatcherPriority.Background);
					}
					else
					{
						anchorable.Show();
						anchorable.IsActive = true;
					}
				}
				else if (layoutContent is AvalonDock.Layout.LayoutDocument document)
				{
					document.IsActive = true;
				}
			}
		}

		private void DockManager_AnchorableClosing(object sender, AvalonDock.AnchorableClosingEventArgs e)
		{
			e.Cancel = true;
			e.Anchorable.Hide();
		}

		private AvalonDock.Layout.LayoutContent? FindLayoutContent(string contentId)
		{
			// Поиск среди документов и панелей
			foreach (var desc in DockManager.Layout.Descendents())
			{
				if (desc is AvalonDock.Layout.LayoutContent content && content.ContentId == contentId)
				{
					return content;
				}
			}
			return null;
		}

		private void MainWindow_Loaded(object sender, RoutedEventArgs e)
		{
			// Сохраняем исходную схему по умолчанию (из XAML)
			try
			{
				var serializer = new XmlLayoutSerializer(DockManager);
				using (var writer = new StringWriter())
				{
					serializer.Serialize(writer);
					_defaultLayoutXml = writer.ToString();
				}
			}
			catch { }

			// 1. Загружаем глобальную сессию
			_globalState = EditorSessionManager.LoadGlobalSession();
			UpdateRecentProjectsMenu();

			// 2. Загружаем локальный макет
			_layoutState = EditorSessionManager.LoadLayoutSession();
			RestoreWindowLayout();

			// 3. Инициализируем рендер движка (через хендл RenderWidget)
			if (_renderWidget != null)
			{
				_isEngineInitialized = Initialize(_renderWidget.RenderHandle);
				if (_isEngineInitialized)
				{
					CompositionTarget.Rendering += CompositionTarget_Rendering;
				}
			}
		}

		private bool _isDirty = false;
		private string? _currentProjectPath = null; // null означает, что проект не открыт

		public bool IsDirty
		{
			get => _isDirty;
			set
			{
				if (_isDirty != value)
				{
					_isDirty = value;
					UpdateWindowTitle();
					CommandManager.InvalidateRequerySuggested();
				}
			}
		}

		private void UpdateWindowTitle()
		{
			string appName = EditorConstants.ApplicationName;
			string noProjectText = TryFindResource("Menu_File_NoProject") as string ?? "Нет проекта";
			
			string projectInfo = string.IsNullOrEmpty(_currentProjectPath) 
				? noProjectText 
				: Path.GetFileNameWithoutExtension(_currentProjectPath);
			
			// Задаем заголовок окна и обновляем текстовые блоки внутри шапки
			Title = $"{appName} - [{projectInfo}]{(_isDirty ? "*" : "")}";
			
			if (TxtTitleAppName != null) TxtTitleAppName.Text = appName;
			if (TxtTitleProjectInfo != null) TxtTitleProjectInfo.Text = projectInfo;
			if (TxtTitleDirty != null)
			{
				TxtTitleDirty.Visibility = _isDirty ? Visibility.Visible : Visibility.Collapsed;
			}
		}

		// Обработчики кнопок управления окном
		private void ChromeMinimize_Click(object sender, RoutedEventArgs e)
		{
			WindowState = WindowState.Minimized;
		}

		private void ChromeMaximize_Click(object sender, RoutedEventArgs e)
		{
			if (WindowState == WindowState.Maximized)
			{
				WindowState = WindowState.Normal;
				BtnChromeMaximize.Content = "🗖";
			}
			else
			{
				WindowState = WindowState.Maximized;
				BtnChromeMaximize.Content = "🗗";
			}
		}

		private void ChromeClose_Click(object sender, RoutedEventArgs e)
		{
			Close();
		}

		private void SetupSerializationCallback(XmlLayoutSerializer serializer)
		{
			serializer.LayoutSerializationCallback += (s, args) =>
			{
				object? widgetContent = args.Model.ContentId switch
				{
					"SceneTreeWidget" => PaneSceneTree?.Content,
					"InspectorWidget" => PaneInspector?.Content,
					"AssetsWidget" => PaneAssets?.Content,
					"ConsoleWidget" => PaneConsole?.Content,
					"SettingsWidget" => PaneSettings?.Content,
					"BuildWidget" => PaneBuild?.Content,
					"RenderWidget" => PaneRender?.Content,
					_ => null
				};

				if (widgetContent != null)
				{
					args.Content = widgetContent;

					// Перепривязываем ссылки MainWindow к новым элементам дерева
					if (args.Model is LayoutAnchorable anchorable)
					{
						switch (args.Model.ContentId)
						{
							case "SceneTreeWidget": PaneSceneTree = anchorable; break;
							case "InspectorWidget": PaneInspector = anchorable; break;
							case "AssetsWidget": PaneAssets = anchorable; break;
							case "ConsoleWidget": PaneConsole = anchorable; break;
							case "SettingsWidget": PaneSettings = anchorable; break;
							case "BuildWidget": PaneBuild = anchorable; break;
							case "RenderWidget": PaneRender = anchorable; break;
						}
					}
				}
			};
		}

		private void RestoreWindowLayout()
		{
			// Восстановление координат окна
			if (_layoutState.Width > 100 && _layoutState.Height > 100)
			{
				Width = _layoutState.Width;
				Height = _layoutState.Height;
				Left = _layoutState.Left;
				Top = _layoutState.Top;
				if (_layoutState.IsMaximized)
				{
					WindowState = WindowState.Maximized;
				}
			}

			IsDirty = _layoutState.IsDirty;
			UpdateWindowTitle();

			// Восстановление расположения вкладок AvalonDock
			if (!string.IsNullOrEmpty(_layoutState.LayoutXml))
			{
				try
				{
					var serializer = new XmlLayoutSerializer(DockManager);
					SetupSerializationCallback(serializer);
					using (var reader = new StringReader(_layoutState.LayoutXml))
					{
						serializer.Deserialize(reader);
					}
				}
				catch
				{
					// При сбое сериализатора загружается раскладка по умолчанию (уже задана в XAML)
				}
			}
		}

		private void SaveWindowLayout()
		{
			_layoutState.Width = Width;
			_layoutState.Height = Height;
			_layoutState.Left = Left;
			_layoutState.Top = Top;
			_layoutState.IsMaximized = (WindowState == WindowState.Maximized);
			_layoutState.IsDirty = IsDirty;

			try
			{
				var serializer = new XmlLayoutSerializer(DockManager);
				using (var writer = new StringWriter())
				{
					serializer.Serialize(writer);
					_layoutState.LayoutXml = writer.ToString();
				}
			}
			catch
			{
				// Пропускаем сбой сериализации макета
			}

			EditorSessionManager.SaveLayoutSession(_layoutState);
		}

		private void CompositionTarget_Rendering(object? sender, EventArgs e)
		{
			if (_isEngineInitialized)
			{
				Tick();
			}
		}

		private void MainWindow_Closing(object? sender, System.ComponentModel.CancelEventArgs e)
		{
			// Проверка состояния Dirty (наличие несохраненных данных)
			if (IsDirty)
			{
				string title = TryFindResource("Dialog_Close_Title") as string ?? "Выход";
				string message = TryFindResource("Dialog_Close_Unsaved") as string ?? "Сохранить проект перед выходом?";
				
				var result = MessageBox.Show(message, title, MessageBoxButton.YesNoCancel, MessageBoxImage.Warning);
				if (result == MessageBoxResult.Yes)
				{
					// Сохраняем проект (имитируем сохранение)
					IsDirty = false;
				}
				else if (result == MessageBoxResult.Cancel)
				{
					e.Cancel = true;
					return;
				}
			}

			// Сохраняем состояние сессии
			SaveWindowLayout();
			EditorSessionManager.SaveGlobalSession(_globalState);
		}

		private void MainWindow_Closed(object? sender, EventArgs e)
		{
			if (_isEngineInitialized)
			{
				CompositionTarget.Rendering -= CompositionTarget_Rendering;
				Deinitialize();
			}
		}

		// Логика меню и кнопок
		private void Menu_CreateProject_Click(object sender, RoutedEventArgs e)
		{
			MessageBox.Show("Создание нового проекта (заглушка)", "Проект", MessageBoxButton.OK, MessageBoxImage.Information);
			_currentProjectPath = @"C:\Projects\NewProject.zzz";
			IsDirty = true;
		}

		private void Menu_OpenProject_Click(object sender, RoutedEventArgs e)
		{
			MessageBox.Show("Открытие существующего проекта (заглушка)", "Проект", MessageBoxButton.OK, MessageBoxImage.Information);
			string projectPath = @"C:\Projects\ZzzGame_" + Random.Shared.Next(100) + ".zzz";
			_currentProjectPath = projectPath;
			IsDirty = false;

			// Имитация добавления в список последних проектов
			AddRecentProject(projectPath);
		}

		private void AddRecentProject(string path)
		{
			var list = new System.Collections.Generic.List<string>(_globalState.RecentProjects ?? Array.Empty<string>());
			list.Remove(path);
			list.Insert(0, path);
			if (list.Count > 10)
			{
				list.RemoveAt(10);
			}
			_globalState.RecentProjects = list.ToArray();
			UpdateRecentProjectsMenu();
		}

		private void UpdateRecentProjectsMenu()
		{
			MenuRecentProjects.Items.Clear();
			if (_globalState.RecentProjects == null || _globalState.RecentProjects.Length == 0)
			{
				var emptyItem = new MenuItem { Header = "Нет последних проектов", IsEnabled = false };
				MenuRecentProjects.Items.Add(emptyItem);
				return;
			}

			foreach (var path in _globalState.RecentProjects)
			{
				var item = new MenuItem { Header = path };
				item.Click += (s, e) => {
					MessageBox.Show($"Открываем последний проект: {path}", "Проект");
					_currentProjectPath = path;
					IsDirty = false;
					UpdateWindowTitle();
				};
				MenuRecentProjects.Items.Add(item);
			}
		}

		private void Menu_Exit_Click(object sender, RoutedEventArgs e)
		{
			Close();
		}

		private void Menu_About_Click(object sender, RoutedEventArgs e)
		{
			var aboutDialog = new editor.Views.AboutDialog();
			aboutDialog.Owner = this;
			aboutDialog.ShowDialog();
		}

		// Команды Undo/Redo логики
		private void ExecuteUndo(object sender, ExecutedRoutedEventArgs e) { Undo_Click(sender, e); }
		private void CanExecuteUndo(object sender, CanExecuteRoutedEventArgs e) { e.CanExecute = IsDirty; }

		private void ExecuteRedo(object sender, ExecutedRoutedEventArgs e) { Redo_Click(sender, e); }
		private void CanExecuteRedo(object sender, CanExecuteRoutedEventArgs e) { e.CanExecute = IsDirty; }

		private void Undo_Click(object sender, RoutedEventArgs e)
		{
			// Логика отмены действия (Undo) будет реализована позже
		}

		private void Redo_Click(object sender, RoutedEventArgs e)
		{
			// Логика повтора действия (Redo) будет реализована позже
		}

		private void Play_Click(object sender, RoutedEventArgs e)
		{
			// Запуск симуляции
		}

		private void Pause_Click(object sender, RoutedEventArgs e)
		{
			// Пауза симуляции
		}

		private void Step_Click(object sender, RoutedEventArgs e)
		{
			// Остановка симуляции (Stop)
		}

		private void TitleBar_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
		{
			if (e.ChangedButton == MouseButton.Left)
			{
				if (e.ClickCount == 2)
				{
					ChromeMaximize_Click(sender, e);
				}
				else
				{
					this.DragMove();
				}
			}
		}
	}
}