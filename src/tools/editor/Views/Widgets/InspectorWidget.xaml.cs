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

        // Применяет значение поля по Enter, не дожидаясь потери фокуса (AcceptsReturn=False,
        // так что Enter иначе ничего не делает - поле однострочное).
        private void ValueTextBox_PreviewKeyDown(object sender, System.Windows.Input.KeyEventArgs e)
        {
            if (e.Key == System.Windows.Input.Key.Enter && sender is TextBox textBox)
            {
                textBox.GetBindingExpression(TextBox.TextProperty)?.UpdateSource();
                e.Handled = true;
            }
        }
    }
}
