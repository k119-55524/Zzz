using System;
using System.Globalization;
using System.Windows.Data;
using editor.Models;
using editor.Views.Widgets;

namespace editor.Views.Converters
{
    public class WidgetTypeToViewConverter : IValueConverter
    {
        public object? Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is WidgetType type)
            {
                return type switch
                {
                    WidgetType.World => new WorldWidget(),
                    WidgetType.Game => new GameWidget(),
                    WidgetType.Inspector => new InspectorWidget(),
                    WidgetType.SceneTree => new SceneTreeWidget(),
                    WidgetType.Assets => new AssetsWidget(),
                    WidgetType.Console => new ConsoleWidget(),
                    WidgetType.Settings => new SettingsWidget(),
                    WidgetType.Build => new BuildWidget(),
                    _ => null
                };
            }
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
