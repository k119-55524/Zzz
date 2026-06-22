using System;
using System.Windows;
using System.Windows.Controls;
using editor.Models;
using editor.Views.Widgets;

namespace editor.ViewModels
{
    public class PaneViewModel : ViewModelBase
    {
        public PaneViewModel(WidgetType type)
        {
            Type = type;

            var meta = WidgetRules.GetMetadata(type);
            ContentId = meta.SystemName;
            CanClose = !meta.IsRequired;
            Title = Application.Current?.TryFindResource(meta.TitleKey) as string ?? meta.TitleKey;
            ViewContent = CreateView(type);
        }

        public WidgetType Type { get; }

        public string ContentId { get; }

        public string Title { get; }

        public bool CanClose { get; }

        public UserControl ViewContent { get; }

        private static UserControl CreateView(WidgetType type)
        {
            return type switch
            {
                WidgetType.Render => new RenderWidget(),
                WidgetType.Inspector => new InspectorWidget(),
                WidgetType.SceneTree => new SceneTreeWidget(),
                WidgetType.Assets => new AssetsWidget(),
                WidgetType.Console => new ConsoleWidget(),
                WidgetType.Settings => new SettingsWidget(),
                WidgetType.Build => new BuildWidget(),
                _ => throw new ArgumentOutOfRangeException(nameof(type), type, null)
            };
        }
    }
}
