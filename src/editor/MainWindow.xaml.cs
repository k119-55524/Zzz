using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using System.Runtime.InteropServices;
using System.Windows.Interop;

namespace editor;

/// <summary>
/// Interaction logic for MainWindow.xaml
/// </summary>
public partial class MainWindow : Window
{
    [DllImport("editorDLL.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern bool Initialize(IntPtr hwnd);

    [DllImport("editorDLL.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern void Deinitialize();

    public MainWindow()
    {
        InitializeComponent();
        this.Loaded += MainWindow_Loaded;
        this.Closed += MainWindow_Closed;
    }

    private void MainWindow_Loaded(object sender, RoutedEventArgs e)
    {
        var helper = new WindowInteropHelper(this);
        Initialize(helper.Handle);
    }

    private void MainWindow_Closed(object? sender, EventArgs e)
    {
        Deinitialize();
    }
}