using System;
using System.Windows.Controls;

namespace editor.Views.Widgets
{
    public partial class GameWidget : UserControl
    {
        public IntPtr RenderHandle => ViewHost.Handle;

        public GameWidget()
        {
            InitializeComponent();
        }
    }
}
