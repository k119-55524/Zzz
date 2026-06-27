using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace editor.Views.Widgets
{
    public partial class InspectorWidget : UserControl
    {
        public InspectorWidget()
        {
            InitializeComponent();
        }

        private void ClearSearch_Click(object sender, RoutedEventArgs e)
        {
            var button = sender as DependencyObject;
            while (button != null && !(button is TextBox))
            {
                button = VisualTreeHelper.GetParent(button);
            }
            if (button is TextBox textBox)
            {
                textBox.Text = string.Empty;
                textBox.Focus();
            }
        }
    }
}
