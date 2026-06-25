using editor.Models;
using editor.Views.Widgets;

namespace editor.ViewModels
{
    public class WorldPaneViewModel : PaneViewModel
    {
        private bool _isToolbarEnabled;

        public object View { get; } = new WorldWidget();

        public WorldPaneViewModel() : base(WidgetType.World)
        {
        }

        public bool IsToolbarEnabled
        {
            get => _isToolbarEnabled;
            set => SetField(ref _isToolbarEnabled, value);
        }
    }
}
