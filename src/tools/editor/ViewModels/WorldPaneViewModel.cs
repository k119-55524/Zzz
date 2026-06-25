using editor.Models;
using editor.Views.Widgets;

namespace editor.ViewModels
{
    public class WorldPaneViewModel : PaneViewModel
    {
        private bool _isToolbarVisible;

        public object View { get; } = new WorldWidget();

        public WorldPaneViewModel() : base(WidgetType.World)
        {
        }

        public bool IsToolbarVisible
        {
            get => _isToolbarVisible;
            set => SetField(ref _isToolbarVisible, value);
        }
    }
}
