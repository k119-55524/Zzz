using System;
using System.Windows.Controls;

namespace editor.Views.Widgets
{
    public partial class WorldWidget : UserControl
    {
        public IntPtr RenderHandle => ViewHost.Handle;

        public WorldWidget()
        {
            InitializeComponent();
        }
    }
}
