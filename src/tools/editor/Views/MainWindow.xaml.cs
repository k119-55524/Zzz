
using System.Windows;
using System.Windows.Media;
using System.Runtime.InteropServices;

namespace editor;

public partial class MainWindow : Window
{
	[DllImport("editorDLL.dll", CallingConvention = CallingConvention.Cdecl)]
	private static extern bool Initialize(IntPtr hwnd);

	[DllImport("editorDLL.dll", CallingConvention = CallingConvention.Cdecl)]
	private static extern void Deinitialize();

	[DllImport("editorDLL.dll", CallingConvention = CallingConvention.Cdecl)]
	private static extern void Tick();

	public MainWindow()
	{
		InitializeComponent();
		this.Loaded += MainWindow_Loaded;
		this.Closed += MainWindow_Closed;
	}

	private void MainWindow_Loaded(object sender, RoutedEventArgs e)
	{
		// Передаем Handle нашего дочернего окна HwndHost в движок,
		// чтобы движок рисовал только в отведенной ему области, а не на всем окне.
		bool res = Initialize(ViewHost.Handle);
		if (res)
		{
			// Подписываемся на покадровое обновление от WPF
			CompositionTarget.Rendering += CompositionTarget_Rendering;
		}
	}

	private void CompositionTarget_Rendering(object? sender, EventArgs e)
	{
		Tick();
	}

	private void MainWindow_Closed(object? sender, EventArgs e)
	{
		CompositionTarget.Rendering -= CompositionTarget_Rendering;
		Deinitialize();
	}
}