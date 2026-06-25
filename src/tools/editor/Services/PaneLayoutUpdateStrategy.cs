
using editor.Models;
using editor.ViewModels;
using AvalonDock.Layout;

namespace editor.Services
{
	public class PaneLayoutUpdateStrategy : ILayoutUpdateStrategy
	{
		public const string LeftPaneName = "PaneLeft";
		public const string LeftBottomPaneName = "PaneLeftBottom";
		public const string CenterPaneName = "PaneCenterTop";
		public const string BottomPaneName = "PaneBottom";
		public const string RightPaneName = "PaneRight";

		public bool BeforeInsertAnchorable(LayoutRoot layout, LayoutAnchorable anchorableToShow, ILayoutContainer destinationContainer)
		{
			if (anchorableToShow.Content is not PaneViewModel pane)
			{
				return false;
			}

			var titleBinding = new System.Windows.Data.Binding("Title")
			{
				Source = pane,
				Mode = System.Windows.Data.BindingMode.OneWay
			};
			System.Windows.Data.BindingOperations.SetBinding(anchorableToShow, LayoutAnchorable.TitleProperty, titleBinding);
			anchorableToShow.ContentId = pane.ContentId;
			anchorableToShow.CanClose = pane.CanClose;
			anchorableToShow.PropertyChanged += (s, args) =>
			{
				if (args.PropertyName == nameof(LayoutAnchorable.IsFloating))
				{
					bool isFloating = anchorableToShow.IsFloating;
					anchorableToShow.CanAutoHide = !isFloating;
				}
			};
			anchorableToShow.CanAutoHide = !anchorableToShow.IsFloating;

			string targetPaneName = pane.Type switch
			{
				WidgetType.SceneTree => LeftPaneName,
				WidgetType.World => CenterPaneName,
				WidgetType.Game => CenterPaneName,
				WidgetType.Settings => CenterPaneName,
				WidgetType.Build => CenterPaneName,
				WidgetType.Assets => LeftBottomPaneName,
				WidgetType.Console => BottomPaneName,
				WidgetType.Inspector => RightPaneName,
				_ => CenterPaneName
			};

			var targetPane = layout.Descendents().OfType<LayoutAnchorablePane>().FirstOrDefault(p => p.Name == targetPaneName);
			if (targetPane == null)
			{
				return false;
			}

			targetPane.Children.Add(anchorableToShow);
			return true;
		}

		public void AfterInsertAnchorable(LayoutRoot layout, LayoutAnchorable anchorableShown)
		{
		}

		public bool BeforeInsertDocument(LayoutRoot layout, LayoutDocument anchorableToShow, ILayoutContainer destinationContainer) => false;

		public void AfterInsertDocument(LayoutRoot layout, LayoutDocument anchorableShown)
		{
		}
	}
}
