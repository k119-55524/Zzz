using System.Windows;

namespace editor.Views
{
	public partial class AboutDialog : Window
	{
		public AboutDialog()
		{
			InitializeComponent();
			TxtVersion.Text = $"Версия: {EditorConstants.Version}";
			TxtBuildDate.Text = $"Дата сборки: {EditorConstants.BuildDate}";
		}

		private void OkButton_Click(object sender, RoutedEventArgs e)
		{
			DialogResult = true;
			Close();
		}
	}
}
