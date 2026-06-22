using System.Linq;
using AvalonDock.Layout;
using editor.Models;
using editor.ViewModels;

namespace editor.Services
{
    // Размещает панели, приходящие из AnchorablesSource, в нужную статическую зону раскладки
    // (воспроизводит расположение, ранее заданное прямо в XAML).
    public class PaneLayoutUpdateStrategy : ILayoutUpdateStrategy
    {
        public const string LeftPaneName = "PaneLeft";
        public const string CenterPaneName = "PaneCenterTop";
        public const string BottomPaneName = "PaneBottom";
        public const string RightPaneName = "PaneRight";

        public bool BeforeInsertAnchorable(LayoutRoot layout, LayoutAnchorable anchorableToShow, ILayoutContainer destinationContainer)
        {
            if (anchorableToShow.Content is not PaneViewModel pane)
            {
                return false;
            }

            // AvalonDock сам проксирует Title/ContentId/IsSelected/IsActive/CanClose со своей модели
            // на сгенерированный LayoutAnchorableItem — задаём их здесь на модели, а не через Style/Binding
            // (см. AvalonDock.Controls.LayoutItem.SetDefaultBindings), иначе конкурирующий Binding уходит
            // в бесконечную рекурсию SetValue.
            anchorableToShow.Title = pane.Title;
            anchorableToShow.ContentId = pane.ContentId;
            anchorableToShow.CanClose = pane.CanClose;

            anchorableToShow.PropertyChanged += (s, args) =>
            {
                if (args.PropertyName == nameof(LayoutAnchorable.IsFloating))
                {
                    anchorableToShow.CanAutoHide = !anchorableToShow.IsFloating;
                }
            };
            anchorableToShow.CanAutoHide = !anchorableToShow.IsFloating;

            string targetPaneName = pane.Type switch
            {
                WidgetType.SceneTree => LeftPaneName,
                WidgetType.Render => CenterPaneName,
                WidgetType.Settings => CenterPaneName,
                WidgetType.Build => CenterPaneName,
                WidgetType.Assets => BottomPaneName,
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
