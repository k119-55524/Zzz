using System;
using System.Windows.Controls;
using editor.Models;

namespace editor.Views.Widgets
{
    public partial class RenderWidget : UserControl
    {
        public IntPtr RenderHandle => ViewHost.Handle;

        public RenderWidget()
        {
            InitializeComponent();
        }
    }
}
