using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;
using System.Windows.Shell;
using System.Windows.Threading;
using AvalonDock.Layout;
using editor.Services;
using editor.ViewModels;
using editor.Views.Widgets;

namespace editor
{
	public partial class MainWindow : Window
	{
		private readonly MainWindowViewModel _viewModel;
		private string _defaultLayoutXml = "";
		private bool _isEngineInitialized = false;

		// LayoutAnchorablePaneControl не подхватывает implicit-стиль ни по своему типу, ни по типу
		// TabControl (от которого наследует DefaultStyleKey) — AvalonDock явно резолвит Style через
		// внутренний механизм, минующий обычный resource lookup. Назначаем стиль самостоятельно через
		// class handler на Loaded (это Direct-событие, не всплывает — обычный AddHandler на окне его
		// не поймает, нужен RegisterClassHandler).
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
			if (sender is DependencyObject obj)
			{
				var button = FindVisualChild<FrameworkElement>(obj, "SinglePaneContextMenu");
				if (button != null)
				{
					button.Visibility = Visibility.Collapsed;
					button.Width = 0;
					button.Height = 0;
				}
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

			// SizeChanged самого TabControl не срабатывает, когда меняется только внутреннее
			// расположение закладок (например, после скрытия/восстановления соседней панели —
			// общий размер строки закладок не меняется, а позиции внутри неё съезжают).
			// LayoutUpdated ловит любой завершённый проход разметки в поддереве, включая такие случаи —
			// но дёргать TranslatePoint прямо из самого LayoutUpdated реентерабельно форсирует ещё один
			// проход разметки и может уйти в рекурсию до переполнения стека. Поэтому пересчёт всегда
			// откладывается на отдельный тик диспетчера, и при этом схлопывается (не более одного
			// отложенного пересчёта одновременно на один TabControl).
			var scheduler = new OutlineUpdateScheduler(tabControl);
			tabControl.SelectionChanged += (s, _) => scheduler.Schedule();
			tabControl.LayoutUpdated += (s, _) => scheduler.Schedule();
			tabControl.Loaded += (s, _) => scheduler.Schedule();
			scheduler.Schedule();
		}

		private sealed class OutlineUpdateScheduler
		{
			// Защита от зацикливания диспетчера на случай непредвиденного сценария, где условие
			// готовности контейнера так и не становится истинным (см. UpdateTabSelectionOutline) —
			// после нескольких неудачных попыток просто ждём следующего настоящего события.
			private const int MaxConsecutiveRetries = 5;

			private readonly TabControl _tabControl;
			private bool _isPending;
			private int _retryCount;

			public OutlineUpdateScheduler(TabControl tabControl)
			{
				_tabControl = tabControl;
			}

			// Вызывается из настоящих WPF-событий (SelectionChanged/LayoutUpdated/Loaded) — сбрасывает
			// счётчик повторов, так как это новая попытка, не связанная с предыдущей серией неудач.
			public void Schedule()
			{
				_retryCount = 0;
				ScheduleCore();
			}

			// Вызывается только из UpdateTabSelectionOutline, когда контейнер закладки временно
			// не готов сразу после Show()/Hide(). Ограничена числом попыток.
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

		// Контур вокруг активной закладки и поля контента — это одна непрерывная фигура "флажком"
		// (закладка обычно уже области контента), а не два отдельных прямоугольника. Форма зависит
		// от ширины и позиции активной закладки среди соседей, поэтому пересчитывается каждый раз,
		// когда меняется выбор закладки или размер панели — обычный статичный Border её не нарисует.
		// retryScheduler вызывается, если контейнер выбранной закладки ещё не пересоздан AvalonDock'ом
		// (например, сразу после Show()/Hide()) — пробуем пересчитать ещё раз на следующем тике.
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

				// Причина может быть как временной (сразу после Show()/Hide() контейнер выбранной
				// закладки ещё не пересоздан AvalonDock'ом — ItemContainerGenerator отстаёт на один
				// проход разметки), так и постоянной (панель целиком свёрнута в автоскрытие, и
				// SelectedItem может оставаться null, пока её не раскроют обратно). Различать эти
				// случаи здесь ненадёжно (например, IsVisible бывает временно false даже у панелей,
				// которые вообще не трогали, — отдельная попытка так и сделать чуть выше привела
				// к тому, что у них контур пропадал навсегда). Поэтому retryScheduler ограничен
				// числом попыток (см. OutlineUpdateScheduler.ScheduleRetry) — этого достаточно,
				// чтобы не зациклить диспетчер, и не важно, какая из причин сработала.
				retryScheduler();
				return;
			}

			const double radius = 5;

			Rect tabBounds = new Rect(selectedTab.TranslatePoint(new Point(0, 0), rootGrid), new Size(selectedTab.ActualWidth, selectedTab.ActualHeight));
			Rect contentBounds = new Rect(contentPanel.TranslatePoint(new Point(0, 0), rootGrid), new Size(contentPanel.ActualWidth, contentPanel.ActualHeight));

			// Каждый пересчёт создаёт новый объект PathGeometry, что инвалидирует Measure у Path
			// и может спровоцировать ещё один проход разметки (а значит — ещё один LayoutUpdated).
			// Если границы не изменились с прошлого раза, переприсваивать Data не нужно — это рвёт
			// потенциальный цикл инвалидации.
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
				WindowState = WindowState.Normal;
				Width = 1280;
				Height = 720;
				Left = (SystemParameters.PrimaryScreenWidth - Width) / 2;
				Top = (SystemParameters.PrimaryScreenHeight - Height) / 2;
				RestoreLayout(_defaultLayoutXml);
			};
			_viewModel.ShowWidgetRequested += (s, pane) => ShowWidget(pane);

			Loaded += MainWindow_Loaded;
			Closing += MainWindow_Closing;
			Closed += MainWindow_Closed;
		}

		private void MainWindow_Loaded(object sender, RoutedEventArgs e)
		{
			// Сохраняем исходную схему по умолчанию (раскладку, построенную PaneLayoutUpdateStrategy)
			_defaultLayoutXml = AvalonDockLayoutPersistence.Serialize(DockManager);

			// 1. Загружаем глобальную сессию (недавние проекты и т.п.)
			_viewModel.LoadSession();

			// 2. Загружаем локальный макет
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

			_viewModel.IsDirty = layoutState.IsDirty;
			RestoreLayout(layoutState.LayoutXml);

			// 3. Инициализируем рендер движка (через хендл RenderWidget)
			if (_viewModel.RenderPane.ViewContent is RenderWidget renderWidget)
			{
				_isEngineInitialized = EngineRuntime.TryInitialize(renderWidget.RenderHandle);
				if (_isEngineInitialized)
				{
					CompositionTarget.Rendering += CompositionTarget_Rendering;
				}
			}
		}

		private void RestoreLayout(string layoutXml)
		{
			// При отсутствии сохранённой раскладки (или сбое десериализации) остаётся раскладка по умолчанию
			AvalonDockLayoutPersistence.TryDeserialize(DockManager, layoutXml, _viewModel.Panes);
		}

		// AvalonDock не имеет MVVM-эквивалента для Show()/Float() — это единственное место,
		// где код-бихайнд напрямую обращается к дереву раскладки AvalonDock.
		private void ShowWidget(PaneViewModel pane)
		{
			var anchorable = DockManager.Layout.Descendents()
				.OfType<LayoutAnchorable>()
				.FirstOrDefault(a => ReferenceEquals(a.Content, pane));

			if (anchorable == null)
			{
				return;
			}

			// Show() сам возвращает панель в нужную статическую зону через PaneLayoutUpdateStrategy.
			// Float() здесь не нужен — он создаёт новое плавающее окно в координатах (0,0) экрана,
			// из-за чего панель визуально "не появлялась" для пользователя.
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

		private void CompositionTarget_Rendering(object? sender, EventArgs e)
		{
			if (_isEngineInitialized)
			{
				EngineRuntime.Tick();
			}
		}

		private void MainWindow_Closing(object? sender, System.ComponentModel.CancelEventArgs e)
		{
			if (!_viewModel.RequestClose())
			{
				e.Cancel = true;
				return;
			}

			var layoutState = new LayoutSessionState
			{
				Width = Width,
				Height = Height,
				Left = Left,
				Top = Top,
				IsMaximized = WindowState == WindowState.Maximized,
				IsDirty = _viewModel.IsDirty,
				LayoutXml = AvalonDockLayoutPersistence.Serialize(DockManager)
			};
			EditorSessionManager.SaveLayoutSession(layoutState);

			_viewModel.SaveSession();
		}

		private void MainWindow_Closed(object? sender, EventArgs e)
		{
			if (_isEngineInitialized)
			{
				CompositionTarget.Rendering -= CompositionTarget_Rendering;
				EngineRuntime.Shutdown();
			}
		}

		// SystemCommands.XxxWindowCommand — это лишь токены команд, без CommandBinding они ничего не делают
		private void MinimizeWindow_Executed(object sender, ExecutedRoutedEventArgs e) => SystemCommands.MinimizeWindow(this);
		private void MaximizeWindow_Executed(object sender, ExecutedRoutedEventArgs e) => SystemCommands.MaximizeWindow(this);
		private void RestoreWindow_Executed(object sender, ExecutedRoutedEventArgs e) => SystemCommands.RestoreWindow(this);
		private void CloseWindow_Executed(object sender, ExecutedRoutedEventArgs e) => SystemCommands.CloseWindow(this);
	}
}
