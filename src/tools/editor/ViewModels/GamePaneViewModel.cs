using editor.Models;
using editor.Views.Widgets;

namespace editor.ViewModels
{
    public class GamePaneViewModel : PaneViewModel
    {
        private bool _isToolbarEnabled;

        public object View { get; } = new GameWidget();

        public GamePaneViewModel() : base(WidgetType.Game)
        {
        }

        public bool IsToolbarEnabled
        {
            get => _isToolbarEnabled;
            set => SetField(ref _isToolbarEnabled, value);
        }
    }
}
