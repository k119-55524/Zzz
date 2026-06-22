using System.Collections.Generic;
using System.IO;
using System.Linq;
using AvalonDock;
using AvalonDock.Layout.Serialization;
using editor.ViewModels;

namespace editor.Services
{
    public static class AvalonDockLayoutPersistence
    {
        public static string Serialize(DockingManager dockManager)
        {
            try
            {
                var serializer = new XmlLayoutSerializer(dockManager);
                using var writer = new StringWriter();
                serializer.Serialize(writer);
                return writer.ToString();
            }
            catch
            {
                return string.Empty;
            }
        }

        public static bool TryDeserialize(DockingManager dockManager, string layoutXml, IEnumerable<PaneViewModel> panes)
        {
            if (string.IsNullOrEmpty(layoutXml))
            {
                return false;
            }

            try
            {
                var serializer = new XmlLayoutSerializer(dockManager);
                serializer.LayoutSerializationCallback += (s, args) =>
                {
                    var pane = panes.FirstOrDefault(p => p.ContentId == args.Model.ContentId);
                    if (pane != null)
                    {
                        args.Content = pane;
                    }
                };

                using var reader = new StringReader(layoutXml);
                serializer.Deserialize(reader);
                return true;
            }
            catch
            {
                return false;
            }
        }
    }
}
