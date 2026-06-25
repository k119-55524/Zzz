using editor.Models;
using editor.Views.Widgets;

namespace editor.ViewModels
{
    public class GamePaneViewModel : PaneViewModel
    {
        private bool _isToolbarVisible;

        public object View { get; } = new GameWidget();

        public GamePaneViewModel() : base(WidgetType.Game)
        {
        }

        public bool IsToolbarVisible
        {
            get => _isToolbarVisible;
            set => SetField(ref _isToolbarVisible, value);
        }
    }
}
