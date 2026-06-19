using System;
using System.Runtime.InteropServices;
using System.Windows.Interop;

namespace editor
{
	public class EngineViewHost : HwndHost
	{
		// Константы стилей окна Win32 (WS_CHILD | WS_VISIBLE)
		internal const int WS_CHILD = 0x40000000;
		internal const int WS_VISIBLE = 0x10000000;
		internal const int HOST_ID = 0x00000002;

		[DllImport("user32.dll", EntryPoint = "CreateWindowEx", CharSet = CharSet.Unicode)]
		internal static extern IntPtr CreateWindowEx(
			int dwExStyle,
			string lpszClassName,
			string lpszWindowName,
			int style,
			int x, int y,
			int width, int height,
			IntPtr hwndParent,
			IntPtr hMenu,
			IntPtr hInst,
			[MarshalAs(UnmanagedType.AsAny)] object pvParam);

		[DllImport("user32.dll", EntryPoint = "DestroyWindow", CharSet = CharSet.Unicode)]
		internal static extern bool DestroyWindow(IntPtr hwnd);

		protected override HandleRef BuildWindowCore(HandleRef hwndParent)
		{
			// Создаем дочернее окно Win32. 
			// "static" - это самый простой класс окна, он ничего не рисует сам,
			// что идеально для графического API, который будет рендерить поверх него.
			IntPtr hwndHost = CreateWindowEx(
				0,
				"static",
				"",
				WS_CHILD | WS_VISIBLE,
				0, 0,
				0, 0, // Размерами будет управлять WPF через Layout
				hwndParent.Handle,
				(IntPtr)HOST_ID,
				IntPtr.Zero,
				0);

			return new HandleRef(this, hwndHost);
		}

		protected override void DestroyWindowCore(HandleRef hwnd)
		{
			DestroyWindow(hwnd.Handle);
		}
	}
}
