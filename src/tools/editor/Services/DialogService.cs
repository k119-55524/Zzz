
using System.Windows;

namespace editor.Services
{
	public class DialogService : IDialogService
	{
		public MessageBoxResult ShowMessage(string message, string title, MessageBoxButton buttons, MessageBoxImage icon)
		{
			return MessageBox.Show(message, title, buttons, icon);
		}

		public void ShowAbout()
		{
			var aboutDialog = new editor.Views.AboutDialog
			{
				Owner = Application.Current?.MainWindow
			};
			aboutDialog.ShowDialog();
		}
	}
}
