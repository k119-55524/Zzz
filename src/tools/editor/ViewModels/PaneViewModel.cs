using System;
using System.Windows;
using editor.Models;

namespace editor.ViewModels
{
    public class PaneViewModel : ViewModelBase
    {
        private string _title = string.Empty;

        public PaneViewModel(WidgetType type)
        {
            Type = type;

            var meta = WidgetRules.GetMetadata(type);
            ContentId = meta.SystemName;
            CanClose = !meta.IsRequired;
            UpdateTitle();
        }

        public WidgetType Type { get; }

        public string ContentId { get; }

        public string Title
        {
            get => _title;
            set => SetField(ref _title, value);
        }

        public bool CanClose { get; }

        public virtual void UpdateTitle()
        {
            var meta = WidgetRules.GetMetadata(Type);
            Title = Application.Current?.TryFindResource(meta.TitleKey) as string ?? meta.TitleKey;
        }
    }
}
