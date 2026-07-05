using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace editor.Views.Converters
{
    public class InverseBooleanToVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            bool flag = false;
            if (value is bool b)
            {
                flag = b;
            }
            return flag ? Visibility.Collapsed : Visibility.Visible;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is Visibility vis)
            {
                return vis != Visibility.Visible;
            }
            return false;
        }
    }
}
