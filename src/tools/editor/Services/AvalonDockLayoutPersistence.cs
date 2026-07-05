
using System.IO;
using AvalonDock;
using editor.ViewModels;
using AvalonDock.Layout;
using AvalonDock.Layout.Serialization;

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
					string contentId = args.Model.ContentId;
					if (contentId == "RenderWidget")
					{
						contentId = "WorldWidget";
					}
					var pane = panes.FirstOrDefault(p => p.ContentId == contentId);
					if (pane != null)
					{
						args.Content = pane;
						if (args.Model is LayoutAnchorable anchorable)
						{
							var titleBinding = new System.Windows.Data.Binding("Title")
							{
								Source = pane,
								Mode = System.Windows.Data.BindingMode.OneWay
							};
							System.Windows.Data.BindingOperations.SetBinding(anchorable, LayoutAnchorable.TitleProperty, titleBinding);
						}
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
