using System.Windows;
using editor.ViewModels;

namespace editor.Views
{
	public partial class AboutDialog : Window
	{
		public AboutDialog()
		{
			InitializeComponent();

			var viewModel = new AboutDialogViewModel();
			viewModel.CloseRequested += (s, e) =>
			{
				DialogResult = true;
				Close();
			};
			DataContext = viewModel;
		}
	}
}
