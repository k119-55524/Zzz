using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace editor.Themes
{
	// Code-behind для DarkTheme.xaml - нужен только для обработчиков событий, используемых
	// внутри общих стилей (например, кнопка очистки в Style_SearchTextBox).
	public partial class DarkTheme : ResourceDictionary
	{
		public DarkTheme()
		{
			InitializeComponent();
		}

		private void ClearSearch_Click(object sender, RoutedEventArgs e)
		{
			DependencyObject current = sender as DependencyObject;
			while (current != null && !(current is TextBox))
			{
				current = VisualTreeHelper.GetParent(current);
			}

			if (current is TextBox textBox)
			{
				textBox.Text = string.Empty;
				textBox.Focus();
			}
		}
	}
}
