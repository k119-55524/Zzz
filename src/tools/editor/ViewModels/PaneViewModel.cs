using System;
using System.Windows;
using editor.Models;

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
        }

        public WidgetType Type { get; }

        public string ContentId { get; }

        public string Title { get; }

        public bool CanClose { get; }
    }
}
