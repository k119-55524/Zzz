
using System.Windows.Controls;

namespace editor.Views.Widgets
{
	public partial class AssetsWidget : UserControl
	{
		public AssetsWidget()
		{
			InitializeComponent();
		}

		private void TreeView_SelectedItemChanged(object sender, System.Windows.RoutedPropertyChangedEventArgs<object> e)
		{
			App.SelectionService.SelectedItem = e.NewValue;
		}
	}
}
